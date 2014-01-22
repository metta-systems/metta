#### Basic low-level design assumptions

*Single address space system* - pointers can be passed freely between domains without any translation.
Memory protection implemented via MMU.
There is still non-shared address space between nodes in the network.

Support only *single synchronization primitive - the event*.
All other primitives are built upon this type by the userspace libraries.

*Minimal privileged-mode core*.
Only the IRQ handlers and privileged MMU instructions are in this core.
Rest of the system runs with user-space privileges, even the device drivers - although they are given some freedom
in accessing certain ports and performing memory mappings through the MMU control interface.

*Type system* for interfaces/module traits in the system.
Reflection/introspection and schemas/ontologies to let components learn about the surroundings.

System is built-up from the smaller interconnected components, which are linked using *interfaces*.
System is defined by semantics of these interfaces and expected effects of calling methods on these interfaces.
The actual implementations may vary and be replaceable.

Most of the system components shall be *replaceable at run time*.
Ideally, the privileged core should also be possible to swap out.
