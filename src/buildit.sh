#!/bin/sh

mkdir -p _build_host_ _build_target_

# First build the host executables using native compiler and environment.
echo
echo "*** Making tools"
echo
cd _build_host_
if [ ! -f CMakeCache.txt ]; then
    cmake -G Ninja -DCONFIG_PLATFORM=pc99 \
        -DLLVM_DIR=$(pwd)/../../toolchain/clang/lib/cmake/llvm ..
fi
ninja

# Then build the target system using cross toolchain we built.
echo
echo "*** Making system"
echo
cd ../_build_target_
if [ ! -f CMakeCache.txt ]; then
	cmake -G Ninja -DCONFIG_PLATFORM=pc99 -DCMAKE_TOOLCHAIN_FILE=../cmake/cross.toolchain -DIMPORT_EXECUTABLES=../_build_host_/ImportExecutables.cmake ..
fi
ninja

cd ..
