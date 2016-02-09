#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "map.h"
#include "set.h"
#include "unordered_map.h"
#include "unordered_set.h"
#include "heap.h"
#include "list.h"
#include "fibonacci_heap.h"
#include "stable_sort.h"

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
    map* p_map = map_alloc(int_comparator);
    map_iterator* p_iterator;
    
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
    int seed;
    int* array = malloc(sizeof(int) * sz);
 
    puts("--- PERFORMANCE OF map ---");
    
    for (i = 0; i < sz; ++i) 
        array[i] = i;
    
    seed = time(NULL);
    printf("Seed: %d.\n", seed);
    srand(seed);
    
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
        map_put(p_map, (void*) array[i], (void*)(3 * array[i]));
    }
    
    duration += ((double) clock() - t);
    
    printf("Healthy: %d\n", map_is_healthy(p_map));
    
    p_iterator = map_iterator_alloc(p_map);
    
    t = clock();
    
    while (map_iterator_has_next(p_iterator)) 
    {
        map_iterator_next(p_iterator, &p_key, &p_value);
        
        if (3 * (int) p_key != (int) p_value) exit(1);
    }
    
    duration += ((double) clock() - t);
    
    t = clock();
    
    for (i = 0; i < 5; ++i) 
    {
        for (j = 0; j < sz; ++j) 
        {
            value = map_get(p_map, array[i]);
            
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
        value = map_remove(p_map, array[i]);
        
        if (value != 3 * array[i]) 
        {
            printf("Key: %d, value: %d, index: %d, map size: %zu, "
                   "contains: %d.\n",
                    array[i], 
                    value, 
                    i, 
                    map_size(p_map), 
                    map_contains_key(p_map, array[i]));
        } 
    }
    
    duration += ((double) clock() - t);
    printf("Healthy: %d\n", map_is_healthy(p_map));
    
    p_iterator = map_iterator_alloc(p_map);
    
    /* Empty iterator. */
    while (map_iterator_has_next(p_iterator)) 
    {
        printf("Element: %d\n", map_iterator_next(p_iterator, &p_key, &p_value));
    }
    
    printf("Duration: %f seconds.\n", duration / CLOCKS_PER_SEC);    
}

static size_t hash_function(void* v)
{
    return (size_t) v;
}

static bool equals_function(void* a, void* b)
{
    return a == b;
}

static void test_unordered_map_performance()
{
    unordered_map* p_map = unordered_map_alloc(7, 
                                               0.75f, 
                                               hash_function, 
                                               equals_function);
    unordered_map_iterator* p_iterator;
    
    const int sz = 1000000;
    bool* p_set = calloc(sz, sizeof(bool));
    
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
    int seed;
    int* array = malloc(sizeof(int) * sz);
 
    puts("--- PERFORMANCE OF unordered_map ---");
    
    for (i = 0; i < sz; ++i) 
        array[i] = i;
    
    seed = time(NULL);
    printf("Seed: %d.\n", seed);
    srand(seed);
    
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
        unordered_map_put(p_map, (void*) array[i], (void*)(3 * array[i]));
    }
    
    duration += ((double) clock() - t);
    printf("Healthy: %d\n", unordered_map_is_healthy(p_map));
    
    p_iterator = unordered_map_iterator_alloc(p_map);
    i = 0;
    t = clock();
    
    while (unordered_map_iterator_has_next(p_iterator)) 
    {
        unordered_map_iterator_next(p_iterator, &p_key, &p_value);
        
        if (3 * (int) p_key != (int) p_value && (int) p_key != i) exit(1);
        
        p_set[(int) p_key] = true;
    }
    
    duration += ((double) clock() - t);
    
    for (i = 0; i < sz; ++i) 
    {
        if (!p_set[i]) 
        {
            printf("ERROR: Key %d was not iterated.", i);
            break;
        }
    }
    
    t = clock();
    
    for (i = 0; i < 5; ++i) 
    {
        for (j = 0; j < sz; ++j) 
        {   
            value = unordered_map_get(p_map, array[j]);
            
            if (value != 3 * array[j]) 
            {
                printf("Value: %d, key: %d, index: %d\n", value, array[j], j);
                exit(3);
            }
        }
    }
    
    duration += ((double) clock() - t);
    
    t = clock();
    
    for (i = 0; i < sz; ++i) 
    {   
        value = unordered_map_remove(p_map, array[i]);
        
        if (value != 3 * array[i]) 
        {
            printf("Key: %d, value: %d, index: %d, map size: %zu, "
                   "contains: %d.\n",
                    array[i], 
                    value, 
                    i, 
                    unordered_map_size(p_map), 
                    unordered_map_contains_key(p_map, array[i]));
        } 
    }
    
    
    duration += ((double) clock() - t);
    printf("Healthy: %d\n", unordered_map_is_healthy(p_map));
    
    p_iterator = unordered_map_iterator_alloc(p_map);
    
    /* Empty iterator. */
    while (unordered_map_iterator_has_next(p_iterator)) 
    {
        printf("Element: %d\n", unordered_map_iterator_next(p_iterator, 
                                                              &p_key, 
                                                              &p_value));
    }
    
    printf("Duration: %f seconds.\n", duration / CLOCKS_PER_SEC);    
}

static void test_set_performance()
{
    set* p_set = set_alloc(int_comparator);
    set_iterator* p_iterator;
    
    const int sz = 1000000;
    
    clock_t t;
    double duration = 0.0;
    int i;
    int j;
    int a;
    int b;
    int tmp;
    void* p_element;
    int seed;
    int* array = malloc(sizeof(int) * sz);
 
    puts("--- PERFORMANCE OF set ---");
    
    for (i = 0; i < sz; ++i) 
        array[i] = i;
    
    seed = time(NULL);
    printf("Seed: %d.\n", seed);
    srand(seed);
    
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
        set_add(p_set, (void*) array[i]);
    }
    
    duration += ((double) clock() - t);
    
    printf("Healthy: %d\n", set_is_healthy(p_set));
    
    p_iterator = set_iterator_alloc(p_set);
    
    t = clock();
    i = 0;
    
    while (set_iterator_has_next(p_iterator)) 
    {
        set_iterator_next(p_iterator, &p_element);
        
        if (i++ != (int) p_element) exit(1);
    }
    
    duration += ((double) clock() - t);
    
    t = clock();
    
    for (i = 0; i < 5; ++i) 
        for (j = 0; j < sz; ++j) 
            if (!set_contains(p_set, (void*) array[i])) 
                exit(2);
        
    duration += ((double) clock() - t);
    
    t = clock();
    
    for (i = 0; i < sz; ++i) 
    {
        if(!ASSERT(set_remove(p_set, (void*) array[i])))
        {
            printf("Fails at index %d\n", i);
            exit(3);
        }
    }
    
    duration += ((double) clock() - t);
    printf("Healthy: %d\n", set_is_healthy(p_set));
    
    p_iterator = set_iterator_alloc(p_set);
    
    /* Empty iterator. */
    while (set_iterator_has_next(p_iterator)) 
    {
        set_iterator_next(p_iterator, &p_element);
    }
    
    printf("Duration: %f seconds.\n", duration / CLOCKS_PER_SEC);    
}

void test_map_correctness() 
{
    int i;
    void* p_key;
    void* p_value;
    int expected_size;
    map* p_map = map_alloc(int_comparator);
    map_iterator* p_iterator;
    
    for (i = -10; i < 10; ++i) 
    {
        ASSERT(map_contains_key(p_map, (void*) i) == false);
        ASSERT(map_get(p_map, (void*) i) == NULL);
        ASSERT(map_size(p_map) == (i + 10));
        
        map_put(p_map, (void*) i, (void*)(3 * i));
        
        ASSERT(map_contains_key(p_map, i) == true);
        ASSERT(map_get(p_map, (void*) i) == (void*)(3 * i));
        ASSERT(map_size(p_map) == (i + 10) + 1);
    }
    
    expected_size = map_size(p_map);
    p_iterator = map_iterator_alloc(p_map);
    ASSERT(expected_size == 20);
    
    for (i = -10; i < 10; ++i) 
    {
        ASSERT(map_iterator_has_next(p_iterator) == 10 - i);
        ASSERT(map_iterator_next(p_iterator, &p_key, &p_value));
        ASSERT(3 * (int) p_key == (int) p_value);
    }
    
    ASSERT(map_iterator_has_next(p_iterator) == 0);
    ASSERT(map_size(p_map) == expected_size);
    
    map_clear(p_map);
    
    ASSERT(map_size(p_map) == 0);
    
    ASSERT(map_put(p_map, (void*) 1, (void*) 11) == NULL);
    ASSERT(map_size(p_map) == 1);
    ASSERT(map_put(p_map, (void*) 1, (void*) 12) == (void*) 11);
    ASSERT(map_size(p_map) == 1);
    ASSERT(map_contains_key(p_map, (void*) 1) == true);
    ASSERT(map_contains_key(p_map, (void*) 2) == false);
    ASSERT(map_get(p_map, (void*) 1) == (void*) 12);
    ASSERT(map_get(p_map, (void*) 2) == (void*) 0);
    
    ASSERT(map_contains_key(p_map, (void*) 10) == false);
    ASSERT(map_get(p_map, (void*) 10) == 0);
    ASSERT(map_put(p_map, (void*) 10, (void*) 30) == 0);
    ASSERT(map_get(p_map, (void*) 10) == (void*) 30);
    ASSERT(map_contains_key(p_map, (void*) 10) == true);
    ASSERT(map_remove(p_map, (void*) 11) == NULL);
    ASSERT(map_get(p_map, (void*) 10) == (void*) 30);
    ASSERT(map_contains_key(p_map, (void*) 10) == true);
    ASSERT(map_remove(p_map, (void*) 10) == (void*) 30);
    ASSERT(map_get(p_map, (void*) 10) == 0);
    ASSERT(map_contains_key(p_map, (void*) 10) == false);
}

void test_unordered_map_correctness() 
{
    int i;
    void* p_key;
    void* p_value;
    int expected_size;
    unordered_map* p_map = unordered_map_alloc(7, 0.4f, hash_function, equals_function);
    unordered_map_iterator* p_iterator;
    
    for (i = -10; i < 10; ++i) 
    {
        ASSERT(unordered_map_contains_key(p_map, (void*) i) == false);
        ASSERT(unordered_map_get(p_map, (void*) i) == NULL);
        ASSERT(unordered_map_size(p_map) == (i + 10));
        
        unordered_map_put(p_map, (void*) i, (void*)(3 * i));
        
        ASSERT(unordered_map_contains_key(p_map, i) == true);
        ASSERT(unordered_map_get(p_map, (void*) i) == (void*)(3 * i));
        ASSERT(unordered_map_size(p_map) == (i + 10) + 1);
    }
    
    expected_size = unordered_map_size(p_map);
    p_iterator = unordered_map_iterator_alloc(p_map);
    ASSERT(expected_size == 20);
    
    for (i = -10; i < 10; ++i) 
    {
        ASSERT(unordered_map_iterator_has_next(p_iterator) == 10 - i);
        ASSERT(unordered_map_iterator_next(p_iterator, &p_key, &p_value));
        ASSERT(3 * (int) p_key == (int) p_value);
    }
    
    ASSERT(unordered_map_iterator_has_next(p_iterator) == 0);
    ASSERT(unordered_map_size(p_map) == expected_size);
    
    unordered_map_clear(p_map);
    
    ASSERT(unordered_map_size(p_map) == 0);
    ASSERT(unordered_map_put(p_map, (void*) 1, (void*) 11) == NULL);
    ASSERT(unordered_map_size(p_map) == 1);
    ASSERT(unordered_map_put(p_map, (void*) 1, (void*) 12) == (void*) 11);
    ASSERT(unordered_map_size(p_map) == 1);
    ASSERT(unordered_map_contains_key(p_map, (void*) 1) == true);
    ASSERT(unordered_map_contains_key(p_map, (void*) 2) == false);
    ASSERT(unordered_map_get(p_map, (void*) 1) == (void*) 12);
    ASSERT(unordered_map_get(p_map, (void*) 2) == (void*) 0);
  
    ASSERT(unordered_map_contains_key(p_map, (void*) 10) == false);
    ASSERT(unordered_map_get(p_map, (void*) 10) == 0);
    ASSERT(unordered_map_put(p_map, (void*) 10, (void*) 30) == 0);
    ASSERT(unordered_map_get(p_map, (void*) 10) == (void*) 30);
    ASSERT(unordered_map_contains_key(p_map, (void*) 10) == true);
    ASSERT(unordered_map_remove(p_map, (void*) 11) == NULL);
    ASSERT(unordered_map_get(p_map, (void*) 10) == (void*) 30);
    ASSERT(unordered_map_contains_key(p_map, (void*) 10) == true);
    ASSERT(unordered_map_remove(p_map, (void*) 10) == (void*) 30);
    ASSERT(unordered_map_get(p_map, (void*) 10) == 0);
    ASSERT(unordered_map_contains_key(p_map, (void*) 10) == false);
    
    for (i = 100; i < 110; ++i) 
    {
        ASSERT(unordered_map_put(p_map, (void*) i, (void*) i) == NULL);
    }
    
    unordered_map_free(p_map);
}

void test_set_correctness() 
{
    int i;
    void* p_key;
    int expected_size;
    set* p_set = set_alloc(int_comparator);
    set_iterator* p_iterator;
    
    for (i = -10; i < 10; ++i) 
    {
        ASSERT(set_contains(p_set, (void*) i) == false);
        ASSERT(set_size(p_set) == (i + 10));
        
        ASSERT(set_add(p_set, (void*) i) == true);
        
        ASSERT(set_contains(p_set, (void*) i) == true);
        ASSERT(set_size(p_set) == (i + 10) + 1);
    }
    
    expected_size = set_size(p_set);
    p_iterator = set_iterator_alloc(p_set);
    ASSERT(expected_size == 20);
    
    for (i = -10; i < 10; ++i) 
    {
        ASSERT(set_iterator_has_next(p_iterator) == 10 - i);
        ASSERT(set_iterator_next(p_iterator, &p_key) == true);
        ASSERT((int) p_key == i);
    }
    
    ASSERT(set_iterator_has_next(p_iterator) == 0);
    ASSERT(set_size(p_set) == expected_size);
    
    set_clear(p_set);
    
    ASSERT(set_size(p_set) == 0);
    
    ASSERT(set_add      (p_set, (void*) 1));
    ASSERT(set_size     (p_set) == 1);
    ASSERT(set_add      (p_set, (void*) 1) == false);
    ASSERT(set_size     (p_set) == 1);
    ASSERT(set_contains (p_set, (void*) 1));
    ASSERT(set_contains (p_set, (void*) 2) == false);
  
    ASSERT(set_contains (p_set, (void*) 10) == false);
    ASSERT(set_add      (p_set, (void*) 10));
    ASSERT(set_contains (p_set, (void*) 10));
    ASSERT(set_remove   (p_set, (void*) 11) == false);
    ASSERT(set_remove   (p_set, (void*) 10));
    ASSERT(set_contains (p_set, (void*) 10) == false);
    
}

void test_unordered_set_correctness() 
{
    int i;
    void* p_key;
    int expected_size;
    unordered_set* p_set = unordered_set_t_alloc(7, 0.8f, hash_function, equals_function);
    unordered_set_iterator* p_iterator;
    
    for (i = -10; i < 10; ++i) 
    {
        ASSERT(unordered_set_t_contains(p_set, (void*) i) == false);
        ASSERT(unordered_set_t_size(p_set) == (i + 10));
        
        ASSERT(unordered_set_t_add(p_set, (void*) i) == true);
        
        ASSERT(unordered_set_t_contains(p_set, (void*) i) == true);
        ASSERT(unordered_set_t_size(p_set) == (i + 10) + 1);
    }
    
    expected_size = unordered_set_t_size(p_set);
    p_iterator = unordered_set_iterator_t_alloc(p_set);
    ASSERT(expected_size == 20);
    
    for (i = -10; i < 10; ++i) 
    {
        ASSERT(unordered_set_iterator_t_has_next(p_iterator) == 10 - i);
        ASSERT(unordered_set_iterator_t_next(p_iterator, &p_key) == true);
        ASSERT((int) p_key == i);
    }
    
    ASSERT(unordered_set_iterator_t_has_next(p_iterator) == 0);
    ASSERT(unordered_set_t_size(p_set) == expected_size);
    
    unordered_set_t_clear(p_set);
    
    ASSERT(unordered_set_t_size(p_set) == 0);
    
    ASSERT(unordered_set_t_add      (p_set, (void*) 1));
    ASSERT(unordered_set_t_size     (p_set) == 1);
    ASSERT(unordered_set_t_add      (p_set, (void*) 1) == false);
    ASSERT(unordered_set_t_size     (p_set) == 1);
    ASSERT(unordered_set_t_contains (p_set, (void*) 1));
    ASSERT(unordered_set_t_contains (p_set, (void*) 2) == false);
    
    ASSERT(unordered_set_t_contains (p_set, (void*) 10) == false);
    ASSERT(unordered_set_t_add      (p_set, (void*) 10));
    ASSERT(unordered_set_t_contains (p_set, (void*) 10));
    ASSERT(unordered_set_t_remove   (p_set, (void*) 11) == false);
    ASSERT(unordered_set_t_remove   (p_set, (void*) 10));
    ASSERT(unordered_set_t_contains (p_set, (void*) 10) == false);
}

static void test_unordered_set_performance()
{
    unordered_set* p_set = unordered_set_t_alloc(7, 0.8f, hash_function, equals_function);
    unordered_set_iterator* p_iterator;
    
    const int sz = 1000000;
    
    clock_t t;
    double duration = 0.0;
    int i;
    int j;
    int a;
    int b;
    int tmp;
    void* p_element;
    bool* p_check_list;
    int seed;
    int* array = malloc(sizeof(int) * sz);
    p_check_list = calloc(sz, sizeof(bool));
    
    puts("--- PERFORMANCE OF unordered_set ---");
    
    for (i = 0; i < sz; ++i) 
        array[i] = i;
    
    seed = time(NULL);
    printf("Seed: %d.\n", seed);
    srand(seed);
    
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
        unordered_set_t_add(p_set, (void*) array[i]);
    }
    
    duration += ((double) clock() - t);
    
    printf("Healthy: %d\n", unordered_set_t_is_healthy(p_set));
    p_iterator = unordered_set_iterator_t_alloc(p_set);
    t = clock();
    
    while (unordered_set_iterator_t_has_next(p_iterator)) 
    {
        unordered_set_iterator_t_next(p_iterator, &p_element);
        
        p_check_list[(int) p_element] = true;
    }
    
    
    duration += ((double) clock() - t);
    
    for (i = 0; i < sz; ++i) 
    {
        if (!p_check_list[i])
        {
            puts("Not all elements set.");
            exit(1);
        }
    }
    
    t = clock();
    
    for (i = 0; i < 5; ++i) 
        for (j = 0; j < sz; ++j) 
            if (!unordered_set_t_contains(p_set, (void*) array[i])) 
                exit(2);
        
    duration += ((double) clock() - t);
    
    t = clock();
    
    for (i = 0; i < sz; ++i) 
    {
        if(!ASSERT(unordered_set_t_remove(p_set, (void*) array[i])))
        {
            printf("Fails at index %d\n", i);
            exit(3);
        }
    }
    
    duration += ((double) clock() - t);
    printf("Healthy: %d\n", unordered_set_t_is_healthy(p_set));
    
    p_iterator = unordered_set_iterator_t_alloc(p_set);
    
    /* Empty iterator. */
    while (unordered_set_iterator_t_has_next(p_iterator)) 
    {
        unordered_set_iterator_t_next(p_iterator, &p_element);
    }
    
    printf("Duration: %f seconds.\n", duration / CLOCKS_PER_SEC);    
}

static int priority_cmp(void* a, void* b) 
{
    return ((int) a) - ((int) b);
}

static void test_heap_correctness() 
{
    heap* p_heap;
    size_t i;
    
    p_heap = heap_alloc(2, 
                        10, 
                        1.0f, 
                        hash_function, 
                        equals_function, priority_cmp);
    
    ASSERT(heap_is_healthy(p_heap));
    
    for (i = 0; i < 30; ++i) 
    {
        ASSERT(heap_add(p_heap, i, 30 - i));
    }
    
    for (i = 0; i < 30; ++i)
    {
        ASSERT(!heap_add(p_heap, i, i));
    }
    
    ASSERT(heap_size(p_heap) == 30);
    
    for (i = 0; i < 30; ++i) 
    {
        ASSERT(heap_contains(p_heap, i));
    }
    
    for (i = 30; i < 40; ++i) 
    {
        ASSERT(heap_contains(p_heap, i) == false);
    }
    
    ASSERT(heap_is_healthy(p_heap));
    
    for (i = 29; i != (size_t) -1; --i) 
    {
        ASSERT((int) heap_extract_min(p_heap) == i);
    }
    
    ASSERT(heap_size(p_heap) == 0);
    ASSERT(heap_is_healthy(p_heap));
    
    for (i = 10; i < 100; ++i) 
    {
        ASSERT(heap_add(p_heap, i, i));
    }
    
    ASSERT(heap_decrease_key(p_heap, 50, 0));
    
    ASSERT((size_t) heap_min(p_heap) == 50);
    ASSERT((size_t) heap_extract_min(p_heap) == 50);
    
    for (i = 10; i < 50; ++i) 
    {
        ASSERT((size_t) heap_min(p_heap) == i);
        ASSERT((size_t) heap_extract_min(p_heap) == i);
    }
    
    for (i = 51; i < 100; ++i) 
    {
        ASSERT((size_t) heap_min(p_heap) == i);
        ASSERT((size_t) heap_extract_min(p_heap) == i);
    }
    
    ASSERT(heap_min(p_heap) == NULL);
    ASSERT(heap_extract_min(p_heap) == NULL);
    
    ASSERT(heap_add(p_heap, 30, 30));
    ASSERT(heap_add(p_heap, 40, 40));
    ASSERT(heap_contains(p_heap, 30));
    ASSERT(heap_contains(p_heap, 40));
    ASSERT(heap_size(p_heap) == 2);
    
    heap_clear(p_heap);
    
    ASSERT(heap_size(p_heap) == 0);
    ASSERT(heap_extract_min(p_heap) == NULL);
}

static void test_heap_performance()
{
    heap* p_heap;
    size_t degree;
    size_t i;
    clock_t t;
    const size_t sz = 1000000;
    double duration;
    
    puts("--- PERFORMANCE OF heap_t ---");
    
    for (degree = 2; degree <= 10; ++degree)
    {
        duration = 0.0;
        printf("Degree %zu:\n", degree);
        p_heap = heap_alloc(degree,
                              10,
                              1.0f,
                              hash_function,
                              equals_function,
                              priority_cmp);
        t = clock();
        
        for (i = 0; i < sz; ++i) 
        {
            heap_add(p_heap, i, 500000 + sz - i);
        }
        
        /* State: 999999, 999998, 999997, ... */
        ASSERT(heap_is_healthy(p_heap));
        
        for (i = sz / 2; i < sz; ++i)
        {
            heap_decrease_key(p_heap, i, i);
        }
        
        ASSERT(heap_is_healthy(p_heap));
        
        for (i = 0; i < sz; ++i)
        {
            heap_extract_min(p_heap);
        }
        
        duration += ((double) clock() - t);
        
        heap_free(p_heap);
        
        printf("Duration: %f seconds.\n", duration / CLOCKS_PER_SEC);
    }
}

static bool int_eq(void* a, void* b)
{
    return a == b;
}

static void test_list_correctness()
{
    size_t i;
    list* p_list = list_alloc(10);
    
    for (i = 0; i < 40; i++) 
    {
        ASSERT(list_push_back(p_list, 2 * i));
    }
    
    ASSERT(list_size(p_list) == 40);
    
    for (i = 0; i < 40; ++i) 
    {
        ASSERT((size_t) list_get(p_list, i) == 2 * i);
    }
    
    for (i = 0; i < 40; ++i) 
    {
        ASSERT(list_size(p_list) > 0);
        ASSERT((size_t) list_pop_front(p_list) == 2 * i);
    }
    
    ASSERT(list_size(p_list) == 0);
    
    for (i = 200; i > 0; --i) 
    {
        ASSERT(list_push_front(p_list, i));
    }
    
    ASSERT(list_size(p_list) == 200);
    
    for (i = 200; i > 0; --i) 
    {
        ASSERT((size_t) list_pop_back(p_list) == i);
    }
    
    ASSERT(list_size(p_list) == 0);
    
    ASSERT(list_insert(p_list, 0, 0));
    ASSERT(list_insert(p_list, 1, 1));
    ASSERT(list_insert(p_list, 0, 2));
    
    ASSERT((size_t) list_get(p_list, 0) == 2);
    ASSERT((size_t) list_get(p_list, 1) == 0);
    ASSERT((size_t) list_get(p_list, 2) == 1);
    
    /* <2, 0, 1> */
    ASSERT(list_insert(p_list, 0, 10));
    /* <10, 2, 0, 1> */
    ASSERT(list_insert(p_list, 4, 11));
    /* <10, 2, 0, 1, 11> */
    ASSERT(list_insert(p_list, 2, 12));
    /* <10, 2, 12, 0, 1, 11> */
    ASSERT(list_insert(p_list, 3, 14));
    /* <10, 2, 12, 14, 0, 1, 11> */
    
    ASSERT((size_t) list_get(p_list, 0) == 10);
    ASSERT((size_t) list_get(p_list, 1) == 2);
    ASSERT((size_t) list_get(p_list, 2) == 12);
    ASSERT((size_t) list_get(p_list, 3) == 14);
    ASSERT((size_t) list_get(p_list, 4) == 0);
    ASSERT((size_t) list_get(p_list, 5) == 1);
    ASSERT((size_t) list_get(p_list, 6) == 11);
    
    ASSERT((size_t) list_set(p_list, 5, 100) == 1);
    ASSERT((size_t) list_get(p_list, 5) == 100);
    
    ASSERT(list_size(p_list) == 7);
    
    ASSERT(list_contains(p_list,  10,  int_eq));
    ASSERT(list_contains(p_list,  2,   int_eq));
    ASSERT(list_contains(p_list,  12,  int_eq));
    ASSERT(list_contains(p_list,  14,  int_eq));
    ASSERT(list_contains(p_list,  0,   int_eq));
    ASSERT(list_contains(p_list,  100, int_eq));
    ASSERT(list_contains(p_list,  11,  int_eq));
    ASSERT(!list_contains(p_list, 15,  int_eq));
    ASSERT(!list_contains(p_list, 16,  int_eq));
    
    ASSERT(list_size(p_list) == 7);
    
    /* <10, 2, 12, 14, 0, 100, 11*/
    ASSERT(list_remove_at(p_list, 4) == 0);
    /* <10, 2, 12, 14, 100, 11> */
    ASSERT((int) list_remove_at(p_list, 4) == 100);
    /* <10, 2, 12, 14, 11> */
    ASSERT((int) list_remove_at(p_list, 4) == 11);
    /* <10, 2, 12, 14> */
    ASSERT((int) list_remove_at(p_list, 0) == 10);
    /* <2, 12, 14> */
    ASSERT(list_remove_at(p_list, 4) == NULL);
    ASSERT(list_remove_at(p_list, 3) == NULL);
    /* <2, 12, 14> */
    ASSERT((int) list_remove_at(p_list, 1) == 12);
    /* <2, 14> */
    ASSERT((int) list_remove_at(p_list, 1) == 14);
    /* <2> */
    ASSERT((int) list_remove_at(p_list, 0) == 2);
    
    ASSERT(list_size(p_list) == 0);
    
    for (i = 0; i < 5; ++i) 
    {
        ASSERT(list_push_front(p_list, 2 * i + 1));
    }
    
    ASSERT((int) list_remove_at(p_list, 2) == 5);
    ASSERT((int) list_pop_front(p_list) == 9);
    ASSERT((int) list_pop_back(p_list) == 1);
    ASSERT((int) list_pop_front(p_list) == 7);
    ASSERT((int) list_pop_back(p_list) == 3);
    
    list_insert(p_list, 0, 1);
    list_insert(p_list, 0, 2);
    
    ASSERT(list_size(p_list) == 2);
    list_clear(p_list);
    ASSERT(list_size(p_list) == 0);
    ASSERT(list_push_front(p_list, 2));
    ASSERT(list_push_front(p_list, 3));
    ASSERT(list_push_front(p_list, 4));
    list_free(p_list);
}

static void test_list_performance()
{
    list* p_list = list_alloc(10);
    const int sz = 1000000;
    
    clock_t t;
    double duration = 0.0;
    int i;
    int j;
    int a;
    int b;
    int tmp;
    int time_;
    void* p_element;
    
    puts("--- PERFORMANCE OF list_t ---");
    
    time_ = time(NULL);
    printf("Seed: %d.\n", time_);
    srand(time_);
    
    t = clock();
    
    for (i = 0; i < sz / 2; ++i) 
    {
        ASSERT(list_push_front(p_list, sz / 2 - i - 1));
    }
    
    for (i = sz / 2; i < sz; ++i) 
    {
        ASSERT(list_push_back(p_list, i));
    }
    
    duration += ((double) clock() - t);
    
    t = clock();

    for (i = 0; i < sz; ++i) 
    {
        ASSERT((int) list_get(p_list, i) == i);
    }
    
    duration += ((double) clock() - t);
    
    t = clock();
    
    for (i = 0; i < sz; ++i)
    {
        ASSERT((int) list_pop_front(p_list) == i);
    }
    
    duration += ((double) clock() - t);
    
    printf("Duration: %f seconds.\n", duration / CLOCKS_PER_SEC);        
}

static void test_fibonacci_heap_correctness() 
{
    fibonacci_heap* p_heap;
    size_t i;
    p_heap = fibonacci_heap_alloc(10, 
                                    1.0f, 
                                    hash_function, 
                                    equals_function, 
                                    priority_cmp);
    
    ASSERT(fibonacci_heap_is_healthy(p_heap));
    
    for (i = 0; i < 30; ++i) 
    {
        ASSERT(fibonacci_heap_add(p_heap, i, 30 - i));
    }
    
    for (i = 0; i < 30; ++i)
    {
        ASSERT(!fibonacci_heap_add(p_heap, i, i));
    }
    
    ASSERT(fibonacci_heap_size(p_heap) == 30);
    
    for (i = 0; i < 30; ++i) 
    {
        ASSERT(fibonacci_heap_contains_key(p_heap, i));
    }
    
    for (i = 30; i < 40; ++i) 
    {
        ASSERT(fibonacci_heap_contains_key(p_heap, i) == false);
    }
    
    ASSERT(fibonacci_heap_is_healthy(p_heap));
    
    for (i = 29; i != (size_t) -1; --i) 
    {
        ASSERT((size_t) fibonacci_heap_extract_min(p_heap) == i);
    }
    
    ASSERT(fibonacci_heap_size(p_heap) == 0);
    ASSERT(fibonacci_heap_is_healthy(p_heap));
    
    for (i = 10; i < 100; ++i) 
    {
        ASSERT(fibonacci_heap_add(p_heap, i, i));
    }
    
    ASSERT(fibonacci_heap_decrease_key(p_heap, 50, 0));
    
    ASSERT((size_t) fibonacci_heap_min(p_heap) == 50);
    ASSERT((size_t) fibonacci_heap_extract_min(p_heap) == 50);
    
    for (i = 10; i < 50; ++i) 
    {
        ASSERT((size_t) fibonacci_heap_min(p_heap) == i);
        ASSERT((size_t) fibonacci_heap_extract_min(p_heap) == i);
    }
    
    for (i = 51; i < 100; ++i) 
    {
        ASSERT((size_t) fibonacci_heap_min(p_heap) == i);
        ASSERT((size_t) fibonacci_heap_extract_min(p_heap) == i);
    }
    
    ASSERT(fibonacci_heap_min(p_heap) == NULL);
    ASSERT(fibonacci_heap_extract_min(p_heap) == NULL);
    
    for (i = 20; i < 40; ++i) 
    {
        ASSERT(fibonacci_heap_add(p_heap, i, i));
    }
    
    ASSERT(fibonacci_heap_size(p_heap) == 20);
    
    fibonacci_heap_clear(p_heap);
    
    ASSERT(fibonacci_heap_size(p_heap) == 0);
    ASSERT(fibonacci_heap_is_healthy(p_heap));
    
    for (i = 0; i < 100; ++i) 
    {
        ASSERT(fibonacci_heap_contains_key(p_heap, i) == false);
    }
    
    ASSERT(fibonacci_heap_add(p_heap, 2, 2));
    ASSERT(fibonacci_heap_add(p_heap, 3, 3));
    fibonacci_heap_free(p_heap);
}

static void test_fibonacci_heap_performance()
{
    fibonacci_heap* p_heap;
    size_t i;
    clock_t t;
    const size_t sz = 1000000;
    double duration;
    
    puts("--- PERFORMANCE OF fibonacci_heap_t ---");
    
    duration = 0.0;
    p_heap = fibonacci_heap_alloc(10,
                                    1.0f,
                                    hash_function,
                                    equals_function,
                                    priority_cmp);
    t = clock();

    for (i = 0; i < sz; ++i) 
    {
        fibonacci_heap_add(p_heap, i, 500000 + sz - i);
    }

    /* State: 999999, 999998, 999997, ... */
    ASSERT(fibonacci_heap_is_healthy(p_heap));

    for (i = sz / 2; i < sz; ++i)
    {
        fibonacci_heap_decrease_key(p_heap, i, i);
    }

    ASSERT(fibonacci_heap_is_healthy(p_heap));

    for (i = 0; i < sz; ++i)
    {
        fibonacci_heap_extract_min(p_heap);
    }

    duration += ((double) clock() - t);

    fibonacci_heap_free(p_heap);

    printf("Duration: %f seconds.\n", duration / CLOCKS_PER_SEC);
}

static int int_compare(const void* a, const void* b)
{
    return (*(int*) a - *(int*)b);
}

static void test_stable_sort() 
{
    size_t i;
    int arr[] = { 3, 0, 2, 3, 9, 7, 1, 5, 4, 6, 7 };

    stable_sort(arr, 11, sizeof(int), int_compare);
    puts("Sorted array:");
    
    for (i = 0; i < 11; ++i) 
    {
        printf("%d ", arr[i]);
    }
//    
//    int arr2[8];
//    
////    for (i = 0; i < 8; ++i) 
////    {
////        printf("%llu ", arr[i]);
////    }
//    
//    merge(arr, arr2, sizeof(int), 2, 3, 2, int_compare);
//    
//    for (i = 2; i < 7; ++i) 
//    {
//        printf("%d ", arr2[i]);
//    }
}

int main(int argc, char** argv) {
//    test_list_correctness();
//    test_list_performance();
//    
//    test_unordered_map_correctness();
//    test_unordered_map_performance();
//    
//    test_unordered_set_correctness();
//    test_unordered_set_performance();
//    
//    test_map_correctness();
//    test_map_performance();
//    
//    test_set_correctness();
//    test_set_performance();
//    
//    test_heap_correctness();
//    test_heap_performance();
//    
//    test_fibonacci_heap_correctness();
//    test_fibonacci_heap_performance(); 
    
    test_stable_sort();
    
    return (EXIT_SUCCESS);
}
