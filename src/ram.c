#include <string.h>

#include <ram.h>

/**
 * El profesor no mencionó que la RAM sea vista como un archivo, por tanto
 * solo la manipularemos como un arreglo global de palabras, más sencillo que
 * manipular un archivo.
 */
word_t ram[RAM_TOTAL_WORD_SIZE]; 

/**
 * \brief Inicializa la RAM del simulador.
 * 
 * Por ahora creo que no hace nada, pero la dejo si hace falta inicializar algo
 * en el futuro.
 */
void
ram_init(void)
{
}

/**
 * \brief Obtiene una palabra en la RAM dada la dirección en bytes especificada
 * 
 * Esta función lee y regresa una palabra completa de la RAM del simulador.
 * Nota: si la dirección no es múltiplo de ocho bytes se regresará la palabra 
 * en la que está dicho byte.
 * 
 * \param addr Dirección de memoria a leer, especificada en bytes
 * \return La palabra leída de la RAM en la dirección especificada, se regresa
 *         una palabra con solo ceros a nivel de bits si la dirección es inválida.
 */
word_t
ram_read_word(uint16_t addr)
{
    return ram[addr / WORD_SIZE];
}

/**
 * \brief Realiza una lectura en la RAM del simulador.
 * 
 * \param dst Memoria en donde se escribirá los datos leidos en RAM
 * \param addr Dirección de memoria a leer, especificada en bytes
 * \param size Cantidad de bytes a leer desde la dirección especificada, se
 *             supone que el destino es del tamaño adecuado para los bytes que
 *             se van a leer.
 * 
 * \return 0 en caso de lectura existosa, 1 si la región a leer es inválida, 2
 *         si no es posible escribir en el destino.
 */
int32_t
ram_read(void *dst, const uint16_t addr, const uint16_t size)
{
    if (!dst) {
        return 2;
    }
    if ((addr + size) >= RAM_TOTAL_BYTE_SIZE) {
        return 1;
    }

    /* Tal vez sea mejor idea usar ram_read_word en un bucle for (? */
    memcpy(dst, &ram[addr / WORD_SIZE], size);

    return 0;
}

/**
 * \brief Escribe la palabra word en la memoria RAM
 * 
 * Esta función toma una palabra completa y la escribe en la memoria RAM
 * Nota: si la dirección no es múltiplo de ocho se tomará la palabra en
 *       la que se localiza el byte especificado
 * 
 * \param addr Dirección en bytes donde escribir la palabra
 * \param word Palabra a escribir
 */
int32_t
ram_write_word(const uint16_t addr, const word_t word)
{
    if (addr >= RAM_TOTAL_BYTE_SIZE) {
        return 1;
    }
    ram[addr / WORD_SIZE] = word;
    return 0;
}

/**
 * \brief Realiza una escritura en la RAM del simulador.
 * 
 * \param addr Dirección de memoria a escribir, especificada en bytes
 * \param data Datos a escribir en la RAM
 * \param size Cantidad de bytes a escribir desde la dirección especificada, se
 *             supone que el destino es del tamaño adecuado para los bytes que
 *             se van a escribir.
 * 
 * \return 0 en caso de escritura existosa, 1 si la región a escribir es inválida,
 *         2 los datos no se pueden leer.
 */
int32_t
ram_write(const uint16_t addr, const void *data, const uint16_t size)
{
    if (!data) {
        return 2;
    }
    if ((addr + size) >= RAM_TOTAL_BYTE_SIZE) {
        return 1;
    }

    memcpy(&ram[addr / WORD_SIZE], data, size);
    return 0;
}

uint16_t
ram_frame_to_addr(uint16_t frame_num)
{
    return frame_num * WORD_SIZE;
}