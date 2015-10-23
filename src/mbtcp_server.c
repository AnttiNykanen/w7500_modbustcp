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

#include "mb_common.h"
#include "mb_register.h"
#include "mb_tcp_slave.h"
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



void mbtcp_server_run(uint8_t seqnum)
{
    uint8_t sock, rv;
    uint16_t rx_len;
    size_t tx_len;
    mb_tcp_adu_t *rx_adu, *tx_adu;

    sock = mbtcp_server_get_socket(seqnum);
    rx_adu = &mbtcp_server_states[seqnum].rx_adu;
    tx_adu = &mbtcp_server_states[seqnum].tx_adu;

    switch(getSn_SR(sock)) {
    case SOCK_ESTABLISHED:
        if ((rx_len = getSn_RX_RSR(sock)) >= 7) {
            recv(sock, (uint8_t *)rx_adu, rx_len);
            tx_len = mb_tcp_slave_handle_request(rx_adu, tx_adu);
            send(sock, (uint8_t *)tx_adu, tx_len);
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
