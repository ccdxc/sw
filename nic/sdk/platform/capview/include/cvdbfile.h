
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __CAPVIEWDBFILE_H__
#define __CAPVIEWDBFILE_H__

struct cvdb_sect {
    uint32_t offs;
    uint32_t size;
};
struct cvdb_hdr {
    uint32_t magic;
    uint16_t build;
    uint16_t ver;
    uint32_t ver_reg_idx;
    struct cvdb_sect sect[4];
};
#define CVDB_MAGIC      0xcaf1e1db

enum {
    SECT_REG_TAB, SECT_FIELD_TAB, SECT_REG_STRTAB, SECT_FIELD_STRTAB
};
struct cvdb_field {
    uint16_t hi;
    uint16_t lo;
    uint32_t name;      // name index from field_strtab
};
struct cvdb_field_tab {
    uint32_t nfields;
    struct cvdb_field field[0];
};
struct cvdb_reg {
    uint32_t addr;      // address
    uint32_t nrows;     // table row count
    uint32_t name;      // name index from reg_strtab
    uint32_t fldoffs;   // field index from field_tab
    uint16_t width;     // register width;
    uint16_t stride;    // table stride (<= width)
};

#endif
