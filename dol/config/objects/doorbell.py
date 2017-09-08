#! /usr/bin/python3

import infra.config.base as base
import infra.common.objects as objects
from infra.common.logging   import cfglogger

import model_sim.src.model_wrap as model_wrap


class Doorbell(base.ConfigObjectBase):
    def __init__(self):
        super().__init__()
        return

    def Init(self, ring, spec):
        self.GID(spec.id)
        self.ring = ring
        self.spec = spec

        # Address
        self.queue_type = ring.queue.queue_type.type
        self.lif_id = 0
        self.upd = 0

        # Data
        self.p_index = 0
        self.ring_id = 0
        self.queue_id = ring.queue.id
        self.pid = 0

    def Ring(self, test_spec, lgh = cfglogger):
        # Address
        self.upd = getattr(test_spec, 'upd', 0x3)
        self.lif_id = self.ring.queue.queue_type.lif.hw_lif_id

        # Data
        self.p_index = self.ring.queue.qstate.get_pindex(self.ring_id)
        self.pid = getattr(test_spec, 'pid', 0)

        address = 0x400000 + (self.upd << 17) + (self.lif_id << 6) + (self.queue_type << 3)
        data = (self.pid << 48) | (self.queue_id << 24) | (self.ring_id << 16) | self.p_index

        lgh.info("Ringing Doorbell: %s" % self.GID())
        lgh.info("- Addr:0x%x (Qtype:%d/LIF:%d/Upd:%d)" %\
                 (address, self.queue_type, self.lif_id, self.upd))
        lgh.info("- Data:0x%x (Pindex:%d/RingID:%d/QID:%d/PID:%d)" %\
                 (data, self.p_index, self.ring_id, self.queue_id, self.pid))

        model_wrap.step_doorbell(address, data)

        self.ring.queue.qstate.Read()
