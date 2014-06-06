//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "bga.h"
#include "pci_bus.h"
#include "card_registers.h"
#include "default_console.h"

using namespace bga_card;

namespace graphics {

void bga::reg_write(int regno, uint16_t value)
{
    x86_cpu_t::outw(VBE_DISPI_IOPORT_INDEX, regno);
    x86_cpu_t::outw(VBE_DISPI_IOPORT_DATA, value);
}

uint16_t bga::reg_read(int regno)
{
    x86_cpu_t::outw(VBE_DISPI_IOPORT_INDEX, regno);
    return x86_cpu_t::inw(VBE_DISPI_IOPORT_DATA);
}

bool bga::is_available()
{
    return reg_read(VBE_DISPI_INDEX_ID) == VBE_DISPI_ID5;
}

#define BAR0 0x10

void bga::configure(pci_device_t* card)
{
    uint32_t bar0 = card->read_config_space(BAR0); // LFB

    kconsole << "This bga uses LFB at " << bar0 << endl;
    lfb = (void*)bar0;
}

// OSdev wiki sez:
// Based on source code examination for Bochs (iodev/vga.cc)
// setting VBE_DISPI_GETCAPS in VBE_DISPI_INDEX_ENABLE makes the
// VBE_DISPI_INDEX_ (XRES / YRES / BPP) fields return their maximum
// values when read instead of the current values.
void bga::init()
{
    kconsole << "Initializing BGA." << endl;
    set_caps();
    set_mode(640, 480, 32);
}

void bga::set_mode(int width, int height, int bpp)
{
    reg_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
    reg_write(VBE_DISPI_INDEX_XRES, width);
    reg_write(VBE_DISPI_INDEX_YRES, height);
    reg_write(VBE_DISPI_INDEX_BPP, bpp);
    reg_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED|VBE_DISPI_LFB_ENABLED);
}

void bga::set_caps()
{
    reg_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_GETCAPS);
    xres_max = reg_read(VBE_DISPI_INDEX_XRES);
    yres_max = reg_read(VBE_DISPI_INDEX_YRES);
    bpp_max = reg_read(VBE_DISPI_INDEX_BPP);
    reg_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED|VBE_DISPI_LFB_ENABLED);
    kconsole << "Maximum supported mode " << xres_max << "x" << yres_max << "_" << bpp_max << endl;
}

} // namespace graphics
