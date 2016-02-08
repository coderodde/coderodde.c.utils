#ifndef STABLE_SORT_H
#define	STABLE_SORT_H

#include <stdlib.h>

#ifdef	__cplusplus
extern "C" {
#endif

    /***************************************************************************
    * Sorts an array starting from 'base' of 'num' elements each 'size' bytes  *
    * long using comparator 'comparator'. This sort is stable, i.e., does not  *
    * rearrange equal elements.                                                *  
    ***************************************************************************/ 
    void stable_sort(void* base, 
                     size_t num, 
                     size_t size, 
                     int (*comparator)(const void*, const void*));

#ifdef	__cplusplus
}
#endif

#endif	/* STABLE_SORT_H */
