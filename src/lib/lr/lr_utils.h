#ifndef __LR_UTILS_H
#define __LR_UTILS_H

#include "types.h"

namespace lr {

/// Swaps two objects.
template <class Type>
inline void lr_swap(Type & first,Type & second) {
	Type temp(first);
	first=second,second=temp;
}

/// Aritmetic swap routine.
template <class Numeric>
void lr_xor_swap(Numeric & first,Numeric & second) {
	((first) == (second)) || (((first) ^= (second)), ((second) ^= (first)), ((first) ^= (second)));
}

/// Specialization for 32 bit signed integers.
template <>
inline void lr_swap(int32_t & first,int32_t & second) {
	lr_xor_swap(first,second);
}

/// Specialization for 32 bit unsigned integers.
template <>
inline void lr_swap(uint32_t & first,uint32_t & second) {
	lr_xor_swap(first,second);
}

/// Returns largest of two objects.
/**
 Return value is determined by > operator.
*/
template <class Type>
inline const Type& lr_max(const Type & first,const Type & second) {
    return first > second ? first : second;
}

/// Returns smallest of two objects.
/**
 Return value is determined by < operator.
*/
template <class Type>
inline const Type& lr_min(const Type & first,const Type & second) {
    return first < second ? first : second;
}

/// Returns absolute value of numeric object.
template <class Numeric>
Numeric lr_abs(const Numeric & value) {
	return value < 0 ? -value : value;
}

/// lrIsSigned::Result is true for numeric objects that are signed.
template <class T>
struct lrIsSigned {
    // This part might generate unwanted warnings for unsigned types.
    enum { Result=static_cast < T >(-1) < 0 };
};

/// lrIsPointer::Result is true for pointer type template parameters.
template < class T >
struct lrIsPointer {
	
	enum { Result=0 };
};

/// lrIsPointer::Result is true for pointer type template parameters.
template < class T >
struct lrIsPointer < T * > {
	
	enum { Result=1 };
};

/// lrIsSameType accepts two template parameters and determines if both are of the same type.
/**
 lrIsSameType::Result is true for same type parameters.
*/
template < class A,class B >
struct lrIsSameType {

	enum { Result=0 };
};

/// lrIsSameType accepts two template parameters and determines if both are of the same type.
/**
 lrIsSameType::Result is true for same type parameters.
*/
template < class A >
struct lrIsSameType < A,A > {

	enum { Result=1 };
};

/// lrBooleanExpresion::Result is of type lrTrue if T evaluates to true and is of type lrFalse otherwise.
template < bool T >
struct lrBooleanExpression {};

/// True type tag - used to identify true compile time expressions.
struct lrTrue {};

/// False type tag - used to identify false compile time expressions.
struct lrFalse {};

/// lrBooleanExpresion::Result is of type lrTrue if T evaluates to true and is of type lrFalse otherwise.
template <>
struct lrBooleanExpression < true > {

	typedef lrTrue	Result;
};

/// lrBooleanExpresion::Result is of type lrTrue if T evaluates to true and is of type lrFalse otherwise.
template <>
struct lrBooleanExpression < false > {

	typedef lrFalse	Result;
};

}

#endif
