//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#define FTLV4_APOLLO_ENTRY_NUM_HINTS 2
struct __attribute__((__packed__)) ftlv4_entry_t {
    // data after key
    uint32_t __pad_to_512b : 7;
    uint32_t entry_valid : 1;
    uint32_t nexthop_id : 16;
    uint32_t flow_role : 1;
    uint32_t more_hints : 20;
    uint32_t more_hashes : 1;
    uint32_t hint2 : 20;
    uint32_t hash2 : 9;
    uint32_t hint1 : 20;
    uint32_t hash1 : 9;

    // key
    uint32_t bd_id : 16;
    uint32_t dst;
    uint32_t src;
    uint32_t proto : 8;
    uint32_t sport : 16;
    uint32_t dport : 16;

    // data before key
    uint32_t nexthop_type : 2;
    uint32_t nexthop_valid : 1;
    uint32_t session_id : 21;
    uint32_t epoch : 8;

#ifdef __cplusplus
public:
    void set_nexthop_index(uint32_t index) {
        nexthop_id = index & 0xffff;    // 16 bits
    }

    void tostr(char *buff, uint32_t len) {
        snprintf(buff, len,
                "entry_valid: %u, nexthop_id: %u, "
                "more_hints: %u, ""more_hashes: %u, "
                "hint2: %u, hash2: %u, "
                "hint1: %u, hash1: %u, "
                "bd_id: %u, dst: %u, "
                "src: %u, proto: %u, "
                "sport: %u, dport: %u, "
                "nexthop_type: %u, nexthop_valid: %u, "
                "flow_role: %u, session_id: %u, "
                "epoch: %u\n",
                entry_valid, nexthop_id,
                more_hints, more_hashes,
                hint2, hash2,
                hint1, hash1,
                bd_id, dst,
                src, proto,
                sport, dport,
                nexthop_type, nexthop_valid,
                flow_role, session_id,
                epoch);

    }

    static void keyheader2str(char *buff, uint32_t len) {
        char *cur = buff;
        cur += snprintf(cur, len, "%16s\t%16s\t%5s\t%5s\t%3s\t%4s\t%8s\n",
                        "SrcIP", "DstIP", "SrcPort", "DstPort", "Proto", "BdId", "Session");
        snprintf(cur, buff + len - cur, "%16s\t%16s\t%5s\t%5s\t%3s\t%4s\t%8s\n",
                 "-----", "-----", "-----", "-------", "-------", "-----", "----");
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
                 proto, bd_id, session_id);
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
        ftlv4_entry_t s;
        memset(&s, 0, sizeof(ftlv4_entry_t));
        copy_key(&s);
    }

    void clear_data() {
        ftlv4_entry_t s;
        memset(&s, 0, sizeof(ftlv4_entry_t));
        copy_data(&s);;
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
        bd_id = s->bd_id;
        dst = s->dst;
        src = s->src;
        proto = s->proto;
        sport = s->sport;
        dport = s->dport;
    }

    void copy_data(ftlv4_entry_t *s) {
        nexthop_id = s->nexthop_id;
        nexthop_type = s->nexthop_type;
        nexthop_valid = s->nexthop_valid;
        flow_role = s->flow_role;
        session_id = s->session_id;
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
        if (bd_id != s->bd_id) return false;
        if (dst != s->dst) return false;
        if (src != s->src) return false;
        if (proto != s->proto) return false;
        if (sport != s->sport) return false;
        if (dport != s->dport) return false;
        return true;
    }

    void set_hint_hash(uint32_t slot, uint32_t hint, uint32_t hash) {
        assert(slot);
        switch (slot) {
        case 1: hint1 = hint; hash1 = hash; break;
        case 2: hint2 = hint; hash2 = hash; break;
        default: more_hashes = hash; more_hints = hint; break;
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
        return FTLV4_APOLLO_ENTRY_NUM_HINTS;
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
        return session_id;
    }

    void set_session_index(uint32_t session) {
        session_id = session;
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
        bd_id = lookup_id;
    }

    void set_epoch(uint8_t val)
    {
        epoch = val;
    }

#endif
};
