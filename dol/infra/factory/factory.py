#! /usr/bin/python3
import pdb

import infra.factory.pktfactory as pktfactory
import infra.factory.memfactory as memfactory

import infra.common.objects     as objfactory
import infra.common.parser      as parser
import infra.common.defs        as defs
import infra.common.utils       as utils
import infra.factory.testcase   as testcase
import infra.factory.template   as template

from infra.common.logging       import logger

from infra.factory.store        import FactoryStore as FactoryStore

TestCaseIdAllocator = objfactory.TemplateFieldObject("range/1/65535")

def init():
    objlist = template.ParseTestobjectTemplates()
    FactoryStore.testobjects.SetAll(objlist)

    logger.info("Initializing Packet Factory.")
    pktfactory.init()

    logger.info("Initializing Memory Factory.")
    memfactory.init()
    return

def Generate(fwdata):
    for flow in fwdata.CfgFlows:
        tcid = TestCaseIdAllocator.get()
        tc = testcase.TestCase(tcid, flow, fwdata)
        fwdata.TestCases.append(tc)
    return defs.status.SUCCESS

