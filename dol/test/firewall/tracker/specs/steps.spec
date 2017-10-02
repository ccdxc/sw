# TCP Tracker Step Specifications
steps:
    - step:
        id          : IFLOW_BASE
        direction   : iflow
        payloadsize : 0
        advance     : True
        permit      : True
        fields      :
            flags   : None

    - step:
        id          : RFLOW_BASE
        direction   : rflow
        payloadsize : 0
        advance     : True
        permit      : True
        fields      :
            flags   : None
   
    - step:
        id          : IFLOW_SYN
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        fields      :
            flags   : syn

    - step:
        id          : RFLOW_SYN_ACK
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        fields      :
            flags   : syn,ack

    - step:
        id          : IFLOW_ACK
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        fields      :
            flags   : ack

    - step:
        id          : RFLOW_ACK
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        fields      :
            flags   : ack

    - step:
        id          : IFLOW_DATA
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        payloadsize : 100
        fields      :
            flags   : ack

    - step:
        id          : RFLOW_DATA
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        payloadsize : 100
        fields      :
            flags   : ack

    - step:
        id          : IFLOW_FIN
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        fields      :
            flags   : fin,ack

    - step:
        id          : RFLOW_FIN
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        fields      :
            flags   : fin,ack

