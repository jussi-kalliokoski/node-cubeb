import Options
from os import unlink, symlink, popen
from os.path import exists 

srcdir = "."
blddir = "build"
VERSION = "0.0.1"

def set_options(opt):
  opt.tool_options("compiler_cxx")

def configure(conf):
  conf.check_tool("compiler_cxx")
  conf.check_tool("node_addon")
  conf.check(lib='cubeb', libpath=['/lib', '/usr/lib', '/usr/local/lib'])

def build(bld):
  obj = bld.new_task_gen("cxx", "shlib", "node_addon")
  obj.target = "node-cubeb"
  obj.source = "src/common.cpp src/context.cpp src/stream.cpp src/module.cpp"
  obj.uselib = "cubeb"
  obj.linkflags = ['-lcubeb']
  obj.cxxflags = ["-D_FILE_OFFSET_BITS=64", "-D_LARGEFILE_SOURCE", "-Wall"]

def shutdown():
  if Options.commands['clean']:
    if exists('node-cubeb.node'): unlink('node-cubeb.node')
  else:
    if exists('build/default/node-cubeb.node') and not exists('node-cubeb.node'):
      symlink('build/default/node-cubeb.node', 'node-cubeb.node')

