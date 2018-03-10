from infra.factory.testcase import *
import infra.common.loader      as loader

from infra.engine.trigger2  import E2ETriggerEngine
from infra.engine.verif import E2EVerifEngine

class E2ETestCase(TestCase):
    def __init__(self, tcid, root, module, loopid = 0):
        super().__init__(tcid, root, module, loopid)
        self.trigger_engine = E2ETriggerEngine
        self.verif_engine = E2EVerifEngine
        return

    def __setup_command(self, command):
        cmd = TestCaseTrigExpCommandObject()
        if objects.IsCallback(command.endpoint):
            cmd.object = endpoint.endpoint.call(self)
            if cmd.object is None:
                self.info("- Object Callback return None")
                return None
        else: # Its a reference
            cmd.object = command.endpoint.Get(self)
            
        command_with_args = [command.cmd]
        if command.args:
            for arg in command.args:
                if objects.IsReference(arg.arg):
                    command_with_args.append(arg.arg.Get(self))
                elif objects.IsCallback(arg.arg):
                    command_with_args.append(arg.arg.call(self))
                else:
                    command_with_args.append(arg.arg)
        cmd.command = " ".join(command_with_args)
        if hasattr(command, "background"):
            cmd.background = command.background
        return cmd
   
    def __setup_commands(self, tcsn, spsn):
        if spsn.commands == None: return
        for cmd in spsn.commands:
            tc_cmd = self.__setup_command(cmd.command)
            tcsn.commands.append(tc_cmd)
    
    def _setup_trigger(self, tcstep, spstep):
        self.info("- Setting up Trigger.")
        if spstep.trigger == None: return
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
        self._generate()