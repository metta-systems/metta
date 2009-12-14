// Allocate runs of small fixed-size objects, reusing freed space as soon as possible.
// Allocates 2**factor units of sizeof(type_t) as backing storage.
template<typename type_t, int factor>
class fixed_small_allocator_t
{
};

template<typename type_t>
class list_node_t
{
    type_t value;
    node_t* previous;
    node_t* next;
};

template<typename type_t>
class list_t
{
    typedef list_node_t<type_t> node_t;
    typedef fixed_small_allocator_t<node_t, 8> allocator_t;
};
