ProtoObject: vrf_pb2
Service: Vrf
enabled: True
ignore:
    - op : Get
    - op : Update
create:
    api      : VrfCreate
    request  : VrfRequestMsg
    response : VrfResponseMsg
    pre_cb   : callback://vrf/PreCreateCb
    post_cb  : callback://vrf/PostCreateCb
update:
    api      : VrfUpdate
    request  : VrfRequestMsg
    response : VrfResponseMsg
    pre_cb   : callback://vrf/PreUpdateCb
    post_cb  : callback://vrf/PostUpdateCb
delete:
    api      : VrfDelete
    request  : VrfDeleteRequestMsg
    response : VrfDeleteResponseMsg
    pre_cb   : None
    post_cb  : None
get:
    api      : VrfGet
    request  : VrfGetRequestMsg
    response : VrfGetResponseMsg
    pre_cb   : None
    post_cb  : callback://vrf/PostGetCb
