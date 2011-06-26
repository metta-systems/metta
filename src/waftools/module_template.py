#!/usr/bin/env python
# -*- coding: utf-8 -*-
import Utils

def set_options(opt):
    pass

def setup_module_build(bld, name):
    mod = bld.new_task_gen('cxx', 'program')
    mod.target = name+'.comp'
    mod.env = bld.env_of_name('KERNEL_ENV').copy()
    mod.env.append_unique('LINKFLAGS', ['-Wl,-r']); # Components are relocatable
    mod.env.append_unique('LINKFLAGS', ['-T', '../modules/component.lds', '-Wl,-Map,'+name+'.map'])
    mod.includes = '. ../../runtime ../../runtime/stl ../../interfaces ../../kernel/api ../../kernel/generic ../../kernel/arch/x86 ../../kernel/platform/pc99 ../../modules'
    mod.uselib_local = 'component_support interfaces kernel platform common runtime'
    bld.new_task_gen(
        source = name+'.comp',
        rule = 'nm -u ${SRC[0].abspath(env)}',
        name = name+' module nm undef check',
        after = 'cxx_link'
    )
    return mod

Utils.g_module.__dict__['setup_module_build'] = setup_module_build
