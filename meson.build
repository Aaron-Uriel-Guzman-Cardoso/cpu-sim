project('simple-os-simulator', 'c',
  default_options: ['c_std=gnu17', 'warning_level=3', 'optimization=g'],
  version: '0.0.1'
)

includes = include_directories('include')

subdir('src')
ncurses = dependency('ncurses', required:true)
if get_option('enable-tests')
  subdir('tests')
endif
