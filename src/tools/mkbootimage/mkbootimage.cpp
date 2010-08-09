//
// Read list file with component_file:component_id pairs and create corresponding initfs image.
// Run mkinitfs file.lst initfs.img
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
#include <assert.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include "types.h"
#include "fourcc.h"
#include "bootimage.h"
#include "bootimage_private.h"
#include "raiifile.h"

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
int main(int argc, char** argv)
{
    if (argc != 4)
        throw runtime_error("usage: mkbootimage base-path components.lst init.img");

    std::string prefix(argv[1]);
    std::string input(argv[2]);
    std::string output(argv[3]);

    try {
        file      in(input, ios::in);
        file      out(output, ios::out | ios::binary);
        filebinio io(out);

//         uint32_t                   i;
        std::string                str;
//         initfs_t::header_t         header;
//         vector<initfs_t::entry_t>  entry;
        vector<char>               name_storage;
        int                        name_offset = 0;
        int                        data_offset = 0;//sizeof(header);

        io.write32le(FourCC<'B', 'I', 'M', 'G'>::value);
        io.write32le(version);
        data_offset += 8; // sizeof(bootimage_n::header_t)

        // Parse the input file, should be somewhat more complex structure to accomodate different types of components.
        while (in.getline(str))
        {
            vector<std::string> strs;
            boost::split(strs, str, boost::is_any_of(":"));

            if (strs.size() < 2)
                continue;

            std::string left = strs.front();
            strs.erase(strs.begin());
            std::string right = boost::join(strs, ":");

            printf("Adding %s...\n", (prefix + left).c_str());
            file in_data(prefix + left, ios::in | ios::binary);
            long in_size = in_data.size();
            // Write module header
            bootimage_n::root_domain_t rdom;
            rdom.tag = bootimage_n::kind_root_domain;
            rdom.length = sizeof(rdom) + in_size;
            rdom.address = data_offset + sizeof(rdom);
            rdom.size = in_size;
            rdom.name = 0;
//             rdom.name = name string offset;
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
//             entry.push_back(initfs_t::entry_t(stringtable_append(name_storage, right), data_offset, in_size));
            data_offset += in_size;
        }

        align_output(out);

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
