
#ifndef	_NICMGR_IF_HPP_
#define	_NICMGR_IF_HPP_

#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdio.h>
#include "nic/include/eth_common.h"
#include "nic/include/storage_seq_common.h"
#include "nic/include/accel_ring.h"

namespace nicmgr_if {

int nicmgr_if_reset(void);
int nicmgr_if_init(void);
int nicmgr_if_identify(uint64_t *ret_seq_lif,
                       uint32_t *ret_seq_queues_per_lif,
                       accel_ring_t *ret_accel_ring_tbl = nullptr,
                       uint32_t accel_ring_tbl_size = 0);
int nicmgr_if_lif_init(uint64_t seq_lif);
int nicmgr_if_admin_queue_init(uint64_t seq_lif,
                               uint16_t log2_num_entries,
                               uint64_t base_addr);
int nicmgr_if_seq_queue_init(uint64_t lif,
                             storage_seq_qgroup_t qgroup,
                             uint32_t qid,
                             uint8_t total_wrings,
                             uint8_t host_wrings,
                             uint16_t log2_num_entries,
                             uint64_t base_addr,
                             uint16_t log2_entry_size);
}  // namespace nicmgr_if

#endif /* _NICMGR_IF_HPP_ */
