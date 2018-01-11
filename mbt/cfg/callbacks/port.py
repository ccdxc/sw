import port_pb2
mac_id = None
mac_ch = None
num_lanes = None
def PreCreateCb(data, req_spec, resp_spec):
    req_spec.request[0].port_type = port_pb2.PORT_TYPE_ETH
    global mac_id, mac_ch, num_lanes
    mac_id = req_spec.request[0].mac_id
    mac_ch = req_spec.request[0].mac_ch
    num_lanes = req_spec.request[0].num_lanes
    if ( req_spec.request[0].port_speed == port_pb2.PORT_SPEED_NONE ):
        req_spec.request[0].port_speed = port_pb2.PORT_SPEED_100G
    if ( req_spec.request[0].port_speed == port_pb2.PORT_SPEED_100G ):
        req_spec.request[0].num_lanes = 4
    else:
        req_spec.request[0].num_lanes = 1

def PostCreateCb(data, req_spec, resp_spec):
    data.exp_data.spec = req_spec.request[0]
    data.exp_data.spec.ClearField("meta")

def PostGetCb(data, req_spec, resp_spec):
    data.actual_data.spec = resp_spec.response[0].spec
    
def PreUpdateCb(data, req_spec, resp_spec):
    req_spec.request[0].port_type = port_pb2.PORT_TYPE_ETH
    global mac_id, mac_ch
    req_spec.request[0].mac_id = mac_id
    req_spec.request[0].mac_ch = mac_ch
    req_spec.request[0].num_lanes = num_lanes
