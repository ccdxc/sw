"""
   Copyright 2019 Pensando Systems Inc. All rights reserved

    @package pds upgrade code generator module
    This module consumes pds upgrade json and generates c++
    code for the pds upgrade state machine.
"""
import argparse
import inspect
import json
import os
import sys
import errno

msg = {
    # message id : message
    1: "invalid input !",
    2: "Null stage transition passed !",
    3: "Null svc config passed !",
    4: "Null stage passed !",
    5: "At least one Null object passed !",
    6: "IOError (Code: {0}) - Failed to write !",
    7: "Successfully generated pds upgrade state machine.",
    8: "IOError (Code: {0}) - Failed to load Json !",
    9: "Successfully loaded json",
    10: "Event sequence({0}) not found!",
    11: "Stage ({0}) not found!",
    12: "Service response({0}) not found!",
    13: "Value not found path: {0}",
    14: "Arguments can't be Null",
    15: "Failed to create destination directory {0} !",
    16: "Failed to generate required intermediate json !",
    17: "Successfully generated required intermediate json."
}


def message(message_id):
    """ Return the message  for the id """
    return msg.get(message_id, "")


class Log(RuntimeError):
    """!@brief  Logging class
        This Class logs/prints messages with appropriate severity.
        if severity is lower than or equal WARNING , it just print/log.
        if severity is above WARNING, it prints the log and exit from the
        script. This also prints function call stack if 'call_stack=True'
        is passed from teh caller
    """

    def __init__(self, function_name, severity, log, call_stack=False):
        """!@brief  initialize the Logging object and print
            @param function_name The name of the function and line number.
            @param severity Severity of the log.
            @param log The message to be printed.
            @param call_stack If function call stack needs to be printed.
        """
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
        """
        Print function call stack  if 'call_stack=True'
        is passed from teh caller
        """
        if self.__call_stack__ is True:
            for tid, frame in sys._current_frames().items():
                # skip this frame
                frame = frame.f_back
                while frame:
                    code = frame.f_code
                    print("File {}, line {}, in {}".format(
                        code.co_filename,
                        frame.f_lineno,
                        code.co_name,
                    ))
                    frame = frame.f_back

    def __print_log__(self):
        """  Print the log  """
        print("{0} : {1}: {2}".format(self.__function_name__,
                                      self.__severity__,
                                      self.__log__))

    def __print_and_exit__(self):
        """  Print the log and exit with a failure from the script  """
        print("{0} : {1}: {2}".format(self.__function_name__,
                                      self.__severity__,
                                      self.__log__))
        self.__print_call_stack__()
        print("Exiting ... !")
        sys.exit(1)


class GenerateStateMachine(object):
    """!@brief Class to generate state machine

        This class generates the code for PDS upgrade state machine.
        This requires a pre validated in put json
    """

    def __init__(self, file_name, out_put_file_name):
        """!@brief  initialize the GenerateStateMachine ojbect
            @param file_name The input upgrade json file.
            @param out_put_file_name The output source code file.
        """
        self.stage_name_to_id = {}
        self.container = {}
        self.file_name = file_name
        self.out_put_file_name = out_put_file_name
        self.svs_rsp_name_to_id = {}
        self.evt_seq_name_to_id = {}
        self.stage_transitions = []
        self.svc_list = []
        self.stage_list = []

        self.__load__json__()
        self.__load_stages__()
        self.__load_svs_rsp_name_to_id__()
        self.__load_evt_seq_name_to_id__()
        self.__load_fmt__()

    def to_strings(self, key):
        """!@brief  Return the string value of a given json key
            @param key An input key of the upgrade json file.
        """
        if key in self.container:
            return str(self.container[key])
        else:
            frame_info = inspect.getframeinfo(inspect.currentframe())
            file = str(frame_info.filename) + str(frame_info.lineno)
            Log(file, "ERROR", message(1), call_stack=True)

    def get_data(self, key):
        """!@brief  Return the value of a given json key
            @param key An input key of the upgrade json file.
        """
        if key in self.container:
            return self.container[key]
        else:
            frame_info = inspect.getframeinfo(inspect.currentframe())
            file = str(frame_info.filename) + ":" + str(frame_info.lineno)
            Log(file, "ERROR", message(1), call_stack=True)

    def make_event_sequence(self):
        """ Create and return global service event sequence """
        event_sequence = self.__get_data_by_key_walk__(
            default="parallel",
            key_path=self.default_event_seq_fmt)

        event_sequence = self.event_sequence.format(
            self.__to_evt_seq_id__(event_sequence)) + os.linesep
        return event_sequence

    def make_entry_stage(self):
        """ Create and return entry stage """
        entry_stage = self.__get_data_by_key_walk__(
            default="compatcheck",
            key_path=self.default_entry_stage_fmt)

        entry_stage = self.entry_stage.format(
            self.__to_stage_id__(entry_stage)) + os.linesep
        return entry_stage

    def make_stage_transition(self):
        """ Create and return global stage transition  """
        objects = self.__generate_stage_transitions__()
        if objects is None:
            frame_info = inspect.getframeinfo(inspect.currentframe())
            file = str(frame_info.filename) + str(frame_info.lineno)
            Log(file, "ERROR", message(2), call_stack=True)
        return self.__construct_data_block__(self.stage_transition, objects,
                                             True)

    def make_svc(self):
        """ Create and return service configuration  """
        objects = self.__generate_svc__()
        if objects is None:
            frame_info = inspect.getframeinfo(inspect.currentframe())
            file = str(frame_info.filename) + str(frame_info.lineno)
            Log(file, "ERROR", message(3), call_stack=True)
        return self.__construct_data_block__(self.svc, objects, True)

    def make_stages(self):
        """ Create and return individual stage object  """
        objects = self.__generate_stages__()
        if objects is None:
            frame_info = inspect.getframeinfo(inspect.currentframe())
            file = str(frame_info.filename) + str(frame_info.lineno)
            Log(file, "ERROR", message(4), call_stack=True)

        return self.__construct_data_block__(self.runtime_stage_cfg,
                                             objects,
                                             list_block=False,
                                             last_block=True,
                                             tab_count=1)

    def build_header_file(self, transition, svc, event_sequence,
                          stages, entry_stage):
        """ Create and return header part of the file including #define,
        #includes
        """
        if all(obj is not None for obj in [transition, svc,
                                           event_sequence, stages,
                                           entry_stage]):

            header_file_data = "{" + os.linesep
            header_file_data = header_file_data + transition + os.linesep
            header_file_data = header_file_data + svc + os.linesep
            header_file_data = header_file_data + "\t" + event_sequence + \
                               os.linesep
            header_file_data = header_file_data + "\t " + entry_stage + \
                               os.linesep
            header_file_data = header_file_data + stages + os.linesep
            header_file_data = header_file_data + "}" + \
                               os.linesep

            return header_file_data
        else:
            frame_info = inspect.getframeinfo(inspect.currentframe())
            file = str(frame_info.filename) + str(frame_info.lineno)
            Log(file, "ERROR", message(5), call_stack=True)

    def dump_header(self, data):
        """ Write 'data' to a give output file  """
        try:
            with open(self.out_put_file_name, 'w') as f:
                f.write(data)
        except IOError as e:
            frame_info = inspect.getframeinfo(inspect.currentframe())
            file = str(frame_info.filename) + str(frame_info.lineno)
            Log(file, "FATAL", message(6).format(type(e).__name__),
                call_stack=True)
        else:
            frame_info = inspect.getframeinfo(inspect.currentframe())
            file = str(frame_info.filename) + str(frame_info.lineno)
            Log(file, "INFO", message(7))

    def __load__json__(self):
        """ Load input json file in key-value pair  """
        try:
            with open(self.file_name, 'r') as f:
                self.container = json.load(f)
        except IOError as e:
            frame_info = inspect.getframeinfo(inspect.currentframe())
            file = str(frame_info.filename) + str(frame_info.lineno)
            Log(file, "FATAL", message(8).format(type(e).__name__),
                call_stack=True)
        else:
            frame_info = inspect.getframeinfo(inspect.currentframe())
            file = str(frame_info.filename) + ":" + str(frame_info.lineno)
            Log(file, "INFO", message(9))

    def __load_fmt__(self):
        """ Initialize constants key format/patters  """
        self.default_event_seq_fmt = "svc.event_sequence"
        self.default_domain_fmt = "svc.domain"
        self.default_discovery_fmt = "svc.discovery"
        self.default_discovery_fmt = "svc.discovery"
        self.default_rsp_timeout_fmt = "svc.rsp_timeout"
        self.default_entry_stage_fmt = "stages.entry_stage"
        self.rsp_keys_to_walk_fmt = "stages.{0}.events.{1}.name"
        self.next_stage_keys_to_walk_fmt = "stages.{0}.events.{1}.next_stage"
        self.rsp_timeout_fmt = "stages.{0}.svc.rsp_timeout"
        self.domain_fmt = "stages.{0}.svc.domain"
        self.discovery_fmt = "stages.{0}.svc.discovery"
        self.svc_seq_fmt = "stages.{0}.svc.event_sequence"
        self.pre_hook_list_fmt = "stages.{0}.pre_hooks"
        self.post_hook_list_fmt = "stages.{0}.post_hooks"
        self.svc_list_fmt = "stages.{0}.svc.names"

        self.runtime_stage_cfg = self.__wrap_quote__("upg_stages")
        self.svc = self.__wrap_quote__("upg_svc")
        self.stage_transition = self.__wrap_quote__("upg_stage_transitions")
        self.event_sequence = self.__wrap_quote__("event_sequence")
        self.event_sequence += " : {0},"
        self.entry_stage = self.__wrap_quote__("entry_stage")
        self.entry_stage += " : {0},"
        self.rsp_timeout = self.__wrap_quote__("rsp_timeout")
        self.rsp_timeout += " : {0},"
        self.svc_sequence = self.__wrap_quote__("svc_sequence")
        self.svc_sequence += " : {0},"
        self.event_sequence = self.__wrap_quote__("event_sequence")
        self.event_sequence += " : {0},"
        self.pre_hook = self.__wrap_quote__("pre_hook")
        self.pre_hook += " : {0},"
        self.post_hook = self.__wrap_quote__("post_hook")
        self.post_hook += " : {0},"
        self.domain = self.__wrap_quote__("domain")
        self.domain += " : {0},"
        self.discovery = self.__wrap_quote__("discovery")
        self.discovery += " : {0},"

    def __load_stages__(self):
        """ Initialize constants key-value pair of stages and enum """
        self.stage_name_to_id["compatcheck"] = \
            self.__wrap_quote__("compatcheck")
        self.stage_name_to_id["start"] = self.__wrap_quote__("start")
        self.stage_name_to_id["prepare"] = self.__wrap_quote__("prepare")
        self.stage_name_to_id["backup"] = self.__wrap_quote__("backup")
        self.stage_name_to_id["repeal"] = self.__wrap_quote__("repeal")
        self.stage_name_to_id["rollback"] = self.__wrap_quote__("rollback")
        self.stage_name_to_id["sync"] = self.__wrap_quote__("sync")
        self.stage_name_to_id["prepare_switchover"] = \
            self.__wrap_quote__("prepare_switchover")
        self.stage_name_to_id["switchover"] = self.__wrap_quote__("switchover")
        self.stage_name_to_id["ready"] = self.__wrap_quote__("ready")
        self.stage_name_to_id["respawn"] = self.__wrap_quote__("respawn")
        self.stage_name_to_id["finish"] = self.__wrap_quote__("finish")
        self.stage_name_to_id["exit"] = self.__wrap_quote__("exit")

    def __load_svs_rsp_name_to_id__(self):
        """
        Initialize constants key-value pair of service response
        and enum constants
        """
        self.svs_rsp_name_to_id["svc_rsp_ok"] = self.__wrap_quote__("ok")
        self.svs_rsp_name_to_id["svc_rsp_fail"] = self.__wrap_quote__("fail")
        self.svs_rsp_name_to_id["svc_rsp_crit"] = \
            self.__wrap_quote__("critical")
        self.svs_rsp_name_to_id["svc_no_rsp"] = \
            self.__wrap_quote__("no_response")

    def __load_evt_seq_name_to_id__(self):
        """ Initialize constants key-value pair of event sequence and enum """
        self.evt_seq_name_to_id["parallel"] = self.__wrap_quote__("parallel")
        self.evt_seq_name_to_id["serial"] = self.__wrap_quote__("serial")

    def __to_evt_seq_id__(self, key):
        """ Return enum of event sequence  """
        if key not in self.evt_seq_name_to_id.keys():
            frame_info = inspect.getframeinfo(inspect.currentframe())
            file = str(frame_info.filename) + str(frame_info.lineno)
            Log(file, "ERROR", message(10).format(key), call_stack=True)
        else:
            return self.evt_seq_name_to_id[key]

    def __to_stage_id__(self, key):
        """ Return enum of state machine stage  """
        if key not in self.stage_name_to_id.keys():
            frame_info = inspect.getframeinfo(inspect.currentframe())
            file = str(frame_info.filename) + str(frame_info.lineno)
            Log(file, "ERROR", message(11).format(key), call_stack=True)
        else:
            return self.stage_name_to_id[key]

    def __to_svs_rsp_name_to_id__(self, key):
        """ Return enum of service response """
        if key not in self.svs_rsp_name_to_id.keys():
            frame_info = inspect.getframeinfo(inspect.currentframe())
            file = str(frame_info.filename) + str(frame_info.lineno)
            Log(file, "ERROR", message(12).format(key), call_stack=True)
        else:
            return self.svs_rsp_name_to_id[key]

    def __get_stages__(self):
        stages = list(self.__get_data_by_key_walk__("stages").keys())
        stages.remove("entry_stage")
        stages.remove("exit")
        return stages

    def __get_svc_rsp__(self):
        return self.svs_rsp_name_to_id.keys()

    def __get_data_by_key_walk__(self, key_path=None,
                                 default=None, check_values=False,
                                 ignore_case=False, path_delimiter="."):
        """ Return value of given key in teh format of a '.' separated
        path for example x.y.z.Key """
        val = None
        current_data = self.container
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
                Log(file, "ERROR", message(13).format(key_path),
                    call_stack=True)
        return val

    def __generate_stage_transitions__(self):
        """ Return stage transition objects """
        rsp_code_counts = len(self.__get_svc_rsp__())
        for current_stage in self.__get_stages__():
            for idx in range(rsp_code_counts):
                rsp_keys_to_walk = self.rsp_keys_to_walk_fmt. \
                    format(current_stage, idx)
                next_stage_keys_to_walk = self.next_stage_keys_to_walk_fmt. \
                    format(current_stage, idx)

                stage_from = current_stage
                svc_rsp_code = self.__get_data_by_key_walk__(rsp_keys_to_walk)
                stage_to = self.__get_data_by_key_walk__(
                    next_stage_keys_to_walk)

                transition_obj = self.__get_stage_transaction_obj__(
                    stage_curr=stage_from,
                    svc_rsp=svc_rsp_code,
                    stage_next=stage_to)

                obj = "{0}{1}".format(str(transition_obj), ",\n")
                self.stage_transitions.append(obj)

        return "".join(self.stage_transitions)[:-2]

    def __generate_svc__(self):
        """ Return svc objects  """
        svc_keys_to_walk = "svc.names"
        svc_counts = self.__get_data_by_key_walk__(svc_keys_to_walk)
        for svc in range(len(svc_counts)):
            svc_name = self.__get_data_by_key_walk__("{0}.{1}".
                                                     format(svc_keys_to_walk,
                                                            svc))
            svc_name = self.__wrap_quote__(svc_name)

            obj = "{0}{1}{2}".format('\t' * 2, svc_name, ',\n')
            self.svc_list.append(obj)

        return "".join(self.svc_list)[:-2]

    def __generate_stages__(self):
        """ Return stage objects  """
        rsp_timeout_format = self.rsp_timeout_fmt
        svc_seq_format = self.svc_seq_fmt
        pre_hook_list_format = self.pre_hook_list_fmt
        post_hook_list_format = self.post_hook_list_fmt
        svc_list_format = self.svc_list_fmt
        domain_format = self.domain_fmt
        discovery_format = self.discovery_fmt

        default_timeout = self.__get_data_by_key_walk__(
            default="",
            key_path=self.default_rsp_timeout_fmt)

        default_event_sequence = self.__get_data_by_key_walk__(
            default="",
            key_path=self.default_event_seq_fmt)

        default_domain = self.__get_data_by_key_walk__(
            default="A",
            key_path=self.default_domain_fmt)

        default_discovery = self.__get_data_by_key_walk__(
            default="no",
            key_path=self.default_discovery_fmt)

        for current_stage in self.__get_stages__():
            timeout_keys_to_walk = rsp_timeout_format.format(current_stage)
            svc_seq_keys_to_walk = svc_seq_format.format(current_stage)
            svc_list_keys_to_walk = svc_list_format.format(current_stage)
            pre_hooks_keys_to_walk = pre_hook_list_format.format(current_stage)
            post_hooks_keys_to_walk = post_hook_list_format.format(
                current_stage)
            domain_keys_to_walk = domain_format.format(current_stage)
            discovery_keys_to_walk = discovery_format.format(current_stage)

            rsp_timeout = self.__get_data_by_key_walk__(
                default=default_timeout,
                key_path=timeout_keys_to_walk)

            event_sequence = self.__get_data_by_key_walk__(
                default=default_event_sequence,
                key_path=svc_seq_keys_to_walk)

            svc_list = self.__get_data_by_key_walk__(
                default="",
                key_path=svc_list_keys_to_walk)

            post_hooks = self.__get_data_by_key_walk__(
                default="",
                key_path=post_hooks_keys_to_walk)

            pre_hooks = self.__get_data_by_key_walk__(
                default="",
                key_path=pre_hooks_keys_to_walk)

            domain = self.__get_data_by_key_walk__(
                default=default_domain,
                key_path=domain_keys_to_walk)

            discovery = self.__get_data_by_key_walk__(
                default=default_discovery,
                key_path=discovery_keys_to_walk)

            current_stage = self.__wrap_quote__(current_stage)

            rsp_timeout = self.__wrap_quote__(rsp_timeout)
            rsp_timeout = self.rsp_timeout.format(rsp_timeout)

            event_sequence = self.__wrap_quote__(event_sequence)
            event_sequence = self.event_sequence.format(event_sequence)

            svc_sequence = self.__wrap_quote__(":".join(svc_list))
            svc_sequence = self.svc_sequence.format(svc_sequence)

            pre_hooks = self.__wrap_quote__(":".join(pre_hooks))
            pre_hooks = self.pre_hook.format(pre_hooks)

            post_hooks = self.__wrap_quote__(":".join(post_hooks))
            post_hooks = self.pre_hook.format(post_hooks)

            domain = self.__wrap_quote__(domain)
            domain = self.domain.format(domain)

            discovery = self.__wrap_quote__(discovery)
            discovery = self.discovery.format(discovery)
            discovery = discovery[:-1]
            delimiter = "\n\t\t\t"
            stage = delimiter.join([delimiter[1:] + rsp_timeout,
                                    svc_sequence, event_sequence,
                                    pre_hooks, post_hooks, domain,
                                    discovery])
            stage = self.__construct_data_block__(current_stage,
                                                  stage,
                                                  list_block=False,
                                                  last_block=False,
                                                  no_newline=False,
                                                  tab_count=2)

            self.stage_list.append(stage)

        stages = "\t" + "\n\t".join(self.stage_list)[:-1]
        return stages

    def __get_stage_transaction_obj__(self, stage_curr=None, svc_rsp=None,
                                      stage_next=None):
        """ Return stage transition objects  """
        if stage_curr is None or svc_rsp is None or stage_next is None:
            frame_info = inspect.getframeinfo(inspect.currentframe())
            file = str(frame_info.filename) + str(frame_info.lineno)
            Log(file, "ERROR", message(14), call_stack=True)

        stage_from = self.__to_stage_id__(stage_curr)
        stage_to = self.__to_stage_id__(stage_next)
        rsp = self.__to_svs_rsp_name_to_id__(svc_rsp)
        obj = "\t\t[ {0}, {1}, {2} ]".format(stage_from, rsp, stage_to)
        return obj

    @staticmethod
    def __construct_data_block__(block, objects, list_block=False,
                                 last_block=False, no_newline=False,
                                 tab_count=1):
        block = "\t" + block

        new_line = "" if no_newline else os.linesep
        tab = "\t" * tab_count

        if list_block:
            block = block + " : [" + new_line
            block = block + objects + new_line
            block = block + tab + "],"
        else:
            block = block + " : {" + new_line
            block = block + objects + new_line
            block = block + tab + "},"

        if last_block:
            block = block[:-1] + new_line

        return block

    @staticmethod
    def __wrap_quote__(data):
        date = "{0}{1}{2}".format('"', data, '"')
        return date


def crete_dest_dir(output_json):
    try:
        if not os.path.exists(output_json):
            out_dir = os.path.dirname(output_json)
            os.makedirs(out_dir)
    except OSError as e:
        if e.errno != errno.EEXIST:
            frame = inspect.getframeinfo(inspect.currentframe())
            filename = str(frame.filename) + str(frame.lineno)
            Log(filename, "ERROR", message(15).format(output_json),
                call_stack=True)
        pass
    except:
        frame = inspect.getframeinfo(inspect.currentframe())
        filename = str(frame.filename) + str(frame.lineno)
        Log(filename, "ERROR", message(15).format(output_json), call_stack=True)


def main(input_json, output_json):
    try:
        crete_dest_dir(output_json)
        obj = GenerateStateMachine(input_json, output_json)
        transitions = obj.make_stage_transition()
        services = obj.make_svc()
        entry_stage = obj.make_entry_stage()
        event_sequence = obj.make_event_sequence()
        stages = obj.make_stages()
        hpp = obj.build_header_file(transitions, services, event_sequence,
                                    stages, entry_stage)
        #print(hpp)
        obj.dump_header(hpp)
    except:
        frame_info = inspect.getframeinfo(inspect.currentframe())
        file = str(frame_info.filename) + str(frame_info.lineno)
        Log(file, "ERROR", message(16), call_stack=True)

    else:
        frame = inspect.getframeinfo(inspect.currentframe())
        filename = str(frame.filename) + str(frame.lineno)
        Log(filename, "INFO", message(17), call_stack=True)
        sys.exit(0)


"""
    Parse Arguments and start the code generation

    usage: fsm_code_gen.py [-h] -i [INPUT_FILE_NAME] -o [OUTPUT_FILE_NAME]

    Generate state machine for PDS upgrade

    optional arguments:
      -h, --help            show this help message and exit
      -i [INPUT_FILE_NAME]  PDS upgrade Json (example: 'graceful.json')
      -o [OUTPUT_FILE_NAME] PDS fsm Json (example: 'graceful_upgrade.json')
"""

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Generate state machine '
                                                 'for PDS upgrade')
    parser.add_argument('-i',
                        nargs='?',
                        dest='input_file_name',
                        required=True,
                        type=str,
                        help='PDS upgrade Json '
                             '(example: \'upgrade.json\')')
    parser.add_argument('-o',
                        nargs='?',
                        dest='output_file_name',
                        type=str,
                        required=True,
                        help='PDS fsm header file '
                             '(example: \'graceful_upgrade.json\')')

    args = parser.parse_args()
    main(args.input_file_name, args.output_file_name)
    sys.exit(0)

