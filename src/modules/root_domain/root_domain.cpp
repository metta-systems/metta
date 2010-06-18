#include "bootimage.h"
#include "elf_parser.h"
#include "root_domain.h"

// root_domain_t::root_domain_t(bootimage_t& img)
// {
//     start = img.find_root_domain(&size);
// }
// 
// address_t root_domain_t::entry()
// {
//     elf_parser_t elf(start);
//     return elf.get_entry_point();
// }
