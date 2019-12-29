// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Defines data structure to read/store init config

#ifndef __PDS_MS_CONFIG_HPP__
#define __PDS_MS_CONFIG_HPP__

#include <nbase.h>
extern "C" {
#include <a0spec.h>
#include <o0mac.h>
#include <a0cust.h>
#include <a0glob.h>
#include <a0mib.h>
#include <ambips.h>
#include <a0stubs.h>
#include <a0cpif.h>
#include "smsiincl.h"
#include "smsincl.h"
}

// TODO: check if nbbxtype.h include is enough

namespace pds_ms_stub {

typedef struct pds_ms_config_s {
    NBB_ULONG   oid_len;
    NBB_ULONG   data_len;
    NBB_ULONG   correlator;               
    NBB_LONG    row_status;
    NBB_LONG    admin_status;
    NBB_ULONG   entity_index;
    NBB_LONG    stateful;
    NBB_ULONG   interface_id;
    NBB_ULONG   partner_type;
    NBB_ULONG   partner_index;
    NBB_ULONG   sub_index;
    NBB_ULONG   slave_entity_index;
    NBB_LONG    slave_type;
    NBB_ULONG   join_index;
    NBB_LONG    afi;
    NBB_LONG    safi;
}pds_ms_config_t;

}
#endif /*__PDS_MS_CONFIG_HPP__*/
