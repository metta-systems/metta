//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "cpu.h"

/** Offsets into PCI configuration space. */
#define PCI_CONFIG_VENDOR_ID        0x00    /**< Vendor ID        - 16-bit. */
#define PCI_CONFIG_DEVICE_ID        0x02    /**< Device ID        - 16-bit. */

#define PCI_CONFIG_COMMAND      0x04    /**< Command          - 16-bit. */
#define PCI_CONFIG_STATUS       0x06    /**< Status           - 16-bit. */

#define PCI_CONFIG_REVISION     0x08    /**< Revision ID      - 8-bit. */
#define PCI_CONFIG_PI           0x09    /**< Prog. Interface  - 8-bit.  */
#define PCI_CONFIG_SUB_CLASS        0x0A    /**< Sub-class        - 8-bit.  */
#define PCI_CONFIG_BASE_CLASS       0x0B    /**< Base class       - 8-bit.  */

#define PCI_CONFIG_CACHE_LINE_SIZE  0x0C    /**< Cache line size  - 8-bit.  */
#define PCI_CONFIG_LATENCY      0x0D    /**< Latency timer    - 8-bit.  */
#define PCI_CONFIG_HEADER_TYPE      0x0E    /**< Header type      - 8-bit.  */
#define PCI_CONFIG_BIST         0x0F    /**< BIST             - 8-bit.  */

#define PCI_CONFIG_BAR0         0x10    /**< BAR0             - 32-bit. */
#define PCI_CONFIG_BAR1         0x14    /**< BAR1             - 32-bit. */
#define PCI_CONFIG_BAR2         0x18    /**< BAR2             - 32-bit. */
#define PCI_CONFIG_BAR3         0x1C    /**< BAR3             - 32-bit. */
#define PCI_CONFIG_BAR4         0x20    /**< BAR4             - 32-bit. */
#define PCI_CONFIG_BAR5         0x24    /**< BAR5             - 32-bit. */
#define PCI_CONFIG_CARDBUS_CIS      0x28    /**< Cardbus CIS Ptr  - 32-bit. */

#define PCI_CONFIG_SUBSYS_VENDOR    0x2C    /**< Subsystem vendor - 16-bit. */
#define PCI_CONFIG_SUBSYS_ID        0x2E    /**< Subsystem ID     - 16-bit. */

#define PCI_CONFIG_ROM_ADDR     0x30    /**< ROM base address - 32-bit. */

#define PCI_CONFIG_INTERRUPT_LINE   0x3C    /**< Interrupt line   - 8-bit.  */
#define PCI_CONFIG_INTERRUPT_PIN    0x3D    /**< Interrupt pin    - 8-bit.  */
#define PCI_CONFIG_MIN_GRANT        0x3E    /**< Min grant        - 8-bit.  */
#define PCI_CONFIG_MAX_LATENCY      0x3F    /**< Max latency      - 8-bit.  */

/** Bits in the PCI command register. */
#define PCI_COMMAND_IO          (1<<0)  /**< I/O Space enable. */
#define PCI_COMMAND_MEMORY      (1<<1)  /**< Memory Space enable. */
#define PCI_COMMAND_BUS_MASTER      (1<<2)  /**< Bus Mastering enable. */
#define PCI_COMMAND_SPECIAL     (1<<3)  /**< Special Cycles enable. */
#define PCI_COMMAND_MWI         (1<<4)  /**< Memory Write & Invalidate enable. */
#define PCI_COMMAND_VGA_SNOOP       (1<<5)  /**< VGA Pallette Snoop enable. */
#define PCI_COMMAND_PARITY      (1<<6)  /**< Parity Check enable. */
#define PCI_COMMAND_STEPPING        (1<<7)  /**< Stepping enable. */
#define PCI_COMMAND_SERR        (1<<8)  /**< SERR enable. */
#define PCI_COMMAND_FASTB2B     (1<<9)  /**< Fast Back-to-Back enable. */
#define PCI_COMMAND_INT_DISABLE     (1<<10) /**< I/O interrupt disable. */

/** Mask to clear special bits from an BAR. */
#define PCI_IO_ADDRESS_MASK     0xFFFFFFFC
#define PCI_MEM_ADDRESS_MASK        0xFFFFFFF0

class device_tree_node_t
{
};
/*
public:
    class attribute_t
    {
        std::string name;
        enum { STRING, UINT8, UINT16, UINT32, UINT64, BOOLEAN } type;
        union {
            uint8_t uint8;
            uint16_t uint16;
            uint32_t uint32;
            uint64_t uint64;
            bool boolean;
            const char* string; // can't use std::string in union
        } value;
    public:
        attribute_t(name, type, value);
    };

private:
    typedef std::map<std::string, device_tree_node_t*> children_t;

    device_tree_node_t* parent;
    std::string name;
    children_t children;
    std::vector<attribute_t> attributes;

public:
    device_tree_node_t(std::string _name, device_tree_node_t* _parent, std::vector<attribute_t> attrs)
        : name(_name)
        , parent(_parent)
        , attributes(attrs)
    {}

    **
     * The child must have been previously initialized with this node as parent.
     *
    void append_child(device_tree_node_t* child) { children.insert(child->name, child); }
};

// the separation logic is as follows:
// device tree items contain only simple attributes (string, various integers, boolean)
// the concrete device has device-specific data and links to a corresponding device tree node.
*
Device tree:
/
+--NUMA domain "SMP"
   +--CPU0
   +--CPU1 etc
   +--PCI Controller #0   <-- pci_bus_t
      +--Graphics Controller etc

in kiwi:
<root>
+--bus
   +--pci


the bus_manager/pci should be called with device node pointer with the pci-bus-to-be parent.
then
if (pci_bus_t::detect())
{
    subnode = new(PVS(heap)) device_tree_node_t("PCI Controller #0", node, attrs); // there MAY be multiple PCI controllers within a single domain, but we leave this as excercise for the future
    node->add_child(subnode);
    pci_bus_t::scan(subnode, 0);
}
*/


class pci_bus_t
{
    /**
     * Configuration Address Register:
     * ------------------------------------------------------------------
     * | 31 | 30 - 24  | 23 - 16 | 15 - 11 | 10 - 8   | 7 - 2   | 1 - 0 |
     * |----------------------------------------------------------------|
     * | EB | Reserved | Bus No. | Dev No. | Func No. | Reg No. | 00    |
     * ------------------------------------------------------------------
     */
    static const int PCI_CONFIG_ADDRESS = 0xcf8;
    static const int PCI_CONFIG_DATA = 0xcfc;

    // Standard PCI read mechanism.
    static uint32_t pci_read_mechanism_1(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
    {
        /* create configuration address */
        address_t address = (static_cast<address_t>(bus & 0xff) << 16) |
                            (static_cast<address_t>(slot & 0x3f) << 11) |
                            (static_cast<address_t>(func & 0x7) << 8) |
                            (static_cast<address_t>(offset & 0xfc)) |
                            0x80000000U; // enable bit
 
        /* write out the address */
        x86_cpu_t::outl(PCI_CONFIG_ADDRESS, address);
        /* read in the data */
        return x86_cpu_t::inl(PCI_CONFIG_DATA);
    }

    /**
     * Detect the presence of PCI bus.
     * Should be arch-dependent, this one is x86 specific.
     */
    static bool detect()
    {
        x86_cpu_t::outl(PCI_CONFIG_ADDRESS, 0x80000000U);
        return x86_cpu_t::inl(PCI_CONFIG_ADDRESS) == 0x80000000U;
    }

public:
    pci_bus_t()
    {
    }

    static uint32_t read_config_space(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
    {
        return pci_read_mechanism_1(bus, slot, func, offset);
    }
/*
    void pci_bus_t::scan_device(device_tree_node_t* node, int bus, int slot, int func)
    {
        pci_device_t dev(bus, slot, func);

        if (!dev.is_present())
            return;

        subnode = new(PVS(heap)) device_tree_node_t(name, node, dev.attrs());
        node->append_child(subnode);

        if (dev.is_pci_to_pci_bridge())
            scan(subnode, dev.bridge_id());
    }

    void pci_bus_t::scan(device_tree_node_t* node, int bus_id)
    {
        // up to 32 slots on a bus
        i = 0..31
        pci_device_t dev(bus_id, i, 0);
        if (dev.is_multifunction_device())
        {
            // scan each function separately
            f = 0..7
            scan_device(node, bus_id, i, f);
        }
        else
        {
            scan_device(node, bus_id, i, 0);
        }
    }*/
};

class pci_driver_t
{
};

class pci_device_t
{
    pci_driver_t* driver;
    device_tree_node_t* node;

    /** Location of the device. */
    uint8_t bus;                /**< Bus ID. */
    uint8_t slot;               /**< Device number. */
    uint8_t function;           /**< Function number. */

    /** Information about the device. */
    uint16_t vendor_id;         /**< Vendor ID. */
    uint16_t device_id;         /**< Device ID. */
    uint8_t base_class;         /**< Class ID. */
    uint8_t sub_class;          /**< Sub-class ID. */
    uint8_t prog_iface;         /**< Programming interface. */
    uint8_t revision;           /**< Revision. */
    uint8_t cache_line_size;    /**< Cache line size (number of DWORDs). */
    uint8_t header_type;        /**< Header type. */
    uint16_t subsys_vendor;     /**< Subsystem vendor. */
    uint16_t subsys_id;         /**< Subsystem ID. */
    uint8_t interrupt_line;     /**< Interrupt line. */
    uint8_t interrupt_pin;      /**< Interrupt pin. */

public:
    pci_device_t(uint8_t _bus, uint8_t _slot, uint8_t _func) : driver(0), node(0), bus(_bus), slot(_slot), function(_func)
    {
        uint32_t vendor_device = read_config_space(0x0);
        vendor_id = vendor_device & 0xffff;
        device_id = (vendor_device >> 16) & 0xffff;

        if (!is_present())
            return;

        uint32_t class_subclass = read_config_space(0x8);
        base_class = (class_subclass >> 24) & 0xff;
        sub_class = (class_subclass >> 16) & 0xff;
        prog_iface = (class_subclass >> 8) & 0xff;
        revision = (class_subclass) & 0xff;

        uint32_t header = read_config_space(0xc);
        cache_line_size = (header) & 0xff;
        header_type = (header >> 16) & 0xff;

        uint32_t subsys = read_config_space(0x2c);
        subsys_vendor = subsys & 0xffff;
        subsys_id = (subsys >> 16) & 0xffff;

        uint32_t interrupt = read_config_space(0x3c);
        interrupt_line = interrupt & 0xff;
        interrupt_pin = (interrupt >> 8) & 0xff;
    }

    inline bool is_present()
    {
        return vendor() != 0xffff;
    }

    inline bool is_pci_to_pci_bridge()
    {
        return is_present() && (base_class == 0x06) && (sub_class == 0x04);
    }

    inline bool is_multifunction_device()
    {
        return is_present() && (header_type & 0x80);
    }

    inline uint16_t vendor() { return vendor_id; }
    inline uint16_t device() { return device_id; }

    inline uint32_t read_config_space(uint16_t offset)
    {
        return pci_bus_t::read_config_space(bus, slot, function, offset);
    }

    void dump();
};
