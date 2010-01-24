// TODO: merge_mmap algo will go into this class
// Copyright (c) 2008 James Molloy, Jörg Pfähler, Matthew Iselin

#pragma once

#include "stl/list"

/*!
 * This class manages a list of ranges. It automatically merges adjacent entries in the list.
 *
 * @param[in] type_t the integer type the range start and length are encoded in.
 */
template<typename type_t>
class range_list_t
{
public:
    class range_t
    {
        public:
            inline range_t(type_t start_, type_t length_) : start(start_), length(length_) {}
            type_t start, length;
    };

    typedef std::list<range_t*> list_type;
    typedef typename list_type::iterator iterator;
    typedef typename list_type::const_iterator const_iterator;

    iterator begin() { return ranges.begin(); }
    const_iterator begin() const { return ranges.begin(); }
    iterator end() { return ranges.end(); }
    const_iterator end() const { return ranges.end(); }

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
    ~range_list_t() { clear(); }

    /*! Deep-copy constructor. */
    range_list_t(const range_list_t& other)
        : ranges()
    {
        clear();
        iterator it(other.ranges.begin());
        for (; it != other.ranges.end(); ++it)
        {
            range_t* new_range = new range_t((*it)->start, (*it)->length);
            ranges.push_back(new_range);
        }
    }

    /*!
     * Allocate a range of a specific size. Pick a first-fit region.
     * @param[in] length the requested length
     * @param[in,out] start the beginning address of the allocated range
     * @returns true, if successfully allocated (and address is valid), false otherwise
     */
    bool allocate(type_t length, type_t* start)
    {
        ASSERT(start);
        iterator cur(ranges.begin());
        const_iterator end(ranges.end());
        for (; cur != end; ++cur)
        {
            if ((*cur)->length >= length)
            {
                *start = (*cur)->start;
                (*cur)->start += length;
                (*cur)->length -= length;
                if ((*cur)->length == 0)
                {
                    delete *cur;
                    ranges.erase(cur);
                }
                return true;
            }
        }
        return false;
    }

    /*!
     * Allocate a range of specific size and beginning address.
     * @param[in] length the length
     * @param[in] start the beginning address
     * @return true, if successfully allocated, false otherwise
     */
    bool allocate(type_t length, type_t start)
    {
        iterator cur(ranges.begin());
        const_iterator end(ranges.end());
        for (; cur != end; ++cur)
        {
            // exact match
            if ((*cur)->start == start && (*cur)->length == length)
            {
                delete *cur;
                ranges.erase(cur);
                return true;
            }
            // match at start
            else if ((*cur)->start == start && (*cur)->length > length)
            {
                (*cur)->start += length;
                (*cur)->length -= length;
                return true;
            }
            // match at end
            else if ((*cur)->start < start && ((*cur)->start + (*cur)->length) == (start + length))
            {
                (*cur)->length -= length;
                return true;
            }
            // split
            else if ((*cur)->start < start && ((*cur)->start + (*cur)->length) > (start + length))
            {
                range_t* new_range = new range_t(start + length, (*cur)->start + (*cur)->length - start - length);
                ranges.push_back(new_range);
                (*cur)->length = start - (*cur)->start;
                return true;
            }
        }
        return false;
    }

    /*!
     * Allocate a range of specific size and beginning address, with overlapping allowed.
     * @param[in] length the length
     * @param[in] start the beginning address
     */
//     * @return true, if successfully allocated at least one part of the range, false otherwise
    void allocate_overlapping(type_t length, type_t start)
    {
        iterator cur(ranges.begin());
        const_iterator end(ranges.end());
        for (; cur != end; ++cur)
        {
            // full overlap
            if ((*cur)->start >= start &&
                ((*cur)->start + (*cur)->length) <= (start + length))
            {
                delete *cur;
                ranges.erase(cur);
            }
            // start overlaps, end out of bounds
            else if ((*cur)->start >= start && (*cur)->start < (start + length) &&
                ((*cur)->start + (*cur)->length) >= (start + length))
            {
                (*cur)->start += length - ((*cur)->start - start);
                (*cur)->length -= length - ((*cur)->start - start);
            }
            // start out of bounds, end overlaps
            else if ((*cur)->start < start &&
                ((*cur)->start + (*cur)->length) <= (start + length))
            {
                (*cur)->length = (*cur)->start - start;
            }
            // existing range larger
            else if ((*cur)->start < start &&
                ((*cur)->start + (*cur)->length) > (start + length))
            {
                range_t* new_range = new range_t(start + length, (*cur)->start + (*cur)->length - start - length);
                ranges.push_back(new_range);
                (*cur)->length = start - (*cur)->start;
            }
        }
    }

    /*!
     * Free a range.
     * @param[in] length length of the range
     * @param[in] start beginning address of the range
     */
    void free(type_t length, type_t start)
    {
        iterator cur(ranges.begin());
        const_iterator end(ranges.end());
        // merge left
        for (; cur != end; ++cur)
        {
            if (((*cur)->start + (*cur)->length) == start)
            {
                start = (*cur)->start;
                length += (*cur)->length;
                delete *cur;
                ranges.erase(cur);
                break;
            }
        }

        cur = ranges.begin();
        end = ranges.end();
        // merge right
        for (; cur != end; ++cur)
        {
            if ((*cur)->start == (start + length))
            {
                length += (*cur)->length;
                delete *cur;
                ranges.erase(cur);
                break;
            }
        }

        range_t* new_range = new range_t(start, length);
        ranges.push_back(new_range);
    }

    void clear()
    {
        const_iterator cur(ranges.begin());
        const_iterator end(ranges.end());
        for (; cur != end; ++cur)
            delete *cur;
        ranges.clear();
    }

    /*!
     * Get the number of ranges in the list.
     * @return the number of ranges in the list.
     */
    inline size_t size() const { return ranges.size(); }

private:
    std::list<range_t*> ranges;
    range_list_t& operator =(const range_list_t& other);
};
