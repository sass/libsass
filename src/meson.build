src_cpp = files(
  'ast.cpp',
  'ast2c.cpp',
  'ast_fwd_decl.cpp',
  'ast_sel_cmp.cpp',
  'ast_sel_super.cpp',
  'ast_sel_unify.cpp',
  'ast_sel_weave.cpp',
  'ast_selectors.cpp',
  'ast_supports.cpp',
  'ast_values.cpp',
  'backtrace.cpp',
  'base64vlq.cpp',
  'bind.cpp',
  'c2ast.cpp',
  'check_nesting.cpp',
  'color_maps.cpp',
  'constants.cpp',
  'context.cpp',
  'cssize.cpp',
  'emitter.cpp',
  'environment.cpp',
  'error_handling.cpp',
  'eval.cpp',
  'eval_selectors.cpp',
  'expand.cpp',
  'extender.cpp',
  'extension.cpp',
  'file.cpp',
  'fn_colors.cpp',
  'fn_lists.cpp',
  'fn_maps.cpp',
  'fn_miscs.cpp',
  'fn_numbers.cpp',
  'fn_selectors.cpp',
  'fn_strings.cpp',
  'fn_utils.cpp',
  'inspect.cpp',
  'json.cpp',
  'lexer.cpp',
  'listize.cpp',
  'operators.cpp',
  'output.cpp',
  'parser.cpp',
  'parser_selectors.cpp',
  'plugins.cpp',
  'position.cpp',
  'prelexer.cpp',
  'remove_placeholders.cpp',
  'sass.cpp',
  'sass2scss.cpp',
  'sass_context.cpp',
  'sass_functions.cpp',
  'sass_values.cpp',
  'source.cpp',
  'source_map.cpp',
  'stylesheet.cpp',
  'to_value.cpp',
  'units.cpp',
  'utf8_string.cpp',
  'util.cpp',
  'util_string.cpp',
  'values.cpp'
)
src_c = files(
  'c99func.c',
  'cencode.c'
)

subdir('memory')

sass_lib = library('sass', src_cpp, src_c, src_res,
  include_directories: sass_inc,
  dependencies: deps,
  c_args: ['-DADD_EXPORTS'],
  cpp_args: ['-DADD_EXPORTS'],
  install: true
)

sass_dep = declare_dependency(link_with: sass_lib, include_directories: sass_inc)

import('pkgconfig').generate(
  sass_lib,
  description: 'A C implementation of a Sass compiler'
)
