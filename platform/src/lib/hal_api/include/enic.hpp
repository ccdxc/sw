
#ifndef __ENIC_HPP__
#define __ENIC_HPP__

#include <vector>

#include "lib/indexer/indexer.hpp"

#include "hal_types.hpp"
#include "hal.hpp"
#include "vrf.hpp"
#include "l2seg.hpp"


// lif
typedef uint64_t enic_classic_key_t;

typedef struct l2seg_info_s {
    HalL2Segment *l2seg;
    uint32_t filter_ref_cnt;
} l2seg_info_t;


class Enic : public HalObject
{
public:
    static Enic *Factory(Lif *ethlif);
    static hal_irisc_ret_t Destroy(Enic *enic);

    hal_irisc_ret_t HalEnicCreate();
    hal_irisc_ret_t HalEnicDelete();


    // Classic ENIC APIs only
    hal_irisc_ret_t AddVlan(vlan_t vlan);
    void DelVlan(vlan_t vlan, bool skip_vlan = false);

    ~Enic() {};

    uint64_t GetId();
    HalL2Segment *GetL2seg(vlan_t vlan);

private:
    Enic(Lif *ethlif);

    uint32_t id;

    mac_t _mac;
    vlan_t _vlan;

    intf::InterfaceSpec spec;

    Lif *ethlif;
    std::map<vlan_t, l2seg_info_t *> l2seg_refs;

    static sdk::lib::indexer *allocator;
    static constexpr uint32_t max_enics = 4096;

    hal_irisc_ret_t TriggerHalUpdate();
};

#endif /* __ENIC_HPP__ */
