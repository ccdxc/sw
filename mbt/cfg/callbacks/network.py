import cfg.callbacks.vrf as vrf
import cfg.callbacks.l2segment as l2segment

def PreCreateCb(data, req_spec, resp_spec):
    if req_spec.request[0].vrf_key_handle.vrf_id == vrf.infra_vrf_id:
        if l2segment.current_infra_types == l2segment.max_infra_types:
            req_spec.request[0].vrf_key_handle.vrf_id = vrf.customer_vrf_id
    req_spec.request[0].gateway_ep_handle = 0

def PostCreateCb(data, req_spec, resp_spec):
    data.exp_data.spec = req_spec.request[0]
    data.exp_data.spec.ClearField("meta")

def PostGetCb(data, req_spec, resp_spec):
    data.actual_data.spec = resp_spec.response[0].spec
    
def PostUpdateCb(data, req_spec, resp_spec):
    PostCreateCb(data, req_spec, resp_spec)
