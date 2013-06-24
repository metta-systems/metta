set_build_for_target()

include_directories(${CMAKE_SOURCE_DIR}/interfaces ${CMAKE_BINARY_DIR}/interfaces ${CMAKE_BINARY_DIR}/interfaces/nemesis)

add_library(debugger STATIC
	arch/${ARCH}/panic.cpp
	arch/${ARCH}/debugger.cpp
	arch/${ARCH}/registers.s)

add_library(common STATIC
    generic/elf_parser.cpp
	generic/console.cpp
	generic/module_loader.cpp)

add_library(kernel STATIC
	arch/${ARCH}/bootinfo.cpp
	arch/${ARCH}/bootimage.cpp
	arch/shared/bootinfo.cpp)

add_library(platform STATIC
	platform/${PLATFORM}/multiboot.cpp
	platform/${PLATFORM}/default_console.cpp)
