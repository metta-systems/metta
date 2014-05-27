//
// RAII file wrapper.
// Usable only on host system atm.
//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include <iostream>
#include <fstream>
#include <string>
#include "types.h"

namespace raii_wrapper {

class file_error
{
public:
    file_error(const char* msg_) : msg(msg_) {}
    const char* message() { return msg; }
private:
    const char* msg;//REFACTOR: use std::string
};

class file
{
public:
    file() {}

    file(const char* fname, std::fstream::openmode mode)
    {
        open(fname, mode);
    }

    file(const std::string& fname, std::fstream::openmode mode)
    {
        open(fname.c_str(), mode);
    }

    ~file() { file_.close(); }

    void open(const char* fname, std::fstream::openmode mode)
    {
        file_.open(fname, mode);
        if (!file_.good())
            throw file_error("file open failure");
    }

    void write(const void* buf, size_t count)
    {
        file_.write((const char*)buf, count);
        if (file_.bad())
            throw file_error("file write failure");
    }

    size_t read(void* buf, size_t size)
    {
        file_.read((char*)buf, size);
        return file_.gcount();
    }

    long read_pos()
    {
        return file_.tellg();
    }

    long write_pos()
    {
        return file_.tellp();
    }

    bool read_seek(long pos)
    {
        file_.seekg(pos);
        return !file_.fail();
    }

    bool write_seek(long pos)
    {
        file_.seekp(pos);
        return !file_.fail();
    }

    long size()
    {
        long old = read_pos();
        file_.seekg(0, std::fstream::end);
        long sz = read_pos();
        read_seek(old);
        return sz;
    }

    bool getline(std::string& str, char delim)
    {
        std::istream& is = std::getline(file_, str, delim);
        return !is.eof() && !is.bad() && !is.fail();
    }

    bool getline(std::string& str)
    {
        std::istream& is = std::getline(file_, str);
        return !is.eof() && !is.bad() && !is.fail();
    }

private:
    std::fstream file_;

    // prevent copying and assignment; only declarations
    file(const file&);
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

    void read32le(uint32_t& datum) { file_.read(&datum, sizeof(uint32_t)); }

private:
    file& file_;
};

// Example of using filebinio:
// filebinio& operator << (filebinio& io, string str)
// {
//     io.write((const char*)str, str.length());
//     return io;
// }

}
