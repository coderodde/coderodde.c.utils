#include "stable_sort.h"
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
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

static void run_length_queue_free(run_length_queue* queue) 
{
    if (queue) 
    {
        if (queue->storage) 
        {
            free(queue->storage);
        }
        
        free(queue);
    }
}

static void reverse_run(char* base, size_t num, size_t size, void* swap_buffer)
{
    size_t left = 0;
    size_t right = num - 1;

    while (left < right)
    {
        memcpy(swap_buffer, base + size * left, size);
        memcpy(base + size * left, base + size * right, size);
        memcpy(base + size * right, swap_buffer, size);

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
    queue = run_length_queue_alloc((num >> 1) + 1);

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

            if (previous_was_descending
                    && cmp(((char*) base) + size * (head - 1),
                           ((char*) base) + size * head) <= 0)
            {
                    run_length_queue_add_to_last(queue, run_length);
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

            if (previous_was_descending
                    && cmp(((char*) base) + size * (head - 1),
                           ((char*) base) + size * head) <= 0)
            {
                run_length_queue_add_to_last(queue, run_length);
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

void merge(void* source,
                  void* target,
                  size_t size,
                  size_t offset,
                  size_t left_run_length,
                  size_t right_run_length,
                  int (*cmp)(const void*, const void*)) 
{
    size_t left  = offset;
    size_t right = left + left_run_length;
    const size_t left_bound = right;
    const size_t right_bound = right + right_run_length;
    size_t target_index = offset;

    while (left < left_bound && right < right_bound)
    {
        if (cmp(((char*) source) + size * right, 
                ((char*) source) + size * left) < 0) 
        {
            memcpy(((char*) target) + size * target_index, 
                   ((char*) source) + size * right,
                   size);
            ++right;
        } 
        else
        {
            memcpy(((char*) target) + size * target_index,
                   ((char*) source) + size * left,
                   size);
            ++left;
        }

        ++target_index;
    }

    memcpy(((char*) target) + size * target_index, 
           ((char*) source) + size * left,
           (left_bound - left) * size);

    memcpy(((char*) target) + size * target_index,
           ((char*) source) + size * right,
           (right_bound - right) * size);
}

static size_t get_number_of_leading_zeros(size_t number)
{
    size_t mask = 1; 
    size_t number_of_leading_zeros = 0;

    mask <<= (sizeof number) * CHAR_BIT - 1;

    while (mask && ((mask & number) == 0))
    {
        ++number_of_leading_zeros;
        mask >>= 1;
    }

    return number_of_leading_zeros;
}

static size_t get_number_of_merge_passes(size_t runs) 
{
    return sizeof(size_t) * CHAR_BIT - 
           get_number_of_leading_zeros(runs - 1);
}

void stable_sort(void* base, size_t num, size_t size, int (*comparator)(const void*, const void*))
{
    size_t i;

    run_length_queue* queue;

    void* buffer;
    void* source;
    void* target;
    void* tmp;

    size_t offset;
    size_t merge_passes;
    size_t runs_remaining;
    size_t tail_run_length;
    size_t left_run_length;
    size_t right_run_length;

    if (!base || !comparator || num < 2 || size == 0) 
    {
        return;
    }

    buffer = malloc(num * size);

    if (!buffer)
    {
        qsort(base, num, size, comparator);
        return;
    }

    queue = build_run_length_queue(base, num, size, comparator);

    if (!queue) 
    {
        free(buffer);
        
        /* Cannot allocate the run length queue. Resort to qsort and possibly 
           fail in the same manner as qsort. */
        qsort(base, num, size, comparator);
        return;
    }

    merge_passes = get_number_of_merge_passes(run_length_queue_size(queue));

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

    offset = 0;
    runs_remaining = run_length_queue_size(queue);

    while (run_length_queue_size(queue) > 1) 
    {
        left_run_length  = run_length_queue_dequeue(queue);
        right_run_length = run_length_queue_dequeue(queue);

        merge(source,
              target,
              size,
              offset,
              left_run_length,
              right_run_length,
              comparator);

        run_length_queue_enqueue(queue, left_run_length + right_run_length);
        runs_remaining -= 2;
        offset += left_run_length + right_run_length;

        switch (runs_remaining)
        {
            case 1:
                tail_run_length = run_length_queue_dequeue(queue);
                memcpy(((char*) target) + offset * size,
                       ((char*) source) + offset * size,
                       size * tail_run_length);
                run_length_queue_enqueue(queue, tail_run_length);
                /* FALL THROUGH! */

            case 0:
                runs_remaining = run_length_queue_size(queue);
                offset = 0;

                tmp = source;
                source = target;
                target = tmp;
                break;
        }
    }

    run_length_queue_free(queue);
    free(buffer);
}
