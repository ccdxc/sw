ProtoObject: port_pb2
Service: Port
enabled : False
graphEnabled : False
objects:
    - object :
        name : Port
        ignore:
            - op : Get
            - op : Update
            - op : Create
            - op : Delete
        create:
            api      : PortCreate
            request  : PortRequestMsg
            response : PortResponseMsg
            pre_cb   : callback://port/PreCreateCb
            post_cb  : None
        update:
            api      : PortUpdate
            request  : PortRequestMsg
            response : PortResponseMsg
            pre_cb   : callback://port/PreUpdateCb
            post_cb  : None
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
            post_cb  : None

