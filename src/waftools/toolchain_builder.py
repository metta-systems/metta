#!/usr/bin/env python
# -*- coding: utf-8 -*-
from waflib.Configure import conf
# import Utils, Options

def options(opt):
    pass

@conf
def ensure_local_toolchain(bld):
    print "*********************************************************"
    print "*** To build local toolchain run ./build_toolchain.sh ***"
    print "*********************************************************"
