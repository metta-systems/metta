//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
//
// Read list file with component_file:component_id pairs and create corresponding initfs image.
// Run mkinitfs file.lst initfs.img
//
#include "types.h"
#include "initfs.h"
#include "string.h"
#include <assert.h>
#include <cstdio>

#define ALIGN 4
// If you ever need more than 256 entries in initfs, feel free to adjust this constant.
#define MAX_ENTRIES 256

using metta::kernel::string;
using Bstrlib::CBStream;

// Some nicely common file RAII wrappers. TODO: Factor out.
class file_error
{
public:
    file_error(const char* msg) : msg_(msg) {}
private:
    const char* msg_;
};

class file
{
public:
    file(const char* fname, const char* mode) : file_(std::fopen(fname, mode))
    {
        if (!file_)
            throw file_error("file open failure");
    }
    ~file() { std::fclose(file_); }
    void write(const void* buf, size_t count)
    {
        if (EOF == std::fwrite(buf, 1, count, file_))
            throw file_error("file write failure");
    }
    size_t read(void* buf, size_t size, size_t nmemb)
    {
        return std::fread(buf, size, nmemb, file_);
    }
    long pos()
    {
        return std::ftell(file_);
    }
    bool seek(long pos)
    {
        return std::fseek(file_, pos, SEEK_SET) == 0;
    }

private:
    FILE* file_;

    // prevent copying and assignment; not implemented
    file (const file&);
    file& operator= (const file&);
};

class filebinio
{
public:
    filebinio(file& f) : file_(f) {}

    void write(const void* buf, size_t count) { file_.write(buf, count); }
    void write8(uint8_t datum)     { file_.write(&datum, sizeof(uint8_t));  }
    void write16le(uint16_t datum) { file_.write(&datum, sizeof(uint16_t)); }
    void write32le(uint32_t datum) { file_.write(&datum, sizeof(uint32_t)); }

private:
    file& file_;
};

filebinio& operator << (filebinio& io, string str)
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
    return f->read(buff, elsize, nelem);
}

int main(int argc, char** argv)
{
    if (argc != 3)
        return 255;
    const char *file_in = argv[1];
    const char *file_out = argv[2];

    int i, padding;
    file out(file_out, "w+b");
    file in(file_in, "r");
    filebinio io(out);
    CBStream in_stream(read_func, &in);

    initfs::header header;
    initfs::entry  entry[MAX_ENTRIES];
    string name_storage; // use bstring to store names as multiple \0-terminated strings concatenated together.
    int name_offset = 0;
    int data_offset = sizeof(header);

    io << header;

    while (1) {
        string input = in_stream.readLine('\n');
        if (input.length() == 0)
            break;
        int pos = input.find(':');
        string left(input.midstr(0, pos));
        string right(input.midstr(pos + 1, input.length() - pos - 1));
        right[right.length()-1] = '\0';

        {
            file in_data(left, "rb");
            char buf[4096];
            size_t bytes = in_data.read(buf, 1, 4096);
            while (bytes > 0) {
                out.write(buf, bytes);
                bytes = in_data.read(buf, 1, 4096);
            }
            entry[header.count].name_offset = name_offset;
            entry[header.count].location = data_offset;
            entry[header.count].size = in_data.pos();
            name_storage += right;
            name_offset += right.length();
            data_offset += entry[header.count].size;
            header.count++;
            assert(header.count < MAX_ENTRIES);
        }
    }

    if (out.pos() % ALIGN)
    {
        padding = ALIGN - out.pos() % ALIGN;
        for (i = 0; i < padding; i++)
            io.write("\0", 1);
    }

    name_offset = out.pos();
    header.names_offset = name_offset;
    io << name_storage;

    header.names_size = out.pos() - name_offset;

    if (out.pos() % ALIGN)
    {
        padding = ALIGN - out.pos() % ALIGN;
        for (i = 0; i < padding; i++)
            io.write("\0", 1);
    }

    header.index_offset = out.pos();
    for (i = 0; i < header.count; i++)
    {
        entry[i].name_offset += name_offset;
        io << entry[i];
    }

    // rewrite file header
    out.seek(0);
    io << header;
}

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
