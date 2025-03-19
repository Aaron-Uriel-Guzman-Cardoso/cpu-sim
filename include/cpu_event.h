#ifndef CPU_EVENT_H
#define CPU_EVENT_H

/*
 * Posibles sucesos que pueden ocurrir durante la ejecución de la CPU.
 * Estos indican eventos de utilidad para el sistema operativo que serán
 * manejados por este mismo.
 */
enum cpu_event {
    CPU_INSTRUCTION_EXECUTED,
    CPU_INSTRUCTION_ILEGAL,
    CPU_INSTRUCTION_INVALID,
    CPU_REGISTER_OVERFLOW,
    CPU_DIVISION_BY_ZERO,
    CPU_HALT,
    CPU_NONE
};

#endif // CPU_EVENT_H