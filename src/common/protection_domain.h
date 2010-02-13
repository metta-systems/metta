//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2009 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "range_list.h"

class stretch_driver_t;

/*!
 * Stretch is a user-visible kernel entity describing a range of virtual addresses in address space.
 *
 * Stretch provides frame allocation and page fault handling interface inside protection domain.
 *
 * Each stretch has a stretch driver. Stretch driver provides stretch with physical frames, page fault handling and
 * mapping setup. A default_stretch_driver provides default handling for all applications.
 */
class stretch_t
{
public:
    typedef uint32_t access_t;
    static const uint32_t read = 0x1;
    static const uint32_t write = 0x2;
    static const uint32_t execute = 0x4;
    static const uint32_t meta = 0x8;

    /*!
     * @note Creates stretch descriptor in the kernel space.
     */
    static stretch_t* create(size_t size, access_t access, address_t base = 0);

    /*!
     * Bind a stretch to userspace stretch driver, which will provide backing store
     * and handle page faults and other memory exceptions for this stretch.
     */
    void bind(stretch_driver_t* driver);
    void unbind();
    access_t access();
    void set_access(access_t access);

private:
    stretch_t(address_t base, size_t size, access_t rights);

    address_t address;
    size_t    size;
    access_t  access_rights;
};

/*!
 * Privileged class (private) in kernel domain.
 */
class protection_domain_t
{
public:
    /*! The default constructor. */
    inline protection_domain_t() {}
    /*! Declare destructor virtual */
    inline virtual ~protection_domain_t() {}

    /*!
     * Privileged space is mapped in all address spaces and is usually used to access
     * kernel and common functionality in all domains.
     */
    static protection_domain_t& privileged();
    static protection_domain_t* create(); // factory method to make new address spaces

    // -- stretch driver --
    stretch_t* stretch(void* virtual_address); // clients only see stretches and do stretch management through
    // stretch_driver

    /*!
     * @returns true if address is valid in current address space, that is, address is within the bounds of
     * available stretches. Addresses outside of available stretches are considered invalid and cannot be
     * used for mapping.
     * @note This is currently O(N) operation, use wisely.
     */
    virtual bool is_valid(void* virtual_address) = 0;
    /*!
     * @returns true if virtual_address is mapped into one of current protection domain's stretches.
     */
    virtual bool is_mapped(void* virtual_address) = 0;
    virtual bool map(physical_address_t physical_address,
                     void* virtual_address,
                     flags_t flags) = 0;
    inline bool map(physical_address_t physical_address,
                    address_t virtual_address,
                    flags_t flags)
    {
        return map(physical_address, reinterpret_cast<void*>(virtual_address), flags);
    }
    virtual void mapping(void* virtual_address,
                         physical_address_t& physical_address,
                         flags_t& flags) = 0;
    virtual void unmap(void* virtual_address) = 0;
    inline void unmap(address_t virtual_address)
    {
        unmap(reinterpret_cast<void*>(virtual_address));
    }
    // -- /stretch driver --

protected:
    std::list<stretch_t*> stretches; //! Stretches owned by this protection domain.
    range_list_t<physical_address_t> owned_frames; //! Physical frames in possession of the domain.

private:
    /*!
     * Disable the copy constructor.
     * @note NOT implemented
     */
    protection_domain_t(const protection_domain_t&);
    /*!
     * Disable assignment operator.
     * @note Not implemented
     */
    protection_domain_t& operator =(const protection_domain_t&);
};



