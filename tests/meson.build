# Pruebas que si deberían de pasar exitosamente
cpu_test_pass = executable('cpu_test_pass', 'cpu_test_pass.c',
  include_directories : includes,
  link_with : cpu_sim_lib
)

main_test = executable('main', 'main.c',
  include_directories : includes,
  link_with : cpu_sim_lib,
  dependencies : [ncurses, rt_dep]
)
