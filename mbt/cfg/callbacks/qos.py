import kh_pb2
import random

key_choices = [kh_pb2.USER_DEFINED_1, kh_pb2.USER_DEFINED_2, kh_pb2.USER_DEFINED_3,
           kh_pb2.USER_DEFINED_4, kh_pb2.USER_DEFINED_5, kh_pb2.USER_DEFINED_6]
dot1q_pcp = [i for i in range(1,8)]
ip_dscp = [i for i in range(0,64)]
reclaim_dict = {}
pfc_class_counter = 0
max_pfc_class_counter = 3
def PreCreateCb(data, req_spec, resp_spec):
    global key_choices
    global dot1q_pcp
    global ip_dscp
    global pfc_class_counter
    # We cannot have more than 6 QOS objects. 
    if len(key_choices) == 0:
        assert False
    key_choice = random.choice(key_choices)
    key_choices.remove(key_choice)

    dot1q_pcp_choice = random.choice(dot1q_pcp)
    dot1q_pcp.remove(dot1q_pcp_choice)

    ip_dscp_choice = random.choice(ip_dscp)
    ip_dscp.remove(ip_dscp_choice)

    req_spec.request[0].pfc.pfc_cos = random.randint(0,7)

    req_spec.request[0].key_or_handle.qos_group = key_choice
    req_spec.request[0].uplink_class_map.dot1q_pcp = dot1q_pcp_choice
    req_spec.request[0].uplink_class_map.ip_dscp[0] = ip_dscp_choice
    req_spec.request[0].mtu = random.randint(64,9216)

    pfc_class_counter += 1

    if pfc_class_counter > max_pfc_class_counter:
        req_spec.request[0].ClearField("pfc")
        req_spec.request[0].ClearField("buffer")

    # Create a dict with the key and pcp/dscp values, so that they can be reclaimed
    # whenever they're modified as part of update
    reclaim_dict[key_choice] = (dot1q_pcp_choice, ip_dscp_choice)

def PostCreateCb(data, req_spec, resp_spec):
    data.exp_data.spec = req_spec.request[0]
    data.exp_data.spec.ClearField("meta")

def PostGetCb(data, req_spec, resp_spec):
    data.actual_data.spec = resp_spec.response[0].spec

def PreUpdateCb(data, req_spec, resp_spec):
    global dot1q_pcp
    global ip_dscp
    global pfc_class_counter

    dot1q_pcp_choice = random.choice(dot1q_pcp)
    dot1q_pcp.remove(dot1q_pcp_choice)

    ip_dscp_choice = random.choice(ip_dscp)
    ip_dscp.remove(ip_dscp_choice)

    req_spec.request[0].pfc.pfc_cos = random.randint(0,8)

    req_spec.request[0].uplink_class_map.dot1q_pcp = dot1q_pcp_choice
    req_spec.request[0].uplink_class_map.ip_dscp[0] = ip_dscp_choice
    req_spec.request[0].mtu = random.randint(64,9217)

    # pfc_class_counter += 1

    # if pfc_class_counter > max_pfc_class_counter:
    #     req_spec.request[0].ClearField("pfc")
    #     req_spec.request[0].ClearField("buffer")

    # Add back the pcp/dscp stored earlier to the pool of choices.
    try:
        pcp, dscp = reclaim_dict[req_spec.request[0].key_or_handle.qos_group]
        dot1q_pcp.append(pcp)
        ip_dscp.append(dscp)
    except KeyError:
        pass

def PostUpdateCb(data, req_spec, resp_spec):
    PostCreateCb(data, req_spec, resp_spec)
