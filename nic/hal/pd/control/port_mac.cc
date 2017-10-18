#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <atomic>
#include "nic/model_sim/include/lib_model_client.h"
#include "port_mac.hpp"

int mac_cfg (uint32_t port_num, uint32_t speed, uint32_t num_lanes)
{
    return lib_model_mac_msg_send(port_num, speed, BUFF_TYPE_MAC_CFG, num_lanes, 0);
}

int mac_enable (uint32_t port_num, uint32_t speed,
                uint32_t num_lanes, bool enable)
{
    if(enable == false) {
        return lib_model_mac_msg_send(port_num, speed, BUFF_TYPE_MAC_EN,
                num_lanes, 0);
    }

    return lib_model_mac_msg_send(port_num, speed, BUFF_TYPE_MAC_EN, num_lanes, 1);
}

int mac_soft_reset (uint32_t port_num, uint32_t speed,
                    uint32_t num_lanes, bool reset)
{
    if(reset == false) {
        return lib_model_mac_msg_send(port_num, speed, BUFF_TYPE_MAC_SOFT_RESET,
                num_lanes, 0);
    }

    return lib_model_mac_msg_send(port_num, speed, BUFF_TYPE_MAC_SOFT_RESET,
            num_lanes, 1);
}

int mac_stats_reset (uint32_t port_num, uint32_t speed,
                     uint32_t num_lanes, bool reset)
{
    if(reset == false) {
        return lib_model_mac_msg_send(
                port_num, speed, BUFF_TYPE_MAC_STATS_RESET, num_lanes, 0);
    }

    return lib_model_mac_msg_send(port_num, speed, BUFF_TYPE_MAC_STATS_RESET,
            num_lanes, 1);
}

int mac_intr_clear (uint32_t port_num, uint32_t speed, uint32_t num_lanes)
{
    return lib_model_mac_msg_send(port_num, speed, BUFF_TYPE_MAC_INTR_CLR,
            num_lanes, 0);
}

int mac_intr_enable (uint32_t port_num, uint32_t speed,
                     uint32_t num_lanes, bool enable)
{
    if(enable == false) {
        return lib_model_mac_msg_send(port_num, speed, BUFF_TYPE_MAC_INTR_EN,
                num_lanes, 0);
    }

    return lib_model_mac_msg_send(port_num, speed, BUFF_TYPE_MAC_INTR_EN,
            num_lanes, 1);
}

