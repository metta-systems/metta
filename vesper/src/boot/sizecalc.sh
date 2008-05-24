#!/bin/bash

OFFSET=`ls -l grub|grep -v total|awk 'BEGIN {sum=0} {sum = sum + int(($5+511)/512)} END{print sum}'`
SIZE=`ls -l kernel.bin|grep -v total|awk '{print int(($5+511)/512)}'`

echo "Start bochs then enter in grub menu:"
echo "> kernel ${OFFSET}+${SIZE}"
echo "> boot"
