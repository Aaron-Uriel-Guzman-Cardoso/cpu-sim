sources = files('cpu.c', 'insts.c', 'msg.c', 'ncurses_utilities.c')
cpu_sim_lib = library('cpu_sim_lib', sources,
  install : true,
  include_directories : includes,
  dependencies: ncurses # Esta no se va a ocupar cuando separemos front-end de simulador
)
main_test = executable('main', 'main.c',
  include_directories : includes,
  link_with : cpu_sim_lib,
  dependencies : ncurses
)
