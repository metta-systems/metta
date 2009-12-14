#pragma once

struct string_ascii_trait
{
    typedef char code_point;

    size_t get_sequence_length(const char data) const;
    code_point get_code_point(const char* data) const;
};

struct string_utf8_trait
{
    typedef uint32_t code_point;

    size_t get_sequence_length(const char data) const;
    code_point get_code_point(const char* data) const;
};

template <class string_type_trait>
class string_t
{
public:
    typedef string_type_trait::code_point code_point;

    string_t();
    string_t(const char* data);

    size_t length() { return size; }

private:
    size_t size;
    char*  data;
};

typedef string_t<string_ascii_trait> cstring_t;
typedef string_t<string_utf8_trait> utf8_string_t;
