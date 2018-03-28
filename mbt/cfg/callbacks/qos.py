import kh_pb2
import random
import config_mgr

key_choices = [kh_pb2.USER_DEFINED_1, kh_pb2.USER_DEFINED_2, kh_pb2.USER_DEFINED_3,
           kh_pb2.USER_DEFINED_4, kh_pb2.USER_DEFINED_5, kh_pb2.USER_DEFINED_6]
dot1q_pcp = [i for i in range(1,8)]
ip_dscp = [i for i in range(0,64)]
reclaim_dict = {}
dot1q_pcp_marking = [i for i in range(1,8)]
ip_dscp_marking = [i for i in range(0,64)]
reclaim_dict_marking = {}
pfc_class_counter = 0
max_pfc_class_counter = 3
def PreCreateCb(data, req_spec, resp_spec):
    global key_choices
    global dot1q_pcp
    global ip_dscp
    global reclaim_dict
    global dot1q_pcp_marking
    global ip_dscp_marking
    global reclaim_dict_marking
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

    req_spec.request[0].pfc.cos = random.randint(1,7)

    req_spec.request[0].key_or_handle.qos_group = key_choice
    req_spec.request[0].uplink_class_map.dot1q_pcp = dot1q_pcp_choice
    req_spec.request[0].uplink_class_map.ip_dscp[0] = ip_dscp_choice
    req_spec.request[0].mtu = random.randint(64,9216)

    # Create a dict with the key and pcp/dscp values, so that they can be reclaimed
    # whenever they're modified as part of update
    reclaim_dict[key_choice] = (dot1q_pcp_choice, ip_dscp_choice)

    dot1q_pcp_choice = random.choice(dot1q_pcp_marking)
    dot1q_pcp_marking.remove(dot1q_pcp_choice)

    ip_dscp_choice = random.choice(ip_dscp_marking)
    ip_dscp_marking.remove(ip_dscp_choice)

    req_spec.request[0].marking.dot1q_pcp = dot1q_pcp_choice
    req_spec.request[0].marking.ip_dscp = ip_dscp_choice

    pfc_class_counter += 1

    if pfc_class_counter > max_pfc_class_counter:
        print("Cleared PFC")
        req_spec.request[0].ClearField("pfc")

    # Create a dict with the key and pcp/dscp values, so that they can be reclaimed
    # whenever they're modified as part of update
    reclaim_dict_marking[key_choice] = (dot1q_pcp_choice, ip_dscp_choice)

def PostCreateCb(data, req_spec, resp_spec):
    global dot1q_pcp
    global ip_dscp
    global reclaim_dict
    global dot1q_pcp_marking
    global ip_dscp_marking
    global reclaim_dict_marking

    data.exp_data.spec = req_spec.request[0]
    data.exp_data.spec.ClearField("meta")

    key_choice = req_spec.request[0].key_or_handle.qos_group

    dot1q_pcp_val = req_spec.request[0].uplink_class_map.dot1q_pcp
    if req_spec.request[0].uplink_class_map.ip_dscp:
        ip_dscp_val = req_spec.request[0].uplink_class_map.ip_dscp[0]
    else:
        ip_dscp_val = 0

    if (dot1q_pcp.count(dot1q_pcp_val)):
        dot1q_pcp.remove(dot1q_pcp_val)
    if ip_dscp_val:
        if (ip_dscp.count(ip_dscp_val)):
            ip_dscp.remove(ip_dscp_val)

    # Create a dict with the key and pcp/dscp values, so that they can be reclaimed
    # whenever they're modified as part of update
    reclaim_dict[key_choice] = (dot1q_pcp_val, ip_dscp_val)

    dot1q_pcp_val = req_spec.request[0].marking.dot1q_pcp
    ip_dscp_val = req_spec.request[0].marking.ip_dscp

    if (dot1q_pcp_marking.count(dot1q_pcp_val)):
        dot1q_pcp_marking.remove(dot1q_pcp_val)
    if (ip_dscp_marking.count(ip_dscp_val)):
        ip_dscp_marking.remove(ip_dscp_val)

    # Create a dict with the key and pcp/dscp values, so that they can be reclaimed
    # whenever they're modified as part of update
    reclaim_dict_marking[key_choice] = (dot1q_pcp_val, ip_dscp_val)

def PostGetCb(data, req_spec, resp_spec):
    data.actual_data.spec = resp_spec.response[0].spec

def PreUpdateCb(data, req_spec, resp_spec):
    global dot1q_pcp
    global ip_dscp
    global reclaim_dict
    global dot1q_pcp_marking
    global ip_dscp_marking
    global reclaim_dict_marking
    global pfc_class_counter

    dot1q_pcp_choice = random.choice(dot1q_pcp)
    dot1q_pcp.remove(dot1q_pcp_choice)

    ip_dscp_choice = random.choice(ip_dscp)
    ip_dscp.remove(ip_dscp_choice)

    config_object = config_mgr.GetConfigObjectFromKey(req_spec.request[0].key_or_handle)
    cache_create_msg = config_object._msg_cache[config_mgr.ConfigObjectMeta.CREATE]

    if (cache_create_msg.request[0].pfc.cos == 0):
        req_spec.request[0].ClearField("pfc")
    else:
        req_spec.request[0].pfc.cos = random.randint(1,7)
        req_spec.request[0].pfc.xon_threshold = random.randint(64,20480)
        req_spec.request[0].pfc.xoff_clear_limit = random.randint(64,20480)

    req_spec.request[0].uplink_class_map.dot1q_pcp = dot1q_pcp_choice
    req_spec.request[0].uplink_class_map.ip_dscp[0] = ip_dscp_choice
    req_spec.request[0].mtu = random.randint(64,9217)

    # Add back the pcp/dscp stored earlier to the pool of choices.
    try:
        pcp, dscp = reclaim_dict[req_spec.request[0].key_or_handle.qos_group]
        dot1q_pcp.append(pcp)
        ip_dscp.append(dscp)
    except KeyError:
        pass

    dot1q_pcp_choice = random.choice(dot1q_pcp_marking)
    dot1q_pcp_marking.remove(dot1q_pcp_choice)

    ip_dscp_choice = random.choice(ip_dscp_marking)
    ip_dscp_marking.remove(ip_dscp_choice)

    req_spec.request[0].marking.dot1q_pcp = dot1q_pcp_choice
    req_spec.request[0].marking.ip_dscp = ip_dscp_choice

    # pfc_class_counter += 1

    # if pfc_class_counter > max_pfc_class_counter:
    #     req_spec.request[0].ClearField("pfc")

    # Add back the pcp/dscp stored earlier to the pool of choices.
    try:
        pcp, dscp = reclaim_dict_marking[req_spec.request[0].key_or_handle.qos_group]
        dot1q_pcp_marking.append(pcp)
        ip_dscp_marking.append(dscp)
    except KeyError:
        pass

def PostUpdateCb(data, req_spec, resp_spec):
    PostCreateCb(data, req_spec, resp_spec)
