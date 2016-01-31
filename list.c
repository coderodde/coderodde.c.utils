#include "list.h"
#include <stdbool.h>
#include <stdlib.h>

struct list {
    void** p_table;
    size_t size;
    size_t capacity;
    size_t head;
    size_t mask;
};

static const size_t MINIMUM_CAPACITY = 16;

static size_t max(size_t a, size_t b)
{
    return a < b ? b : a;
}

static size_t fix_initial_capacity(size_t initial_capacity)
{
    size_t ret = 1;

    initial_capacity = max(initial_capacity, MINIMUM_CAPACITY);

    while (ret < initial_capacity) 
    {
        ret <<= 1;
    }
    
    return ret;
}

list* list_alloc(size_t initial_capacity)
{
    list* p_ret = malloc(sizeof(*p_ret));

    if (!p_ret) 
    {
        return NULL;
    }
    
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

static bool ensure_capacity_before_add(list* my_list)
{
    void** p_new_table;
    size_t i;
    size_t new_capacity;

    if (my_list->size < my_list->capacity)
    {
        return true;
    }
    
    new_capacity = 2 * my_list->capacity;
    p_new_table  = malloc(sizeof(void*) * new_capacity);

    if (!p_new_table) 
    {
        return false;
    }
    
    for (i = 0; i < my_list->size; ++i) 
    {
        p_new_table[i] = my_list->p_table[(my_list->head + i) & my_list->mask];
    }

    free(my_list->p_table);
    my_list->p_table  = p_new_table;
    my_list->capacity = new_capacity;
    my_list->mask     = new_capacity - 1;
    my_list->head     = 0;

    return true;
}

bool list_push_front(list* my_list, void* element)
{
    if (!my_list)           
    {
        return false;
    }
    
    if (!ensure_capacity_before_add(my_list)) 
    {
        return false;
    }
    
    my_list->head = (my_list->head - 1) & my_list->mask;
    my_list->p_table[my_list->head] = element;
    my_list->size++;
    
    return true;
}

bool list_push_back(list* my_list, void* element)
{
    if (!my_list)                             
    {
        return false;
    }
    
    if (!ensure_capacity_before_add(my_list))
    {
        return false;
    }
    
    my_list->p_table[(my_list->head + my_list->size) & my_list->mask] = element;
    my_list->size++;
    return true;
}

bool list_insert(list* my_list, size_t index, void* element)
{
    size_t elements_before;
    size_t elements_after;
    size_t i;
    size_t head;
    size_t mask;
    size_t size;

    if (!my_list)                          
    {
        return false;
    }
        
    if (!ensure_capacity_before_add(my_list)) 
    {
        return false;
    }
    
    if (index > my_list->size)                
    {
        return false;
    }
    
    elements_before = index;
    elements_after  = my_list->size - index;
    head            = my_list->head;
    mask            = my_list->mask;
    size            = my_list->size;

    if (elements_before < elements_after) 
    {
        /* Move preceding elements one position to the left. */
        for (i = 0; i < elements_before; ++i)
        {
            my_list->p_table[(head + i - 1) & mask] =
            my_list->p_table[(head + i) & mask];
        }

        head = (head - 1) & mask;
        my_list->p_table[(head + index) & mask] = element;
        my_list->head = head;
    }
    else
    {
        /* Move the following elements one position to the right. */
        for (i = 0; i < elements_after; ++i)
        {
            my_list->p_table[(head + size - i) & mask] =
            my_list->p_table[(head + size - i - 1) & mask];
        }

        my_list->p_table[(head + index) & mask] = element;
    }

    my_list->size++;
    return true;
}

size_t list_size(list* my_list) 
{
    return my_list ? my_list->size : 0;
}

void* list_get(list* my_list, size_t index)
{
    if (!my_list)               
    {
        return NULL;
    }
    
    if (index >= my_list->size) 
    {
        return NULL;
    }
    
    return my_list->p_table[(my_list->head + index) & my_list->mask];
}

void* list_set(list* my_list, size_t index, void* p_new_value) 
{
    void* p_ret;

    if (!my_list)      
    {
        return NULL;
    }
    
    if (index >= my_list->size) 
    {
        return NULL;
    }
    
    p_ret = my_list->p_table[(my_list->head + index) & my_list->mask];
    my_list->p_table[(my_list->head + index) & my_list->mask] = p_new_value;
    return p_ret;
}

void* list_pop_front(list* my_list)
{
    void* p_ret;

    if (!my_list)           
    {
        return NULL;   
    }
    
    if (my_list->size == 0) 
    {
        return NULL;
    }
    
    p_ret = my_list->p_table[my_list->head];
    my_list->head = (my_list->head + 1) & my_list->mask;
    my_list->size--;
    return p_ret;
}

void* list_pop_back(list* my_list)
{
    void* p_ret;

    if (!my_list)          
    {
        return NULL;
    }
    
    if (my_list->size == 0) 
    {
        return NULL;
    }
    
    p_ret = my_list->p_table[(my_list->head + my_list->size - 1) & 
                              my_list->mask];
    my_list->size--;
    return p_ret;
}

void* list_remove_at(list* my_list, size_t index)
{
    void* p_ret;
    size_t head;
    size_t mask;
    size_t elements_before;
    size_t elements_after;
    size_t i;
    size_t j;

    if (!my_list)               
    {
        return NULL;
    }
        
    if (index >= my_list->size)
    {
        return NULL;
    }
    
    head = my_list->head;
    mask = my_list->mask;

    p_ret = my_list->p_table[(head + index) & mask];

    elements_before = index;
    elements_after  = my_list->size - index - 1;

    if (elements_before < elements_after)
    {
        /* Move the preceding elements one position to the right. */
        for (j = elements_before; j > 0; --j)
        {
            my_list->p_table[(head + j) & mask] =
            my_list->p_table[(head + j - 1) & mask];
        }

        my_list->head = (head + 1) & mask;
    }
    else
    {
        /* Move the following elements one position to the left. */
        for (i = 0; i < elements_after; ++i) 
        {
            my_list->p_table[(head + index + i) & mask] =
            my_list->p_table[(head + index + i + 1) & mask];
        }
    }

    my_list->size--;
    return p_ret;
}

bool list_contains(list* my_list, 
                        void* p_element,
                        bool (*p_equals_function)(void*, void*))
{
    size_t i;

    if (!my_list)           
    {
        return false;
    }
    
    if (!p_equals_function) 
    {
        return false;
    }
    
    for (i = 0; i < my_list->size; ++i) 
    {
        if (p_equals_function(p_element, 
                              my_list->p_table[(my_list->head + i) & 
                              my_list->mask]))
        {
            return true;
        }
    }

    return false;
}

void list_clear(list* my_list)
{
    if (!my_list) 
    {
        return;
    }
    
    my_list->head = 0;
    my_list->size = 0;
}

void list_free(list* my_list)
{
    if (!my_list) 
    {
        return;
    }
    
    free(my_list->p_table);
    free(my_list);
}
