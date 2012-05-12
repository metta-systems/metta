//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "ne2k.h"
#include "card_registers.h"
#include "default_console.h"
#include "pci_bus.h"
#include "cpu.h"
#include "pic.h"
#include "nucleus.h"

using namespace ne2k_card;

// Packet ring buffer offsets (from tyndur)
#define PAGE_TX     0x40
#define PAGE_RX     0x50
#define PAGE_STOP   0x80


void ne2k::irq_handler::run(registers_t*)
{
    parent->handle_irq();
}

void ne2k::handle_irq()
{
    uint8_t reason = reg_read(INTERRUPT_STATUS_BANK0_RW);
    uint8_t handled_reasons = 0;

    if (reason & INTERRUPT_STATUS_OVERWRITE_WARNING)
    {
        overflow();
        handled_reasons |= INTERRUPT_STATUS_OVERWRITE_WARNING;
    }

    if (reason & INTERRUPT_STATUS_RECEIVE_ERROR)
    {
        receive_error();
        handled_reasons |= INTERRUPT_STATUS_RECEIVE_ERROR;
    }
    if (reason & INTERRUPT_STATUS_PACKET_RECEIVED)
    {
        reg_write(INTERRUPT_MASK_BANK0_W, INTERRUPT_MASK_ENABLE_ALL_EXCEPT_RECEIVE);
        packet_received();
        reg_write(INTERRUPT_MASK_BANK0_W, INTERRUPT_MASK_ENABLE_ALL);
        handled_reasons |= INTERRUPT_STATUS_PACKET_RECEIVED;
    }

    if (reason & INTERRUPT_STATUS_TRANSMIT_ERROR)
    {
        transmit_error();
        handled_reasons |= INTERRUPT_STATUS_TRANSMIT_ERROR;
    }
    if (reason & INTERRUPT_STATUS_PACKET_TRANSMITTED)
    {
        packet_transmitted();
        handled_reasons |= INTERRUPT_STATUS_PACKET_TRANSMITTED;
    }

    if (handled_reasons != 0)
        reg_write(INTERRUPT_STATUS_BANK0_RW, handled_reasons); // Clear all interrupt reasons that we've handled above.
}

void ne2k::reg_write(int regno, uint8_t value)
{
    x86_cpu_t::outb(port_base + regno, value);
}

void ne2k::reg_write_word(int regno, uint16_t value)
{
    x86_cpu_t::outw(port_base + regno, value);
}

uint8_t ne2k::reg_read(int regno)
{
    return x86_cpu_t::inb(port_base + regno);
}

uint16_t ne2k::reg_read_word(int regno)
{
    return x86_cpu_t::inw(port_base + regno);
}

#define BAR0 0x10
#define INTERRUPT 0x3c

void ne2k::configure(pci_device_t* card)
{
    // read PCI BARs and figure out our base address and IRQ
    uint32_t bar0 = card->read_config_space(BAR0);
    uint32_t intr = card->read_config_space(INTERRUPT);//replace with card->interrupt_line

    if (bar0 & 1)
    {
        port_base = bar0 & ~3;
        kconsole << "This ne2k supports I/O port access, with port base 0x" << (uint32_t)port_base << "." << endl;
    }
    else
    {
        kconsole << "WARNING: This device doesn't use I/O, cannot support it yet." << endl;
        port_base = 0xffff;
        return;
    }
    irq = intr & 0xff;
    kconsole << "This ne2k uses irq line " << irq << endl;

    nucleus::install_irq_handler(irq, &handler);
}

// ne2k card initialization sequence
void ne2k::init()
{
    kconsole << "Initializing NE2000." << endl;
    reg_write(COMMAND_BANK012_RW, COMMAND_BANK0|COMMAND_REMOTEDMA_ABORT|COMMAND_STOP); // 1
    reg_write(DATA_CONFIGURATION_BANK0_W, DATA_CONFIGURATION_NON_LOOPBACK_SELECT |
                                          DATA_CONFIGURATION_WORD_TRANSFER_SELECT |
                                          DATA_CONFIGURATION_AUTO_INITIALIZE_REMOTE | // bochs doesn't support?? [NE2K ] DCR write - AR set ???
                                          DATA_CONFIGURATION_FIFO_THRESHOLD_1WORD); // 2
    reg_write(REMOTE_BYTE_COUNT0_BANK0_W, 0); // 3
    reg_write(REMOTE_BYTE_COUNT1_BANK0_W, 0); // 3
    reg_write(RECEIVE_CONFIGURATION_BANK0_W, 0); // 4
    reg_write(TRANSMIT_CONFIGURATION_BANK0_W, TRANSMIT_CONFIGURATION_INTERNAL_LOOPBACK); // 5
    reg_write(BOUNDARY_POINTER_BANK0_RW, PAGE_RX); // 6
    reg_write(PAGE_START_BANK0_W, PAGE_RX); // 6
    reg_write(PAGE_STOP_BANK0_W, PAGE_STOP); // 6
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

    reg_write(MULTICAST_ADDRESS0_BANK1_RW, 0xff); // 9ii
    reg_write(MULTICAST_ADDRESS1_BANK1_RW, 0xff); // 9ii
    reg_write(MULTICAST_ADDRESS2_BANK1_RW, 0xff); // 9ii
    reg_write(MULTICAST_ADDRESS3_BANK1_RW, 0xff); // 9ii
    reg_write(MULTICAST_ADDRESS4_BANK1_RW, 0xff); // 9ii
    reg_write(MULTICAST_ADDRESS5_BANK1_RW, 0xff); // 9ii
    reg_write(MULTICAST_ADDRESS6_BANK1_RW, 0xff); // 9ii
    reg_write(MULTICAST_ADDRESS7_BANK1_RW, 0xff); // 9ii

    uint8_t my_mac[6];
    my_mac[0] = reg_read(PHYSICAL_ADDRESS0_BANK1_RW);
    my_mac[1] = reg_read(PHYSICAL_ADDRESS1_BANK1_RW);
    my_mac[2] = reg_read(PHYSICAL_ADDRESS2_BANK1_RW);
    my_mac[3] = reg_read(PHYSICAL_ADDRESS3_BANK1_RW);
    my_mac[4] = reg_read(PHYSICAL_ADDRESS4_BANK1_RW);
    my_mac[5] = reg_read(PHYSICAL_ADDRESS5_BANK1_RW);

    next_packet = PAGE_RX + 1;
    reg_write(CURRENT_PAGE_BANK1_RW, next_packet); // 9iii

    // Go back to bank 0
    reg_write(COMMAND_BANK012_RW, COMMAND_BANK0|COMMAND_REMOTEDMA_ABORT|COMMAND_STOP);

    // Enable full promiscuity for tesing.
    reg_write(RECEIVE_CONFIGURATION_BANK0_W, RECEIVE_CONFIGURATION_ACCEPT_RUNT_PACKETS |
                                             RECEIVE_CONFIGURATION_ACCEPT_BROADCAST |
                                             RECEIVE_CONFIGURATION_ACCEPT_MULTICAST |
                                             RECEIVE_CONFIGURATION_PROMISCUOUS_PHYSICAL);

    // Clear pending interrupts, enable them all, and begin card operation
    reg_write(INTERRUPT_STATUS_BANK0_RW, INTERRUPT_STATUS_CLEAR_ALL);
    reg_write(INTERRUPT_MASK_BANK0_W, INTERRUPT_MASK_ENABLE_ALL);

    // Configure RemoteDMA to SendPacket mode, which will transfer data automatically into configured buffer space.
    reg_write(COMMAND_BANK012_RW, COMMAND_BANK0|COMMAND_REMOTEDMA_ABORT|COMMAND_START); // 10
    reg_write(TRANSMIT_CONFIGURATION_BANK0_W, TRANSMIT_CONFIGURATION_NORMAL_OPERATION); // 11
    // Now the NIC is ready to receive and transmit.

    ia32_pic_t::enable_irq(irq);

    kconsole << "Finished initializing NE2000 with MAC " << my_mac[0] << ":" << my_mac[1] << ":" << my_mac[2] << ":" << my_mac[3] << ":" << my_mac[4] << ":" << my_mac[5] << "." << endl;
}

// Handle packet receive overflow.
void ne2k::overflow()
{
    kconsole << "Receive buffer overflow." << endl;
}

void ne2k::receive_error()
{
    kconsole << "Receive error." << endl;
}

void ne2k::packet_received()
{
    kconsole << "Packet received." << endl;

    // Get packet off of card onto the host and dump it to screen.
    reg_write(COMMAND_BANK012_RW, COMMAND_BANK1|COMMAND_REMOTEDMA_ABORT|COMMAND_STOP);
    uint8_t current = reg_read(CURRENT_PAGE_BANK1_RW);
    reg_write(COMMAND_BANK012_RW, COMMAND_BANK0|COMMAND_REMOTEDMA_ABORT|COMMAND_STOP);

    while (next_packet != current)
    {
        reg_write(REMOTE_START_ADDRESS0_BANK0_W, 0);
        reg_write(REMOTE_START_ADDRESS1_BANK0_W, next_packet);
        // 4 bytes - 2 for status, 2 for length
        reg_write(REMOTE_BYTE_COUNT0_BANK0_W, 4);
        reg_write(REMOTE_BYTE_COUNT1_BANK0_W, 0);

        reg_write(COMMAND_BANK012_RW, COMMAND_BANK0|COMMAND_REMOTEDMA_READ|COMMAND_START);

        uint16_t status = reg_read_word(DATA_PORT_BANK012_RW);
        uint16_t length = reg_read_word(DATA_PORT_BANK012_RW);

        kconsole << "Received packet with status " << (status & 0xff) << " of length " << length << ", next packet at " << (status >> 8) << endl;

        if(!length) {
            break;
        }
        // Remove status and length
        length -= 4;
#define MIN_ETH_FRAME_LENGTH (6+6+2+4)
        if (length >= MIN_ETH_FRAME_LENGTH)
        {
            // Verify status
            while(!(reg_read(INTERRUPT_STATUS_BANK0_RW) & INTERRUPT_STATUS_REMOTEDMA_COMPLETE));
            reg_write(INTERRUPT_STATUS_BANK0_RW, INTERRUPT_STATUS_REMOTEDMA_COMPLETE);
            // Read the packet
            reg_write(REMOTE_START_ADDRESS0_BANK0_W, 4);
            reg_write(REMOTE_START_ADDRESS1_BANK0_W, next_packet);
            reg_write(REMOTE_BYTE_COUNT0_BANK0_W, length & 0xff);
            reg_write(REMOTE_BYTE_COUNT1_BANK0_W, (length >> 8) & 0xff);
            reg_write(COMMAND_BANK012_RW, COMMAND_BANK0|COMMAND_REMOTEDMA_READ|COMMAND_START);
            uint16_t data[(length / 2) + 1];
            int i, words = (length / 2);
            for(i = 0; i < words; ++i) {
                data[i] = reg_read_word(DATA_PORT_BANK012_RW);
            }
            if(length & 1) {
                data[i] = reg_read(DATA_PORT_BANK012_RW);
            }
            // Verify status
            while(!(reg_read(INTERRUPT_STATUS_BANK0_RW) & INTERRUPT_STATUS_REMOTEDMA_COMPLETE));
            reg_write(INTERRUPT_STATUS_BANK0_RW, INTERRUPT_STATUS_REMOTEDMA_COMPLETE);

            next_packet = status >> 8;

            reg_write(BOUNDARY_POINTER_BANK0_RW, (next_packet == PAGE_RX) ? (PAGE_STOP - 1) : (next_packet - 1));
            debugger_t::dump_memory((address_t)data, length);
        }
    }
}

void ne2k::transmit_error()
{
    kconsole << "Transmit error." << endl;
}

void ne2k::packet_transmitted()
{
    kconsole << "Packet transmitted." << endl;
}

/*
 * buf must contain properly prepared ethernet packet.
 * Source and destination addresses, length and type should be filled in the preamble.
 * CRC will be added by the card.
 */
void ne2k::send_packet(void* buf, uint16_t size)
{
    // Send the length/address for this write
    reg_write(REMOTE_START_ADDRESS0_BANK0_W, 0);
    reg_write(REMOTE_START_ADDRESS1_BANK0_W, PAGE_TX);
    reg_write(REMOTE_BYTE_COUNT0_BANK0_W, (size > 64) ? (size & 0xff) : 64);
    reg_write(REMOTE_BYTE_COUNT1_BANK0_W, size >> 8);
    reg_write(COMMAND_BANK012_RW, COMMAND_BANK0|COMMAND_REMOTEDMA_WRITE|COMMAND_START);
    // Write to the NIC
    uint16_t *p = (uint16_t*)buf;
    size_t i;
    for(i = 0; (i + 1) < size; i += 2) {
        reg_write_word(DATA_PORT_BANK012_RW, p[i/2]);
    }
    // Handle odd bytes
    if(size & 1) {
        reg_write(DATA_PORT_BANK012_RW, p[i/2]);
        i++;
    }
    // Pad to 64 bytes, if needed
    for(; i < 64; i += 2) {
        reg_write_word(DATA_PORT_BANK012_RW, 0);
    }
    // Await the transfer completion and then transmit
    while(!(reg_read(INTERRUPT_STATUS_BANK0_RW) & INTERRUPT_STATUS_REMOTEDMA_COMPLETE));
    reg_write(INTERRUPT_STATUS_BANK0_RW, INTERRUPT_STATUS_REMOTEDMA_COMPLETE);

    reg_write(TRANSMIT_BYTE_COUNT0_BANK0_W, (size > 64) ? (size & 0xff) : 64);
    reg_write(TRANSMIT_BYTE_COUNT1_BANK0_W, size >> 8);
    reg_write(TRANSMIT_PAGE_START_ADDRESS_BANK0_W, PAGE_TX);
    reg_write(COMMAND_BANK012_RW, COMMAND_BANK0|COMMAND_REMOTEDMA_ABORT|COMMAND_TRANSMIT_PACKET|COMMAND_START);
}









