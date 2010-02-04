//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"

enum {
    DW_TAG_array_type               = 0x01,
    DW_TAG_class_type               = 0x02,
    DW_TAG_entry_point              = 0x03,
    DW_TAG_enumeration_type         = 0x04,
    DW_TAG_formal_parameter         = 0x05,
    DW_TAG_imported_declaration     = 0x08,
    DW_TAG_label                    = 0x0a,
    DW_TAG_lexical_block            = 0x0b,
    DW_TAG_member                   = 0x0d,
    DW_TAG_pointer_type             = 0x0f,
    DW_TAG_reference_type           = 0x10,
    DW_TAG_compile_unit             = 0x11,
    DW_TAG_string_type              = 0x12,
    DW_TAG_structure_type           = 0x13,
    DW_TAG_subroutine_type          = 0x15,
    DW_TAG_typedef                  = 0x16,
    DW_TAG_union_type               = 0x17,
    DW_TAG_unspecified_parameters   = 0x18,
    DW_TAG_variant                  = 0x19,
    DW_TAG_common_block             = 0x1a,
    DW_TAG_common_inclusion         = 0x1b,
    DW_TAG_inheritance              = 0x1c,
    DW_TAG_inlined_subroutine       = 0x1d,
    DW_TAG_module                   = 0x1e,
    DW_TAG_ptr_to_member_type       = 0x1f,
    DW_TAG_set_type                 = 0x20,
    DW_TAG_subrange_type            = 0x21,
    DW_TAG_with_stmt                = 0x22,
    DW_TAG_access_declaration       = 0x23,
    DW_TAG_base_type                = 0x24,
    DW_TAG_catch_block              = 0x25,
    DW_TAG_const_type               = 0x26,
    DW_TAG_constant                 = 0x27,
    DW_TAG_enumerator               = 0x28,
    DW_TAG_file_type                = 0x29,
    DW_TAG_friend                   = 0x2a,
    DW_TAG_namelist                 = 0x2b,
    DW_TAG_namelist_item            = 0x2c,
    DW_TAG_packed_type              = 0x2d,
    DW_TAG_subprogram               = 0x2e, // NB!
    DW_TAG_template_type_parameter  = 0x2f,
    DW_TAG_template_value_parameter = 0x30,
    DW_TAG_thrown_type              = 0x31,
    DW_TAG_try_block                = 0x32,
    DW_TAG_variant_part             = 0x33,
    DW_TAG_variable                 = 0x34,
    DW_TAG_volatile_type            = 0x35,
    DW_TAG_dwarf_procedure          = 0x36, // dwarf3
    DW_TAG_restrict_type            = 0x37, // dwarf3
    DW_TAG_interface_type           = 0x38, // dwarf3
    DW_TAG_namespace                = 0x39, // dwarf3
    DW_TAG_imported_module          = 0x3a, // dwarf3
    DW_TAG_unspecified_type         = 0x3b, // dwarf3
    DW_TAG_partial_unit             = 0x3c, // dwarf3
    DW_TAG_imported_unit            = 0x3d, // dwarf3
    DW_TAG_condition                = 0x3f, // dwarf3
    DW_TAG_shared_type              = 0x40, // dwarf3
    DW_TAG_lo_user                  = 0x4080,
    DW_TAG_hi_user                  = 0xffff
};

enum {
    DW_CHILDREN_no  = 0x00,
    DW_CHILDREN_yes = 0x01
};

enum {                                // Classes:
    DW_AT_sibling              = 0x01, // reference
    DW_AT_location             = 0x02, // block, loclistptr
    DW_AT_name                 = 0x03, // string
    DW_AT_ordering             = 0x09, // constant
    DW_AT_byte_size            = 0x0b, // block, constant, reference
    DW_AT_bit_offset           = 0x0c, // block, constant, reference
    DW_AT_bit_size             = 0x0d, // block, constant, reference
    DW_AT_stmt_list            = 0x10, // lineptr
    DW_AT_low_pc               = 0x11, // address
    DW_AT_high_pc              = 0x12, // address
    DW_AT_language             = 0x13, // constant
    DW_AT_discr                = 0x15, // reference
    DW_AT_discr_value          = 0x16, // constant
    DW_AT_visibility           = 0x17, // constant
    DW_AT_import               = 0x18, // reference
    DW_AT_string_length        = 0x19, // block, loclistptr
    DW_AT_common_reference     = 0x1a, // reference
    DW_AT_comp_dir             = 0x1b, // string
    DW_AT_const_value          = 0x1c, // block, constant, string
    DW_AT_containing_type      = 0x1d, // reference
    DW_AT_default_value        = 0x1e, // reference
    DW_AT_inline               = 0x20, // constant
    DW_AT_is_optional          = 0x21, // flag
    DW_AT_lower_bound          = 0x22, // block, constant, reference
    DW_AT_producer             = 0x25, // string
    DW_AT_prototyped           = 0x27, // flag
    DW_AT_return_addr          = 0x2a, // block, loclistptr
    DW_AT_start_scope          = 0x2c, // constant
    DW_AT_bit_stride           = 0x2e, // constant
    DW_AT_upper_bound          = 0x2f, // block, constant, reference
    DW_AT_abstract_origin      = 0x31, // reference
    DW_AT_accessibility        = 0x32, // constant
    DW_AT_address_class        = 0x33, // constant
    DW_AT_artificial           = 0x34, // flag
    DW_AT_base_types           = 0x35, // reference
    DW_AT_calling_convention   = 0x36, // constant
    DW_AT_count                = 0x37, // block, constant, reference
    DW_AT_data_member_location = 0x38, // block, constant, loclistptr
    DW_AT_decl_column          = 0x39, // constant
    DW_AT_decl_file            = 0x3a, // constant
    DW_AT_decl_line            = 0x3b, // constant
    DW_AT_declaration          = 0x3c, // flag
    DW_AT_discr_list           = 0x3d, // block
    DW_AT_encoding             = 0x3e, // constant
    DW_AT_external             = 0x3f, // flag
    DW_AT_frame_base           = 0x40, // block, loclistptr
    DW_AT_friend               = 0x41, // reference
    DW_AT_identifier_case      = 0x42, // constant
    DW_AT_macro_info           = 0x43, // macptr
    DW_AT_namelist_item        = 0x44, // block
    DW_AT_priority             = 0x45, // reference
    DW_AT_segment              = 0x46, // block, loclistptr
    DW_AT_specification        = 0x47, // reference
    DW_AT_static_link          = 0x48, // block, loclistptr
    DW_AT_type                 = 0x49, // reference
    DW_AT_use_location         = 0x4a, // block, loclistptr
    DW_AT_variable_parameter   = 0x4b, // flag
    DW_AT_virtuality           = 0x4c, // constant
    DW_AT_vtable_elem_location = 0x4d, // block, loclistptr
    DW_AT_allocated            = 0x4e, // block, constant, reference (dwarf3)
    DW_AT_associated           = 0x4f, // block, constant, reference (dwarf3)
    DW_AT_data_location        = 0x50, // block (dwarf3)
    DW_AT_byte_stride          = 0x51, // block, constant, reference (dwarf3)
    DW_AT_entry_pc             = 0x52, // address (dwarf3)
    DW_AT_use_UTF8             = 0x53, // flag (dwarf3)
    DW_AT_extension            = 0x54, // reference (dwarf3)
    DW_AT_ranges               = 0x55, // rangelistptr (dwarf3)
    DW_AT_trampoline           = 0x56, // address, flag, reference, string (dwarf3)
    DW_AT_call_column          = 0x57, // constant (dwarf3)
    DW_AT_call_file            = 0x58, // constant (dwarf3)
    DW_AT_call_line            = 0x59, // constant (dwarf3)
    DW_AT_description          = 0x5a, // string (dwarf3)
    DW_AT_binary_scale         = 0x5b, // constant (dwarf3)
    DW_AT_decimal_scale        = 0x5c, // constant (dwarf3)
    DW_AT_small                = 0x5d, // reference (dwarf3)
    DW_AT_decimal_sign         = 0x5e, // constant (dwarf3)
    DW_AT_digit_count          = 0x5f, // constant (dwarf3)
    DW_AT_picture_string       = 0x60, // string (dwarf3)
    DW_AT_mutable              = 0x61, // flag (dwarf3)
    DW_AT_threads_scaled       = 0x62, // flag (dwarf3)
    DW_AT_explicit             = 0x63, // flag (dwarf3)
    DW_AT_object_pointer       = 0x64, // reference (dwarf3)
    DW_AT_endianity            = 0x65, // constant (dwarf3)
    DW_AT_elemental            = 0x66, // flag (dwarf3)
    DW_AT_pure                 = 0x67, // flag (dwarf3)
    DW_AT_recursive            = 0x68, // flag (dwarf3)
    DW_AT_lo_user              = 0x2000,
    DW_AT_GNU_cpp_mangled_name = 0x2007, // C++ mangled name
    DW_AT_hi_user              = 0x3fff
};

enum {
    DW_FORM_addr      = 0x01, // address
    DW_FORM_block2    = 0x03, // block
    DW_FORM_block4    = 0x04, // block
    DW_FORM_data2     = 0x05, // constant
    DW_FORM_data4     = 0x06, // constant, lineptr, loclistptr, macptr, rangelistptr
    DW_FORM_data8     = 0x07, // constant, lineptr, loclistptr, macptr, rangelistptr
    DW_FORM_string    = 0x08, // string
    DW_FORM_block     = 0x09, // block
    DW_FORM_block1    = 0x0a, // block
    DW_FORM_data1     = 0x0b, // constant
    DW_FORM_flag      = 0x0c, // flag
    DW_FORM_sdata     = 0x0d, // constant
    DW_FORM_strp      = 0x0e, // string
    DW_FORM_udata     = 0x0f, // constant
    DW_FORM_ref_addr  = 0x10, // reference
    DW_FORM_ref1      = 0x11, // reference
    DW_FORM_ref2      = 0x12, // reference
    DW_FORM_ref4      = 0x13, // reference
    DW_FORM_ref8      = 0x14, // reference
    DW_FORM_ref_udata = 0x15, // reference
    DW_FORM_indirect  = 0x16
};

extern const char* tag2name(uint32_t tag);
extern const char* attr2name(uint32_t attr);
extern const char* form2name(uint32_t form);
