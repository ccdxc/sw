#! /usr/bin/python3
import pdb
import time

import infra.common.defs as defs

from infra.common.logging       import logger as logger
from infra.common.glopts        import GlobalOptions
from infra.asic.model           import ModelConnector
from infra.asic.pktcollector    import PacketCollector

import infra.e2e.main as E2E 

class TriggerError(defs.Error):
   """ Raised when the trigger fails """
   pass

class TriggerEngineObject:

    def __init__(self):
        pass

    def _trigger_step(self, tc, step):
        return

    def Trigger(self, tc):
        assert(tc != None)
        tc.status = self._trigger(tc)
        return

    def __resolve_status(self, status1, status2):
        if status2 is defs.status.OVERRIDE:
            return defs.status.SUCCESS
        if status1 is defs.status.ERROR or status2 is defs.status.ERROR:
            return defs.status.ERROR
        return defs.status.SUCCESS

    def __verify_step(self, tc, step):
        vfstatus = tc.verif_engine.Verify(step, tc)
        cbstatus = tc.StepVerifyCallback(step)
        step.status = self.__resolve_status(vfstatus, cbstatus)
        if step.status is defs.status.ERROR:
            logger.error("Step%d FINAL STATUS = %s (Verify:%s Callback:%s)" %\
                         (step.step_id, 'IGNORE' if tc.IsIgnore() else 'FAIL',
                         defs.status.str(vfstatus),
                         defs.status.str(cbstatus)))
            return step.status
        logger.info("Step%d FINAL STATUS = PASS" % step.step_id)
        return step.status

    def _trigger(self, tc):
        tc.PreTriggerCallback()
        status = defs.status.SUCCESS
        vfstatus = defs.status.SUCCESS
        cbstatus = defs.status.SUCCESS
        try:
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
        except TriggerError as e:
            logger.error(f"Testcase raised exception {e}")
            status = defs.status.ERROR
        if status is defs.status.ERROR:
            logger.error("TESTCASE FINAL STATUS = %s(Verify:%s Callback:%s)" %\
                         ('IGNORE' if tc.IsIgnore() else 'FAIL',
                         defs.status.str(vfstatus),
                         defs.status.str(cbstatus)))
        else:
            logger.info("TESTCASE FINAL STATUS = PASS")
        tc.TeardownCallback()
        return status

class DolTriggerEngineObject(TriggerEngineObject):
    def __init__(self):
        super().__init__()
        return

    def __copy__(self):
        assert(0)
        return

    def __trigger_descriptors(self, tc, step):
        if GlobalOptions.dryrun and not GlobalOptions.savepcap:
            return

        if step.trigger.descriptors == None:
            return

        for dbsp in step.trigger.descriptors:
            if dbsp.descriptor.object is None:
                continue

            ring = dbsp.descriptor.ring
            descr = dbsp.descriptor.object

            if GlobalOptions.savepcap:
                PacketCollector.SaveTx(descr.GetTxPacket(), ring.GetLif())
            else:
                logger.info("Posting Descriptor:%s on Ring:%s" %\
                            (descr.GID(), ring.GID()))
                ring.Post(descr)
        return

    def __trigger_packets(self, tc, step):
        if step.trigger.packets == None:
            return

        for p in step.trigger.packets:
            if p.packet == None: break
            rawpkt = p.packet.rawbytes
            port = p.ports[0]
            logger.info("Sending Input Packet:%s of Length:%d on Port:%d" %\
                        (p.packet.GID(), len(rawpkt), port))
            if GlobalOptions.savepcap:
                PacketCollector.Save(tc.GID(), p.packet.GetScapyPacket(), port)
            else:
                ModelConnector.Transmit(rawpkt, port)
        return

    def __ring_doorbell(self, step):
        if GlobalOptions.dryrun:
            return
        dbsp = step.trigger.doorbell
        if dbsp is None or dbsp.object is None:
            return

        db = dbsp.object
        logger.info("Posting doorbell %s" % db)
        db.Ring(dbsp.spec)
        return

    def __trigger_config(self, step):
        if step.trigger.configs == None:
            return

        for config in step.trigger.configs:
            if config.method is not None:
                method = getattr(config.actual_object, config.method)
                if not method:
                    assert 0
                if not method(config.spec):
                    raise TriggerError("config method failed in step trigger")
        return

    def __trigger_delay(self, step):
        if GlobalOptions.dryrun:
            return
        if step.trigger.delay:
           logger.info("Trigger Delay: %d" % step.trigger.delay)
           time.sleep(step.trigger.delay)
        return

    def _trigger_step(self, tc, step):
        super()._trigger_step(tc, step)
        self.__trigger_config(step)
        self.__trigger_delay(step)
        self.__trigger_descriptors(tc, step)
        self.__ring_doorbell(step)
        self.__trigger_packets(tc, step)
        tc.TriggerCallback()
        return

class E2ETriggerEngineObject(TriggerEngineObject):
    def __init__(self):
        super().__init__()
        return

    def __trigger_commands(self, step):
        for cmd in step.trigger.commands:
            logger.info("Running command %s : %s" % (cmd.object.GID(), cmd.command))
            if GlobalOptions.dryrun:
                return
            cmd.status = E2E.RunCommand(cmd.object.GID(), cmd.command, timeout=cmd.timeout,
                                         background=cmd.background)
        return

    def __trigger_setups(self, step):
        for setup in getattr(step.trigger, "setups", []):
            for file in setup.files:
                logger.info("Running setup %s : %s" % (setup.endpoint.GID(), file))
                E2E.CopyFile(setup.endpoint.GID(), file)
        return

    def _trigger_step(self, tc, step):
        self.__trigger_setups(step)
        self.__trigger_commands(step)
        tc.TriggerCallback()
        return

DolTriggerEngine = DolTriggerEngineObject()
E2ETriggerEngine = E2ETriggerEngineObject()
