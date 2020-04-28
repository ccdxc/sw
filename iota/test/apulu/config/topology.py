#! /usr/bin/python3
import iota.test.apulu.config.init as init

#Following come from dol/infra
import infra.common.defs as defs
from iota.harness.infra.glopts import GlobalOptions
import infra.common.glopts as glopts
glopts.GlobalOptions = GlobalOptions
import infra.common.parser as parser
import iota.harness.api as api
from infra.common.logging import logger as logger
import apollo.config.generator as generator
from apollo.config.store import EzAccessStore
from apollo.config.store import client as EzAccessStoreClient
from apollo.config.store import Init as EzAccessStoreInit
import infra.common.objects as objects
import apollo.config.utils as utils
import iota.test.apulu.config.api as config_api
import iota.test.apulu.utils.learn as learn_utils

def __generate_rmappings_from_lmappings():
    nodes = api.GetNaplesHostnames()
    lmapClient = config_api.GetObjClient('lmapping')
    rmapClient = config_api.GetObjClient('rmapping')
    rmapSpec = dict()
    for targetNode in nodes:
        isOverlayRoutingEnabled = EzAccessStoreClient[targetNode].IsDeviceOverlayRoutingEnabled()
        if isOverlayRoutingEnabled:
            rmapSpec['origin'] = 'discovered'
        else:
            rmapSpec['origin'] = 'fixed'
        if rmapClient.GetNumObjects(targetNode) != 0:
            # Skip since Remote mapping configs are already generated from cfgspec
            api.Logger.verbose(f"Skipping Remote mapping generation "
                               f"since they already exist")
            continue
        for srcNode in nodes:
            num = 0
            if targetNode == srcNode:
                continue
            lmappings = lmapClient.Objects(srcNode)
            for lmap in lmappings:
                if lmap.VNIC.IsIgwVnic():
                    api.Logger.verbose(f"Skipping Remote mapping generation "
                                       f"for IgwVnic's lmapping {lmap.GID()}")
                    continue
                mac = "macaddr/%s" %lmap.VNIC.MACAddr.get()
                rmapSpec['rmacaddr'] = objects.TemplateFieldObject(mac)
                rmapSpec['ripaddr'] = lmap.IP
                if lmap.AddrFamily == 'IPV6':
                    ipversion = utils.IP_VERSION_6
                else:
                    ipversion = utils.IP_VERSION_4
                parent = config_api.GetObjClient('subnet').GetSubnetObject(targetNode, lmap.VNIC.SUBNET.SubnetId)
                rmapClient.GenerateObj(targetNode, parent, rmapSpec, ipversion)
                num += 1
            api.Logger.info(f"Generated {num} RMAPPING Objects in {targetNode}")
        # Create the generated objects if their origin is fixed.
        if not isOverlayRoutingEnabled:
            if not rmapClient.CreateObjects(targetNode):
                api.Logger.error(f"Failed to create remote mappings on "
                                 f"{targetNode}, Aborting!!!")
                return False
            if not rmapClient.ReadObjects(targetNode):
                api.Logger.error(f"Failed to read remote mappings on "
                                 f"{targetNode}, Aborting!!!")
                return False
    return True

def __update_nexthops_from_uplink_info():
    nodes = api.GetNaplesHostnames()
    nhClient = config_api.GetObjClient('nexthop')
    interfaceClient = config_api.GetObjClient('interface')
    for targetNode in nodes:
        nexthops = nhClient.GetUnderlayNexthops(targetNode)
        if len(nexthops) == 0:
            # Skip updating nexthops if none of them are created via config
            api.Logger.verbose(f"Skipping nexthops updates as "
                               f"none exist in {targetNode}")
            continue
        num = 0
        for nh in nexthops:
            api.Logger.verbose(f"Updating NH {nh} in node {targetNode}")
            for srcNode in nodes:
                if targetNode == srcNode:
                    continue
                peer_intf_mac = interfaceClient.GetNHInterfaceMac(srcNode, 'dsc0')
                if peer_intf_mac is None:
                    # ignore for dry run
                    api.Logger.error(f"Failed to retrive dsc0 intf mac from "
                                     f"{srcNode} - Skip updating NH {nh} "
                                     f"in {targetNode}")
                    continue
                api.Logger.debug(f"Updating NH {nh} with intf-mac "
                                 f"{peer_intf_mac} of {srcNode}")
                nh.underlayMACAddr = peer_intf_mac
                num += 1
        nhClient.UpdateUnderlayObjects(targetNode)
        api.Logger.debug(f"Updated {num} NH Objects in {targetNode}")
    return

def Main(args):
    defs.DOL_PATH = "/iota/"
    defs.TEST_TYPE = "IOTA"
    dhcprelayspec = api.GetDHCPRelayInfo()
    EzAccessStore.SetDHCPRelayInfo(dhcprelayspec)
    for node, cfgyml in vars(args.spec).items():
        api.Logger.info(f"Generating Configuration for Spec {cfgyml}")
        cfgspec = parser.ParseFile('test/apulu/config/cfg/', f'{cfgyml}')

        naples_uuid_map = api.GetNaplesNodeUuidMap()
        EzAccessStoreInit(node)
        EzAccessStore.SetUuidMap(naples_uuid_map)
        EzAccessStoreClient[node].SetUnderlayIPs(api.GetNicUnderlayIPs(node))

        generator.Main(node, cfgspec, api.GetNicMgmtIP(node))

    if not __generate_rmappings_from_lmappings():
        api.Logger.error("Failed to convert local mappings to remote mappings")
        return api.types.status.FAILURE

    # Update static NextHop objects with the mac-addresses of the peer's interfaces
    # This is temporary until the dynamic underlay NH stitching support comes in soon.
    __update_nexthops_from_uplink_info()

    for node in api.GetNaplesHostnames():
        storeClient = EzAccessStoreClient[node]
        if storeClient.IsDeviceLearningEnabled():
            if not learn_utils.SetDeviceLearnTimeout(storeClient.GetDevice().LearnAgeTimeout):
                return api.types.status.FAILURE
            if not learn_utils.ClearLearnData():
                return api.types.status.FAILURE

    return api.types.status.SUCCESS
