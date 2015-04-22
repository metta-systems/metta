#!/bin/sh

echo "===================================================================="
echo "I will try to fetch and build everything needed for a freestanding"
echo "cross-compiler toolchain. This includes llvm, clang, lld and polly"
echo "and may take quite a while to build. Play some tetris and check back"
echo "every once in a while. The process is largely automatic and should"
echo "not require any manual intervention. Fingers crossed!"
echo
echo "You'll need UNIX tools git, cmake, and ninja."
echo
echo "Specify LIBCXX_TRIPLE if you're not on mac"
echo "Use FETCH_ONLY=1 to only download the sources"
echo "===================================================================="
echo

# *** USER-ADJUSTABLE SETTINGS ***

export LLVM_TARGETS="X86;ARM;AArch64;Mips"

export LLVM_REVISION=master
export CLANG_REVISION=master
export LLD_REVISION=master
export POLLY_REVISION=master
export COMPILER_RT_REVISION=master
export LIBCXXABI_REVISION=master
export LIBCXX_REVISION=master
if [ -z $LIBCXX_TRIPLE ]; then
    export LIBCXX_TRIPLE=-apple-
fi

# END OF USER-ADJUSTABLE SETTINGS

which git || (echo "Install git: brew install git"; exit)
which cmake || (echo "Install cmake: brew install cmake"; exit)
which ninja || (echo "Install ninja: brew install ninja"; exit)

mkdir -p toolchain/{build/{llvm,libcxxabi,libcxx},sources}
cd toolchain/

export TOOLCHAIN_DIR=`pwd`

# *** FUNCTIONS ***

# $1 name
# $2 url
# $3 revision
# $4 path under sources/
function checkout {
    echo "===================================================================="
    echo "Getting $1 [$3]..."

    if [ ! -d sources/$4 ]; then
        git clone -n $2 sources/$4
        (cd sources/$4; git reset --hard $3)
    else
        (cd sources/$4; git fetch; git reset --hard $3)
    fi
}

# $1 path/name
# $2 cmdline
function configure {
    echo "===================================================================="
    echo "Configuring $1..."

    if [ ! -f build/$1/.config.succeeded ]; then
        cd build/$1 && \
        $2 ../../sources/$1 && \
        touch .config.succeeded && \
        cd ../.. || exit 1
    else
        echo "build/$1/.config.succeeded exists, NOT reconfiguring $1!"
    fi
}

# $1 path/name
# $2 optional cmdline, default to `cmake build`
function build {
    echo "===================================================================="
    echo "Building $1..."

    cmd="cmake --build ."
    if [ -n "$2" ]
    then
        cmd="$2"
    fi

    if [ ! -f build/$1/.build.succeeded ]; then
        cd build/$1 && \
        $cmd && \
        touch .build.succeeded && \
        cd ../.. || exit 1
    else
        echo "build/$1/.build.succeeded exists, NOT rebuilding $1!"
    fi
}

# $1 path/name
# $2 optional cmdline, default to `cmake install target`
# $3 optional title (use $1 if empty)
function install {
    echo "===================================================================="
    if [ -z "$3" ]
    then
        echo "Installing $1..."
    else
        echo "Installing $3..."
    fi

    cmd="cmake --build . --target install"
    if [ -n "$2" ]
    then
        cmd="$2"
    fi

    if [ ! -f build/$1/.install.succeeded ]; then
        cd build/$1 && \
        $cmd && \
        touch .install.succeeded && \
        cd ../.. || exit 1
    else
        echo "build/$1/.install.succeeded exists, NOT reinstalling $1!"
    fi
}

# END OF FUNCTIONS

#
# Main action
#
checkout "llvm" "http://llvm.org/git/llvm.git" "$LLVM_REVISION" "llvm"
checkout "compiler-rt" "http://llvm.org/git/compiler-rt.git" "$COMPILER_RT_REVISION" \
    "llvm/projects/compiler-rt"
checkout "clang" "http://llvm.org/git/clang.git" "$CLANG_REVISION" "llvm/tools/clang"
checkout "lld" "http://llvm.org/git/lld.git" "$LLD_REVISION" "llvm/tools/lld"
checkout "polly" "http://llvm.org/git/polly.git" "$POLLY_REVISION" "llvm/tools/polly"
checkout "libc++abi" "http://llvm.org/git/libcxxabi.git" "$LIBCXXABI_REVISION" \
    "llvm/projects/libcxxabi"
checkout "libc++" "http://llvm.org/git/libcxx.git" "$LIBCXX_REVISION" "llvm/projects/libcxx"

if [ -n "$FETCH_ONLY" ] && [ "$FETCH_ONLY" -eq "1" ]; then
    echo "Fetch complete."
    exit 0
fi

configure "llvm" "cmake -G Ninja -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$TOOLCHAIN_DIR/clang \
    -DLLVM_APPEND_VC_REV=ON \
    -DLLVM_BUILD_RUNTIME=ON \
    -DLLVM_ENABLE_CXX1Y=ON -DLLVM_ENABLE_LIBCXX=ON \
    -DLLVM_TARGETS_TO_BUILD=$LLVM_TARGETS"
build "llvm"
install "llvm" "" "llvm, clang, lld, polly, libcxx & libcxxabi"

echo "===================================================================="
echo "To clean up:"
echo "cd toolchain"
echo "rm -rf build sources"
echo
echo "Toolchain binaries will remain in clang/ where Metta configure"
echo "will find them."
echo "===================================================================="
echo
echo "===================================================================="
echo "===================================================================="
echo "All done, enjoy!"
echo "===================================================================="
echo "===================================================================="
cd ..
