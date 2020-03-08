project('libsass', 'c', 'cpp', version: '3.6.4', default_options: ['cpp_std=c++11'])

if target_machine.system() == 'windows'
  add_project_arguments('-D_WIN32', language: 'cpp')
endif

deps = [
  meson.get_compiler('cpp').find_library('dl', required:true)
]

subdir('include')
subdir('src')
subdir('contrib')
