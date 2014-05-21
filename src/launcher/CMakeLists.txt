set_build_for_target()

include_directories(. ${CMAKE_SOURCE_DIR}/nucleus/${ARCH}
    ${CMAKE_SOURCE_DIR}/modules/tcb/root_domain
    ${CMAKE_SOURCE_DIR}/interfaces
    ${CMAKE_BINARY_DIR}/interfaces)

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

set(LAUNCHER_SRC
    loader.cpp)

if (PLATFORM STREQUAL "pc99")
    list(APPEND LAUNCHER_SRC
        pc99/loader.nasm
        pc99/loader-ia32.cpp
        pc99/loader-multiboot.cpp
        pc99/multiboot-ia32.cpp)
    set(LAUNCHER_LDSCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/pc99/launcher.lds)
endif (PLATFORM STREQUAL "pc99")

if (PLATFORM STREQUAL "uefi")
    list(APPEND LAUNCHER_SRC
        uefi/loader-uefi.cpp)
    set(LAUNCHER_LDSCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/uefi/launcher.lds)
endif (PLATFORM STREQUAL "uefi")

if (ARCH STREQUAL "x86")
    list(APPEND LAUNCHER_SRC
        x86/startup.cpp
        ../kernel/arch/x86/bootinfo.cpp
        ../kernel/arch/x86/continuation.nasm)
endif (ARCH STREQUAL "x86")

if (ARCH STREQUAL "x86_64")
    list(APPEND LAUNCHER_SRC
        x86_64/startup.cpp)
endif (ARCH STREQUAL "x86")

add_component(launcher
    ${LAUNCHER_SRC}
    NOT_RELOC # Launcher is not relocatable.
    LINK_SCRIPT ${LAUNCHER_LDSCRIPT}
    LIBS common platform kernel runtime debugger interfaces cxx)
