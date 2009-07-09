//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
//
// Read list file with component_file:component_id pairs and create corresponding initfs image.
// Run mkinitfs file.lst initfs.img
//
// TODO: use STL/boost for i/o
//
#include "types.h"
#include "initfs.h"
#include "bstrwrap.h"
#include "raiifile.h"
#include <assert.h>

using Bstrlib::CBString;
using Bstrlib::CBStream;
using namespace raii_wrapper;

#define ALIGN 4
// If you ever need more than 256 entries in initfs, feel free to adjust this constant.
#define MAX_ENTRIES 256

filebinio& operator << (filebinio& io, CBString str)
{
    io.write((const char*)str, str.length());
    return io;
}

filebinio& operator << (filebinio& io, initfs::header& header)
{
    io.write32le(header.magic);
    io.write32le(header.version);
    io.write32le(header.index_offset);
    io.write32le(header.names_offset);
    io.write32le(header.names_size);
    io.write32le(header.count);
    return io;
}

filebinio& operator << (filebinio& io, initfs::entry& entry)
{
    io.write32le(entry.magic);
    io.write32le(entry.name_offset);
    io.write32le(entry.location);
    io.write32le(entry.size);
    return io;
}

size_t read_func(void *buff, size_t elsize, size_t nelem, void *parm)
{
    file* f = (file*)parm;
    return f->read(buff, elsize * nelem);
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
    {
        std::printf("usage: mkinitfs components.lst initfs.img\n");
        return 255;
    }
    const char *file_in = argv[1];
    const char *file_out = argv[2];

    try {

    uint32_t i;
    file out(file_out, std::ios::out | std::ios::binary);
    file in(file_in, std::ios::in);
    filebinio io(out);
    CBStream in_stream(read_func, &in);

    initfs::header header;
    initfs::entry  entry[MAX_ENTRIES];
    CBString name_storage; // use bstring to store names as multiple \0-terminated strings concatenated together.
    int name_offset = 0;
    int data_offset = sizeof(header);

    io << header;

    while (1) {
        CBString input = in_stream.readLine('\n');
        if (input.length() == 0)
            break;
        int pos = input.find(':');
        if (pos == -1)
            continue;
        CBString left(input.midstr(0, pos));
        CBString right(input.midstr(pos + 1, input.length() - pos - 1));
        right[right.length()-1] = '\0';

        {
            file in_data((const char *)left, std::ios::in | std::ios::binary);
            long in_size = in_data.size();
            char buf[4096];
            size_t bytes = in_data.read(buf, 4096);
            while (bytes > 0) {
                out.write(buf, bytes);
                bytes = in_data.read(buf, 4096);
            }
            entry[header.count].name_offset = name_offset;
            entry[header.count].location = data_offset;
            entry[header.count].size = in_size;
            name_storage += right;
            name_offset += right.length();
            data_offset += in_size;
            header.count++;
            assert(header.count < MAX_ENTRIES);
        }
    }

    align_output(out);

    name_offset = out.write_pos();
    header.names_offset = name_offset;
    io << name_storage;

    header.names_size = out.write_pos() - name_offset;

    align_output(out);

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
        std::printf("%s\n", e.message());
        unlink(file_out);
    }
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
