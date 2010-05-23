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
#include "initfs.h"
#include "raiifile.h"

using namespace std;
using namespace raii_wrapper;

const uint32_t ALIGN = 4;

filebinio& operator << (filebinio& io, vector<char> stringtable)
{
    io.write(stringtable.data(), stringtable.size());
    return io;
}

filebinio& operator << (filebinio& io, initfs_t::header_t& header)
{
    io.write32le(header.magic);
    io.write32le(header.version);
    io.write32le(header.index_offset);
    io.write32le(header.names_offset);
    io.write32le(header.names_size);
    io.write32le(header.count);
    return io;
}

filebinio& operator << (filebinio& io, initfs_t::entry_t& entry)
{
    io.write32le(entry.magic);
    io.write32le(entry.name_offset);
    io.write32le(entry.location);
    io.write32le(entry.size);
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

int main(int argc, char** argv)
{
    if (argc != 3)
        throw runtime_error("usage: mkbootimage components.lst init.img");

    try {
        file      in(argv[1], ios::in);
        file      out(argv[2], ios::out | ios::binary);
        filebinio io(out);

        uint32_t                   i;
        std::string                str;
        initfs_t::header_t         header;
        vector<initfs_t::entry_t>  entry;
        vector<char>               name_storage;
        int                        name_offset = 0;
        int                        data_offset = sizeof(header);

        io << header;

        while (in.getline(str))
        {
            vector<std::string> strs;
            boost::split(strs, str, boost::is_any_of(":"));

            if (strs.size() < 2)
                continue;

            std::string left = strs.front();
            strs.erase(strs.begin());
            std::string right = boost::join(strs, ":");

            file in_data(left, ios::in | ios::binary);
            long in_size = in_data.size();
            char buf[4096];
            size_t bytes = in_data.read(buf, 4096);
            while (bytes > 0) {
                out.write(buf, bytes);
                bytes = in_data.read(buf, 4096);
            }
            entry.push_back(initfs_t::entry_t(stringtable_append(name_storage, right), data_offset, in_size));
            data_offset += in_size;
        }

        align_output(out);

        name_offset = out.write_pos();
        header.names_offset = name_offset;
        io << name_storage;

        header.names_size = out.write_pos() - name_offset;

        align_output(out);

        header.count = entry.size();
        header.index_offset = out.write_pos();
        for (i = 0; i < header.count; i++)
        {
            entry[i].name_offset += name_offset;
            io << entry[i];
        }

        // rewrite file header
        out.write_seek(0);
        io << header;
    } // try
    catch(file_error& e)
    {
        printf("%s\n", e.message());
        unlink(argv[2]); // remove invalid output
    }

    return 0;
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
