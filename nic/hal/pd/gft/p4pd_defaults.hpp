#ifndef __P4PD_DEFAULTS_HPP__
#define __P4PD_DEFAULTS_HPP__
#include "nic/p4/gft/include/table_sizes.h"

// Transposition table size: 64k
#define TX_TRANSPOSITION_NOP_ENTRY       (HDR_TRANSPOSITIONS_TABLE_SIZE - 1)
#define RX_TRANSPOSITION_NOP_ENTRY       (HDR_TRANSPOSITIONS_TABLE_SIZE - 1)

#endif  // __P4PD_DEFAULTS_HPP__
