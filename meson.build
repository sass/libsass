project('libsass', 'c', 'cpp', version: '3.6.4', meson_version: '>=0.49', default_options: ['c_std=c11','cpp_std=c++11'])
cpp = meson.get_compiler('cpp')
if target_machine.system() == 'windows'
  add_project_arguments('-D_WIN32', cpp.get_supported_arguments('-Wno-non-virtual-dtor'), language: 'cpp')
endif

deps = [
  meson.get_compiler('cpp').find_library('dl', required: false)
]

subdir('res') # Must be before src as src_res is declared here and used there
subdir('include')
subdir('src')
subdir('contrib')
