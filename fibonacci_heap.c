#include "fibonacci_heap.h"
#include "unordered_map.h"
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

static const double LOG_PHI = 0.4813;
static const size_t DEFAULT_NODE_ARRAY_CAPACITY = 16;

typedef struct fibonacci_heap_node_t {
    void*                         p_element;
    void*                         p_priority;
    struct fibonacci_heap_node_t* p_parent;
    struct fibonacci_heap_node_t* p_left;
    struct fibonacci_heap_node_t* p_right;
    struct fibonacci_heap_node_t* p_child;
    size_t                        degree;
    bool                          marked;
} fibonacci_heap_node_t;

typedef struct fibonacci_heap_t {
    unordered_map_t*        p_node_map;
    fibonacci_heap_node_t*  p_minimum_node;
    fibonacci_heap_node_t** p_node_array;
    size_t                  node_array_capacity;
    size_t                (*p_hash_function)(void*);
    bool                  (*p_equals_function)(void*, void*);
    int                   (*p_key_compare_function)(void*, void*);
} fibonacci_heap_t;

static fibonacci_heap_node_t* fibonacci_heap_node_t_alloc(void* p_element, 
                                                          void* p_priority) {
    fibonacci_heap_node_t* p_node;
    
    if (!p_element || !p_priority) 
    {
        return NULL;
    }
    
    p_node = malloc(sizeof(fibonacci_heap_node_t));
    
    if (!p_node) 
    {
        return NULL;
    }
    
    p_node->p_element  = p_element;
    p_node->p_priority = p_priority;
    p_node->p_parent   = NULL;
    p_node->p_left     = p_node;
    p_node->p_right    = p_node;
    p_node->p_child    = NULL;
    p_node->degree     = 0U;
    p_node->marked     = false;
    
    return p_node;
}

fibonacci_heap_t* 
fibonacci_heap_t_alloc(size_t map_initial_capacity,
                       float load_factor,
                       size_t (*p_hash_function)(void*),
                       bool (*p_equals_function)(void*, void*),
                       int (*p_key_compare_function)(void*, void*))
{
    fibonacci_heap_t* p_ret;
    
    if (!p_hash_function)        return NULL;
    if (!p_equals_function)      return NULL;
    if (!p_key_compare_function) return NULL;
    
    p_ret = malloc(sizeof(fibonacci_heap_t));
    
    if (!p_ret) return NULL;
    
    p_ret->p_node_array = malloc(sizeof(fibonacci_heap_node_t*) *
                                 DEFAULT_NODE_ARRAY_CAPACITY);
    
    if (!p_ret->p_node_array) 
    {
        free(p_ret);
        return NULL;
    }
    
    p_ret->node_array_capacity = DEFAULT_NODE_ARRAY_CAPACITY;
    p_ret->p_node_map = unordered_map_t_alloc(map_initial_capacity, 
                                              load_factor, 
                                              p_hash_function, 
                                              p_equals_function);
    
    if (!p_ret->p_node_map)
    {
        free(p_ret->p_node_array);
        free(p_ret);
        return NULL;
    }
    
    p_ret->p_minimum_node         = NULL;
    p_ret->p_hash_function        = p_hash_function;
    p_ret->p_equals_function      = p_equals_function;
    p_ret->p_key_compare_function = p_key_compare_function;
    
    return p_ret;
}

bool fibonacci_heap_t_add(fibonacci_heap_t* p_heap, 
                          void* p_element, 
                          void* p_priority) 
{
    fibonacci_heap_node_t* p_node;
    
    if (!p_heap) return false;
    if (unordered_map_t_contains_key(p_heap->p_node_map, 
                                     p_element)) return false;
    
    p_node = fibonacci_heap_node_t_alloc(p_element, p_priority);
    
    if (!p_node) return false;
    
    if (p_heap->p_minimum_node)
    {
        p_node->p_left = p_heap->p_minimum_node;
        p_node->p_right = p_heap->p_minimum_node->p_right;
        p_heap->p_minimum_node->p_right = p_node;
        p_node->p_right->p_left = p_node;
        
        if (p_heap->p_key_compare_function(p_priority, 
                                           p_heap->p_minimum_node->p_priority)) 
        {
            p_heap->p_minimum_node = p_node;
        }
    } 
    else p_heap->p_minimum_node = p_node;
    
    unordered_map_t_put(p_heap->p_node_map, p_element, p_node);
    return true;
}

static bool check_array(fibonacci_heap_t* p_heap, size_t size)
{
    if (p_heap->node_array_capacity < size) 
    {
        free(p_heap->p_node_array);
        p_heap->p_node_array = malloc(sizeof(fibonacci_heap_t*) * size);
        
        if (!p_heap->p_node_array) 
        {
            return false;
        }
        
        p_heap->node_array_capacity = size;
        return true;
    } 
    else 
    {
        return true;
    }
}

static void consolidate(fibonacci_heap_t* p_heap)
{
    size_t array_size = 
            (size_t)(floor(log(unordered_map_t_size(p_heap)) / LOG_PHI)) + 1;
    size_t number_of_roots;
    size_t degree;
    size_t i;
    fibonacci_heap_node_t* x;
    fibonacci_heap_node_t* y;
    fibonacci_heap_node_t* tmp;
    fibonacci_heap_node_t* next;
    
    check_array(p_heap, array_size);
    
    /* Set the internal node array components to NULL. */
    memset(p_heap->p_node_array, 
           0, 
           array_size * sizeof(fibonacci_heap_node_t*));
    
    number_of_roots = 0;
    x = p_heap->p_minimum_node;
    
    if (x) 
    {
        ++number_of_roots;
        x = x->p_right;
        
        while (x != p_heap->p_minimum_node)
        {
            ++number_of_roots;
            x = x->p_right;
        }
    }
    
    while (number_of_roots > 0) 
    {
        degree = x->degree;
        next = x->p_right;
        
        while(true)
        {
            y = p_heap->p_node_array[degree];
            
            if (!y) break;
            
            if (p_heap->p_key_compare_function(x->p_priority, 
                                               y->p_priority) > 0) 
            {
                tmp = y;
                y = x;
                x = tmp;
            }
            
            link(y, x);
            p_heap->p_node_array[degree] = NULL;
            ++degree;
        }
        
        p_heap->p_node_array[degree] = x;
        x = next;
        --number_of_roots;
    }
    
    p_heap->p_minimum_node = NULL;
    
    for (i = 0; i < array_size; ++i) 
    {
        y = p_heap->p_node_array[i];
        
        if (!y) continue;
        
        if (p_heap->p_minimum_node) 
        {
            y->p_left->p_right = y->p_right;
            y->p_right->p_left = y->p_left;
            
        }
        else
        {
            p_heap->p_minimum_node = y;
        }
    }
}

void* fibonacci_heap_t_extract_min(fibonacci_heap_t* p_heap)
{
    fibonacci_heap_node_t* z;
    fibonacci_heap_node_t* x;
    fibonacci_heap_node_t* tmp_right;
    void* p_ret;
    size_t number_of_children;
    
    if (!p_heap) return NULL;
    
    z = p_heap->p_minimum_node;
    
    if (!z) return NULL; /* Heap is empty. */
    
    number_of_children = z->degree;
    x = z->p_child;

    while (number_of_children > 0) 
    {
        tmp_right = x->p_right;

        x->p_left->p_right = x->p_right;
        x->p_right->p_left = x->p_left;

        x->p_left = p_heap->p_minimum_node;
        x->p_right = p_heap->p_minimum_node->p_right;
        p_heap->p_minimum_node->p_right = x;
        x->p_right->p_left = x;

        x->p_parent = NULL;
        x = tmp_right;
        --number_of_children;
    }
        
    z->p_left->p_right = z->p_right;
    z->p_right->p_left = z->p_left;
    
    p_ret = p_heap->p_minimum_node->p_element;
    
    if (z == z->p_right)
    {
        p_heap->p_minimum_node = NULL;
    }
    else 
    {
        p_heap->p_minimum_node = z->p_right;
        consolidate(p_heap);
    }
    
    unordered_map_t_remove(p_heap->p_node_map, p_ret);
    return p_ret;
}

void fibonacci_heap_t_free(fibonacci_heap_t* p_heap)
{
    if (!p_heap) 
    {
        return;
    }
    
    if (p_heap->p_node_array) 
    {
        free(p_heap->p_node_array);
    }
    
    if (p_heap->p_node_map)
    {
        unordered_map_t_free(p_heap->p_node_map);
    }
    
    free(p_heap);
}

