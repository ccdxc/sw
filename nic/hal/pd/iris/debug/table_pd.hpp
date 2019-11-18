// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __TABLE_PD_HPP__
#define __TABLE_PD_HPP__

namespace hal {
namespace pd {

typedef struct directmap_entry_cb_s {
    directmap *dm;
    TableIndexMsg *msg;
} directmap_entry_cb_t;

typedef struct sldirectmap_entry_cb_s {
    sldirectmap *sldm;
    TableIndexMsg *msg;
} sldirectmap_entry_cb_t;

typedef struct tcam_entry_cb_s {
    tcam *tcam_table;
    TableTcamMsg *msg;
} tcam_entry_cb_t;

typedef struct pd_hash_entry_cb_s {
    sdk_hash *hash_table;
    TableHashMsg *msg;
} pd_hash_entry_cb_t;

typedef struct pd_met_entry_cb_s {
    TableMetMsg *msg;
} pd_met_entry_cb_t;

} // namespace pd
} // namespace hal

#endif    // __TABLE_PD_HPP__
