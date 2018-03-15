#! /usr/bin/python3
import pdb
import time

import infra.asic.model as model
import infra.common.defs as defs

from infra.common.glopts    import GlobalOptions
from infra.asic.model       import ModelConnector
import infra.e2e.main as E2E 

class TriggerEngineObject:

    def __init__(self):
        pass

    def _trigger_step(self, tc, step):
        return
        
    def _trigger(self, tc):
        return defs.status.SUCCESS

    def Trigger(self, tc):
        assert(tc != None)
        tc.status = self._trigger(tc)
        return

    def __resolve_status(self, status1, status2):
        if status1 is defs.status.ERROR or status2 is defs.status.ERROR:
            return defs.status.ERROR
        return defs.status.SUCCESS

    def __verify_step(self, tc, step):
        vfstatus = tc.verif_engine.Verify(step, tc)
        cbstatus = tc.StepVerifyCallback(step)
        step.status = self.__resolve_status(vfstatus, cbstatus)
        if step.status is defs.status.ERROR:
            tc.error("Step%d FINAL STATUS = %s (Verify:%s Callback:%s)" %\
                     (step.step_id, 'IGNORE' if tc.IsIgnore() else 'FAIL',
                      defs.status.str(vfstatus),
                      defs.status.str(cbstatus)))
            return step.status
        tc.info("Step%d FINAL STATUS = PASS" % step.step_id)
        return step.status

    def _trigger(self, tc):
        status = defs.status.SUCCESS
        for step in tc.session.steps:
            tc.StepSetupCallback(step)
            tc.StepTriggerCallback(step)
            self._trigger_step(tc, step)
            vfstatus = self.__verify_step(tc, step)
            tc.StepTeardownCallback(step)
            if vfstatus is defs.status.ERROR:
                break
        cbstatus = tc.VerifyCallback()
        status = self.__resolve_status(cbstatus, vfstatus)
        if status is defs.status.ERROR:
            tc.error("TESTCASE FINAL STATUS = %s(Verify:%s Callback:%s)" %\
                     ('IGNORE' if tc.IsIgnore() else 'FAIL',
                      defs.status.str(vfstatus),
                      defs.status.str(cbstatus)))
        else:
            tc.info("TESTCASE FINAL STATUS = PASS")
        tc.TeardownCallback()
        return status    
        
class DolTriggerEngineObject(TriggerEngineObject):
    def __init__(self):
        super().__init__()
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

    def __trigger_config(self, step, lgh):
        if step.trigger.configs == None:
            return

        for config in step.trigger.configs:
            method = getattr(config.actual_object, config.method)
            if not method:
                 assert 0
            method(config_spec.spec)
        return

    def __trigger_delay(self, step, lgh):
        if GlobalOptions.dryrun:
            return
        if step.trigger.delay:
           lgh.info("Trigger Delay: %d" % step.trigger.delay)
           time.sleep(step.trigger.delay)
        return

    def _trigger_step(self, tc, step):
        super()._trigger_step(tc, step)
        self.__trigger_delay(step, tc)
        self.__trigger_descriptors(step, tc)
        self.__ring_doorbell(step, tc)
        self.__trigger_packets(step, tc)
        tc.TriggerCallback()
        return

class E2ETriggerEngineObject(TriggerEngineObject):
    
    def __init__(self):
        super().__init__()
        
    def __trigger_commands(self, step, lgh):
        for cmd in step.trigger.commands:
            lgh.info("Running command %s : %s" % (cmd.object.GID(), cmd.command))
            if GlobalOptions.dryrun:
                return        
            cmd.status = E2E.RunCommand(cmd.object.GID(), cmd.command,
                                         background=cmd.background)
    
    def _trigger_step(self, tc, step):
        self.__trigger_commands(step, tc)
        tc.TriggerCallback()
        return


DolTriggerEngine = DolTriggerEngineObject()
E2ETriggerEngine = E2ETriggerEngineObject()
