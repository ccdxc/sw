//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include "platform/src/lib/nicmgr/include/logger.hpp"
#include "filter.hpp"
#include "lif.hpp"
#include "print.hpp"
#include "utils.hpp"
#include "endpoint.hpp"
#include "devapi_mem.hpp"
#include "enic.hpp"
#include "multicast.hpp"

namespace iris {

devapi_filter *
devapi_filter::factory(devapi_lif *lif, mac_t mac, vlan_t vlan,
                       filter_type_t type)
{
    sdk_ret_t ret = SDK_RET_OK;
    void *mem = NULL;
    devapi_filter *filter = NULL;

    api_trace("filter create");

    mem = (devapi_filter *)DEVAPI_CALLOC(DEVAPI_MEM_ALLOC_FILTER,
                                         sizeof(devapi_filter));
    if (mem) {
        filter = new (mem) devapi_filter();
        ret = filter->init_(lif, mac, vlan, type);
        if (ret != SDK_RET_OK) {
            goto end;
        }
        ret = filter->filter_halcreate();
        if (ret != SDK_RET_OK) {
            goto end;
        }
    }

end:
    if (ret != SDK_RET_OK) {
        filter->~devapi_filter();
        DEVAPI_FREE(DEVAPI_MEM_ALLOC_FILTER, mem);
        filter = NULL;
    }
    return filter;
}

sdk_ret_t
devapi_filter::init_(devapi_lif *lif,
                     mac_t mac, vlan_t vlan,
                     filter_type_t type)
{
    type_ = type;
    mac_ = mac;
    vlan_ = vlan;
    lif_ = lif;
    ep_ = NULL;
    mcast_ = NULL;

    return SDK_RET_OK;
}

void
devapi_filter::destroy(devapi_filter *filter)
{
    filter->filter_haldelete();
    filter->~devapi_filter();
    DEVAPI_FREE(DEVAPI_MEM_ALLOC_FILTER, filter);
}

sdk_ret_t
devapi_filter::filter_halcreate()
{
    sdk_ret_t         ret = SDK_RET_OK;
    grpc::Status      status;
    FilterSpec        *req;
    FilterResponse    rsp;
    FilterRequestMsg  req_msg;
    FilterResponseMsg rsp_msg;
    devapi_enic       *enic;
    devapi_l2seg      *l2seg;

    NIC_LOG_DEBUG("Mac-Vlan entity creation. Type: {}, lif: {}, mac: {}, vlan: {}. ",
                    type_,
                    lif_->get_id(),
                    macaddr2str(mac_), vlan_);

    // For smart we should not even be coming here
    // if (lif_->is_classicfwd())
    if (true) {
        // Unicast:
        //  - Add vlan to devapi_enic. May have already been added.
        //  - Create EP
        // devapi_mcast:
        //  - Add devapi_enic to multicast
        enic = lif_->get_enic();
        ret = enic->add_vlan(vlan_);
        if (ret != SDK_RET_OK) {
            // Nothing to cleanup. Just skip adding mac vlan filter
            NIC_LOG_ERR("Failed to add vlan {} on enic: {}, lif: {}",
                        vlan_, enic->get_id(), lif_->get_id());
            goto end;
        }
        l2seg = enic->get_l2seg(vlan_);
        if (!l2seg) {
            NIC_LOG_ERR("l2seg should have been added in AddVlan. Please check.");
            assert(0);
        }
        if (is_multicast(mac_)) {
            mcast_ = devapi_mcast::find_or_create(l2seg, mac_, true);
            if (mcast_ == NULL) {
                NIC_LOG_ERR("Failed to create Mcast: vlan: {}, mac: {}, enic: {}, lif: {}",
                            vlan_, macaddr2str(mac_), enic->get_id(), lif_->get_id());
                // Remove vlan from devapi_enic.
                enic->del_vlan(vlan_);
                ret = SDK_RET_ERR;
                goto end;
            }
            mcast_->add_enic(enic);
        } else {
            ep_ = devapi_ep::factory(l2seg, mac_, enic, NULL);
            if (ep_ == NULL) {
                NIC_LOG_ERR("Failed to create EP: vlan: {}, mac: {}, enic: {}, lif: {}",
                            vlan_, macaddr2str(mac_), enic->get_id(), lif_->get_id());
                // Remove vlan from devapi_enic.
                enic->del_vlan(vlan_);
                ret = SDK_RET_ERR;
                goto end;
            }
        }
    } else {
        // Create Filter to HAL
        req = req_msg.add_request();
        req->mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(lif_->get_id());
        req->mutable_key_or_handle()->mutable_filter_key()->set_mac_address(mac_);
        req->mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(vlan_);
        req->mutable_key_or_handle()->mutable_filter_key()->set_type(type_);

        VERIFY_HAL();
        status = hal->filter_create(req_msg, rsp_msg);
        if (status.ok()) {
            rsp = rsp_msg.response(0);
            if (rsp.api_status() == types::API_STATUS_OK) {
                NIC_LOG_DEBUG("Created Filter mac: {} vlan: {}",
                              mac_, vlan_);
            } else {
                NIC_LOG_ERR("Failed to create Filter mac: {} vlan: {}. err: {}",
                            mac_, vlan_, rsp.api_status());
                ret = SDK_RET_ERR;
                goto end;
            }
        } else {
            NIC_LOG_ERR("Failed to create Filter mac: {} vlan: {}. err: {}:{}",
                        mac_, vlan_, status.error_code(), status.error_message());
            ret = SDK_RET_ERR;
            goto end;
        }
    }
end:
    return ret;
}

sdk_ret_t
devapi_filter::filter_haldelete()
{
    sdk_ret_t                 ret = SDK_RET_OK;
    grpc::ClientContext       context;
    grpc::Status              status;
    FilterDeleteRequest       *req;
    FilterDeleteResponse      rsp;
    FilterDeleteRequestMsg    req_msg;
    FilterDeleteResponseMsg   rsp_msg;
    devapi_enic               *enic;
    devapi_l2seg              *l2seg;
    devapi_ep                 *ep;

    NIC_LOG_DEBUG("Mac-Vlan entity deletion. lif: {}, mac: {}, vlan: {}. ",
                    lif_->get_id(),
                    macaddr2str(mac_), vlan_);

    // For smart we should not even come here
    // if (lif_->is_classicfwd()) {
    if (true) {
        enic = lif_->get_enic();
        l2seg = enic->get_l2seg(vlan_);
        if (is_multicast(mac_)) {
            mcast_ = devapi_mcast::find_or_create(l2seg, mac_, false);
            mcast_->del_enic(enic);

            // Delete Mcast entry if #enics become 0.
            if (!mcast_->get_numenics()) {
                devapi_mcast::destroy(mcast_);
            }
        } else {
            ep = devapi_ep::lookup(l2seg, mac_);
            devapi_ep::destroy(ep);
        }
        enic->del_vlan(vlan_);
    } else {
        // Create Filter to HAL
        req = req_msg.add_request();
        req->mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->
            set_lif_id(lif_->get_id());
        req->mutable_key_or_handle()->mutable_filter_key()->set_mac_address(mac_);
        req->mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(vlan_);
        req->mutable_key_or_handle()->mutable_filter_key()->set_type(type_);

        VERIFY_HAL();
        status = hal->filter_delete(req_msg, rsp_msg);
        if (status.ok()) {
            rsp = rsp_msg.response(0);
            if (rsp.api_status() == types::API_STATUS_OK) {
                NIC_LOG_DEBUG("Delted Filter mac: {} vlan: {}",
                              mac_, vlan_);
            } else {
                NIC_LOG_ERR("Failed to delete Filter mac: {} vlan: {}. err: {}",
                            mac_, vlan_, rsp.api_status());
                ret = SDK_RET_ERR;
                goto end;
            }
        } else {
            NIC_LOG_ERR("Failed to delete Filter mac: {} vlan: {}. err: {}:{}",
                        mac_, vlan_, status.error_code(), status.error_message());
            ret = SDK_RET_ERR;
            goto end;
        }
    }

end:
    return ret;
}

}    // namespace iris
