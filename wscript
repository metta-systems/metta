#!/usr/bin/env python

srcdir = '.'
blddir = 'build'

def set_options(opt):
	opt.sub_options("vesper/src")

def build(bld):
	bld.sub_build("vesper/src")

def configure(conf):
	conf.sub_config("vesper/src")
