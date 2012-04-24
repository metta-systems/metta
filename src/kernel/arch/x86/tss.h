//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

struct tss_t
{
    uint16_t   link;
    uint16_t   link_unused;

    // For syscalls, you must set up SS0:ESP0 to point to a ring0 kernel stack for handling interrupts.
    uint32_t*  esp0; // 4 bytes ESP0 value
    uint16_t   ss0;
    uint16_t   ss0_unused;

    uint32_t   esp1;
    uint16_t   ss1;
    uint16_t   ss1_unused;

    uint32_t   esp2;
    uint16_t   ss2;
    uint16_t   ss2_unused;

    uint32_t   cr3;
    uint32_t   eip;
    uint32_t   eflags;

    uint32_t   eax;
    uint32_t   ecx;
    uint32_t   edx;
    uint32_t   ebx;

    uint32_t   esp;
    uint32_t   ebp;

    uint32_t   esi;
    uint32_t   edi;

    uint16_t   es;
    uint16_t   es_unused;

    uint16_t   cs;
    uint16_t   cs_unused;

    uint16_t   ss;
    uint16_t   ss_unused;

    uint16_t   ds;
    uint16_t   ds_unused;

    uint16_t   fs;
    uint16_t   fs_unused;

    uint16_t   gs;
    uint16_t   gs_unused;

    uint16_t   ldt;
    uint16_t   ldt_unused;

    uint16_t   trap;
    uint16_t   iomap;
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
