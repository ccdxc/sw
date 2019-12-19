//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __OIF_LIST_API_HPP__
#define __OIF_LIST_API_HPP__

#include "nic/include/base.hpp"
#include "nic/hal/plugins/cfg/nw/interface_api.hpp"
#include "nic/hal/plugins/cfg/nw/l2segment_api.hpp"
#include "nic/hal/svc/debug_svc.hpp"
#include "gen/proto/multicast.pb.h"

using multicast::Oif;
using multicast::OifList;

namespace hal {

// Place for all public definitions of the OIF List manager.

#define OIF_LIST_ID_INVALID (0)

typedef struct oif_db_s {
    hal_handle_t      if_hndl;
    hal_handle_t      l2seg_hndl;
    uint32_t          qid;
    intf::LifQPurpose purpose;
    dllist_ctxt_t     dllist_ctxt;
} __PACK__ oif_db_t;

typedef struct oif_list_s {
    oif_list_id_t id;
    dllist_ctxt_t oifs;
    oif_list_id_t attached_to;
    ht_ctxt_t     ht_ctxt;
    bool          honor_ingress;
} __PACK__ oif_list_t;

typedef struct oif_s {
    if_t     *intf;
    l2seg_t  *l2seg;
    uint32_t qid;  // RDMA QID - only set by RDMA when adding QPs to repl list
    intf::LifQPurpose purpose; // Same as above, set by RDMA
    oif_s():intf(NULL),l2seg(NULL),qid(0),purpose(intf::LIF_QUEUE_PURPOSE_NONE){}
} __PACK__ oif_t;

typedef struct oifl_get_s {
    oif_list_id_t id;
    grpc::ServerWriter<debug::OifListGetResponseMsg> *writer;
    debug::OifListGetResponseMsg msg;
    uint32_t count;
} oifl_get_t;

// Retrieves the OIF List entry from OIF List ID
hal_ret_t oif_list_get(oif_list_id_t list_id, OifList *rsp);
hal_ret_t oif_list_get(oif_list_t *oif_list, OifList *rsp);
// Creates a new oif_list and returns handle
hal_ret_t oif_list_create(oif_list_id_t *list);
// Creates a contiguous block of oif_lists and returns handle to the first one
hal_ret_t oif_list_create_block(oif_list_id_t *list, uint32_t size);
// Takes an oiflis_handle and deletes it
hal_ret_t oif_list_delete(oif_list_id_t list);
// Takes an oiflis_handle and deletes a block starting from it
hal_ret_t oif_list_delete_block(oif_list_id_t list, uint32_t size);
hal_ret_t oif_list_attach(oif_list_id_t frm, oif_list_id_t to);;
hal_ret_t oif_list_detach(oif_list_id_t frm);
// Adds an oif to list
hal_ret_t oif_list_add_oif(oif_list_id_t list, oif_t *oif);
// Adds an QP based oif to list
hal_ret_t oif_list_add_qp_oif(oif_list_id_t list, oif_t *oif);
// Removes an oif from list
hal_ret_t oif_list_remove_oif(oif_list_id_t list, oif_t *oif);
// Check if an oif is present in the list
hal_ret_t oif_list_is_member(oif_list_id_t list, oif_t *oif);
// Get the number of oifs in the list
hal_ret_t oif_list_get_num_oifs(oif_list_id_t list, uint32_t &num_oifs);
// Get an array of all oifs in the list
hal_ret_t oif_list_get_oif_array(oif_list_id_t list, uint32_t &num_oifs, oif_t *oifs);
// Adds a special node for ingress driven copy
hal_ret_t oif_list_set_honor_ingress(oif_list_id_t list);
// Deletes the special node for ingress driven copy
hal_ret_t oif_list_clr_honor_ingress(oif_list_id_t list);


hal_ret_t oif_list_copy(oif_list_id_t dst_oifl_id, oif_list_id_t src_oifl_id);
hal_ret_t oif_list_copy_oifs(oif_list_id_t dst_oifl_id, oif_list_id_t src_oifl_id);
hal_ret_t oif_list_remove_oifs(oif_list_id_t oifl_id);

void *oif_list_get_key_func(void *entry);
uint32_t oif_list_key_size(void);

hal_ret_t oiflist_get_all_stream(grpc::ServerWriter<debug::OifListGetResponseMsg> *writer);
hal_ret_t oiflist_get_stream (debug::OifListGetRequest& req, 
                              grpc::ServerWriter<debug::OifListGetResponseMsg> *writer);
}    // namespace hal

#endif /* __OIF_LIST_API_HPP__ */
