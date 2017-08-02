
#ifndef CAP_DPR_CSR_H
#define CAP_DPR_CSR_H

#include "cap_csr_base.h" 
#include "cap_dprmem_csr.h" 
#include "cap_dprstats_csr.h" 
#include "cap_dprcfg_csr.h" 
#include "cap_dprhdrfld_csr.h" 

using namespace std;
class cap_dpr_csr_cfg_interrupt_mask_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dpr_csr_cfg_interrupt_mask_t(string name = "cap_dpr_csr_cfg_interrupt_mask_t", cap_csr_base *parent = 0);
        virtual ~cap_dpr_csr_cfg_interrupt_mask_t();
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
    
        typedef pu_cpp_int< 1 > err_pktin_sop_no_eop_cpp_int_t;
        err_pktin_sop_no_eop_cpp_int_t int_var__err_pktin_sop_no_eop;
        void err_pktin_sop_no_eop (const cpp_int  & l__val);
        cpp_int err_pktin_sop_no_eop() const;
    
        typedef pu_cpp_int< 1 > err_pktin_eop_no_sop_cpp_int_t;
        err_pktin_eop_no_sop_cpp_int_t int_var__err_pktin_eop_no_sop;
        void err_pktin_eop_no_sop (const cpp_int  & l__val);
        cpp_int err_pktin_eop_no_sop() const;
    
        typedef pu_cpp_int< 1 > err_csum_offset_gt_pkt_size_cpp_int_t;
        err_csum_offset_gt_pkt_size_cpp_int_t int_var__err_csum_offset_gt_pkt_size;
        void err_csum_offset_gt_pkt_size (const cpp_int  & l__val);
        cpp_int err_csum_offset_gt_pkt_size() const;
    
        typedef pu_cpp_int< 1 > err_csum_loc_gt_pkt_size_cpp_int_t;
        err_csum_loc_gt_pkt_size_cpp_int_t int_var__err_csum_loc_gt_pkt_size;
        void err_csum_loc_gt_pkt_size (const cpp_int  & l__val);
        cpp_int err_csum_loc_gt_pkt_size() const;
    
        typedef pu_cpp_int< 1 > err_crc_offset_gt_pkt_size_cpp_int_t;
        err_crc_offset_gt_pkt_size_cpp_int_t int_var__err_crc_offset_gt_pkt_size;
        void err_crc_offset_gt_pkt_size (const cpp_int  & l__val);
        cpp_int err_crc_offset_gt_pkt_size() const;
    
        typedef pu_cpp_int< 1 > err_crc_loc_gt_pkt_size_cpp_int_t;
        err_crc_loc_gt_pkt_size_cpp_int_t int_var__err_crc_loc_gt_pkt_size;
        void err_crc_loc_gt_pkt_size (const cpp_int  & l__val);
        cpp_int err_crc_loc_gt_pkt_size() const;
    
        typedef pu_cpp_int< 1 > err_pkt_eop_early_cpp_int_t;
        err_pkt_eop_early_cpp_int_t int_var__err_pkt_eop_early;
        void err_pkt_eop_early (const cpp_int  & l__val);
        cpp_int err_pkt_eop_early() const;
    
        typedef pu_cpp_int< 32 > rsvd_cpp_int_t;
        rsvd_cpp_int_t int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
}; // cap_dpr_csr_cfg_interrupt_mask_t
    
class cap_dpr_csr_cfg_error_mask_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dpr_csr_cfg_error_mask_t(string name = "cap_dpr_csr_cfg_error_mask_t", cap_csr_base *parent = 0);
        virtual ~cap_dpr_csr_cfg_error_mask_t();
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
    
        typedef pu_cpp_int< 1 > err_pktin_sop_no_eop_cpp_int_t;
        err_pktin_sop_no_eop_cpp_int_t int_var__err_pktin_sop_no_eop;
        void err_pktin_sop_no_eop (const cpp_int  & l__val);
        cpp_int err_pktin_sop_no_eop() const;
    
        typedef pu_cpp_int< 1 > err_pktin_eop_no_sop_cpp_int_t;
        err_pktin_eop_no_sop_cpp_int_t int_var__err_pktin_eop_no_sop;
        void err_pktin_eop_no_sop (const cpp_int  & l__val);
        cpp_int err_pktin_eop_no_sop() const;
    
        typedef pu_cpp_int< 1 > err_csum_offset_gt_pkt_size_cpp_int_t;
        err_csum_offset_gt_pkt_size_cpp_int_t int_var__err_csum_offset_gt_pkt_size;
        void err_csum_offset_gt_pkt_size (const cpp_int  & l__val);
        cpp_int err_csum_offset_gt_pkt_size() const;
    
        typedef pu_cpp_int< 1 > err_csum_loc_gt_pkt_size_cpp_int_t;
        err_csum_loc_gt_pkt_size_cpp_int_t int_var__err_csum_loc_gt_pkt_size;
        void err_csum_loc_gt_pkt_size (const cpp_int  & l__val);
        cpp_int err_csum_loc_gt_pkt_size() const;
    
        typedef pu_cpp_int< 1 > err_crc_offset_gt_pkt_size_cpp_int_t;
        err_crc_offset_gt_pkt_size_cpp_int_t int_var__err_crc_offset_gt_pkt_size;
        void err_crc_offset_gt_pkt_size (const cpp_int  & l__val);
        cpp_int err_crc_offset_gt_pkt_size() const;
    
        typedef pu_cpp_int< 1 > err_crc_loc_gt_pkt_size_cpp_int_t;
        err_crc_loc_gt_pkt_size_cpp_int_t int_var__err_crc_loc_gt_pkt_size;
        void err_crc_loc_gt_pkt_size (const cpp_int  & l__val);
        cpp_int err_crc_loc_gt_pkt_size() const;
    
        typedef pu_cpp_int< 1 > err_pkt_eop_early_cpp_int_t;
        err_pkt_eop_early_cpp_int_t int_var__err_pkt_eop_early;
        void err_pkt_eop_early (const cpp_int  & l__val);
        cpp_int err_pkt_eop_early() const;
    
        typedef pu_cpp_int< 32 > rsvd_cpp_int_t;
        rsvd_cpp_int_t int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
}; // cap_dpr_csr_cfg_error_mask_t
    
class cap_dpr_csr_cfg_global_2_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dpr_csr_cfg_global_2_t(string name = "cap_dpr_csr_cfg_global_2_t", cap_csr_base *parent = 0);
        virtual ~cap_dpr_csr_cfg_global_2_t();
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
    
}; // cap_dpr_csr_cfg_global_2_t
    
class cap_dpr_csr_cfg_global_1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dpr_csr_cfg_global_1_t(string name = "cap_dpr_csr_cfg_global_1_t", cap_csr_base *parent = 0);
        virtual ~cap_dpr_csr_cfg_global_1_t();
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
    
        typedef pu_cpp_int< 1 > intrinsic_len_rw_en_cpp_int_t;
        intrinsic_len_rw_en_cpp_int_t int_var__intrinsic_len_rw_en;
        void intrinsic_len_rw_en (const cpp_int  & l__val);
        cpp_int intrinsic_len_rw_en() const;
    
        typedef pu_cpp_int< 1 > intrinsic_drop_rw_en_cpp_int_t;
        intrinsic_drop_rw_en_cpp_int_t int_var__intrinsic_drop_rw_en;
        void intrinsic_drop_rw_en (const cpp_int  & l__val);
        cpp_int intrinsic_drop_rw_en() const;
    
        typedef pu_cpp_int< 1 > padding_en_cpp_int_t;
        padding_en_cpp_int_t int_var__padding_en;
        void padding_en (const cpp_int  & l__val);
        cpp_int padding_en() const;
    
        typedef pu_cpp_int< 16 > pkt_min_size_cpp_int_t;
        pkt_min_size_cpp_int_t int_var__pkt_min_size;
        void pkt_min_size (const cpp_int  & l__val);
        cpp_int pkt_min_size() const;
    
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
    
        typedef pu_cpp_int< 1 > no_csum_update_on_csum_err_cpp_int_t;
        no_csum_update_on_csum_err_cpp_int_t int_var__no_csum_update_on_csum_err;
        void no_csum_update_on_csum_err (const cpp_int  & l__val);
        cpp_int no_csum_update_on_csum_err() const;
    
        typedef pu_cpp_int< 1 > no_crc_update_on_crc_err_cpp_int_t;
        no_crc_update_on_crc_err_cpp_int_t int_var__no_crc_update_on_crc_err;
        void no_crc_update_on_crc_err (const cpp_int  & l__val);
        cpp_int no_crc_update_on_crc_err() const;
    
        typedef pu_cpp_int< 1 > no_sf_cpp_int_t;
        no_sf_cpp_int_t int_var__no_sf;
        void no_sf (const cpp_int  & l__val);
        cpp_int no_sf() const;
    
}; // cap_dpr_csr_cfg_global_1_t
    
class cap_dpr_csr_cfg_global_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dpr_csr_cfg_global_t(string name = "cap_dpr_csr_cfg_global_t", cap_csr_base *parent = 0);
        virtual ~cap_dpr_csr_cfg_global_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 4 > input_pacer_timer_cpp_int_t;
        input_pacer_timer_cpp_int_t int_var__input_pacer_timer;
        void input_pacer_timer (const cpp_int  & l__val);
        cpp_int input_pacer_timer() const;
    
        typedef pu_cpp_int< 4 > input_pacer_release_cpp_int_t;
        input_pacer_release_cpp_int_t int_var__input_pacer_release;
        void input_pacer_release (const cpp_int  & l__val);
        cpp_int input_pacer_release() const;
    
        typedef pu_cpp_int< 3 > pktout_fc_threshold_cpp_int_t;
        pktout_fc_threshold_cpp_int_t int_var__pktout_fc_threshold;
        void pktout_fc_threshold (const cpp_int  & l__val);
        cpp_int pktout_fc_threshold() const;
    
        typedef pu_cpp_int< 10 > csum_ff_fc_threshold_cpp_int_t;
        csum_ff_fc_threshold_cpp_int_t int_var__csum_ff_fc_threshold;
        void csum_ff_fc_threshold (const cpp_int  & l__val);
        cpp_int csum_ff_fc_threshold() const;
    
        typedef pu_cpp_int< 8 > phv_fifo_almost_full_threshold_cpp_int_t;
        phv_fifo_almost_full_threshold_cpp_int_t int_var__phv_fifo_almost_full_threshold;
        void phv_fifo_almost_full_threshold (const cpp_int  & l__val);
        cpp_int phv_fifo_almost_full_threshold() const;
    
        typedef pu_cpp_int< 4 > ohi_fifo_almost_full_threshold_cpp_int_t;
        ohi_fifo_almost_full_threshold_cpp_int_t int_var__ohi_fifo_almost_full_threshold;
        void ohi_fifo_almost_full_threshold (const cpp_int  & l__val);
        cpp_int ohi_fifo_almost_full_threshold() const;
    
        typedef pu_cpp_int< 10 > pktin_fifo_almost_full_threshold_cpp_int_t;
        pktin_fifo_almost_full_threshold_cpp_int_t int_var__pktin_fifo_almost_full_threshold;
        void pktin_fifo_almost_full_threshold (const cpp_int  & l__val);
        cpp_int pktin_fifo_almost_full_threshold() const;
    
}; // cap_dpr_csr_cfg_global_t
    
class cap_dpr_csr_base_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dpr_csr_base_t(string name = "cap_dpr_csr_base_t", cap_csr_base *parent = 0);
        virtual ~cap_dpr_csr_base_t();
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
    
}; // cap_dpr_csr_base_t
    
class cap_dpr_csr_t : public cap_block_base {
    public:
        cpp_int_helper hlp;
    
        cap_dpr_csr_t(string name = "cap_dpr_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_dpr_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_dpr_csr_base_t base;
    
        cap_dpr_csr_cfg_global_t cfg_global;
    
        cap_dpr_csr_cfg_global_1_t cfg_global_1;
    
        cap_dpr_csr_cfg_global_2_t cfg_global_2;
    
        cap_dpr_csr_cfg_error_mask_t cfg_error_mask;
    
        cap_dpr_csr_cfg_interrupt_mask_t cfg_interrupt_mask;
    
        cap_dprhdrfld_csr_t hdrfld;
    
        cap_dprcfg_csr_t cfg;
    
        cap_dprstats_csr_t stats;
    
        cap_dprmem_csr_t mem;
    
}; // cap_dpr_csr_t
    
#endif // CAP_DPR_CSR_H
        