//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"

/*!
Bootimage is similar to Nemesis' nexus - it contains information, modules, dependencies, namespaces etc.
required for successful bootstrap.

Index information in bootimage should allow quick and easy dependency calculation and module instantiation.
Modules can be ELF executables or data blobs loaded and mapped at specified address in memory.

Deps are lists of items from common stringtable. (ofs,len) pairs for ndeps count.

Root entry in bootimage is main startup code, the system privileged domain or root domain.

<data blob>
address
size
name ofs
<module>
address
size
name ofs
upcall record (PCB) location
dependencies list (ndeps * name ofs entries)

*/

/*!

// version 3: current.
              * initfs::header
              * aligned: modules data
              * aligned: nexus with metadata index in tagged format.
 */

class bootimage_t
{
public:
    explicit bootimage_t(address_t start) : location(start) {}

    /*!
     * Silly iterator interface
     * TODO: replace with normal iterator.
     */
    address_t   get_file(uint32_t num);
    const char* get_file_name(uint32_t num);
    uint32_t    get_file_size(uint32_t num);
    uint32_t    count();

    bool valid();

private:
    address_t location;
};
 
// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
