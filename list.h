#ifndef LIST_H
#define	LIST_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct list_t list_t;
    
    /***************************************************************************
    * Allocates the new, empty list with initial capacity.                     *
    ***************************************************************************/  
    list_t* list_t_alloc(size_t initial_capacity);
    
    /***************************************************************************
    * Inserts the element to in front of the head of the list. Returns true if *
    * operation was successful.                                                *
    ***************************************************************************/  
    bool    list_t_push_front(list_t* p_list, void* p_element);
    
    /***************************************************************************
    * Appends the element to the tail of the list. Returns true if operation   *
    * was successful.                                                          * 
    ***************************************************************************/  
    bool    list_t_push_back(list_t* p_list, void* p_element);
    
    /***************************************************************************
    * Inserts the element into the list before index'th element. Returns true  *
    * if operation was successful.                                             * 
    ***************************************************************************/  
    bool    list_t_insert(list_t* p_list, size_t index, void* p_element);
    
    /***************************************************************************
    * Returns the index'th element of the list. Returns NULL if the index is   *
    * out of range.                                                            * 
    ***************************************************************************/  
    void*   list_t_get(list_t* p_list, size_t index);
    
    /***************************************************************************
    * Sets the index'th element of the list. Returns the old value. If the     *
    * index is out of range, returns NULL.                                     * 
    ***************************************************************************/  
    void*   list_t_set(list_t* p_list, size_t index, void* p_new_value);
    
    /***************************************************************************
    * Removes and returns the front element of the list. If list is empty,     *
    * returns NULL.                                                            * 
    ***************************************************************************/  
    void*   list_t_pop_front(list_t* p_list);
    
    /***************************************************************************
    * Removes and returns the last element of the list. If list is empty,      *
    * returns NULL.                                                            *
    ***************************************************************************/  
    void*   list_t_pop_back(list_t* p_list);
    
    /***************************************************************************
    * Removes the element at index 'index' from the list and returns the       *
    * it. If the list is empty or the index is out of range, returns NULL.     * 
    ***************************************************************************/  
    void*   list_t_remove_at(list_t* p_list, size_t index);
    
    /***************************************************************************
    * Returns true if the list contains the specified element using the        *
    * equality function. Returns false otherwise.                              * 
    ***************************************************************************/  
    bool    list_t_contains(list_t* p_list, 
                            void* p_element,
                            bool (*p_equals_function)(void*, void*));
    
    /***************************************************************************
    * Clears this list. The client programmer is responsible for memory-       *
    * managing the contents.                                                   * 
    ***************************************************************************/  
    void    list_t_clear(list_t* p_list);
    
    /***************************************************************************
    * Clears and deallocates the list.                                         *
    ***************************************************************************/  
    void    list_t_free(list_t* p_list);

#ifdef	__cplusplus
}
#endif

#endif	/* LIST_H */

