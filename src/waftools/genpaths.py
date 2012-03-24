import os
from waflib.TaskGen import feature

def options(opt):
    pass

# Convert INCLUDE_DIRS into includes variable with a given prefix, work around absolute include paths if given.
@feature('_dummy_unused') # just force binding to task_gen
def gen_incpaths(bld, prefix):
    # print 'gen_incpaths for %s' % bld.name
    incs = []
    for x in bld.env.INCLUDE_DIRS:
        if os.path.isabs(x):
            incs.append(x)
        else:
            incs.append(prefix+x)
    bld.includes = incs
