#ifndef __LR_VECTOR_H
#define __LR_VECTOR_H

#include "lr_defs.h"
#include "lr_utils.h"
#include "lr_allocator.h"

namespace lr {
	
///\cond
namespace detail_vector {
	typedef uint32		size_type;
	typedef sint32		difference_type;
}
///\endcond

/// Container class that implements subset of STL's vector interface.
/**
 Methods that provide functionality not specified by STL's vector interface are documented, otherwise STL's vector
 documentation can be used.
 lrVector class does not follow any memory allocation policy meaning that memory for each element is allocated
 when requested - block by block.
 lrVector class does not release memory automaticley when removing elements,
 only when it is requested explicitly or container class is destroyed.
 Template parameter T - specifies type of vector contents.
 Template parameter A - specifies allocator type used for allocating memory for containers contents.
*/
template < class T,class A=lrAllocatorUnsafe < T > >
class lrVectorBase {

public :

	typedef A				allocator;
	typedef T				value_type;
	/// Size(addressing) type definition for vector container class.
	typedef detail_vector::size_type	size_type;
	/// Iterator difference(distance) type definition for vector container class.
	typedef detail_vector::difference_type	difference_type;
	typedef T * 				iterator;
	typedef const T *			const_iterator;
	typedef T &				reference;
	typedef const T &			const_reference;

protected :

	value_type *	mVector;
	size_type	mSize,
			mCapacity;
			
public :

	lrVectorBase(void) : mVector(0),mSize(0),mCapacity(0) {}
	
	/// Creates vector object and reserves specified amount of memory.
	lrVectorBase(size_type reserveSize) : mVector(0),mSize(0),mCapacity(0) {
		reserve(reserveSize);
	}
	
	lrVectorBase(const lrVectorBase < T,A > & vector) : mVector(0),mSize(0),mCapacity(0) {
		copy_from(vector);
	}
	
	/// Destroys vector object.
	~lrVectorBase(void) {
		destroy ();
	}
	
	/// Returns true if vector container contains no items. O(1).
	bool empty(void) const {
		return !mSize;
	}
	
	/// Returns size of vector(item count). O(1).
	const size_type & size(void) const {
		return mSize;
	}
	
	/// Returns item count vector can store without reallocating memory. O(1)
	const size_type & capacity ( void ) const {
		return mCapacity;
	}
	
	/// Returns iterator to begining of the sequence.
	iterator begin(void) {
		return mVector;
	}
	
	/// Returns iterator to begining of the sequence.
	const_iterator begin(void) const {
		return mVector;
	}
	
	/// Returns iterator to end of the sequence.
	iterator end(void) {
		return mVector + mSize;
	}
	
	/// Returns iterator to end of the sequence.
	const_iterator end(void) const {
		return mVector + mSize;
	}
	
	/// Returns reference to object stored at specified index.
	/**
	 When lrDEBUG is defined checks if index is smaller than vector size.
	*/
	value_type & operator [] (size_type index) {
		lr_assert(index < mSize);
		return mVector[index];
	}
	
	/// Returns reference to object stored at specified index.
	/**
	 When lrDEBUG is defined checks if index is smaller than vector size.
	*/
	const value_type & operator [] (size_type index) const {
		lr_assert(index < mSize);
		return mVector[index];
	}
	
	/// Preallocates memory so that vector can store at least specified amount of items.
	/**
	 Does not shrink allocated memory.
	*/
	void reserve(size_type reserveSize) {
		if (reserveSize > mCapacity) 
			realloc(reserveSize);
	}
	
	/// Resizes vector without initializing allocated object memory.
	void resize_unsafe(size_type newSize);
	
	/// Releases memory allocated for data storage and clears vector.
	void destroy(void) {
		allocator::deallocate(mVector,mVector + mSize);
		mVector=0;
		mCapacity=mSize=0;
	}
	
	/// Clears vector object.
	/**
	 Clearing vector does not release any memory allocated for content storage,
	 it merely deinitializes data and "marks" vector as empty.
	*/
	void clear(void) {
		allocator::destruct(begin(),end());
		mSize=0;
	}
	
	/// Copies data from other vector object.
	void copy_from(const lrVectorBase < T,A > & src);
	
	lrVectorBase < T,A > & operator = (const lrVectorBase < T,A > & vector) {
		if (this != &vector)
			copy_from(vector);
		return *this;
	}
	
protected :

	/// Reallocates memory so that vector can store at least specified amount of items.
	/**
	 If new size is smaller than current vector is shrinked.
	*/
	void realloc(size_type newSize) {
		value_type * ptr=allocator::reallocate(mVector,newSize);
		lr_assert(ptr);
		mVector=ptr;mCapacity=newSize;
	}
};

/// More extensive STL's vector class implementation.
/**
 Template parameter T - specifies type of vector contents.
 Template parameter A - specifies allocator type used for allocating memory for containers contents.
*/
template < class T,class A=lrAllocator < T > >
class lrVector : public lrVectorBase < T,A > {

private :
	
	typedef lrVectorBase < T,A > Base;

public :

	lrVector(void) : Base() {}
	
	lrVector(typename Base::size_type reserveSize) : Base(reserveSize) {}
	
	/// Returns reference to first item in sequence.
	typename Base::reference front(void) {
		lr_assert(!Base::empty());
		return *Base::begin();
	}
	
	/// Returns reference to first item in sequence.
	typename Base::const_reference front(void) const {
		lr_assert(!Base::empty());
		return *Base::begin();
	}
	
	/// Returns reference to last item in sequence.
	typename Base::reference back(void) {
		lr_assert(!Base::empty());
		return *(Base::end() - 1);
	}
	
	/// Returns reference to last item in sequence.
	typename Base::const_reference back(void) const {
		lr_assert(!Base::empty());
		return *(Base::end() - 1);
	}
	
	/// Appends specified value to the end of the sequence. O(1).
	/**
	 Reallocates memory if necessary.
	*/
	void push_back(const typename Base::value_type & value) {
		Base::reserve(Base::mSize + 1);
		Base::allocator::construct(Base::mVector + Base::mSize++,value);
	}

	/// Appends range between two iterators to the end of container. O(last - first).
	void push_back(typename Base::const_iterator first,typename Base::const_iterator last) {
		typename Base::size_type distance=lr_distance(first,last);
		Base::reserve(Base::mSize + distance);
		Base::allocator::construct(	Base::mVector + Base::mSize,
						Base::mVector + Base::mSize + distance,first,last);
		Base::mSize+=distance;
	}
	
	/// Removes element at the back of the sequence. O(1).
	void pop_back(void) {
		if (Base::mSize)
			Base::allocator::destruct(Base::mVector + --Base::mSize);
	}
	
	/// Resizes sequence to specified size. O(n).
	/**
	 Newly allocated objects are copy constructed using specified value.
	*/
	void resize(	typename Base::size_type newSize,
			const typename Base::value_type & value=typename Base::value_type());
	
	/// Inserts specified value at specified position in sequence. O(n).
	void insert(typename Base::size_type pos,const typename Base::value_type & value);
	
	/// Inserts specified value at specified position in sequence. O(n).
	void insert(typename Base::const_iterator pos,const typename Base::value_type & value) {
		insert(static_cast < typename Base::size_type > (pos - Base::mVector),value);
	}

	/// Inserts specified range of values at specified position in sequence. O(n * (last - first)).
	void insert(typename Base::const_iterator pos,	typename Base::const_iterator first,
							typename Base::const_iterator last);
	
	/// Removes value at specified position in sequence. O(n).
	void erase(typename Base::size_type pos);
	
	/// Removes value at specified position in sequence. O(n).
	void erase(typename Base::const_iterator pos) {
		erase(static_cast < typename Base::size_type > (pos - Base::mVector));
	}

	/// Removes range of values between two specified iterators in sequence. O(n * (last - first)).
	void erase(typename Base::const_iterator first,typename Base::const_iterator last);
	
	/// Swaps contents of two vectors.
	void swap(lrVector < T,A > & other) {
		lr_swap(Base::mVector,other.mVector);
		lr_swap(Base::mSize,other.mSize);
		lr_swap(Base::mCapacity,other.mCapacity);
	}
};

template < class T,class A >
void lrVectorBase < T,A >::copy_from(const lrVectorBase < T,A > & src) {
	allocator::destruct(begin(),end());
	reserve(mSize=src.size());
	if (mSize)
		allocator::construct(begin(),end(),src.begin(),src.end());
}

template < class T,class A >
void lrVectorBase < T,A >::resize_unsafe(size_type newSize) {
	if (newSize > mSize)
		reserve(newSize);
	else if (newSize < mSize)
		allocator::destruct(begin() + newSize,end());
	mSize=newSize;
}

template < class T,class A >
void lrVector < T,A >::resize(typename Base::size_type newSize,const typename Base::value_type & value) {
	if (newSize > Base::mSize) {
		Base::reserve(newSize);
		Base::allocator::construct(Base::end(),Base::begin() + newSize,value);
		}
	else if (newSize < Base::mSize)
		Base::allocator::destruct(Base::begin() + newSize,Base::end());
	Base::mSize=newSize;
}

template < class T,class A >
void lrVector < T,A >::insert(typename Base::size_type pos,const typename Base::value_type & value) {
	if (pos < Base::mSize) {
		Base::reserve(Base::mSize + 1);
		typename Base::iterator offset=Base::begin() + pos;
		for (typename Base::iterator it=Base::end(); it != offset; it--)
			Base::allocator::construct(it,*(it - 1));
		
		Base::allocator::construct(offset,value);
		Base::mSize++;
		}
	else
		push_back(value);
}

template < class T,class A >
void lrVector < T,A >::insert(	typename Base::const_iterator pos,
				typename Base::const_iterator first,
				typename Base::const_iterator last) {
	// Lazy insert...
	typename Base::size_type i=lr_distance((typename Base::const_iterator)Base::begin(),pos);
	Base::reserve(Base::mSize + lr_distance(first,last));
	while (last-- != first)
		insert(i,*last);
}

template < class T,class A >
void lrVector < T,A >::erase(typename Base::size_type pos) {
	if (Base::mSize == 1)
		Base::clear();
	else if (pos < Base::mSize) {
		typename Base::iterator it=Base::begin() + pos;
		Base::allocator::destruct(it);
		--Base::mSize;
		while (it != Base::end()) {
			Base::allocator::construct(it,static_cast < const typename Base::value_type & >(*(it + 1)));
			++it;
			}
		}
	else
		pop_back();
}

template < class T,class A >
void lrVector < T,A >::erase(typename Base::const_iterator first,typename Base::const_iterator last) {
	// Lazy erase...
	typename Base::size_type i	=lr_distance((typename Base::const_iterator)Base::begin(),first),
				 len	=lr_distance(first,last);
	while (len--)
		erase(i);
	
}

}

#endif
