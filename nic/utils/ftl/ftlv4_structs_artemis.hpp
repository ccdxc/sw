//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#define FTLV4_ARTEMIS_ENTRY_NUM_HINTS 2
struct __attribute__((__packed__)) ftlv4_entry_t {
    // data after key
    uint32_t __pad_to_512b : 7;
    uint32_t entry_valid : 1;
    uint32_t more_hints_pad : 8;
    uint32_t more_hints: 23;
    uint32_t more_hashes : 1;
    uint32_t hint2 : 23;
    uint32_t hash2 : 9;
    uint32_t hint1 : 23;
    uint32_t hash1 : 9;

    // key
    uint32_t vpc_id : 8;
    uint32_t sport : 16;
    uint32_t dport : 16;
    uint32_t src;
    uint32_t proto : 8;
    uint32_t dst;

    uint32_t pad8 : 8;
    uint32_t flow_role : 1;
    uint32_t session_index : 23;
    uint32_t epoch : 8;

#ifdef __cplusplus
public:
    void tostr(char *buff, uint32_t len) {
        snprintf(buff, len, "more_hints:%d more_hashes:%d "
            "hint2:%d hash2:%#x hint1:%d hash1:%#x sport:%d dport:%d "
            "src:%08x dst:%08x vpc_id:%d proto:%d "
            "flow_role:%d session_index:%d epoch:%d entry_valid:%d",
            more_hints, more_hashes, hint2, hash2, hint1, hash1, sport,
            dport, src, dst, vpc_id, proto, flow_role,
            session_index, epoch, entry_valid);
    }

    static void keyheader2str(char *buff, uint32_t len) {
        char *cur = buff;
        cur += snprintf(cur, len, "%16s\t%16s\t%5s\t%5s\t%3s\t%4s\t%8s\n",
                        "SrcIP", "DstIP", "SrcPort", "DstPort", "Proto", "VPC", "Session");
        snprintf(cur, buff + len - cur, "%16s\t%16s\t%5s\t%5s\t%3s\t%4s\t%8s\n",
                 "-----", "-----", "-------", "-------", "-----", "----", "----");
        return;
    }

    void key2str(char *buff, uint32_t len) {
        char srcstr[INET6_ADDRSTRLEN], dststr[INET6_ADDRSTRLEN];
        uint32_t sip, dip;

        sip = htonl(src);
        dip = htonl(dst);
        inet_ntop(AF_INET, &sip, srcstr, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &dip, dststr, INET_ADDRSTRLEN);
        snprintf(buff, len, "%16s\t%16s\t%5u\t%5u\t%3u\t%4u\t%8u\n",
                 srcstr, dststr, sport, dport,
                 proto, vpc_id, session_index);
    }

    void clear_hints() {
        __pad_to_512b = 0; 
        more_hints = 0; 
        more_hashes = 0; 
        hash2 = 0; 
        hint2 = 0; 
        hint1 = 0; 
        hash1 = 0; 
    }

    void clear_key() { 
        sport = 0; 
        dport = 0; 
        src = 0;
        dst = 0;
        proto = 0;
        vpc_id = 0;
        entry_valid = 0; 
    }

    void clear_data() {
        flow_role = 0;
        session_index = 0;
        epoch = 0;
    }

    void clear_key_data() {
        clear_key();
        clear_data();
    }

    void clear() {
        clear_key();
        clear_data();
        clear_hints();
    }

    void copy_key(ftlv4_entry_t *s) {
        sport = s->sport; 
        dport = s->dport; 
        src = s->src;
        dst = s->dst;
        proto = s->proto;
        vpc_id = s->vpc_id;
    }

    void copy_data(ftlv4_entry_t *s) {
        flow_role = s->flow_role;
        session_index = s->session_index;
        epoch = s->epoch;
    }

    void copy_key_data(ftlv4_entry_t *s) {
        copy_key(s);
        copy_data(s);
    }

    void build_key(ftlv4_entry_t *s) {
        copy_key(s);
        clear_data();
        clear_hints();
        entry_valid = 0;
    }
    
    bool compare_key(ftlv4_entry_t *s) {
        if (s->sport != sport) return false;
        if (s->dport != dport) return false;
        if (s->proto != proto) return false;
        if (s->vpc_id != vpc_id) return false;
        if (s->src != src) return false;
        if (s->dst != dst) return false;
        return true;
    }

    void set_hint_hash(uint32_t slot, uint32_t hint, uint32_t hash) {
        assert(slot);
        switch (slot) { 
        case 1: hint1 = hint; hash1 = hash; break;
        case 2: hint2 = hint; hash2 = hash; break;
        default: more_hashes = 1; more_hints = hint; break;
        } 
    }

    void get_hint_hash(uint32_t slot, uint32_t &hint, uint16_t &hash) {
        assert(slot);
        switch (slot) { 
        case 1: hint = hint1; hash = hash1; break;
        case 2: hint = hint2; hash = hash2; break;
        default: hint = more_hints; hash = more_hashes; break;
        } 
    }

    void get_hint(uint32_t slot, uint32_t &hint) {
        assert(slot);
        switch (slot) { 
        case 1: hint = hint1; break;
        case 2: hint = hint2; break;
        default: hint = more_hints; break;
        } 
    }

    uint32_t get_num_hints() {
        return FTLV4_ARTEMIS_ENTRY_NUM_HINTS;
    }

    uint32_t get_more_hint_slot() {
        return get_num_hints() + 1;
    }

    bool is_hint_slot_valid(uint32_t slot) {
        if (slot && slot <= get_more_hint_slot()) {
            return true;
        }
        return false;
    }

    uint32_t find_last_hint() {
        if (more_hints && more_hashes) {
            return get_more_hint_slot();
        } else if (hint2) {
            return 2;
        } else if (hint1) {
            return 1;
        }
        return 0;
    }

    uint32_t get_session_index() {
        return session_index;
    } 

    void set_session_index(uint32_t session) {
        session_index = session;
    }
 
    void set_key(uint32_t sip,
                 uint32_t dip,
                 uint8_t ip_proto,
                 uint16_t src_port,
                 uint16_t dst_port,
                 uint16_t lookup_id)
    {
        src = sip;
        dst = dip;
        proto = ip_proto;
        sport = src_port;
        dport = dst_port;
        vpc_id = lookup_id;
    }

    void set_epoch(uint8_t val)
    {
        epoch = val;
    }

#endif

};
