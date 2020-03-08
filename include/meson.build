sass_inc = include_directories('.')
version_h = configure_file(input: 'sass/version.h.in', output: 'version.h',
  configuration: {'PACKAGE_VERSION': meson.version()},
  install: true,
  install_dir: get_option('includedir') / 'sass'
)

install_subdir('sass', install_dir: get_option('includedir'), exclude_files: ['meson.build', 'sass/version.h', 'sass/version.h.in'])
install_headers('sass.h', 'sass2scss.h')
