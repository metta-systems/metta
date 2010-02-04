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

/*!
 * Virtual address space in Metta is single, shared between all processes. This means that virtual to physical
 * mapping is equivalent in all domains. virtual_address_space_t describes only part of the whole address space
 * available to a particular process.
 *
 * Each virtual space has @c stretches of addresses with particular protection properties.
 - kernel, user or reserved. Kernel stretches
 * correspond to kernel data and code, common in all address spaces. User type addresses are available to the process.
 * Reserved types are either reserved for memory-mapped I/O or for future use by kernel structures and are unavailable
 * to memory allocation inside process.
 *
 * virtual_address_space_t encapsulates protection domain control. (@todo rename protection_domain_t)
 * virtual_address_space_t also wraps around processor's paging mechanism and provides mapping/unmapping facilities
 * for memory pages.
 *
 * Each stretch has a stretch driver. stretch_driver provides stretch with physical frames, page fault handling and
 * mapping setup. A default_stretch_driver provides default handling for applications.
 */
/*!
The translation system deals with inserting, retrieving or deleting mappings between virtual and physical addresses.
As such it may be considered an interface to a table of information held about these mappings; the actual mapping
will typically be performed as necessary by whatever memory management hardware or software is present.

The translation system is divided into two parts: a high-level management module, and the low-level trap handlers
and system calls. The high-level part is private to the system domain, and handles the following:

  * Bootstrapping the `MMU' (in hardware or software), and setting up initial mappings.
  * Adding, modifying or deleting ranges of virtual addresses, and performing the associated page table management.
  * Creating and deleting protection domains.
  * Initialising and partly maintaining the RamTab; this is a simple data structure maintaining information about the current use of frames of main memory.

The high-level translation system is used by both the stretch allocator and the frames allocator.
The stretch allocator uses it to setup initial entries in the page table for stretches it has created,
or to remove such entries when a stretch is destroyed. These entries contain protection information but are
by default invalid: i.e. addresses within the range will cause a page fault if accessed.
The frames allocator, on the other hand, uses the RamTab to record the owner and logical frame width
of allocated frames of main memory.

The high-level part of the translation system is also in the system domain: this is machine-dependent code responsible for the construction of page tables, and the setting up of NULL mappings for freshly allocated virtual addresses. These mappings are used to hold the initial protection information, and by default are set up to cause a page fault on the first access. Placing this functionality within the system domain means that the low-level translation system does not need to be concerned with the allocation of page-table memory. It also allows protection faults, page faults and ``unallocated address'' faults to be distinguished and dispatched to the faulting application.


high-level module is encapsulated by protection_domain_t, it uses a factory to create new protection domains.
protection domain gives interface to the stretch allocator, which gives out stretches of virtual addresses belonging
to the domain and manages them.
stretch driver is located inside application space, provided by the shared library code or implemented by the
application itself. it interfaces with frame allocator to provide backing RAM storage for stretches it manages.
frame allocation and ramtab maintenance handled by frame_allocator_t.

*/
class stretch_driver_t;

class stretch_t
{
public:
    typedef uint32_t access_t;

    static stretch_t* create(address_t base, size_t size);

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

    const address_t address;
    const size_t    size;
    access_t        access_rights;
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
     */
    virtual bool is_valid(void* virtual_address) = 0;
    /*!
     * @returns true if virtual_address is mapped into one of current protection domain's stretches.
     */
    virtual bool is_mapped(void* virtual_address) = 0;
    virtual bool map(physical_address_t physical_address,
                     void* virtual_address,
                     flags_t flags) = 0;
    virtual void mapping(void* virtual_address,
                         physical_address_t& physical_address,
                         flags_t& flags) = 0;
    virtual void unmap(void* virtual_address) = 0;
    // -- /stretch driver --

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



