#include "leb128.h"
#include "dwarf_parser.h"

// Read the data in given form
class form_reader_t
{
public:
    static form_reader_t* create(dwarf_parser_t& parser, uint32_t form); // factory

    form_reader_t(dwarf_parser_t& p) : parser(p) {}
    virtual bool decode(address_t from, size_t& offset) = 0;
    virtual void print() = 0;

protected:
    dwarf_parser_t& parser;
};

class addr_form_reader_t : public form_reader_t
{
public:
    uint32_t data;

    addr_form_reader_t(dwarf_parser_t& p) : form_reader_t(p) {}
    virtual bool decode(address_t from, size_t& offset);
    virtual void print();
};

// Arbitrary data blocks.
class block_form_reader_t : public form_reader_t
{
public:
    const char* data;
    uleb128_t length;

    block_form_reader_t(dwarf_parser_t& p) : form_reader_t(p) {}
    virtual bool decode(address_t from, size_t& offset);
    virtual void print();
};

class block1_form_reader_t : public form_reader_t
{
public:
    const char* data;
    uint8_t length;

    block1_form_reader_t(dwarf_parser_t& p) : form_reader_t(p) {}
    virtual bool decode(address_t from, size_t& offset);
    virtual void print();
};

class block2_form_reader_t : public form_reader_t
{
public:
    const char* data;
    uint16_t length;

    block2_form_reader_t(dwarf_parser_t& p) : form_reader_t(p) {}
    virtual bool decode(address_t from, size_t& offset);
    virtual void print();
};

class block4_form_reader_t : public form_reader_t
{
public:
    const char* data;
    uint32_t length;

    block4_form_reader_t(dwarf_parser_t& p) : form_reader_t(p) {}
    virtual bool decode(address_t from, size_t& offset);
    virtual void print();
};

// Constants.
class sdata_form_reader_t : public form_reader_t
{
public:
    sleb128_t data;

    sdata_form_reader_t(dwarf_parser_t& p) : form_reader_t(p) {}
    virtual bool decode(address_t from, size_t& offset);
    virtual void print();
};

class udata_form_reader_t : public form_reader_t
{
public:
    uleb128_t data;

    udata_form_reader_t(dwarf_parser_t& p) : form_reader_t(p) {}
    virtual bool decode(address_t from, size_t& offset);
    virtual void print();
};

class data1_form_reader_t : public form_reader_t
{
public:
    uint8_t data;

    data1_form_reader_t(dwarf_parser_t& p) : form_reader_t(p) {}
    virtual bool decode(address_t from, size_t& offset);
    virtual void print();
};

class data2_form_reader_t : public form_reader_t
{
public:
    uint16_t data;

    data2_form_reader_t(dwarf_parser_t& p) : form_reader_t(p) {}
    virtual bool decode(address_t from, size_t& offset);
    virtual void print();
};

class data4_form_reader_t : public form_reader_t
{
public:
    uint32_t data;

    data4_form_reader_t(dwarf_parser_t& p) : form_reader_t(p) {}
    virtual bool decode(address_t from, size_t& offset);
    virtual void print();
};

class data8_form_reader_t : public form_reader_t
{
public:
    uint64_t data;

    data8_form_reader_t(dwarf_parser_t& p) : form_reader_t(p) {}
    virtual bool decode(address_t from, size_t& offset);
    virtual void print();
};

class flag_form_reader_t : public form_reader_t
{
public:
    uint8_t data;

    flag_form_reader_t(dwarf_parser_t& p) : form_reader_t(p) {}
    virtual bool decode(address_t from, size_t& offset);
    virtual void print();
};

class string_form_reader_t : public form_reader_t
{
public:
    const char* data;

    string_form_reader_t(dwarf_parser_t& p) : form_reader_t(p) {}
    virtual bool decode(address_t from, size_t& offset);
    virtual void print();
};

class strp_form_reader_t : public form_reader_t
{
public:
    const char* data;

    strp_form_reader_t(dwarf_parser_t& p) : form_reader_t(p) {}
    virtual bool decode(address_t from, size_t& offset);
    virtual void print();
};

// Non-relocated references (inside compilation unit)
class ref1_form_reader_t : public form_reader_t
{
public:
    uint8_t data;

    ref1_form_reader_t(dwarf_parser_t& p) : form_reader_t(p) {}
    virtual bool decode(address_t from, size_t& offset);
    virtual void print();
};

class ref2_form_reader_t : public form_reader_t
{
public:
    uint16_t data;

    ref2_form_reader_t(dwarf_parser_t& p) : form_reader_t(p) {}
    virtual bool decode(address_t from, size_t& offset);
    virtual void print();
};

class ref4_form_reader_t : public form_reader_t
{
public:
    uint32_t data;

    ref4_form_reader_t(dwarf_parser_t& p) : form_reader_t(p) {}
    virtual bool decode(address_t from, size_t& offset);
    virtual void print();
};

class ref8_form_reader_t : public form_reader_t
{
public:
    uint64_t data;

    ref8_form_reader_t(dwarf_parser_t& p) : form_reader_t(p) {}
    virtual bool decode(address_t from, size_t& offset);
    virtual void print();
};

class ref_udata_form_reader_t : public form_reader_t
{
public:
    uleb128_t data;

    ref_udata_form_reader_t(dwarf_parser_t& p) : form_reader_t(p) {}
    virtual bool decode(address_t from, size_t& offset);
    virtual void print();
};

// Relocated references (to other compilation units)
class ref_addr_form_reader_t : public form_reader_t
{
public:
    uint32_t data; // In DWARF32 32 bits. It is offset from start of .debug_info

    ref_addr_form_reader_t(dwarf_parser_t& p) : form_reader_t(p) {}
    virtual bool decode(address_t from, size_t& offset);
    virtual void print();
};

// Indirect form, encodes class together with the data.
class indirect_form_reader_t : public form_reader_t
{
public:
    uleb128_t form;
    form_reader_t* data;

    indirect_form_reader_t(dwarf_parser_t& p) : form_reader_t(p) {}
    virtual bool decode(address_t from, size_t& offset);
    virtual void print();
};
