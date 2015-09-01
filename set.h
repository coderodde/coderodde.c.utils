#ifndef SET_H
#define	SET_H

#include <stdlib.h>
#include <stdbool.h>

#ifdef	__cplusplus
extern "C" {
#endif
    
    typedef struct set_t set_t;
    typedef struct set_iterator_t set_iterator_t;

    /***************************************************************************
    * Allocates a new, empty set with given comparator function.               *
    ***************************************************************************/ 
    set_t* set_t_alloc        (int (*p_comparator)(void*, void*));

    /***************************************************************************
    * Adds 'p_element' to the set if not already there. Returns true if the    *
    * structure of the tree changed.                                           * 
    ***************************************************************************/ 
    bool  set_t_add          (set_t* p_set, void* p_element);

    /***************************************************************************
    * Returns a true if the set contains the element.                          *
    ***************************************************************************/
    bool  set_t_contains     (set_t* p_set, void* p_element);

    /***************************************************************************
    * If 'p_element' is in 'p_set', removes it and returns true.               * 
    ***************************************************************************/ 
    bool  set_t_remove       (set_t* p_set, void* p_element);

    /***************************************************************************
    * Removes all the contents of the set.                                     * 
    ***************************************************************************/ 
    void   set_t_clear        (set_t* p_set);

    /***************************************************************************
    * Returns the size of the set.                                             *
    ***************************************************************************/ 
    int    set_t_size         (set_t* p_set);

    /***************************************************************************
    * Checks that the set maintains the AVL-tree invariant.                    *
    ***************************************************************************/  
    bool   set_t_is_healthy   (set_t* p_set);

    /***************************************************************************
    * Deallocates the entire set. Only the set and its nodes are deallocated.  *
    * The user is responsible to deallocate the actual data stored in the set. *
    ***************************************************************************/ 
    void   set_t_free         (set_t* p_set);

    /***************************************************************************
    * Returns the iterator over the set. The nodes are iterated in order.      *
    ***************************************************************************/  
    set_iterator_t* set_iterator_t_alloc        (set_t* p_set);

    /***************************************************************************
    * Returns the number of elements not yet iterated over.                    *
    ***************************************************************************/ 
    int             set_iterator_t_has_next     (set_iterator_t* p_iterator);

    /***************************************************************************
    * Sets to 'pp_element' the next element in the iteration order.            *
    ***************************************************************************/  
    bool            set_iterator_t_next         (set_iterator_t* p_iterator, 
                                                 void** pp_element);

    /***************************************************************************
    * Returns a positive integer if the set was modified during the iteration. *
    ***************************************************************************/  
    bool            set_iterator_t_is_disturbed (set_iterator_t* p_iterator);

    /***************************************************************************
    * Deallocates the set iterator.                                            *
    ***************************************************************************/  
    void            set_iterator_t_free         (set_iterator_t* p_iterator);

#ifdef	__cplusplus
}
#endif

#endif	/* SET_H */

