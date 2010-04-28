//
// buildpcb tool reads list of input ELF files and builds PCB structures for use in bootimage.
//
// usage:
//     buildpcb output.pcb input1.elf input2.elf .. inputN.elf
//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//

// first elf image indicates the bootstrap file, which is then rooted at start of pcb image.
// subsequent images are relocated to their absolute address to allow running them during startup.


// 

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
