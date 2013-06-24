set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KERNEL_CXX_FLAGS}")
add_definitions(${KERNEL_DEFINES})
set(CMAKE_LINK_FLAGS "${CMAKE_LINK_FLAGS} ${KERNEL_LINK_FLAGS}")

include_directories(include)

add_library(cxx STATIC src/hash.cpp src/vector_inst.cpp src/string.cpp)
