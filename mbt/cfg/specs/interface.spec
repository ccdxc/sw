ProtoObject: interface_pb2
Service: Interface
enabled : True
graphEnabled : True
objects:
    - object :
        name : Interface
        key_handle : InterfaceKeyHandle
        ignore:
        create:
            api      : InterfaceCreate
            request  : InterfaceRequestMsg
            response : InterfaceResponseMsg
            pre_cb   : callback://interface/PreCreateCb
            post_cb  : None
        update:
            api      : InterfaceUpdate
            request  : InterfaceRequestMsg
            response : InterfaceResponseMsg
            pre_cb   : callback://interface/PreUpdateCb
            post_cb  : None
        delete:
            api      : InterfaceDelete
            request  : InterfaceDeleteRequestMsg
            response : InterfaceDeleteResponseMsg
            pre_cb   : None
            post_cb  : None
        get:
            api      : InterfaceGet
            request  : InterfaceGetRequestMsg
            response : InterfaceGetResponseMsg
            pre_cb   : None
            post_cb  : None
    - object :
        name : Lif
        key_handle : LifKeyHandle
        ignore:
            - op : Get
            - op : Update
            - op : Delete
            - op : Create
        create:
            api      : LifCreate
            request  : LifRequestMsg
            response : LifResponseMsg
            pre_cb   : callback://interface/LifPreCreateCb
            post_cb  : None
        update:
            api      : LifUpdate
            request  : LifRequestMsg
            response : LifResponseMsg
            pre_cb   : callback://interface/LifPreCreateCb
            post_cb  : None
        delete:
            api      : LifDelete
            request  : LifDeleteRequestMsg
            response : LifDeleteResponseMsg
            pre_cb   : None
            post_cb  : None
        get:
            api      : LifGet
            request  : LifGetRequestMsg
            response : LifGetResponseMsg
            pre_cb   : None
            post_cb  : None
    - object :
        name : InterfaceL2Segment
        ignore:
            - op : Get
            - op : Update
            - op : Delete
            - op : Create
        create:
            api      : AddL2SegmentOnUplink
            request  : InterfaceL2SegmentRequestMsg
            response : InterfaceL2SegmentResponseMsg
            pre_cb   : None
            post_cb  : None
        update:
            api      : None
            request  : None
            response : None
            pre_cb   : None
            post_cb  : None
        delete:
            api      : DelL2SegmentOnUplink
            request  : InterfaceL2SegmentRequestMsg
            response : InterfaceL2SegmentResponseMsg
            pre_cb   : None
            post_cb  : None
        get:
            api      : None
            request  : None
            response : None
            pre_cb   : None
            post_cb  : None
