# Sistemas operativos
Bien, hoy tomaremos dos actividades grandes:
1. definir la estructura del proyecto en cuanto a código y estructura de archivos de forma que no se nos complique más adelante la entrega 4.2.
2. Una vez definida la estructura, encontrar actividades clave para su realización y tomar una de ellas para la implementación (pensando en la implementación del MMU para la CPU y como es que esta consume instrucciones principalmente).

Y yo creo dos actividades pequeñas que nos sean útiles:
1. Verificar que todas las pruebas unitarias funcionen, diseñar pruebas para las entregas pasadas.
2. Manejar cuando el código no tiene END. 

Y si ya tengo tiempo haré:
1. Agregar ejemplos a la documentación y mejorar la calidad de esta.
2. Hacer que instrucciones ya no use tanto malloc y free.

Ok, empecemos por aclarar la estructura del proyecto y desarrollar lo que se tiene que hacer:

## CPU
El MMU será una abstracción que permitirá a la CPU acceder a las instrucciones en la memoria SWAP. La CPU recurrirá al MMU para obtener la siguiente instrucción y cargarla en IR ¿cómo hacemos eso ahorita?
- En vez de recurrir a un instmem que tiene la CPU, ahora lo que haremos será que CPU llame a un método de MMU llamado `mmu_get_inst(mmu, self->regs[PC])` será internamente el MMU el que sabrá de que proceso estamos.
¿Cómo sabemos en que proceso estamos desde el MMU? este tiene que conocer el estado del sistema operativo o de cierta forma el sistema operativo tiene que tener expuesto el proceso actual en ejecución tipo `os_get_curr_pid(os)`. Entonces hay que hallar la forma en que el MMU pueda preguntarle al sistema operativo el proceso actual en ejecución para así consultar las tablas. Veriamos que el MMU tendría entonces los siguientes miembros:
- Una tabla de marcos en SWAP, siendo un arreglo de 4096 elementos donde dentro estará indicado el proceso al que pertenece, siendo el valor 0 que está libre.

### OS: obtener el proceso actual en ejecución
El MMU necesita saber que proceso se está ejecutando actualmente, entonces podrá consultar al sistema operativo cual es. ¿Cómo exponemos esto si no hay un objeto de sistema operativo? Todos los miembros están expuestos como variables globales, así que no hay tanto problema.


### MMU: encontrar la instrucción de la dirección real
Dado que siempre obtendremos la instrucción del proceso actual, podemos obtener el PCB de dicho y así conocer su propia tabla de marcos, así obtenemos rápidamente la instrucción del marco que deseamos junto con su dirección real.