set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KERNEL_CXX_FLAGS}")
add_definitions(${KERNEL_DEFINES})
set(CMAKE_LINK_FLAGS "${CMAKE_LINK_FLAGS} ${KERNEL_LINK_FLAGS}")

include_directories(libc++/include)
include_directories(../kernel/arch/shared) # for panic.h
include_directories(../kernel/arch/x86) # for ia32.h
include_directories(../kernel/platform/shared) # for default_console.h

list(APPEND runtime_SOURCES memutils.cpp cstring.cpp setjmp.s)
#if (PLATFORM NOT STREQUAL "hosted")
    list(APPEND runtime_SOURCES g++support stdlib newdelete)
#endif ()
add_library(runtime STATIC ${runtime_SOURCES})

# Minruntime is a version of runtime with dynamic memory allocation replaced with dummy implementation.
list(APPEND minruntime_SOURCES dummy_delete.cpp memutils.cpp cstring.cpp)
#if (PLATFORM NOT STREQUAL "hosted")
    list(APPEND minruntime_SOURCES g++support)
#endif ()
add_library(minruntime STATIC ${minruntime_SOURCES})
