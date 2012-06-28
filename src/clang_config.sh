PATH=`pwd`/toolchain/clang/bin:$PATH \
CLANG=`pwd`/toolchain/clang/bin/clang++ \
CPP=cpp-4.6 \
CROSS_CC=`pwd`/toolchain/clang/bin/clang \
CROSS_CXX=`pwd`/toolchain/clang/bin/clang++ \
AS=i686-pc-elf-as \
AR=i686-pc-elf-ar \
RANLIB=i686-pc-elf-ranlib \
NM=i686-pc-elf-nm \
./waf configure --clang=yes

# as ar etc could be set up using -gcc-toolchain param for clang
