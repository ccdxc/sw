import interface_pb2

def PreCreateCb(data, req_spec, resp_spec):
    req_spec.request[0].type = interface_pb2.IF_TYPE_ENIC
    req_spec.request[0].if_enic_info.enic_type = interface_pb2.IF_ENIC_TYPE_CLASSIC
    req_spec.request[0].if_enic_info.classic_enic_info.native_l2segment_handle = 0
    req_spec.request[0].if_enic_info.pinned_uplink_if_handle = 0

def PostCreateCb(data, req_spec, resp_spec):
    data.exp_data.spec = req_spec.request[0]
    data.exp_data.spec.ClearField("meta")

def PostGetCb(data, req_spec, resp_spec):
    data.actual_data.spec = resp_spec.response[0].spec

def PreUpdateCb(data, req_spec, resp_spec):
    PreCreateCb(data, req_spec, resp_spec)

def PostUpdateCb(data, req_spec, resp_spec):
    PostCreateCb(data, req_spec, resp_spec)
