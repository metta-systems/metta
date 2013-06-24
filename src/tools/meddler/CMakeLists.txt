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

include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/../../toolchain/clang/include) # llvm includes
include_directories(${CMAKE_SOURCE_DIR}/kernel/generic) # macros.h - huh?
add_definitions(-D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS)

# -Wdocumentation not yet enabled - need to clean up llvm source (git pull?)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ferror-limit=3 -std=c++11 -stdlib=libc++ -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-private-field")
set(CMAKE_LINK_FLAGS "${CMAKE_LINK_FLAGS} -stdlib=libc++")

find_library(LLVM_SUPPORT LLVMSupport PATHS ${CMAKE_SOURCE_DIR}/../../toolchain/clang/lib)

add_executable(meddler meddler.cpp parser.cpp lexer.cpp ast.cpp symbol_table.cpp emit_cpp.cpp)
target_link_libraries(meddler ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES} ${LLVM_SUPPORT})
