include_directories(.)

add_kernel_component(pcibus_mod pcibus_mod.cpp ../../devices/network/ne2000_pci/ne2k.cpp ../../devices/graphics/bochs_emu/bga.cpp)
