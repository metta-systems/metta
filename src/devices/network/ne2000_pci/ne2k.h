#pragma once

#include "types.h"

class pci_bus_device_t;

class ne2k
{
	uint16_t port_base;
	uint16_t irq;

	void reg_write(int regno, int value);
	uint8_t reg_read(int regno);

public:
	void configure(pci_bus_device_t* card);
	void init();
	void overflow();
};
