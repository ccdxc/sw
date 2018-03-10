#! /usr/bin/python3
import pdb

import infra.factory.pktfactory as pktfactory
import infra.factory.memfactory as memfactory
import infra.factory.template   as template

from infra.common.logging       import logger
from infra.factory.store        import FactoryStore as FactoryStore

def init():
    objlist = template.ParseTestobjectTemplates()
    FactoryStore.testobjects.SetAll(objlist)
    
    objlist = template.ParseTestTypeTemplates()
    FactoryStore.templates.SetAll(objlist)

    logger.info("Initializing Packet Factory.")
    pktfactory.init()

    logger.info("Initializing Memory Factory.")
    memfactory.init()
    return

