# All BRQ definitions
meta:
    id: BRQ_DEF

entries :
    - entry:
        name: "BRQ_ENCRYPT_GCM"
        defn: "BRQ_GCM"
        count: 1024
        entry_defn: "BRQ_GCM_ENTRY"
        type: 1
    - entry:
        name: "BRQ_DECRYPT_GCM"
        defn: "BRQ_GCM"
        count: 1024
        entry_defn: "BRQ_GCM_ENTRY"
        type: 2
    - entry:
        name: "BRQ_ENCRYPT_CCM"
        defn: "BRQ_CCM"
        count: 1024
        entry_defn: "BRQ_CCM_ENTRY"
        type: 6
    - entry:
        name: "BRQ_DECRYPT_CCM"
        defn: "BRQ_CCM"
        count: 1024
        entry_defn: "BRQ_CCM_ENTRY"
        type: 6
    - entry:
        name: "BRQ_ENCRYPT_CBC"
        defn: "BRQ_CBC"
        count: 1024
        entry_defn: "BRQ_CBC_ENTRY"
        type: 8
    - entry:
        name: "BRQ_DECRYPT_CBC"
        defn: "BRQ_CBC"
        count: 1024
        entry_defn: "BRQ_CBC_ENTRY"
        type: 8

