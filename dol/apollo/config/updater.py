import infra.common.defs as defs
import infra.common.utils as utils
import infra.common.parser as parser
import infra.common.timeprofiler as timeprofiler

import apollo.config.objects.batch as batch
import apollo.config.objects.device as device

import apollo.config.agent.api as agentapi

from infra.common.logging import logger as logger
from infra.asic.model import ModelConnector
from apollo.config.store import Store
from infra.common.glopts import GlobalOptions

def __modify():
    # Generate Batch Object
    # batch.client.GenerateObjects(topospec)

    device.client.ModifyObjects()
    #TODO for other objects

def __update():
    # Start the Batch
    batch.client.Start()

    # Update Device Object
    device.client.UpdateObjects()

    #TODO for other objects

    # Commit the Batch
    batch.client.Commit()
    return

def Main():
    timeprofiler.ConfigTimeProfiler.Start()
    agentapi.Init()

    logger.info("Updating Configuration for Topology = %s" % GlobalOptions.topology)
    __modify()

    __update()
