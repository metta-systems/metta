set_build_for_target()

list(APPEND runtime_SOURCES memutils.cpp cstring.cpp setjmp.nasm)
if (NOT PLATFORM STREQUAL "hosted")
    list(APPEND runtime_SOURCES g++support.cpp stdlib.cpp newdelete.cpp)
endif ()
add_library(runtime STATIC ${runtime_SOURCES})

# Minruntime is a version of runtime with dynamic memory allocation replaced with dummy implementation.
list(APPEND minruntime_SOURCES dummy_delete.cpp memutils.cpp cstring.cpp)
if (NOT PLATFORM STREQUAL "hosted")
    list(APPEND minruntime_SOURCES g++support.cpp)
endif ()
add_library(minruntime STATIC ${minruntime_SOURCES})
