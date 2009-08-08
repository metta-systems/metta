################ Library version #####################################

LIBNAME		= ustl
MAJOR		= 1
MINOR		= 2
BUILD		= 0

################ Build options #######################################

BUILD_SHARED	= 1 
#BUILD_STATIC	= 1
#DEBUG		= 1
NOLIBSTDCPP	= 1

################ Progams #############################################

CXX		= g++ 
LD		= g++ 
AR		= ar
RANLIB		= ranlib
DOXYGEN		= doxygen
INSTALL		= /bin/install

INSTALLDIR	= ${INSTALL} -d
INSTALLLIB	= ${INSTALL} -p -m 644
INSTALLEXE	= ${INSTALL} -p -m 755
INSTALLDATA	= ${INSTALL} -p -m 644

################ Destination #########################################

prefix		= /usr/local
exec_prefix	= /usr/local
BINDIR		= /usr/local/bin
INCDIR		= /usr/local/include
LIBDIR		= /usr/local/lib

################ Compiler options ####################################

WARNOPTS	= -Wall -Wpointer-arith -Wno-cast-align -Wcast-qual -Wsynth \
		-W -Wconversion -Wsign-promo -Woverloaded-virtual -Wshadow  \
		-Wwrite-strings -Wredundant-decls 
TGT_OPTS	= -mmmx -msse -mfpmath=sse -msse2  \
		 --param max-inline-insns-single=1024 \
		--param large-function-growth=65535 \
		--param inline-unit-growth=1024 \
		-fvisibility-inlines-hidden

CXXFLAGS	=   ${TGT_OPTS} ${WARNOPTS}
LDFLAGS		=  
ifdef DEBUG
    CXXFLAGS	+= -O0 -g
else
    CXXFLAGS	+= -O3 -DNDEBUG=1
    LDFLAGS	+= -s
endif

################ Linker options ######################################

ifdef NOLIBSTDCPP
    LD		= gcc
    STAL_LIBS	= -lsupc++   
    LIBS	= ${STAL_LIBS}
endif
SHBLDFL		= -shared -Wl,-soname=${LIBSOLNK}
LIBA		= lib${LIBNAME}.a
LIBSO		= lib${LIBNAME}.so
ifdef MAJOR
    LIBSOLNK	= ${LIBSO}.${MAJOR}
    LIBSOBLD	= ${LIBSO}.${MAJOR}.${MINOR}.${BUILD}
endif

