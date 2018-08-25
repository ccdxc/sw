
#include <iostream>
#include <grpc++/grpc++.h>

#include "types.grpc.pb.h"

#include "uplink.hpp"
#include "vrf.hpp"
#include "print.hpp"

using namespace std;

std::map<uint64_t, Uplink*> Uplink::uplink_db;

Uplink *
Uplink::Factory(uplink_id_t id)
{
    api_trace("Uplink Create");

    if (uplink_db.find(id) != uplink_db.end()) {
        HAL_TRACE_WARN("Duplicate Create of Uplink with id: {}",
                       id);
        return NULL;
    }


    Uplink *uplink = new Uplink(id);

    // Store in DB for disruptive restart
    uplink_db[uplink->GetId()] = uplink;

    // Classic: Create VRF for each uplink
    if (hal->GetMode() == FWD_MODE_CLASSIC) {
        uplink->vrf = Vrf::Factory();
    }

    return uplink;
}

void
Uplink::Destroy(Uplink *uplink)
{
    api_trace("Uplink Delete");
    if (uplink) {
        uplink->~Uplink();
    }
}


Uplink::Uplink(uplink_id_t id)
{
    HAL_TRACE_DEBUG("Uplink Create: {}", id);
    this->id = id;
}

Uplink::~Uplink()
{
    // Remove from DB
    uplink_db.erase(id);

    // Delete Vrf
    Vrf::Destroy(vrf);
}

uint32_t
Uplink::GetId()
{
    return id;
}

uint64_t
Uplink::GetHandle()
{
    return handle;
}

uint32_t
Uplink::GetPortNum()
{
    return port_num;
}

Vrf *
Uplink::GetVrf()
{
    return vrf;
}

void
Uplink::SetPortNum()
{
    this->port_num = port_num;
}
