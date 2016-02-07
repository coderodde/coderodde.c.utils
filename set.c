#include "set.h"
#include <stdlib.h>
#include <stdbool.h>

typedef struct set_entry {
    void*             element;
    struct set_entry* left;
    struct set_entry* right;
    struct set_entry* parent;
    int               height;
} set_entry;

struct set {
    set_entry*   root;
    int        (*comparator)(void*, void*);
    size_t       size;
    size_t       mod_count;
};

struct set_iterator {
    set*       owner_set;
    set_entry* next;
    size_t     iterated_count;
    size_t     expected_mod_count;
};
    
/*******************************************************************************
* Creates a new set entry and initializes its fields.                          *
*******************************************************************************/  
static set_entry* set_entry_alloc(void* element) 
{
    set_entry* entry = malloc(sizeof(*entry));

    if (!entry) 
    {
        return NULL;
    }
    
    entry->element = element;
    entry->left    = NULL;
    entry->right   = NULL;
    entry->parent  = NULL;
    entry->height  = 0;

    return entry;
}

/*******************************************************************************
* Returns the height of an entry. The height of a non-existent entry is        *
* assumed to be -1.                                                            *
*******************************************************************************/
static int height(set_entry* entry) 
{
    return entry ? entry->height : -1;
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
static set_entry* left_rotate(set_entry* node_1)
{
    set_entry* node_2 = node_1->right;
    node_2->parent    = node_1->parent;
    node_1->parent    = node_2;
    node_1->right     = node_2->left;
    node_2->left      = node_1;

    if (node_1->right) 
    {
        node_1->right->parent = node_1;
    }
    
    node_1->height = max(height(node_1->left), height(node_1->right)) + 1;
    node_2->height = max(height(node_2->left), height(node_2->right)) + 1;
    
    return node_2;
}

/*******************************************************************************
* Performs a right rotation and returns the new root of a (sub)tree.           *
*******************************************************************************/
static set_entry* right_rotate(set_entry* node_1)
{
    set_entry* node_2 = node_1->left;
    node_2->parent   = node_1->parent;
    node_1->parent   = node_2;
    node_1->left     = node_2->right;
    node_2->right    = node_1;

    if (node_1->left)
    {
        node_1->left->parent = node_1;
    }
    
    node_1->height = max(height(node_1->left), height(node_1->right)) + 1;
    node_2->height = max(height(node_2->left), height(node_2->right)) + 1;
    
    return node_2;
}

/*******************************************************************************
* Performs a right rotation following by a left rotation and returns the root  *
* of the new (sub)tree.                                                        * 
*******************************************************************************/
static set_entry* right_left_rotate(set_entry* node_1) 
{
    set_entry* node_2 = node_1->right;
    node_1->right = right_rotate(node_2);
    return left_rotate(node_1);
}

/*******************************************************************************
* Performs a left rotation following by a right rotation and returns the root  *
* of the new (sub)tree.                                                        * 
*******************************************************************************/
static set_entry* left_right_rotate(set_entry* node_1)
{
    set_entry* node_2 = node_1->left;
    node_1->left = left_rotate(node_2);
    return right_rotate(node_1);
}

/*******************************************************************************
* Fixes the tree in order to balance it. Basically, we start from 'p_entry'    *
* go up the chain towards parents. If a parent is disbalanced, a set of        *
* rotations are applied. If 'insertion_mode' is on, it means that previous     *  
* modification was insertion of an entry. In such a case we need to perform    *
* only one rotation. If 'insertion_mode' is off, the last operation was        *
* removal and we need to go up until the root node.                            *
*******************************************************************************/  
static void fix_after_modification(set* my_set, 
                                   set_entry* entry,
                                   bool insertion_mode)
{
    set_entry* parent = entry->parent;
    set_entry* grand_parent;
    set_entry* sub_tree;

    while (parent) 
    {
        if (height(parent->left) == height(parent->right) + 2)
        {
            grand_parent = parent->parent;

            if (height(parent->left->left) > height(parent->left->right)) 
            {
                sub_tree = right_rotate(parent);
            }
            else
            {
                sub_tree = left_right_rotate(parent);
            }

            if (!grand_parent)
            {
                my_set->root = sub_tree;
            }
            else if (grand_parent->left == parent) 
            {
                grand_parent->left = sub_tree;
            }
            else
            {
                grand_parent->right = sub_tree;
            }
                
            if (grand_parent)
            {
                grand_parent->height = 
                        max(height(grand_parent->left),
                                   height(grand_parent->right)) + 1;
            }

            /* Fixing after insertion requires only one rotation. */
            if (insertion_mode)
            {
                return;
            }
        }
        else if (height(parent->right) == height(parent->left) + 2) 
        {
            grand_parent = parent->parent;

            if (height(parent->right->right) > height(parent->right->left)) 
            {
                sub_tree = left_rotate(parent);
            }
            else
            {
                sub_tree = right_left_rotate(parent);
            }
            
            if (!grand_parent)
            {
                my_set->root = sub_tree;
            }
            else if (grand_parent->left == parent)
            {
                grand_parent->left = sub_tree;
            }
            
            else
            {
                grand_parent->right = sub_tree;
            }
                
            if (grand_parent)
            {
                grand_parent->height = 
                        max(height(grand_parent->left),
                            height(grand_parent->right)) + 1;
            }

            /* Fixing after insertion requires only one rotation. */
            if (insertion_mode)
            {
                return;
            }
        }

        parent->height = max(height(parent->left), height(parent->right)) + 1;
        parent = parent->parent;
    }
}

/*******************************************************************************
* Performs the actual insertion of an entry.                                   *
*******************************************************************************/
static bool insert(set* my_set, void* element) 
{
    set_entry* new_entry = set_entry_alloc(element);
    set_entry* x;
    set_entry* parent;

    if (!new_entry) 
    {
        return false;
    }
    
    if (!my_set->root)
    {
        my_set->root = new_entry;
        my_set->size++;
        my_set->mod_count++;
        return true;
    }

    x = my_set->root;
    parent = NULL;

    while (x) 
    {
        parent = x;

        if (my_set->comparator(new_entry->element, x->element) < 0)
        {
            x = x->left;
        }
        else
        {
            x = x->right;
        }
    }

    new_entry->parent = parent;

    if (my_set->comparator(new_entry->element, parent->element) < 0) 
    {
        parent->left = new_entry;
    }
    else
    {
        parent->right = new_entry;
    }

    /** 'true' means we choose the insertion mode for fixing the tree. */
    fix_after_modification(my_set, new_entry, true);
    my_set->size++;
    my_set->mod_count++;
    
    return true;
}

/*******************************************************************************
* Returns the minimum entry of a subtree rooted at 'p_entry'.                  *
*******************************************************************************/  
static set_entry* min_entry(set_entry* entry)
{
    while (entry->left)
    {
        entry = entry->left;
    }
    
    return entry;
}

/*******************************************************************************
* Returns the successor entry as specified by the order implied by the         *
* comparator.                                                                  *
*******************************************************************************/
static set_entry* get_successor_entry(set_entry* entry)
{
    set_entry* parent;

    if (entry->right)
    {
        return min_entry(entry->right);
    }
    
    parent = entry->parent;

    while (parent && parent->right == entry)
    {
        entry = parent;
        parent = parent->parent;
    }

    return parent;
}

/*******************************************************************************
* This routine is responsible for removing entries from the tree.              *
*******************************************************************************/  
static set_entry* delete_entry(set* my_set, set_entry* entry)
{
    set_entry* parent;
    set_entry* child;
    set_entry* successor;

    void* p_tmp_element;

    if (!entry->left && !entry->right)
    {
        /** The node to delete has no children. */
        parent = entry->parent;

        if (!parent) 
        {
            my_set->root = NULL;
            my_set->size--;
            my_set->mod_count++;
            return entry;
        }

        if (entry == parent->left) 
        {
            parent->left = NULL;
        }
        else
        {
            parent->right = NULL;
        }

        my_set->size--;
        my_set->mod_count++;
        
        return entry;
    }

    if (!entry->left || !entry->right)
    {
        /** The node has exactly one child. */
        if (entry->left)
        {
            child = entry->left;
        }
        else
        {
            child = entry->right;
        }
            
        parent = entry->parent;
        child->parent = parent;

        if (!parent) 
        {
            my_set->root = child;
            my_set->size--;
            my_set->mod_count++;
            return entry;
        }

        if (entry == parent->left)
        {
            parent->left = child;
        }
        else
        {
            parent->right = child;
        }
            
        my_set->size--;
        my_set->mod_count++;
        
        return entry;
    }

    /** The node to remove has both children. */
    p_tmp_element  = entry->element;
    successor      = min_entry(entry->right);
    entry->element = successor->element;
    child          = successor->right;
    parent         = successor->parent;

    if (parent->left == successor)
    {
        parent->left = child;
    }
    else
    {
        parent->right = child;
    }
        
    if (child)
    {
        child->parent = parent;
    }
        
    my_set->size--;
    my_set->mod_count++;
    successor->element = p_tmp_element;
    
    return successor;
}

/*******************************************************************************
* Searches for an entry with key 'key'. Returns NULL if there is no such.      *
*******************************************************************************/  
static set_entry* find_entry(set* my_set, void* element)
{
    set_entry* entry = my_set->root;

    while (entry && my_set->comparator(element, entry->element) != 0)
    {
        if (my_set->comparator(element, entry->element) < 0)
        {
            entry = entry->left;
        }
        else
        {
            entry = entry->right;
        }
    }

    return entry;
}

set* set_alloc(int (*comparator)(void*, void*)) 
{
    set* my_set;

    if (!comparator) 
    {
        return NULL;
    }
    
    my_set = malloc(sizeof(*my_set));

    if (!my_set) 
    {
        return NULL;
    }
    
    my_set->root = NULL;
    my_set->comparator = comparator;
    my_set->size = 0;
    my_set->mod_count = 0;

    return my_set;
}

bool set_add(set* my_set, void* element)
{
    if (!my_set)            
    {
        return false;
    }
    
    if (find_entry(my_set, element))
    {
        return false;
    }
    
    return insert(my_set, element);
}

bool set_contains(set* my_set, void* element)
{
    if (!my_set)
    {
        return false;
    }
    
    return find_entry(my_set, element) ? true : false;
}

bool set_remove(set* my_set, void* element)
{
    set_entry* entry;

    if (!my_set) 
    {
        return false;
    }
    
    entry = find_entry(my_set, element);

    if (!entry) 
    {
        return false;
    }
    
    entry = delete_entry(my_set, entry);
    fix_after_modification(my_set, entry, false);
    free(entry);
    return true;
}

/*******************************************************************************
* This routine implements the actual checking of tree balance.                 *
*******************************************************************************/  
static bool check_balance_factors_impl(set_entry* entry)
{
    if (!entry)
    {
        return true;
    }
        
    if (abs(height(entry->left) - height(entry->right)) > 1)        
    {
        return false;
    }
        
    if (!check_balance_factors_impl(entry->left)) 
    {
        return false;
    }
    
    if (!check_balance_factors_impl(entry->right)) 
    {
        return false;
    }
    
    return true;
}

/*******************************************************************************
* Checks that every node in the map is balanced.                               *
*******************************************************************************/  
static int check_balance_factors(set* my_set) 
{
    return check_balance_factors_impl(my_set->root);
}

/*******************************************************************************
* This routine implements the actual height verification algorithm. It uses a  *
* sentinel value of -2 for denoting the fact that a current subtree contains   *
* at least one unbalanced node.                                                *  
*******************************************************************************/  
static int check_heights_impl(set_entry* entry)
{
    int height_left;
    int height_right;
    int height_both;

    /**********************************************************
    * The base case: the height of a non-existent leaf is -1. *
    **********************************************************/ 
    if (!entry) 
    {
        return -1;
    }
    
    height_left = check_heights_impl(entry->left) + 1;

    if (height_left == -2) 
    {
        return -2;
    }
    
    height_right = check_heights_impl(entry->right) + 1;

    if (height_right == -2)  
    {
        return -2;
    }
    
    if ((height_both = max(height_left, 
                           height_right)) != entry->height) 
    {
        return -2;
    }
    
    return height_both;
}

/*******************************************************************************
* This routine checks that the height field of each map entry (node) is        *
* correct.                                                                     *
*******************************************************************************/  
static int check_heights(set* my_set)
{
    return check_heights_impl(my_set->root) != -2;
}

bool set_is_healthy(set* my_set) 
{
    if (!my_set)              
    {
        return false;
    }
        
    if (!check_heights(my_set)) 
    {
        return false;
    }
        
    return check_balance_factors(my_set);
}

/*******************************************************************************
* Implements the actual deallocation of the tree entries by traversing the     *
* tree in post-order.                                                          * 
*******************************************************************************/  
static void set_free_impl(set_entry* entry)
{
    if (!entry)
    {
        return;
    }
    
    set_free_impl(entry->left);
    set_free_impl(entry->right);
    free(entry);
}

void set_free(set* my_set) 
{
    if (!my_set)       
    {
        return;
    }
    
    if (!my_set->root)
    {
        return;
    }
    
    set_free_impl(my_set->root);
    free(my_set);
}

void set_clear(set* my_set) 
{
    if (!my_set)        
    {
        return;
    }
    
    if (!my_set->root) 
    {
        return;
    }
    
    set_free_impl(my_set->root);
    my_set->mod_count += my_set->size;
    my_set->root = NULL;
    my_set->size = 0;
}

size_t set_size(set* my_set) 
{
    return my_set ? my_set->size : 0;
}

set_iterator* set_iterator_alloc(set* my_set)
{
    set_iterator* iterator;
    
    if (!my_set) 
    {
        return NULL;
    }
    
    iterator = malloc(sizeof(*iterator));
    iterator->expected_mod_count = my_set->mod_count;
    iterator->iterated_count = 0;
    iterator->owner_set = my_set;
    iterator->next = my_set->root ? min_entry(my_set->root) : NULL;
    
    return iterator;
}

size_t set_iterator_has_next(set_iterator* iterator) 
{
    if (!iterator) 
    {
        return 0;
    }
    
    /** If the map was modified, stop iteration. */
    if (set_iterator_is_disturbed(iterator)) 
    {
        return 0;
    }
    
    return iterator->owner_set->size - iterator->iterated_count;
}

bool set_iterator_next(set_iterator* iterator, void** element_pointer)
{
    if (!iterator)                           
    {
        return false;
    }
        
    if (!iterator->next)      
    {
        return false;
    }
    
    if (set_iterator_is_disturbed(iterator)) 
    {
        return false;
    } 
    
    *element_pointer = iterator->next->element;
    iterator->iterated_count++;
    iterator->next = get_successor_entry(iterator->next);
    
    return true;
}

bool set_iterator_is_disturbed(set_iterator* iterator) 
{
    if (!iterator) 
    {
        return false;
    }
    
    return iterator->expected_mod_count != iterator->owner_set->mod_count;
}

void set_iterator_free(set_iterator* iterator) 
{
    iterator->owner_set  = NULL;
    iterator->next = NULL;
    free(iterator);
}
