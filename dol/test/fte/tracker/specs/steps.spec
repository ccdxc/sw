# ALG Tracker Step Specifications
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
        id          : IFLOW_RRQ
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        fields      :
            proto   : UDP
            sport   : 65530
            dport   : 69
            tftpop  : 'RRQ'
   
    - step:
        id          : RFLOW_DATA
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        fields      :
            proto   : UDP
            dport   : 65530
            tftpop  : 'DATA'

    - step:
        id          : IFLOW_WRQ
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        fields      :
            sport   : 65530
            dport   : 69
            tftpop  : 'WRQ'

    - step:
        id          : RFLOW_ACK
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        fields      :
            dport   : 65530
            tftpop  : 'ACK'

    - step:
        id          : RFLOW_OACK
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        fields      :
            dport   : 65530
            tftpop  : 'OACK'

    - step:
        id          : RFLOW_ERROR
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        fields      :
            dport   : 65530
            tftpop  : 'ERROR'    

    - step:
        id          : IFLOW_SRPC_SYN
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        fields      :
            flags   : syn
            sport   : 65529
            dport   : 111
        
    - step:
        id          : RFLOW_SRPC_SYN_ACK
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        fields      :
            flags   : syn,ack
            sport   : 111
            dport   : 65529

    - step:
        id          : IFLOW_SRPC_ACK
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        fields      :
            flags   : ack
            sport   : 65529
            dport   : 111

    - step:
        id           : IFLOW_GETPORT
        base         : ref://trackerstore/steps/id=IFLOW_BASE
        fields       :
            sport    : 65529
            dport    : 111
            l7:
                xid      : 1122334455
                msg_type : 0
                pgm      : 100000
                pgmvers  : 2
                proc     : 3
                datapgm  : 100024
                datavers : 1
                dataproto: 17 

    - step:
        id           : RFLOW_GETPORT_RESPONSE
        base         : ref://trackerstore/steps/id=RFLOW_BASE
        fields       :
            sport    : 111
            dport    : 65529
            l7:
                xid      : 1122334455
                msg_type : 1
                reply    : 0
                accept   : 0
                dataport : 54891

    - step:
        id          : IFLOW_VERS4_DUMP
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        fields      :
            sport   : 65529
            dport   : 111
            l7:
                xid     : 1122334466
                frag_hdr: 0x80000000
                msg_type: 0
                pgm     : 100000
                pgmvers : 4
                proc    : 4

    - step:
        id            : IFLOW_VERS4_PMAP_STAT
        base          : ref://trackerstore/steps/id=RFLOW_BASE
        fields        :
            sport     : 111
            dport     : 65529
            l7:
                xid       : 1122334466
                frag_hdr  : 0x80000000
                msg_type  : 1
                pgm       : 100000
                pgmvers   : 4
                proc      : 4
                reply     : 0
                accept    : 0
                valfollows: 1

    - step:
        id          : IFLOW_DUMP_DATA
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        fields      :
            dport   : 32776 

    - step:
        id          : IFLOW_GETPORT_DATA
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        fields      :
            dport   : 54891

    - step:
        id          : IFLOW_MSRPC_DG_LE
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        fields      : 
           sport    : 65528
           dport    : 135
           l7:
               rpc_vers : 4
               ptype    : 0  #PDU_REQ
               flags1   : 2
               serial_lo: 1
               drep     : 0x100000 
               object:
                   time_lo     : 00000000
                   time_mid    : 0000
                   time_hi_vers: 0000
                   clock_seq_hi: 00
                   clock_seq_lo: 00
                   node_hi     : 000000
                   node_lo     : 000000
               if_id: 
                   time_lo     : 0x0883afe1
                   time_mid    : 0x1f5d
                   time_hi_vers: 0xc911
                   clock_seq_hi: 0xa4
                   clock_seq_lo: 0x08
                   node_hi     : 0x002b14
                   node_lo     : 0xa0fa03
               act_id:
                   time_lo     : 00000000
                   time_mid    : 0000
                   time_hi_vers: 0000
                   clock_seq_hi: 00
                   clock_seq_lo: 00
                   node_hi     : 000000
                   node_lo     : 000100
               seqnum   : 1
               opnum    : 0
               fragnum  : 0
        
    - step:
        id          : RFLOW_MSRPC_DG_LE
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        fields      :
           sport    : 65527
           dport    : 65528 
           l7:
               rpc_vers : 4
               ptype    : 2 #PDU_RSP
               flags1   : 2
               serial_lo: 1
               drep     : 0x100000
               object:
                   time_lo     : 00000000
                   time_mid    : 0000
                   time_hi_vers: 0000
                   clock_seq_hi: 00
                   clock_seq_lo: 00
                   node_hi     : 000000
                   node_lo     : 000000
               if_id:
                   time_lo     : 0x0883afe1
                   time_mid    : 0x1f5d
                   time_hi_vers: 0xc911
                   clock_seq_hi: 0xa4
                   clock_seq_lo: 0x08
                   node_hi     : 0x002b14
                   node_lo     : 0xa0fa03
               act_id:
                   time_lo     : 00000000
                   time_mid    : 0000
                   time_hi_vers: 0000
                   clock_seq_hi: 00
                   clock_seq_lo: 00
                   node_hi     : 000000
                   node_lo     : 000100
               seqnum   : 1
               opnum    : 0
               fragnum  : 0

    - step:
        id          : IFLOW_MSRPC_DG_DATA
        base        : ref://trackerstore/steps/id=IFLOW_BASE
        fields      :
           sport    : 65528
           dport    : 65527
           l7:
               rpc_vers : 4
               ptype    : 'PDU_REQ'
               flags1   : 2
               serial_lo: 1
               drep     : 0x100000
               object:
                   time_lo     : 00000000
                   time_mid    : 0000
                   time_hi_vers: 0000
                   clock_seq_hi: 00
                   clock_seq_lo: 00
                   node_hi     : 000000
                   node_lo     : 000000
               if_id:
                   time_lo     : 0x0883afe1
                   time_mid    : 0x1f5d
                   time_hi_vers: 0xc911
                   clock_seq_hi: 0xa4
                   clock_seq_lo: 0x08
                   node_hi     : 0x002b14
                   node_lo     : 0xa0fa03
               act_id:
                   time_lo     : 00000000
                   time_mid    : 0000
                   time_hi_vers: 0000
                   clock_seq_hi: 00
                   clock_seq_lo: 00
                   node_hi     : 000000
                   node_lo     : 000100
               seqnum   : 2
               opnum    : 0
               fragnum  : 0

    - step:
        id          : RFLOW_MSRPC_DG_DATA
        base        : ref://trackerstore/steps/id=RFLOW_BASE
        fields      :
           sport    : 65527
           dport    : 65528
           l7:
               rpc_vers : 4
               ptype    : 'PDU_RSP'
               flags1   : 2
               serial_lo: 1
               drep     : 0x100000
               object:
                   time_lo     : 00000000
                   time_mid    : 0000
                   time_hi_vers: 0000
                   clock_seq_hi: 00
                   clock_seq_lo: 00
                   node_hi     : 000000
                   node_lo     : 000000
               if_id:
                   time_lo     : 0x0883afe1
                   time_mid    : 0x1f5d
                   time_hi_vers: 0xc911
                   clock_seq_hi: 0xa4
                   clock_seq_lo: 0x08
                   node_hi     : 0x002b14
                   node_lo     : 0xa0fa03
               act_id:
                   time_lo     : 00000000
                   time_mid    : 0000
                   time_hi_vers: 0000
                   clock_seq_hi: 00
                   clock_seq_lo: 00
                   node_hi     : 000000
                   node_lo     : 000100
               seqnum   : 2
               opnum    : 0
               fragnum  : 0
