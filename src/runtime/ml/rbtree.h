//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

// a V type should provide comparison operators that maintain total linear order by comparing the keys

template <typename V>
struct rbnode_t
{
    typedef rbnode_t<V> self_type;

    enum color_e { red, black };
    color_e color;
    self_type* left;
    self_type* right;
    self_type* parent;
    V value;

    rbnode_t(V data, self_type* dad = 0) : color(red), left(0), right(0), parent(dad), value(data) {}
    ~rbnode_t() { delete left; delete right; }

    self_type* grandparent()
    {
        if (parent)
            return parent->parent;
        return 0;
    }

    self_type* uncle()
    {
        self_type* g = grandparent();
        if (g == 0)
            return 0; // No grandparent means no uncle
        if (parent == g->left)
            return g->right;
        else
            return g->left;
    }

    self_type* sibling()
    {
        if (!parent)
            return 0;
        if (this == parent->left)
            return parent->right;
        else
            return parent->left;
    }
};

/*!
* Red-black tree class, derived from example code in Wikipedia
* http://en.wikipedia.org/wiki/Rbtree
*/
template <typename V>
class rbtree_t
{
    typedef V           value_type;
    typedef rbnode_t<V> node_t;

    node_t* root;

public:
    rbtree_t() : root(0) {}
    ~rbtree_t() { delete root; }

    void insert(value_type value)
    {
        node_t* node = root;

        if (node == 0)
        {
            root = new node_t(value);
            root->color = node_t::black;
        }
        else
        {
            while (node != 0)
            {
                if (value < node->value)
                {
                    if (node->left == 0)
                    {
                        node->left = new node_t(value, node);
                        node = node->left;
                        break;
                    }
                    else
                        node = node->left;
                }
                else
                {
                    if (node->right == 0)
                    {
                        node->right = new node_t(value, node);
                        node = node->right;
                        break;
                    }
                    else
                        node = node->right;
                }
            }
            // insertion failed?
        }

        // Do tree rebalancing if needed.
        if (node)
            insert_case2(node);
    }

    bool search(V& value)
    {
        node_t* node = root;

        while (node != 0)
        {
            if (node->value == value)
            {
                value = node->value;
                return true;
            }
            if (value < node->value)
                node = node->left;
            else if (value > node->value)
                node = node->right;
        }

        return false;
    }

    bool fulfills_invariant() const
    {
        return tree_invariant_internal(root) > 0;
    }

private:
    void insert_case1(node_t* n)
    {
        if (n->parent == 0)
            n->color = node_t::black;
        else
            insert_case2(n);
    }

    void insert_case2(node_t* n)
    {
        if (n->parent->color == node_t::black)
            return; /* Tree is still valid */
        else
            insert_case3(n);
    }

    void insert_case3(node_t* n)
    {
        node_t* u = n->uncle();

        if (u && (u->color == node_t::red))
        {
            n->parent->color = node_t::black;
            u->color = node_t::black;
            node_t* g = n->grandparent();
            g->color = node_t::red;
            insert_case1(g); // fix grandparent node
        }
        else
        {
            insert_case4(n);
        }
    }

    void insert_case4(node_t* n)
    {
        node_t* g = n->grandparent();

        if ((n == n->parent->right) && (n->parent == g->left))
        {
            rotate_left(n->parent);
            n = n->left;
        }
        else if ((n == n->parent->left) && (n->parent == g->right))
        {
            rotate_right(n->parent);
            n = n->right;
        }
        insert_case5(n);
    }

    void insert_case5(node_t* n)
    {
        node_t* g = n->grandparent();

        n->parent->color = node_t::black;
        g->color = node_t::red;
        if ((n == n->parent->left) && (n->parent == g->left))
        {
            rotate_right(g);
        }
        else /* (n == n->parent->right) and (n->parent == g->right) */
        {
            rotate_left(g);
        }
    }

    void delete_one_child(node_t* n)
    {
        /*
         * Precondition: n has at most one non-null child.
         */
        node_t* child = (n->right == 0) ? n->left : n->right;

        replace_node(n, child);
        if (n->color == node_t::black)
        {
            if (child->color == node_t::red)
                child->color = node_t::black;
            else
                delete_case1(child);
        }
        delete n;
    }

    void delete_case1(node_t* n)
    {
        if (n->parent != NULL)
            delete_case2(n);
    }

    void delete_case2(node_t* n)
    {
        node_t* s = n->sibling();

        if (s->color == node_t::red)
        {
            n->parent->color = node_t::red;
            s->color = node_t::black;
            if (n == n->parent->left)
                rotate_left(n->parent);
            else
                rotate_right(n->parent);
        }
        delete_case3(n);
    }

    void delete_case3(node_t* n)
    {
        node_t* s = n->sibling();

        if ((n->parent->color == node_t::black) &&
            (s->color == node_t::black) &&
            (s->left->color == node_t::black) &&
            (s->right->color == node_t::black))
        {
            s->color = node_t::red;
            delete_case1(n->parent);
        }
        else
            delete_case4(n);
    }

    void delete_case4(node_t* n)
    {
        node_t* s = n->sibling();

        if ((n->parent->color == node_t::red) &&
            (s->color == node_t::black) &&
            (s->left->color == node_t::black) &&
            (s->right->color == node_t::black))
        {
            s->color = node_t::red;
            n->parent->color = node_t::black;
        }
        else
            delete_case5(n);
    }

    void delete_case5(node_t* n)
    {
        node_t* s = n->sibling();

        /* this if statement is trivial, due to Case 2 (even though Case 2 changed the sibling to a sibling's child,
        the sibling's child can't be red, since no red parent can have a red child). */
        if (s->color == node_t::black)
        {
            // the following statements just force the red to be on the left of the left of the parent,
            // or right of the right, so case six will rotate correctly.
            if ((n == n->parent->left) &&
                (s->right->color == node_t::black) &&
                (s->left->color == node_t::red)) // this last test is trivial too due to cases 2-4.
            {
                s->color = node_t::red;
                s->left->color = node_t::black;
                rotate_right(s);
            }
            else if ((n == n->parent->right) &&
                (s->left->color == node_t::black) &&
                (s->right->color == node_t::red)) // this last test is trivial too due to cases 2-4.
            {
                s->color = node_t::red;
                s->right->color = node_t::black;
                rotate_left(s);
            }
        }
        delete_case6(n);
    }

    void delete_case6(node_t* n)
    {
        node_t* s = n->sibling();

        s->color = n->parent->color;
        n->parent->color = node_t::black;

        if (n == n->parent->left)
        {
            s->right->color = node_t::black;
            rotate_left(n->parent);
        }
        else
        {
            s->left->color = node_t::black;
            rotate_right(n->parent);
        }
    }

    void rotate_right(node_t* rotation_root)
    {
        node_t* parent = rotation_root->parent;
        node_t* pivot = rotation_root->left;

        if (pivot)
        {
            rotation_root->left = pivot->right;
            pivot->right = rotation_root;
            if (parent)
            {
                if (parent->left == root)
                    parent->left = pivot;
                else
                    parent->right = pivot;
            }
            else // if no parent, this is the root node, it should become pivot
                root = pivot;
        }
    }

    void rotate_left(node_t* rotation_root)
    {
        node_t* parent = rotation_root->parent;
        node_t* pivot = rotation_root->right;

        if (pivot)
        {
            rotation_root->right = pivot->left;
            pivot->left = rotation_root;
            if (parent)
            {
                if (parent->left == root)
                    parent->left = pivot;
                else
                    parent->right = pivot;
            }
            else // if no parent, this is the root node, it should become pivot
                root = pivot;
        }
    }

    inline bool is_red(node_t* node) const
    {
        return node && node->color == node_t::red;
    }

    int tree_invariant_internal(node_t* root_node) const
    {
        int left_height, right_height;

        if (root_node == NULL)
            return 1;

        node_t* left_node = root_node->left;
        node_t* right_node = root_node->right;

        /* Consecutive red links */
        if (is_red(root_node))
        {
            if (is_red(left_node) || is_red(right_node))
            {
                // Red violation!
                return 0;
            }
        }

        left_height = tree_invariant_internal(left_node);
        right_height = tree_invariant_internal(right_node);

        /* Invalid binary search tree */
        if ((left_node && (left_node->value >= root_node->value))
        || (right_node && (right_node->value <= root_node->value)))
        {
            // Binary tree violation!
            return 0;
        }

        /* Black height mismatch */
        if (left_height != 0 && right_height != 0 && left_height != right_height)
        {
            // Black violation!
            return 0;
        }

        /* Only count black links */
        if (left_height != 0 && right_height != 0)
            return is_red(root_node) ? left_height : left_height + 1;

        return 0;
    }
};
