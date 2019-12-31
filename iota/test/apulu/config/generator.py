#! /usr/bin/python3
import pdb
import sys
import time
import os
import json
from collections import defaultdict

import iota.test.apulu.config.init as init

#Following come from dol/infra
import infra.common.defs as defs
import infra.common.utils as utils
import infra.common.glopts as glopts
class GlobalOptions: pass
glopts.GlobalOptions = GlobalOptions()
glopts.GlobalOptions.debug = True
glopts.GlobalOptions.verbose = True

import infra.common.parser as parser
import infra.common.timeprofiler as timeprofiler

import iota.test.apulu.config.resmgr as resmgr
import iota.test.apulu.config.agent.api as agentapi

import iota.test.apulu.config.objects.batch as batch
import iota.test.apulu.config.objects.device as device
import iota.test.apulu.config.objects.lmapping as lmapping
import iota.test.apulu.config.objects.meter as meter
import iota.test.apulu.config.objects.mirror as mirror
import iota.test.apulu.config.objects.nexthop as nexthop
import iota.test.apulu.config.objects.policy as policy
import iota.test.apulu.config.objects.rmapping as rmapping
import iota.test.apulu.config.objects.route as route
import iota.test.apulu.config.objects.subnet as subnet
import iota.test.apulu.config.objects.tag as tag
import iota.test.apulu.config.objects.tunnel as tunnel
import iota.test.apulu.config.objects.vnic as vnic
import iota.test.apulu.config.objects.vpc as vpc
import iota.test.apulu.config.objects.interface as interface
import iota.test.apulu.config.objects.port as port
import iota.test.apulu.config.utils as utils
import iota.harness.api as api

from infra.common.logging import logger as logger
#from infra.asic.model import ModelConnector
from iota.test.apulu.config.store import Store
from iota.harness.infra.glopts import GlobalOptions

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
    batch.client.GenerateObjects(node)

    # Generate Device Configuration
    device.client.GenerateObjects(node, topospec)

    # Generate Port Configuration
    #port.client.GenerateObjects(topospec)

    # Generate Interface Configuration
    #interface.client.GenerateObjects(node, topospec)

    # Generate Mirror session configuration before vnic
    mirror.client.GenerateObjects(node, topospec)

    # Generate VPC configuration
    vpc.client.GenerateObjects(node, topospec)

    # Validate configuration
    __validate(node)

    return

def __create(node):
    # Start the Batch
    batch.client.Start(node)

    # Create Device Object
    device.client.CreateObjects(node)

    # Create Interface Objects
    #interface.client.CreateObjects(node)

    # Create VPC Objects
    vpc.client.CreateObjects(node)

    # Commit the Batch
    batch.client.Commit(node)

    """
    # Start separate batch for mirror
    # so that mapping gets programmed before mirror
    batch.client.Start()

    # Create Mirror session objects
    mirror.client.CreateObjects()

    # Commit the Batch
    batch.client.Commit()
    """
    return

def __read(node):
    # Read all objects
    interface.client.ReadObjects()
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

def Main(args):
    timeprofiler.ConfigTimeProfiler.Start()
    #Store.reset()
    #Have to look into config push

    node = args.node
    agentapi.Init(node, api.GetNicMgmtIP(node)) 

    #resmgr.Init()

    logger.info("Initializing object info")
    __initialize_object_info()

    defs.DOL_PATH = "/iota/"
    logger.info("Generating Configuration for Spec  = %s" % args.spec)
    if args.spec == 'dummy':
        return api.types.status.SUCCESS
    cfgpec = parser.ParseFile('test/apulu/config/cfg/', '%s'%args.spec)
    __generate(node, cfgpec)
    
    logger.info("Creating objects in Agent")
    __create(node)

    logger.info("Reading objects via Agent")
    # __read(node)

    timeprofiler.ConfigTimeProfiler.Stop()

    #ModelConnector.ConfigDone()
    return api.types.status.SUCCESS

