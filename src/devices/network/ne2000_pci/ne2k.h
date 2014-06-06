//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "isr.h"

class pci_device_t;

/**
 * NE2000 NIC driver.
 */
class ne2k
{
	uint16_t port_base;
	uint16_t irq;

	void reg_write(int regno, uint8_t value);
	void reg_write_word(int regno, uint16_t value);
	uint8_t reg_read(int regno);
	uint16_t reg_read_word(int regno);

	class irq_handler : public interrupt_service_routine_t
	{
		ne2k* parent;
	public:
		irq_handler(ne2k* p) : parent(p) {}
	    virtual void run(registers_t*);
	};

	irq_handler handler;
	uint8_t next_packet;

public:
	ne2k() : handler(this) {}

	void configure(pci_device_t* card);
	void init();
	void handle_irq();
	void overflow();
	void receive_error();
	void packet_received();
	void transmit_error();
	void packet_transmitted();
	void send_packet(void* buf, uint16_t length);
};
