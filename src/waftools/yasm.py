#! /usr/bin/env python
# encoding: utf-8

import waflib.Tools.asm
from waflib.TaskGen import feature
def apply_yasm_vars(self):
	self.env.append_value('YASM_FLAGS',self.to_list(getattr(self,'yasm_flags',[])))
	if hasattr(self,'includes'):
		for inc in self.to_list(self.includes):
			node=self.path.find_dir(inc)
			if not node:
				raise Utils.WafError('cannot find the dir '+inc)
			self.env.append_value('YASM_INCLUDES','-I%s'%node.srcpath(self.env))
			self.env.append_value('YASM_INCLUDES','-I%s'%node.bldpath(self.env))
def configure(conf):
	nasm=conf.find_program(['yasm'],var='AS')
	conf.env.AS_TGT_F=['-o']
	conf.env.ASLNK_TGT_F=['-o']

feature('asm')(apply_yasm_vars)


# import os
# import TaskGen,Task,Utils
# from waflib.TaskGen import taskgen,before,extension
# yasm_str='${YASM} ${YASM_FLAGS} ${YASM_INCLUDES} ${SRC} -o ${TGT}'
# EXT_YASM=['.s','.S','.asm','.ASM','.spp','.SPP']
# def yasm_file(self,node):
# 	try:obj_ext=self.obj_ext
# 	except AttributeError:obj_ext='_%d.o'%self.idx
# 	task=self.create_task('yasm')
# 	task.inputs=[node]
# 	task.outputs=[node.change_ext(obj_ext)]
# 	self.compiled_tasks.append(task)
# 	self.meths.append('apply_yasm_vars')
# Task.simple_task_type('yasm',yasm_str,color='BLUE',ext_out='.o',shell=False)
# def detect(conf):
# 	yasm=conf.find_program('yasm',var='YASM')
# 	if not yasm:conf.fatal('could not find yasm, install it or set PATH env var')

# before('apply_link')(apply_yasm_vars)
# extension(EXT_YASM)(yasm_file)
