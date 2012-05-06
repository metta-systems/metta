#pragma once

#include "types.h"
#include "isr.h"

class pci_bus_device_t;

// Simple packet wrapper.
class buffer_t
{
	void* ptr;
	size_t size;
public:
	buffer_t(void* _ptr, size_t _size) : ptr(_ptr), size(_size) {}
};

class ne2k
{
	uint16_t port_base;
	uint16_t irq;

	void reg_write(int regno, int value);
	uint8_t reg_read(int regno);

	class irq_handler : public interrupt_service_routine_t
	{
		ne2k* parent;
	public:
		irq_handler(ne2k* p) : parent(p) {}
	    virtual void run(registers_t*);
	};

	irq_handler handler;

public:
	ne2k() : handler(this) {}

	void configure(pci_bus_device_t* card);
	void init();
	void handle_irq();
	void overflow();
	void receive_error();
	void packet_received();
	void transmit_error();
	void packet_transmitted();
	void send_packet(buffer_t buf);
};
