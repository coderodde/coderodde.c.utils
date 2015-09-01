#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "map.h"
#include "set.h"

#define ASSERT(CONDITION) assert(CONDITION, #CONDITION, __FILE__, __LINE__)

static bool assert(bool cond, char* err_msg, char* file_name, int line)
{
    if (!cond)
        fprintf(stderr, 
                "'%s' is not true in file '%s' at line %d.\n", 
                err_msg,
                file_name,
                line);
    
    return cond;
}

static int int_comparator(void* a, void* b) 
{
    return (int) a - (int) b;
}

static void test_map_performance()
{
    map_t* p_map = map_t_alloc(int_comparator);
    map_iterator_t* p_iterator;
    
    const int sz = 1000000;
    
    clock_t t;
    double duration = 0.0;
    int i;
    int j;
    int a;
    int b;
    int tmp;
    int value;
    void* p_key;
    void* p_value;
    
    int* array = malloc(sizeof(int) * sz);
 
    puts("--- PERFORMANCE ---");
    
    for (i = 0; i < sz; ++i) 
        array[i] = i;
    
    int time_ = time(NULL);
    printf("Time: %d.\n", time_);
    srand(time_);
    puts("Shuffling the integer array.");
    
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
        map_t_put(p_map, (void*) array[i], (void*)(3 * array[i]));
    }
    
    duration += ((double) clock() - t);
    
    printf("Healthy: %d\n", map_t_is_healthy(p_map));
    
    p_iterator = map_iterator_t_alloc(p_map);
    
    t = clock();
    
    while (map_iterator_t_has_next(p_iterator)) 
    {
        map_iterator_t_next(p_iterator, &p_key, &p_value);
        
        if (3 * (int) p_key != (int) p_value) exit(1);
    }
    
    duration += ((double) clock() - t);
    
    t = clock();
    
    for (i = 0; i < 5; ++i) 
    {
        for (j = 0; j < sz; ++j) 
        {
            value = map_t_get(p_map, array[i]);
            
            if (value != 3 * array[i]) 
            {
                exit(3);
            }
        }
    }
    
    duration += ((double) clock() - t);
    
    t = clock();
    
    for (i = 0; i < sz; ++i) 
    {   
        value = map_t_remove(p_map, array[i]);
        
        if (value != 3 * array[i]) 
        {
            printf("Key: %d, value: %d, index: %d, map size: %d, "
                   "contains: %d.\n",
                    array[i], 
                    value, 
                    i, 
                    map_t_size(p_map), 
                    map_t_contains_key(p_map, array[i]));
        } 
    }
    
    duration += ((double) clock() - t);
    printf("Healthy: %d\n", map_t_is_healthy(p_map));
    
    p_iterator = map_iterator_t_alloc(p_map);
    
    /* Empty iterator. */
    while (map_iterator_t_has_next(p_iterator)) 
    {
        printf("Element: %d\n", map_iterator_t_next(p_iterator, &p_key, &p_value));
    }
    
    printf("Duration: %f seconds.\n", duration / CLOCKS_PER_SEC);    
}

static void test_set_performance()
{
    set_t* p_set = set_t_alloc(int_comparator);
    set_iterator_t* p_iterator;
    
    const int sz = 1000000;
    
    clock_t t;
    double duration = 0.0;
    int i;
    int j;
    int a;
    int b;
    int tmp;
    void* p_element;
    
    int* array = malloc(sizeof(int) * sz);
 
    puts("--- PERFORMANCE ---");
    
    for (i = 0; i < sz; ++i) 
        array[i] = i;
    
    int time_ = time(NULL);
    printf("Time: %d.\n", time_);
    srand(time_);
    puts("Shuffling the integer array.");
    
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
        set_t_add(p_set, (void*) array[i]);
    }
    
    duration += ((double) clock() - t);
    
    printf("Healthy: %d\n", set_t_is_healthy(p_set));
    
    p_iterator = set_iterator_t_alloc(p_set);
    
    t = clock();
    
    i = 0;
    
    while (set_iterator_t_has_next(p_iterator)) 
    {
        set_iterator_t_next(p_iterator, &p_element);
        
        if (i++ != (int) p_element) exit(1);
    }
    
    duration += ((double) clock() - t);
    
    t = clock();
    
    for (i = 0; i < 5; ++i) 
        for (j = 0; j < sz; ++j) 
            if (!set_t_contains(p_set, (void*) array[i])) 
                exit(2);
        
    duration += ((double) clock() - t);
    
    t = clock();
    
    for (i = 0; i < sz; ++i) 
        if(!ASSERT(set_t_remove(p_set, array[i])))
            printf("Yoo!: %d\n", i);
    
    duration += ((double) clock() - t);
    printf("Healthy: %d\n", set_t_is_healthy(p_set));
    
    p_iterator = set_iterator_t_alloc(p_set);
    
    /* Empty iterator. */
    while (map_iterator_t_has_next(p_iterator)) 
    {
        printf("Element: %d\n", set_iterator_t_next(p_iterator, &p_element));
    }
    
    printf("Duration: %f seconds.\n", duration / CLOCKS_PER_SEC);    
}

void test_map_correctness() 
{
    int i;
    void* p_key;
    void* p_value;
    int expected_size;
    map_t* p_map = map_t_alloc(int_comparator);
    map_iterator_t* p_iterator;
    
    for (i = -10; i < 10; ++i) 
    {
        ASSERT(map_t_contains_key(p_map, (void*) i) == false);
        ASSERT(map_t_get(p_map, (void*) i) == NULL);
        ASSERT(map_t_size(p_map) == (i + 10));
        
        map_t_put(p_map, (void*) i, (void*)(3 * i));
        
        ASSERT(map_t_contains_key(p_map, i) == true);
        ASSERT(map_t_get(p_map, (void*) i) == (void*)(3 * i));
        ASSERT(map_t_size(p_map) == (i + 10) + 1);
    }
    
    expected_size = map_t_size(p_map);
    p_iterator = map_iterator_t_alloc(p_map);
    ASSERT(expected_size == 20);
    
    for (i = -10; i < 10; ++i) 
    {
        ASSERT(map_iterator_t_has_next(p_iterator) == 10 - i);
        ASSERT(map_iterator_t_next(p_iterator, &p_key, &p_value));
        ASSERT(3 * (int) p_key == (int) p_value);
    }
    
    ASSERT(map_iterator_t_has_next(p_iterator) == 0);
    ASSERT(map_t_size(p_map) == expected_size);
    
    map_t_clear(p_map);
    
    ASSERT(map_t_size(p_map) == 0);
    
    ASSERT(map_t_put(p_map, (void*) 1, (void*) 11) == NULL);
    ASSERT(map_t_size(p_map) == 1);
    ASSERT(map_t_put(p_map, (void*) 1, (void*) 12) == (void*) 11);
    ASSERT(map_t_size(p_map) == 1);
    ASSERT(map_t_contains_key(p_map, (void*) 1) == true);
    ASSERT(map_t_contains_key(p_map, (void*) 2) == false);
    ASSERT(map_t_get(p_map, (void*) 1) == (void*) 12);
    ASSERT(map_t_get(p_map, (void*) 2) == (void*) 0);
    
    ASSERT(map_t_contains_key(p_map, (void*) 10) == false);
    ASSERT(map_t_get(p_map, (void*) 10) == 0);
    ASSERT(map_t_put(p_map, (void*) 10, (void*) 30) == 0);
    ASSERT(map_t_get(p_map, (void*) 10) == (void*) 30);
    ASSERT(map_t_contains_key(p_map, (void*) 10) == true);
    ASSERT(map_t_remove(p_map, (void*) 11) == NULL);
    ASSERT(map_t_get(p_map, (void*) 10) == (void*) 30);
    ASSERT(map_t_contains_key(p_map, (void*) 10) == true);
    ASSERT(map_t_remove(p_map, (void*) 10) == (void*) 30);
    ASSERT(map_t_get(p_map, (void*) 10) == 0);
    ASSERT(map_t_contains_key(p_map, (void*) 10) == false);
}

void test_set_correctness() 
{
    int i;
    void* p_key;
    int expected_size;
    set_t* p_set = set_t_alloc(int_comparator);
    set_iterator_t* p_iterator;
    
    for (i = -10; i < 10; ++i) 
    {
        ASSERT(set_t_contains(p_set, (void*) i) == false);
        ASSERT(set_t_size(p_set) == (i + 10));
        
        ASSERT(set_t_add(p_set, (void*) i) == true);
        
        ASSERT(set_t_contains(p_set, (void*) i) == true);
        ASSERT(set_t_size(p_set) == (i + 10) + 1);
    }
    
    expected_size = set_t_size(p_set);
    p_iterator = set_iterator_t_alloc(p_set);
    ASSERT(expected_size == 20);
    
    for (i = -10; i < 10; ++i) 
    {
        ASSERT(set_iterator_t_has_next(p_iterator) == 10 - i);
        ASSERT(set_iterator_t_next(p_iterator, &p_key) == true);
        ASSERT((int) p_key == i);
    }
    
    ASSERT(set_iterator_t_has_next(p_iterator) == 0);
    ASSERT(set_t_size(p_set) == expected_size);
    
    set_t_clear(p_set);
    
    ASSERT(set_t_size(p_set) == 0);
    
    ASSERT(set_t_add      (p_set, (void*) 1));
    ASSERT(set_t_size     (p_set) == 1);
    ASSERT(set_t_add      (p_set, (void*) 1) == false);
    ASSERT(set_t_size     (p_set) == 1);
    ASSERT(set_t_contains (p_set, (void*) 1));
    ASSERT(set_t_contains (p_set, (void*) 2) == false);
    
    ASSERT(set_t_contains (p_set, (void*) 10) == false);
    ASSERT(set_t_add      (p_set, (void*) 10));
    ASSERT(set_t_contains (p_set, (void*) 10));
    ASSERT(set_t_remove   (p_set, (void*) 11) == false);
    ASSERT(set_t_remove   (p_set, (void*) 10));
    ASSERT(set_t_contains (p_set, (void*) 10) == false);
}

int main(int argc, char** argv) {
    test_map_correctness();
    test_map_performance();
    test_set_correctness();
    test_set_performance();
    return (EXIT_SUCCESS);
}
