#! /usr/bin/env python
# encoding: utf-8

import shutil
import os.path

VERSION = '0.0.0+trunk'
APPNAME = 'modipulate'

def options(opt):
    opt.load('compiler_cxx')

def configure(conf):
    conf.load('compiler_cxx')
    
    conf.check_cfg(
        package='portaudio-2.0',
        args='--libs --cflags',
        uselib_store='portaudio',
        atleast_version='19',
        mandatory=1)
    
    conf.check_cfg(
        package='lua5.1',
        uselib_store='lua',
        atleast_version='5.1',
        mandatory=0,
        args='--cflags-only-I')

def build(bld):
    bld.add_post_fun(post_build)
    
    bld.env.append_value('CFLAGS', ['-O2', '-g', '-fPIC'])
    bld.env.append_value('CXXFLAGS', ['-O2', '-g', '-fPIC'])
    bld.env.append_value('LINKFLAGS', ['-O2', '-g', '-fPIC'])
    
    print('build!')
    bld.recurse('src/modipulate')
    bld.recurse('src/modipulate_lua')
    bld.recurse('demos')
    
    #if bld.cmd == 'clean':
    #    paths = bld.path.ant_glob(['**/libmodipulate.so'])
    #    for i in paths:
    #        i.delete()

def post_build(bld):
    # Copy files.
    modipulate_path = 'build/src/modipulate/libmodipulate.so'
    modipulate_lua_path = 'build/src/modipulate_lua/libmodipulatelua.so'
    
    if os.path.isfile(modipulate_path) :
        shutil.copy2(modipulate_path, '.')
    
    if os.path.isfile(modipulate_lua_path) :
        shutil.copy2(modipulate_lua_path, '.')
    
    lua_demos = ['8vb', 'console', 'dr-pentris', 'not-ddr']
    if os.path.isfile(modipulate_lua_path) :
        for i in lua_demos:
            shutil.copy2(modipulate_lua_path, 'demos/modipulate-lua/' + i)

