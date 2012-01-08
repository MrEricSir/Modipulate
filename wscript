import shutil
import os.path

VERSION = '0.0.0+trunk'
APPNAME = 'modipulate'

# Where to copy final file.
results = [
    'modipulate.so',
    'demos/8vb/modipulate.so',
    'demos/dr-pentris/modipulate.so',
    'demos/console/modipulate.so',
    'demos/not-ddr/modipulate.so' ]

def options(opt):
    opt.load('compiler_cxx')

def configure(conf):
    conf.load('compiler_cxx')
    
    conf.check_cfg(
        package='openal',
        args='--libs --cflags',
        uselib_store='openal',
        atleast_version='1.1',
        mandatory=1)
    
    conf.check_cfg(
        package='freealut',
        args='--libs --cflags',
        uselib_store='alut',
        atleast_version='1.1',
        mandatory=1)
        
    conf.check_cfg(
        package='alure',
        args='--libs --cflags',
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
    bld.env.append_value('LINKFLAGS', ['-O2', '-g', '-fPIC', '-lalut'])
    
    bld.shlib(
        features  = 'c cxx cxxshlib cshlib',
        includes  = ['.', 'src', 'libmodplug-hacked'],
        source    = bld.path.ant_glob(['**/*.c', '**/*.cpp']),
        target    = 'modipulate',
        use       = 'lua alure openal alut')
    
    if bld.cmd == 'clean':
        for i in results:
            if os.path.isfile(i) :
                os.remove(i)

def post_build(bld):
    # Copy results.
    modipulate_path = 'build/libmodipulate.so'
    if os.path.isfile(modipulate_path) :
        for i in results:
            shutil.copy2(modipulate_path, i)

