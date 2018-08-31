// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "linkmgr.hpp"
#include "linkmgr_rw.hpp"
#include "port_serdes.hpp"
#include "linkmgr_types.hpp"
#include "linkmgr_internal.hpp"
#include "include/sdk/asic/capri/cap_mx_api.h"
#include "third-party/avago/include/aapl/aapl.h"

namespace sdk {
namespace linkmgr {

// global aapl info
Aapl_t *aapl = NULL;

#define SPICO_INT_ENABLE 0x1
#define SPICO_INT_RESET  0x39

//---------------------------------------------------------------------------
// HAPS platform methods
//---------------------------------------------------------------------------

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
serdes_sbus_reset_default (uint32_t sbus_addr, int hard)
{
    return SDK_RET_OK;
}

int
serdes_spico_reset_default (uint32_t sbus_addr)
{
    return SDK_RET_OK;
}

int
serdes_eye_get_default(uint32_t sbus_addr, int eye_type)
{
    return 0;
}

int
serdes_ical_start_default(uint32_t sbus_addr)
{
    return 0;
}

int
serdes_pcal_start_default(uint32_t sbus_addr)
{
    return 0;
}

int
serdes_pcal_continuous_start_default(uint32_t sbus_addr)
{
    return 0;
}

int
serdes_dfe_status_default(uint32_t sbus_addr)
{
    return 1;
}

int
serdes_rx_lpbk_default(uint32_t sbus_addr, bool enable)
{
    return 0;
}

int
serdes_spico_status_default(uint32_t sbus_addr)
{
    return 0;
}

int
serdes_get_rev_default(uint32_t sbus_addr)
{
    return 0;
}

uint32_t
serdes_get_eng_id_default(uint32_t sbus_addr)
{
    return 0;
}

int
serdes_get_build_id_default(uint32_t sbus_addr)
{
    return 0;
}

bool
serdes_spico_crc_default(uint32_t sbus_addr)
{
    return true;
}

int
serdes_spico_upload_default (uint32_t sbus_addr, const char* file_name)
{
    return 0;
}

//---------------------------------------------------------------------------
// HW methods
//---------------------------------------------------------------------------

uint32_t
sbus_access (uint32_t sbus_addr,
             unsigned char reg_addr,
             unsigned char command,
             uint *sbus_data)
{
    uint32_t chip_id     = 0;
    uint32_t status      = 0;
    uint32_t ring_number = ((sbus_addr >> 8 ) & 0x0f); 
    uint32_t sbus_id     = (sbus_addr         & 0xff);

    //Required commands are:
    //1: write
    //2: read
    //0: reset
    //sbus_data: Pointer to the SBus data to write. Results of SBus read operations will be placed here.

    switch (command) {
    case 0x1:   // sbus write
        SDK_TRACE_DEBUG("sbus_write."
                " sbus_addr 0x%x, reg_addr: 0x%x, cmd: 0x%x, sbus_data: 0x%x,"
                " ring: 0x%x, sbus_id: 0x%x",
                sbus_addr, reg_addr, command, *sbus_data, ring_number, sbus_id);

        if(ring_number == 0) {
            SDK_TRACE_ERR("NO-OP for ring: %d", ring_number);
            // cap_pp_sbus_write(chip_id, sbus_id, reg_addr, *sbus_data);
        } else {
            cap_ms_sbus_write(chip_id, sbus_id, reg_addr, *sbus_data);
        }
        status = 1;
        break;

    case 0x2:   // sbus read
        if(ring_number == 0) {
            SDK_TRACE_ERR("NO-OP for ring: %d", ring_number);
            // *sbus_data = cap_pp_sbus_read(chip_id, sbus_id, reg_addr);
        } else {
            *sbus_data = cap_ms_sbus_read(chip_id, sbus_id, reg_addr);
        }

        SDK_TRACE_DEBUG("sbus_read."
                " sbus_addr 0x%x, reg_addr: 0x%x, cmd: 0x%x, sbus_data: 0x%x,"
                " ring: 0x%x, sbus_id: 0x%x, sbus:data: 0x%x",
                sbus_addr, reg_addr, command, *sbus_data, ring_number, sbus_id,
                *sbus_data);

        status = 1;
        break;

    case 0x0:   // sbus reset
        SDK_TRACE_DEBUG("sbus_reset."
                " sbus_addr 0x%x, reg_addr: 0x%x, cmd: 0x%x, sbus_data: 0x%x,"
                " ring: 0x%x, sbus_id: 0x%x",
                sbus_addr, reg_addr, command, *sbus_data, ring_number, sbus_id);

        if(ring_number == 0) {
            SDK_TRACE_ERR("NO-OP for ring: %d", ring_number);
            // cap_pp_sbus_reset(chip_id, sbus_id);
        } else {
            // cap_ms_sbus_reset(chip_id, sbus_id);
        }
        status = 1;
        break;

    default:
        SDK_TRACE_DEBUG("Invalid cmd."
                " sbus_addr 0x%x, reg_addr: 0x%x, cmd: 0x%x, sbus_data: 0x%x,"
                " ring: 0x%x, sbus_id: 0x%x",
                sbus_addr, reg_addr, command, *sbus_data, ring_number, sbus_id);

        status = 0;
        break;
    }

    return status;
}

uint32_t
aapl_sbus_access (Aapl_t *aapl,
                  uint32_t sbus_addr,
                  unsigned char reg_addr,
                  unsigned char command,
                  uint *sbus_data)
{
    return sbus_access(sbus_addr, reg_addr, command, sbus_data);
}

uint32_t spico_int (Aapl_t *aapl, uint32_t addr, int int_code, int int_data)
{
    return 0;
}

void
serdes_get_ip_info(int chip_reset)
{
    if (aapl != NULL) {
        /* Gather information about the device and place into AAPL struct */
        aapl_get_ip_info(aapl, chip_reset);
    }
}

Aapl_t*
serdes_global_init_hw(uint32_t     jtag_id,
                      int          num_sbus_rings,
                      bool         aacs_server_en,
                      bool         aacs_connect,
                      int          port,
                      std::string& ip)
{
    Aapl_comm_method_t comm_method = AVAGO_SBUS;

    // TODO CLI
    int         verbose        = 1;
    int         debug          = 8;

    // TODO read from catalog
    int      num_chips      = 1;

    (void)aacs_server_en;

    Aapl_t *aapl = aapl_construct();

    // set the appl init params
    aapl->communication_method = comm_method;
    aapl->jtag_idcode[0]       = jtag_id;
    aapl->sbus_rings           = num_sbus_rings;
    aapl->chips                = num_chips;
    aapl->debug                = debug;
    aapl->verbose              = verbose;

    if (aacs_connect == true) {
        aapl->aacs  = aacs_connect;
    } else {
        // register access methods
        aapl_register_sbus_fn(aapl, aapl_sbus_access, NULL, NULL);
        // aapl_register_spico_int_fn(aapl, spico_int);
    }

    // Make a connection to the device
    aapl_connect(aapl, ip.c_str(), port);

    if(aapl->return_code < 0) {
        aapl_destruct(aapl);
        return NULL;
    }

    return aapl;
}

bool
serdes_signal_detect_hw (uint32_t sbus_addr)
{
    return true;
}

bool
serdes_rdy_hw (uint32_t sbus_addr)
{
    int tx_rdy = 0;
    int rx_rdy = 0;

    avago_serdes_get_tx_rx_ready(aapl, sbus_addr, &tx_rdy, &rx_rdy);

    if (tx_rdy == 0 || rx_rdy == 0) {
        return false;
    }

    return true;
}

int
serdes_output_enable_hw (uint32_t sbus_addr, bool enable)
{
    if (avago_serdes_set_tx_output_enable(aapl, sbus_addr, enable) == -1) {
        return SDK_RET_ERR;
    }

    return SDK_RET_OK;
}

int
serdes_cfg_hw (uint32_t sbus_addr, serdes_info_t *serdes_info)
{
    uint32_t divider = serdes_info->sbus_divider;
    uint32_t width   = serdes_info->width;
    uint32_t tx_slip = serdes_info->tx_slip_value;
    uint32_t rx_slip = serdes_info->rx_slip_value;

    SDK_TRACE_DEBUG("sbus_addr: %u, divider: %u, width: %u",
                    sbus_addr, divider, width);

    Avago_serdes_init_config_t *cfg = avago_serdes_init_config_construct(aapl);
    if (NULL == cfg) {
        SDK_TRACE_ERR("Failed to construct avago config");
        return SDK_RET_ERR;
    }

    cfg->init_mode = Avago_serdes_init_mode_t::AVAGO_INIT_ONLY;

    // divider and width
    cfg->tx_divider = divider;
    cfg->rx_divider = divider;
    cfg->tx_width   = width;
    cfg->rx_width   = width;

    // Disable signal_ok and tx_output
    cfg->signal_ok_en = 0;
    cfg->tx_output_en = 0;

    // Tx/Rx enable
    cfg->init_tx = 1;
    cfg->init_rx = 1;

    // resets
    cfg->sbus_reset  = 1;
    cfg->spico_reset = 1;

    // reset the earlier error code
    aapl->return_code = 0;

    avago_serdes_init(aapl, sbus_addr, cfg);

    if(aapl->return_code) {
        SDK_TRACE_ERR("Failed to initialize SerDes\n");
    }

    if (tx_slip != 0) {
        // Tx slip value
        avago_spico_int_check(aapl, __func__, __LINE__,
                              sbus_addr, 0xd, tx_slip);
    }

    if (rx_slip != 0) {
        // Rx slip value
        avago_spico_int_check(aapl, __func__, __LINE__,
                              sbus_addr, 0xe, rx_slip);
    }

    avago_serdes_init_config_destruct(aapl, cfg);

    return SDK_RET_OK;
}

int
serdes_tx_rx_enable_hw (uint32_t sbus_addr, bool enable)
{
    // To be set only during init stage.
    // Need to wait for Tx/Rx ready once set
    return SDK_RET_OK;

#if 0
    int  mask     = 0;
    bool rc       = false;
    int  int_code = SPICO_INT_ENABLE;

    mask = serdes_get_int01_bits(aapl, sbus_addr, ~0x3) | (enable ? 0x3 : 0x0);

    rc = avago_spico_int_check(aapl, __func__, __LINE__, sbus_addr, int_code, mask);
    return rc ? SDK_RET_OK : SDK_RET_ERR;
#endif
}

int
serdes_sbus_reset_hw (uint32_t sbus_addr, int hard)
{
    avago_sbus_reset(aapl, sbus_addr, hard);

    return 0;

#if 0
    int  mask     = 0;
    bool rc       = false;
    int  int_code = SPICO_INT_RESET;

    if (reset == true) {
        mask = 1;
    }

    rc = avago_spico_int_check(aapl, __func__, __LINE__, sbus_addr, int_code, mask);
    return rc ? SDK_RET_OK : SDK_RET_ERR;
#endif
}

int
serdes_spico_reset_hw (uint32_t sbus_addr)
{
    int rc = avago_spico_reset (aapl, sbus_addr);

    if (rc < 0) {
        SDK_TRACE_ERR("spico reset failed for sbus: %u", sbus_addr);
    }

    return rc;

#if 0
    int  mask     = 0;
    bool rc       = false;
    int  int_code = SPICO_INT_RESET;

    if (reset == true) {
        mask = 1;
    }

    rc = avago_spico_int_check(aapl, __func__, __LINE__, sbus_addr, int_code, mask);
    return rc ? SDK_RET_OK : SDK_RET_ERR;
#endif
}

int
serdes_spico_upload_hw (uint32_t sbus_addr, const char* filename)
{
    int rc = avago_spico_upload_file(aapl, sbus_addr, 0, filename);

    if (rc < 0) {
        SDK_TRACE_ERR("spico upload failed for sbus: %u", sbus_addr);
    }

    return rc;
}

int
serdes_ical_start_hw(uint32_t sbus_addr)
{
    Avago_serdes_dfe_tune_t dfe;

    avago_serdes_tune_init(aapl, &dfe);
    dfe.tune_mode = Avago_serdes_dfe_tune_mode_t::AVAGO_DFE_ICAL;

    avago_serdes_tune(aapl, sbus_addr, &dfe);

    return 0;
}

int
serdes_pcal_start_hw(uint32_t sbus_addr)
{
    Avago_serdes_dfe_tune_t dfe;

    avago_serdes_tune_init(aapl, &dfe);
    dfe.tune_mode = Avago_serdes_dfe_tune_mode_t::AVAGO_DFE_PCAL;

    avago_serdes_tune(aapl, sbus_addr, &dfe);

    return 0;
}

int
serdes_pcal_continuous_start_hw(uint32_t sbus_addr)
{
    Avago_serdes_dfe_tune_t dfe;

    avago_serdes_tune_init(aapl, &dfe);
    dfe.tune_mode = Avago_serdes_dfe_tune_mode_t::AVAGO_DFE_START_ADAPTIVE;

    avago_serdes_tune(aapl, sbus_addr, &dfe);

    return 0;
}

int
serdes_dfe_status_hw(uint32_t sbus_addr)
{
    return avago_serdes_dfe_wait_timeout(aapl, sbus_addr, 0);
}

int
serdes_eye_get_hw(uint32_t sbus_addr, int eye_type)
{
    Avago_serdes_eye_config_t *cfg =
                                avago_serdes_eye_config_construct(aapl);

    Avago_serdes_eye_data_t *edata = avago_serdes_eye_data_construct(aapl);

    switch (eye_type) {
        case 0:
            cfg->ec_eye_type = AVAGO_EYE_SIZE;
            break;

        case 1:
            cfg->ec_eye_type = AVAGO_EYE_FULL;
            break;

        default:
            cfg->ec_eye_type = AVAGO_EYE_HEIGHT_DVOS;
            break;
    }

    cfg->ec_min_dwell_bits = 0x1e6;
    cfg->ec_max_dwell_bits = 0x1e8;
    cfg->ec_x_resolution   = 64;
    cfg->ec_y_points       = 512;

    // cfg->ec_cmp_mode       = AVAGO_SERDES_RX_CMP_MODE_XOR;
    // cfg->ec_y_step_size = 1;

    avago_serdes_eye_get(aapl, sbus_addr, cfg, edata);

    switch (eye_type) {
        case 0:
            SDK_TRACE_DEBUG ("Eye width (in real PI steps): %d\n"
                             "Eye height (in DAC steps):    %d\n"
                             "Eye width (in mUI):           %d\n"
                             "Eye height (in mV):           %d",
                             edata->ed_width,
                             edata->ed_height,
                             edata->ed_width_mUI,
                             edata->ed_height_mV);
            break;

        case 1:
            avago_serdes_eye_plot_write(stdout, edata);
            break;

        default:
            avago_serdes_eye_vbtc_log_print(aapl, AVAGO_INFO,
                                    __func__, __LINE__, &(edata->ed_vbtc[0]));
            avago_serdes_eye_hbtc_log_print(aapl, AVAGO_INFO,
                                    __func__, __LINE__, &(edata->ed_hbtc[0]));
            break;
    }

    avago_serdes_eye_data_destruct(aapl, edata);
    avago_serdes_eye_config_destruct(aapl, cfg);

    return 0;
}

int
serdes_rx_lpbk_hw(uint32_t sbus_addr, bool enable)
{
    avago_serdes_set_rx_input_loopback(aapl, sbus_addr, enable);
    return 0;
}

int
serdes_spico_status_hw(uint32_t sbus_addr)
{
    Avago_spico_status_t state;
    memset(&state, 0, sizeof(Avago_spico_status_t));

    avago_spico_status(aapl, sbus_addr, false /*no cache*/, &state);

    SDK_TRACE_DEBUG("rev: 0x%x, build: 0x%x, pc: 0x%x, en: %d, state: %d,"
                    " clk: %d",
                    state.revision, state.build, state.pc, state.enabled,
                    state.state, state.clk);
    return 0;
}

int
serdes_get_rev_hw(uint32_t sbus_addr)
{
    return aapl_get_firmware_rev(aapl, sbus_addr);
}

uint32_t
serdes_get_eng_id_hw(uint32_t sbus_addr)
{
    return avago_firmware_get_engineering_id(aapl, sbus_addr);
}

int
serdes_get_build_id_hw(uint32_t sbus_addr)
{
    return aapl_get_firmware_build(aapl, sbus_addr);
}

bool
serdes_spico_crc_hw(uint32_t sbus_addr)
{
    return avago_spico_crc(aapl, sbus_addr);
}

sdk_ret_t
port_serdes_fn_init(platform_type_t platform_type,
                    uint32_t        jtag_id,
                    int             num_sbus_rings,
                    bool            aacs_server_en,
                    bool            aacs_connect,
                    int             port,
                    std::string     ip)
{
    serdes_fn_t        *serdes_fn = &serdes_fns;

    serdes_fn->serdes_cfg           = &serdes_cfg_default;
    serdes_fn->serdes_signal_detect = &serdes_signal_detect_default;
    serdes_fn->serdes_rdy           = &serdes_rdy_default;
    serdes_fn->serdes_output_enable = &serdes_output_enable_default;
    serdes_fn->serdes_tx_rx_enable  = &serdes_tx_rx_enable_default;
    serdes_fn->serdes_sbus_reset    = &serdes_sbus_reset_default;
    serdes_fn->serdes_spico_reset   = &serdes_spico_reset_default;
    serdes_fn->serdes_eye_get       = &serdes_eye_get_default;
    serdes_fn->serdes_ical_start    = &serdes_ical_start_default;
    serdes_fn->serdes_pcal_start    = &serdes_pcal_start_default;
    serdes_fn->serdes_pcal_continuous_start =
                            &serdes_pcal_continuous_start_default;
    serdes_fn->serdes_dfe_status    = &serdes_dfe_status_default;
    serdes_fn->serdes_rx_lpbk       = &serdes_rx_lpbk_default;
    serdes_fn->serdes_spico_upload  = &serdes_spico_upload_default;
    serdes_fn->serdes_spico_status  = &serdes_spico_status_default;
    serdes_fn->serdes_get_rev       = &serdes_get_rev_default;
    serdes_fn->serdes_get_eng_id    = &serdes_get_eng_id_default;
    serdes_fn->serdes_get_build_id  = &serdes_get_build_id_default;
    serdes_fn->serdes_spico_crc     = &serdes_spico_crc_default;

    switch (platform_type) {
    case platform_type_t::PLATFORM_TYPE_HW:
        serdes_fn->serdes_cfg           = &serdes_cfg_hw;
        serdes_fn->serdes_signal_detect = &serdes_signal_detect_hw;
        serdes_fn->serdes_rdy           = &serdes_rdy_hw;
        serdes_fn->serdes_output_enable = &serdes_output_enable_hw;
        serdes_fn->serdes_tx_rx_enable  = &serdes_tx_rx_enable_hw;
        serdes_fn->serdes_sbus_reset    = &serdes_sbus_reset_hw;
        serdes_fn->serdes_spico_reset   = &serdes_spico_reset_hw;
        serdes_fn->serdes_eye_get       = &serdes_eye_get_hw;
        serdes_fn->serdes_ical_start    = &serdes_ical_start_hw;
        serdes_fn->serdes_pcal_start    = &serdes_pcal_start_hw;
        serdes_fn->serdes_pcal_continuous_start =
                                &serdes_pcal_continuous_start_hw;
        serdes_fn->serdes_dfe_status    = &serdes_dfe_status_hw;
        serdes_fn->serdes_rx_lpbk       = &serdes_rx_lpbk_hw;
        serdes_fn->serdes_spico_upload  = &serdes_spico_upload_hw;
        serdes_fn->serdes_spico_status  = &serdes_spico_status_hw;
        serdes_fn->serdes_get_rev       = &serdes_get_rev_hw;
        serdes_fn->serdes_get_eng_id    = &serdes_get_eng_id_hw;
        serdes_fn->serdes_get_build_id  = &serdes_get_build_id_hw;
        serdes_fn->serdes_spico_crc     = &serdes_spico_crc_hw;

        // serdes global init
        aapl = serdes_global_init_hw(jtag_id, num_sbus_rings,
                                     aacs_server_en, aacs_connect, port, ip);
        break;

    default:
        break;
    }

    return SDK_RET_OK;
}

}    // namespace linkmgr
}    // namespace sdk
