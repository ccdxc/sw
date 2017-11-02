ProtoObject: nwsec_pb2
Service: NwSecurity
enabled: True
ignore:
create:
    api      : SecurityProfileCreate
    request  : SecurityProfileRequestMsg
    response : SecurityProfileRequestMsg
    pre_cb   : None
    post_cb  : callback://security_profile/PostCreateCb
update:
    api      : SecurityProfileUpdate
    request  : SecurityProfileRequestMsg
    response : SecurityProfileRequestMsg
    pre_cb   : None
    post_cb  : callback://security_profile/PostUpdateCb
delete:
    api      : SecurityProfileDelete
    request  : SecurityProfileDeleteRequestMsg
    response : SecurityProfileDeleteResponseMsg
    pre_cb   : None
    post_cb  : None
get:
    api      : SecurityProfileGet
    request  : SecurityProfileGetRequestMsg
    response : SecurityProfileGetResponseMsg
    pre_cb   : None
    post_cb  : callback://security_profile/PostGetCb
