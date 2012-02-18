YASM=/usr/local/bin/yasm CROSSPFX=/opt/crossgcc/bin/i586-elf- CC=${CROSSPFX}gcc CXX=${CROSSPFX}g++ AR=${CROSSPFX}ar RANLIB=${CROSSPFX}ranlib CPP=${CROSSPFX}cpp ./waf
#CLANG=clang-2.9 CPP=/usr/local/bin/i686-elf-cpp AR=/usr/local/bin/i686-elf-ar RANLIB=/usr/local/bin/i686-elf-ranlib CC=/usr/local/bin/i686-elf-gcc CXX=/usr/local/bin/i686-elf-g++ ./waf configure --clang=yes
