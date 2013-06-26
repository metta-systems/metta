set_build_for_target()

include_directories(. x86 ${CMAKE_SOURCE_DIR}/interfaces ${CMAKE_BINARY_DIR}/interfaces)

################################################################################################
# Nucleus.
#
# Privileged mode ring0 nucleus.
# Nucleus is the only ring0 privileged part of the system.
# It includes interrupt handlers and some minimal syscall processing.
################################################################################################

add_component(nucleus
    x86/idt.cpp
    x86/isr.cpp
    x86/interrupt.nasm
    x86/init_nucleus.cpp
    x86/nucleus.cpp
    LINK_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/x86/nucleus.lds
    LIBS common kernel platform debugger minruntime cxx)
