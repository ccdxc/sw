//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/plugins/cfg/mcast/oif_list_mgr.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/base.hpp"

namespace hal {

// ----------------------------------------------------------------------------
// hash table key => entry
//  - Get key from entry
// ----------------------------------------------------------------------------
void *
oif_list_get_key_func (void *entry)
{
    SDK_ASSERT(entry);
    return (void *)&(((oif_list_t*)entry)->id);
}

// ----------------------------------------------------------------------------
// hash table key size
// ----------------------------------------------------------------------------
uint32_t
oif_list_key_size ()
{
    return sizeof(oif_list_id_t);
}

// allocate an oif_list instance
static inline oif_list_t *oif_list_alloc (void)
{
    return (oif_list_t *)g_hal_state->oif_list_slab()->alloc();
}

// initialize an oif_list instance
static inline oif_list_t *oif_list_init (oif_list_t *oif_list)
{
    if (!oif_list) {
        return NULL;
    }

    // initialize the key
    oif_list->id = OIF_LIST_ID_INVALID;

    // initialize meta information
    sdk::lib::dllist_reset(&oif_list->oifs);
    oif_list->ht_ctxt.reset();
    return oif_list;
}

// allocate and initialize an oif_list instance
static inline oif_list_t *oif_list_alloc_init (void)
{
    return oif_list_init(oif_list_alloc());
}

static inline hal_ret_t oif_list_free (oif_list_t *oif_list)
{
    return hal::delay_delete_to_slab(HAL_SLAB_OIF_LIST, oif_list);
}

static inline oif_list_t *find_oif_list_by_key (oif_list_id_t list_id)
{
    return (oif_list_t *)g_hal_state->oif_list_id_ht()->lookup(&list_id);
}

static hal_ret_t oif_list_add_to_db(oif_list_id_t list_id)
{
    hal_ret_t ret;
    oif_list_t *oif_list = oif_list_alloc_init();

    if (oif_list == NULL) {
        HAL_TRACE_ERR("Failed to allocate list {} in db", list_id);
        return HAL_RET_OOM;
    }

    oif_list->id = list_id;
    ret = hal_sdk_ret_to_hal_ret(g_hal_state->oif_list_id_ht()->
                                 insert_with_key(&oif_list->id,
                                 oif_list, &oif_list->ht_ctxt));
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add list {} to ht, err {}", list_id, ret);
        oif_list_free(oif_list);
        return ret;
    }

    return HAL_RET_OK;
}

static hal_ret_t oif_list_remove_from_db(oif_list_id_t list_id)
{
    auto oif_list = (oif_list_t *)g_hal_state->oif_list_id_ht()->remove(&list_id);

    if (oif_list == NULL) {
        HAL_TRACE_ERR("Failed to find list {}", list_id);
        return HAL_RET_ENTRY_NOT_FOUND;
    }

    SDK_ASSERT(dllist_count(&oif_list->oifs) == 0);
    return oif_list_free(oif_list);
}

static oif_db_t *find_oif_by_key(oif_list_t *oif_list, oif_t *oif)
{
    dllist_ctxt_t  *curr = NULL;
    oif_db_t       *db_oif = NULL;

    dllist_for_each(curr, &oif_list->oifs) {
        db_oif = dllist_entry(curr, oif_db_t, dllist_ctxt);
        if (db_oif->qid == oif->qid &&
            db_oif->purpose == oif->purpose &&
            (oif->intf ? db_oif->if_hndl == oif->intf->hal_handle :
             db_oif->if_hndl == HAL_HANDLE_INVALID) &&
            db_oif->l2seg_hndl == oif->l2seg->hal_handle) {
            return db_oif;
        }
    }

    return NULL;
}

static hal_ret_t oif_add_to_db(oif_list_id_t list_id, oif_t *oif)
{
    oif_db_t   *db_oif = NULL;
    oif_list_t *oif_list = find_oif_list_by_key(list_id);
    if (oif_list == NULL) {
        HAL_TRACE_ERR("Failed to find list {}", list_id);
        return HAL_RET_ENTRY_NOT_FOUND;
    }

    db_oif = find_oif_by_key(oif_list, oif);
    if (db_oif) {
        HAL_TRACE_ERR("Oif intf {} l2seg {} qid {} purpose {} already exists",
                      oif->intf ? oif->intf->hal_handle : HAL_HANDLE_INVALID,
                      oif->l2seg->hal_handle,
                      oif->qid, oif->purpose);
        return HAL_RET_ENTRY_EXISTS;
    }

    db_oif = (oif_db_t *)g_hal_state->oif_slab()->alloc();
    if (db_oif == NULL) {
        HAL_TRACE_ERR("Failed to allocate intf {} l2seg {} qid {} purpose {} in DB",
                      oif->intf ? oif->intf->hal_handle : HAL_HANDLE_INVALID,
                      oif->l2seg->hal_handle,
                      oif->qid, oif->purpose);
        return HAL_RET_OOM;
    }

    db_oif->qid = oif->qid;
    db_oif->purpose = oif->purpose;
    db_oif->if_hndl = oif->intf ? oif->intf->hal_handle : HAL_HANDLE_INVALID;
    db_oif->l2seg_hndl = oif->l2seg->hal_handle;
    sdk::lib::dllist_init(&db_oif->dllist_ctxt);
    sdk::lib::dllist_add(&oif_list->oifs, &db_oif->dllist_ctxt);
    HAL_TRACE_DEBUG("Added intf {} l2seg {} qid {} purpose {} from DB",
                    oif->intf ? oif->intf->hal_handle : HAL_HANDLE_INVALID,
                    oif->l2seg->hal_handle,
                    oif->qid, oif->purpose);
    return HAL_RET_OK;
}

static hal_ret_t oif_remove_from_db(oif_list_id_t list_id, oif_t *oif)
{
    oif_db_t      *db_oif = NULL;
    oif_list_t *oif_list = find_oif_list_by_key(list_id);
    if (oif_list == NULL) {
        HAL_TRACE_ERR("Failed to find list {}", list_id);
        return HAL_RET_ENTRY_NOT_FOUND;
    }

    db_oif = find_oif_by_key(oif_list, oif);
    if (db_oif == NULL) {
        HAL_TRACE_ERR("Failed to find intf {} l2seg {} qid {} purpose {} in DB",
                      oif->intf ? oif->intf->hal_handle : HAL_HANDLE_INVALID,
                      oif->l2seg->hal_handle,
                      oif->qid, oif->purpose);
        HAL_TRACE_DEBUG("Current OifList {} has the following Oifs:", oif_list->id);
        dllist_ctxt_t  *curr = NULL;
        dllist_for_each(curr, &oif_list->oifs) {
            db_oif = dllist_entry(curr, oif_db_t, dllist_ctxt);
            HAL_TRACE_DEBUG("OIF: intf {} l2seg {} qid {} purpose {}",
                            db_oif->if_hndl, db_oif->l2seg_hndl,
                            db_oif->qid, db_oif->purpose);
        }
        return HAL_RET_ENTRY_NOT_FOUND;
    }

    sdk::lib::dllist_del(&db_oif->dllist_ctxt);
    hal::delay_delete_to_slab(HAL_SLAB_OIF, db_oif);

    HAL_TRACE_DEBUG("Removed intf {} l2seg {} qid {} purpose {} from DB",
                    oif->intf ? oif->intf->hal_handle : HAL_HANDLE_INVALID,
                    oif->l2seg->hal_handle,
                    oif->qid, oif->purpose);
    return HAL_RET_OK;
}

// Creates a new oif_list and returns handle
hal_ret_t oif_list_create(oif_list_id_t *list_id)
{
    hal_ret_t ret;
    pd::pd_func_args_t pd_func_args = {};
    pd::pd_oif_list_create_args_t args = {};

    args.list = list_id;
    pd_func_args.pd_oif_list_create = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_OIFL_CREATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD call failed, list {} err {}", *list_id, ret);
        return ret;
    }

    ret = oif_list_add_to_db(*list_id);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add oif list {} to id db, err : {}",
                      *list_id, ret);

        pd::pd_oif_list_delete_args_t del_args = {};
        del_args.list = *list_id;
        pd_func_args.pd_oif_list_delete = &del_args;
        pd::hal_pd_call(pd::PD_FUNC_ID_OIFL_DELETE, &pd_func_args);
        return ret;
    }

    HAL_TRACE_DEBUG("Created list {}", *list_id);
    return HAL_RET_OK;
}

// Creates a contiguous block of oif_lists and returns handle to the first one
hal_ret_t oif_list_create_block(oif_list_id_t *list_id, uint32_t size)
{
    hal_ret_t ret;
    pd::pd_func_args_t pd_func_args = {};
    pd::pd_oif_list_create_block_args_t args = {};

    args.size = size;
    args.list = list_id;
    pd_func_args.pd_oif_list_create_block = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_OIFL_CREATE_BLOCK, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD call failed, list {} + {} err {}", *list_id, size, ret);
        return ret;
    }

    for (oif_list_id_t id = *list_id; id < (*list_id + size); id++) {
        ret = oif_list_add_to_db(id);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to add oif list {} + {} to id db, err : {}",
                          *list_id, size, ret);
            for (oif_list_id_t id2 = *list_id; id2 < id; id2++) {
                oif_list_remove_from_db(id2);
            }

            pd::pd_oif_list_delete_block_args_t del_args = {};
            del_args.size = size;
            del_args.list = *list_id;
            pd_func_args.pd_oif_list_delete_block = &del_args;
            pd::hal_pd_call(pd::PD_FUNC_ID_OIFL_DELETE_BLOCK, &pd_func_args);
            return ret;
        }
    }

    HAL_TRACE_DEBUG("Created list {} + {}", *list_id, size);
    return HAL_RET_OK;
}

// Takes an oiflis_handle and deletes it
hal_ret_t oif_list_delete(oif_list_id_t list_id)
{
    hal_ret_t ret;
    pd::pd_func_args_t pd_func_args = {};
    pd::pd_oif_list_delete_args_t args = {};

    args.list = list_id;
    pd_func_args.pd_oif_list_delete = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_OIFL_DELETE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD call failed, list {} err {}", list_id, ret);
        return ret;
    }

    ret = oif_list_remove_from_db(list_id);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete oif list {} to id db, err : {}",
                      list_id, ret);
        return ret;
    }

    HAL_TRACE_VERBOSE("Deleted list {}", list_id);
    return HAL_RET_OK;
}

// Takes an oiflis_handle and deletes a block starting from it
hal_ret_t oif_list_delete_block(oif_list_id_t list_id, uint32_t size)
{
    hal_ret_t ret;
    pd::pd_func_args_t pd_func_args = {};
    pd::pd_oif_list_delete_block_args_t args = {};

    args.size = size;
    args.list = list_id;
    pd_func_args.pd_oif_list_delete_block = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_OIFL_DELETE_BLOCK, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD call failed, list {} + {} err {}", list_id, size, ret);
        return ret;
    }

    for (oif_list_id_t id = list_id; id < (list_id + size); id++) {
        ret = oif_list_remove_from_db(id);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to delete oif list {} + {} to id db, err : {}",
                          list_id, size, ret);
            return ret;
        }
    }


    HAL_TRACE_DEBUG("Deleted list {} + {}", list_id, size);
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Attach an existing Replication List to another existing Replication List
// This is useful for jumping to (*, G) entries at the end of (S, G) entries
// Also helpful in jumping to all-multicast list at the end of specific lists
// ----------------------------------------------------------------------------
hal_ret_t oif_list_attach(oif_list_id_t frm, oif_list_id_t to)
{
    hal_ret_t ret;
    pd::pd_func_args_t pd_func_args = {};
    pd::pd_oif_list_attach_args_t args = {};
    oif_list_t *oif_list = find_oif_list_by_key(frm);

    if (oif_list == NULL) {
        HAL_TRACE_ERR("Could not find list {}", frm);
        return HAL_RET_ENTRY_NOT_FOUND;
    }

    args.to = to;
    args.frm = frm;
    pd_func_args.pd_oif_list_attach = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_OIFL_ATTACH, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD call failed, list {} err {}", frm, ret);
        return ret;
    }

    oif_list->attached_to = to;
    HAL_TRACE_DEBUG("Attached list {} -> {}", frm, to);
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Detach an existing Replication List from another existing Replication List
// ----------------------------------------------------------------------------
hal_ret_t oif_list_detach(oif_list_id_t frm)
{
    hal_ret_t ret;
    pd::pd_func_args_t pd_func_args = {};
    pd::pd_oif_list_detach_args_t args = {};
    oif_list_t *oif_list = find_oif_list_by_key(frm);

    if (oif_list == NULL) {
        HAL_TRACE_ERR("Could not find list {}", frm);
        return HAL_RET_ENTRY_NOT_FOUND;
    }

    args.frm = frm;
    pd_func_args.pd_oif_list_detach = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_OIFL_DETACH, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD call failed, list {} err {}", frm, ret);
        return ret;
    }

    oif_list->attached_to = OIF_LIST_ID_INVALID;
    HAL_TRACE_DEBUG("Detached list {} -/->", frm);
    return HAL_RET_OK;
}

// Adds an oif to list
hal_ret_t oif_list_add_oif(oif_list_id_t list_id, oif_t *oif)
{
    hal_ret_t ret;
    pd::pd_func_args_t pd_func_args = {};
    pd::pd_oif_list_add_oif_args_t args = {};

    ret = oif_add_to_db(list_id, oif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add to db list {}", list_id);
        return ret;
    }

    args.oif = oif;
    args.list = list_id;
    pd_func_args.pd_oif_list_add_oif = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_OIFL_ADD_OIF, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD call failed, list {} err {}", list_id, ret);
        oif_remove_from_db(list_id, oif);
        return ret;
    }

    HAL_TRACE_DEBUG("Added intf {} l2seg {} qid {} purpose {} to list {}",
                    oif->intf ? oif->intf->hal_handle : HAL_HANDLE_INVALID,
                    oif->l2seg->hal_handle,
                    oif->qid, oif->purpose, list_id);
    return HAL_RET_OK;
}

// Adds an RDMA QP oif to list
hal_ret_t oif_list_add_qp_oif(oif_list_id_t list_id, oif_t *oif)
{
    hal_ret_t ret;
    pd::pd_func_args_t pd_func_args = {};
    pd::pd_oif_list_add_qp_oif_args_t args = {};

    ret = oif_add_to_db(list_id, oif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add to db list {}", list_id);
        return ret;
    }

    args.oif = oif;
    args.list = list_id;
    pd_func_args.pd_oif_list_add_qp_oif = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_OIFL_ADD_QP_OIF, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD call failed, list {} err {}", list_id, ret);
        oif_remove_from_db(list_id, oif);
        return ret;
    }

    HAL_TRACE_DEBUG("Added intf {} l2seg {} qid {} purpose {} to list {}",
                    oif->intf ? oif->intf->hal_handle : HAL_HANDLE_INVALID,
                    oif->l2seg->hal_handle,
                    oif->qid, oif->purpose, list_id);
    return HAL_RET_OK;
}

// Removes an oif from list
hal_ret_t oif_list_remove_oif(oif_list_id_t list_id, oif_t *oif)
{
    hal_ret_t ret;
    pd::pd_func_args_t pd_func_args = {};
    pd::pd_oif_list_remove_oif_args_t args = {};

    ret = oif_remove_from_db(list_id, oif);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to remove frm db list {}", list_id);
        return ret;
    }

    args.oif = oif;
    args.list = list_id;
    pd_func_args.pd_oif_list_remove_oif = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_OIFL_REM_OIF, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD call failed, list {} err {}", list_id, ret);
        return ret;
    }

    HAL_TRACE_DEBUG("Removed intf {} l2seg {} qid {} purpose {} from list {}",
                    oif->intf ? oif->intf->hal_handle : HAL_HANDLE_INVALID,
                    oif->l2seg->hal_handle,
                    oif->qid, oif->purpose, list_id);
    return HAL_RET_OK;
}

// Check if an oif is present in the list
hal_ret_t oif_list_is_member(oif_list_id_t list_id, oif_t *oif)
{
    hal_ret_t ret;
    pd::pd_func_args_t pd_func_args = {};
    pd::pd_oif_list_is_member_args_t args = {};
    args.oif = oif;
    args.list = list_id;
    pd_func_args.pd_oif_list_is_member = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_OIFL_IS_MEMBER, &pd_func_args);
    return ret;
}

// Get the number of oifs in the list
hal_ret_t oif_list_get_num_oifs(oif_list_id_t list, uint32_t &num_oifs)
{
    pd::pd_oif_list_get_num_oifs_args_t args;
    pd::pd_func_args_t          pd_func_args = {};
    args.list = list;
    args.num_oifs = &num_oifs;
    pd_func_args.pd_oif_list_get_num_oifs = &args;
    return pd::hal_pd_call(pd::PD_FUNC_ID_GET_NUM_OIFS, &pd_func_args);
}

hal_ret_t oif_list_set_honor_ingress(oif_list_id_t list_id)
{
    hal_ret_t ret;
    pd::pd_func_args_t pd_func_args = {};
    pd::pd_oif_list_set_honor_ingress_args_t args = {};
    oif_list_t *oif_list = find_oif_list_by_key(list_id);

    if (oif_list == NULL) {
        HAL_TRACE_ERR("Failed to find list {}", list_id);
        return HAL_RET_ENTRY_NOT_FOUND;
    }

    args.list = list_id;
    pd_func_args.pd_oif_list_set_honor_ingress = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_SET_HONOR_ING, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD call failed, list {} err {}", list_id, ret);
        return ret;
    }

    oif_list->honor_ingress = true;
    HAL_TRACE_DEBUG("Set Honor Ingress on list {}", list_id);
    return HAL_RET_OK;
}

hal_ret_t oif_list_clr_honor_ingress(oif_list_id_t list_id)
{
    hal_ret_t ret;
    pd::pd_func_args_t pd_func_args = {};
    pd::pd_oif_list_clr_honor_ingress_args_t args = {};
    oif_list_t *oif_list = find_oif_list_by_key(list_id);

    if (oif_list == NULL) {
        HAL_TRACE_ERR("Failed to find list {}", list_id);
        return HAL_RET_ENTRY_NOT_FOUND;
    }

    args.list = list_id;
    pd_func_args.pd_oif_list_clr_honor_ingress = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_CLR_HONOR_ING, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD call failed, list {} err {}", list_id, ret);
        return ret;
    }

    oif_list->honor_ingress = false;
    HAL_TRACE_DEBUG("Cleared Honor Ingress on list {}", list_id);
    return HAL_RET_OK;
}

hal_ret_t oif_list_copy_oifs (oif_list_id_t dst_oifl_id, oif_list_id_t src_oifl_id)
{
    oif_t oif;
    oif_db_t *db_oif = NULL;
    if_t *hal_if = NULL;
    l2seg_t *l2seg = NULL;
    dllist_ctxt_t *curr_node = NULL;
    oif_list_t *src_oif_list = find_oif_list_by_key(src_oifl_id);
    oif_list_t *dst_oif_list = find_oif_list_by_key(src_oifl_id);

    if (src_oif_list == NULL || dst_oif_list == NULL) {
        HAL_TRACE_ERR("Failed to find src {}, dst {}", src_oifl_id,
                      dst_oifl_id);
        return HAL_RET_ENTRY_NOT_FOUND;
    }

    dllist_for_each(curr_node, &src_oif_list->oifs) {
        db_oif = dllist_entry(curr_node, oif_db_t, dllist_ctxt);
        hal_if = (if_t *)hal_handle_get_obj(db_oif->if_hndl);
        l2seg = (l2seg_t *)hal_handle_get_obj(db_oif->l2seg_hndl);
        oif.intf = hal_if;
        oif.l2seg = l2seg;
        oif.qid = db_oif->qid;
        oif.purpose = db_oif->purpose;
        oif_list_add_oif(dst_oifl_id, &oif);
    }

    return HAL_RET_OK;
}

hal_ret_t oif_list_copy (oif_list_id_t dst_oifl_id, oif_list_id_t src_oifl_id)
{
    oif_t oif;
    oif_db_t *db_oif = NULL;
    if_t *hal_if = NULL;
    l2seg_t *l2seg = NULL;
    dllist_ctxt_t *curr_node = NULL;
    oif_list_t *src_oif_list = find_oif_list_by_key(src_oifl_id);
    oif_list_t *dst_oif_list = find_oif_list_by_key(src_oifl_id);

    if (src_oif_list == NULL || dst_oif_list == NULL) {
        HAL_TRACE_ERR("Failed to find src {}, dst {}", src_oifl_id,
                      dst_oifl_id);
        return HAL_RET_ENTRY_NOT_FOUND;
    }

    dst_oif_list->attached_to = src_oif_list->attached_to;
    dst_oif_list->honor_ingress = src_oif_list->honor_ingress;

    dllist_for_each(curr_node, &src_oif_list->oifs) {
        db_oif = dllist_entry(curr_node, oif_db_t, dllist_ctxt);
        hal_if = (if_t *)hal_handle_get_obj(db_oif->if_hndl);
        l2seg = (l2seg_t *)hal_handle_get_obj(db_oif->l2seg_hndl);
        oif.intf = hal_if;
        oif.l2seg = l2seg;
        oif.qid = db_oif->qid;
        oif.purpose = db_oif->purpose;
        oif_list_add_oif(dst_oifl_id, &oif);
    }

    return HAL_RET_OK;
}

hal_ret_t oif_list_remove_oifs (oif_list_id_t oifl_id)
{
    oif_t oif;
    oif_db_t *db_oif = NULL;
    dllist_ctxt_t    *curr, *next;
    if_t *hal_if = NULL;
    l2seg_t *l2seg = NULL;
    oif_list_t *oif_list = find_oif_list_by_key(oifl_id);

    if (oif_list == NULL) {
        HAL_TRACE_ERR("Failed to find list {}", oifl_id);
        return HAL_RET_ENTRY_NOT_FOUND;
    }
    dllist_for_each_safe(curr, next, &oif_list->oifs) {
        db_oif = dllist_entry(curr, oif_db_t, dllist_ctxt);
        hal_if = (if_t *)hal_handle_get_obj(db_oif->if_hndl);
        l2seg = (l2seg_t *)hal_handle_get_obj(db_oif->l2seg_hndl);
        oif.intf = hal_if;
        oif.l2seg = l2seg;
        oif_list_remove_oif(oifl_id, &oif);
    }

    return HAL_RET_OK;
}

hal_ret_t oif_list_get(oif_list_id_t list_id, OifList *rsp)
{
    dllist_ctxt_t *curr_node = NULL;
    pd::pd_func_args_t pd_func_args = {};
    pd::pd_oif_list_get_args_t args = {};

    HAL_TRACE_DEBUG("oifl get for id: {}", list_id);
    oif_list_t *oif_list = find_oif_list_by_key(list_id);
    if (oif_list == NULL) {
        HAL_TRACE_ERR("Failed to find list {}", list_id);
        return HAL_RET_ENTRY_NOT_FOUND;
    }

    HAL_TRACE_DEBUG("found oifl get for id: {}", list_id);

    rsp->set_id(oif_list->id);
    rsp->set_attached_list_id(oif_list->attached_to);
    rsp->set_is_honor_ingress(oif_list->honor_ingress);

    dllist_for_each(curr_node, &oif_list->oifs) {
        oif_db_t *db_oif = dllist_entry(curr_node, oif_db_t, dllist_ctxt);
        if_t *hal_if = (if_t *)hal_handle_get_obj(db_oif->if_hndl);
        l2seg_t *l2seg = (l2seg_t *)hal_handle_get_obj(db_oif->l2seg_hndl);
        Oif   *oif_rsp = rsp->add_oifs();
        SDK_ASSERT(db_oif && l2seg && oif_rsp);
        oif_rsp->set_q_id(db_oif->qid);
        oif_rsp->set_q_purpose(db_oif->purpose);
        oif_rsp->mutable_interface()->set_interface_id(hal_if ? hal_if->if_id : 0);
        oif_rsp->mutable_l2segment()->set_segment_id(l2seg->seg_id);
    }

    // OIF PD Status
    args.list = list_id;
    args.rsp = rsp;
    pd_func_args.pd_oif_list_get = &args;
    return pd::hal_pd_call(pd::PD_FUNC_ID_OIFL_GET, &pd_func_args);
}

hal_ret_t oif_list_get(oif_list_t *oif_list, OifList *rsp)
{
    dllist_ctxt_t *curr_node = NULL;
    pd::pd_func_args_t pd_func_args = {};
    pd::pd_oif_list_get_args_t args = {};

    HAL_TRACE_DEBUG("found oifl get for id: {}", oif_list->id);

    rsp->set_id(oif_list->id);
    rsp->set_attached_list_id(oif_list->attached_to);
    rsp->set_is_honor_ingress(oif_list->honor_ingress);

    dllist_for_each(curr_node, &oif_list->oifs) {
        oif_db_t *db_oif = dllist_entry(curr_node, oif_db_t, dllist_ctxt);
        if_t *hal_if = (if_t *)hal_handle_get_obj(db_oif->if_hndl);
        l2seg_t *l2seg = (l2seg_t *)hal_handle_get_obj(db_oif->l2seg_hndl);
        Oif   *oif_rsp = rsp->add_oifs();
        SDK_ASSERT(db_oif && l2seg && oif_rsp);
        oif_rsp->set_q_id(db_oif->qid);
        oif_rsp->set_q_purpose(db_oif->purpose);
        oif_rsp->mutable_interface()->set_interface_id(hal_if ? hal_if->if_id : 0);
        oif_rsp->mutable_l2segment()->set_segment_id(l2seg->seg_id);
    }

    // OIF PD Status
    args.list = oif_list->id;
    args.rsp = rsp;
    pd_func_args.pd_oif_list_get = &args;
    return pd::hal_pd_call(pd::PD_FUNC_ID_OIFL_GET, &pd_func_args);
}

#define OIFL_GET_STREAM_COUNT 200
hal_ret_t
oiflist_get_stream (debug::OifListGetRequest& req, 
                    grpc::ServerWriter<debug::OifListGetResponseMsg> *writer)
{
    hal_ret_t ret = HAL_RET_OK;
    debug::OifListGetResponseMsg msg;
    oifl_get_t oifl_get;

    ret = oif_list_get(req.id(), msg.add_response()->mutable_oif_list());
    if (ret != HAL_RET_OK) {
        msg.set_api_status(types::API_STATUS_NOT_FOUND);
    }
    writer->Write(msg);

    return ret;
}

hal_ret_t
oiflist_get_all_stream(grpc::ServerWriter<debug::OifListGetResponseMsg> *writer)
{
    oifl_get_t oifl_get;

    oifl_get.writer = writer;
    oifl_get.msg.Clear();
    oifl_get.count = 0;

    HAL_TRACE_DEBUG("Getting all oifls");

    auto walk_func = [](void *entry, void *ctxt) {
        oif_list_t* oifl = (oif_list_t *)entry;
        oifl_get_t *oifl_get = (oifl_get_t *)ctxt;
        HAL_TRACE_DEBUG("walking oifl get for id: {}", oifl->id);
        hal::oif_list_get(oifl, oifl_get->msg.add_response()->mutable_oif_list());
        oifl_get->count++;
        if (oifl_get->count == OIFL_GET_STREAM_COUNT) {
            oifl_get->writer->Write(oifl_get->msg);
            oifl_get->msg.Clear();
            oifl_get->count = 0;
        }
        return false;
    };

    sdk_ret_t ret = g_hal_state->oif_list_id_ht()->walk_safe(walk_func, &oifl_get);
    if (oifl_get.count) {
        oifl_get.writer->Write(oifl_get.msg);
        oifl_get.msg.Clear();
        oifl_get.count = 0;
    }

    return hal_sdk_ret_to_hal_ret(ret);
}


}    // namespace hal
