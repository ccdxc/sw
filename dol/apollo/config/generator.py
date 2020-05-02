#! /usr/bin/python3
import sys

import infra.common.timeprofiler as timeprofiler
import apollo.config.resmgr as resmgr
from apollo.config.agent.api import ObjectTypes as APIObjTypes
import apollo.config.agent.api as agentapi

from apollo.oper.oper import client as OperClient
from apollo.config.objects.batch import client as BatchClient
from apollo.config.objects.device import client as DeviceClient
from apollo.config.objects.port import client as PortClient
from apollo.config.objects.interface import client as InterfaceClient
from apollo.config.objects.nexthop import client as NexthopClient
from apollo.config.objects.nexthop_group import client as NHGroupClient
from apollo.config.objects.tunnel import client as TunnelClient
from apollo.config.objects.route import client as RouteTableClient
from apollo.config.objects.tag import client as TagClient
from apollo.config.objects.policy import client as PolicyClient
from apollo.config.objects.security_profile import client as SecurityProfileClient
from apollo.config.objects.lmapping import client as LmappingClient
from apollo.config.objects.rmapping import client as RmappingClient
from apollo.config.objects.vnic import client as VnicClient
from apollo.config.objects.subnet import client as SubnetClient
from apollo.config.objects.vpc import client as VpcClient
from apollo.config.objects.meter import client as MeterClient
from apollo.config.objects.mirror import client as MirrorClient
from apollo.config.objects.dhcprelay import client as DHCPRelayClient
from apollo.config.objects.dhcpproxy import client as DHCPProxyClient
from apollo.config.objects.nat_pb import client as NATPbClient
from apollo.config.objects.policer import client as PolicerClient
from apollo.config.objects.metaswitch.bgp import client as BGPClient
from apollo.config.objects.metaswitch.bgp_peer import client as BGPPeerClient
from apollo.config.objects.metaswitch.bgp_peeraf import client as BGPPeerAfClient
from apollo.config.objects.metaswitch.evpnevi import client as EvpnEviClient
from apollo.config.objects.metaswitch.evpnevirt import client as EvpnEviRtClient
from apollo.config.objects.metaswitch.evpnipvrf import client as EvpnIpVrfClient
from apollo.config.objects.metaswitch.evpnipvrfrt import client as EvpnIpVrfRtClient
from infra.common.glopts import GlobalOptions
from apollo.config.store import client as EzAccessStoreClient
import apollo.config.store as store
import apollo.config.utils as utils

from infra.common.logging import logger as logger

ObjectInfo = dict()

def __initialize_object_info():
    ObjectInfo[APIObjTypes.DEVICE.name.lower()] = DeviceClient
    ObjectInfo[APIObjTypes.TUNNEL.name.lower()] = TunnelClient
    ObjectInfo[APIObjTypes.INTERFACE.name.lower()] = InterfaceClient
    ObjectInfo[APIObjTypes.NEXTHOP.name.lower()] = NexthopClient
    ObjectInfo[APIObjTypes.NEXTHOPGROUP.name.lower()] = NHGroupClient
    ObjectInfo[APIObjTypes.VPC.name.lower()] = VpcClient
    ObjectInfo[APIObjTypes.SUBNET.name.lower()] = SubnetClient
    ObjectInfo[APIObjTypes.VNIC.name.lower()] = VnicClient
    ObjectInfo[APIObjTypes.LMAPPING.name.lower()] = LmappingClient
    ObjectInfo[APIObjTypes.RMAPPING.name.lower()] = RmappingClient
    ObjectInfo[APIObjTypes.ROUTE_TABLE.name.lower()] = RouteTableClient
    ObjectInfo[APIObjTypes.POLICY.name.lower()] = PolicyClient
    ObjectInfo[APIObjTypes.SECURITY_PROFILE.name.lower()] = SecurityProfileClient
    ObjectInfo[APIObjTypes.MIRROR.name.lower()] = MirrorClient
    ObjectInfo[APIObjTypes.METER.name.lower()] = MeterClient
    ObjectInfo[APIObjTypes.TAG.name.lower()] = TagClient
    ObjectInfo[APIObjTypes.DHCP_RELAY.name.lower()] = DHCPRelayClient
    ObjectInfo[APIObjTypes.DHCP_PROXY.name.lower()] = DHCPProxyClient
    ObjectInfo[APIObjTypes.NAT.name.lower()] = NATPbClient
    ObjectInfo[APIObjTypes.POLICER.name.lower()] = PolicerClient
    ObjectInfo[APIObjTypes.BGP.name.lower()] = BGPClient
    ObjectInfo[APIObjTypes.BGP_PEER.name.lower()] = BGPPeerClient
    ObjectInfo[APIObjTypes.BGP_PEER_AF.name.lower()] = BGPPeerAfClient
    ObjectInfo[APIObjTypes.BGP_EVPN_EVI.name.lower()] = EvpnEviClient
    ObjectInfo[APIObjTypes.BGP_EVPN_EVI_RT.name.lower()] = EvpnEviRtClient
    ObjectInfo[APIObjTypes.BGP_EVPN_IP_VRF.name.lower()] = EvpnIpVrfClient
    ObjectInfo[APIObjTypes.BGP_EVPN_IP_VRF_RT.name.lower()] = EvpnIpVrfRtClient
    ObjectInfo[APIObjTypes.OPER.name.lower()] = OperClient
    return

def __validate_object_config(node, client):
    res, err = client.IsValidConfig(node)
    if not res:
        logger.error(f"ERROR: {err}")
        sys.exit(1)
    return

def __validate(node):
    # Validate objects are generated within their scale limit
    for objtype in APIObjTypes:
        client = ObjectInfo.get(objtype.name.lower(), None)
        if not client:
            logger.error(f"Skipping scale validation for {objtype.name}")
            continue
        __validate_object_config(node, client)
    return

def __generate(node, topospec):
    # Generate Batch Object
    BatchClient.GenerateObjects(node, topospec)

    # Generate Device Configuration
    DeviceClient.GenerateObjects(node, topospec)

    # Generate Port Configuration
    PortClient.GenerateObjects(node, topospec)

    # Generate Host Interface Configuration
    InterfaceClient.GenerateHostInterfaces(node, topospec)

    # Generate Mirror session configuration before vnic
    MirrorClient.GenerateObjects(node, topospec)

    # Generate Policer config
    PolicerClient.GenerateObjects(node, topospec)

    # Generate security policy configuration
    SecurityProfileClient.GenerateObjects(node, topospec)

    # Generate VPC configuration
    VpcClient.GenerateObjects(node, topospec)

    # Generate Oper objects
    OperClient.GenerateObjects(node)

    # Validate configuration
    __validate(node)

    return

def __create(node):
    if utils.IsSkipSetup():
        logger.info("Skip Creating objects in pds-agent for node ", node)
        return

    logger.info("Creating objects in pds-agent for node ", node)
    # Start the Batch
    BatchClient.Start(node)

    # Create Device Object
    DeviceClient.CreateObjects(node)

    if (EzAccessStoreClient[node].IsDeviceOverlayRoutingEnabled()):
        # Cannot extend batch across controlplane hijacked objects
        # when Overlay routing is enabled
        # Commit the Batch
        BatchClient.Commit(node)
        # Start a new Batch
        BatchClient.Start(node)

    # Create Interface Objects
    InterfaceClient.CreateObjects(node)

    # Create VPC Objects
    VpcClient.CreateObjects(node)

    SecurityProfileClient.CreateObjects(node)

    NHGroupClient.CreateObjects(node)
    TunnelClient.CreateObjects(node)

    # Commit the Batch
    BatchClient.Commit(node)

    if not utils.IsDol() and GlobalOptions.netagent:
        SubnetClient.UpdateHostInterfaces(node)

    # Start separate batch for mirror
    # so that mapping gets programmed before mirror
    BatchClient.Start(node)

    # Create Mirror session objects
    MirrorClient.CreateObjects(node)

    # Commit the Batch
    BatchClient.Commit(node)

    # Create remote mapping objects one by one
    # RmappingClient.OperateObjects(node, 'Create')
    return

def __read(node):
    # Read all objects
    logger.info("Reading objects from pds-agent for node ", node)

    if (EzAccessStoreClient[node].IsDeviceOverlayRoutingEnabled()):
        logger.info("Wait 5 seconds for control-plane convergence")
        utils.Sleep(5)
    failingObjects = []
    # read & validate the objects
    for objtype in APIObjTypes:
        objname = objtype.name
        client = ObjectInfo.get(objname.lower(), None)
        if not client:
            logger.error(f"Skipping read validation for {objname}")
            continue
        if not client.IsReadSupported():
            logger.error(f"Read unsupported for {objname}")
            continue
        if not client.ReadObjects(node):
            logger.error(f"Read validation failed for {objname}")
            failingObjects.append(objname)
    if len(failingObjects):
        logger.error(f"Read validation failed for {failingObjects}")
        # assert here as there is no point in proceeding further
        assert(0)
    return

def Main(node, topospec, ip=None):
    logger.info("Initializing object info")
    __initialize_object_info()
    store.EzAccessStore.SetConfigClientDict(ObjectInfo)

    agentapi.Init(node, ip)

    resmgr.Init(node)
    store.Init(node)

    __generate(node, topospec)

    timeprofiler.ConfigTimeProfiler.Start()
    __create(node)
    timeprofiler.ConfigTimeProfiler.Stop()

    __read(node)

    if utils.IsDol():
        from infra.asic.model import ModelConnector
        ModelConnector.ConfigDone()
    return
