namespace nucleus
{

/*!
Minimal supervisor-mode nucleus, responsible for little more than context switches.

- Most functionality is provided by servers that execute in user mode without special privileges.
- The kernel is responsible only for switching between protection domains.
- If something can be run at user level, it is.
*/
class nucleus_t
{
public:
    void enter_trap(int portal_no); // called from assembler glue code to process client trap and call corresponding portal
    void create_pd(); // portal to create new address space and assign it to a pd
    void destroy_pd();
private:
    vector_base<pd_t> spaces;
};

}
