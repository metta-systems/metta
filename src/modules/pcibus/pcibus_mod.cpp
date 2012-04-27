#include "pci_bus.h"
#include "closure_interface.h"
#include "closure_impl.h"
#include "default_console.h"

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

void pci_bus_device_t::dump()
{
	kconsole << "PCI device: bus " << bus << ", slot " << slot << ", func " << func << ", vendor " << vendor_id << ", device " << device_id << ", class " << (class_and_subclass >> 8) << ", subclass " << (class_and_subclass & 0xff) << ", header type " << header_type << endl;
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
	kconsole << "  Class ID: " << class2string(class_and_subclass >> 8) << endl;
}

//====
// implement interface closure
//====

static void
entry(closure::closure_t* self)
{
    for ( unsigned int bus=0; bus<256; bus++ ) {
        for ( unsigned int slot=0; slot<32; slot++ ) {
            for ( unsigned int func=0; func<7; func++ ) {
				pci_bus_device_t dev(bus, slot, func);

				if (dev.is_present())
				{
					// kconsole << "PCI device PRESENT!" << endl;
					dev.dump();
				}
				// else
					// break;

                // if ( pci->get_info(pci, bus, slot, func, &info ) < 0 ) break;
                // kprintf ("PCI   : bus:%d slot:%d func:%d  device:%X vendor:%X class:%d:%d\n",
                //          bus, slot, func, info.device_id, info.vendor_id, info.class_code, info.subclass_code );

                // Do stuff here
            }
            // if ( func==0 ) break;
        }
    }

//===============



};

static const closure::ops_t methods =
{
	entry
};

static const closure::closure_t clos =
{
    &methods,
    NULL
};

// EXPORT_CLOSURE_TO_ROOTDOM(pci_bus, v1, clos);
extern "C" const closure::closure_t* const exported_pcibus_rootdom = &clos;
