set_build_for_host()

# ===== LLVM recommended detection and configuration =====
# See http://llvm.org/docs/CMake.html#embedding-llvm-in-your-project
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(LLVM_LIBS support)
# ===== end LLVM recommended detection and configuration =====

include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIR})
add_definitions(-D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS)

add_executable(meddler meddler.cpp parser.cpp lexer.cpp ast.cpp symbol_table.cpp emit_cpp.cpp)
target_link_libraries(meddler ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES} ${LLVM_LIBS})
