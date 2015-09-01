#ifndef UNORDERED_MAP_H
#define	UNORDERED_MAP_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct unordered_map_t          unordered_map_t;
    typedef struct unordered_map_iterator_t unordered_map_iterator_t;
    
    /***************************************************************************
    * Allocates a new, empty map with given comparator function.               *
    ***************************************************************************/ 
    unordered_map_t* unordered_map_t_alloc (int (*p_comparator)(void*, void*),
                                            size_t initial_capacity,
                                            float load_factor);

    /***************************************************************************
    * If p_map contains the key p_key, associates it with value p_value and    *
    * returns the old value of that key.                                       *
    ***************************************************************************/ 
    void*  unordered_map_t_put             (unordered_map_t* p_map, 
                                            void* p_key, 
                                            void* p_value);

    /***************************************************************************
    * Returns a positive value if p_key is mapped to some value in this map.   *
    ***************************************************************************/
    int    unordered_map_t_contains_key    (unordered_map_t* p_map, 
                                            void* p_key);

    /***************************************************************************
    * Returns the value associated with the p_key, or NULL if p_key is not     *
    * mapped in the map.                                                       *
    ***************************************************************************/
    void*  unordered_map_t_get             (unordered_map_t* p_map, 
                                            void* p_key);

    /***************************************************************************
    * If p_key is mapped in the map, removes the mapping and returns the value *
    * of that mapping. If the map did not contain the mapping, return NULL.    *
    ***************************************************************************/ 
    void*  unordered_map_t_remove          (unordered_map_t* p_map, 
                                            void* p_key);

    /***************************************************************************
    * Removes all the contents of the map.                                     * 
    ***************************************************************************/ 
    void   unordered_map_t_clear           (unordered_map_t* p_map);

    /***************************************************************************
    * Returns the size of the map, or namely, the amount of key/value mappings *
    * in the map.                                                              *
    ***************************************************************************/ 
    int    unordered_map_t_size            (unordered_map_t* p_map);

    /***************************************************************************
    * Checks that the map maintains the AVL-tree invariant.                    *
    ***************************************************************************/  
    bool   unordered_map_t_is_healthy      (unordered_map_t* p_map);

    /***************************************************************************
    * Deallocates the entire map. Only the map and its nodes are deallocated.  *
    * The user is responsible to deallocate the actual data stored in the map. *
    ***************************************************************************/ 
    void   unordered_map_t_free            (unordered_map_t* p_map);

    /***************************************************************************
    * Returns the iterator over the map. The entries are iterated in order.    *
    ***************************************************************************/  
    unordered_map_iterator_t* unordered_map_iterator_t_alloc
                             (unordered_map_t* p_map);

    /***************************************************************************
    * Returns the number of keys not yet iterated over.                        *
    ***************************************************************************/ 
    int unordered_map_iterator_t_has_next(unordered_map_iterator_t* p_iterator);

    /***************************************************************************
    * Returns the next entry in the iteration order.                           *
    ***************************************************************************/  
    bool unordered_map_iterator_t_next(map_iterator_t* p_iterator, 
                                       void** pp_key, 
                                       void** pp_value);

    /***************************************************************************
    * Returns a positive integer if the map was modified during the iteration. *
    ***************************************************************************/  
    bool unordered_map_iterator_t_is_disturbed
        (unordered_map_iterator_t* p_iterator);

    /***************************************************************************
    * Deallocates the map iterator.                                            *
    ***************************************************************************/  
    void unordered_map_iterator_t_free(unordered_map_iterator_t* p_iterator);


#ifdef	__cplusplus
}
#endif

#endif	/* UNORDERED_MAP_H */

