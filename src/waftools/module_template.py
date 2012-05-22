#!/usr/bin/env python
# -*- coding: utf-8 -*-
from waflib.Configure import conf
from waflib import Options #, Utils
import os

def options(opt):
    pass

# Check resulting filename for undefined symbols.
# Environment: TARGET
@conf
def undef_check(bld, filename):
    bld(
        source = bld.path.find_or_declare(filename),
        rule = '%s -u ${SRC[0].abspath()}' % bld.env['NM'],
        name = 'nm undef check ('+filename+')',
        after = 'init.img'
    )

@conf
def setup_module_build(bld, name, prefix, sources):
    if prefix: prefix = prefix + '/'
    arch = Options.options.arch
    platform = Options.options.platform

    mod = bld.program(
        source = sources,
        target = name+'.comp',
        # debug depends on platform
        # runtime doesn't depend on anything
        use = 'component_support interfaces kernel debug platform common c++ runtime',
        env = bld.all_envs['KERNEL_ENV'].derive()
    )
    mod.gen_incpaths(prefix+'../')
    mod.includes.append('.')
    mod.includes.append(prefix)

    mod.env.append_unique('LINKFLAGS', ['-Wl,-r']); # Components are relocatable
    if platform != 'hosted':
        # mod.env.append_unique('LINKFLAGS', ['-T', '../modules/component.lds'])
        mod.env.append_unique('LINKFLAGS', ['-T', '../modules/component.lds', '-Wl,-Map,'+name+'.map'])
    bld.undef_check(mod.target)
    node = bld.path.find_or_declare(mod.target)
    bld.all_comp_targets += [node.bldpath()]
    return mod
