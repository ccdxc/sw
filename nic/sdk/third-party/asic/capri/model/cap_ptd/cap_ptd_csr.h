
#ifndef CAP_PTD_CSR_H
#define CAP_PTD_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_ptd_csr_int_intf_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_int_intf_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_int_intf_int_enable_clear_t(string name = "cap_ptd_csr_int_intf_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_int_intf_int_enable_clear_t();
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
    
        typedef pu_cpp_int< 1 > pb_err_enable_cpp_int_t;
        cpp_int int_var__pb_err_enable;
        void pb_err_enable (const cpp_int  & l__val);
        cpp_int pb_err_enable() const;
    
        typedef pu_cpp_int< 1 > pb_sop_err_enable_cpp_int_t;
        cpp_int int_var__pb_sop_err_enable;
        void pb_sop_err_enable (const cpp_int  & l__val);
        cpp_int pb_sop_err_enable() const;
    
        typedef pu_cpp_int< 1 > pb_eop_err_enable_cpp_int_t;
        cpp_int int_var__pb_eop_err_enable;
        void pb_eop_err_enable (const cpp_int  & l__val);
        cpp_int pb_eop_err_enable() const;
    
        typedef pu_cpp_int< 1 > npv_resub_err_enable_cpp_int_t;
        cpp_int int_var__npv_resub_err_enable;
        void npv_resub_err_enable (const cpp_int  & l__val);
        cpp_int npv_resub_err_enable() const;
    
        typedef pu_cpp_int< 1 > npv_resub_sop_err_enable_cpp_int_t;
        cpp_int int_var__npv_resub_sop_err_enable;
        void npv_resub_sop_err_enable (const cpp_int  & l__val);
        cpp_int npv_resub_sop_err_enable() const;
    
        typedef pu_cpp_int< 1 > npv_resub_eop_err_enable_cpp_int_t;
        cpp_int int_var__npv_resub_eop_err_enable;
        void npv_resub_eop_err_enable (const cpp_int  & l__val);
        cpp_int npv_resub_eop_err_enable() const;
    
        typedef pu_cpp_int< 1 > ma_err_enable_cpp_int_t;
        cpp_int int_var__ma_err_enable;
        void ma_err_enable (const cpp_int  & l__val);
        cpp_int ma_err_enable() const;
    
        typedef pu_cpp_int< 1 > ma_sop_err_enable_cpp_int_t;
        cpp_int int_var__ma_sop_err_enable;
        void ma_sop_err_enable (const cpp_int  & l__val);
        cpp_int ma_sop_err_enable() const;
    
        typedef pu_cpp_int< 1 > ma_eop_err_enable_cpp_int_t;
        cpp_int int_var__ma_eop_err_enable;
        void ma_eop_err_enable (const cpp_int  & l__val);
        cpp_int ma_eop_err_enable() const;
    
}; // cap_ptd_csr_int_intf_int_enable_clear_t
    
class cap_ptd_csr_int_intf_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_int_intf_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_int_intf_int_test_set_t(string name = "cap_ptd_csr_int_intf_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_int_intf_int_test_set_t();
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
    
        typedef pu_cpp_int< 1 > pb_err_interrupt_cpp_int_t;
        cpp_int int_var__pb_err_interrupt;
        void pb_err_interrupt (const cpp_int  & l__val);
        cpp_int pb_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > pb_sop_err_interrupt_cpp_int_t;
        cpp_int int_var__pb_sop_err_interrupt;
        void pb_sop_err_interrupt (const cpp_int  & l__val);
        cpp_int pb_sop_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > pb_eop_err_interrupt_cpp_int_t;
        cpp_int int_var__pb_eop_err_interrupt;
        void pb_eop_err_interrupt (const cpp_int  & l__val);
        cpp_int pb_eop_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > npv_resub_err_interrupt_cpp_int_t;
        cpp_int int_var__npv_resub_err_interrupt;
        void npv_resub_err_interrupt (const cpp_int  & l__val);
        cpp_int npv_resub_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > npv_resub_sop_err_interrupt_cpp_int_t;
        cpp_int int_var__npv_resub_sop_err_interrupt;
        void npv_resub_sop_err_interrupt (const cpp_int  & l__val);
        cpp_int npv_resub_sop_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > npv_resub_eop_err_interrupt_cpp_int_t;
        cpp_int int_var__npv_resub_eop_err_interrupt;
        void npv_resub_eop_err_interrupt (const cpp_int  & l__val);
        cpp_int npv_resub_eop_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > ma_err_interrupt_cpp_int_t;
        cpp_int int_var__ma_err_interrupt;
        void ma_err_interrupt (const cpp_int  & l__val);
        cpp_int ma_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > ma_sop_err_interrupt_cpp_int_t;
        cpp_int int_var__ma_sop_err_interrupt;
        void ma_sop_err_interrupt (const cpp_int  & l__val);
        cpp_int ma_sop_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > ma_eop_err_interrupt_cpp_int_t;
        cpp_int int_var__ma_eop_err_interrupt;
        void ma_eop_err_interrupt (const cpp_int  & l__val);
        cpp_int ma_eop_err_interrupt() const;
    
}; // cap_ptd_csr_int_intf_int_test_set_t
    
class cap_ptd_csr_int_intf_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_int_intf_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_int_intf_t(string name = "cap_ptd_csr_int_intf_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_int_intf_t();
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
    
        cap_ptd_csr_int_intf_int_test_set_t intreg;
    
        cap_ptd_csr_int_intf_int_test_set_t int_test_set;
    
        cap_ptd_csr_int_intf_int_enable_clear_t int_enable_set;
    
        cap_ptd_csr_int_intf_int_enable_clear_t int_enable_clear;
    
}; // cap_ptd_csr_int_intf_t
    
class cap_ptd_csr_int_grp2_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_int_grp2_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_int_grp2_int_enable_clear_t(string name = "cap_ptd_csr_int_grp2_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_int_grp2_int_enable_clear_t();
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
    
        typedef pu_cpp_int< 1 > wr_invalid_cmd_enable_cpp_int_t;
        cpp_int int_var__wr_invalid_cmd_enable;
        void wr_invalid_cmd_enable (const cpp_int  & l__val);
        cpp_int wr_invalid_cmd_enable() const;
    
        typedef pu_cpp_int< 1 > wr_pend_sz_enable_cpp_int_t;
        cpp_int int_var__wr_pend_sz_enable;
        void wr_pend_sz_enable (const cpp_int  & l__val);
        cpp_int wr_pend_sz_enable() const;
    
        typedef pu_cpp_int< 1 > wr_axi_rd_resp_enable_cpp_int_t;
        cpp_int int_var__wr_axi_rd_resp_enable;
        void wr_axi_rd_resp_enable (const cpp_int  & l__val);
        cpp_int wr_axi_rd_resp_enable() const;
    
        typedef pu_cpp_int< 1 > pkt_err_eop_forced_enable_cpp_int_t;
        cpp_int int_var__pkt_err_eop_forced_enable;
        void pkt_err_eop_forced_enable (const cpp_int  & l__val);
        cpp_int pkt_err_eop_forced_enable() const;
    
        typedef pu_cpp_int< 1 > pkt_no_eop_err_seen_enable_cpp_int_t;
        cpp_int int_var__pkt_no_eop_err_seen_enable;
        void pkt_no_eop_err_seen_enable (const cpp_int  & l__val);
        cpp_int pkt_no_eop_err_seen_enable() const;
    
        typedef pu_cpp_int< 1 > pkt_cmd_eop_no_data_enable_cpp_int_t;
        cpp_int int_var__pkt_cmd_eop_no_data_enable;
        void pkt_cmd_eop_no_data_enable (const cpp_int  & l__val);
        cpp_int pkt_cmd_eop_no_data_enable() const;
    
        typedef pu_cpp_int< 1 > rdrsp_axi_enable_cpp_int_t;
        cpp_int int_var__rdrsp_axi_enable;
        void rdrsp_axi_enable (const cpp_int  & l__val);
        cpp_int rdrsp_axi_enable() const;
    
        typedef pu_cpp_int< 1 > wrrsp_axi_enable_cpp_int_t;
        cpp_int int_var__wrrsp_axi_enable;
        void wrrsp_axi_enable (const cpp_int  & l__val);
        cpp_int wrrsp_axi_enable() const;
    
        typedef pu_cpp_int< 1 > spurious_rd_resp_enable_cpp_int_t;
        cpp_int int_var__spurious_rd_resp_enable;
        void spurious_rd_resp_enable (const cpp_int  & l__val);
        cpp_int spurious_rd_resp_enable() const;
    
        typedef pu_cpp_int< 1 > spurious_wr_resp_enable_cpp_int_t;
        cpp_int int_var__spurious_wr_resp_enable;
        void spurious_wr_resp_enable (const cpp_int  & l__val);
        cpp_int spurious_wr_resp_enable() const;
    
        typedef pu_cpp_int< 1 > wrreq_info_first_missing_enable_cpp_int_t;
        cpp_int int_var__wrreq_info_first_missing_enable;
        void wrreq_info_first_missing_enable (const cpp_int  & l__val);
        cpp_int wrreq_info_first_missing_enable() const;
    
        typedef pu_cpp_int< 1 > wrreq_num_bytes_zero_enable_cpp_int_t;
        cpp_int int_var__wrreq_num_bytes_zero_enable;
        void wrreq_num_bytes_zero_enable (const cpp_int  & l__val);
        cpp_int wrreq_num_bytes_zero_enable() const;
    
        typedef pu_cpp_int< 1 > wrreq_num_bytes_more_than_64_enable_cpp_int_t;
        cpp_int int_var__wrreq_num_bytes_more_than_64_enable;
        void wrreq_num_bytes_more_than_64_enable (const cpp_int  & l__val);
        cpp_int wrreq_num_bytes_more_than_64_enable() const;
    
}; // cap_ptd_csr_int_grp2_int_enable_clear_t
    
class cap_ptd_csr_int_grp2_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_int_grp2_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_int_grp2_int_test_set_t(string name = "cap_ptd_csr_int_grp2_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_int_grp2_int_test_set_t();
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
    
        typedef pu_cpp_int< 1 > wr_invalid_cmd_interrupt_cpp_int_t;
        cpp_int int_var__wr_invalid_cmd_interrupt;
        void wr_invalid_cmd_interrupt (const cpp_int  & l__val);
        cpp_int wr_invalid_cmd_interrupt() const;
    
        typedef pu_cpp_int< 1 > wr_pend_sz_interrupt_cpp_int_t;
        cpp_int int_var__wr_pend_sz_interrupt;
        void wr_pend_sz_interrupt (const cpp_int  & l__val);
        cpp_int wr_pend_sz_interrupt() const;
    
        typedef pu_cpp_int< 1 > wr_axi_rd_resp_interrupt_cpp_int_t;
        cpp_int int_var__wr_axi_rd_resp_interrupt;
        void wr_axi_rd_resp_interrupt (const cpp_int  & l__val);
        cpp_int wr_axi_rd_resp_interrupt() const;
    
        typedef pu_cpp_int< 1 > pkt_err_eop_forced_interrupt_cpp_int_t;
        cpp_int int_var__pkt_err_eop_forced_interrupt;
        void pkt_err_eop_forced_interrupt (const cpp_int  & l__val);
        cpp_int pkt_err_eop_forced_interrupt() const;
    
        typedef pu_cpp_int< 1 > pkt_no_eop_err_seen_interrupt_cpp_int_t;
        cpp_int int_var__pkt_no_eop_err_seen_interrupt;
        void pkt_no_eop_err_seen_interrupt (const cpp_int  & l__val);
        cpp_int pkt_no_eop_err_seen_interrupt() const;
    
        typedef pu_cpp_int< 1 > pkt_cmd_eop_no_data_interrupt_cpp_int_t;
        cpp_int int_var__pkt_cmd_eop_no_data_interrupt;
        void pkt_cmd_eop_no_data_interrupt (const cpp_int  & l__val);
        cpp_int pkt_cmd_eop_no_data_interrupt() const;
    
        typedef pu_cpp_int< 1 > rdrsp_axi_interrupt_cpp_int_t;
        cpp_int int_var__rdrsp_axi_interrupt;
        void rdrsp_axi_interrupt (const cpp_int  & l__val);
        cpp_int rdrsp_axi_interrupt() const;
    
        typedef pu_cpp_int< 1 > wrrsp_axi_interrupt_cpp_int_t;
        cpp_int int_var__wrrsp_axi_interrupt;
        void wrrsp_axi_interrupt (const cpp_int  & l__val);
        cpp_int wrrsp_axi_interrupt() const;
    
        typedef pu_cpp_int< 1 > spurious_rd_resp_interrupt_cpp_int_t;
        cpp_int int_var__spurious_rd_resp_interrupt;
        void spurious_rd_resp_interrupt (const cpp_int  & l__val);
        cpp_int spurious_rd_resp_interrupt() const;
    
        typedef pu_cpp_int< 1 > spurious_wr_resp_interrupt_cpp_int_t;
        cpp_int int_var__spurious_wr_resp_interrupt;
        void spurious_wr_resp_interrupt (const cpp_int  & l__val);
        cpp_int spurious_wr_resp_interrupt() const;
    
        typedef pu_cpp_int< 1 > wrreq_info_first_missing_interrupt_cpp_int_t;
        cpp_int int_var__wrreq_info_first_missing_interrupt;
        void wrreq_info_first_missing_interrupt (const cpp_int  & l__val);
        cpp_int wrreq_info_first_missing_interrupt() const;
    
        typedef pu_cpp_int< 1 > wrreq_num_bytes_zero_interrupt_cpp_int_t;
        cpp_int int_var__wrreq_num_bytes_zero_interrupt;
        void wrreq_num_bytes_zero_interrupt (const cpp_int  & l__val);
        cpp_int wrreq_num_bytes_zero_interrupt() const;
    
        typedef pu_cpp_int< 1 > wrreq_num_bytes_more_than_64_interrupt_cpp_int_t;
        cpp_int int_var__wrreq_num_bytes_more_than_64_interrupt;
        void wrreq_num_bytes_more_than_64_interrupt (const cpp_int  & l__val);
        cpp_int wrreq_num_bytes_more_than_64_interrupt() const;
    
}; // cap_ptd_csr_int_grp2_int_test_set_t
    
class cap_ptd_csr_int_grp2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_int_grp2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_int_grp2_t(string name = "cap_ptd_csr_int_grp2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_int_grp2_t();
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
    
        cap_ptd_csr_int_grp2_int_test_set_t intreg;
    
        cap_ptd_csr_int_grp2_int_test_set_t int_test_set;
    
        cap_ptd_csr_int_grp2_int_enable_clear_t int_enable_set;
    
        cap_ptd_csr_int_grp2_int_enable_clear_t int_enable_clear;
    
}; // cap_ptd_csr_int_grp2_t
    
class cap_ptd_csr_int_grp1_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_int_grp1_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_int_grp1_int_enable_clear_t(string name = "cap_ptd_csr_int_grp1_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_int_grp1_int_enable_clear_t();
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
    
        typedef pu_cpp_int< 1 > rcv_phv_dma_ptr_enable_cpp_int_t;
        cpp_int int_var__rcv_phv_dma_ptr_enable;
        void rcv_phv_dma_ptr_enable (const cpp_int  & l__val);
        cpp_int rcv_phv_dma_ptr_enable() const;
    
        typedef pu_cpp_int< 1 > rcv_phv_addr_enable_cpp_int_t;
        cpp_int int_var__rcv_phv_addr_enable;
        void rcv_phv_addr_enable (const cpp_int  & l__val);
        cpp_int rcv_phv_addr_enable() const;
    
        typedef pu_cpp_int< 1 > rcv_exceed_16byte_enable_cpp_int_t;
        cpp_int int_var__rcv_exceed_16byte_enable;
        void rcv_exceed_16byte_enable (const cpp_int  & l__val);
        cpp_int rcv_exceed_16byte_enable() const;
    
        typedef pu_cpp_int< 1 > rcv_phv_not_sop_enable_cpp_int_t;
        cpp_int int_var__rcv_phv_not_sop_enable;
        void rcv_phv_not_sop_enable (const cpp_int  & l__val);
        cpp_int rcv_phv_not_sop_enable() const;
    
        typedef pu_cpp_int< 1 > rcv_pkt_order_ff_full_enable_cpp_int_t;
        cpp_int int_var__rcv_pkt_order_ff_full_enable;
        void rcv_pkt_order_ff_full_enable (const cpp_int  & l__val);
        cpp_int rcv_pkt_order_ff_full_enable() const;
    
        typedef pu_cpp_int< 1 > rcv_pend_phv_more_than_2_enable_cpp_int_t;
        cpp_int int_var__rcv_pend_phv_more_than_2_enable;
        void rcv_pend_phv_more_than_2_enable (const cpp_int  & l__val);
        cpp_int rcv_pend_phv_more_than_2_enable() const;
    
        typedef pu_cpp_int< 1 > rcv_pend_phv_less_enable_cpp_int_t;
        cpp_int int_var__rcv_pend_phv_less_enable;
        void rcv_pend_phv_less_enable (const cpp_int  & l__val);
        cpp_int rcv_pend_phv_less_enable() const;
    
        typedef pu_cpp_int< 1 > rcv_cmd_nop_eop_enable_cpp_int_t;
        cpp_int int_var__rcv_cmd_nop_eop_enable;
        void rcv_cmd_nop_eop_enable (const cpp_int  & l__val);
        cpp_int rcv_cmd_nop_eop_enable() const;
    
        typedef pu_cpp_int< 1 > rcv_cmd_out_not_sop_enable_cpp_int_t;
        cpp_int int_var__rcv_cmd_out_not_sop_enable;
        void rcv_cmd_out_not_sop_enable (const cpp_int  & l__val);
        cpp_int rcv_cmd_out_not_sop_enable() const;
    
        typedef pu_cpp_int< 1 > rcv_pkt2mem_seen_enable_cpp_int_t;
        cpp_int int_var__rcv_pkt2mem_seen_enable;
        void rcv_pkt2mem_seen_enable (const cpp_int  & l__val);
        cpp_int rcv_pkt2mem_seen_enable() const;
    
        typedef pu_cpp_int< 1 > rcv_skip_seen_enable_cpp_int_t;
        cpp_int int_var__rcv_skip_seen_enable;
        void rcv_skip_seen_enable (const cpp_int  & l__val);
        cpp_int rcv_skip_seen_enable() const;
    
        typedef pu_cpp_int< 1 > rcv_phv_eop_no_cmd_eop_enable_cpp_int_t;
        cpp_int int_var__rcv_phv_eop_no_cmd_eop_enable;
        void rcv_phv_eop_no_cmd_eop_enable (const cpp_int  & l__val);
        cpp_int rcv_phv_eop_no_cmd_eop_enable() const;
    
        typedef pu_cpp_int< 1 > rcv_m2m_dst_not_seen_enable_cpp_int_t;
        cpp_int int_var__rcv_m2m_dst_not_seen_enable;
        void rcv_m2m_dst_not_seen_enable (const cpp_int  & l__val);
        cpp_int rcv_m2m_dst_not_seen_enable() const;
    
        typedef pu_cpp_int< 1 > rcv_m2m_src_not_seen_enable_cpp_int_t;
        cpp_int int_var__rcv_m2m_src_not_seen_enable;
        void rcv_m2m_src_not_seen_enable (const cpp_int  & l__val);
        cpp_int rcv_m2m_src_not_seen_enable() const;
    
        typedef pu_cpp_int< 1 > rdreq_invalid_cmd_seen_enable_cpp_int_t;
        cpp_int int_var__rdreq_invalid_cmd_seen_enable;
        void rdreq_invalid_cmd_seen_enable (const cpp_int  & l__val);
        cpp_int rdreq_invalid_cmd_seen_enable() const;
    
        typedef pu_cpp_int< 1 > rdreq_mem2mem_psize_zero_enable_cpp_int_t;
        cpp_int int_var__rdreq_mem2mem_psize_zero_enable;
        void rdreq_mem2mem_psize_zero_enable (const cpp_int  & l__val);
        cpp_int rdreq_mem2mem_psize_zero_enable() const;
    
        typedef pu_cpp_int< 1 > rdreq_m2m_phv2mem_exceed_16byte_enable_cpp_int_t;
        cpp_int int_var__rdreq_m2m_phv2mem_exceed_16byte_enable;
        void rdreq_m2m_phv2mem_exceed_16byte_enable (const cpp_int  & l__val);
        cpp_int rdreq_m2m_phv2mem_exceed_16byte_enable() const;
    
        typedef pu_cpp_int< 1 > rdreq_phv2mem_fence_exceed_16byte_enable_cpp_int_t;
        cpp_int int_var__rdreq_phv2mem_fence_exceed_16byte_enable;
        void rdreq_phv2mem_fence_exceed_16byte_enable (const cpp_int  & l__val);
        cpp_int rdreq_phv2mem_fence_exceed_16byte_enable() const;
    
        typedef pu_cpp_int< 1 > rdreq_mem2pkt_psize_zero_enable_cpp_int_t;
        cpp_int int_var__rdreq_mem2pkt_psize_zero_enable;
        void rdreq_mem2pkt_psize_zero_enable (const cpp_int  & l__val);
        cpp_int rdreq_mem2pkt_psize_zero_enable() const;
    
}; // cap_ptd_csr_int_grp1_int_enable_clear_t
    
class cap_ptd_csr_int_grp1_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_int_grp1_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_int_grp1_int_test_set_t(string name = "cap_ptd_csr_int_grp1_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_int_grp1_int_test_set_t();
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
    
        typedef pu_cpp_int< 1 > rcv_phv_dma_ptr_interrupt_cpp_int_t;
        cpp_int int_var__rcv_phv_dma_ptr_interrupt;
        void rcv_phv_dma_ptr_interrupt (const cpp_int  & l__val);
        cpp_int rcv_phv_dma_ptr_interrupt() const;
    
        typedef pu_cpp_int< 1 > rcv_phv_addr_interrupt_cpp_int_t;
        cpp_int int_var__rcv_phv_addr_interrupt;
        void rcv_phv_addr_interrupt (const cpp_int  & l__val);
        cpp_int rcv_phv_addr_interrupt() const;
    
        typedef pu_cpp_int< 1 > rcv_exceed_16byte_interrupt_cpp_int_t;
        cpp_int int_var__rcv_exceed_16byte_interrupt;
        void rcv_exceed_16byte_interrupt (const cpp_int  & l__val);
        cpp_int rcv_exceed_16byte_interrupt() const;
    
        typedef pu_cpp_int< 1 > rcv_phv_not_sop_interrupt_cpp_int_t;
        cpp_int int_var__rcv_phv_not_sop_interrupt;
        void rcv_phv_not_sop_interrupt (const cpp_int  & l__val);
        cpp_int rcv_phv_not_sop_interrupt() const;
    
        typedef pu_cpp_int< 1 > rcv_pkt_order_ff_full_interrupt_cpp_int_t;
        cpp_int int_var__rcv_pkt_order_ff_full_interrupt;
        void rcv_pkt_order_ff_full_interrupt (const cpp_int  & l__val);
        cpp_int rcv_pkt_order_ff_full_interrupt() const;
    
        typedef pu_cpp_int< 1 > rcv_pend_phv_more_than_2_interrupt_cpp_int_t;
        cpp_int int_var__rcv_pend_phv_more_than_2_interrupt;
        void rcv_pend_phv_more_than_2_interrupt (const cpp_int  & l__val);
        cpp_int rcv_pend_phv_more_than_2_interrupt() const;
    
        typedef pu_cpp_int< 1 > rcv_pend_phv_less_interrupt_cpp_int_t;
        cpp_int int_var__rcv_pend_phv_less_interrupt;
        void rcv_pend_phv_less_interrupt (const cpp_int  & l__val);
        cpp_int rcv_pend_phv_less_interrupt() const;
    
        typedef pu_cpp_int< 1 > rcv_cmd_nop_eop_interrupt_cpp_int_t;
        cpp_int int_var__rcv_cmd_nop_eop_interrupt;
        void rcv_cmd_nop_eop_interrupt (const cpp_int  & l__val);
        cpp_int rcv_cmd_nop_eop_interrupt() const;
    
        typedef pu_cpp_int< 1 > rcv_cmd_out_not_sop_interrupt_cpp_int_t;
        cpp_int int_var__rcv_cmd_out_not_sop_interrupt;
        void rcv_cmd_out_not_sop_interrupt (const cpp_int  & l__val);
        cpp_int rcv_cmd_out_not_sop_interrupt() const;
    
        typedef pu_cpp_int< 1 > rcv_pkt2mem_seen_interrupt_cpp_int_t;
        cpp_int int_var__rcv_pkt2mem_seen_interrupt;
        void rcv_pkt2mem_seen_interrupt (const cpp_int  & l__val);
        cpp_int rcv_pkt2mem_seen_interrupt() const;
    
        typedef pu_cpp_int< 1 > rcv_skip_seen_interrupt_cpp_int_t;
        cpp_int int_var__rcv_skip_seen_interrupt;
        void rcv_skip_seen_interrupt (const cpp_int  & l__val);
        cpp_int rcv_skip_seen_interrupt() const;
    
        typedef pu_cpp_int< 1 > rcv_phv_eop_no_cmd_eop_interrupt_cpp_int_t;
        cpp_int int_var__rcv_phv_eop_no_cmd_eop_interrupt;
        void rcv_phv_eop_no_cmd_eop_interrupt (const cpp_int  & l__val);
        cpp_int rcv_phv_eop_no_cmd_eop_interrupt() const;
    
        typedef pu_cpp_int< 1 > rcv_m2m_dst_not_seen_interrupt_cpp_int_t;
        cpp_int int_var__rcv_m2m_dst_not_seen_interrupt;
        void rcv_m2m_dst_not_seen_interrupt (const cpp_int  & l__val);
        cpp_int rcv_m2m_dst_not_seen_interrupt() const;
    
        typedef pu_cpp_int< 1 > rcv_m2m_src_not_seen_interrupt_cpp_int_t;
        cpp_int int_var__rcv_m2m_src_not_seen_interrupt;
        void rcv_m2m_src_not_seen_interrupt (const cpp_int  & l__val);
        cpp_int rcv_m2m_src_not_seen_interrupt() const;
    
        typedef pu_cpp_int< 1 > rdreq_invalid_cmd_seen_interrupt_cpp_int_t;
        cpp_int int_var__rdreq_invalid_cmd_seen_interrupt;
        void rdreq_invalid_cmd_seen_interrupt (const cpp_int  & l__val);
        cpp_int rdreq_invalid_cmd_seen_interrupt() const;
    
        typedef pu_cpp_int< 1 > rdreq_mem2mem_psize_zero_interrupt_cpp_int_t;
        cpp_int int_var__rdreq_mem2mem_psize_zero_interrupt;
        void rdreq_mem2mem_psize_zero_interrupt (const cpp_int  & l__val);
        cpp_int rdreq_mem2mem_psize_zero_interrupt() const;
    
        typedef pu_cpp_int< 1 > rdreq_m2m_phv2mem_exceed_16byte_interrupt_cpp_int_t;
        cpp_int int_var__rdreq_m2m_phv2mem_exceed_16byte_interrupt;
        void rdreq_m2m_phv2mem_exceed_16byte_interrupt (const cpp_int  & l__val);
        cpp_int rdreq_m2m_phv2mem_exceed_16byte_interrupt() const;
    
        typedef pu_cpp_int< 1 > rdreq_phv2mem_fence_exceed_16byte_interrupt_cpp_int_t;
        cpp_int int_var__rdreq_phv2mem_fence_exceed_16byte_interrupt;
        void rdreq_phv2mem_fence_exceed_16byte_interrupt (const cpp_int  & l__val);
        cpp_int rdreq_phv2mem_fence_exceed_16byte_interrupt() const;
    
        typedef pu_cpp_int< 1 > rdreq_mem2pkt_psize_zero_interrupt_cpp_int_t;
        cpp_int int_var__rdreq_mem2pkt_psize_zero_interrupt;
        void rdreq_mem2pkt_psize_zero_interrupt (const cpp_int  & l__val);
        cpp_int rdreq_mem2pkt_psize_zero_interrupt() const;
    
}; // cap_ptd_csr_int_grp1_int_test_set_t
    
class cap_ptd_csr_int_grp1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_int_grp1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_int_grp1_t(string name = "cap_ptd_csr_int_grp1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_int_grp1_t();
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
    
        cap_ptd_csr_int_grp1_int_test_set_t intreg;
    
        cap_ptd_csr_int_grp1_int_test_set_t int_test_set;
    
        cap_ptd_csr_int_grp1_int_enable_clear_t int_enable_set;
    
        cap_ptd_csr_int_grp1_int_enable_clear_t int_enable_clear;
    
}; // cap_ptd_csr_int_grp1_t
    
class cap_ptd_csr_int_fifo_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_int_fifo_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_int_fifo_int_enable_clear_t(string name = "cap_ptd_csr_int_fifo_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_int_fifo_int_enable_clear_t();
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
    
        typedef pu_cpp_int< 1 > lat_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__lat_ff_ovflow_enable;
        void lat_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int lat_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > wdata_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__wdata_ff_ovflow_enable;
        void wdata_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int wdata_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > rcv_stg_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__rcv_stg_ff_ovflow_enable;
        void rcv_stg_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int rcv_stg_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > cmdflit_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__cmdflit_ff_ovflow_enable;
        void cmdflit_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int cmdflit_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > cmd_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__cmd_ff_ovflow_enable;
        void cmd_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int cmd_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pkt_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pkt_ff_ovflow_enable;
        void pkt_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pkt_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > wr_mem_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__wr_mem_ff_ovflow_enable;
        void wr_mem_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int wr_mem_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > dfence_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__dfence_ff_ovflow_enable;
        void dfence_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int dfence_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > ffence_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__ffence_ff_ovflow_enable;
        void ffence_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int ffence_ff_ovflow_enable() const;
    
}; // cap_ptd_csr_int_fifo_int_enable_clear_t
    
class cap_ptd_csr_int_fifo_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_int_fifo_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_int_fifo_int_test_set_t(string name = "cap_ptd_csr_int_fifo_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_int_fifo_int_test_set_t();
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
    
        typedef pu_cpp_int< 1 > lat_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__lat_ff_ovflow_interrupt;
        void lat_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int lat_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > wdata_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__wdata_ff_ovflow_interrupt;
        void wdata_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int wdata_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > rcv_stg_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__rcv_stg_ff_ovflow_interrupt;
        void rcv_stg_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int rcv_stg_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > cmdflit_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__cmdflit_ff_ovflow_interrupt;
        void cmdflit_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int cmdflit_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > cmd_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__cmd_ff_ovflow_interrupt;
        void cmd_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int cmd_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pkt_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pkt_ff_ovflow_interrupt;
        void pkt_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pkt_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > wr_mem_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__wr_mem_ff_ovflow_interrupt;
        void wr_mem_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int wr_mem_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > dfence_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__dfence_ff_ovflow_interrupt;
        void dfence_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int dfence_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > ffence_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__ffence_ff_ovflow_interrupt;
        void ffence_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int ffence_ff_ovflow_interrupt() const;
    
}; // cap_ptd_csr_int_fifo_int_test_set_t
    
class cap_ptd_csr_int_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_int_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_int_fifo_t(string name = "cap_ptd_csr_int_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_int_fifo_t();
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
    
        cap_ptd_csr_int_fifo_int_test_set_t intreg;
    
        cap_ptd_csr_int_fifo_int_test_set_t int_test_set;
    
        cap_ptd_csr_int_fifo_int_enable_clear_t int_enable_set;
    
        cap_ptd_csr_int_fifo_int_enable_clear_t int_enable_clear;
    
}; // cap_ptd_csr_int_fifo_t
    
class cap_ptd_csr_int_ecc_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_int_ecc_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_int_ecc_int_enable_clear_t(string name = "cap_ptd_csr_int_ecc_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_int_ecc_int_enable_clear_t();
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
    
        typedef pu_cpp_int< 1 > rdata_mem_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__rdata_mem_uncorrectable_enable;
        void rdata_mem_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int rdata_mem_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > rdata_mem_correctable_enable_cpp_int_t;
        cpp_int int_var__rdata_mem_correctable_enable;
        void rdata_mem_correctable_enable (const cpp_int  & l__val);
        cpp_int rdata_mem_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > lat_mem_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__lat_mem_uncorrectable_enable;
        void lat_mem_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int lat_mem_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > lat_mem_correctable_enable_cpp_int_t;
        cpp_int int_var__lat_mem_correctable_enable;
        void lat_mem_correctable_enable (const cpp_int  & l__val);
        cpp_int lat_mem_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > wdata_mem_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__wdata_mem_uncorrectable_enable;
        void wdata_mem_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int wdata_mem_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > wdata_mem_correctable_enable_cpp_int_t;
        cpp_int int_var__wdata_mem_correctable_enable;
        void wdata_mem_correctable_enable (const cpp_int  & l__val);
        cpp_int wdata_mem_correctable_enable() const;
    
}; // cap_ptd_csr_int_ecc_int_enable_clear_t
    
class cap_ptd_csr_int_ecc_intreg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_int_ecc_intreg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_int_ecc_intreg_t(string name = "cap_ptd_csr_int_ecc_intreg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_int_ecc_intreg_t();
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
    
        typedef pu_cpp_int< 1 > rdata_mem_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__rdata_mem_uncorrectable_interrupt;
        void rdata_mem_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int rdata_mem_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > rdata_mem_correctable_interrupt_cpp_int_t;
        cpp_int int_var__rdata_mem_correctable_interrupt;
        void rdata_mem_correctable_interrupt (const cpp_int  & l__val);
        cpp_int rdata_mem_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > lat_mem_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__lat_mem_uncorrectable_interrupt;
        void lat_mem_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int lat_mem_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > lat_mem_correctable_interrupt_cpp_int_t;
        cpp_int int_var__lat_mem_correctable_interrupt;
        void lat_mem_correctable_interrupt (const cpp_int  & l__val);
        cpp_int lat_mem_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > wdata_mem_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__wdata_mem_uncorrectable_interrupt;
        void wdata_mem_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int wdata_mem_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > wdata_mem_correctable_interrupt_cpp_int_t;
        cpp_int int_var__wdata_mem_correctable_interrupt;
        void wdata_mem_correctable_interrupt (const cpp_int  & l__val);
        cpp_int wdata_mem_correctable_interrupt() const;
    
}; // cap_ptd_csr_int_ecc_intreg_t
    
class cap_ptd_csr_int_ecc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_int_ecc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_int_ecc_t(string name = "cap_ptd_csr_int_ecc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_int_ecc_t();
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
    
        cap_ptd_csr_int_ecc_intreg_t intreg;
    
        cap_ptd_csr_int_ecc_intreg_t int_test_set;
    
        cap_ptd_csr_int_ecc_int_enable_clear_t int_enable_set;
    
        cap_ptd_csr_int_ecc_int_enable_clear_t int_enable_clear;
    
}; // cap_ptd_csr_int_ecc_t
    
class cap_ptd_csr_intreg_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_intreg_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_intreg_status_t(string name = "cap_ptd_csr_intreg_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_intreg_status_t();
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
    
        typedef pu_cpp_int< 1 > int_ecc_interrupt_cpp_int_t;
        cpp_int int_var__int_ecc_interrupt;
        void int_ecc_interrupt (const cpp_int  & l__val);
        cpp_int int_ecc_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_fifo_interrupt_cpp_int_t;
        cpp_int int_var__int_fifo_interrupt;
        void int_fifo_interrupt (const cpp_int  & l__val);
        cpp_int int_fifo_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_grp1_interrupt_cpp_int_t;
        cpp_int int_var__int_grp1_interrupt;
        void int_grp1_interrupt (const cpp_int  & l__val);
        cpp_int int_grp1_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_grp2_interrupt_cpp_int_t;
        cpp_int int_var__int_grp2_interrupt;
        void int_grp2_interrupt (const cpp_int  & l__val);
        cpp_int int_grp2_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_intf_interrupt_cpp_int_t;
        cpp_int int_var__int_intf_interrupt;
        void int_intf_interrupt (const cpp_int  & l__val);
        cpp_int int_intf_interrupt() const;
    
}; // cap_ptd_csr_intreg_status_t
    
class cap_ptd_csr_int_groups_int_enable_rw_reg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_int_groups_int_enable_rw_reg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_int_groups_int_enable_rw_reg_t(string name = "cap_ptd_csr_int_groups_int_enable_rw_reg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_int_groups_int_enable_rw_reg_t();
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
    
        typedef pu_cpp_int< 1 > int_ecc_enable_cpp_int_t;
        cpp_int int_var__int_ecc_enable;
        void int_ecc_enable (const cpp_int  & l__val);
        cpp_int int_ecc_enable() const;
    
        typedef pu_cpp_int< 1 > int_fifo_enable_cpp_int_t;
        cpp_int int_var__int_fifo_enable;
        void int_fifo_enable (const cpp_int  & l__val);
        cpp_int int_fifo_enable() const;
    
        typedef pu_cpp_int< 1 > int_grp1_enable_cpp_int_t;
        cpp_int int_var__int_grp1_enable;
        void int_grp1_enable (const cpp_int  & l__val);
        cpp_int int_grp1_enable() const;
    
        typedef pu_cpp_int< 1 > int_grp2_enable_cpp_int_t;
        cpp_int int_var__int_grp2_enable;
        void int_grp2_enable (const cpp_int  & l__val);
        cpp_int int_grp2_enable() const;
    
        typedef pu_cpp_int< 1 > int_intf_enable_cpp_int_t;
        cpp_int int_var__int_intf_enable;
        void int_intf_enable (const cpp_int  & l__val);
        cpp_int int_intf_enable() const;
    
}; // cap_ptd_csr_int_groups_int_enable_rw_reg_t
    
class cap_ptd_csr_intgrp_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_intgrp_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_intgrp_status_t(string name = "cap_ptd_csr_intgrp_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_intgrp_status_t();
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
    
        cap_ptd_csr_intreg_status_t intreg;
    
        cap_ptd_csr_int_groups_int_enable_rw_reg_t int_enable_rw_reg;
    
        cap_ptd_csr_intreg_status_t int_rw_reg;
    
}; // cap_ptd_csr_intgrp_status_t
    
class cap_ptd_csr_dhs_dbg_mem_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_dhs_dbg_mem_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_dhs_dbg_mem_entry_t(string name = "cap_ptd_csr_dhs_dbg_mem_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_dhs_dbg_mem_entry_t();
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
    
        typedef pu_cpp_int< 532 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // cap_ptd_csr_dhs_dbg_mem_entry_t
    
class cap_ptd_csr_dhs_dbg_mem_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_dhs_dbg_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_dhs_dbg_mem_t(string name = "cap_ptd_csr_dhs_dbg_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_dhs_dbg_mem_t();
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
    
        cap_ptd_csr_dhs_dbg_mem_entry_t entry;
    
}; // cap_ptd_csr_dhs_dbg_mem_t
    
class cap_ptd_csr_CNT_axi_rd_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_CNT_axi_rd_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_CNT_axi_rd_t(string name = "cap_ptd_csr_CNT_axi_rd_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_CNT_axi_rd_t();
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
    
        typedef pu_cpp_int< 16 > req_vld_cpp_int_t;
        cpp_int int_var__req_vld;
        void req_vld (const cpp_int  & l__val);
        cpp_int req_vld() const;
    
        typedef pu_cpp_int< 16 > rsp_vld_cpp_int_t;
        cpp_int int_var__rsp_vld;
        void rsp_vld (const cpp_int  & l__val);
        cpp_int rsp_vld() const;
    
}; // cap_ptd_csr_CNT_axi_rd_t
    
class cap_ptd_csr_CNT_axi_wr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_CNT_axi_wr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_CNT_axi_wr_t(string name = "cap_ptd_csr_CNT_axi_wr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_CNT_axi_wr_t();
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
    
        typedef pu_cpp_int< 16 > req_vld_cpp_int_t;
        cpp_int int_var__req_vld;
        void req_vld (const cpp_int  & l__val);
        cpp_int req_vld() const;
    
        typedef pu_cpp_int< 16 > rsp_vld_cpp_int_t;
        cpp_int int_var__rsp_vld;
        void rsp_vld (const cpp_int  & l__val);
        cpp_int rsp_vld() const;
    
}; // cap_ptd_csr_CNT_axi_wr_t
    
class cap_ptd_csr_cfg_dhs_dbg_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_cfg_dhs_dbg_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_cfg_dhs_dbg_mem_t(string name = "cap_ptd_csr_cfg_dhs_dbg_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_cfg_dhs_dbg_mem_t();
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
    
        typedef pu_cpp_int< 16 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 1 > eccbypass_cpp_int_t;
        cpp_int int_var__eccbypass;
        void eccbypass (const cpp_int  & l__val);
        cpp_int eccbypass() const;
    
}; // cap_ptd_csr_cfg_dhs_dbg_mem_t
    
class cap_ptd_csr_cfg_debug_bus_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_cfg_debug_bus_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_cfg_debug_bus_t(string name = "cap_ptd_csr_cfg_debug_bus_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_cfg_debug_bus_t();
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
    
        typedef pu_cpp_int< 256 > bkp_cpp_int_t;
        cpp_int int_var__bkp;
        void bkp (const cpp_int  & l__val);
        cpp_int bkp() const;
    
}; // cap_ptd_csr_cfg_debug_bus_t
    
class cap_ptd_csr_cfg_debug_ctrl_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_cfg_debug_ctrl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_cfg_debug_ctrl_t(string name = "cap_ptd_csr_cfg_debug_ctrl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_cfg_debug_ctrl_t();
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
    
        typedef pu_cpp_int< 1 > deq_stall_en_cpp_int_t;
        cpp_int int_var__deq_stall_en;
        void deq_stall_en (const cpp_int  & l__val);
        cpp_int deq_stall_en() const;
    
        typedef pu_cpp_int< 3 > dbg_mem_sel_cpp_int_t;
        cpp_int int_var__dbg_mem_sel;
        void dbg_mem_sel (const cpp_int  & l__val);
        cpp_int dbg_mem_sel() const;
    
        typedef pu_cpp_int< 2 > dbg_bus_sel_cpp_int_t;
        cpp_int int_var__dbg_bus_sel;
        void dbg_bus_sel (const cpp_int  & l__val);
        cpp_int dbg_bus_sel() const;
    
        typedef pu_cpp_int< 3 > cmn_dbg_sel_cpp_int_t;
        cpp_int int_var__cmn_dbg_sel;
        void cmn_dbg_sel (const cpp_int  & l__val);
        cpp_int cmn_dbg_sel() const;
    
}; // cap_ptd_csr_cfg_debug_ctrl_t
    
class cap_ptd_csr_sta_xoff_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_sta_xoff_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_sta_xoff_t(string name = "cap_ptd_csr_sta_xoff_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_sta_xoff_t();
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
    
        typedef pu_cpp_int< 11 > numphv_counter_cpp_int_t;
        cpp_int int_var__numphv_counter;
        void numphv_counter (const cpp_int  & l__val);
        cpp_int numphv_counter() const;
    
        typedef pu_cpp_int< 1 > numphv_xoff_cpp_int_t;
        cpp_int int_var__numphv_xoff;
        void numphv_xoff (const cpp_int  & l__val);
        cpp_int numphv_xoff() const;
    
}; // cap_ptd_csr_sta_xoff_t
    
class cap_ptd_csr_sta_wrreq_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_sta_wrreq_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_sta_wrreq_t(string name = "cap_ptd_csr_sta_wrreq_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_sta_wrreq_t();
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
    
        typedef pu_cpp_int< 7 > spurious_id_cpp_int_t;
        cpp_int int_var__spurious_id;
        void spurious_id (const cpp_int  & l__val);
        cpp_int spurious_id() const;
    
        typedef pu_cpp_int< 3 > err_vec_cpp_int_t;
        cpp_int int_var__err_vec;
        void err_vec (const cpp_int  & l__val);
        cpp_int err_vec() const;
    
        typedef pu_cpp_int< 2 > err_log_cpp_int_t;
        cpp_int int_var__err_log;
        void err_log (const cpp_int  & l__val);
        cpp_int err_log() const;
    
        typedef pu_cpp_int< 2 > wrreq_state_cpp_int_t;
        cpp_int int_var__wrreq_state;
        void wrreq_state (const cpp_int  & l__val);
        cpp_int wrreq_state() const;
    
        typedef pu_cpp_int< 1 > dfence_state_cpp_int_t;
        cpp_int int_var__dfence_state;
        void dfence_state (const cpp_int  & l__val);
        cpp_int dfence_state() const;
    
        typedef pu_cpp_int< 1 > ffence_state_cpp_int_t;
        cpp_int int_var__ffence_state;
        void ffence_state (const cpp_int  & l__val);
        cpp_int ffence_state() const;
    
}; // cap_ptd_csr_sta_wrreq_t
    
class cap_ptd_csr_sta_pkt_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_sta_pkt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_sta_pkt_t(string name = "cap_ptd_csr_sta_pkt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_sta_pkt_t();
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
    
        typedef pu_cpp_int< 1 > state_cpp_int_t;
        cpp_int int_var__state;
        void state (const cpp_int  & l__val);
        cpp_int state() const;
    
}; // cap_ptd_csr_sta_pkt_t
    
class cap_ptd_csr_sta_rd_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_sta_rd_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_sta_rd_t(string name = "cap_ptd_csr_sta_rd_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_sta_rd_t();
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
    
        typedef pu_cpp_int< 7 > spurious_id_cpp_int_t;
        cpp_int int_var__spurious_id;
        void spurious_id (const cpp_int  & l__val);
        cpp_int spurious_id() const;
    
}; // cap_ptd_csr_sta_rd_t
    
class cap_ptd_csr_sta_wrrsp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_sta_wrrsp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_sta_wrrsp_t(string name = "cap_ptd_csr_sta_wrrsp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_sta_wrrsp_t();
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
    
        typedef pu_cpp_int< 7 > axi_wr_id_cpp_int_t;
        cpp_int int_var__axi_wr_id;
        void axi_wr_id (const cpp_int  & l__val);
        cpp_int axi_wr_id() const;
    
        typedef pu_cpp_int< 2 > axi_wr_resp_cpp_int_t;
        cpp_int int_var__axi_wr_resp;
        void axi_wr_resp (const cpp_int  & l__val);
        cpp_int axi_wr_resp() const;
    
}; // cap_ptd_csr_sta_wrrsp_t
    
class cap_ptd_csr_sta_rdrsp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_sta_rdrsp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_sta_rdrsp_t(string name = "cap_ptd_csr_sta_rdrsp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_sta_rdrsp_t();
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
    
        typedef pu_cpp_int< 7 > axi_rd_id_cpp_int_t;
        cpp_int int_var__axi_rd_id;
        void axi_rd_id (const cpp_int  & l__val);
        cpp_int axi_rd_id() const;
    
        typedef pu_cpp_int< 2 > axi_rd_resp_cpp_int_t;
        cpp_int int_var__axi_rd_resp;
        void axi_rd_resp (const cpp_int  & l__val);
        cpp_int axi_rd_resp() const;
    
        typedef pu_cpp_int< 1 > rsp_state_cpp_int_t;
        cpp_int int_var__rsp_state;
        void rsp_state (const cpp_int  & l__val);
        cpp_int rsp_state() const;
    
}; // cap_ptd_csr_sta_rdrsp_t
    
class cap_ptd_csr_CNT_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_CNT_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_CNT_err_t(string name = "cap_ptd_csr_CNT_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_CNT_err_t();
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
    
        typedef pu_cpp_int< 8 > wr_lat_drop_cpp_int_t;
        cpp_int int_var__wr_lat_drop;
        void wr_lat_drop (const cpp_int  & l__val);
        cpp_int wr_lat_drop() const;
    
        typedef pu_cpp_int< 8 > wr_wdata_drop_cpp_int_t;
        cpp_int int_var__wr_wdata_drop;
        void wr_wdata_drop (const cpp_int  & l__val);
        cpp_int wr_wdata_drop() const;
    
        typedef pu_cpp_int< 8 > pkt_force_eop_cpp_int_t;
        cpp_int int_var__pkt_force_eop;
        void pkt_force_eop (const cpp_int  & l__val);
        cpp_int pkt_force_eop() const;
    
        typedef pu_cpp_int< 8 > pkt_no_eop_seen_cpp_int_t;
        cpp_int int_var__pkt_no_eop_seen;
        void pkt_no_eop_seen (const cpp_int  & l__val);
        cpp_int pkt_no_eop_seen() const;
    
        typedef pu_cpp_int< 8 > pkt_cmd_eop_no_data_cpp_int_t;
        cpp_int int_var__pkt_cmd_eop_no_data;
        void pkt_cmd_eop_no_data (const cpp_int  & l__val);
        cpp_int pkt_cmd_eop_no_data() const;
    
}; // cap_ptd_csr_CNT_err_t
    
class cap_ptd_csr_sta_wr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_sta_wr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_sta_wr_t(string name = "cap_ptd_csr_sta_wr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_sta_wr_t();
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
    
        typedef pu_cpp_int< 3 > err_vec_cpp_int_t;
        cpp_int int_var__err_vec;
        void err_vec (const cpp_int  & l__val);
        cpp_int err_vec() const;
    
        typedef pu_cpp_int< 24 > err_log_cpp_int_t;
        cpp_int int_var__err_log;
        void err_log (const cpp_int  & l__val);
        cpp_int err_log() const;
    
        typedef pu_cpp_int< 2 > err_rdresp_cpp_int_t;
        cpp_int int_var__err_rdresp;
        void err_rdresp (const cpp_int  & l__val);
        cpp_int err_rdresp() const;
    
        typedef pu_cpp_int< 11 > err_lif_id_cpp_int_t;
        cpp_int int_var__err_lif_id;
        void err_lif_id (const cpp_int  & l__val);
        cpp_int err_lif_id() const;
    
        typedef pu_cpp_int< 64 > err_axi_addr_cpp_int_t;
        cpp_int int_var__err_axi_addr;
        void err_axi_addr (const cpp_int  & l__val);
        cpp_int err_axi_addr() const;
    
        typedef pu_cpp_int< 48 > err_timestamp_cpp_int_t;
        cpp_int int_var__err_timestamp;
        void err_timestamp (const cpp_int  & l__val);
        cpp_int err_timestamp() const;
    
        typedef pu_cpp_int< 3 > wr_state_cpp_int_t;
        cpp_int int_var__wr_state;
        void wr_state (const cpp_int  & l__val);
        cpp_int wr_state() const;
    
}; // cap_ptd_csr_sta_wr_t
    
class cap_ptd_csr_sta_rdreq_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_sta_rdreq_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_sta_rdreq_t(string name = "cap_ptd_csr_sta_rdreq_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_sta_rdreq_t();
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
    
        typedef pu_cpp_int< 5 > err_vec_cpp_int_t;
        cpp_int int_var__err_vec;
        void err_vec (const cpp_int  & l__val);
        cpp_int err_vec() const;
    
        typedef pu_cpp_int< 29 > err_log_cpp_int_t;
        cpp_int int_var__err_log;
        void err_log (const cpp_int  & l__val);
        cpp_int err_log() const;
    
        typedef pu_cpp_int< 3 > rd_state_cpp_int_t;
        cpp_int int_var__rd_state;
        void rd_state (const cpp_int  & l__val);
        cpp_int rd_state() const;
    
        typedef pu_cpp_int< 1 > wdata_state_cpp_int_t;
        cpp_int int_var__wdata_state;
        void wdata_state (const cpp_int  & l__val);
        cpp_int wdata_state() const;
    
}; // cap_ptd_csr_sta_rdreq_t
    
class cap_ptd_csr_sta_rcv_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_sta_rcv_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_sta_rcv_t(string name = "cap_ptd_csr_sta_rcv_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_sta_rcv_t();
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
    
        typedef pu_cpp_int< 5 > err_vec1_cpp_int_t;
        cpp_int int_var__err_vec1;
        void err_vec1 (const cpp_int  & l__val);
        cpp_int err_vec1() const;
    
        typedef pu_cpp_int< 34 > err_log1_cpp_int_t;
        cpp_int int_var__err_log1;
        void err_log1 (const cpp_int  & l__val);
        cpp_int err_log1() const;
    
        typedef pu_cpp_int< 7 > err_vec2_cpp_int_t;
        cpp_int int_var__err_vec2;
        void err_vec2 (const cpp_int  & l__val);
        cpp_int err_vec2() const;
    
        typedef pu_cpp_int< 36 > err_log2_cpp_int_t;
        cpp_int int_var__err_log2;
        void err_log2 (const cpp_int  & l__val);
        cpp_int err_log2() const;
    
        typedef pu_cpp_int< 2 > rcv_state_cpp_int_t;
        cpp_int int_var__rcv_state;
        void rcv_state (const cpp_int  & l__val);
        cpp_int rcv_state() const;
    
        typedef pu_cpp_int< 2 > cmd_state_cpp_int_t;
        cpp_int int_var__cmd_state;
        void cmd_state (const cpp_int  & l__val);
        cpp_int cmd_state() const;
    
}; // cap_ptd_csr_sta_rcv_t
    
class cap_ptd_csr_sta_dfence_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_sta_dfence_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_sta_dfence_mem_t(string name = "cap_ptd_csr_sta_dfence_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_sta_dfence_mem_t();
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
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
}; // cap_ptd_csr_sta_dfence_mem_t
    
class cap_ptd_csr_cfg_dfence_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_cfg_dfence_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_cfg_dfence_mem_t(string name = "cap_ptd_csr_cfg_dfence_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_cfg_dfence_mem_t();
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
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_ptd_csr_cfg_dfence_mem_t
    
class cap_ptd_csr_sta_ffence_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_sta_ffence_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_sta_ffence_mem_t(string name = "cap_ptd_csr_sta_ffence_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_sta_ffence_mem_t();
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
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
}; // cap_ptd_csr_sta_ffence_mem_t
    
class cap_ptd_csr_cfg_ffence_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_cfg_ffence_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_cfg_ffence_mem_t(string name = "cap_ptd_csr_cfg_ffence_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_cfg_ffence_mem_t();
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
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_ptd_csr_cfg_ffence_mem_t
    
class cap_ptd_csr_sta_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_sta_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_sta_phv_mem_t(string name = "cap_ptd_csr_sta_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_sta_phv_mem_t();
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
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
}; // cap_ptd_csr_sta_phv_mem_t
    
class cap_ptd_csr_cfg_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_cfg_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_cfg_phv_mem_t(string name = "cap_ptd_csr_cfg_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_cfg_phv_mem_t();
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
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_ptd_csr_cfg_phv_mem_t
    
class cap_ptd_csr_sta_wdata_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_sta_wdata_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_sta_wdata_mem_t(string name = "cap_ptd_csr_sta_wdata_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_sta_wdata_mem_t();
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
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 20 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 8 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
}; // cap_ptd_csr_sta_wdata_mem_t
    
class cap_ptd_csr_cfg_wdata_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_cfg_wdata_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_cfg_wdata_mem_t(string name = "cap_ptd_csr_cfg_wdata_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_cfg_wdata_mem_t();
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
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        cpp_int int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        cpp_int int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_ptd_csr_cfg_wdata_mem_t
    
class cap_ptd_csr_sta_lat_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_sta_lat_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_sta_lat_mem_t(string name = "cap_ptd_csr_sta_lat_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_sta_lat_mem_t();
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
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 9 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 9 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
}; // cap_ptd_csr_sta_lat_mem_t
    
class cap_ptd_csr_cfg_lat_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_cfg_lat_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_cfg_lat_mem_t(string name = "cap_ptd_csr_cfg_lat_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_cfg_lat_mem_t();
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
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        cpp_int int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        cpp_int int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_ptd_csr_cfg_lat_mem_t
    
class cap_ptd_csr_sta_rdata_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_sta_rdata_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_sta_rdata_mem_t(string name = "cap_ptd_csr_sta_rdata_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_sta_rdata_mem_t();
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
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 20 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 9 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
}; // cap_ptd_csr_sta_rdata_mem_t
    
class cap_ptd_csr_cfg_rdata_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_cfg_rdata_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_cfg_rdata_mem_t(string name = "cap_ptd_csr_cfg_rdata_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_cfg_rdata_mem_t();
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
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        cpp_int int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        cpp_int int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_ptd_csr_cfg_rdata_mem_t
    
class cap_ptd_csr_cfg_spare_dbg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_cfg_spare_dbg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_cfg_spare_dbg_t(string name = "cap_ptd_csr_cfg_spare_dbg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_cfg_spare_dbg_t();
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
    
        typedef pu_cpp_int< 32 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // cap_ptd_csr_cfg_spare_dbg_t
    
class cap_ptd_csr_CNT_phv_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_CNT_phv_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_CNT_phv_t(string name = "cap_ptd_csr_CNT_phv_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_CNT_phv_t();
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
    
        typedef pu_cpp_int< 32 > no_data_cpp_int_t;
        cpp_int int_var__no_data;
        void no_data (const cpp_int  & l__val);
        cpp_int no_data() const;
    
        typedef pu_cpp_int< 32 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
        typedef pu_cpp_int< 32 > err_cpp_int_t;
        cpp_int int_var__err;
        void err (const cpp_int  & l__val);
        cpp_int err() const;
    
        typedef pu_cpp_int< 32 > recirc_cpp_int_t;
        cpp_int int_var__recirc;
        void recirc (const cpp_int  & l__val);
        cpp_int recirc() const;
    
}; // cap_ptd_csr_CNT_phv_t
    
class cap_ptd_csr_CNT_pb_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_CNT_pb_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_CNT_pb_t(string name = "cap_ptd_csr_CNT_pb_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_CNT_pb_t();
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
    
        typedef pu_cpp_int< 40 > sop_cpp_int_t;
        cpp_int int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        cpp_int int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
        typedef pu_cpp_int< 8 > err_cpp_int_t;
        cpp_int int_var__err;
        void err (const cpp_int  & l__val);
        cpp_int err() const;
    
        typedef pu_cpp_int< 8 > sop_err_cpp_int_t;
        cpp_int int_var__sop_err;
        void sop_err (const cpp_int  & l__val);
        cpp_int sop_err() const;
    
        typedef pu_cpp_int< 8 > eop_err_cpp_int_t;
        cpp_int int_var__eop_err;
        void eop_err (const cpp_int  & l__val);
        cpp_int eop_err() const;
    
}; // cap_ptd_csr_CNT_pb_t
    
class cap_ptd_csr_CNT_npv_resub_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_CNT_npv_resub_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_CNT_npv_resub_t(string name = "cap_ptd_csr_CNT_npv_resub_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_CNT_npv_resub_t();
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
    
        typedef pu_cpp_int< 40 > sop_cpp_int_t;
        cpp_int int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        cpp_int int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
        typedef pu_cpp_int< 8 > err_cpp_int_t;
        cpp_int int_var__err;
        void err (const cpp_int  & l__val);
        cpp_int err() const;
    
        typedef pu_cpp_int< 8 > sop_err_cpp_int_t;
        cpp_int int_var__sop_err;
        void sop_err (const cpp_int  & l__val);
        cpp_int sop_err() const;
    
        typedef pu_cpp_int< 8 > eop_err_cpp_int_t;
        cpp_int int_var__eop_err;
        void eop_err (const cpp_int  & l__val);
        cpp_int eop_err() const;
    
}; // cap_ptd_csr_CNT_npv_resub_t
    
class cap_ptd_csr_CNT_ma_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_CNT_ma_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_CNT_ma_t(string name = "cap_ptd_csr_CNT_ma_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_CNT_ma_t();
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
    
        typedef pu_cpp_int< 40 > sop_cpp_int_t;
        cpp_int int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        cpp_int int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
        typedef pu_cpp_int< 8 > err_cpp_int_t;
        cpp_int int_var__err;
        void err (const cpp_int  & l__val);
        cpp_int err() const;
    
        typedef pu_cpp_int< 8 > sop_err_cpp_int_t;
        cpp_int int_var__sop_err;
        void sop_err (const cpp_int  & l__val);
        cpp_int sop_err() const;
    
        typedef pu_cpp_int< 8 > eop_err_cpp_int_t;
        cpp_int int_var__eop_err;
        void eop_err (const cpp_int  & l__val);
        cpp_int eop_err() const;
    
}; // cap_ptd_csr_CNT_ma_t
    
class cap_ptd_csr_cfg_xoff_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_cfg_xoff_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_cfg_xoff_t(string name = "cap_ptd_csr_cfg_xoff_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_cfg_xoff_t();
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
    
        typedef pu_cpp_int< 11 > numphv_thresh_cpp_int_t;
        cpp_int int_var__numphv_thresh;
        void numphv_thresh (const cpp_int  & l__val);
        cpp_int numphv_thresh() const;
    
        typedef pu_cpp_int< 1 > numphv_en_cpp_int_t;
        cpp_int int_var__numphv_en;
        void numphv_en (const cpp_int  & l__val);
        cpp_int numphv_en() const;
    
}; // cap_ptd_csr_cfg_xoff_t
    
class cap_ptd_csr_axi_attr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_axi_attr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_axi_attr_t(string name = "cap_ptd_csr_axi_attr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_axi_attr_t();
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
    
        typedef pu_cpp_int< 4 > arcache_0_cpp_int_t;
        cpp_int int_var__arcache_0;
        void arcache_0 (const cpp_int  & l__val);
        cpp_int arcache_0() const;
    
        typedef pu_cpp_int< 4 > awcache_0_cpp_int_t;
        cpp_int int_var__awcache_0;
        void awcache_0 (const cpp_int  & l__val);
        cpp_int awcache_0() const;
    
        typedef pu_cpp_int< 4 > arcache_1_cpp_int_t;
        cpp_int int_var__arcache_1;
        void arcache_1 (const cpp_int  & l__val);
        cpp_int arcache_1() const;
    
        typedef pu_cpp_int< 4 > awcache_1_cpp_int_t;
        cpp_int int_var__awcache_1;
        void awcache_1 (const cpp_int  & l__val);
        cpp_int awcache_1() const;
    
        typedef pu_cpp_int< 3 > prot_cpp_int_t;
        cpp_int int_var__prot;
        void prot (const cpp_int  & l__val);
        cpp_int prot() const;
    
        typedef pu_cpp_int< 4 > arqos_cpp_int_t;
        cpp_int int_var__arqos;
        void arqos (const cpp_int  & l__val);
        cpp_int arqos() const;
    
        typedef pu_cpp_int< 4 > awqos_0_cpp_int_t;
        cpp_int int_var__awqos_0;
        void awqos_0 (const cpp_int  & l__val);
        cpp_int awqos_0() const;
    
        typedef pu_cpp_int< 4 > awqos_1_cpp_int_t;
        cpp_int int_var__awqos_1;
        void awqos_1 (const cpp_int  & l__val);
        cpp_int awqos_1() const;
    
        typedef pu_cpp_int< 1 > lock_cpp_int_t;
        cpp_int int_var__lock;
        void lock (const cpp_int  & l__val);
        cpp_int lock() const;
    
}; // cap_ptd_csr_axi_attr_t
    
class cap_ptd_csr_sta_id_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_sta_id_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_sta_id_t(string name = "cap_ptd_csr_sta_id_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_sta_id_t();
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
    
        typedef pu_cpp_int< 8 > wr_pend_cnt_cpp_int_t;
        cpp_int int_var__wr_pend_cnt;
        void wr_pend_cnt (const cpp_int  & l__val);
        cpp_int wr_pend_cnt() const;
    
        typedef pu_cpp_int< 8 > rd_pend_cnt_cpp_int_t;
        cpp_int int_var__rd_pend_cnt;
        void rd_pend_cnt (const cpp_int  & l__val);
        cpp_int rd_pend_cnt() const;
    
        typedef pu_cpp_int< 8 > rd_pend_rsrc_cnt_cpp_int_t;
        cpp_int int_var__rd_pend_rsrc_cnt;
        void rd_pend_rsrc_cnt (const cpp_int  & l__val);
        cpp_int rd_pend_rsrc_cnt() const;
    
}; // cap_ptd_csr_sta_id_t
    
class cap_ptd_csr_cfg_bkp_dbg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_cfg_bkp_dbg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_cfg_bkp_dbg_t(string name = "cap_ptd_csr_cfg_bkp_dbg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_cfg_bkp_dbg_t();
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
    
        typedef pu_cpp_int< 1 > round_read_64byte_en_cpp_int_t;
        cpp_int int_var__round_read_64byte_en;
        void round_read_64byte_en (const cpp_int  & l__val);
        cpp_int round_read_64byte_en() const;
    
}; // cap_ptd_csr_cfg_bkp_dbg_t
    
class cap_ptd_csr_cfg_ctrl_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_cfg_ctrl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_cfg_ctrl_t(string name = "cap_ptd_csr_cfg_ctrl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_cfg_ctrl_t();
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
    
        typedef pu_cpp_int< 8 > max_rd_req_cnt_cpp_int_t;
        cpp_int int_var__max_rd_req_cnt;
        void max_rd_req_cnt (const cpp_int  & l__val);
        cpp_int max_rd_req_cnt() const;
    
        typedef pu_cpp_int< 8 > max_wr_req_cnt_cpp_int_t;
        cpp_int int_var__max_wr_req_cnt;
        void max_wr_req_cnt (const cpp_int  & l__val);
        cpp_int max_wr_req_cnt() const;
    
        typedef pu_cpp_int< 9 > wdata_ff_thresh_cpp_int_t;
        cpp_int int_var__wdata_ff_thresh;
        void wdata_ff_thresh (const cpp_int  & l__val);
        cpp_int wdata_ff_thresh() const;
    
        typedef pu_cpp_int< 4 > pkt_ff_thresh_cpp_int_t;
        cpp_int int_var__pkt_ff_thresh;
        void pkt_ff_thresh (const cpp_int  & l__val);
        cpp_int pkt_ff_thresh() const;
    
        typedef pu_cpp_int< 5 > mem_ff_thresh_cpp_int_t;
        cpp_int int_var__mem_ff_thresh;
        void mem_ff_thresh (const cpp_int  & l__val);
        cpp_int mem_ff_thresh() const;
    
        typedef pu_cpp_int< 1 > wr_non_host_xn_64byte_en_cpp_int_t;
        cpp_int int_var__wr_non_host_xn_64byte_en;
        void wr_non_host_xn_64byte_en (const cpp_int  & l__val);
        cpp_int wr_non_host_xn_64byte_en() const;
    
        typedef pu_cpp_int< 1 > wr_host_xn_64byte_en_cpp_int_t;
        cpp_int int_var__wr_host_xn_64byte_en;
        void wr_host_xn_64byte_en (const cpp_int  & l__val);
        cpp_int wr_host_xn_64byte_en() const;
    
        typedef pu_cpp_int< 1 > rd_non_host_xn_64byte_en_cpp_int_t;
        cpp_int int_var__rd_non_host_xn_64byte_en;
        void rd_non_host_xn_64byte_en (const cpp_int  & l__val);
        cpp_int rd_non_host_xn_64byte_en() const;
    
        typedef pu_cpp_int< 1 > rd_host_xn_64byte_en_cpp_int_t;
        cpp_int int_var__rd_host_xn_64byte_en;
        void rd_host_xn_64byte_en (const cpp_int  & l__val);
        cpp_int rd_host_xn_64byte_en() const;
    
        typedef pu_cpp_int< 1 > err_drop_en_cpp_int_t;
        cpp_int int_var__err_drop_en;
        void err_drop_en (const cpp_int  & l__val);
        cpp_int err_drop_en() const;
    
        typedef pu_cpp_int< 1 > rd_err_cmd_drop_en_cpp_int_t;
        cpp_int int_var__rd_err_cmd_drop_en;
        void rd_err_cmd_drop_en (const cpp_int  & l__val);
        cpp_int rd_err_cmd_drop_en() const;
    
        typedef pu_cpp_int< 1 > phv_addr_err_force_en_cpp_int_t;
        cpp_int int_var__phv_addr_err_force_en;
        void phv_addr_err_force_en (const cpp_int  & l__val);
        cpp_int phv_addr_err_force_en() const;
    
        typedef pu_cpp_int< 1 > rd_round_hbm_en_cpp_int_t;
        cpp_int int_var__rd_round_hbm_en;
        void rd_round_hbm_en (const cpp_int  & l__val);
        cpp_int rd_round_hbm_en() const;
    
        typedef pu_cpp_int< 8 > spare_cpp_int_t;
        cpp_int int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
}; // cap_ptd_csr_cfg_ctrl_t
    
class cap_ptd_csr_cfg_debug_port_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_cfg_debug_port_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_cfg_debug_port_t(string name = "cap_ptd_csr_cfg_debug_port_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_cfg_debug_port_t();
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
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        cpp_int int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 1 > select_cpp_int_t;
        cpp_int int_var__select;
        void select (const cpp_int  & l__val);
        cpp_int select() const;
    
}; // cap_ptd_csr_cfg_debug_port_t
    
class cap_ptd_csr_sta_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_sta_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_sta_fifo_t(string name = "cap_ptd_csr_sta_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_sta_fifo_t();
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
    
        typedef pu_cpp_int< 1 > lat_ff_full_cpp_int_t;
        cpp_int int_var__lat_ff_full;
        void lat_ff_full (const cpp_int  & l__val);
        cpp_int lat_ff_full() const;
    
        typedef pu_cpp_int< 1 > lat_ff_empty_cpp_int_t;
        cpp_int int_var__lat_ff_empty;
        void lat_ff_empty (const cpp_int  & l__val);
        cpp_int lat_ff_empty() const;
    
        typedef pu_cpp_int< 1 > wdata_ff_full_cpp_int_t;
        cpp_int int_var__wdata_ff_full;
        void wdata_ff_full (const cpp_int  & l__val);
        cpp_int wdata_ff_full() const;
    
        typedef pu_cpp_int< 1 > wdata_ff_empty_cpp_int_t;
        cpp_int int_var__wdata_ff_empty;
        void wdata_ff_empty (const cpp_int  & l__val);
        cpp_int wdata_ff_empty() const;
    
        typedef pu_cpp_int< 1 > rcv_stg_ff_full_cpp_int_t;
        cpp_int int_var__rcv_stg_ff_full;
        void rcv_stg_ff_full (const cpp_int  & l__val);
        cpp_int rcv_stg_ff_full() const;
    
        typedef pu_cpp_int< 1 > rcv_stg_ff_empty_cpp_int_t;
        cpp_int int_var__rcv_stg_ff_empty;
        void rcv_stg_ff_empty (const cpp_int  & l__val);
        cpp_int rcv_stg_ff_empty() const;
    
        typedef pu_cpp_int< 1 > cmdflit_ff_full_cpp_int_t;
        cpp_int int_var__cmdflit_ff_full;
        void cmdflit_ff_full (const cpp_int  & l__val);
        cpp_int cmdflit_ff_full() const;
    
        typedef pu_cpp_int< 1 > cmdflit_ff_empty_cpp_int_t;
        cpp_int int_var__cmdflit_ff_empty;
        void cmdflit_ff_empty (const cpp_int  & l__val);
        cpp_int cmdflit_ff_empty() const;
    
        typedef pu_cpp_int< 1 > cmd_ff_full_cpp_int_t;
        cpp_int int_var__cmd_ff_full;
        void cmd_ff_full (const cpp_int  & l__val);
        cpp_int cmd_ff_full() const;
    
        typedef pu_cpp_int< 1 > cmd_ff_empty_cpp_int_t;
        cpp_int int_var__cmd_ff_empty;
        void cmd_ff_empty (const cpp_int  & l__val);
        cpp_int cmd_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pkt_ff_full_cpp_int_t;
        cpp_int int_var__pkt_ff_full;
        void pkt_ff_full (const cpp_int  & l__val);
        cpp_int pkt_ff_full() const;
    
        typedef pu_cpp_int< 1 > pkt_ff_empty_cpp_int_t;
        cpp_int int_var__pkt_ff_empty;
        void pkt_ff_empty (const cpp_int  & l__val);
        cpp_int pkt_ff_empty() const;
    
        typedef pu_cpp_int< 1 > wr_mem_ff_full_cpp_int_t;
        cpp_int int_var__wr_mem_ff_full;
        void wr_mem_ff_full (const cpp_int  & l__val);
        cpp_int wr_mem_ff_full() const;
    
        typedef pu_cpp_int< 1 > wr_mem_ff_empty_cpp_int_t;
        cpp_int int_var__wr_mem_ff_empty;
        void wr_mem_ff_empty (const cpp_int  & l__val);
        cpp_int wr_mem_ff_empty() const;
    
        typedef pu_cpp_int< 1 > dfence_ff_full_cpp_int_t;
        cpp_int int_var__dfence_ff_full;
        void dfence_ff_full (const cpp_int  & l__val);
        cpp_int dfence_ff_full() const;
    
        typedef pu_cpp_int< 1 > dfence_ff_empty_cpp_int_t;
        cpp_int int_var__dfence_ff_empty;
        void dfence_ff_empty (const cpp_int  & l__val);
        cpp_int dfence_ff_empty() const;
    
        typedef pu_cpp_int< 1 > ffence_ff_full_cpp_int_t;
        cpp_int int_var__ffence_ff_full;
        void ffence_ff_full (const cpp_int  & l__val);
        cpp_int ffence_ff_full() const;
    
        typedef pu_cpp_int< 1 > ffence_ff_empty_cpp_int_t;
        cpp_int int_var__ffence_ff_empty;
        void ffence_ff_empty (const cpp_int  & l__val);
        cpp_int ffence_ff_empty() const;
    
        typedef pu_cpp_int< 1 > phv_lpbk_in_drdy_cpp_int_t;
        cpp_int int_var__phv_lpbk_in_drdy;
        void phv_lpbk_in_drdy (const cpp_int  & l__val);
        cpp_int phv_lpbk_in_drdy() const;
    
        typedef pu_cpp_int< 1 > phv_lpbk_out_srdy_cpp_int_t;
        cpp_int int_var__phv_lpbk_out_srdy;
        void phv_lpbk_out_srdy (const cpp_int  & l__val);
        cpp_int phv_lpbk_out_srdy() const;
    
        typedef pu_cpp_int< 1 > ma_srdy_cpp_int_t;
        cpp_int int_var__ma_srdy;
        void ma_srdy (const cpp_int  & l__val);
        cpp_int ma_srdy() const;
    
        typedef pu_cpp_int< 1 > ma_drdy_cpp_int_t;
        cpp_int int_var__ma_drdy;
        void ma_drdy (const cpp_int  & l__val);
        cpp_int ma_drdy() const;
    
        typedef pu_cpp_int< 1 > axi_wr_valid_cpp_int_t;
        cpp_int int_var__axi_wr_valid;
        void axi_wr_valid (const cpp_int  & l__val);
        cpp_int axi_wr_valid() const;
    
        typedef pu_cpp_int< 1 > axi_wr_ready_cpp_int_t;
        cpp_int int_var__axi_wr_ready;
        void axi_wr_ready (const cpp_int  & l__val);
        cpp_int axi_wr_ready() const;
    
        typedef pu_cpp_int< 1 > axi_rd_valid_cpp_int_t;
        cpp_int int_var__axi_rd_valid;
        void axi_rd_valid (const cpp_int  & l__val);
        cpp_int axi_rd_valid() const;
    
        typedef pu_cpp_int< 1 > axi_rd_ready_cpp_int_t;
        cpp_int int_var__axi_rd_ready;
        void axi_rd_ready (const cpp_int  & l__val);
        cpp_int axi_rd_ready() const;
    
}; // cap_ptd_csr_sta_fifo_t
    
class cap_ptd_csr_csr_intr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_csr_intr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_csr_intr_t(string name = "cap_ptd_csr_csr_intr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_csr_intr_t();
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
    
}; // cap_ptd_csr_csr_intr_t
    
class cap_ptd_csr_base_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_base_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_base_t(string name = "cap_ptd_csr_base_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_base_t();
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
    
}; // cap_ptd_csr_base_t
    
class cap_ptd_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_csr_t(string name = "cap_ptd_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_csr_t();
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
    
        cap_ptd_csr_base_t base;
    
        cap_ptd_csr_csr_intr_t csr_intr;
    
        cap_ptd_csr_sta_fifo_t sta_fifo;
    
        cap_ptd_csr_cfg_debug_port_t cfg_debug_port;
    
        cap_ptd_csr_cfg_ctrl_t cfg_ctrl;
    
        cap_ptd_csr_cfg_bkp_dbg_t cfg_bkp_dbg;
    
        cap_ptd_csr_sta_id_t sta_id;
    
        cap_ptd_csr_axi_attr_t axi_attr;
    
        cap_ptd_csr_cfg_xoff_t cfg_xoff;
    
        cap_ptd_csr_CNT_ma_t CNT_ma;
    
        cap_ptd_csr_CNT_npv_resub_t CNT_npv_resub;
    
        cap_ptd_csr_CNT_pb_t CNT_pb;
    
        cap_ptd_csr_CNT_phv_t CNT_phv;
    
        cap_ptd_csr_cfg_spare_dbg_t cfg_spare_dbg;
    
        cap_ptd_csr_cfg_rdata_mem_t cfg_rdata_mem;
    
        cap_ptd_csr_sta_rdata_mem_t sta_rdata_mem;
    
        cap_ptd_csr_cfg_lat_mem_t cfg_lat_mem;
    
        cap_ptd_csr_sta_lat_mem_t sta_lat_mem;
    
        cap_ptd_csr_cfg_wdata_mem_t cfg_wdata_mem;
    
        cap_ptd_csr_sta_wdata_mem_t sta_wdata_mem;
    
        cap_ptd_csr_cfg_phv_mem_t cfg_phv_mem;
    
        cap_ptd_csr_sta_phv_mem_t sta_phv_mem;
    
        cap_ptd_csr_cfg_ffence_mem_t cfg_ffence_mem;
    
        cap_ptd_csr_sta_ffence_mem_t sta_ffence_mem;
    
        cap_ptd_csr_cfg_dfence_mem_t cfg_dfence_mem;
    
        cap_ptd_csr_sta_dfence_mem_t sta_dfence_mem;
    
        cap_ptd_csr_sta_rcv_t sta_rcv;
    
        cap_ptd_csr_sta_rdreq_t sta_rdreq;
    
        cap_ptd_csr_sta_wr_t sta_wr;
    
        cap_ptd_csr_CNT_err_t CNT_err;
    
        cap_ptd_csr_sta_rdrsp_t sta_rdrsp;
    
        cap_ptd_csr_sta_wrrsp_t sta_wrrsp;
    
        cap_ptd_csr_sta_rd_t sta_rd;
    
        cap_ptd_csr_sta_pkt_t sta_pkt;
    
        cap_ptd_csr_sta_wrreq_t sta_wrreq;
    
        cap_ptd_csr_sta_xoff_t sta_xoff;
    
        cap_ptd_csr_cfg_debug_ctrl_t cfg_debug_ctrl;
    
        cap_ptd_csr_cfg_debug_bus_t cfg_debug_bus;
    
        cap_ptd_csr_cfg_dhs_dbg_mem_t cfg_dhs_dbg_mem;
    
        cap_ptd_csr_CNT_axi_wr_t CNT_axi_wr;
    
        cap_ptd_csr_CNT_axi_rd_t CNT_axi_rd;
    
        cap_ptd_csr_dhs_dbg_mem_t dhs_dbg_mem;
    
        cap_ptd_csr_intgrp_status_t int_groups;
    
        cap_ptd_csr_int_ecc_t int_ecc;
    
        cap_ptd_csr_int_fifo_t int_fifo;
    
        cap_ptd_csr_int_grp1_t int_grp1;
    
        cap_ptd_csr_int_grp2_t int_grp2;
    
        cap_ptd_csr_int_intf_t int_intf;
    
}; // cap_ptd_csr_t
    
#endif // CAP_PTD_CSR_H
        