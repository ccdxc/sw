ProtoObject: tenant_pb2
Service: Tenant
enabled: True
ignore:
    - op : Get
    - op : Update
create:
    api      : TenantCreate
    request  : TenantRequestMsg
    response : TenantResponseMsg
    pre_cb   : callback://tenant/PreCreateCb
    post_cb  : callback://tenant/PostCreateCb
update:
    api      : TenantUpdate
    request  : TenantRequestMsg
    response : TenantResponseMsg
    pre_cb   : callback://tenant/PreUpdateCb
    post_cb  : callback://tenant/PostUpdateCb
delete:
    api      : TenantDelete
    request  : TenantDeleteRequestMsg
    response : TenantDeleteResponseMsg
    pre_cb   : None
    post_cb  : None
get:
    api      : TenantGet
    request  : TenantGetRequestMsg
    response : TenantGetResponseMsg
    pre_cb   : None
    post_cb  : callback://tenant/PostGetCb