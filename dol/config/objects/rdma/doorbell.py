#! /usr/bin/python3

import config.objects.doorbell      as doorbell
from infra.common.logging   import cfglogger

import model_sim.src.model_wrap as model_wrap


class Doorbell(doorbell.Doorbell):

    def Ring(self, test_spec, lgh=cfglogger):
        # Address
        upd = getattr(test_spec, 'upd', 0xb)
        queue_type = self.ring.queue.queue_type.type
        lif_id = self.ring.queue.queue_type.lif.hw_lif_id

        # Data
        ring_id = getattr(self.ring, 'num', 0)
        queue_id = self.ring.queue.id

        # Data
        p_index = self.ring.queue.qstate.get_pindex(ring_id)
        pid = getattr(test_spec, 'pid', 0)

        address = 0x400000 + (upd << 17) + (lif_id << 6) + (queue_type << 3)
        data = (pid << 48) | (queue_id << 24) | (ring_id << 16) | p_index

        lgh.info("Ringing Doorbell: %s" % self.GID())
        lgh.info("- Addr:0x%x (Qtype:%d/LIF:%d/Upd:%d)" %
                 (address, queue_type, lif_id, upd))
        lgh.info("- Data:0x%x (Pindex:%d/RingID:%d/QID:%d/PID:%d)" %
                 (data, p_index, ring_id, queue_id, pid))

        model_wrap.step_doorbell(address, data)
