import os
from infra.factory.testcase import *
import infra.common.loader      as loader

from infra.engine.trigger2  import E2ETriggerEngine
from infra.engine.verif import E2EVerifEngine

class TestCaseTrigExpCommandObject:
    def __init__(self):
        self.object       = None
        self.setups       = []
        self.command      = None
        self.background   = False
        self.timeout      = None
        self.ignore_error = False;
        self.status       = None
        return

class TestCaseSessionStepTriggerExpectReceivedObject:
    def __init__(self):
        self.setups   = []
        self.commands = []
        #self.ignore_excess_packets = False
        return

class TestCaseEndpointSetUpObject:
    def __init__(self):
        self.endpoint = None
        self.files    = []

class E2ETestCase(TestCase):
    def __init__(self, tcid, root, module, loopid = 0):
        super().__init__(tcid, root, module, loopid)
        self.trigger_engine = E2ETriggerEngine
        self.verif_engine = E2EVerifEngine
        if hasattr(self.testspec, "package"):
            self.pvtdata.path = self.testspec.package.replace(".", "/")
        return

    def __get_endpoint(self, endpoint):
        if objects.IsCallback(endpoint):
            return endpoint.call(self)
        else: # Its a reference
            return endpoint.Get(self)
                    
    def __setup_command(self, command):
        cmd = TestCaseTrigExpCommandObject()
        cmd.object = self.__get_endpoint(command.endpoint)
        command_with_args = [command.cmd]
        if command.args:
            for arg in command.args:
                if objects.IsReference(arg.arg):
                    command_with_args.append(str(arg.arg.Get(self)))
                elif objects.IsCallback(arg.arg):
                    command_with_args.append(arg.arg.call(self))
                else:
                    command_with_args.append(arg.arg)
        try:
            cmd.command = " ".join(command_with_args)
        except:
            pdb.set_trace()
        if hasattr(command, "background"):
            cmd.background = command.background
        if hasattr(command, "timeout"):
            cmd.timeout = command.timeout            
        cmd.ignore_error = getattr(command, "ignore_error", False)
        return cmd
   
    def __setup_setup(self, tcsetup):
        setup = TestCaseEndpointSetUpObject()
        setup.endpoint = self.__get_endpoint(tcsetup.endpoint)
        ws_top = os.environ["WS_TOP"]
        dtop = ws_top + defs.DOL_PATH
        for file in tcsetup.files:
            filename = "%s/%s/%s" % (dtop, self.pvtdata.path, file.file)
            setup.files.append(filename)
            logger.verbose("Created Full Path = %s" % filename)
        return setup
    
    def __setup_setups(self, tcsn, spsn):
        if not hasattr(spsn, "setups") or spsn.setups == None: return
        logger.info("Setting up setups.")
        tcsn.setups = []
        for setup in spsn.setups:
            tc_setup = self.__setup_setup(setup.setup)
            tcsn.setups.append(tc_setup)
        
    def __setup_commands(self, tcsn, spsn):
        if spsn.commands == None: return
        for cmd in spsn.commands:
            tc_cmd = self.__setup_command(cmd.command)
            tcsn.commands.append(tc_cmd)
    
    def _setup_trigger(self, tcstep, spstep):
        logger.info("- Setting up Trigger.")
        if spstep.trigger == None: return
        self.__setup_setups(tcstep.trigger, spstep.trigger)
        super()._setup_trigger(tcstep, spstep)
        self.__setup_commands(tcstep.trigger, spstep.trigger)
        return defs.status.SUCCESS

    def _setup_expect(self, tcstep, spstep):
        super()._setup_expect(tcstep, spstep)
        tcstep.expect.commands = tcstep.trigger.commands
        #self.__setup_commands(tcstep.trigger, spstep.trigger)
        return defs.status.SUCCESS

    def SetUp(self):
        root = self.testspec.selectors.root.Get(ConfigStore)
        module_hdl = loader.ImportModule(root.meta.package, root.meta.module)
        assert(module_hdl)
        config_obj = module_hdl.GenerateMatchingConfigObject(self.testspec.selectors)
        #Config should be generated for the testcase.
        assert(config_obj)
        self._setup_config(config_obj)
        super()._generate()
