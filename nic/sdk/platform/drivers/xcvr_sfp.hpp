// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __SDK_XCVR_SFP_HPP__
#define __SDK_XCVR_SFP_HPP__

namespace sdk {
namespace platform {

#define SFP_OFFSET_LENGTH_CU                 18
#define SFP_OFFSET_EXT_SPEC_COMPLIANCE_CODES 36

inline sdk_ret_t
sfp_sprom_parse (int port, uint8_t *data)
{
    // SFF 8472

    if (data[SFP_OFFSET_LENGTH_CU] != 0) {
        set_cable_type(port, cable_type_t::CABLE_TYPE_CU);
    } else {
        set_cable_type(port, cable_type_t::CABLE_TYPE_FIBER);
    }

    if (data[3] != 0) {
        if (data[3] & (1 << 4)) {
            // 10G Base SR
        } else if (data[3] & ( 1 << 5)) {
            // 10G Base LR
        } else if (data[3] & ( 1 << 6)) {
            // 10G Base LRM
        } else if (data[3] & ( 1 << 7)) {
            // 10G Base ER
        }
    }

    switch (data[SFP_OFFSET_EXT_SPEC_COMPLIANCE_CODES]) {
    case 0x1:
        // 25GAUI C2M AOC - BER 5x10^(-5)
        break;

    case 0x2:
        // 25GBASE-SR
        break;

    case 0x3:
        // 25GBASE-LR
        break;

    case 0x4:
        // 25GBASE-ER
        break;

    case 0x8:
        // 25GAUI C2M ACC - BER 5x10^(-5)
        break;

    case 0xb:
        // 25GBASE-CR CA-L
        break;

    case 0xc:
        // 25GBASE-CR CA-S
        break;

    case 0xd:
        // 25GBASE-CR CA-N
        break;

    case 0x16:
        // 10GBASE-T
        break;

    case 0x18:
        // 25GAUI C2M AOC - BER 10^(-12)
        break;

    case 0x19:
        // 25GAUI C2M ACC - BER 10^(-12)
        break;

    default:
        break;
    }

    return SDK_RET_OK;
}

} // namespace sdk
} // namespace platform

#endif  // __SDK_XCVR_SFP_HPP__
