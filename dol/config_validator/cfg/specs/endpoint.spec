ProtoObject: endpoint_pb2
Service: Endpoint
enabled : False
graphEnabled : True
objects:
    - object :
        name : Endpoint
        ignore:
            - op : Get
            - op : Update
        create:
            api      : EndpointCreate
            request  : EndpointRequestMsg
            response : EndpointResponseMsg
            pre_cb   : callback://endpoint/PreCreateCb
            post_cb  : callback://endpoint/PostCreateCb
        update:
            api      : EndpointUpdate
            request  : EndpointRequestMsg
            response : EndpointResponseMsg
            pre_cb   : callback://endpoint/PreUpdateCb
            post_cb  : callback://endpoint/PostUpdateCb
        delete:
            api      : EndpointDelete
            request  : EndpointDeleteRequestMsg
            response : EndpointDeleteResponseMsg
            pre_cb   : None
            post_cb  : None
        get:
            api      : EndpointGet
            request  : EndpointGetRequestMsg
            response : EndpointGetResponseMsg
            pre_cb   : None
            post_cb  : callback://endpoint/PostGetCb

