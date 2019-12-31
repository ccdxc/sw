#! /usr/bin/python3
import pdb
import random
from scapy.all import *

#import model_sim.src.model_wrap as model_wrap

import infra.common.defs        as defs
import infra.common.objects     as objects
from infra.common.logging       import logger
from infra.common.glopts        import GlobalOptions
import infra.config.base        as base

import iota.test.apulu.config.utils as utils
from iota.test.apulu.config.store        import Store
import iota.test.apulu.config.objects.host.queue_type as queue_type

NICMGR_DEV_CMD_TIMEOUT = 10

# platform/drivers/common/ionic_if.h
dev_cmd_regs_ = [
        LEIntField("doorbell", 0),
        LEIntField("done", 0),
    ]

class LifInitCmd(Packet):
    fields_desc = dev_cmd_regs_ + [
        ByteField("opcode", 0),
        ByteField("type", 0),
        LEShortField("index", 0),
        LEIntField("rsvd", 0),
        LELongField("info_pa", 0),
        BitField("rsvd2", 0, 48),
    ]

class DevCmdObject(object):

    def __init__(self, addr, __data_class__):
        self.addr = addr
        self.__data_class__ = __data_class__
        self.size = len(self.__data_class__())
        #self.data = __data_class__(model_wrap.read_mem(self.addr, self.size))

    def Show(self, lgh = logger):
        lgh.ShowScapyObject(self.data)

    def Write(self, lgh = logger):
        if GlobalOptions.skipverify:
            return
        # post devcmd
        data = self.data[self.__data_class__]
        # set cmd done
        data.done = 0
        # set doorbell
        data.doorbell = 1
        lgh.info("Writing %s @0x%x size: %d" % (self.__data_class__.__name__,
            self.addr, self.size))
        self.Show()
        #model_wrap.write_mem_pcie(self.addr, bytes(self.data), len(self.data))
        # wait for nicmgr to process this devcmd
        utils.Sleep(NICMGR_DEV_CMD_TIMEOUT)


class LifInitDevCmdObject(DevCmdObject):

    def __init__(self, addr):
        super().__init__(addr, LifInitCmd)
        data = self.data[self.__data_class__]
        data.opcode = 21 #CMD_OPCODE_LIF_INIT
        # TODO: derive index
        data.index = 0
        data.info_pa = 0
        data.rsvd = 0
        data.rsvd2 = 0


class LifObject(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        self.Clone(Store.templates.Get('LIF'))
        return

    def Init(self, ifinfo, spec, namespace = None):
        self.id = namespace.get()
        self.GID("Lif%d" % self.id)
        self.spec       = spec
        self.hw_lif_id = self.id
        self.devcmdaddr = ifinfo.DevcmdMemAddr
        self.qstate_base = ifinfo.Lif2QstateMap.get(self.hw_lif_id, {})
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

    def InitLif(self):
        logger.info(" Posting Lif Init Dev Cmd for lif %d @ devcmdaddr 0x%x"%(self.hw_lif_id, self.devcmdaddr))
        cmd = LifInitDevCmdObject(self.devcmdaddr)
        cmd.Write()

    def Configure(self):
        if GlobalOptions.dryrun:
            return 0
        logger.info(" Configuring Lif %d" %(self.hw_lif_id))
        # post CMD_OPCODE_LIF_INIT
        self.InitLif()
        return

    def ConfigureQueueTypes(self):
        if GlobalOptions.dryrun:
            return 0
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

    def Generate(self, ifinfo, spec, namespace):
        if namespace is None:
            return
        count = namespace.GetCount()
        logger.info("Generating %d Lifs" % (count))
        for l in range(count):
            lif = LifObject()
            lif.Init(ifinfo, spec, namespace)
            self.lifs.append(lif)
        return

    def Configure(self):
        if len(self.lifs) == 0:
            return
        logger.info("Configuring %d LIFs." % len(self.lifs))
        for lif in self.lifs:
            lif.Configure()
            lif.ConfigureQueueTypes()
        Store.objects.SetAll(self.lifs)

    def GetRandomHostLif(self):
        if len(self.lifs) == 0:
            return None
        selected_lif = random.choice(self.lifs)
        return selected_lif
