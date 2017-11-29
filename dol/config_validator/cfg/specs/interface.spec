ProtoObject: interface_pb2
Service: Interface
enabled : True
graphEnabled : True
objects:
    - object :
        name : Interface
        ignore:
            - op : Get
            - op : Update
            - op : Create
            - op : Delete
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
            pre_cb   : None
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
        ignore:
            - op : Get
            - op : Update
            - op : Delete
            - op : Create
        create:
            api      : LifCreate
            request  : LifRequestMsg
            response : LifResponseMsg
            pre_cb   : None
            post_cb  : None
        update:
            api      : LifUpdate
            request  : LifRequestMsg
            response : LifResponseMsg
            pre_cb   : None
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

