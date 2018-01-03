ProtoObject: endpoint_pb2
Service: Endpoint
enabled : False
graphEnabled : False
objects:
    - object :
        name : Endpoint
        ignore:
            - op : Get
            - op : Update
            - op : Delete
            - op : Create
        create:
            api      : EndpointCreate
            request  : EndpointRequestMsg
            response : EndpointResponseMsg
            pre_cb   : None
            post_cb  : None
        update:
            api      : EndpointUpdate
            request  : EndpointRequestMsg
            response : EndpointResponseMsg
            pre_cb   : None
            post_cb  : None
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
            post_cb  : None

