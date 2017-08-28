#! /usr/bin/python3
import pdb

import infra.common.defs        as defs
import infra.common.objects     as objects
import config.resmgr            as resmgr
import config.objects.ring      as ring

import config.hal.api           as halapi
import config.hal.defs          as haldefs

from config.store               import Store
from infra.common.logging       import cfglogger

class QueueObject(objects.FrameworkObject):
    def __init__(self):
        super().__init__()
        return

    def Init(self, lif, spec):
        self.GID(spec.id)
        self.lif        = lif
        self.type       = spec.type
        self.purpose    = spec.purpose.upper()
        self.size       = spec.size
        self.rings      = objects.ObjectDatabase(cfglogger)
        self.obj_helper_ring = ring.RingObjectHelper()
        self.obj_helper_ring.Generate(self, spec.ring)
        self.rings.SetAll(self.obj_helper_ring.rings)
        self.Show()
        return

    def PrepareHALRequestSpec(self, req_spec):
        return

    def ProcessHALResponse(self, req_spec, resp_spec):
        return

    def Show(self):
        cfglogger.info('Queue: %s' % self.GID())
        cfglogger.info('- lif    : %s' % self.lif.GID())
        cfglogger.info('- type   : %s' % self.type)
        cfglogger.info('- purpose: %s' % self.purpose)
        cfglogger.info('- size   : %s' % self.size)
        return

class QueueObjectHelper:
    def __init__(self):
        self.queues = []
        return

    def Generate(self, lif, lifspec):
        for espec in lifspec.entries:
            for e in range(espec.entry.count):
                queue = QueueObject()
                queue.Init(lif, espec.entry)
                self.queues.append(queue)
        return
