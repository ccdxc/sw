ProtoObject: telemetry_pb2
Service: Telemetry
enabled : True
dolEnabled : True
objects:
    - object :
        name : Collector
        key_handle : CollectorKeyHandle
        ignore:
            - op : Get
        ignore_v2:
            - op : Update
        create:
            api      : CollectorCreate
            request  : CollectorRequestMsg
            response : CollectorResponseMsg
            pre_cb   : callback://telemetry/CollectorPreCreateCb
            post_cb  : None
        update:
            api      : None
            request  : None
            response : None
            pre_cb   : None
            post_cb  : None
        delete:
            api      : None
            request  : None
            response : None
            pre_cb   : None
            post_cb  : None
        get:
            api      : None
            request  : None
            response : None
            pre_cb   : None
            post_cb  : None
    - object :
        name : ExportControl
        key_handle : ExportControlKeyHandle
        ignore:
            - op : Get
        ignore_v2:
            - op : Update
        create:
            api      : None
            request  : None
            response : None
            pre_cb   : None
            post_cb  : None
        update:
            api      : None
            request  : None
            response : None
            pre_cb   : None
            post_cb  : None
        delete:
            api      : None
            request  : None
            response : None
            pre_cb   : None
            post_cb  : None
        get:
            api      : None
            request  : None
            response : None
            pre_cb   : None
            post_cb  : None
    - object :
        name : FlowMonitorRule
        key_handle : FlowMonitorRuleKeyHandle
        ignore:
            - op : Get
        ignore_v2:
            - op : Update
        create:
            api      : FlowMonitorRuleCreate
            request  : FlowMonitorRuleRequestMsg
            response : FlowMonitorRuleResponseMsg
            pre_cb   : callback://telemetry/FlowMatchPreCreateCb
            post_cb  : None
        update:
            api      : None
            request  : None
            response : None
            pre_cb   : None
            post_cb  : None
        delete:
            api      : FlowMonitorRuleDelete
            request  : FlowMonitorRuleDeleteRequestMsg
            response : FlowMonitorRuleDeleteResponseMsg
            pre_cb   : None
            post_cb  : None
        get:
            api      : None
            request  : None
            response : None
            pre_cb   : None
            post_cb  : None
    - object :
        name : DropMonitorRule
        key_handle : DropMonitorRuleKeyHandle
        ignore:
            - op : Get
        ignore_v2:
            - op : Update
        create:
            api      : DropMonitorRuleCreate
            request  : DropMonitorRuleRequestMsg
            response : DropMonitorRuleResponseMsg
            pre_cb   : None
            post_cb  : None
        update:
            api      : None
            request  : None
            response : None
            pre_cb   : None
            post_cb  : None
        delete:
            api      : DropMonitorRuleDelete
            request  : DropMonitorRuleDeleteRequestMsg
            response : DropMonitorRuleDeleteResponseMsg
            pre_cb   : None
            post_cb  : None
        get:
            api      : None
            request  : None
            response : None
            pre_cb   : None
            post_cb  : None
    - object :
        name : MirrorSession
        key_handle : MirrorSessionKeyHandle
        ignore:
            - op : Get
        ignore_v2:
            - op : Update
        create:
            api      : MirrorSessionCreate
            request  : MirrorSessionRequestMsg
            response : MirrorSessionResponseMsg
            pre_cb   : None
            post_cb  : None
        update:
            api      : None
            request  : None
            response : None
            pre_cb   : None
            post_cb  : None
        delete:
            api      : MirrorSessionDelete
            request  : MirrorSessionDeleteRequestMsg
            response : MirrorSessionDeleteResponseMsg
            pre_cb   : None
            post_cb  : None
        get:
            api      : MirrorSessionGet
            request  : MirrorSessionGetRequestMsg
            response : MirrorSessionGetResponseMsg
            pre_cb   : None
            post_cb  : None
