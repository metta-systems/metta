[See Cap'n Proto](http://kentonv.github.io/capnproto/language.html#interfaces) for RPC protocol description.

autodoc comments should be spilled into generated `_interface.h` or `_interface.cpp` files and then processed by doxygen?

also, autodoc comments should be entered into the typedefs descriptions table, so that tools like Monger could display it as inline docs.


First we parse the idl into some sort of AST.

Type information should be entered into the global type table.

Then we do lookups for types, checking if type is builtin.
If it is not - it should be a fully qualified type within the same or another interface.
In the same interface, no extra includes needed but the order of definitions should be maintained, so that types 
are defined or declared before first use.
For different interface an include of `"<interface name>_interface.h"` is required to bring in type declarations.

Then the type mapping is adjusted - `<interface name>.<type>` is converted into `<interface name>_<type>` for C or maybe to `<interface name as namespace>::<type>` for C++?

**to change** C++ generator should not be concerned with C interface problems as long as interface is binary compatible.


We check the imported types while parsing field, parameter declarations - if a type is not builtin or fully qualified, it is considered local type, subject to intra-interface resolution, otherwise if type is fully qualified it is regarded as type import and added to interface's `imported_types` list.
During code emission inclusion of other interfaces information is resolved (for C++ - by adding `#include "importname_interface.h"` lines).


#### Type code generation

* need to maintain structural equivalence - types with same structure and methods should generate same code (fingerprint).

For simplicity we require same naming on methods and composite types (but not on field names or method arguments).

interface name and parent interface name below break structural equivalence requirement!

`interface_name{parent_interface_name{types;exceptions;methods;}types;exceptions;methods;}`
