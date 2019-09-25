/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    debug.cc
 *
 * @brief   This file has helper routines for troubleshooting the system
 */

#include <cerrno>
#include <sys/un.h>
#include <unordered_map>
#include "nic/sdk/linkmgr/port_mac.hpp"
#include "nic/sdk/include/sdk/fd.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/port.hpp"
#include "nic/apollo/api/debug.hpp"

#define FD_RECV_SOCKET_PATH             "/var/run/fd_recv_sock"

using std::unordered_map;

namespace debug {

debug::cmd_fd_db_t g_cmd_fd_map;

int
fd_get_from_cid (int64_t cid)
{
    debug::cmd_fd_db_t::const_iterator iterator;
    iterator = g_cmd_fd_map.find(cid);
    if (iterator != g_cmd_fd_map.end()) {
        return iterator->second;
    }
    return FD_INVALID;
}

void
cid_remove_from_fd_map (int64_t cid)
{
    g_cmd_fd_map.erase(cid);
}

void *
fd_recv_thread_start (void *ctxt)
{
    int ret;
    int sock_fd, sock_fd2, max_fd;
    fd_set master_set, active_set;
    struct sockaddr_un sock_addr;

    // initialize unix socket
    if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        PDS_TRACE_ERR("Failed to open unix domain socket for fd receive");
        return NULL;
    }

    memset(&sock_addr, 0, sizeof (sock_addr));
    sock_addr.sun_family = AF_UNIX;
    strcpy(sock_addr.sun_path, FD_RECV_SOCKET_PATH);

    if (bind(sock_fd, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) == -1) {
        PDS_TRACE_ERR ("Failed to bind unix domain socket for fd receive");
        return NULL;
    }
    if (listen(sock_fd, 1) == -1) {
        PDS_TRACE_ERR ("Failed to bind unix domain socket for fd receive");
        return NULL;
    }

    FD_ZERO(&master_set);
    FD_SET(sock_fd, &master_set);
    max_fd = sock_fd;

    while (1) {
        active_set = master_set;

        if ((ret = select(max_fd + 1, &active_set, NULL, NULL, NULL)) < 0) {
            if (errno == EAGAIN) {
                continue;
            } else {
                PDS_TRACE_ERR("Select failed");
                break;
            }
        }

        for (int i = 0; i <= max_fd && ret > 0; i++) {
            // check if FD is ready
            if (FD_ISSET(i, &active_set)) {
                ret--;
                if (i == sock_fd) {
                    // accept incoming connection
                    if ((sock_fd2 = accept(sock_fd, NULL, NULL)) == -1) {
                        PDS_TRACE_ERR("Accept failed");
                        continue;
                    }
                    FD_SET(sock_fd2, &master_set);
                    if (sock_fd2 > max_fd) {
                        max_fd = sock_fd2;
                    }
                } else {
                    int64_t cid;
                    int fd;

                    // read from existing connection
                    if (fd_recv(i, &fd, &cid)) {
                        PDS_TRACE_ERR("Receive fd failed");
                    } else {
                        // add fd to map
                        g_cmd_fd_map.insert(make_pair(cid, fd));
                        PDS_TRACE_DEBUG("Received file descriptor %d, CID %lx",
                                        fd, cid);
                    }
                    // close connection
                    close(i);
                    FD_CLR(i, &master_set);
                    if (i == max_fd) {
                        while (FD_ISSET (max_fd, &master_set) == FALSE) {
                            max_fd -= 1;
                        }
                    }
                }
            }
        }
    }
    return NULL;
}

/**
 * @brief        set clock frequency
 * @param[in]    freq clock frequency to be set
 * @return       #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_clock_frequency_update (pds_clock_freq_t freq)
{
    return impl_base::asic_impl()->set_frequency(freq);
}

/**
 * @brief        set arm clock frequency
 * @param[in]    freq clock frequency to be set
 * @return       #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_arm_clock_frequency_update (pds_clock_freq_t freq)
{
    return impl_base::asic_impl()->set_arm_frequency(freq);
}

/**
 * @brief        get system temperature
 * @param[out]   Temperate to be read
 * @return       #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_get_system_temperature (pds_system_temperature_t *temp)
{
    return impl_base::asic_impl()->get_system_temperature(temp);
}

/**
 * @brief        get system power
 * @param[out]   Power to be read
 * @return       #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_get_system_power (pds_system_power_t *pow)
{
    return impl_base::asic_impl()->get_system_power(pow);
}

/**
 * @brief       Gets API stats for different tables
 * @param[in]   CB to fill API stats & ctxt
 * @return      #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_table_stats_get (debug::table_stats_get_cb_t cb, void *ctxt)
{
    return impl_base::pipeline_impl()->table_stats(cb, ctxt);
}

/**
 * @brief       Gets API stats for different tables
 * @param[in]   ctxt  Context for CLI handler
 * @return      #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_mapping_dump (debug::cmd_ctxt_t *ctxt)
{
    return impl_base::pipeline_impl()->handle_cmd(ctxt);
}

/**
 * @brief       Setup LLC
 * @param[in]   llc_counters_t with type set
 * @return      #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_llc_setup (llc_counters_t *llc_args)
{
    return impl_base::asic_impl()->llc_setup(llc_args);
}

/**
 * @brief       LLC Stats Get
 * @param[out]  llc_counters_t to be filled
 * @return      #SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
pds_llc_get (llc_counters_t *llc_args)
{
    return impl_base::asic_impl()->llc_get(llc_args);
}

sdk_ret_t
pds_pb_stats_get (debug::pb_stats_get_cb_t cb, void *ctxt)
{
    return impl_base::asic_impl()->pb_stats(cb, ctxt);
}

sdk_ret_t
pds_meter_stats_get (debug::meter_stats_get_cb_t cb, uint32_t lowidx,
                     uint32_t highidx, void *ctxt)
{
    return impl_base::pipeline_impl()->meter_stats(cb, lowidx, highidx, ctxt);
}

sdk_ret_t
pds_session_stats_get (debug::session_stats_get_cb_t cb, uint32_t lowidx,
                       uint32_t highidx, void *ctxt)
{
    return impl_base::pipeline_impl()->session_stats(cb, lowidx, highidx, ctxt);
}

sdk_ret_t
pds_fte_api_stats_get (void)
{
    return SDK_RET_OK;
}

sdk_ret_t
pds_fte_table_stats_get (void)
{
    return SDK_RET_OK;
}

sdk_ret_t
pds_fte_api_stats_clear (void)
{
    return SDK_RET_OK;
}

sdk_ret_t
pds_fte_table_stats_clear (void)
{
    return SDK_RET_OK;
}

sdk_ret_t
pds_session_get (debug::session_get_cb_t cb, void *ctxt)
{
    return impl_base::pipeline_impl()->session(cb, ctxt);
}

sdk_ret_t
pds_flow_get (debug::flow_get_cb_t cb, void *ctxt)
{
    return impl_base::pipeline_impl()->flow(cb, ctxt);
}

sdk_ret_t
pds_session_clear (uint32_t idx)
{
    return impl_base::pipeline_impl()->session_clear(idx);
}

sdk_ret_t
pds_flow_clear (uint32_t idx)
{
    return impl_base::pipeline_impl()->flow_clear(idx);
}

}    // namespace debug
