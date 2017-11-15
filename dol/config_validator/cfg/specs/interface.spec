ProtoObject: interface_pb2
Service: Interface
enabled : False
graphEnabled : True
objects:
    - object :
        name : Interface
        ignore:
            - op : Get
            - op : Update
        create:
            api      : InterfaceCreate
            request  : InterfaceRequestMsg
            response : InterfaceResponseMsg
            pre_cb   : callback://interface/PreCreateCb
            post_cb  : callback://interface/PostCreateCb
        update:
            api      : InterfaceUpdate
            request  : InterfaceRequestMsg
            response : InterfaceResponseMsg
            pre_cb   : callback://interface/PreUpdateCb
            post_cb  : callback://interface/PostUpdateCb
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
            post_cb  : callback://interface/PostGetCb
    - object :
        name : Lif
        ignore:
            - op : Get
            - op : Update
        create:
            api      : LifCreate
            request  : LifRequestMsg
            response : LifResponseMsg
            pre_cb   : callback://vrf/PreCreateCb
            post_cb  : callback://vrf/PostCreateCb
        update:
            api      : LifUpdate
            request  : LifRequestMsg
            response : LifResponseMsg
            pre_cb   : callback://vrf/PreUpdateCb
            post_cb  : callback://vrf/PostUpdateCb
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
            post_cb  : callback://vrf/PostGetCb

