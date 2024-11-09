#!/bin/sh

echo "===================================================================="
echo "I will try to fetch and build everything needed for a freestanding"
echo "cross-compiler toolchain. This includes llvm, clang, and lld"
echo "and may take quite a while to build. Play some tetris and check back"
echo "every once in a while. The process is largely automatic and should"
echo "not require any manual intervention. Fingers crossed!"
echo
echo "You'll need UNIX tools git, cmake, and ninja."
echo
echo "Specify LIBCXX_TRIPLE if you're not on mac"
echo "===================================================================="
echo

# *** USER-ADJUSTABLE SETTINGS ***

#X86;ARM;AArch64;Mips;
export LLVM_TARGETS="RISCV"

export LLVM_REVISION=main
if [ -z $LIBCXX_TRIPLE ]; then
    export LIBCXX_TRIPLE=-apple-
fi

# END OF USER-ADJUSTABLE SETTINGS

which git || (echo "Install git: brew install git"; exit)
which cmake || (echo "Install cmake: brew install cmake"; exit)
which ninja || (echo "Install ninja: brew install ninja"; exit)

mkdir -p toolchain/{build/llvm-project,sources}
pushd toolchain/

export TOOLCHAIN_DIR=`pwd`

REPOBASE=https://github.com/llvm

echo "===================================================================="
echo "Checking out llvm-project [$LLVM_REVISION]..."
echo "===================================================================="

if [ ! -d sources/llvm-project ]; then
    git clone --depth 1 --shallow-submodules --no-tags -b $LLVM_REVISION $REPOBASE/llvm-project.git sources/llvm-project
else
    (cd sources/llvm-project; git fetch; git checkout $LLVM_REVISION)
fi

echo "===================================================================="
echo "Configuring llvm..."
echo "===================================================================="
#-DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra;libcxx;libcxxabi;libunwind;lldb;compiler-rt;lld;polly" \

if [ ! -f build/llvm-project/.config.succeeded ]; then
    pushd build/llvm-project && \
    cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$TOOLCHAIN_DIR/llvm \
        -DLLVM_ENABLE_PROJECTS="clang;lldb" \
        -DLLVM_TARGETS_TO_BUILD=$LLVM_TARGETS \
        -DLLVM_USE_SPLIT_DWARF=True -DLLVM_OPTIMIZED_TABLEGEN=True \
        -DLLVM_BUILD_TESTS=False -DLLVM_INCLUDE_TESTS=False -DLLDB_INCLUDE_TESTS=False \
        -DLLVM_BUILD_DOCS=False -DLLVM_INCLUDE_DOCS=False \
        -DLLVM_ENABLE_OCAMLDOC=False -DLLVM_ENABLE_BINDINGS=False \
        -DLLDB_USE_SYSTEM_DEBUGSERVER=True \
        ../../sources/llvm-project/llvm && \
    touch .config.succeeded && \
    popd || exit 1
else
    echo "build/llvm-project/.config.succeeded exists, NOT reconfiguring llvm!"
fi

echo "===================================================================="
echo "Building llvm... this may take a long while"
echo "===================================================================="

if [ ! -f build/llvm-project/.build.succeeded ]; then
    pushd build/llvm-project && \
    cmake --build . && \
    touch .build.succeeded && \
    popd || exit 1
else
    echo "build/llvm-project/.build.succeeded exists, NOT rebuilding llvm!"
fi

echo "===================================================================="
echo "Installing llvm and all tools..."
echo "===================================================================="

if [ ! -f build/llvm-project/.install.succeeded ]; then
    pushd build/llvm-project && \
    cmake --build . --target install && \
    touch .install.succeeded && \
    popd || exit 1
else
    echo "build/llvm-project/.install.succeeded exists, NOT reinstalling llvm!"
fi

popd
exit 0

echo "===================================================================="
echo "===================================================================="
echo "Rebuilding LLVM libraries with freshly installed clang..."
echo "===================================================================="
echo "===================================================================="

# TODO: copy libc++.so.1 to clang bin directory for Linux building or adjust LD_LIBRARY_PATH...

echo "===================================================================="
echo "Configuring llvm..."
echo "===================================================================="

# We rebuild using just built fresh clang for the sole reason of being able
# to use recent libcxx (which we link against in tools), so LLVM libs have
# to be built against this same libcxx too.

# Check if polly and lld can be built with this llvm version without errors
# and enable:
# --enable-polly

# Force use of local libcxx for new clang build.
# This doesn't enable the options, merely records them, the real activation
# happens below in make command invocation.

export EXTRA_OPTIONS="-I$TOOLCHAIN_DIR/libcxx/include"
export EXTRA_LD_OPTIONS="-L$TOOLCHAIN_DIR/libcxx/lib -lc++"

if [ ! -f build/llvm2/.config2.succeeded ]; then
    cd build/llvm2 && \
    CC=$TOOLCHAIN_DIR/clang/bin/clang CXX=$TOOLCHAIN_DIR/clang/bin/clang++ \
    ../../sources/llvm/configure --prefix=$TOOLCHAIN_DIR/clang/ --enable-jit --enable-optimized \
    --enable-libcpp --disable-docs \
    --with-binutils-include=$TOOLCHAIN_DIR/sources/binutils-${BINUTILS_VER}/include/ --enable-pic \
    --enable-targets=$LLVM_TARGETS  && \
    touch .config2.succeeded && \
    cd ../.. || exit 1
else
    echo "build/llvm2/.config2.succeeded exists, NOT reconfiguring llvm!"
fi

echo "===================================================================="
echo "Building llvm... this may take a long while"
echo "===================================================================="

if [ ! -f build/llvm2/.build2.succeeded ]; then
    cd build/llvm2 && \
    make -j$MAKE_THREADS EXTRA_OPTIONS="$EXTRA_OPTIONS" EXTRA_LD_OPTIONS="$EXTRA_LD_OPTIONS" && \
    make check && \
    touch .build2.succeeded && \
    cd ../.. || exit 1
else
    echo "build/llvm2/.build2.succeeded exists, NOT rebuilding llvm!"
fi

echo "===================================================================="
echo "Installing llvm & clang..."
echo "===================================================================="

if [ ! -f build/llvm2/.install2.succeeded ]; then
    cd build/llvm2 && \
    make install EXTRA_OPTIONS="$EXTRA_OPTIONS" EXTRA_LD_OPTIONS="$EXTRA_LD_OPTIONS" && \
    touch .install2.succeeded && \
    cd ../.. || exit 1
else
    echo "build/llvm2/.install2.succeeded exists, NOT reinstalling llvm!"
fi

echo "===================================================================="
echo "To clean up:"
echo "cd toolchain"
echo "rm -rf build sources"
echo
echo "Toolchain binaries will remain in clang/ and libcxx/"
echo "where Metta configure will find them."
echo "===================================================================="
echo
echo "===================================================================="
echo "===================================================================="
echo "All done, enjoy!"
echo "===================================================================="
echo "===================================================================="
popd

