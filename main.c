#include <stdio.h>
#include <stdlib.h>
#include "map.h"

#define NUM(a) ((void*) a)

static int int_comparator(void* a, void* b) 
{
    return (int) a - (int) b;
}

int main(int argc, char** argv) {
    int i;
    map_t* p_map = map_t_alloc(int_comparator);

    for (i = 0; i < 100; ++i) 
        map_t_put(p_map, (void*) i, (void*) (3 * i));
    
    for (i = -10; i < 110; ++i) 
        printf("%3d: %3d (%d)\n", i, map_t_contains_key(p_map, i), map_t_get(p_map, i));

    printf("Healthy: %d\n", map_t_is_healthy(p_map));
    
    printf("Root height: %d\n", p_map->p_root->height);
    
    return (EXIT_SUCCESS);
}

