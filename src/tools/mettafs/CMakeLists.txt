set_build_for_host()

find_library(UUID_LIBRARY uuid PATHS /usr/local/opt/ossp-uuid/lib)
mark_as_advanced(UUID_LIBRARY)
include_directories(/usr/local/opt/ossp-uuid/includes)

include_directories(${CMAKE_SOURCE_DIR}/kernel/arch/x86) # fourcc.h

add_executable(mkmettafs mkfs.cpp block_device.cpp block_cache.cpp block_device_mapper.cpp)
target_link_libraries(mkmettafs ${OPENSSL_LIBRARIES} ${UUID_LIBRARY})
