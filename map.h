#ifndef MAP_H
#define	MAP_H

#include <stdlib.h>

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct map_entry_t {
        void*               p_key;
        void*               p_value;
        struct map_entry_t* p_left;
        struct map_entry_t* p_right;
        struct map_entry_t* p_parent;
        int                 height;
    } map_entry_t;
    
    typedef struct map_t {
        map_entry_t* p_root;
        int (*p_comparator)(void*, void*);
        size_t size;
        size_t mod_count;
    } map_t;
    
    typedef struct map_iterator_t {
        map_t* p_map;
        map_entry_t p_next;
        size_t expected_mod_count;
    } map_iterator_t;

    /***************************************************************************
    * Allocates a new, empty map with given comparator function.               *
    ***************************************************************************/ 
    map_t* map_t_alloc(int (*p_comparator)(void*, void*));
    
    /***************************************************************************
    * If p_map contains the key p_key, associates it with value p_value and    *
    * returns the old value of that key.                                       *
    ***************************************************************************/ 
    void*  map_t_put          (map_t* p_map, void* p_key, void* p_value);
    
    /***************************************************************************
    * Returns a positive value if p_key is mapped to some value in this map.   *
    ***************************************************************************/
    int    map_t_contains_key (map_t* p_map, void* p_key);
    
    /***************************************************************************
    * Returns the value associated with the p_key, or NULL if p_key is not     *
    * mapped in the map.                                                       *
    ***************************************************************************/
    void*  map_t_get          (map_t* p_map, void* p_key);
    
    /***************************************************************************
    * If p_key is mapped in the map, removes the mapping and returns the value *
    * of that mapping. If the map did not contain the mapping, return NULL.    *
    ***************************************************************************/ 
    void*  map_t_remove       (map_t* p_map, void* p_key);
    
    /***************************************************************************
    * Returns the size of the map, or namely, the amount of key/value mappings *
    * in the map.                                                              *
    ***************************************************************************/ 
    int map_t_size         (map_t* p_map);
    
    /***************************************************************************
    * Checks that the map maintains the AVL-tree invariant.                    *
    ***************************************************************************/  
    int    map_t_is_healthy   (map_t* p_map);
    
    /***************************************************************************
    * Deallocates the entire map. Only the map and its nodes are deallocated.  *
    * The user is responsible to deallocate the actual data stored in the map. *
    ***************************************************************************/ 
    void   map_t_free         (map_t* p_map);
    
    /***************************************************************************
    * Returns the iterator over the map. The keys are iterated in order.       *
    ***************************************************************************/  
    map_iterator_t* map_t_iterator          (map_t* p_map);
    
    /***************************************************************************
    * Returns the number of keys not yet iterated over.                        *
    ***************************************************************************/ 
    int             map_iterator_t_has_next (map_iterator_t* p_iterator);
    
    /***************************************************************************
    * Returns the next key in the iteration order.                             *
    ***************************************************************************/  
    void*           map_iterator_t_next     (map_iterator_t* p_iterator);

#ifdef	__cplusplus
}
#endif

#endif	/* MAP_H */

