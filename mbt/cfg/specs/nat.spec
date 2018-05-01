ProtoObject: nat_pb2
Service: Nat
enabled : True
dolEnabled : True
objects:
    - object :
        name : NatPool
        key_handle : NatPoolKeyHandle
        ignore:
        create:
            api      : NatPoolCreate
            request  : NatPoolRequestMsg
            response : NatPoolResponseMsg
            pre_cb   : callback://nat/PreCreateCb
            post_cb  : None
        update:
            api      : None
            request  : None
            response : None
            pre_cb   : None
            post_cb  : None
        delete:
            api      : NatPoolDelete
            request  : NatPoolDeleteRequestMsg
            response : NatPoolDeleteResponseMsg
            pre_cb   : None
            post_cb  : None
        get:
            api      : NatPoolGet
            request  : NatPoolGetRequestMsg
            response : NatPoolGetResponseMsg
            pre_cb   : None
            post_cb  : None
    - object :
        name : NatMapping
        key_handle : NatMappingKeyHandle
        ignore:
        create:
            api      : NatMappingCreate
            request  : NatMappingRequestMsg
            response : NatMappingResponseMsg
            pre_cb   : callback://nat/MappingPreCreateCb
            post_cb  : None
        update:
            api      : None
            request  : None
            response : None
            pre_cb   : None
            post_cb  : None
        delete:
            api      : NatMappingDelete
            request  : NatMappingDeleteRequestMsg
            response : NatMappingDeleteResponseMsg
            pre_cb   : None
            post_cb  : None
        get:
            api      : NatMappingGet
            request  : NatMappingGetRequestMsg
            response : NatMappingGetResponseMsg
            pre_cb   : None
            post_cb  : None
