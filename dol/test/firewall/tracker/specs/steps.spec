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
        id          : IFLOW_SYN_WS
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        fields      :
            flags   : syn
            options :
                scale       : ref://step/ifstate/scale

    - step:
        id          : IFLOW_SYN_MSS
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        fields      :
            flags   : syn
            options :
                mss       : ref://step/ifstate/mss

    - step:
        id          : IFLOW_SYN_WS_MSS
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        fields      :
            flags   : syn
            options :
                scale     : ref://step/ifstate/scale
                mss       : ref://step/ifstate/mss

    - step:
        id          : IFLOW_SYN_TS
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        fields      :
            flags   : syn
            options :
                timestamp       : 0x12345678
                NOP             : 0

    - step:
        id          : IFLOW_SYN_DROP
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        permit      : False
        fields      :
            flags   : syn

    # Used for 5 way handshake. We need to decrement the seq
    # so that we send the original syn seq again.
    - step:
        id          : IFLOW_SYN_ACK
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        advance     : False
        fields      :
            flags   : syn,ack
            seq     : callback://firewall/alu/Sub/val=1

    - step:
        id          : RFLOW_SYN_ACK
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        fields      :
            flags   : syn,ack

    - step:
        id          : RFLOW_SYN_ACK_WS
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        fields      :
            flags   : syn,ack
            options :
                scale       : ref://step/rfstate/scale

    - step:
        id          : RFLOW_SYN_ACK_MSS
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        fields      :
            flags   : syn,ack
            options :
                mss       : ref://step/rfstate/mss

    - step:
        id          : RFLOW_SYN_ACK_WS_MSS
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        fields      :
            flags   : syn,ack
            options :
                scale     : ref://step/rfstate/scale
                mss       : ref://step/rfstate/mss

    - step:
        id          : RFLOW_SYN_ACK_TS
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        fields      :
            flags   : syn,ack
            options :
                timestamp       : 0x23456789
                nop             : none

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
        id          : RFLOW_SYN_DROP
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        permit      : False
        fields      :
            flags   : syn

    - step:
        id          : IFLOW_DATA
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        payloadsize : 1000
        fields      :
            flags   : ack

    - step:
        id          : IFLOW_DATA_DROP
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        payloadsize : 1000
        permit      : False
        fields      :
            flags   : ack

    - step:
        id          : RFLOW_DATA
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        payloadsize : 1000
        fields      :
            flags   : ack

    - step:
        id          : RFLOW_DATA_DROP
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        payloadsize : 1000
        permit      : False
        fields      :
            flags   : ack

    - step:
        id          : IFLOW_FIN
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        fields      :
            flags   : fin,ack

    - step:
        id          : IFLOW_FIN_ACK_MINUS_1
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        fields      :
            flags   : fin,ack
            ack     : callback://firewall/alu/Sub/val=1

    - step:
        id          : RFLOW_FIN
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        fields      :
            flags   : fin,ack

    - step:
        id          : RFLOW_FIN_ACK_MINUS_1
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        fields      :
            flags   : fin,ack
            ack     : callback://firewall/alu/Sub/val=1

    - step:
        id          : IFLOW_DATA_OVERLAP_LEFT
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        payloadsize : 1000
        fields      :
            seq     : callback://firewall/alu/Sub/val=50
            flags   : ack

    - step:
        id          : RFLOW_DATA_OVERLAP_LEFT
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        payloadsize : 1000
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

    - step:
        id          : IFLOW_RST
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        payloadsize : 0
        fields      :
            flags   : rst

    - step:
        id          : RFLOW_RST
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        payloadsize : 0
        fields      :
            flags   : rst

    - step:
        id          : RFLOW_RST_FOR_SYN
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        payloadsize : 0
        fields      :
            flags   : rst
            seq     : 0

    - step:
        id          : RFLOW_RST_FOR_SYN_INVALID_ACK_NUM
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        payloadsize : 0
        permit      : False
        fields      :
            flags   : rst
            seq     : 0
            ack     : callback://firewall/alu/Sub/val=1

    - step:
        id          : IFLOW_DATA_ABS
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        payloadsize : 100
        fields      :
            flags   : ack
            options :
                scale       : ref://step/ifstate/scale
                mss         : ref://step/ifstate/mss
                timestamp   : 12345678
                
        state       :
            iflow   :
                seq : 9876
                ack : 5432
                window: 3859

    - step:
        id          : RFLOW_DATA_ABS
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        payloadsize : 100
        fields      :
            flags   : ack
        state       :
            rflow   :
                seq : 6789
                ack : 2345
                window: 4096

    - step:
        id          : IFLOW_DATA_ABS_BOTH
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        payloadsize : 100
        fields      :
            flags   : ack
        state       :
            iflow   :
                seq : 9876222
                ack : 5432111
                window: 4096
            rflow   :
                seq : 4677789
                ack : 5442378
