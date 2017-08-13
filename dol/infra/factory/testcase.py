#! /usr/bin/python3
import pdb

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
        self.packet = None
        self.ports  = None
        return

class TestCaseTrigExpDescriptorObject:
    def __init__(self):
        self.object = None
        self.ring   = None
        return

class TestCaseSessionEntryObject:
    def __init__(self):
        self.step = None
        return

class TestCaseSessionStepTriggerExpectReceivedObject:
    def __init__(self):
        self.packets        = []
        self.descriptors    = []
        return

class TestCaseSessionStepObject:
    def __init__(self):
        self.trigger = TestCaseSessionStepTriggerExpectReceivedObject()
        self.expect  = TestCaseSessionStepTriggerExpectReceivedObject()
        self.received = TestCaseSessionStepTriggerExpectReceivedObject()
        return

class TestCase(objects.FrameworkObject):
    def __init__(self, tcid, flow, infra_data, module):
        super().__init__()
        self.template = FactoryStore.testobjects.Get('TESTCASE')
        self.Clone(self.template)
        self.LockAttributes()
        
        self.ID(tcid)
        self.__init_config(flow)

        self.packets        = objects.ObjectDatabase(logger = self)
        self.descriptors    = objects.ObjectDatabase(logger = self)
        self.buffers        = objects.ObjectDatabase(logger = self)

        self.module         = module
        self.infra_data     = infra_data
        self.testspec       = module.testspec
        self.logger         = infra_data.Logger
        self.session        = []

        self.logpfx         = "TC%06d:" % self.ID()
        self.__generate()
        return

    def __init_config(self, flow):
        self.config.flow    = flow
        
        self.config.src.tenant  = flow.GetSrcTenant()
        self.config.dst.tenant  = flow.GetDstTenant()
       
        self.config.src.segment = flow.GetSrcSegment()
        self.config.dst.segment = flow.GetDstSegment()

        self.config.src.endpoint = flow.GetSrcEndpoint()
        self.config.dst.endpoint = flow.GetDstEndpoint()
        return

    def info(self, *args, **kwargs):
        self.logger.info(self.logpfx, *args, **kwargs)
        return

    def debug(self, *args, **kwargs):
        self.logger.debug(self.logpfx, *args, **kwargs)
        return

    def verbose(self, *args, **kwargs):
        self.logger.verbose(self.logpfx, *args, **kwargs)
        return

    def GetLogPrefix(self):
        return self.logger.GetLogPrefix() + ' ' + self.logpfx

    def __generate_objects(self):
        pktfactory.GeneratePackets(self)
        memfactory.GenerateDescriptors(self)
        memfactory.GenerateBuffers(self)
        return
        
    def __setup_packets(self, tc_section, spec_section):
        for spec_pkt in spec_section.packets:
            if spec_pkt.packet.object == None: continue
            tc_pkt = TestCaseTrigExpPacketObject()
            tc_pkt.packet = spec_pkt.packet.object.Get(self)
            if objects.IsReference(spec_pkt.packet.port):
                tc_pkt.ports = spec_pkt.packet.port.Get(self)
            else:
                tc_pkt.ports = [ spec_pkt.packet.port ]
            self.info("    - Adding Packet: %s" % tc_pkt.packet.GID())
            self.info("    -        Ports :", tc_pkt.ports)
            tc_section.packets.append(tc_pkt)
        return

    def __setup_descriptors(self, tc_section, spec_section):
        for spec_desc_entry in spec_section.descriptors:
            if spec_desc_entry.descriptor.object == None: continue
            tc_desc = TestCaseTrigExpDescriptorObject()
            tc_desc.object  = spec_desc_entry.descriptor.object.Get(self)
            tc_desc.ring    = spec_desc_entry.descriptor.ring.Get(self)
            self.info("    - Adding Descriptor: %s, Ring: %s" %\
                      (tc_desc.object.GID(), tc_desc.ring.GID()))
            tc_section.descriptors.append(tc_desc)
            return
    
    def __setup_trigger(self, tc_ssn_step, spec_ssn_step):
        self.info("  - Setting up Trigger.")
        self.__setup_packets(tc_ssn_step.trigger,
                             spec_ssn_step.trigger)
        self.__setup_descriptors(tc_ssn_step.trigger,
                                 spec_ssn_step.trigger)
        return defs.status.SUCCESS

    def __setup_expect(self, tc_ssn_step, spec_ssn_step):
        self.info("  - Setting up Expect.")
        self.__setup_packets(tc_ssn_step.expect,
                             spec_ssn_step.expect)
        self.__setup_descriptors(tc_ssn_step.expect,
                                 spec_ssn_step.expect)
        if hasattr(spec_ssn_step.expect, "callback"):
            assert(objects.IsCallback(spec_ssn_step.expect.callback))
            tc_ssn_step.expect.callback = spec_ssn_step.expect.callback
        return defs.status.SUCCESS

    def __setup_session(self):
        self.info("Setting Up Session")
        for spec_ssn_entry in self.testspec.session:
            tc_ssn_entry = TestCaseSessionEntryObject()
            tc_ssn_entry.step = TestCaseSessionStepObject()
            
            self.info("- Setting Up Session Step")
            self.__setup_trigger(tc_ssn_entry.step,
                                 spec_ssn_entry.step)

            self.__setup_expect(tc_ssn_entry.step,
                                spec_ssn_entry.step)

            self.session.append(tc_ssn_entry)
        return defs.status.SUCCESS

    def __generate(self):
        self.info("Generating TestCase")
        self.__generate_objects()
        self.__setup_session()
        return defs.status.SUCCESS

    def verify_callback(self):
        return defs.status.SUCCESS
