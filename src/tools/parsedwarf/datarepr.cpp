//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2010 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "datarepr.h"

const char* tag2name(uint32_t tag)
{
#define TAGNAME(tag) case tag: return #tag
    switch (tag) {
        TAGNAME(DW_TAG_array_type               );
        TAGNAME(DW_TAG_class_type               );
        TAGNAME(DW_TAG_entry_point              );
        TAGNAME(DW_TAG_enumeration_type         );
        TAGNAME(DW_TAG_formal_parameter         );
        TAGNAME(DW_TAG_imported_declaration     );
        TAGNAME(DW_TAG_label                    );
        TAGNAME(DW_TAG_lexical_block            );
        TAGNAME(DW_TAG_member                   );
        TAGNAME(DW_TAG_pointer_type             );
        TAGNAME(DW_TAG_reference_type           );
        TAGNAME(DW_TAG_compile_unit             );
        TAGNAME(DW_TAG_string_type              );
        TAGNAME(DW_TAG_structure_type           );
        TAGNAME(DW_TAG_subroutine_type          );
        TAGNAME(DW_TAG_typedef                  );
        TAGNAME(DW_TAG_union_type               );
        TAGNAME(DW_TAG_unspecified_parameters   );
        TAGNAME(DW_TAG_variant                  );
        TAGNAME(DW_TAG_common_block             );
        TAGNAME(DW_TAG_common_inclusion         );
        TAGNAME(DW_TAG_inheritance              );
        TAGNAME(DW_TAG_inlined_subroutine       );
        TAGNAME(DW_TAG_module                   );
        TAGNAME(DW_TAG_ptr_to_member_type       );
        TAGNAME(DW_TAG_set_type                 );
        TAGNAME(DW_TAG_subrange_type            );
        TAGNAME(DW_TAG_with_stmt                );
        TAGNAME(DW_TAG_access_declaration       );
        TAGNAME(DW_TAG_base_type                );
        TAGNAME(DW_TAG_catch_block              );
        TAGNAME(DW_TAG_const_type               );
        TAGNAME(DW_TAG_constant                 );
        TAGNAME(DW_TAG_enumerator               );
        TAGNAME(DW_TAG_file_type                );
        TAGNAME(DW_TAG_friend                   );
        TAGNAME(DW_TAG_namelist                 );
        TAGNAME(DW_TAG_namelist_item            );
        TAGNAME(DW_TAG_packed_type              );
        TAGNAME(DW_TAG_subprogram               );
        TAGNAME(DW_TAG_template_type_parameter  );
        TAGNAME(DW_TAG_template_value_parameter );
        TAGNAME(DW_TAG_thrown_type              );
        TAGNAME(DW_TAG_try_block                );
        TAGNAME(DW_TAG_variant_part             );
        TAGNAME(DW_TAG_variable                 );
        TAGNAME(DW_TAG_volatile_type            );
        TAGNAME(DW_TAG_dwarf_procedure          );
        TAGNAME(DW_TAG_restrict_type            );
        TAGNAME(DW_TAG_interface_type           );
        TAGNAME(DW_TAG_namespace                );
        TAGNAME(DW_TAG_imported_module          );
        TAGNAME(DW_TAG_unspecified_type         );
        TAGNAME(DW_TAG_partial_unit             );
        TAGNAME(DW_TAG_imported_unit            );
        TAGNAME(DW_TAG_condition                );
        TAGNAME(DW_TAG_shared_type              );
    }
    return "<UNKNOWN TAG>";
#undef TAGNAME
}

const char* attr2name(uint32_t attr)
{
#define ATTRNAME(attr) case attr: return #attr
    switch (attr) {
        ATTRNAME(DW_AT_sibling              );
        ATTRNAME(DW_AT_location             );
        ATTRNAME(DW_AT_name                 );
        ATTRNAME(DW_AT_ordering             );
        ATTRNAME(DW_AT_byte_size            );
        ATTRNAME(DW_AT_bit_offset           );
        ATTRNAME(DW_AT_bit_size             );
        ATTRNAME(DW_AT_stmt_list            );
        ATTRNAME(DW_AT_low_pc               );
        ATTRNAME(DW_AT_high_pc              );
        ATTRNAME(DW_AT_language             );
        ATTRNAME(DW_AT_discr                );
        ATTRNAME(DW_AT_discr_value          );
        ATTRNAME(DW_AT_visibility           );
        ATTRNAME(DW_AT_import               );
        ATTRNAME(DW_AT_string_length        );
        ATTRNAME(DW_AT_common_reference     );
        ATTRNAME(DW_AT_comp_dir             );
        ATTRNAME(DW_AT_const_value          );
        ATTRNAME(DW_AT_containing_type      );
        ATTRNAME(DW_AT_default_value        );
        ATTRNAME(DW_AT_inline               );
        ATTRNAME(DW_AT_is_optional          );
        ATTRNAME(DW_AT_lower_bound          );
        ATTRNAME(DW_AT_producer             );
        ATTRNAME(DW_AT_prototyped           );
        ATTRNAME(DW_AT_return_addr          );
        ATTRNAME(DW_AT_start_scope          );
        ATTRNAME(DW_AT_bit_stride           );
        ATTRNAME(DW_AT_upper_bound          );
        ATTRNAME(DW_AT_abstract_origin      );
        ATTRNAME(DW_AT_accessibility        );
        ATTRNAME(DW_AT_address_class        );
        ATTRNAME(DW_AT_artificial           );
        ATTRNAME(DW_AT_base_types           );
        ATTRNAME(DW_AT_calling_convention   );
        ATTRNAME(DW_AT_count                );
        ATTRNAME(DW_AT_data_member_location );
        ATTRNAME(DW_AT_decl_column          );
        ATTRNAME(DW_AT_decl_file            );
        ATTRNAME(DW_AT_decl_line            );
        ATTRNAME(DW_AT_declaration          );
        ATTRNAME(DW_AT_discr_list           );
        ATTRNAME(DW_AT_encoding             );
        ATTRNAME(DW_AT_external             );
        ATTRNAME(DW_AT_frame_base           );
        ATTRNAME(DW_AT_friend               );
        ATTRNAME(DW_AT_identifier_case      );
        ATTRNAME(DW_AT_macro_info           );
        ATTRNAME(DW_AT_namelist_item        );
        ATTRNAME(DW_AT_priority             );
        ATTRNAME(DW_AT_segment              );
        ATTRNAME(DW_AT_specification        );
        ATTRNAME(DW_AT_static_link          );
        ATTRNAME(DW_AT_type                 );
        ATTRNAME(DW_AT_use_location         );
        ATTRNAME(DW_AT_variable_parameter   );
        ATTRNAME(DW_AT_virtuality           );
        ATTRNAME(DW_AT_vtable_elem_location );
        ATTRNAME(DW_AT_allocated            );
        ATTRNAME(DW_AT_associated           );
        ATTRNAME(DW_AT_data_location        );
        ATTRNAME(DW_AT_byte_stride          );
        ATTRNAME(DW_AT_entry_pc             );
        ATTRNAME(DW_AT_use_UTF8             );
        ATTRNAME(DW_AT_extension            );
        ATTRNAME(DW_AT_ranges               );
        ATTRNAME(DW_AT_trampoline           );
        ATTRNAME(DW_AT_call_column          );
        ATTRNAME(DW_AT_call_file            );
        ATTRNAME(DW_AT_call_line            );
        ATTRNAME(DW_AT_description          );
        ATTRNAME(DW_AT_binary_scale         );
        ATTRNAME(DW_AT_decimal_scale        );
        ATTRNAME(DW_AT_small                );
        ATTRNAME(DW_AT_decimal_sign         );
        ATTRNAME(DW_AT_digit_count          );
        ATTRNAME(DW_AT_picture_string       );
        ATTRNAME(DW_AT_mutable              );
        ATTRNAME(DW_AT_threads_scaled       );
        ATTRNAME(DW_AT_explicit             );
        ATTRNAME(DW_AT_object_pointer       );
        ATTRNAME(DW_AT_endianity            );
        ATTRNAME(DW_AT_elemental            );
        ATTRNAME(DW_AT_pure                 );
        ATTRNAME(DW_AT_recursive            );
        ATTRNAME(DW_AT_GNU_cpp_mangled_name );
    }
    return "<UNKNOWN ATTR>";
#undef ATTRNAME
}

const char* form2name(uint32_t form)
{
#define FORMNAME(form) case form: return #form
    switch (form) {
        FORMNAME(DW_FORM_addr      );
        FORMNAME(DW_FORM_block2    );
        FORMNAME(DW_FORM_block4    );
        FORMNAME(DW_FORM_data2     );
        FORMNAME(DW_FORM_data4     );
        FORMNAME(DW_FORM_data8     );
        FORMNAME(DW_FORM_string    );
        FORMNAME(DW_FORM_block     );
        FORMNAME(DW_FORM_block1    );
        FORMNAME(DW_FORM_data1     );
        FORMNAME(DW_FORM_flag      );
        FORMNAME(DW_FORM_sdata     );
        FORMNAME(DW_FORM_strp      );
        FORMNAME(DW_FORM_udata     );
        FORMNAME(DW_FORM_ref_addr  );
        FORMNAME(DW_FORM_ref1      );
        FORMNAME(DW_FORM_ref2      );
        FORMNAME(DW_FORM_ref4      );
        FORMNAME(DW_FORM_ref8      );
        FORMNAME(DW_FORM_ref_udata );
        FORMNAME(DW_FORM_indirect  );
    }
    return "<UNKNOWN FORM>";
#undef FORMNAME
}
