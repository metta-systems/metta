set_build_for_target()

include_directories(.)
include_directories(../nucleus ../nucleus/${ARCH})
include_directories(${CMAKE_SOURCE_DIR}/interfaces ${CMAKE_BINARY_DIR}/interfaces ${CMAKE_BINARY_DIR}/interfaces/nemesis)
add_library(component_support module-entry.cpp heap_new.cpp)

add_subdirectory(tcb)
add_subdirectory(heap_mod)
add_subdirectory(context_mod)
add_subdirectory(hashtables_mod)
add_subdirectory(stretch_table_mod)
add_subdirectory(exceptions_mod)
add_subdirectory(pcibus)
