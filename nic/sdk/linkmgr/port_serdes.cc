// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <atomic>
#include "port.hpp"
#include "linkmgr_rw.hpp"
#include "linkmgr_pd.hpp"

namespace sdk {
namespace linkmgr {
namespace pd {

//---------------------------------------------------------------------------
// HAPS platform methods
//---------------------------------------------------------------------------

bool
serdes_signal_detect_haps (uint32_t sbus_addr)
{
    return true;
}

bool
serdes_rdy_haps (uint32_t sbus_addr)
{
    return true;
}

int
serdes_output_enable_haps (uint32_t sbus_addr, bool enable)
{
    // sbus_addr repurposed as instance id for HAPS
    uint32_t chip = sbus_addr;
    uint64_t addr = MXP_BASE_HAPS +
                    (chip * MXP_INST_STRIDE_HAPS) +
                    PHY_RESET_OFFSET_HAPS;
    uint32_t data = 0x0;

    if (enable == false) {
        data = 0x1;
    }

    // for HAPS, serdes enable/disable is un-reset/reset
    WRITE_REG_BASE(chip, addr, data);

    return SDK_RET_OK;
}

int
serdes_cfg_haps (uint32_t sbus_addr)
{
    // for HAPS, serdes cfg is to un-reset serdes
    serdes_output_enable_haps (sbus_addr, true);
    return SDK_RET_OK;
}

int
serdes_tx_rx_enable_haps (uint32_t sbus_addr, bool enable)
{
    return SDK_RET_OK;
}

int
serdes_reset_haps (uint32_t sbus_addr, bool reset)
{
    return SDK_RET_OK;
}

//---------------------------------------------------------------------------
// default methods
//---------------------------------------------------------------------------

int
serdes_cfg_default (uint32_t sbus_addr)
{
    return SDK_RET_OK;
}

bool
serdes_signal_detect_default (uint32_t sbus_addr)
{
    return true;
}

bool
serdes_rdy_default (uint32_t sbus_addr)
{
    return true;
}

int
serdes_output_enable_default (uint32_t sbus_addr, bool enable)
{
    return SDK_RET_OK;
}

int
serdes_tx_rx_enable_default (uint32_t sbus_addr, bool enable)
{
    return SDK_RET_OK;
}

int
serdes_reset_default (uint32_t sbus_addr, bool reset)
{
    return SDK_RET_OK;
}

sdk_ret_t
linkmgr::pd::port::port_serdes_fn_init(linkmgr_cfg_t *cfg)
{
    linkmgr::pd::serdes_fn_t *serdes_fn = &linkmgr::pd::port::serdes_fn;
    platform_type_t platform_type = cfg->platform_type;

    serdes_fn->serdes_cfg = &serdes_cfg_default;
    serdes_fn->serdes_signal_detect = &serdes_signal_detect_default;
    serdes_fn->serdes_rdy = &serdes_rdy_default;
    serdes_fn->serdes_output_enable = &serdes_output_enable_default;
    serdes_fn->serdes_tx_rx_enable = &serdes_tx_rx_enable_default;
    serdes_fn->serdes_reset = &serdes_reset_default;

    switch (platform_type) {
    case platform_type_t::PLATFORM_TYPE_HAPS:
        serdes_fn->serdes_cfg = &serdes_cfg_haps;
        serdes_fn->serdes_signal_detect = &serdes_signal_detect_haps;
        serdes_fn->serdes_rdy = &serdes_rdy_haps;
        serdes_fn->serdes_output_enable = &serdes_output_enable_haps;
        serdes_fn->serdes_tx_rx_enable = &serdes_tx_rx_enable_haps;
        serdes_fn->serdes_reset = &serdes_reset_haps;
        break;

    default:
        break;
    }

    return SDK_RET_OK;
}

}    // namespace pd
}    // namespace linkmgr
}    // namespace sdk
