#pragma once

#include "cpu.h"

class pci_bus_t
{
	// Standard PCI read mechanism.
	static uint32_t pci_read_mechanism_1(uint16_t bus, uint16_t slot, uint16_t func, uint16_t offset)
	{
		const int PCI_CONFIG_ADDRESS = 0xcf8;
		const int PCI_CONFIG_DATA = 0xcfc;

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

public:
	pci_bus_t()
	{
	}

	static uint32_t read_config_space(uint16_t bus, uint16_t slot, uint16_t func, uint16_t offset)
	{
		return pci_read_mechanism_1(bus, slot, func, offset);
	}

};

class pci_bus_device_t
{
	uint16_t bus;
	uint16_t slot;
	uint16_t func;

	uint16_t vendor_id;
	uint16_t device_id;
	uint16_t class_and_subclass;
	uint8_t header_type;

public:
	pci_bus_device_t(uint16_t _bus, uint16_t _slot, uint16_t _func) : bus(_bus), slot(_slot), func(_func)
	{
		uint32_t vendor_device = pci_bus_t::read_config_space(bus, slot, func, 0x0);
		vendor_id = vendor_device & 0xffff;
		device_id = (vendor_device >> 16) & 0xffff;
		uint32_t class_subclass = pci_bus_t::read_config_space(bus, slot, func, 0x8);
		class_and_subclass = (class_subclass >> 16) & 0xffff;
		uint32_t header = pci_bus_t::read_config_space(bus, slot, func, 0xc);
		header_type = (header >> 16) & 0xff;
	}

	inline bool is_present()
	{
		return vendor_id != 0xffff;
	}

	void dump();
};
