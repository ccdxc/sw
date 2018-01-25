import types_pb2
import config_mgr

max_infra_types = 1
current_infra_types = 0

def PreCreateCb(data, req_spec, resp_spec):
    global current_infra_types
    if req_spec.request[0].segment_type == types_pb2.L2_SEGMENT_TYPE_INFRA:
        if current_infra_types == max_infra_types:
            req_spec.request[0].segment_type = types_pb2.L2_SEGMENT_TYPE_TENANT
        else:
            vrf_message = config_mgr.GetConfigMessageFromKey(req_spec.request[0].vrf_key_handle)
            if vrf_message.request[0].vrf_type != types_pb2.VRF_TYPE_INFRA:
                req_spec.request[0].segment_type = types_pb2.L2_SEGMENT_PRIMARY
            else:
                current_infra_types += 1

def PostCreateCb(data, req_spec, resp_spec):
    data.exp_data.spec = req_spec.request[0]
    data.exp_data.spec.ClearField("meta")

def PostGetCb(data, req_spec, resp_spec):
    data.actual_data.spec = resp_spec.response[0].spec
    
def PostUpdateCb(data, req_spec, resp_spec):
    PostCreateCb(data, req_spec, resp_spec)
