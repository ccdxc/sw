#! /usr/bin/python3
import infra.common.parser as parser
import infra.common.objects as objects
from infra.common.logging import logger as logger
from test.firewall.tracker.store import TrackerStore as TrackerStore

class ConnectionParamsObject(objects.FrameworkTemplateObject):
    def __init__(self, spec):
        super().__init__()
        assert(spec is not None)
        self.Clone(spec)
        self.GID(spec.id)
        return

def __parse_connection_specs():
    path = 'test/firewall/tracker/specs/'
    filename = 'connections.spec'
    return parser.ParseDirectory(path, filename)

def LoadConnectionSpecs():
    specs = __parse_connection_specs()
    for spec in specs:
        for cspec in spec.connections:
            obj = ConnectionParamsObject(cspec.connection)
            logger.info("- Loading Tracker Connnection Spec: %s" % obj.GID())
            TrackerStore.connections.Set(obj.GID(), obj)
    return

