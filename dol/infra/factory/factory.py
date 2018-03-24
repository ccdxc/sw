#! /usr/bin/python3
import pdb

import infra.factory.pktfactory as pktfactory
import infra.factory.memfactory as memfactory
import infra.factory.template   as template
import infra.common.loader      as loader

from infra.common.logging       import logger
from infra.factory.store        import FactoryStore as FactoryStore
from infra.factory.dol_testcase import DOLTestCase as DOLTestCase 
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

def GetTestCase(tcid, root, module, loopid):
    if module.testspec:
        test_spec = module.testspec.type.Get(FactoryStore)
        module_hdl = loader.ImportModule(test_spec.meta.package, test_spec.meta.module)
        return getattr(module_hdl, test_spec.meta.cls)(tcid, root, module, loopid)
    return DOLTestCase(tcid, root, module, loopid)
 
