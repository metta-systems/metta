# Build bochs on SnowLeo, taken from http://stackoverflow.com/questions/1677324/compiling-bochs-on-mac-os-x-snow-leopard

set echo
PREFIX="/usr/local"
CFLAGS="-arch i386 -m32 -pipe -O3 -I$PREFIX/include -fomit-frame-pointer -finline-functions -falign-loops=16 -falign-jumps=16 -falign-functions=16 -falign-labels=16 -falign-loops-max-skip=15 -falign-jumps-max-skip=15 -fprefetch-loop-arrays $CFLAGS"
CPATH="$PREFIX/include"
CPPFLAGS=""
CXXFLAGS="$CFLAGS"
LDFLAGS="-arch i386 -m32 -L$PREFIX/lib"
CXX="g++ -arch i386 -m32"

export CFLAGS
export CPATH
export CPPFLAGS
export CXXFLAGS
export LDFLAGS
export CXX

../configure \
	--enable-sb16 \
	--enable-ne2000 \
	--enable-all-optimizations \
	--enable-cpu-level=6 \
	--enable-x86-64 \
	--enable-pci \
	--enable-acpi \
	--enable-clgd54xx \
	--enable-usb \
	--enable-usb-ohci \
	--enable-plugins \
	--enable-show-ips \
	--enable-debugger \
	--disable-debugger-gui \
	--prefix=$PREFIX \
	--enable-smp \
	--enable-iodebug \
	--without-x \
	--without-x11 \
	--without-beos \
	--without-win32 \
	--without-macos \
	--with-carbon \
	--with-nogui \
	--with-term \
	--without-rfb \
	--without-amigaos \
	--without-sdl \
	--without-svga \
	--without-wx
