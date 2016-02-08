#ifndef FIBONACCI_HEAP_H
#define	FIBONACCI_HEAP_H

#include <stdbool.h>
#include <stdlib.h>

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct fibonacci_heap fibonacci_heap;

    /***************************************************************************
    * Allocates a new empty Fibonacci heap.                                    *
    ***************************************************************************/  
    fibonacci_heap* 
    fibonacci_heap_alloc(size_t initial_capacity,
                         float  load_factor,
                         size_t (*hash_function)(void*),
                         bool   (*equals_function)(void*, void*),
                         int    (*priority_compare_function)(void*, void*));

    /***************************************************************************
    * Adds a new element and its priority to the heap only if it is not        *
    * already present. Returns true only if the input element was not already  * 
    * in the heap.                                                             *
    ***************************************************************************/  
    bool fibonacci_heap_add(fibonacci_heap* heap, 
                            void* element, 
                            void* priority);

    /***************************************************************************
    * Attempts to assign a higher priority to the element. Returns true only   *       
    * if the structure of the heap changed due to this call.                   * 
    ***************************************************************************/  
    bool fibonacci_heap_decrease_key(fibonacci_heap* heap, 
                                     void* element, 
                                     void* priority);

    /***************************************************************************
    * Returns true only if the element is in the heap.                         * 
    ***************************************************************************/  
    bool fibonacci_heap_contains_key(fibonacci_heap* heap, void* element);

    /***************************************************************************
    * Removes the highest priority element and returns it.                     * 
    ***************************************************************************/  
    void* fibonacci_heap_extract_min(fibonacci_heap* heap);

    /***************************************************************************
    * Returns the highest priority element without removing it.                * 
    ***************************************************************************/  
    void* fibonacci_heap_min(fibonacci_heap* heap);

    /***************************************************************************
    * Returns the size of this heap.                                           * 
    ***************************************************************************/  
    int fibonacci_heap_size(fibonacci_heap* heap);

    /***************************************************************************
    * Drops all the contents of the heap. Only internal structures are         *
    * deallocated; the user is responsible for memory-managing the contents.   * 
    ***************************************************************************/  
    void fibonacci_heap_clear(fibonacci_heap* heap);

    /***************************************************************************
    * Checks that the heap maintains the min-heap property.                    *
    ***************************************************************************/  
    bool fibonacci_heap_is_healthy(fibonacci_heap* heap);

    /***************************************************************************
    * Deallocates the entire heap with its internal structures. The client     *
    * programmer must, however, memory-manage the contents.                    * 
    ***************************************************************************/  
    void fibonacci_heap_free(fibonacci_heap* heap);

#ifdef	__cplusplus
}
#endif

#endif	/* FIBONACCI_HEAP_H */
