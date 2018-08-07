# Flow generation configuration template.
meta:
    id: MULTICAST_GROUPS_CLASSIC

entries:
    - entry:
        type    : mac
        source  : ipaddr/0.0.0.0
        group   : macstep/0100.5e40.4000/0000.0000.0001/64
        l3type  : ipv4
        gip     : ipstep/228.64.64.1/0.0.0.1/64
        session :
            label           : classicl2mc
            multicast       : True
            initiator       :
                ethertype   : 0x800

    - entry:
        type    : mac
        source  : ipaddr/0.0.0.0
        group   : macstep/3333.0040.4000/0000.0000.0001/64
        l3type  : ipv6
        gip     : ipv6step/FF05::0040:4001/0::1/64
        session :
            label           : classicl2mc
            multicast       : True
            initiator       :
                ethertype   : 0x86DD

    - entry:
        type    : mac
        config  : False
        source  : ipaddr/0.0.0.0
        group   : macstep/0100.5e40.4000/0000.0000.0001/64
        l3type  : ipv4
        gip     : ipstep/228.64.64.1/0.0.0.1/64
        session :
            label           : classic_allmc
            multicast       : True
            initiator       :
                ethertype   : 0x800

    - entry:
        type    : mac
        config  : False
        source  : ipaddr/0.0.0.0
        group   : macstep/3333.0040.4000/0000.0000.0001/64
        l3type  : ipv6
        gip     : ipv6step/FF05::0040:4001/0::1/64
        session :
            label           : classic_allmc
            multicast       : True
            initiator       :
                ethertype   : 0x86DD
