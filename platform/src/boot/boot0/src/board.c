
/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include "dtls.h"
#include "cap_board.h"
#include "cap_cpld.h"
#include "defs.h"
#include "cap_ms_c_hdr.h"
#include "iomem.h"

// Mapping of [track][attempt] to fwid
static const bsm_fwid_map_t std_bsm_fwid_map = {
    .map = {
        [FW_MAIN_A] = { FW_MAIN_A, FW_MAIN_B, FW_GOLD,   FW_GOLD   },
        [FW_MAIN_B] = { FW_MAIN_B, FW_MAIN_A, FW_GOLD,   FW_GOLD   },
        [FW_GOLD]   = { FW_GOLD,   FW_GOLD,   FW_GOLD,   FW_GOLD   },
        [FW_DIAG]   = { FW_DIAG,   FW_DIAG,   FW_GOLD,   FW_GOLD   },
    }
};

// Flash partitions.
struct part_map_s {
    const char *name;
    uint32_t addr;
    uint32_t size;
};
static const struct part_map_s std_parts[] = {
    { "ubootb",     0x74600000, 0x00400000 },
    { "uboota",     0x74200000, 0x00400000 },
    { "bootlog",    0x74090000, 0x00020000 },
    { "fwsel",      0x74030000, 0x00020000 },
    { "golduboot",  0x70180000, 0x00200000 },
    { NULL }
};

// Board info.
struct board_info {
    uint8_t     chip_type;              // key: chip type to match
    uint8_t     cpld_id;                // key: cpld id to match
    uint8_t     qspi_read_delay;        // qspi read delay
    uint8_t     bsm_wdt_disable;        // do not use the wdt
    uint8_t     reset_on_panic;         // reset system upon panic
    uint16_t    log2_bfl_secsize;       // boot fault log sector size
    uint32_t    qspi_frequency;         // qspi frequency
    const bsm_fwid_map_t *fwid_map;     // bsm fwid map
    const struct part_map_s *parts;     // qspi partitions
};
// Non-boolean field defaults:
#define BRD_DEF_LOG2_BFL_SECSIZE        16
#define BRD_DEF_QSPI_FREQUENCY          50000000
#define BRD_DEF_FWID_MAP                &std_bsm_fwid_map
#define BRD_DEF_PARTS                   std_parts

static const struct board_info brd[BOARD_TYPE_NUM] = {
    [BOARD_TYPE_FORIO] = {
        .chip_type          = CHIP_TYPE_ASIC,
        .cpld_id            = CPLD_ID_FORIO,
        .qspi_read_delay    = 1,
        .qspi_frequency     = 40000000,
    },
    [BOARD_TYPE_VOMERO] = {
        .chip_type          = CHIP_TYPE_ASIC,
        .cpld_id            = CPLD_ID_VOMERO,
        .qspi_read_delay    = 1,
        .qspi_frequency     = 40000000,
    }
};

int
get_chip_type(void)
{
    uint32_t sta_ver;

    sta_ver = readreg(MS_(STA_VER));
    return CAP_MS_CSR_STA_VER_CHIP_TYPE_GET(sta_ver);
}

int
get_cpld_id(void)
{
    static int cpld_id = -1;
    int chip_type;

    if (cpld_id < 0) {
        chip_type = get_chip_type();
        cpld_id = (chip_type == CHIP_TYPE_ASIC) ? cpld_read(CPLD_ID) : 0;
    }
    return cpld_id;
}

static int
init_board_type(void)
{
    int chip_type, cpld_id, i, board_type;

    chip_type = get_chip_type();
    cpld_id = get_cpld_id();
    board_type = BOARD_TYPE_UNKNOWN;
    for (i = 0; i < ARRAY_SIZE(brd); i++) {
        if (brd[i].chip_type == chip_type && brd[i].cpld_id == cpld_id) {
            board_type = i;
            break;
        }
    }
    return board_type;
}

int
cap_board_type(void)
{
    static int b_type = BOARD_TYPE_NONE;
    if (b_type == BOARD_TYPE_NONE) {
        b_type = init_board_type();
    }
    return b_type;
}

static const struct board_info *
board_info_ptr(void)
{
    return &brd[cap_board_type()];
}

int
board_get_bfl_log2_secsize(void)
{
    uint32_t val = board_info_ptr()->log2_bfl_secsize;
    return val ?: BRD_DEF_LOG2_BFL_SECSIZE;
}

uint32_t
board_qspi_frequency(void)
{
    uint32_t val = board_info_ptr()->qspi_frequency;
    return val ?: BRD_DEF_QSPI_FREQUENCY;
}

uint8_t
board_qspi_read_delay(void)
{
    return board_info_ptr()->qspi_read_delay;
}

uint8_t
board_bsm_wdt_disable(void)
{
    return board_info_ptr()->bsm_wdt_disable;
}

const bsm_fwid_map_t *
board_bsm_fwid_map(void)
{
    const bsm_fwid_map_t *mp = board_info_ptr()->fwid_map;
    return mp ?: BRD_DEF_FWID_MAP;
}

uint8_t
board_reset_on_panic(void)
{
    return board_info_ptr()->reset_on_panic;
}

int
board_get_part(const char *name, intptr_t *addrp, uint32_t *sizep)
{
    const struct board_info *p = board_info_ptr();
    const struct part_map_s *parts;
    int i;

    parts = p->parts ?: BRD_DEF_PARTS;
    for (i = 0; parts[i].name != NULL; i++) {
        if (strcmp(name, parts[i].name) == 0) {
            if (addrp) *addrp = parts[i].addr;
            if (sizep) *sizep = parts[i].size;
            return 0;
        }
    }
    return -1;
}
