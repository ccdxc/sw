import types_pb2
max_infra_types = 1
current_infra_types = 0
infra_vrf_id = 0
customer_vrf_id = 0

def PreCreateCb(data, req_spec, resp_spec):
    global current_infra_types
    global max_infra_types
    global infra_vrf_id
    global customer_vrf_id

    if req_spec.request[0].vrf_type == types_pb2.VRF_TYPE_INFRA:
        if customer_vrf_id == 0 or current_infra_types == max_infra_types:
            req_spec.request[0].vrf_type = types_pb2.VRF_TYPE_CUSTOMER
        else:
            current_infra_types += 1
            infra_vrf_id = req_spec.request[0].key_or_handle.vrf_id

    elif req_spec.request[0].vrf_type == types_pb2.VRF_TYPE_NONE:
        req_spec.request[0].vrf_type = types_pb2.VRF_TYPE_CUSTOMER

    if req_spec.request[0].vrf_type == types_pb2.VRF_TYPE_CUSTOMER:
        customer_vrf_id = req_spec.request[0].key_or_handle.vrf_id

def PostCreateCb(data, req_spec, resp_spec):
    data.exp_data.spec = req_spec.request[0]
    data.exp_data.spec.ClearField("meta")

def PostGetCb(data, req_spec, resp_spec):
    data.actual_data.spec = resp_spec.response[0].spec

def PreUpdateCb(data, req_spec, resp_spec):
    PreCreateCb(data, req_spec, resp_spec)

def PostUpdateCb(data, req_spec, resp_spec):
    PostCreateCb(data, req_spec, resp_spec)
