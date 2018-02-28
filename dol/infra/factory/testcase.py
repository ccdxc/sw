#! /usr/bin/python3
import pdb
import copy

import infra.common.defs        as defs
import infra.common.parser      as parser
import infra.common.objects     as objects
import infra.factory.pktfactory as pktfactory
import infra.factory.memfactory as memfactory

from config.store import Store  as ConfigStore
from infra.factory.store import FactoryStore as FactoryStore
from infra.common.logging import logger
from infra.misc.coverage import TestCaseCoverageHelper
from infra.common.glopts import GlobalOptions as GlobalOptions
from infra.asic.model import ModelConnector

class TestCaseParser(parser.ParserBase):
    def __init__(self, path, filename):
        super().__init__()
        self.path = path
        self.filename = filename
        return
 
    def parse(self):
        objlist = super().Parse(self.path, self.filename)
        assert(len(objlist) == 1)
        return objlist[0]

class TestCaseReceivedPacketObject:
    def __init__(self):
        self.rawpkt = None
        self.port = None
        return

class TestCaseTrigExpPacketObject:
    def __init__(self):
        self.packet     = None
        self.ports      = None
        return

class TestCaseTrigExpDoorbellObject:
    def __init__(self):
        self.object = None
        self.spec = None
        return

    def GID(self):
        return self.object.GID()

class TestCaseTrigExpDescriptorObject:
    def __init__(self):
        self.object = None
        self.ring   = None
        self.buffer = None
        self.packet = None
        return

    def GID(self):
        return self.object.GID()

class TestCaseTrigExpDescriptorSpec:
    def __init__(self):
        self.descriptor = TestCaseTrigExpDescriptorObject()
        return

class TestCaseTrigExpConfigObject:
    def __init__(self):
        self.original_object = None
        self.actual_object = None
        self.method = None
        self.spec = None
        return
 
class TestCaseSessionEntryObject:
    def __init__(self):
        self.step = None
        return

class TestCaseSessionStepTriggerExpectReceivedObject:
    def __init__(self):
        self.delay                 = 0
        self.packets               = []
        self.descriptors           = []
        self.doorbell              = TestCaseTrigExpDoorbellObject()
        self.configs               = []
        #self.ignore_excess_packets = False
        return

class TestCaseSessionStepObject:
    def __init__(self):
        self.trigger = TestCaseSessionStepTriggerExpectReceivedObject()
        self.expect  = TestCaseSessionStepTriggerExpectReceivedObject()
        self.received = TestCaseSessionStepTriggerExpectReceivedObject()
        return

class TestCaseSessionObject:
    def __init__(self):
        self.steps = []
        return

    def AddStep(self, step):
        self.steps.append(step)
        return

class TestCasePrivateData:
    def __init__(self):
        return

class TestCase(objects.FrameworkObject):
    def __init__(self, tcid, root, module, loopid = 0):
        super().__init__()
        self.template = FactoryStore.testobjects.Get('TESTCASE')
        self.Clone(self.template)
        self.__root = None
        self.drop = False
        self.loopid = None
        self.pendol = module.IsPendolHeaderEnabled()
        self.__retry_enable = root.IsRetryEnabled()
        self.LockAttributes()
        
        self.GID(tcid)
        self.__setup_config(root)

        self.packets        = objects.ObjectDatabase(logger = self)
        self.descriptors    = objects.ObjectDatabase(logger = self)
        self.buffers        = objects.ObjectDatabase(logger = self)
        self.objects        = objects.ObjectDatabase(logger = self)

        self.module         = module
        self.tracker        = module.GetTracker()
        self.infra_data     = module.infra_data
        self.testspec       = module.testspec
        self.logger         = self.infra_data.Logger
        self.session        = TestCaseSessionObject()
        self.step_id        = 0
        self.pvtdata        = TestCasePrivateData()
        self.coverage       = TestCaseCoverageHelper(self)
        
        self.loopid = loopid + 1
        if GlobalOptions.tcscale:
            self.logpfx = "TC%06d-%04d:" % (self.ID(), self.loopid)
        else:
            self.logpfx = "TC%06d" % self.ID()
        self.__generate()
        return

    def IsPendolHeaderEnabled(self):
        return self.pendol

    def GetStepId(self):
        return self.step_id
    def GetTcId(self):
        return self.GID()

    def GetRoot(self):
        return self.__root

    def SetDrop(self):
        self.drop = True
        return
    def IsDrop(self):
        return self.drop

    def IsIgnore(self):
        return self.module.IsIgnore()

    def IsRetryEnabled(self):
        return self.__retry_enable
    def SetRetryEnabled(self, val):
        self.__retry_enable = val
        return

    def __setup_config(self, root):
        self.__root = root
        root.SetupTestcaseConfig(self.config)
        return

    def __init_config(self, flow):
        self.config.flow    = flow
        
        self.config.src.tenant  = flow.GetSrcTenant()
        self.config.dst.tenant  = flow.GetDstTenant()
       
        self.config.src.segment = flow.GetSrcSegment()
        self.config.dst.segment = flow.GetDstSegment()

        self.config.src.endpoint = flow.GetSrcEndpoint()
        self.config.dst.endpoint = flow.GetDstEndpoint()

        self.config.ingress_mirror.session1 = flow.GetIngressMirrorSession(idx = 1)
        self.config.ingress_mirror.session2 = flow.GetIngressMirrorSession(idx = 2)
        self.config.ingress_mirror.session3 = flow.GetIngressMirrorSession(idx = 3)
        self.config.egress_mirror.session1 = flow.GetEgressMirrorSession(idx = 1)
        self.config.egress_mirror.session2 = flow.GetEgressMirrorSession(idx = 2)
        self.config.egress_mirror.session3 = flow.GetEgressMirrorSession(idx = 3)
        return

    def info(self, *args, **kwargs):
        self.logger.info(self.logpfx, *args, **kwargs)
        return

    def error(self, *args, **kwargs):
        self.logger.error(self.logpfx, *args, **kwargs)
        return

    def warn(self, *args, **kwargs):
        self.logger.warn(self.logpfx, *args, **kwargs)
        return

    def debug(self, *args, **kwargs):
        self.logger.debug(self.logpfx, *args, **kwargs)
        return

    def verbose(self, *args, **kwargs):
        self.logger.verbose(self.logpfx, *args, **kwargs)
        return

    def GetLogPrefix(self):
        return self.logger.GetLogPrefix() + ' ' + self.logpfx
   
    def ShowScapyObject(self, scapyobj):
        scapyobj.show2(indent = 0,
                        label_lvl = self.GetLogPrefix())

    def __generate_packets_from_spec(self, pspec):
        pktspec = getattr(pspec, 'packet', None)
        if pktspec is None: return
        if pktspec.id is None: return
        packet = pktfactory.Packet(self, pktspec)
        packet.Build(self)
        self.packets.Add(packet)
        return

    def __generate_packets_from_callback(self, pspec):
        cb = getattr(pspec, 'callback', None)
        if cb is None: return
        pkts = cb.call(self)
        if pkts:
            self.packets.SetAll(pkts)
        return

    def __generate_packets(self):
        self.info("Generating Packet Objects")
        if self.testspec.packets == None: return
        for pspec in self.testspec.packets:
            self.__generate_packets_from_spec(pspec)
            self.__generate_packets_from_callback(pspec)
        return

    def __generate_objects(self):
        self.__generate_packets()
        memfactory.GenerateBuffers(self)
        memfactory.GenerateDescriptors(self)
        return

    def __setup_packet(self, step_id, spkt):
        if spkt.packet.object == None:
            return

        tpkt = TestCaseTrigExpPacketObject()
        if objects.IsCallback(spkt.packet.object):
            tpkt.packet = spkt.packet.object.call(self)
            if tpkt.packet is None:
                self.info("- Packet Callback return None")
                return None
        else: # Its a reference
            tpkt.packet = spkt.packet.object.Get(self)
        
        tpkt.packet = copy.deepcopy(tpkt.packet)
        tpkt.packet.SetStepId(step_id)
        tpkt.packet.Build(self)
        return tpkt

    def __setup_packet_ports(self, tpkt, spkt):
        # Resolve the ports
        if objects.IsReference(spkt.packet.port):
            tpkt.ports = spkt.packet.port.Get(self)
        elif objects.IsCallback(spkt.packet.port):
            tpkt.ports = spkt.packet.port.call(self)
        else:
            tpkt.ports = [ spkt.packet.port ]
        return

    def __setup_packets(self, step_id, tcsn, spsn):
        if spsn.packets == None: return
        for spkt in spsn.packets:
            self.info("- Setting up  Packet: %s:" % spkt)
            tpkt = self.__setup_packet(step_id, spkt)            
            if tpkt is None:
                continue

            self.__setup_packet_ports(tpkt, spkt)
            self.info("- Adding Packet: %s, Ports :" %\
                      tpkt.packet.GID(), tpkt.ports)
            tpkt.packet.Show(self)
            tcsn.packets.append(tpkt)
        return

    def __setup_descriptors(self, tcsn, spsn):
        if spsn.descriptors == None: return
        for spec_desc_entry in spsn.descriptors:
            if spec_desc_entry.descriptor.object == None: continue
            tc_desc_spec = TestCaseTrigExpDescriptorSpec()
            tc_desc_spec.descriptor.object = spec_desc_entry.descriptor.object.Get(self)
            if objects.IsCallback(spec_desc_entry.descriptor.ring):
                tc_desc_spec.descriptor.ring = spec_desc_entry.descriptor.ring.call(self)
            else:
                tc_desc_spec.descriptor.ring   = spec_desc_entry.descriptor.ring.Get(self)
            if tc_desc_spec.descriptor.ring == None: continue
            self.info("- Adding Descriptor: %s, Ring: %s" %\
                      (tc_desc_spec.descriptor.object.GID(), tc_desc_spec.descriptor.ring.GID()))
            buff = getattr(spec_desc_entry.descriptor, 'buffer', None)
            if buff:
                tc_desc_spec.descriptor.buffer = spec_desc_entry.descriptor.buffer.Get(self)
                self.info("  - Expected Buffer: %s" %\
                          tc_desc_spec.descriptor.buffer.GID())
                packet = getattr(spec_desc_entry.descriptor, 'packet', None)
                if packet:
                    tc_desc_spec.descriptor.packet = spec_desc_entry.descriptor.packet.Get(self)
                    self.info("  - Expected Packet: %s" %\
                              tc_desc_spec.descriptor.packet.GID())
            tcsn.descriptors.append(tc_desc_spec)
        return
 
    def __setup_doorbell(self, tcsn, spsn):
        if spsn.doorbell == None: return
        if spsn.doorbell.object == None: return
        if objects.IsCallback(spsn.doorbell.object):
            tcsn.doorbell.object = spsn.doorbell.object.call(self)
        else:
            tcsn.doorbell.object = spsn.doorbell.object.Get(self)
        tcsn.doorbell.spec = spsn.doorbell.fields
        self.info("- Adding Doorbell: %s" % tcsn.doorbell.object.GID())
        return
   
    def __setup_config_objects(self, tcsn, spsn):
        if not hasattr(spsn, "configs") or spsn.configs == None:
            return
        for spec_config_entry in spsn.configs:
            if spec_config_entry.object == None:
                continue
            tc_config = TestCaseTrigExpConfigObject()
            tc_config.actual_object = spec_config_entry.object.Get(self)
            tc_config.original_object = copy.copy(tc_config.actual_object)
            tc_config.method = spec_config_entry.method
            tc_config.spec = getattr(spec_config_entry, "fields", None)
            tcsn.configs.append(tc_config)
   
    #def __setup_ignore_excess_packets(self, tcsn, spsn):
    #    tcsn.ignore_excess_packets = getattr(spsn, 'ignore_excess_packets', False)

    def __setup_delay(self, tcsn, spsn):
        spdelay = getattr(spsn, 'delay', 0)
        if objects.IsCallback(spdelay):
            tcsn.delay = spdelay.call(self)
        elif objects.IsReference(spdelay):
            tcsn.delay = spdelay.Get(self)
        else:
            tcsn.delay = spdelay
        if tcsn.delay:
            self.info("  - Adding Delay of %d seconds." % tcsn.delay)
        return

    def __setup_trigger(self, tcstep, spstep):
        self.info("- Setting up Trigger.")
        if spstep.trigger == None: return
        self.__setup_delay(tcstep.trigger, spstep.trigger)
        self.__setup_packets(tcstep.step_id, tcstep.trigger, spstep.trigger)
        self.__setup_descriptors(tcstep.trigger, spstep.trigger)
        self.__setup_config_objects(tcstep.trigger, spstep.trigger)
        self.__setup_doorbell(tcstep.trigger, spstep.trigger)
        return defs.status.SUCCESS

    def __setup_expect(self, tcstep, spstep):
        self.info("- Setting up Expect.")
        if spstep.expect == None: return
        self.__setup_delay(tcstep.expect, spstep.expect)
        #self.__setup_ignore_excess_packets(tcstep.expect, spstep.expect)
        self.__setup_packets(tcstep.step_id, tcstep.expect, spstep.expect)
        self.__setup_descriptors(tcstep.expect, spstep.expect)
        self.__setup_config_objects(tcstep.expect, spstep.expect)
        if hasattr(spstep.expect, "callback"):
            assert(objects.IsCallback(spstep.expect.callback))
            tcstep.expect.callback = spstep.expect.callback
        return defs.status.SUCCESS

    def __setup_session(self):
        self.info("Setting Up Session")
        for tspstep in self.testspec.session:
            step = TestCaseSessionStepObject()
            step.step_id = self.step_id
            self.step_id += 1

            self.module.RunModuleCallback('TestCaseStepSetup', self, step)
            self.info("- Setting Up Session Step")
            self.__setup_trigger(step, tspstep.step)
            self.__setup_expect(step, tspstep.step)

            self.session.AddStep(step)
        return defs.status.SUCCESS

    def __setup_callback(self):
        self.module.RunModuleCallback('TestCaseSetup', self)
        ModelConnector.TestCaseBegin(self.GID(), self.loopid)
        return

    def __show_config_objects(self):
        self.__root.ShowTestcaseConfig(self.config, self)
        return
    
    def __generate(self):
        self.info("%s Starting TestCase. %s" % ('=' * 20, '=' * 20))
        self.__show_config_objects()
        self.__setup_callback()
        self.__generate_objects()
        self.__setup_session()
        return defs.status.SUCCESS

    def __process_verify_callback_retval(self, ret):
        if ret is defs.status.SUCCESS or ret is True:
            self.info("- Verify Callback Status = Success")
            status =  defs.status.SUCCESS
        else:
            self.error("- Verify Callback Status = Failure")
            status = defs.status.ERROR
        return status

    def VerifyCallback(self):
        if GlobalOptions.skipverify: return defs.status.SUCCESS
        self.info("Invoking TestCaseVerify.")
        ret = self.module.RunModuleCallback('TestCaseVerify', self)
        return self.__process_verify_callback_retval(ret)

    def TeardownCallback(self):
        self.info("Invoking TestCaseTeardown.")
        self.module.RunModuleCallback('TestCaseTeardown', self)
        ModelConnector.TestCaseEnd(self.GID(), self.loopid)
        self.coverage.Process()
        return

    def TriggerCallback(self):
        self.info("Invoking TestCaseTrigger.")
        self.module.RunModuleCallback('TestCaseTrigger', self)
        return

    def StepSetupCallback(self, step):
        self.info("Invoking TestCaseStepSetup.")
        self.module.RunModuleCallback('TestCaseStepSetup', self, step)
        return

    def StepTriggerCallback(self, step):
        self.info("Invoking TestCaseStepTrigger.")
        self.module.RunModuleCallback('TestCaseStepTrigger', self, step)
        return

    def StepVerifyCallback(self, step):
        if GlobalOptions.skipverify: return defs.status.SUCCESS
        self.info("Invoking TestCaseStepVerify.")
        ret = self.module.RunModuleCallback('TestCaseStepVerify', self, step)
        return self.__process_verify_callback_retval(ret)

    def StepTeardownCallback(self, step):
        self.info("Invoking TestCaseStepTeardown.")
        self.module.RunModuleCallback('TestCaseStepTeardown', self, step)
        return
