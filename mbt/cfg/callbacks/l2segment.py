import types_pb2
import config_mgr
import random

import cfg.callbacks.vrf as vrf

max_infra_types = 1
current_infra_types = 0
wire_encap_type = [types_pb2.ENCAP_TYPE_NONE, types_pb2.ENCAP_TYPE_DOT1Q, types_pb2.ENCAP_TYPE_VXLAN]
tunnel_encap_type = [types_pb2.ENCAP_TYPE_NONE, types_pb2.ENCAP_TYPE_VXLAN]

def PreCreateCb(data, req_spec, resp_spec):
    global current_infra_types
    global max_infra_types

    if req_spec.request[0].vrf_key_handle.vrf_id == vrf.infra_vrf_id:
        if current_infra_types == max_infra_types:
            req_spec.request[0].vrf_key_handle.vrf_id = vrf.customer_vrf_id
        else:
            current_infra_types += 1

    req_spec.request[0].wire_encap.encap_type = random.choice(wire_encap_type)
    req_spec.request[0].tunnel_encap.encap_type = random.choice(tunnel_encap_type)

def PostCreateCb(data, req_spec, resp_spec):
    data.exp_data.spec = req_spec.request[0]
    data.exp_data.spec.ClearField("meta")

def PostGetCb(data, req_spec, resp_spec):
    data.actual_data.spec = resp_spec.response[0].spec

def PreUpdateCb(data, req_spec, resp_spec):
    req_spec.request[0].wire_encap.encap_type = random.choice(wire_encap_type)
    req_spec.request[0].tunnel_encap.encap_type = random.choice(tunnel_encap_type)

def PostUpdateCb(data, req_spec, resp_spec):
    PostCreateCb(data, req_spec, resp_spec)
