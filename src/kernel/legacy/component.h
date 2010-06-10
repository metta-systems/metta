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
// See http://developer.apple.com/mac/library/documentation/Cocoa/Conceptual/DistrObjects/Concepts/messaging.html for
// information on (de)marshalling method invocations.
//
// http://en.wikipedia.org/wiki/Internet_Communications_Engine zeroc ice has marshalling and QoS in IceStorm
//
// SOM prevents fragile base class problem by providing only late binding, to allow the run-time linker to re-build
// the table on the fly. This way, changes to the underlying libraries are resolved when they are loaded into programs,
// although there is a performance cost.
//
// The SOM description for "types and classes" is essentially the same as that described in the OODBTG Reference Model
// entry in this section (http://www.objs.com/x3h7/oodbtg.htm) in that a "type" defines a protocol shared by a group
// of objects, called "instances" of the type and a class defines an implementation shared by a group of objects.
//
// In SOM all classes are real objects. SOM supports a class object which represents the metaclass for the creation
// of all SOM classes. The SOM metaclass defines the behavior common to all class objects. Since it inherits from
// the root SOM object it exists at run time and contains the methods for manufacturing object instances.
// It also has the methods used to dynamically obtain information about a class and its methods at run time.
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
