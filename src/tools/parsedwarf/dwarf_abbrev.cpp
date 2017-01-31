//
// Part of Metta OS. Check https://atta-metta.net for latest version.
//
// Copyright 2010 - 2017, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "dwarf_abbrev.h"
#include "datarepr.h"
#include "dwarf_debug.h"

void abbrev_declaration_t::decode(address_t from, size_t& offset)
{
    abbreviation_code.decode(from, offset);
    if (abbreviation_code == 0)
    {
#if DWARF_DEBUG
        printf("found last abbrev code in set\n");
#endif
        return;
    }
    tag.decode(from, offset);
    has_children = *reinterpret_cast<uint8_t*>(from + offset);
    offset += sizeof(uint8_t);

    abbrev_attr_t abbr;
    while (1)
    {
        abbr.decode(from, offset);
        attributes.push_back(abbr);
        if (abbr.is_empty())
            break;
    }
}

bool dwarf_debug_abbrev_t::load_abbrev_set(size_t& offset)
{
    abbrevs.clear();
    while (1)
    {
        abbrev_declaration_t abbrev;

        abbrev.decode(start, offset);
        abbrevs.push_back(abbrev);
        if (abbrev.abbreviation_code == 0)
            break;
#if DWARF_DEBUG
        printf("Loaded abbreviation: code %d, tag %s, has_children %d\n", (uint32_t)abbrev.abbreviation_code, tag2name(abbrev.tag), abbrev.has_children);
#endif
        for (unsigned i = 0; i < abbrev.attributes.size()-1; ++i)
        {
            abbrev_attr_t a;
            a = abbrev.attributes[i];
#if DWARF_DEBUG
            printf(" attr %s, form %s\n", attr2name(a.name), form2name(a.form));
#endif
        }
    }
    return true;
}

abbrev_declaration_t* dwarf_debug_abbrev_t::find_abbrev(uint32_t abbreviation_code)
{
    for (unsigned int i = 0; i < abbrevs.size(); ++i)
    {
        if (abbrevs[i].abbreviation_code == abbreviation_code)
            return &abbrevs[i];
    }
    return 0;
}
