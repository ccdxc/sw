#! /usr/bin/python3
import pdb
import copy
import os
import signal
import subprocess
import time

import iota.harness.infra.utils.timeprofiler as timeprofiler
import iota.harness.infra.types as types
import iota.harness.infra.utils.loader as loader
import iota.harness.api as api
import iota.harness.infra.utils.utils as utils

from iota.harness.infra.utils.logger import Logger as Logger
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions
from iota.harness.infra.exceptions import *
from iota.harness.infra.utils import periodic_timer as taskmgr

gl_owner_db = {}

defaultTcTimeout = 3600

def tcTimeout():
    def alarmWrapper(func):
        def alarmHandler(sig,frame):
            Logger.error("testcase {0} timed out".format(func.__name__))
            raise TestcaseTimeoutException()
        def wrapped(*args, **kwargs):
            signal.signal(signal.SIGALRM,alarmHandler)
            try: seconds=args[0].GetTimeout()
            except: seconds=defaultTcTimeout
            Logger.info("testcase timeout: {0}".format(seconds))
            signal.alarm(seconds)
            try:
                return func(*args, **kwargs)
            finally:
                signal.alarm(0)
        return wrapped
    return alarmWrapper

def get_owner(filename):
    global gl_owner_db
    if filename in gl_owner_db:
        return gl_owner_db[filename]

    try:
        result = subprocess.run([ "git", "log", "--format=%an", "%s" % filename], stdout=subprocess.PIPE)
        stdout = result.stdout.decode('utf-8')
        # Use the name of the creator of the file as the owner.
        owner_full_name = stdout.split('\n')[-2]
        owner = owner_full_name.split(' ')[0]
        gl_owner_db[filename] = owner
    except:
        owner = "NA"
    return owner

class SetupStep:
    def __init__(self, spec):
        self.__spec = spec
        self.__timer = timeprofiler.TimeProfiler()
        self.__resolve()
        self.__status = api.types.status.FAILURE
        return

    def __resolve(self):
        Logger.debug("Resolving testcase setup module: %s" % self.__spec.step)
        self.__mod = loader.Import(self.__spec.step, self.__spec.packages)
        return

    def __execute(self):
        Logger.debug("Running testcase setup module: %s" % self.__spec.step)
        return loader.RunCallback(self.__mod, 'Main', True, None)

    def Main(self):
        self.__timer.Start()
        self.__status = self.__execute()
        self.__timer.Stop()
        if self.__status != api.types.status.SUCCESS:
            return api.types.status.FAILURE
        return self.__status

    def __get_owner(self):
        return get_owner(self.__mod.__file__)

    def PrintResultSummary(self):
        modname = "- %s" % self.__mod.__name__.split('.')[-1]
        print(types.FORMAT_TESTCASE_SUMMARY % (modname, self.__get_owner(), types.status.str(self.__status).title(), self.__timer.TotalTime()))
        return types.status.SUCCESS

class VerifStep:
    def __init__(self, spec):
        self.__spec = spec
        self.__timer = timeprofiler.TimeProfiler()
        self.__resolve()
        self.__status = api.types.status.FAILURE
        return

    def __resolve(self):
        Logger.debug("Resolving testcase verif module: %s" % self.__spec.step)
        self.__mod = loader.Import(self.__spec.step, self.__spec.packages)
        return

    def __execute(self):
        Logger.debug("Running testcase verif module: %s" % self.__spec.step)
        return loader.RunCallback(self.__mod, 'Main', True, None)

    def Main(self):
        self.__timer.Start()
        self.__status = self.__execute()
        self.__timer.Stop()
        if self.__status != api.types.status.SUCCESS:
            return api.types.status.FAILURE
        return self.__status

    def __get_owner(self):
        return get_owner(self.__mod.__file__)

    def PrintResultSummary(self):
        modname = "- %s" % self.__mod.__name__.split('.')[-1]
        print(types.FORMAT_TESTCASE_SUMMARY % ("",modname, self.__get_owner(), types.status.str(self.__status).title(), self.__timer.TotalTime()))
        return types.status.SUCCESS

class DebugStep:
    def __init__(self, spec):
        self.__spec = spec
        self.__timer = timeprofiler.TimeProfiler()
        self.__resolve()
        self.__status = api.types.status.FAILURE
        return

    def __resolve(self):
        Logger.debug("Resolving testcase debug module: %s" % self.__spec.step)
        self.__mod = loader.Import(self.__spec.step, self.__spec.packages)
        return

    def __execute(self, iter_data):
        Logger.debug("Running testcase debug module: %s" % self.__spec.step)
        return loader.RunCallback(self.__mod, 'Main', True, iter_data)

    def Main(self, iter_data):
        self.__timer.Start()
        self.__status = self.__execute(iter_data)
        self.__timer.Stop()
        if self.__status != api.types.status.SUCCESS:
            return api.types.status.FAILURE
        return self.__status

    def __get_owner(self):
        return get_owner(self.__mod.__file__)

    def PrintResultSummary(self):
        modname = "- %s" % self.__mod.__name__.split('.')[-1]
        print(types.FORMAT_TESTCASE_SUMMARY % (modname, self.__get_owner(), types.status.str(self.__status).title(), self.__timer.TotalTime()))
        return types.status.SUCCESS

class TriggerStep:
    def __init__(self, spec):
        self.__spec = spec
        self.__timer = timeprofiler.TimeProfiler()
        self.__trigger = 'auto'
        self.__run = 'once'
        self.__sleep_interval = 0
        self.__terminate = 'teardown'
        self.__background = True
        self.__resolve()
        self.__status = api.types.status.FAILURE
        return

    def __resolve(self):
        Logger.debug("Resolving testcase trigger  module: %s" % self.__spec.step)
        self.__mod = loader.Import(self.__spec.step, self.__spec.packages)
        if hasattr(self.__spec, 'args') and hasattr(self.__spec.args, 'trigger'):
            self.__trigger = self.__spec.args.trigger

        if hasattr(self.__spec, 'args') and hasattr(self.__spec.args, 'run'):
            self.__run = self.__spec.args.run

        if hasattr(self.__spec, 'args') and hasattr(self.__spec.args, 'sleep_interval'):
            self.__sleep_interval = self.__spec.args.sleep_interval

        if hasattr(self.__spec, 'args') and hasattr(self.__spec.args, 'terminate'):
            self.__terminate = self.__spec.args.terminate

        if hasattr(self.__spec, 'args') and hasattr(self.__spec.args, 'background'):
            self.__background = self.__spec.args.background
        return

    def __execute(self, iter_data):
        if self.__background:
            Logger.debug("Running common trigger module in background: %s" % self.__spec.step)
            bt_inst = TestcaseBackgroundTrigger('Main', self.__trigger, self.__run, 
                                                 self.__sleep_interval, self.__terminate)
            return bt_inst.StartTask(self.__mod, iter_data)
        else:
            Logger.debug("Running common trigger module in foreground: %s" % self.__spec.step)
            return loader.RunCallback(self.__mod, 'Main', True, iter_data)

    def Main(self, iter_data):
        self.__timer.Start()
        self.__status = self.__execute(iter_data)
        self.__timer.Stop()
        if self.__status != api.types.status.SUCCESS:
            return api.types.status.FAILURE
        return self.__status

    def __get_owner(self):
        return get_owner(self.__mod.__file__)

    def PrintResultSummary(self):
        modname = "- %s" % self.__mod.__name__.split('.')[-1]
        print(types.FORMAT_TESTCASE_SUMMARY % (modname, self.__get_owner(), types.status.str(self.__status).title(), self.__timer.TotalTime()))
        return types.status.SUCCESS

class TeardownStep:
    def __init__(self, spec):
        self.__spec = spec
        self.__timer = timeprofiler.TimeProfiler()
        self.__trigger = 'auto'
        self.__run = 'once'
        self.__sleep_interval = 0
        self.__terminate = 'teardown'
        self.__background = True
        self.__resolve()
        self.__status = api.types.status.FAILURE
        return

    def __resolve(self):
        Logger.debug("Resolving testcase teardown module: %s" % self.__spec.step)
        self.__mod = loader.Import(self.__spec.step, self.__spec.packages)
        if hasattr(self.__spec, 'args') and hasattr(self.__spec.args, 'trigger'):
            self.__trigger = self.__spec.args.trigger

        if hasattr(self.__spec, 'args') and hasattr(self.__spec.args, 'background'):
            self.__background = self.__spec.args.background
        return

    def __execute(self, iter_data):
        if self.__background:
            Logger.debug("Running common teardown module in background: %s" % self.__spec.step)
            bt_inst = TestcaseBackgroundTrigger('Main', self.__trigger, self.__run, 
                                                 self.__sleep_interval, self.__terminate)
            return bt_inst.StartTask(self.__mod, iter_data)
        else:
            Logger.debug("Running common teardown module in foreground: %s" % self.__spec.step)
            return loader.RunCallback(self.__mod, 'Main', True, iter_data)

    def Main(self, iter_data):
        self.__timer.Start()
        self.__status = self.__execute(iter_data)
        self.__timer.Stop()
        if self.__status != api.types.status.SUCCESS:
            return api.types.status.FAILURE
        return self.__status

    def __get_owner(self):
        return get_owner(self.__mod.__file__)

    def PrintResultSummary(self):
        modname = "- %s" % self.__mod.__name__.split('.')[-1]
        print(types.FORMAT_TESTCASE_SUMMARY % (modname, self.__get_owner(), types.status.str(self.__status).title(), self.__timer.TotalTime()))
        return types.status.SUCCESS


class IterationResults(object):
    def __init__(self):
        self.results = {}

    def addResults(self, title, iteration):
        self.results[title] = iteration

    def toJson(self):
        try:
            return json.dumps(self.results)
        except:
            Logger.debug("failed to export iteration results")
            return "{}"

class TestcaseDataIters:
    def __init__(self):
        self.__summary = None
        return

    def __add_to_summary(self, kv_summary):
        if self.__summary:
            self.__summary += kv_summary
        else:
            self.__summary = kv_summary
        return

    def AddKV(self, key, val):
        self.__add_to_summary("%s:%s," % (key, str(val)))
        return setattr(self, key, val)

    def Summary(self):
        return self.__summary

class TestcaseData:
    def __init__(self, args):
        self.__status = types.status.FAILURE
        self.__timer = timeprofiler.TimeProfiler()
        self.__test_count = 1
        self.args = args
        self.__instid = ""
        self.iterators = TestcaseDataIters()
        self.selected = None
        self.__package = None
        return

    def SetTestCount(self, count):
        self.__test_count = count
        return

    def GetTestCount(self):
        return self.__test_count

    def SetInstanceId(self, instid):
        self.__instid = instid
        self.__logs_dir = "%s/tcdata/%s/" % (api.GetTestsuiteLogsDir(), instid)
        os.system("mkdir -p %s" % self.__logs_dir)
        return

    def GetInstanceId(self):
        return self.__instid

    def GetInst(self):
        return self.__instid

    def SetStatus(self, status):
        Logger.info("setting TestcaseData {0} status to {1}".format(self.GetInst(), status))
        self.__status = status
        return

    def GetStatus(self):
        return self.__status

    def GetLogsDir(self):
        return self.__logs_dir

    def StartTime(self):
        self.__timer.Start()
        return

    def StopTime(self):
        self.__timer.Stop()
        return

    def TotalTime(self):
        return self.__timer.TotalTime()

    def SetSelected(self, selected):
        self.selected = selected

    def SetBundleStore(self, store):
        self.bundle_store = store

    def GetBundleStore(self):
        return self.bundle_store

    def GetSelector(self):
        return self.selected

    def Name(self):
        return self.__instid

    def GetPackage(self):
        return self.__package

    def SetPackage(self, package):
        self.__package = package

class TestcaseBackgroundTrigger:
    """
    Encapsulation of BackgroundTrigger :
    """

    def __init__(self, task, trigger, run, sleep_interval, terminate):
        """
        :param task: Name of task/function to invoke
        :param trigger: Trigger mode - auto/manual
        :param run: once or repeat
        :param sleep_interval: Sleep interval in sec between repeat-run. Ignored for run-once
        :param terminate: Termination at verify|teardown
        """
        self.__status = types.status.FAILURE
        self.__task = task
        self.__sleep_interval = sleep_interval
        self.__trigger = trigger
        self.__run = run
        self.__terminate = terminate
        self.__task_inst = None

    def IsAutoTriggerEnabled(self):
        return self.__trigger == 'auto'

    def StartTask(self, module, args):
        callback_fn = getattr(module, self.__task, None)
        assert(callback_fn)

        self.__task_inst = taskmgr.BackgroundTask(callback_fn, self.__sleep_interval, 
                                                  self.__run=='repeat', [args])
        self.__task_inst.setName(self.__task)
        self.__task_inst.start()
        return types.status.SUCCESS

    def StopTask(self, exec_stage):
        if self.__task_inst:
            if self.__terminate == exec_stage:
                if self.IsTaskRunning():
                    self.__task_inst.cancel()
        return types.status.SUCCESS

    def IsTaskRunning(self):
        return self.__task_inst and self.__task_inst.is_alive()

    def CollectTask(self): 
        if self.__task_inst:
            self.StopTask(self.__terminate)  # Provide another attempt to 
            self.__task_inst.join(0) # Terminate task/thread
        return

class Testcase:
    def __init__(self, spec, parent):
        self.__spec = spec
        self.__parent = parent
        self.__spec.name = self.__spec.name.replace(' ', '_')
        self.__tc = None
        self.__setups = []
        self.__verifs = []
        self.__debugs = []
        self.__triggers = []
        self.__teardowns = []
        self.__background_tasks = {}
        self.__iterid = 0
        self.__resolve()
        self.__enable = getattr(self.__spec, 'enable', True)
        self.__ignored = getattr(self.__spec, "ignore", False)
        self.__stress = getattr(self.__spec, "stress", GlobalOptions.stress)
        self.__package = self.__spec.packages
        try: self.timeout = spec.args.timeout
        except: self.timeout = defaultTcTimeout

        self.__timer = timeprofiler.TimeProfiler()
        self.__iters = []
        self.__aborted = False
        self.status = types.status.UNAVAIL
        self.iterRes = IterationResults()

        self.__setup_iters()
        self.__setup_background_tasks()
        self.__apply_stress_factor()

        self.__stats_pass = 0
        self.__stats_fail = 0
        self.__stats_ignored = 0
        self.__stats_error = 0
        self.selected = None
        return

    def __apply_stress_factor(self):
        if GlobalOptions.stress == False or self.__stress == False:
            return

        newlist = []
        for itdata in self.__iters:
            for s in range(GlobalOptions.stress_factor):
                sdata = copy.deepcopy(itdata)
                newlist.append(sdata)
        assert(len(newlist) != 0)
        self.__iters = newlist
        return

    def __get_instance_id(self, iter_id):
        return "%s_%d" % (self.__spec.name, iter_id)

    def __new_TestcaseData(self, src = None):
        if src:
            td = copy.deepcopy(src)
        else:
            td = TestcaseData(getattr(self.__spec, 'args', None))

        if not self.__enable:
            td.SetStatus(types.status.DISABLED)
        elif self.__ignored:
            td.SetStatus(types.status.IGNORED)
        td.SetPackage(self.GetPackage())
        return td

    def GetTimeout(self):
        return self.timeout

    def SetSelected(self, selected):
        self.selected = selected

    def SetBundleStore(self, store):
        self.bundle_store = store

    def GetBundleStore(self):
        return self.bundle_store

    def GetPackage(self):
        return self.__package

    def __setup_simple_iters(self, spec):
        Logger.debug("Setting up simple iterators.")
        min_len = 0
        for k,v in spec.__dict__.items():
            if isinstance(v, list):
                iter_len = len(v)
                if min_len == 0 or min_len > iter_len: min_len = iter_len
                spec.__dict__[k] = iter(v)
            else:
                spec.__dict__[k] = None

        for inst in range(min_len):
            new_data = self.__new_TestcaseData()
            for k,v in spec.__dict__.items():
                if not v: continue
                new_data.iterators.AddKV(k, next(v))
                Logger.debug("- Adding K:%s V:" % k, v)
            self.__iters.append(new_data)
        return

    def __setup_nested_iters_mul(self, key, values, curr_data_list):
        new_data_list = []
        for val in values:
            if len(curr_data_list):
                for data in curr_data_list:
                    new_data = self.__new_TestcaseData(data)
                    new_data.iterators.AddKV(key, val)
                    new_data_list.append(new_data)
            else:
                new_data = self.__new_TestcaseData()
                new_data.iterators.AddKV(key, val)
                new_data_list.append(new_data)
        return new_data_list

    def __setup_nested_iters(self, spec):
        data_list = []
        for k,v in spec.__dict__.items():
            if not isinstance(v, list): continue
            data_list = self.__setup_nested_iters_mul(k, v, data_list)
        self.__iters = data_list
        return

    def __setup_iters(self):
        spec = getattr(self.__spec, 'iterators', None)
        if spec is None:
            self.__iters.append(self.__new_TestcaseData())
            return
        iter_type = getattr(spec, 'type', None)
        if iter_type and iter_type == 'simple':
            return self.__setup_simple_iters(spec)
        elif iter_type and iter_type == 'nested':
            return self.__setup_nested_iters(spec)
        else:
            api.Logger.error("Invalid Iterator Type: %s" % iter_type)
            assert(0)
        return

    def __setup_background_tasks(self):
        tasks = getattr(self.__spec, 'background_tasks', None)
        if tasks is None:
            return

        for task_name, task_attr in tasks.__dict__.items():
            # Build BackgroundTask instance and prepare for trigger
            trigger = getattr(task_attr, 'trigger', 'auto') # or manual
            run = getattr(task_attr, 'run', 'once') # or repeat
            sleep_interval = getattr(task_attr, 'sleep_interval', 60)
            terminate = getattr(task_attr, 'terminate', 'Teardown')
            Logger.info("Task %s attributes : trigger %s, sleep %d and terminate %s" % 
                    (task_name, trigger, sleep_interval, terminate))
            bt = TestcaseBackgroundTrigger(task_name, trigger, run, sleep_interval, terminate)
            self.__background_tasks[task_name] = bt
        return

    def __resolve_testcase(self):
        Logger.debug("Resolving testcase module: %s" % self.__spec.testcase)
        self.__tc = loader.Import(self.__spec.testcase, self.__spec.packages)
        if getattr(self.__spec, "selector", None):
            self.__sel_module = loader.Import(self.__spec.selector.module, self.__spec.packages)
            self.__sel_module_args = self.__spec.selector.args
        else:
            self.__sel_module = None
        setattr(self.__tc, 'parent',  self)
        setups_spec = getattr(self.__spec, 'setups', [])
        if setups_spec is None:
            return types.status.SUCCESS
        for v in setups_spec:
            v.packages = self.__spec.packages
            setup = SetupStep(v)
            self.__setups.append(setup)
        verifs_spec = getattr(self.__spec, 'verifs', [])
        if verifs_spec is None:
            return types.status.SUCCESS
        for v in verifs_spec:
            v.packages = self.__spec.packages
            verif = VerifStep(v)
            self.__verifs.append(verif)
        debugs_spec = getattr(self.__spec, 'debugs', [])
        if debugs_spec is None:
            return types.status.SUCCESS
        for d in debugs_spec:
            d.packages = self.__spec.packages
            debug = DebugStep(d)
            self.__debugs.append(debug)
        common_triggers_spec = getattr(self.__spec, 'triggers', {})
        if common_triggers_spec is None:
            return types.status.SUCCESS
        for t in common_triggers_spec:
            t.packages = self.__spec.packages
            trigger = TriggerStep(t)
            self.__triggers.append(trigger)
        common_teardowns_spec = getattr(self.__spec, 'teardowns', {})
        if common_teardowns_spec is None:
            return types.status.SUCCESS
        for t in common_teardowns_spec:
            t.packages = self.__spec.packages
            teardown = TeardownStep(t)
            self.__teardowns.append(teardown)
        return types.status.SUCCESS

    def __resolve(self):
        ret = self.__resolve_testcase()
        if ret != types.status.SUCCESS:
            return ret
        return types.status.SUCCESS

    def __mk_testcase_directory(self, newdir):
        Logger.debug("Creating Testcase directory: %s" % newdir)
        command = "mkdir -p %s && chmod 777 %s" % (newdir, newdir)
        req = api.Trigger_CreateAllParallelCommandsRequest()
        for nodename in api.GetWorkloadNodeHostnames():
            api.Trigger_AddHostCommand(req, nodename, command)
        for wl in api.GetWorkloads():
            if api.IsWorkloadRunning(wl.workload_name):
                api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, command, timeout=60)
        resp = api.Trigger(req)
        if not api.Trigger_IsSuccess(resp):
            Logger.error("Failed to create destination directory %s" % newdir)
            return types.status.FAILURE
        return types.status.SUCCESS

    def __update_stats(self, status, count):
        if status == types.status.SUCCESS:
            self.__stats_pass += count
        elif status == types.status.FAILURE:
            self.__stats_fail += count
        elif status == types.status.IGNORED:
            self.__stats_ignored += count
        else:
            self.__stats_error += count
        return

    def __run_common_triggers(self, iter_data):
        result = types.status.SUCCESS
        for t in self.__triggers:
            status = t.Main(iter_data)
            if status != types.status.SUCCESS:
                result = status
        return result

    def __run_common_verifs(self):
        result = types.status.SUCCESS
        for s in self.__verifs:
            status = s.Main()
            if status != types.status.SUCCESS:
                result = status
        return result

    def __run_common_teardowns(self, iter_data):
        result = types.status.SUCCESS
        for t in self.__teardowns:
            status = t.Main(iter_data)
            if status != types.status.SUCCESS:
                result = status
        return result

    def __run_common_debugs(self, iter_data):
        result = types.status.SUCCESS
        for s in self.__debugs:
            status = s.Main(iter_data)
            if status != types.status.SUCCESS:
                result = status
        return result

    def MakeTestcaseDirectory(self):
        instance_id = self.__get_instance_id(self.__iterid)
        self.__mk_testcase_directory(instance_id)

    def TriggerBackgroundTasks(self, tasks, iter_data):
        result = types.status.SUCCESS
        for task in tasks:
            bt = self.__background_tasks[task_name]
            assert(bt)

            Logger.debug("Triggering BackgroundTask %s - manual trigger" % task_name)
            bt_trigger_result = bt.StartTask(self.__tc, iter_data)
            if bt_trigger_result != types.status.SUCCESS:
                result = bt_trigger_result
        return result

    def StopBackgroundTasks(self, tasks):
        result = types.status.SUCCESS
        for task in tasks:
            bt = self.__background_tasks[task_name]
            assert(bt)

            Logger.debug("Triggering BackgroundTask %s - manual trigger" % task_name)
            bt_trigger_result = bt.CollectTask()
            if bt_trigger_result != types.status.SUCCESS:
                result = bt_trigger_result
        return result

    @tcTimeout()
    def __execute(self):
        final_result = types.status.SUCCESS
        for iter_data in self.__iters:
            self.__iterid += 1
            Logger.debug("Create new iter TestcaseData. ID:%d" % self.__iterid)
            iter_data.StartTime()
            api.ChangeDirectory("")
            instance_id = self.__get_instance_id(self.__iterid)
            iter_data.SetInstanceId(instance_id)
            iter_data.SetSelected(self.selected)
            iter_data.SetBundleStore(self.bundle_store)
            Logger.SetTestcase(instance_id)
            Logger.debug("Testcase Iteration directory = %s" % instance_id)
            ret = self.__mk_testcase_directory(instance_id)
            if ret != types.status.SUCCESS:
                iter_data.SetStatus(ret)
                iter_data.StopTime()
                return ret

            api.ChangeDirectory(instance_id)

            result = types.status.SUCCESS
            setup_result = loader.RunCallback(self.__tc, 'Setup', False, iter_data)
            if setup_result != types.status.SUCCESS:
                Logger.error("Setup callback failed, Cannot continue, switching to Teardown")
                loader.RunCallback(self.__tc, 'Teardown', False, iter_data)
                result = setup_result
            else:
                for task_name, bt in self.__background_tasks.items():
                    if bt.IsAutoTriggerEnabled():
                        Logger.debug("Triggering BackgroundTask %s - auto trigger" % task_name)
                        bt_trigger_result = bt.StartTask(self.__tc, iter_data)
                        if bt_trigger_result != types.status.SUCCESS:
                            result = bt_trigger_result
                    else:
                        Logger.debug("Skipping BackgroundTask %s - manual trigger" % task_name)

                trigger_result = self.__run_common_triggers(iter_data)

                trigger_result = loader.RunCallback(self.__tc, 'Trigger', True, iter_data)
                if trigger_result != types.status.SUCCESS:
                    result = trigger_result

                verify_result = loader.RunCallback(self.__tc, 'Verify', True, iter_data)
                if verify_result != types.status.SUCCESS:
                    result = verify_result

                for task_name, bt in self.__background_tasks.items():
                    bt_stop_result = bt.StopTask('verify')

                verify_result = self.__run_common_verifs();
                if verify_result != types.status.SUCCESS:
                    Logger.error("Common verifs failed.")
                    result = verify_result
                self.iterRes.addResults(iter_data.GetInstanceId(), result)
                iter_data.SetStatus(result)

                #If the tests have failed, lets run debug actions.
                if result != types.status.SUCCESS:
                    debug_result = self.__run_common_debugs(iter_data);
                    if debug_result != types.status.SUCCESS:
                        Logger.error("Common debugs failed.")
                        result = debug_result

                for task_name, bt in self.__background_tasks.items():
                    bt_stop_result = bt.StopTask('teardown')
                    bt.CollectTask()

                teardown_result = self.__run_common_teardowns(iter_data)
                teardown_result = loader.RunCallback(self.__tc, 'Teardown', False, iter_data)
                if teardown_result != types.status.SUCCESS:
                    Logger.error("Teardown callback failed.")
                    result = teardown_result

                iter_data.StopTime()

                if self.__aborted:
                    Logger.info("Iteration Instance: %s FINAL RESULT = %d" % (instance_id, types.status.ABORTED))
                    iter_data.SetStatus(types.status.ABORTED)
                    return types.status.FAILURE

                if result != types.status.SUCCESS and GlobalOptions.no_keep_going:
                    Logger.info("Iteration Instance: %s FINAL RESULT = %d" % (instance_id, result))
                    Logger.error("Error: STOPPING ON FIRST FAILURE.")
                    iter_data.SetStatus(result)
                    raise OfflineTestbedException
                    #return types.status.FAILURE

                if result == types.status.CRITICAL and GlobalOptions.stop_on_critical:
                    Logger.info("Iteration Instance: %s FINAL RESULT = %d" % (instance_id, result))
                    Logger.error("Error: STOPPING ON CRITICAL FAILURE.")
                    iter_data.SetStatus(result)
                    return types.status.CRITICAL

                if result == types.status.OFFLINE_TESTBED:
                    Logger.info("Iteration Instance: %s FINAL RESULT = %d" % (instance_id, result))
                    Logger.error("Error: STOPPING ON OFFLINE TESTBED REQUEST.")
                    iter_data.SetStatus(result)
                    raise OfflineTestbedException

                iter_data.SetStatus(result)
                Logger.info("Iteration Instance: %s FINAL RESULT = %d" % (instance_id, result))

            if result != types.status.SUCCESS or GlobalOptions.dryrun:
                if self.__ignored or result == types.status.IGNORED:
                    Logger.info("Test failed, marking as ignored")
                    Logger.info("Iteration Instance: %s FINAL RESULT = %d" % (instance_id, result))
                    iter_data.SetStatus(types.status.IGNORED)
                    final_result = types.status.SUCCESS
                else:
                    final_result = result

        api.ChangeDirectory("")
        return final_result

    def __get_owner(self):
        return get_owner(self.__tc.__file__)


    def PrintResultSummary(self):
        if not self.__enable: return
        for iter_data in self.__iters:
            iters_str = iter_data.iterators.Summary()
            print(types.FORMAT_TESTCASE_SUMMARY %\
                  (self.__parent.Name(), iter_data.Name(), self.__get_owner(),
                   types.status.str(iter_data.GetStatus()).title(),
                   iter_data.TotalTime()))
            if iters_str: print("- Iterators: %s" % iters_str)
            for v in self.__verifs:
                v.PrintResultSummary()
        return

    def Name(self):
        return self.__spec.name

    def Abort(self):
        self.__aborted = True
        return

    def __aggregate_stats(self):
        for iter_data in self.__iters:
            self.__update_stats(iter_data.GetStatus(), iter_data.GetTestCount())
        return

    def GetStats(self):
        self.__aggregate_stats()
        return (self.__stats_pass, self.__stats_fail, self.__stats_ignored, self.__stats_error)

    def Main(self):
        if GlobalOptions.testcases and self.Name() not in GlobalOptions.testcases:
            Logger.info("Skipping Testcase: %s due to cmdline filter." % self.Name())
            self.__enable = False
            return types.status.SUCCESS

        if GlobalOptions.markers_present:
            if self.Name() == GlobalOptions.testcase_begin:
                Logger.debug("Match found for Testcase starting marker %s" % self.Name())
                GlobalOptions.inb_markers = True

        if GlobalOptions.markers_present and not GlobalOptions.inb_markers:
            Logger.info("Skipping Testcase: %s due to cmdline testcase begin/end markers." % self.Name())
            self.__enable = False
            return types.status.SUCCESS

        if self.__enable:
            Logger.SetTestcase(self.Name())
            Logger.info("Starting Testcase: %s" % self.Name())
            Logger.info("Testcase {0} timestamp: {1}".format(self.Name(), time.asctime()))
            self.__timer.Start()
            try:
                self.status = self.__execute()
            except OfflineTestbedException:
                utils.LogException(Logger)
                Logger.error("EXCEPTION: Aborting Testcase Execution. Reason: testbed failure")
                raise
            except:
                utils.LogException(Logger)
                Logger.error("EXCEPTION: Aborting Testcase Execution.")
                self.status = types.status.ERROR
                if self.__ignored:
                    Logger.error("IGNORE: testcase in ignore mode, ignoring exception.")
                    self.status = types.status.SUCCESS
            self.__timer.Stop()
            Logger.info("Testcase %s FINAL RESULT = %d" % (self.Name(), self.status))
        else:
            self.status = types.status.SUCCESS

        if GlobalOptions.markers_present:
            if self.Name() == GlobalOptions.testcase_end:
                Logger.debug("Match found for Testcase ending marker %s" % self.Name())
                GlobalOptions.inb_markers = False

        return self.status
