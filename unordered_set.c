#include "unordered_set.h"
#include <stdbool.h>
#include <stdlib.h>

typedef struct unordered_set_entry_t {
    void*                         p_key;
    struct unordered_set_entry_t* p_chain_next;
    struct unordered_set_entry_t* p_prev;
    struct unordered_set_entry_t* p_next;
} unordered_set_entry_t;

typedef struct unordered_set_t {
    unordered_set_entry_t** p_table;
    unordered_set_entry_t*  p_head;
    unordered_set_entry_t*  p_tail;
    size_t                (*p_hash_function)(void*);
    bool                  (*p_equals_function)(void*, void*);
    size_t                  mod_count;
    size_t                  table_capacity;
    size_t                  size;
    size_t                  mask;
    size_t                  max_allowed_size;
    float                   load_factor;
} unordered_set_t;

typedef struct unordered_set_iterator_t {
    unordered_set_t*       p_map;
    unordered_set_entry_t* p_next_entry;
    size_t                 iterated_count;
    size_t                 expected_mod_count;
} unordered_set_iterator_t;

static unordered_set_entry_t* unordered_set_entry_t_alloc(void* p_key)
{
    unordered_set_entry_t* p_ret = malloc(sizeof(*p_ret));

    if (!p_ret) return NULL;

    p_ret->p_key        = p_key;
    p_ret->p_chain_next = NULL;
    p_ret->p_next       = NULL;
    p_ret->p_prev       = NULL;

    return p_ret;
}

static const float  MINIMUM_LOAD_FACTOR = 0.2f;
static const size_t MINIMUM_INITIAL_CAPACITY = 16;

static float maxf(float a, float b) 
{
    return a < b ? b : a;
}

static int maxi(int a, int b) 
{
    return a < b ? b : a;
}

/*******************************************************************************
* Makes sure that the load factor is no less than a minimum threshold.         *
*******************************************************************************/  
static float fix_load_factor(float load_factor)
{
    return maxf(load_factor, MINIMUM_LOAD_FACTOR);
}

/*******************************************************************************
* Makes sure that the initial capacity is no less than a minimum allowed and   *
* is a power of two.                                                           * 
*******************************************************************************/  
static size_t fix_initial_capacity(size_t initial_capacity) 
{
    size_t ret;

    initial_capacity = maxi(initial_capacity, MINIMUM_INITIAL_CAPACITY);
    ret = 1;

    while (ret < initial_capacity) ret <<= 1;
    return ret;
}

unordered_set_t* unordered_set_t_alloc(size_t initial_capacity,
                                       float load_factor,
                                       size_t (*p_hash_function)(void*),
                                       bool (*p_equals_function)(void*, void*))
{
    unordered_set_t* p_ret;

    if (!p_hash_function)   return NULL;
    if (!p_equals_function) return NULL;

    p_ret = malloc(sizeof(*p_ret));

    if (!p_ret) return NULL;

    load_factor      = fix_load_factor(load_factor);
    initial_capacity = fix_initial_capacity(initial_capacity);

    p_ret->load_factor       = load_factor;
    p_ret->table_capacity    = initial_capacity;
    p_ret->size              = 0;
    p_ret->mod_count         = 0;
    p_ret->p_head            = NULL;
    p_ret->p_tail            = NULL;
    p_ret->p_table           = calloc(initial_capacity, 
                                      sizeof(unordered_set_entry_t*));
    p_ret->p_hash_function   = p_hash_function;
    p_ret->p_equals_function = p_equals_function;
    p_ret->mask              = initial_capacity - 1;
    p_ret->max_allowed_size  = (size_t)(initial_capacity * load_factor);

    return p_ret;
}

static void ensure_capacity(unordered_set_t* p_set) 
{
    size_t new_capacity;
    size_t new_mask;
    size_t index;
    unordered_set_entry_t*  p_entry;
    unordered_set_entry_t** p_new_table;

    if (p_set->size < p_set->max_allowed_size) return;
    
    new_capacity = 2 * p_set->table_capacity;
    new_mask = new_capacity - 1;
    p_new_table = calloc(new_capacity, sizeof(unordered_set_entry_t*));

    if (!p_new_table) return;

    /* Rehash the entries. */
    for (p_entry = p_set->p_head; p_entry; p_entry = p_entry->p_next)
    {
        index = p_set->p_hash_function(p_entry->p_key) & new_mask;
        p_entry->p_chain_next = p_new_table[index];
        p_new_table[index] = p_entry;
    }

    free(p_set->p_table);
    
    p_set->p_table          = p_new_table;
    p_set->table_capacity   = new_capacity;
    p_set->mask             = new_mask;
    p_set->max_allowed_size = (size_t)(new_capacity * p_set->load_factor);
}

bool unordered_set_t_add(unordered_set_t* p_set, void* p_key)
{
    size_t index;
    unordered_set_entry_t* p_entry;

    if (!p_set) return NULL;

    index = p_set->p_hash_function(p_key) & p_set->mask;

    for (p_entry = p_set->p_table[index]; 
         p_entry;
         p_entry = p_entry->p_chain_next)
    {
        if (p_set->p_equals_function(p_entry->p_key, p_key))
        {
            return false;
        }
    }

    ensure_capacity(p_set);

    /* Recompute the index since it is possibly changed by 'ensure_capacity' */
    index = p_set->p_hash_function(p_key) & p_set->mask;
    p_entry               = unordered_set_entry_t_alloc(p_key);
    p_entry->p_chain_next = p_set->p_table[index];
    p_set->p_table[index] = p_entry;

    /* Link the new entry to the tail of the list. */
    if (!p_set->p_tail)
    {
        p_set->p_head = p_entry;
        p_set->p_tail = p_entry;
    }
    else
    {
        p_set->p_tail->p_next = p_entry;
        p_entry->p_prev = p_set->p_tail;
        p_set->p_tail = p_entry;
    }

    p_set->size++;
    p_set->mod_count++;
    return true;
}

bool unordered_set_t_contains(unordered_set_t* p_set, void* p_key)
{
    size_t index;
    unordered_set_entry_t* p_entry;

    if (!p_set) return false;

    index = p_set->p_hash_function(p_key) & p_set->mask;

    for (p_entry = p_set->p_table[index]; 
         p_entry; 
         p_entry = p_entry->p_chain_next) 
    {
        if (p_set->p_equals_function(p_key, p_entry->p_key)) return true;
    }

    return false;
}

bool unordered_set_t_remove(unordered_set_t* p_set, void* p_key)
{
    void*  p_ret;
    size_t index;
    unordered_set_entry_t* p_prev_entry;
    unordered_set_entry_t* p_current_entry;

    if (!p_set) return false;

    index = p_set->p_hash_function(p_key) & p_set->mask;

    p_prev_entry = NULL;

    for (p_current_entry = p_set->p_table[index];
         p_current_entry;
         p_current_entry = p_current_entry->p_chain_next)
    {
        if (p_set->p_equals_function(p_key, p_current_entry->p_key)) 
        {
            if (p_prev_entry)
            {
                /* Omit the 'p_current_entry' in the collision chain. */
                p_prev_entry->p_chain_next = p_current_entry->p_chain_next;
            }
            else
            {
                // Here?
                p_set->p_table[index] = p_current_entry->p_chain_next;
            }

            /* Unlink from the global iteration chain. */
            if (p_current_entry->p_prev && p_current_entry->p_next) 
            {
                /* Once here, the current entry has both next and previous. */
                p_current_entry->p_prev->p_next = p_current_entry->p_next;
                p_current_entry->p_next->p_prev = p_current_entry->p_prev;
            }
            else if (!p_current_entry->p_prev && !p_current_entry->p_next)
            {
                /* Once here, the current entry 
                   is the only entry in the chain. */
                p_set->p_head = NULL;
                p_set->p_tail = NULL;
            }
            else if (p_current_entry->p_next)
            {
                /* Once here, the current entry is the head of the chain. */
                p_set->p_head = p_current_entry->p_next;
                p_set->p_head->p_prev = NULL;
            }
            else
            {
                /* Once here, the current entry is the tail of the chain. */
                p_set->p_tail = p_current_entry->p_prev;
                p_set->p_tail->p_next = NULL;
            }

            p_set->size--;
            p_set->mod_count++;
            free(p_current_entry);
            return true;
        }

        p_prev_entry = p_current_entry;
    }

    return false;
}

void unordered_set_t_clear(unordered_set_t* p_map)
{
    unordered_set_entry_t* p_entry;
    unordered_set_entry_t* p_next_entry;
    size_t index;

    if (!p_map) return;

    p_entry = p_map->p_head;

    while (p_entry)
    {
        index = p_map->p_hash_function(p_entry->p_key) & p_map->mask;
        p_next_entry = p_entry->p_next;
        free(p_entry);
        p_entry = p_next_entry;

        if (p_map->p_table[index])
        {
            p_map->p_table[index] = p_map->p_table[index]->p_chain_next;
        }
    }

    p_map->mod_count += p_map->size;
    p_map->size = 0;
    p_map->p_head = NULL;
    p_map->p_tail = NULL;
}

size_t unordered_set_t_size(unordered_set_t* p_set)
{
    return p_set ? p_set->size : 0;
}

bool unordered_set_t_is_healthy(unordered_set_t* p_map)
{
    size_t counter;
    unordered_set_entry_t* p_entry;

    if (!p_map) return false;

    counter = 0;
    p_entry = p_map->p_head;

    if (p_entry && p_entry->p_prev) return false;

    for (; p_entry; p_entry = p_entry->p_next)
    {
        counter++;
    }

    return counter == p_map->size;
}

void unordered_set_t_free(unordered_set_t* p_map)
{
    if (!p_map) return;

    unordered_set_t_clear(p_map);
    free(p_map->p_table);
    free(p_map);
}

unordered_set_iterator_t* 
unordered_set_iterator_t_alloc(unordered_set_t* p_set)
{
    unordered_set_iterator_t* p_ret;

    if (!p_set) return NULL;

    p_ret = malloc(sizeof(*p_ret));

    if (!p_ret) return NULL;

    p_ret->p_map              = p_set;
    p_ret->iterated_count     = 0;
    p_ret->p_next_entry       = p_set->p_head;
    p_ret->expected_mod_count = p_set->mod_count;

    return p_ret;
}

size_t unordered_set_iterator_t_has_next(unordered_set_iterator_t* p_iterator)
{
    if (!p_iterator) return 0;

    if (unordered_set_iterator_t_is_disturbed(p_iterator)) return 0;

    return p_iterator->p_map->size - p_iterator->iterated_count;
}

bool unordered_set_iterator_t_next(unordered_set_iterator_t* p_iterator, 
                                   void** pp_key)
{
    if (!p_iterator)                                       return false;
    if (!p_iterator->p_next_entry)                         return false;
    if (unordered_set_iterator_t_is_disturbed(p_iterator)) return false;

    *pp_key = p_iterator->p_next_entry->p_key;
    p_iterator->iterated_count++;
    p_iterator->p_next_entry = p_iterator->p_next_entry->p_next;
    return true;
}

bool
unordered_set_iterator_t_is_disturbed(unordered_set_iterator_t* p_iterator)
{
    if (!p_iterator) false;

    return p_iterator->expected_mod_count != p_iterator->p_map->mod_count;
}

void unordered_set_iterator_t_free(unordered_set_iterator_t* p_iterator)
{
    if (!p_iterator) return;

    p_iterator->p_map = NULL;
    p_iterator->p_next_entry = NULL;
    free(p_iterator);
}

