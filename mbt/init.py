#! /usr/bin/python3

import types_pb2
import utils
import cfg.callbacks.vrf as vrf

if (utils.mbt_v2()):
    from mbt_v2.msg import GrpcReqRspMsg
else:
    from grpc_meta.msg import GrpcReqRspMsg

def infra_vrf_init(api, req_msg_type):

    expected_status = 'API_STATUS_OK'

    if api == None or req_msg_type == None:
        return

    req_msg = req_msg_type()

    vrf_spec = req_msg.request.add()

    vrf_spec.key_or_handle.vrf_id = 1
    #vrf_spec.security_key_handle.profile_id = 1
    vrf_spec.vrf_type = types_pb2.VRF_TYPE_INFRA

    vrf.infra_vrf_id = 1
    vrf.current_infra_types = 1

    rsp_msg = api(req_msg)

    api_status = GrpcReqRspMsg.GetApiStatusObject(rsp_msg)

    if expected_status != api_status:
        print ("Infra VRF create failed")
        print ("Expected: " + expected_status + ", Got: " + api_status)
        assert False

def infra_l2seg_init(api, req_msg_type):

    expected_status = 'API_STATUS_OK'

    if api == None or req_msg_type == None:
        return

    req_msg = req_msg_type()

    l2seg_spec = req_msg.request.add()

    l2seg_spec.key_or_handle.segment_id = 1
    l2seg_spec.vrf_key_handle.vrf_id = vrf.infra_vrf_id
    l2seg_spec.wire_encap.encap_type = types_pb2.ENCAP_TYPE_DOT1Q
    l2seg_spec.wire_encap.encap_value = 100

    rsp_msg = api(req_msg)

    api_status = GrpcReqRspMsg.GetApiStatusObject(rsp_msg)

    if expected_status != api_status:
        print ("Infra L2seg create failed")
        print ("Expected: " + expected_status + ", Got: " + api_status)
        assert False
