message("## Configuring for UEFI platform")

# uefi - core2duo or later, 64 bit, using uefi bootloader, gpt partition tables and all that

set(TARGET "x86_64-pc-elf" CACHE FORCE "target set by platform uefi.cmake")
set(BOARD "" CACHE FORCE "board set by platform uefi.cmake")
set(ARCH "x86_64" CACHE FORCE "arch set by platform uefi.cmake")
set(PLATFORM "uefi" CACHE FORCE "platform set by platform uefi.cmake")
