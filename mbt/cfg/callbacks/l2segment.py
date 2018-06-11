import types_pb2
import random

import cfg.callbacks.vrf as vrf
import cfg.callbacks.network as network

# Infra VRF can have only one L2 segment
# So when a second L2 segment is being created for an Infra VRF
# we modify the VRF ID in the callback to a customer VRF.
# We also make sure the network object referenced also belongs
# to a customer VRF

max_infra_types = 1
current_infra_types = 0
wire_encap_type = [types_pb2.ENCAP_TYPE_NONE, types_pb2.ENCAP_TYPE_DOT1Q, types_pb2.ENCAP_TYPE_VXLAN]
tunnel_encap_type = [types_pb2.ENCAP_TYPE_NONE, types_pb2.ENCAP_TYPE_VXLAN]
uniq_val_dict = {}

def get_unique_int_val(key_str):
    if key_str in uniq_val_dict:
        uniq_val_dict[key_str] += 1
    else:
        uniq_val_dict[key_str] = 10000

    return uniq_val_dict[key_str]

# Select a random enum for given enum type
def get_random_enum_desc_val(enum_type):
    index = random.randrange(len(enum_type.DESCRIPTOR.values))
    return enum_type.DESCRIPTOR.values[index]

# Returns a randomly selected enum for given enum type
# and a unique value for that enum
def get_enum_unique_val(enum_type):
    enum_desc_val = get_random_enum_desc_val(enum_type)
    return (enum_desc_val.number, get_unique_int_val(enum_desc_val.name))

def PreCreateCb(data, req_spec, resp_spec):
    global current_infra_types
    global max_infra_types

    if req_spec.request[0].vrf_key_handle.vrf_id == vrf.infra_vrf_id:
        if current_infra_types == max_infra_types:
            # Modify to be a customer vrf L2seg
            req_spec.request[0].vrf_key_handle.vrf_id = network.nw_cust_obj["vrf_id"]
            req_spec.request[0].network_key_handle[0].nw_key.vrf_key_handle.vrf_id = network.nw_cust_obj["vrf_id"]
            req_spec.request[0].network_key_handle[0].nw_key.ip_prefix.address.ip_af = network.nw_cust_obj["ip_af"]
            req_spec.request[0].network_key_handle[0].nw_key.ip_prefix.prefix_len = network.nw_cust_obj["prefix_len"]
            if network.nw_cust_obj["ip_af"] == types_pb2.IP_AF_INET6:
                req_spec.request[0].network_key_handle[0].nw_key.ip_prefix.address.v6_addr = network.nw_cust_obj["ipv6"]
            else:
                req_spec.request[0].network_key_handle[0].nw_key.ip_prefix.address.v4_addr = network.nw_cust_obj["ipv4"]
        else:
            current_infra_types += 1

    req_spec.request[0].wire_encap.encap_type  = random.choice(wire_encap_type)
    req_spec.request[0].wire_encap.encap_value = get_unique_int_val('wire_encap_type')

    req_spec.request[0].tunnel_encap.encap_type = random.choice(tunnel_encap_type)

def PostCreateCb(data, req_spec, resp_spec):
    data.exp_data.spec = req_spec.request[0]
    data.exp_data.spec.ClearField("meta")

def PostGetCb(data, req_spec, resp_spec):
    data.actual_data.spec = resp_spec.response[0].spec

def PreUpdateCb(data, req_spec, resp_spec):
    nw_vrf_id = req_spec.request[0].network_key_handle[0].nw_key.vrf_key_handle.vrf_id
    l2seg_vrf_id = req_spec.request[0].vrf_key_handle.vrf_id

    if l2seg_vrf_id != nw_vrf_id:
        # Modify to be a customer vrf L2seg
        req_spec.request[0].vrf_key_handle.vrf_id = network.nw_cust_obj["vrf_id"]
        req_spec.request[0].network_key_handle[0].nw_key.vrf_key_handle.vrf_id = network.nw_cust_obj["vrf_id"]
        req_spec.request[0].network_key_handle[0].nw_key.ip_prefix.address.ip_af = network.nw_cust_obj["ip_af"]
        req_spec.request[0].network_key_handle[0].nw_key.ip_prefix.prefix_len = network.nw_cust_obj["prefix_len"]
        if network.nw_cust_obj["ip_af"] == types_pb2.IP_AF_INET6:
            req_spec.request[0].network_key_handle[0].nw_key.ip_prefix.address.v6_addr = network.nw_cust_obj["ipv6"]
        else:
            req_spec.request[0].network_key_handle[0].nw_key.ip_prefix.address.v4_addr = network.nw_cust_obj["ipv4"]

    req_spec.request[0].wire_encap.encap_type = random.choice(wire_encap_type)
    req_spec.request[0].tunnel_encap.encap_type = random.choice(tunnel_encap_type)

def PostUpdateCb(data, req_spec, resp_spec):
    PostCreateCb(data, req_spec, resp_spec)
