ProtoObject: nw_pb2
Service: Network
enabled: True
graphEnabled : True
objects:
    - object:
        name : Network
        ignore:
            - op : Get
            - op : Update
            - op : Delete
            - op : Create
        create:
            api      : NetworkCreate
            request  : NetworkRequestMsg
            response : NetworkResponseMsg
            pre_cb   : callback://network/PreCreateCb
            post_cb  : callback://network/PostCreateCb
        update:
            api      : NetworkUpdate
            request  : NetworkRequestMsg
            response : NetworkResponseMsg
            pre_cb   : None
            post_cb  : callback://network/PostUpdateCb
        delete:
            api      : NetworkDelete
            request  : NetworkDeleteRequestMsg
            response : NetworkDeleteResponseMsg
            pre_cb   : None
            post_cb  : None
        get:
            api      : NetworkGet
            request  : NetworkGetRequestMsg
            response : NetworkGetResponseMsg
            pre_cb   : None
            post_cb  : callback://network/PostGetCb

