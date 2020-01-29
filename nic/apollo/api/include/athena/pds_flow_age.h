//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file defines Flow Aging API
///
//----------------------------------------------------------------------------


#ifndef __PDS_FLOW_AGE_H__
#define __PDS_FLOW_AGE_H__

#include "nic/sdk/include/sdk/base.hpp"

#ifdef __cplusplus
extern "C" {
#endif

/// \defgroup PDS_FLOW_AGE
/// @{

/// \brief types of flow entry that is expiring
typedef enum pds_flow_age_expiry_type_e {
    EXPIRY_TYPE_SESSION,                ///< session table entry type
    EXPIRY_TYPE_CONNTRACK,              ///< connection tracking table entry type
} pds_flow_age_expiry_type_t;

/// \brief inactivity timeouts (in seconds) specification
typedef struct pds_flow_age_timeout_s {
    uint32_t    tcp_syn_tmo;            ///< TCP SYN sent/received
    uint32_t    tcp_est_tmo;            ///< TCP established
    uint32_t    tcp_fin_tmo;            ///< TCP FIN sent/received
    uint32_t    tcp_timewait_tmo;       ///< TCP in timewait state
    uint32_t    tcp_rst_tmo;            ///< TCP RST sent/received
    uint32_t    icmp_tmo;               ///< ICMP established or not established
    uint32_t    udp_tmo;                ///< UDP not established
    uint32_t    udp_est_tmo;            ///< UDP established
    uint32_t    other_tmo;              ///< All others established or not established
} __PACK__ pds_flow_age_timeouts_t;

/// \brief user directives supplied by user expiry callback
typedef struct pds_flow_age_user_directives_s {
    bool                del;            ///< TRUE to have the calling function delete this flow
    bool                del_rflow;      ///< TRUE to have the calling function also delete
                                        ///< the reverse flow, if any
} pds_flow_age_user_directives_t;

/// \brief     Start all instances of HW scanner
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    Once PDS has created hardware scanners, the initialization process would
/// \remark    eventually and automatically activate these scanners so there is no need
/// \remark    for the user to invoke this API. However, if the user chose
/// \remark    to stop the scanners using pds_flow_age_hw_scanners_stop(), they can be
/// \remark    restarted at any time using pds_flow_age_hw_scanners_start().
sdk_ret_t pds_flow_age_hw_scanners_start(void);

/// \brief     Stop all instances of HW scanner
/// \param[in] flag to request wait until all scanners have reached quiesce state
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    This function disables the scheduling of all hardware scanners and returns
/// \remark   them to the idle state.
sdk_ret_t pds_flow_age_hw_scanners_stop(bool quiesce_check);

/// \brief     Flush all software poller queues
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    This function discards all pending "poller slot data" in all poller queues
// \remark     and returns them to the empty state.
sdk_ret_t pds_flow_age_sw_pollers_flush(void);

/// \brief     Set normal inactivity timeout values for different types of flow
/// \param[in] normal timeouts specification
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    Normal inactivity timeouts are assigned default values at initialization time and can
/// \remark    be changed at any time using this function (i.e., it is not necessary to
/// \remark    stop and restart the hardware scanners when modifying normal timeouts).
sdk_ret_t pds_flow_age_normal_timeouts_set(const pds_flow_age_timeouts_t *norm_age_timeouts);

/// \brief     Set accelerated inactivity timeout values for different types of flow
/// \param[in] accelerated timeouts specification
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    Similarly, accelerated inactivity timeouts are assigned default values at initialization
/// \remark    time and can be changed at any time using this function (i.e., it is not necessary to
/// \remark    stop and restart the hardware scanners when modifying accelerated timeouts).
sdk_ret_t pds_flow_age_accel_timeouts_set(const pds_flow_age_timeouts_t *accel_age_timeouts);

/// \brief     Select or deselect accelerated aging
/// \param[in] True to select, false to deselect
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    When selected, scanners will use the accelerated timeouts template for detecting
/// \remark    inactive flows. When deselected, scanners will use the normal timeouts template.
/// \remark    The default is false (deselect).
sdk_ret_t pds_flow_age_accel_control(bool enable_sense);

/// \brief     Supply a flow expiry user callback function
/// \param[in] user callback function (NULL is allowed)
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    The user can supply a callback function to influence the decision as to whether or
/// \remark    not to delete a given expired flow. The user callback function will be called
/// \remark    from software poller threads for every session/conntrack ID found in their
/// \remark    corresponding queues. This means the call can be made from any lcores on which
/// \remark    poller threads are hosted.
sdk_ret_t pds_flow_age_expiry_user_callback_set(void (*user_cb)(uint32_t expiry_id,
                                                pds_flow_age_expiry_type_t expiry_type,
                                                const pds_flow_age_user_directives_t *user_directives));
/// @}

#ifdef __cplusplus
}
#endif

#endif  // __PDS_FLOW_AGE_H__
