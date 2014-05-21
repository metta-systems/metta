message("## Configuring for Raspberry Pi board")

set(TARGET "armv6hf-elf-eabi" CACHE FORCE "target set by board raspi.cmake")
set(BOARD "raspi" CACHE FORCE "board set by board raspi.cmake")
set(ARCH "armv6" CACHE FORCE "arch set by board raspi.cmake")
set(PLATFORM "raspi" CACHE FORCE "platform set by board raspi.cmake")
