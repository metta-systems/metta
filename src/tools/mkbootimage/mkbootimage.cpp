//
// Read file with image description and create corresponding initfs image.
//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <map>
#include <assert.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/foreach.hpp>
#include "types.h"
#include "fourcc.h"
#include "bootimage.h"
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

filebinio& operator << (filebinio& io, vector<char> stringtable)
{
    io.write(stringtable.data(), stringtable.size());
    return io;
}

static int needed_align(size_t pos)
{
    if (pos % ALIGN)
        return ALIGN - pos % ALIGN;
    return 0;
}

static void align_output(file& out, int& data_offset)
{
    size_t pos = out.write_pos();
    for (int i = 0; i < needed_align(pos); i++)
        out.write("\0", 1);
    data_offset += needed_align(pos);
}

class stringtable_t
{
public:
    stringtable_t() {}

    uint32_t append(const std::string& addend);
    size_t size() const;
    bool write(file& out, int& data_offset) const;

private:
    vector<char> table;
};

/*!
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

bool stringtable_t::write(file& out, int& data_offset) const
{
    out.write(&table[0], table.size());
    data_offset += table.size();
    return true;
}

struct param
{
    enum { integer, string, sym } tag;
    int int_val;
    std::string string_val;
    void* sym_val;
};

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
    bool write(file& out, int& data_offset);

private:
    std::string name;
    std::string file_name;
    ns_map namespace_entries;

    bool add_ns_entry(std::string key, param val, bool override);
};

class module_namespace1_t
{
public:
    module_namespace1_t(module_info::ns_map namespace_entries);

    bool write(file& out, int& data_offset);
    size_t size() const;

private:
    std::vector<namespace_entry_t> entries;
    stringtable_t string_table;
};

module_namespace1_t::module_namespace1_t(module_info::ns_map namespace_entries)
{
    BOOST_FOREACH(auto entry, namespace_entries)
    {
        namespace_entry_t e;
        e.name_off = string_table.append(entry.first);
        e.value = entry.second.sym_val;
        entries.push_back(e);
    }
}

size_t module_namespace1_t::size() const
{
    return entries.size() * 8/*sizeof(namespace_entry_t)*/ + string_table.size();
}

bool module_namespace1_t::write(file& out, int& data_offset)
{
    D(cout << "Writing " << entries.size() << " ns entries" << endl);
    filebinio io(out);
    data_offset += entries.size() * 8;//sizeof(namespace_entry_t);
    BOOST_FOREACH(auto entry, entries)
    {
        io.write32le(entry.name_off + data_offset);
//         io.write32le(entry.tag);
        io.write32le(entry.value_int);
    }
    string_table.write(out, data_offset);
    return true;
}


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
    p.int_val = val;
    return add_ns_entry(key, p, false);
}

bool module_info::add_ns_entry(std::string key, std::string val)
{
    param p;
    p.string_val = val;
    return add_ns_entry(key, p, false);
}

bool module_info::add_ns_entry(std::string key, void* val)
{
    param p;
    p.sym_val = val;
    return add_ns_entry(key, p, false);
}

void module_info::override_ns_entry(std::string key, int val)
{
    param p;
    p.int_val = val;
    add_ns_entry(key, p, true);
}

void module_info::override_ns_entry(std::string key, std::string val)
{
    param p;
    p.string_val = val;
    add_ns_entry(key, p, true);
}

void module_info::override_ns_entry(std::string key, void* val)
{
    param p;
    p.sym_val = val;
    add_ns_entry(key, p, true);
}

void module_info::dump()
{
    D(cout << name << " in " << file_name << endl);
/*    typedef std::map<std::string, param> nm_map;
    nm_map namespace_entries;*/
}

bool module_info::write(file& out, int& data_offset)
{
    bootimage_n::module_t mod;

    file in_data(file_name, ios::in | ios::binary);
    size_t in_size = in_data.size();

    // Prepare namespace
    module_namespace1_t namesp(namespace_entries);
    long ns_size = namespace_entries.size() > 0 ? namesp.size() : 0;
    long ns_size_a = ns_size + needed_align(data_offset + sizeof(mod) + ns_size);

    // Write module header
    mod.tag = bootimage_n::kind_module;
    mod.length = sizeof(mod) + in_size + ns_size;
    mod.address = data_offset + sizeof(mod) + ns_size_a;
    mod.size = in_size;
    mod.name = 0;
//     rdom.name = name string offset;
    mod.local_namespace_offset = ns_size ? data_offset + sizeof(mod) : 0;
//     rdom.entry_point = 0;
    out.write(&mod, sizeof(mod));
    data_offset += sizeof(mod);
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

static bool starts_with(string str, char prefix)
{
    uint32_t i = 0;
    while (isspace(str[i]) && i < str.length())
        ++i;
    if (i < str.length() && str[i] == prefix)
       return true;
    return false;
}

line_reader_t::line_reader_t(ifstream& in)
    : lines()
{
    while (in)
    {
        string str;
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

// get the line, parse module until the next module starts or end of file, then push current module to modules.
static void parse_module_lines(std::vector<module_info>& modules, line_reader_t& reader, string& prefix)
{
    module_info mod;
    std::string modline = reader.getline();
    size_t pos;
    if ((pos = modline.find_first_of(":")) != string::npos)
    {
        string name = modline.substr(0, pos);
        string file = prefix + modline.substr(pos+1);
        D(cerr << "parse_module_lines: module " << name << " in " << file << endl);
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
        string key = nsline.substr(0, pos);
        string val = nsline.substr(pos+1);
        D(cerr << "parse_module_lines: nsp " << key << " with " << val << endl);
        mod.override_ns_entry(key, val);
    }
    modules.push_back(mod);
}

/*!
 * Call like:
 * mkbootimg _build_/x86-pc99-release/modules/ components.lst init.img
 */
/*
!! namespace mapping can be a module name or module symbol name, fully qualified, or an integer or string constant.
*/
int main(int argc, char** argv)
{
    if (argc != 4)
        throw runtime_error("usage: mkbootimage base-path components.lst init.img");

    std::string prefix(argv[1]);
    std::string input(argv[2]);
    std::string output(argv[3]);

    try {
        file      out(output, ios::out | ios::binary);
        filebinio io(out);

        ifstream in(input, ios::in);
        line_reader_t reader(in);
        std::vector<module_info> modules;

        while (!reader.end())
        {
            parse_module_lines(modules, reader, prefix);
        }

        int data_offset = 0;//sizeof(header);

        io.write32le(FourCC<'B', 'I', 'M', 'G'>::value);
        io.write32le(version);
        data_offset += 8; // sizeof(bootimage_n::header_t)

        BOOST_FOREACH(module_info& mod, modules)
        {
            printf("Adding...");
            mod.dump();
            mod.write(out, data_offset);
        }

/*
//         uint32_t                   i;
        std::string                str;
//         initfs_t::header_t         header;
//         vector<initfs_t::entry_t>  entry;
        vector<char>               name_storage;
        int                        name_offset = 0;

        name_offset = out.write_pos();
//         header.names_offset = name_offset;
        io << name_storage;

//         header.names_size = out.write_pos() - name_offset;

        align_output(out);

//         header.count = entry.size();
//         header.index_offset = out.write_pos();
//         for (i = 0; i < header.count; i++)
//         {
//             entry[i].name_offset += name_offset;
//             io << entry[i];
//         }

        // rewrite file header
//         out.write_seek(0);
//         io << header;
*/
    } // try
    catch(file_error& e)
    {
        printf("%s\n", e.message());
        unlink(output.c_str()); // remove invalid output
    }

    return 0;
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
