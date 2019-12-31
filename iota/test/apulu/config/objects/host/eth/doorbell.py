#! /usr/bin/python3

from infra.common.logging import logger
#import model_sim.src.model_wrap as model_wrap

import iota.test.apulu.config.objects.host.doorbell as doorbell

class Doorbell(doorbell.Doorbell):

    def Init(self, queue_type, spec):
        self.queue_type = queue_type

    def RingDB(self, queue_id, ring_id, p_index, pid, sched=True, lgh=logger):
        hw_qtype = self.queue_type.type
        lif_id = self.queue_type.lif.hw_lif_id

        upd = self.queue_type.upd
        if not sched:
            upd &= ~0x1

        address = 0x400000 + (upd << 17) + (lif_id << 6) + (hw_qtype << 3)
        data = (pid << 48) | (queue_id << 24) | (ring_id << 16) | p_index

        lgh.info("Ringing Doorbell:")
        lgh.info("- Addr:0x%x (Qtype:%d/LIF:%d/Upd:%d)" %
                 (address, hw_qtype, lif_id, upd))
        lgh.info("- Data:0x%x (Pindex:%d/RingID:%d/QID:%d/PID:%d)" %
                 (data, p_index, ring_id, queue_id, pid))

        #model_wrap.step_doorbell(address, data)

    def Ring(self, spec, lgh=logger):
        # Resolve the queue and ring for the doorbell
        queue_id = getattr(spec, 'queue_id', 0)
        queue = self.queue_type.obj_helper_q.queues[queue_id]

        # Frequently used doorbell behaviors
        how = getattr(spec, 'how', None)
        if how == 'arm_index':
            ring_id = 1
            ring = queue.obj_helper_ring.rings[ring_id]
            index_mask = ring.size - 1
            index = ring.ci
        else:
            ring_id = getattr(spec, 'ring_id', 0)
            ring = queue.obj_helper_ring.rings[ring_id]
            index_mask = ring.size - 1
            index = ring.pi

        # Other ways to modify the doorbell value
        index = getattr(spec, 'index_alt', index)
        if index == 'ci':
            index = ring.ci

        index = (index + getattr(spec, 'index_plus', 0)) & index_mask
        ring_id = getattr(spec, 'ring_alt', ring_id)
        pid = getattr(spec, 'pid', queue.pid)

        # Step the model after ringing the doorbell?
        sched = getattr(spec, 'sched', True)

        self.RingDB(queue_id, ring_id, index, pid, sched, lgh)
