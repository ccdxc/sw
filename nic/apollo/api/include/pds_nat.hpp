//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines nat APIs
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_NAT_HPP__
#define __INCLUDE_API_PDS_NAT_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/apollo/api/include/pds.hpp"

/// \defgroup PDS_NAT nat APIs
/// @{

/// \brief nat port block specification
typedef struct pds_nat_port_block_spec_s {
    pds_nat_port_block_key_t key;            ///< NAT port block key
    pds_vpc_key_t            vpc;            ///< vpc
    uint8_t                  ip_proto;       ///< ip protocol
    ipvx_range_t             nat_ip_range;   ///< NAT ip range
    port_range_t             nat_port_range; ///< NAT port range
} __PACK__ pds_nat_port_block_spec_t;

/// \brief nat port block status
typedef struct pds_nat_port_block_status_s {
} __PACK__ pds_nat_port_block_status_t;

/// \brief nat port block statistics
typedef struct pds_nat_port_block_stats_s {
    uint32_t    in_use_count;               ///< number of NAT ports in use
    uint32_t    session_count;              ///< number of sessions using
                                            ///< the port block
} __PACK__ pds_nat_port_block_stats_t;

/// \brief nat port block information
typedef struct pds_nat_port_block_info_s {
    pds_nat_port_block_spec_t   spec;      ///< specification
    pds_nat_port_block_status_t status;    ///< status
    pds_nat_port_block_stats_t  stats;     ///< statistics
} __PACK__ pds_nat_port_block_info_t;

/// \brief     create nat port block
/// \param[in] spec nat port block specification
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return    #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_nat_port_block_create(pds_nat_port_block_spec_t *spec,
                                    pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief      read a given nat port block
/// \param[in]  key   key/id of the nat port block
/// \param[out] info  nat port block information
/// \return     #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_nat_port_block_read(pds_nat_port_block_key_t *key,
                                  pds_nat_port_block_info_t *info);

/// \brief     update nat_port_block
/// \param[in] spec   nat port block specification
/// \param[in] bctxt  batch context if API is invoked in a batch
/// \return    #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_nat_port_block_update(pds_nat_port_block_spec_t *spec,
                                    pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// \brief     delete a given nat port block
/// \param[in] key/id id of the nat port block
/// \param[in] bctxt batch context if API is invoked in a batch
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid nat_port_block id should be passed
sdk_ret_t pds_nat_port_block_delete(pds_nat_port_block_key_t *key,
                                    pds_batch_ctxt_t bctxt = PDS_BATCH_CTXT_INVALID);

/// @}

#endif    // __INCLUDE_API_PDS_NAT_HPP__
