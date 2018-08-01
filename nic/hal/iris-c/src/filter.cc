

#include "filter.hpp"

using namespace std;


static bool
is_multicast(uint64_t mac) {
    return ((mac & 0x010000000000) == 0x010000000000);
}

/**
 * MAC-VLAN filter
 */
MacVlanFilter::MacVlanFilter(
    shared_ptr<Lif> lif,
    shared_ptr<Vrf> vrf,
    mac_t mac, vlan_t vlan)
{
    _mac = mac;
    _vlan = vlan;

    vrf_ref = vrf;
    lif_ref = lif;

    if (hal->GetMode() == FWD_MODE_CLASSIC) {
        enic_ref = Enic::GetInstance(lif, vrf);
        if (is_multicast(mac)) {
            mcast_ref = Multicast::GetInstance(vrf, mac, vlan);
            mcast_ref->AddEnic(enic_ref);
        } else {
            enic_ref->AddVlan(vlan);
            ep_ref = make_shared<Endpoint>(enic_ref, vrf, mac, vlan);
        }
    }

    if (hal->GetMode() == FWD_MODE_HOSTPIN) {
        enic_ref = Enic::GetInstance(lif, vrf, mac, vlan);
        ep_ref = make_shared<Endpoint>(enic_ref, vrf, mac, vlan);
    }
}

MacVlanFilter::~MacVlanFilter()
{
    if (hal->GetMode() == FWD_MODE_CLASSIC) {
        if (is_multicast(_mac)) {
            mcast_ref->DelEnic(enic_ref);
            mcast_ref.reset();
        } else {
            enic_ref->DelVlan(_vlan);
            ep_ref.reset();
        }
    }

    if (hal->GetMode() == FWD_MODE_HOSTPIN) {
        ep_ref.reset();
    }

    enic_ref.reset();
    lif_ref.reset();
    vrf_ref.reset();
}
