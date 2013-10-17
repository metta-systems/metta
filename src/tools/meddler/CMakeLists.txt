set_build_for_host()

#======== from LLVM wiki =========
# A convenience variable:
set(LLVM_ROOT "" CACHE PATH "Root of LLVM install.")

# A bit of a sanity check:
if( NOT EXISTS ${LLVM_ROOT}/include/llvm )
message(FATAL_ERROR "LLVM_ROOT (${LLVM_ROOT}) is not a valid LLVM install")
endif()

# We incorporate the CMake features provided by LLVM:
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${LLVM_ROOT}/share/llvm/cmake")
include(LLVMConfig)

# Now set the header and library paths:
include_directories( ${LLVM_INCLUDE_DIRS} )
link_directories( ${LLVM_LIBRARY_DIRS} )
add_definitions( ${LLVM_DEFINITIONS} )

llvm_map_components_to_libraries(LLVM_SUPPORT option)
#=========

include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIR})
add_definitions(-D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS)

add_executable(meddler meddler.cpp parser.cpp lexer.cpp ast.cpp symbol_table.cpp emit_cpp.cpp)
target_link_libraries(meddler ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES} ${LLVM_SUPPORT})
