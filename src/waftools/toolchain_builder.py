#!/usr/bin/env python
# -*- coding: utf-8 -*-
from waflib.Configure import conf
# import Utils, Options

def options(opt):
    pass

# Checkout, build and install local version of clang toolchain with libc++.
# Installation directory is srcdir+'_toolchain_' by default.
# Will install only once, remove toolchain dir to force reinstall.
@conf
def ensure_local_toolchain(bld):
    # toolchain_dir = bld.make_node('_toolchain_')
    # print "*** Checking local toolchain presence in " + toolchain_dir + "."
    print "*** Building local clang toolchain."
    print "*** Building local libc++."


# Utils.g_module.__dict__['ensure_local_toolchain'] = ensure_local_toolchain

