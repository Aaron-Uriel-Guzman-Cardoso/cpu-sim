#include <assert.h>
#include <bits/time.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include <msg.h>
#include <cpu.h>
#include <insts.h>
#include <time.h>
#include <queue.h>


/**
 * \brief Unión que representa los dos formatos que pueden tener las
 *        operaciones de las instrucciones: de registro a registro e
 *        inmediatas.
 */
union op_fn {
    int32_t (*reg_to_reg)(struct cpu *, enum reg, enum reg);
    int32_t (*imm)(struct cpu *, enum reg, int32_t);
};

int32_t cpu_mov(struct cpu *self, enum reg ra, enum reg rb);
int32_t cpu_movi(struct cpu *self, enum reg ra, int imm);
int32_t cpu_add(struct cpu *self, enum reg ra, enum reg rb);
int32_t cpu_addi(struct cpu *self, enum reg ra, int imm);
int32_t cpu_sub(struct cpu *self, enum reg ra, enum reg rb);
int32_t cpu_subi(struct cpu *self, enum reg ra, int imm);
int32_t cpu_mul(struct cpu *self, enum reg ra, enum reg rb);
int32_t cpu_muli(struct cpu *self, enum reg ra, int imm);
int32_t cpu_div(struct cpu *self, enum reg ra, enum reg rb);
int32_t cpu_divi(struct cpu *self, enum reg ra, int imm);
int32_t cpu_inc(struct cpu *self, enum reg ra, int32_t imm);
int32_t cpu_dec(struct cpu *self, enum reg ra,  int32_t imm);
int32_t cpu_nop(struct cpu *self, enum reg ra,  int32_t imm);
int32_t cpu_end(struct cpu *self, enum reg ra,  int32_t imm);

/*
 * Definimos las instrucciones que podrán ser llamadas por la CPU, no todas
 * están definidas pues algunas son únicamente inmediatas.
 */
union op_fn ops[OP_LIMIT] = {
    { .reg_to_reg = cpu_mov}, { .reg_to_reg = cpu_add }, { .reg_to_reg = cpu_sub},
    { .reg_to_reg = cpu_mul}, { .reg_to_reg = cpu_div }, { .imm = cpu_inc },
    { .imm = cpu_dec }, { .imm = cpu_nop }, {.imm = cpu_end }, {.imm = cpu_movi },
    { .imm = cpu_addi }, { .imm = cpu_subi }, { .imm = cpu_muli },
    { .imm = cpu_divi }
};

int32_t
cpu_mov(struct cpu *self, enum reg ra, enum reg rb)
{
    if (ra >= REG_LIMIT || rb >= REG_LIMIT) {
        return 1;
    }
    self->regs[ra] = self->regs[rb];
    return 0;
}

int32_t
cpu_movi(struct cpu *self, enum reg ra, int imm)
{
    if (ra >= REG_LIMIT) {
        return 1;
    }
    self->regs[ra] = imm;
    return 0;
}

int32_t
cpu_add(struct cpu *self, enum reg ra, enum reg rb)
{
    if (ra >= REG_LIMIT || rb >= REG_LIMIT) {
        return 1;
    }
    self->overflow = (self->regs[ra] > 0 && self->regs[ra] > INT32_MAX - self->regs[rb]) ||
                     (self->regs[ra] < 0 && self->regs[ra] < INT32_MIN - self->regs[rb]);
    if (!self->overflow) {
        self->regs[ra] += self->regs[rb];
    }
    return 0;
}

int32_t
cpu_addi(struct cpu *self, enum reg ra, int imm)
{
    if (ra >= REG_LIMIT) {
        return 1;
    }
    self->overflow = (self->regs[ra] > 0 && self->regs[ra] > INT32_MAX - imm) ||
                     (self->regs[ra] < 0 && self->regs[ra] < INT32_MIN - imm);
    if (!self->overflow) {
        self->regs[ra] += imm;
    }
    return 0;
}


int32_t
cpu_sub(struct cpu *self, enum reg ra, enum reg rb)
{
    if (ra >= REG_LIMIT || rb >= REG_LIMIT) {
        return 1;
    }
    self->overflow = (self->regs[ra] > 0 && self->regs[ra] > INT32_MAX + self->regs[rb]) ||
                     (self->regs[ra] < 0 && self->regs[ra] < INT32_MIN + self->regs[rb]);
    if (!self->overflow) {
        self->regs[ra] -= self->regs[rb];
    }
    return 0;
}

int32_t
cpu_subi(struct cpu *self, enum reg ra, int imm)
{
    if (ra >= REG_LIMIT) {
        return 1;
    }
    self->overflow = (self->regs[ra] > 0 && self->regs[ra] > INT32_MAX + imm) ||
                     (self->regs[ra] < 0 && self->regs[ra] < INT32_MIN + imm);
    if (!self->overflow) {
        self->regs[ra] -= imm;
    }
    return 0;
}

int32_t
cpu_mul(struct cpu *self, enum reg ra, enum reg rb)
{
    if (ra >= REG_LIMIT || rb >= REG_LIMIT) {
        return 1;
    }
    self->overflow = (self->regs[ra] > 0 && self->regs[ra] > INT32_MAX / self->regs[rb]) ||
                     (self->regs[ra] < 0 && self->regs[ra] < INT32_MIN / self->regs[rb]) ||
                     ((self->regs[ra] == -1) && (self->regs[rb] == INT32_MIN)) ||
                     ((self->regs[ra] == INT32_MIN) && (self->regs[rb] == -1));
    if (!self->overflow) {
        self->regs[ra] *= self->regs[rb];
    }
    return 0;
}

int32_t
cpu_muli(struct cpu *self, enum reg ra, int imm)
{
    if (ra >= REG_LIMIT) {
        return 1;
    }
    self->overflow = (self->regs[ra] > 0 && self->regs[ra] > INT32_MAX / imm) ||
                     (self->regs[ra] < 0 && self->regs[ra] < INT32_MIN / imm) ||
                     ((self->regs[ra] == -1) && (imm == INT32_MIN)) ||
                     ((self->regs[ra] == INT32_MIN) && (imm == -1));
    if (!self->overflow) {
        self->regs[ra] *= imm;
    }
    return 0;
}

int32_t
cpu_div(struct cpu *self, enum reg ra, enum reg rb)
{
    if (ra >= REG_LIMIT || rb >= REG_LIMIT) {
        return 1;
    }
    if (self->regs[rb] == 0) {
        self->div_by_zero = true;
    }
    else {
        self->regs[ra] /= self->regs[rb];
    }
    return 0;
}

int32_t
cpu_divi(struct cpu *self, enum reg ra, int imm)
{
    if (ra >= REG_LIMIT) {
        return 1;
    }
    if (imm == 0) {
        self->div_by_zero = true;
    }
    else {
        self->regs[ra] /= imm;
    }
    return 0;
}

int32_t
cpu_inc(struct cpu *self, enum reg ra, int32_t imm)
{
    if (ra >= REG_LIMIT) {
        return 1;
    }
    self->overflow = (self->regs[ra] == INT32_MAX);
    if (!self->overflow) {
        self->regs[ra] += 1;
    }
    return 0;
}

int32_t
cpu_dec(struct cpu *self, enum reg ra, int32_t imm)
{
    if (ra >= REG_LIMIT) {
        return 1;
    }
    self->overflow = (self->regs[ra] == INT32_MIN);
    if (!self->overflow) {
        self->regs[ra] -= 1;
    }
    return 0;
}

int32_t
cpu_nop(struct cpu *self, enum reg ra, int32_t imm)
{
    if (ra >= REG_LIMIT) {
        return 1;
    }
    return 0;
}

int32_t
nopi(struct cpu *self, enum reg ra, int32_t imm)
{
    if (ra >= REG_LIMIT) {
        return 1;
    }
    return 0;
}

/*
 * \brief Realiza la instrucción END en la CPU.
 *
 * Lo único que se encarga esta función es de indicar a la CPU que la
 * ejecución deberá parar.
 */
int32_t 
cpu_end(struct cpu *self, enum reg ra, int32_t imm)
{
    if (ra >= REG_LIMIT) {
        return 1;
    }
    self->halt = true;
    return 0;
}

enum cpu_event cpu_next_cycle(struct cpu *self);

struct cpu *
cpu_new(void)
{
    struct cpu *new_cpu = calloc(1, sizeof(*new_cpu));
    if (new_cpu) {
        new_cpu->target_freq = (struct timespec) {
            .tv_sec = 2,
            .tv_nsec = 0
        };
        new_cpu->halt = true;
        new_cpu->events = queue_new();
    }
    return new_cpu;
}

/*
 * \brief Prepara la CPU para poder ejecutar un nuevo programa
 */
int32_t
cpu_reset(struct cpu *self)
{
    memset(self->instmem, 0, sizeof(self->instmem));
    memset(self->regs, 0, sizeof(self->regs));
    self->halt = true;
    self->div_by_zero = false;
    self->overflow = false;
    return 0;
}

/*
 * Procesa una instrucción individual y la añade a la memoria de instrucciones
 * Esta función está realizada de esta forma para obtener un funcionamiento
 * genérico para cpu_load_insts_from_file y cpu_load_insts_from_str
 * Retorna: 0 si todo bien, 1 si hay error en la instrucción, 2 si es END
 */
static int32_t 
cpu_parse_and_load_inst(struct cpu *self, const char *inst_str, 
                              size_t *instmem_end)
{
    if (*instmem_end >= INSTS_MAX) {
        return -1;
    }
    struct inst *tmp = inst_from_str(inst_str);
    bool is_end = false;
    if (!tmp) {
        char logstr[100];
        /**
         * TODO: imprimir de forma genérica desde la CPU para compatibilidad
         *       con front-end de ncurses y para diagnóstico en las pruebas
         *       unitarias.
         */
        snprintf(logstr, sizeof(logstr),
                 "Instrucción \"%s\" inválida, remplazada por END\n", inst_str);
        msg_log(LOG_LEVEL_WARN, logstr);
        tmp = inst_from_str("END");
        if (!tmp) {
            return 1;
        }
        is_end = true;
    }

    if (tmp->op == OP_END) {
        is_end = true;
    }
    
    self->instmem[*instmem_end] = *tmp;
    free(tmp);
    (*instmem_end) += 1;
    
    return (is_end)? 2 : 0;
}

/**
 * \brief Prepara la CPU para la ejecución una vez inicializado instmem
 *
 * Realiza la copia de la primera instrucción en instmem a IR para que la CPU
 * esté lista. Es requerido que instmem ya esté inicializado con anterioridad
 * caso contrario esta función no hará nada útil.
 */
static void
cpu_prepare(struct cpu *self)
{
    /** TODO: Hacer esta verificación en tiempo de compilación */
    /**
     * Esta verificación es realizada debido a que IR es un int64_t y la
     * estructura inst está hecha de forma que ocupe menos de 64 bits,
     * verificamos por si acaso que esta quepa sin problemas
     */
    assert(sizeof(self->instmem[0]) <= sizeof(self->regs[REG_IR]));

    /*
     * Cargamos primer instrucción y actualizamos PC para que apunte a
     * siguiente instrucción.
     */
    memset(&self->regs[REG_IR], 0, sizeof(self->regs[REG_IR]));
    memcpy(&self->regs[REG_IR], &self->instmem[0], sizeof(self->instmem[0]));
    self->regs[REG_PC] = 1;

    /**
     * El "ciclo cero" es realizado al momento de instanciar la CPU.
     * Esto permite dar un delay y que la primera instrucción no sea
     * ejecutada inmediatamente.
     */
    clock_gettime(CLOCK_MONOTONIC, &self->last_cycle);

    /**
     * Habilitamos la CPU para ejecutarse
     */
    self->halt = false;
}

/**
 * \brief Carga un archivo de instrucciones para ser ejecutada por la CPU.
 * 
 * Toma el archiivo especificado y lo carga para su procesamiento en la CPU,
 * cargar las instrucciones será un paso necesario para poder ejecutar la CPU.
 * 
 * \return Si hubo error al cargar las instrucciones desde el archivo:
 *          - 0 no hubo error
 *          - 1 hubo error al leer una instrucción del archivo
 *          - 2 se llenó la memoria de instrucciones 
 */
int32_t
cpu_load_insts_from_file(struct cpu *self, const char *filename)
{
    if (!self || !filename) { return -1; }
    FILE *instfile = fopen(filename, "r");
    if (!instfile) {
        return 2;
    }
    size_t instmem_end = 0;
    char buf[32];
    bool end_found = false;
    while (fgets(buf, sizeof(buf), instfile) && !end_found) {
        if (instmem_end > INSTS_MAX) {
            /*
             * Nos pasamos del límite de instrucciones :(
             */
            return 2;
        }
        for (size_t i = 0; buf[i] != '\0'; i += 1) {
            buf[i] = toupper(buf[i]);
        }
        int32_t result = cpu_parse_and_load_inst(self, buf, &instmem_end);
        if (result == 1) {
            return 1;
        } else if (result == 2) {
            end_found = true;
        }
    }
    cpu_prepare(self);
    return 0;
}

int32_t
cpu_load_insts_from_str(struct cpu *self, char *str)
{
    if (!self || !str) { return -1; }
    for (size_t i = 0; str[i] != '\0'; i += 1) {
        str[i] = toupper(str[i]);
    }
    size_t instmem_end = 0;
    char *str_state;
    char *whole_inst_tok = strtok_r(str, "\n", &str_state);
    if (!whole_inst_tok) {
        /*
         * Formato inválido de programa de instrucciones
         */
        return 2;
    }
    int32_t result = cpu_parse_and_load_inst(self, whole_inst_tok, &instmem_end);
    if (result == 1) {
        return 1;
    } else if (result == 2) {
        cpu_prepare(self);
        return 0;
    }
    while ((whole_inst_tok = strtok_r(NULL, "\n", &str_state))) {
        result = cpu_parse_and_load_inst(self, whole_inst_tok, &instmem_end);
        if (result == 1) {
            return 1;
        } else if (result == 2) {
            break;
        }
    }
    cpu_prepare(self);
    return 0;
}

/**
 * \brief Ejecuta una instrucción en la CPU.
 *
 * \param self la CPU a manipular
 * \param inst la instrucción a ejecutar
 * \return Si la instrucción está malformada o si algún parámetro lo está
 */
int32_t
cpu_execute(struct cpu *self, struct inst *inst)
{
    if (!self || !inst || inst->op >= OP_LIMIT) {
        return 2;
    }
    if (inst->op >= OP_INC && inst->op <= OP_LIMIT) {
        return ops[inst->op].imm(self, inst->ra, inst->imm);
    } else {
        return ops[inst->op].reg_to_reg(self, inst->ra, inst->rb);
    }
}

/**
 * \brief Realiza un ciclo de reloj en la CPU intentando ejecutar la siguiente
 *        instrucción en memoria.
 *
 * \return El evento que ocurrió al intentar ejecutar la siguiente
 *         instrucción, indicará si la instrucción se ejecutó correctamente o
 *         hubo algún problema.
 * \sa \struct cpu
 */
enum cpu_event
cpu_next_cycle(struct cpu *self)
{
    enum cpu_event ocurred_event;
    if (!self) { return -1; }
    if (self->halt) {
        return CPU_NONE;
    }
    /* Cargamos la instrucción de IR para su ejecución */
    struct inst *inst = (struct inst *)&self->regs[REG_IR];
    if (inst) {
        ocurred_event = (cpu_execute(self, inst) == 1)? CPU_INSTRUCTION_INVALID
                                                      : CPU_INSTRUCTION_EXECUTED;
        if (self->halt == 1) {
            ocurred_event = CPU_HALT;
        }
        if (self->div_by_zero == true) {
            ocurred_event = CPU_DIVISION_BY_ZERO;
            self->div_by_zero = false;
        }
        if (self->overflow == true) {
            ocurred_event = CPU_REGISTER_OVERFLOW;
            self->overflow = false;
        } 
    } else {
        ocurred_event = CPU_INSTRUCTION_INVALID;
    }
    if ((self->regs[REG_PC] - 1) < INSTS_MAX && !self->halt) {
        /**
         * Cargamos la siguiente instrucción a ejecutar en el IR y movemos el
         * PC a la siguiente siguiente instrucción.
         */
        memset(&self->regs[REG_IR], 0, sizeof(self->regs[REG_IR]));
        memcpy(&self->regs[REG_IR], &self->instmem[self->regs[REG_PC]],
               sizeof(self->instmem[self->regs[REG_PC]]));
        self->regs[REG_PC] += 1;
    } else {
        /**
         * Si nos pasamos del final de la memoria, el siguiente ciclo
         * ejecutará un END.
         */
        struct inst end = {
            .op = OP_END,
            .ra = REG_AX,
            .imm = 0
        };
        memcpy(&self->regs[REG_IR], &end, sizeof(end));
    }
    return ocurred_event;
}


/**
 * \brief Actualiza el estado interno de la CPU al instante actual.
 *
 * Realiza una actualización del estado interno para que este esté tenga el
 * estado que debería de tener en el instante del llamado, la CPU realizará
 * los ciclos de ejecución necesarios en caso de haberse tomado mucho tiempo
 * entre esta y la última llamada a cpu_sync().
 *
 * \param cpu la CPU a manipular
 * \return Si la CPU se ha actualizado internamente desde el último llamado a
 *         cpu_sync()
 */
int32_t
cpu_sync(struct cpu *self)
{
    if (self->halt) {
        return 0;
    }
    struct timespec current;
    clock_gettime(CLOCK_MONOTONIC, &current);
    struct timespec delta = {
        .tv_sec = current.tv_sec - self->last_cycle.tv_sec,
        .tv_nsec = current.tv_nsec - self->last_cycle.tv_nsec
    };
    uint32_t missed_cycles =
        (delta.tv_sec + ((double)delta.tv_nsec / 1E9)) /
        (self->target_freq.tv_sec + ((double)self->target_freq.tv_nsec / 1E9));
    /**
     * Temporal solo para ver si la CPU está funcionando
     */
    missed_cycles = (missed_cycles)? 1: 0;
    if (missed_cycles) {
        self->last_cycle = current;
        /**
         * Al sincronizar vamos a ejecutar la cantidad de ciclos que nos hemos
         * perdido entre cada sincronización.
         */
        for (uint32_t i = 0; i < missed_cycles; i += 1) {
            enum cpu_event event = cpu_next_cycle(self);
            queue_enqueue(self->events, event);
            if (event == CPU_HALT) {
                break;
            }
        }
        return 1;
    }
    return 0;
}

/**
 * \brief Establece la frecuencia de operación de la CPU
 *
 * \param self CPU a manipular
 * \param freq Frecuencia en Hz
 */
void
cpu_set_freq(struct cpu *self, double freq)
{
    double period = 1 / freq;

    self->target_freq.tv_sec = period;
    self->target_freq.tv_nsec = (period - self->target_freq.tv_sec)*1E9;
}

enum cpu_event 
cpu_poll_event(struct cpu *self)
{
    return queue_dequeue(self->events);
}

/*
 */
double
cpu_get_freq(struct cpu *self)
{
    /**
     * TODO: Medir la frecuencia real de la CPU y no usar solo la frecuencia objetivo
     */
    return 1 / (self->target_freq.tv_sec +
        ((double)self->target_freq.tv_nsec / 1E9));
}

struct cpu_context
cpu_dump_context(struct cpu *self)
{
    struct cpu_context context;
    memcpy(context.regs, self->regs, sizeof(self->regs));
    return context;
}

/**
 * \brief Carga el estado de la CPU desde un contexto guardado y memoria de instrucciones.
 *
 * Esta función restaura completamente el estado de una CPU a partir de un contexto
 * previamente guardado y una memoria de instrucciones. Es utilizada principalmente
 * para la implementación del algoritmo round-robin, permitiendo retomar la ejecución
 * de un proceso desde el punto donde fue interrumpido.
 *
 * \param self La CPU a manipular
 * \param context El contexto previamente guardado con los valores de registros
 * \param instmem El arreglo de instrucciones a cargar en la memoria de instrucciones
 * \return 0 si fue exitoso, < 0 en caso de error
 */
int32_t
cpu_load_from_context(struct cpu *self, struct cpu_context context, struct inst instmem[INSTS_MAX])
{
    if (!self) {
        return -1;
    }
    
    // Restaurar registros
    memcpy(self->regs, context.regs, sizeof(self->regs));
    
    // Restaurar memoria de instrucciones
    memcpy(self->instmem, instmem, sizeof(self->instmem));
    
    // Habilitar la CPU para ejecución
    self->halt = false;
    self->div_by_zero = false;
    self->overflow = false;
    
    // Actualizar el tiempo del último ciclo para sincronización
    clock_gettime(CLOCK_MONOTONIC, &self->last_cycle);
    
    // Limpiar eventos pendientes
    while (cpu_poll_event(self) != CPU_NONE) {
        // Vaciar la cola de eventos
    }
    
    return 0;
}
