import types_pb2
import config_mgr

max_infra_types = 1
current_infra_types = 0

def PreCreateCb(data, req_spec, resp_spec):
    global current_infra_types

def PostCreateCb(data, req_spec, resp_spec):
    data.exp_data.spec = req_spec.request[0]
    data.exp_data.spec.ClearField("meta")

def PostGetCb(data, req_spec, resp_spec):
    data.actual_data.spec = resp_spec.response[0].spec
    
def PostUpdateCb(data, req_spec, resp_spec):
    PostCreateCb(data, req_spec, resp_spec)
