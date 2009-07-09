#ifndef __LR_ALGORITHM_H
#define __LR_ALGORITHM_H

#include "types.h"
#include "lr_utils.h"

namespace lr {

/// Compile time iterator type information.
template < class Iterator >
struct lrIteratorTraits {

	typedef sint32 difference_type;
};

/// Predicate function that determines if first object is smaller than second. O(1).
template < class First,class Second >
bool lr_less(const First & first,const Second & second) {
	return first < second;
}

/// Reverses range between two iterators begin and end. O(n).
template < class Iterator >
void lr_reverse(Iterator begin,Iterator end) {
	while(true)
		if (begin == end || begin == --end)
			return;
		else {
			lr_swap(*begin,*end);
			++begin;
			}
}

/// Finds value if any in given range, returns iterator referencing location of found value. O(n).
template < class Iterator,class Value >
Iterator lr_linear_search(Iterator first,Iterator last,const Value & value) {
	while (first != last && !(*first == value))
		++first;
	return first;
}

/// Finds value if any in given range using predicate function, returns iterator referencing location of found value. O(n).
template < class Iterator,class Value,class Predicate >
Iterator lr_linear_search(Iterator first,Iterator last,const Value & value,Predicate predicate) {
	while (first != last && !(predicate(*first,value)))
		++first;
	return first;
}

/// Returns lower bound. Olog(n).
/**
 Returns reference to position within specified range where specified value can be inserted without breaking 
 order of the sequence.
*/
template < class Iterator,class Value >
Iterator lr_lower_bound(Iterator first,Iterator last,const Value & value) {
	typedef typename lrIteratorTraits < Iterator >::difference_type Difference;

	Difference	len=lr_distance(first,last),
			half;
	Iterator	middle;
	while (len > 0) {
		half=len >> 1;
		middle=first + half;
		if (*middle < value) {
			first=middle + 1;
			len=len - half - 1;
			}
		else
			len=half;
		}
	return first;
}

/// Returns lower bound using predicate functor. Olog(n).
/**
 Returns reference to position within specified range where specified value can be inserted without breaking 
 order of the sequence.
*/
template < class Iterator,class Value,class Predicate >
Iterator lr_lower_bound(Iterator first,Iterator last,const Value & value,Predicate predicate) {
	typedef typename lrIteratorTraits < Iterator >::difference_type Difference;

	Difference	len=lr_distance(first,last),
			half;
	Iterator	middle;
	while (len > 0) {
		half=len >> 1;
		middle=first + half;
		if (predicate(*middle,value)) {
			first=middle + 1;
			len=len - half - 1;
			}
		else
			len=half;
		}
	return first;
}

/// Returns distance between two iterators. Complexity depends on iterator type - O(1) for random access iterators.
template < class Iterator >
typename lrIteratorTraits < Iterator >::difference_type lr_distance(Iterator begin,Iterator end) {
	return static_cast < typename lrIteratorTraits < Iterator >::difference_type > (end - begin);
}

/// Checks if iterator is equal to value. Meant for usage with lr_lower_bound results. O(1).
template < class Iterator,class Value >
bool lr_is_found(Iterator iterator,Iterator end,const Value & value) {
	return iterator != end && !(*iterator < value) && !(value < *iterator);
}

/// Checks if iterator is equal to value using predicate functor. O(1).
template < class Iterator,class Value,class Predicate >
bool lr_is_found(Iterator iterator,Iterator end,const Value & value,Predicate predicate) {
	return iterator != end && !predicate(*iterator,value) && !predicate(value,*iterator);
}

}

#endif
