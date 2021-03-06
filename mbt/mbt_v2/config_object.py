#!/usr/bin/python

import utils
import json
import sys
import os
from enum import Enum
from google.protobuf import json_format
from infra.common.logging import logger
from collections import defaultdict

import utils
from msg import *
import mbt_obj_store

# Types of config methods
class ConfigMethodType(Enum):
    CREATE = 1
    GET    = 2
    UPDATE = 3
    DELETE = 4
    GETALL = 5

class Object(object):
    pass

class ConfigData():
    def __init__(self):
        self.exp_data    = Object()
        self.actual_data = Object()

class ConfigMethod():
    def __init__(self, stub, module, config_method, ignore):
        self._api     = None
        if config_method.api != None:
            self._api = getattr(stub, config_method.api)

        self._request = None
        self._request_method_name = config_method.request
        if config_method.request != None:
            self._request  = getattr(module, config_method.request)

        self._response = None
        self._response_method_name = config_method.response
        if config_method.response != None:
            self._response  = getattr(module, config_method.response)

        self._pre_cb   = config_method.pre_cb
        self._post_cb  = config_method.post_cb

        self._ignore = ignore

    def request(self):
        return self._request

    def request_method_name(self):
        return self._request_method_name

    def pre_cb(self):
        return self._pre_cb

    def post_cb(self):
        return self._post_cb

    def api(self):
        return self._api

    def ignore(self):
        return self._ignore

class ConfigSpecObjectWrapper():
    def __init__(self, config_spec, cfg_spec_obj, hal_channel):

        self._config_spec  = config_spec
        self._service_name = config_spec.Service

        # create client stub object from module
        module           = utils.load_module(config_spec.ProtoObject)
        stub_method_name = self._service_name + "Stub"

        # connect to client stub to GRPC server
        stub = getattr(module, stub_method_name)(hal_channel)

        self._cfg_spec_obj_name = cfg_spec_obj.name

        self._max_objects = mbt_obj_store.default_max_objects()

        if 'max_objects' in dir(cfg_spec_obj):
            self._max_objects = int(cfg_spec_obj.max_objects)

        self._key_or_handle_str = ""
        if 'key_handle' in dir(cfg_spec_obj):
            self._key_or_handle_str = cfg_spec_obj.key_handle

        # config methods which are ignored
        ignore = {}
        ignore[ConfigMethodType.CREATE] = False
        ignore[ConfigMethodType.GET]    = False
        ignore[ConfigMethodType.UPDATE] = False
        ignore[ConfigMethodType.DELETE] = False

        if 'ignore_v2' in dir(cfg_spec_obj):
            ignore_ops = cfg_spec_obj.ignore_v2
        else:
            ignore_ops = cfg_spec_obj.ignore

        for index in ignore_ops or []:
            if index.op == 'Create':
                ignore[ConfigMethodType.CREATE] = True

            if index.op == 'Get':
                ignore[ConfigMethodType.GET] = True

            if index.op == 'Update':
                ignore[ConfigMethodType.UPDATE] = True

            if index.op == 'Delete':
                ignore[ConfigMethodType.DELETE] = True

        # configuration methods for this object
        self._config_methods = {}

        cfg_method_type = ConfigMethodType.CREATE
        self._config_methods[cfg_method_type] = ConfigMethod(stub, module, cfg_spec_obj.create, ignore[cfg_method_type])

        cfg_method_type = ConfigMethodType.GET
        self._config_methods[cfg_method_type] = ConfigMethod(stub, module, cfg_spec_obj.get, ignore[cfg_method_type])

        cfg_method_type = ConfigMethodType.UPDATE
        self._config_methods[cfg_method_type] = ConfigMethod(stub, module, cfg_spec_obj.update, ignore[cfg_method_type])

        cfg_method_type = ConfigMethodType.DELETE
        self._config_methods[cfg_method_type] = ConfigMethod(stub, module, cfg_spec_obj.delete, ignore[cfg_method_type])

        # constraints specified in the config spec object
        self._constraints = []
        if 'constraints' in dir(cfg_spec_obj):
            cfg_spec_obj_constraints = getattr(cfg_spec_obj, 'constraints')
            for cfg_spec_obj_constraint in cfg_spec_obj_constraints:
                constraint = GrpcReqRspMsg.extract_constraints(cfg_spec_obj_constraint.constraint)
                self._constraints.append(constraint)

        # store external reference object for the specific key_handle
        # __DREPRECATED__
        self._ext_ref_obj_list = []

        # cache of mbt_handles of this type
        self._mbt_handle_list = []

        # stats for this object
        self._num_create_ops = 0
        self._num_get_ops    = 0
        self._num_update_ops    = 0
        self._num_delete_ops = 0

    def service_name(self):
        return self._service_name

    def name(self):
        return self._cfg_spec_obj_name

    def max_objects(self):
        return self._max_objects

    def create_ops(self):
        return self._num_create_ops

    def get_config_object_list(self):
        return self._mbt_handle_list

    def get_config_object(self, index):
        if index < len(self._mbt_handle_list):
            return self._mbt_handle_list[index]
        else:
            return None


    # Invoke pre_cb.
    # Invoke API (send the message).
    # Store in global store:
    #     key:   allocated mbt_handle
    #     value: (key_or_handle, ext_refs, immutable_objs) extracted from generated message
    # Store in global key_or_handle store:
    #     key:   key_or_handle
    #     value: mbt_handle
    # Invoke post_cb.
    #
    # Returns: mbt_status, api_status, mbt_handle, rsp_msg
    def invoke_create_api(self, req_msg):

        api_status = 'API_STATUS_ERR'
        mbt_handle = -1
        rsp_msg    = None

        config_method = self._config_methods[ConfigMethodType.CREATE]

        if self.create_ops() >= self.max_objects():
            debug_print("Max objects: " + str(self.max_objects()) + " reached for Service: " + self.service_name() + ", Object: " + self.name())
            return (mbt_obj_store.MbtRetStatus.MBT_RET_MAX_REACHED, api_status, mbt_handle, rsp_msg)

        data = ConfigData()

        if config_method.pre_cb():
            debug_print ("PRE_CB for object: " + config_method.request_method_name())

            ret_val = config_method.pre_cb().call(data, req_msg, None)

            debug_print ("PRE_CB done for object: " + config_method.request_method_name())

            # pre_cb returns False if msg has to be ignored (not to be sent)
            if ret_val is not None and ret_val == False:
                return (mbt_obj_store.MbtRetStatus.MBT_RET_ERR, api_status, mbt_handle, rsp_msg)

        debug_print (req_msg)

        rsp_msg = config_method.api()(req_msg)

        key_or_handle  = GrpcReqRspMsg.GetKeyObject(req_msg)
        ext_refs       = GrpcReqRspMsg.GetExtRefObjects(req_msg)
        immutable_objs = GrpcReqRspMsg.GetImmutableObjects(req_msg)

        # allocate a unique handle
        mbt_handle = utils.alloc_handle()

        # store the req msg in global store
        mbt_obj_store.config_objects_insert(mbt_handle,
                                            (self._service_name,
                                             key_or_handle,
                                             ext_refs,
                                             immutable_objs,
                                             req_msg))

        self._mbt_handle_list.append(mbt_handle)

        # store mbt_handle in global store to be queried by key_handle instance
        key = str(type(key_or_handle)) + str(key_or_handle.SerializeToString())

        mbt_obj_store.config_objects_kh_insert(key, 'create', mbt_handle)

        if config_method.post_cb():
            debug_print ("POST_CB for object: " + config_method.request_method_name())
            config_method.post_cb().call(data, req_msg, rsp_msg)
            debug_print ("POST_CB done for object: " + config_method.request_method_name())

        self._num_create_ops += 1

        if rsp_msg is not None:
            api_status = GrpcReqRspMsg.GetApiStatusObject(rsp_msg)

        return (mbt_obj_store.MbtRetStatus.MBT_RET_OK, api_status, mbt_handle, rsp_msg)


    def create_cb(self,
                  req_msg,
                  cb_args,
                  ext_refs,
                  ext_constraints,
                  immutable_objs,
                  enums_list,
                  field_values):

        ret_list            = cb_args['ret_list']
        expected_api_status = cb_args['expected_api_status']

        (mbt_status, api_status, mbt_handle, rsp_msg) = self.invoke_create_api(req_msg)

        # terminate the walk since max objects is reached
        if mbt_status == mbt_obj_store.MbtRetStatus.MBT_RET_MAX_REACHED:
            return False

        # MBT_RET_ERR indicates error during msg generation and msg was not sent out.
        # Continue the walk since only the current msg is affected
        if mbt_status == mbt_obj_store.MbtRetStatus.MBT_RET_ERR:
            return True

        ret_list.append((mbt_status, api_status, mbt_handle, rsp_msg))

        if api_status != expected_api_status:
            print("Expected: " + expected_api_status + ", Got: " + api_status)
            assert False

        return True


    # Generates a message.
    # Invokes API (sends the message).
    # If any error during generation/send, retries max_retries times
    def create_with_constraints(self, ext_refs, ext_constraints, immutable_objs, max_retries=1):
        mbt_status = mbt_obj_store.MbtRetStatus.MBT_RET_ERR
        api_status = 'API_STATUS_ERR'
        mbt_handle = -1
        rsp_msg    = None

        config_method = self._config_methods[ConfigMethodType.CREATE]

        if config_method.ignore() == True:
            debug_print ("Ignoring CREATE")
            api_status = 'API_STATUS_OK'
            return (mbt_obj_store.MbtRetStatus.MBT_RET_OK, api_status, mbt_handle, rsp_msg)

        if self.create_ops() >= self.max_objects():
            debug_print("Max objects: " + str(self.max_objects()) + " reached for Service: " + self.service_name() + ", Object: " + self.name())
            return (mbt_obj_store.MbtRetStatus.MBT_RET_MAX_REACHED, api_status, mbt_handle, rsp_msg)

        grpc_req_rsp_msg = GrpcReqRspMsg(config_method.request()())

        enums_list   = []
        field_values = {}

        count = 0

        while True:
            debug_print ("Generating request msg for object: " + config_method.request_method_name())

            # generate a msg
            req_msg = grpc_req_rsp_msg.generate_message(None,
                                                        ext_refs,
                                                        ext_constraints,
                                                        immutable_objs,
                                                        enums_list,
                                                        field_values)

            debug_print ("DONE generating request msg for object: " + config_method.request_method_name())

            (mbt_status, api_status, mbt_handle, rsp_msg) = self.invoke_create_api(req_msg)

            # if msg send was successful, then break
            if mbt_status != mbt_obj_store.MbtRetStatus.MBT_RET_ERR:
                break

            count += 1
            if count == max_retries:
                break

        return (mbt_status, api_status, mbt_handle, rsp_msg)


    def create(self, expected_api_status, num_objects):
        api_status = 'API_STATUS_ERR'
        mbt_handle = -1
        rsp_msg    = None

        config_method = self._config_methods[ConfigMethodType.CREATE]

        if config_method.ignore() == True:
            debug_print ("Ignoring CREATE")
            api_status = 'API_STATUS_OK'
            return [(mbt_obj_store.MbtRetStatus.MBT_RET_OK, api_status, mbt_handle, rsp_msg)]

        if self.create_ops() >= self.max_objects():
            debug_print("Max objects: " + str(self.max_objects()) + " reached for Service: " + self.service_name() + ", Object: " + self.name())
            return [(mbt_obj_store.MbtRetStatus.MBT_RET_MAX_REACHED, api_status, mbt_handle, rsp_msg)]

        ext_refs        = {}
        immutable_objs  = {}
        ext_constraints = None

        cb_args                        = {}
        cb_args['ret_list']            = []
        cb_args['expected_api_status'] = expected_api_status

        walk_proto(config_method.request(),
                   self.create_cb,
                   cb_args,
                   num_objects,
                   ext_refs,
                   ext_constraints,
                   immutable_objs)

        return cb_args['ret_list']


    def internal_op(self, mbt_handle, config_method_type, config_method_name):
        api_status = 'API_STATUS_ERR'
        rsp_msg    = None

        config_method = self._config_methods[config_method_type]

        if config_method.ignore() == True:
            debug_print ("Ignoring " + config_method_name)
            api_status = 'API_STATUS_OK'
            return (api_status, rsp_msg)

        if mbt_obj_store.config_objects(mbt_handle) is None:
            print (config_method_name + ": config object could not be found for handle: " + str(mbt_handle))
            return (api_status, rsp_msg)

        grpc_req_rsp_msg = GrpcReqRspMsg(config_method.request()())

        # get object details from global store
        (service_name, key_or_handle, ext_refs, immutable_objs, create_req_msg) = mbt_obj_store.config_objects(mbt_handle)

        ext_constraints = None

        req_msg = grpc_req_rsp_msg.generate_message(key_or_handle,
                                                    ext_refs,
                                                    ext_constraints,
                                                    immutable_objs)

        data = ConfigData()

        if config_method.pre_cb():
            config_method.pre_cb().call(data, req_msg, None)

        debug_print (req_msg)

        rsp_msg = config_method.api()(req_msg)

        if config_method.post_cb():
            config_method.post_cb().call(data, req_msg, rsp_msg)

        if config_method_type == ConfigMethodType.CREATE:
            self._num_create_ops += 1
        elif config_method_type == ConfigMethodType.GET:
            self._num_get_ops += 1
        elif config_method_type == ConfigMethodType.UPDATE:
            self._num_update_ops += 1
        elif config_method_type == ConfigMethodType.DELETE:
            self._num_delete_ops += 1

        if rsp_msg is not None:
            api_status = GrpcReqRspMsg.GetApiStatusObject(rsp_msg)

        return (api_status, rsp_msg)

    def get(self, mbt_handle):
        return self.internal_op(mbt_handle, ConfigMethodType.GET, "GET")

    def update(self, mbt_handle):
        return self.internal_op(mbt_handle, ConfigMethodType.UPDATE, "UPDATE")

    def delete(self, mbt_handle):
        return self.internal_op(mbt_handle, ConfigMethodType.DELETE, "DELETE")

    # __DREPRECATED__
    def ext_ref_obj_add(self, mbt_handle):
        assert False
        self._ext_ref_obj_list.append(mbt_handle)

    # __DREPRECATED__
    def ext_ref_obj_list(self):
        assert False
        return self._ext_ref_obj_list

    def indent_reset(self):
        global indent
        indent = 0
