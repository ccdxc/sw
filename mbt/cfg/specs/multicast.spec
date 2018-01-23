ProtoObject: multicast_pb2
Service: Multicast
enabled : True
graphEnabled : True
objects:
    - object:
        name : Multicast
        key_handle : MulticastEntryKeyHandle
        ignore:
            - op : Get
            - op : Update
            - op : Delete
            - op : Create
        create:
            api      : MulticastEntryCreate
            request  : MulticastEntryRequestMsg
            response : MulticastEntryResponseMsg
            pre_cb   : callback://multicast/PreCreateCb
            post_cb  : None
        update:
            api      : MulticastEntryUpdate
            request  : MulticastEntryRequestMsg
            response : MulticastEntryResponseMsg
            pre_cb   : callback://multicast/PreUpdateCb
            post_cb  : None
        delete:
            api      : MulticastEntryDelete
            request  : MulticastEntryDeleteRequestMsg
            response : MulticastEntryDeleteResponseMsg
            pre_cb   : None
            post_cb  : None
        get:
            api      : MulticastEntryGet
            request  : MulticastEntryGetRequestMsg
            response : MulticastEntryGetResponseMsg
            pre_cb   : None
            post_cb  : None

