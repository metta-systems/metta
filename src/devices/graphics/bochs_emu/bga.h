//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2013, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"

class pci_device_t;

namespace graphics {

/**
 * Bochs BGA graphics adapter driver.
 *
 * Presently supports only LFB (see no reason to use banked modes).
 */
class bga
{
	void* lfb{(void*)0xe0000000};

	void reg_write(int regno, uint16_t value);
	uint16_t reg_read(int regno);

public:
	bga() {}

	/**
	 * Check for BGA support.
	 * @return true if BGA is available, false otherwise.
	 */
	bool is_available();
	void configure(pci_device_t* card);
	void init();
	void set_mode(int width, int height, int bpp);
};

} // namespace graphics
