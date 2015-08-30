#include "map.h"
#include <stdlib.h>

#define FALSE 0
#define TRUE (~FALSE)

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
* Deallocates the map entry.                                                   *
*******************************************************************************/
static void map_entry_t_free(map_entry_t* p_node) 
{
    if (p_node) free(p_node);
}

/*******************************************************************************
* Returns the height of an entry.                                              *
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
* Fixes the tree in order to make it balanced after inserting a new entry.     *
* The algorithm iterates from an inserted node up to its parent, and the first *
* parent node that is unbalanced is rotated and algorithm terminates, since    *
* after addition at most one rotation is enough for making the tree balanced.  *  
*******************************************************************************/  
static void fix_after_modification(map_t* p_map, 
                                   map_entry_t* p_entry,
                                   int insertion_mode)
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
            
            if (insertion_mode) return;
        }
        
        p_parent->height = max(height(p_parent->p_left),
                               height(p_parent->p_right)) + 1;
        p_parent = p_parent->p_parent;
    }
}

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
    fix_after_modification(p_map, p_new_entry, TRUE);
    p_map->size++;
    p_map->mod_count++;
    return (EXIT_SUCCESS);
}
static map_entry_t* min_entry(map_entry_t* p_entry)
{
    while (p_entry->p_left) p_entry = p_entry->p_left;
    return p_entry;
}

static map_entry_t* delete_entry(map_t* p_map, map_entry_t* p_entry)
{
    map_entry_t* p_parent;
    map_entry_t* p_child;
    map_entry_t* p_successor;
    
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
            p_map->p_root = p_parent;
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
    return p_successor;
}

/*******************************************************************************
* Searches for an entry with key 'key'. Returns NULL if there is no such.      *
*******************************************************************************/  
static map_entry_t* find_entry(map_t* p_map, void* key)
{
    map_entry_t* p_entry = p_map->p_root;
    
    while (p_entry && p_entry->p_key != key)
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
    
    if (!p_map)               return NULL;
    if (!p_map->p_comparator) return NULL;
    
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

int map_t_contains_key (map_t* p_map, void* p_key)
{
    if (!p_map)               return 0;
    if (!p_map->p_comparator) return 0;
    
    return find_entry(p_map, p_key) ? 1 : 0;
}

void* map_t_get(map_t* p_map, void* p_key)
{
    map_entry_t* p_entry;
    
    if (!p_map)               return NULL;
    if (!p_map->p_comparator) return NULL;
    
    p_entry = find_entry(p_map, p_key);
    return p_entry ? p_entry->p_value : NULL;
}

void* map_t_remove(map_t* p_map, void* p_key)
{
    map_entry_t* p_entry;
    
    if (!p_map)               return NULL;
    if (!p_map->p_comparator) return NULL;
    
    p_entry = find_entry(p_map, p_key);
    
    if (!p_entry) return NULL;
    
    p_entry = delete_entry(p_map, p_entry);
    fix_after_modification(p_map, p_entry, FALSE);
    return p_entry->p_value;
}

/*******************************************************************************
* This routine implements the actual checking of tree balance.                 *
*******************************************************************************/  
static int check_balance_factors_impl(map_entry_t* p_entry)
{
    if (!p_entry) return 1;
    if (abs(height(p_entry->p_left) - height(p_entry->p_right)) > 1) return 0;
    if (!check_balance_factors_impl(p_entry->p_left))  return 0;
    if (!check_balance_factors_impl(p_entry->p_right)) return 0;
    return 1;
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

int map_t_is_healthy(map_t* p_map) 
{
    if (!p_map) return 0;
    
    if (!check_heights(p_map)) return 0;
    
    return check_balance_factors(p_map);
}