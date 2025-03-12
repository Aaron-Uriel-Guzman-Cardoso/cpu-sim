project('simple-os-simulator', 'c',
  default_options: ['c_std=gnu17', 'warning_level=3', 'optimization=g'],
  version: '0.0.1'
)

includes = include_directories('include')
ncurses = dependency('ncurses', required:true)

subdir('src')

os = executable('os', 'src/main.c',
  include_directories : includes,
  link_with : cpu_sim_lib,
  dependencies : ncurses
)

if get_option('enable-tests')
  subdir('tests')
endif
