import argparse
import datetime
import inspect
import json
import os
import re
import subprocess
import sys
import errno
import time
import atexit

msg = {
    1: "Invalid input !",
    2: "  Executing Command  : '{0}', return code is '{1}', error is '{2}'",
    3: "  Executing Command  : '{0}', return code is '{1}', result is '{2}'",
    4: "  Executing Command  : '{0}'",
    5: "  Arguments are      : '{0}', execution Time is '{1}'",
    7: "  Failed to execute test",
    8: "  {0} Finished Test Execution {0}".format("---"),
    9: "  Starting '{0}' ...",
    10: "  Stopping '{0}' ...",
    11: "  Executing Test Case: {0}",
    12: "  Test Description   : {0}",
    13: "  Execution status   : {0}",
    14: "  NOTE               : Expected '{1}' , Got '{2}'",
    15: " Summary Of Test Run : Total - {0} , PASSED - {1}, FAILED - {2}, "
        "Exit Status - {3}",
    16: "PDS UPGRADE FSM TEST: EXECUTING {0} TEST CASES",
    17: "Invalid key: '{0}'",
    18: "  Log Files          : '{0}'"
}


def message(message_id):
    sys.stdout.flush()
    return msg.get(message_id, "")


class Log(RuntimeError):
    def __init__(self, function_name, severity, log, call_stack=False):
        self.__log__ = log
        self.__function_name__ = function_name
        self.__severity__ = severity
        self.__call_stack__ = call_stack
        self.__msg__ = {}
        self.__sev__ = {
            "INFO": 0,
            "TRACE": 1,
            "DEBUG": 2,
            "WARN": 3,
            "ERROR": 4,
            "FATAL": 5
        }
        if self.__severity__ in self.__sev__:
            if self.__sev__[self.__severity__] < self.__sev__["WARN"]:
                self.__print_log__()
            else:
                self.__print_and_exit__()
        else:
            print("Invalid severity !")
            print("Exiting ... !")
            self.__print_call_stack__()
            sys.exit(1)

    def __print_call_stack__(self):

        if self.__call_stack__ is True:
            for tid, frame in sys._current_frames().items():
                # skip this frame
                frame = frame.f_back
                while frame:
                    code = frame.f_code
                    print("File {0}, line {1}, in {2}".format(
                        code.co_filename,
                        frame.f_lineno,
                        code.co_name,
                    ))
                    frame = frame.f_back

    def __print_log__(self):
        ts = time.time()
        ts = datetime.datetime.fromtimestamp(ts).strftime('%Y-%m-%d %H:%M:%S')
        if self.__function_name__ == "":
            print("{0}:{1}:{2}".format(ts, self.__severity__, self.__log__))
        else:
            print("{0}:{1}:{2}:{3}".format(self.__function_name__, ts,
                                           self.__severity__,
                                           self.__log__))

    def __print_and_exit__(self):
        ts = time.time()
        ts = datetime.datetime.fromtimestamp(ts).strftime('%Y-%m-%d %H:%M:%S')
        if self.__function_name__ == "":
            print("{0}:{1}:{2}".format(ts, self.__severity__, self.__log__))
        else:
            print("{0}:{1}:{2}:{3}".format(self.__function_name__, ts,
                                           self.__severity__,
                                           self.__log__))
        self.__print_call_stack__()
        sys.exit(1)


@atexit.register
def at_exit():
    os.system("pkill fsm_test")
    os.system("pkill pdsupgmgr")
    os.system("pkill pdsupgmgr")
    PDSPKG_TOPDIR = os.getenv("PDSPKG_TOPDIR")
    os.system("rm -f {0}/fsm_test_*.log".format(PDSPKG_TOPDIR))
    os.system("rm -f {0}/upgrade.log".format(PDSPKG_TOPDIR))


def add_timestamp(caller):
    def timed(**kwargs):
        start_time = time.time()
        result = caller(**kwargs)
        end_time = time.time()
       # Log("", "INFO", message(5).format(kwargs, end_time - start_time),
       #    call_stack=False)
        return result

    return timed


@add_timestamp
def execute(**kwargs):
    if "cmd" in kwargs:
        cmd = kwargs["cmd"]
    else:
        Log("", "ERROR", message(1), call_stack=True)

    if "return_check" in kwargs:
        return_check = kwargs["return_check"]
    else:
        return_check = True

    if "is_background" in kwargs:
        is_background = kwargs["is_background"]
    else:
        is_background = False

    if is_background:
        cmd = "stdbuf -i0 -o0 -e0 " + cmd
        cmd += " &"
        os.system(cmd)
        Log("", "INFO", message(4).format(cmd),call_stack=False)

    else:
        cmd += " 2>&1 > /dev/null "
        process = subprocess.Popen(cmd,
                                   shell=True,
                                   stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE)
        (result, error) = process.communicate()
        ret_code = process.wait()
        if return_check:
            if ret_code != 0:
                frame_info = inspect.getframeinfo(inspect.currentframe())
                file = str(frame_info.filename) + str(frame_info.lineno)
                Log(file, "ERROR", message(2).format(cmd, ret_code, error),
                    call_stack=True)
            else:
                Log("", "INFO", message(3).format(cmd, ret_code, result),
                    call_stack=False)
        else:
            Log("", "INFO", message(3).format(
                cmd, ret_code, "Empty" if len(result) is 0 else result),
                call_stack=False)

        return ret_code


class ExecutePdsUpgradeFsmTest(object):

    def __init__(self, test_json):
        self.json_container = {}
        self.test_case_json = test_json
        self.PDSPKG_TOPDIR = ""
        self.BUILD_DIR = ""
        self.PIPELINE = ""
        self.fsm_test_service = ""
        self.pdsupgclient = "pdsupgclient"
        self.pdsupgmgr = "pdsupgmgr"
        self.setup_upgrade_gtests = ""
        self.setup_env_mock = ""
        self.logs_dir = ""
        self.fsm_logs = ""
        self.upgrade_log = ""
        self.pdsagent_log = ""
        self.summary = "*" * 100 + os.linesep
        self.number_of_test = 0
        self.number_of_pass = 0
        self.number_of_fail = 0
        self.exit_status = 0

        self.__set_env__()
        self.__load__test_case_json__()
        self.__init_key_value_fmts__()

    def start(self):
        self.__start_tests_execution__()
        return self.exit_status

    def __set_env__(self):
        self.PDSPKG_TOPDIR = os.getenv("PDSPKG_TOPDIR",
                                       "PDSPKG_TOPDIR does not exist")
        self.BUILD_DIR = os.getenv("BUILD_DIR",
                                   "BUILD_DIR does not exist")
        self.PIPELINE = os.getenv("PIPELINE",
                                   "PIPELINE does not exist")

        self.fsm_test_service = os.path.join(self.PDSPKG_TOPDIR, "sdk", "test",
                                             "upgrade", "fsm",
                                             "start_services.sh")
        self.pdsupgclient = os.path.join(self.PDSPKG_TOPDIR, "build", "x86_64",
                                         self.PIPELINE, "capri", "bin",
                                         self.pdsupgclient)
        self.pdsupgmgr = os.path.join(self.PDSPKG_TOPDIR, "build", "x86_64",
                                         self.PIPELINE, "capri", "bin",
                                         self.pdsupgmgr)
        self.setup_upgrade_gtests = os.path.join(self.PDSPKG_TOPDIR,"apollo",
                                                 "test","tools",
                                                 "setup_upgrade_gtests.sh")

        self.setup_env_mock = os.path.join(self.PDSPKG_TOPDIR,"apollo","tools",
                                           "setup_env_mock.sh")
        self.setup_env_mock += self.PIPELINE

        self.fsm_logs = "{0}/fsm_test_*.log".format(self.PDSPKG_TOPDIR)
        self.upgrade_log = "{0}/upgrade.log".format(self.PDSPKG_TOPDIR)
        self.pdsagent_log = "{0}/pdsagent.log".format(self.PDSPKG_TOPDIR)
        self.logs_dir = os.path.join("/tmp", "upgmgr_fsm_test")
        execute(cmd="rm -rf {0}".format(self.logs_dir), return_check=False)
        execute(cmd="mkdir -p {0}".format(self.logs_dir), return_check=False)


    def __get_data_by_key_walk__(self, key_path=None,
                                 default=None, check_values=False,
                                 ignore_case=False, path_delimiter="."):
        """ Return value of given key in the format of a '.' separated
        path for example x.y.z.Key """
        val = None
        current_data = self.json_container
        if key_path is None or key_path == '':
            return current_data

        try:
            for key in key_path.split(path_delimiter):
                if isinstance(current_data, list):
                    val = current_data[int(key)]
                else:
                    if ignore_case:
                        for target_key in current_data.keys():
                            if target_key.lower() == key.lower():
                                key = target_key
                                break
                    val = current_data[key]
                current_data = val
        except (KeyError, ValueError, IndexError, TypeError):
            val = default

        if check_values:
            if val is None or val == default:
                frame_info = inspect.getframeinfo(inspect.currentframe())
                file = str(frame_info.filename) + str(frame_info.lineno)
                Log(file, "ERROR", message(17).format(key_path),
                    call_stack=True)
        return val

    def __load__test_case_json__(self):
        try:
            with open(self.test_case_json, 'r') as f:
                self.json_container = json.load(f)

        except IOError as e:
            frame_info = inspect.getframeinfo(inspect.currentframe())
            file = str(frame_info.filename) + str(frame_info.lineno)
            Log(file, "FATAL", message(8).format(type(e).__name__),
                call_stack=True)
        else:
            Log("", "INFO", message(9).format(" Test Automation suite"))

    def __back_path__(self, test_id):
        path = os.path.join(self.logs_dir, test_id)
        execute(cmd="mkdir -p {0}".format(path), return_check=True)
        return path

    def __backup_log__(self, dest):
        copy="cp {0} {1}".format(self.fsm_logs, dest)
        execute(cmd=copy, return_check=False)
        copy="cp {0} {1}".format(self.upgrade_log, dest)
        execute(cmd=copy, return_check=False)
        copy="cp {0} {1}".format(self.pdsagent_log, dest)
        execute(cmd=copy, return_check=False)

    def __generate_log__(self):
        command = "source {0}".format(self.setup_upgrade_gtests)
        command += " && upg_finish upgmgr"
        execute(cmd=command, return_check=True)

    def __print_log__(self, log_file):
        print ("*"*100)
        print ("Dumping Log File {0}".format(log_file))
        print ("*"*100)
        with open(log_file, "r") as f:
            contents =f.read()
            print (contents)
        print ("*"*100)


    def __setup__(self):
        command = "source {0}".format(self.setup_upgrade_gtests)
        command += " && upg_init"
        execute(cmd=command, return_check=True)
        execute(cmd="rm -f /dev/shm/upgradelog", return_check=False)
        execute(cmd="rm -f {0}".format(self.fsm_logs), return_check=False)
        execute(cmd="rm -f {0}".format(self.upgrade_log), return_check=False)

    def __cleanup__(self):
        execute(cmd="pkill fsm_test", return_check=True)
        execute(cmd="pkill pdsupgmgr", return_check=True)
        execute(cmd="source {0} && upg_finish upgmgr".format(
            self.setup_upgrade_gtests),
            return_check=True)

    def __start_pds_upgrade_client__(self):
        Log("", "INFO", message(9).format(self.pdsupgclient), call_stack=False)
        command = self.pdsupgclient
        ret = execute(cmd=command, return_check=False)
        return ret

    def __start_pds_upgrade__(self):
        Log("", "INFO", message(9).format(self.pdsupgmgr), call_stack=False)
        command = self.pdsupgmgr + " -t {0}sdk/test/upgrade/fsm/scripts/".format(self.PDSPKG_TOPDIR)
        command +=  " 2>&1 > /dev/null"
        ret = execute(cmd=command, return_check=True, is_background=True)
        ret = execute(cmd="sleep 3", return_check=True)
        return ret

    def __start_fsm_test__(self, svc_name, svc_id, err_code=None,
                           fsm_stage=None):
        command = self.fsm_test_service
        command += " " + os.path.join(self.BUILD_DIR, "bin")
        command += " " + svc_name
        command += " " + svc_id
        if err_code is not None:
            command += " " + err_code
            command += " " + fsm_stage
        command +=  " 2>&1 | tee fsm_test_{0}.log > /dev/null".format(svc_name)
        ret = execute(cmd=command, return_check=True, is_background=True)

    def __match_log__(self, log_file, logs_to_match):
        log_dir = os.getenv(self.nic_log_dir, "value does not exist")
        log_file = os.path.join(log_dir, log_file)

        for log_to_match in logs_to_match:
            line_regex = re.compile(log_to_match, re.IGNORECASE)
            with open(log_file, "r") as in_file:
                for line in in_file:
                    match = line_regex.search(line)
                    if not match:
                        return False
        return True

    def __start_test_service__(self, svc, test_id):
        svc_name = svc
        svc_id = self.__get_data_by_key_walk__(
            self.test_svc_ipc_id_fmt.format(test_id, svc))
        err_code = self.__get_data_by_key_walk__(
            self.test_svc_rsp_code_fmt.format(test_id, svc),
            default=None)
        fsm_stage = self.__get_data_by_key_walk__(
            self.test_svc_rsp_stage_fmt.format(test_id, svc),
            default=None)

        if err_code is None:
            fsm_stage = None

        self.__start_fsm_test__(svc_name, svc_id, err_code, fsm_stage)

    def __execute_test__(self, test_id):
        desc = self.__get_data_by_key_walk__(
            self.test_desc_fmt.format(test_id))
        expected_result = self.__get_data_by_key_walk__(
            self.test_exp_api_return_fmt.format(test_id))
        services = list(self.__get_data_by_key_walk__(
            self.test_svc_name_fmt.format(test_id)).keys())

        Log("", "INFO", message(11).format(test_id),
            call_stack=False)
        Log("", "INFO", message(12).format(desc),
            call_stack=False)
        self.summary += message(11).format(test_id) + os.linesep
        self.summary += message(12).format(desc) + os.linesep

        for svc in services:
            self.__start_test_service__(svc, test_id)

        ret = self.__start_pds_upgrade__()
        ret = self.__start_pds_upgrade_client__()
        self.__generate_log__()
        self.summary += message(14).format(test_id, expected_result, ret)
        self.summary += os.linesep

        for svc in services:
            log = "{0}/fsm_test_{1}.log".format(self.PDSPKG_TOPDIR, svc)
            self.__print_log__(log)

        self.__print_log__(self.upgrade_log)

        expected_result = int(expected_result)
        if ret == expected_result:
            self.number_of_pass = self.number_of_pass + 1
            self.summary += message(13).format("SUCCESS") + os.linesep
            Log("", "INFO", message(13).format("SUCCESS"),
                call_stack=False)
        else:
            self.exit_status = 1
            self.number_of_fail = self.number_of_fail + 1
            path = self.__back_path__(test_id)
            self.summary +=  message(18).format(path) + os.linesep
            self.__backup_log__(path)
            self.summary += message(13).format("FAILURE") + os.linesep
            Log("", "INFO", message(13).format("FAILURE"),
                call_stack=False)


        # log_files = list(self.__get_data_by_key_walk__(
        #     self.test_exp_log_file_fmt.format(test_id)))
        #
        # for log_file in log_files:
        #     log = self.__get_data_by_key_walk__(
        #         self.test_exp_log_fmt.format(test_id, log_file))
        #     log_file = log_file + ".log"
        #     print(log)
        #     # ret = self.__match_log__(log_file, log)

    def __start_tests_execution__(self):
        test_cases = list(self.__get_data_by_key_walk__("").keys())
        self.number_of_test = len(test_cases)
        self.summary += message(16).format(self.number_of_test) + os.linesep
        for test_id in test_cases:
            self.summary += "-" * 100 + os.linesep
            self.__setup__()
            self.__execute_test__(test_id)
            self.__cleanup__()
            print ("-" * 100)
            os.system("sleep 5")


        self.summary += "=" * 100 + os.linesep
        self.summary += message(15).format(self.number_of_test,
                                           self.number_of_pass,
                                           self.number_of_fail,
                                           self.exit_status) + os.linesep

        self.summary += "*" * 100 + os.linesep
        self.summary = 10*os.linesep + self.summary

        print(self.summary)

    def __init_key_value_fmts__(self):
        """ Initialize constants key format/patters  """
        self.test_desc_fmt = "{0}.description"
        self.test_svc_name_fmt = "{0}.svc"
        self.test_svc_ipc_id_fmt = "{0}.svc.{1}.ipc_id"
        self.test_svc_rsp_code_fmt = "{0}.svc.{1}.svc_rsp_code"
        self.test_svc_rsp_stage_fmt = "{0}.svc.{1}.svc_rsp_stage"

        self.test_expected_result_fmt = "{0}.expected_result"
        self.test_exp_api_return_fmt = "{0}.expected_result.api_return_code"
        self.test_exp_log_file_fmt = "{0}.expected_result.log"
        self.test_exp_log_fmt = "{0}.expected_result.log.{1}"


def main(test_case_json):
    try:
        obj = ExecutePdsUpgradeFsmTest(test_case_json)
        status = obj.start()
    except:
        frame_info = inspect.getframeinfo(inspect.currentframe())
        file = str(frame_info.filename) + str(frame_info.lineno)
        Log(file, "ERROR", message(7), call_stack=True)
        sys.exit(1)
    else:
        print(message(8))
        sys.exit(status)


"""
    Parse Arguments and start the code generation

    usage: fsm_test_executor.py [-h] -i [INPUT_FILE_NAME]

    PDS upgrade test executor
    optional arguments:
      -h, --help            show this help message and exit
      -i [INPUT_FILE_NAME]  PDS upgrade Json (example: 'graceful.json')
"""

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='PDS upgrade test executor')
    parser.add_argument('-i',
                        nargs='?',
                        dest='input_file_name',
                        required=True,
                        type=str,
                        help='PDS upgrade fsm test Json '
                             '(example: \'upgrade_fsm_test_cases.json\')')

    args = parser.parse_args()
    main(args.input_file_name)
    sys.exit(0)

