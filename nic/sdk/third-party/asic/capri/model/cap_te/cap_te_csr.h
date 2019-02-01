
#ifndef CAP_TE_CSR_H
#define CAP_TE_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_te_csr_int_info_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_int_info_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_int_info_int_enable_clear_t(string name = "cap_te_csr_int_info_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_int_info_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > profile_cam_hit0_enable_cpp_int_t;
        cpp_int int_var__profile_cam_hit0_enable;
        void profile_cam_hit0_enable (const cpp_int  & l__val);
        cpp_int profile_cam_hit0_enable() const;
    
        typedef pu_cpp_int< 1 > profile_cam_hit1_enable_cpp_int_t;
        cpp_int int_var__profile_cam_hit1_enable;
        void profile_cam_hit1_enable (const cpp_int  & l__val);
        cpp_int profile_cam_hit1_enable() const;
    
        typedef pu_cpp_int< 1 > profile_cam_hit2_enable_cpp_int_t;
        cpp_int int_var__profile_cam_hit2_enable;
        void profile_cam_hit2_enable (const cpp_int  & l__val);
        cpp_int profile_cam_hit2_enable() const;
    
        typedef pu_cpp_int< 1 > profile_cam_hit3_enable_cpp_int_t;
        cpp_int int_var__profile_cam_hit3_enable;
        void profile_cam_hit3_enable (const cpp_int  & l__val);
        cpp_int profile_cam_hit3_enable() const;
    
        typedef pu_cpp_int< 1 > profile_cam_hit4_enable_cpp_int_t;
        cpp_int int_var__profile_cam_hit4_enable;
        void profile_cam_hit4_enable (const cpp_int  & l__val);
        cpp_int profile_cam_hit4_enable() const;
    
        typedef pu_cpp_int< 1 > profile_cam_hit5_enable_cpp_int_t;
        cpp_int int_var__profile_cam_hit5_enable;
        void profile_cam_hit5_enable (const cpp_int  & l__val);
        cpp_int profile_cam_hit5_enable() const;
    
        typedef pu_cpp_int< 1 > profile_cam_hit6_enable_cpp_int_t;
        cpp_int int_var__profile_cam_hit6_enable;
        void profile_cam_hit6_enable (const cpp_int  & l__val);
        cpp_int profile_cam_hit6_enable() const;
    
        typedef pu_cpp_int< 1 > profile_cam_hit7_enable_cpp_int_t;
        cpp_int int_var__profile_cam_hit7_enable;
        void profile_cam_hit7_enable (const cpp_int  & l__val);
        cpp_int profile_cam_hit7_enable() const;
    
        typedef pu_cpp_int< 1 > profile_cam_hit8_enable_cpp_int_t;
        cpp_int int_var__profile_cam_hit8_enable;
        void profile_cam_hit8_enable (const cpp_int  & l__val);
        cpp_int profile_cam_hit8_enable() const;
    
        typedef pu_cpp_int< 1 > profile_cam_hit9_enable_cpp_int_t;
        cpp_int int_var__profile_cam_hit9_enable;
        void profile_cam_hit9_enable (const cpp_int  & l__val);
        cpp_int profile_cam_hit9_enable() const;
    
        typedef pu_cpp_int< 1 > profile_cam_hit10_enable_cpp_int_t;
        cpp_int int_var__profile_cam_hit10_enable;
        void profile_cam_hit10_enable (const cpp_int  & l__val);
        cpp_int profile_cam_hit10_enable() const;
    
        typedef pu_cpp_int< 1 > profile_cam_hit11_enable_cpp_int_t;
        cpp_int int_var__profile_cam_hit11_enable;
        void profile_cam_hit11_enable (const cpp_int  & l__val);
        cpp_int profile_cam_hit11_enable() const;
    
        typedef pu_cpp_int< 1 > profile_cam_hit12_enable_cpp_int_t;
        cpp_int int_var__profile_cam_hit12_enable;
        void profile_cam_hit12_enable (const cpp_int  & l__val);
        cpp_int profile_cam_hit12_enable() const;
    
        typedef pu_cpp_int< 1 > profile_cam_hit13_enable_cpp_int_t;
        cpp_int int_var__profile_cam_hit13_enable;
        void profile_cam_hit13_enable (const cpp_int  & l__val);
        cpp_int profile_cam_hit13_enable() const;
    
        typedef pu_cpp_int< 1 > profile_cam_hit14_enable_cpp_int_t;
        cpp_int int_var__profile_cam_hit14_enable;
        void profile_cam_hit14_enable (const cpp_int  & l__val);
        cpp_int profile_cam_hit14_enable() const;
    
        typedef pu_cpp_int< 1 > profile_cam_hit15_enable_cpp_int_t;
        cpp_int int_var__profile_cam_hit15_enable;
        void profile_cam_hit15_enable (const cpp_int  & l__val);
        cpp_int profile_cam_hit15_enable() const;
    
        typedef pu_cpp_int< 1 > profile_cam_miss_enable_cpp_int_t;
        cpp_int int_var__profile_cam_miss_enable;
        void profile_cam_miss_enable (const cpp_int  & l__val);
        cpp_int profile_cam_miss_enable() const;
    
}; // cap_te_csr_int_info_int_enable_clear_t
    
class cap_te_csr_int_info_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_int_info_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_int_info_int_test_set_t(string name = "cap_te_csr_int_info_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_int_info_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > profile_cam_hit0_interrupt_cpp_int_t;
        cpp_int int_var__profile_cam_hit0_interrupt;
        void profile_cam_hit0_interrupt (const cpp_int  & l__val);
        cpp_int profile_cam_hit0_interrupt() const;
    
        typedef pu_cpp_int< 1 > profile_cam_hit1_interrupt_cpp_int_t;
        cpp_int int_var__profile_cam_hit1_interrupt;
        void profile_cam_hit1_interrupt (const cpp_int  & l__val);
        cpp_int profile_cam_hit1_interrupt() const;
    
        typedef pu_cpp_int< 1 > profile_cam_hit2_interrupt_cpp_int_t;
        cpp_int int_var__profile_cam_hit2_interrupt;
        void profile_cam_hit2_interrupt (const cpp_int  & l__val);
        cpp_int profile_cam_hit2_interrupt() const;
    
        typedef pu_cpp_int< 1 > profile_cam_hit3_interrupt_cpp_int_t;
        cpp_int int_var__profile_cam_hit3_interrupt;
        void profile_cam_hit3_interrupt (const cpp_int  & l__val);
        cpp_int profile_cam_hit3_interrupt() const;
    
        typedef pu_cpp_int< 1 > profile_cam_hit4_interrupt_cpp_int_t;
        cpp_int int_var__profile_cam_hit4_interrupt;
        void profile_cam_hit4_interrupt (const cpp_int  & l__val);
        cpp_int profile_cam_hit4_interrupt() const;
    
        typedef pu_cpp_int< 1 > profile_cam_hit5_interrupt_cpp_int_t;
        cpp_int int_var__profile_cam_hit5_interrupt;
        void profile_cam_hit5_interrupt (const cpp_int  & l__val);
        cpp_int profile_cam_hit5_interrupt() const;
    
        typedef pu_cpp_int< 1 > profile_cam_hit6_interrupt_cpp_int_t;
        cpp_int int_var__profile_cam_hit6_interrupt;
        void profile_cam_hit6_interrupt (const cpp_int  & l__val);
        cpp_int profile_cam_hit6_interrupt() const;
    
        typedef pu_cpp_int< 1 > profile_cam_hit7_interrupt_cpp_int_t;
        cpp_int int_var__profile_cam_hit7_interrupt;
        void profile_cam_hit7_interrupt (const cpp_int  & l__val);
        cpp_int profile_cam_hit7_interrupt() const;
    
        typedef pu_cpp_int< 1 > profile_cam_hit8_interrupt_cpp_int_t;
        cpp_int int_var__profile_cam_hit8_interrupt;
        void profile_cam_hit8_interrupt (const cpp_int  & l__val);
        cpp_int profile_cam_hit8_interrupt() const;
    
        typedef pu_cpp_int< 1 > profile_cam_hit9_interrupt_cpp_int_t;
        cpp_int int_var__profile_cam_hit9_interrupt;
        void profile_cam_hit9_interrupt (const cpp_int  & l__val);
        cpp_int profile_cam_hit9_interrupt() const;
    
        typedef pu_cpp_int< 1 > profile_cam_hit10_interrupt_cpp_int_t;
        cpp_int int_var__profile_cam_hit10_interrupt;
        void profile_cam_hit10_interrupt (const cpp_int  & l__val);
        cpp_int profile_cam_hit10_interrupt() const;
    
        typedef pu_cpp_int< 1 > profile_cam_hit11_interrupt_cpp_int_t;
        cpp_int int_var__profile_cam_hit11_interrupt;
        void profile_cam_hit11_interrupt (const cpp_int  & l__val);
        cpp_int profile_cam_hit11_interrupt() const;
    
        typedef pu_cpp_int< 1 > profile_cam_hit12_interrupt_cpp_int_t;
        cpp_int int_var__profile_cam_hit12_interrupt;
        void profile_cam_hit12_interrupt (const cpp_int  & l__val);
        cpp_int profile_cam_hit12_interrupt() const;
    
        typedef pu_cpp_int< 1 > profile_cam_hit13_interrupt_cpp_int_t;
        cpp_int int_var__profile_cam_hit13_interrupt;
        void profile_cam_hit13_interrupt (const cpp_int  & l__val);
        cpp_int profile_cam_hit13_interrupt() const;
    
        typedef pu_cpp_int< 1 > profile_cam_hit14_interrupt_cpp_int_t;
        cpp_int int_var__profile_cam_hit14_interrupt;
        void profile_cam_hit14_interrupt (const cpp_int  & l__val);
        cpp_int profile_cam_hit14_interrupt() const;
    
        typedef pu_cpp_int< 1 > profile_cam_hit15_interrupt_cpp_int_t;
        cpp_int int_var__profile_cam_hit15_interrupt;
        void profile_cam_hit15_interrupt (const cpp_int  & l__val);
        cpp_int profile_cam_hit15_interrupt() const;
    
        typedef pu_cpp_int< 1 > profile_cam_miss_interrupt_cpp_int_t;
        cpp_int int_var__profile_cam_miss_interrupt;
        void profile_cam_miss_interrupt (const cpp_int  & l__val);
        cpp_int profile_cam_miss_interrupt() const;
    
}; // cap_te_csr_int_info_int_test_set_t
    
class cap_te_csr_int_info_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_int_info_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_int_info_t(string name = "cap_te_csr_int_info_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_int_info_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_te_csr_int_info_int_test_set_t intreg;
    
        cap_te_csr_int_info_int_test_set_t int_test_set;
    
        cap_te_csr_int_info_int_enable_clear_t int_enable_set;
    
        cap_te_csr_int_info_int_enable_clear_t int_enable_clear;
    
}; // cap_te_csr_int_info_t
    
class cap_te_csr_int_err_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_int_err_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_int_err_int_enable_clear_t(string name = "cap_te_csr_int_err_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_int_err_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > miss_sop_enable_cpp_int_t;
        cpp_int int_var__miss_sop_enable;
        void miss_sop_enable (const cpp_int  & l__val);
        cpp_int miss_sop_enable() const;
    
        typedef pu_cpp_int< 1 > miss_eop_enable_cpp_int_t;
        cpp_int int_var__miss_eop_enable;
        void miss_eop_enable (const cpp_int  & l__val);
        cpp_int miss_eop_enable() const;
    
        typedef pu_cpp_int< 1 > phv_max_size_enable_cpp_int_t;
        cpp_int int_var__phv_max_size_enable;
        void phv_max_size_enable (const cpp_int  & l__val);
        cpp_int phv_max_size_enable() const;
    
        typedef pu_cpp_int< 1 > spurious_axi_rsp_enable_cpp_int_t;
        cpp_int int_var__spurious_axi_rsp_enable;
        void spurious_axi_rsp_enable (const cpp_int  & l__val);
        cpp_int spurious_axi_rsp_enable() const;
    
        typedef pu_cpp_int< 1 > spurious_tcam_rsp_enable_cpp_int_t;
        cpp_int int_var__spurious_tcam_rsp_enable;
        void spurious_tcam_rsp_enable (const cpp_int  & l__val);
        cpp_int spurious_tcam_rsp_enable() const;
    
        typedef pu_cpp_int< 1 > te2mpu_timeout_enable_cpp_int_t;
        cpp_int int_var__te2mpu_timeout_enable;
        void te2mpu_timeout_enable (const cpp_int  & l__val);
        cpp_int te2mpu_timeout_enable() const;
    
        typedef pu_cpp_int< 1 > axi_rdrsp_err_enable_cpp_int_t;
        cpp_int int_var__axi_rdrsp_err_enable;
        void axi_rdrsp_err_enable (const cpp_int  & l__val);
        cpp_int axi_rdrsp_err_enable() const;
    
        typedef pu_cpp_int< 1 > axi_bad_read_enable_cpp_int_t;
        cpp_int int_var__axi_bad_read_enable;
        void axi_bad_read_enable (const cpp_int  & l__val);
        cpp_int axi_bad_read_enable() const;
    
        typedef pu_cpp_int< 1 > tcam_req_idx_fifo_enable_cpp_int_t;
        cpp_int int_var__tcam_req_idx_fifo_enable;
        void tcam_req_idx_fifo_enable (const cpp_int  & l__val);
        cpp_int tcam_req_idx_fifo_enable() const;
    
        typedef pu_cpp_int< 1 > tcam_rsp_idx_fifo_enable_cpp_int_t;
        cpp_int int_var__tcam_rsp_idx_fifo_enable;
        void tcam_rsp_idx_fifo_enable (const cpp_int  & l__val);
        cpp_int tcam_rsp_idx_fifo_enable() const;
    
        typedef pu_cpp_int< 1 > mpu_req_idx_fifo_enable_cpp_int_t;
        cpp_int int_var__mpu_req_idx_fifo_enable;
        void mpu_req_idx_fifo_enable (const cpp_int  & l__val);
        cpp_int mpu_req_idx_fifo_enable() const;
    
        typedef pu_cpp_int< 1 > axi_req_idx_fifo_enable_cpp_int_t;
        cpp_int int_var__axi_req_idx_fifo_enable;
        void axi_req_idx_fifo_enable (const cpp_int  & l__val);
        cpp_int axi_req_idx_fifo_enable() const;
    
        typedef pu_cpp_int< 1 > proc_tbl_vld_wo_proc_enable_cpp_int_t;
        cpp_int int_var__proc_tbl_vld_wo_proc_enable;
        void proc_tbl_vld_wo_proc_enable (const cpp_int  & l__val);
        cpp_int proc_tbl_vld_wo_proc_enable() const;
    
        typedef pu_cpp_int< 1 > pend_wo_wb_enable_cpp_int_t;
        cpp_int int_var__pend_wo_wb_enable;
        void pend_wo_wb_enable (const cpp_int  & l__val);
        cpp_int pend_wo_wb_enable() const;
    
        typedef pu_cpp_int< 1 > pend1_wo_pend0_enable_cpp_int_t;
        cpp_int int_var__pend1_wo_pend0_enable;
        void pend1_wo_pend0_enable (const cpp_int  & l__val);
        cpp_int pend1_wo_pend0_enable() const;
    
        typedef pu_cpp_int< 1 > both_pend_down_enable_cpp_int_t;
        cpp_int int_var__both_pend_down_enable;
        void both_pend_down_enable (const cpp_int  & l__val);
        cpp_int both_pend_down_enable() const;
    
        typedef pu_cpp_int< 1 > pend_wo_proc_down_enable_cpp_int_t;
        cpp_int int_var__pend_wo_proc_down_enable;
        void pend_wo_proc_down_enable (const cpp_int  & l__val);
        cpp_int pend_wo_proc_down_enable() const;
    
        typedef pu_cpp_int< 1 > both_pend_went_up_enable_cpp_int_t;
        cpp_int int_var__both_pend_went_up_enable;
        void both_pend_went_up_enable (const cpp_int  & l__val);
        cpp_int both_pend_went_up_enable() const;
    
        typedef pu_cpp_int< 1 > loaded_but_no_proc_enable_cpp_int_t;
        cpp_int int_var__loaded_but_no_proc_enable;
        void loaded_but_no_proc_enable (const cpp_int  & l__val);
        cpp_int loaded_but_no_proc_enable() const;
    
        typedef pu_cpp_int< 1 > loaded_but_no_proc_tbl_vld_enable_cpp_int_t;
        cpp_int int_var__loaded_but_no_proc_tbl_vld_enable;
        void loaded_but_no_proc_tbl_vld_enable (const cpp_int  & l__val);
        cpp_int loaded_but_no_proc_tbl_vld_enable() const;
    
}; // cap_te_csr_int_err_int_enable_clear_t
    
class cap_te_csr_int_err_intreg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_int_err_intreg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_int_err_intreg_t(string name = "cap_te_csr_int_err_intreg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_int_err_intreg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > miss_sop_interrupt_cpp_int_t;
        cpp_int int_var__miss_sop_interrupt;
        void miss_sop_interrupt (const cpp_int  & l__val);
        cpp_int miss_sop_interrupt() const;
    
        typedef pu_cpp_int< 1 > miss_eop_interrupt_cpp_int_t;
        cpp_int int_var__miss_eop_interrupt;
        void miss_eop_interrupt (const cpp_int  & l__val);
        cpp_int miss_eop_interrupt() const;
    
        typedef pu_cpp_int< 1 > phv_max_size_interrupt_cpp_int_t;
        cpp_int int_var__phv_max_size_interrupt;
        void phv_max_size_interrupt (const cpp_int  & l__val);
        cpp_int phv_max_size_interrupt() const;
    
        typedef pu_cpp_int< 1 > spurious_axi_rsp_interrupt_cpp_int_t;
        cpp_int int_var__spurious_axi_rsp_interrupt;
        void spurious_axi_rsp_interrupt (const cpp_int  & l__val);
        cpp_int spurious_axi_rsp_interrupt() const;
    
        typedef pu_cpp_int< 1 > spurious_tcam_rsp_interrupt_cpp_int_t;
        cpp_int int_var__spurious_tcam_rsp_interrupt;
        void spurious_tcam_rsp_interrupt (const cpp_int  & l__val);
        cpp_int spurious_tcam_rsp_interrupt() const;
    
        typedef pu_cpp_int< 1 > te2mpu_timeout_interrupt_cpp_int_t;
        cpp_int int_var__te2mpu_timeout_interrupt;
        void te2mpu_timeout_interrupt (const cpp_int  & l__val);
        cpp_int te2mpu_timeout_interrupt() const;
    
        typedef pu_cpp_int< 1 > axi_rdrsp_err_interrupt_cpp_int_t;
        cpp_int int_var__axi_rdrsp_err_interrupt;
        void axi_rdrsp_err_interrupt (const cpp_int  & l__val);
        cpp_int axi_rdrsp_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > axi_bad_read_interrupt_cpp_int_t;
        cpp_int int_var__axi_bad_read_interrupt;
        void axi_bad_read_interrupt (const cpp_int  & l__val);
        cpp_int axi_bad_read_interrupt() const;
    
        typedef pu_cpp_int< 1 > tcam_req_idx_fifo_interrupt_cpp_int_t;
        cpp_int int_var__tcam_req_idx_fifo_interrupt;
        void tcam_req_idx_fifo_interrupt (const cpp_int  & l__val);
        cpp_int tcam_req_idx_fifo_interrupt() const;
    
        typedef pu_cpp_int< 1 > tcam_rsp_idx_fifo_interrupt_cpp_int_t;
        cpp_int int_var__tcam_rsp_idx_fifo_interrupt;
        void tcam_rsp_idx_fifo_interrupt (const cpp_int  & l__val);
        cpp_int tcam_rsp_idx_fifo_interrupt() const;
    
        typedef pu_cpp_int< 1 > mpu_req_idx_fifo_interrupt_cpp_int_t;
        cpp_int int_var__mpu_req_idx_fifo_interrupt;
        void mpu_req_idx_fifo_interrupt (const cpp_int  & l__val);
        cpp_int mpu_req_idx_fifo_interrupt() const;
    
        typedef pu_cpp_int< 1 > axi_req_idx_fifo_interrupt_cpp_int_t;
        cpp_int int_var__axi_req_idx_fifo_interrupt;
        void axi_req_idx_fifo_interrupt (const cpp_int  & l__val);
        cpp_int axi_req_idx_fifo_interrupt() const;
    
        typedef pu_cpp_int< 1 > proc_tbl_vld_wo_proc_interrupt_cpp_int_t;
        cpp_int int_var__proc_tbl_vld_wo_proc_interrupt;
        void proc_tbl_vld_wo_proc_interrupt (const cpp_int  & l__val);
        cpp_int proc_tbl_vld_wo_proc_interrupt() const;
    
        typedef pu_cpp_int< 1 > pend_wo_wb_interrupt_cpp_int_t;
        cpp_int int_var__pend_wo_wb_interrupt;
        void pend_wo_wb_interrupt (const cpp_int  & l__val);
        cpp_int pend_wo_wb_interrupt() const;
    
        typedef pu_cpp_int< 1 > pend1_wo_pend0_interrupt_cpp_int_t;
        cpp_int int_var__pend1_wo_pend0_interrupt;
        void pend1_wo_pend0_interrupt (const cpp_int  & l__val);
        cpp_int pend1_wo_pend0_interrupt() const;
    
        typedef pu_cpp_int< 1 > both_pend_down_interrupt_cpp_int_t;
        cpp_int int_var__both_pend_down_interrupt;
        void both_pend_down_interrupt (const cpp_int  & l__val);
        cpp_int both_pend_down_interrupt() const;
    
        typedef pu_cpp_int< 1 > pend_wo_proc_down_interrupt_cpp_int_t;
        cpp_int int_var__pend_wo_proc_down_interrupt;
        void pend_wo_proc_down_interrupt (const cpp_int  & l__val);
        cpp_int pend_wo_proc_down_interrupt() const;
    
        typedef pu_cpp_int< 1 > both_pend_went_up_interrupt_cpp_int_t;
        cpp_int int_var__both_pend_went_up_interrupt;
        void both_pend_went_up_interrupt (const cpp_int  & l__val);
        cpp_int both_pend_went_up_interrupt() const;
    
        typedef pu_cpp_int< 1 > loaded_but_no_proc_interrupt_cpp_int_t;
        cpp_int int_var__loaded_but_no_proc_interrupt;
        void loaded_but_no_proc_interrupt (const cpp_int  & l__val);
        cpp_int loaded_but_no_proc_interrupt() const;
    
        typedef pu_cpp_int< 1 > loaded_but_no_proc_tbl_vld_interrupt_cpp_int_t;
        cpp_int int_var__loaded_but_no_proc_tbl_vld_interrupt;
        void loaded_but_no_proc_tbl_vld_interrupt (const cpp_int  & l__val);
        cpp_int loaded_but_no_proc_tbl_vld_interrupt() const;
    
}; // cap_te_csr_int_err_intreg_t
    
class cap_te_csr_int_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_int_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_int_err_t(string name = "cap_te_csr_int_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_int_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_te_csr_int_err_intreg_t intreg;
    
        cap_te_csr_int_err_intreg_t int_test_set;
    
        cap_te_csr_int_err_int_enable_clear_t int_enable_set;
    
        cap_te_csr_int_err_int_enable_clear_t int_enable_clear;
    
}; // cap_te_csr_int_err_t
    
class cap_te_csr_intreg_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_intreg_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_intreg_status_t(string name = "cap_te_csr_intreg_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_intreg_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > int_err_interrupt_cpp_int_t;
        cpp_int int_var__int_err_interrupt;
        void int_err_interrupt (const cpp_int  & l__val);
        cpp_int int_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_info_interrupt_cpp_int_t;
        cpp_int int_var__int_info_interrupt;
        void int_info_interrupt (const cpp_int  & l__val);
        cpp_int int_info_interrupt() const;
    
}; // cap_te_csr_intreg_status_t
    
class cap_te_csr_int_groups_int_enable_rw_reg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_int_groups_int_enable_rw_reg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_int_groups_int_enable_rw_reg_t(string name = "cap_te_csr_int_groups_int_enable_rw_reg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_int_groups_int_enable_rw_reg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > int_err_enable_cpp_int_t;
        cpp_int int_var__int_err_enable;
        void int_err_enable (const cpp_int  & l__val);
        cpp_int int_err_enable() const;
    
        typedef pu_cpp_int< 1 > int_info_enable_cpp_int_t;
        cpp_int int_var__int_info_enable;
        void int_info_enable (const cpp_int  & l__val);
        cpp_int int_info_enable() const;
    
}; // cap_te_csr_int_groups_int_enable_rw_reg_t
    
class cap_te_csr_intgrp_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_intgrp_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_intgrp_status_t(string name = "cap_te_csr_intgrp_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_intgrp_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_te_csr_intreg_status_t intreg;
    
        cap_te_csr_int_groups_int_enable_rw_reg_t int_enable_rw_reg;
    
        cap_te_csr_intreg_status_t int_rw_reg;
    
}; // cap_te_csr_intgrp_status_t
    
class cap_te_csr_dhs_table_profile_ctrl_sram_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_dhs_table_profile_ctrl_sram_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_dhs_table_profile_ctrl_sram_entry_t(string name = "cap_te_csr_dhs_table_profile_ctrl_sram_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_dhs_table_profile_ctrl_sram_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 2 > km_mode0_cpp_int_t;
        cpp_int int_var__km_mode0;
        void km_mode0 (const cpp_int  & l__val);
        cpp_int km_mode0() const;
    
        typedef pu_cpp_int< 2 > km_mode1_cpp_int_t;
        cpp_int int_var__km_mode1;
        void km_mode1 (const cpp_int  & l__val);
        cpp_int km_mode1() const;
    
        typedef pu_cpp_int< 2 > km_mode2_cpp_int_t;
        cpp_int int_var__km_mode2;
        void km_mode2 (const cpp_int  & l__val);
        cpp_int km_mode2() const;
    
        typedef pu_cpp_int< 2 > km_mode3_cpp_int_t;
        cpp_int int_var__km_mode3;
        void km_mode3 (const cpp_int  & l__val);
        cpp_int km_mode3() const;
    
        typedef pu_cpp_int< 3 > km_profile0_cpp_int_t;
        cpp_int int_var__km_profile0;
        void km_profile0 (const cpp_int  & l__val);
        cpp_int km_profile0() const;
    
        typedef pu_cpp_int< 3 > km_profile1_cpp_int_t;
        cpp_int int_var__km_profile1;
        void km_profile1 (const cpp_int  & l__val);
        cpp_int km_profile1() const;
    
        typedef pu_cpp_int< 3 > km_profile2_cpp_int_t;
        cpp_int int_var__km_profile2;
        void km_profile2 (const cpp_int  & l__val);
        cpp_int km_profile2() const;
    
        typedef pu_cpp_int< 3 > km_profile3_cpp_int_t;
        cpp_int int_var__km_profile3;
        void km_profile3 (const cpp_int  & l__val);
        cpp_int km_profile3() const;
    
        typedef pu_cpp_int< 1 > km_new_key0_cpp_int_t;
        cpp_int int_var__km_new_key0;
        void km_new_key0 (const cpp_int  & l__val);
        cpp_int km_new_key0() const;
    
        typedef pu_cpp_int< 1 > km_new_key1_cpp_int_t;
        cpp_int int_var__km_new_key1;
        void km_new_key1 (const cpp_int  & l__val);
        cpp_int km_new_key1() const;
    
        typedef pu_cpp_int< 1 > km_new_key2_cpp_int_t;
        cpp_int int_var__km_new_key2;
        void km_new_key2 (const cpp_int  & l__val);
        cpp_int km_new_key2() const;
    
        typedef pu_cpp_int< 1 > km_new_key3_cpp_int_t;
        cpp_int int_var__km_new_key3;
        void km_new_key3 (const cpp_int  & l__val);
        cpp_int km_new_key3() const;
    
        typedef pu_cpp_int< 3 > lkup_cpp_int_t;
        cpp_int int_var__lkup;
        void lkup (const cpp_int  & l__val);
        cpp_int lkup() const;
    
        typedef pu_cpp_int< 4 > tableid_cpp_int_t;
        cpp_int int_var__tableid;
        void tableid (const cpp_int  & l__val);
        cpp_int tableid() const;
    
        typedef pu_cpp_int< 3 > hash_sel_cpp_int_t;
        cpp_int int_var__hash_sel;
        void hash_sel (const cpp_int  & l__val);
        cpp_int hash_sel() const;
    
        typedef pu_cpp_int< 2 > hash_chain_cpp_int_t;
        cpp_int int_var__hash_chain;
        void hash_chain (const cpp_int  & l__val);
        cpp_int hash_chain() const;
    
        typedef pu_cpp_int< 2 > hash_store_cpp_int_t;
        cpp_int int_var__hash_store;
        void hash_store (const cpp_int  & l__val);
        cpp_int hash_store() const;
    
}; // cap_te_csr_dhs_table_profile_ctrl_sram_entry_t
    
class cap_te_csr_dhs_table_profile_ctrl_sram_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_dhs_table_profile_ctrl_sram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_dhs_table_profile_ctrl_sram_t(string name = "cap_te_csr_dhs_table_profile_ctrl_sram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_dhs_table_profile_ctrl_sram_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        #if 192 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_te_csr_dhs_table_profile_ctrl_sram_entry_t, 192> entry;
        #else 
        cap_te_csr_dhs_table_profile_ctrl_sram_entry_t entry[192];
        #endif
        int get_depth_entry() { return 192; }
    
}; // cap_te_csr_dhs_table_profile_ctrl_sram_t
    
class cap_te_csr_CNT_stall_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_CNT_stall_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_CNT_stall_t(string name = "cap_te_csr_CNT_stall_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_CNT_stall_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > reason0_cpp_int_t;
        cpp_int int_var__reason0;
        void reason0 (const cpp_int  & l__val);
        cpp_int reason0() const;
    
        typedef pu_cpp_int< 16 > reason1_cpp_int_t;
        cpp_int int_var__reason1;
        void reason1 (const cpp_int  & l__val);
        cpp_int reason1() const;
    
        typedef pu_cpp_int< 16 > reason2_cpp_int_t;
        cpp_int int_var__reason2;
        void reason2 (const cpp_int  & l__val);
        cpp_int reason2() const;
    
        typedef pu_cpp_int< 16 > reason3_cpp_int_t;
        cpp_int int_var__reason3;
        void reason3 (const cpp_int  & l__val);
        cpp_int reason3() const;
    
}; // cap_te_csr_CNT_stall_t
    
class cap_te_csr_cfg_hbm_addr_align_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_cfg_hbm_addr_align_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_cfg_hbm_addr_align_t(string name = "cap_te_csr_cfg_hbm_addr_align_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_cfg_hbm_addr_align_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > en_cpp_int_t;
        cpp_int int_var__en;
        void en (const cpp_int  & l__val);
        cpp_int en() const;
    
}; // cap_te_csr_cfg_hbm_addr_align_t
    
class cap_te_csr_cfg_bist_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_cfg_bist_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_cfg_bist_t(string name = "cap_te_csr_cfg_bist_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_cfg_bist_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > run_cpp_int_t;
        cpp_int int_var__run;
        void run (const cpp_int  & l__val);
        cpp_int run() const;
    
}; // cap_te_csr_cfg_bist_t
    
class cap_te_csr_sta_bist_done_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_sta_bist_done_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_sta_bist_done_t(string name = "cap_te_csr_sta_bist_done_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_sta_bist_done_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > pass_cpp_int_t;
        cpp_int int_var__pass;
        void pass (const cpp_int  & l__val);
        cpp_int pass() const;
    
        typedef pu_cpp_int< 1 > fail_cpp_int_t;
        cpp_int int_var__fail;
        void fail (const cpp_int  & l__val);
        cpp_int fail() const;
    
}; // cap_te_csr_sta_bist_done_t
    
class cap_te_csr_sta_peek_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_sta_peek_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_sta_peek_t(string name = "cap_te_csr_sta_peek_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_sta_peek_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 64 > signals_cpp_int_t;
        cpp_int int_var__signals;
        void signals (const cpp_int  & l__val);
        cpp_int signals() const;
    
}; // cap_te_csr_sta_peek_t
    
class cap_te_csr_csr_intr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_csr_intr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_csr_intr_t(string name = "cap_te_csr_csr_intr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_csr_intr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > dowstream_cpp_int_t;
        cpp_int int_var__dowstream;
        void dowstream (const cpp_int  & l__val);
        cpp_int dowstream() const;
    
        typedef pu_cpp_int< 1 > dowstream_enable_cpp_int_t;
        cpp_int int_var__dowstream_enable;
        void dowstream_enable (const cpp_int  & l__val);
        cpp_int dowstream_enable() const;
    
}; // cap_te_csr_csr_intr_t
    
class cap_te_csr_sta_bad_axi_read_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_sta_bad_axi_read_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_sta_bad_axi_read_t(string name = "cap_te_csr_sta_bad_axi_read_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_sta_bad_axi_read_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 65 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 7 > id_cpp_int_t;
        cpp_int int_var__id;
        void id (const cpp_int  & l__val);
        cpp_int id() const;
    
        typedef pu_cpp_int< 4 > len_cpp_int_t;
        cpp_int int_var__len;
        void len (const cpp_int  & l__val);
        cpp_int len() const;
    
        typedef pu_cpp_int< 3 > sz_cpp_int_t;
        cpp_int int_var__sz;
        void sz (const cpp_int  & l__val);
        cpp_int sz() const;
    
}; // cap_te_csr_sta_bad_axi_read_t
    
class cap_te_csr_sta_axi_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_sta_axi_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_sta_axi_err_t(string name = "cap_te_csr_sta_axi_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_sta_axi_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 2 > resp_cpp_int_t;
        cpp_int int_var__resp;
        void resp (const cpp_int  & l__val);
        cpp_int resp() const;
    
        typedef pu_cpp_int< 7 > id_cpp_int_t;
        cpp_int int_var__id;
        void id (const cpp_int  & l__val);
        cpp_int id() const;
    
}; // cap_te_csr_sta_axi_err_t
    
class cap_te_csr_sta_wait_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_sta_wait_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_sta_wait_t(string name = "cap_te_csr_sta_wait_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_sta_wait_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 512 > entry_dat_cpp_int_t;
        cpp_int int_var__entry_dat;
        void entry_dat (const cpp_int  & l__val);
        cpp_int entry_dat() const;
    
}; // cap_te_csr_sta_wait_t
    
class cap_te_csr_sta_wait_fsm_states_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_sta_wait_fsm_states_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_sta_wait_fsm_states_t(string name = "cap_te_csr_sta_wait_fsm_states_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_sta_wait_fsm_states_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 3 > entry0_cpp_int_t;
        cpp_int int_var__entry0;
        void entry0 (const cpp_int  & l__val);
        cpp_int entry0() const;
    
        typedef pu_cpp_int< 3 > entry1_cpp_int_t;
        cpp_int int_var__entry1;
        void entry1 (const cpp_int  & l__val);
        cpp_int entry1() const;
    
        typedef pu_cpp_int< 3 > entry2_cpp_int_t;
        cpp_int int_var__entry2;
        void entry2 (const cpp_int  & l__val);
        cpp_int entry2() const;
    
        typedef pu_cpp_int< 3 > entry3_cpp_int_t;
        cpp_int int_var__entry3;
        void entry3 (const cpp_int  & l__val);
        cpp_int entry3() const;
    
        typedef pu_cpp_int< 3 > entry4_cpp_int_t;
        cpp_int int_var__entry4;
        void entry4 (const cpp_int  & l__val);
        cpp_int entry4() const;
    
        typedef pu_cpp_int< 3 > entry5_cpp_int_t;
        cpp_int int_var__entry5;
        void entry5 (const cpp_int  & l__val);
        cpp_int entry5() const;
    
        typedef pu_cpp_int< 3 > entry6_cpp_int_t;
        cpp_int int_var__entry6;
        void entry6 (const cpp_int  & l__val);
        cpp_int entry6() const;
    
        typedef pu_cpp_int< 3 > entry7_cpp_int_t;
        cpp_int int_var__entry7;
        void entry7 (const cpp_int  & l__val);
        cpp_int entry7() const;
    
        typedef pu_cpp_int< 3 > entry8_cpp_int_t;
        cpp_int int_var__entry8;
        void entry8 (const cpp_int  & l__val);
        cpp_int entry8() const;
    
        typedef pu_cpp_int< 3 > entry9_cpp_int_t;
        cpp_int int_var__entry9;
        void entry9 (const cpp_int  & l__val);
        cpp_int entry9() const;
    
        typedef pu_cpp_int< 3 > entry10_cpp_int_t;
        cpp_int int_var__entry10;
        void entry10 (const cpp_int  & l__val);
        cpp_int entry10() const;
    
        typedef pu_cpp_int< 3 > entry11_cpp_int_t;
        cpp_int int_var__entry11;
        void entry11 (const cpp_int  & l__val);
        cpp_int entry11() const;
    
        typedef pu_cpp_int< 3 > entry12_cpp_int_t;
        cpp_int int_var__entry12;
        void entry12 (const cpp_int  & l__val);
        cpp_int entry12() const;
    
        typedef pu_cpp_int< 3 > entry13_cpp_int_t;
        cpp_int int_var__entry13;
        void entry13 (const cpp_int  & l__val);
        cpp_int entry13() const;
    
        typedef pu_cpp_int< 3 > entry14_cpp_int_t;
        cpp_int int_var__entry14;
        void entry14 (const cpp_int  & l__val);
        cpp_int entry14() const;
    
        typedef pu_cpp_int< 3 > entry15_cpp_int_t;
        cpp_int int_var__entry15;
        void entry15 (const cpp_int  & l__val);
        cpp_int entry15() const;
    
}; // cap_te_csr_sta_wait_fsm_states_t
    
class cap_te_csr_sta_wait_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_sta_wait_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_sta_wait_entry_t(string name = "cap_te_csr_sta_wait_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_sta_wait_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 590 > key_cpp_int_t;
        cpp_int int_var__key;
        void key (const cpp_int  & l__val);
        cpp_int key() const;
    
        typedef pu_cpp_int< 108 > cam_cpp_int_t;
        cpp_int int_var__cam;
        void cam (const cpp_int  & l__val);
        cpp_int cam() const;
    
        typedef pu_cpp_int< 17 > haz_cpp_int_t;
        cpp_int int_var__haz;
        void haz (const cpp_int  & l__val);
        cpp_int haz() const;
    
        typedef pu_cpp_int< 3 > fsm_cpp_int_t;
        cpp_int int_var__fsm;
        void fsm (const cpp_int  & l__val);
        cpp_int fsm() const;
    
}; // cap_te_csr_sta_wait_entry_t
    
class cap_te_csr_cfg_timeout_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_cfg_timeout_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_cfg_timeout_t(string name = "cap_te_csr_cfg_timeout_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_cfg_timeout_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 24 > te2mpu_cpp_int_t;
        cpp_int int_var__te2mpu;
        void te2mpu (const cpp_int  & l__val);
        cpp_int te2mpu() const;
    
}; // cap_te_csr_cfg_timeout_t
    
class cap_te_csr_cfg_page_size_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_cfg_page_size_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_cfg_page_size_t(string name = "cap_te_csr_cfg_page_size_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_cfg_page_size_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 4 > sz_cpp_int_t;
        cpp_int int_var__sz;
        void sz (const cpp_int  & l__val);
        cpp_int sz() const;
    
}; // cap_te_csr_cfg_page_size_t
    
class cap_te_csr_cfg_read_wait_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_cfg_read_wait_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_cfg_read_wait_entry_t(string name = "cap_te_csr_cfg_read_wait_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_cfg_read_wait_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 4 > idx_cpp_int_t;
        cpp_int int_var__idx;
        void idx (const cpp_int  & l__val);
        cpp_int idx() const;
    
}; // cap_te_csr_cfg_read_wait_entry_t
    
class cap_te_csr_cfg_addr_target_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_cfg_addr_target_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_cfg_addr_target_t(string name = "cap_te_csr_cfg_addr_target_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_cfg_addr_target_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 6 > host_cpp_int_t;
        cpp_int int_var__host;
        void host (const cpp_int  & l__val);
        cpp_int host() const;
    
        typedef pu_cpp_int< 7 > sram_cpp_int_t;
        cpp_int int_var__sram;
        void sram (const cpp_int  & l__val);
        cpp_int sram() const;
    
}; // cap_te_csr_cfg_addr_target_t
    
class cap_te_csr_sta_debug_bus_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_sta_debug_bus_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_sta_debug_bus_t(string name = "cap_te_csr_sta_debug_bus_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_sta_debug_bus_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 256 > vec_cpp_int_t;
        cpp_int int_var__vec;
        void vec (const cpp_int  & l__val);
        cpp_int vec() const;
    
}; // cap_te_csr_sta_debug_bus_t
    
class cap_te_csr_cfg_debug_bus_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_cfg_debug_bus_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_cfg_debug_bus_t(string name = "cap_te_csr_cfg_debug_bus_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_cfg_debug_bus_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 3 > axi_rdata_sel_cpp_int_t;
        cpp_int int_var__axi_rdata_sel;
        void axi_rdata_sel (const cpp_int  & l__val);
        cpp_int axi_rdata_sel() const;
    
        typedef pu_cpp_int< 3 > tcam_key_sel_cpp_int_t;
        cpp_int int_var__tcam_key_sel;
        void tcam_key_sel (const cpp_int  & l__val);
        cpp_int tcam_key_sel() const;
    
        typedef pu_cpp_int< 3 > hash_key_sel_cpp_int_t;
        cpp_int int_var__hash_key_sel;
        void hash_key_sel (const cpp_int  & l__val);
        cpp_int hash_key_sel() const;
    
        typedef pu_cpp_int< 4 > sel_cpp_int_t;
        cpp_int int_var__sel;
        void sel (const cpp_int  & l__val);
        cpp_int sel() const;
    
}; // cap_te_csr_cfg_debug_bus_t
    
class cap_te_csr_cnt_mpu_out_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_cnt_mpu_out_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_cnt_mpu_out_t(string name = "cap_te_csr_cnt_mpu_out_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_cnt_mpu_out_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_te_csr_cnt_mpu_out_t
    
class cap_te_csr_cnt_tcam_rsp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_cnt_tcam_rsp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_cnt_tcam_rsp_t(string name = "cap_te_csr_cnt_tcam_rsp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_cnt_tcam_rsp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_te_csr_cnt_tcam_rsp_t
    
class cap_te_csr_cnt_tcam_req_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_cnt_tcam_req_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_cnt_tcam_req_t(string name = "cap_te_csr_cnt_tcam_req_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_cnt_tcam_req_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_te_csr_cnt_tcam_req_t
    
class cap_te_csr_cnt_axi_rdrsp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_cnt_axi_rdrsp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_cnt_axi_rdrsp_t(string name = "cap_te_csr_cnt_axi_rdrsp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_cnt_axi_rdrsp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_te_csr_cnt_axi_rdrsp_t
    
class cap_te_csr_cnt_axi_rdreq_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_cnt_axi_rdreq_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_cnt_axi_rdreq_t(string name = "cap_te_csr_cnt_axi_rdreq_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_cnt_axi_rdreq_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_te_csr_cnt_axi_rdreq_t
    
class cap_te_csr_cnt_phv_out_eop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_cnt_phv_out_eop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_cnt_phv_out_eop_t(string name = "cap_te_csr_cnt_phv_out_eop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_cnt_phv_out_eop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_te_csr_cnt_phv_out_eop_t
    
class cap_te_csr_cnt_phv_out_sop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_cnt_phv_out_sop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_cnt_phv_out_sop_t(string name = "cap_te_csr_cnt_phv_out_sop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_cnt_phv_out_sop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_te_csr_cnt_phv_out_sop_t
    
class cap_te_csr_cnt_phv_in_eop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_cnt_phv_in_eop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_cnt_phv_in_eop_t(string name = "cap_te_csr_cnt_phv_in_eop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_cnt_phv_in_eop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_te_csr_cnt_phv_in_eop_t
    
class cap_te_csr_cnt_phv_in_sop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_cnt_phv_in_sop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_cnt_phv_in_sop_t(string name = "cap_te_csr_cnt_phv_in_sop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_cnt_phv_in_sop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_te_csr_cnt_phv_in_sop_t
    
class cap_te_csr_dhs_single_step_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_dhs_single_step_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_dhs_single_step_t(string name = "cap_te_csr_dhs_single_step_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_dhs_single_step_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > key_cpp_int_t;
        cpp_int int_var__key;
        void key (const cpp_int  & l__val);
        cpp_int key() const;
    
        typedef pu_cpp_int< 1 > lkup_cpp_int_t;
        cpp_int int_var__lkup;
        void lkup (const cpp_int  & l__val);
        cpp_int lkup() const;
    
        typedef pu_cpp_int< 1 > mpu_cpp_int_t;
        cpp_int int_var__mpu;
        void mpu (const cpp_int  & l__val);
        cpp_int mpu() const;
    
}; // cap_te_csr_dhs_single_step_t
    
class cap_te_csr_cfg_km_profile_bit_loc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_cfg_km_profile_bit_loc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_cfg_km_profile_bit_loc_t(string name = "cap_te_csr_cfg_km_profile_bit_loc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_cfg_km_profile_bit_loc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        cpp_int int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 5 > bit_loc_cpp_int_t;
        cpp_int int_var__bit_loc;
        void bit_loc (const cpp_int  & l__val);
        cpp_int bit_loc() const;
    
}; // cap_te_csr_cfg_km_profile_bit_loc_t
    
class cap_te_csr_cfg_km_profile_bit_sel_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_cfg_km_profile_bit_sel_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_cfg_km_profile_bit_sel_t(string name = "cap_te_csr_cfg_km_profile_bit_sel_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_cfg_km_profile_bit_sel_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 13 > bit_sel_cpp_int_t;
        cpp_int int_var__bit_sel;
        void bit_sel (const cpp_int  & l__val);
        cpp_int bit_sel() const;
    
}; // cap_te_csr_cfg_km_profile_bit_sel_t
    
class cap_te_csr_cfg_km_profile_byte_sel_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_cfg_km_profile_byte_sel_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_cfg_km_profile_byte_sel_t(string name = "cap_te_csr_cfg_km_profile_byte_sel_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_cfg_km_profile_byte_sel_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > byte_sel_cpp_int_t;
        cpp_int int_var__byte_sel;
        void byte_sel (const cpp_int  & l__val);
        cpp_int byte_sel() const;
    
}; // cap_te_csr_cfg_km_profile_byte_sel_t
    
class cap_te_csr_cfg_table_mpu_const_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_cfg_table_mpu_const_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_cfg_table_mpu_const_t(string name = "cap_te_csr_cfg_table_mpu_const_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_cfg_table_mpu_const_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 64 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_te_csr_cfg_table_mpu_const_t
    
class cap_te_csr_cfg_table_property_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_cfg_table_property_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_cfg_table_property_t(string name = "cap_te_csr_cfg_table_property_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_cfg_table_property_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > axi_cpp_int_t;
        cpp_int int_var__axi;
        void axi (const cpp_int  & l__val);
        cpp_int axi() const;
    
        typedef pu_cpp_int< 10 > key_mask_hi_cpp_int_t;
        cpp_int int_var__key_mask_hi;
        void key_mask_hi (const cpp_int  & l__val);
        cpp_int key_mask_hi() const;
    
        typedef pu_cpp_int< 9 > key_mask_lo_cpp_int_t;
        cpp_int int_var__key_mask_lo;
        void key_mask_lo (const cpp_int  & l__val);
        cpp_int key_mask_lo() const;
    
        typedef pu_cpp_int< 2 > fullkey_km_sel0_cpp_int_t;
        cpp_int int_var__fullkey_km_sel0;
        void fullkey_km_sel0 (const cpp_int  & l__val);
        cpp_int fullkey_km_sel0() const;
    
        typedef pu_cpp_int< 2 > fullkey_km_sel1_cpp_int_t;
        cpp_int int_var__fullkey_km_sel1;
        void fullkey_km_sel1 (const cpp_int  & l__val);
        cpp_int fullkey_km_sel1() const;
    
        typedef pu_cpp_int< 1 > lock_en_cpp_int_t;
        cpp_int int_var__lock_en;
        void lock_en (const cpp_int  & l__val);
        cpp_int lock_en() const;
    
        typedef pu_cpp_int< 1 > tbl_entry_sz_raw_cpp_int_t;
        cpp_int int_var__tbl_entry_sz_raw;
        void tbl_entry_sz_raw (const cpp_int  & l__val);
        cpp_int tbl_entry_sz_raw() const;
    
        typedef pu_cpp_int< 1 > mpu_pc_dyn_cpp_int_t;
        cpp_int int_var__mpu_pc_dyn;
        void mpu_pc_dyn (const cpp_int  & l__val);
        cpp_int mpu_pc_dyn() const;
    
        typedef pu_cpp_int< 28 > mpu_pc_cpp_int_t;
        cpp_int int_var__mpu_pc;
        void mpu_pc (const cpp_int  & l__val);
        cpp_int mpu_pc() const;
    
        typedef pu_cpp_int< 1 > mpu_pc_raw_cpp_int_t;
        cpp_int int_var__mpu_pc_raw;
        void mpu_pc_raw (const cpp_int  & l__val);
        cpp_int mpu_pc_raw() const;
    
        typedef pu_cpp_int< 9 > mpu_pc_ofst_err_cpp_int_t;
        cpp_int int_var__mpu_pc_ofst_err;
        void mpu_pc_ofst_err (const cpp_int  & l__val);
        cpp_int mpu_pc_ofst_err() const;
    
        typedef pu_cpp_int< 4 > mpu_vec_cpp_int_t;
        cpp_int int_var__mpu_vec;
        void mpu_vec (const cpp_int  & l__val);
        cpp_int mpu_vec() const;
    
        typedef pu_cpp_int< 64 > addr_base_cpp_int_t;
        cpp_int int_var__addr_base;
        void addr_base (const cpp_int  & l__val);
        cpp_int addr_base() const;
    
        typedef pu_cpp_int< 1 > addr_vf_id_en_cpp_int_t;
        cpp_int int_var__addr_vf_id_en;
        void addr_vf_id_en (const cpp_int  & l__val);
        cpp_int addr_vf_id_en() const;
    
        typedef pu_cpp_int< 6 > addr_vf_id_loc_cpp_int_t;
        cpp_int int_var__addr_vf_id_loc;
        void addr_vf_id_loc (const cpp_int  & l__val);
        cpp_int addr_vf_id_loc() const;
    
        typedef pu_cpp_int< 5 > addr_shift_cpp_int_t;
        cpp_int int_var__addr_shift;
        void addr_shift (const cpp_int  & l__val);
        cpp_int addr_shift() const;
    
        typedef pu_cpp_int< 6 > chain_shift_cpp_int_t;
        cpp_int int_var__chain_shift;
        void chain_shift (const cpp_int  & l__val);
        cpp_int chain_shift() const;
    
        typedef pu_cpp_int< 7 > addr_sz_cpp_int_t;
        cpp_int int_var__addr_sz;
        void addr_sz (const cpp_int  & l__val);
        cpp_int addr_sz() const;
    
        typedef pu_cpp_int< 3 > lg2_entry_size_cpp_int_t;
        cpp_int int_var__lg2_entry_size;
        void lg2_entry_size (const cpp_int  & l__val);
        cpp_int lg2_entry_size() const;
    
        typedef pu_cpp_int< 10 > max_bypass_cnt_cpp_int_t;
        cpp_int int_var__max_bypass_cnt;
        void max_bypass_cnt (const cpp_int  & l__val);
        cpp_int max_bypass_cnt() const;
    
        typedef pu_cpp_int< 1 > lock_en_raw_cpp_int_t;
        cpp_int int_var__lock_en_raw;
        void lock_en_raw (const cpp_int  & l__val);
        cpp_int lock_en_raw() const;
    
        typedef pu_cpp_int< 1 > mpu_lb_cpp_int_t;
        cpp_int int_var__mpu_lb;
        void mpu_lb (const cpp_int  & l__val);
        cpp_int mpu_lb() const;
    
        typedef pu_cpp_int< 6 > mpu_pc_loc_cpp_int_t;
        cpp_int int_var__mpu_pc_loc;
        void mpu_pc_loc (const cpp_int  & l__val);
        cpp_int mpu_pc_loc() const;
    
        typedef pu_cpp_int< 4 > mpu_pc_ofst_shift_cpp_int_t;
        cpp_int int_var__mpu_pc_ofst_shift;
        void mpu_pc_ofst_shift (const cpp_int  & l__val);
        cpp_int mpu_pc_ofst_shift() const;
    
        typedef pu_cpp_int< 20 > oflow_base_idx_cpp_int_t;
        cpp_int int_var__oflow_base_idx;
        void oflow_base_idx (const cpp_int  & l__val);
        cpp_int oflow_base_idx() const;
    
        typedef pu_cpp_int< 1 > en_pc_axi_err_cpp_int_t;
        cpp_int int_var__en_pc_axi_err;
        void en_pc_axi_err (const cpp_int  & l__val);
        cpp_int en_pc_axi_err() const;
    
        typedef pu_cpp_int< 1 > en_pc_phv_err_cpp_int_t;
        cpp_int int_var__en_pc_phv_err;
        void en_pc_phv_err (const cpp_int  & l__val);
        cpp_int en_pc_phv_err() const;
    
        typedef pu_cpp_int< 1 > memory_only_cpp_int_t;
        cpp_int int_var__memory_only;
        void memory_only (const cpp_int  & l__val);
        cpp_int memory_only() const;
    
}; // cap_te_csr_cfg_table_property_t
    
class cap_te_csr_cfg_table_profile_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_cfg_table_profile_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_cfg_table_profile_t(string name = "cap_te_csr_cfg_table_profile_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_cfg_table_profile_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 4 > mpu_results_cpp_int_t;
        cpp_int int_var__mpu_results;
        void mpu_results (const cpp_int  & l__val);
        cpp_int mpu_results() const;
    
        typedef pu_cpp_int< 8 > seq_base_cpp_int_t;
        cpp_int int_var__seq_base;
        void seq_base (const cpp_int  & l__val);
        cpp_int seq_base() const;
    
}; // cap_te_csr_cfg_table_profile_t
    
class cap_te_csr_cfg_table_profile_ctrl_sram_ext_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_cfg_table_profile_ctrl_sram_ext_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_cfg_table_profile_ctrl_sram_ext_t(string name = "cap_te_csr_cfg_table_profile_ctrl_sram_ext_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_cfg_table_profile_ctrl_sram_ext_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > adv_phv_flit_cpp_int_t;
        cpp_int int_var__adv_phv_flit;
        void adv_phv_flit (const cpp_int  & l__val);
        cpp_int adv_phv_flit() const;
    
        typedef pu_cpp_int< 1 > done_cpp_int_t;
        cpp_int int_var__done;
        void done (const cpp_int  & l__val);
        cpp_int done() const;
    
}; // cap_te_csr_cfg_table_profile_ctrl_sram_ext_t
    
class cap_te_csr_cfg_table_profile_cam_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_cfg_table_profile_cam_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_cfg_table_profile_cam_t(string name = "cap_te_csr_cfg_table_profile_cam_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_cfg_table_profile_cam_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        cpp_int int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 8 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
        typedef pu_cpp_int< 8 > mask_cpp_int_t;
        cpp_int int_var__mask;
        void mask (const cpp_int  & l__val);
        cpp_int mask() const;
    
}; // cap_te_csr_cfg_table_profile_cam_t
    
class cap_te_csr_cfg_table_profile_key_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_cfg_table_profile_key_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_cfg_table_profile_key_t(string name = "cap_te_csr_cfg_table_profile_key_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_cfg_table_profile_key_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 9 > sel0_cpp_int_t;
        cpp_int int_var__sel0;
        void sel0 (const cpp_int  & l__val);
        cpp_int sel0() const;
    
        typedef pu_cpp_int< 9 > sel1_cpp_int_t;
        cpp_int int_var__sel1;
        void sel1 (const cpp_int  & l__val);
        cpp_int sel1() const;
    
        typedef pu_cpp_int< 9 > sel2_cpp_int_t;
        cpp_int int_var__sel2;
        void sel2 (const cpp_int  & l__val);
        cpp_int sel2() const;
    
        typedef pu_cpp_int< 9 > sel3_cpp_int_t;
        cpp_int int_var__sel3;
        void sel3 (const cpp_int  & l__val);
        cpp_int sel3() const;
    
        typedef pu_cpp_int< 9 > sel4_cpp_int_t;
        cpp_int int_var__sel4;
        void sel4 (const cpp_int  & l__val);
        cpp_int sel4() const;
    
        typedef pu_cpp_int< 9 > sel5_cpp_int_t;
        cpp_int int_var__sel5;
        void sel5 (const cpp_int  & l__val);
        cpp_int sel5() const;
    
        typedef pu_cpp_int< 9 > sel6_cpp_int_t;
        cpp_int int_var__sel6;
        void sel6 (const cpp_int  & l__val);
        cpp_int sel6() const;
    
        typedef pu_cpp_int< 9 > sel7_cpp_int_t;
        cpp_int int_var__sel7;
        void sel7 (const cpp_int  & l__val);
        cpp_int sel7() const;
    
}; // cap_te_csr_cfg_table_profile_key_t
    
class cap_te_csr_cfg_global_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_cfg_global_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_cfg_global_t(string name = "cap_te_csr_cfg_global_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_cfg_global_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > sw_rst_cpp_int_t;
        cpp_int int_var__sw_rst;
        void sw_rst (const cpp_int  & l__val);
        cpp_int sw_rst() const;
    
        typedef pu_cpp_int< 1 > step_key_cpp_int_t;
        cpp_int int_var__step_key;
        void step_key (const cpp_int  & l__val);
        cpp_int step_key() const;
    
        typedef pu_cpp_int< 1 > step_lkup_cpp_int_t;
        cpp_int int_var__step_lkup;
        void step_lkup (const cpp_int  & l__val);
        cpp_int step_lkup() const;
    
        typedef pu_cpp_int< 1 > step_mpu_cpp_int_t;
        cpp_int int_var__step_mpu;
        void step_mpu (const cpp_int  & l__val);
        cpp_int step_mpu() const;
    
}; // cap_te_csr_cfg_global_t
    
class cap_te_csr_base_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_base_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_base_t(string name = "cap_te_csr_base_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_base_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > scratch_reg_cpp_int_t;
        cpp_int int_var__scratch_reg;
        void scratch_reg (const cpp_int  & l__val);
        cpp_int scratch_reg() const;
    
}; // cap_te_csr_base_t
    
class cap_te_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_csr_t(string name = "cap_te_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_te_csr_base_t base;
    
        cap_te_csr_cfg_global_t cfg_global;
    
        cap_te_csr_cfg_table_profile_key_t cfg_table_profile_key;
    
        #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_te_csr_cfg_table_profile_cam_t, 16> cfg_table_profile_cam;
        #else 
        cap_te_csr_cfg_table_profile_cam_t cfg_table_profile_cam[16];
        #endif
        int get_depth_cfg_table_profile_cam() { return 16; }
    
        #if 192 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_te_csr_cfg_table_profile_ctrl_sram_ext_t, 192> cfg_table_profile_ctrl_sram_ext;
        #else 
        cap_te_csr_cfg_table_profile_ctrl_sram_ext_t cfg_table_profile_ctrl_sram_ext[192];
        #endif
        int get_depth_cfg_table_profile_ctrl_sram_ext() { return 192; }
    
        #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_te_csr_cfg_table_profile_t, 16> cfg_table_profile;
        #else 
        cap_te_csr_cfg_table_profile_t cfg_table_profile[16];
        #endif
        int get_depth_cfg_table_profile() { return 16; }
    
        #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_te_csr_cfg_table_property_t, 16> cfg_table_property;
        #else 
        cap_te_csr_cfg_table_property_t cfg_table_property[16];
        #endif
        int get_depth_cfg_table_property() { return 16; }
    
        #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_te_csr_cfg_table_mpu_const_t, 16> cfg_table_mpu_const;
        #else 
        cap_te_csr_cfg_table_mpu_const_t cfg_table_mpu_const[16];
        #endif
        int get_depth_cfg_table_mpu_const() { return 16; }
    
        #if 256 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_te_csr_cfg_km_profile_byte_sel_t, 256> cfg_km_profile_byte_sel;
        #else 
        cap_te_csr_cfg_km_profile_byte_sel_t cfg_km_profile_byte_sel[256];
        #endif
        int get_depth_cfg_km_profile_byte_sel() { return 256; }
    
        #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_te_csr_cfg_km_profile_bit_sel_t, 128> cfg_km_profile_bit_sel;
        #else 
        cap_te_csr_cfg_km_profile_bit_sel_t cfg_km_profile_bit_sel[128];
        #endif
        int get_depth_cfg_km_profile_bit_sel() { return 128; }
    
        #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_te_csr_cfg_km_profile_bit_loc_t, 16> cfg_km_profile_bit_loc;
        #else 
        cap_te_csr_cfg_km_profile_bit_loc_t cfg_km_profile_bit_loc[16];
        #endif
        int get_depth_cfg_km_profile_bit_loc() { return 16; }
    
        cap_te_csr_dhs_single_step_t dhs_single_step;
    
        cap_te_csr_cnt_phv_in_sop_t cnt_phv_in_sop;
    
        cap_te_csr_cnt_phv_in_eop_t cnt_phv_in_eop;
    
        cap_te_csr_cnt_phv_out_sop_t cnt_phv_out_sop;
    
        cap_te_csr_cnt_phv_out_eop_t cnt_phv_out_eop;
    
        cap_te_csr_cnt_axi_rdreq_t cnt_axi_rdreq;
    
        cap_te_csr_cnt_axi_rdrsp_t cnt_axi_rdrsp;
    
        cap_te_csr_cnt_tcam_req_t cnt_tcam_req;
    
        cap_te_csr_cnt_tcam_rsp_t cnt_tcam_rsp;
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_te_csr_cnt_mpu_out_t, 4> cnt_mpu_out;
        #else 
        cap_te_csr_cnt_mpu_out_t cnt_mpu_out[4];
        #endif
        int get_depth_cnt_mpu_out() { return 4; }
    
        cap_te_csr_cfg_debug_bus_t cfg_debug_bus;
    
        cap_te_csr_sta_debug_bus_t sta_debug_bus;
    
        cap_te_csr_cfg_addr_target_t cfg_addr_target;
    
        cap_te_csr_cfg_read_wait_entry_t cfg_read_wait_entry;
    
        cap_te_csr_cfg_page_size_t cfg_page_size;
    
        cap_te_csr_cfg_timeout_t cfg_timeout;
    
        cap_te_csr_sta_wait_entry_t sta_wait_entry;
    
        cap_te_csr_sta_wait_fsm_states_t sta_wait_fsm_states;
    
        cap_te_csr_sta_wait_t sta_wait;
    
        cap_te_csr_sta_axi_err_t sta_axi_err;
    
        cap_te_csr_sta_bad_axi_read_t sta_bad_axi_read;
    
        cap_te_csr_csr_intr_t csr_intr;
    
        cap_te_csr_sta_peek_t sta_peek;
    
        cap_te_csr_sta_bist_done_t sta_bist_done;
    
        cap_te_csr_cfg_bist_t cfg_bist;
    
        cap_te_csr_cfg_hbm_addr_align_t cfg_hbm_addr_align;
    
        cap_te_csr_CNT_stall_t CNT_stall;
    
        cap_te_csr_dhs_table_profile_ctrl_sram_t dhs_table_profile_ctrl_sram;
    
        cap_te_csr_intgrp_status_t int_groups;
    
        cap_te_csr_int_err_t int_err;
    
        cap_te_csr_int_info_t int_info;
    
}; // cap_te_csr_t
    
#endif // CAP_TE_CSR_H
        