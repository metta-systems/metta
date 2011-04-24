#!/usr/bin/env python
# -*- coding: utf-8 -*-

def setup_module_build(name, bld):
    mod = bld.new_task_gen('cxx', 'program')
    mod.env = bld.env_of_name('KERNEL_ENV').copy()
    mod.env.append_unique('LINKFLAGS', ['-Wl,-r']); # Components are relocatable
    mod.env.append_unique('LINKFLAGS', ['-T', '../modules/component.lds', '-Wl,-Map,'+name+'.map'])
    mod.target = name+'.comp'
    bld.new_task_gen(
        source = name+'.comp',
        rule = 'nm -u ${SRC[0].abspath(env)}',
        name = name+' module nm undef check',
        after = 'cxx_link'
    )
    return mod
