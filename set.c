#include "set.h"
#include <stdlib.h>
#include <stdbool.h>

typedef struct set_entry_t {
    void*               p_element;
    struct set_entry_t* p_left;
    struct set_entry_t* p_right;
    struct set_entry_t* p_parent;
    int                 height;
} set_entry_t;

typedef struct set_t {
    set_entry_t*   p_root;
    int          (*p_comparator)(void*, void*);
    size_t         size;
    size_t         mod_count;
} set_t;

struct set_iterator_t {
    set_t*       p_set;
    set_entry_t* p_next;
    size_t       iterated_count;
    size_t       expected_mod_count;
};
    
/*******************************************************************************
* Creates a new set entry and initializes its fields.                          *
*******************************************************************************/  
static set_entry_t* set_entry_t_alloc(void* p_element) 
{
    set_entry_t* p_ret = malloc(sizeof(*p_ret));

    if (!p_ret) return NULL;

    p_ret->p_element    = p_element;
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
static int height(set_entry_t* p_node) 
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
static set_entry_t* left_rotate(set_entry_t* p_node_1)
{
    set_entry_t* p_node_2 = p_node_1->p_right;
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
static set_entry_t* right_rotate(set_entry_t* p_node_1)
{
    set_entry_t* p_node_2 = p_node_1->p_left;
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
static set_entry_t* right_left_rotate(set_entry_t* p_node_1) 
{
    set_entry_t* p_node_2 = p_node_1->p_right;
    p_node_1->p_right = right_rotate(p_node_2);
    return left_rotate(p_node_1);
}

/*******************************************************************************
* Performs a left rotation following by a right rotation and returns the root  *
* of the new (sub)tree.                                                        * 
*******************************************************************************/
static set_entry_t* left_right_rotate(set_entry_t* p_node_1)
{
    set_entry_t* p_node_2 = p_node_1->p_left;
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
static void fix_after_modification(set_t* p_set, 
                                   set_entry_t* p_entry,
                                   bool insertion_mode)
{
    set_entry_t* p_parent = p_entry->p_parent;
    set_entry_t* p_grand_parent;
    set_entry_t* p_sub_tree;

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
                p_set->p_root = p_sub_tree;
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
                p_set->p_root = p_sub_tree;
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
static int insert(set_t* p_set, void* p_element) 
{
    set_entry_t* p_new_entry = set_entry_t_alloc(p_element);
    set_entry_t* p_x;
    set_entry_t* p_parent;

    if (!p_new_entry) return (EXIT_FAILURE);

    if (!p_set->p_root)
    {
        p_set->p_root = p_new_entry;
        p_set->size++;
        p_set->mod_count++;
        return (EXIT_SUCCESS);
    }

    p_x = p_set->p_root;
    p_parent = NULL;

    while (p_x) 
    {
        p_parent = p_x;

        if (p_set->p_comparator(p_new_entry->p_element, p_x->p_element) < 0)
            p_x = p_x->p_left;
        else
            p_x = p_x->p_right;
    }

    p_new_entry->p_parent = p_parent;

    if (p_set->p_comparator(p_new_entry->p_element, p_parent->p_element) < 0) 
        p_parent->p_left = p_new_entry;
    else
        p_parent->p_right = p_new_entry;

    /** 'true' means we choose the insertion mode for fixing the tree. */
    fix_after_modification(p_set, p_new_entry, true);
    p_set->size++;
    p_set->mod_count++;
    return (EXIT_SUCCESS);
}

/*******************************************************************************
* Returns the minimum entry of a subtree rooted at 'p_entry'.                  *
*******************************************************************************/  
static set_entry_t* min_entry(set_entry_t* p_entry)
{
    while (p_entry->p_left) p_entry = p_entry->p_left;
    return p_entry;
}

/*******************************************************************************
* Returns the successor entry as specified by the order implied by the         *
* comparator.                                                                  *
*******************************************************************************/
static set_entry_t* get_successor_entry(set_entry_t* p_entry)
{
    set_entry_t* p_parent;

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
static set_entry_t* delete_entry(set_t* p_set, set_entry_t* p_entry)
{
    set_entry_t* p_parent;
    set_entry_t* p_child;
    set_entry_t* p_successor;

    void* p_tmp_element;

    if (!p_entry->p_left && !p_entry->p_right)
    {
        /** The node to delete has no children. */
        p_parent = p_entry->p_parent;

        if (!p_parent) 
        {
            p_set->p_root = NULL;
            p_set->size--;
            p_set->mod_count++;
            return p_entry;
        }

        if (p_entry == p_parent->p_left) 
            p_parent->p_left = NULL;
        else
            p_parent->p_right = NULL;

        p_set->size--;
        p_set->mod_count++;
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
            p_set->p_root = p_child;
            p_set->size--;
            p_set->mod_count++;
            return p_entry;
        }

        if (p_entry == p_parent->p_left)
            p_parent->p_left = p_child;
        else 
            p_parent->p_right = p_child;

        p_set->size--;
        p_set->mod_count++;
        return p_entry;
    }

    /** The node to remove has both children. */
    p_tmp_element      = p_entry->p_element;
    p_successor        = min_entry(p_entry->p_right);
    p_entry->p_element = p_successor->p_element;
    p_child            = p_successor->p_right;
    p_parent           = p_successor->p_parent;

    if (p_parent->p_left == p_successor)
        p_parent->p_left = p_child;
    else
        p_parent->p_right = p_child;

    if (p_child)
        p_child->p_parent = p_parent;

    p_set->size--;
    p_set->mod_count++;
    p_successor->p_element = p_tmp_element;
    return p_successor;
}

/*******************************************************************************
* Searches for an entry with key 'key'. Returns NULL if there is no such.      *
*******************************************************************************/  
static set_entry_t* find_entry(set_t* p_set, void* element)
{
    set_entry_t* p_entry = p_set->p_root;

    while (p_entry && p_set->p_comparator(element, p_entry->p_element) != 0)
    {
        if (p_set->p_comparator(element, p_entry->p_element) < 0)
            p_entry = p_entry->p_left;
        else 
            p_entry = p_entry->p_right;
    }

    return p_entry;
}

set_t* set_t_alloc(int (*p_comparator)(void*, void*)) 
{
    set_t* p_ret;

    if (!p_comparator) return NULL;

    p_ret = malloc(sizeof(*p_ret));

    if (!p_ret) return NULL;

    p_ret->p_root = NULL;
    p_ret->p_comparator = p_comparator;
    p_ret->size = 0;
    p_ret->mod_count = 0;

    return p_ret;
}

bool set_t_add(set_t* p_set, void* p_element)
{
    set_entry_t* p_target;
    void* p_old_value;

    if (!p_set)                       return false;
    if (find_entry(p_set, p_element)) return false;
    
    insert(p_set, p_element);
    return true;
}

bool set_t_contains(set_t* p_set, void* p_element)
{
    if (!p_set) return false;

    return find_entry(p_set, p_element) ? true : false;
}

bool set_t_remove(set_t* p_set, void* p_element)
{
    set_entry_t* p_entry;

    if (!p_set) return false;

    p_entry = find_entry(p_set, p_element);

    if (!p_entry) return false;

    p_entry = delete_entry(p_set, p_entry);
    fix_after_modification(p_set, p_entry, false);
    free(p_entry);
    return true;
}

/*******************************************************************************
* This routine implements the actual checking of tree balance.                 *
*******************************************************************************/  
static bool check_balance_factors_impl(set_entry_t* p_entry)
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
static int check_balance_factors(set_t* p_set) 
{
    return check_balance_factors_impl(p_set->p_root);
}

/*******************************************************************************
* This routine implements the actual height verification algorithm. It uses a  *
* sentinel value of -2 for denoting the fact that a current subtree contains   *
* at least one unbalanced node.                                                *  
*******************************************************************************/  
static int check_heights_impl(set_entry_t* p_entry)
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
static int check_heights(set_t* p_set)
{
    return check_heights_impl(p_set->p_root) != -2;
}

bool set_t_is_healthy(set_t* p_set) 
{
    if (!p_set)                return false;
    if (!check_heights(p_set)) return false;
    return check_balance_factors(p_set);
}

/*******************************************************************************
* Implements the actual deallocation of the tree entries by traversing the     *
* tree in post-order.                                                          * 
*******************************************************************************/  
static void set_free_impl(set_entry_t* p_entry)
{
    if (!p_entry) return;

    set_free_impl(p_entry->p_left);
    set_free_impl(p_entry->p_right);
    free(p_entry);
}

void set_t_free(set_t* p_set) 
{
    if (!p_set)         return;
    if (!p_set->p_root) return;

    set_free_impl(p_set->p_root);
    free(p_set);
}

void set_t_clear(set_t* p_set) 
{
    if (!p_set)         return;
    if (!p_set->p_root) return;

    set_free_impl(p_set->p_root);
    p_set->mod_count += p_set->size;
    p_set->p_root = NULL;
    p_set->size = 0;
}

size_t set_t_size(set_t* p_set) 
{
    return p_set ? p_set->size : 0;
}

set_iterator_t* set_iterator_t_alloc(set_t* p_set)
{
    if (!p_set) return NULL;
    set_iterator_t* p_iterator = malloc(sizeof(*p_iterator));
    p_iterator->expected_mod_count = p_set->mod_count;
    p_iterator->iterated_count = 0;
    p_iterator->p_set = p_set;
    p_iterator->p_next = p_set->p_root ? min_entry(p_set->p_root) : NULL;
    return p_iterator;
}

size_t set_iterator_t_has_next(set_iterator_t* p_iterator) 
{
    if (!p_iterator) return 0;

    /** If the map was modified, stop iteration. */
    if (set_iterator_t_is_disturbed(p_iterator)) return 0;

    return p_iterator->p_set->size - p_iterator->iterated_count;
}

bool set_iterator_t_next(set_iterator_t* p_iterator, void** pp_element)
{
    if (!p_iterator)                             return false;
    if (!p_iterator->p_next)                     return false;
    if (set_iterator_t_is_disturbed(p_iterator)) return false;

    *pp_element   = p_iterator->p_next->p_element;
    p_iterator->iterated_count++;
    p_iterator->p_next = get_successor_entry(p_iterator->p_next);
    return true;
}

bool set_iterator_t_is_disturbed(set_iterator_t* p_iterator) 
{
    if (!p_iterator) return false;

    return p_iterator->expected_mod_count != p_iterator->p_set->mod_count;
}

void set_iterator_t_free(set_iterator_t* p_iterator) 
{
    p_iterator->p_set  = NULL;
    p_iterator->p_next = NULL;
    free(p_iterator);
}
