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

include(CTest)

#include(CMakeForceCompiler)
#cmake_force_cxx_compiler(${CMAKE_SOURCE_DIR}/../../toolchain/clang/bin/clang++ Clang)

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
set(RAMTAB_DEBUG 0)
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
macro(add_component name sources)
    add_executable(${name} ${sources})
    set_target_properties(${name} PROPERTIES OUTPUT_NAME ${name}.comp)
    # Components are relocatable
    set_property(TARGET ${name} PROPERTY LINK_FLAGS '-r')
    if (NOT PLATFORM STREQUAL "hosted")
        set_property(TARGET ${name} PROPERTY LINK_FLAGS "-Wl,-T,${CMAKE_SOURCE_DIR}/modules/component.lds -Wl,-Map,${name}.map")
    endif ()
endmacro()


include_directories(kernel/generic) # macros.h - huh?
include_directories(runtime) # required_features.h - huh?

add_subdirectory(tools/meddler)
add_subdirectory(tools/mettafs)
add_subdirectory(tools/buildboot)
#add_subdirectory(tools/parsedwarf)
add_subdirectory(runtime)
add_subdirectory(runtime/libc++)
