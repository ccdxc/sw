#! /usr/bin/python3
import iota.test.apulu.config.init as init

#Following come from dol/infra
import infra.common.defs as defs
from iota.harness.infra.glopts import GlobalOptions
import infra.common.glopts as glopts
glopts.GlobalOptions = GlobalOptions
import infra.common.parser as parser
import iota.harness.api as api
from infra.common.logging import logger as logger
import apollo.config.generator as generator
from apollo.config.store import EzAccessStore
from apollo.config.store import client as EzAccessStoreClient
from apollo.config.store import Init as EzAccessStoreInit

def Main(args):
    node = args.node
    defs.DOL_PATH = "/iota/"
    defs.TEST_TYPE = "IOTA"
    api.Logger.info(f"Generating Configuration for Spec {args.spec}")
    if args.spec == 'dummy':
        return api.types.status.SUCCESS
    cfgspec = parser.ParseFile('test/apulu/config/cfg/', '%s'%args.spec)

    naples_uuid_map = api.GetNaplesNodeUuidMap()
    EzAccessStoreInit(node)
    EzAccessStore.SetUuidMap(naples_uuid_map)
    EzAccessStoreClient[node].SetUnderlayIPs(api.GetNicUnderlayIPs(node))

    generator.Main(node, cfgspec, api.GetNicMgmtIP(node))

    return api.types.status.SUCCESS
