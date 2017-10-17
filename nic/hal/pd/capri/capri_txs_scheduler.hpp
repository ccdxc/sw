/*
 * capri_txs_scheduler.hpp
 * Vishwas Danivas (Pensando Systems)
 */

#ifndef __CAPRI_TXS_SCHEDULER_HPP__
#define __CAPRI_TXS_SCHEDULER_HPP__

#include <stdio.h>
#include <string>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include "nic/include/base.h"

/** capri_txs_scheduler_init
 * API to init the txs scheduler module
 *
 * @return hal_ret_t: Status of the operation
 */

hal_ret_t capri_txs_scheduler_init();

#endif
