import types_pb2
max_infra_types = 4
current_infra_types = 0

def PreCreateCb(data, req_spec, resp_spec):
    global current_infra_types
    if req_spec.request[0].tenant_type == types_pb2.TENANT_TYPE_INFRA:
        if current_infra_types == max_infra_types:
            req_spec.request[0].tenant_type = types_pb2.TENANT_TYPE_CUSTOMER
        else:
            current_infra_types += 1
        
def PostCreateCb(data, req_spec, resp_spec):
    data.exp_data.spec = req_spec.request[0]
    data.exp_data.spec.ClearField("meta")

def PostGetCb(data, req_spec, resp_spec):
    data.actual_data.spec = resp_spec.response[0].spec

def PreUpdateCb(data, req_spec, resp_spec):
    PreCreateCb(data, req_spec, resp_spec)
      
def PostUpdateCb(data, req_spec, resp_spec):
    PostCreateCb(data, req_spec, resp_spec)