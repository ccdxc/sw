#!/usr/bin/python
#
# Capri-Non-Compiler-Compiler (capri-ncc)
# Parag Bhide (Pensando Systems)

capri_model = {
    'name': 'Capri-Asic',
    'phv': {
        'max_size' : 4096,
        'num_flits' : 10,   # change containers as well (below)
        'max_hw_flits' : 12,    # max allowed value for num_flits
        'flit_size' : 512, # max_size/num_flits
        'containers': {8: 640}, # {size:num} all 8 bit containers
        'wide_key_start_flit' : 2
    },
    'match_action': {
        'num_stages' : 6,
        'num_tables' : 8,
        'max_ki_size' : 512,
        'max_kd_size' : 512,
        'num_key_makers' : 4,
        'key_maker_width' : 256,
        'num_bit_extractors' : 16, # per key_maker
        'num_predicate_bits' : 8, # per stage
        'num_table_profiles' : 16,    # tcam table to select table profile
        'num_km_profiles' : 8,
        'num_predicate_bits' : 8,   # max bit used for predicate tcam key
        'range_predicate_bit' : 512,  # predicate bits must be placed in first N (512) bits
        'action_id_size' : 8,       # XXX can become 8bits. Branch (relative) addr of action routine
        'max_te_cycles' : 192,      # divide them among 16 profiles as needed
        'raw_pc_size' : 28, # size of mpu_pc provided to raw table
        'collision_index_sz' : 32,
        'te_consts' : {
            # hardware constants
            'no_load_byte' : '0x3FF',
            'no_load_bit' : '0x1FFF',
            # lkup types
            'no_op'             : '0',
            'mpu_only'          : '1', 
            'hash_only'         : '2', 
            'tcam_only'         : '3', 
            'tcam_sram'         : '4', 
            'hash_otcam_sram'   : '5',
            'direct'            : '6', 
            'rsvd'              : '7', 
            # hash_type
            'num_hash_types'    : 4,    # 0-3 CRC hash, 4= toeplitz hash
        }
    },
    'parser': {
        'num_init_profiles' : 16,
        'num_states' : 256,
        'lkp_regs' : [16, 16, 16],
        'num_extracts' : 16,
        'num_ohi' : 64,
        'num_ohi_per_state' : 4,
        'ohi_threshold' : 32,   # artificial limit to test and also to reserve some ohis for crc..
        'parser_num_flits' : 5,
        'max_lkp_offset' : 64,      # bytes
        'max_extract' : 64,         # bytes
        'max_offset_adv' : 256,     # bytes XXX: this restriction is not removed XXX
        'flit_reserve' : 128,       # reserve 128bit in case some field crosses 512b boundary
                                    # capasm does not allow it
        'max_hv_bits' : 128,
        'hv_location' : 384,        # [511:384]
        'phv_pkt_len_location' : 384,
        'phv_pkt_trunc_location' : 385,
        'hv_pkt_len_location' : 127,
        'hv_pkt_trunc_location' : 126,
        #'flit_hv_bits' : [40, 40, 24, 16, 4, 4], # total must be 128
        # hv bit logic in hw has changed, so reserving # bits per flit is not needed anymore
        # XXX remove this logic from add_cfield()
        'flit_hv_bits' : [32, 32, 32, 24, 4, 4], # total must be 128
        'flit_size' : 1024,
        'parser_consts' : {
            'meta_ops' : {
                'nop' : '0',
                'set_hv' : '1',
                'or_mux_data' : '2',
                'set_mux_data' : '3',
                'or_val' : '4',
                'set_val' : '5'
            },
        },
        'max_csum_engines' : 4
    },
    'deparser': {
        'max_hdr_flds' : 256,
        'dpa_src_cfg' : 0,
        'dpa_src_fixed' : 0,
        'dpa_src_phv' : 1,
        'dpa_src_ohi' : 2,
        'dpa_src_pkt' : 3,
        'len_phv_start' : 512,
        'max_csum_engines' : 4,
        'dpa_start_hvb_in_phv' : 511
    },
}
