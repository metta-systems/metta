//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <stdlib.h>
#include <stdexcept>
#include "raiifile.h"
#include "elf_parser.h"
#include "leb128.h"
#include "dwarf_parser.h"
#include "dwarf_aranges.h"
#include "dwarf_abbrev.h"
#include "dwarf_info.h"

using namespace std;
using namespace raii_wrapper;
using namespace elf32; // FIXME: only elf32 is supported, will fail on x86-64

string strmid(string str, int pos, int len)
{
    string s;
    try {
        s = str.substr(pos, len);
    } catch(std::out_of_range&) {
    }
    return s;
}

void panic(const char* s)
{
    printf("%s\n", s);
    exit(-1);
}

int main(int argc, char** argv)
{
    if (argc < 4)
        throw runtime_error("usage: parsedwarf format logfile elf_with_debug\nformat = metta");

    string format(argv[1]);

    // Load binary file with debug info.
    // TODO: use some sort of mmap()ed access to reduce memory usage.
    file f(argv[3], fstream::in);
    size_t fsize = f.size();
    char* buffer = new char [fsize];
    f.read(buffer, fsize);
    address_t start = reinterpret_cast<address_t>(buffer);
    elf_parser_t elf(start);
    dwarf_parser_t dwarf(elf);

    // start parsing the log file
    if (format == string("metta"))
    {
        /*
        *** Backtrace *** Tracing all stack frames:
        | 0x0010380f
        | 0x00103898
        | 0x0010398e
        | 0x00103ae1
        | 0x0010343f
        | 0x00103627
        | 0x0010347b
        | 0x001033d7
        | 0x001032b6
        | 0x001030b6
        | 0x00102d7e // dwarf_parser_t bugs out on this and next line, FIXME
        | 0x001029fe
        */
        string str;
        ifstream input(argv[2], ios::in);
        int line = 0;
        bool in_stack_dump = false;

        while (getline(input, str))
        {
            line++;

            if (in_stack_dump)
            {
                if (str.find("| ") == 0)
                {
                    address_t addr = strtoul(strmid(str, 2, 10).c_str(), NULL, 0);
                    dwarf.lookup(addr);
                }
                else
                    in_stack_dump = false;
            }

            if (str.find("*** Backtrace ***") == 0)
            {
                in_stack_dump = true;
            }
        }
    }
    else
        throw runtime_error("Invalid log format specified!");

    return 0;
}
