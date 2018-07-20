// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <atomic>
#include "port.hpp"
#include "linkmgr_rw.hpp"
#include "linkmgr.hpp"
#include "aapl.h"

namespace sdk {
namespace linkmgr {

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
serdes_cfg_haps (uint32_t sbus_addr, serdes_info_t *serdes_info)
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
serdes_cfg_default (uint32_t sbus_addr, serdes_info_t *serdes_info)
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

//---------------------------------------------------------------------------
// HW methods
//---------------------------------------------------------------------------

bool
serdes_signal_detect_hw (uint32_t sbus_addr)
{
    return true;
}

bool
serdes_rdy_hw (uint32_t sbus_addr)
{
    return true;
}

int
serdes_output_enable_hw (uint32_t sbus_addr, bool enable)
{
    return SDK_RET_OK;
}

uint32_t sbus_access (Aapl_t *aapl, uint32_t addr, unsigned char reg_addr,
                      unsigned char command, uint *sbus_data)
{
    return 0;
}

uint32_t spico_int (Aapl_t *aapl, uint32_t addr, int int_code, int int_data)
{
    return 0;
}

int
serdes_cfg_hw (uint32_t sbus_addr, serdes_info_t *serdes_info)
{
    Aapl_comm_method_t comm_method = AVAGO_OFFLINE;

    uint32_t   divider      = serdes_info->sbus_divider;
    uint32_t   debug        = 1;   /* default debug level */
    Aapl_t     *aapl         = aapl_construct();

    aapl->communication_method = comm_method;
    aapl->debug = debug;

    aapl_register_sbus_fn(aapl, sbus_access, NULL, NULL);

    aapl_register_spico_int_fn(aapl, spico_int);

    // Make a connection to the device
    aapl_connect(aapl, NULL, 0); if( aapl->return_code < 0 ) AAPL_EXIT(1);

    /* Gather information about the device and place into AAPL struct */
    aapl_get_ip_info(aapl,1);                 if( aapl->return_code < 0 ) AAPL_EXIT(1);

    /* Initialize serdes (requires that firmware has already been loaded) */
    avago_serdes_init_quick(aapl, sbus_addr, divider);

    if( aapl->return_code ) SDK_TRACE_ERR("ERROR: Failed to initialize SerDes\n");

    aapl_destruct(aapl);

    return SDK_RET_OK;
}

int
serdes_tx_rx_enable_hw (uint32_t sbus_addr, bool enable)
{
    return SDK_RET_OK;
}

int
serdes_reset_hw (uint32_t sbus_addr, bool reset)
{
    return SDK_RET_OK;
}

sdk_ret_t
port::port_serdes_fn_init(linkmgr_cfg_t *cfg)
{
    serdes_fn_t        *serdes_fn = &port::serdes_fn;
    platform_type_t    platform_type = cfg->platform_type;

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

    case platform_type_t::PLATFORM_TYPE_HW:
    case platform_type_t::PLATFORM_TYPE_MOCK:
        serdes_fn->serdes_cfg = &serdes_cfg_hw;
        serdes_fn->serdes_signal_detect = &serdes_signal_detect_hw;
        serdes_fn->serdes_rdy = &serdes_rdy_hw;
        serdes_fn->serdes_output_enable = &serdes_output_enable_hw;
        serdes_fn->serdes_tx_rx_enable = &serdes_tx_rx_enable_hw;
        serdes_fn->serdes_reset = &serdes_reset_hw;
        break;

    default:
        break;
    }

    return SDK_RET_OK;
}

}    // namespace linkmgr
}    // namespace sdk
