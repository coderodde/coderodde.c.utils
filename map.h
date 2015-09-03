#ifndef MAP_H
#define	MAP_H

#include <stdlib.h>
#include <stdbool.h>

#ifdef	__cplusplus
extern "C" {
#endif
    
    typedef struct map_t map_t;
    typedef struct map_iterator_t map_iterator_t;

    /***************************************************************************
    * Allocates a new, empty map with given comparator function.               *
    ***************************************************************************/ 
    map_t* map_t_alloc        (int (*p_comparator)(void*, void*));

    /***************************************************************************
    * If the map does not contain the key, inserts it in the map and associates*
    * the value with it, returning NULL. Otherwise, the value is updated and   *
    * the old value is returned.                                               *  
    ***************************************************************************/ 
    void*  map_t_put          (map_t* p_map, void* p_key, void* p_value);

    /***************************************************************************
    * Returns true if the key is mapped in the map.                            *
    ***************************************************************************/
    bool   map_t_contains_key (map_t* p_map, void* p_key);

    /***************************************************************************
    * Returns the value associated with the key, or NULL if the key is not     *
    * mapped in the map.                                                       *
    ***************************************************************************/
    void*  map_t_get          (map_t* p_map, void* p_key);

    /***************************************************************************
    * If the key is mapped in the map, removes the mapping and returns the value*
    * of that mapping. If the map did not contain the mapping, returns NULL.   *
    ***************************************************************************/ 
    void*  map_t_remove       (map_t* p_map, void* p_key);

    /***************************************************************************
    * Removes all the contents of the map. Deallocates the map structures. The *
    * client user is responsible for deallocating the actual contents.         *  
    ***************************************************************************/ 
    void   map_t_clear        (map_t* p_map);

    /***************************************************************************
    * Returns the size of the map, or namely, the amount of key/value mappings *
    * in the map.                                                              *
    ***************************************************************************/ 
    size_t map_t_size         (map_t* p_map);

    /***************************************************************************
    * Checks that the map maintains the AVL-tree property.                     *
    ***************************************************************************/  
    bool   map_t_is_healthy   (map_t* p_map);

    /***************************************************************************
    * Deallocates the entire map. Only the map and its nodes are deallocated.  *
    * The user is responsible for deallocating the actual data stored in the   * 
    * map.                                                                     *
    ***************************************************************************/ 
    void   map_t_free         (map_t* p_map);

    /***************************************************************************
    * Returns the iterator over the map. The entries are iterated in order.    *
    ***************************************************************************/  
    map_iterator_t* map_iterator_t_alloc        (map_t* p_map);

    /***************************************************************************
    * Returns the number of keys not yet iterated over.                        *
    ***************************************************************************/ 
    size_t          map_iterator_t_has_next     (map_iterator_t* p_iterator);

    /***************************************************************************
    * Loads the next mapping in the iteration order.                           *
    ***************************************************************************/  
    bool            map_iterator_t_next         (map_iterator_t* p_iterator, 
                                                 void** pp_key, 
                                                 void** pp_value);

    /***************************************************************************
    * Returns a true if the map was modified during the iteration.             *
    ***************************************************************************/  
    bool            map_iterator_t_is_disturbed (map_iterator_t* p_iterator);

    /***************************************************************************
    * Deallocates the map iterator.                                            *
    ***************************************************************************/  
    void            map_iterator_t_free         (map_iterator_t* p_iterator);

#ifdef	__cplusplus
}
#endif

#endif	/* MAP_H */

