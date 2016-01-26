#include "fibonacci_heap.h"
#include "unordered_map.h"
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

static const double PHI = 1.618;
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
