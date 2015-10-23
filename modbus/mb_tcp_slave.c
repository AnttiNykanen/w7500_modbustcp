#include <strings.h>
#include "mb_register.h"
#include "mb_tcp_slave.h"

static void mb_tcp_slave_read_holding_registers(uint8_t *data, mb_tcp_adu_t *tx_adu)
{
    mb_word_t address, count;
    mb_retval_t rv;

    address.u8[0] = data[1];
    address.u8[1] = data[0];

    count.u8[0] = data[3];
    count.u8[1] = data[2];

    rv = mb_register_holding_read_multiple(address.u16, count.u16, &tx_adu->pdu.data[1]);
    if (rv == MB_SUCCESS) {
        tx_adu->pdu.function_code = MB_FC_READ_HOLDING_REGISTERS;
        tx_adu->pdu.data[0] = (uint8_t)(count.u8[0] * 2);
        tx_adu->length.u8[1] = tx_adu->pdu.data[0] + 3; // data length + unit id + function code + data0
    } else {
        tx_adu->pdu.function_code = MB_FC_EXCEPTION_READ_HOLDING_REGISTERS;
        tx_adu->pdu.data[0] = MB_EXCEPTION_ILLEGAL_DATA_ADDRESS; // illegal data address
        tx_adu->length.u8[1] = 3; // unit id + function code + data0
    }
}

static void mb_tcp_slave_write_holding_registers(uint8_t *rx_data, mb_tcp_adu_t *tx_adu)
{
    mb_word_t address, count;
    mb_retval_t rv;

    address.u8[0] = rx_data[1];
    address.u8[1] = rx_data[0];

    count.u8[0] = rx_data[3];
    count.u8[1] = rx_data[2];

    // rx_data[4] is byte count, can be ignored

    rv = mb_register_holding_write_multiple(address.u16, count.u16, rx_data + 5);
    if (rv == MB_SUCCESS) {
        tx_adu->pdu.function_code = MB_FC_WRITE_HOLDING_REGISTERS;
        tx_adu->pdu.data[0] = rx_data[0];
        tx_adu->pdu.data[1] = rx_data[1];
        tx_adu->pdu.data[2] = rx_data[2];
        tx_adu->pdu.data[3] = rx_data[3];
        tx_adu->length.u8[1] = 6; // unit id + function code + data{0,1} = reference number + data{2,3} = word count
    } else {
        tx_adu->pdu.function_code = MB_FC_EXCEPTION_WRITE_HOLDING_REGISTERS;
        tx_adu->pdu.data[0] = MB_EXCEPTION_ILLEGAL_DATA_ADDRESS; // illegal data address
        tx_adu->length.u8[1] = 3; // unit id + function code + data0
    }
}

static void mb_tcp_slave_write_single_holding_register(uint8_t *rx_data, mb_tcp_adu_t *tx_adu)
{
    mb_word_t address;
    mb_retval_t rv;

    address.u8[0] = rx_data[1];
    address.u8[1] = rx_data[0];

    rv = mb_register_holding_write_multiple(address.u16, 1, &rx_data[2]);

    if (rv == MB_SUCCESS) {
        tx_adu->pdu.function_code = MB_FC_WRITE_SINGLE_HOLDING_REGISTER;
        tx_adu->pdu.data[0] = rx_data[0];
        tx_adu->pdu.data[1] = rx_data[1];
        tx_adu->pdu.data[2] = rx_data[2];
        tx_adu->pdu.data[3] = rx_data[3];
        tx_adu->length.u8[1] = 6; // unit id + function code + data{0..3}
    } else {
        tx_adu->pdu.function_code = MB_FC_EXCEPTION_WRITE_SINGLE_HOLDING_REGISTER;
        tx_adu->pdu.data[0] = MB_EXCEPTION_ILLEGAL_DATA_ADDRESS;
        tx_adu->length.u8[1] = 3; // unit id + function code + data[0]
    }
}

static void mb_tcp_slave_write_coil(uint8_t *rx_data, mb_tcp_adu_t *tx_adu)
{
    mb_word_t address;
    mb_retval_t rv;

    address.u8[0] = rx_data[1];
    address.u8[1] = rx_data[0];

    rv = mb_register_coil_write(address.u16, rx_data[2]);

    if (rv == MB_SUCCESS) {
        tx_adu->pdu.function_code = MB_FC_WRITE_COIL;
        tx_adu->pdu.data[0] = rx_data[0];
        tx_adu->pdu.data[1] = rx_data[1];
        tx_adu->pdu.data[2] = rx_data[2];
        tx_adu->pdu.data[3] = 0x00;
        tx_adu->length.u8[1] = 6; // unit id + function code + data{0..3}
    } else {
        tx_adu->pdu.function_code = MB_FC_EXCEPTION_WRITE_COIL;
        tx_adu->pdu.data[0] = MB_EXCEPTION_ILLEGAL_DATA_ADDRESS;
        tx_adu->length.u8[1] = 3; // unit id + function code + data[0]
    }
}

static void mb_tcp_slave_read_coils(uint8_t *rx_data, mb_tcp_adu_t *tx_adu)
{
    mb_word_t address, bitcount;
    mb_retval_t rv;

    address.u8[0] = rx_data[1];
    address.u8[1] = rx_data[0];
    bitcount.u8[0] = rx_data[3];
    bitcount.u8[1] = rx_data[2];

    rv = mb_register_coil_read_multiple(address.u16, bitcount.u16, &tx_adu->pdu.data[1]);
    if (rv == MB_SUCCESS) {
        tx_adu->pdu.function_code = MB_FC_READ_COILS;
        tx_adu->pdu.data[0] = (uint8_t)((bitcount.u16 + 7) / 8);
        tx_adu->length.u8[1] = tx_adu->pdu.data[0] + 3; // data length + unit id + function code + data0
    } else {
        tx_adu->pdu.function_code = MB_FC_EXCEPTION_READ_COILS;
        tx_adu->pdu.data[0] = MB_EXCEPTION_ILLEGAL_DATA_ADDRESS;
        tx_adu->length.u8[1] = 3; // unit id + function code + data[0]
    }
}

static void mb_tcp_slave_read_input_discretes(uint8_t *rx_data, mb_tcp_adu_t *tx_adu)
{
    mb_word_t address, bitcount;
    mb_retval_t rv;

    address.u8[0] = rx_data[1];
    address.u8[1] = rx_data[0];
    bitcount.u8[0] = rx_data[3];
    bitcount.u8[1] = rx_data[2];

    rv = mb_register_discrete_read_multiple(address.u16, bitcount.u16, &tx_adu->pdu.data[1]);
    if (rv == MB_SUCCESS) {
        tx_adu->pdu.function_code = MB_FC_READ_INPUT_DISCRETES;
        tx_adu->pdu.data[0] = (uint8_t)((bitcount.u16 + 7) / 8);
        tx_adu->length.u8[1] = tx_adu->pdu.data[0] + 3; // data length + unit id + function code + data0
    } else {
        tx_adu->pdu.function_code = MB_FC_EXCEPTION_READ_INPUT_DISCRETES;
        tx_adu->pdu.data[0] = MB_EXCEPTION_ILLEGAL_DATA_ADDRESS;
        tx_adu->length.u8[1] = 3; // unit id + function code + data[0]
    }
}

static void mb_tcp_slave_read_input_registers(uint8_t *data, mb_tcp_adu_t *tx_adu)
{
    mb_word_t address, count;
    mb_retval_t rv;

    address.u8[0] = data[1];
    address.u8[1] = data[0];

    count.u8[0] = data[3];
    count.u8[1] = data[2];

    rv = mb_register_input_read_multiple(address.u16, count.u16, &tx_adu->pdu.data[1]);
    if (rv == MB_SUCCESS) {
        tx_adu->pdu.function_code = MB_FC_READ_INPUT_REGISTERS;
        tx_adu->pdu.data[0] = (uint8_t)(count.u8[0] * 2);
        tx_adu->length.u8[1] = tx_adu->pdu.data[0] + 3; // data length + unit id + function code + data0
    } else {
        tx_adu->pdu.function_code = MB_FC_EXCEPTION_READ_INPUT_REGISTERS;
        tx_adu->pdu.data[0] = MB_EXCEPTION_ILLEGAL_DATA_ADDRESS; // illegal data address
        tx_adu->length.u8[1] = 3; // unit id + function code + data0
    }
}

static void mb_tcp_slave_illegal_function(mb_tcp_adu_t *rx_adu, mb_tcp_adu_t *tx_adu)
{
    tx_adu->pdu.function_code = rx_adu->pdu.function_code + 0x80;
    tx_adu->pdu.data[0] = MB_EXCEPTION_ILLEGAL_FUNCTION;
    tx_adu->length.u8[1] = 3;
}

size_t mb_tcp_slave_handle_request(mb_tcp_adu_t *rx_adu, mb_tcp_adu_t *tx_adu)
{
    bzero(tx_adu, sizeof(mb_tcp_adu_t));
    
    switch (rx_adu->pdu.function_code) {
    case MB_FC_READ_HOLDING_REGISTERS:
        mb_tcp_slave_read_holding_registers(rx_adu->pdu.data, tx_adu);
        break;
    case MB_FC_WRITE_HOLDING_REGISTERS:
        mb_tcp_slave_write_holding_registers(rx_adu->pdu.data, tx_adu);
        break;
    case MB_FC_WRITE_SINGLE_HOLDING_REGISTER:
        mb_tcp_slave_write_single_holding_register(rx_adu->pdu.data, tx_adu);
        break;
    case MB_FC_WRITE_COIL:
        mb_tcp_slave_write_coil(rx_adu->pdu.data, tx_adu);
        break;
    case MB_FC_READ_COILS:
        mb_tcp_slave_read_coils(rx_adu->pdu.data, tx_adu);
        break;
    case MB_FC_READ_INPUT_DISCRETES:
        mb_tcp_slave_read_input_discretes(rx_adu->pdu.data, tx_adu);
        break;
    case MB_FC_READ_INPUT_REGISTERS:
        mb_tcp_slave_read_input_registers(rx_adu->pdu.data, tx_adu);
        break;
    default:
        mb_tcp_slave_illegal_function(rx_adu, tx_adu);
    }

    tx_adu->transaction_id.u16 = rx_adu->transaction_id.u16;

    return (tx_adu->length.u8[1] + 6);
}
