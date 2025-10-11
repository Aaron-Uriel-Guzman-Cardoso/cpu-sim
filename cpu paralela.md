# Hacer este proyecto una CPU paralela
Bien, veamos, como proyecto para Sistemas Operativos (materia que estoy recursando) tenemos que deseamos hacer paralela la ejecución de la CPU, es decir que sea multinúcleo y permita ejecutar varios programas paralelamente. Más que solo hacerlo yo, la idea es que todos participemos en equipo, todos tengamos idea de lo que estamos haciendo y al final cualquiera pueda explicar lo que se hizo. Por eso ahorita mi primera actividad será hacer explicar lo que hay que hacer con texto y diagramas.

# Lo que hay que hacer
Agregaremos a nuestro simulador de CPU la capacidad de multiprocesamiento, hablando en términos más modernos, hacer que nuestra CPU simulada sea multinúcleo y por tanto tenga soporte de múltiples hilos de ejecución por Hardware.

## Visión
El simulador incluye ahora abstracciones simples para permitir ejecución concurrente:
- `LOAD bubblesort.asm` carga el algoritmo en RAM, listo para su ejecución.
- `EXEC 0 bubblesort` indica que el PC del nucleo 0 apunte al inicio de la sección ejecutable de bubblesort
- `EXEC 1 bubblesort.asm` lo mismo pero para núcleo 1
- `EXEC 2 bubblesort.asm` lo mismo pero para núcleo 2
- `LOAD 0x9000 numbers` cargamos los números que cada núcleo tomará y ordenará
- `BOOT` inicia ejecución de todos los núcleos al mismo tiempo, útil para facilitar entendimiento de la ejecución.

En cuanto a las unidades de ejecución (Cores o Harts):
- Ahora soporta las instrucciones `LR` (Load Reserved) y `SC` (store conditional) para facilitar la concurrencia sin bloqueos.
- Implementamos un `NOP`, cuyo propósito es hacer nada durante un ciclo de ejecución.
- Los bloqueos con estructuras como mutex tendrán que ser creados por el mismo usuario apoyándose en las instrucciones anteriormente mencionadas.
- Instrucciones del estilo de `FENCE` y AMO de RISC-V no son implementadas, por simplicidad del proyecto y que además nuestro procesador no es Out of Order (según yo).
- En código podrás ejecutarlas con la función `core_execute(struct core *self, struct inst *inst).
- Ahora el MMU también deberá conocer el núcleo desde el que se le llama para así poder traducir direcciones virtuales adecuadamente.
- Los núcleos ahora reportarán si: están esperando, accedieron a una dirección inválida o terminaron ejecución.

Y ahora, para la nueva CPU:
- Esta contendrá a las unidades de ejecución anteriormente mencionadas.
- No se me ocurre otro rol para esta estructura más que contener los núcleos.