#! /usr/bin/python3
import infra.common.parser as parser
import infra.common.objects as objects
import test.firewall.tracker.connection as base
from infra.common.logging import logger as logger
from test.fte.alg.tracker.store import TrackerStore

class ALGConnectionParamsObject(base.ConnectionParamsObject):
    def __init__(self, spec):
        super().__init__(spec)

def __parse_connection_specs():
    path = 'test/fte/tracker/specs/'
    filename = 'connections.spec'
    return parser.ParseDirectory(path, filename)

def LoadConnectionSpecs():
    specs = __parse_connection_specs()
    for spec in specs:
        for cspec in spec.connections:
            obj = ALGConnectionParamsObject(cspec.connection)
            logger.info("- Loading Tracker Connnection Spec: %s" % obj.GID())
            TrackerStore.connections.Set(obj.GID(), obj)
    return

