#include "fibonacci_heap.h"
#include "unordered_map.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static const double LOG_PHI = 0.4813;
static const size_t DEFAULT_NODE_ARRAY_CAPACITY = 8;

typedef struct heap_node {
    void*             element;
    void*             priority;
    struct heap_node* parent;
    struct heap_node* left;
    struct heap_node* right;
    struct heap_node* child;
    size_t            degree;
    bool              marked;
} heap_node;

struct fibonacci_heap {
    unordered_map_t* node_map;
    heap_node*       minimum_node;
    heap_node**      node_array;
    size_t           node_array_capacity;
    size_t         (*hash_function)(void*);
    bool           (*equals_function)(void*, void*);
    int            (*key_compare_function)(void*, void*);
};

static heap_node* fibonacci_heap_node_alloc(void* element, void* priority) {
    heap_node* node = malloc(sizeof(heap_node));

    if (!node) 
    {
        return NULL;
    }

    node->element  = element;
    node->priority = priority;
    node->parent   = NULL;
    node->left     = node;
    node->right    = node;
    node->child    = NULL;
    node->degree   = 0U;
    node->marked   = false;

    return node;
}

static void fibonacci_heap_node_free(heap_node* node) 
{
    heap_node* child;
    heap_node* first_child;
    heap_node* sibling;

    child = node->child;

    if (!child) 
    {
        free(node);
        return;
    }

    first_child = child;

    while (true)
    {
        sibling = child->right;
        fibonacci_heap_node_free(child);
        child = sibling;

        if (child == first_child) 
        {
            break;
        }
    }

    free(node);
}

fibonacci_heap* 
fibonacci_heap_alloc(size_t map_initial_capacity,
                     float map_load_factor,
                     size_t (*hash_function)(void*),
                     bool (*equals_function)(void*, void*),
                     int (*key_compare_function)(void*, void*))
{
    fibonacci_heap* heap;

    if (!hash_function)       
    {
        return NULL;
    }

    if (!equals_function)     
    {
        return NULL;
    }

    if (!key_compare_function) 
    {
        return NULL;
    }

    heap = malloc(sizeof(fibonacci_heap));

    if (!heap) 
    {
        return NULL;
    }

    heap->node_array = malloc(sizeof(heap_node*) * DEFAULT_NODE_ARRAY_CAPACITY);

    if (!heap->node_array) 
    {
        free(heap);
        return NULL;
    }

    heap->node_array_capacity = DEFAULT_NODE_ARRAY_CAPACITY;
    heap->node_map = unordered_map_t_alloc(map_initial_capacity, 
                                           map_load_factor, 
                                           hash_function, 
                                           equals_function);

    if (!heap->node_map)
    {
        free(heap->node_array);
        free(heap);
        return NULL;
    }

    heap->minimum_node         = NULL;
    heap->hash_function        = hash_function;
    heap->equals_function      = equals_function;
    heap->key_compare_function = key_compare_function;

    return heap;
}

bool fibonacci_heap_add(fibonacci_heap* heap, void* element, void* priority) 
{
    heap_node* node;

    if (!heap) {
        return false;
    }

    if (unordered_map_t_contains_key(heap->node_map, element))
    {
        return false;
    }

    node = fibonacci_heap_node_alloc(element, priority);

    if (!node){
        return false;
    }

    if (heap->minimum_node)
    {
        node->left = heap->minimum_node;
        node->right = heap->minimum_node->right;
        heap->minimum_node->right = node;
        node->right->left = node;

        if (heap->key_compare_function(priority, 
                                       heap->minimum_node->priority) < 0) 
        {
            heap->minimum_node = node;
        }
    } 
    else 
    {
        heap->minimum_node = node;
    }

    unordered_map_t_put(heap->node_map, element, node);
    return true;
}

static void cut(fibonacci_heap* heap, heap_node* x, heap_node* y)
{
    x->left->right = x->right;
    x->right->left = x->left;
    y->degree--;

    if (y->child == x) 
    {
        y->child = x->right;
    }

    if (y->degree == 0) 
    {
        y->child = NULL;
    }

    x->left = heap->minimum_node;
    x->right = heap->minimum_node->right;
    heap->minimum_node->right = x;
    x->right->left = x;

    x->parent = NULL;
    x->marked = false;
}

static void cascading_cut(fibonacci_heap* heap, heap_node* y)
{
    heap_node* z = y->parent;

    if (z)
    {
        if (y->marked)
        {
            cut(heap, y, z);
            cascading_cut(heap, z);
        }
        else 
        {
            y->marked = true;
        }
    }
}

bool fibonacci_heap_decrease_key(fibonacci_heap* heap, 
                                 void* element, 
                                 void* priority)
{
    heap_node* x;
    heap_node* y;

    if (!heap)
    {
        return false;
    }

    x = unordered_map_t_get(heap->node_map, element);

    if (!x) 
    {
        return false;
    }

    if (heap->key_compare_function(x->priority, priority) <= 0)
    {
        /* Cannot improve priority of the input element. */
        return false;
    }

    x->priority = priority;
    y = x->parent;

    if (y && heap->key_compare_function(x->priority, y->priority) < 0) 
    {
        cut(heap, x, y);
        cascading_cut(heap, y);
    }

    if (heap->key_compare_function(x->priority, heap->minimum_node->priority) < 0)
    {
        heap->minimum_node = x;
    }

    return true;
}

static bool try_expand_array(fibonacci_heap* heap, size_t size)
{
    if (heap->node_array_capacity < size) 
    {
        free(heap->node_array);
        heap->node_array = malloc(sizeof(heap_node*) * size);

        if (!heap->node_array) 
        {
            return false;
        }

        heap->node_array_capacity = size;
        return true;
    } 
    else 
    {
        return true;
    }
}

static void link(heap_node* y, heap_node* x)
{
    y->left->right = y->right;
    y->right->left = y->left;

    y->parent = x;

    if (!x->child)
    {
        x->child = y;
        y->right = y;
        y->left = y;
    }
    else
    {
        y->left = x->child;
        y->right = x->child->right;
        x->child->right = y;
        y->right->left = y;
    }

    x->degree++;
    y->marked = false;
}

static void consolidate(fibonacci_heap* heap)
{
    size_t array_size = (size_t)(floor
                                    (log(unordered_map_t_size(heap->node_map)) 
                                     / LOG_PHI)) + 1;
    size_t     number_of_roots;
    size_t     degree;
    size_t     i;
    heap_node* x;
    heap_node* y;
    heap_node* tmp;
    heap_node* next;

    try_expand_array(heap, array_size);

    /* Set the internal node array components to NULL. */
    memset(heap->node_array, 0, array_size * sizeof(heap_node*));

    number_of_roots = 0;
    x = heap->minimum_node;

    if (x) 
    {
        ++number_of_roots;
        x = x->right;

        while (x != heap->minimum_node)
        {
            ++number_of_roots;
            x = x->right;
        }
    }

    while (number_of_roots > 0) 
    {
        degree = x->degree;
        next = x->right;

        while(true)
        {
            y = heap->node_array[degree];

            if (!y) break;

            if (heap->key_compare_function(x->priority, 
                                               y->priority) > 0) 
            {
                tmp = y;
                y = x;
                x = tmp;
            }

            link(y, x);
            heap->node_array[degree] = NULL;
            ++degree;
        }

        heap->node_array[degree] = x;
        x = next;
        --number_of_roots;
    }

    heap->minimum_node = NULL;

    for (i = 0; i < array_size; ++i) 
    {
        y = heap->node_array[i];

        if (!y)
        {
            continue;
        }

        if (heap->minimum_node) 
        {
            y->left->right = y->right;
            y->right->left = y->left;

            y->left = heap->minimum_node;
            y->right = heap->minimum_node->right;
            heap->minimum_node->right = y;
            y->right->left = y;

            if (heap->key_compare_function(
                   y->priority, 
                   heap->minimum_node->priority) < 0)
            {
                heap->minimum_node = y;
            }
        }
        else
        {
            heap->minimum_node = y;
        }
    }
}

void* fibonacci_heap_extract_min(fibonacci_heap* heap)
{
    heap_node* z;
    heap_node* x;
    heap_node* tmp_right;
    heap_node* node_to_free;

    void* p_ret;
    size_t number_of_children;

    if (!heap) 
    {
        return NULL;
    }

    z = heap->minimum_node;

    if (!z) 
    {
        return NULL; /* Heap is empty. */
    }

    number_of_children = z->degree;
    x = z->child;

    while (number_of_children > 0) 
    {
        tmp_right = x->right;

        x->left->right = x->right;
        x->right->left = x->left;

        x->left = heap->minimum_node;
        x->right = heap->minimum_node->right;
        heap->minimum_node->right = x;
        x->right->left = x;

        x->parent = NULL;
        x = tmp_right;
        --number_of_children;
    }

    z->left->right = z->right;
    z->right->left = z->left;

    p_ret = heap->minimum_node->element;

    if (z == z->right)
    {
        node_to_free = heap->minimum_node;
        heap->minimum_node = NULL;
    }
    else 
    {
        node_to_free = heap->minimum_node;
        heap->minimum_node = z->right;
        consolidate(heap);
    }

    unordered_map_t_remove(heap->node_map, p_ret);
    free(node_to_free);
    return p_ret;
}

bool fibonacci_heap_contains_key(fibonacci_heap* heap, void* element)
{
    if (!heap) 
    {
        return false;
    }

    return unordered_map_t_contains_key(heap->node_map, element);
}

void* fibonacci_heap_min(fibonacci_heap* heap)
{
    if (!heap)    
    {
        return NULL;
    }

    if (heap->minimum_node) 
    {
        return heap->minimum_node->element;
    }

    return NULL;
}

int fibonacci_heap_size(fibonacci_heap* heap)
{
    if (!heap)
    {
        return 0;
    }

    return unordered_map_t_size(heap->node_map);
}

void fibonacci_heap_clear(fibonacci_heap* heap)
{
    heap_node* current;
    heap_node* sibling;
    heap_node* first_root;

    if (!heap) 
    {
        return;
    }

    if (!heap->minimum_node)
    {
        return;
    }

    current = heap->minimum_node;
    first_root = current;

    while (true)
    {
        sibling = current->right;
        fibonacci_heap_node_free(current);
        current = sibling;

        if (current == first_root)
        {
            break;
        }
    }

    heap->minimum_node = NULL;
    unordered_map_t_clear(heap->node_map);
}

static bool tree_is_healthy(fibonacci_heap* heap, heap_node* node)
{
    heap_node* begin;

    if (!node) 
    {
        return true;
    }

    begin = node;

    while (true) 
    {
        if (heap->key_compare_function(node->priority, 
                                       node->parent->priority) < 0) 
        {
            return false;
        }

        if (!tree_is_healthy(heap, node)) 
        {
            return false;
        }

        begin = begin->right;

        if (begin == node) 
        {
            return false;
        }
    }

    return true;
}

static bool check_root_list(fibonacci_heap* heap)
{
    heap_node* current = heap->minimum_node;

    while (true)
    {
        if (heap->key_compare_function(current->priority,
                                       heap->minimum_node->priority) < 0) 
        {
            return false;
        }

        current = current->right;

        if (current == heap->minimum_node) 
        {
            return true;
        }
    }
}

bool fibonacci_heap_is_healthy(fibonacci_heap* heap)
{
    heap_node* root;

    if (!heap)
    {
        return false;
    }

    if (!heap->minimum_node) 
    {
        return true;
    }

    /* Check that in the root list, 'minimum_node' points to the node
       with largest priority. 
     */
    if (!check_root_list(heap))
    {
        return false;
    }

    root = heap->minimum_node;

    /* Check that all trees are min-heap ordered: the priority of any child is
     * not higher than the priority of its parent. */
    while (root)
    {
        if (!tree_is_healthy(heap, root->child))
        {
            return false;
        }

        root = root->right;

        if (root == heap->minimum_node)
        {
            return true;
        }
    }

    return false;
}

void fibonacci_heap_free(fibonacci_heap* heap)
{
    if (!heap) 
    {
        return;
    }

    if (heap->node_array) 
    {
        free(heap->node_array);
    }

    fibonacci_heap_clear(heap);

    if (heap->node_map)
    {
        unordered_map_t_free(heap->node_map);
    }

    free(heap);
}
