#! /usr/bin/python3
import pdb
import random

import infra.common.defs        as defs
import infra.common.objects     as objects
from infra.common.logging       import logger
from infra.common.glopts        import GlobalOptions
import infra.config.base        as base

import apollo.config.resmgr     as resmgr
from apollo.config.store        import Store
import apollo.config.objects.queue_type     as queue_type

class LifObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('LIF'))
        return

    def Init(self, spec, namespace = None):
        self.id = namespace.get()
        self.GID("Lif%d" % self.id)
        self.qstate_base = {}
        self.spec       = spec
        #TODO: get hw lif id from interface
        self.hw_lif_id = self.id
        self.queue_types = objects.ObjectDatabase()
        self.obj_helper_q = queue_type.QueueTypeObjectHelper()
        self.obj_helper_q.Generate(self, spec)
        self.queue_types.SetAll(self.obj_helper_q.queue_types)
        self.queue_types_list = self.obj_helper_q.queue_types
        self.queue_list = []
        for q_type in self.queue_types_list:
            for queue in q_type.queues.GetAll():
                self.queue_list.append(queue)
        self.Show()

    def GetQt(self, type):
        return self.queue_types.Get(type)

    def GetQ(self, type, qid):
        qt = self.queue_types.Get(type)
        if qt is not None:
            return qt.queues.Get(str(qid))

    def GetQstateAddr(self, type):
        if GlobalOptions.dryrun:
            return 0
        return self.qstate_base[type]

    def ConfigureQueueTypes(self):
        if GlobalOptions.dryrun:
            return 0
        self.qstate_base = resmgr.Lif2QstateMap.get(self.hw_lif_id, None)
        logger.info("Configuring lif %d qstate base %s " %(self.hw_lif_id, list(map(lambda x: hex(x), self.qstate_base))))
        self.obj_helper_q.Configure()

    def Show(self):
        logger.info("- LIF   : %s" % self.GID())
        logger.info("  - # Queue Types    : %d" % len(self.obj_helper_q.queue_types))

    def IsFilterMatch(self, spec):
        return super().IsFilterMatch(spec.filters)

class LifObjectHelper:
    def __init__(self):
        self.lifs = []

    def Generate(self, spec, namespace):
        if namespace is None:
            return
        count = namespace.GetCount()
        logger.info("Generating %d Lifs" % (count))
        for l in range(count):
            lif = LifObject()
            lif.Init(spec, namespace)
            self.lifs.append(lif)
        return

    def Configure(self):
        if len(self.lifs) == 0:
            return
        logger.info("Configuring %d LIFs." % len(self.lifs))
        for lif in self.lifs:
            lif.ConfigureQueueTypes()
        Store.objects.SetAll(self.lifs)

    def GetRandomHostLif(self):
        if len(self.lifs) == 0:
            return None
        selected_lif = random.choice(self.lifs)
        return selected_lif
