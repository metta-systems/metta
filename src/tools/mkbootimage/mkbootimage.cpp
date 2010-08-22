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

struct param
{
    int int_val;
    std::string string_val;
    void* sym_val;
};

class module_info
{
public:
    module_info() {}
    module_info(std::string name, std::string file) : name(name), file(file) {}

    void init(std::string name, std::string file) { this->name = name; this->file = file; }

    bool add_ns_entry(std::string key, int val);
    bool add_ns_entry(std::string key, std::string val);
    bool add_ns_entry(std::string key, void* val);
    void override_ns_entry(std::string key, int val);
    void override_ns_entry(std::string key, std::string val);
    void override_ns_entry(std::string key, void* val);

    void dump();

    string file_name() const { return file; }

private:
    std::string name;
    std::string file;
    typedef std::map<std::string, param> nm_map;
    nm_map namespace_entries;

    bool add_ns_entry(std::string key, param val, bool override);
};

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
    std::cout << name << " in " << file << endl;
/*    typedef std::map<std::string, param> nm_map;
    nm_map namespace_entries;*/
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

const uint32_t version = 1;
const uint32_t ALIGN = 4;

filebinio& operator << (filebinio& io, vector<char> stringtable)
{
    io.write(stringtable.data(), stringtable.size());
    return io;
}

/*!
 * Append 0-terminated string to string table, return starting string offset in table.
 */
uint32_t stringtable_append(vector<char>& table, const std::string& addend)
{
    uint32_t last_offset = table.size();
    table.resize(table.size() + addend.size() + 1);
    copy(addend.begin(), addend.end(), table.begin() + last_offset);
    table.back() = 0;
    return last_offset;
}

void align_output(file& out)
{
    uint32_t i, padding;
    if (out.write_pos() % ALIGN)
    {
        padding = ALIGN - out.write_pos() % ALIGN;
        for (i = 0; i < padding; i++)
            out.write("\0", 1);
    }
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
            file in_data(mod.file_name(), ios::in | ios::binary);
            long in_size = in_data.size();
            // Write module header
            bootimage_n::root_domain_t rdom;
            rdom.tag = bootimage_n::kind_root_domain;
            rdom.length = sizeof(rdom) + in_size;
            rdom.address = data_offset + sizeof(rdom);
            rdom.size = in_size;
            rdom.name = 0;
            //         rdom.name = name string offset;
            rdom.local_namespace_offset = 0;
            rdom.entry_point = 0;
            out.write(&rdom, sizeof(rdom));
            // Write module data
            char buf[4096];
            size_t bytes = in_data.read(buf, 4096);
            while (bytes > 0) {
                out.write(buf, bytes);
                bytes = in_data.read(buf, 4096);
            }
            //         entry.push_back(initfs_t::entry_t(stringtable_append(name_storage, right), data_offset, in_size));
            data_offset += in_size;

            align_output(out);
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
