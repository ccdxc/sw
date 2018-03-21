#! /usr/bin/python3

import config.objects.doorbell      as doorbell
from infra.common.logging   import logger

import model_sim.src.model_wrap as model_wrap


class Doorbell(doorbell.Doorbell):

    def Init(self, queue_type, spec):
        # Address
        self.queue_type = queue_type.type
        self.lif_id = queue_type.lif.hw_lif_id
        self.upd = queue_type.upd

    def Ring(self, queue_id, ring_id, p_index, pid, lgh=logger):
        address = 0x400000 + (self.upd << 17) + (self.lif_id << 6) + (self.queue_type << 3)
        data = (pid << 48) | (queue_id << 24) | (ring_id << 16) | p_index

        lgh.info("Ringing Doorbell: %s" % self.GID())
        lgh.info("- Addr:0x%x (Qtype:%d/LIF:%d/Upd:%d)" %
                 (address, self.queue_type, self.lif_id, self.upd))
        lgh.info("- Data:0x%x (Pindex:%d/RingID:%d/QID:%d/PID:%d)" %
                 (data, p_index, ring_id, queue_id, pid))

        model_wrap.step_doorbell(address, data)
