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
private:
    vector_base<pd_t> spaces;
};

}
