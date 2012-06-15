#!/usr/bin/env python
# encoding: utf-8
#
# Convert interface file into set of header and implementation files.
# name_vX.if -> name_vX_interface.h
#            -> name_vX_interface.cpp
#            -> name_vX_impl.h
#
from waflib.Task import Task
from waflib.TaskGen import extension

# First input is the name of meddler executable, as prepared by process_src below.
class meddle(Task):
    color = 'CYAN'
    verbose = False
    def run(self):
        cwd = self.outputs[0].parent.bldpath()
        incd = [self.inputs[1].parent.bldpath()]
        # print "includes:"
        for i in self.env.INCLUDES:
            # print "include "+i
            incd = incd + [i]

        if (self.verbose):
            cmd = '%s -v %s -o%s -I%s' % (self.inputs[0].abspath(), self.inputs[1].abspath(), cwd, ' -I'.join(incd))
        else:
            cmd = '%s %s -o%s -I%s' % (self.inputs[0].abspath(), self.inputs[1].abspath(), cwd, ' -I'.join(incd))

        # print "Running "+cmd
        return self.exec_command(cmd)

@extension('.if')
def process_src(self, node):
	tg = self.bld.get_tgen_by_name('meddler')
	comp = tg.tasks[-1].outputs[0]
	tsk = self.create_task('meddle', [comp, node], 
		[node.change_ext('_interface.cpp'), node.change_ext('_interface.h'), node.change_ext('_impl.h'), node.change_ext('_typedefs.cpp')]) # Use node.name.replace instead of change_ext?
	self.source.extend(tsk.outputs)

# Added _interface.h and _impl.h files need not be processed.
@extension('.h')
def foo(*k, **kw):
	pass
