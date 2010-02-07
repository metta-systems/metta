#! /usr/bin/env python
# encoding: utf-8

import os
import TaskGen,Task,Utils
from TaskGen import taskgen,before,extension
yasm_str='${YASM} ${YASM_FLAGS} ${YASM_INCLUDES} ${SRC} -o ${TGT}'
EXT_YASM=['.s','.S','.asm','.ASM','.spp','.SPP']
def apply_yasm_vars(self):
	if hasattr(self,'yasm_flags'):
		for flag in self.to_list(self.yasm_flags):
			self.env.append_value('YASM_FLAGS',flag)
	if hasattr(self,'includes'):
		for inc in self.to_list(self.includes):
			node=self.path.find_dir(inc)
			if not node:
				raise Utils.WafError('cannot find the dir'+inc)
			self.env.append_value('YASM_INCLUDES','-I%s'%node.srcpath(self.env))
			self.env.append_value('YASM_INCLUDES','-I%s'%node.bldpath(self.env))
def yasm_file(self,node):
	try:obj_ext=self.obj_ext
	except AttributeError:obj_ext='_%d.o'%self.idx
	task=self.create_task('yasm')
	task.inputs=[node]
	task.outputs=[node.change_ext(obj_ext)]
	self.compiled_tasks.append(task)
	self.meths.append('apply_yasm_vars')
Task.simple_task_type('yasm',yasm_str,color='BLUE',ext_out='.o',shell=False)
def detect(conf):
	yasm=conf.find_program('yasm',var='YASM')
	if not yasm:conf.fatal('could not find yasm, install it or set PATH env var')

before('apply_link')(apply_yasm_vars)
extension(EXT_YASM)(yasm_file)
