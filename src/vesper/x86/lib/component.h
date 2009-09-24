// Component is a gemstone or a facade, providing access to different aspects or interfaces
// of the object/server.
// Namespaces shorten component pathnames, e.g. "metta:" ns expands to "com.exquance.metta"
//
// dbus like interface specifications
//
// dbus: org.freedesktop.dbus + /dbus/DesktopServices + Notify
// object path + interface + method
//
// metta:vm_server/Kernel.v1#alloc_frame
// namespace:object path/interface#method[paramspec]
//
// you can find the component by namespace and object path from the trader.
// then query interface from the component and call methods on this interface.
// querying interface causes portal manager to build portals between components.
//
class component_t
{
    portal_set_t* query_interface(symbol_t interface_spec);
    // symbol aka atom is global string representation
    // vs.
    portal_set_t* query_interface(string_t interface_spec);
};
