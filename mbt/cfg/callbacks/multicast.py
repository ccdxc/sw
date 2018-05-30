import types_pb2
mcMacAddr = 1101088883717
def PreCreateCb(data, req_spec, resp_spec):
    global mcMacAddr
    req_spec.request[0].key_or_handle.key.mac.group = mcMacAddr
    mcMacAddr += 1

def PostCreateCb(data, req_spec, resp_spec):
    data.exp_data.spec = req_spec.request[0]
    data.exp_data.spec.ClearField("meta")

def PostGetCb(data, req_spec, resp_spec):
    data.actual_data.spec = resp_spec.response[0].spec
    
def PreUpdateCb(data, req_spec, resp_spec):
    global mcMacAddr
    req_spec.request[0].key_or_handle.key.mac.group = mcMacAddr
    mcMacAddr += 1
