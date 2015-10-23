/*
 * mbtcp_server.h
 *
 *  Created on: Oct 8, 2015
 *      Author: aon
 */

#include "mb_common.h"

#ifndef _MBTCP_SERVER_H
#define _MBTCP_SERVER_H

#define MBTCP_DEBUG

#define MBTCP_NSOCK 4
#define MBTCP_DATALEN 256
#define MBTCP_PORT 502

typedef struct {
    mb_tcp_adu_t rx_adu;
    mb_tcp_adu_t tx_adu;
} mbtcp_server_state_t;

typedef enum {
    MBTCP_SERVER_STATE_IDLE,
    MBTCP_SERVER_STATE_REQUEST_RECEIVED,
    MBTCP_SERVER_STATE_DATA_READY
} e_mbtcp_server_state;

void mbtcp_server_init(uint8_t *socklist, uint8_t nsocks);
void mbtcp_server_run(uint8_t seqnum);

#endif /* _MBTCP_SERVER_H */
