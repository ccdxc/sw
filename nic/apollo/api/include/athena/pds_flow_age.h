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

#include "pds_base.h"

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
    uint32_t    others_tmo;             ///< All others established or not established
    uint32_t    session_tmo;            ///< Session table base timeout
} __PACK__ pds_flow_age_timeouts_t;

/// \brief handler to process an expired session or conntrack ID
typedef pds_ret_t (*pds_flow_expiry_fn_t)(uint32_t expiry_id,
                                          pds_flow_age_expiry_type_t expiry_type,
                                          void *user_ctx);

/// \brief     Initialize flow aging module
/// \return    #PDS_RET_OK on success, failure status code on error
/// \remark    This function implements one-time initialization of the flow aging
/// \remark    module. Calling the same function multiple times will have no
/// \remark    adverse effects and simply be treated as no-op.
pds_ret_t pds_flow_age_init(void);

/// \brief     Finalize flow aging module
/// \return    #PDS_RET_OK on success, failure status code on error
/// \remark    This function stops and quiesces all hardware scanners. Pollers
/// \remark    queues are also flushed.
void pds_flow_age_fini(void);

/// \brief     Start all instances of HW scanner
/// \return    #PDS_RET_OK on success, failure status code on error
/// \remark    Once PDS has created hardware scanners, the initialization process would
/// \remark    eventually and automatically activate these scanners so there is no need
/// \remark    for the user to invoke this API. However, if the user chose
/// \remark    to stop the scanners using pds_flow_age_hw_scanners_stop(), they can be
/// \remark    restarted at any time using pds_flow_age_hw_scanners_start().
pds_ret_t pds_flow_age_hw_scanners_start(void);

/// \brief     Stop all instances of HW scanner
/// \param[in] flag to request wait until all scanners have reached quiesce state
/// \return    #PDS_RET_OK on success, failure status code on error
/// \remark    This function disables the scheduling of all hardware scanners and returns
/// \remark    them to the idle state.
pds_ret_t pds_flow_age_hw_scanners_stop(bool quiesce_check);

/// \brief     Flush all software poller queues
/// \return    #PDS_RET_OK on success, failure status code on error
/// \remark    This function discards all pending "poller slot data" in all poller queues
/// \remark    and returns the queues to the empty state.
pds_ret_t pds_flow_age_sw_pollers_flush(void);

/// \brief     Return the number of poller queues configured
/// \param[out] ret_qcount: location to hold the returned queue count
/// \return    #PDS_RET_OK on success, failure status code on error
/// \remark    This function returns the number of poller queues.
pds_ret_t pds_flow_age_sw_pollers_qcount(uint32_t *ret_qcount);

/// \brief     Return the default PDS expiry handler
/// \param[out] ret_fn_dflt: location to hold the returned handler address
/// \return    #PDS_RET_OK on success, failure status code on error
/// \remark    This function returns the address of the default PDS
/// \remark    function that handles flow expiry.
pds_ret_t pds_flow_age_sw_pollers_expiry_fn_dflt(pds_flow_expiry_fn_t *ret_fn_dflt);

/// \brief     Set polling control
/// \param[in] user_will_poll: true if user will poll; false leaves all the polling
///            to PDS software (default)
/// \param[in] expiry_fn: the user may also supply a function to handle expired
///            entries. If NULL, PDS software will provide a representative
///            handler to delete the entry (NULL is the default).
/// \return    #PDS_RET_OK on success, failure status code on error
/// \remark    This function provides an indication to PDS software whether
/// \remark    the user will take responsibility for issuing calls to 
/// \remark    pds_flow_age_sw_pollers_poll() in its own threads, or leave
/// \remark    that responsibilty to PDS software. Regardless of the choice,
/// \remark    the user may supply a handler function to intercept and
/// \remark    process the expired flows. If no handler is given, PDS software
/// \remark    will provide a default handler to delete the expired flows.
pds_ret_t pds_flow_age_sw_pollers_poll_control(bool user_will_poll,
                                               pds_flow_expiry_fn_t expiry_fn);

/// \brief     Submit a poll request (a burst dequeue) on a poller queue
/// \param[in] poller_id: poller queue ID
/// \param[in] user_ctx: pointer to user context, if any
/// \return    #PDS_RET_OK on success, failure status code on error
/// \remark    This function should only be called if the user intended to take
/// \remark    responsibility for doing all polling in its own threads. The 
/// \remark    function will submit a burst dequeue on a poller queue
/// \remark    corresponding to poller_id. Any expired entries found will be
/// \remark    sent to the handler function previously registered with 
/// \remark    pds_flow_age_sw_pollers_poll_control(), or the default handler
/// \remark    if none were registered. The argument user_ctx will also be
/// \remark    passed on to the handler function.
pds_ret_t pds_flow_age_sw_pollers_poll(uint32_t poller_id,
                                       void *user_ctx);

/// \brief     Set normal inactivity timeout values for different types of flow
/// \param[in] normal timeouts specification
/// \return    #PDS_RET_OK on success, failure status code on error
/// \remark    Normal inactivity timeouts are assigned default values at initialization time and can
/// \remark    be changed at any time using this function (i.e., it is not necessary to
/// \remark    stop and restart the hardware scanners when modifying normal timeouts).
pds_ret_t pds_flow_age_normal_timeouts_set(const pds_flow_age_timeouts_t *norm_age_timeouts);

/// \brief     Return the currently configured set of normal timeout values
/// \param[out] location to hold the returned normal timeouts specification
/// \return    #PDS_RET_OK on success, failure status code on error
/// \remark    This function retrieves and returns the currently configured set
/// \remark    of normal timeout values.
pds_ret_t pds_flow_age_normal_timeouts_get(pds_flow_age_timeouts_t *norm_age_timeouts);

/// \brief     Set accelerated inactivity timeout values for different types of flow
/// \param[in] accelerated timeouts specification
/// \return    #PDS_RET_OK on success, failure status code on error
/// \remark    Similarly, accelerated inactivity timeouts are assigned default values at initialization
/// \remark    time and can be changed at any time using this function (i.e., it is not necessary to
/// \remark    stop and restart the hardware scanners when modifying accelerated timeouts).
pds_ret_t pds_flow_age_accel_timeouts_set(const pds_flow_age_timeouts_t *accel_age_timeouts);

/// \brief     Return the currently configured set of accelerated timeout values
/// \param[out] location to hold the returned accelerated timeouts specification
/// \return    #PDS_RET_OK on success, failure status code on error
/// \remark    This function retrieves and returns the currently configured set
/// \remark    of accelerated timeout values.
pds_ret_t pds_flow_age_accel_timeouts_get(pds_flow_age_timeouts_t *accel_age_timeouts);

/// \brief     Select or deselect accelerated aging
/// \param[in] True to select, false to deselect
/// \return    #PDS_RET_OK on success, failure status code on error
/// \remark    When selected, scanners will use the accelerated timeouts template for detecting
/// \remark    inactive flows. When deselected, scanners will use the normal timeouts template.
/// \remark    The default is false (deselect).
pds_ret_t pds_flow_age_accel_control(bool enable_sense);

/// @}

#ifdef __cplusplus
}
#endif

#endif  // __PDS_FLOW_AGE_H__
