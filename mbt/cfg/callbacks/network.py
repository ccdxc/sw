import cfg.callbacks.vrf as vrf
import grpc_meta.msg as msg
import types_pb2

nw_cust_obj = {}

def PreCreateCb(data, req_spec, resp_spec):
    req_spec.request[0].gateway_ep_handle = 0

def PostCreateCb(data, req_spec, resp_spec):
    global nw_cust_obj
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

    if req_spec.request[0].key_or_handle.nw_key.vrf_key_handle.vrf_id in vrf.customer_vrf_id:
        nw_cust_obj["vrf_id"] = req_spec.request[0].key_or_handle.nw_key.vrf_key_handle.vrf_id
        nw_cust_obj["ip_af"] = req_spec.request[0].key_or_handle.nw_key.ip_prefix.address.ip_af
        nw_cust_obj["prefix_len"] = req_spec.request[0].key_or_handle.nw_key.ip_prefix.prefix_len
        if nw_cust_obj["ip_af"] == types_pb2.IP_AF_INET6:
            nw_cust_obj["ipv6"] = req_spec.request[0].key_or_handle.nw_key.ip_prefix.address.v6_addr
        else:
            nw_cust_obj["ipv4"] = req_spec.request[0].key_or_handle.nw_key.ip_prefix.address.v4_addr
        print(nw_cust_obj)

def PostGetCb(data, req_spec, resp_spec):
    data.actual_data.spec = resp_spec.response[0].spec
    
def PostUpdateCb(data, req_spec, resp_spec):
    PostCreateCb(data, req_spec, resp_spec)
