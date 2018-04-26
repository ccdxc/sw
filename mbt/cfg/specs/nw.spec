ProtoObject: nw_pb2
Service: Network
enabled : True
dolEnabled : True
objects:
    - object:
        name : Network
        key_handle : NetworkKeyHandle
        ignore:
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
    - object:
        name : Nexthop
        key_handle : NexthopKeyHandle
        ignore:
        create:
            api      : NexthopCreate
            request  : NexthopRequestMsg
            response : NexthopResponseMsg
            pre_cb   : None
            post_cb  : None
        update:
            api      : NexthopUpdate
            request  : NexthopRequestMsg
            response : NexthopResponseMsg
            pre_cb   : None
            post_cb  : None
        delete:
            api      : NexthopDelete
            request  : NexthopDeleteRequestMsg
            response : NexthopDeleteResponseMsg
            pre_cb   : None
            post_cb  : None
        get:
            api      : NexthopGet
            request  : NexthopGetRequestMsg
            response : NexthopGetResponseMsg
            pre_cb   : None
            post_cb  : None
    - object:
        name : Route
        key_handle : RouteKeyHandle
        ignore:
        create:
            api      : RouteCreate
            request  : RouteRequestMsg
            response : RouteResponseMsg
            pre_cb   : None
            post_cb  : None
        update:
            api      : RouteUpdate
            request  : RouteRequestMsg
            response : RouteResponseMsg
            pre_cb   : None
            post_cb  : None
        delete:
            api      : RouteDelete
            request  : RouteDeleteRequestMsg
            response : RouteDeleteResponseMsg
            pre_cb   : None
            post_cb  : None
        get:
            api      : RouteGet
            request  : RouteGetRequestMsg
            response : RouteGetResponseMsg
            pre_cb   : None
            post_cb  : None
