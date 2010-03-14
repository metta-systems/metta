//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Component is a gemstone or a facade, providing access to different aspects or interfaces of the object/server.
// Namespaces shorten component pathnames, e.g. "metta:" ns expands to "com.exquance.metta"
//
// dbus like interface specifications
//
// dbus: org.freedesktop.dbus + /dbus/DesktopServices + Notify
// object path + interface + method
//
// metta:vm_server/Kernel.v1#alloc_frame[i]
// namespace:object path/interface#method[paramspec]
//
// you can find the component by namespace and object path from the trader.
// then query interface from the component and call methods on this interface.
// querying interface causes portal manager to build portals between components.
//
// Component interface is further improved by providing a typesafe wrapper
// around portal_set_t, with templates:
// component_if_t<vm_server_interface_v1_t> get_interface("facet_name");
//
// http://en.wikipedia.org/wiki/IBM_System_Object_Model (vs. COM)
// http://en.wikipedia.org/wiki/Distributed_Objects_Everywhere
// http://en.wikipedia.org/wiki/Portable_Distributed_Objects
//
class component_t
{
    portal_set_t* query_interface(symbol_t interface_spec);
    // symbol aka atom is global string representation
    // vs.
    portal_set_t* query_interface(string_t interface_spec);
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
