/** 
 * @file mb_tcp_slave.h
 * 
 * Modbus/TCP slave functions
 *
 */

#ifndef _MB_TCP_SLAVE_H
#define _MB_TCP_SLAVE_H

#include "mb_common.h"

/**
 * Handle received MB/TCP ADU and prepare the response to it
 * 
 * @param rx_adu received MB/TCP request
 * @param tx_adu pointer to response ADU
 */
size_t mb_tcp_slave_handle_request(mb_tcp_adu_t *rx_adu, mb_tcp_adu_t *tx_adu);

#endif /* _MB_TCP_SLAVE_H */
