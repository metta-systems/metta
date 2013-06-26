#cmakedefine CONFIG_PLATFORM "@PLATFORM@"
#cmakedefine CONFIG_BOARD "@BOARD@"
#cmakedefine CONFIG_ARCH "@ARCH@"
#cmakedefine CONFIG_TARGET "@TARGET@"

#cmakedefine CONFIG_INLINING 1
#cmakedefine CONFIG_DEBUG_SYMBOLS 1
#cmakedefine SYSTEM_DEBUG 1
#cmakedefine SYSTEM_VERBOSE_DEBUG 0
#cmakedefine HEAP_DEBUG 0
#cmakedefine MEMORY_DEBUG 1
#cmakedefine BOOTIMAGE_DEBUG 0
#cmakedefine DWARF_DEBUG 0
#cmakedefine RAMTAB_DEBUG 0
/* Overarching tools debugging enabler, disable to turn off all tools debugging prints. */
#cmakedefine TOOLS_DEBUG 1
/* Per-tool: Enable Meddler debug prints. Needs TOOLS_DEBUG. */
#cmakedefine MEDDLER_DEBUG 0
#cmakedefine CONFIG_COMPORT 0
#cmakedefine CONFIG_COMSPEED 115200
#cmakedefine CONFIG_X86_PSE 1
#cmakedefine CONFIG_X86_PGE 1
#cmakedefine CONFIG_X86_FXSR 1
#cmakedefine CONFIG_X86_SYSENTER 1
#cmakedefine CONFIG_IOAPIC 1
#cmakedefine PCIBUS_TEST 1
