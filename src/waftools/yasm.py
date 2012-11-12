#! /usr/bin/env python
# encoding: utf-8

import waflib.Tools.asm
from waflib.TaskGen import feature, extension
from waflib import Task

def configure(conf):
	yasm=conf.find_program(['yasm'],var='YASM',path_list=['/usr/bin','/usr/local/bin'])
	if not yasm:conf.fatal('could not find yasm, install it or set PATH env var')
	conf.env.AS_TGT_F=['-o']
	conf.env.ASLNK_TGT_F=['-o']

def apply_yasm_vars(self):
	self.env.append_value('YASM_FLAGS',self.to_list(getattr(self,'yasm_flags',[])))
	self.env.append_value('YASM_INCLUDES'," -I".join([''] + self.to_list(self.env.INCPATHS)).split())
feature('asm')(apply_yasm_vars)

Task.simple_task_type('yasm','${YASM} ${YASM_FLAGS} ${YASM_INCLUDES} ${SRC} -o ${TGT}',color='BLUE',ext_out='.o',shell=False)

def yasm_hook(self,node):
	self.meths.append('apply_yasm_vars')
	return self.create_compiled_task('yasm',node)

extension('.s')(yasm_hook)
