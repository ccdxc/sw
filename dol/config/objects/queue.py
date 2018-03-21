#! /usr/bin/python3
import pdb
import math

import infra.common.defs        as defs
import infra.common.objects     as objects
import config.resmgr            as resmgr
import config.objects.ring      as ring
import config.objects.eth.ring  as ring
import config.hal.api           as halapi
import config.hal.defs          as haldefs

from config.store               import Store
from infra.common.logging       import logger

class QueueObject(objects.FrameworkObject):
    def __init__(self):
        super().__init__()

    def Init(self, queue_type, spec):
        self.spec       = spec
        self.queue_type = queue_type
        self.id         = queue_type.GetQid()
        self.size       = getattr(spec, 'size', 0)

        self.GID('Q%d' % self.id)

        self.rings      = objects.ObjectDatabase()
        self.obj_helper_ring = ring.EthRingObjectHelper()
        self.obj_helper_ring.Generate(self, spec)
        self.rings.SetAll(self.obj_helper_ring.rings)

        self.Show()

    def GetQstateBaseAddr(self):
        return self.queue_type.GetQstateAddr() + (self.id * self.queue_type.size)

    def ConfigureRings(self):
        self.obj_helper_ring.Configure()

    def PrepareHALRequestSpec(self, req_spec):
        pass

    def Show(self):
        logger.info('Queue: %s' % self.GID())
        logger.info('- type   : %s' % self.queue_type.GID())
        logger.info('- id     : %s' % self.id)

class QueueObjectHelper:
    def __init__(self):
        self.queues = []

    def Generate(self, queue_type, spec):
        for espec in spec.queues:
            for qspec in range(espec.queue.count):
                queue = QueueObject()
                queue.Init(queue_type, espec.queue)
                self.queues.append(queue)

    def Configure(self):
        for queue in self.queues:
            queue.ConfigureRings()
