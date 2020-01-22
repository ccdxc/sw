#! /usr/bin/python3
from apollo.config.store import EzAccessStore
import apollo.config.resmgr as resmgr
import apollo.config.utils as utils
import apollo.config.agent.api as agentapi
import apollo.config.generator as generator

from infra.common.logging import logger as logger
from infra.common.glopts import GlobalOptions
import infra.common.parser as parser

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
    topospec = __get_topo_spec()
    EzAccessStore.SetDUTNode(topospec.dutnode)
    agentapi.Init(topospec.dutnode)
    resmgr.Init()
    
    for node in topospec.node:
        generator.Main(node.id, node)
    return
