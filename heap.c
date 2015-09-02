#include "heap.h"
#include "unordered_map.h"

typedef struct heap_node_t {
    void*  p_element;
    void*  p_priority;
    size_t index;
} heap_node_t;

typedef struct heap_t {
    unordered_map_t* p_node_map;
    heap_node_t**    p_table;
    size_t         (*p_hash_function)(void*);
    bool           (*p_equals_function)(void*, void*);
    int            (*p_key_compare_function)(void*, void*);
    size_t           size;
    size_t           capacity;
    size_t           degree;
    size_t*          p_indices;
} heap_t;

static heap_node_t* heap_node_t_alloc(void* p_element, void* p_priority) 
{
    heap_node_t* p_ret = malloc(sizeof(*p_ret));
    
    if (!p_ret) return NULL;
    
    p_ret->p_element = p_element;
    p_ret->p_priority = p_priority;
    
    return p_ret;
}

static const size_t MINIMUM_CAPACITY = 16;

static int fix_degree(size_t degree) 
{
    return degree < 2 ? 2 : degree;
}

static size_t fix_initial_capacity(size_t initial_capacity) 
{
    return initial_capacity < MINIMUM_CAPACITY ? 
        MINIMUM_CAPACITY : 
        initial_capacity;
}

heap_t* heap_t_alloc(size_t degree,
                     size_t initial_capacity,
                     float load_factor,
                     size_t (*p_hash_function)(void*),
                     bool (*p_equals_function)(void*, void*),
                     int (*p_key_compare_function)(void*, void*))
{
    heap_t* p_ret;
    unordered_map_t* p_map;
    
    if (!p_hash_function)        return NULL;
    if (!p_equals_function)      return NULL;
    if (!p_key_compare_function) return NULL;
    
    p_ret = malloc(sizeof(*p_ret));
    
    if (!p_ret) return NULL;
    
    p_map = unordered_map_t_alloc(initial_capacity,
                                  load_factor,
                                  p_hash_function,
                                  p_equals_function);
    
    if (!p_map) 
    {
        free(p_ret);
        return NULL;
    }
    
    degree = fix_degree(degree);
    initial_capacity = fix_initial_capacity(initial_capacity);
    
    p_ret->p_table = malloc(sizeof(heap_node_t*) * initial_capacity);
    
    if (!p_ret->p_table) 
    {
        unordered_map_t_free(p_map);
        free(p_ret);
        return NULL;
    }
    
    p_ret->p_indices = malloc(sizeof(size_t) * degree);
    
    if (!p_ret->p_indices) 
    {
        unordered_map_t_free(p_map);
        free(p_ret->p_table);
        free(p_ret);
        return NULL;
    }
    
    p_ret->p_node_map             = p_map;
    p_ret->capacity               = initial_capacity;
    p_ret->size                   = 0;
    p_ret->degree                 = degree;
    p_ret->p_hash_function        = p_hash_function;
    p_ret->p_equals_function      = p_equals_function;
    p_ret->p_key_compare_function = p_key_compare_function;
    
    return p_ret;
}

static size_t get_parent_index(heap_t* p_heap, size_t child_index)
{
    return (index - 1) / p_heap->degree;
}

static void sift_up(heap_t* p_heap, size_t index)
{
    size_t parent_index;
    heap_node_t* p_target_node;
    heap_node_t* p_parent_node;
    
    if (index == 0) return;
    
    parent_index = get_parent_index(index);
    p_target_node = p_heap->p_table[index];
    
    for (;;) 
    {
        p_parent_node = p_heap->p_table[parent_index];
        
        if (p_heap->p_key_compare_function(p_parent_node->p_priority,
                                           p_target_node->p_priority) > 0)
        {
            p_heap->p_table[index] = p_parent_node;
            p_parent_node->index = index;
            index = parent_index;
            parent_index = get_parent_index(p_heap, index);
        }
        else
        {
            break;
        }
    }
    
    p_heap->p_table[index] = p_target_node;
    p_target_node->index = index;
}

static void compute_children_indices(heap_t* p_heap, size_t index)
{
    size_t degree = p_heap->degree;
    size_t i;
    
    for (i = 0; i < degree; ++i)
    {
        p_heap->p_indices[i] = degree * index + i + 1;
        
        if (p_heap->p_indices[i] >= p_heap->size)
        {
            p_heap->p_indices[i] = (size_t) -1;
            return;
        }
    }
}

static void sift_down_root(heap_t* p_heap) 
{
    heap_node_t* p_target = p_heap->p_table[0];
    void* p_priority = p_target->p_priority;
    void* p_min_child_priority;
    void* p_tentative_priority;
    size_t min_child_index;
    size_t i;
    size_t degree = p_heap->degree;
    size_t index = 0;
    
    for (;;) 
    {
        p_min_child_priority = p_priority;
        min_child_index = -1; /* Very large value to denote "no children". */
        compute_children_indices(p_heap, index);
        
        for (i = 0; i < degree; ++i) 
        {
            if (p_heap->p_indices[i] == (size_t) -1)
            {
                break;
            }
            
            p_tentative_priority = p_heap->p_table[p_heap->p_indices[i]]
                                         ->p_priority;
            
            if (p_heap->p_key_compare_function(p_min_child_priority,
                                               p_tentative_priority) > 0)
            {
                p_min_child_priority = p_tentative_priority;
                min_child_index = p_heap->p_indices[i];
            }
        }
        
        if (min_child_index == (size_t) -1)
        {
            p_heap->p_table[index] = p_target;
            p_target->index = index;
            return;
        }
        
        p_heap->p_table[index] = p_heap->p_table[min_child_index];
        p_heap->p_table[index]->index = index;
        
        index = min_child_index;
    }
}

/***************************************************************************
* Adds a new element and its priority to the heap only if it is not        *
* already present.                                                         *
***************************************************************************/  
bool heap_t_add(heap_t* p_heap, void* p_element, void* p_priority)
{
    heap_node_t* p_node;
    
    if (!p_heap) return false;
    
    /* Already in the heap? */
    if (unordered_map_t_contains_key(p_heap->p_node_map, p_element)) 
        return false; 
    
    p_node = heap_node_t_alloc(p_element, p_priority);
    
    if (!p_node) return false;
    
    p_node->index = p_heap->size;
    p_heap->p_table[p_heap->size] = p_node;
    unordered_map_t_put(p_heap->p_node_map, p_element, p_node);
    sift_up(p_heap, p_heap->size);
    p_heap->size++;
    return true;
}

/***************************************************************************
* Attempts to assign a higher priority to the 'element'. Return true only  *       
* if the structure of the heap changed due to this call.                   * 
***************************************************************************/  
bool heap_t_decrease_key(heap_t* p_heap, void* p_element, void* p_priority);

/***************************************************************************
* Return true only if 'p_element' is in the heap.                          * 
***************************************************************************/  
bool heap_t_contains_key(heap_t* p_heap, void* p_element);

/***************************************************************************
* Removes the highest priority element and returns it.                     * 
***************************************************************************/  
void* heap_t_extract_min(heap_t* p_heap);

/***************************************************************************
* Returns the highest priority element without removing it.                * 
***************************************************************************/  
void* heap_t_min(heap_t* p_heap);

/***************************************************************************
* Returns the size of this heap.                                           * 
***************************************************************************/  
size_t heap_t_size(heap_t* p_heap);

/***************************************************************************
* Drops all the contents of the heap. Only internal structures are         *
* deallocated; the user is responsible for memory-managing the contents.   * 
***************************************************************************/  
void heap_t_clear(heap_t* p_heap);
