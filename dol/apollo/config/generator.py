#! /usr/bin/python3
import sys

import infra.common.timeprofiler as timeprofiler
import apollo.config.resmgr as resmgr
import apollo.config.agent.api as agentapi

import apollo.config.objects.batch as batch
import apollo.config.objects.device as device
#import apollo.config.objects.lmapping as lmapping
import apollo.config.objects.meter as meter
import apollo.config.objects.mirror as mirror
import apollo.config.objects.nexthop as nexthop
import apollo.config.objects.policy as policy
#import apollo.config.objects.rmapping as rmapping
import apollo.config.objects.route as route
import apollo.config.objects.subnet as subnet
import apollo.config.objects.tag as tag
import apollo.config.objects.tunnel as tunnel
import apollo.config.objects.vnic as vnic
import apollo.config.objects.vpc as vpc
import apollo.config.objects.interface as interface
import apollo.config.objects.port as port
import apollo.config.objects.dhcprelay as dhcp_relay
import apollo.config.objects.nat_pb as nat_pb
import apollo.config.utils as utils

from infra.common.logging import logger as logger

ObjectInfo = [None] * agentapi.ObjectTypes.MAX

def __initialize_object_info():
    ObjectInfo[agentapi.ObjectTypes.DEVICE] = device
    ObjectInfo[agentapi.ObjectTypes.TUNNEL] = tunnel
    ObjectInfo[agentapi.ObjectTypes.VPC] = vpc
    ObjectInfo[agentapi.ObjectTypes.SUBNET] = subnet
    ObjectInfo[agentapi.ObjectTypes.VNIC] = vnic
    ObjectInfo[agentapi.ObjectTypes.ROUTE] = route
    ObjectInfo[agentapi.ObjectTypes.POLICY] = policy
    ObjectInfo[agentapi.ObjectTypes.MIRROR] = mirror
    ObjectInfo[agentapi.ObjectTypes.DHCP_RELAY] = dhcp_relay
    ObjectInfo[agentapi.ObjectTypes.NAT_PB] = nat_pb
    return

def __validate_object_config(node, objid):
    if ObjectInfo[objid] is None:
        return
    obj = ObjectInfo[objid]
    res, err = obj.client.IsValidConfig(node)
    if not res:
        logger.error("ERROR: %s" %(err))
        sys.exit(1)
    return

def __validate(node):
    # Validate objects are generated within their scale limit
    for objid in range(agentapi.ObjectTypes.MAX):
        __validate_object_config(node, objid)
    return

def __generate(node, topospec):
    # Generate Batch Object
    batch.client.GenerateObjects(node, topospec)

    # Generate Device Configuration
    device.client.GenerateObjects(node, topospec)

    # Generate Port Configuration
    port.client.GenerateObjects(node, topospec)

    if utils.IsDol():
        # Generate Interface Configuration
        interface.client.GenerateObjects(node, None, topospec)

    # Generate Mirror session configuration before vnic
    mirror.client.GenerateObjects(node, topospec)

    # Generate VPC configuration
    vpc.client.GenerateObjects(node, topospec)

    if utils.IsDol():
        # Generate DHCP configuration
        dhcp_relay.client.GenerateObjects(node, topospec)

    # Validate configuration
    __validate(node)

    return

def __create(node):
    logger.info("Creating objects in Agent")
    # Start the Batch
    batch.client.Start(node)

    # Create Device Object
    device.client.CreateObjects(node)

    # Create Interface Objects
    interface.client.CreateObjects(node)

    # Create VPC Objects
    vpc.client.CreateObjects(node)

    # Commit the Batch
    batch.client.Commit(node)

    # Start separate batch for mirror
    # so that mapping gets programmed before mirror
    batch.client.Start(node)

    # Create Mirror session objects
    mirror.client.CreateObjects(node)

    # Create DHCP Relay Objects
    dhcp_relay.client.CreateObjects(node)

    # Commit the Batch
    batch.client.Commit(node)
    return

def __read(node):
    # Read all objects
    logger.info("Reading objects via Agent")
    interface.client.ReadObjects(node)
    device.client.ReadObjects(node)
    vpc.client.ReadObjects(node)
    subnet.client.ReadObjects(node)
    vnic.client.ReadObjects(node)
    tunnel.client.ReadObjects(node)
    nexthop.client.ReadObjects(node)
    mirror.client.ReadObjects(node)
    meter.client.ReadObjects(node)
    policy.client.ReadObjects(node)
    tag.client.ReadObjects(node)
    route.client.ReadObjects(node)
    # dhcp_relay.client.ReadObjects(node)
    if utils.IsDol():
        nat_pb.client.ReadObjects(node)
    # lmapping.client.ReadObjects(node)
    # rmapping.client.ReadObjects(node)
    return

def Main(node, topospec, ip=None):
    timeprofiler.ConfigTimeProfiler.Start()

    logger.info("Initializing object info")
    __initialize_object_info()

    logger.info("Initializing Agent ", ip)
    agentapi.Init(node, ip)

    if utils.IsDol():
        resmgr.Init()

    __generate(node, topospec)

    logger.info("Creating objects in Agent for node ", node)
    __create(node)

    if utils.IsDol():
        logger.info("Reading objects via Agent for node ", node)
        __read(node)

    timeprofiler.ConfigTimeProfiler.Stop()

    if utils.IsDol():
        from infra.asic.model import ModelConnector
        ModelConnector.ConfigDone()
    return
