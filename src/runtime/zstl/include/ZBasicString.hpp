/*
	ZBasicString.hpp
	Author: James Russell <jcrussell@762studios.com>
	Created: 12/25/2011

	Purpose: 

	Basic ASCII string implementation.

	License:

	This program is free software. It comes without any warranty, to
	the extent permitted by applicable law. You can redistribute it
	and/or modify it under the terms of the Do What The Fuck You Want
	To Public License, Version 2, as published by Sam Hocevar. See
	http://sam.zoy.org/wtfpl/COPYING for more details.

*/

#pragma once

#ifndef _ZBASICSTRING_HPP
#define _ZBASICSTRING_HPP

#include <ZSTL/ZArray.hpp>

#include <string.h>	//for strcmp, memset, memmove
#include <ctype.h>	//for isspace, tolower
#include <stdio.h>	//for sprintf
#include <stdarg.h>	//for va_start, va_end

//Null Terminator Definition
#define ZBASICSTRING_NULL_TERMINATOR ((char)'\0')

//Default buffer size for a ZBasicString
#ifndef ZBASICSTRING_DEFAULT_LOCAL_STORAGE
#define ZBASICSTRING_DEFAULT_LOCAL_STORAGE (128)
#endif

/*
Dynamic ASCII string implementation.

The template parameter N is the local storage size that will be passed along
to the contained ZArray.
*/
template <size_t N = ZBASICSTRING_DEFAULT_LOCAL_STORAGE>
class ZBasicString
{
protected:
	//Array used to hold the characters
	ZArray<char, N> StringArray;

	//Integrity check
	inline void CheckIntegrity()
	{
		#if ZSTL_CHECK_INTEGRITY

		ZSTL_ASSERT(StringArray.Size() > 0, "StringArray is empty with no null terminator!");
		ZSTL_ASSERT(StringArray[Length()] == ZBASICSTRING_NULL_TERMINATOR, "StringArray has no null terminator!");

		#endif //ZSTL_CHECK_INTEGRITY
	}

public:
	/*
	Used when an index is to be returned to indicate an invalid index.
	*/
	const static int InvalidPos = -1;

	//////////////////////
	/* Static Functions */
	//////////////////////

	/*
	public static ZBasicString<N>::BuildNumeric
	
	Builds a string from a double-precision float.  Has a practical length limit of 64 to avoid extra allocation.
	
	@param _number - the number to use
	@return (ZBasicString<N>) - the constructed string
	@context (all)
	*/
	static ZBasicString<N> BuildNumeric(double _number);

	/*
	public static ZBasicString<N>::BuildNumeric
	
	Builds a string from an integer.  Has a practical length limit of 64 to avoid extra allocation.
	
	@param _number - the number to use
	@return (ZBasicString<N>) - the constructed string
	*/
	static ZBasicString<N> BuildNumeric(int _number);

	/*
	public static ZBasicString<N>::BuildPrintf
	
	Constructs a ZBasicString<N> using printf-style formatted output.  Has a practical limit on the length of the 
	string of 4096 to avoid allocation.
	
	@param _fmt - the null-terminated printf format string
	@param  ... - the variable arguments for the format string
	@return (ZBasicString<N>) - the formatted string
	*/
	static ZBasicString<N> BuildPrintf(const char *_fmt, ...);

	/*
	public static ZBasicString<N>::BuildRepeat
	
	Constructs a ZBasicString<N> by repeating the given null-terminated string the given number of times.
	
	@param _string - the string to repeat
	@param _count - the number of times to repeat it
	@return (ZBasicString<N>) - the constructed string
	*/
	static ZBasicString<N> BuildRepeat(const char *_string, size_t _count);

	//////////////////////
	/* Member Functions */
	//////////////////////

	/*
	Default constructor.  Constructs an empty string.
	*/
	ZBasicString();

	/*
	Parameterized constructor.  This constructor will initialize a copy of 
	the provided null-terminated string.

	@param _string - the null-terminated string to initialize to
	*/
	ZBasicString(const char *_string);

	/*
	Parameterized constructor.  This constructor will initialize a copy of the provided
	string, but it will only copy in as many characters as defined.

	@param _string - the string to initialize to
	@param _count - the number of chars to copy when constructing the string
	*/
	ZBasicString(const char *_string, const size_t _count);

	/*
	Secondary constructor.  This constructor will initialize a copy of the provided
	null-terminated string.
	
	@param _string - the string to initialize to
	@param _allocator - the allocator to use for the string
	*/
	ZBasicString(const char *_string, ZArrayAllocator<char> *_allocator);

	/*
	Copy constructor.

	@param _other - the other string
	*/
	ZBasicString(const ZBasicString<N>& _other);

	/*
	Destructor.
	*/
	~ZBasicString();

	/*
	[] operator overload.  Allows indexing into the characters of this string.

	@param _index - integer index into the string
	@return (char&) - character at the given index
	*/
	char& operator [] (const int _index) const;

	/*
	+ operator overload.  Allows two strings to be added, appending one
	to the other.

	@param _other - the string to append to this string
	@return (ZBasicString<N>) - a string that is the concatenation of this string and the other
	*/
	ZBasicString<N> operator + (const ZBasicString<N>& _other) const;

	/*
	+ operator overload.  Allows a character to be added to this string.

	@param _char - the character to append to this string
	@return (ZBasicString<N>) - a string that is the concatenation of this string and the char
	*/
	ZBasicString<N> operator + (const char _char) const;

	/*
	+= operator overload.  Appends a copy of the second string to the first
	string.

	@param _other - the string to append to this string
	@return (ZBasicString<N>&) - this string
	*/
	ZBasicString<N>& operator += (const ZBasicString<N>& _other);
	
	/*
	+= operator overload.  Adds a character to the end of this string.

	@param _char - the char to append to this string
	@return (ZBasicString<N>&) - this string
	*/
	ZBasicString<N>& operator += (const char _char);

	/*
	= operator overload.  Makes this string equivalent to the other.

	@param _other - the string to set this equal to
	@return (ZBasicString<N>&) - this string
	*/
	ZBasicString<N>& operator = (const ZBasicString<N>& _other);

	/*
	== operator overload.  Determines if this string is equal to another.

	@param _other - the string to compare to
	@return (bool) - true if equal, false otherwise
	*/
	bool operator == (const ZBasicString<N>& _other) const;

	/*
	== operator overload.  Determines if this string is equal to
	a raw C string. This improves performance when comparing a
	ZBasicString to a string constant since no temporary ZBasicString needs
	to be constructed.

	@param _other - the string to compare to
	@return (bool) - true if equal, false otherwise
	*/
	bool operator == (const char* _other) const;

	/*
	< operator overload.  Lexographically compares strings.

	@param _other - the string to compare to
	@return (bool) - true if less than, false otherwise
	*/
	bool operator < (const ZBasicString<N>& _other) const;

	/*
	< operator overload.  Lexographically compares strings.

	@param _other - the string to compare to
	@return (bool) - true if less than, false otherwise
	*/
	bool operator <= (const ZBasicString<N>& _other) const;

	/*
	!= operator overload.  Determines if this string is not equal to another.

	@param _other - the string to compare to
	@return (bool) - true if not equal, false otherwise
	*/
	bool operator != (const ZBasicString<N>& _other) const;

	/*
	Hash code override.  Uses the Java 6 string hash function.

	@return (ZHashValue) - hash code for this string
	*/
	operator ZHashValue () const;

	/*
	public ZBasicString<N>::Append

	Append function, which appends the other string to this one.  Functionally
	equivalent to +=.

	@param _other - string to append
	@return (ZBasicString<N>&) - this string
	*/
	ZBasicString<N>& Append(const ZBasicString<N>& _other);

	/*
	public ZBasicString<N>::Clear

	Clears out the string to the empty string.

	@return (ZBasicString<N>&) - this string
	*/
	ZBasicString<N>& Clear();

	/*
	public ZBasicString<N>::Empty

	Determines if the string is the empty string.

	@return (bool) - true if the string is empty string, false otherwise
	*/
	bool Empty() const;

	/*
	public ZBasicString<N>::EndsWith

	Returns true if the string ends with the given substring.
	
	@param substr - the substring to check for 
	@return (bool) - true if this string ends with substr, false otherwise
	*/
	bool EndsWith(const ZBasicString<N>& substr) const;

	/*
	public ZBasicString<N>::Erase

	Erase function.  Erases the given character.

	@param _index - the index of the character to erase
	@return (ZBasicString<N>&) - this string
	*/
	ZBasicString<N>& Erase(const int _index);

	/*
	public ZBasicString<N>::Erase

	Erase function.  Erases characters between the given indices.

	@param _i - first index
	@param _j - second index (exclusive)
	@return (ZBasicString<N>&) - this string
	*/
	ZBasicString<N>& Erase(const int _i, const int _j);

	/*
	public ZBasicString<N>::IsNumeric

	Determines if this string represents a numeric quantity.

	@return (bool) - true if this string is numeric, false otherwise
	*/
	bool IsNumeric() const;

	/*
	public ZBasicString<N>::Find

	Finds the first occurrence of the specified substring after the given
	index.  Returns -1 if not found.

	@param _substring - the substring to look for
	@param _index - the index to start looking at
	@return (int) - integer index into the string where the match occured, ZBasicString<N>::InvalidPos if no match
	*/
	int Find(const ZBasicString<N>& _substring, const int _index = 0) const;

	/*
	public ZBasicString<N>::FirstOf

	Finds the first occurrence of a character in the specified substring.  Returns
	-1 if not found.

	@param _delims - characters to look for; order by expected frequency for best performance
	@param _index - index to start looking at
	@return (int) - index to the first of the specified characters, ZBasicString<N>::InvalidPos if not found
	*/
	int FirstOf(const ZBasicString<N>& _delims, const int _index = 0) const;

	/*
	public ZBasicString<N>::FirstNotOf

	Finds the first occurrence of a character that is not in the specified set of delimiters.
	Returns -1 if not found.

	@param _delims - the characters to avoid
	@param _index - the index to start looking at
	@return (int) - index to the first occurence of characters not in delims, ZBasicString<N>::InvalidPos if not found
	*/
	int FirstNotOf(const ZBasicString<N>& _delims, const int _index = 0) const;

	/*
	public ZBasicString<N>::Insert

	Insert function.  Inserts a string into this string.

	@param _index - index to insert at
	@param _other - string to insert
	@return (ZBasicString<N>&) - this string
	*/
	ZBasicString<N>& Insert(const int _index, const ZBasicString<N>& _other);

	/*
	public ZBasicString<N>::Length

	Gives the length of the string, not including the null terminator.
	
	@return (size_t) - length of the string
	*/
	size_t Length() const;

	/*
	public ZBasicString<N>::Pop

	Pops a character off the string.

	@return (char) - the character removed from the string
	*/
	char Pop();

	/*
	public ZBasicString<N>::Push

	Pushes a character onto the string.

	@param _char - character to push onto the string
	@return (ZBasicString<N>&) - this string
	*/
	ZBasicString<N>& Push(const char& _char);

	/*
	public ZBasicString<N>::Replace

	Replaces a section of the string with another.

	@param _i - starting location
	@param _j - ending location (exclusive)
	@param _str - string to replace with
	@return (ZBasicString<N>&) - this string
	*/
	ZBasicString<N>& Replace(const int _i, const int _j, const ZBasicString<N>& _str);

	/*
	public ZBasicString<N>::Slice

	Slice operation.  Returns a subset of the string given indices.

	@param _i - first index
	@param _j - second index (exclusive)
	@return (ZBasicString<N>) - subset of the string
	*/
	ZBasicString<N> Slice(const int _i, const int _j) const;

	/*
	public ZBasicString<N>::Swap

	Swap operation, which swaps string content.

	@param _other - string to swap contents with
	@return (ZBasicString<N>&) - this string
	*/
	ZBasicString<N>& Swap(ZBasicString<N>& _other);

	/*
	public ZBasicString<N>::Split

	Split operation, which splits the string into an array of strings based
	off a set of delimiters.  

	@param _delims - the delimiters to look for
	@return (ZArray<ZBasicString<N>>) - Array of strings
	*/
	ZArray< ZBasicString<N> > Split(const ZBasicString<N>& _delims) const;

	/*
	public ZBasicString<N>::Split

	Split operation, which splits the string into an array of strings based
	off a set of delimiters.  

	@param _delims - the delimiters to look for
	@param _count - maximum number of times to split
	@return (ZArray<ZBasicString<N>>) - Array of strings
	*/
	ZArray< ZBasicString<N> > Split(const ZBasicString<N>& _delims,  int _count) const;

	/*
	public ZBasicString<N>::StartsWith
	
	Returns true if the string starts with the given substring.
	
	@param substr - the substring to check for
	@return (bool) - true if it starts with substr, false otherwise
	*/
	bool StartsWith(const ZBasicString<N>& substr) const;

	/*
	public ZBasicString<N>::Strip

	Strip operation, which strips a character from the string (defaults to whitespace).

	@param _char - the character to strip from the string
	@return (ZBasicString<N>&) - this string
	*/
	ZBasicString<N>& Strip(const char _char = ' ');
	
	/*
	public ZBasicString<N>::ToArray

	Gets the string as an array of characters.
	
	@return (ZArray<char, N>) - ZArray of characters, including null terminator
	*/
	ZArray<char, N> ToArray();

	/*
	public ZBasicString<N>::ToCString

	Returns this string as a c-style string.

	@return (const char*) - a c-style string
	*/
	const char* ToCString() const;

	/*
	public ZBasicString<N>::ToDouble

	Gets the string as a floating point value.

	@return (double) - this string as a float value
	*/
	double ToDouble() const;

	/*
	public ZBasicString<N>::ToInt

	Gets the string as an integer value.

	@return (int) - this string as an integer
	*/
	int ToInt() const;

	/*
	public ZBasicString<N>::ToLower

	Gets a lowercase version of this string.

	@return (ZBasicString<N>) - this string as lowercase
	*/
	ZBasicString<N> ToLower() const;

	/*
	public ZBasicString<N>::ToUpper

	Gets an uppercase version of this string.

	@return (ZBasicString<N>) - this string as uppercase
	*/
	ZBasicString<N> ToUpper() const;

	/*
	public ZBasicString<N>::Tokenize

	Tokenize function, which returns the next token in this string up to 
	the delimiter, consuming the delimiter in the process.

	@param _delims - the delimiters to look for
	@return (ZBasicString<N>) - the token
	*/
	ZBasicString<N> Tokenize(const ZBasicString<N>& _delims);

	/*
	public ZBasicString<N>::Trim

	Gets a version of this string with whitespace trimmed off both sides.

	@return (ZBasicString<N>&) - this string with leading / tailing whitespace trimmed
	*/
	ZBasicString<N>& Trim();

	/*
	public ZBasicString<N>::TrimLeft
	
	Returns a copy of this string with whitespace trimmed off the left side.
	
	@return (ZBasicString<N>&) - a copy of the string with no left whitespace
	*/
	ZBasicString<N>& TrimLeft();

	/*
	public ZBasicString<N>::TrimRight
	
	Returns a copy of this string with whitespace trimmed off the right side.
	
	@return (ZBasicString<N>&) - a copy of the string with no right whitespace
	*/
	ZBasicString<N>& TrimRight();
};

///////////////////////////////////////
/* ZBasicString Non-Member Functions */
///////////////////////////////////////

/*
Non member addition function.  Allows adding a CString to a ZBasicString<N> 
with the CString as the right side of the operator.

@param _lhs - string to append to the left side
@param _rhs - char * to append to
@return - a string that is the concatenation of _lhs with _rhs
*/
template <size_t N>
ZBasicString<N> operator + (const ZBasicString<N>& _lhs, const char *_rhs)
{
	return _lhs + ZBasicString<N>(_rhs);
}

/*
Non member addition function.  Allows adding a ZBasicString<N> to a CString 
with the CString as the left side of the operator.

@param _lhs - char * to append to
@param _rhs - string to append to the left side
@return - a string that is the concatenation of _lhs with _rhs
*/
template <size_t N>
ZBasicString<N> operator + (const char *_lhs, const ZBasicString<N>& _rhs)
{
	return ZBasicString<N>(_lhs) + _rhs;
}

////////////////////////////////////////////////
/* ZBasicString Static Method Implementations */
////////////////////////////////////////////////

template <size_t N>
ZBasicString<N> ZBasicString<N>::BuildNumeric( double _number )
{
	char buffer[64];

	#ifdef _MSC_VER
	sprintf_s(buffer, sizeof(buffer), "%f", _number);
	#else
	sprintf(buffer, "%f", _number);
	#endif

	buffer[sizeof(buffer)-1] = ZBASICSTRING_NULL_TERMINATOR;

	ZBasicString<N> output(buffer);

	return output;
}

template <size_t N>
ZBasicString<N> ZBasicString<N>::BuildNumeric( int _number )
{
	char buffer[64];
	
	#ifdef _MSC_VER
	sprintf_s(buffer, sizeof(buffer), "%i", _number);
	#else
	sprintf(buffer, "%i", _number);
	#endif
	buffer[sizeof(buffer)-1] = ZBASICSTRING_NULL_TERMINATOR;

	ZBasicString<N> output(buffer);

	return output;
}

template <size_t N>
ZBasicString<N> ZBasicString<N>::BuildPrintf( const char* _fmt, ... )
{
	va_list args;
	char string[4096];

	//todo: necessary?
	//MemSetChar(string, 0, 4096);

	va_start(args, _fmt);

	#ifdef _MSC_VER
	vsnprintf_s(string, sizeof(string) - 1, sizeof(string) - 1, _fmt, args);
	#else
	vsnprintf(string, sizeof(string), _fmt, args);
	#endif


	va_end(args);

	//String is constructed
	ZBasicString<N> output(string);


	return output;
}

template <size_t N>
ZBasicString<N> ZBasicString<N>::BuildRepeat( const char *_string, size_t _count )
{
	size_t i;
	ZBasicString<N> output;

	for (i = 0; i < _count; i++)
		output += _string;

	return output;
}

/////////////////////////////////////////
/* ZBasicString Method Implementations */
/////////////////////////////////////////

template <size_t N>
ZBasicString<N>::ZBasicString()
: StringArray(ZBASICSTRING_DEFAULT_LOCAL_STORAGE)
{
	//Make sure we have our null terminator
	StringArray.Push(ZBASICSTRING_NULL_TERMINATOR);

	CheckIntegrity();
}

template <size_t N>
ZBasicString<N>::ZBasicString(const ZBasicString<N>& _other)
: StringArray(_other.StringArray)
{
	CheckIntegrity();
}

template <size_t N>
ZBasicString<N>::ZBasicString(const char *_string)
: StringArray( _string != NULL ? strlen(_string) + 1 : 1)
{
	//Set the string array size equal to the starting capacity
	StringArray.Resize(StringArray.Capacity());

	//Copy the string
	memmove((char*)StringArray, _string, Length() * sizeof(char));

	//Set our final null terminator
	StringArray[Length()] = ZBASICSTRING_NULL_TERMINATOR;

	//Internal integrity check
	CheckIntegrity();
}

template <size_t N>
ZBasicString<N>::ZBasicString(const char *_string, const size_t _count)
: StringArray(_count + 1)
{
	//Set the string array size equal to the starting capacity
	StringArray.Resize(StringArray.Capacity());

	//Copy the string
	memmove((char*)StringArray, _string, _count * sizeof(char));

	//Set our final null terminator
	StringArray[Length()] = ZBASICSTRING_NULL_TERMINATOR;

	//Internal integrity check
	CheckIntegrity();
}

template <size_t N>
ZBasicString<N>::ZBasicString(const char *_string, ZArrayAllocator<char> *_allocator)
: StringArray(strlen(_string) + 1, _allocator)
{
	//Set the string array size equal to the starting capacity
	StringArray.Resize(StringArray.Capacity());

	//Copy the string
	memmove((char*)StringArray, _string, Length() * sizeof(char));

	//Set our final null terminator
	StringArray[Length()] = ZBASICSTRING_NULL_TERMINATOR;

	//Internal integrity check
	CheckIntegrity();
}

template <size_t N>
ZBasicString<N>::~ZBasicString()
{
	//Internal integrity check
	CheckIntegrity();
}

template <size_t N>
char& ZBasicString<N>::operator [] (const int _index) const
{
	#if ZSTL_DISABLE_NEGATIVE_INDEXING

		#if !ZSTL_DISABLE_RUNTIME_CHECKS
		ZSTL_ASSERT(_index >= 0 && _index < (int)Length(), "ZBasicString out of bounds access!");
		#endif

	return StringArray[_index];

	#else //ZSTL_DISABLE_NEGATIVE_INDEXING

		#if !ZSTL_DISABLE_RUNTIME_CHECKS
		ZSTL_ASSERT(_index < (int)Length() && -1 * _index <= (int)Length(), "ZBasicString out of bounds access!");
		#endif

	if (_index < 0)
		return StringArray[_index - 1]; //Ensures -1 gives the last character, not the null terminator
	else
		return StringArray[_index];

	#endif
}

template <size_t N>
ZBasicString<N> ZBasicString<N>::operator + (const ZBasicString<N>& _other) const
{
	ZBasicString<N> ret(*this);

	//Append the other string to the return value
	ret += _other;

	return ret;
}

template <size_t N>
ZBasicString<N> ZBasicString<N>::operator + (const char _char) const
{
	ZBasicString<N> ret(*this);

	//Push the character to the return value
	ret.Push(_char);

	return ret;
}	

template <size_t N>
ZBasicString<N>& ZBasicString<N>::operator += (const ZBasicString<N>& _other)
{
	int index = Length();

	//Make sure we have enough room for this string, the other string, and the null terminator
	StringArray.Resize(Length() + _other.Length() + 1);

	//Copy the data
	memmove( &((char*)StringArray)[index], (char*)_other.StringArray, _other.StringArray.Size() * sizeof(char) );

	//Internal integrity check
	CheckIntegrity();

	return *this;
}

template <size_t N>
ZBasicString<N>& ZBasicString<N>::operator += (const char _char)
{
	//Just push on the new character
	return Push(_char);
}

template <size_t N>
ZBasicString<N>& ZBasicString<N>::operator = (const ZBasicString<N>& _other)
{
	//Make sure we have enough room for the other string and the null terminator
	StringArray.Resize(_other.Length() + 1);

	//Copy the string
	memmove( (char*)StringArray, (char*)_other.StringArray, (_other.Length() + 1) * sizeof(char));

	//Internal integrity check
	CheckIntegrity();

	return *this;
}

template <size_t N>
bool ZBasicString<N>::operator == (const ZBasicString<N>& _other) const
{
	//Perform a string comparison
	return strcmp((char*)StringArray, (char*)_other.StringArray) == 0;
}

template <size_t N>
bool ZBasicString<N>::operator == (const char* _other) const
{
	//Perform a string comparison
	return strcmp((char*)StringArray, _other) == 0;
}

template <size_t N>
bool ZBasicString<N>::operator < (const ZBasicString<N>& _other) const
{
	//Use the result of strcmp
	return strcmp((char*)StringArray, (char*)_other.StringArray) < 0;
}

template <size_t N>
bool ZBasicString<N>::operator <= (const ZBasicString<N>& _other) const
{
	//Use the result of strcmp
	return strcmp((char*)StringArray, (char*)_other.StringArray) <= 0;
}

template <size_t N>
bool ZBasicString<N>::operator != (const ZBasicString<N>& _other) const
{
	//Make this dependent on ==
	return !(*this == _other);
}

template <size_t N>
ZBasicString<N>::operator ZHashValue() const
{
	//This whole thing is basically the java 6 string hash function
	size_t i;
	ZHashValue hash;

	for (hash = 0, i = 0; i < Length(); i++) 
		hash = (hash << 5) - hash + ((char*)StringArray)[i];

	return hash;
}

template <size_t N>
ZBasicString<N>& ZBasicString<N>::Append(const ZBasicString<N>& _other)
{
	//Append the other
	return *this += _other;
}

template <size_t N>
ZBasicString<N>& ZBasicString<N>::Clear()
{
	//Clear out the string array
	StringArray.Clear();

	//Push on the null terminator
	StringArray.Push(ZBASICSTRING_NULL_TERMINATOR);

	//Internal integrity check
	CheckIntegrity();

	return *this;
}

template <size_t N>
bool ZBasicString<N>::EndsWith( const ZBasicString<N>& substr ) const
{
	//Defer to Find
	return Find(substr) == (int)(Length() - substr.Length());
}

template <size_t N>
bool ZBasicString<N>::Empty() const
{
	//Ensure the first character is a null terminator
	return ((char*)StringArray)[0] == ZBASICSTRING_NULL_TERMINATOR;
}

template <size_t N>
ZBasicString<N>& ZBasicString<N>::Erase(const int _index)
{
	//Defer to the next Erase method
	return Erase(_index, _index + 1);
}

template <size_t N>
ZBasicString<N>& ZBasicString<N>::Erase(const int _i, const int _j)
{
	//Make sure we're within bounds and not including the null terminator
	#if !ZSTL_DISABLE_RUNTIME_CHECKS
	ZSTL_ASSERT(StringArray.AbsoluteIndex(_i) <= StringArray.AbsoluteIndex(_j) && 
					StringArray.AbsoluteIndex(_j) <= Length(), 
					"Invalid indices to string erase!");
	#endif

	//Call ZArray::Erase
	StringArray.Erase(_i, _j);

	//Internal integrity check
	CheckIntegrity();

	return *this;
}

template <size_t N>
int ZBasicString<N>::Find(const ZBasicString<N>& _substring, const int _index) const
{
	int i, j;

	//Basic edge case check
	if (_substring.Empty() || Empty())
		return ZBasicString<N>::InvalidPos;

	//Call ZArray::Find, looking for the first character
	i = StringArray.Find(_substring[0], _index);

	//If we've found it
	while (i != ZArray<char, N>::InvalidPos)
	{
		//If we found a match, check for the whole substring
		for (j = 1; j < (int)_substring.Length() && i + j < (int)Length(); j++)
		{
			if (StringArray[i + j] != _substring[j])
				break;
		}

		if (j == (int)_substring.Length())
			return i;

		i = StringArray.Find(_substring[0], i + 1);
	}

	return (int)ZBasicString<N>::InvalidPos;
}

template <size_t N>
int ZBasicString<N>::FirstOf(const ZBasicString<N>& _delims, const int _index) const
{
	int i, j;

	//Iterate, looking for the first occurence of any of the delimiters
	for (i = _index; i < (int)Length(); i++)
	{
		//Check each location for the delimiters
		for (j = 0; j < (int)_delims.Length(); j++)
		{
			if ( ((char*)StringArray)[i] == ((char*)_delims.StringArray)[j] )
				return i;
		}
	}

	return ZBasicString<N>::InvalidPos;
}

template <size_t N>
int ZBasicString<N>::FirstNotOf(const ZBasicString<N>& _delims, const int _index) const
{
	int i, j;
	bool noMatch;

	//Iterate, looking for the first occurrence that is not one of the delimiters
	for (i = _index; i < (int)Length(); i++)
	{
		//Assume we have no match, and then start looking at each character for each of the delimiters
		for (j = 0, noMatch = true; j < (int)_delims.Length() && noMatch != false; j++)
		{
			noMatch = noMatch && ( ((char*)StringArray)[i] != ((char*)_delims.StringArray)[j] );
		}

		//If this location was not a match for one of the delimiters, return it
		if (noMatch)
			return i;
	}

	return ZBasicString<N>::InvalidPos;
}

template <size_t N>
ZBasicString<N>& ZBasicString<N>::Insert(const int _index, const ZBasicString<N>& _other)
{
	ZBasicString<N> str(_other);

	//Remove the null terminators from both srings
	str.StringArray.Pop();
	StringArray.Pop();

	//Insert the other string
	StringArray.Insert(_index, str.StringArray);

	//Push on the null terminator
	StringArray.Push(ZBASICSTRING_NULL_TERMINATOR);

	//Needed so CheckIntegrity doesn't fail
	str.StringArray.Push(ZBASICSTRING_NULL_TERMINATOR);

	//Internal integrity check
	CheckIntegrity();

	return *this;
}

template <size_t N>
bool ZBasicString<N>::IsNumeric() const
{
	bool digitSeen = false;
	bool decimalPointSeen = false;
	const char* s;

	//Basic easy out
	if (Empty())
		return false;

	s  = (const char*)StringArray;

	//Ensure we see [-]?[0-9]+[.]?[0-9]*
	// *  := 0 or more times
	// +  := 1 or more times
	// ?  := 0 or 1 times


	//May begin with '-' or digit.
	if(s[0] != '-')
	{
		if(!isdigit(s[0]))
			return false;
		digitSeen = true;
	}


	/*
		Algorithm: Since we know we've either seen a digit or negative sign,
		scan for more digits and an optional decimal point, allowing only
		a single decimal point. If we terminate the loop before seeing a single
		digit, and we didn't see one before checking for the negative sign, then
		return false.
	*/
	
	for(size_t i=1; i<Length(); i++)
	{
		//Not a digit?
		if(!isdigit(s[i]))
		{
			//Only allow 1 decimal point
			if(!decimalPointSeen && s[i] == '.')
			{
				decimalPointSeen = true;
				continue;
			}

			//Not numeric
			return false;
		}
		else
			digitSeen = true;
	}

	//Must have seen at least one digit for this to be considered numeric.
	return digitSeen;
}

template <size_t N>
size_t ZBasicString<N>::Length() const
{
	//We don't count the null terminator
	return StringArray.Size() - 1; 
}

template <size_t N>
char ZBasicString<N>::Pop()
{
	char ret;

	#if !ZSTL_DISABLE_RUNTIME_CHECKS
	ZSTL_ASSERT(!Empty(), "Cannot pop character from empty ZBasicString!");
	#endif

	//Remove the null terminator
	StringArray.Pop();

	//Get the ending character
	ret = StringArray.Pop();

	//Add back on the null terminator
	StringArray.Push( ZBASICSTRING_NULL_TERMINATOR );

	//Internal integrity check
	CheckIntegrity();

	return ret;
}

template <size_t N>
ZBasicString<N>& ZBasicString<N>::Push(const char& _char)
{
	//Add the character where the null terminator resides
	StringArray[StringArray.Size() - 1] = _char;

	//Add on the new null terminator
	StringArray.Push(ZBASICSTRING_NULL_TERMINATOR);

	//Internal integrity check
	CheckIntegrity();

	return *this;
}

template <size_t N>
ZBasicString<N>& ZBasicString<N>::Replace(const int _i, const int _j, const ZBasicString<N>& _str)
{
	//Erase using the array method
	StringArray.Erase(_i, _j);

	//Insert the provided string
	Insert(_i, _str);

	//Internal integrity check
	CheckIntegrity();

	return *this;
}

template <size_t N>
ZBasicString<N> ZBasicString<N>::Slice(const int _i, const int _j) const
{
	if (_i == _j)
		return ZBasicString<N>();

	//Some sanity checks
	#if !ZSTL_DISABLE_RUNTIME_CHECKS
	ZSTL_ASSERT(_i < _j, "Invalid parameters passed to ZBasicString::Slice! (i >= j)");
	ZSTL_ASSERT(_i < (int)Length(), "Invalid first parameter to ZBasicString::Slice! (_i >= Length)");
	ZSTL_ASSERT(_j <= (int)Length(), "Invalid second parameter to ZBasicString::Slice! (_j > Length)");
	#endif

	//Construct a new array using the slice operator
	ZArray<char, N> str(StringArray.Slice(_i, _j));

	//If the slice is empty or we don't end up with a null terminator
	if (str.Size() == 0 || str[str.Size() - 1] != ZBASICSTRING_NULL_TERMINATOR)
		str.Push(ZBASICSTRING_NULL_TERMINATOR);

	return ZBasicString<N>( (char*)str );
}

template <size_t N>
ZArray< ZBasicString<N> > ZBasicString<N>::Split(const ZBasicString<N>& _delims) const
{
	//Copy this string for tokenization
	ZBasicString<N> str(*this);
	ZArray< ZBasicString<N> > strings;

	//While the string copy still has characters
	while (!str.Empty())
	{
		//Tokenize with the provided delimiters
		strings.Push(str.Tokenize(_delims));

		//Pop off any empty strings
		if (strings[strings.Size() - 1].Empty())
			strings.Pop();
	}

	return strings;
}

template <size_t N>
ZArray< ZBasicString<N> > ZBasicString<N>::Split(const ZBasicString<N>& _delims, int _count) const
{
	//Copy this string for tokenization
	ZBasicString<N> str(*this);
	ZArray< ZBasicString<N> > strings;
	int currentCount;

	//We start with nothing
	currentCount = 0;

	//While the string copy still has characters
	while (!str.Empty())
	{
		//When we have enough, stop
		if (currentCount > _count)
			break;

		//Tokenize with the provided delimiters
		strings.Push(str.Tokenize(_delims));

		//Pop off any empty string and increment count for actual strings
		if (strings[strings.Size() - 1].Empty())
			strings.Pop();
		else
			currentCount++;
	}

	return strings;
}

template <size_t N>
bool ZBasicString<N>::StartsWith( const ZBasicString<N>& substr ) const
{
	//If we find it at the beginning, it starts with it
	return Find(substr) == 0;
}

template <size_t N>
ZBasicString<N>& ZBasicString<N>::Strip(const char _char)
{
	int i;

	//Iterate the string and erase characters that match
	for (i = 0; i < (int)Length(); i++)
	{
		if ( ((char*)StringArray)[i] == _char )
		{
			Erase(i, i + 1);
			i--;
		}
	}

	//Internal integrity check
	CheckIntegrity();

	return *this;
}

template <size_t N>
ZBasicString<N>& ZBasicString<N>::Swap(ZBasicString<N>& _other)
{
	//Swap our contained array contents
	StringArray.Swap(_other.StringArray);

	//Internal integrity check
	CheckIntegrity();

	return *this;
}

template <size_t N>
ZArray<char, N> ZBasicString<N>::ToArray()
{
	//Return a copy of the backed array
	return StringArray;
}

template <size_t N>
const char* ZBasicString<N>::ToCString() const
{
	//Return the string array that backs it
	return (char*)StringArray;
}

template <size_t N>
double ZBasicString<N>::ToDouble() const
{
	//Defer to atof
	return atof((char*)StringArray);
}

template <size_t N>
int ZBasicString<N>::ToInt() const
{
	//Defer to atoi
	return atoi((char*)StringArray);
}

template <size_t N>
ZBasicString<N> ZBasicString<N>::ToLower() const
{
	int i;
	ZBasicString<N> str(*this);

	//use tolower on all characters
	for (i = 0; i < (int)str.Length(); i++)
		((char*)str.StringArray)[i] = (char) tolower( ((char*)str.StringArray)[i] );

	return str;
}

template <size_t N>
ZBasicString<N> ZBasicString<N>::ToUpper() const
{
	int i;
	ZBasicString<N> str(*this);

	//Use toupper on all characters
	for (i = 0; i < (int)str.Length(); i++)
		((char*)str.StringArray)[i] = (char) toupper( ((char*)str.StringArray)[i] );

	return str;

}

template <size_t N>
ZBasicString<N> ZBasicString<N>::Tokenize(const ZBasicString<N>& _delims)
{
	int i, j;
	ZBasicString<N> token;

	//Find the first element that is nto one of the delimiters
	i = FirstNotOf(_delims, 0);

	//Make sure we dont' consist entirely of delimiters
	if (i == ZBasicString<N>::InvalidPos)
	{
		Clear();
		return token;
	}

	//Find the first after that index that is one of the delimiters
	j = FirstOf(_delims, i);

	//If that is off the end of the array
	if (j == ZBasicString<N>::InvalidPos)
	{
		//Just slice from here to the end
		token = Slice(i, Length());
		Clear();
		return token;
	}

	//Grab that token
	token = Slice(i, j);

	//Erase that token from this string
	Erase(i, j);

	//Internal integrity check
	CheckIntegrity();

	//Return the token
	return token;
}

template <size_t N>
ZBasicString<N>& ZBasicString<N>::Trim()
{
	ZBasicString<N>& str = *this;

	//Just trim both left and right on this string
	return str.TrimLeft().TrimRight();
}

template <size_t N>
ZBasicString<N>& ZBasicString<N>::TrimLeft()
{
	int i;
	ZBasicString<N>& str = *this;

	//Remove any character that is a space character as determined by isspace
	for (i = 0; i < (int)str.Length(); i++) 
	{
		if (!isspace( ((char*)str.StringArray)[i] )) 
			break;
	}

	//If we are all spaces, just clear and return this string
	if (i == (int)str.Length()) 
	{
		str.Clear();
		return str;
	} 
	else 
	{
		str.Erase(0, i);
	}

	//Internal integrity check
	CheckIntegrity();

	return str;
}

template <size_t N>
ZBasicString<N>& ZBasicString<N>::TrimRight()
{
	int i;
	ZBasicString<N>& str = *this;

	//Remove any character on the right that is whitespace as determined by isspace
	for (i = Length() - 1; ; i--) 
	{
		if (!isspace( ((char*)str.StringArray)[i] )) 
		{
			str.Erase(i + 1, str.Length());
			break;
		}

		//If we went all the way to the beginning, clear the string
		if (i == 0) 
		{
			str.Clear();
			break;
		}
	}

	//Internal integrity check
	CheckIntegrity();

	return str;
}

#endif
