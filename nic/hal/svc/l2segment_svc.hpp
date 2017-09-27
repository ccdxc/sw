#ifndef __L2SEGMENT_SVC_HPP__
#define __L2SEGMENT_SVC_HPP__

#include "nic/include/base.h"
#include "grpc++/grpc++.h"
#include "nic/proto/types.pb.h"
#include "nic/proto/hal/l2segment.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using l2segment::L2Segment;
using l2segment::L2SegmentSpec;
using l2segment::L2SegmentStatus;
using l2segment::L2SegmentResponse;
using l2segment::L2SegmentKeyHandle;
using l2segment::L2SegmentRequestMsg;
using l2segment::L2SegmentResponseMsg;
using l2segment::L2SegmentDeleteRequestMsg;
using l2segment::L2SegmentDeleteResponseMsg;
using l2segment::L2SegmentGetRequest;
using l2segment::L2SegmentGetRequestMsg;
using l2segment::L2SegmentGetResponse;
using l2segment::L2SegmentGetResponseMsg;

class L2SegmentServiceImpl final : public L2Segment::Service {
public:
    Status L2SegmentCreate(ServerContext *context,
                           const L2SegmentRequestMsg *req,
                           L2SegmentResponseMsg *rsp) override;

    Status L2SegmentUpdate(ServerContext *context,
                           const L2SegmentRequestMsg *req,
                           L2SegmentResponseMsg *rsp) override;


    Status L2SegmentDelete(ServerContext *context,
                           const L2SegmentDeleteRequestMsg *req,
                           L2SegmentDeleteResponseMsg *rsp) override;

    Status L2SegmentGet(ServerContext *context,
                        const L2SegmentGetRequestMsg *req,
                        L2SegmentGetResponseMsg *rsp) override;
};

#endif    // __L2SEGMENT_SVC_HPP__

