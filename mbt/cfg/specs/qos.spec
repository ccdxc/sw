ProtoObject: qos_pb2
Service: QOS
enabled : True
dolEnabled : False
objects:
    - object:
        name : Qos
        key_handle : QosClassKeyHandle
        ignore:
        create:
            api      : QosClassCreate
            request  : QosClassRequestMsg
            response : QosClassResponseMsg
            pre_cb   : callback://qos/PreCreateCb
            post_cb  : None
        update:
            api      : QosClassUpdate
            request  : QosClassRequestMsg
            response : QosClassResponseMsg
            pre_cb   : callback://qos/PreUpdateCb
            post_cb  : None
        delete:
            api      : QosClassDelete
            request  : QosClassDeleteRequestMsg
            response : QosClassDeleteResponseMsg
            pre_cb   : None
            post_cb  : None
        get:
            api      : QosClassGet
            request  : QosClassGetRequestMsg
            response : QosClassGetResponseMsg
            pre_cb   : None
            post_cb  : None
    - object:
        name : Copp
        key_handle : CoppKeyHandle
        ignore:
            - op : Get
            - op : Update
            - op : Delete
            - op : GetAll
        create:
            api      : CoppUpdate
            request  : CoppRequestMsg
            response : CoppResponseMsg
            pre_cb   : None
            post_cb  : None
        update:
            api      : CoppUpdate
            request  : CoppRequestMsg
            response : CoppResponseMsg
            pre_cb   : None
            post_cb  : None
        delete:
            api      : None
            request  : None
            response : None
            pre_cb   : None
            post_cb  : None
        get:
            api      : CoppGet
            request  : CoppGetRequestMsg
            response : CoppGetResponseMsg
            pre_cb   : None
            post_cb  : None



