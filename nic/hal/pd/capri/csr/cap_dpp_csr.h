
#ifndef CAP_DPP_CSR_H
#define CAP_DPP_CSR_H

#include "cap_csr_base.h" 
#include "cap_dppmem_csr.h" 
#include "cap_dppstats_csr.h" 
#include "cap_dppcsum_csr.h" 
#include "cap_dpphdrfld_csr.h" 
#include "cap_dpphdr_csr.h" 

using namespace std;
class cap_dpp_csr_cfg_ohi_payload_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dpp_csr_cfg_ohi_payload_t(string name = "cap_dpp_csr_cfg_ohi_payload_t", cap_csr_base *parent = 0);
        virtual ~cap_dpp_csr_cfg_ohi_payload_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 64 > ohi_slot_payload_ptr_bm_cpp_int_t;
        ohi_slot_payload_ptr_bm_cpp_int_t int_var__ohi_slot_payload_ptr_bm;
        void ohi_slot_payload_ptr_bm (const cpp_int  & l__val);
        cpp_int ohi_slot_payload_ptr_bm() const;
    
}; // cap_dpp_csr_cfg_ohi_payload_t
    
class cap_dpp_csr_cfg_interrupt_mask_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dpp_csr_cfg_interrupt_mask_t(string name = "cap_dpp_csr_cfg_interrupt_mask_t", cap_csr_base *parent = 0);
        virtual ~cap_dpp_csr_cfg_interrupt_mask_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > err_phv_sop_no_eop_cpp_int_t;
        err_phv_sop_no_eop_cpp_int_t int_var__err_phv_sop_no_eop;
        void err_phv_sop_no_eop (const cpp_int  & l__val);
        cpp_int err_phv_sop_no_eop() const;
    
        typedef pu_cpp_int< 1 > err_phv_eop_no_sop_cpp_int_t;
        err_phv_eop_no_sop_cpp_int_t int_var__err_phv_eop_no_sop;
        void err_phv_eop_no_sop (const cpp_int  & l__val);
        cpp_int err_phv_eop_no_sop() const;
    
        typedef pu_cpp_int< 1 > err_ohi_sop_no_eop_cpp_int_t;
        err_ohi_sop_no_eop_cpp_int_t int_var__err_ohi_sop_no_eop;
        void err_ohi_sop_no_eop (const cpp_int  & l__val);
        cpp_int err_ohi_sop_no_eop() const;
    
        typedef pu_cpp_int< 1 > err_ohi_eop_no_sop_cpp_int_t;
        err_ohi_eop_no_sop_cpp_int_t int_var__err_ohi_eop_no_sop;
        void err_ohi_eop_no_sop (const cpp_int  & l__val);
        cpp_int err_ohi_eop_no_sop() const;
    
        typedef pu_cpp_int< 1 > err_framer_credit_overrun_cpp_int_t;
        err_framer_credit_overrun_cpp_int_t int_var__err_framer_credit_overrun;
        void err_framer_credit_overrun (const cpp_int  & l__val);
        cpp_int err_framer_credit_overrun() const;
    
        typedef pu_cpp_int< 1 > err_packets_in_flight_credit_overrun_cpp_int_t;
        err_packets_in_flight_credit_overrun_cpp_int_t int_var__err_packets_in_flight_credit_overrun;
        void err_packets_in_flight_credit_overrun (const cpp_int  & l__val);
        cpp_int err_packets_in_flight_credit_overrun() const;
    
        typedef pu_cpp_int< 1 > err_null_hdr_vld_cpp_int_t;
        err_null_hdr_vld_cpp_int_t int_var__err_null_hdr_vld;
        void err_null_hdr_vld (const cpp_int  & l__val);
        cpp_int err_null_hdr_vld() const;
    
        typedef pu_cpp_int< 1 > err_null_hdrfld_vld_cpp_int_t;
        err_null_hdrfld_vld_cpp_int_t int_var__err_null_hdrfld_vld;
        void err_null_hdrfld_vld (const cpp_int  & l__val);
        cpp_int err_null_hdrfld_vld() const;
    
        typedef pu_cpp_int< 1 > err_max_pkt_size_cpp_int_t;
        err_max_pkt_size_cpp_int_t int_var__err_max_pkt_size;
        void err_max_pkt_size (const cpp_int  & l__val);
        cpp_int err_max_pkt_size() const;
    
        typedef pu_cpp_int< 1 > err_max_active_hdrs_cpp_int_t;
        err_max_active_hdrs_cpp_int_t int_var__err_max_active_hdrs;
        void err_max_active_hdrs (const cpp_int  & l__val);
        cpp_int err_max_active_hdrs() const;
    
        typedef pu_cpp_int< 1 > err_phv_no_data_reference_ohi_cpp_int_t;
        err_phv_no_data_reference_ohi_cpp_int_t int_var__err_phv_no_data_reference_ohi;
        void err_phv_no_data_reference_ohi (const cpp_int  & l__val);
        cpp_int err_phv_no_data_reference_ohi() const;
    
        typedef pu_cpp_int< 1 > err_csum_multiple_hdr_cpp_int_t;
        err_csum_multiple_hdr_cpp_int_t int_var__err_csum_multiple_hdr;
        void err_csum_multiple_hdr (const cpp_int  & l__val);
        cpp_int err_csum_multiple_hdr() const;
    
        typedef pu_cpp_int< 1 > err_crc_multiple_hdr_cpp_int_t;
        err_crc_multiple_hdr_cpp_int_t int_var__err_crc_multiple_hdr;
        void err_crc_multiple_hdr (const cpp_int  & l__val);
        cpp_int err_crc_multiple_hdr() const;
    
        typedef pu_cpp_int< 1 > err_ptr_fifo_credit_overrun_cpp_int_t;
        err_ptr_fifo_credit_overrun_cpp_int_t int_var__err_ptr_fifo_credit_overrun;
        void err_ptr_fifo_credit_overrun (const cpp_int  & l__val);
        cpp_int err_ptr_fifo_credit_overrun() const;
    
        typedef pu_cpp_int< 1 > err_clip_max_pkt_size_cpp_int_t;
        err_clip_max_pkt_size_cpp_int_t int_var__err_clip_max_pkt_size;
        void err_clip_max_pkt_size (const cpp_int  & l__val);
        cpp_int err_clip_max_pkt_size() const;
    
        typedef pu_cpp_int< 32 > rsvd_cpp_int_t;
        rsvd_cpp_int_t int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
}; // cap_dpp_csr_cfg_interrupt_mask_t
    
class cap_dpp_csr_cfg_error_mask_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dpp_csr_cfg_error_mask_t(string name = "cap_dpp_csr_cfg_error_mask_t", cap_csr_base *parent = 0);
        virtual ~cap_dpp_csr_cfg_error_mask_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > err_phv_sop_no_eop_cpp_int_t;
        err_phv_sop_no_eop_cpp_int_t int_var__err_phv_sop_no_eop;
        void err_phv_sop_no_eop (const cpp_int  & l__val);
        cpp_int err_phv_sop_no_eop() const;
    
        typedef pu_cpp_int< 1 > err_phv_eop_no_sop_cpp_int_t;
        err_phv_eop_no_sop_cpp_int_t int_var__err_phv_eop_no_sop;
        void err_phv_eop_no_sop (const cpp_int  & l__val);
        cpp_int err_phv_eop_no_sop() const;
    
        typedef pu_cpp_int< 1 > err_ohi_sop_no_eop_cpp_int_t;
        err_ohi_sop_no_eop_cpp_int_t int_var__err_ohi_sop_no_eop;
        void err_ohi_sop_no_eop (const cpp_int  & l__val);
        cpp_int err_ohi_sop_no_eop() const;
    
        typedef pu_cpp_int< 1 > err_ohi_eop_no_sop_cpp_int_t;
        err_ohi_eop_no_sop_cpp_int_t int_var__err_ohi_eop_no_sop;
        void err_ohi_eop_no_sop (const cpp_int  & l__val);
        cpp_int err_ohi_eop_no_sop() const;
    
        typedef pu_cpp_int< 1 > err_framer_credit_overrun_cpp_int_t;
        err_framer_credit_overrun_cpp_int_t int_var__err_framer_credit_overrun;
        void err_framer_credit_overrun (const cpp_int  & l__val);
        cpp_int err_framer_credit_overrun() const;
    
        typedef pu_cpp_int< 1 > err_packets_in_flight_credit_overrun_cpp_int_t;
        err_packets_in_flight_credit_overrun_cpp_int_t int_var__err_packets_in_flight_credit_overrun;
        void err_packets_in_flight_credit_overrun (const cpp_int  & l__val);
        cpp_int err_packets_in_flight_credit_overrun() const;
    
        typedef pu_cpp_int< 1 > err_null_hdr_vld_cpp_int_t;
        err_null_hdr_vld_cpp_int_t int_var__err_null_hdr_vld;
        void err_null_hdr_vld (const cpp_int  & l__val);
        cpp_int err_null_hdr_vld() const;
    
        typedef pu_cpp_int< 1 > err_null_hdrfld_vld_cpp_int_t;
        err_null_hdrfld_vld_cpp_int_t int_var__err_null_hdrfld_vld;
        void err_null_hdrfld_vld (const cpp_int  & l__val);
        cpp_int err_null_hdrfld_vld() const;
    
        typedef pu_cpp_int< 1 > err_max_pkt_size_cpp_int_t;
        err_max_pkt_size_cpp_int_t int_var__err_max_pkt_size;
        void err_max_pkt_size (const cpp_int  & l__val);
        cpp_int err_max_pkt_size() const;
    
        typedef pu_cpp_int< 1 > err_max_active_hdrs_cpp_int_t;
        err_max_active_hdrs_cpp_int_t int_var__err_max_active_hdrs;
        void err_max_active_hdrs (const cpp_int  & l__val);
        cpp_int err_max_active_hdrs() const;
    
        typedef pu_cpp_int< 1 > err_phv_no_data_reference_ohi_cpp_int_t;
        err_phv_no_data_reference_ohi_cpp_int_t int_var__err_phv_no_data_reference_ohi;
        void err_phv_no_data_reference_ohi (const cpp_int  & l__val);
        cpp_int err_phv_no_data_reference_ohi() const;
    
        typedef pu_cpp_int< 1 > err_csum_multiple_hdr_cpp_int_t;
        err_csum_multiple_hdr_cpp_int_t int_var__err_csum_multiple_hdr;
        void err_csum_multiple_hdr (const cpp_int  & l__val);
        cpp_int err_csum_multiple_hdr() const;
    
        typedef pu_cpp_int< 1 > err_crc_multiple_hdr_cpp_int_t;
        err_crc_multiple_hdr_cpp_int_t int_var__err_crc_multiple_hdr;
        void err_crc_multiple_hdr (const cpp_int  & l__val);
        cpp_int err_crc_multiple_hdr() const;
    
        typedef pu_cpp_int< 1 > err_ptr_fifo_credit_overrun_cpp_int_t;
        err_ptr_fifo_credit_overrun_cpp_int_t int_var__err_ptr_fifo_credit_overrun;
        void err_ptr_fifo_credit_overrun (const cpp_int  & l__val);
        cpp_int err_ptr_fifo_credit_overrun() const;
    
        typedef pu_cpp_int< 1 > err_clip_max_pkt_size_cpp_int_t;
        err_clip_max_pkt_size_cpp_int_t int_var__err_clip_max_pkt_size;
        void err_clip_max_pkt_size (const cpp_int  & l__val);
        cpp_int err_clip_max_pkt_size() const;
    
        typedef pu_cpp_int< 32 > rsvd_cpp_int_t;
        rsvd_cpp_int_t int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
}; // cap_dpp_csr_cfg_error_mask_t
    
class cap_dpp_csr_cfg_global_2_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dpp_csr_cfg_global_2_t(string name = "cap_dpp_csr_cfg_global_2_t", cap_csr_base *parent = 0);
        virtual ~cap_dpp_csr_cfg_global_2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > dump_dop_no_data_phv_cpp_int_t;
        dump_dop_no_data_phv_cpp_int_t int_var__dump_dop_no_data_phv;
        void dump_dop_no_data_phv (const cpp_int  & l__val);
        cpp_int dump_dop_no_data_phv() const;
    
        typedef pu_cpp_int< 31 > rsvd_cpp_int_t;
        rsvd_cpp_int_t int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
}; // cap_dpp_csr_cfg_global_2_t
    
class cap_dpp_csr_cfg_global_1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dpp_csr_cfg_global_1_t(string name = "cap_dpp_csr_cfg_global_1_t", cap_csr_base *parent = 0);
        virtual ~cap_dpp_csr_cfg_global_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 5 > ptr_fifo_fc_thr_cpp_int_t;
        ptr_fifo_fc_thr_cpp_int_t int_var__ptr_fifo_fc_thr;
        void ptr_fifo_fc_thr (const cpp_int  & l__val);
        cpp_int ptr_fifo_fc_thr() const;
    
        typedef pu_cpp_int< 10 > phv_fifo_almost_full_threshold_cpp_int_t;
        phv_fifo_almost_full_threshold_cpp_int_t int_var__phv_fifo_almost_full_threshold;
        void phv_fifo_almost_full_threshold (const cpp_int  & l__val);
        cpp_int phv_fifo_almost_full_threshold() const;
    
        typedef pu_cpp_int< 10 > ohi_fifo_almost_full_threshold_cpp_int_t;
        ohi_fifo_almost_full_threshold_cpp_int_t int_var__ohi_fifo_almost_full_threshold;
        void ohi_fifo_almost_full_threshold (const cpp_int  & l__val);
        cpp_int ohi_fifo_almost_full_threshold() const;
    
        typedef pu_cpp_int< 16 > max_hdrfld_size_cpp_int_t;
        max_hdrfld_size_cpp_int_t int_var__max_hdrfld_size;
        void max_hdrfld_size (const cpp_int  & l__val);
        cpp_int max_hdrfld_size() const;
    
        typedef pu_cpp_int< 16 > max_pkt_size_cpp_int_t;
        max_pkt_size_cpp_int_t int_var__max_pkt_size;
        void max_pkt_size (const cpp_int  & l__val);
        cpp_int max_pkt_size() const;
    
        typedef pu_cpp_int< 1 > pkt_truncation_en_cpp_int_t;
        pkt_truncation_en_cpp_int_t int_var__pkt_truncation_en;
        void pkt_truncation_en (const cpp_int  & l__val);
        cpp_int pkt_truncation_en() const;
    
        typedef pu_cpp_int< 16 > ignore_hdrfld_size_0_cpp_int_t;
        ignore_hdrfld_size_0_cpp_int_t int_var__ignore_hdrfld_size_0;
        void ignore_hdrfld_size_0 (const cpp_int  & l__val);
        cpp_int ignore_hdrfld_size_0() const;
    
        typedef pu_cpp_int< 1 > bypass_mode_cpp_int_t;
        bypass_mode_cpp_int_t int_var__bypass_mode;
        void bypass_mode (const cpp_int  & l__val);
        cpp_int bypass_mode() const;
    
        typedef pu_cpp_int< 1 > phv_drop_bypass_en_cpp_int_t;
        phv_drop_bypass_en_cpp_int_t int_var__phv_drop_bypass_en;
        void phv_drop_bypass_en (const cpp_int  & l__val);
        cpp_int phv_drop_bypass_en() const;
    
}; // cap_dpp_csr_cfg_global_1_t
    
class cap_dpp_csr_cfg_global_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dpp_csr_cfg_global_t(string name = "cap_dpp_csr_cfg_global_t", cap_csr_base *parent = 0);
        virtual ~cap_dpp_csr_cfg_global_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 7 > hdr_vld_phv_mask_cpp_int_t;
        hdr_vld_phv_mask_cpp_int_t int_var__hdr_vld_phv_mask;
        void hdr_vld_phv_mask (const cpp_int  & l__val);
        cpp_int hdr_vld_phv_mask() const;
    
        typedef pu_cpp_int< 4 > input_pacer_timer_cpp_int_t;
        input_pacer_timer_cpp_int_t int_var__input_pacer_timer;
        void input_pacer_timer (const cpp_int  & l__val);
        cpp_int input_pacer_timer() const;
    
        typedef pu_cpp_int< 4 > input_pacer_release_cpp_int_t;
        input_pacer_release_cpp_int_t int_var__input_pacer_release;
        void input_pacer_release (const cpp_int  & l__val);
        cpp_int input_pacer_release() const;
    
        typedef pu_cpp_int< 3 > packet_in_flight_credit_cpp_int_t;
        packet_in_flight_credit_cpp_int_t int_var__packet_in_flight_credit;
        void packet_in_flight_credit (const cpp_int  & l__val);
        cpp_int packet_in_flight_credit() const;
    
        typedef pu_cpp_int< 1 > packet_in_flight_credit_en_cpp_int_t;
        packet_in_flight_credit_en_cpp_int_t int_var__packet_in_flight_credit_en;
        void packet_in_flight_credit_en (const cpp_int  & l__val);
        cpp_int packet_in_flight_credit_en() const;
    
        typedef pu_cpp_int< 2 > phv_framer_credit_cpp_int_t;
        phv_framer_credit_cpp_int_t int_var__phv_framer_credit;
        void phv_framer_credit (const cpp_int  & l__val);
        cpp_int phv_framer_credit() const;
    
        typedef pu_cpp_int< 1 > phv_framer_credit_en_cpp_int_t;
        phv_framer_credit_en_cpp_int_t int_var__phv_framer_credit_en;
        void phv_framer_credit_en (const cpp_int  & l__val);
        cpp_int phv_framer_credit_en() const;
    
        typedef pu_cpp_int< 6 > framer_ptr_fifo_credit_cpp_int_t;
        framer_ptr_fifo_credit_cpp_int_t int_var__framer_ptr_fifo_credit;
        void framer_ptr_fifo_credit (const cpp_int  & l__val);
        cpp_int framer_ptr_fifo_credit() const;
    
        typedef pu_cpp_int< 1 > framer_ptr_fifo_credit_en_cpp_int_t;
        framer_ptr_fifo_credit_en_cpp_int_t int_var__framer_ptr_fifo_credit_en;
        void framer_ptr_fifo_credit_en (const cpp_int  & l__val);
        cpp_int framer_ptr_fifo_credit_en() const;
    
}; // cap_dpp_csr_cfg_global_t
    
class cap_dpp_csr_base_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dpp_csr_base_t(string name = "cap_dpp_csr_base_t", cap_csr_base *parent = 0);
        virtual ~cap_dpp_csr_base_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > scratch_reg_cpp_int_t;
        scratch_reg_cpp_int_t int_var__scratch_reg;
        void scratch_reg (const cpp_int  & l__val);
        cpp_int scratch_reg() const;
    
}; // cap_dpp_csr_base_t
    
class cap_dpp_csr_t : public cap_block_base {
    public:
        cpp_int_helper hlp;
    
        cap_dpp_csr_t(string name = "cap_dpp_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_dpp_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_dpp_csr_base_t base;
    
        cap_dpp_csr_cfg_global_t cfg_global;
    
        cap_dpp_csr_cfg_global_1_t cfg_global_1;
    
        cap_dpp_csr_cfg_global_2_t cfg_global_2;
    
        cap_dpp_csr_cfg_error_mask_t cfg_error_mask;
    
        cap_dpp_csr_cfg_interrupt_mask_t cfg_interrupt_mask;
    
        cap_dpp_csr_cfg_ohi_payload_t cfg_ohi_payload;
    
        cap_dpphdr_csr_t hdr;
    
        cap_dpphdrfld_csr_t hdrfld;
    
        cap_dppcsum_csr_t csum;
    
        cap_dppstats_csr_t stats;
    
        cap_dppmem_csr_t mem;
    
}; // cap_dpp_csr_t
    
#endif // CAP_DPP_CSR_H
        