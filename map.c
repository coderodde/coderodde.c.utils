#include "map.h"
#include <stdlib.h>
#include <stdbool.h>

typedef struct map_entry_t {
    void*               p_key;
    void*               p_value;
    struct map_entry_t* p_left;
    struct map_entry_t* p_right;
    struct map_entry_t* p_parent;
    int                 height;
} map_entry_t;

typedef struct map_t {
    map_entry_t* p_root;
    int (*p_comparator)(void*, void*);
    size_t size;
    size_t mod_count;
} map_t;

typedef struct map_iterator_t {
    map_t*       p_map;
    map_entry_t* p_next;
    size_t       iterated_count;
    size_t       expected_mod_count;
} map_iterator_t;
    
/*******************************************************************************
* Creates a new map entry and initializes its fields.                          *
*******************************************************************************/  
static map_entry_t* map_entry_t_alloc(void* p_key, void* p_value) 
{
    map_entry_t* p_ret = malloc(sizeof(*p_ret));

    if (!p_ret) return NULL;

    p_ret->p_key    = p_key;
    p_ret->p_value  = p_value;
    p_ret->p_left   = NULL;
    p_ret->p_right  = NULL;
    p_ret->p_parent = NULL;
    p_ret->height   = 0;

    return p_ret;
}

/*******************************************************************************
* Returns the height of an entry. The height of a non-existent entry is        *
* assumed to be -1.                                                            *
*******************************************************************************/
static int height(map_entry_t* p_node) 
{
    return p_node ? p_node->height : -1;
}

/*******************************************************************************
* Returns the maximum of the two input integers.                               *
*******************************************************************************/
static int max(int a, int b) 
{
    return a > b ? a : b;
}

/*******************************************************************************
* Performs a left rotation and returns the new root of a (sub)tree.            *
*******************************************************************************/
static map_entry_t* left_rotate(map_entry_t* p_node_1)
{
    map_entry_t* p_node_2 = p_node_1->p_right;
    p_node_2->p_parent   = p_node_1->p_parent;
    p_node_1->p_parent   = p_node_2;
    p_node_1->p_right    = p_node_2->p_left;
    p_node_2->p_left     = p_node_1;

    if (p_node_1->p_right) p_node_1->p_right->p_parent = p_node_1;

    p_node_1->height = max(height(p_node_1->p_left), 
                           height(p_node_1->p_right)) + 1;
    p_node_2->height = max(height(p_node_2->p_left),
                           height(p_node_2->p_right)) + 1;
    return p_node_2;
}

/*******************************************************************************
* Performs a right rotation and returns the new root of a (sub)tree.           *
*******************************************************************************/
static map_entry_t* right_rotate(map_entry_t* p_node_1)
{
    map_entry_t* p_node_2 = p_node_1->p_left;
    p_node_2->p_parent   = p_node_1->p_parent;
    p_node_1->p_parent   = p_node_2;
    p_node_1->p_left     = p_node_2->p_right;
    p_node_2->p_right    = p_node_1;

    if (p_node_1->p_left) p_node_1->p_left->p_parent = p_node_1;

    p_node_1->height = max(height(p_node_1->p_left),
                           height(p_node_1->p_right)) + 1;
    p_node_2->height = max(height(p_node_2->p_left),
                           height(p_node_2->p_right)) + 1;
    return p_node_2;
}

/*******************************************************************************
* Performs a right rotation following by a left rotation and returns the root  *
* of the new (sub)tree.                                                        * 
*******************************************************************************/
static map_entry_t* right_left_rotate(map_entry_t* p_node_1) 
{
    map_entry_t* p_node_2 = p_node_1->p_right;
    p_node_1->p_right = right_rotate(p_node_2);
    return left_rotate(p_node_1);
}

/*******************************************************************************
* Performs a left rotation following by a right rotation and returns the root  *
* of the new (sub)tree.                                                        * 
*******************************************************************************/
static map_entry_t* left_right_rotate(map_entry_t* p_node_1)
{
    map_entry_t* p_node_2 = p_node_1->p_left;
    p_node_1->p_left = left_rotate(p_node_2);
    return right_rotate(p_node_1);
}

/*******************************************************************************
* Fixes the tree in order to balance it. Basically, we start from 'p_entry'    *
* go up the chain towards parents. If a parent is disbalanced, a set of        *
* rotations are applied. If 'insertion_mode' is on, it means that previous     *  
* modification was insertion of an entry. In such a case we need to perform    *
* only one rotation. If 'insertion_mode' is off, the last operation was        *
* removal and we need to go up until the root node.                            *
*******************************************************************************/  
static void fix_after_modification(map_t* p_map, 
                                   map_entry_t* p_entry,
                                   bool insertion_mode)
{
    map_entry_t* p_parent = p_entry->p_parent;
    map_entry_t* p_grand_parent;
    map_entry_t* p_sub_tree;

    while (p_parent) 
    {
        if (height(p_parent->p_left) == height(p_parent->p_right) + 2)
        {
            p_grand_parent = p_parent->p_parent;

            if (height(p_parent->p_left->p_left) > 
                height(p_parent->p_left->p_right)) 
                p_sub_tree = right_rotate(p_parent);
            else 
                p_sub_tree = left_right_rotate(p_parent);

            if (!p_grand_parent) 
                p_map->p_root = p_sub_tree;
            else if (p_grand_parent->p_left == p_parent) 
                p_grand_parent->p_left = p_sub_tree;
            else
                p_grand_parent->p_right = p_sub_tree;

            if (p_grand_parent)
                p_grand_parent->height = 
                        max(height(p_grand_parent->p_left),
                                   height(p_grand_parent->p_right)) + 1;

            /* Fixing after insertion requires only one rotation. */
            if (insertion_mode) return;
        }

        if (height(p_parent->p_right) == height(p_parent->p_left) + 2) 
        {
            p_grand_parent = p_parent->p_parent;

            if (height(p_parent->p_right->p_right) > 
                height(p_parent->p_right->p_left)) 
                p_sub_tree = left_rotate(p_parent);
            else
                p_sub_tree = right_left_rotate(p_parent);

            if (!p_grand_parent)
                p_map->p_root = p_sub_tree;
            else if (p_grand_parent->p_left == p_parent)
                p_grand_parent->p_left = p_sub_tree;
            else
                p_grand_parent->p_right = p_sub_tree;

            if (p_grand_parent)
                p_grand_parent->height = 
                        max(height(p_grand_parent->p_left),
                            height(p_grand_parent->p_right)) + 1;

            /* Fixing after insertion requires only one rotation. */
            if (insertion_mode) return;
        }

        p_parent->height = max(height(p_parent->p_left),
                               height(p_parent->p_right)) + 1;
        p_parent = p_parent->p_parent;
    }
}

/*******************************************************************************
* Performs the actual insertion of an entry.                                   *
*******************************************************************************/
static int insert(map_t* p_map, void* p_key, void* p_value) 
{
    map_entry_t* p_new_entry = map_entry_t_alloc(p_key, p_value);
    map_entry_t* p_x;
    map_entry_t* p_parent;

    if (!p_new_entry) return (EXIT_FAILURE);

    if (!p_map->p_root)
    {
        p_map->p_root = p_new_entry;
        p_map->size++;
        p_map->mod_count++;
        return (EXIT_SUCCESS);
    }

    p_x = p_map->p_root;
    p_parent = NULL;

    while (p_x) 
    {
        p_parent = p_x;

        if (p_map->p_comparator(p_new_entry->p_key, p_x->p_key) < 0)
            p_x = p_x->p_left;
        else
            p_x = p_x->p_right;
    }

    p_new_entry->p_parent = p_parent;

    if (p_map->p_comparator(p_new_entry->p_key, p_parent->p_key) < 0) 
        p_parent->p_left = p_new_entry;
    else
        p_parent->p_right = p_new_entry;

    /** TRUE means we choose the insertion mode for fixing the tree. */
    fix_after_modification(p_map, p_new_entry, true);
    p_map->size++;
    p_map->mod_count++;
    return (EXIT_SUCCESS);
}

/*******************************************************************************
* Returns the minimum entry of a subtree rooted at 'p_entry'.                  *
*******************************************************************************/  
static map_entry_t* min_entry(map_entry_t* p_entry)
{
    while (p_entry->p_left) p_entry = p_entry->p_left;
    return p_entry;
}

/*******************************************************************************
* Returns the successor entry as specified by the order implied by the         *
* comparator.                                                                  *
*******************************************************************************/
static map_entry_t* get_successor_entry(map_entry_t* p_entry)
{
    map_entry_t* p_parent;

    if (p_entry->p_right) return min_entry(p_entry->p_right);

    p_parent = p_entry->p_parent;

    while (p_parent && p_parent->p_right == p_entry)
    {
        p_entry = p_parent;
        p_parent = p_parent->p_parent;
    }

    return p_parent;
}

/*******************************************************************************
* This routine is responsible for removing entries from the tree.              *
*******************************************************************************/  
static map_entry_t* delete_entry(map_t* p_map, map_entry_t* p_entry)
{
    map_entry_t* p_parent;
    map_entry_t* p_child;
    map_entry_t* p_successor;

    void* p_tmp_key;
    void* p_tmp_value;

    if (!p_entry->p_left && !p_entry->p_right)
    {
        /** The node to delete has no children. */
        p_parent = p_entry->p_parent;

        if (!p_parent) 
        {
            p_map->p_root = NULL;
            p_map->size--;
            p_map->mod_count++;
            return p_entry;
        }

        if (p_entry == p_parent->p_left) 
            p_parent->p_left = NULL;
        else
            p_parent->p_right = NULL;

        p_map->size--;
        p_map->mod_count++;
        return p_entry;
    }

    if (!p_entry->p_left || !p_entry->p_right)
    {
        /** The node has exactly one child. */
        if (p_entry->p_left)
            p_child = p_entry->p_left;
        else
            p_child = p_entry->p_right;

        p_parent = p_entry->p_parent;
        p_child->p_parent = p_parent;

        if (!p_parent) 
        {
            p_map->p_root = p_child;
            p_map->size--;
            p_map->mod_count++;
            return p_entry;
        }

        if (p_entry == p_parent->p_left)
            p_parent->p_left = p_child;
        else 
            p_parent->p_right = p_child;

        p_map->size--;
        p_map->mod_count++;
        return p_entry;
    }

    /** The node to remove has both children. */
    p_tmp_key        = p_entry->p_key;
    p_tmp_value      = p_entry->p_value;
    p_successor      = min_entry(p_entry->p_right);
    p_entry->p_key   = p_successor->p_key;
    p_entry->p_value = p_successor->p_value;
    p_child          = p_successor->p_right;
    p_parent         = p_successor->p_parent;

    if (p_parent->p_left == p_successor)
        p_parent->p_left = p_child;
    else
        p_parent->p_right = p_child;

    if (p_child)
        p_child->p_parent = p_parent;

    p_map->size--;
    p_map->mod_count++;
    p_successor->p_key   = p_tmp_key;
    p_successor->p_value = p_tmp_value;
    return p_successor;
}

/*******************************************************************************
* Searches for an entry with key 'key'. Returns NULL if there is no such.      *
*******************************************************************************/  
static map_entry_t* find_entry(map_t* p_map, void* key)
{
    map_entry_t* p_entry = p_map->p_root;

    while (p_entry && p_map->p_comparator(key, p_entry->p_key) != 0)
    {
        if (p_map->p_comparator(key, p_entry->p_key) < 0)
            p_entry = p_entry->p_left;
        else 
            p_entry = p_entry->p_right;
    }

    return p_entry;
}

map_t* map_t_alloc(int (*p_comparator)(void*, void*)) 
{
    map_t* p_ret;

    if (!p_comparator) return NULL;

    p_ret = malloc(sizeof(*p_ret));

    if (!p_ret) return NULL;

    p_ret->p_root = NULL;
    p_ret->p_comparator = p_comparator;
    p_ret->size = 0;
    p_ret->mod_count = 0;

    return p_ret;
}

void* map_t_put(map_t* p_map, void* p_key, void* p_value)
{
    map_entry_t* p_target;
    void* p_old_value;

    if (!p_map) return NULL;

    p_target = find_entry(p_map, p_key);

    if (p_target)
    {
        p_old_value = p_target->p_value;
        p_target->p_value = p_value;
        return p_old_value; 
    } 

    insert(p_map, p_key, p_value);
    return NULL;
}

bool map_t_contains_key (map_t* p_map, void* p_key)
{
    if (!p_map) return false;

    return find_entry(p_map, p_key);
}

void* map_t_get(map_t* p_map, void* p_key)
{
    map_entry_t* p_entry;

    if (!p_map) return NULL;

    p_entry = find_entry(p_map, p_key);
    return p_entry ? p_entry->p_value : NULL;
}

void* map_t_remove(map_t* p_map, void* p_key)
{
    void* ret;
    map_entry_t* p_entry;

    if (!p_map) return NULL;

    p_entry = find_entry(p_map, p_key);

    if (!p_entry) return NULL;

    ret = p_entry->p_value;
    p_entry = delete_entry(p_map, p_entry);
    fix_after_modification(p_map, p_entry, false);
    free(p_entry);
    return ret;
}

/*******************************************************************************
* This routine implements the actual checking of tree balance.                 *
*******************************************************************************/  
static bool check_balance_factors_impl(map_entry_t* p_entry)
{
    if (!p_entry) return true;
    if (abs(height(p_entry->p_left) - 
            height(p_entry->p_right)) > 1)             return false;
    if (!check_balance_factors_impl(p_entry->p_left))  return false;
    if (!check_balance_factors_impl(p_entry->p_right)) return false;
    return true;
}

/*******************************************************************************
* Checks that every node in the map is balanced.                               *
*******************************************************************************/  
static int check_balance_factors(map_t* p_map) 
{
    return check_balance_factors_impl(p_map->p_root);
}

/*******************************************************************************
* This routine implements the actual height verification algorithm. It uses a  *
* sentinel value of -2 for denoting the fact that a current subtree contains   *
* at least one unbalanced node.                                                *  
*******************************************************************************/  
static int check_heights_impl(map_entry_t* p_entry)
{
    int height_left;
    int height_right;
    int height_both;

    /**********************************************************
    * The base case: the height of a non-existent leaf is -1. *
    **********************************************************/ 
    if (!p_entry) return -1;

    height_left = check_heights_impl(p_entry->p_left) + 1;

    if (height_left == -2) return -2;

    height_right = check_heights_impl(p_entry->p_right) + 1;

    if (height_right == -2)  return -2;

    if ((height_both = max(height_left, 
                           height_right)) != p_entry->height) return -2;

    return height_both;
}

/*******************************************************************************
* This routine checks that the height field of each map entry (node) is        *
* correct.                                                                     *
*******************************************************************************/  
static int check_heights(map_t* p_map)
{
    return check_heights_impl(p_map->p_root) != -2;
}

bool map_t_is_healthy(map_t* p_map) 
{
    if (!p_map) return false;

    if (!check_heights(p_map)) return false;

    return check_balance_factors(p_map);
}

/*******************************************************************************
* Implements the actual deallocation of the tree entries by traversing the     *
* tree in post-order.                                                          * 
*******************************************************************************/  
static void map_free_impl(map_entry_t* p_entry)
{
    if (!p_entry) return;

    map_free_impl(p_entry->p_left);
    map_free_impl(p_entry->p_right);
    free(p_entry);
}

void map_t_free(map_t* p_map) 
{
    if (!p_map)         return;
    if (!p_map->p_root) return;

    map_free_impl(p_map->p_root);
    free(p_map);
}

void map_t_clear(map_t* p_map) 
{
    if (!p_map)         return;
    if (!p_map->p_root) return;

    map_free_impl(p_map->p_root);
    p_map->mod_count += p_map->size;
    p_map->p_root = NULL;
    p_map->size = 0;
}

int map_t_size(map_t* p_map) 
{
    return p_map ? p_map->size : -1;
}

map_iterator_t* map_iterator_t_alloc(map_t* p_map)
{
    if (!p_map) return NULL;
    map_iterator_t* p_iterator = malloc(sizeof(*p_iterator));
    p_iterator->expected_mod_count = p_map->mod_count;
    p_iterator->iterated_count = 0;
    p_iterator->p_map = p_map;
    p_iterator->p_next = p_map->p_root ? min_entry(p_map->p_root) : NULL;
    return p_iterator;
}

int map_iterator_t_has_next(map_iterator_t* p_iterator) 
{
    if (!p_iterator) return false;

    /** If the map was modified, stop iteration. */
    if (map_iterator_t_is_disturbed(p_iterator)) return 0;

    return p_iterator->p_map->size - p_iterator->iterated_count;
}

bool map_iterator_t_next(map_iterator_t* p_iterator, 
                         void** pp_key, 
                         void** pp_value)
{
    if (!p_iterator)                             return false;
    if (!p_iterator->p_next)                     return false;
    if (map_iterator_t_is_disturbed(p_iterator)) return false;

    *pp_key   = p_iterator->p_next->p_key;
    *pp_value = p_iterator->p_next->p_value;
    p_iterator->iterated_count++;
    p_iterator->p_next = get_successor_entry(p_iterator->p_next);
    return true;
}

bool map_iterator_t_is_disturbed(map_iterator_t* p_iterator) 
{
    if (!p_iterator) return false;

    return p_iterator->expected_mod_count != p_iterator->p_map->mod_count;
}

void map_iterator_t_free(map_iterator_t* p_iterator) 
{
    if (!p_iterator) return;
    
    p_iterator->p_map = NULL;
    p_iterator->p_next = NULL;
    free(p_iterator);
}