#! /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import config.resmgr            as resmgr

import config.hal.api            as halapi
import config.hal.defs           as haldefs

from config.store               import Store
from    config.objects.tcp_proxy_cb     import TcpCbHelper

class QueueObject(objects.FrameworkObject):
    def __init__(self, spec, parent):
        super().__init__()
        self.id     = resmgr.QueueIdAllocator.get()
        self.spec   = spec

        qtypename = 'LIF_QUEUE_TYPE_' + spec.type.upper()
        self.type   = haldefs.interface.LifQType.Value(qtypename)

        self.parent = parent
        TcpCbHelper.main(self)
        return

    def configure(self, req_spec):
        req_spec.qtype = self.type
        req_spec.queue_id = self.id
        return

    def __str__(self):
        string = 'QUEUE: ID:%d Type:%s' % (self.id, self.qtypename)
        return string

def LoadConfigSpec(spec, lif):
    objlist = []
    for qidx in range(spec.queues):
        obj = QueueObject(spec, lif)
        objlist.append(obj)
    return objlist

def Configure():
    objlist = Store.queues()
    cfglogger.info("Configuring %d QUEUEs." % len(objlist)) 
    halapi.ConfigureQueues(objlist)
    return
