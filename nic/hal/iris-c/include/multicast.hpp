
#ifndef __MULTICAST_HPP__
#define __MULTICAST_HPP__

#include <vector>

#include "sdk/indexer.hpp"

#include "types.hpp"
#include "hal.hpp"
#include "lif.hpp"
#include "vrf.hpp"
#include "l2seg.hpp"
#include "endpoint.hpp"


// (l2seg, mac)
typedef std::tuple<L2Segment *, mac_t> mcast_key_t;


class Multicast : public HalObject
{
public:
    static Multicast *Factory(L2Segment *l2seg, mac_t mac);
    static void Destroy(Multicast *mcast);

    static Multicast *GetInstance(L2Segment *l2seg, mac_t mac);

    void AddEnic(Enic *enic);

    void DelEnic(Enic *enic);

    void TriggerHal();

    L2Segment *GetL2Seg();
    mac_t GetMac();
    uint32_t GetNumEnics();

private:
    Multicast(L2Segment *l2seg,mac_t mac);
    ~Multicast();

    L2Segment *l2seg;
    mac_t mac;

    std::map<uint64_t, Enic*> enic_refs;

    static std::map<mcast_key_t, Multicast*> mcast_db;
};

#endif  /* __MULTICAST_HPP__ */
