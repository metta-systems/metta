#ifndef __LR_STRING_UTF8_H
#define __LR_STRING_UTF8_H

#include "lr_vector.h"
#include "lr_algorithm.h"

namespace lr {

/// UTF8 string container class.
/**
 Template parameter A - specifies allocator used by underlying storage class.
*/
template < class A=lrAllocatorUnsafe < uint8 > >
class lrStringUTF8 {

public :

	typedef A				Allocator;
	typedef lrVector < uint8,Allocator >	Storage;

	typedef uint32				size_type;
	typedef uint8				value_type;
	typedef uint32				code_point;

	typedef uint8				Buffer[5];

	/// Converter from ASCII string to UTF8 string representation.
	struct ASCII {

		const sint8 * mString;

		/// Initializes converter with pointer to 8 bit ASCII string.
		ASCII(const sint8 * string) : mString(string) {}

		/// Invokes conversion from ASCII string to UTF8 string.
		const ASCII & operator ()(lrStringUTF8 < A > & utf8) const {
			if (mString && *mString) {
				const sint8 * ptr=mString;
				do {
					utf8.push_back(*ptr++);
				} while(*ptr);
				}
			return *this;
		}
	};

	/// Constant random access iterator for accessing individual code points in UTF8 string.
	/**
	 If string is simple(only 8 bit encodings) access is O(1), otherwise O(n).
	*/
	struct const_iterator {

		friend class lrStringUTF8 < A >;

	private :

		mutable const value_type * 	mPtr;
		bool				mSimple;

	public :

		const_iterator(const value_type * ptr,bool simple)
			: mPtr(ptr),mSimple(simple) {}

		/// Returns code point at current iterator's position.
		code_point operator * (void) const {
			return get_code_point();
		}

		/// Increments iterator so that it references next code point in sequence.
		const const_iterator & operator ++ (void) const {
			advance(1);
			return *this;
		}

		/// Increments iterator so that it references next code point in sequence.
		void operator ++ (int) const {
			advance(1);
		}

		/// Adds specified offset to iterator's position.
		const const_iterator & operator + (size_type offset) const {
			advance(offset);
			return *this;
		}

		/// Returns true if two iterators are referencing same code point in sequence.
		bool operator == (const const_iterator & it) const {
			return mPtr == it.mPtr;
		}

		/// Returns true if two iterators are not referencing same code point in sequence.
		bool operator != (const const_iterator & it) const {
			return mPtr != it.mPtr;
		}

	private :

		code_point get_code_point(void) const {
			uint8 length=get_sequence_length(*mPtr);
			switch (length) {
				case 1 :
					return code_point(*mPtr);
				case 2 :
					return ((*mPtr << 6) & 0x7FF) + (*(mPtr + 1) & 0x3F);
				case 3 :
					return ((*mPtr << 12) & 0xFFFF) + ((*(mPtr + 1) << 6) & 0xFFF) + (*(mPtr + 2) & 0x3F);
				case 4 :
					return ((*mPtr << 18) & 0x1FFFFF) + ((*(mPtr + 1) << 12) & 0x3FFFF) + ((*(mPtr + 2) << 6) & 0xFFF) + ((*mPtr + 3) & 0x3F);
				}
			return 0;
		}

		static uint8 get_sequence_length(value_type byte) {
			if (byte < 0x80)
				return 1;
			else if ((byte >> 5) == 0x6)
				return 2;
			else if ((byte >> 4) == 0xE)
				return 3;
			else if ((byte >> 3) == 0x1E)
				return 4;
			return 0;
		}

		void advance(size_type offset) const {
			if (mSimple)
				mPtr+=offset;
			else while(offset--) {
				uint8 distance=get_sequence_length(*mPtr);
				mPtr+=distance;
				}
		}
	};

private :

	Storage		mString;
	size_type	mLength;
	bool		mSimple;

public :

	lrStringUTF8(void) : mString(),mLength(0),mSimple(true) {}
	
	/// Initializes string from specified iterator and iteration count.
	lrStringUTF8(const_iterator begin,size_type count) : mString(length),mLength(0),mSimple(true) {
		while (count--)
			push_back(*begin++);	
	}

	/// Initializes UTF8 string from ASCII string.
	lrStringUTF8(const ASCII & ascii) : mString(),mLength(0),mSimple(true) {
		ascii(*this);
	}

	~lrStringUTF8(void) {}

	/// Returns iterator to beginning of string sequence.
	const_iterator begin(void) const {
		return const_iterator(mString.begin(),mSimple);
	}

	/// Returns iterator to end of string sequence.
	const_iterator end(void) const {
		return const_iterator(mString.end(),mSimple);
	}

	/// Returns code point at specified offset.
	code_point operator [] (size_type offset) {
		lr_assert(offset < mLength);
		return *(begin() + offset);
	}

	/// Returns true if string is empty.
	bool empty(void) const {
		return !mLength;
	}

	/// Returns length of string (in code-points).
	const size_type & length(void) const {
		return mLength;
	}

	/// Appends specified code point to end of the sequence.
	void push_back(code_point cp) {
		Buffer buffer;
		uint8 cpLength=encode(cp,buffer);
		lr_assert(cpLength);
		if (cpLength > 1)
			mSimple=false;
		mString.push_back(&buffer[0],&buffer[cpLength]);
		++mLength;
	}

	/// Inserts code point at specified offset.
	void insert(size_type offset,code_point cp) {
		lr_assert(offset <= mLength);
		Buffer buffer;
		uint8 cpLength=encode(cp,buffer);
		lr_assert(cpLength);
		if (cpLength > 1)
			mSimple=false;
		const_iterator pos=begin() + offset;
		mString.insert(pos.mPtr,&buffer[0],&buffer[cpLength]);
		++mLength;
	}

	/// Erases code point at specified offset.
	void erase(size_type offset) {
		lr_assert(offset < mLength);
		const_iterator pos=begin() + offset;
		mString.erase(pos.mPtr,
			pos.mPtr + const_iterator::get_sequence_length(*pos.mPtr));
		--mLength;
	}

	/// Replaces code point at specified offset.
	void replace(size_type offset,code_point cp) {
		lr_assert(offset < mLength);
		Buffer buffer;
		uint8 cpLength=encode(cp,buffer);
		lr_assert(cpLength);
		if (cpLength > 1)
			mSimple=false;
		const_iterator pos=begin() + offset;
		value_type * ptr=const_cast < value_type * > (pos.mPtr);
		uint8 currCPLen=const_iterator::get_sequence_length(*ptr);
		if (currCPLen == cpLength) {
			lr_memcpy(ptr,buffer,cpLength);
			}
		else {
			typename Storage::size_type storageOffset=lr_distance(mString.begin(),ptr);
			mString.erase(ptr,ptr + currCPLen);
			mString.insert(mString.begin() + storageOffset,&buffer[0],&buffer[cpLength]);
			}
	}

	/// Appends specified code point to end of the sequence.
	lrStringUTF8 < A > & operator += (code_point cp) {
		push_back(cp);
		return *this;
	}

	/// Appends specified string object to end of the sequence.
	lrStringUTF8 < A > & operator += (const lrStringUTF8 < A > & string) {
		mSimple=mSimple && string.mSimple;
		mString.push_back(string.mString.begin(),string.mString.end());
		mLength+=string.mLength;
		return *this;
	}

	lrStringUTF8 < A > operator + (const lrStringUTF8 < A > & string) {
		lrStringUTF8 < A > temp(*this);
		temp+=string;
		return temp;
	}

	/// Clears string.
	void clear(void) {
		mString.clear();
		mLength=0;
		mSimple=true;
	}

	/// Returns reference to underlaying storage object.
	const Storage& get_storage(void) const {
		return mString;
	}

private:

    uint8 encode(code_point cp,uint8 * buffer)
    {
        uint8 cpLength=get_cp_length(cp);
        if (cpLength == 1)
            *buffer=static_cast < uint8 >(cp);
        else if (cpLength == 2) {
            *buffer++=static_cast < uint8 >((cp >> 6) | 0xC0);
            *buffer  =static_cast < uint8 >((cp & 0x3F) | 0x80);
        }
        else if (cpLength == 3) {
            *buffer++=static_cast < uint8 >((cp >> 12) | 0xE0);
            *buffer++=static_cast < uint8 >((cp >> 6) & 0x3F | 0x80);
            *buffer  =static_cast < uint8 >((cp & 0x3F) | 0x80);
        }
        else if (cpLength == 4) {
            *buffer++=static_cast < uint8 >((cp >> 18) | 0xF0);
            *buffer++=static_cast < uint8 >((cp >> 12) & 0x3F | 0x80);
            *buffer++=static_cast < uint8 >((cp >> 6) & 0x3F | 0x80);
            *buffer  =static_cast < uint8 >((cp & 0x3F) | 0x80);
        }
        return cpLength;
    }

    static uint8 get_cp_length(code_point cp) {
        if (cp < 0x80)
            return 1;
        else if (cp < 0x800)
            return 2;
        else if (cp < 0x10000)
            return 3;
        else if (cp < 0x0010FFFFu)
            return 4;
        return 0;
    }
};

}

#endif
