#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <atomic>
#include "port.hpp"
#include "port_mac.hpp"
#include "nic/include/asic_pd.hpp"
#include "nic/model_sim/include/lib_model_client.h"

int mac_cfg_sim (uint32_t port_num, uint32_t speed, uint32_t num_lanes)
{
    return hal::pd::asic_port_cfg(port_num, speed, BUFF_TYPE_MAC_CFG, num_lanes, 0);
}

int mac_enable_sim (uint32_t port_num, uint32_t speed,
                uint32_t num_lanes, bool enable)
{
    if(enable == false) {
        return hal::pd::asic_port_cfg(port_num, speed, BUFF_TYPE_MAC_EN,
                num_lanes, 0);
    }

    return hal::pd::asic_port_cfg(port_num, speed, BUFF_TYPE_MAC_EN, num_lanes, 1);
}

int mac_soft_reset_sim (uint32_t port_num, uint32_t speed,
                    uint32_t num_lanes, bool reset)
{
    if(reset == false) {
        return hal::pd::asic_port_cfg(port_num, speed, BUFF_TYPE_MAC_SOFT_RESET,
                num_lanes, 0);
    }

    return hal::pd::asic_port_cfg(port_num, speed, BUFF_TYPE_MAC_SOFT_RESET,
            num_lanes, 1);
}

int mac_stats_reset_sim (uint32_t port_num, uint32_t speed,
                     uint32_t num_lanes, bool reset)
{
    if(reset == false) {
        return hal::pd::asic_port_cfg(
                port_num, speed, BUFF_TYPE_MAC_STATS_RESET, num_lanes, 0);
    }

    return hal::pd::asic_port_cfg(port_num, speed, BUFF_TYPE_MAC_STATS_RESET,
            num_lanes, 1);
}

int mac_intr_clear_sim (uint32_t port_num, uint32_t speed, uint32_t num_lanes)
{
    return hal::pd::asic_port_cfg(port_num, speed, BUFF_TYPE_MAC_INTR_CLR,
            num_lanes, 0);
}

int mac_intr_enable_sim (uint32_t port_num, uint32_t speed,
                     uint32_t num_lanes, bool enable)
{
    if(enable == false) {
        return hal::pd::asic_port_cfg(port_num, speed, BUFF_TYPE_MAC_INTR_EN,
                num_lanes, 0);
    }

    return hal::pd::asic_port_cfg(port_num, speed, BUFF_TYPE_MAC_INTR_EN,
            num_lanes, 1);
}

hal_ret_t
hal::pd::port::port_mac_init(bool is_sim)
{
    hal::pd::mac_fn_t *mac_fn = &hal::pd::port::mac_fn;

    if (is_sim == true) {
        mac_fn->mac_cfg         = &mac_cfg_sim;
        mac_fn->mac_enable      = &mac_enable_sim;
        mac_fn->mac_soft_reset  = &mac_soft_reset_sim;
        mac_fn->mac_stats_reset = &mac_stats_reset_sim;
        mac_fn->mac_intr_clear  = &mac_intr_clear_sim;
        mac_fn->mac_intr_enable = &mac_intr_enable_sim;
    }

    return HAL_RET_OK;
}

