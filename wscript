import shutil
import os.path

VERSION = '0.0.0+trunk'
APPNAME = 'modipulate'

def options(opt):
    opt.load('compiler_cxx')

def configure(conf):
    conf.load('compiler_cxx')
    
    conf.check_cfg(
        package='openal',
        uselib_store='openal',
        atleast_version='1.1',
        mandatory=1)
    
    conf.check_cfg(
        package='freealut',
        uselib_store='alut',
        atleast_version='1.1',
        mandatory=1)
        
    conf.check_cfg(
        package='alure',
        uselib_store='alure',
        atleast_version='1.0',
        mandatory=1)
    
    # Eventually we'll move Lua this into another (optional) library.
    conf.check_cfg(
        package='lua5.1',
        uselib_store='lua',
        atleast_version='5.1',
        mandatory=1,
        args='--cflags-only-I')

def build(bld):
    bld.add_post_fun(post_build)
    
    bld.env.append_value('CFLAGS', ['-O2', '-g', '-fPIC'])
    bld.env.append_value('LINKFLAGS', ['-O2', '-g', '-fPIC', '-lalut', '-lalure'])
    
    bld.shlib(
        features  = 'c cxx cxxshlib cshlib',
        includes  = ['.', 'src', 'libmodplug-hacked'],
        source    = bld.path.ant_glob(['**/*.c', '**/*.cpp']),
        target    = 'modipulate',
        use       = 'lua openal alut alure')

def post_build(bld):
    # Copy results.
    modipulate_path = 'build/libmodipulate.so'
    
    if os.path.isfile(modipulate_path) :
        shutil.copy2(modipulate_path, 'modipulate.so')
        shutil.copy2(modipulate_path, 'demos/8vb/modipulate.so')
        shutil.copy2(modipulate_path, 'demos/dr-pentris/modipulate.so')
        shutil.copy2(modipulate_path, 'demos/console/modipulate.so')
        shutil.copy2(modipulate_path, 'demos/not-ddr/modipulate.so')


