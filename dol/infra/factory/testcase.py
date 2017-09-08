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

class TestCaseSessionEntryObject:
    def __init__(self):
        self.step = None
        return

class TestCaseSessionStepTriggerExpectReceivedObject:
    def __init__(self):
        self.packets        = []
        self.descriptors    = []
        self.doorbell       = TestCaseTrigExpDoorbellObject()
        return

class TestCaseSessionStepObject:
    def __init__(self):
        self.trigger = TestCaseSessionStepTriggerExpectReceivedObject()
        self.expect  = TestCaseSessionStepTriggerExpectReceivedObject()
        self.received = TestCaseSessionStepTriggerExpectReceivedObject()
        return

class TestCasePrivateData:
    def __init__(self):
        return

class TestCase(objects.FrameworkObject):
    def __init__(self, tcid, config_root_obj, infra_data, module):
        super().__init__()
        self.template = FactoryStore.testobjects.Get('TESTCASE')
        self.Clone(self.template)
        self.__config_root_obj = None
        self.LockAttributes()
        
        self.GID(tcid)
        self.__setup_config(config_root_obj)

        self.packets        = objects.ObjectDatabase(logger = self)
        self.descriptors    = objects.ObjectDatabase(logger = self)
        self.buffers        = objects.ObjectDatabase(logger = self)
        self.objects        = objects.ObjectDatabase(logger = self)

        self.module         = module
        self.infra_data     = infra_data
        self.testspec       = module.testspec
        self.logger         = infra_data.Logger
        self.session        = []
        self.step_id        = 0
        self.pvtdata        = TestCasePrivateData()

        self.logpfx         = "TC%06d:" % self.ID()
        self.__generate()
        return

    def __setup_config(self, config_root_obj):
        self.__config_root_obj = config_root_obj
        config_root_obj.SetupTestcaseConfig(self.config)
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

    def __generate_packets(self):
        self.info("Generating Packet Objects")
        for pspec in self.testspec.packets:
            packet = pktfactory.Packet(self, pspec.packet)
            packet.Build(self)
            self.packets.Add(packet)
        return

    def __generate_objects(self):
        self.__generate_packets()
        memfactory.GenerateBuffers(self)
        memfactory.GenerateDescriptors(self)
        return
        
    def __setup_packets(self, step_id, tcsn, spsn):
        if spsn.packets == None: return
        for spec_pkt in spsn.packets:
            if spec_pkt.packet.object == None: continue
            tc_pkt = TestCaseTrigExpPacketObject()
            # Resolve the packet.
            if objects.IsCallback(spec_pkt.packet.object):
                tc_pkt.packet = spec_pkt.packet.object.call(self)
                if tc_pkt.packet == None: return
            else:
                tc_pkt.packet = spec_pkt.packet.object.Get(self)
            
            tc_pkt.packet = copy.deepcopy(tc_pkt.packet)
            tc_pkt.packet.SetStepId(step_id)
            tc_pkt.packet.Build(self)

            # Resolve the ports
            if objects.IsReference(spec_pkt.packet.port):
                tc_pkt.ports = spec_pkt.packet.port.Get(self)
            elif objects.IsCallback(spec_pkt.packet.port):
                tc_pkt.ports = spec_pkt.packet.port.call(self)
            else:
                tc_pkt.ports = [ spec_pkt.packet.port ]
            self.info("- Adding Packet: %s, Ports :" %\
                      tc_pkt.packet.GID(), tc_pkt.ports)
            tc_pkt.packet.Show(self)
            tcsn.packets.append(tc_pkt)
        return

    def __setup_descriptors(self, tcsn, spsn):
        if spsn.descriptors == None: return
        for spec_desc_entry in spsn.descriptors:
            if spec_desc_entry.descriptor.object == None: continue
            tc_desc_spec = TestCaseTrigExpDescriptorSpec()
            tc_desc_spec.descriptor.object = spec_desc_entry.descriptor.object.Get(self)
            tc_desc_spec.descriptor.ring   = spec_desc_entry.descriptor.ring.Get(self)
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
        tcsn.doorbell.object = spsn.doorbell.object.Get(self)
        tcsn.doorbell.spec = spsn.doorbell.fields
        self.info("- Adding Doorbell: %s" % tcsn.doorbell.object.GID())
        return
   
    def __setup_trigger(self, tcstep, spstep):
        self.info("- Setting up Trigger.")
        self.__setup_packets(tcstep.step_id, tcstep.trigger, spstep.trigger)
        self.__setup_descriptors(tcstep.trigger, spstep.trigger)
        self.__setup_doorbell(tcstep.trigger, spstep.trigger)
        return defs.status.SUCCESS

    def __setup_expect(self, tcstep, spstep):
        self.info("- Setting up Expect.")
        if spstep.expect == None: return
        self.__setup_packets(tcstep.step_id, tcstep.expect, spstep.expect)
        self.__setup_descriptors(tcstep.expect, spstep.expect)
        if hasattr(spstep.expect, "callback"):
            assert(objects.IsCallback(spstep.expect.callback))
            tcstep.expect.callback = spstep.expect.callback
        return defs.status.SUCCESS

    def __setup_session(self):
        self.info("Setting Up Session")
        for spec_ssn_entry in self.testspec.session:
            tc_ssn_entry = TestCaseSessionEntryObject()
            tc_ssn_entry.step = TestCaseSessionStepObject()
            tc_ssn_entry.step.step_id = self.step_id
            self.step_id += 1

            self.info("- Setting Up Session Step")
            self.__setup_trigger(tc_ssn_entry.step,
                                 spec_ssn_entry.step)

            self.__setup_expect(tc_ssn_entry.step,
                                spec_ssn_entry.step)

            self.session.append(tc_ssn_entry)
        return defs.status.SUCCESS

    def __setup_callback(self):
        self.module.RunModuleCallback('TestCaseSetup', self)
        return

    def __teardown_callback(self):
        self.module.RunModuleCallback('TestCaseTeardown', self)
        return

    def __show_config_objects(self):
        self.__config_root_obj.ShowTestcaseConfig(self.config, self)
        return
    
    def __generate(self):
        self.info("%s Starting TestCase. %s" % ('=' * 20, '=' * 20))
        self.__show_config_objects()
        self.__setup_callback()
        self.__generate_objects()
        self.__setup_session()
        return defs.status.SUCCESS

    def verify_callback(self):
        if self.module.RunModuleCallback('TestCaseVerify', self):
            status =  defs.status.SUCCESS
        else:
            status = defs.status.ERROR
        self.__teardown_callback()
        return status
