//
// Part of Metta OS. Check https://atta-metta.net for latest version.
//
// Copyright 2007 - 2017, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
//B-trees C++ implement by Zhu Xinquan@2006-3-26
 
#include <iostream>
#include <ctime>
#include <vector>
#define ___DEBUG
using namespace std;
 
 
//the class definition of the element stored in each node;
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
    BTreeNode(BTreeNode* mp=NULL) : m_parent(mp){}
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
    //When the number of current node's element is overflow, perform split insertion.
    bool Split_insert(Elem &);
    //store the result of every search
    BTreeNode* search_result;
 
 
public:
    BTree(v_func f, int n, BTreeNode* r=NULL):m_visit(f),m_order(n),m_root(r)
    {
       search_result = NULL;
    };
    ~BTree();
    bool Insertion(Elem&);//Insert a element to the B-tree.
    bool Deletion(Elem&);
    BTreeNode* Search(Elem&);//Search an Element in B-tree.
    void travel(BTreeNode*) const; // travel the Btree
    void print() const { travel(m_root); } // print the elements in a sorted order
};
 
BTree::~BTree()
{
    //something to be added
   
}
void BTree::travel(BTreeNode* p) const
{
    if(p==NULL)
       return;
    int i;
    for(i=0;i < p->m_child.size()-1; i++)
    {
       travel(p->m_child[i]);
       m_visit(p->m_data[i]);
    }
    travel(p->m_child[i]);
   
}
//Search an Element in B-tree. Return the node pointer which contain the
//desired element or return NULL if the element isn't found.
BTreeNode* BTree::Search(Elem& t)
{
    BTreeNode* p = m_root;
    while (p)
    {
       //store the current search result.
       search_result = p;
       //perform binary search of the node
       int first=0, last=p->m_data.size()-1, mid = (first+last)/2;
       while (first<=last)
       {
           mid=(first+last)/2;
           if (p->m_data[mid]==t) {
              return p;
           }
           if (p->m_data[mid]>t) {
              last=mid-1;
           }
           if (p->m_data[mid]<t) {
              first=mid+1;
           }
       }
       //continue search in the child.
       if(p->m_data[mid]>t)
           p = p->m_child[mid];
       else
           p = p->m_child[mid+1];
    }
    return p;
}
 
bool BTree::Insertion(Elem& t)
{
    if(Search(t))//The element already exits in the tree
       return false;
    if(m_root==NULL)//The tree is empty
    {
       m_root = new BTreeNode;
       m_root->m_data.push_back(t);
       m_root->m_child.push_back(NULL);
       m_root->m_child.push_back(NULL);
       m_root->m_parent = NULL;
       return true;
    }
    else
    {
       if(search_result)
       {
           //If the element can fit into vector m_data, slide all the elements
           //greater than the arguement forward one position and insert the newly
           //vacated slot, then return true. Otherwise performan split.
           vector<Elem> *p = &search_result->m_data;
           vector<BTreeNode*> *pc = &search_result->m_child;
           int i;
           //locate the right position to insert
           for(i=0;i<p->size() && (*p)[i]<t;i++);
           p->insert(p->begin()+i,t);//insert the new Element
           pc->insert(pc->begin()+i,NULL);//insert the new child.
           if(p->size() < m_order)//dosen't need to split this node
           {
#ifdef ___DEBUG
              cout<<"Needn't to split! "<<endl;
              cout<<"Now the tree is ################  ";
              print();
              cout<<"\n\n\n";
#endif
              return true;
           }
           else
              return Split_insert(t);
       }
       else
           return false;
    }
 
}
 
 
bool BTree::Split_insert(Elem &t)
{
    BTreeNode* sr = search_result;
    if(sr)
    {
       BTreeNode* sr = search_result;
       vector<Elem> *p = &sr->m_data;
       vector<BTreeNode*> *pc = &sr->m_child;
       //begin to insert and split
       while(p->size() >= m_order)
       {     
           int split_point = p->size()/2;
           BTreeNode* new_node = new BTreeNode(sr->m_parent);
           //new node recieve rightmost half of current node. And after copying
           //data to new node, delete the righmost half of the current node
           int i,total= p->size();
           for(i=split_point+1;i<total;i++)
           {
              new_node->m_data.push_back((*p)[split_point+1]);
              new_node->m_child.push_back((*pc)[split_point+1]);
              //make the new node be the parent of the righmost half's children
              if((*pc)[split_point+1])
                  (*pc)[split_point+1]->m_parent = new_node;
              p->erase(p->begin()+split_point+1);
              pc->erase(pc->begin()+split_point+1);
           }
           //deal with the one more child
           new_node->m_child.push_back((*pc)[split_point+1]);
           //make the new node be the parent of the righmost half's children
           if((*pc)[split_point+1])
              (*pc)[split_point+1]->m_parent = new_node;
           pc->erase(pc->begin()+split_point+1);
           //delete the split_point
           Elem upward = (*p)[split_point];
           p->erase(p->begin()+split_point);
           //make the current's parent to be new_node's parent
           new_node->m_parent = sr->m_parent;
           if(sr == m_root)  //reach the root
           {
              //make a new node to be root.
              BTreeNode* new_node2 = new BTreeNode(NULL);
              m_root = new_node2;
              new_node2->m_data.push_back(upward);
              //the new root's two children are the splited two from current node
              new_node2->m_child.push_back(sr);
               new_node2->m_child.push_back(new_node);
              sr->m_parent = new_node2;
              new_node->m_parent = new_node2;
#ifdef ___DEBUG
              cout<<"\nNeed to make a new node to be root.";
              cout<<"The new node is "<<new_node2->m_data[0].data;
              cout<<"\nNow the tree is ################  ";
              print();
              cout<<"\n\n\n";
#endif
              return true;
           }
          
#ifdef ___DEBUG
           cout<<"\n^^^^^^^^^^^^\n"<<upward.data;
           cout<<" need to upward. The split node become the two:\n";
           for(i=0;i<p->size();i++)
              cout<<(*p)[i].data<<" ";
           cout<<"===";
           for(i=0;i<new_node->m_data.size();i++)
              cout<<new_node->m_data[i].data<<" ";
#endif
           //add the upward element to the parent of current node            
           p = &sr->m_parent->m_data;
           pc = &sr->m_parent->m_child;
           //locate the right position to insert
           for(i=0;i<p->size() && (*p)[i]<upward;i++);
           p->insert(p->begin()+i,upward);
           //adjust the parent's child
           pc->insert(pc->begin()+i+1,new_node);
#ifdef ___DEBUG
           cout<<"The new parent is: ";
           for(i=0;i<p->size();i++)
              cout<<(*p)[i].data<<" ";
           cout<<"\n^^^^^^^^^^^^^^\n";
#endif
 
           sr=sr->m_parent;    
       }
#ifdef ___DEBUG
       cout<<"\nNow the tree is ################  ";
       print();
       cout<<"\n\n\n";
#endif
       return true;
    }
    return false;
}
static void dump(Elem& e)
{
    cout<<e.data<<" ";
}

int main(int argc, char* argv[])
{
    BTree b(&dump,5);
    srand( (unsigned) time(NULL));
    int i,j;
    for(i=0;i<=1000;i++)
    {
       int num = rand()%100;
       Elem e1(num);
       cout<<"Insert "<<e1.data<<" ";
       b.Insertion(e1);
       BTreeNode* temp;
       if(temp = b.Search(e1))
       {
           cout<<"\nOK! find in the memory: 0x"<<temp;
           for(j=0;j<temp->m_data.size();j++)
              cout<<" "<<temp->m_data[j].data<<" ";
           cout<<endl;
       }
    }
   
    return 0;
}
