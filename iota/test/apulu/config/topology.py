#! /usr/bin/python3
import iota.test.apulu.config.init as init

#Following come from dol/infra
import infra.common.defs as defs
from iota.harness.infra.glopts import GlobalOptions
import infra.common.glopts as glopts
#class GlobalOptions: pass
glopts.GlobalOptions = GlobalOptions
#glopts.GlobalOptions.debug = True
#glopts.GlobalOptions.verbose = True
import infra.common.parser as parser
import iota.test.apulu.config.agent.api as agentapi
import iota.harness.api as api
from infra.common.logging import logger as logger
import apollo.config.generator as generator

def Main(args):
    #Store.reset()
    #Have to look into config push
    #resmgr.Init()

    node = args.node
    defs.DOL_PATH = "/iota/"
    defs.TEST_TYPE = "IOTA"
    logger.info("Generating Configuration for Spec  = %s" % args.spec)
    if args.spec == 'dummy':
        return api.types.status.SUCCESS
    cfgspec = parser.ParseFile('test/apulu/config/cfg/', '%s'%args.spec)
    agentapi.Init(node, api.GetNicMgmtIP(node)) 
    generator.Main(node, cfgspec, api.GetNicMgmtIP(node))

    return api.types.status.SUCCESS
