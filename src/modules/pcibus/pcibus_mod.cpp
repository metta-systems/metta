//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "pci_bus.h"
#include "closure_interface.h"
#include "closure_impl.h"
#include "default_console.h"

// temporary testing
#include "../../devices/network/ne2000_pci/ne2k.h"
#include "../../devices/graphics/bochs_emu/bga.h"

static const char* class2string(int class_id)
{
    switch(class_id)
    {
        case 0x00: return "Device was built prior definition of the class code field";
        case 0x01: return "Mass Storage Controller";
        case 0x02: return "Network Controller";
        case 0x03: return "Display Controller";
        case 0x04: return "Multimedia Controller";
        case 0x05: return "Memory Controller";
        case 0x06: return "Bridge Device";
        case 0x07: return "Simple Communication Controllers";
        case 0x08: return "Base System Peripherals";
        case 0x09: return "Input Devices";
        case 0x0A: return "Docking Stations";
        case 0x0B: return "Processors";
        case 0x0C: return "Serial Bus Controllers";
        case 0x0D: return "Wireless Controllers";
        case 0x0E: return "Intelligent I/O Controllers";
        case 0x0F: return "Satellite Communication Controllers";
        case 0x10: return "Encryption/Decryption Controllers";
        case 0x11: return "Data Acquisition and Signal Processing Controllers";
        case 0xFF: return "Device does not fit any defined class.";
        default: return "Reserved";
    }
}

void pci_device_t::dump()
{
    kconsole << "PCI device: bus " << bus << ", slot " << slot << ", func " << function << ", vendor " << vendor_id << ", device " << device_id << ", class " << base_class << ", subclass " << sub_class << ", header type " << header_type << endl;
    kconsole << "ProgIF " << prog_iface << ", revision " << revision << ", cache line size " << cache_line_size << ", subsys vendor " << subsys_vendor << ", subsys id " << subsys_id << ", INT# line " << interrupt_line << ", INT# pin " << interrupt_pin << endl;
    if (header_type & 0x80)
        kconsole << "  Multifunction device";
    else
        kconsole << "  Single function device";
    switch (header_type & 0x7f)
    {
        case 0:
            kconsole << ", standard header." << endl;
            break;
        case 1:
            kconsole << ", PCI-to-PCI bridge." << endl;
            break;
        case 2:
            kconsole << ", CardBus bridge." << endl;
            break;
        default:
            kconsole << ", unknown header type." << endl;
            break;
    }
    kconsole << "  Class ID: " << class2string(base_class) << endl;
}

//====
// implement interface closure
//====

// pci bus probing will merely record device and vendor ids present on pci bus and its topology
// vendors and device ids are recorded into a pcibus lookup table
// later when device drivers are probing they will use this table to look up if device they support is actually present
// in this case they will attempt the device initialization and mark this device as driven in the pcibus table
// pcibus_scan table of entries [vendor_id, device_id, has_driver, driver_id?]

static void
entry(closure::closure_t* self)
{
    for ( unsigned int bus=0; bus<256; bus++ ) {
        for ( unsigned int slot=0; slot<32; slot++ ) {
            for ( unsigned int func=0; func<7; func++ ) {
                pci_device_t dev(bus, slot, func);

                if (dev.is_present())
                {
                    dev.dump();

// template<typename T>
// class inclusive_range_t
// {
//  T from;
//  T to;
//  bool includes(T value) const { return value >= from and value <= to; }
// };

// Driver registry:
// {
//  inclusive_range_t<uint16_t> vendor_range;
//  inclusive_range_t<uint16_t> device_range;
// }

// if (vendor_range.includes(dev.vendor()) and device_range.includes(dev.device())) {
//  // device has matched, instantiate the driver and probe()
// }

                    if ((dev.vendor() == 0x1234) && (dev.device() == 0x1111))
                    {
                        graphics::bga bga;
                        bga.configure(&dev);
                        if (bga.is_available()) {
                            bga.init();
                        }
                        else {
                            kconsole << "BGA init failed!" << endl;
                        }
                    }

                    if ((dev.vendor() == 0x10ec) && (dev.device() == 0x8029))
                    {
                        ne2k ne;
                        ne.configure(&dev);
                        ne.init();

                        // Send a nice hello world to everyone
                        uint8_t hello[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                           0xb0, 0xc4, 0x20, 0x00, 0x00, 0x00,
                                           0x00, 0x10,
                                           'H', 'e', 'l', 'l', 'o', ' ', 'n', 'e', 't', ' ', 'w', 'o', 'r', 'l', 'd', '!'};
                        ne.send_packet(hello, sizeof(hello));
                    }
                }
            }
        }
    }
};

static const closure::ops_t methods =
{
    entry
};

static const closure::closure_t clos =
{
    &methods,
    nullptr
};

// EXPORT_CLOSURE_TO_ROOTDOM(pci_bus, v1, clos);
extern "C" const closure::closure_t* const exported_pcibus_rootdom = &clos;
