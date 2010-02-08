#! /usr/bin/env python
# -*- coding: utf-8 -*-

import os,sys
import Configure,Options,Utils
import ccroot,ar
from Configure import conftest
def find_clang(conf):
    v=conf.env
    clang=None
    if v['CLANG']:clang=v['CLANG']
    elif'CLANG'in conf.environ:cxx=conf.environ['CLANG']
    if not clang:clang=conf.find_program('clang',var='CLANG')
    if not clang:clang=conf.find_program('clang++',var='CLANG')
    if not clang:conf.fatal('clang was not found')
    clang=conf.cmd_to_list(clang)
    ccroot.get_cc_version(conf,clang)
    v['CXX_NAME']='clang'
    v['CXX']=clang
    v['CC_NAME']='clang'
    v['CC']=clang
def clang_common_flags(conf):
    v=conf.env
    v['CXX_SRC_F']=''
    v['CXX_TGT_F']=['-c','-o','']
    v['CPPPATH_ST']='-I%s'
    if not v['LINK_CXX']:v['LINK_CXX']=v['CXX']
    v['CXXLNK_SRC_F']=''
    v['CXXLNK_TGT_F']=['-o','']
    v['LIB_ST']='-l%s'
    v['LIBPATH_ST']='-L%s'
    v['STATICLIB_ST']='-l%s'
    v['STATICLIBPATH_ST']='-L%s'
    v['RPATH_ST']='-Wl,-rpath,%s'
    v['CXXDEFINES_ST']='-D%s'
    v['SONAME_ST']='-Wl,-h,%s'
    v['SHLIB_MARKER']='-Wl,-Bdynamic'
    v['STATICLIB_MARKER']='-Wl,-Bstatic'
    v['FULLSTATIC_MARKER']='-static'
    v['program_PATTERN']='%s'
    v['shlib_CXXFLAGS']=['-fPIC','-DPIC']
    v['shlib_LINKFLAGS']=['-shared']
    v['shlib_PATTERN']='lib%s.so'
    v['staticlib_LINKFLAGS']=['-Wl,-Bstatic']
    v['staticlib_PATTERN']='lib%s.a'
    v['LINKFLAGS_MACBUNDLE']=['-bundle','-undefined','dynamic_lookup']
    v['CCFLAGS_MACBUNDLE']=['-fPIC']
    v['macbundle_PATTERN']='%s.bundle'
def detect(conf):
    conf.find_clang()
    #conf.find_cpp()
    #conf.find_ar()
    conf.clang_common_flags()
    conf.cxx_load_tools()
    conf.cxx_add_flags()

conftest(find_clang)
conftest(clang_common_flags)
