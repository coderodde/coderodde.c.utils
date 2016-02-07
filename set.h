#ifndef SET_H
#define	SET_H

#include <stdlib.h>
#include <stdbool.h>

#ifdef	__cplusplus
extern "C" {
#endif
    
    typedef struct set set;
    typedef struct set_iterator set_iterator;

    /***************************************************************************
    * Allocates a new, empty set with given comparator function.               *
    ***************************************************************************/ 
    set* set_alloc (int (*comparator)(void*, void*));

    /***************************************************************************
    * Adds an element to the set if not already there. Returns true if the     *
    * structure of the set changed.                                            * 
    ***************************************************************************/ 
    bool set_add (set* my_set, void* element);

    /***************************************************************************
    * Returns true if the set contains the element.                            *
    ***************************************************************************/
    bool set_contains (set* my_set, void* element);

    /***************************************************************************
    * If the element is in the set, removes it and returns true.               * 
    ***************************************************************************/ 
    bool set_remove (set* my_set, void* element);

    /***************************************************************************
    * Removes all the contents of the set. The client programmer is responsible*
    * for deallocating the actual contents.                                    * 
    ***************************************************************************/ 
    void set_clear (set* my_set);

    /***************************************************************************
    * Returns the size of the set.                                             *
    ***************************************************************************/ 
    size_t set_size (set* my_set);

    /***************************************************************************
    * Checks that the set maintains the AVL-tree property.                     *
    ***************************************************************************/  
    bool set_is_healthy (set* my_set);

    /***************************************************************************
    * Deallocates the entire set. Only the set and its nodes are deallocated.  *
    * The user is responsible for deallocating the actual data stored in the   *
    * set.                                                                     *
    ***************************************************************************/ 
    void set_free (set* my_set);

    /***************************************************************************
    * Returns the iterator over the set. The nodes are iterated in order.      *
    ***************************************************************************/  
    set_iterator* set_iterator_alloc (set* my_set);

    /***************************************************************************
    * Returns the number of elements not yet iterated over.                    *
    ***************************************************************************/ 
    size_t set_iterator_has_next (set_iterator* iterator);

    /***************************************************************************
    * Loads the next element in the iteration order.                           *
    ***************************************************************************/  
    bool set_iterator_next (set_iterator* iterator, void** element_pointer);

    /***************************************************************************
    * Returns true if the set was modified during the iteration.               *
    ***************************************************************************/  
    bool set_iterator_is_disturbed (set_iterator* iterator);

    /***************************************************************************
    * Deallocates the set iterator.                                            *
    ***************************************************************************/  
    void set_iterator_free (set_iterator* iterator);

#ifdef	__cplusplus
}
#endif

#endif	/* SET_H */

