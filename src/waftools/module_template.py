#!/usr/bin/env python
# -*- coding: utf-8 -*-
import Utils, Options

def set_options(opt):
    pass

# Check resulting filename for undefined symbols.
# Environment: TARGET
def undef_check(bld, filename):
    bld.new_task_gen(
        source = filename,
        rule = bld.env['NM']+' -u ${SRC[0].abspath(env)}',
        name = 'nm undef check ('+filename+')',
        after = 'cxx_link'
    )

def setup_module_build(bld, name, prefix):
    if prefix: prefix = prefix + '/'
    arch = Options.options.arch
    platform = Options.options.platform
    mod = bld.new_task_gen('cxx', 'program')
    mod.target = name+'.comp'
    mod.env = bld.env_of_name('KERNEL_ENV').copy()
    mod.env.append_unique('LINKFLAGS', ['-Wl,-r']); # Components are relocatable
    if platform != 'hosted':
        mod.env.append_unique('LINKFLAGS', ['-T', '../modules/component.lds', '-Wl,-Map,'+name+'.map'])
    mod.includes = ['.', prefix+'../runtime', prefix+'../runtime/stl', prefix+'../interfaces', prefix+'../kernel/api', prefix+'../kernel/generic', prefix+'../kernel/arch/'+arch, prefix+'../kernel/platform/'+platform, prefix+'../kernel/arch/shared', prefix+'../kernel/platform/shared', prefix]
    mod.uselib_local = 'component_support interfaces kernel platform common runtime debug'
    undef_check(bld, name+'.comp')
    return mod

Utils.g_module.__dict__['undef_check'] = undef_check
Utils.g_module.__dict__['setup_module_build'] = setup_module_build

