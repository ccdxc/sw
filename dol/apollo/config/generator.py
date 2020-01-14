#! /usr/bin/python3
import pdb
import sys
import time

import infra.common.parser as parser
import infra.common.timeprofiler as timeprofiler

from apollo.config.store import EzAccessStore
import apollo.config.resmgr as resmgr
import apollo.config.agent.api as agentapi

import apollo.config.objects.batch as batch
import apollo.config.objects.device as device
import apollo.config.objects.lmapping as lmapping
import apollo.config.objects.meter as meter
import apollo.config.objects.mirror as mirror
import apollo.config.objects.nexthop as nexthop
import apollo.config.objects.policy as policy
import apollo.config.objects.rmapping as rmapping
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
from infra.asic.model import ModelConnector
from infra.common.glopts import GlobalOptions

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
    ObjectInfo[agentapi.ObjectTypes.DHCPRELAY] = dhcp_relay
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

    # Generate Interface Configuration
    interface.client.GenerateObjects(node, None, topospec)

    # Generate Mirror session configuration before vnic
    mirror.client.GenerateObjects(node, topospec)

    # Generate VPC configuration
    vpc.client.GenerateObjects(node, topospec)

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
    nat_pb.client.ReadObjects(node)
    # lmapping.client.ReadObjects(node)
    # rmapping.client.ReadObjects(node)
    return

def __get_topo_file():
    topo_file = '%s.topo' % GlobalOptions.topology
    return topo_file

def __get_topo_path(default=False):
    pipeline = utils.GetPipelineName()
    if default:
        pipeline = 'apollo'
    topo_file = '%s/config/topology/%s/' % (pipeline, GlobalOptions.topology)
    return topo_file

def __get_topo_spec():
    topofile = __get_topo_file()
    topopaths = []
    # get pipeline specfic topo
    topopaths.append(__get_topo_path())
    # fallback to apollo topo
    topopaths.append(__get_topo_path(True))
    for path in topopaths:
        logger.info("Generating Configuration for Topology %s/%s" % (path, topofile))
        topospec = parser.ParseFile(path, topofile)
        if topospec:
            return topospec
    logger.error("Invalid topofile %s" % (topofile))
    assert(0)
    return None

def Main():
    timeprofiler.ConfigTimeProfiler.Start()

    logger.info("Initializing object info")
    __initialize_object_info()

    topospec = __get_topo_spec()
    if hasattr(topospec, "dutnode"):
        EzAccessStore.SetDUTNode(topospec.dutnode)
    
    for node in topospec.node:
        agentapi.Init(node.id)
    resmgr.Init()

    for node in topospec.node:
        __generate(node.id, node)

        logger.info("Creating objects in Agent for node ", node.id)
        __create(node.id)

        logger.info("Reading objects via Agent for node ", node.id)
        __read(node.id)

    timeprofiler.ConfigTimeProfiler.Stop()

    ModelConnector.ConfigDone()
    return

