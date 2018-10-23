
#ifndef __MULTICAST_HPP__
#define __MULTICAST_HPP__

#include <vector>

#include "sdk/indexer.hpp"

#include "hal_types.hpp"
#include "hal.hpp"
#include "lif.hpp"
#include "vrf.hpp"
#include "l2seg.hpp"
#include "endpoint.hpp"


// (l2seg, mac)
typedef std::tuple<HalL2Segment *, mac_t> mcast_key_t;


class HalMulticast : public HalObject
{
public:
    static HalMulticast *Factory(HalL2Segment *l2seg, mac_t mac);
    static void Destroy(HalMulticast *mcast);

    static HalMulticast *GetInstance(HalL2Segment *l2seg, mac_t mac);

    void AddEnic(Enic *enic);

    void DelEnic(Enic *enic);

    void TriggerHal();

    HalL2Segment *GetL2Seg();
    mac_t GetMac();
    uint32_t GetNumEnics();

private:
    HalMulticast(HalL2Segment *l2seg,mac_t mac);
    ~HalMulticast();

    HalL2Segment *l2seg;
    mac_t mac;

    std::map<uint64_t, Enic*> enic_refs;

    static std::map<mcast_key_t, HalMulticast*> mcast_db;
};

#endif  /* __MULTICAST_HPP__ */
