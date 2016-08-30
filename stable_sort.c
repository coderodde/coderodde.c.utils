#include "stable_sort.h"
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct run_length_queue
{
    size_t* storage;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t size;
    size_t mask;
}
run_length_queue;

static size_t fix_capacity_to_power_of_two(const size_t capacity)
{
    size_t ret = 1;
    
    while (ret < capacity)
    {
        ret <<= 1; // Multiply 'ret' by two.
    }
    
    return ret; // Now 'ret' is a power of two no less than 'capacity'.
}

static size_t max(const size_t a, const size_t b)
{
    return a > b ? a : b;
}

static void run_length_queue_init(run_length_queue *const queue,
                                  size_t capacity)
{
    capacity = max(capacity, fix_capacity_to_power_of_two(capacity));
    queue->storage = malloc(sizeof(size_t) * capacity);
    
    if (!queue->storage)
    {
        fputs("Could not allocate memory for the storage array of the run "
              "length queue\n",
              stderr);
        
        abort();
    }
    
    queue->capacity = capacity;
    queue->mask = capacity - 1;
    queue->head = 0;
    queue->tail = 0;
    queue->size = 0;
}

static inline void run_length_queue_add_to_last(run_length_queue *const queue,
                                                const size_t run_length)
{
    queue->storage[(queue->tail - 1) & queue->mask] += run_length;
}

static inline void run_length_queue_enqueue(run_length_queue *const queue,
                                            const size_t run_length)
{
    queue->storage[queue->tail] = run_length;
    queue->tail = (queue->tail + 1) & queue->mask;
    queue->size++;
}

static inline size_t run_length_queue_dequeue(run_length_queue *const queue)
{
    const size_t run_length = queue->storage[queue->head];
    queue->head = (queue->head + 1) & queue->mask;
    queue->size--;
    return run_length;
}

static inline size_t run_length_queue_size(const run_length_queue *const queue)
{
    return queue->size;
}

void run_length_queue_free(const run_length_queue *const queue)
{
    free(queue->storage);
}

static inline void reverse_run(char* base,
                               size_t num,
                               size_t size,
                               char* swap_buffer)
{
    char* left  = (char*) base;
    char* right = (char*)(base + ((num - 1) * size));
    
    while (left < right)
    {
        memcpy(swap_buffer, left, size);
        memcpy(left, right, size);
        memcpy(right, swap_buffer, size);
        
        left  += size;
        right -= size;
    }
}

static run_length_queue*
build_run_length_queue(void* base,
                       size_t num,
                       size_t size,
                       const int (*cmp)(const void*, const void*))
{
    run_length_queue* queue = malloc(sizeof(run_length_queue));
    
    if (!queue)
    {
        fputs("Could not allocate memory for the actual run length queue.\n",
              stderr);
        
        abort();
    }
    
    run_length_queue_init(queue, (num + 1) / 2);
    
    // The buffer for doing the run reversals:
    char* swap_buffer = malloc(size);
    
    if (!swap_buffer)
    {
        fputs("Could not allocate memory for the swap buffer.\n", stderr);
        abort();
    }
    
    char* head  = (char*) base;
    char* left  = head;
    char* right = ((char*) base) + size;
    char* last  = ((char*) base) + size * (num - 1);
    
    size_t run_length;
    bool previous_run_was_descending;
    
    // Find the very first run:
    if (cmp(left, right) <= 0)
    {
        // Once here, the first run is ascending.
        left  += size;
        right += size;
        
        while (left < last && cmp(left, right) <= 0)
        {
            left  += size;
            right += size;
        }
        
        run_length = (right - head) / size;
        run_length_queue_enqueue(queue, run_length);
        previous_run_was_descending = false;
    }
    else
    {
        left  += size;
        right += size;
        
        while (left < last && cmp(left, right) > 0)
        {
            left  += size;
            right += size;
        }
        
        run_length = (right - head) / size;
        run_length_queue_enqueue(queue, run_length);
        reverse_run(head, run_length, size, swap_buffer);
        previous_run_was_descending = true;
    }
    
    left  += size;
    right += size;
    
    // Scan all other runs:
    while (left < last)
    {
        head = left;
        
        // Decide the direction of the current run:
        if (cmp(left, right) <= 0)
        {
            // Once here, the current run is ascending.
            left  += size;
            right += size;
            
            while (left < last && cmp(left, right) <= 0)
            {
                left  += size;
                right += size;
            }
            
            run_length = (right - head) / size;
            
            if (previous_run_was_descending
                && cmp(head - size, head) <= 0)
            {
                // We can extend the size of the preceding run:
                run_length_queue_add_to_last(queue, run_length);
            }
            else
            {
                run_length_queue_enqueue(queue, run_length);
            }
            
            previous_run_was_descending = false;
        }
        else
        {
            // Once here, the current run is descending.
            left  += size;
            right += size;
            
            while (left < last && cmp(left, right) > 0)
            {
                left  += size;
                right += size;
            }
            
            run_length = (right - head) / size;
            reverse_run(head, run_length, size, swap_buffer);
            
            if (previous_run_was_descending && cmp(head - size, head) <= 0)
            {
                run_length_queue_add_to_last(queue, run_length);
            }
            else
            {
                run_length_queue_enqueue(queue, run_length);
            }
            
            previous_run_was_descending = true;
        }
        
        left  += size;
        right += size;
    }
    
    if (left == last)
    {
        if (cmp(last - size, last) <= 0)
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

static inline void merge(char *const source,
                         char* target,
                         const size_t left_run_length,
                         const size_t right_run_length,
                         const size_t size,
                         const int (*cmp)(const void*, const void*))
{
    const char* left_upper_bound  = source + left_run_length * size;
    const char* right_upper_bound = left_upper_bound + right_run_length * size;
    
    char* left  = source;
    char* right = left_upper_bound;
    
    while (left != left_upper_bound && right != right_upper_bound)
    {
        if (cmp(right, left) < 0)
        {
            memcpy(target, right, size);
            right += size;
        }
        else
        {
            memcpy(target, left, size);
            left += size;
        }
        
        target += size;
    }
    
    memcpy(target, left,  left_upper_bound - left);
    memcpy(target, right, right_upper_bound - right);
}

static size_t get_number_of_leading_zeros(const size_t number)
{
    size_t mask = 1;
    size_t number_of_leading_zeros = 0;
    mask <<= sizeof (size_t) * CHAR_BIT - 1;
    
    while (mask && ((mask & number) == 0))
    {
        ++number_of_leading_zeros;
        mask >>= 1;
    }
    
    return number_of_leading_zeros;
}

static size_t get_number_of_merge_passes(const size_t runs)
{
    return sizeof(size_t) * CHAR_BIT - get_number_of_leading_zeros(runs - 1);
}

void stable_sort(void* base,
                 const size_t num,
                 const size_t size,
                 const int (*cmp)(const void*, const void*))
{
    if (!base || !cmp)
    {
        return;
    }
    
    char* buffer = malloc(num * size);
    
    if (!buffer)
    {
        fputs("Could not allocate memory for the buffer array.\n", stderr);
        abort();
    }
    
    run_length_queue* queue = build_run_length_queue(base, num, size, cmp);
    const size_t merge_passes = get_number_of_merge_passes(
                                                           run_length_queue_size(queue));
    char* source;
    char* target;
    
    if ((merge_passes & 1) == 1)
    {
        source = buffer;
        target = base;
        memcpy(buffer, base, num * size);
    }
    else
    {
        source = base;
        target = buffer;
    }
    
    size_t remaining_runs = run_length_queue_size(queue);
    char* target_pointer = target;
    char* source_pointer = source;
    
    while (run_length_queue_size(queue) != 1)
    {
        const size_t left_run_length  = run_length_queue_dequeue(queue);
        const size_t right_run_length = run_length_queue_dequeue(queue);
        
        merge(source_pointer,
              target_pointer,
              left_run_length,
              right_run_length,
              size,
              cmp);
        
        run_length_queue_enqueue(queue, left_run_length + right_run_length);
        
        const size_t current_bytes =
        (left_run_length + right_run_length) * size;
        
        target_pointer += current_bytes;
        source_pointer += current_bytes;
        remaining_runs -= 2;
        
        size_t tail_run_length;
        
        switch (remaining_runs)
        {
            case 1:
                tail_run_length = run_length_queue_dequeue(queue);
                memcpy(target_pointer, source_pointer, tail_run_length * size);
                run_length_queue_enqueue(queue, tail_run_length);
                // FALLTHROUGH!
                
            case 0:
                remaining_runs = run_length_queue_size(queue);
                
                // Swap the roles of the arrays:
                char* tmp = source;
                source = target;
                target = tmp;
                
                // Reset the pointer to the respective beginnig of the arrays:
                target_pointer = target;
                source_pointer = source;
        }
    }
    
    run_length_queue_free(queue);
    free(buffer);
}
