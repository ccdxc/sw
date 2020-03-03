//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines Doorbell API
///
//----------------------------------------------------------------------------

#ifndef __SDK_ASIC_DB_HPP__
#define __SDK_ASIC_DB_HPP__

#include "include/sdk/base.hpp"

namespace sdk {
namespace asic {
namespace pd {

/// \brief     doorbell update scheduler bits
typedef enum asic_db_upd_sched_e {
    ASIC_DB_UPD_SCHED_NONE = 0,
    ASIC_DB_UPD_SCHED_EVAL = 1,
    ASIC_DB_UPD_SCHED_COSA = 2,
    ASIC_DB_UPD_SCHED_COSB = 3,
} asic_db_upd_sched_t;

/// \brief     doorbell update index bits
typedef enum asic_db_upd_index_e {
    ASIC_DB_UPD_INDEX_UPDATE_NONE = 0,
    ASIC_DB_UPD_INDEX_SET_CINDEX  = 1,
    ASIC_DB_UPD_INDEX_SET_PINDEX  = 2,
    ASIC_DB_UPD_INDEX_INCR_PINDEX = 3,
} asic_db_upd_index_t;

#define ASIC_INDEX_UPD_SHIFT    2
#define ASIC_PID_CHECK_SHIFT    4
/// \brief     doorbell update bits
#define ASIC_DB_ADDR_UPD_FILL(sched, indx, pid_check)               \
    ((uint8_t) sched | (uint8_t) (indx << ASIC_INDEX_UPD_SHIFT) |   \
    (pid_check ? (1 << ASIC_PID_CHECK_SHIFT) : 0))

/// \brief     doorbell address info struct
typedef struct asic_db_addr_s {
    uint16_t lif_id;
    uint8_t upd;
    uint8_t q_type;
} asic_db_addr_t;

/// \brief     get host doorbell address
/// \param[in] lif id
/// \param[in] queue type
/// \param[in] update bits
/// \return    doorbell address for the lif
uint64_t asic_hostdb_addr(uint16_t lif_id, uint8_t q_type, uint8_t upd);

/// \brief     get local doorbell address
/// \param[in] lif id
/// \param[in] queue type
/// \param[in] update bits
/// \return    doorbell address for the lif
uint64_t asic_localdb_addr(uint16_t lif_id, uint8_t q_type, uint8_t upd);

/// \brief     ring host doorbell 64 bit data
/// \param[in] doorbell address info
/// \param[in] doorbell data
void asic_ring_hostdb(asic_db_addr_t *db_addr, uint64_t data);

/// \brief     ring doorbell 64 bit data
/// \param[in] doorbell address info
/// \param[in] doorbell data
void asic_ring_db(asic_db_addr_t *db_addr, uint64_t data);

/// \brief     ring doorbell 32 bit data
/// \param[in] doorbell address info
/// \param[in] doorbell data
void asic_ring_db(asic_db_addr_t *db_addr, uint32_t data);

/// \brief     ring doorbell 16 bit data
/// \param[in] doorbell address info
/// \param[in] doorbell data
void asic_ring_db(asic_db_addr_t *db_addr, uint16_t data);


}    // namespace pd
}    // namespace asic
}    // namespace sdk

using sdk::asic::pd::asic_db_addr_t;
using sdk::asic::pd::asic_db_upd_index_t;
using sdk::asic::pd::asic_db_upd_sched_t;
using asic_db_upd_sched_t::ASIC_DB_UPD_SCHED_NONE;
using asic_db_upd_sched_t::ASIC_DB_UPD_SCHED_EVAL;
using asic_db_upd_sched_t::ASIC_DB_UPD_SCHED_COSA;
using asic_db_upd_sched_t::ASIC_DB_UPD_SCHED_COSB;
using asic_db_upd_index_t::ASIC_DB_UPD_INDEX_UPDATE_NONE;
using asic_db_upd_index_t::ASIC_DB_UPD_INDEX_SET_CINDEX;
using asic_db_upd_index_t::ASIC_DB_UPD_INDEX_SET_PINDEX;
using asic_db_upd_index_t::ASIC_DB_UPD_INDEX_INCR_PINDEX;

#endif    // __SDK_ASIC_DB_HPP__
