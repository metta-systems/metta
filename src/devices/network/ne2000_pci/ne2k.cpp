#include "ne2k.h"
#include "card_registers.h"
#include "default_console.h"
#include "pci_bus.h"
#include "cpu.h"

using namespace ne2k_card;

#define INIT_PAGE 1
#define STOP_PAGE 0

void ne2k::reg_write(int regno, int value)
{
	x86_cpu_t::outb(port_base + regno, value);
}

uint8_t ne2k::reg_read(int regno)
{
	return x86_cpu_t::inb(port_base + regno);
}

#define BAR0 0x10
#define INTERRUPT 0x3c

void ne2k::configure(pci_bus_device_t* card)
{
	// read PCI BARs and figure out our base address and IRQ
	uint32_t bar0 = card->read_config_space(BAR0);
	uint32_t intr = card->read_config_space(INTERRUPT);

	if (bar0 & 1)
	{
		port_base = bar0 & ~3;
		kconsole << "This ne2k supports I/O port access, with port base 0x" << (uint32_t)port_base << "." << endl;
	}
	else
	{
		kconsole << "WARNING: This device doesn't use I/O, cannot support it yet." << endl;
		port_base = 0xffff;
	}
	irq = intr & 0xff;
	kconsole << "This ne2k uses irq line " << irq << endl;
}

// ne2k card initialization sequence
void ne2k::init()
{
	kconsole << "Initializing NE2000." << endl;
	reg_write(COMMAND_BANK012_RW, COMMAND_BANK0|COMMAND_REMOTEDMA_ABORT|COMMAND_STOP); // 1
	reg_write(DATA_CONFIGURATION_BANK0_W, DATA_CONFIGURATION_NON_LOOPBACK_SELECT); // 2
	reg_write(REMOTE_BYTE_COUNT0_BANK0_W, 0); // 3
	reg_write(REMOTE_BYTE_COUNT1_BANK0_W, 0); // 3
	reg_write(RECEIVE_CONFIGURATION_BANK0_W, 0); // 4
	reg_write(TRANSMIT_CONFIGURATION_BANK0_W, TRANSMIT_CONFIGURATION_INTERNAL_LOOPBACK); // 5
	reg_write(BOUNDARY_POINTER_BANK0_RW, INIT_PAGE); // 6
	reg_write(PAGE_START_BANK0_W, INIT_PAGE); // 6
	reg_write(PAGE_STOP_BANK0_W, STOP_PAGE); // 6
	reg_write(INTERRUPT_STATUS_BANK0_RW, 0xff); // 7
	reg_write(INTERRUPT_MASK_BANK0_W, INTERRUPT_MASK_PACKET_RECEIVED_ENABLE |
								  INTERRUPT_MASK_PACKET_TRANSMITTED_ENABLE |
								  INTERRUPT_MASK_RECEIVE_ERROR_ENABLE |
								  INTERRUPT_MASK_TRANSMIT_ERROR_ENABLE |
								  INTERRUPT_MASK_OVERWRITE_WARNING_ENABLE); // 8
	reg_write(COMMAND_BANK012_RW, COMMAND_BANK1|COMMAND_REMOTEDMA_ABORT|COMMAND_STOP); // 9
	uint8_t mac[6] = { 0xb0, 0xc4, 0x20, 0x00, 0x00, 0x00 };
	reg_write(PHYSICAL_ADDRESS0_BANK1_RW, mac[0]); // 9i
	reg_write(PHYSICAL_ADDRESS1_BANK1_RW, mac[1]); // 9i
	reg_write(PHYSICAL_ADDRESS2_BANK1_RW, mac[2]); // 9i
	reg_write(PHYSICAL_ADDRESS3_BANK1_RW, mac[3]); // 9i
	reg_write(PHYSICAL_ADDRESS4_BANK1_RW, mac[4]); // 9i
	reg_write(PHYSICAL_ADDRESS5_BANK1_RW, mac[5]); // 9i
	// 9ii writes multicast address, skip for now
	uint8_t my_mac[6];
	my_mac[0] = reg_read(PHYSICAL_ADDRESS0_BANK1_RW);
	my_mac[1] = reg_read(PHYSICAL_ADDRESS1_BANK1_RW);
	my_mac[2] = reg_read(PHYSICAL_ADDRESS2_BANK1_RW);
	my_mac[3] = reg_read(PHYSICAL_ADDRESS3_BANK1_RW);
	my_mac[4] = reg_read(PHYSICAL_ADDRESS4_BANK1_RW);
	my_mac[5] = reg_read(PHYSICAL_ADDRESS5_BANK1_RW);

	reg_write(CURRENT_PAGE_BANK1_RW, INIT_PAGE); // 9iii
	reg_write(COMMAND_BANK012_RW, COMMAND_BANK0|COMMAND_REMOTEDMA_ABORT|COMMAND_START); // 10
	reg_write(TRANSMIT_CONFIGURATION_BANK0_W, TRANSMIT_CONFIGURATION_NORMAL_OPERATION); // 11
	// Now the NIC is ready to receive and transmit.

	kconsole << "Finished initializing NE2000 with MAC " << my_mac[0] << ":" << my_mac[1] << ":" << my_mac[2] << ":" << my_mac[3] << ":" << my_mac[4] << ":" << my_mac[5] << "." << endl;
}

// Handle packet receive overflow.
void ne2k::overflow()
{

}
