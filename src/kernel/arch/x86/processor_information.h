#pragma once

enum processor_type_e
{
    processor_type_unknown,
    processor_type_486,
    processor_type_pentium,
    processor_type_ppro
};

struct processor_info_t
{
//     cpu_id_t cpu_id;
//     apic_id_t apic_id;
    processor_type_e type;
    uint8_t          vendor[16];
    unsigned int     stepping : 4;
    unsigned int     model : 4;
    unsigned int     family: 4;
    uint32_t         features;
};
