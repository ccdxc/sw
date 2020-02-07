#! /usr/bin/python3
import sys

import infra.common.timeprofiler as timeprofiler
import apollo.config.resmgr as resmgr
from apollo.config.agent.api import ObjectTypes as APIObjTypes
import apollo.config.agent.api as agentapi

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
from apollo.config.objects.lmapping import client as LmappingClient
from apollo.config.objects.rmapping import client as RmappingClient
from apollo.config.objects.vnic import client as VnicClient
from apollo.config.objects.subnet import client as SubnetClient
from apollo.config.objects.vpc import client as VpcClient
from apollo.config.objects.meter import client as MeterClient
from apollo.config.objects.mirror import client as MirrorClient
from apollo.config.objects.dhcprelay import client as DHCPRelayClient
from apollo.config.objects.nat_pb import client as NATPbClient
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
    ObjectInfo[APIObjTypes.ROUTE.name.lower()] = RouteTableClient
    ObjectInfo[APIObjTypes.POLICY.name.lower()] = PolicyClient
    ObjectInfo[APIObjTypes.MIRROR.name.lower()] = MirrorClient
    ObjectInfo[APIObjTypes.DHCP_RELAY.name.lower()] = DHCPRelayClient
    ObjectInfo[APIObjTypes.NAT_PB.name.lower()] = NATPbClient
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

    if utils.IsDol():
        # Generate Host Interface Configuration
        InterfaceClient.GenerateHostInterfaces(node, topospec)

    # Generate Mirror session configuration before vnic
    MirrorClient.GenerateObjects(node, topospec)

    # Generate VPC configuration
    VpcClient.GenerateObjects(node, topospec)

    # Generate DHCP configuration
    DHCPRelayClient.GenerateObjects(node, topospec)

    # Validate configuration
    __validate(node)

    return

def __create(node):
    logger.info("Creating objects in pds-agent for node ", node)
    # Start the Batch
    BatchClient.Start(node)

    # Create Device Object
    DeviceClient.CreateObjects(node)

    # Create Interface Objects
    InterfaceClient.CreateObjects(node)

    # Create VPC Objects
    VpcClient.CreateObjects(node)

    # Commit the Batch
    BatchClient.Commit(node)

    # Start separate batch for mirror
    # so that mapping gets programmed before mirror
    BatchClient.Start(node)

    # Create Mirror session objects
    MirrorClient.CreateObjects(node)

    # Create DHCP Relay Objects
    DHCPRelayClient.CreateObjects(node)

    # Commit the Batch
    BatchClient.Commit(node)
    return

def __read(node):
    # Read all objects
    logger.info("Reading objects from pds-agent for node ", node)
    # TODO - assert if read fails
    InterfaceClient.ReadObjects(node)
    DeviceClient.ReadObjects(node)
    VpcClient.ReadObjects(node)
    SubnetClient.ReadObjects(node)
    VnicClient.ReadObjects(node)
    TunnelClient.ReadObjects(node)
    NexthopClient.ReadObjects(node)
    MirrorClient.ReadObjects(node)
    MeterClient.ReadObjects(node)
    PolicyClient.ReadObjects(node)
    TagClient.ReadObjects(node)
    RouteTableClient.ReadObjects(node)
    # DHCPRelayClient.ReadObjects(node)
    NATPbClient.ReadObjects(node)
    # LmappingClient.ReadObjects(node)
    # RmappingClient.ReadObjects(node)
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
