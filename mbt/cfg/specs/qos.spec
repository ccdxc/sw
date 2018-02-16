ProtoObject: qos_pb2
Service: QOS
enabled : True
graphEnabled : True
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

