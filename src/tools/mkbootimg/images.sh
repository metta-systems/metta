#!/bin/sh
# Script taken from Pedigree and slightly modified.

BUILDDIR=../../_build_
CDISO=$BUILDDIR/metta.iso

# Create an ISO image
mkdir -p iso/boot/grub
cp stage2_eltorito iso/boot/grub
cp $BUILDDIR/tools/mkbootimg/menu.lst.cd  iso/boot/grub/menu.lst
cp $BUILDDIR/launcher.sys      iso/launcher
cp $BUILDDIR/nucleus.sys       iso/nucleus
cp $BUILDDIR/init.img          iso/sysimage
mkisofs -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table -input-charset utf-8 -o $CDISO iso
rm -rf iso
