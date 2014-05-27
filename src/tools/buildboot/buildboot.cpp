//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
/**
 * Read file with image description and create corresponding boot image.
 *
 * Run with:
 * buildboot _build_/x86-pc99-release/modules/ components.lst init.img
 *                                  ^                ^          ^
 *                                  |                |          |
 *  module search path      --------+                |          |
 *  list of modules        --------------------------+          |
 *  output file           --------------------------------------+
 */
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <string>
#include <map>
#include <assert.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include "types.h"
#include "fourcc.h"
#include "bootimage_private.h"
#include "raiifile.h"
#include "config.h"

#if TOOLS_DEBUG
#define D(...) __VA_ARGS__
#else
#define D(...)
#endif

using namespace std;
using namespace raii_wrapper;
using namespace bootimage_n;

const uint32_t version = 1;
const uint32_t ALIGN = 4;

//======================================================================================================================
// helper functions
//======================================================================================================================

filebinio& operator << (filebinio& io, const std::string& str)
{
    io.write(str.c_str(), str.size() + 1);
    return io;
}

static int needed_align(size_t pos)
{
    if (pos % ALIGN)
        return ALIGN - pos % ALIGN;
    return 0;
}

static void align_output(file& out, uintptr_t& data_offset)
{
    size_t pos = out.write_pos();
    for (int i = 0; i < needed_align(pos); i++)
        out.write("\0", 1);
    data_offset += needed_align(pos);
}

//======================================================================================================================
// We want to be able to save headers from bootimage_private header on a 64-bits host system and have them suitable
// for use by a 32-bits target. Instead of fiddling with structure sizes we define writer helpers to output these
// structures to a storage file.
//======================================================================================================================

#define LIMIT32(x) (uintptr_t(x) & 0xffffffffu)

filebinio& operator << (filebinio& io, bootimage_n::header_t& h)
{
    io.write32le(h.magic);
    io.write32le(h.version);
    return io;
}

filebinio& operator << (filebinio& io, bootimage_n::rec_t& r)
{
    io.write32le(r.tag);
    io.write32le(r.length);
    return io;
}

filebinio& operator << (filebinio& io, bootimage_n::glue_code_t& gc)
{
    io << static_cast<rec_t&>(gc);
    io.write32le(LIMIT32(gc.text));
    io.write32le(LIMIT32(gc.data));
    io.write32le(LIMIT32(gc.bss));
    io.write32le(LIMIT32(gc.text_size));
    io.write32le(LIMIT32(gc.data_size));
    io.write32le(LIMIT32(gc.bss_size));
    return io;
}

filebinio& operator << (filebinio& io, bootimage_n::namespace_t& ns)
{
    io << static_cast<rec_t&>(ns);
    io.write32le(LIMIT32(ns.address));
    io.write32le(LIMIT32(ns.size));
    io.write32le(LIMIT32(ns.name));
    return io;
}

filebinio& operator << (filebinio& io, bootimage_n::module_t& mod)
{
    io << static_cast<namespace_t&>(mod);
    io.write32le(LIMIT32(mod.local_namespace_offset));
    return io;
}

filebinio& operator << (filebinio& io, bootimage_n::root_domain_t& dom)
{
    io << static_cast<module_t&>(dom);
    io.write32le(LIMIT32(dom.entry_point));
    return io;
}

filebinio& operator << (filebinio& io, bootimage_n::namespace_entry_t& ent)
{
    io.write32le(LIMIT32(ent.tag));
    io.write32le(LIMIT32(ent.name_off));
    io.write32le(LIMIT32(ent.value_int));
    return io;
}

//======================================================================================================================
// string table
//======================================================================================================================

class stringtable_t
{
public:
    stringtable_t() {}

    uint32_t append(const std::string& addend);
    size_t size() const;
    bool write(file& out, uintptr_t& data_offset) const;

private:
    vector<char> table;
};

/**
 * Append 0-terminated string to string table, return starting string offset in table.
 */
uint32_t stringtable_t::append(const std::string& addend)
{
    uint32_t last_offset = table.size();
    table.resize(table.size() + addend.size() + 1);
    copy(addend.begin(), addend.end(), table.begin() + last_offset);
    table.back() = 0;
    return last_offset;
}

size_t stringtable_t::size() const
{
    return table.size();
}

bool stringtable_t::write(file& out, uintptr_t& data_offset) const
{
    out.write(&table[0], table.size());
    data_offset += table.size();
    return true;
}

//======================================================================================================================
// namespace entry value
//======================================================================================================================

// is this struct needed? just use namespace_entry_t?
struct param
{
    namespace_entry_t::tag_t tag;
    // can't have a union here, because members of std::map need to have default ctor.
    int int_val;
    std::string string_val;
    void* sym_val;
};

//======================================================================================================================
// module info
//======================================================================================================================

class module_info
{
public:
    typedef std::map<std::string, param> ns_map;

    module_info() {}
    module_info(std::string name, std::string file) : name(name), file_name(file) {}

    void init(std::string name, std::string file) { this->name = name; this->file_name = file; }

    bool add_ns_entry(std::string key, int val);
    bool add_ns_entry(std::string key, std::string val);
    bool add_ns_entry(std::string key, void* val);
    void override_ns_entry(std::string key, int val);
    void override_ns_entry(std::string key, std::string val);
    void override_ns_entry(std::string key, void* val);

    void dump();

    // Write out module information together with the namespace and file data.
    bool write(file& out, uintptr_t& data_offset);

private:
    std::string name;
    std::string file_name;
    ns_map namespace_entries;

    bool add_ns_entry(std::string key, param val, bool override);
    bool write_root_domain_header(file& out, uintptr_t& data_offset, int in_size, int ns_size);
    bool write_module_header(file& out, uintptr_t& data_offset, int in_size, int ns_size);
};

//======================================================================================================================
// module namespace
//======================================================================================================================

class module_namespace1_t
{
public:
    module_namespace1_t(module_info::ns_map namespace_entries);

    bool write(file& out, uintptr_t& data_offset);
    size_t size() const;

private:
    std::vector<namespace_entry_t> entries;
    stringtable_t string_table;
};

module_namespace1_t::module_namespace1_t(module_info::ns_map namespace_entries)
{
    for(auto entry : namespace_entries)
    {
        namespace_entry_t e;
        e.tag = entry.second.tag;
        e.name_off = string_table.append(entry.first);
        switch (e.tag)
        {
            case namespace_entry_t::integer:
                e.value_int = entry.second.int_val;
                break;
            case namespace_entry_t::string:
                e.value_int = string_table.append(entry.second.string_val);
                break;
            case namespace_entry_t::symbol:
                e.value = entry.second.sym_val;
                break;
        }
        entries.push_back(e);
    }
}

size_t module_namespace1_t::size() const
{
    return 4 + entries.size() * SIZEOF_ONDISK_NAMESPACE_ENTRY + string_table.size();
}

bool module_namespace1_t::write(file& out, uintptr_t& data_offset)
{
    D(cout << "Writing " << entries.size() << " namespace entries" << endl);
    filebinio io(out);

    io.write32le(LIMIT32(entries.size()));

    data_offset += 4 + entries.size() * SIZEOF_ONDISK_NAMESPACE_ENTRY;
    for(auto entry : entries)
    {
        entry.name_off += data_offset; // Turn into a global bootimage file position.
        if (entry.tag == namespace_entry_t::string)
            entry.value_int += data_offset; // Adjust offset for string namespace entries too.
        io << entry;
    }
    string_table.write(out, data_offset);
    return true;
}

//======================================================================================================================
// module info
//======================================================================================================================

bool module_info::add_ns_entry(std::string key, param val, bool override)
{
    if (override || (namespace_entries.find(key) != namespace_entries.end()))
    {
        namespace_entries[key] = val;
        return true;
    }
    return false;
}

bool module_info::add_ns_entry(std::string key, int val)
{
    param p;
    p.tag = namespace_entry_t::integer;
    p.int_val = val;
    return add_ns_entry(key, p, false);
}

bool module_info::add_ns_entry(std::string key, std::string val)
{
    param p;
    p.tag = namespace_entry_t::string;
    p.string_val = val;
    return add_ns_entry(key, p, false);
}

bool module_info::add_ns_entry(std::string key, void* val)
{
    param p;
    p.tag = namespace_entry_t::symbol;
    p.sym_val = val;
    return add_ns_entry(key, p, false);
}

void module_info::override_ns_entry(std::string key, int val)
{
    param p;
    p.tag = namespace_entry_t::integer;
    p.int_val = val;
    add_ns_entry(key, p, true);
}

void module_info::override_ns_entry(std::string key, std::string val)
{
    param p;
    p.tag = namespace_entry_t::string;
    p.string_val = val;
    add_ns_entry(key, p, true);
}

void module_info::override_ns_entry(std::string key, void* val)
{
    param p;
    p.tag = namespace_entry_t::symbol;
    p.sym_val = val;
    add_ns_entry(key, p, true);
}

void module_info::dump()
{
    D(cout << name << " in " << file_name << endl);
/*    typedef std::map<std::string, param> nm_map;
    nm_map namespace_entries;*/
}

// module: header|name|namespace|<align>data<align>next header
//                ^    ^                ^          ^
//              --+    |                |          |
//             --------+                |          |
//            --------------------------+          |
//           --------------------------------------+
bool module_info::write_module_header(file& out, uintptr_t& data_offset, int in_size, int ns_size)
{
    bootimage_n::module_t mod;

    int name_s_a = name.size() + 1;

    data_offset += SIZEOF_ONDISK_MODULE;

    mod.tag = bootimage_n::kind_module;
    mod.length = SIZEOF_ONDISK_MODULE + name_s_a + ns_size + in_size;
    mod.address = data_offset + name_s_a + ns_size;
    mod.size = in_size;
    mod.name = (const char*)data_offset;
    mod.local_namespace_offset = ns_size ? data_offset + name_s_a : 0;

    filebinio io(out);
    io << mod << name;

    data_offset += name.size() + 1;

    return true;
}

bool module_info::write_root_domain_header(file& out, uintptr_t& data_offset, int in_size, int ns_size)
{
    bootimage_n::root_domain_t rdom;

    int name_s_a = name.size() + 1;

    data_offset += SIZEOF_ONDISK_ROOT_DOMAIN;

    rdom.tag = bootimage_n::kind_root_domain;
    rdom.length = SIZEOF_ONDISK_ROOT_DOMAIN + name_s_a + ns_size + in_size;
    rdom.address = data_offset + name_s_a + ns_size;
    rdom.size = in_size;
    rdom.name = (const char*)data_offset;
    rdom.local_namespace_offset = ns_size ? data_offset + name_s_a : 0;
    rdom.entry_point = 0xefbeadde;

    filebinio io(out);
    io << rdom << name;

    data_offset += name.size() + 1;

    return true;
}

bool module_info::write(file& out, uintptr_t& data_offset)
{
    size_t header_size = name == "root_domain" ? SIZEOF_ONDISK_ROOT_DOMAIN : SIZEOF_ONDISK_MODULE;
    file in_data(file_name, ios::in | ios::binary);
    size_t in_size = in_data.size();

    // Prepare namespace
    module_namespace1_t namesp(namespace_entries);
    int name_s_a = name.size() + 1;
    size_t ns_size = namespace_entries.size() > 0 ? namesp.size() : 0;
    size_t ns_size_a = ns_size + needed_align(data_offset + header_size + name_s_a + ns_size);

    size_t in_size_a = in_size + needed_align(data_offset + header_size + name_s_a + ns_size_a + in_size);

    if (name == "root_domain")
        write_root_domain_header(out, data_offset, in_size_a, ns_size_a);
    else
        write_module_header(out, data_offset, in_size_a, ns_size_a);

    if (ns_size)
        namesp.write(out, data_offset);
    align_output(out, data_offset);

    // Write module data
    char buf[4096];
    size_t out_size = 0;
    size_t bytes = in_data.read(buf, 4096);
    while (bytes > 0) {
        out.write(buf, bytes);
        out_size += bytes;
        bytes = in_data.read(buf, 4096);
    }
    if (in_size != out_size)
    {
        throw file_error("File was not entirely copied.");
    }
    data_offset += in_size;
    align_output(out, data_offset);

    return true;
}

//======================================================================================================================
// lst file parser
//======================================================================================================================

class line_reader_t
{
public:
    line_reader_t(ifstream& in);
    string getline();
    void putback(string line);
    bool end();

private:
    list<string> lines;
};

static bool starts_with(const std::string& str, char prefix)
{
    uint32_t i = 0;
    while (isspace(str[i]) && i < str.size())
        ++i;
    if (i < str.size() && str[i] == prefix)
       return true;
    return false;
}

line_reader_t::line_reader_t(ifstream& in)
    : lines()
{
    while (in)
    {
        std::string str;
        std::getline(in, str);
        if (!str.empty() && !(starts_with(str, '#')))
        {
            lines.push_back(str);
        }
    }
}

string line_reader_t::getline()
{
    if (!lines.empty())
    {
        string str = lines.front();
        lines.pop_front();
        return str;
    }
    return string();
}

void line_reader_t::putback(string line)
{
    lines.push_front(line);
}

bool line_reader_t::end()
{
    return lines.empty();
}

//======================================================================================================================
// main parser driver
//
// get the line, parse module until the next module starts or end of file, then push current module to modules.
//======================================================================================================================

static void parse_module_lines(std::vector<module_info>& modules, line_reader_t& reader, string& prefix)
{
    module_info mod;
    std::string modline = reader.getline();
    size_t pos;
    if ((pos = modline.find_first_of(":")) != string::npos)
    {
        string name = modline.substr(0, pos);
        string file = prefix + modline.substr(pos+1);
        //D(cerr << "parse_module_lines: module " << name << " in " << file << endl);
        mod.init(name, file);
    }
    else
        return;

    while (1)
    {
        string nsline = reader.getline();
        if ((pos = nsline.find_first_of(">")) == string::npos)
        {
            reader.putback(nsline);
            break;
        }
        // TODO
        // Add a symbol namespace syntax: [Symbol] means to take address of C symbol named Symbol and add it as val.
        // Needs parsing the module file with an elf parser...
        // e.g.
        // typesystem_mod:typesystem_mod.comp
        // [Types]
        // would pick a C symbol Types from the typesystem_mod.comp and put its address into the namespace...

        string key = nsline.substr(0, pos);
        string val = nsline.substr(pos+1);
        D(cerr << "parse_module_lines: nsp " << key << " with " << val << endl);
        mod.override_ns_entry(key, val);
    }
    modules.push_back(mod);
}

/*
!! namespace mapping could be a module name or module symbol name, fully qualified, or an integer or string constant.
*/
int main(int argc, char** argv)
{
    if (argc != 4)
        throw runtime_error("usage: buildboot base-path components.lst init.img");

    std::string prefix(argv[1]);
    std::string input(argv[2]);
    std::string output(argv[3]);

    try {
        file      out(output, ios::out | ios::binary);
        filebinio io(out);

        ifstream in(input.c_str(), ios::in);
        line_reader_t reader(in);
        std::vector<module_info> modules;

        while (!reader.end())
        {
            parse_module_lines(modules, reader, prefix);
        }

        uintptr_t data_offset = 0;//sizeof(header);

        bootimage_n::header_t hdr;
        hdr.magic = four_cc<'B', 'I', 'M', 'G'>::value;
        hdr.version = version;
        io << hdr;
        data_offset += 8; // sizeof(output bootimage_n::header_t)

        for (module_info& mod : modules)
        {
            printf("Adding...");
            mod.dump();

            mod.write(out, data_offset);
        }
    } // try
    catch(file_error& e)
    {
        printf("%s\n", e.message());
        unlink(output.c_str()); // remove invalid output
    }

    return 0;
}
