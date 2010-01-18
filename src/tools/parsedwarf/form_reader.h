#include "leb128.h"

// Read the data in given form
class form_reader_t
{
public:
    static form_reader_t* create(uint32_t form); // factory

    virtual bool decode(address_t from, size_t& offset) = 0;
};

class addr_form_reader_t : public form_reader_t
{
public:
    uint32_t data;

    virtual bool decode(address_t from, size_t& offset);
};

// Arbitrary data blocks.
class block_form_reader_t : public form_reader_t
{
public:
    const char* data;
    uleb128_t length;

    virtual bool decode(address_t from, size_t& offset);
};

class block1_form_reader_t : public form_reader_t
{
public:
    const char* data;
    uint8_t length;

    virtual bool decode(address_t from, size_t& offset);
};

class block2_form_reader_t : public form_reader_t
{
public:
    const char* data;
    uint16_t length;

    virtual bool decode(address_t from, size_t& offset);
};

class block4_form_reader_t : public form_reader_t
{
public:
    const char* data;
    uint32_t length;

    virtual bool decode(address_t from, size_t& offset);
};

// Constants.
class sdata_form_reader_t : public form_reader_t
{
public:
    sleb128_t data;

    virtual bool decode(address_t from, size_t& offset);
};

class udata_form_reader_t : public form_reader_t
{
public:
    uleb128_t data;

    virtual bool decode(address_t from, size_t& offset);
};

class data1_form_reader_t : public form_reader_t
{
public:
    uint8_t data;

    virtual bool decode(address_t from, size_t& offset);
};

class data2_form_reader_t : public form_reader_t
{
public:
    uint16_t data;

    virtual bool decode(address_t from, size_t& offset);
};

class data4_form_reader_t : public form_reader_t
{
public:
    uint32_t data;

    virtual bool decode(address_t from, size_t& offset);
};

class data8_form_reader_t : public form_reader_t
{
public:
    uint64_t data;

    virtual bool decode(address_t from, size_t& offset);
};

class flag_form_reader_t : public form_reader_t
{
public:
    uint8_t data;

    virtual bool decode(address_t from, size_t& offset);
};

class string_form_reader_t : public form_reader_t
{
public:
    const char* data;

    virtual bool decode(address_t from, size_t& offset);
};

class strp_form_reader_t : public form_reader_t
{
public:
    const char* data;

    virtual bool decode(address_t from, size_t& offset);
};

// Non-relocated references (inside compilation unit)
class ref1_form_reader_t : public form_reader_t
{
public:
    uint8_t data;

    virtual bool decode(address_t from, size_t& offset);
};

class ref2_form_reader_t : public form_reader_t
{
public:
    uint16_t data;

    virtual bool decode(address_t from, size_t& offset);
};

class ref4_form_reader_t : public form_reader_t
{
public:
    uint32_t data;

    virtual bool decode(address_t from, size_t& offset);
};

class ref8_form_reader_t : public form_reader_t
{
public:
    uint64_t data;

    virtual bool decode(address_t from, size_t& offset);
};

class ref_udata_form_reader_t : public form_reader_t
{
public:
    uleb128_t data;

    virtual bool decode(address_t from, size_t& offset);
};

// Relocated references (to other compilation units)
class ref_addr_form_reader_t : public form_reader_t
{
public:
    uint32_t data; // In DWARF32 32 bits. It is offset from start of .debug_info

    virtual bool decode(address_t from, size_t& offset);
};

// Indirect form, encodes class together with the data.
class indirect_form_reader_t : public form_reader_t
{
public:
    uleb128_t form;
    form_reader_t* data;

    virtual bool decode(address_t from, size_t& offset);
};
