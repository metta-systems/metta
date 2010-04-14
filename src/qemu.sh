#!/bin/sh
[ -n "$NORUN" ] && OPT="-S" || OPT=""
qemu $OPT -s -kernel _build_/x86-pc99-release/kickstart.sys -initrd _build_/x86-pc99-release/kernel-startup.sys -cdrom _build_/x86-pc99-release/metta.iso
