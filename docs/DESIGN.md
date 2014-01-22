#### Basic low-level design assumptions

Single address space system - pointers can be passed freely between domains without any translation.
Memory protection implemented via MMU.
There is still non-shared address space between nodes in the network.

Support only single synchronization primitive - the event.
All other primitives are built upon this type by the userspace libraries.

Minimal privileged-mode core.
Only the IRQ handlers and privileged MMU instructions are in this core.

Type system for interfaces/module traits in the system.
Reflection/introspection and schemas/ontologies to let components learn about the surroundings.
