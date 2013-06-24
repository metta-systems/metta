cmake_minimum_required(VERSION 2.8.10)
project(Metta)

include(CTest)

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

# -Wdocumentation not yet enabled - need to clean up llvm source (git pull fresher copy?)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ferror-limit=3 -std=c++11 -stdlib=libc++ -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-private-field")

# Lion has deprecated some OpenSSL stuff we use.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

set(CMAKE_LINK_FLAGS "${CMAKE_LINK_FLAGS} -stdlib=libc++")

include_directories(kernel/generic) # macros.h - huh?
include_directories(runtime) # required_features.h - huh?

add_subdirectory(tools/meddler)
add_subdirectory(tools/mettafs)
