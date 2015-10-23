/** 
 * @file mb_register.h
 *
 * Modbus register related function prototypes and definitions
 */

#ifndef _MB_REGISTER_H
#define _MB_REGISTER_H

#include "mb_common.h"

/** Holding address offset */
#define MB_REGISTER_HOLDING_ADDRESS_OFFSET 0

/** Coil address offset */
#define MB_REGISTER_COIL_ADDRESS_OFFSET 0

/** Discrete input address offset */
#define MB_REGISTER_DISCRETE_ADDRESS_OFFSET 0

/** Input register address offset */
#define MB_REGISTER_INPUT_ADDRESS_OFFSET 0

/**
 * Initialize coil register space
 *
 * @param ptr pointer to 8-bit coil registers
 * @param size length of coil register space
 */
void mb_register_coil_init(uint8_t *ptr, uint16_t size);

/**
 * Initialize discrete input register space
 *
 * @param ptr pointer to 8-bit discrete input registers
 * @param size length of discrete input register space
 */
void mb_register_discrete_init(uint8_t *ptr, uint16_t size);

/**
 * Initialize input register space
 *
 * @param ptr pointer to 16-bit input registers
 * @param size length of input register space
 */
void mb_register_input_init(uint16_t *ptr, uint16_t size);

/**
 * Initialize holding register space
 *
 * @param ptr pointer to 16-bit holding registers
 * @param size length of holding register space
 */
void mb_register_holding_init(uint16_t *ptr, uint16_t size);

/**
 * Write a single coil
 *
 * @param addr		coil address
 * @param value		coil value (0 or 1)
 */
mb_retval_t mb_register_coil_write(uint16_t addr, uint8_t value);

/**
 * Read multiple coils
 *
 * @param addr		start address
 * @param nbits		number of bits to read
 * @param dst		pointer to memory where values are to be stored, most significant byte/bit first
 */
mb_retval_t mb_register_coil_read_multiple(uint16_t addr, uint16_t nbits, uint8_t *dst);

/**
 * Read multiple discrete inputs
 *
 * @param addr		start address
 * @param nbits		number of bits to read
 * @param dst		pointer to memory where values are to be stored, most significant byte/bit first
 */
mb_retval_t mb_register_discrete_read_multiple(uint16_t addr, uint16_t nbits, uint8_t *dst);

/**
 * Read multiple input registers
 *
 * @param addr		start address
 * @param nwords	number of 16-bit words to read
 * @param dst		pointer to memory where values are to be stored, most significant byte/bit of each word first
 */
mb_retval_t mb_register_input_read_multiple(uint16_t addr, uint16_t nwords, uint8_t *dst);

/**
 * Read multiple holding registers
 *
 * @param addr		start address
 * @param nwords	number of 16-bit words to read
 * @param dst		pointer to memory where values are to be stored, most significant byte/bit of each word first
 */
mb_retval_t mb_register_holding_read_multiple(uint16_t addr, uint16_t nwords, uint8_t *dst);

/**
 * Write multiple holding registers
 *
 * @param addr		start address
 * @param nwords	number of 16-bit words to write
 * @param dst		pointer to memory where values are to be read from, most significant byte/bit first
 */
mb_retval_t mb_register_holding_write_multiple(uint16_t addr, uint16_t nwords, uint8_t *src);
#endif /* _MB_REGISTER_H */
