//
//  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
//

#include <gen/proto/types.pb.h>
#include <gen/proto/debug.pb.h>
#include <gen/proto/nat.pb.h>
#include "nic/vpp/infra/ipc/uds_internal.h"
#include "pdsa_uds_hdlr.h"

bool
vpp_uds_nat_iterate(pds_nat_iterate_params_t *params)
{
    pds_nat_port_block_export_t *pb = params->pb;

    pds::NatPortBlock nat_msg = pds::NatPortBlock();
    pds::NatPortBlockSpec *spec = nat_msg.mutable_spec();
    pds::NatPortBlockStats *stats = nat_msg.mutable_stats();

    spec->set_id((const char *)pb->id, PDS_MAX_KEY_LEN);
    if (pb->address_type == NAT_ADDR_TYPE_INFRA) {
        spec->set_addresstype(types::ADDR_TYPE_SERVICE);
    } else if (pb->address_type == NAT_ADDR_TYPE_INTERNET) {
        spec->set_addresstype(types::ADDR_TYPE_PUBLIC);
    }
    auto nat_pfx = spec->mutable_nataddress()->mutable_prefix()->mutable_ipv4subnet();
    nat_pfx->mutable_addr()->set_v4addr(pb->addr);
    nat_pfx->mutable_addr()->set_af(types::IP_AF_INET);
    nat_pfx->set_len(32);
    spec->set_protocol(pb->protocol);
    spec->mutable_ports()->set_portlow(pb->start_port);
    spec->mutable_ports()->set_porthigh(pb->end_port);
    stats->set_inusecount(pb->in_use_cnt);
    stats->set_sessioncount(pb->session_cnt);
    nat_msg.SerializeToFileDescriptor(params->fd);
    return false;
}

// Callback to dump NAT entries via UDS
static void
vpp_uds_nat_dump(int fd)
{
    pds_nat_port_block_export_t pb;

    pds_nat_iterate_params_t params;

    params.itercb = vpp_uds_nat_iterate;
    params.pb = &pb;
    params.fd = fd;

    memset(pb.id, 0, sizeof(pb.id));

    nat_pb_iterate(&params);

    pds::NatPortBlock nat_msg = pds::NatPortBlock();
    pds::NatPortBlockSpec *spec = nat_msg.mutable_spec();
    // Last message
    spec->set_protocol(0);
    nat_msg.SerializeToFileDescriptor(fd);
}

// initializes callbacks for flow dump
void
pds_nat_dump_init(void)
{
    vpp_uds_register_cb(VPP_UDS_NAT_PB_DUMP, vpp_uds_nat_dump);
}
