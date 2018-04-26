import types_pb2
from grpc_meta.msg import GrpcReqRspMsg

def PreCreateCb(data, req_spec, resp_spec):
    global current_infra_types
    global max_infra_types
    global infra_vrf_id
    global customer_vrf_id

    if req_spec.request[0].key_or_handle.endpoint_key.HasField("l3_key"):
        GrpcReqRspMsg.static_generate_message(req_spec.request[0].key_or_handle.endpoint_key.l2_key)
