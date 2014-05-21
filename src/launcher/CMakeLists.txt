set_build_for_target()

include_directories(. ${CMAKE_SOURCE_DIR}/nucleus/x86 ${CMAKE_SOURCE_DIR}/modules/tcb/root_domain ${CMAKE_SOURCE_DIR}/interfaces ${CMAKE_BINARY_DIR}/interfaces)

# Initial loader.
# Initial loader provides boot sequence abstraction for the kernel.
# It may also choose which kernel to boot depending on the environment.

################################################################################################
# Launcher.
#
# Launcher sets up a necessary environment (for example device trees),
# pulls a necessary version of the nucleus from the boot image, initializes it,
# pulls a right copy of root domain from the boot image and passes control to it, switching to ring3 along the way.
# The privileged execution ends with the launcher and the nucleus.
# The rest of the loading is done by the root domain.
################################################################################################

add_component(launcher
    pc99/loader.nasm
    pc99/loader-ia32.cpp
    pc99/loader-multiboot.cpp
    pc99/multiboot-ia32.cpp
    loader.cpp
    x86/startup.cpp
    ../kernel/arch/x86/bootinfo.cpp
    ../kernel/arch/x86/continuation.nasm
    NOT_RELOC # Launcher is not relocatable.
    LINK_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/pc99/launcher.lds
    LIBS common platform kernel runtime debugger interfaces cxx)
