def PostCreateCb(data, req_spec, resp_spec):
    data.exp_data.spec = req_spec.request[0]
    data.exp_data.spec.ClearField("meta")

def PostGetCb(data, req_spec, resp_spec):
    data.actual_data.spec = resp_spec.response[0].spec
    
def PostUpdateCb(data, req_spec, resp_spec):
    PostCreateCb(data, req_spec, resp_spec)