winres_path = files('resource.rc')
src_res = []

if host_machine.system() == 'windows'
  src_res += [import('windows').compile_resources(winres_path)]
endif
