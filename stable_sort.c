#include "stable_sort.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct run_length_queue {
    int*   storage;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t size;
    size_t mask;
} run_length_queue;

static size_t fix_capacity(size_t capacity) 
{
    size_t ret = 1;
    
    while (ret < capacity) 
    {
        ret <<= 1;
    }
    
    return ret;
}

static size_t max(size_t a, size_t b) 
{
    return a > b ? a : b;
}

static const size_t MINIMUM_RUN_LENGTH_QUEUE_CAPACITY = 256;

static run_length_queue* run_length_queue_alloc(size_t capacity) 
{
    run_length_queue* queue;
    
    capacity = max(capacity, MINIMUM_RUN_LENGTH_QUEUE_CAPACITY);
    capacity = fix_capacity(capacity);
    
    queue = malloc(sizeof(*queue));
    
    if (!queue) 
    {
        return NULL;
    }
    
    queue->storage = malloc(sizeof(int) * capacity);
    
    if (!queue->storage)
    {
        free(queue);
        return NULL;
    }
    
    queue->capacity = capacity;
    queue->mask = capacity - 1;
    queue->head = 0;
    queue->tail = 0;
    queue->size = 0;
    
    return queue;
}

static void run_length_queue_enqueue(run_length_queue* queue, size_t run_size)
{
    queue->storage[queue->tail] = run_size;
    queue->tail = (queue->tail + 1) & queue->mask;
    queue->size++;
}

static void run_length_queue_add_to_last(run_length_queue* queue, 
                                         size_t run_size)
{
    queue->storage[(queue->tail - 1) & queue->mask] += run_size;
}

static size_t run_length_queue_dequeue(run_length_queue* queue)
{
    size_t run_length = queue->storage[queue->head];
    queue->head = (queue->head + 1) & queue->mask;
    queue->size--;
    return run_length;
}

static size_t run_length_queue_size(run_length_queue* queue) 
{
    return queue->size;
}

static void reverse_run(char* base, size_t num, size_t size, void* swap_buffer)
{
    size_t left = 0;
    size_t right = num - 1;
    
    while (left < right)
    {
        memcpy(swap_buffer, base + left * num, size);
        memcpy(base + left * num, base + right * num, size);
        memcpy(base + right * num, swap_buffer, size);
        ++left;
        --right;
    }
}

static run_length_queue* 
build_run_length_queue(void* base, 
                       size_t num,
                       size_t size,
                       int (*cmp)(const void*, const void*)) 
{
    run_length_queue* queue;
    size_t head;
    size_t left;
    size_t right;
    size_t last;
    size_t run_length;
    bool previous_was_descending;
    void* swap_buffer = malloc(size);
    queue = run_length_queue_alloc((num >>= 1) + 1);
    
    if (!queue)
    {
        return NULL;
    }
    
    left = 0;
    right = 1;
    last = num - 1;
    previous_was_descending = false;
    
    while (left < last)
    {
        head = left;
        
        /* Decide the direction of the next run. */
        if (cmp(((char*) base) + size * left++, 
                ((char*) base) + size * right++) <= 0)
        {
            /* The run is ascending. */
            while (left < last 
                    && cmp(((char*) base) + size * left, 
                           ((char*) base) + size * right) <= 0) 
            {
                ++left;
                ++right;
            }
            
            run_length = left - head + 1;
            
            if (previous_was_descending)
            {
                if (cmp(((char*) base) + (head - 1) * size, 
                        ((char*) base) + head * size) <= 0)
                {
                    run_length_queue_add_to_last(queue, run_length);
                }
                else
                {
                    run_length_queue_enqueue(queue, run_length);
                }
            }
            else
            {
                run_length_queue_enqueue(queue, run_length);
            }
            
            previous_was_descending = false;
        }
        else
        {
            /* Scan a strictly descending run. */
            while (left < last
                    && cmp(((char*) base) + size * left, 
                           ((char*) base) + size * right) > 0)
            {
                ++left;
                ++right;
            }
            
            run_length = left - head + 1;
            reverse_run(((char*) base) + head * size, 
                        run_length,
                        size, 
                        swap_buffer);
            
            if (previous_was_descending)
            {
                if (cmp(((char*) base) + size * (head - 1), 
                        ((char*) base) + size * head) <= 0) 
                {
                    run_length_queue_add_to_last(queue, run_length);
                }
                else
                {
                    run_length_queue_enqueue(queue, run_length);
                }
            }
            else
            {
                run_length_queue_enqueue(queue, run_length);
            }
            
            previous_was_descending = true;
        }
        
        ++left;
        ++right;
    }
    
    if (left == last)
    {
        if (cmp(((char*) base) + size * (last - 1), 
                ((char*) base) + size * last) <= 0) 
        {
            run_length_queue_add_to_last(queue, 1);
        }
        else 
        {
            run_length_queue_enqueue(queue, 1);
        }
    }
    
    free(swap_buffer);
    return queue;
}

void stable_sort(void* base, size_t num, size_t size, int (*comparator)(const void*, const void*))
{
    size_t i;
    run_length_queue* queue;
    
    if (!base || !comparator || num < 2 || size == 0) 
    {
        return;
    }
    
    queue = build_run_length_queue(base, num, size, comparator);
    puts("FDSFA");
    printf("size %d\n", run_length_queue_size(queue));
    for (i = 0; i < run_length_queue_size(queue); ++i) 
    {
        printf("%llz ", run_length_queue_dequeue(queue));
    }
    
    if (!queue) 
    {
        /* Cannot allocate the run length queue. Resort to qsort and possibly 
           fail in the same manner as qsort. */
        qsort(base, num, size, comparator);
        return;
    }
}
