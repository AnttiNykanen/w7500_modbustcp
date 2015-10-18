/*
 * mbtcp_server.c
 *
 *  Created on: Oct 8, 2015
 *      Author: aon
 */

#include <strings.h>
#include "wizchip_conf.h"
#include "W7500x_wztoe.h"
#include "socket.h"

#include "common.h"
#include "mb_common.h"
#include "mb_register.h"
#include "mbtcp_server.h"

static uint8_t mbtcp_sockets[MBTCP_NSOCK];
static mbtcp_server_state_t mbtcp_server_states[MBTCP_NSOCK];

static uint8_t mbtcp_server_get_socket(uint8_t seqnum)
{
	return mbtcp_sockets[seqnum];
}

static void mbtcp_server_sockinit(uint8_t *socklist, uint8_t nsocks)
{
	uint8_t i, n;

	if (nsocks > MBTCP_NSOCK)
		n = MBTCP_NSOCK;
	else
		n = nsocks;

	for (i = 0; i < n; i++) {
		mbtcp_sockets[i] = socklist[i];
	}
}

void mbtcp_server_init(uint8_t *socklist, uint8_t nsocks)
{
	mbtcp_server_sockinit(socklist, nsocks);
}

static void mbtcp_server_read_holding_registers(uint8_t *data, mb_tcp_adu_t *tx_adu)
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

static void mbtcp_server_write_holding_registers(uint8_t *rx_data, mb_tcp_adu_t *tx_adu)
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

static void mbtcp_server_write_single_holding_register(uint8_t *rx_data, mb_tcp_adu_t *tx_adu)
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

static void mbtcp_server_write_coil(uint8_t *rx_data, mb_tcp_adu_t *tx_adu)
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

static void mbtcp_server_read_coils(uint8_t *rx_data, mb_tcp_adu_t *tx_adu)
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

static void mbtcp_server_read_input_discretes(uint8_t *rx_data, mb_tcp_adu_t *tx_adu)
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

static void mbtcp_server_read_input_registers(uint8_t *data, mb_tcp_adu_t *tx_adu)
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

void mbtcp_server_illegal_function(mb_tcp_adu_t *rx_adu, mb_tcp_adu_t *tx_adu)
{
	tx_adu->pdu.function_code = rx_adu->pdu.function_code + 0x80;
	tx_adu->pdu.data[0] = MB_EXCEPTION_ILLEGAL_FUNCTION;
	tx_adu->length.u8[1] = 3;
}

void mbtcp_server_run(uint8_t seqnum)
{
	uint8_t sock, rv, state;
	uint16_t len;
	mb_tcp_adu_t *rx_adu, *tx_adu;

	sock = mbtcp_server_get_socket(seqnum);
	state = mbtcp_server_states[seqnum].state;
	rx_adu = &mbtcp_server_states[seqnum].rx_adu;
	tx_adu = &mbtcp_server_states[seqnum].tx_adu;

	switch(getSn_SR(sock)) {
	case SOCK_ESTABLISHED:
		switch (state) {
		case MBTCP_SERVER_STATE_IDLE:
			if ((len = getSn_RX_RSR(sock)) >= 7) {
				recv(sock, (uint8_t *)rx_adu, len);
				mbtcp_server_states[seqnum].state = MBTCP_SERVER_STATE_REQUEST_RECEIVED;
			}
			break;
		case MBTCP_SERVER_STATE_REQUEST_RECEIVED:
			switch (rx_adu->pdu.function_code) {
			case MB_FC_READ_HOLDING_REGISTERS:
				mbtcp_server_read_holding_registers(rx_adu->pdu.data, tx_adu);
				break;
			case MB_FC_WRITE_HOLDING_REGISTERS:
				mbtcp_server_write_holding_registers(rx_adu->pdu.data, tx_adu);
				break;
			case MB_FC_WRITE_SINGLE_HOLDING_REGISTER:
				mbtcp_server_write_single_holding_register(rx_adu->pdu.data, tx_adu);
				break;
			case MB_FC_WRITE_COIL:
				mbtcp_server_write_coil(rx_adu->pdu.data, tx_adu);
				break;
			case MB_FC_READ_COILS:
				mbtcp_server_read_coils(rx_adu->pdu.data, tx_adu);
				break;
			case MB_FC_READ_INPUT_DISCRETES:
				mbtcp_server_read_input_discretes(rx_adu->pdu.data, tx_adu);
				break;
			case MB_FC_READ_INPUT_REGISTERS:
				mbtcp_server_read_input_registers(rx_adu->pdu.data, tx_adu);
				break;
			default:
				mbtcp_server_illegal_function(rx_adu, tx_adu);
			}

			tx_adu->transaction_id.u16 = rx_adu->transaction_id.u16;

			send(sock, (uint8_t *)tx_adu, tx_adu->length.u8[1] + 6);

			bzero(tx_adu->pdu.data, 252);

			mbtcp_server_states[seqnum].state = MBTCP_SERVER_STATE_IDLE;
			break;
		}
		break;
	case SOCK_CLOSE_WAIT:
#ifdef MBTCP_DEBUG
		printf("mbtcp_server (%d): SOCK_CLOSE_WAIT\r\n", sock);
#endif
		disconnect(sock);
		break;
	case SOCK_CLOSED:
#ifdef MBTCP_DEBUG
		printf("mbtcp_server (%d): SOCK_CLOSED\r\n", sock);
#endif
		rv = socket(sock, Sn_MR_TCP, MBTCP_PORT, 0);
#ifdef MBTCP_DEBUG
		if (rv == sock) {
			printf("mbtcp_server (%d): socket()\r\n", sock);
		} else if (rv == SOCKERR_SOCKFLAG) {
			printf("mbtcp_server (%d): SOCKERR_SOCKFLAG\r\n", sock);
		} else if (rv == SOCKERR_SOCKMODE) {
			printf("mbtcp_server (%d): SOCKERR_SOCKMODE\r\n", sock);
		}
#endif
		break;
	case SOCK_INIT:
#ifdef MBTCP_DEBUG
		printf("mbtcp_server (%d): SOCK_INIT\r\n", sock);
#endif
		listen(sock);
		mbtcp_server_states[seqnum].state = MBTCP_SERVER_STATE_IDLE;
		break;
	case SOCK_LISTEN:
		break;
	default:
#ifdef MBTCP_DEBUG
		printf("mbtcp_server (%d): default\r\n", sock);
#endif
		break;
	}
}
