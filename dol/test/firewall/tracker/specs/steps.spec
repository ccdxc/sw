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
        id          : IFLOW_DATA_DROP
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        payloadsize : 100
        permit      : False
        fields      :
            flags   : ack

    - step:
        id          : RFLOW_DATA
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        payloadsize : 100
        fields      :
            flags   : ack

    - step:
        id          : RFLOW_DATA_DROP
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        payloadsize : 100
        permit      : False
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
            seq     : callback://firewall/alu/AddWithWindow/val=1000
            flags   : ack

    - step:
        id          : RFLOW_DATA_FULL_OUT_OF_WINDOW
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        payloadsize : 1000
        advance     : False
        permit      : False
        fields      :
            seq     : callback://firewall/alu/AddWithWindow/val=1000
            flags   : ack

    - step:
        id          : IFLOW_ZERO_DATA_LEFT_OF_WINDOW
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        payloadsize : 0
        advance     : False
        fields      :
            seq     : callback://firewall/alu/SubWithWindow/val=1
            flags   : ack

    - step:
        id          : RFLOW_ZERO_DATA_LEFT_OF_WINDOW
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        payloadsize : 0
        advance     : False
        fields      :
            seq     : callback://firewall/alu/SubWithWindow/val=1
            flags   : ack

    - step:
        id          : IFLOW_ZERO_DATA_ON_WINDOW
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        payloadsize : 0
        advance     : False
        permit      : False
        fields      :
            seq     : callback://firewall/alu/SubWithWindow/val=0
            flags   : ack

    - step:
        id          : RFLOW_ZERO_DATA_ON_WINDOW
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        payloadsize : 0
        advance     : False
        permit      : False
        fields      :
            seq     : callback://firewall/alu/SubWithWindow/val=0
            flags   : ack

    - step:
        id          : IFLOW_ZERO_DATA_RIGHT_OF_WINDOW
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        payloadsize : 0
        advance     : False
        permit      : False
        fields      :
            seq     : callback://firewall/alu/AddWithWindow/val=1
            flags   : ack

    - step:
        id          : RFLOW_ZERO_DATA_RIGHT_OF_WINDOW
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        payloadsize : 0
        advance     : False
        permit      : False
        fields      :
            seq     : callback://firewall/alu/AddWithWindow/val=1
            flags   : ack

    - step:
        id          : IFLOW_DATA_WIN_ZERO
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        payloadsize : 1000
        fields      :
            window  : 0
            flags   : ack

    - step:
        id          : RFLOW_DATA_WIN_ZERO
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        payloadsize : 1000
        fields      :
            window  : 0
            flags   : ack

    - step:
        id          : IFLOW_ZERO_DATA_RCV_NEXT_MINUS_1
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        payloadsize : 0
        fields      :
            flags   : ack
            seq     : callback://firewall/alu/Sub/val=1

    - step:
        id          : RFLOW_ZERO_DATA_RCV_NEXT_MINUS_1
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        payloadsize : 0
        fields      :
            flags   : ack
            seq     : callback://firewall/alu/Sub/val=1

    - step:
        id          : IFLOW_ZERO_DATA_RCV_NEXT_MINUS_2
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        payloadsize : 0
        permit      : False
        fields      :
            flags   : ack
            seq     : callback://firewall/alu/Sub/val=2

    - step:
        id          : RFLOW_ZERO_DATA_RCV_NEXT_MINUS_2
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        payloadsize : 0
        permit      : False
        fields      :
            flags   : ack
            seq     : callback://firewall/alu/Sub/val=2

    - step:
        id          : IFLOW_ZERO_DATA_RCV_NEXT_PLUS_1
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        payloadsize : 0
        permit      : False
        fields      :
            flags   : ack
            seq     : callback://firewall/alu/Add/val=1

    - step:
        id          : RFLOW_ZERO_DATA_RCV_NEXT_PLUS_1
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        payloadsize : 0
        permit      : False
        fields      :
            flags   : ack
            seq     : callback://firewall/alu/Add/val=1
