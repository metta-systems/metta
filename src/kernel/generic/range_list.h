//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2008 James Molloy, Jörg Pfähler, Matthew Iselin
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "default_console.h"
#include "logger.h"

/**
 * This class manages a list of ranges. It automatically merges adjacent entries in the list.
 *
 * @param[in] type_t integer type the range start and length are encoded with.
 */
template <typename type_t>
class range_list_t
{
public:
    // template <typename _Extra_t>// todo: add extra data that goes with the range
    class range_t
    {
        type_t d_start, d_length;
    public:
        range_t() : d_start(0), d_length(0) {}
        inline range_t(type_t start, type_t length) : d_start(start), d_length(length) {}
        inline void set(type_t start, type_t length)
        {
            logger::trace() << "range_t::set(" << start << ", " << length << ")";
            d_start = start;
            d_length = length;
        }
        inline void reset() { d_start = 0; d_length = 0; }
        type_t start() { return d_start; }
        type_t size()  { return d_length; }
        type_t end()   { return d_start + d_length; }
    };

    // typedef std::list<range_t*> list_type;
    // typedef typename list_type::iterator iterator;
    // typedef typename list_type::const_iterator const_iterator;
    //
    // iterator begin() { return ranges.begin(); }
    // iterator end()   { return ranges.end(); }
    // const_iterator begin() const { return ranges.begin(); }
    // const_iterator end()   const { return ranges.end(); }
    //
    // inline range_list_t() : ranges() {}

    /**
     * Construct with a preexisting range.
     * @param[in] start  beginning of the range.
     * @param[in] length length of the range.
     */
    // inline range_list_t(type_t start, type_t length)
    //     : ranges()
    // {
    //     ranges.push_back(new range_t(start, length));
    // }

    /** Deep-copy constructor. */
//     range_list_t(const range_list_t& other)
//         : ranges()
//     {
//         clear();
//         const_iterator it(other.ranges.begin());
//         for (; it != other.ranges.end(); ++it)
//         {
//             range_t* new_range = new range_t((*it)->start, (*it)->length);
//             ranges.push_back(new_range);
//         }
// /*        foreach(auto r, other.ranges)
//             ranges.push_back(new range_t(r->start, r->length));*/
//     }

    /** Deep-copy assignment operator. */
//     range_list_t& operator =(const range_list_t& other)
//     {
//         clear();
//         const_iterator it(other.ranges.begin());
//         for (; it != other.ranges.end(); ++it)
//         {
//             range_t* new_range = new range_t((*it)->start, (*it)->length);
//             ranges.push_back(new_range);
//         }
// /*        foreach(auto r, other.ranges)
//             ranges.push_back(new range_t(r->start, r->length));*/
//         return *this;
//     }

    /** Destructor frees the list. */
    // ~range_list_t() { clear(); }

    /**
     * Allocate a range of a specific size. Pick a first-fit region.
     * @param[in,out] start The beginning address of the allocated range.
     * @param[in] length The requested length.
     * @returns true, if successfully allocated (and address is valid), false otherwise.
     */
    // bool allocate(type_t* start, type_t length)
    // {
    //     ASSERT(start);
    //     iterator cur(ranges.begin());
    //     const_iterator end(ranges.end());
    //     for (; cur != end; ++cur)
    //     {
    //         if ((*cur)->length >= length)
    //         {
    //             *start = (*cur)->start;
    //             (*cur)->start += length;
    //             (*cur)->length -= length;
    //             if ((*cur)->length == 0)
    //             {
    //                 delete *cur;
    //                 ranges.erase(cur);
    //             }
    //             return true;
    //         }
    //     }
    //     return false;
    // }

    /**
     * Allocate a range of specific size and beginning address.
     * @param[in] start The beginning address.
     * @param[in] length The length.
     * @return true, if successfully allocated, false otherwise.
     */
    // bool allocate(type_t start, type_t length)
    // {
    //     iterator cur(ranges.begin());
    //     const_iterator end(ranges.end());
    //     for (; cur != end; ++cur)
    //     {
    //         // exact match
    //         if ((*cur)->start == start && (*cur)->length == length)
    //         {
    //             delete *cur;
    //             ranges.erase(cur);
    //             return true;
    //         }
    //         // match at start
    //         else if ((*cur)->start == start && (*cur)->length > length)
    //         {
    //             (*cur)->start += length;
    //             (*cur)->length -= length;
    //             return true;
    //         }
    //         // match at end
    //         else if ((*cur)->start < start && ((*cur)->start + (*cur)->length) == (start + length))
    //         {
    //             (*cur)->length -= length;
    //             return true;
    //         }
    //         // split
    //         else if ((*cur)->start < start && ((*cur)->start + (*cur)->length) > (start + length))
    //         {
    //             range_t* new_range = new range_t(start + length, (*cur)->start + (*cur)->length - start - length);
    //             ranges.push_back(new_range);
    //             (*cur)->length = start - (*cur)->start;
    //             return true;
    //         }
    //     }
    //     return false;
    // }

    /**
     * Allocate a range of specific size and beginning address, with overlapping allowed.
     * @param[in] start The beginning address.
     * @param[in] length The length.
     */
//     * @return true, if successfully allocated at least one part of the range, false otherwise.
    // void allocate_overlapping(type_t start, type_t length)
    // {
    //     iterator cur(ranges.begin());
    //     const_iterator end(ranges.end());
    //     for (; cur != end; ++cur)
    //     {
    //         // full overlap
    //         if ((*cur)->start >= start &&
    //             ((*cur)->start + (*cur)->length) <= (start + length))
    //         {
    //             delete *cur;
    //             ranges.erase(cur);
    //         }
    //         // start overlaps, end out of bounds
    //         else if ((*cur)->start >= start && (*cur)->start < (start + length) &&
    //             ((*cur)->start + (*cur)->length) >= (start + length))
    //         {
    //             (*cur)->start += length - ((*cur)->start - start);
    //             (*cur)->length -= length - ((*cur)->start - start);
    //         }
    //         // start out of bounds, end overlaps
    //         else if ((*cur)->start < start &&
    //             ((*cur)->start + (*cur)->length) <= (start + length))
    //         {
    //             (*cur)->length = (*cur)->start - start;
    //         }
    //         // existing range larger
    //         else if ((*cur)->start < start &&
    //             ((*cur)->start + (*cur)->length) > (start + length))
    //         {
    //             range_t* new_range = new range_t(start + length, (*cur)->start + (*cur)->length - start - length);
    //             ranges.push_back(new_range);
    //             (*cur)->length = start - (*cur)->start;
    //         }
    //     }
    // }

    /**
     * Free a range.
     * @param[in] start Beginning address of the range.
     * @param[in] length Length of the range.
     */
    // void free(type_t start, type_t length)
    // {
    //     iterator cur(ranges.begin());
    //     const_iterator end(ranges.end());
    //     // merge left
    //     for (; cur != end; ++cur)
    //     {
    //         if (((*cur)->start + (*cur)->length) == start)
    //         {
    //             start = (*cur)->start;
    //             length += (*cur)->length;
    //             delete *cur;
    //             ranges.erase(cur);
    //             break;
    //         }
    //     }
    //
    //     cur = ranges.begin();
    //     end = ranges.end();
    //     // merge right
    //     for (; cur != end; ++cur)
    //     {
    //         if ((*cur)->start == (start + length))
    //         {
    //             length += (*cur)->length;
    //             delete *cur;
    //             ranges.erase(cur);
    //             break;
    //         }
    //     }
    //
    //     range_t* new_range = new range_t(start, length);
    //     ranges.push_back(new_range);
    // }
    //
    // void clear()
    // {
    //     const_iterator cur(ranges.begin());
    //     const_iterator end(ranges.end());
    //     for (; cur != end; ++cur)
    //         delete *cur;
    //     ranges.clear();
    // }

    /**
     * Get the number of ranges in the list.
     * @return the number of ranges in the list.
     */
    // inline size_t size() const { return ranges.size(); }

// private:
//     list_type ranges;
};
