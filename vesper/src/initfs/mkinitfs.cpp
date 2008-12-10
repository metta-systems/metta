//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
//
// Read list file with component_id:component_path pairs and create corresponding initfs image.
// Run mkinitfs file.lst initfs.img
//
#include "initfs.h"
#include <cstdio>

class file
{
public:
    file(const char* fname, const char* mode) : file_(std::open(fname, mode))
    {
        if (!file_)
            throw std::runtime_error("file open failure");
    }
    ~file() { std::fclose(file_); }
    void write (const void* buf, size_t count)
    {
        if (EOF == std::fwrite(buf, 1, count, file_))
            throw std::runtime_error("file write failure");
    }
    // TODO: add seek()

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

    void write8(uint8_t datum)     { file_.write(&datum, sizeof(uint8_t));  }
    void write16le(uint16_t datum) { file_.write(&datum, sizeof(uint16_t)); }
    void write32le(uint32_t datum) { file_.write(&datum, sizeof(uint32_t)); }

private:
    file& file_;
};

filebinio& operator << (filebinio& io, initfs_header& header)
{
    io.write32le(header.magic);
    io.write32le(header.index_offset);
    return io;
}

filebinio& operator << (filebinio& io, initfs_entry& entry)
{
    io.write32le(entry.magic);
    io.write32le(entry.name_offset);
    io.write32le(entry.location);
    io.write32le(entry.size);
    return io;
}

filebinio& operator << (filebinio& io, initfs_index& index)
{
    io.write32le(entry.magic);
    io.write32le(entry.count);
    return io;
}

int main(int argc, char** argv)
{
    if (argc != 3)
        return 255;
    const char *file_in = argv[1];
    const char *file_out = argv[2];

    file out(file_out, "w+b");
    file in(file_in, "r");
    filebinio io(in);
    //TODO: use bstring to store names as multiple \0-terminated strings
    // concatenated together.

    io << header;
    // write file data
    // page align and put names
    // page align and
    io << index;
    for (i = 0; i < index.count; i++)
        io << entry[i];
}

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
