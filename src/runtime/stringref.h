//===--- stringref_t.h - Constant String Reference Wrapper --------*- C++ -*-===//
// Taken from 
//                     The LLVM Compiler Infrastructure

#pragma once

#include "types.h"

/// stringref_t - Represent a constant reference to a string, i.e. a character
/// array and a length, which need not be null terminated.
///
/// This class does not own the string data, it is expected to be used in
/// situations where the character data resides in some other buffer, whose
/// lifetime extends past that of the stringref_t. For this reason, it is not in
/// general safe to store a stringref_t.
class stringref_t {
public:
  typedef const char *iterator;
  typedef const char *const_iterator;
  static const size_t npos = ~size_t(0);
  typedef size_t size_type;

private:
  /// The start of the string, in an external buffer.
  const char *Data;

  /// The length of the string.
  size_t Length;

  // Workaround memcmp issue with null pointers (undefined behavior)
  // by providing a specialized version
  static int compareMemory(const char *Lhs, const char *Rhs, size_t Length) {
    if (Length == 0) { return 0; }
    return memutils::memory_difference(Lhs,Rhs,Length);
  }
  
public:
  /// @name Constructors
  /// @{

  /// Construct an empty string ref.
  /*implicit*/ stringref_t() : Data(0), Length(0) {}

  /// Construct a string ref from a cstring.
  /*implicit*/ stringref_t(const char *Str)
    : Data(Str) {
      Length = memutils::string_length(Str);
    }

  /// Construct a string ref from a pointer and length.
  /*implicit*/ stringref_t(const char *data, size_t length)
    : Data(data), Length(length) {
      assert((data || length == 0) &&
      "stringref_t cannot be built from a nullptr argument with non-null length");
    }

  /// @}
  /// @name Iterators
  /// @{

  iterator begin() const { return Data; }

  iterator end() const { return Data + Length; }

  /// @}
  /// @name String Operations
  /// @{

  /// data - Get a pointer to the start of the string (which may not be null
  /// terminated).
  const char *data() const { return Data; }

  /// empty - Check if the string is empty.
  bool empty() const { return Length == 0; }

  /// size - Get the string size.
  size_t size() const { return Length; }

  /// front - Get the first character in the string.
  char front() const {
    assert(!empty());
    return Data[0];
  }

  /// back - Get the last character in the string.
  char back() const {
    assert(!empty());
    return Data[Length-1];
  }

  /// equals - Check for string equality, this is more efficient than
  /// compare() when the relative ordering of inequal strings isn't needed.
  bool equals(stringref_t RHS) const {
    return (Length == RHS.Length &&
            compareMemory(Data, RHS.Data, RHS.Length) == 0);
  }

  /// compare - Compare two strings; the result is -1, 0, or 1 if this string
  /// is lexicographically less than, equal to, or greater than the \arg RHS.
  int compare(stringref_t RHS) const {
    // Check the prefix for a mismatch.
    if (int Res = compareMemory(Data, RHS.Data, std::min(Length, RHS.Length)))
      return Res < 0 ? -1 : 1;

    // Otherwise the prefixes match, so we only need to check the lengths.
    if (Length == RHS.Length)
      return 0;
    return Length < RHS.Length ? -1 : 1;
  }

  /// @}
  /// @name Operator Overloads
  /// @{

  char operator[](size_t Index) const {
    assert(Index < Length && "Invalid index!");
    return Data[Index];
  }

  /// @}
  /// @name String Predicates
  /// @{

  /// startswith - Check if this string starts with the given \arg Prefix.
  bool startswith(stringref_t Prefix) const {
    return Length >= Prefix.Length &&
           compareMemory(Data, Prefix.Data, Prefix.Length) == 0;
  }

  /// endswith - Check if this string ends with the given \arg Suffix.
  bool endswith(stringref_t Suffix) const {
    return Length >= Suffix.Length &&
      compareMemory(end() - Suffix.Length, Suffix.Data, Suffix.Length) == 0;
  }

  /// @}
  /// @name String Searching
  /// @{

  /// find - Search for the first character \arg C in the string.
  ///
  /// \return - The index of the first occurrence of \arg C, or npos if not
  /// found.
  size_t find(char C, size_t From = 0) const {
    for (size_t i = std::min(From, Length), e = Length; i != e; ++i)
      if (Data[i] == C)
        return i;
    return npos;
  }

  /// find - Search for the first string \arg Str in the string.
  ///
  /// \return - The index of the first occurrence of \arg Str, or npos if not
  /// found.
  size_t find(stringref_t Str, size_t From = 0) const;

  /// rfind - Search for the last character \arg C in the string.
  ///
  /// \return - The index of the last occurrence of \arg C, or npos if not
  /// found.
  size_t rfind(char C, size_t From = npos) const {
    From = std::min(From, Length);
    size_t i = From;
    while (i != 0) {
      --i;
      if (Data[i] == C)
        return i;
    }
    return npos;
  }

  /// rfind - Search for the last string \arg Str in the string.
  ///
  /// \return - The index of the last occurrence of \arg Str, or npos if not
  /// found.
  size_t rfind(stringref_t Str) const;

  /// find_first_of - Find the first character in the string that is \arg C,
  /// or npos if not found. Same as find.
  size_type find_first_of(char C, size_t From = 0) const {
    return find(C, From);
  }

  /// find_first_of - Find the first character in the string that is in \arg
  /// Chars, or npos if not found.
  ///
  /// Note: O(size() + Chars.size())
  size_type find_first_of(stringref_t Chars, size_t From = 0) const;

  /// find_first_not_of - Find the first character in the string that is not
  /// \arg C or npos if not found.
  size_type find_first_not_of(char C, size_t From = 0) const;

  /// find_first_not_of - Find the first character in the string that is not
  /// in the string \arg Chars, or npos if not found.
  ///
  /// Note: O(size() + Chars.size())
  size_type find_first_not_of(stringref_t Chars, size_t From = 0) const;

  /// find_last_of - Find the last character in the string that is \arg C, or
  /// npos if not found.
  size_type find_last_of(char C, size_t From = npos) const {
    return rfind(C, From);
  }

  /// find_last_of - Find the last character in the string that is in \arg C,
  /// or npos if not found.
  ///
  /// Note: O(size() + Chars.size())
  size_type find_last_of(stringref_t Chars, size_t From = npos) const;

  /// find_last_not_of - Find the last character in the string that is not
  /// \arg C, or npos if not found.
  size_type find_last_not_of(char C, size_t From = npos) const;

  /// find_last_not_of - Find the last character in the string that is not in
  /// \arg Chars, or npos if not found.
  ///
  /// Note: O(size() + Chars.size())
  size_type find_last_not_of(stringref_t Chars, size_t From = npos) const;

  /// @}
  /// @name Helpful Algorithms
  /// @{

  /// count - Return the number of occurrences of \arg C in the string.
  size_t count(char C) const {
    size_t Count = 0;
    for (size_t i = 0, e = Length; i != e; ++i)
      if (Data[i] == C)
        ++Count;
    return Count;
  }

  /// count - Return the number of non-overlapped occurrences of \arg Str in
  /// the string.
  size_t count(stringref_t Str) const;

  /// @}
  /// @name Substring Operations
  /// @{

  /// substr - Return a reference to the substring from [Start, Start + N).
  ///
  /// \param Start - The index of the starting character in the substring; if
  /// the index is npos or greater than the length of the string then the
  /// empty substring will be returned.
  ///
  /// \param N - The number of characters to included in the substring. If N
  /// exceeds the number of characters remaining in the string, the string
  /// suffix (starting with \arg Start) will be returned.
  stringref_t substr(size_t Start, size_t N = npos) const {
    Start = std::min(Start, Length);
    return stringref_t(Data + Start, std::min(N, Length - Start));
  }
  
  /// drop_front - Return a stringref_t equal to 'this' but with the first
  /// elements dropped.
  stringref_t drop_front(unsigned N = 1) const {
    assert(size() >= N && "Dropping more elements than exist");
    return substr(N);
  }

  /// drop_back - Return a stringref_t equal to 'this' but with the last
  /// elements dropped.
  stringref_t drop_back(unsigned N = 1) const {
    assert(size() >= N && "Dropping more elements than exist");
    return substr(0, size()-N);
  }

  /// slice - Return a reference to the substring from [Start, End).
  ///
  /// \param Start - The index of the starting character in the substring; if
  /// the index is npos or greater than the length of the string then the
  /// empty substring will be returned.
  ///
  /// \param End - The index following the last character to include in the
  /// substring. If this is npos, or less than \arg Start, or exceeds the
  /// number of characters remaining in the string, the string suffix
  /// (starting with \arg Start) will be returned.
  stringref_t slice(size_t Start, size_t End) const {
    Start = std::min(Start, Length);
    End = std::min(std::max(Start, End), Length);
    return stringref_t(Data + Start, End - Start);
  }

  /// split - Split into two substrings around the first occurrence of a
  /// separator character.
  ///
  /// If \arg Separator is in the string, then the result is a pair (LHS, RHS)
  /// such that (*this == LHS + Separator + RHS) is true and RHS is
  /// maximal. If \arg Separator is not in the string, then the result is a
  /// pair (LHS, RHS) where (*this == LHS) and (RHS == "").
  ///
  /// \param Separator - The character to split on.
  /// \return - The split substrings.
  std::pair<stringref_t, stringref_t> split(char Separator) const {
    size_t Idx = find(Separator);
    if (Idx == npos)
      return std::make_pair(*this, stringref_t());
    return std::make_pair(slice(0, Idx), slice(Idx+1, npos));
  }

  /// split - Split into two substrings around the first occurrence of a
  /// separator string.
  ///
  /// If \arg Separator is in the string, then the result is a pair (LHS, RHS)
  /// such that (*this == LHS + Separator + RHS) is true and RHS is
  /// maximal. If \arg Separator is not in the string, then the result is a
  /// pair (LHS, RHS) where (*this == LHS) and (RHS == "").
  ///
  /// \param Separator - The string to split on.
  /// \return - The split substrings.
  std::pair<stringref_t, stringref_t> split(stringref_t Separator) const {
    size_t Idx = find(Separator);
    if (Idx == npos)
      return std::make_pair(*this, stringref_t());
    return std::make_pair(slice(0, Idx), slice(Idx + Separator.size(), npos));
  }

  /// rsplit - Split into two substrings around the last occurrence of a
  /// separator character.
  ///
  /// If \arg Separator is in the string, then the result is a pair (LHS, RHS)
  /// such that (*this == LHS + Separator + RHS) is true and RHS is
  /// minimal. If \arg Separator is not in the string, then the result is a
  /// pair (LHS, RHS) where (*this == LHS) and (RHS == "").
  ///
  /// \param Separator - The character to split on.
  /// \return - The split substrings.
  std::pair<stringref_t, stringref_t> rsplit(char Separator) const {
    size_t Idx = rfind(Separator);
    if (Idx == npos)
      return std::make_pair(*this, stringref_t());
    return std::make_pair(slice(0, Idx), slice(Idx+1, npos));
  }

  /// ltrim - Return string with consecutive characters in \arg Chars starting
  /// from the left removed.
  stringref_t ltrim(stringref_t Chars = " \t\n\v\f\r") const {
    return drop_front(std::min(Length, find_first_not_of(Chars)));
  }

  /// rtrim - Return string with consecutive characters in \arg Chars starting
  /// from the right removed.
  stringref_t rtrim(stringref_t Chars = " \t\n\v\f\r") const {
    return drop_back(Length - std::min(Length, find_last_not_of(Chars) + 1));
  }

  /// trim - Return string with consecutive characters in \arg Chars starting
  /// from the left and right removed.
  stringref_t trim(stringref_t Chars = " \t\n\v\f\r") const {
    return ltrim(Chars).rtrim(Chars);
  }

  /// @}
};

/// @name stringref_t Comparison Operators
/// @{

inline bool operator==(stringref_t LHS, stringref_t RHS) {
  return LHS.equals(RHS);
}

inline bool operator!=(stringref_t LHS, stringref_t RHS) {
  return !(LHS == RHS);
}

inline bool operator<(stringref_t LHS, stringref_t RHS) {
  return LHS.compare(RHS) == -1;
}

inline bool operator<=(stringref_t LHS, stringref_t RHS) {
  return LHS.compare(RHS) != 1;
}

inline bool operator>(stringref_t LHS, stringref_t RHS) {
  return LHS.compare(RHS) == 1;
}

inline bool operator>=(stringref_t LHS, stringref_t RHS) {
  return LHS.compare(RHS) != -1;
}

// inline std::string &operator+=(std::string &buffer, llvm::stringref_t string) {
//   return buffer.append(string.data(), string.size());
// }

/// @}
