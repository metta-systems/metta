/**
 * @opt operations
 * @opt attributes
 * @opt types
 * @hidden
 */
class UMLOptions {}

/* Define some types we use */
/** @hidden */
class portal_ref_t {}
/** @hidden */
class portal_manager_t {}

class thread_t
{
    void fork(); // start a thread in new space
    void domain_fork(); // inherit portal table and address space pages?
}

class space_t
{
    portal_ref_t[] portal_table;
}

class nucleus_t
{
    thread_t[] threads;

    void generic_trap();
}
