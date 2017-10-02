#! /usr/bin/python3
import pdb
import time

import infra.asic.model as model
import infra.common.defs as defs

from infra.common.glopts    import GlobalOptions
from infra.asic.model       import ModelConnector

class TriggerEngineObject:
    def __init__(self):
        return

    def __copy__(self):
        assert(0)
        return

    def __trigger_descriptors(self, step, lgh):
        if GlobalOptions.dryrun:
            return

        if step.trigger.descriptors == None:
            return

        for dbsp in step.trigger.descriptors:
            if dbsp.descriptor.object is None:
                continue

            ring = dbsp.descriptor.ring
            descr = dbsp.descriptor.object

            lgh.info("Posting Descriptor:%s on Ring:%s" %\
                    (ring.GID(), descr.GID()))
            ring.Post(descr)
        return

    def __trigger_packets(self, step, lgh):
        if step.trigger.packets == None:
            return

        for p in step.trigger.packets:
            if p.packet == None: break
            rawpkt = p.packet.rawbytes
            port = p.ports[0]
            lgh.info("Sending Input Packet:%s of Length:%d on Port:%d" %\
                     (p.packet.GID(), len(rawpkt), port))
            ModelConnector.Transmit(rawpkt, port)
        return

    def __ring_doorbell(self, step, lgh):
        if GlobalOptions.dryrun:
            return
        
        dbsp = step.trigger.doorbell
        if dbsp is None or dbsp.object is None: 
            return
        
        db = dbsp.object
        lgh.info("Posting doorbell %s" % db)
        db.Ring(dbsp.spec)
        return

    def __trigger_delay(self, step, lgh):
        if GlobalOptions.dryrun:
            return
        if step.trigger.delay:
           lgh.info("Trigger Delay: %d" % step.trigger.delay)
           time.sleep(step.trigger.delay)
        return

    def __trigger_step(self, tc, step):
        self.__trigger_delay(step, tc)
        self.__trigger_descriptors(step, tc)
        self.__trigger_packets(step, tc)
        self.__ring_doorbell(step, tc)
        tc.TriggerCallback()
        return
        
    def __verify_step(self, tc, step):
        vfstatus = tc.infra_data.VerifEngine.Verify(step, tc)
        cbstatus = tc.VerifyCallback()
        if vfstatus is defs.status.ERROR or cbstatus is defs.status.ERROR:
            step.status = defs.status.ERROR
            tc.error("Step%d FINAL STATUS = FAIL(Verify:%s Callback:%s)" %\
                     (step.step_id, defs.status.str(vfstatus),
                      defs.status.str(cbstatus)))
            return step.status
        step.status = defs.status.SUCCESS
        tc.info("Step%d FINAL STATUS = PASS" % step.step_id)
        return

    def __trigger(self, tc):
        status = defs.status.SUCCESS
        for step in tc.session.steps:
            self.__trigger_step(tc, step)
            status = self.__verify_step(tc, step)
            if status is defs.status.ERROR: break
        tc.TeardownCallback()
        return status

    def Trigger(self, tc):
        assert(tc != None)
        tc.status = self.__trigger(tc)
        return

TriggerEngine = TriggerEngineObject()
