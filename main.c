#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "map.h"

#define NUM(a) ((void*) a)

static int int_comparator(void* a, void* b) 
{
    return (int) a - (int) b;
}

static void test_iterator()
{
    int i;
    map_t* p_map = map_t_alloc(int_comparator);
    
    puts("--- ITERATOR ---");
    
    for (i = -10; i <= 10; ++i)
        map_t_put(p_map, (void*)(i), (void*) (2 * i));
    
    map_iterator_t* p_iterator = map_iterator_t_alloc(p_map);
    
    printf("Iterator has next: %d\n", map_iterator_t_has_next(p_iterator));
    printf("Iterator returned: %d\n", (int) map_iterator_t_next(p_iterator));
    printf("Iterator has next: %d\n", map_iterator_t_has_next(p_iterator));
    printf("Iterator returned: %d\n", (int) map_iterator_t_next(p_iterator));
    printf("Iterator has next: %d\n", map_iterator_t_has_next(p_iterator));
    printf("Iterator returned: %d\n", (int) map_iterator_t_next(p_iterator));
    
    map_t_put(p_map, -10, -111);
    printf("Iterator disturbed: %d\n", map_iterator_t_is_disturbed(p_iterator));
    
    printf("Iterator has next: %d\n", map_iterator_t_has_next(p_iterator));
    printf("Iterator returned: %d\n", (int) map_iterator_t_next(p_iterator));
    printf("Iterator has next: %d\n", map_iterator_t_has_next(p_iterator));
    printf("Iterator returned: %d\n", (int) map_iterator_t_next(p_iterator));
    printf("Iterator has next: %d\n", map_iterator_t_has_next(p_iterator));
    printf("Iterator returned: %d\n", (int) map_iterator_t_next(p_iterator));
    
    map_t_put(p_map, 100, 500);
    printf("Iterator disturbed: %d\n", map_iterator_t_is_disturbed(p_iterator));
    
    printf("Iterator has next: %d\n", map_iterator_t_has_next(p_iterator));
    printf("Iterator returned: %d\n", (int) map_iterator_t_next(p_iterator));
    printf("Iterator has next: %d\n", map_iterator_t_has_next(p_iterator));
    printf("Iterator returned: %d\n", (int) map_iterator_t_next(p_iterator));
    printf("Iterator has next: %d\n", map_iterator_t_has_next(p_iterator));
    printf("Iterator returned: %d\n", (int) map_iterator_t_next(p_iterator));
}

static void test_correctness() 
{
    int i;
    map_t* p_map = map_t_alloc(int_comparator);
    
    puts("--- CORRECTNESS ---");
    
    for (i = 0; i < 100; ++i) 
        map_t_put(p_map, (void*) i, (void*) (3 * i));
    
    for (i = -10; i < 110; ++i) 
        printf("%3d: %3d (%d)\n", i, map_t_contains_key(p_map, i), map_t_get(p_map, i));

    printf("Healthy: %d\n", map_t_is_healthy(p_map));
    printf("Size: %d\n", p_map->size);
    
    puts("");
    puts("AFTER REMOVAL");
    puts("");
    
    for (i = 30; i < 55; ++i) 
        map_t_remove(p_map, (void*) i);
    
    for (i = -10; i < 110; ++i) 
        printf("%3d: %3d (%d)\n", 
                i, 
                map_t_contains_key(p_map, i), 
                map_t_get(p_map, i));

    printf("Healthy: %d\n", map_t_is_healthy(p_map));
    printf("Size: %d\n", map_t_size(p_map));
}

static void test_performance()
{
    map_t* p_map = map_t_alloc(int_comparator);
    
    clock_t t;
    const int sz = 1000000;
    int i;
    int a;
    int b;
    int tmp;
    
    int* array = malloc(sizeof(int) * sz);
 
    puts("--- PERFORMANCE --");
    printf("RAND_MAX: %d\n", RAND_MAX);
    
    for (i = 0; i < sz; ++i) 
        array[i] = i;
    
    srand(time(NULL));
    
    for (i = 0; i < sz; ++i)
    {
        a = rand() % sz;
        b = rand() % sz;
        
        tmp = array[a];
        array[a] = array[b];
        array[b] = tmp;
    }
    
    t = clock();
    
    for (i = 0; i < sz; ++i)
    {
        map_t_put(p_map, i, 3 * i);
    }
    
    
    printf("Healthy: %d\n", map_t_is_healthy(p_map));
    
    for (i = 0; i < sz; ++i) 
    {
        map_t_remove(p_map, array[sz - 1 - i]);
    }
    
    t = clock() - t;
    
    printf("Duration: %f seconds.\n", ((double) clock) / CLOCKS_PER_SEC);    
}

int main(int argc, char** argv) {
    test_iterator();
    test_correctness();
    test_performance();
    return (EXIT_SUCCESS);
}

