//#include "list.h"
//#include <stdbool.h>
//#include <stdlib.h>
//
//typedef struct list_t {
//    void** p_table;
//    size_t size;
//    size_t capacity;
//    size_t head;
//    size_t mask;
//} list_t;
//
//static const size_t MINIMUM_CAPACITY = 16;
//
//static size_t max(size_t a, size_t b)
//{
//    return a < b ? b : a;
//}
//
//static size_t fix_initial_capacity(size_t initial_capacity)
//{
//    size_t ret = 1;
//    
//    initial_capacity = max(initial_capacity, MINIMUM_CAPACITY);
//    
//    while (ret < initial_capacity) ret <<= 1;
//    
//    return ret;
//}
//
//list_t* list_t_alloc(size_t initial_capacity)
//{
//    list_t* p_ret = malloc(sizeof(*p_ret));
//    
//    if (!p_ret) return NULL;
//    
//    initial_capacity = fix_initial_capacity(initial_capacity);
//    
//    p_ret->p_table = malloc(sizeof(void*) * initial_capacity);
//    
//    if (!p_ret->p_table)
//    {
//        free(p_ret);
//        return NULL;
//    }
//    
//    p_ret->capacity = initial_capacity;
//    p_ret->mask     = initial_capacity - 1;
//    p_ret->head     = 0;
//    p_ret->size     = 0;
//    
//    return p_ret;
//}
//
//static bool ensure_capacity_before_add(list_t* p_list)
//{
//    void** p_new_table;
//    size_t i;
//    size_t new_capacity;
//    
//    if (p_list->size < p_list->capacity) return true;
//    
//    new_capacity = 2 * p_list->capacity;
//    p_new_table  = malloc(new_capacity);
//    
//    if (!p_new_table) return false;
//    
//    for (i = 0; i < p_list->size; ++i) 
//    {
//        p_new_table[i] = p_list->p_table[(p_list->head + i) & p_list->mask];
//    }
//    
//    free(p_list->p_table);
//    p_list->p_table  = p_new_table;
//    p_list->capacity = new_capacity;
//    p_list->mask     = new_capacity - 1;
//    p_list->head     = 0;
//    
//    return true;
//}
//
//bool list_t_push_front(list_t* p_list, void* p_element)
//{
//    if (!p_list)                             return false;
//    if (!ensure_capacity_before_add(p_list)) return false;
//    
//    p_list->head = (p_list->head - 1) & p_list->mask;
//    p_list->p_table[p_list->head] = p_element;
//    p_list->size++;
//    return true;
//}
//
//bool list_t_push_back(list_t* p_list, void* p_element)
//{
//    if (!p_list)                             return false;
//    if (!ensure_capacity_before_add(p_list)) return false;
//    
//    p_list->p_table[(p_list->head + p_list->size) & p_list->mask] = p_element;
//    p_list->size++;
//    return true;
//}
//
//bool list_t_insert(list_t* p_list, size_t index, void* p_element)
//{
//    size_t elements_before;
//    size_t elements_after;
//    size_t i;
//    
//    if (!p_list)                             return false;
//    if (!ensure_capacity_before_add(p_list)) return false;
//    if (index > p_list->size)                return false;
//    
//    elements_before = index;
//    elements_after = p_list->size - index;
//    
//    if (elements_before < elements_after) 
//    {
////        for (i = index;)
//    }
//    else
//    {
//        
//    }
//}
//
//void* list_t_get(list_t* p_list, size_t index)
//{
//    
//}
//
//void* list_t_set(list_t* p_list, size_t index, void* p_new_value) 
//{
//
//}
//
//void* list_t_pop_front(list_t* p_list)
//{
//    
//}
// 
//void* list_t_pop_back(list_t* p_list)
//{
//    
//}
//
//void*   list_t_remove_at(list_t* p_list, size_t index)
//{
//    
//}
//
//bool    list_t_contains(list_t* p_list, 
//                        void* p_element,
//                        bool (*p_equals_function)(void*, void*))
//{
//    
//}
//
//void    list_t_clear(list_t* p_list)
//{
//    
//}
//
//void    list_t_free(list_t* p_list)
//{
//    
//}