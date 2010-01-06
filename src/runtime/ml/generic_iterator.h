//
// Generic iterator interface idea from Pedigree OS
// Copyright (c) 2008 James Molloy, Jörg Pfähler, Matthew Iselin
//

/*!
 * @brief An iterator applicable for many data structures.
 *
 * General iterator for structures that provide functions for the next and previous structure
 * in the datastructure and a "value" member. This template provides a bidirectional, a constant
 * bidirectional, a reverse bidirectional and a constant reverse bidirectional iterator.
 *
 * @param[in] original_type the original element type of the iterator
 * @param[in] Struct the datastructure that provides functions for the next/previous datastructure
 *                   and a "value" member
 * @param[in] FunctionPrev pointer to the member function used to iterate backwards.
 * @param[in] FunctionNext pointer to the member function used to iterate forward.
 * @param[in] type_t the real element type of the iterator
 */
template<typename original_type,
            class Struct,
            Struct *(Struct::*FunctionPrev)() = &Struct::previous,
            Struct *(Struct::*FunctionNext)() = &Struct::next,
            typename type_t = original_type>
class generic_iterator_t
{
public:
    /*! Type of the constant bidirectional iterator */
    typedef generic_iterator_t<original_type, Struct, FunctionPrev, FunctionNext, type_t const> const_iterator;
    /*! Type of the reverse iterator */
    typedef generic_iterator_t<original_type, Struct, FunctionNext, FunctionPrev, type_t>       reverse_iterator;
    /*! Type of the constant reverse iterator */
    typedef generic_iterator_t<original_type, Struct, FunctionNext, FunctionPrev, type_t const> const_reverse_iterator;

    /*! The default constructor constructs an invalid/unusable iterator */
    generic_iterator_t()
        : node(0)
    {}
    /*! The copy-constructor
     * @param[in] x the reference object */
    generic_iterator_t(const generic_iterator_t& x)
        : node(x.node)
    {}
    /*! The conversion constructor
     *\param[in] Iterator the reference object */
//     template<typename T2>
//     Iterator(const Iterator<originalT, Struct, FunctionPrev, FunctionNext, T2> &x)
//       : m_Node(x.m_Node){}
    /*! Constructor from a pointer to an instance of the data structure.
     * @param[in] node_ pointer to an instance of the data structure */
    generic_iterator_t(Struct* node_)
        : node(node_)
    {}
    /*! The destructor does nothing */
    ~generic_iterator_t() {}

    /*! The assignment operator
     *\param[in] Iterator the reference object */
    generic_iterator_t& operator =(const generic_iterator_t& x)
    {
        node = x.node;
        return *this;
    }

    /*! Preincrement operator */
    generic_iterator_t& operator ++()
    {
        node = (node->*FunctionNext)();
        return *this;
    }

    /*! Postincrement operator */
    generic_iterator_t operator ++(int)
    {
        generic_iterator_t tmp(this);
        node = (node->*FunctionNext)();
        return tmp;
    }

    /*! Predecrement operator */
    generic_iterator_t& operator --()
    {
        node = (node->*FunctionPrev)();
        return *this;
    }

    /*! Postdecrement operator */
    generic_iterator_t operator --(int)
    {
        generic_iterator_t tmp(this);
        node = (node->*FunctionPrev)();
        return tmp;
    }

    /** Dereference operator yields the element value */
    type_t& operator *()
    {
        // Verify that we actually have a valid node
        if(node)
            return node->value;
        else
        {
            static type_t ret = 0;
            return ret;
        }
    }

    /** Dereference operator yields the element value */
    type_t& operator ->()
    {
        return node->value;
    }

    /** Conversion Operator to a constant iterator */
    operator const_iterator()
    {
        return const_iterator(node);
    }

    /** Get the Node */
//     Struct *__getNode()
//     {
//       return m_Node;
//     }

protected:
    /*! Pointer to the instance of the data structure or NULL. */
    Struct* node;
};
