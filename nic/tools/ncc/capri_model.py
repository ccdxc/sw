#!/usr/bin/python
#
# Capri-Non-Compiler-Compiler (capri-ncc)
# Parag Bhide (Pensando Systems)

capri_model = {
    'name': 'Capri-Asic',
    'phv': {
        'max_size' : 4096,  # derived from num_flits
        'num_flits' : 8,   # change containers as well (below)
        'max_hw_flits' : 12,    # max allowed value for num_flits
        'flit_size' : 512, # max_size/num_flits
        'containers': {8: 512}, # {size:num} all 8 bit containers - derived from num_flits
        'wide_key_start_flit' : 2,
        'gso_csum_phv_start' : 496, # bits 496:511 i.e. byte 62:63 in flit
        'parser_end_off_flit_loc' : 480,  # bits 480:495 i.e. byte 60:61 in flit
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
            'base_clock_freq'  : (833000000),
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

            #Pic Table Opcodes
            'pic_tbl_opcode_operation_none'  : 0, # define PIC_TBL_OPCODE_OPERATION_NONE    0
            'pic_tbl_opcode_operation_add'   : 1, # define PIC_TBL_OPCODE_OPERATION_ADD     1
            'pic_tbl_opcode_operation_sub'   : 2, # define PIC_TBL_OPCODE_OPERATION_SUB     2
            'pic_tbl_opcode_operation_min'   : 3, # define PIC_TBL_OPCODE_OPERATION_MIN     3
            'pic_tbl_opcode_operation_max'   : 4, # define PIC_TBL_OPCODE_OPERATION_MAX     4
            'pic_tbl_opcode_operation_avg'   : 5, # define PIC_TBL_OPCODE_OPERATION_AVG     5
            'pic_tbl_opcode_operation_cpy'   : 6, # define PIC_TBL_OPCODE_OPERATION_CPY     6
            'pic_tbl_opcode_operation_bitset': 7, # define PIC_TBL_OPCODE_OPERATION_BITSET  7
            'pic_tbl_opcode_operation_clear' : 8, # define PIC_TBL_OPCODE_OPERATION_CLEAR   8

            'pic_tbl_opcode_oprd1_sel_counter': 0, # define PIC_TBL_OPCODE_OPRD1_SEL_COUNTER  0
            'pic_tbl_opcode_oprd1_sel_rate'   : 1, # define PIC_TBL_OPCODE_OPRD1_SEL_RATE     1
            'pic_tbl_opcode_oprd1_sel_tbkt'   : 2, # define PIC_TBL_OPCODE_OPRD1_SEL_TBKT     2

            'pic_tbl_opcode_oprd2_sel_one'    : 0, # define PIC_TBL_OPCODE_OPRD2_SEL_ONE       0
            'pic_tbl_opcode_oprd2_sel_pktsize': 1, # define PIC_TBL_OPCODE_OPRD2_SEL_PKTSIZE   1
            'pic_tbl_opcode_oprd2_sel_rate'   : 2, # define PIC_TBL_OPCODE_OPRD2_SEL_RATE      2
            'pic_tbl_opcode_oprd2_sel_policer': 3, # define PIC_TBL_OPCODE_OPRD2_SEL_POLICER   3

            'pic_tbl_opcode_saturate_none'  : 0,  # define PIC_TBL_OPCODE_SATURATE_NONE    0
            'pic_tbl_opcode_saturate_minmax': 1,  # define PIC_TBL_OPCODE_SATURATE_MINMAX  1
            'pic_tbl_opcode_saturate_oprd3' : 2,  # define PIC_TBL_OPCODE_SATURATE_OPRD3   2
            'pic_tbl_opcode_saturate_neg'   : 3,  # define PIC_TBL_OPCODE_SATURATE_NEG     3
        }
    },
    'parser': {
        'num_init_profiles' : 16,
        'num_states' : 288,
        'lkp_regs' : [16, 16, 16],
        'num_extracts' : 16,
        'num_ohi' : 64,
        'num_ohi_per_state' : 4,
        'ohi_threshold' : 60,   # artificial limit to test and also to reserve some ohis for crc..
        'parser_num_flits' : 4, # derived from num_flits = 2*num_flits
        'max_lkp_offset' : 64,      # bytes
        'max_extract' : 64,         # bytes
        'max_offset_adv' : 256,     # bytes XXX: this restriction is not removed XXX
        'flit_reserve' : 128,       # reserve 128bit in case some field crosses 512b boundary
                                    # capasm does not allow it
        'max_hv_bits' : 128,
        'hv_start_offset' : 4,      # First four hdr valid bits can be used for special
                                    # purpose to rewrite phv in deparser. Use case is GSO
        'rw_phv_hv_start_offset' : 0, #First four hdr valid bits can be used for special
                                      #purpose to rewrite phv in deparser. Use case is GSO
        'max_rw_phv_hv_bits' : 4,     #First four hdr valid bits can be used for special
                                      #purpose to rewrite phv in deparser. Use case is GSO

        'hv_location' : 384,        # [511:384] 384th bit is used to pad packet after inserting payload
        'phv_pad_hdr_location' : 384,
        'phv_pkt_len_location' : 385,
        'phv_pkt_trunc_location' : 386,
        'hv_pad_hdr_location' : 127,
        'hv_pkt_len_location' : 126,
        'hv_pkt_trunc_location' : 125,

        #'flit_hv_bits' : [40, 40, 24, 16, 4, 4], # total must be 128
        # hv bit logic in hw has changed, so reserving # bits per flit is not needed anymore
        # XXX remove this logic from add_cfield()
        'flit_hv_bits' : [32, 32, 32, 24, 4, 4], # total must be 128
        'flit_size' : 1024,
        'num_len_chk_profiles' : 4,
        'num_len_chk_inst' : 2,     # per parser state
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
        'max_csum_engines' : 5
    },
    'deparser': {
        'max_hdr_flds' : 256,
        'dpa_src_cfg' : 0,
        'dpa_src_fixed' : 0,
        'dpa_src_phv' : 1,
        'dpa_src_ohi' : 2,
        'dpa_src_pkt' : 3,
        'pad_phv_start' : 528,  #Has to be 16b slots in first flit phv
        'len_phv_start' : 544,  #Has to be 16b slots in first flit phv
        'max_csum_engines' : 5,
        'dpa_start_hvb_in_phv' : 511,
        'max_crc_hdrs' : 16,
        'max_csum_hdrs' : 24,
        'hdrfld_info_start' : 4 #First four hdr fld can be used for special
                                #purpose to rewrite phv in deparser. Use case
                                #is GSO csum
    },
}
