#! /usr/bin/python3
import pdb
import copy

import infra.common.loader      as loader
import infra.common.defs        as defs
import infra.common.parser      as parser
import infra.common.objects     as objects
import infra.factory.pktfactory as pktfactory
import infra.factory.memfactory as memfactory

from infra.factory.store    import FactoryStore as FactoryStore
from infra.common.logging   import logger
from infra.misc.coverage    import TestCaseCoverageHelper
from infra.common.glopts    import GlobalOptions as GlobalOptions
from infra.asic.model       import ModelConnector

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

class TestCaseTrigExpCommandObject:
    def __init__(self):
        self.object     = None
        self.command    = None
        self.background = False
        self.timeout    = None
        self.status     = None
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
        self.commands              = []
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
        self.__retry_enable = False
        self.trigger_engine = None
        self.verif_engine = None
        self.rxpkts = {}
        self.LockAttributes()

        self.GID(tcid)
        
        self.module         = module
        self.infra_data     = module.infra_data
        self.testspec       = module.testspec
        self.session        = TestCaseSessionObject()
        self.step_id        = 0
        self.pvtdata        = TestCasePrivateData()

        self.loopid = loopid + 1
        if GlobalOptions.tcscale:
            self.logpfx = "TC%06d-%04d:" % (self.ID(), self.loopid)
        else:
            self.logpfx = "TC%06d" % self.ID()  
        logger.SetTestcase(self.logpfx)
        return
        
    def _setup_config(self, root):
        self.__root = root
        if root:
            root.SetupTestcaseConfig(self.config)
        return
    
    def IsIgnore(self):
        return self.module.IsIgnore()
                
    def _generate(self):
        logger.info("%s Starting TestCase. %s" % ('=' * 20, '=' * 20))
        self.__show_config_objects()
        self.__setup_callback()
        if self.testspec:
            self._generate_objects()
            self.__setup_session()
        return defs.status.SUCCESS
    
    def IsRetryEnabled(self):
        return self.__retry_enable

    def SetRetryEnabled(self, val):
        self.__retry_enable = val
        return

    def GetStepId(self):
        return self.step_id

    def GetTcId(self):
        return self.GID()

    def GetRoot(self):
        return self.__root

    def _generate_objects(self):
        pass
    
    def __setup_callback(self):
        self.module.RunModuleCallback('TestCaseSetup', self)
        ModelConnector.TestCaseBegin(self.GID(), self.loopid)
        return

    def __show_config_objects(self):
        if self.__root:
            self.__root.ShowTestcaseConfig(self.config)
        return

    def __setup_session(self):
        logger.info("Setting Up Session")
        for tspstep in self.testspec.session:
            step = TestCaseSessionStepObject()
            step.step_id = self.step_id
            self.step_id += 1

            self.module.RunModuleCallback('TestCaseStepSetup', self, step)
            logger.info("- Setting Up Session Step")
            self._setup_trigger(step, tspstep.step)
            self._setup_expect(step, tspstep.step)

            self.session.AddStep(step)
        return defs.status.SUCCESS
    
    def __process_verify_callback_retval(self, ret):
        if ret is defs.status.SUCCESS or ret is True:
            logger.info("- Verify Callback Status = Success")
            status =  defs.status.SUCCESS
        elif ret is defs.status.OVERRIDE:
            logger.info("- Verify Callback Status = Override")
            status =  defs.status.OVERRIDE
        else:
            logger.error("- Verify Callback Status = Failure")
            status = defs.status.ERROR
        return status

    def _setup_trigger(self, tcstep, spstep):
        pass

    def _setup_expect(self, tcstep, spstep):
        pass
    
    def TeardownCallback(self):
        logger.info("Invoking TestCaseTeardown.")
        self.module.RunModuleCallback('TestCaseTeardown', self)
        if self.__root:
            self.__root.TearDownTestcaseConfig(self.config)
        return
    
    def VerifyCallback(self):
        if GlobalOptions.skipverify: return defs.status.SUCCESS
        logger.info("Invoking TestCaseVerify.")
        ret = self.module.RunModuleCallback('TestCaseVerify', self)
        return self.__process_verify_callback_retval(ret)

    def PreTriggerCallback(self):
        logger.info("Invoking TestCasePreTrigger.")
        self.module.RunModuleCallback('TestCasePreTrigger', self)
        return

    def TriggerCallback(self):
        logger.info("Invoking TestCaseTrigger.")
        self.module.RunModuleCallback('TestCaseTrigger', self)
        return

    def StepSetupCallback(self, step):
        logger.info("Invoking TestCaseStepSetup.")
        self.module.RunModuleCallback('TestCaseStepSetup', self, step)
        return

    def StepPreTriggerCallback(self, step):
        logger.info("Invoking TestCaseStepPreTrigger.")
        self.module.RunModuleCallback('TestCaseStepPreTrigger', self, step)
        return

    def StepTriggerCallback(self, step):
        logger.info("Invoking TestCaseStepTrigger.")
        self.module.RunModuleCallback('TestCaseStepTrigger', self, step)
        return

    def StepVerifyCallback(self, step):
        if GlobalOptions.skipverify: return defs.status.SUCCESS
        logger.info("Invoking TestCaseStepVerify.")
        ret = self.module.RunModuleCallback('TestCaseStepVerify', self, step)
        return self.__process_verify_callback_retval(ret)

    def StepTeardownCallback(self, step):
        logger.info("Invoking TestCaseStepTeardown.")
        self.module.RunModuleCallback('TestCaseStepTeardown', self, step)
        return

   
