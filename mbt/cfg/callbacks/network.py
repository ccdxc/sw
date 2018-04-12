import cfg.callbacks.vrf as vrf
import cfg.callbacks.l2segment as l2segment
import grpc_meta.msg as msg
import types_pb2

def PreCreateCb(data, req_spec, resp_spec):
    if req_spec.request[0].key_or_handle.nw_key.vrf_key_handle.vrf_id == vrf.infra_vrf_id:
        if (l2segment.infra_update_done == 1 and 
            l2segment.current_infra_types == l2segment.max_infra_types):
            req_spec.request[0].key_or_handle.nw_key.vrf_key_handle.vrf_id = vrf.customer_vrf_id
    req_spec.request[0].gateway_ep_handle = 0

def PostCreateCb(data, req_spec, resp_spec):
    data.exp_data.spec = req_spec.request[0]
    data.exp_data.spec.ClearField("meta")
    if req_spec.request[0].key_or_handle.nw_key.ip_prefix.address.ip_af == types_pb2.IP_AF_INET6:
        if (req_spec.request[0].key_or_handle.nw_key.ip_prefix.address.v6_addr,
                req_spec.request[0].key_or_handle.nw_key.ip_prefix.prefix_len) not in msg.IPv6GenList:
            msg.IPv6GenList.append((req_spec.request[0].key_or_handle.nw_key.ip_prefix.address.v6_addr,
                    req_spec.request[0].key_or_handle.nw_key.ip_prefix.prefix_len))
    else:
        if (req_spec.request[0].key_or_handle.nw_key.ip_prefix.address.v4_addr,
                req_spec.request[0].key_or_handle.nw_key.ip_prefix.prefix_len) not in msg.IPv4GenList:
            msg.IPv4GenList.append((req_spec.request[0].key_or_handle.nw_key.ip_prefix.address.v4_addr,
                    req_spec.request[0].key_or_handle.nw_key.ip_prefix.prefix_len))

def PostGetCb(data, req_spec, resp_spec):
    data.actual_data.spec = resp_spec.response[0].spec
    
def PostUpdateCb(data, req_spec, resp_spec):
    PostCreateCb(data, req_spec, resp_spec)
