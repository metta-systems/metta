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
#include "protection_domain.h"

typedef address_t cpu_id_t;
typedef uint8_t  apic_id_t;

class cpu_information_t
{
public:
    inline protection_domain_t& current_protection_domain() const
    {
        if (protection_domain)
            return *protection_domain;
        else
            return protection_domain_t::privileged();
    }

    cpu_information_t(cpu_id_t cpu_id) : id(cpu_id), protection_domain(&protection_domain_t::privileged()) {}

private:
    cpu_information_t();
    cpu_information_t(const cpu_information_t&);
    cpu_information_t& operator =(const cpu_information_t&);

private:
    cpu_id_t id;
    protection_domain_t* protection_domain;
};
