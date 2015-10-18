/*
 * mb_register.c
 *
 *  Created on: Oct 9, 2015
 *      Author: aon
 */

#include "mb_common.h"
#include "mb_register.h"

static uint16_t *holding_registers;
static uint16_t holding_registers_size = 0;

static uint8_t *coil_registers;
static uint16_t coil_registers_size = 0;

static uint8_t *discrete_registers;
static uint16_t discrete_registers_size = 0;

static uint16_t *input_registers;
static uint16_t input_registers_size = 0;

void mb_register_holding_init(uint16_t *ptr, uint16_t size)
{
	holding_registers = ptr;
	holding_registers_size = size;
}

void mb_register_coil_init(uint8_t *ptr, uint16_t size)
{
	coil_registers = ptr;
	coil_registers_size = size;
}

void mb_register_discrete_init(uint8_t *ptr, uint16_t size)
{
	discrete_registers = ptr;
	discrete_registers_size = size;
}

void mb_register_input_init(uint16_t *ptr, uint16_t size)
{
	input_registers = ptr;
	input_registers_size = size;
}

static mb_retval_t mb_register_word_read_multiple(uint16_t *src, uint16_t src_size, uint16_t src_offs,
												  uint16_t addr, uint16_t nwords, uint8_t *dst)
{
	uint16_t i;
	addr -= src_offs;

	if (addr > (src_size - 1) || (addr + nwords) > src_size) {
		return MB_ERR_ADDRESS;
	}

	for (i = addr; i < addr + nwords; i++) {
		*(dst++) = (uint8_t)(src[i] >> 8);
		*(dst++) = (uint8_t)src[i];
	}

	return MB_SUCCESS;
}

mb_retval_t mb_register_holding_read_multiple(uint16_t addr, uint16_t nwords, uint8_t *dst)
{
	return mb_register_word_read_multiple(holding_registers, holding_registers_size,
										  MB_REGISTER_HOLDING_ADDRESS_OFFSET, addr, nwords, dst);
}

mb_retval_t mb_register_input_read_multiple(uint16_t addr, uint16_t nwords, uint8_t *dst)
{
	return mb_register_word_read_multiple(input_registers, input_registers_size,
										  MB_REGISTER_INPUT_ADDRESS_OFFSET, addr, nwords, dst);
}

mb_retval_t mb_register_holding_write_multiple(uint16_t addr, uint16_t nwords, uint8_t *src)
{
	uint16_t i;

	addr -= MB_REGISTER_HOLDING_ADDRESS_OFFSET;

	if (addr > (holding_registers_size - 1) || (addr + nwords) > holding_registers_size) {
		return MB_ERR_ADDRESS;
	}

	for (i = addr; i < addr + nwords; i ++) {
		holding_registers[i] = (uint16_t)*(src++) << 8 | *(src++);
	}

	return MB_SUCCESS;
}

mb_retval_t mb_register_coil_write(uint16_t addr, uint8_t value)
{
	uint16_t byte, bit;
	uint8_t mask;

	addr -= MB_REGISTER_COIL_ADDRESS_OFFSET;

	byte = addr / 8;
	if (byte > coil_registers_size - 1)
		return MB_ERR_ADDRESS;

	bit = addr % 8;

	mask = 1 << bit;

	if (value == 0xFF) {
		coil_registers[byte] |= mask;
	} else if (value == 0x00) {
		mask ^= 0xFF;
		coil_registers[byte] &= mask;
	}

	return MB_SUCCESS;
}

static mb_retval_t mb_register_bits_read_multiple(uint8_t *src, uint16_t src_size, uint16_t src_offset,
												  uint16_t addr, uint16_t nbits,
												  uint8_t *dst)
{
	uint16_t sbyte, sbit, ebyte, ebit;
	uint16_t rbyte, wbyte;
	uint8_t rmask, wmask, smask, emask;

	sbyte = addr / 8;
	sbit = addr % 8;
	ebyte = (addr + nbits - 1) / 8;
	ebit = (sbit + nbits - 1) % 8;

	sbyte -= src_offset;
	ebyte -= src_offset;

	if (sbyte > src_size - 1 || ebyte > src_size - 1) {
		return MB_ERR_ADDRESS;
	}

	wbyte = (nbits - 1) / 8;
	emask = 0x01 << (ebit + 1);
	smask = 0x01 << sbit;
	wmask = 0x01;

	for (rbyte = sbyte; rbyte <= ebyte; rbyte++) {
		for (rmask  = (rbyte == sbyte ? smask : 0x01);
			 rmask != (rbyte == ebyte ? emask : 0x00);
			 rmask <<= 1) {

			if (src[rbyte] & rmask) {
				dst[wbyte] |= wmask;
			}

			if ((wmask <<= 1) == 0x00) {
				wmask = 0x01;
				wbyte--;
			}
		}
	}

	return MB_SUCCESS;
}

mb_retval_t mb_register_coil_read_multiple(uint16_t addr, uint16_t nbits, uint8_t *dst)
{
	return mb_register_bits_read_multiple(coil_registers, coil_registers_size,
										  MB_REGISTER_COIL_ADDRESS_OFFSET, addr, nbits, dst);
}

mb_retval_t mb_register_discrete_read_multiple(uint16_t addr, uint16_t nbits, uint8_t *dst)
{
	return mb_register_bits_read_multiple(discrete_registers, discrete_registers_size,
										  MB_REGISTER_DISCRETE_ADDRESS_OFFSET, addr, nbits, dst);
}
