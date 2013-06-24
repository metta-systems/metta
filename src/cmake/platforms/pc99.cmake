message("## Configuring for PC99 platform")

# pc99 - pentium4 or later, 32 bit, using BIOS, GRUB multiboot loading...

set(TARGET "i686-pc-elf" CACHE FORCE "target set by platform pc99.cmake")
set(BOARD "" CACHE FORCE "board set by platform pc99.cmake")
set(ARCH "x86" CACHE FORCE "arch set by platform pc99.cmake")
set(PLATFORM "pc99" CACHE FORCE "platform set by platform pc99.cmake")
