// TODO: merge_mmap algo will go into this class
// Copyright (c) 2008 James Molloy, Jörg Pfähler, Matthew Iselin

#pragma once

#include "list.h"

/*!
 * This class manages a list of ranges. It automatically merges adjacent entries in the list.
 *
 * @param[in] type_t the integer type the range start and length are encoded in.
 */
template<typename type_t>
class range_list_t
{
public:
    inline range_list_t() : ranges() {}

    /*!
     * Construct with a preexisting range.
     * @param[in] start  beginning of the range.
     * @param[in] length length of the range.
     */
    range_list_t(type_t start, type_t length)
        : ranges()
    {
        ranges.push_back(new range_t(start, length));
    }

    /*! Destructor frees the list. */
    ~range_list_t();

    range_list_t(const range_list_t&);

    class range_t
    {
    public:
        inline range_t(type_t start_, type_t length_) : start(start_), length(length_) {}
        type_t start, length;
    };

    /*!
     * Allocate a range of a specific size.
     * @param[in] length the requested length
     * @param[in,out] address the beginning address of the allocated range
     * @returns true, if successfully allocated (and address is valid), false otherwise
     */
    bool allocate(type_t length, type_t* address);

    /*!
     * Allocate a range of specific size and beginning address.
     * @param[in] length the length
     * @param[in] address the beginning address
     * @return true, if successfully allocated, false otherwise
     */
    bool allocate(type_t length, type_t address);

    /*!
     * Allocate a range of specific size and beginning address, with overlapping allowed.
     * @param[in] length the length
     * @param[in] address the beginning address
     * @return true, if successfully allocated at least one part of the range, false otherwise
     */
    void allocate_overlapping(type_t length, type_t address);

    /*!
     * Free a range.
     * @param[in] address beginning address of the range
     * @param[in] length length of the range
     */
    void free(type_t length, type_t address);

    void clear();

    /*!
     * Get the number of ranges in the list.
     * @return the number of ranges in the list.
     */
    inline size_t size() const { return ranges.size(); }

private:
    list_t<range_t*> ranges;
    range_list_t& operator =(const range_list_t& other);
};
