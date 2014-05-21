set_build_for_host()

include_directories(${CMAKE_SOURCE_DIR}/kernel/arch/x86) # fourcc.h
include_directories(${Boost_INCLUDE_DIR})

add_executable(buildboot buildboot.cpp)
target_link_libraries(buildboot ${Boost_LIBRARIES})
