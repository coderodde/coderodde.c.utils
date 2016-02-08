#include "unordered_set.h"
#include <stdbool.h>
#include <stdlib.h>

typedef struct unordered_set_entry {
    void*                       key;
    struct unordered_set_entry* chain_next;
    struct unordered_set_entry* prev;
    struct unordered_set_entry* next;
} unordered_set_entry;

struct unordered_set {
    unordered_set_entry** table;
    unordered_set_entry*  head;
    unordered_set_entry*  tail;
    size_t              (*hash_function)(void*);
    bool                (*equals_function)(void*, void*);
    size_t                mod_count;
    size_t                table_capacity;
    size_t                size;
    size_t                mask;
    size_t                max_allowed_size;
    float                 load_factor;
};

struct unordered_set_iterator {
    unordered_set*       map;
    unordered_set_entry* next_entry;
    size_t               iterated_count;
    size_t               expected_mod_count;
};

static unordered_set_entry* unordered_set_entry_t_alloc(void* key)
{
    unordered_set_entry* entry = malloc(sizeof(*entry));

    if (!entry) 
    {
        return NULL;
    }
    
    entry->key        = key;
    entry->chain_next = NULL;
    entry->next       = NULL;
    entry->prev       = NULL;

    return entry;
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

    while (ret < initial_capacity) 
    {
        ret <<= 1;
    }
        
    return ret;
}

unordered_set* unordered_set_t_alloc(size_t initial_capacity,
                                     float load_factor,
                                     size_t (*hash_function)(void*),
                                     bool (*equals_function)(void*, void*))
{
    unordered_set* set;

    if (!hash_function || !equals_function)   
    {
        return NULL;
    }
     
    set = malloc(sizeof(*set));

    if (!set) 
    {
        return NULL;
    }
    
    load_factor      = fix_load_factor(load_factor);
    initial_capacity = fix_initial_capacity(initial_capacity);

    set->load_factor      = load_factor;
    set->table_capacity   = initial_capacity;
    set->size             = 0;
    set->mod_count        = 0;
    set->head             = NULL;
    set->tail             = NULL;
    set->table            = calloc(initial_capacity, 
                                   sizeof(unordered_set_entry*));
    set->hash_function    = hash_function;
    set->equals_function  = equals_function;
    set->mask             = initial_capacity - 1;
    set->max_allowed_size = (size_t)(initial_capacity * load_factor);

    return set;
}

static void ensure_capacity(unordered_set* set) 
{
    size_t new_capacity;
    size_t new_mask;
    size_t index;
    unordered_set_entry*  entry;
    unordered_set_entry** new_table;

    if (set->size < set->max_allowed_size) 
    {
        return;
    }
    
    new_capacity = 2 * set->table_capacity;
    new_mask = new_capacity - 1;
    new_table = calloc(new_capacity, sizeof(unordered_set_entry*));

    if (!new_table)
    {
        return;
    }
    
    /* Rehash the entries. */
    for (entry = set->head; entry; entry = entry->next)
    {
        index = set->hash_function(entry->key) & new_mask;
        entry->chain_next = new_table[index];
        new_table[index] = entry;
    }

    free(set->table);
    
    set->table            = new_table;
    set->table_capacity   = new_capacity;
    set->mask             = new_mask;
    set->max_allowed_size = (size_t)(new_capacity * set->load_factor);
}

bool unordered_set_t_add(unordered_set* set, void* key)
{
    size_t index;
    size_t hash_value;
    unordered_set_entry* entry;
    
    if (!set) 
    {
        return NULL;
    }
    
    hash_value = set->hash_function(key);
    index      = hash_value & set->mask;

    for (entry = set->table[index]; entry; entry = entry->chain_next)
    {
        if (set->equals_function(entry->key, key))
        {
            return false;
        }
    }

    ensure_capacity(set);

    /* Recompute the index since it is possibly changed by 'ensure_capacity' */
    index             = hash_value & set->mask;
    entry             = unordered_set_entry_t_alloc(key);
    entry->chain_next = set->table[index];
    set->table[index] = entry;

    /* Link the new entry to the tail of the list. */
    if (!set->tail)
    {
        set->head = entry;
        set->tail = entry;
    }
    else
    {
        set->tail->next = entry;
        entry->prev = set->tail;
        set->tail = entry;
    }

    set->size++;
    set->mod_count++;
    
    return true;
}

bool unordered_set_t_contains(unordered_set* set, void* key)
{
    size_t index;
    unordered_set_entry* p_entry;

    if (!set) 
    {
        return false;
    }
    
    index = set->hash_function(key) & set->mask;

    for (p_entry = set->table[index]; p_entry; p_entry = p_entry->chain_next) 
    {
        if (set->equals_function(key, p_entry->key))
        {
            return true;
        }
    }
    
    return false;
}

bool unordered_set_t_remove(unordered_set* set, void* key)
{
    size_t index;
    unordered_set_entry* prev_entry;
    unordered_set_entry* current_entry;

    if (!set) 
    {
        return false;
    }
    
    index = set->hash_function(key) & set->mask;

    prev_entry = NULL;

    for (current_entry = set->table[index];
         current_entry;
         current_entry = current_entry->chain_next)
    {
        if (set->equals_function(key, current_entry->key)) 
        {
            if (prev_entry)
            {
                /* Omit the 'p_current_entry' in the collision chain. */
                prev_entry->chain_next = current_entry->chain_next;
            }
            else
            {
                set->table[index] = current_entry->chain_next;
            }

            /* Unlink from the global iteration chain. */
            if (current_entry->prev)
            {
                current_entry->prev->next = current_entry->next;
            } 
            else
            {
                set->head = current_entry->next;
            }
            
            if (current_entry->next)
            {
                current_entry->next->prev = current_entry->prev;
            }
            else
            {
                set->tail = current_entry->prev;
            }
            
            set->size--;
            set->mod_count++;
            free(current_entry);
            return true;
        }

        prev_entry = current_entry;
    }

    return false;
}

void unordered_set_t_clear(unordered_set* set)
{
    unordered_set_entry* entry;
    unordered_set_entry* next_entry;
    size_t index;

    if (!set) 
    {
        return;
    }
    
    entry = set->head;

    while (entry)
    {
        index = set->hash_function(entry->key) & set->mask;
        next_entry = entry->next;
        free(entry);
        entry = next_entry;
        set->table[index] = NULL;
    }

    set->mod_count += set->size;
    set->size = 0;
    set->head = NULL;
    set->tail = NULL;
}

size_t unordered_set_t_size(unordered_set* set)
{
    return set ? set->size : 0;
}

bool unordered_set_t_is_healthy(unordered_set* set)
{
    size_t counter;
    unordered_set_entry* entry;

    if (!set)
    {
        return false;
    }
    
    counter = 0;
    entry = set->head;

    if (entry && entry->prev) 
    {
        return false;
    }
    
    for (; entry; entry = entry->next)
    {
        counter++;
    }

    return counter == set->size;
}

void unordered_set_t_free(unordered_set* set)
{
    if (!set) 
    {
        return;
    }
    
    unordered_set_t_clear(set);
    free(set->table);
    free(set);
}

unordered_set_iterator* 
unordered_set_iterator_t_alloc(unordered_set* set)
{
    unordered_set_iterator* iterator;

    if (!set) 
    {
        return NULL;
    }
    
    iterator = malloc(sizeof(*iterator));

    if (!iterator) 
    {
        return NULL;
    }
    
    iterator->map                = set;
    iterator->iterated_count     = 0;
    iterator->next_entry         = set->head;
    iterator->expected_mod_count = set->mod_count;

    return iterator;
}

size_t unordered_set_iterator_t_has_next(unordered_set_iterator* iterator)
{
    if (!iterator) 
    {
        return 0;
    }
    
    if (unordered_set_iterator_t_is_disturbed(iterator)) 
    {
        return 0;
    }
    
    return iterator->map->size - iterator->iterated_count;
}

bool unordered_set_iterator_t_next(unordered_set_iterator* iterator, 
                                   void** key_pointer)
{
    if (!iterator)                                      
    {
        return false;
    }
    
    if (!iterator->next_entry)                     
    {
        return false;
    }
    
    if (unordered_set_iterator_t_is_disturbed(iterator)) 
    {
        return false;
    }
    
    *key_pointer = iterator->next_entry->key;
    iterator->iterated_count++;
    iterator->next_entry = iterator->next_entry->next;
    
    return true;
}

bool
unordered_set_iterator_t_is_disturbed(unordered_set_iterator* iterator)
{
    if (!iterator) 
    {
        false;
    }
    
    return iterator->expected_mod_count != iterator->map->mod_count;
}

void unordered_set_iterator_t_free(unordered_set_iterator* iterator)
{
    if (!iterator) 
    {
        return;
    }
    
    iterator->map = NULL;
    iterator->next_entry = NULL;
    free(iterator);
}

