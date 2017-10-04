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
        id          : RFLOW_SYN_ONLY
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        fields      :
            flags   : syn
            ack     : callback://firewall/alu/Mul/val=0

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

    - step:
        id          : IFLOW_DATA_OVERLAP_LEFT
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        payloadsize : 100
        fields      :
            seq     : callback://firewall/alu/Sub/val=50
            flags   : ack

    - step:
        id          : RFLOW_DATA_OVERLAP_LEFT
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        payloadsize : 100
        fields      :
            seq     : callback://firewall/alu/Sub/val=50
            flags   : ack

    - step:
        id          : IFLOW_DATA_RETRANSMIT
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        advance     : False
        payloadsize : 100
        fields      :
            seq     : callback://firewall/alu/Sub/val=100
            flags   : ack

    - step:
        id          : RFLOW_DATA_RETRANSMIT
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        advance     : False
        payloadsize : 100
        fields      :
            seq     : callback://firewall/alu/Sub/val=100
            flags   : ack

    - step:
        id          : IFLOW_DATA_OOO_IN_WINDOW
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        payloadsize : 1000
        advance     : False
        fields      :
            seq     : callback://firewall/alu/Add/val=1000
            flags   : ack
 
    - step:
        id          : RFLOW_DATA_OOO_IN_WINDOW
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        payloadsize : 1000
        advance     : False
        fields      :
            seq     : callback://firewall/alu/Add/val=1000
            flags   : ack
 
    - step:
        id          : IFLOW_DATA_FULL_OUT_OF_WINDOW
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        payloadsize : 1000
        advance     : False
        permit      : False
        fields      :
            # Change the constant below to use rcvr_win_sz
            seq     : callback://firewall/alu/Add/val=10000
            flags   : ack

    - step:
        id          : RFLOW_DATA_FULL_OUT_OF_WINDOW
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        payloadsize : 1000
        advance     : False
        permit      : False
        fields      :
            # Change the constant below to use rcvr_win_sz
            seq     : callback://firewall/alu/Add/val=10000
            flags   : ack

