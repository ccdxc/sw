ProtoObject: l2segment_pb2
Service: L2Segment
enabled : True
graphEnabled : True
objects:
    - object:
        name : L2Segment
        key_handle : L2SegmentKeyHandle
        ignore:
        create:
            api      : L2SegmentCreate
            request  : L2SegmentRequestMsg
            response : L2SegmentResponseMsg
            pre_cb   : callback://l2segment/PreCreateCb
            post_cb  : None
        update:
            api      : L2SegmentUpdate
            request  : L2SegmentRequestMsg
            response : L2SegmentResponseMsg
            pre_cb   : None
            post_cb  : None
        delete:
            api      : L2SegmentDelete
            request  : L2SegmentDeleteRequestMsg
            response : L2SegmentDeleteResponseMsg
            pre_cb   : None
            post_cb  : None
        get:
            api      : L2SegmentGet
            request  : L2SegmentGetRequestMsg
            response : L2SegmentGetResponseMsg
            pre_cb   : None
            post_cb  : None
