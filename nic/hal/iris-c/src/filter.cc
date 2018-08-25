

#include "filter.hpp"
#include "ethlif.hpp"
#include "print.hpp"

using namespace std;


static bool
is_multicast(uint64_t mac) {
    return ((mac & 0x010000000000) == 0x010000000000);
}

MacVlanFilter *
MacVlanFilter::Factory(EthLif *eth_lif, mac_t mac, vlan_t vlan,
                       filter_type_t type)
{
    MacVlanFilter *filter = new MacVlanFilter(eth_lif, mac, vlan, type);

    return filter;
}

void
MacVlanFilter::Destroy(MacVlanFilter *filter)
{
    if (filter) {
        filter->~MacVlanFilter();
    }
}


/**
 * MAC-VLAN filter
 */
MacVlanFilter::MacVlanFilter(
    EthLif *eth_lif,
    mac_t mac, vlan_t vlan, filter_type_t type)
{
    grpc::Status                status;
    endpoint::FilterSpec        *req;
    endpoint::FilterResponse    rsp;
    endpoint::FilterRequestMsg  req_msg;
    endpoint::FilterResponseMsg rsp_msg;
    Enic                        *enic;
    L2Segment                   *l2seg;

    _type = type;
    _mac = mac;
    _vlan = vlan;
    this->eth_lif = eth_lif;

    HAL_TRACE_DEBUG("Mac-Vlan entity creation. Type: {}, lif: {}, mac: {}, vlan: {}. ",
                    _type,
                    eth_lif->GetLif()->GetId(),
                    macaddr2str(mac), vlan);

    if (hal->GetMode() == FWD_MODE_CLASSIC) {
        // Unicast:
        //  - Add vlan to Enic. May have already been added.
        //  - Create EP
        // Multicast:
        //  - Add Enic to multicast
        enic = eth_lif->GetEnic();
        enic->AddVlan(vlan);
        l2seg = enic->GetL2seg(vlan);
        if (!l2seg) {
            HAL_TRACE_ERR("l2seg should have been added in AddVlan. Please check.");
            assert(0);
        }
        if (is_multicast(mac)) {
            mcast = Multicast::GetInstance(l2seg, mac);
            mcast->AddEnic(enic);
        } else {
            ep = Endpoint::Factory(l2seg, mac, enic);
        }
    } else {
        // Create Filter to HAL
        req = req_msg.add_request();
        req->mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->set_lif_id(eth_lif->GetLif()->GetId());
        req->mutable_key_or_handle()->mutable_filter_key()->set_mac_address(mac);
        req->mutable_key_or_handle()->mutable_filter_key()->set_vlan_id(vlan);

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
                handle = rsp.filter_status().filter_handle();
                HAL_TRACE_DEBUG("Created Filter mac: {} vlan: {}, handle: {}",
                                mac, vlan, handle);
            } else {
                HAL_TRACE_ERR("Failed to create Filter mac: {} vlan: {}. err: {}",
                              mac, vlan, rsp.api_status());
            }
        } else {
            HAL_TRACE_ERR("Failed to create Filter mac: {} vlan: {}. err: {}:{}",
                          mac, vlan, status.error_code(), status.error_message());
        }

        // Store spec
        spec.CopyFrom(*req);
    }
}

MacVlanFilter::~MacVlanFilter()
{
    grpc::ClientContext           context;
    grpc::Status                  status;

    endpoint::FilterDeleteRequest        *req;
    endpoint::FilterDeleteResponse       rsp;
    endpoint::FilterDeleteRequestMsg     req_msg;
    endpoint::FilterDeleteResponseMsg    rsp_msg;
    Enic                                 *enic;
    L2Segment                            *l2seg;
    Endpoint                             *ep;

    HAL_TRACE_DEBUG("Mac-Vlan entity deletion. lif: {}, mac: {}, vlan: {}. ",
                    eth_lif->GetLif()->GetId(),
                    macaddr2str(_mac), _vlan);

    if (hal->GetMode() == FWD_MODE_CLASSIC) {
        enic = eth_lif->GetEnic();
        l2seg = enic->GetL2seg(_vlan);
        if (is_multicast(_mac)) {
            mcast = Multicast::GetInstance(l2seg, _mac);
            mcast->DelEnic(enic);

            // Delete Mcast entry if #enics become 0.
            if (!mcast->GetNumEnics()) {
                Multicast::Destroy(mcast);
            }
        } else {
            ep = Endpoint::Lookup(l2seg, _mac);
            Endpoint::Destroy(ep);
        }
        enic->DelVlan(_vlan);
    } else {
        // Create Filter to HAL
        req = req_msg.add_request();
        req->mutable_key_or_handle()->mutable_filter_key()->mutable_lif_key_or_handle()->
            set_lif_id(eth_lif->GetLif()->GetId());
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
                HAL_TRACE_DEBUG("Delted Filter mac: {} vlan: {}, handle: {}",
                                _mac, _vlan, handle);
            } else {
                HAL_TRACE_ERR("Failed to delete Filter mac: {} vlan: {}. err: {}",
                              _mac, _vlan, rsp.api_status());
            }
        } else {
            HAL_TRACE_ERR("Failed to delete Filter mac: {} vlan: {}. err: {}:{}",
                          _mac, _vlan, status.error_code(), status.error_message());
        }
    }
}
