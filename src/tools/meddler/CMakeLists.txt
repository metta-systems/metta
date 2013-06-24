include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/../../toolchain/clang/include) # llvm includes
add_definitions(-D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS)

find_library(LLVM_SUPPORT LLVMSupport PATHS ${CMAKE_SOURCE_DIR}/../../toolchain/clang/lib)

add_executable(meddler meddler.cpp parser.cpp lexer.cpp ast.cpp symbol_table.cpp emit_cpp.cpp)
target_link_libraries(meddler ${OPENSSL_LIBRARIES} ${Boost_LIBRARIES} ${LLVM_SUPPORT})
