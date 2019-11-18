#! /usr/bin/python3
import pdb
import sys
import time

import infra.common.parser as parser
import infra.common.timeprofiler as timeprofiler

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
import apollo.config.utils as utils

from infra.common.logging import logger as logger
from infra.asic.model import ModelConnector
from apollo.config.store import Store
from infra.common.glopts import GlobalOptions

ObjectInfo = [None] * agentapi.ObjectTypes.MAX

def __initialize_object_info():
    ObjectInfo[agentapi.ObjectTypes.SWITCH] = device
    ObjectInfo[agentapi.ObjectTypes.TUNNEL] = tunnel
    ObjectInfo[agentapi.ObjectTypes.VPC] = vpc
    ObjectInfo[agentapi.ObjectTypes.SUBNET] = subnet
    ObjectInfo[agentapi.ObjectTypes.VNIC] = vnic
    ObjectInfo[agentapi.ObjectTypes.ROUTE] = route
    ObjectInfo[agentapi.ObjectTypes.POLICY] = policy
    ObjectInfo[agentapi.ObjectTypes.MIRROR] = mirror
    return

def __validate_object_config(objid):
    if ObjectInfo[objid] is None:
        return
    obj = ObjectInfo[objid]
    res, err = obj.client.IsValidConfig()
    if not res:
        logger.error("ERROR: %s" %(err))
        sys.exit(1)
    return

def __validate():
    # Validate objects are generated within their scale limit
    for objid in range(agentapi.ObjectTypes.MAX):
        __validate_object_config(objid)
    return

def __generate(topospec):
    # Generate Batch Object
    batch.client.GenerateObjects(topospec)

    # Generate Device Configuration
    device.client.GenerateObjects(topospec)

    # Generate Port Configuration
    port.client.GenerateObjects(topospec)

    # Generate Interface Configuration
    interface.client.GenerateObjects(topospec)

    # Generate Mirror session configuration before vnic
    mirror.client.GenerateObjects(topospec)

    # Generate VPC configuration
    vpc.client.GenerateObjects(topospec)

    # Validate configuration
    __validate()

    return

def __create():
    # Start the Batch
    batch.client.Start()

    # Create Device Object
    device.client.CreateObjects()

    # Create Interface Objects
    interface.client.CreateObjects()

    # Create VPC Objects
    vpc.client.CreateObjects()

    # Commit the Batch
    batch.client.Commit()

    # Start separate batch for mirror
    # so that mapping gets programmed before mirror
    batch.client.Start()

    # Create Mirror session objects
    mirror.client.CreateObjects()

    # Commit the Batch
    batch.client.Commit()
    return

def __read():
    # Read all objects
    device.client.ReadObjects()
    vpc.client.ReadObjects()
    subnet.client.ReadObjects()
    vnic.client.ReadObjects()
    tunnel.client.ReadObjects()
    nexthop.client.ReadObjects()
    mirror.client.ReadObjects()
    meter.client.ReadObjects()
    policy.client.ReadObjects()
    tag.client.ReadObjects()
    route.client.ReadObjects()
    # lmapping.client.ReadObjects()
    # rmapping.client.ReadObjects()
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
    agentapi.Init()

    resmgr.Init()

    logger.info("Initializing object info")
    __initialize_object_info()

    topospec = __get_topo_spec()
    __generate(topospec)

    logger.info("Creating objects in Agent")
    __create()

    if not utils.IsPipelineApulu():
        logger.info("Reading objects via Agent")
        __read()

    timeprofiler.ConfigTimeProfiler.Stop()

    ModelConnector.ConfigDone()
    return

