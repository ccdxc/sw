//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// route table handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/route.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {

typedef struct route_table_upd_ctxt_s {
    route_table *rtable;
    api_obj_ctxt_t *obj_ctxt;
    uint64_t upd_bmap;
} __PACK__ route_table_upd_ctxt_t;

route_table::route_table() {
    af_ = IP_AF_NIL;
    num_routes_ = 0;
    ht_ctxt_.reset();
    impl_ = NULL;
}

route_table *
route_table::factory(pds_route_table_spec_t *spec) {
    route_table    *rtable;

    // create route table instance with defaults, if any
    rtable = route_table_db()->alloc();
    if (rtable) {
        new (rtable) route_table();
        rtable->impl_ = impl_base::factory(impl::IMPL_OBJ_ID_ROUTE_TABLE, spec);
        if (rtable->impl_ == NULL) {
            route_table::destroy(rtable);
            return NULL;
        }
    }
    return rtable;
}

route_table::~route_table() {
}

void
route_table::destroy(route_table *rtable) {
    rtable->nuke_resources_();
    if (rtable->impl_) {
        impl_base::destroy(impl::IMPL_OBJ_ID_ROUTE_TABLE, rtable->impl_);
    }
    rtable->~route_table();
    route_table_db()->free(rtable);
}

api_base *
route_table::clone(api_ctxt_t *api_ctxt) {
    route_table *cloned_rtable;

    cloned_rtable = route_table_db()->alloc();
    if (cloned_rtable) {
        new (cloned_rtable) route_table();
        if (cloned_rtable->init_config(api_ctxt) != SDK_RET_OK) {
            goto error;
        }
        cloned_rtable->impl_ = impl_->clone();
        if (unlikely(cloned_rtable->impl_ == NULL)) {
            PDS_TRACE_ERR("Failed to clone route table %s impl",
                          key2str().c_str());
            goto error;
        }
    }
    return cloned_rtable;

error:

    cloned_rtable->~route_table();
    route_table_db()->free(cloned_rtable);
    return NULL;
}

api_base *
route_table::clone(void) {
    route_table *cloned_rtable;

    cloned_rtable = route_table_db()->alloc();
    if (cloned_rtable) {
        new (cloned_rtable) route_table();
        if (cloned_rtable->init_config_(this) != SDK_RET_OK) {
            goto error;
        }
        cloned_rtable->impl_ = impl_->clone();
        if (unlikely(cloned_rtable->impl_ == NULL)) {
            PDS_TRACE_ERR("Failed to clone route table %s impl",
                          key2str().c_str());
            goto error;
        }
    }
    return cloned_rtable;

error:

    cloned_rtable->~route_table();
    route_table_db()->free(cloned_rtable);
    return NULL;
}

sdk_ret_t
route_table::free(route_table *rtable) {
    if (rtable->impl_) {
        impl_base::free(impl::IMPL_OBJ_ID_ROUTE_TABLE, rtable->impl_);
    }
    rtable->~route_table();
    route_table_db()->free(rtable);
    return SDK_RET_OK;
}

sdk_ret_t
route_table::reserve_resources(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    return impl_->reserve_resources(this, orig_obj, obj_ctxt);
}

sdk_ret_t
route_table::release_resources(void) {
    return impl_->release_resources(this);
}

sdk_ret_t
route_table::nuke_resources_(void) {
    return impl_->nuke_resources(this);
}

sdk_ret_t
route_table::init_config(api_ctxt_t *api_ctxt) {
    pds_route_table_spec_t *spec;

    spec = &api_ctxt->api_params->route_table_spec;
    memcpy(&this->key_, &spec->key, sizeof(pds_obj_key_t));
    this->af_ = spec->route_info->af;
    this->num_routes_ = spec->route_info->num_routes;
    return SDK_RET_OK;
}

sdk_ret_t
route_table::init_config_(route_table *rtable) {
    pds_obj_key_t key = rtable->key();

    memcpy(&key_, &key, sizeof(key_));
    af_ = rtable->af();
    num_routes_ = rtable->num_routes();
    return SDK_RET_OK;
}

sdk_ret_t
route_table::program_create(api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;

    PDS_TRACE_DEBUG("Programming route table %s", key2str().c_str());
    ret = impl_->program_hw(this, obj_ctxt);
    // for container objects, element count can change during create processing
    // as individual route add/del/upd can happen in the same batch as route
    // table create, so we need to reflect that in the object
    if (ret == SDK_RET_OK) {
        num_routes_ =
            obj_ctxt->api_params->route_table_spec.route_info->num_routes;
    }
    return ret;
}

sdk_ret_t
route_table::compute_update(api_obj_ctxt_t *obj_ctxt) {
    pds_route_table_spec_t *spec = &obj_ctxt->api_params->route_table_spec;

    // we can enable/disable priority based routing and/or change individual
    // routes in the route table but not the address family
    if (af_ != spec->route_info->af) {
        PDS_TRACE_ERR("Attempt to modify immutable attr \"address family\" "
                      "on route table %s", key_.str());
        return SDK_RET_INVALID_ARG;
    }
    // in all other cases we have to recompute the route table and program in
    // the datapath so we can assume that routes are being modified in all other
    // cases
    obj_ctxt->upd_bmap |= PDS_ROUTE_TABLE_UPD_ROUTES;
    return SDK_RET_OK;
}

static bool
subnet_upd_walk_cb_ (void *api_obj, void *ctxt) {
    subnet_entry *subnet;
    route_table_upd_ctxt_t *upd_ctxt = (route_table_upd_ctxt_t *)ctxt;

    subnet = (subnet_entry *)api_framework_obj((api_base *)api_obj);
    if ((subnet->v4_route_table() == upd_ctxt->rtable->key()) ||
        (subnet->v6_route_table() == upd_ctxt->rtable->key())) {
        api_obj_add_to_deps(upd_ctxt->obj_ctxt->api_op,
                            OBJ_ID_ROUTE_TABLE, upd_ctxt->rtable,
                            OBJ_ID_SUBNET, (api_base *)api_obj,
                            upd_ctxt->upd_bmap);
    }
    return false;
}

static bool
vpc_upd_walk_cb_ (void *api_obj, void *ctxt) {
    vpc_entry *vpc;
    route_table_upd_ctxt_t *upd_ctxt = (route_table_upd_ctxt_t *)ctxt;

    vpc = (vpc_entry *)api_framework_obj((api_base *)api_obj);
    if ((vpc->v4_route_table() == upd_ctxt->rtable->key()) ||
        (vpc->v6_route_table() == upd_ctxt->rtable->key())) {
        api_obj_add_to_deps(upd_ctxt->obj_ctxt->api_op,
                            OBJ_ID_ROUTE_TABLE, upd_ctxt->rtable,
                            OBJ_ID_VPC, (api_base *)api_obj,
                            upd_ctxt->upd_bmap);
    }
    return false;
}

sdk_ret_t
route_table::add_deps(api_obj_ctxt_t *obj_ctxt) {
    route_table_upd_ctxt_t upd_ctxt = { 0 };

    upd_ctxt.rtable = this;
    upd_ctxt.obj_ctxt = obj_ctxt;

    // walk the vpcs and add affected vpcs to dependency list
    upd_ctxt.upd_bmap = PDS_VPC_UPD_ROUTE_TABLE;
    vpc_db()->walk(vpc_upd_walk_cb_, &upd_ctxt);

    // walk the subnets and add affected subnets to dependency list
    upd_ctxt.upd_bmap = PDS_SUBNET_UPD_ROUTE_TABLE;
    subnet_db()->walk(subnet_upd_walk_cb_, &upd_ctxt);
    return SDK_RET_OK;
}

sdk_ret_t
route_table::program_update(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;

    PDS_TRACE_DEBUG("Updating route table %s", key_.str());
    ret = impl_->update_hw(orig_obj, this, obj_ctxt);
    // for container objects, element count can change during update processing
    // as individual route add/del/upd can happen in the same batch as route
    // table update, so we need to reflect that in the object
    if (ret == SDK_RET_OK) {
        num_routes_ =
            obj_ctxt->api_params->route_table_spec.route_info->num_routes;
    }
    return ret;
}

sdk_ret_t
route_table::activate_config(pds_epoch_t epoch, api_op_t api_op,
                             api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    PDS_TRACE_DEBUG("Activating %s config", key2str().c_str());
    return impl_->activate_hw(this, orig_obj, epoch, api_op, obj_ctxt);
}

sdk_ret_t
route_table::fill_spec_(pds_route_table_spec_t *spec) {
    sdk_ret_t ret = SDK_RET_OK;

    memcpy(&spec->key, &key_, sizeof(pds_obj_key_t));
    if (spec->route_info) {
        uint32_t num_routes = spec->route_info->num_routes;
        spec->route_info->af = af_;
        if (!num_routes) {
            // set num routes and return
            spec->route_info->num_routes = num_routes_;
        } else if (num_routes < num_routes_) {
            // buffer is smaller, read all routes and copy over the
            // requested number allocate memory for reading all the routes
            route_info_t *route_info =
                (route_info_t *)SDK_CALLOC(PDS_MEM_ALLOC_ID_ROUTE_TABLE,
                                           ROUTE_INFO_SIZE(num_routes_));
            route_info->num_routes = num_routes_;
            // retrieve all routes
            ret = route_table_db()->retrieve_routes(&key_, route_info);
            if (ret != SDK_RET_OK) {
                SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE, route_info);
                return ret;
            }
            // copy over requested number of routes
            memcpy(spec->route_info, route_info, ROUTE_INFO_SIZE(num_routes));
            spec->route_info->num_routes = num_routes;
            // free allocated memory
            SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE, route_info);
        } else {
            // retrieve routes from lmdb
            ret = route_table_db()->retrieve_routes(&key_, spec->route_info);
        }
    }
    return ret;
}

sdk_ret_t
route_table::read(pds_route_table_info_t *info) {
    sdk_ret_t ret;

    ret = fill_spec_(&info->spec);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    return impl_->read_hw(this, (impl::obj_key_t *)(&info->spec.key),
                          (impl::obj_info_t *)info);
}

sdk_ret_t
route_table::add_to_db(void) {
    PDS_TRACE_VERBOSE("Adding route table %s to db", key2str().c_str());
    return route_table_db()->insert(this);
}

sdk_ret_t
route_table::del_from_db(void) {
    if (route_table_db()->remove(this)) {
        return SDK_RET_OK;
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
route_table::update_db(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    if (route_table_db()->remove((route_table *)orig_obj)) {
        return route_table_db()->insert(this);
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
route_table::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_ROUTE_TABLE, this);
}

route *
route::factory(pds_route_spec_t *spec) {
    route *rt;

    // create route instance with defaults, if any
    rt = route_db()->alloc();
    if (rt) {
        new (rt) route();
    }
    return rt;
}

void
route::destroy(route *rt) {
    rt->~route();
    route_db()->free(rt);
}

api_base *
route::clone(api_ctxt_t *api_ctxt) {
    route *cloned_route;

    cloned_route = route_db()->alloc();
    if (cloned_route) {
        new (cloned_route) route();
        if (cloned_route->init_config(api_ctxt) != SDK_RET_OK) {
            goto error;
        }
    }
    return cloned_route;

error:

    cloned_route->~route();
    route_db()->free(cloned_route);
    return NULL;
}

sdk_ret_t
route::free(route *rt) {
    rt->~route();
    route_db()->free(rt);
    return SDK_RET_OK;
}

route *
route::build(pds_route_key_t *key) {
    route *rt;

    rt = route_db()->alloc();
    if (rt) {
        new (rt) route();
        memcpy(&rt->key_, key, sizeof(*key));
    }
    return rt;
}

void
route::soft_delete(route *route) {
    route->del_from_db();
    route->~route();
    route_db()->free(route);
}

sdk_ret_t
route::init_config(api_ctxt_t *api_ctxt) {
    pds_route_spec_t *spec = &api_ctxt->api_params->route_spec;

    memcpy(&key_, &spec->key, sizeof(key_));
    return SDK_RET_OK;
}

sdk_ret_t
route::add_deps(api_obj_ctxt_t *obj_ctxt) {
    route_table *rtable;
    pds_obj_key_t route_table_key;

    if ((obj_ctxt->api_op == API_OP_CREATE) ||
        (obj_ctxt->api_op == API_OP_UPDATE)) {
        if (obj_ctxt->cloned_obj) {
            route_table_key =
                ((route *)obj_ctxt->cloned_obj)->key_.route_table_id;
        } else {
            route_table_key = key_.route_table_id;
        }
        rtable = route_table_find(&route_table_key);
        if (!rtable) {
            PDS_TRACE_ERR("Failed to perform api op %u on route %s, "
                          "route table %s not found",
                          obj_ctxt->api_op, key_.route_id.str(),
                          route_table_key.str());
            return SDK_RET_INVALID_ARG;
        }
        api_obj_add_to_deps(API_OP_UPDATE, OBJ_ID_ROUTE, this,
                            OBJ_ID_ROUTE_TABLE, rtable,
                            (obj_ctxt->api_op == API_OP_CREATE) ?
                                 PDS_ROUTE_TABLE_UPD_ROUTE_ADD :
                                 PDS_ROUTE_TABLE_UPD_ROUTE_UPD);
    } else {
        rtable = route_table_find(&key_.route_table_id);
        if (!rtable) {
            PDS_TRACE_ERR("Failed to perform api op %u on route %s, "
                          "route table %s not found",
                          obj_ctxt->api_op, key_.route_id.str(),
                          key_.route_table_id.str());
            return SDK_RET_INVALID_ARG;
        }
        api_obj_add_to_deps(API_OP_UPDATE, OBJ_ID_ROUTE, this,
                            OBJ_ID_ROUTE_TABLE, rtable,
                            PDS_ROUTE_TABLE_UPD_ROUTE_DEL);
    }
    return SDK_RET_OK;
}

sdk_ret_t
route::read(pds_route_info_t *info) {
    sdk_ret_t ret;
    uint32_t num_routes;
    route_info_t *route_info;
    pds_route_table_info_t route_table_info;

    memset(&route_table_info, 0, sizeof(route_table_info));
    // get number of routes
    route_table_info.spec.route_info =
        (route_info_t *)SDK_CALLOC(PDS_MEM_ALLOC_ID_ROUTE_TABLE,
                                   ROUTE_INFO_SIZE(0));
    if (unlikely(route_table_info.spec.route_info == NULL)) {
        PDS_TRACE_ERR("Failed to allocate route table %s memory for "
                      "route %s read", key_.route_table_id,
                      key_.route_id.str());
        return SDK_RET_OOM;
    }
    ret = pds_route_table_read(&key_.route_table_id, &route_table_info);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to get route table %s size, err %u",
                      key_.route_table_id.str(), ret);
        goto end;
    }
    num_routes = route_table_info.spec.route_info->num_routes;
    SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE, route_table_info.spec.route_info);
    route_table_info.spec.route_info =
        (route_info_t *)SDK_CALLOC(PDS_MEM_ALLOC_ID_ROUTE_TABLE,
                                  ROUTE_INFO_SIZE(num_routes));
    if (unlikely(route_table_info.spec.route_info == NULL)) {
        PDS_TRACE_ERR("Failed to allocate route table %s memory for "
                      "route %s read", key_.route_table_id,
                      key_.route_id.str());
        return SDK_RET_OOM;
    }
    route_table_info.spec.route_info->num_routes = num_routes;
    ret = pds_route_table_read(&key_.route_table_id, &route_table_info);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to read all route table %s routes, err %u",
                      key_.route_table_id.str(), ret);
        goto end;
    }
    // walk the route table and find the route of interest
    route_info = route_table_info.spec.route_info;
    for (uint32_t i = 0; i < num_routes; i++) {
        if (key_.route_id == route_info->routes[i].key) {
            // fill the spec
            memcpy(&info->spec.key, &key_, sizeof(key_));
            memcpy(&info->spec.attrs, &route_info->routes[i].attrs,
                   sizeof(info->spec.attrs));
            break;
        }
        // continue the search
    }

end:

    if (route_table_info.spec.route_info) {
         SDK_FREE(PDS_MEM_ALLOC_ID_ROUTE_TABLE,
                  route_table_info.spec.route_info);
    }
    return ret;
}

sdk_ret_t
route::add_to_db(void) {
    return route_db()->insert(this);
}

sdk_ret_t
route::del_from_db(void) {
    if (route_db()->remove(this)) {
        return SDK_RET_OK;
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
route::update_db(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    if (route_db()->remove((route *)orig_obj)) {
        return route_db()->insert(this);
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
route::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_ROUTE, this);
}

}    // namespace api
