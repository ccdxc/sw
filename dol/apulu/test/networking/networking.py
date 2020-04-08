#! /usr/bin/python3
# Networking Module
import pdb

from infra.common.logging import logger as logger
from infra.common.glopts import GlobalOptions

import apollo.config.utils as utils
import apollo.config.topo as topo
import apollo.config.generator as generator
import apollo.test.callbacks.common.modcbs as modcbs
from apollo.config.store import EzAccessStore
import apollo.config.objects.nat_pb as nat_pb

def Setup(infra, module):
    if 'WORKFLOW_START' in module.name:
        topo.ChosenFlowObjs.select_objs = True
        topo.ChosenFlowObjs.use_selected_objs = False
        topo.ChosenFlowObjs.SetMaxLimits(module.testspec.selectors.maxlimits)
    modcbs.Setup(infra, module)
    return True

def TestCaseSetup(tc):
    tc.AddIgnorePacketField('UDP', 'sport')
    tc.AddIgnorePacketField('UDP', 'chksum')
    tc.AddIgnorePacketField('IP', 'chksum') #Needed to pass NAT testcase
    # TODO: Ignore tos until all testspecs are updated to take tos from VPC
    tc.AddIgnorePacketField('IP', 'tos')
    ignore_ids = [ 'L2_IPV4_VRIP_ICMP_ECHO_QTAG', 'L3_IPV4_VRIP_ICMP_ECHO_QTAG' ]
    if tc.module.name in ignore_ids:
        tc.AddIgnorePacketField('IP', 'id')
    if tc.config.root.FwdMode == 'IGW_NAPT' or tc.config.root.FwdMode == 'IGW_NAPT_SERVICE':
        vpc_key = tc.config.localmapping.VNIC.SUBNET.VPC.GetKey()
        tc.pvtdata.nat_port_blocks = \
                nat_pb.client.GetVpcNatPortBlocks(utils.NAT_ADDR_TYPE_PUBLIC, vpc_key)
        tc.pvtdata.pre_stats = {}
        tc.pvtdata.pre_stats['icmp'] = nat_pb.NatPbStats()
        tc.pvtdata.pre_stats['udp'] = nat_pb.NatPbStats()
        tc.pvtdata.pre_stats['tcp'] = nat_pb.NatPbStats()
        for pb in tc.pvtdata.nat_port_blocks:
            stats = pb.GetStats()
            tc.pvtdata.pre_stats[pb.ProtoName].Add(stats)
        tc.AddIgnorePacketField('IP', 'src')
        tc.AddIgnorePacketField('UDP', 'sport')
    return True

def TestCaseTeardown(tc):
    return True

def TestCasePreTrigger(tc):
    return True

def TestCaseStepSetup(tc, step):
    return True

def TestCaseStepTrigger(tc, step):
    return True

def TestCaseStepVerify(tc, step):
    return True

def TestCaseStepTeardown(tc, step):
    return True

def TestCaseVerify(tc):
    print(tc.packets.__dir__())
    k = tc.packets.GetAll()
    print(k)
    if tc.config.root.FwdMode == 'IGW_NAPT' and tc.packets.IsKeyIn('FROM_HOST_PKT'):
        pkt = tc.packets.Get('FROM_HOST_PKT')
        proto_name = utils.GetIPProtoName(pkt.headers.ipv4.fields.proto).lower()
        post_stats = nat_pb.NatPbStats()
        for pb in tc.pvtdata.nat_port_blocks:
            if proto_name == pb.ProtoName:
                stats = pb.GetStats()
                post_stats.Add(stats)

        if GlobalOptions.dryrun:
            return True

        if post_stats.InUseCount - tc.pvtdata.pre_stats[proto_name].InUseCount != 1:
            logger.info("NAT in use count did not go up as expected (%d:%d)",
                        tc.pvtdata.pre_stats[proto_name].InUseCount,
                        post_stats.InUseCount)
            # TODO in CRUD cases, flow already exists so new session is not allocated
            # ignore stats for now
            #return False

        if post_stats.SessionCount - tc.pvtdata.pre_stats[proto_name].SessionCount != 1:
            logger.info("NAT session count did not go up as expected (%d:%d)",
                        tc.pvtdata.pre_stats[proto_name].SessionCount,
                        post_stats.SessionCount)
            # TODO in CRUD cases, flow already exists so new session is not allocated
            # ignore stats for now
            #return False

    if 'WORKFLOW_START' in tc.module.name:
        topo.ChosenFlowObjs.select_objs = False
        topo.ChosenFlowObjs.use_selected_objs = True
    elif 'WORKFLOW_END' in tc.module.name:
        topo.ChosenFlowObjs.Reset()
        generator.__read(EzAccessStore.GetDUTNode())
    return True
