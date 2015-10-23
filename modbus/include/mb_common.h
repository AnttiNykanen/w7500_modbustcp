/**
 * @file mb_common.h
 *
 * Common Modbus data structures and definitions
 *
 */

#ifndef _MB_COMMON_H
#define _MB_COMMON_H

#include <stdint.h>

typedef enum {
    MB_SUCCESS,
    MB_ERR_ADDRESS
} mb_retval_t;

typedef union {
    uint16_t u16;
    uint8_t u8[2];
} mb_word_t;

typedef struct {
    uint8_t function_code;
    uint8_t data[252];
} mb_pdu_t;

typedef struct {
    mb_word_t transaction_id;
    mb_word_t protocol_id;
    mb_word_t length;
    uint8_t   unit_id;
    mb_pdu_t  pdu;
} mb_tcp_adu_t;

/* Class 0 commands */
#define MB_FC_READ_HOLDING_REGISTERS 0x03
#define MB_FC_WRITE_HOLDING_REGISTERS 0x10

#define MB_FC_EXCEPTION_READ_HOLDING_REGISTERS 0x83
#define MB_FC_EXCEPTION_WRITE_HOLDING_REGISTERS 0x90

/* Class 1 commands */
#define MB_FC_READ_COILS 0x01
#define MB_FC_WRITE_COIL 0x05
#define MB_FC_READ_INPUT_DISCRETES 0x02
#define MB_FC_READ_INPUT_REGISTERS 0x04
#define MB_FC_WRITE_SINGLE_HOLDING_REGISTER 0x06

#define MB_FC_EXCEPTION_WRITE_SINGLE_HOLDING_REGISTER 0x86
#define MB_FC_EXCEPTION_WRITE_COIL 0x85
#define MB_FC_EXCEPTION_READ_COILS 0x81
#define MB_FC_EXCEPTION_READ_INPUT_DISCRETES 0x82
#define MB_FC_EXCEPTION_READ_INPUT_REGISTERS 0x84

/* Exception codes */
#define MB_EXCEPTION_ILLEGAL_FUNCTION 0x01
#define MB_EXCEPTION_ILLEGAL_DATA_ADDRESS 0x02
#define MB_EXCEPTION_ILLEGAL_DATA_VALUE 0x03




#endif /* _MB_COMMON_H */
