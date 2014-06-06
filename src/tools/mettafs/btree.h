//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

template<typename T>
class Element
{
public:
    T data;
    bool operator < (Element& e) const { return data < e.data;}
    bool operator <= (Element& e) const { return data <= e.data;}
    bool operator > (Element& e) const { return data > e.data;}
    bool operator >= (Element& e) const { return data >= e.data;}
    bool operator == (Element& e) const { return data == e.data;}
    Element& operator = (const Element& e)
    {
        data=e.data;
        return *this;
    }
    Element(T& d) : data(d){}
};

//An integer element.
typedef Element<int> Elem;

class BTreeNode
{
public:
    vector<Elem> m_data;
    //m_child[i] is the child whose keys are smaller than that of m_data[i];
    //and the size of m_child is always one more than m_data
    vector<BTreeNode*> m_child;
    //Here is my B-tree strcture representation
    //
    //                     m_data[0]  m_data[1] ... m_data[i] ...
    //      m_child[0]--> /         |          |             \ <--m_child[i+1]
    //                   /          |          |              \
    //
    BTreeNode* m_parent; // pointer to the parent BTreeNode
    BTreeNode(BTreeNode* mp = nullptr) : m_parent(mp) {}
};

class BTree
{
    typedef void (*v_func)(Elem &data);//visit function
protected:
    int m_order; //maximum number of the elements stored a node
    BTreeNode* m_root;//the root of the tree
    v_func m_visit;
    //return true if the insertion does not need split the current node
    bool Normal_insert(Elem &);
    //When the number of current node's element is overflow, perform split insertion
    bool Split_insert(Elem &);
    //store the result of every search
    BTreeNode* search_result;
public:
    BTree(v_func f, int n, BTreeNode* r = nullptr)
        : m_visit(f)
        , m_order(n)
        , m_root(r)
    {
        search_result = nullptr;
    };
    ~BTree();
    bool Insertion(Elem&);//Insert a element to the B-tree.
    bool Deletion(Elem&);
    BTreeNode* Search(Elem&);//Search an Element in B-tree.
    void travel(BTreeNode*) const; // travel the Btree
    void print() const { travel(m_root); } // print the elements in a sorted order
};

