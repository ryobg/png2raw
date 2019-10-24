#! /usr/bin/env python
# encoding: utf-8

import os

VERSION='1.0.0'
APPNAME='png2raw'

top = '.'
out = 'out'

def options(opt):
    opt.load('compiler_cxx')

def configure(conf):
    conf.load('compiler_cxx')

    if conf.env['CXX_NAME'] is 'gcc':
        conf.check_cxx (msg="Checking for '-std=c++14'", cxxflags='-std=c++14') 
        conf.env.append_unique('CXXFLAGS', ['-std=c++14', "-O2", "-Wno-deprecated-declarations"])
    elif conf.env['CXX_NAME'] is 'msvc':
        conf.env.append_unique('CXXFLAGS', ['/EHsc', '/O2'])

def build(bld):
    bld.program(target=APPNAME, source="png2raw.cpp lodepng.cpp")
    bld.program(target='remap-png', source="remap-png.cpp lodepng.cpp")

