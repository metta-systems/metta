#! /usr/bin/env python
# -*- coding: utf-8 -*-

import os,sys
from waflib.Tools import ccroot, ar
from waflib import Utils,Configure,Options
from waflib.Configure import conf
@conf
def find_clang(conf):
    v=conf.env
    clang=None
    if v['CLANG']:clang=v['CLANG']
    elif'CLANG'in conf.environ:cxx=conf.environ['CLANG']
    if not clang:clang=conf.find_program('clang',var='CLANG')
    if not clang:clang=conf.find_program('clang++',var='CLANG')
    if not clang:conf.fatal('clang was not found')
    clang=conf.cmd_to_list(clang)
    v['CXX_NAME']='clang'
    v['CXX']=clang
    v['COMPILER_CXX']=clang
    v['CC_NAME']='clang'
    v['CC']=clang
    v['COMPILER_CC']=clang
@conf
def clang_common_flags(conf):
    v=conf.env
    v['CXX_SRC_F']=[]
    v['CXX_TGT_F']=['-c','-o','']
    if not v['LINK_CXX']:v['LINK_CXX']=v['CXX']
    if not v['LINK_CC']:v['LINK_CC']=v['CC']
    v['CXXLNK_SRC_F']=[]
    v['CXXLNK_TGT_F']=['-o','']
    v['CPPPATH_ST']='-I%s'
    v['DEFINES_ST']='-D%s'
    v['LIB_ST']='-l%s'
    v['LIBPATH_ST']='-L%s'
    v['STLIB_ST']='-l%s'
    v['STLIBPATH_ST']='-L%s'
    v['RPATH_ST']='-Wl,-rpath,%s'
    v['SONAME_ST']='-Wl,-h,%s'
    v['SHLIB_MARKER']=''#'-Wl,-Bdynamic'
    v['STLIB_MARKER']=''#'-Wl,-Bstatic'
    v['cxxprogram_PATTERN']='%s'
    v['CXXFLAGS_cxxshlib']=['-fPIC']
    v['LINKFLAGS_cxxshlib']=['-shared']
    v['cxxshlib_PATTERN']='lib%s.so'
    v['LINKFLAGS_cxxstlib']=['-Wl,-Bstatic']
    v['cxxstlib_PATTERN']='lib%s.a'
    v['LINKFLAGS_MACBUNDLE']=['-bundle','-undefined','dynamic_lookup']
    v['CXXFLAGS_MACBUNDLE']=['-fPIC']
    v['macbundle_PATTERN']='%s.bundle'
def configure(conf):
    conf.find_clang()
    conf.clang_common_flags()
    conf.cxx_load_tools()
    conf.cxx_add_flags()

# conftest(find_clang)
# conftest(clang_common_flags)
