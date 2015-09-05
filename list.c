#include "list.h"
#include <stdbool.h>
#include <stdlib.h>

typedef struct list_t {
    void** p_table;
    size_t size;
    size_t capacity;
    size_t head;
    size_t mask;
} list_t;

static const size_t MINIMUM_CAPACITY = 16;

static size_t max(size_t a, size_t b)
{
    return a < b ? b : a;
}

static size_t fix_initial_capacity(size_t initial_capacity)
{
    size_t ret = 1;

    initial_capacity = max(initial_capacity, MINIMUM_CAPACITY);

    while (ret < initial_capacity) ret <<= 1;

    return ret;
}

list_t* list_t_alloc(size_t initial_capacity)
{
    list_t* p_ret = malloc(sizeof(*p_ret));

    if (!p_ret) return NULL;

    initial_capacity = fix_initial_capacity(initial_capacity);

    p_ret->p_table = malloc(sizeof(void*) * initial_capacity);

    if (!p_ret->p_table)
    {
        free(p_ret);
        return NULL;
    }

    p_ret->capacity = initial_capacity;
    p_ret->mask     = initial_capacity - 1;
    p_ret->head     = 0;
    p_ret->size     = 0;

    return p_ret;
}

static bool ensure_capacity_before_add(list_t* p_list)
{
    void** p_new_table;
    size_t i;
    size_t new_capacity;

    if (p_list->size < p_list->capacity) return true;

    new_capacity = 2 * p_list->capacity;
    p_new_table  = malloc(sizeof(void*) * new_capacity);

    if (!p_new_table) return false;

    for (i = 0; i < p_list->size; ++i) 
    {
        p_new_table[i] = p_list->p_table[(p_list->head + i) & p_list->mask];
    }

    free(p_list->p_table);
    p_list->p_table  = p_new_table;
    p_list->capacity = new_capacity;
    p_list->mask     = new_capacity - 1;
    p_list->head     = 0;

    return true;
}

bool list_t_push_front(list_t* p_list, void* p_element)
{
    if (!p_list)                             return false;
    if (!ensure_capacity_before_add(p_list)) return false;

    p_list->head = (p_list->head - 1) & p_list->mask;
    p_list->p_table[p_list->head] = p_element;
    p_list->size++;
    return true;
}

bool list_t_push_back(list_t* p_list, void* p_element)
{
    if (!p_list)                             return false;
    if (!ensure_capacity_before_add(p_list)) return false;

    p_list->p_table[(p_list->head + p_list->size) & p_list->mask] = p_element;
    p_list->size++;
    return true;
}

bool list_t_insert(list_t* p_list, size_t index, void* p_element)
{
    size_t elements_before;
    size_t elements_after;
    size_t i;
    size_t head;
    size_t mask;
    size_t size;

    if (!p_list)                             return false;
    if (!ensure_capacity_before_add(p_list)) return false;
    if (index > p_list->size)                return false;

    elements_before = index;
    elements_after  = p_list->size - index;
    head            = p_list->head;
    mask            = p_list->mask;
    size            = p_list->size;

    if (elements_before < elements_after) 
    {
        /* Move preceding elements one position to the left. */
        for (i = 0; i < elements_before; ++i)
        {
            p_list->p_table[(head + i - 1) & mask] =
            p_list->p_table[(head + i) & mask];
        }

        head = (head - 1) & mask;
        p_list->p_table[(head + index) & mask] = p_element;
        p_list->head = head;
    }
    else
    {
        /* Move the following elements one position to the right. */
        for (i = 0; i < elements_after; ++i)
        {
            p_list->p_table[(head + size - i) & mask] =
            p_list->p_table[(head + size - i - 1) & mask];
        }

        p_list->p_table[(head + index) & mask] = p_element;
    }

    p_list->size++;
    return true;
}

size_t list_t_size(list_t* p_list) 
{
    return p_list ? p_list->size : 0;
}

void* list_t_get(list_t* p_list, size_t index)
{
    if (!p_list)               return NULL;
    if (index >= p_list->size) return NULL;

    return p_list->p_table[(p_list->head + index) & p_list->mask];
}

void* list_t_set(list_t* p_list, size_t index, void* p_new_value) 
{
    void* p_ret;

    if (!p_list)               return NULL;
    if (index >= p_list->size) return NULL;

    p_ret = p_list->p_table[(p_list->head + index) & p_list->mask];
    p_list->p_table[(p_list->head + index) & p_list->mask] = p_new_value;
    return p_ret;
}

void* list_t_pop_front(list_t* p_list)
{
    void* p_ret;

    if (!p_list)           return NULL;   
    if (p_list->size == 0) return NULL;

    p_ret = p_list->p_table[p_list->head];
    p_list->head = (p_list->head + 1) & p_list->mask;
    p_list->size--;
    return p_ret;
}

void* list_t_pop_back(list_t* p_list)
{
    void* p_ret;

    if (!p_list)           return NULL;
    if (p_list->size == 0) return NULL;

    p_ret = p_list->p_table[(p_list->head + p_list->size - 1) & p_list->mask];
    p_list->size--;
    return p_ret;
}

void* list_t_remove_at(list_t* p_list, size_t index)
{
    void* p_ret;
    size_t head;
    size_t mask;
    size_t elements_before;
    size_t elements_after;
    size_t i;
    size_t j;

    if (!p_list)               return NULL;
    if (index >= p_list->size) return NULL;

    head = p_list->head;
    mask = p_list->mask;

    p_ret = p_list->p_table[(head + index) & mask];

    elements_before = index;
    elements_after  = p_list->size - index - 1;

    if (elements_before < elements_after)
    {
        /* Move the preceding elements one position to the right. */
        for (j = elements_before; j > 0; --j)
        {
            p_list->p_table[(head + j) & mask] =
            p_list->p_table[(head + j - 1) & mask];
        }

        p_list->head = (head + 1) & mask;
    }
    else
    {
        /* Move the following elements one position to the left. */
        for (i = 0; i < elements_after; ++i) 
        {
            p_list->p_table[(head + index + i) & mask] =
            p_list->p_table[(head + index + i + 1) & mask];
        }
    }

    p_list->size--;
    return p_ret;
}

bool list_t_contains(list_t* p_list, 
                        void* p_element,
                        bool (*p_equals_function)(void*, void*))
{
    size_t i;

    if (!p_list)            return false;
    if (!p_equals_function) return false;

    for (i = 0; i < p_list->size; ++i) 
    {
        if (p_equals_function(p_element, 
                              p_list->p_table[(p_list->head + i) & 
                              p_list->mask]))
        {
            return true;
        }
    }

    return false;
}

void list_t_clear(list_t* p_list)
{
    if (!p_list) return;

    p_list->head = 0;
    p_list->size = 0;
}

void list_t_free(list_t* p_list)
{
    if (!p_list) return;

    free(p_list->p_table);
    free(p_list);
}