set_build_for_target()

include_directories(include)

add_library(cxx STATIC src/hash.cpp src/vector_inst.cpp src/string.cpp)
