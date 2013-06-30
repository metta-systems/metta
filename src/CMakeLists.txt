# arch (x86, x64, ppc, ppc64) - a generic cpu architecture (isa)
# cpu (p4, arm11jsf) - a specific cpu format
# platform (efi, pc99, hosted) - a generic infrastructure
# board (vulcan, beagleboard) - a specific infrastructure
# board usually implies platform, cpu and arch.
# platform sometimes implies arch.
# build type - release, debug, profile

# platform pc99 = -target i686-pc-elf -m32
# platform u/efi = -target x86_64-apple-elf -m64
# platform hosted = whatever host supports
# board raspi = -target arm-elf-eabi -march=arm6 -mfloat-abi=hard

cmake_minimum_required(VERSION 2.8.11)
project(Metta CXX)

set(CMAKE_ASM_NASM_COMPILER "yasm")
enable_language(ASM_NASM)

include(CTest)
include(CMakeParseArguments)

# Configure arch, cpu, platform, board
# If none specified - use hosted
# If both specified - check they're compatible
# If board specified, infer PLATFORM
# If platform specified, board is either also specified or not known.
# 
if (NOT CONFIG_PLATFORM AND NOT CONFIG_BOARD)
	set(CONFIG_PLATFORM hosted)
endif()

# Board may also set platform to bring in any non-specific details
# Board configs may set TARGET, PLATFORM, BOARD and ARCH variables in _cache_.
if (CONFIG_BOARD)
	include(cmake/boards/${CONFIG_BOARD}.cmake)
endif ()

# Platform configs may set TARGET, PLATFORM, BOARD and ARCH variables in _cache_.
if (CONFIG_PLATFORM)
	include(cmake/platforms/${CONFIG_PLATFORM}.cmake)
endif ()

# TODO: include cmake/arches/${ARCH}.cmake here to set arch-specific flags
# TODO: same for CPU?

message("##   TARGET set to ${TARGET}")
message("##   PLATFORM set to ${PLATFORM}")
message("##   BOARD set to ${BOARD}")
message("##   ARCH set to ${ARCH}")

# Generate our config.h
# PLATFORM, BOARD and ARCH are used here from the platform config files.
set(BUILD ${CMAKE_BUILD_TYPE})
set(CONFIG_INLINING 1)
set(CONFIG_DEBUG_SYMBOLS 1)
set(SYSTEM_DEBUG 1)
set(SYSTEM_VERBOSE_DEBUG 0)
set(HEAP_DEBUG 0)
set(MEMORY_DEBUG 1)
set(BOOTIMAGE_DEBUG 0)
set(DWARF_DEBUG 0)
set(TOOLS_DEBUG 1)
set(MEDDLER_DEBUG 0)
set(CONFIG_COMPORT 0)
set(CONFIG_COMSPEED 115200)
set(CONFIG_X86_PSE 1)
set(CONFIG_X86_PGE 1)
set(CONFIG_X86_FXSR 1)
set(CONFIG_X86_SYSENTER 1)
set(CONFIG_IOAPIC 1)
set(PCIBUS_TEST 1)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(ROOT cd)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tools/mkbootimg/menu.lst.in ${CMAKE_CURRENT_BINARY_DIR}/tools/mkbootimg/menu.lst.cd)

#============================
# Set compiler flags.
#============================

# Limit available path for clang to find proper cross-gcc
set(GCC_PATH ${CMAKE_SOURCE_DIR}/../../toolchain/gcc)

# Flags common for host and kernel environments.

# -Wdocumentation not yet enabled - need to clean up llvm source (git pull fresher copy?)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ferror-limit=3 -std=c++11 -stdlib=libc++ -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-private-field -Wno-unused-variable -Wno-gnu-array-member-paren-init -Wno-gcc-compat -fvisibility-inlines-hidden -fvisibility=hidden")
# Lion has deprecated some OpenSSL stuff we use.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
set(CMAKE_LINK_FLAGS "${CMAKE_LINK_FLAGS} -stdlib=libc++")

# Host environment flags.
set(TOOLS_CXX_FLAGS "-emit-llvm -O4")
set(TOOLS_DEFINES "")
set(TOOLS_LINK_FLAGS "-O4")

# Clang invocations debug
#set(CMAKE_CXX_FLAGS "-v ${CMAKE_CXX_FLAGS}")
#set(CMAKE_LINK_FLAGS "-v -### ${CMAKE_LINK_FLAGS}")

# Kernel environment flags.
# TODO: change -isysroot for kernel mode...
set(KERNEL_CXX_FLAGS "-target i686-pc-elf -m32 -integrated-as -ffreestanding -O0 -g -mno-mmx -mno-sse -mno-sse2 -mno-3dnow -ffunction-sections -fdata-sections -fno-stack-protector -fno-strict-aliasing -fno-rtti -fno-exceptions") # -mno-red-zone -nostdinc
set(KERNEL_DEFINES -D__Metta__=1 -D_LIBCPP_NO_IOSTREAMS=1 -D_LIBCPP_NO_WCHAR=1 -D_LIBCPP_NO_EXCEPTIONS=1 -DBOCHS_IO_HACKS=1)
set(KERNEL_LINK_FLAGS "-target i686-pc-elf -nostdlib")

# Make necessary preparations to switch environment for host build.
macro(set_build_for_host)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TOOLS_CXX_FLAGS}")
	add_definitions(${TOOLS_DEFINES})
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${TOOLS_LINK_FLAGS}")
endmacro()

# Make necessary preparations to switch environment for kernel build.
macro(set_build_for_target)
	include_directories(${CMAKE_SOURCE_DIR}/runtime/libc++/include)
	include_directories(${CMAKE_SOURCE_DIR}/kernel/arch/shared)
	include_directories(${CMAKE_SOURCE_DIR}/kernel/arch/${ARCH})
	include_directories(${CMAKE_SOURCE_DIR}/kernel/platform/shared)
	include_directories(${CMAKE_SOURCE_DIR}/kernel/platform/${PLATFORM})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KERNEL_CXX_FLAGS}")
	add_definitions(${KERNEL_DEFINES})
#	set(CMAKE_EXE_LINKER_FLAGS "-gnu-toolchain ${GCC_PATH} ${CMAKE_EXE_LINKER_FLAGS}")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${KERNEL_LINK_FLAGS}")
endmacro()

# Generate a named component file from given sources.
# Syntax: add_component(name sources [NOT_RELOC] [LINK_SCRIPT script_name] [LIBS lib_targets])
macro(add_component name)
	cmake_parse_arguments(AC "NOT_RELOC" "LINK_SCRIPT" "LIBS" ${ARGN})
	#message("These should be the sources: ${AC_UNPARSED_ARGUMENTS}")
	#message("These should be the libs: ${AC_LIBS}")
	set(sources "${AC_UNPARSED_ARGUMENTS}")

    add_executable(${name} ${sources})
    if (AC_LIBS)
    	target_link_libraries(${name} "${AC_LIBS}")
    endif()
    set_target_properties(${name} PROPERTIES OUTPUT_NAME ${name}.comp)

    set(_link_flags)
    if (NOT AC_NOT_RELOC)
	    # Components are relocatable (-r)
    	set(_link_flags "-r")
    endif()
    if (NOT PLATFORM STREQUAL "hosted")
    	if (AC_LINK_SCRIPT)
    		set(_link_flags "${_link_flags} -Wl,-T,${AC_LINK_SCRIPT}")
        else (AC_LINK_SCRIPT)
    		set(_link_flags "${_link_flags} -Wl,-T,${CMAKE_SOURCE_DIR}/modules/component.lds")
        endif (AC_LINK_SCRIPT)
	    set(_link_flags "${_link_flags} -Wl,-Map,${name}.map")
    endif ()
    set_property(TARGET ${name} PROPERTY LINK_FLAGS ${_link_flags})
    set(all_init_components "${all_init_components};${name}" PARENT_SCOPE)
endmacro()

macro(add_kernel_component)
	add_component(${ARGN} LIBS common kernel component_support debugger platform runtime interfaces cxx)
endmacro()

include_directories(kernel/generic) # macros.h - huh?
include_directories(runtime) # required_features.h - huh?

if (NOT CMAKE_CROSSCOMPILING)
#==============================================================================
# Generate host tools.
#==============================================================================

# On mac, use openssl from brew, not the default system one, because it is too old.
# Run 'brew install openssl' to install it.
if (APPLE)
	set(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
	# Set pkg-config path in case pkg-config is installed on the machine.
	set(ENV{PKG_CONFIG_PATH} /usr/local/opt/openssl/lib/pkgconfig)
	# A bug in cmake prevents use of OPENSSL_ROOT_DIR for finding a custom openssl,
	# so we use an internal variable instead. This needs to be fixed in cmake.
	set(_OPENSSL_ROOT_HINTS_AND_PATHS PATHS /usr/local/opt/openssl)
endif (APPLE)

find_package(OpenSSL REQUIRED)
set(Boost_USE_MULTITHREAD ON)
set(Boost_USE_STATIC_LIBS ON) # Easier to deploy elsewhere
find_package(Boost REQUIRED)


add_subdirectory(tools/meddler)
add_subdirectory(tools/mettafs)
add_subdirectory(tools/buildboot)
#add_subdirectory(tools/parsedwarf)

export(TARGETS meddler buildboot FILE ${CMAKE_BINARY_DIR}/ImportExecutables.cmake)

else()
#==============================================================================
# Generate system files.
#==============================================================================

set(CMAKE_AR ${CMAKE_SOURCE_DIR}/../../toolchain/gcc/i686-pc-elf/bin/ar)
set(CMAKE_RANLIB ${CMAKE_SOURCE_DIR}/../../toolchain/gcc/i686-pc-elf/bin/ranlib)

include_directories(runtime/libc)
add_definitions(-D__STDC_HOSTED__=0)

# From cmake cross-compiling page: import the tools from host build.
set(IMPORT_EXECUTABLES "IMPORTFILE-NOTFOUND" CACHE FILEPATH "Point it to the export file from a native build")
include(${IMPORT_EXECUTABLES})

set(all_init_components) # cmake cannot into dependency management, so do it manually...

add_subdirectory(runtime)
add_subdirectory(runtime/libc++)
add_subdirectory(interfaces)
add_subdirectory(modules)
add_subdirectory(kernel)

#if (NOT PLATFORM STREQUAL "hosted")
add_subdirectory(launcher)
add_subdirectory(nucleus)
#endif()

add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/init.img
    COMMAND buildboot ${CMAKE_BINARY_DIR}/ ${CMAKE_SOURCE_DIR}/kernel/platform/${PLATFORM}/bootimage.lst ${CMAKE_BINARY_DIR}/init.img
	DEPENDS launcher nucleus ${all_init_components})

add_custom_command(OUTPUT metta.iso
    COMMAND sh ${CMAKE_SOURCE_DIR}/tools/mkbootimg/images.sh ${CMAKE_BINARY_DIR}
    DEPENDS launcher ${CMAKE_BINARY_DIR}/init.img ${CMAKE_SOURCE_DIR}/tools/mkbootimg/images.sh ${CMAKE_BINARY_DIR}/tools/mkbootimg/menu.lst.cd)

add_custom_target(metta ALL
	DEPENDS metta.iso)

endif()

# TODO: need to force binary PATH to force clang to find proper gcc toolchain, like
# set -x PATH /Users/berkus/Hobby/Metta/toolchain/gcc/bin /usr/bin /bin /usr/sbin /sbin /usr/local/bin
# Find a way to make it automatic... via cmake command?
