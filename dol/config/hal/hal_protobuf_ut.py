#! /usr/bin/python3
import pdb
import grpc
import sys
sys.path.append('/home/vikasd/ws/ws1/hack/saratk/nic/gen')

import l2segment_pb2 as l2segment_pb2
import l2segment_pb2_grpc as l2segment_pb2_grpc
import types_pb2 as types_pb2

channel = grpc.insecure_channel('localhost:50052')

stub = l2segment_pb2_grpc.L2SegmentStub(channel)

req_msg = l2segment_pb2.L2SegmentRequestMsg()
req_spec = req_msg.request.add()

resp_msg = stub.L2SegmentCreate(req_msg)
for resp_spec in resp_msg.response:
    print(types_pb2.ApiStatus.Name(resp_spec.api_status))
