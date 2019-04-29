#! /usr/bin/python3

import iris.config.objects.doorbell      as doorbell
from infra.common.logging   import logger

import model_sim.src.model_wrap as model_wrap


class Doorbell(doorbell.Doorbell):

    def Ring(self, test_spec = None, lgh=logger, upd_bits = 0):
        # Address
        if test_spec is None:
            upd = upd_bits
        else:
            upd = getattr(test_spec, 'upd', 0xb)
        queue_type = self.ring.queue.queue_type.type
        lif_id = self.ring.queue.queue_type.lif.hw_lif_id

        # Data
        ring_id = getattr(self.ring, 'hw_ring_id', 0)
        queue_id = self.ring.queue.id

        # Data

        #upd_lo = upd & 0x3
        upd_hi = (upd >> 2) & 0x3
        if upd_hi == 1:
            index = self.ring.queue.qstate.get_cindex(ring_id)
        elif upd_hi == 2:
            index = self.ring.queue.qstate.get_pindex(ring_id)
        else:
            index = 0
        #lgh.info("UPD: %d Hi: %d, Lo: %d, Index: %d" % (upd, upd_hi, upd_lo, index))

        pid = getattr(test_spec, 'pid', 0)

        address = 0x400000 + (upd << 17) + (lif_id << 6) + (queue_type << 3)
        data = (pid << 48) | (queue_id << 24) | (ring_id << 16) | index

        lgh.info("Ringing Doorbell: %s" % self.GID())
        lgh.info("- Addr:0x%x (Qtype:%d/LIF:%d/Upd:%d)" %
                 (address, queue_type, lif_id, upd))
        lgh.info("- Data:0x%x (Pindex:%d/RingID:%d/QID:%d/PID:%d)" %
                 (data, index, ring_id, queue_id, pid))

        model_wrap.step_doorbell(address, data)
