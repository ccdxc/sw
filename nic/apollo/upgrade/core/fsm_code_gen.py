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
    14: "Arguments can't be Null"
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
        self.idl_stage_list = []

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

    def make_idl_event_sequence(self):
        """ Create and return global service event sequence """
        event_sequence = self.__get_data_by_key_walk__(
            default="parallel",
            key_path=self.event_seq_fmt)

        event_sequence = self.event_sequence.format(
            self.__to_evt_seq_id__(event_sequence)) + os.linesep
        return event_sequence

    def make_idl_stage_transition(self):
        """ Create and return global stage transition  """
        objects = self.__generate_stage_transitions__()
        if objects is None:
            frame_info = inspect.getframeinfo(inspect.currentframe())
            file = str(frame_info.filename) + str(frame_info.lineno)
            Log(file, "ERROR", message(2), call_stack=True)
        return self.__construct_data_block__(self.stage_transition, objects)

    def make_idl_svc_cfg(self):
        """ Create and return service configuration  """
        objects = self.__generate_svc__()
        if objects is None:
            frame_info = inspect.getframeinfo(inspect.currentframe())
            file = str(frame_info.filename) + str(frame_info.lineno)
            Log(file, "ERROR", message(3), call_stack=True)
        return self.__construct_data_block__(self.idl_svc_cfg, objects)

    def make_idl_stage_cfg(self):
        """ Create and return individual stage object  """
        objects = self.__generate_stages__()
        if objects is None:
            frame_info = inspect.getframeinfo(inspect.currentframe())
            file = str(frame_info.filename) + str(frame_info.lineno)
            Log(file, "ERROR", message(4), call_stack=True)
        return self.__construct_data_block__(self.idl_stage_cfg, objects)

    def build_header_file(self, idl_transition, idl_svc, idl_event_sequence,
                          idl_stages):
        """ Create and return header part of the file including #define,
        #includes
        """
        if all(obj is not None for obj in [idl_transition, idl_svc,
                                           idl_event_sequence, idl_stages]):

            header_file_data = self.__make_header__() + os.linesep
            header_file_data = header_file_data + idl_transition + os.linesep
            header_file_data = header_file_data + idl_svc + os.linesep
            header_file_data = header_file_data + idl_event_sequence + \
                               os.linesep
            header_file_data = header_file_data + idl_stages + os.linesep
            header_file_data = header_file_data + self.__make_footer__() + \
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
        self.event_seq_fmt = "svc.event_sequence"
        self.rsp_keys_to_walk_fmt = "stages.{0}.events.{1}.name"
        self.next_stage_keys_to_walk_fmt = "stages.{0}.events.{1}.next_stage"
        self.rsp_timeout_format = "stages.{0}.rsp_timeout"
        self.svc_seq_format = "stages.{0}.svc.event_sequence"
        self.pre_hook_list_format = "stages.{0}.pre_hooks"
        self.post_hook_list_format = "stages.{0}.post_hooks"
        self.svc_list_format = "stages.{0}.svc.names"
        self.idl_stage_format = "idl_stage_t({0}){1}"
        self.idl_stage_cfg = "idl_stage_t idl_stages_cfg[]"
        self.idl_svc_cfg = "svc_t svc[]"
        self.stage_transition = "stage_transition_t stage_transitions[]"
        self.event_sequence = "event_sequence_t event_sequence = {0};"

    def __load_stages__(self):
        """ Initialize constants key-value pair of stages and enum """
        self.stage_name_to_id["validation"] = "STAGE_ID_VALIDATION"
        self.stage_name_to_id["start"] = "STAGE_ID_START"
        self.stage_name_to_id["prepare"] = "STAGE_ID_PREPARE"
        self.stage_name_to_id["backup"] = "STAGE_ID_BACKUP"
        self.stage_name_to_id["upgrade"] = "STAGE_ID_UPGRADE"
        self.stage_name_to_id["verify"] = "STAGE_ID_VERIFY"
        self.stage_name_to_id["finish"] = "STAGE_ID_FINISH"
        self.stage_name_to_id["abort"] = "STAGE_ID_ABORT"
        self.stage_name_to_id["rollback"] = "STAGE_ID_ROLLBACK"
        self.stage_name_to_id["critical"] = "STAGE_ID_CRITICAL"
        self.stage_name_to_id["exit"] = "STAGE_ID_EXIT"

    def __load_svs_rsp_name_to_id__(self):
        """
         Initialize constants key-value pair of service response
         and enum constants
         """
        self.svs_rsp_name_to_id["svc_rsp_ok"] = "SVC_RSP_OK"
        self.svs_rsp_name_to_id["svc_rsp_fail"] = "SVC_RSP_FAIL"
        self.svs_rsp_name_to_id["svc_rsp_crit"] = "SVC_RSP_CRIT"
        self.svs_rsp_name_to_id["svc_no_rsp"] = "SVC_RSP_NONE"

    def __load_evt_seq_name_to_id__(self):
        """ Initialize constants key-value pair of event sequence and enum """
        self.evt_seq_name_to_id["parallel"] = "PARALLEL"
        self.evt_seq_name_to_id["serial"] = "SERIAL"

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
        return self.stage_name_to_id.keys()

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
            obj = "{0}{1}{2}{3}{4}".format('\tsvc_t(', '"', svc_name, '")',
                                           ',\n')
            self.svc_list.append(obj)

        return "".join(self.svc_list)[:-2]

    def __generate_stages__(self):
        """ Return IDL stage objects  """
        rsp_timeout_format = self.rsp_timeout_format
        svc_seq_format = self.svc_seq_format
        pre_hook_list_format = self.pre_hook_list_format
        post_hook_list_format = self.post_hook_list_format
        svc_list_format = self.svc_list_format
        idl_stage_format = self.idl_stage_format

        for current_stage in self.__get_stages__():
            timeout_keys_to_walk = rsp_timeout_format.format(current_stage)
            svc_seq_keys_to_walk = svc_seq_format.format(current_stage)
            svc_list_keys_to_walk = svc_list_format.format(current_stage)
            pre_hooks_keys_to_walk = pre_hook_list_format.format(current_stage)
            post_hooks_keys_to_walk = post_hook_list_format.format(
                current_stage)

            rsp_timeout = self.__get_data_by_key_walk__(
                default="",
                key_path=timeout_keys_to_walk)

            event_sequence = self.__get_data_by_key_walk__(
                default="",
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

            current_stage = "{0}{1}{2}".format('"', current_stage, '" ')
            rsp_timeout = "{0}{1}{2}".format('"', rsp_timeout, '" ')
            event_sequence = "{0}{1}{2}".format('"', event_sequence, '" ')
            service_order = "{0}{1}{2}".format('"', ":".join(svc_list), '" ')
            pre_hooks = "{0}{1}{2}".format('"', ":".join(pre_hooks), '" ')
            post_hooks = "{0}{1}{2}".format('"', ":".join(post_hooks), '" ')

            idl_stage_obj = ",\n\t\t\t\t".join([current_stage, rsp_timeout,
                                                service_order, event_sequence,
                                                pre_hooks, post_hooks])

            idl_stage_obj = idl_stage_format.format(idl_stage_obj, ",\n\n\t")
            self.idl_stage_list.append(idl_stage_obj)

        return "".join(self.idl_stage_list)[:-2]

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
        obj = "stage_transition_t( {0}, {1}, {2} )".format(stage_from, rsp,
                                                           stage_to)
        return obj

    @staticmethod
    def __make_header__():
        header_name = "__FSM_AUTO_GEN__"
        include_path1 = '"nic/apollo/upgrade/core/stage.hpp"'
        include_path2 = '"nic/apollo/upgrade/core/service.hpp"'
        include_path3 = '"nic/apollo/upgrade/core/idl.hpp"'
        upg_namespace = "namespace upg {"

        header = "#ifndef " + header_name + os.linesep
        header = header + "#define " + header_name + 2 * os.linesep
        header = header + "#include " + include_path1 + os.linesep
        header = header + "#include " + include_path2 + os.linesep
        header = header + "#include " + include_path3 + 2 * os.linesep
        header = header + upg_namespace + os.linesep
        return header

    @staticmethod
    def __make_footer__():
        footer = "}"
        footer = footer + os.linesep
        footer = footer + "#endif" + os.linesep
        return footer

    @staticmethod
    def __construct_data_block__(block, objects):
        block = block + " = {" + os.linesep
        block = block + objects + os.linesep
        block = block + "};" + os.linesep
        return block


def main(input_json, output_pds_fsm):
    obj = GenerateStateMachine(input_json, output_pds_fsm)
    transitions = obj.make_idl_stage_transition()
    services = obj.make_idl_svc_cfg()
    event_sequence = obj.make_idl_event_sequence()
    stages = obj.make_idl_stage_cfg()
    hpp = obj.build_header_file(transitions, services, event_sequence, stages)
    #print(hpp)
    obj.dump_header(hpp)
    sys.exit(0)


"""
    Parse Arguments and start the code generation

    usage: fsm_code_gen.py [-h] -i [INPUT_FILE_NAME] -o [OUTPUT_FILE_NAME]

    Generate state machine for PDS upgrade

    optional arguments:
      -h, --help            show this help message and exit
      -i [INPUT_FILE_NAME]  PDS upgrade Json (example: 'upgrade.json')
      -o [OUTPUT_FILE_NAME] PDS fsm header file (example: 'upg_fsm.hpp')
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
                             '(example: \'upg_fsm.hpp\')')

    args = parser.parse_args()
    main(args.input_file_name, args.output_file_name)
    sys.exit(0)


