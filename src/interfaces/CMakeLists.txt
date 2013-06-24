set_build_for_target()

# Interfaces sorted alphabetically.
list(APPEND interface_files
    activation_dispatcher_factory_v1
    activation_dispatcher_v1
    activation_v1
    binder_v1
    binder_callback_v1
    chained_handler_v1
    channel_notify_v1
    channel_v1
    choice_v1
    closure
    domain_v1
    enum_v1
    event_v1
    events_v1
    exports_table_v1
    fault_handler_v1
    frame_allocator_v1
    frames_module_v1
    gatekeeper_v1
    gatekeeper_factory_v1
    heap_v1
    heap_factory_v1
    idc_v1
    idc_client_binding_v1
    idc_offer_v1
    idc_service_v1
    interface_v1
    map_card64_address_v1
    map_card64_address_factory_v1
    map_string_address_v1
    map_string_address_factory_v1
    map_string_address_iterator_v1
    memory_v1
    mmu_module_v1
    mmu_v1
    naming_context_v1
    naming_context_factory_v1
    operation_v1
    pervasives_v1
    protection_domain_v1
    ramtab_v1
    record_v1
    stretch_allocator_module_v1
    stretch_allocator_v1
    stretch_driver_module_v1
    stretch_driver_v1
    stretch_table_v1
    stretch_table_module_v1
    stretch_v1
    system_frame_allocator_v1
    system_stretch_allocator_v1
    threads_factory_v1
    thread_hooks_v1
    thread_v1
    threads_manager_v1
    threads_v1
    time_notify_v1
    time_v1
    timer_v1
    types
    type_system_v1
    type_system_f_v1
    type_system_factory_v1
    vcpu_v1
    nemesis/exception_v1
    nemesis/exception_support_v1
    nemesis/exception_support_setjmp_v1
    nemesis/exception_system_v1
)

foreach (src ${interface_files})
    get_filename_component(src_path "${src}" PATH)
    add_custom_command(OUTPUT
        ${src}_impl.h
        ${src}_interface.h
        ${src}_interface.cpp
        ${src}_typedefs.cpp
        COMMAND
        meddler -o=${CMAKE_CURRENT_BINARY_DIR}/${src_path} -I=${CMAKE_CURRENT_SOURCE_DIR} -I=${CMAKE_CURRENT_SOURCE_DIR}/nemesis ${CMAKE_CURRENT_SOURCE_DIR}/${src}.if
        DEPENDS meddler
        MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${src}.if)
    list(APPEND interface_repo_files
        ${CMAKE_CURRENT_BINARY_DIR}/${src}_typedefs.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/${src}_interface.h)
endforeach()
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/nemesis ${CMAKE_CURRENT_SOURCE_DIR})

# Generate all interface files before starting compile, cmake can't track includes
# dependency in generated source files :|
add_custom_target(prepare_files DEPENDS ${interface_repo_files})

add_component(interface_repository "${interface_repo_files}")
add_dependencies(interface_repository prepare_files)
