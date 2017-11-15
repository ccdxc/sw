ProtoObject: port_pb2
Service: Port
enabled : False
graphEnabled : True
objects:
    - object :
        name : Port
        ignore:
            - op : Get
            - op : Update
        create:
            api      : PortCreate
            request  : PortRequestMsg
            response : PortResponseMsg
            pre_cb   : callback://port/PreCreateCb
            post_cb  : callback://port/PostCreateCb
        update:
            api      : PortUpdate
            request  : PortRequestMsg
            response : PortResponseMsg
            pre_cb   : callback://port/PreUpdateCb
            post_cb  : callback://port/PostUpdateCb
        delete:
            api      : PortDelete
            request  : PortDeleteRequestMsg
            response : PortDeleteResponseMsg
            pre_cb   : None
            post_cb  : None
        get:
            api      : PortGet
            request  : PortGetRequestMsg
            response : PortGetResponseMsg
            pre_cb   : None
            post_cb  : callback://port/PostGetCb

