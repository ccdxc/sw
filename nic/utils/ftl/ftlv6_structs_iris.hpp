//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#define FTLV6_IRIS_ENTRY_NUM_HINTS 5

struct __attribute__((__packed__)) ftlv6_key_entry_t {
    uint64_t flow_lkp_metadata_lkp_inst : 1;
    uint64_t flow_lkp_metadata_lkp_dir : 1;
    uint64_t flow_lkp_metadata_lkp_type : 4;
    uint64_t flow_lkp_metadata_lkp_sport : 16;
    uint64_t flow_lkp_metadata_lkp_dport : 16;
    uint8_t flow_lkp_metadata_lkp_src[16];
    uint8_t flow_lkp_metadata_lkp_dst[16];
    uint64_t flow_lkp_metadata_lkp_vrf : 16;
    uint64_t flow_lkp_metadata_lkp_proto : 8;
};

struct __attribute__((__packed__)) ftlv6_entry_t {
    uint64_t __pad_to_512b : 1;
    uint64_t more_hints : 18;
    uint64_t more_hashes : 1;
    uint64_t hint5 : 18;
    uint64_t hash5 : 11;
    uint64_t hint4 : 18;
    uint64_t hash4 : 11;
    uint64_t hint3 : 18;
    uint64_t hash3 : 11;
    uint64_t hint2 : 18;
    uint64_t hash2 : 11;
    uint64_t hint1 : 18;
    uint64_t flow_lkp_metadata_lkp_inst : 1;
    uint64_t flow_lkp_metadata_lkp_dir : 1;
    uint64_t flow_lkp_metadata_lkp_type : 4;
    uint64_t flow_lkp_metadata_lkp_sport : 16;
    uint64_t flow_lkp_metadata_lkp_dport : 16;
    uint8_t flow_lkp_metadata_lkp_src[16];
    uint8_t flow_lkp_metadata_lkp_dst[16];
    uint64_t flow_lkp_metadata_lkp_vrf : 16;
    uint64_t flow_lkp_metadata_lkp_proto : 8;
    uint64_t hash1 : 11;
    uint64_t pad4 : 4;
    uint64_t flow_index : 20;
    uint64_t export_en : 4;
    uint64_t entry_valid : 1;

#ifdef __cplusplus

public:
    void copy_key(ftlv6_entry_t *s) {
        flow_lkp_metadata_lkp_inst = s->flow_lkp_metadata_lkp_inst;
        flow_lkp_metadata_lkp_dir = s->flow_lkp_metadata_lkp_dir;
        flow_lkp_metadata_lkp_type = s->flow_lkp_metadata_lkp_type;
        flow_lkp_metadata_lkp_sport = s->flow_lkp_metadata_lkp_sport;
        flow_lkp_metadata_lkp_dport = s->flow_lkp_metadata_lkp_dport;
        memcpy(flow_lkp_metadata_lkp_src, s->flow_lkp_metadata_lkp_src, 16);
        memcpy(flow_lkp_metadata_lkp_dst, s->flow_lkp_metadata_lkp_dst, 16);
        flow_lkp_metadata_lkp_vrf = s->flow_lkp_metadata_lkp_vrf;
        flow_lkp_metadata_lkp_proto = s->flow_lkp_metadata_lkp_proto;
    }

    void copy_data(ftlv6_entry_t *s) {
        flow_index = s->flow_index;
        export_en = s->export_en;
        //entry_valid = s->entry_valid;
    }

    void copy_key_data(ftlv6_entry_t *s) {
        copy_key(s);
        copy_data(s);
    }

    void build_key(ftlv6_entry_t *s) {
        copy_key(s);
        clear_data();
        clear_hints();
        entry_valid = 0;
    }

    void clear_key() {
        flow_lkp_metadata_lkp_inst = 0;
        flow_lkp_metadata_lkp_dir = 0;
        flow_lkp_metadata_lkp_type = 0;
        flow_lkp_metadata_lkp_sport = 0;
        flow_lkp_metadata_lkp_dport = 0;
        memset(flow_lkp_metadata_lkp_src, 0, 16);
        memset(flow_lkp_metadata_lkp_dst, 0, 16);
        flow_lkp_metadata_lkp_vrf = 0;
        flow_lkp_metadata_lkp_proto = 0;
    }

    void clear_data(void) {
        flow_index = 0;
        entry_valid = 0;
        export_en = 0;
    }

    void clear_key_data(void) {
        clear_key();
        clear_data();
    }

    void clear_hints(void) {
        __pad_to_512b = 0;
        pad4 = 0;
        more_hints = 0;
        more_hashes = 0;
        hint5 = 0;
        hash5 = 0;
        hint4 = 0;
        hash4 = 0;
        hint3 = 0;
        hash3 = 0;
        hint2 = 0;
        hash2 = 0;
        hint1 = 0;
        hash1 = 0;
    }

    void clear(void) {
        clear_key();
        clear_data();
        clear_hints();
    }

    bool compare_key(ftlv6_entry_t *s) {
        if (flow_lkp_metadata_lkp_inst != s->flow_lkp_metadata_lkp_inst) return false;
        if (flow_lkp_metadata_lkp_dir != s->flow_lkp_metadata_lkp_dir) return false;
        if (flow_lkp_metadata_lkp_type != s->flow_lkp_metadata_lkp_type) return false;
        if (flow_lkp_metadata_lkp_sport != s->flow_lkp_metadata_lkp_sport) return false;
        if (flow_lkp_metadata_lkp_dport != s->flow_lkp_metadata_lkp_dport) return false;
        if (memcmp(flow_lkp_metadata_lkp_src, s->flow_lkp_metadata_lkp_src, 16)) return false;
        if (memcmp(flow_lkp_metadata_lkp_dst, s->flow_lkp_metadata_lkp_dst, 16)) return false;
        if (flow_lkp_metadata_lkp_vrf != s->flow_lkp_metadata_lkp_vrf) return false;
        if (flow_lkp_metadata_lkp_proto != s->flow_lkp_metadata_lkp_proto) return false;
        return true;
    }

    void set_hint_hash(uint32_t slot, uint32_t hint, uint32_t hash) {
        assert(slot);
        switch(slot) {
            case 1:
                hash1 = hash;
                hint1 = hint;
                break;
            case 2:
                hash2 = hash;
                hint2 = hint;
                break;
            case 3:
                hash3 = hash;
                hint3 = hint;
                break;
            case 4:
                hash4 = hash;
                hint4 = hint;
                break;
            case 5:
                hash5 = hash;
                hint5 = hint;
                break;
            default: more_hints = hint; more_hashes = hash; break;
        }
    }

    void get_hint_hash(uint32_t slot, uint32_t &hint, uint16_t &hash) {
        assert(slot);
        switch(slot) {
            case 1:
                hash = hash1;
                hint = hint1;
                break;
            case 2:
                hash = hash2;
                hint = hint2;
                break;
            case 3:
                hash = hash3;
                hint = hint3;
                break;
            case 4:
                hash = hash4;
                hint = hint4;
                break;
            case 5:
                hash = hash5;
                hint = hint5;
                break;
            default: hint = more_hints; hash = more_hashes; break;
        }
    }

   void get_hint(uint32_t slot, uint32_t &hint) {
        assert(slot);
        switch(slot) {
            case 1:
                hint = hint1;
                break;
            case 2:
                hint = hint2;
                break;
            case 3:
                hint = hint3;
                break;
            case 4:
                hint = hint4;
                break;
            case 5:
                hint = hint5;
                break;
            default: hint = more_hints; break;
        }
    }

    uint32_t get_num_hints(void) {
        return 5;
    }

    uint32_t get_more_hint_slot(void) {
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
        } else if (hint5) {
            return 5;
        } else if (hint4) {
            return 4;
        } else if (hint3) {
            return 3;
        } else if (hint2) {
            return 2;
        } else if (hint1) {
            return 1;
        }
        return 0;
    }

    int data2str(char *buff, uint32_t len) {
        return snprintf(buff, len, "data: "
                 "entry_valid: %lu, export_en: %lu, flow_index: %lu, pad4: %lu, hash1: %lu, hint1: %lu, hash2: %lu, hint2: %lu, hash3: %lu, hint3: %lu, hash4: %lu, hint4: %lu, hash5: %lu, hint5: %lu, more_hashes: %lu, more_hints: %lu",
                 entry_valid, export_en, flow_index, pad4, hash1, hint1, hash2, hint2, hash3, hint3, hash4, hint4, hash5, hint5, more_hashes, more_hints);
    }

    int key2str(char *buff, uint32_t len) {
        return snprintf(buff, len, "key: "
                 "flow_lkp_metadata_lkp_proto: %lu, flow_lkp_metadata_lkp_vrf: %lu, flow_lkp_metadata_lkp_dst: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x, flow_lkp_metadata_lkp_src: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x, flow_lkp_metadata_lkp_dport: %lu, flow_lkp_metadata_lkp_sport: %lu, flow_lkp_metadata_lkp_type: %lu, flow_lkp_metadata_lkp_dir: %lu, flow_lkp_metadata_lkp_inst: %lu",
                 flow_lkp_metadata_lkp_proto, flow_lkp_metadata_lkp_vrf, flow_lkp_metadata_lkp_dst[0], flow_lkp_metadata_lkp_dst[1], flow_lkp_metadata_lkp_dst[2], flow_lkp_metadata_lkp_dst[3], flow_lkp_metadata_lkp_dst[4], flow_lkp_metadata_lkp_dst[5], flow_lkp_metadata_lkp_dst[6], flow_lkp_metadata_lkp_dst[7], flow_lkp_metadata_lkp_dst[8], flow_lkp_metadata_lkp_dst[9], flow_lkp_metadata_lkp_dst[10], flow_lkp_metadata_lkp_dst[11], flow_lkp_metadata_lkp_dst[12], flow_lkp_metadata_lkp_dst[13], flow_lkp_metadata_lkp_dst[14], flow_lkp_metadata_lkp_dst[15], flow_lkp_metadata_lkp_src[0], flow_lkp_metadata_lkp_src[1], flow_lkp_metadata_lkp_src[2], flow_lkp_metadata_lkp_src[3], flow_lkp_metadata_lkp_src[4], flow_lkp_metadata_lkp_src[5], flow_lkp_metadata_lkp_src[6], flow_lkp_metadata_lkp_src[7], flow_lkp_metadata_lkp_src[8], flow_lkp_metadata_lkp_src[9], flow_lkp_metadata_lkp_src[10], flow_lkp_metadata_lkp_src[11], flow_lkp_metadata_lkp_src[12], flow_lkp_metadata_lkp_src[13], flow_lkp_metadata_lkp_src[14], flow_lkp_metadata_lkp_src[15], flow_lkp_metadata_lkp_dport, flow_lkp_metadata_lkp_sport, flow_lkp_metadata_lkp_type, flow_lkp_metadata_lkp_dir, flow_lkp_metadata_lkp_inst);
    }

    int tostr(char *buff, uint32_t len) {
        int offset = key2str(buff, len);

        // delimiter b/w key and data
        snprintf(buff + offset, len - offset, ", ");
        return data2str(buff + offset + 2, len - offset - 2);
    }
    void set_export_en(uint8_t _export_en) {
        export_en = _export_en;
    }

    void set_flow_index(uint32_t _flow_index) {
        flow_index = _flow_index;
    }

    void set_flow_lkp_metadata_lkp_proto(uint8_t _flow_lkp_metadata_lkp_proto) {
        flow_lkp_metadata_lkp_proto = _flow_lkp_metadata_lkp_proto;
    }

    void set_flow_lkp_metadata_lkp_vrf(uint16_t _flow_lkp_metadata_lkp_vrf) {
        flow_lkp_metadata_lkp_vrf = _flow_lkp_metadata_lkp_vrf;
    }

    void set_flow_lkp_metadata_lkp_dst(uint8_t * _flow_lkp_metadata_lkp_dst) {
        memcpy(flow_lkp_metadata_lkp_dst, _flow_lkp_metadata_lkp_dst, 16);
    }

    void set_flow_lkp_metadata_lkp_src(uint8_t * _flow_lkp_metadata_lkp_src) {
        memcpy(flow_lkp_metadata_lkp_src, _flow_lkp_metadata_lkp_src, 16);
    }

    void set_flow_lkp_metadata_lkp_dport(uint16_t _flow_lkp_metadata_lkp_dport) {
        flow_lkp_metadata_lkp_dport = _flow_lkp_metadata_lkp_dport;
    }

    void set_flow_lkp_metadata_lkp_sport(uint16_t _flow_lkp_metadata_lkp_sport) {
        flow_lkp_metadata_lkp_sport = _flow_lkp_metadata_lkp_sport;
    }

    void set_flow_lkp_metadata_lkp_type(uint8_t _flow_lkp_metadata_lkp_type) {
        flow_lkp_metadata_lkp_type = _flow_lkp_metadata_lkp_type;
    }

    void set_flow_lkp_metadata_lkp_dir(uint8_t _flow_lkp_metadata_lkp_dir) {
        flow_lkp_metadata_lkp_dir = _flow_lkp_metadata_lkp_dir;
    }

    void set_flow_lkp_metadata_lkp_inst(uint8_t _flow_lkp_metadata_lkp_inst) {
        flow_lkp_metadata_lkp_inst = _flow_lkp_metadata_lkp_inst;
    }

    uint8_t get_export_en(void) {
        return export_en;
    }

    uint32_t get_flow_index(void) {
        return flow_index;
    }

    uint8_t get_flow_lkp_metadata_lkp_proto(void) {
        return flow_lkp_metadata_lkp_proto;
    }

    uint16_t get_flow_lkp_metadata_lkp_vrf(void) {
        return flow_lkp_metadata_lkp_vrf;
    }

    void get_flow_lkp_metadata_lkp_dst(uint8_t *_flow_lkp_metadata_lkp_dst) {
        memcpy(_flow_lkp_metadata_lkp_dst, flow_lkp_metadata_lkp_dst, 16);
        return;
    }

    void get_flow_lkp_metadata_lkp_src(uint8_t *_flow_lkp_metadata_lkp_src) {
        memcpy(_flow_lkp_metadata_lkp_src, flow_lkp_metadata_lkp_src, 16);
        return;
    }

    uint16_t get_flow_lkp_metadata_lkp_dport(void) {
        return flow_lkp_metadata_lkp_dport;
    }

    uint16_t get_flow_lkp_metadata_lkp_sport(void) {
        return flow_lkp_metadata_lkp_sport;
    }

    uint8_t get_flow_lkp_metadata_lkp_type(void) {
        return flow_lkp_metadata_lkp_type;
    }

    uint8_t get_flow_lkp_metadata_lkp_dir(void) {
        return flow_lkp_metadata_lkp_dir;
    }

    uint8_t get_flow_lkp_metadata_lkp_inst(void) {
        return flow_lkp_metadata_lkp_inst;
    }

#endif
};
