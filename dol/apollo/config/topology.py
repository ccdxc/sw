#! /usr/bin/python3
import infra.common.parser as parser
import apollo.config.generator as generator
import apollo.config.resmgr as resmgr
import apollo.config.utils as utils

from infra.common.logging import logger as logger
from infra.common.glopts import GlobalOptions
from apollo.config.store import EzAccessStore

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
    nodes = len(topospec.node)
    EzAccessStore.SetDUTNode(topospec.dutnode)

    for node in topospec.node:
        generator.Main(node.id, node)
    return
