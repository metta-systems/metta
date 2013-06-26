mkdir -p _build_host_ _build_target_

# First build the host executables using native compiler and environment.
cd _build_host_
cmake -G Ninja -DCONFIG_PLATFORM=pc99 ..
ninja

# Then build the target system using cross toolchain we built.
cd ../_build_target_
cmake -G Ninja -DCONFIG_PLATFORM=pc99 -DCMAKE_TOOLCHAIN_FILE=../cmake/cross.toolchain -DIMPORT_EXECUTABLES=../_build_host_/ImportExecutables.cmake ..
ninja metta

cd ..
