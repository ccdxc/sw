#include "nic/fte/acl/itree.hpp"
#include "nic/include/hal_mem.hpp"
#include <iostream>

//------------------------------------------------------------------------
// Immutable interval tree based on the functional red-black insert/delete
// algorithm from http://matt.might.net/papers/germane2014deletion.pdf
//------------------------------------------------------------------------

namespace acl {

#define B 0
#define R 1
#define BB 2 // double black, used during delete

#define E (&itree_t::empty_) // empty
#define EE (&itree_t::eempty_) // double black empty


// static memeber initializer
slab* itree_t::itree_slab_ = slab::factory("itree", hal::HAL_SLAB_ACL_ITREE,
                                           sizeof(itree_t), 256, true, true, true);
slab* itree_t::itree_node_slab_ = slab::factory("itree_node", hal::HAL_SLAB_ACL_ITREE_NODE,
                                                sizeof(itree_t::node_t), 4*1024, true, true, true);
const itree_t::node_t itree_t::empty_{B};
const itree_t::node_t itree_t::eempty_{BB};


//------------------------------------------------------------------------
// allocate a tree node
//------------------------------------------------------------------------
itree_t::node_t *
itree_t::node_alloc()
{
    node_t *node = (node_t *)itree_node_slab_->alloc();

    ref_init(&node->ref_count, [](const ref_t *ref) {
            node_t *node = container_of(ref, node_t, ref_count);

            node->left->deref();
            node->right->deref();

            if (node->has_list) {
                node->list->deref();
            } else {
                ref_dec(node->entry);
            }

            itree_node_slab_->free(node);

        });

    return node;
}

//------------------------------------------------------------------------
// creates new tree node
//------------------------------------------------------------------------
itree_t::node_t *
itree_t::node_create(uint32_t low, uint32_t high, const ref_t *entry)
{
    itree_t::node_t *node = node_alloc();

    node->color = R;
    node->has_list = 0;
    node->low = low;
    node->high = node->max = high;
    node->left = node->right = E;
    node->entry = entry;

    return node;
}

//------------------------------------------------------------------------
// creates a copy of tree node if the node is shared
//------------------------------------------------------------------------
itree_t::node_t *
itree_t::node_copy(const itree_t::node_t *node)
{
    if (!ref_is_shared(&node->ref_count)) {
        return  (itree_t::node_t *)node;
    }

    itree_t::node_t *copy = node_alloc();

    copy->color = node->color;
    copy->has_list = node->has_list;
    if (node->has_list) {
        copy->list = node->list->clone();
    } else {
        copy->entry = ref_clone(node->entry);
    }
    copy->low = node->low;
    copy->high = node->high;
    copy->max = node->max;

    copy->left = node->left->clone();
    copy->right = node->right->clone();

    node->deref();

    return copy;
}

#define EMPTY(n) ((n)->left == (n))

// macros to update tress
#define T(c, l, n, r) ((n)->update(c, l, r))
#define TC(n, c) T(c, (n)->left, n, (n)->right) // update color
#define TR(n, r) T((n)->color, (n)->left, n, r) // update right
#define TL(n, l) T((n)->color, l, n, (n)->right) // update left

// extract tree color, left, right
// returns true if thew color of the node matches
#define ET(node, c, l, n, r)                            \
    (!EMPTY(node) && ((node)->color == (c)) &&          \
     (n = node) && (l = (n)->left) && (r = (n)->right))

//------------------------------------------------------------------------
// balance a tree removing the red-red violation
//------------------------------------------------------------------------
const itree_t::node_t *
itree_t::balance(uint8_t color, const itree_t::node_t *left,
        const itree_t::node_t *node, const itree_t::node_t *right)
{
    const node_t *x, *y, *z, *a, *b, *c, *d;

    node_ref_t refs[] = {node, left, right};

    if (color == B) {
        /*
         *  x,y,z are nodes updated  during rebalance. Nodes with capital
         *  letter are  black nodes. The following are 4 cases with
         *  red parent with a red child needs rebalance.
         *
         *
         *        Z            Z            X                X
         *      /  \         /   \        /  \             /  \
         *     y   d        x     d      a    y           a    z
         *    / \          / \               / \              / \
         *   x   c        a   y             b   z            y  d
         *  / \              / \               / \         /  \
         * a   b            b   c             c   d       b    c
         *
         *   case LL        case LR         case RR          case RL
         *
         *
         * Resulting tree after rebalance.
         *
         *                    y
         *                  /  \
         *                 X    Z
         *                / \  / \
         *               a  b  c  d
         *
         */
        if ((ET(left, R, x, y, c) && ET(x, R, a, x, b)) ||  // case LL
            (ET(left, R, a, x, y) && ET(y, R, b, y ,c))) {  // case LR
            z = node; d = right;
            return T(R, T(B, a, x, b), y, T(B, c, z, d));
        }

        if ((ET(right, R, b, y, z) && ET(z, R, c, z, d)) || // case RR
            (ET(right, R, y, z, d) && ET(y, R, b, y, c))) { // case RL
            x = node; a = left;
            return T(R, T(B, a, x, b), y, T(B, c, z, d));
        }
    } else if (color == BB) {
        /*
         *  Balance when the root is double black.
         *  x,y,z are nodes updated  during rebalance. Nodes with capital
         *  letter are  black nodes, double caps are dowble black noide.
         *  The following are 2 cases with  red parent with a red child
         *  needs rebalance.
         *
         *
         *          ZZ               XX
         *        /   \             /  \
         *       x     d           a    z
         *      / \                    / \
         *     a   y                  y   d
         *        / \               /  \
         *       b   c             b    c
         *
         *     case LR            case RL
         *
         *
         * Resulting tree after rebalance.
         *
         *                    Y
         *                  /  \
         *                 X    Z
         *                / \  / \
         *               a  b  c  d
         */

        if (ET(left, R, a, x, y) && ET(y, R, b, y ,c)) { // case LR
            z = node; d = right;
            return T(B, T(B, a, x, b), y, T(B, c, z, d));
        }

        if (ET(right, R, y, z, d) && ET(y, R, b, y, c)) { // case RR
            x = node; a = left;
            return T(B, T(B, a, x, b), y, T(B, c, z, d));
        }
    }

    return T(color, left, node, right);
}

//------------------------------------------------------------------------
// Rotate the tree removing double black violation
//------------------------------------------------------------------------
const itree_t::node_t *
itree_t::rotate(uint8_t color, const itree_t::node_t *left,
       const itree_t::node_t *node, const itree_t::node_t *right)
{
    const itree_t::node_t *x, *y, *z, *w, *a, *b, *c, *d, *e;

    if (left->color != BB && right->color != BB) {
        return T(color, left, node, right);
    }

    node_ref_t refs[] = {left, node, right};

    // rotate R (T BB a x b) y (T B c z d) = balance B (T R (T B a x b) y c) z d
    if (color == R && ET(left, BB, a, x, b) && ET(right, B, c, z, d)) {
        return balance(B, T(R, T(B, a, x, b), node, c), z, d);
    }

    // rotate R EE y (T B c z d) = balance B (T R E y c) z d
    if (color == R && left == EE && ET(right, B, c, z, d)) {
        return balance(B, T(R, E, node, c), z, d);
    }

    // rotate R (T B a x b) y (T BB c z d) = balance B a x (T R b y (T B c z d))
    if (color == R && ET(left, B, a, x, b) && ET(right, BB, c, z, d)) {
        return balance(B, a, x, T(R, b, node, T(B, c, z, d)));
    }

    // rotate R (T B a x b) y EE = balance B a x (T R b y E)
    if (color == R && ET(left, B, a, x, b) && right == EE) {
        return balance(B, a, x, T(R, b, node, E));
    }

    // rotate B (T BB a x b) y (T B c z d) = balance BB (T R (T B a x b) y c) z d
    if (color == B && ET(left, BB, a, x, b) && ET(right, B, c, z, d)) {
        return balance(BB, T(R, T(B, a, x, b), node, c), z, d);
    }

    // rotate B EE y (T B c z d) = balance BB (T R E y c) z d
    if (color == B && left == EE && ET(right, B, c, z, d)) {
        return balance(BB, T(R, E, node, c), z, d);
    }

    // rotate B (T B a x b) y (T BB c z d) = balance BB a x (T R b y (T B c z d))
    if (color == B && ET(left, B, a, x, b) && ET(right, BB, c, z, d)) {
        return balance(BB, a, x, T(R, b, node, T(B, c, z, d)));
    }

    // rotate B (T B a x b) y EE = balance BB a x (T R b y E)
    if (color == B && ET(left, B, a, x, b) && right == EE) {
        return balance(BB, a, x, T(R, b, node, E));
    }

    // Following 4 cases access grand child, so need to grab a ref for that

    // rotate B (T BB a w b) x (T R (T B c y d) z e) =
    //      T B (balance B (T R (T B a w b) x c) y d) z e
    if (color == B && ET(left, BB, a, w, b) && ET(right, R, y, z, e) && ET(y, B, c, y, d)) {
        node_ref_t ref{y};
        return T(B, balance(B, T(R, T(B, a, w, b), node, c), y, d), z, e);
    }

    // rotate B EE x (T R (T B c y d) z e) = T B (balance B (T R E x c) y d) z e
    if (color == B && left == EE && ET(right, R, y, z, e) && ET(y, B, c, y, d)) {
        node_ref_t ref{y};
        return T(B, balance(B, T(R, E, node, c), y, d), z, e);
    }

    // rotate B (T R a w (T B b x c)) y (T BB d z e) =
    //    T B a w (balance B b x (T R c y (T B d z e)))
    if (color == B && ET(left, R, a, w, x) && ET(x, B, b, x, c) && ET(right, BB, d, z, e)) {
        node_ref_t ref{x};
        return T(B, a, w, balance(B, b, x, T(R, c, node, T(B, d, z, e))));
    }

    // rotate B (T R a w (T B b x c)) y EE = T B a w (balance B b x (T R c y E))
    if (color == B && ET(left, R, a, w, x) && ET(x, B, b, x, c) && right == EE) {
        node_ref_t ref{x};
        return T(B, a, w, balance(B, b, x, T(R, c, node, E)));
    }

    // rotate color a x b = T color a x b
    return T(color, left, node, right);
}

//------------------------------------------------------------------------
// inserts entry to interval tree
// in case if a matching interval already exists, then entry is
// added to an ordered list before the entry for wich match() returns true
//
// Returns pointer to new tree
//------------------------------------------------------------------------
const itree_t::node_t *
itree_t::insert(const itree_t::node_t *node, uint32_t low, uint32_t high,
                const ref_t *entry, const void *arg, itree_t::cb_t match)
{
    if (EMPTY(node)) {
        return node_create(low, high, entry);
    }

    itree_t::node_t *copy = node->copy();

    // inseret into left subtree when
    //  - interval's low is less than node's low
    //  - interval's high is less than node's high and low is
    //    equal to node's low
    // otherwise, insert into right subtree
    if ((low < copy->low) || (low == copy->low && high < copy->high)) {
        return balance(copy->color,
                       insert(copy->left, low, high, entry, arg, match),
                       copy, copy->right);
    } else if (low > copy->low || high > copy->high) {
        return balance(copy->color, copy->left, copy,
                       insert(copy->right, low, high, entry, arg, match));
    }

    // interval matches the current node interval
    // add the entry to current node's list
    if (!copy->has_list) {
        const list_t *list = list_t::create();
        list_t::insert(&list, copy->entry, arg, match);
        copy->list = list;
        copy->has_list = 1;
    }

    list_t::insert(&copy->list, entry, arg, match);
    return copy;
}


//------------------------------------------------------------------------
// deletes the left most node in the tree
//
// Return the deleted node with its right subtree
//------------------------------------------------------------------------
const itree_t::node_t *
itree_t::min_del(const itree_t::node_t *node)
{
    HAL_ASSERT(!EMPTY(node));

    const itree_t::node_t *x, *y, *a, *b, *c;
    node_ref_t ref{node};

    // min_del (T R E x E) = (x,E)
    if (ET(node, R, a, x, b) && a == E && b == E) {
        return x;
    }

    // min_del (T B E x E) = (x,EE)
    if (ET(node, B, a, x, b) && a == E && b == E) {
        return TR(x, EE);
    }

    // min_del (T B E x (T R E y E)) = (x,T B E y E)
    if (ET(node, B, a, x, y) && ET(y, R, b, y, c) && a == E && b == E && c == E) {
        return TR(x, T(B, E, y, E));
    }

    // min_del (T c a x b) = let (x’,a’) = min_del a
    //   in (x’,rotate c a’ x b)
    const itree_t::node_t *min = min_del(node->left);

    return TR(min, rotate(node->color, min->right, node, node->right));
}

//------------------------------------------------------------------------
// removes matching entry from per node list, returns null if last
// entry is removed
//------------------------------------------------------------------------
const itree_t::node_t *
itree_t::del_entry(const itree_t::node_t *node, const void *arg, cb_t match)
{
    if (node->has_list) {
        const list_t *list = node->list;

        if (list_t::remove(&list, arg, match) == HAL_RET_ENTRY_NOT_FOUND) {
            return node;
        }

        // list is updated, create a copy and update list
        itree_t::node_t *copy = node->copy();
        copy->list = list;

        // if the list has only one element,  remove the list and store the
        // entry directly
        if (copy->list->size() <= 1) {
            const list_t *list = copy->list;
            copy->entry = ref_clone(list->find());
            copy->has_list = 0;
            list->deref();
        }

        return copy;
    }

    // single entry, delete the node if the entry matches
    if (!match || match(arg, node->entry)) {
        // delete only the node, not the children
        node = T(node->color, E, node, E);
        node->deref();
        return NULL;
    }

    return node;
}

//------------------------------------------------------------------------
// Delete the matching entry with specified interval
//
// Returns the new updated tree
//------------------------------------------------------------------------
const itree_t::node_t *
itree_t::del(const itree_t::node_t *node, uint32_t low, uint32_t high,
    const void *arg, cb_t match)
{
    const node_t  *y, *z, *a, *b, *c;

    if (EMPTY(node)) {
        return node;
    }

    node_ref_t ref {node};

    // del (T R E y E) | x == y = E
    //                 | x /= y = T R E y E
    if (ET(node, R, a, y, b) && a == E && b == E) {
        if (low == y->low && high == y->high) {
            return del_entry(y, arg, match) ?: E;
        } else {
            return node;
        }
    }


    // del (T B E y E) | x == y = EE
    //                 | x /= y = T B E y E
    if (ET(node, B, a, y, b) && a == E && b == E) {
        if (low == y->low && high == y->high) {
            return del_entry(y, arg, match) ?: EE;
        } else {
            return node;
        }
    }

    //  del (T B (T R E y E) z E) | x < z = T B (del (T R E y E)) z E
    //                            | x == z = T B E y E
    //                            | x > z = T B (T R E y E) z E
    if (ET(node, B, y, z, c) && ET(y, R, a, y, b) && a == E && b == E && c == E) {
        if ((low < z->low) || (low == z->low && high < z->high)) {
            return T(B, del(y, low, high, arg, match), z, E);
        } else if (low == z->low && high == z->high) {
            return del_entry(z, arg, match) ?: T(B, E, y, E);
        } else {
            return node;
        }
    }

    // del (T c a y b) | x < y = rotate c (del a) y b
    //                 | x == y = let (y’,b’) = min_del b
    //                     in rotate c a y’ b’
    //                 | x > y= rotate c a y (del b)
    y = node;
    a = node->left;
    b = node->right;
    uint8_t color = node->color;

    if ((low < y->low) || (low == y->low && high < y->high)) {
        return rotate(color, del(a, low, high, arg, match), y, b);
    } else if (low == y->low && high == y->high) {
        if ((y = del_entry(y, arg, match)) != NULL) {
            return y;
        }
        const itree_t::node_t *min = min_del(b);
        return rotate(color, a, min, min->right);
    } else {
        return rotate(color, a, y, del(b, low, high, arg, match));
    }

    return node;
}

//
// walks the tree calling cb for each intersecting interval,
// callback may return false to abort the list walk
//
// note: entries are orderd by user defined match function (usually rule
//       priority) only when inserted into the node's linked list. While looking
//       for highest priority match, aborting the walk based on the priority
//       makes sense only when walking the list entries.
//       So this function will not abort the tree walk when
//       the callback returns false, it only aborts the current list walk
//       at the matching node and continues the tree walk.
//
// returns no.of intervals mathched
uint32_t
itree_t::walk(const itree_t::node_t *node, uint32_t low,
              uint32_t high, const void *arg, cb_t cb)
{
    uint32_t matches = 0;

    if (EMPTY(node)) {
        return matches;
    }

    // if the walk interval's low is greater than the the node's max
    // value, skip walking this subtree
    if (low > node->max) {
        return matches;
    }


    // left subtree can have any interval from 0 to current node's max
    // so we need to walk the left subtree unconditionally
    matches += walk(node->left, low, high, arg, cb);

    // visit the entries at current node if the node's interval intersects
    // with the walk interval
    if (node->low <= high && node->high >= low) {
        if (node->has_list) {
            node->list->walk(arg, cb);
            matches += node->list->size();
        } else {
            cb(arg, node->entry);
            matches++;
        }
    }

    // right subtree will only have intervals  >= node->low
    // so we can skip the right subtree when walk interval's high
    // is less than current node low.
    if (high >= node->low) {
        matches += walk(node->right, low, high, arg, cb);
    }

    return matches;
}


//------------------------------------------------------------------------
// create a new itree
//------------------------------------------------------------------------
itree_t *
itree_t::create()
{
    itree_t *tree = (itree_t *)itree_slab_->alloc();

    tree->root_ = E;
    ref_init(&tree->ref_count_, [](const ref_t *ref) {
            itree_t *tree = container_of(ref, itree_t, ref_count_);
            tree->root_->deref();
            itree_slab_->free(tree);
        });

    return tree;
}

//------------------------------------------------------------------------
// creates a copy of the tree if it is shared
//------------------------------------------------------------------------
itree_t *
itree_t::copy() const
{
    if (!ref_is_shared(&ref_count_)) {
        return (itree_t *)this;
    }

    itree_t *tree = create();

    tree->root_ = root_->clone();

    this->deref();

    return tree;
}

//------------------------------------------------------------------------
// walks the tree calling callback for each intersecting interval
// callback may return false to abort the list walk
//
// note: entries are orderd by user defined match function (usually rule
//       priority) only when inserted into the node's linked list. While looking
//       for highest priority match, aborting the walk based on the priority
//       makes sense only when walking the list entries.
//       So this function will not abort the tree walk when
//       the callback returns false, it only aborts the current list walk
//       at the matching node and continues the tree walk.
//
// Returns no.of intervals matched
//------------------------------------------------------------------------
uint32_t
itree_t::walk(uint32_t low, uint32_t high, const void *arg, cb_t cb) const
{
    return walk(root_, low, high, arg, cb);
}

//------------------------------------------------------------------------
// Inserts interval in to the tree
// If the interval already exists, new entry will be added to per interval
// list before the first matching entry in that list or at the end of the
// list if no entry matches. If match fn is null entry is added at the front
// of the list.
//
// Returns new tree if the tree is shared, otherwise original tree
// is updated.
//------------------------------------------------------------------------
hal_ret_t
itree_t::insert(const itree_t **treep, uint32_t low, uint32_t high,
                const ref_t *entry, const void* arg, cb_t match)
{
    itree_t *copy = (*treep)->copy();
    *treep = copy;

    copy->root_ = insert(copy->root_, low, high, entry, arg, match);

    // make the root black
    copy->root_ = TC(copy->root_, B);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------
// deletes the first matching interval
//
// Returns new tree if the tree is shared, otherwise original tree
// is updated.
//------------------------------------------------------------------------
hal_ret_t
itree_t::remove(const itree_t **treep, uint32_t low, uint32_t high,
                const void *arg, cb_t match)
{
    itree_t *copy = (*treep)->copy();
    *treep = copy;

    // make the root red if both children are black
    if (!EMPTY(copy->root_) &&
        copy->root_->left->color == B && copy->root_->right->color == B) {
        copy->root_ = TC(copy->root_, R);
    }

    copy->root_ = del(copy->root_, low, high, arg, match);

    // make the root black
    copy->root_ = TC(copy->root_, B);

    return HAL_RET_OK;

}

//------------------------------------------------------------------------
// cost to lookup the specified interval if inserted into the subtree
// rooted at node.
//------------------------------------------------------------------------
uint32_t
itree_t::cost(uint32_t low, uint32_t high) const
{
    static uint32_t (*cost_)(const node_t *node, uint32_t low, uint32_t high) =
        [] (const node_t *node, uint32_t low, uint32_t high) -> uint32_t {
        if (EMPTY(node)) {
            return 0;
        }

        // will be insereted into left subtree when
        //  - interval's low is less than node's low
        //  - interval's high is less than node's high and low is
        //    equal to node's low
        // otherwise, into right subtree
        if ((low < node->low) || (low == node->low && high < node->high)) {
            return 1 + cost_(node->left, low, high);
        } else if (low > node->low || high > node->high) {
            return 1 + cost_(node->right, low, high);
        }

        // interval mathces the cur node, then it will be appended to
        // the per node list (so the cost of insertion will be 1 +
        // all the existing entriees in the subtree whose interval
        // intersects)
        return 1 + walk(node, low, high, NULL,
                        [](const void* arg, const ref_t *ref) { return true; });
    };

    return cost_(root_, low, high);
}


//------------------------------------------------------------------------
// Check if all the red black properties are met
//
// Returns black height or negative error
//------------------------------------------------------------------------
int itree_t::check() const
{
    static int (*check)(const node_t *node) = [](const node_t *node) -> int {
        // check node color (shouldn't be double black)
        if (node->color == BB)
            return -100;

        if (EMPTY(node))
            return 0;

        // check red-red viollation
        if (node->color == R && (node->left->color == R || node->right->color == R))
            return -110;

        // check interval
        if ((node->left->low > node->low) ||
            (!EMPTY(node->right) && node->right->low < node->low))
            return -120;

        // check max
        if (node->max != MAX(node->high, MAX(node->left->max, node->right->max)))
            return -130;

        // check left
        int lheight = check(node->left);
        if (lheight < 0)
            return lheight;

        // check right
        int rheight = check(node->right);
        if (rheight < 0)
            return rheight;

        // check height
        if (lheight != rheight)
            return -140;

        return (node->color == B) ? lheight + 1 : lheight;
    };

    if (this->root_->color != B)
        return -1;

    return check(this->root_);
}

//------------------------------------------------------------------------
// dump the tree
//------------------------------------------------------------------------
void
itree_t::dump() const
{
    static void (*dump)(const node_t *node, char typ, const std::string &prefix) =
        [](const node_t *node, char typ, const std::string &prefix) {
        if (!EMPTY(node)) {
            std::cout << prefix << typ << ":" << node << " " << (int)node->color << " ";
            std::cout <<  node->low << "-" << node->high << " " << node->ref_count.count <<  "\n";
            dump(node->left, 'L', prefix + "  ");
            dump(node->right, 'R', prefix + "  ");
        }
    };

    dump(this->root_, '*', "");
    std::cout << "\n";
}

} // namespace acl



