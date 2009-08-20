#include "debugger.h"
#include "default_console.h"
#include "registers.h"

void debugger_t::dump_memory(address_t start, size_t size)
{
    char *ptr = (char *)start;
    int run;

    // Silly limitation, probably.
    if (size > 256)
        size = 256;

    kconsole.newline();

    while (size > 0)
    {
        kconsole.print((unsigned int)ptr);
        kconsole.print("  ");
        run = size < 16 ? size : 16;
        for(int i = 0; i < run; i++)
        {
            kconsole.print((unsigned char)*(ptr+i));
            kconsole.print(' ');
            if (i == 7)
                kconsole.print(' ');
        }
        if (run < 16)// pad
        {
            if(run < 8)
                kconsole.print(' ');
            for(int i = 0; i < 16-run; i++)
                kconsole.print("   ");
        }
        kconsole.print(' ');
        for(int i = 0; i < run; i++)
        {
            char c = *(ptr+i);
            if (c == kconsole.eol)
                c = ' ';
            kconsole.print(c);
        }
        kconsole.newline();
        ptr += run;
        size -= run;
    }
}

address_t debugger_t::backtrace(address_t base_pointer, address_t& return_address)
{
    // We take a stack base pointer (in base_bointer), return what it's pointing at
    // and put the address just above it in the stack in return_address.
    address_t next_base = *((address_t*)base_pointer);
    return_address    = *((address_t*)(base_pointer+sizeof(address_t)));
    return next_base;
}

address_t debugger_t::backtrace(int n)
{
    address_t base_pointer = read_base_pointer();
    address_t eip = 1;
    int i = 0;
    while (base_pointer && eip /*&& eip < 0x87000000*/)
    {
        base_pointer = backtrace(base_pointer, eip);
        if (i == n)
        {
            return eip;
        }
        i++;
    }
    return 0;
}

void debugger_t::print_backtrace(address_t base_pointer, int n)
{
    address_t eip = 1; // Don't initialise to 0, will kill the loop immediately.
    if (base_pointer == 0)
    {
        base_pointer = read_base_pointer();
    }
    kconsole << GREEN << "*** Backtrace *** Tracing ";
    if (n == 0)
        kconsole << "all";
    else
        kconsole << n;
    kconsole << " stack frames:" << endl;
    int i = 0;
    while (base_pointer && eip &&
        ( (n && i<n) || !n) &&
        eip < 0x87000000)//FIXME higher half kernel!
    {
        base_pointer = backtrace(base_pointer, eip);
//         unsigned int offset;
//         char *symbol = kernel_elf_parser.find_symbol(eip, &offset);
//         offset = eip - offset;
        kconsole << "| " << (unsigned)eip << endl; //" <" << (symbol ? symbol : "UNRESOLVED") << "+0x" << offset << ">" << endl;
        i++;
    }
}

void debugger_t::print_stacktrace(unsigned int n)
{
    address_t esp = read_stack_pointer();
    address_t esp_base = esp;
    kconsole << GREEN << "<ESP=" << esp << ">" << endl;
    for (unsigned int i = 0; i < n; i++)
    {
        kconsole << "<ESP+" << (int)(esp - esp_base) << "> " << *(address_t*)esp << endl;
        esp += sizeof(address_t);
    }
}
