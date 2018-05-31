#! /usr/bin/python3

import types_pb2
import utils

if (utils.mbt_v2()):
    from mbt_v2.msg import GrpcReqRspMsg
else:
    from grpc_meta.msg import GrpcReqRspMsg

def vrf_init(api, req_msg_type):
    # TODO
    return

    expected_status = 'API_STATUS_OK'

    if api == None or req_msg_type == None:
        return

    req_msg = req_msg_type()

    vrf_spec = req_msg.request.add()

    vrf_spec.meta.vrf_id = 1
    vrf_spec.key_or_handle.vrf_id = 10
    vrf_spec.security_key_handle.profile_id = 1
    vrf_spec.vrf_type = types_pb2.VRF_TYPE_INFRA

    rsp_msg = api(req_msg)

    api_status = GrpcReqRspMsg.GetApiStatusObject(rsp_msg)

    if expected_status != api_status:
        print ("Infra VRF create failed")
        print ("Expected: " + expected_status + ", Got: " + api_status)
        assert False

