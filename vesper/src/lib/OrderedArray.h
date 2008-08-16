#ifndef __INCLUDED_ORDERED_ARRAY_H
#define __INCLUDED_ORDERED_ARRAY_H

#include "common.h"

/**
   This array is insertion sorted - it always remains in a sorted state (between calls).
   type_t must implement operator <()
**/
template<class type_t>
class OrderedArray
{
	public:
		/**
			Create an ordered array.
		**/
		OrderedArray(uint32_t max_size);
		OrderedArray(void *addr, uint32_t max_size); // placement ctor
		~OrderedArray();

		void insert(type_t *item);
		type_t *lookup(uint32_t i);
		void remove(uint32_t i);

	public://private:
		type_t **array;
		uint32_t size;
		uint32_t max_size;
		bool placed; // @c true if array was constucted using placement ctor
};

template<class type_t>
OrderedArray<type_t>::OrderedArray(uint32_t max_size)
{
	this->max_size = max_size;
	array = kmalloc(max_size * sizeof(type_t *));
	memset(array, 0, max_size * sizeof(type_t *));
	size = 0;
	placed = false;
}

template<class type_t>
OrderedArray<type_t>::OrderedArray(void *addr, uint32_t max_size)
{
	this->max_size = max_size;
	array = (type_t **)addr;
	memset(array, 0, max_size * sizeof(type_t *));
	size = 0;
	placed = true;
}

template<class type_t>
OrderedArray<type_t>::~OrderedArray()
{
	if (!placed)
		kfree((uint32_t)array);
}

template<class type_t>
void OrderedArray<type_t>::insert(type_t *item)
{
	uint32_t iterator = 0;
	while (iterator < size && *array[iterator] < *item)
		iterator++;
	if (iterator == size) // just add at the end of the array
		array[size++] = item;
	else
	{
		type_t *tmp = array[iterator];
		array[iterator] = item;
		while (iterator < size)
		{
			iterator++;
			type_t *tmp2 = array[iterator];
			array[iterator] = tmp;
			tmp = tmp2;
		}
		size++;
	}
}

template<class type_t>
type_t *OrderedArray<type_t>::lookup(uint32_t i)
{
    ASSERT(i < size);
    return array[i];
}

template<class type_t>
void OrderedArray<type_t>::remove(uint32_t i)
{
    size--;
    while (i < size)
    {
        array[i] = array[i+1];
        i++;
    }
}

#endif // __INCLUDED_ORDERED_ARRAY_H
