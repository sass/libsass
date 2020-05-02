src_memory = files(
  'allocator.cpp',
  'shared_ptr.cpp'
)
memory_lib_args = []
if get_option('SASS_CUSTOM_ALLOCATOR')
  memory_lib_args += ['-DSASS_CUSTOM_ALLOCATOR']
endif
memory_lib = static_library('memory', src_memory, include_directories: sass_inc, cpp_args: memory_lib_args)
