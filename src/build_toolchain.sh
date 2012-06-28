#!/bin/sh

echo "===================================================================="
echo "I will try to fetch and build everything needed for a freestanding"
echo "cross-compiler toolchain. This includes binutils, gcc, llvm, clang"
echo "and may take quite a while to build. Play some tetris and check back"
echo "every once in a while. The process is largely automatic and should"
echo "not require any manual intervention. Fingers crossed!"
echo
echo "You'll need UNIX tools make, curl and tar."
echo "===================================================================="
echo "WARNING: This script assumes brewed gcc 4.6 installed."
echo "To install: 'brew install gcc --enable-cxx --use-llvm'"
echo "It will take a long time, drink twenty coffees."
echo "WARNING: Obviously, this works on OSX only."
echo "===================================================================="
echo

mkdir -p toolchain/{build-llvm,build-gcc,build-binutils,tarballs}
cd toolchain/

# *** USER-ADJUSTABLE SETTINGS ***

export LLVM_TARGETS=x86,arm
export MAKE_THREADS=4

export LLVM_REVISION=154283
export COMPILER_RT_REVISION=151531
export CLANG_REVISION=154283

# binutils 2.21 won't work, see https://trac.macports.org/ticket/22679
BINUTILS_VER=2.19.1
GCC_VER=4.6.2
MPFR_VER=3.1.0
MPC_VER=0.9
GMP_VER=5.0.4

# END OF USER-ADJUSTABLE SETTINGS

export TOOLCHAIN_DIR=`pwd`
export SOURCE_PREFIX=$TOOLCHAIN_DIR/tarballs

export PREFIX=$TOOLCHAIN_DIR/gcc
export TARGET=i686-pc-elf

# Brewed host gcc 4.6.2
export CC=/usr/local/bin/x86_64-apple-darwin11.3.0-gcc-4.6
export CXX=/usr/local/bin/x86_64-apple-darwin11.3.0-g++-4.6
export LD=/usr/bin/ld # not /usr/bin/gcc-4.2!!
export CPP=/usr/local/bin/cpp-4.6

echo "===================================================================="
echo "Checking out llvm..."
echo "===================================================================="

if [ ! -d llvm ]; then
	svn co -r$LLVM_REVISION http://llvm.org/svn/llvm-project/llvm/trunk llvm
else
	#svn up llvm
	echo "Not checking out llvm, directory exists."
fi

if [ ! -d llvm/projects/compiler-rt ]; then
	cd llvm/projects/
	svn co -r$COMPILER_RT_REVISION http://llvm.org/svn/llvm-project/compiler-rt/trunk compiler-rt
	cd ../..
fi

echo "===================================================================="
echo "Checking out clang..."
echo "===================================================================="

if [ ! -d llvm/tools/clang ]; then
	cd llvm/tools/
	svn co -r$CLANG_REVISION http://llvm.org/svn/llvm-project/cfe/trunk clang
	cd ../..
else
	#svn up llvm/tools/clang
	echo "Not checking out clang, directory exists."
fi

echo "===================================================================="
echo "Fetching binutils..."
echo "===================================================================="
if [ ! -f ${SOURCE_PREFIX}/binutils-${BINUTILS_VER}.tar.bz2 ]; then
	echo "binutils"
	curl http://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VER}.tar.bz2 -o ${SOURCE_PREFIX}/binutils-${BINUTILS_VER}.tar.bz2
fi

echo "===================================================================="
echo "Fetching gcc..."
echo "===================================================================="

if [ ! -f ${SOURCE_PREFIX}/gcc-core-${GCC_VER}.tar.bz2 ]; then
	echo "gcc-core"
	curl http://ftp.gnu.org/gnu/gcc/gcc-${GCC_VER}/gcc-core-${GCC_VER}.tar.bz2 -o ${SOURCE_PREFIX}/gcc-core-${GCC_VER}.tar.bz2
fi

if [ ! -f ${SOURCE_PREFIX}/gcc-g++-${GCC_VER}.tar.bz2 ]; then
	echo "gcc-g++"
	curl http://ftp.gnu.org/gnu/gcc/gcc-${GCC_VER}/gcc-g++-${GCC_VER}.tar.bz2 -o ${SOURCE_PREFIX}/gcc-g++-${GCC_VER}.tar.bz2
fi

echo "===================================================================="
echo "Fetching gcc libraries..."
echo "===================================================================="

if [ ! -f ${SOURCE_PREFIX}/mpfr-${MPFR_VER}.tar.bz2 ]; then
	echo "mpfr"
	curl http://ftp.gnu.org/gnu/mpfr/mpfr-${MPFR_VER}.tar.bz2 -o ${SOURCE_PREFIX}/mpfr-${MPFR_VER}.tar.bz2
fi

if [ ! -f ${SOURCE_PREFIX}/mpc-${MPC_VER}.tar.gz ]; then
	echo "mpc"
	curl http://www.multiprecision.org/mpc/download/mpc-${MPC_VER}.tar.gz -o ${SOURCE_PREFIX}/mpc-${MPC_VER}.tar.gz
fi

if [ ! -f ${SOURCE_PREFIX}/gmp-${GMP_VER}.tar.bz2 ]; then
	echo "gmp"
	curl http://ftp.gnu.org/gnu/gmp/gmp-${GMP_VER}.tar.bz2 -o ${SOURCE_PREFIX}/gmp-${GMP_VER}.tar.bz2
fi

echo "===================================================================="
echo "Augmenting PATH with local binutils..."
echo "===================================================================="
# Clean also your $PATH as much as possible:
export PATH=/usr/bin:/bin:/usr/sbin:/sbin:$PREFIX/bin

echo "===================================================================="
echo "Extracting binutils..."
echo "===================================================================="

if [ ! -d binutils-${BINUTILS_VER} ]; then
	tar xf ${SOURCE_PREFIX}/binutils-${BINUTILS_VER}.tar.bz2
fi

echo "===================================================================="
echo "Configuring binutils..."
echo "===================================================================="

if [ ! -f build-binutils/.config.succeeded ]; then
	cd build-binutils && \
	../binutils-${BINUTILS_VER}/configure --prefix=$PREFIX --target=$TARGET --program-prefix=$TARGET- \
	--enable-languages=c,c++ --disable-werror \
	--disable-nls --disable-shared --disable-multilib \
	--enable-gold --enable-plugins && \
	touch .config.succeeded && \
	cd .. || exit 1
else
	echo "build-binutils/.config.succeeded exists, NOT reconfiguring binutils!"
fi

echo "===================================================================="
echo "Building binutils..."
echo "===================================================================="

if [ ! -f build-binutils/.build.succeeded ]; then
	cd build-binutils && \
	time make -j$MAKE_THREADS && \
	time make -j$MAKE_THREADS all-gold && \
	touch .build.succeeded && \
	cd .. || exit 1
else
	echo "build-binutils/.build.succeeded exists, NOT rebuilding binutils!"
fi

echo "===================================================================="
echo "Installing binutils..."
echo "===================================================================="

if [ ! -f build-binutils/.install.succeeded ]; then
	cd build-binutils && \
	make install && \
	touch .install.succeeded && \
	cd .. || exit 1
else
	echo "build-binutils/.install.succeeded exists, NOT reinstalling binutils!"
fi

export STRIP_FOR_TARGET=$PREFIX/bin/$TARGET-strip

echo "===================================================================="
echo "Extracting gcc..."
echo "===================================================================="

if [ ! -d gcc-${GCC_VER} ]; then
	tar xf ${SOURCE_PREFIX}/gcc-core-${GCC_VER}.tar.bz2
	tar xf ${SOURCE_PREFIX}/gcc-g++-${GCC_VER}.tar.bz2
	tar xf ${SOURCE_PREFIX}/gmp-${GMP_VER}.tar.bz2
	tar xf ${SOURCE_PREFIX}/mpc-${MPC_VER}.tar.gz
	tar xf ${SOURCE_PREFIX}/mpfr-${MPFR_VER}.tar.bz2
	cd gcc-${GCC_VER}
	ln -s ../gmp-${GMP_VER} gmp
	ln -s ../mpc-${MPC_VER} mpc
	ln -s ../mpfr-${MPFR_VER} mpfr
	cd ..
fi

echo "===================================================================="
echo "Configuring gcc..."
echo "===================================================================="

if [ ! -f build-gcc/.config.succeeded ]; then
	cd build-gcc && \
	../gcc-${GCC_VER}/configure --prefix=$PREFIX --target=$TARGET --program-prefix=$TARGET- \
	--with-gmp --with-mpfr --with-mpc --with-system-zlib --enable-stage1-checking --enable-plugin \
	--enable-lto --enable-languages=c,c++ \
	--disable-nls --disable-shared --disable-multilib && \
	touch .config.succeeded && \
	cd .. || exit 1
else
	echo "build-gcc/.config.succeeded exists, NOT reconfiguring gcc!"
fi

echo "===================================================================="
echo "Building gcc..."
echo "===================================================================="

if [ ! -f build-gcc/.build.succeeded ]; then
	cd build-gcc && \
	make -j$MAKE_THREADS all-gcc && \
	make -j$MAKE_THREADS all-target-libgcc && \
	touch .build.succeeded && \
	cd .. || exit 1
else
	echo "build-gcc/.build.succeeded exists, NOT rebuilding gcc!"
fi

echo "===================================================================="
echo "Installing gcc..."
echo "===================================================================="

if [ ! -f build-gcc/.install.succeeded ]; then
	cd build-gcc && \
	make install-gcc && \
	make install-target-libgcc && \
	touch .install.succeeded && \
	cd .. || exit 1
else
	echo "build-gcc/.install.succeeded exists, NOT reinstalling gcc!"
fi

echo "===================================================================="
echo "Configuring llvm..."
echo "===================================================================="

unset CC
unset CXX
unset LD
unset CPP

# To do: add this to build llvm gold plugin and use gold ...
# --with-binutils-include=$TOOLCHAIN_DIR/binutils-${BINUTILS_VER}/include/ --enable-pic

if [ ! -f build-llvm/.config.succeeded ]; then
	cd build-llvm && \
	../llvm/configure --prefix=$TOOLCHAIN_DIR/clang/ --enable-jit --enable-optimized \
	--enable-targets=$LLVM_TARGETS  && \
	touch .config.succeeded && \
	cd .. || exit 1
else
	echo "build-llvm/.config.succeeded exists, NOT reconfiguring llvm!"
fi

echo "===================================================================="
echo "Building llvm... this may take a long while"
echo "===================================================================="

if [ ! -f build-llvm/.build.succeeded ]; then
	cd build-llvm && \
	make -j$MAKE_THREADS && \
	touch .build.succeeded && \
	cd .. || exit 1
else
	echo "build-llvm/.build.succeeded exists, NOT rebuilding llvm!"
fi

echo "===================================================================="
echo "Installing llvm & clang..."
echo "===================================================================="

if [ ! -f build-llvm/.install.succeeded ]; then
	cd build-llvm && \
	make install && \
	touch .install.succeeded && \
	cd .. || exit 1
else
	echo "build-llvm/.install.succeeded exists, NOT reinstalling llvm!"
fi

echo "===================================================================="
echo "To clean up:"
echo "cd toolchain"
echo "rm -rf tarballs build-llvm build-gcc build-binutils"
echo "rm -rf llvm gcc-${GCC_VER} binutils-${BINUTILS_VER}"
echo "rm -rf mpfr-${MPFR_VER} gmp-${GMP_VER} mpc-${MPC_VER}"
echo
echo "Toolchain binaries will remain in gcc/ and clang/"
echo "where Metta configure will find them."
echo "===================================================================="
echo
echo "===================================================================="
echo "===================================================================="
echo "All done, enjoy!"
echo "===================================================================="
echo "===================================================================="
cd ..
