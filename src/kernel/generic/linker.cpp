// OMOS-style operations:
// Merge: binds the symbol definitions found in one operand to the references 
// found in another. Multiple definitions of a symbol constitutes an error.
// Override: merges two operands, resolving conflicting bindings (multiple definitions)
// in favor of the second operand.
// Hide: removes a given set of symbol definitions from the operand symbol table, 
// fixing any internal references to the symbol in the process.
// Show: hides all but a given set of symbol definitions.
// Rename: systematically changes names in the operand symbol table.
// The rename module operation can optionally work on either symbol references, symbol
// definitions or both.
// Copy: duplicates a symbol. The new symbol has the original binding but 
// under a different name.
// Restrict: erases the definition of a symbol and adds a symbol as a pure virtual
// to the object interface of the class.
// Project: restricts all but a given set of symbols.
// List: associate two or more objects in a list.
// Constrain: constraints the virtual address ranges the operand(s) may occupy.
// Annotate: prints an informational message.
// Source: produces a module from a source object.

namespace linker {

/**
 * linker object represents a linkable module.
 */
class object_t
{
    list<Symbol> symbols;
    // symbols, sections and names (strtab)
};

class operation_t
{
};

/**
 * merge_op_t is the simple basic linker operation, minimum necessary for linking.
 */
class merge_op_t : public operation_t
{
};

}
