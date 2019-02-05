#include "filter.hpp"
#include "lif.hpp"
#include "print.hpp"
#include "utils.hpp"

using namespace std;

MacVlanFilter *
MacVlanFilter::Factory(Lif *lif, mac_t mac, vlan_t vlan,
                       filter_type_t type)
{
    hal_irisc_ret_t ret = HAL_IRISC_RET_SUCCESS;
    MacVlanFilter *filter = new MacVlanFilter(lif, mac, vlan, type);

    ret = filter->MacVlanFilterCreate();
    if (ret != HAL_IRISC_RET_SUCCESS) {
        NIC_LOG_DEBUG("Filter Create failed. ret: {}", ret);
        goto end;
    }

end:
    if (ret != HAL_IRISC_RET_SUCCESS) {
        if (filter) {
            delete filter;
            filter = NULL;
        }
    }
    return filter;
}

hal_irisc_ret_t
MacVlanFilter::Destroy(MacVlanFilter *filter)
{
    hal_irisc_ret_t ret = HAL_IRISC_RET_SUCCESS;
    if (filter) {
        filter->~MacVlanFilter();
    }
    ret = filter->MacVlanFilterDelete();
    if (ret != HAL_IRISC_RET_SUCCESS) {
        NIC_LOG_CRIT("Filter Delete failed. ret: {}", ret);
        goto end;
    }
    delete filter;

end:
    return ret;

}

MacVlanFilter::MacVlanFilter(Lif *lif,
                             mac_t mac, vlan_t vlan,
                             filter_type_t type)
{
    _type = type;
    _mac = mac;
    _vlan = vlan;
    this->lif = lif;
}

hal_irisc_ret_t
MacVlanFilter::MacVlanFilterCreate()
{
    hal_irisc_ret_t             ret = HAL_IRISC_RET_SUCCESS;
    grpc::Status                status;
    endpoint::FilterSpec        *req;
    endpoint::FilterResponse    rsp;
    endpoint::FilterRequestMsg  req_msg;
    endpoint::FilterResponseMsg rsp_msg;
    Enic                        *enic;
    HalL2Segment                *l2seg;


    NIC_LOG_DEBUG("Mac-Vlan entity creation. Type: {}, lif: {}, mac: {}, vlan: {}. ",
                    _type,
                    lif->GetId(),
                    macaddr2str(_mac), _vlan);

    if (lif->IsClassicForwarding()) {
        // Unicast:
        //  - Add vlan to Enic. May have already been added.
        //  - Create EP
        // HalMulticast:
        //  - Add Enic to multicast
        enic = lif->GetEnic();
        ret = enic->AddVlan(_vlan);
        if (ret != HAL_IRISC_RET_SUCCESS) {
            // Nothing to cleanup. Just skip adding mac vlan filter
            NIC_LOG_ERR("Failed to add vlan {} on enic: {}, lif: {}", _vlan, enic->GetId(), lif->GetId());
            goto end;
        }
        l2seg = enic->GetL2seg(_vlan);
        if (!l2seg) {
            NIC_LOG_ERR("l2seg should have been added in AddVlan. Please check.");
            assert(0);
        }
        if (is_multicast(_mac)) {
            mcast = HalMulticast::GetInstance(l2seg, _mac);
            if (mcast == NULL) {
                NIC_LOG_ERR("Failed to create Mcast: vlan: {}, mac: {}, enic: {}, lif: {}",
                            _vlan, macaddr2str(_mac), enic->GetId(), lif->GetId());
                // Remove vlan from Enic.
                enic->DelVlan(_vlan);
                ret = HAL_IRISC_RET_FAIL;
                goto end;
            }
            mcast->AddEnic(enic);
        } else {
            ep = HalEndpoint::Factory(l2seg, _mac, enic);
            if (ep == NULL) {
                NIC_LOG_ERR("Failed to create EP: vlan: {}, mac: {}, enic: {}, lif: {}",
                            _vlan, macaddr2str(_mac), enic->GetId(), lif->GetId());
                // Remove vlan from Enic.
                enic->DelVlan(_vlan);
                ret = HAL_IRISC_RET_FAIL;
                goto end;
            }
        }
    } else {
        // Create Filter to HAL
        req = req_msg.add_request();
        req->mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(lif->GetId());
        req->mutable_key_or_handle()->mutable_filter_key()->set_mac_address(_mac);
        req->mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(_vlan);

        req->mutable_key_or_handle()->mutable_filter_key()->set_type(_type);
#if 0
        if (mac && vlan) {
            // (Mac, Vlan)
            req->mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_MAC_VLAN);
        } else if (!vlan) {
            // Mac
            req->mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_MAC);
        } else if (!mac) {
            // Vlan
            req->mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_VLAN);
        } else {
            // Lif,*,*
            req->mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF);
        }
#endif

        status = hal->filter_create(req_msg, rsp_msg);
        if (status.ok()) {
            rsp = rsp_msg.response(0);
            if (rsp.api_status() == types::API_STATUS_OK) {
                NIC_LOG_DEBUG("Created Filter mac: {} vlan: {}",
                              _mac, _vlan);
            } else {
                NIC_LOG_ERR("Failed to create Filter mac: {} vlan: {}. err: {}",
                            _mac, _vlan, rsp.api_status());
                ret = HAL_IRISC_RET_FAIL;
                goto end;
            }
        } else {
            NIC_LOG_ERR("Failed to create Filter mac: {} vlan: {}. err: {}:{}",
                        _mac, _vlan, status.error_code(), status.error_message());
            ret = HAL_IRISC_RET_FAIL;
            goto end;
        }

        // Store spec
        spec.CopyFrom(*req);
    }
end:
    return ret;
}

hal_irisc_ret_t
MacVlanFilter::MacVlanFilterDelete()
{
    hal_irisc_ret_t                     ret = HAL_IRISC_RET_SUCCESS;
    grpc::ClientContext                 context;
    grpc::Status                        status;

    endpoint::FilterDeleteRequest       *req;
    endpoint::FilterDeleteResponse      rsp;
    endpoint::FilterDeleteRequestMsg    req_msg;
    endpoint::FilterDeleteResponseMsg   rsp_msg;
    Enic                                *enic;
    HalL2Segment                        *l2seg;
    HalEndpoint                         *ep;

    NIC_LOG_DEBUG("Mac-Vlan entity deletion. lif: {}, mac: {}, vlan: {}. ",
                    lif->GetId(),
                    macaddr2str(_mac), _vlan);

    if (lif->IsClassicForwarding()) {
        enic = lif->GetEnic();
        l2seg = enic->GetL2seg(_vlan);
        if (is_multicast(_mac)) {
            mcast = HalMulticast::GetInstance(l2seg, _mac);
            mcast->DelEnic(enic);

            // Delete Mcast entry if #enics become 0.
            if (!mcast->GetNumEnics()) {
                HalMulticast::Destroy(mcast);
            }
        } else {
            ep = HalEndpoint::Lookup(l2seg, _mac);
            HalEndpoint::Destroy(ep);
        }
        enic->DelVlan(_vlan);
    } else {
        // Create Filter to HAL
        req = req_msg.add_request();
        req->mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->
            set_lif_id(lif->GetId());
        req->mutable_key_or_handle()->mutable_filter_key()->set_mac_address(_mac);
        req->mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(_vlan);
        req->mutable_key_or_handle()->mutable_filter_key()->set_type(_type);
#if 0
        if (_mac && _vlan) {
            // (Mac, Vlan)
            req->mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_MAC_VLAN);
        } else if (!_vlan) {
            // Mac
            req->mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_MAC);
        } else if (!_mac) {
            // Vlan
            req->mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF_VLAN);
        } else {
            // Lif,*,*
            req->mutable_key_or_handle()->mutable_filter_key()->set_type(kh::FILTER_LIF);
        }
#endif

        status = hal->filter_delete(req_msg, rsp_msg);
        if (status.ok()) {
            rsp = rsp_msg.response(0);
            if (rsp.api_status() == types::API_STATUS_OK) {
                NIC_LOG_DEBUG("Delted Filter mac: {} vlan: {}",
                              _mac, _vlan);
            } else {
                NIC_LOG_ERR("Failed to delete Filter mac: {} vlan: {}. err: {}",
                            _mac, _vlan, rsp.api_status());
                ret = HAL_IRISC_RET_FAIL;
                goto end;
            }
        } else {
            NIC_LOG_ERR("Failed to delete Filter mac: {} vlan: {}. err: {}:{}",
                        _mac, _vlan, status.error_code(), status.error_message());
            ret = HAL_IRISC_RET_FAIL;
            goto end;
        }
    }

end:
    return ret;
}
