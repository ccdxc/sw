
#ifndef CAP_PXB_CSR_H
#define CAP_PXB_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_pxb_csr_int_err_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_int_err_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_int_err_int_enable_clear_t(string name = "cap_pxb_csr_int_err_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_int_err_int_enable_clear_t();
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
    
        typedef pu_cpp_int< 1 > cpl_ecrc_enable_cpp_int_t;
        cpp_int int_var__cpl_ecrc_enable;
        void cpl_ecrc_enable (const cpp_int  & l__val);
        cpp_int cpl_ecrc_enable() const;
    
        typedef pu_cpp_int< 1 > cpl_rxbuf_err_enable_cpp_int_t;
        cpp_int int_var__cpl_rxbuf_err_enable;
        void cpl_rxbuf_err_enable (const cpp_int  & l__val);
        cpp_int cpl_rxbuf_err_enable() const;
    
        typedef pu_cpp_int< 1 > cpl_stat_enable_cpp_int_t;
        cpp_int int_var__cpl_stat_enable;
        void cpl_stat_enable (const cpp_int  & l__val);
        cpp_int cpl_stat_enable() const;
    
        typedef pu_cpp_int< 1 > cpl_unexp_enable_cpp_int_t;
        cpp_int int_var__cpl_unexp_enable;
        void cpl_unexp_enable (const cpp_int  & l__val);
        cpp_int cpl_unexp_enable() const;
    
        typedef pu_cpp_int< 1 > cpl_timeout_enable_cpp_int_t;
        cpp_int int_var__cpl_timeout_enable;
        void cpl_timeout_enable (const cpp_int  & l__val);
        cpp_int cpl_timeout_enable() const;
    
        typedef pu_cpp_int< 1 > cpl_len_enable_cpp_int_t;
        cpp_int int_var__cpl_len_enable;
        void cpl_len_enable (const cpp_int  & l__val);
        cpp_int cpl_len_enable() const;
    
        typedef pu_cpp_int< 1 > cpl_poison_enable_cpp_int_t;
        cpp_int int_var__cpl_poison_enable;
        void cpl_poison_enable (const cpp_int  & l__val);
        cpp_int cpl_poison_enable() const;
    
        typedef pu_cpp_int< 1 > cpl_lowaddr_enable_cpp_int_t;
        cpp_int int_var__cpl_lowaddr_enable;
        void cpl_lowaddr_enable (const cpp_int  & l__val);
        cpp_int cpl_lowaddr_enable() const;
    
        typedef pu_cpp_int< 1 > tgt_ind_rsp_unexp_enable_cpp_int_t;
        cpp_int int_var__tgt_ind_rsp_unexp_enable;
        void tgt_ind_rsp_unexp_enable (const cpp_int  & l__val);
        cpp_int tgt_ind_rsp_unexp_enable() const;
    
        typedef pu_cpp_int< 1 > tgt_axi_rsp_unexp_enable_cpp_int_t;
        cpp_int int_var__tgt_axi_rsp_unexp_enable;
        void tgt_axi_rsp_unexp_enable (const cpp_int  & l__val);
        cpp_int tgt_axi_rsp_unexp_enable() const;
    
        typedef pu_cpp_int< 1 > tgt_axi_rsp_err_enable_cpp_int_t;
        cpp_int int_var__tgt_axi_rsp_err_enable;
        void tgt_axi_rsp_err_enable (const cpp_int  & l__val);
        cpp_int tgt_axi_rsp_err_enable() const;
    
        typedef pu_cpp_int< 1 > tgt_req_4k_err_enable_cpp_int_t;
        cpp_int int_var__tgt_req_4k_err_enable;
        void tgt_req_4k_err_enable (const cpp_int  & l__val);
        cpp_int tgt_req_4k_err_enable() const;
    
        typedef pu_cpp_int< 1 > itr_req_bdfmiss_enable_cpp_int_t;
        cpp_int int_var__itr_req_bdfmiss_enable;
        void itr_req_bdfmiss_enable (const cpp_int  & l__val);
        cpp_int itr_req_bdfmiss_enable() const;
    
        typedef pu_cpp_int< 1 > itr_req_axi_unsupp_enable_cpp_int_t;
        cpp_int int_var__itr_req_axi_unsupp_enable;
        void itr_req_axi_unsupp_enable (const cpp_int  & l__val);
        cpp_int itr_req_axi_unsupp_enable() const;
    
        typedef pu_cpp_int< 1 > itr_req_msg_align_err_enable_cpp_int_t;
        cpp_int int_var__itr_req_msg_align_err_enable;
        void itr_req_msg_align_err_enable (const cpp_int  & l__val);
        cpp_int itr_req_msg_align_err_enable() const;
    
        typedef pu_cpp_int< 1 > itr_bus_master_dis_enable_cpp_int_t;
        cpp_int int_var__itr_bus_master_dis_enable;
        void itr_bus_master_dis_enable (const cpp_int  & l__val);
        cpp_int itr_bus_master_dis_enable() const;
    
        typedef pu_cpp_int< 1 > itr_atomic_seq_err_enable_cpp_int_t;
        cpp_int int_var__itr_atomic_seq_err_enable;
        void itr_atomic_seq_err_enable (const cpp_int  & l__val);
        cpp_int itr_atomic_seq_err_enable() const;
    
        typedef pu_cpp_int< 1 > itr_atomic_resource_err_enable_cpp_int_t;
        cpp_int int_var__itr_atomic_resource_err_enable;
        void itr_atomic_resource_err_enable (const cpp_int  & l__val);
        cpp_int itr_atomic_resource_err_enable() const;
    
        typedef pu_cpp_int< 1 > tgt_rmw_axi_resperr_enable_cpp_int_t;
        cpp_int int_var__tgt_rmw_axi_resperr_enable;
        void tgt_rmw_axi_resperr_enable (const cpp_int  & l__val);
        cpp_int tgt_rmw_axi_resperr_enable() const;
    
        typedef pu_cpp_int< 1 > tgt_ind_notify_axi_resperr_enable_cpp_int_t;
        cpp_int int_var__tgt_ind_notify_axi_resperr_enable;
        void tgt_ind_notify_axi_resperr_enable (const cpp_int  & l__val);
        cpp_int tgt_ind_notify_axi_resperr_enable() const;
    
}; // cap_pxb_csr_int_err_int_enable_clear_t
    
class cap_pxb_csr_int_err_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_int_err_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_int_err_int_test_set_t(string name = "cap_pxb_csr_int_err_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_int_err_int_test_set_t();
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
    
        typedef pu_cpp_int< 1 > cpl_ecrc_interrupt_cpp_int_t;
        cpp_int int_var__cpl_ecrc_interrupt;
        void cpl_ecrc_interrupt (const cpp_int  & l__val);
        cpp_int cpl_ecrc_interrupt() const;
    
        typedef pu_cpp_int< 1 > cpl_rxbuf_err_interrupt_cpp_int_t;
        cpp_int int_var__cpl_rxbuf_err_interrupt;
        void cpl_rxbuf_err_interrupt (const cpp_int  & l__val);
        cpp_int cpl_rxbuf_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > cpl_stat_interrupt_cpp_int_t;
        cpp_int int_var__cpl_stat_interrupt;
        void cpl_stat_interrupt (const cpp_int  & l__val);
        cpp_int cpl_stat_interrupt() const;
    
        typedef pu_cpp_int< 1 > cpl_unexp_interrupt_cpp_int_t;
        cpp_int int_var__cpl_unexp_interrupt;
        void cpl_unexp_interrupt (const cpp_int  & l__val);
        cpp_int cpl_unexp_interrupt() const;
    
        typedef pu_cpp_int< 1 > cpl_timeout_interrupt_cpp_int_t;
        cpp_int int_var__cpl_timeout_interrupt;
        void cpl_timeout_interrupt (const cpp_int  & l__val);
        cpp_int cpl_timeout_interrupt() const;
    
        typedef pu_cpp_int< 1 > cpl_len_interrupt_cpp_int_t;
        cpp_int int_var__cpl_len_interrupt;
        void cpl_len_interrupt (const cpp_int  & l__val);
        cpp_int cpl_len_interrupt() const;
    
        typedef pu_cpp_int< 1 > cpl_poison_interrupt_cpp_int_t;
        cpp_int int_var__cpl_poison_interrupt;
        void cpl_poison_interrupt (const cpp_int  & l__val);
        cpp_int cpl_poison_interrupt() const;
    
        typedef pu_cpp_int< 1 > cpl_lowaddr_interrupt_cpp_int_t;
        cpp_int int_var__cpl_lowaddr_interrupt;
        void cpl_lowaddr_interrupt (const cpp_int  & l__val);
        cpp_int cpl_lowaddr_interrupt() const;
    
        typedef pu_cpp_int< 1 > tgt_ind_rsp_unexp_interrupt_cpp_int_t;
        cpp_int int_var__tgt_ind_rsp_unexp_interrupt;
        void tgt_ind_rsp_unexp_interrupt (const cpp_int  & l__val);
        cpp_int tgt_ind_rsp_unexp_interrupt() const;
    
        typedef pu_cpp_int< 1 > tgt_axi_rsp_unexp_interrupt_cpp_int_t;
        cpp_int int_var__tgt_axi_rsp_unexp_interrupt;
        void tgt_axi_rsp_unexp_interrupt (const cpp_int  & l__val);
        cpp_int tgt_axi_rsp_unexp_interrupt() const;
    
        typedef pu_cpp_int< 1 > tgt_axi_rsp_err_interrupt_cpp_int_t;
        cpp_int int_var__tgt_axi_rsp_err_interrupt;
        void tgt_axi_rsp_err_interrupt (const cpp_int  & l__val);
        cpp_int tgt_axi_rsp_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > tgt_req_4k_err_interrupt_cpp_int_t;
        cpp_int int_var__tgt_req_4k_err_interrupt;
        void tgt_req_4k_err_interrupt (const cpp_int  & l__val);
        cpp_int tgt_req_4k_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > itr_req_bdfmiss_interrupt_cpp_int_t;
        cpp_int int_var__itr_req_bdfmiss_interrupt;
        void itr_req_bdfmiss_interrupt (const cpp_int  & l__val);
        cpp_int itr_req_bdfmiss_interrupt() const;
    
        typedef pu_cpp_int< 1 > itr_req_axi_unsupp_interrupt_cpp_int_t;
        cpp_int int_var__itr_req_axi_unsupp_interrupt;
        void itr_req_axi_unsupp_interrupt (const cpp_int  & l__val);
        cpp_int itr_req_axi_unsupp_interrupt() const;
    
        typedef pu_cpp_int< 1 > itr_req_msg_align_err_interrupt_cpp_int_t;
        cpp_int int_var__itr_req_msg_align_err_interrupt;
        void itr_req_msg_align_err_interrupt (const cpp_int  & l__val);
        cpp_int itr_req_msg_align_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > itr_bus_master_dis_interrupt_cpp_int_t;
        cpp_int int_var__itr_bus_master_dis_interrupt;
        void itr_bus_master_dis_interrupt (const cpp_int  & l__val);
        cpp_int itr_bus_master_dis_interrupt() const;
    
        typedef pu_cpp_int< 1 > itr_atomic_seq_err_interrupt_cpp_int_t;
        cpp_int int_var__itr_atomic_seq_err_interrupt;
        void itr_atomic_seq_err_interrupt (const cpp_int  & l__val);
        cpp_int itr_atomic_seq_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > itr_atomic_resource_err_interrupt_cpp_int_t;
        cpp_int int_var__itr_atomic_resource_err_interrupt;
        void itr_atomic_resource_err_interrupt (const cpp_int  & l__val);
        cpp_int itr_atomic_resource_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > tgt_rmw_axi_resperr_interrupt_cpp_int_t;
        cpp_int int_var__tgt_rmw_axi_resperr_interrupt;
        void tgt_rmw_axi_resperr_interrupt (const cpp_int  & l__val);
        cpp_int tgt_rmw_axi_resperr_interrupt() const;
    
        typedef pu_cpp_int< 1 > tgt_ind_notify_axi_resperr_interrupt_cpp_int_t;
        cpp_int int_var__tgt_ind_notify_axi_resperr_interrupt;
        void tgt_ind_notify_axi_resperr_interrupt (const cpp_int  & l__val);
        cpp_int tgt_ind_notify_axi_resperr_interrupt() const;
    
}; // cap_pxb_csr_int_err_int_test_set_t
    
class cap_pxb_csr_int_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_int_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_int_err_t(string name = "cap_pxb_csr_int_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_int_err_t();
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
    
        cap_pxb_csr_int_err_int_test_set_t intreg;
    
        cap_pxb_csr_int_err_int_test_set_t int_test_set;
    
        cap_pxb_csr_int_err_int_enable_clear_t int_enable_set;
    
        cap_pxb_csr_int_err_int_enable_clear_t int_enable_clear;
    
}; // cap_pxb_csr_int_err_t
    
class cap_pxb_csr_int_tgt_ecc_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_int_tgt_ecc_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_int_tgt_ecc_int_enable_clear_t(string name = "cap_pxb_csr_int_tgt_ecc_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_int_tgt_ecc_int_enable_clear_t();
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
    
        typedef pu_cpp_int< 1 > aximst_0_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__aximst_0_uncorrectable_enable;
        void aximst_0_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int aximst_0_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > aximst_0_correctable_enable_cpp_int_t;
        cpp_int int_var__aximst_0_correctable_enable;
        void aximst_0_correctable_enable (const cpp_int  & l__val);
        cpp_int aximst_0_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > aximst_1_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__aximst_1_uncorrectable_enable;
        void aximst_1_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int aximst_1_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > aximst_1_correctable_enable_cpp_int_t;
        cpp_int int_var__aximst_1_correctable_enable;
        void aximst_1_correctable_enable (const cpp_int  & l__val);
        cpp_int aximst_1_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > aximst_2_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__aximst_2_uncorrectable_enable;
        void aximst_2_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int aximst_2_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > aximst_2_correctable_enable_cpp_int_t;
        cpp_int int_var__aximst_2_correctable_enable;
        void aximst_2_correctable_enable (const cpp_int  & l__val);
        cpp_int aximst_2_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > aximst_3_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__aximst_3_uncorrectable_enable;
        void aximst_3_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int aximst_3_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > aximst_3_correctable_enable_cpp_int_t;
        cpp_int int_var__aximst_3_correctable_enable;
        void aximst_3_correctable_enable (const cpp_int  & l__val);
        cpp_int aximst_3_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > aximst_4_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__aximst_4_uncorrectable_enable;
        void aximst_4_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int aximst_4_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > aximst_4_correctable_enable_cpp_int_t;
        cpp_int int_var__aximst_4_correctable_enable;
        void aximst_4_correctable_enable (const cpp_int  & l__val);
        cpp_int aximst_4_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > rxcrbfr_0_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__rxcrbfr_0_uncorrectable_enable;
        void rxcrbfr_0_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int rxcrbfr_0_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > rxcrbfr_0_correctable_enable_cpp_int_t;
        cpp_int int_var__rxcrbfr_0_correctable_enable;
        void rxcrbfr_0_correctable_enable (const cpp_int  & l__val);
        cpp_int rxcrbfr_0_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > rxcrbfr_1_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__rxcrbfr_1_uncorrectable_enable;
        void rxcrbfr_1_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int rxcrbfr_1_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > rxcrbfr_1_correctable_enable_cpp_int_t;
        cpp_int int_var__rxcrbfr_1_correctable_enable;
        void rxcrbfr_1_correctable_enable (const cpp_int  & l__val);
        cpp_int rxcrbfr_1_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > rxcrbfr_2_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__rxcrbfr_2_uncorrectable_enable;
        void rxcrbfr_2_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int rxcrbfr_2_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > rxcrbfr_2_correctable_enable_cpp_int_t;
        cpp_int int_var__rxcrbfr_2_correctable_enable;
        void rxcrbfr_2_correctable_enable (const cpp_int  & l__val);
        cpp_int rxcrbfr_2_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > rxcrbfr_3_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__rxcrbfr_3_uncorrectable_enable;
        void rxcrbfr_3_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int rxcrbfr_3_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > rxcrbfr_3_correctable_enable_cpp_int_t;
        cpp_int int_var__rxcrbfr_3_correctable_enable;
        void rxcrbfr_3_correctable_enable (const cpp_int  & l__val);
        cpp_int rxcrbfr_3_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > rxinfo_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__rxinfo_uncorrectable_enable;
        void rxinfo_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int rxinfo_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > rxinfo_correctable_enable_cpp_int_t;
        cpp_int int_var__rxinfo_correctable_enable;
        void rxinfo_correctable_enable (const cpp_int  & l__val);
        cpp_int rxinfo_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > cplst_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__cplst_uncorrectable_enable;
        void cplst_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int cplst_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > cplst_correctable_enable_cpp_int_t;
        cpp_int int_var__cplst_correctable_enable;
        void cplst_correctable_enable (const cpp_int  & l__val);
        cpp_int cplst_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > romask_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__romask_uncorrectable_enable;
        void romask_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int romask_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > romask_correctable_enable_cpp_int_t;
        cpp_int int_var__romask_correctable_enable;
        void romask_correctable_enable (const cpp_int  & l__val);
        cpp_int romask_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > pmr_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__pmr_uncorrectable_enable;
        void pmr_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int pmr_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > pmr_correctable_enable_cpp_int_t;
        cpp_int int_var__pmr_correctable_enable;
        void pmr_correctable_enable (const cpp_int  & l__val);
        cpp_int pmr_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > prt_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__prt_uncorrectable_enable;
        void prt_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int prt_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > prt_correctable_enable_cpp_int_t;
        cpp_int int_var__prt_correctable_enable;
        void prt_correctable_enable (const cpp_int  & l__val);
        cpp_int prt_correctable_enable() const;
    
}; // cap_pxb_csr_int_tgt_ecc_int_enable_clear_t
    
class cap_pxb_csr_int_tgt_ecc_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_int_tgt_ecc_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_int_tgt_ecc_int_test_set_t(string name = "cap_pxb_csr_int_tgt_ecc_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_int_tgt_ecc_int_test_set_t();
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
    
        typedef pu_cpp_int< 1 > aximst_0_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__aximst_0_uncorrectable_interrupt;
        void aximst_0_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int aximst_0_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > aximst_0_correctable_interrupt_cpp_int_t;
        cpp_int int_var__aximst_0_correctable_interrupt;
        void aximst_0_correctable_interrupt (const cpp_int  & l__val);
        cpp_int aximst_0_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > aximst_1_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__aximst_1_uncorrectable_interrupt;
        void aximst_1_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int aximst_1_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > aximst_1_correctable_interrupt_cpp_int_t;
        cpp_int int_var__aximst_1_correctable_interrupt;
        void aximst_1_correctable_interrupt (const cpp_int  & l__val);
        cpp_int aximst_1_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > aximst_2_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__aximst_2_uncorrectable_interrupt;
        void aximst_2_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int aximst_2_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > aximst_2_correctable_interrupt_cpp_int_t;
        cpp_int int_var__aximst_2_correctable_interrupt;
        void aximst_2_correctable_interrupt (const cpp_int  & l__val);
        cpp_int aximst_2_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > aximst_3_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__aximst_3_uncorrectable_interrupt;
        void aximst_3_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int aximst_3_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > aximst_3_correctable_interrupt_cpp_int_t;
        cpp_int int_var__aximst_3_correctable_interrupt;
        void aximst_3_correctable_interrupt (const cpp_int  & l__val);
        cpp_int aximst_3_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > aximst_4_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__aximst_4_uncorrectable_interrupt;
        void aximst_4_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int aximst_4_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > aximst_4_correctable_interrupt_cpp_int_t;
        cpp_int int_var__aximst_4_correctable_interrupt;
        void aximst_4_correctable_interrupt (const cpp_int  & l__val);
        cpp_int aximst_4_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > rxcrbfr_0_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__rxcrbfr_0_uncorrectable_interrupt;
        void rxcrbfr_0_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int rxcrbfr_0_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > rxcrbfr_0_correctable_interrupt_cpp_int_t;
        cpp_int int_var__rxcrbfr_0_correctable_interrupt;
        void rxcrbfr_0_correctable_interrupt (const cpp_int  & l__val);
        cpp_int rxcrbfr_0_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > rxcrbfr_1_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__rxcrbfr_1_uncorrectable_interrupt;
        void rxcrbfr_1_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int rxcrbfr_1_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > rxcrbfr_1_correctable_interrupt_cpp_int_t;
        cpp_int int_var__rxcrbfr_1_correctable_interrupt;
        void rxcrbfr_1_correctable_interrupt (const cpp_int  & l__val);
        cpp_int rxcrbfr_1_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > rxcrbfr_2_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__rxcrbfr_2_uncorrectable_interrupt;
        void rxcrbfr_2_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int rxcrbfr_2_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > rxcrbfr_2_correctable_interrupt_cpp_int_t;
        cpp_int int_var__rxcrbfr_2_correctable_interrupt;
        void rxcrbfr_2_correctable_interrupt (const cpp_int  & l__val);
        cpp_int rxcrbfr_2_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > rxcrbfr_3_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__rxcrbfr_3_uncorrectable_interrupt;
        void rxcrbfr_3_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int rxcrbfr_3_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > rxcrbfr_3_correctable_interrupt_cpp_int_t;
        cpp_int int_var__rxcrbfr_3_correctable_interrupt;
        void rxcrbfr_3_correctable_interrupt (const cpp_int  & l__val);
        cpp_int rxcrbfr_3_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > rxinfo_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__rxinfo_uncorrectable_interrupt;
        void rxinfo_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int rxinfo_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > rxinfo_correctable_interrupt_cpp_int_t;
        cpp_int int_var__rxinfo_correctable_interrupt;
        void rxinfo_correctable_interrupt (const cpp_int  & l__val);
        cpp_int rxinfo_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > cplst_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__cplst_uncorrectable_interrupt;
        void cplst_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int cplst_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > cplst_correctable_interrupt_cpp_int_t;
        cpp_int int_var__cplst_correctable_interrupt;
        void cplst_correctable_interrupt (const cpp_int  & l__val);
        cpp_int cplst_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > romask_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__romask_uncorrectable_interrupt;
        void romask_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int romask_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > romask_correctable_interrupt_cpp_int_t;
        cpp_int int_var__romask_correctable_interrupt;
        void romask_correctable_interrupt (const cpp_int  & l__val);
        cpp_int romask_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > pmr_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__pmr_uncorrectable_interrupt;
        void pmr_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int pmr_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > pmr_correctable_interrupt_cpp_int_t;
        cpp_int int_var__pmr_correctable_interrupt;
        void pmr_correctable_interrupt (const cpp_int  & l__val);
        cpp_int pmr_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > prt_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__prt_uncorrectable_interrupt;
        void prt_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int prt_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > prt_correctable_interrupt_cpp_int_t;
        cpp_int int_var__prt_correctable_interrupt;
        void prt_correctable_interrupt (const cpp_int  & l__val);
        cpp_int prt_correctable_interrupt() const;
    
}; // cap_pxb_csr_int_tgt_ecc_int_test_set_t
    
class cap_pxb_csr_int_tgt_ecc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_int_tgt_ecc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_int_tgt_ecc_t(string name = "cap_pxb_csr_int_tgt_ecc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_int_tgt_ecc_t();
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
    
        cap_pxb_csr_int_tgt_ecc_int_test_set_t intreg;
    
        cap_pxb_csr_int_tgt_ecc_int_test_set_t int_test_set;
    
        cap_pxb_csr_int_tgt_ecc_int_enable_clear_t int_enable_set;
    
        cap_pxb_csr_int_tgt_ecc_int_enable_clear_t int_enable_clear;
    
}; // cap_pxb_csr_int_tgt_ecc_t
    
class cap_pxb_csr_int_itr_ecc_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_int_itr_ecc_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_int_itr_ecc_int_enable_clear_t(string name = "cap_pxb_csr_int_itr_ecc_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_int_itr_ecc_int_enable_clear_t();
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
    
        typedef pu_cpp_int< 1 > pcihdrt_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__pcihdrt_uncorrectable_enable;
        void pcihdrt_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int pcihdrt_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > pcihdrt_correctable_enable_cpp_int_t;
        cpp_int int_var__pcihdrt_correctable_enable;
        void pcihdrt_correctable_enable (const cpp_int  & l__val);
        cpp_int pcihdrt_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > portmap_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__portmap_uncorrectable_enable;
        void portmap_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int portmap_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > portmap_correctable_enable_cpp_int_t;
        cpp_int int_var__portmap_correctable_enable;
        void portmap_correctable_enable (const cpp_int  & l__val);
        cpp_int portmap_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > wrhdr_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__wrhdr_uncorrectable_enable;
        void wrhdr_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int wrhdr_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > wrhdr_correctable_enable_cpp_int_t;
        cpp_int int_var__wrhdr_correctable_enable;
        void wrhdr_correctable_enable (const cpp_int  & l__val);
        cpp_int wrhdr_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > rdhdr_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__rdhdr_uncorrectable_enable;
        void rdhdr_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int rdhdr_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > rdhdr_correctable_enable_cpp_int_t;
        cpp_int int_var__rdhdr_correctable_enable;
        void rdhdr_correctable_enable (const cpp_int  & l__val);
        cpp_int rdhdr_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > wrbfr_0_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__wrbfr_0_uncorrectable_enable;
        void wrbfr_0_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int wrbfr_0_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > wrbfr_0_correctable_enable_cpp_int_t;
        cpp_int int_var__wrbfr_0_correctable_enable;
        void wrbfr_0_correctable_enable (const cpp_int  & l__val);
        cpp_int wrbfr_0_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > wrbfr_1_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__wrbfr_1_uncorrectable_enable;
        void wrbfr_1_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int wrbfr_1_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > wrbfr_1_correctable_enable_cpp_int_t;
        cpp_int int_var__wrbfr_1_correctable_enable;
        void wrbfr_1_correctable_enable (const cpp_int  & l__val);
        cpp_int wrbfr_1_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > wrbfr_2_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__wrbfr_2_uncorrectable_enable;
        void wrbfr_2_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int wrbfr_2_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > wrbfr_2_correctable_enable_cpp_int_t;
        cpp_int int_var__wrbfr_2_correctable_enable;
        void wrbfr_2_correctable_enable (const cpp_int  & l__val);
        cpp_int wrbfr_2_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > wrbfr_3_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__wrbfr_3_uncorrectable_enable;
        void wrbfr_3_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int wrbfr_3_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > wrbfr_3_correctable_enable_cpp_int_t;
        cpp_int int_var__wrbfr_3_correctable_enable;
        void wrbfr_3_correctable_enable (const cpp_int  & l__val);
        cpp_int wrbfr_3_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > cplbfr_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__cplbfr_uncorrectable_enable;
        void cplbfr_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int cplbfr_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > cplbfr_correctable_enable_cpp_int_t;
        cpp_int int_var__cplbfr_correctable_enable;
        void cplbfr_correctable_enable (const cpp_int  & l__val);
        cpp_int cplbfr_correctable_enable() const;
    
}; // cap_pxb_csr_int_itr_ecc_int_enable_clear_t
    
class cap_pxb_csr_int_itr_ecc_intreg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_int_itr_ecc_intreg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_int_itr_ecc_intreg_t(string name = "cap_pxb_csr_int_itr_ecc_intreg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_int_itr_ecc_intreg_t();
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
    
        typedef pu_cpp_int< 1 > pcihdrt_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__pcihdrt_uncorrectable_interrupt;
        void pcihdrt_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int pcihdrt_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > pcihdrt_correctable_interrupt_cpp_int_t;
        cpp_int int_var__pcihdrt_correctable_interrupt;
        void pcihdrt_correctable_interrupt (const cpp_int  & l__val);
        cpp_int pcihdrt_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > portmap_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__portmap_uncorrectable_interrupt;
        void portmap_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int portmap_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > portmap_correctable_interrupt_cpp_int_t;
        cpp_int int_var__portmap_correctable_interrupt;
        void portmap_correctable_interrupt (const cpp_int  & l__val);
        cpp_int portmap_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > wrhdr_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__wrhdr_uncorrectable_interrupt;
        void wrhdr_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int wrhdr_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > wrhdr_correctable_interrupt_cpp_int_t;
        cpp_int int_var__wrhdr_correctable_interrupt;
        void wrhdr_correctable_interrupt (const cpp_int  & l__val);
        cpp_int wrhdr_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > rdhdr_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__rdhdr_uncorrectable_interrupt;
        void rdhdr_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int rdhdr_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > rdhdr_correctable_interrupt_cpp_int_t;
        cpp_int int_var__rdhdr_correctable_interrupt;
        void rdhdr_correctable_interrupt (const cpp_int  & l__val);
        cpp_int rdhdr_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > wrbfr_0_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__wrbfr_0_uncorrectable_interrupt;
        void wrbfr_0_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int wrbfr_0_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > wrbfr_0_correctable_interrupt_cpp_int_t;
        cpp_int int_var__wrbfr_0_correctable_interrupt;
        void wrbfr_0_correctable_interrupt (const cpp_int  & l__val);
        cpp_int wrbfr_0_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > wrbfr_1_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__wrbfr_1_uncorrectable_interrupt;
        void wrbfr_1_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int wrbfr_1_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > wrbfr_1_correctable_interrupt_cpp_int_t;
        cpp_int int_var__wrbfr_1_correctable_interrupt;
        void wrbfr_1_correctable_interrupt (const cpp_int  & l__val);
        cpp_int wrbfr_1_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > wrbfr_2_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__wrbfr_2_uncorrectable_interrupt;
        void wrbfr_2_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int wrbfr_2_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > wrbfr_2_correctable_interrupt_cpp_int_t;
        cpp_int int_var__wrbfr_2_correctable_interrupt;
        void wrbfr_2_correctable_interrupt (const cpp_int  & l__val);
        cpp_int wrbfr_2_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > wrbfr_3_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__wrbfr_3_uncorrectable_interrupt;
        void wrbfr_3_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int wrbfr_3_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > wrbfr_3_correctable_interrupt_cpp_int_t;
        cpp_int int_var__wrbfr_3_correctable_interrupt;
        void wrbfr_3_correctable_interrupt (const cpp_int  & l__val);
        cpp_int wrbfr_3_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > cplbfr_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__cplbfr_uncorrectable_interrupt;
        void cplbfr_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int cplbfr_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > cplbfr_correctable_interrupt_cpp_int_t;
        cpp_int int_var__cplbfr_correctable_interrupt;
        void cplbfr_correctable_interrupt (const cpp_int  & l__val);
        cpp_int cplbfr_correctable_interrupt() const;
    
}; // cap_pxb_csr_int_itr_ecc_intreg_t
    
class cap_pxb_csr_int_itr_ecc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_int_itr_ecc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_int_itr_ecc_t(string name = "cap_pxb_csr_int_itr_ecc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_int_itr_ecc_t();
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
    
        cap_pxb_csr_int_itr_ecc_intreg_t intreg;
    
        cap_pxb_csr_int_itr_ecc_intreg_t int_test_set;
    
        cap_pxb_csr_int_itr_ecc_int_enable_clear_t int_enable_set;
    
        cap_pxb_csr_int_itr_ecc_int_enable_clear_t int_enable_clear;
    
}; // cap_pxb_csr_int_itr_ecc_t
    
class cap_pxb_csr_intreg_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_intreg_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_intreg_status_t(string name = "cap_pxb_csr_intreg_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_intreg_status_t();
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
    
        typedef pu_cpp_int< 1 > int_tgt_ecc_interrupt_cpp_int_t;
        cpp_int int_var__int_tgt_ecc_interrupt;
        void int_tgt_ecc_interrupt (const cpp_int  & l__val);
        cpp_int int_tgt_ecc_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_itr_ecc_interrupt_cpp_int_t;
        cpp_int int_var__int_itr_ecc_interrupt;
        void int_itr_ecc_interrupt (const cpp_int  & l__val);
        cpp_int int_itr_ecc_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_err_interrupt_cpp_int_t;
        cpp_int int_var__int_err_interrupt;
        void int_err_interrupt (const cpp_int  & l__val);
        cpp_int int_err_interrupt() const;
    
}; // cap_pxb_csr_intreg_status_t
    
class cap_pxb_csr_int_groups_int_enable_rw_reg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_int_groups_int_enable_rw_reg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_int_groups_int_enable_rw_reg_t(string name = "cap_pxb_csr_int_groups_int_enable_rw_reg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_int_groups_int_enable_rw_reg_t();
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
    
        typedef pu_cpp_int< 1 > int_tgt_ecc_enable_cpp_int_t;
        cpp_int int_var__int_tgt_ecc_enable;
        void int_tgt_ecc_enable (const cpp_int  & l__val);
        cpp_int int_tgt_ecc_enable() const;
    
        typedef pu_cpp_int< 1 > int_itr_ecc_enable_cpp_int_t;
        cpp_int int_var__int_itr_ecc_enable;
        void int_itr_ecc_enable (const cpp_int  & l__val);
        cpp_int int_itr_ecc_enable() const;
    
        typedef pu_cpp_int< 1 > int_err_enable_cpp_int_t;
        cpp_int int_var__int_err_enable;
        void int_err_enable (const cpp_int  & l__val);
        cpp_int int_err_enable() const;
    
}; // cap_pxb_csr_int_groups_int_enable_rw_reg_t
    
class cap_pxb_csr_intgrp_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_intgrp_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_intgrp_status_t(string name = "cap_pxb_csr_intgrp_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_intgrp_status_t();
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
    
        cap_pxb_csr_intreg_status_t intreg;
    
        cap_pxb_csr_int_groups_int_enable_rw_reg_t int_enable_rw_reg;
    
        cap_pxb_csr_intreg_status_t int_rw_reg;
    
}; // cap_pxb_csr_intgrp_status_t
    
class cap_pxb_csr_sta_inval_cam_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_inval_cam_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_inval_cam_entry_t(string name = "cap_pxb_csr_sta_inval_cam_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_inval_cam_entry_t();
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
    
        typedef pu_cpp_int< 38 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // cap_pxb_csr_sta_inval_cam_entry_t
    
class cap_pxb_csr_sta_inval_cam_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_inval_cam_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_inval_cam_t(string name = "cap_pxb_csr_sta_inval_cam_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_inval_cam_t();
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
    
        #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_sta_inval_cam_entry_t, 64> entry;
        #else 
        cap_pxb_csr_sta_inval_cam_entry_t entry[64];
        #endif
        int get_depth_entry() { return 64; }
    
}; // cap_pxb_csr_sta_inval_cam_t
    
class cap_pxb_csr_filter_addr_ctl_value_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_filter_addr_ctl_value_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_filter_addr_ctl_value_t(string name = "cap_pxb_csr_filter_addr_ctl_value_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_filter_addr_ctl_value_t();
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
    
        typedef pu_cpp_int< 4 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_pxb_csr_filter_addr_ctl_value_t
    
class cap_pxb_csr_filter_addr_ctl_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_filter_addr_ctl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_filter_addr_ctl_t(string name = "cap_pxb_csr_filter_addr_ctl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_filter_addr_ctl_t();
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
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_filter_addr_ctl_value_t, 8> value;
        #else 
        cap_pxb_csr_filter_addr_ctl_value_t value[8];
        #endif
        int get_depth_value() { return 8; }
    
}; // cap_pxb_csr_filter_addr_ctl_t
    
class cap_pxb_csr_filter_addr_hi_data_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_filter_addr_hi_data_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_filter_addr_hi_data_t(string name = "cap_pxb_csr_filter_addr_hi_data_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_filter_addr_hi_data_t();
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
    
        typedef pu_cpp_int< 28 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_pxb_csr_filter_addr_hi_data_t
    
class cap_pxb_csr_filter_addr_hi_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_filter_addr_hi_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_filter_addr_hi_t(string name = "cap_pxb_csr_filter_addr_hi_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_filter_addr_hi_t();
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
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_filter_addr_hi_data_t, 8> data;
        #else 
        cap_pxb_csr_filter_addr_hi_data_t data[8];
        #endif
        int get_depth_data() { return 8; }
    
}; // cap_pxb_csr_filter_addr_hi_t
    
class cap_pxb_csr_filter_addr_lo_data_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_filter_addr_lo_data_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_filter_addr_lo_data_t(string name = "cap_pxb_csr_filter_addr_lo_data_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_filter_addr_lo_data_t();
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
    
        typedef pu_cpp_int< 28 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_pxb_csr_filter_addr_lo_data_t
    
class cap_pxb_csr_filter_addr_lo_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_filter_addr_lo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_filter_addr_lo_t(string name = "cap_pxb_csr_filter_addr_lo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_filter_addr_lo_t();
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
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_filter_addr_lo_data_t, 8> data;
        #else 
        cap_pxb_csr_filter_addr_lo_data_t data[8];
        #endif
        int get_depth_data() { return 8; }
    
}; // cap_pxb_csr_filter_addr_lo_t
    
class cap_pxb_csr_dhs_tgt_pmt_ind_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_pmt_ind_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_pmt_ind_entry_t(string name = "cap_pxb_csr_dhs_tgt_pmt_ind_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_pmt_ind_entry_t();
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
    
        typedef pu_cpp_int< 2 > cmd_cpp_int_t;
        cpp_int int_var__cmd;
        void cmd (const cpp_int  & l__val);
        cpp_int cmd() const;
    
}; // cap_pxb_csr_dhs_tgt_pmt_ind_entry_t
    
class cap_pxb_csr_dhs_tgt_pmt_ind_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_pmt_ind_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_pmt_ind_t(string name = "cap_pxb_csr_dhs_tgt_pmt_ind_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_pmt_ind_t();
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
    
        cap_pxb_csr_dhs_tgt_pmt_ind_entry_t entry;
    
}; // cap_pxb_csr_dhs_tgt_pmt_ind_t
    
class cap_pxb_csr_dhs_tgt_ind_rsp_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_ind_rsp_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_ind_rsp_entry_t(string name = "cap_pxb_csr_dhs_tgt_ind_rsp_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_ind_rsp_entry_t();
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
    
        typedef pu_cpp_int< 32 > data0_cpp_int_t;
        cpp_int int_var__data0;
        void data0 (const cpp_int  & l__val);
        cpp_int data0() const;
    
        typedef pu_cpp_int< 32 > data1_cpp_int_t;
        cpp_int int_var__data1;
        void data1 (const cpp_int  & l__val);
        cpp_int data1() const;
    
        typedef pu_cpp_int< 32 > data2_cpp_int_t;
        cpp_int int_var__data2;
        void data2 (const cpp_int  & l__val);
        cpp_int data2() const;
    
        typedef pu_cpp_int< 32 > data3_cpp_int_t;
        cpp_int int_var__data3;
        void data3 (const cpp_int  & l__val);
        cpp_int data3() const;
    
        typedef pu_cpp_int< 3 > cpl_stat_cpp_int_t;
        cpp_int int_var__cpl_stat;
        void cpl_stat (const cpp_int  & l__val);
        cpp_int cpl_stat() const;
    
        typedef pu_cpp_int< 3 > port_id_cpp_int_t;
        cpp_int int_var__port_id;
        void port_id (const cpp_int  & l__val);
        cpp_int port_id() const;
    
        typedef pu_cpp_int< 7 > axi_id_cpp_int_t;
        cpp_int int_var__axi_id;
        void axi_id (const cpp_int  & l__val);
        cpp_int axi_id() const;
    
        typedef pu_cpp_int< 1 > fetch_rsp_cpp_int_t;
        cpp_int int_var__fetch_rsp;
        void fetch_rsp (const cpp_int  & l__val);
        cpp_int fetch_rsp() const;
    
}; // cap_pxb_csr_dhs_tgt_ind_rsp_entry_t
    
class cap_pxb_csr_dhs_tgt_ind_rsp_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_ind_rsp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_ind_rsp_t(string name = "cap_pxb_csr_dhs_tgt_ind_rsp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_ind_rsp_t();
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
    
        cap_pxb_csr_dhs_tgt_ind_rsp_entry_t entry;
    
}; // cap_pxb_csr_dhs_tgt_ind_rsp_t
    
class cap_pxb_csr_dhs_tgt_notify_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_notify_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_notify_entry_t(string name = "cap_pxb_csr_dhs_tgt_notify_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_notify_entry_t();
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
    
        typedef pu_cpp_int< 16 > posted_cpp_int_t;
        cpp_int int_var__posted;
        void posted (const cpp_int  & l__val);
        cpp_int posted() const;
    
        typedef pu_cpp_int< 16 > completed_cpp_int_t;
        cpp_int int_var__completed;
        void completed (const cpp_int  & l__val);
        cpp_int completed() const;
    
}; // cap_pxb_csr_dhs_tgt_notify_entry_t
    
class cap_pxb_csr_dhs_tgt_notify_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_notify_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_notify_t(string name = "cap_pxb_csr_dhs_tgt_notify_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_notify_t();
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
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_tgt_notify_entry_t, 8> entry;
        #else 
        cap_pxb_csr_dhs_tgt_notify_entry_t entry[8];
        #endif
        int get_depth_entry() { return 8; }
    
}; // cap_pxb_csr_dhs_tgt_notify_t
    
class cap_pxb_csr_dhs_itr_cplbfr15_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr15_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr15_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr15_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr15_entry_t();
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
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr15_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr15_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr15_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr15_t(string name = "cap_pxb_csr_dhs_itr_cplbfr15_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr15_t();
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
    
        #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_itr_cplbfr15_entry_t, 512> entry;
        #else 
        cap_pxb_csr_dhs_itr_cplbfr15_entry_t entry[512];
        #endif
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr15_t
    
class cap_pxb_csr_dhs_itr_cplbfr14_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr14_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr14_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr14_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr14_entry_t();
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
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr14_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr14_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr14_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr14_t(string name = "cap_pxb_csr_dhs_itr_cplbfr14_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr14_t();
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
    
        #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_itr_cplbfr14_entry_t, 512> entry;
        #else 
        cap_pxb_csr_dhs_itr_cplbfr14_entry_t entry[512];
        #endif
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr14_t
    
class cap_pxb_csr_dhs_itr_cplbfr13_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr13_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr13_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr13_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr13_entry_t();
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
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr13_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr13_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr13_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr13_t(string name = "cap_pxb_csr_dhs_itr_cplbfr13_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr13_t();
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
    
        #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_itr_cplbfr13_entry_t, 512> entry;
        #else 
        cap_pxb_csr_dhs_itr_cplbfr13_entry_t entry[512];
        #endif
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr13_t
    
class cap_pxb_csr_dhs_itr_cplbfr12_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr12_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr12_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr12_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr12_entry_t();
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
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr12_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr12_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr12_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr12_t(string name = "cap_pxb_csr_dhs_itr_cplbfr12_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr12_t();
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
    
        #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_itr_cplbfr12_entry_t, 512> entry;
        #else 
        cap_pxb_csr_dhs_itr_cplbfr12_entry_t entry[512];
        #endif
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr12_t
    
class cap_pxb_csr_dhs_itr_cplbfr11_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr11_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr11_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr11_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr11_entry_t();
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
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr11_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr11_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr11_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr11_t(string name = "cap_pxb_csr_dhs_itr_cplbfr11_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr11_t();
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
    
        #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_itr_cplbfr11_entry_t, 512> entry;
        #else 
        cap_pxb_csr_dhs_itr_cplbfr11_entry_t entry[512];
        #endif
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr11_t
    
class cap_pxb_csr_dhs_itr_cplbfr10_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr10_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr10_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr10_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr10_entry_t();
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
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr10_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr10_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr10_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr10_t(string name = "cap_pxb_csr_dhs_itr_cplbfr10_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr10_t();
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
    
        #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_itr_cplbfr10_entry_t, 512> entry;
        #else 
        cap_pxb_csr_dhs_itr_cplbfr10_entry_t entry[512];
        #endif
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr10_t
    
class cap_pxb_csr_dhs_itr_cplbfr9_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr9_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr9_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr9_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr9_entry_t();
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
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr9_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr9_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr9_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr9_t(string name = "cap_pxb_csr_dhs_itr_cplbfr9_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr9_t();
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
    
        #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_itr_cplbfr9_entry_t, 512> entry;
        #else 
        cap_pxb_csr_dhs_itr_cplbfr9_entry_t entry[512];
        #endif
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr9_t
    
class cap_pxb_csr_dhs_itr_cplbfr8_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr8_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr8_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr8_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr8_entry_t();
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
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr8_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr8_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr8_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr8_t(string name = "cap_pxb_csr_dhs_itr_cplbfr8_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr8_t();
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
    
        #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_itr_cplbfr8_entry_t, 512> entry;
        #else 
        cap_pxb_csr_dhs_itr_cplbfr8_entry_t entry[512];
        #endif
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr8_t
    
class cap_pxb_csr_dhs_itr_cplbfr7_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr7_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr7_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr7_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr7_entry_t();
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
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr7_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr7_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr7_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr7_t(string name = "cap_pxb_csr_dhs_itr_cplbfr7_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr7_t();
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
    
        #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_itr_cplbfr7_entry_t, 512> entry;
        #else 
        cap_pxb_csr_dhs_itr_cplbfr7_entry_t entry[512];
        #endif
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr7_t
    
class cap_pxb_csr_dhs_itr_cplbfr6_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr6_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr6_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr6_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr6_entry_t();
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
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr6_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr6_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr6_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr6_t(string name = "cap_pxb_csr_dhs_itr_cplbfr6_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr6_t();
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
    
        #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_itr_cplbfr6_entry_t, 512> entry;
        #else 
        cap_pxb_csr_dhs_itr_cplbfr6_entry_t entry[512];
        #endif
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr6_t
    
class cap_pxb_csr_dhs_itr_cplbfr5_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr5_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr5_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr5_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr5_entry_t();
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
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr5_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr5_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr5_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr5_t(string name = "cap_pxb_csr_dhs_itr_cplbfr5_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr5_t();
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
    
        #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_itr_cplbfr5_entry_t, 512> entry;
        #else 
        cap_pxb_csr_dhs_itr_cplbfr5_entry_t entry[512];
        #endif
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr5_t
    
class cap_pxb_csr_dhs_itr_cplbfr4_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr4_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr4_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr4_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr4_entry_t();
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
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr4_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr4_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr4_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr4_t(string name = "cap_pxb_csr_dhs_itr_cplbfr4_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr4_t();
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
    
        #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_itr_cplbfr4_entry_t, 512> entry;
        #else 
        cap_pxb_csr_dhs_itr_cplbfr4_entry_t entry[512];
        #endif
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr4_t
    
class cap_pxb_csr_dhs_itr_cplbfr3_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr3_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr3_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr3_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr3_entry_t();
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
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr3_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr3_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr3_t(string name = "cap_pxb_csr_dhs_itr_cplbfr3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr3_t();
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
    
        #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_itr_cplbfr3_entry_t, 512> entry;
        #else 
        cap_pxb_csr_dhs_itr_cplbfr3_entry_t entry[512];
        #endif
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr3_t
    
class cap_pxb_csr_dhs_itr_cplbfr2_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr2_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr2_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr2_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr2_entry_t();
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
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr2_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr2_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr2_t(string name = "cap_pxb_csr_dhs_itr_cplbfr2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr2_t();
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
    
        #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_itr_cplbfr2_entry_t, 512> entry;
        #else 
        cap_pxb_csr_dhs_itr_cplbfr2_entry_t entry[512];
        #endif
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr2_t
    
class cap_pxb_csr_dhs_itr_cplbfr1_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr1_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr1_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr1_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr1_entry_t();
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
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr1_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr1_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr1_t(string name = "cap_pxb_csr_dhs_itr_cplbfr1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr1_t();
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
    
        #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_itr_cplbfr1_entry_t, 512> entry;
        #else 
        cap_pxb_csr_dhs_itr_cplbfr1_entry_t entry[512];
        #endif
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr1_t
    
class cap_pxb_csr_dhs_itr_cplbfr0_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr0_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr0_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr0_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr0_entry_t();
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
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr0_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr0_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_cplbfr0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_cplbfr0_t(string name = "cap_pxb_csr_dhs_itr_cplbfr0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_cplbfr0_t();
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
    
        #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_itr_cplbfr0_entry_t, 512> entry;
        #else 
        cap_pxb_csr_dhs_itr_cplbfr0_entry_t entry[512];
        #endif
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr0_t
    
class cap_pxb_csr_dhs_tgt_rc_bdfmap_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_rc_bdfmap_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_rc_bdfmap_entry_t(string name = "cap_pxb_csr_dhs_tgt_rc_bdfmap_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_rc_bdfmap_entry_t();
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
    
        typedef pu_cpp_int< 16 > rc_bdf_cpp_int_t;
        cpp_int int_var__rc_bdf;
        void rc_bdf (const cpp_int  & l__val);
        cpp_int rc_bdf() const;
    
        typedef pu_cpp_int< 3 > port_id_cpp_int_t;
        cpp_int int_var__port_id;
        void port_id (const cpp_int  & l__val);
        cpp_int port_id() const;
    
        typedef pu_cpp_int< 11 > VFID_cpp_int_t;
        cpp_int int_var__VFID;
        void VFID (const cpp_int  & l__val);
        cpp_int VFID() const;
    
}; // cap_pxb_csr_dhs_tgt_rc_bdfmap_entry_t
    
class cap_pxb_csr_dhs_tgt_rc_bdfmap_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_rc_bdfmap_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_rc_bdfmap_t(string name = "cap_pxb_csr_dhs_tgt_rc_bdfmap_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_rc_bdfmap_t();
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
    
        #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_tgt_rc_bdfmap_entry_t, 64> entry;
        #else 
        cap_pxb_csr_dhs_tgt_rc_bdfmap_entry_t entry[64];
        #endif
        int get_depth_entry() { return 64; }
    
}; // cap_pxb_csr_dhs_tgt_rc_bdfmap_t
    
class cap_pxb_csr_dhs_tgt_rdcontext_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_rdcontext_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_rdcontext_entry_t(string name = "cap_pxb_csr_dhs_tgt_rdcontext_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_rdcontext_entry_t();
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
    
        typedef pu_cpp_int< 2 > state_cpp_int_t;
        cpp_int int_var__state;
        void state (const cpp_int  & l__val);
        cpp_int state() const;
    
        typedef pu_cpp_int< 3 > portid_cpp_int_t;
        cpp_int int_var__portid;
        void portid (const cpp_int  & l__val);
        cpp_int portid() const;
    
        typedef pu_cpp_int< 1 > is_nonposted_wr_cpp_int_t;
        cpp_int int_var__is_nonposted_wr;
        void is_nonposted_wr (const cpp_int  & l__val);
        cpp_int is_nonposted_wr() const;
    
        typedef pu_cpp_int< 1 > is_posted_wr_cpp_int_t;
        cpp_int int_var__is_posted_wr;
        void is_posted_wr (const cpp_int  & l__val);
        cpp_int is_posted_wr() const;
    
}; // cap_pxb_csr_dhs_tgt_rdcontext_entry_t
    
class cap_pxb_csr_dhs_tgt_rdcontext_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_rdcontext_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_rdcontext_t(string name = "cap_pxb_csr_dhs_tgt_rdcontext_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_rdcontext_t();
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
    
        #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_tgt_rdcontext_entry_t, 128> entry;
        #else 
        cap_pxb_csr_dhs_tgt_rdcontext_entry_t entry[128];
        #endif
        int get_depth_entry() { return 128; }
    
}; // cap_pxb_csr_dhs_tgt_rdcontext_t
    
class cap_pxb_csr_dhs_itr_rdcontext_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_rdcontext_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_rdcontext_entry_t(string name = "cap_pxb_csr_dhs_itr_rdcontext_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_rdcontext_entry_t();
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
    
        typedef pu_cpp_int< 3 > state_cpp_int_t;
        cpp_int int_var__state;
        void state (const cpp_int  & l__val);
        cpp_int state() const;
    
        typedef pu_cpp_int< 3 > portid_cpp_int_t;
        cpp_int int_var__portid;
        void portid (const cpp_int  & l__val);
        cpp_int portid() const;
    
        typedef pu_cpp_int< 7 > tag_cpp_int_t;
        cpp_int int_var__tag;
        void tag (const cpp_int  & l__val);
        cpp_int tag() const;
    
        typedef pu_cpp_int< 7 > axi_id_cpp_int_t;
        cpp_int int_var__axi_id;
        void axi_id (const cpp_int  & l__val);
        cpp_int axi_id() const;
    
        typedef pu_cpp_int< 9 > raddr_cpp_int_t;
        cpp_int int_var__raddr;
        void raddr (const cpp_int  & l__val);
        cpp_int raddr() const;
    
        typedef pu_cpp_int< 9 > rsize_cpp_int_t;
        cpp_int int_var__rsize;
        void rsize (const cpp_int  & l__val);
        cpp_int rsize() const;
    
        typedef pu_cpp_int< 10 > timer_cpp_int_t;
        cpp_int int_var__timer;
        void timer (const cpp_int  & l__val);
        cpp_int timer() const;
    
        typedef pu_cpp_int< 16 > bdf_cpp_int_t;
        cpp_int int_var__bdf;
        void bdf (const cpp_int  & l__val);
        cpp_int bdf() const;
    
        typedef pu_cpp_int< 1 > aerr_cpp_int_t;
        cpp_int int_var__aerr;
        void aerr (const cpp_int  & l__val);
        cpp_int aerr() const;
    
        typedef pu_cpp_int< 1 > derr_cpp_int_t;
        cpp_int int_var__derr;
        void derr (const cpp_int  & l__val);
        cpp_int derr() const;
    
        typedef pu_cpp_int< 1 > terr_cpp_int_t;
        cpp_int int_var__terr;
        void terr (const cpp_int  & l__val);
        cpp_int terr() const;
    
        typedef pu_cpp_int< 2 > lineaddr_cpp_int_t;
        cpp_int int_var__lineaddr;
        void lineaddr (const cpp_int  & l__val);
        cpp_int lineaddr() const;
    
        typedef pu_cpp_int< 3 > linesize_cpp_int_t;
        cpp_int int_var__linesize;
        void linesize (const cpp_int  & l__val);
        cpp_int linesize() const;
    
        typedef pu_cpp_int< 1 > seg_first_cpp_int_t;
        cpp_int int_var__seg_first;
        void seg_first (const cpp_int  & l__val);
        cpp_int seg_first() const;
    
        typedef pu_cpp_int< 1 > seg_last_cpp_int_t;
        cpp_int int_var__seg_last;
        void seg_last (const cpp_int  & l__val);
        cpp_int seg_last() const;
    
        typedef pu_cpp_int< 7 > seg_next_cpp_int_t;
        cpp_int int_var__seg_next;
        void seg_next (const cpp_int  & l__val);
        cpp_int seg_next() const;
    
        typedef pu_cpp_int< 1 > is_raw_cpp_int_t;
        cpp_int int_var__is_raw;
        void is_raw (const cpp_int  & l__val);
        cpp_int is_raw() const;
    
        typedef pu_cpp_int< 1 > is_nonposted_wr_cpp_int_t;
        cpp_int int_var__is_nonposted_wr;
        void is_nonposted_wr (const cpp_int  & l__val);
        cpp_int is_nonposted_wr() const;
    
        typedef pu_cpp_int< 2 > atomic_state_cpp_int_t;
        cpp_int int_var__atomic_state;
        void atomic_state (const cpp_int  & l__val);
        cpp_int atomic_state() const;
    
        typedef pu_cpp_int< 1 > is_narrow_cpp_int_t;
        cpp_int int_var__is_narrow;
        void is_narrow (const cpp_int  & l__val);
        cpp_int is_narrow() const;
    
}; // cap_pxb_csr_dhs_itr_rdcontext_entry_t
    
class cap_pxb_csr_dhs_itr_rdcontext_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_rdcontext_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_rdcontext_t(string name = "cap_pxb_csr_dhs_itr_rdcontext_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_rdcontext_t();
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
    
        #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_itr_rdcontext_entry_t, 128> entry;
        #else 
        cap_pxb_csr_dhs_itr_rdcontext_entry_t entry[128];
        #endif
        int get_depth_entry() { return 128; }
    
}; // cap_pxb_csr_dhs_itr_rdcontext_t
    
class cap_pxb_csr_dhs_itr_rdhdr_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_rdhdr_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_rdhdr_entry_t(string name = "cap_pxb_csr_dhs_itr_rdhdr_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_rdhdr_entry_t();
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
    
        typedef pu_cpp_int< 80 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 8 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_rdhdr_entry_t
    
class cap_pxb_csr_dhs_itr_rdhdr_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_rdhdr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_rdhdr_t(string name = "cap_pxb_csr_dhs_itr_rdhdr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_rdhdr_t();
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
    
        #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_itr_rdhdr_entry_t, 128> entry;
        #else 
        cap_pxb_csr_dhs_itr_rdhdr_entry_t entry[128];
        #endif
        int get_depth_entry() { return 128; }
    
}; // cap_pxb_csr_dhs_itr_rdhdr_t
    
class cap_pxb_csr_dhs_itr_wrhdr_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_wrhdr_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_wrhdr_entry_t(string name = "cap_pxb_csr_dhs_itr_wrhdr_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_wrhdr_entry_t();
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
    
        typedef pu_cpp_int< 80 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 8 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_wrhdr_entry_t
    
class cap_pxb_csr_dhs_itr_wrhdr_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_wrhdr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_wrhdr_t(string name = "cap_pxb_csr_dhs_itr_wrhdr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_wrhdr_t();
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
    
        #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_itr_wrhdr_entry_t, 128> entry;
        #else 
        cap_pxb_csr_dhs_itr_wrhdr_entry_t entry[128];
        #endif
        int get_depth_entry() { return 128; }
    
}; // cap_pxb_csr_dhs_itr_wrhdr_t
    
class cap_pxb_csr_dhs_itr_wrbfr3_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_wrbfr3_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_wrbfr3_entry_t(string name = "cap_pxb_csr_dhs_itr_wrbfr3_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_wrbfr3_entry_t();
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
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_wrbfr3_entry_t
    
class cap_pxb_csr_dhs_itr_wrbfr3_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_wrbfr3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_wrbfr3_t(string name = "cap_pxb_csr_dhs_itr_wrbfr3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_wrbfr3_t();
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
    
        #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_itr_wrbfr3_entry_t, 1024> entry;
        #else 
        cap_pxb_csr_dhs_itr_wrbfr3_entry_t entry[1024];
        #endif
        int get_depth_entry() { return 1024; }
    
}; // cap_pxb_csr_dhs_itr_wrbfr3_t
    
class cap_pxb_csr_dhs_itr_wrbfr2_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_wrbfr2_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_wrbfr2_entry_t(string name = "cap_pxb_csr_dhs_itr_wrbfr2_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_wrbfr2_entry_t();
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
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_wrbfr2_entry_t
    
class cap_pxb_csr_dhs_itr_wrbfr2_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_wrbfr2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_wrbfr2_t(string name = "cap_pxb_csr_dhs_itr_wrbfr2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_wrbfr2_t();
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
    
        #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_itr_wrbfr2_entry_t, 1024> entry;
        #else 
        cap_pxb_csr_dhs_itr_wrbfr2_entry_t entry[1024];
        #endif
        int get_depth_entry() { return 1024; }
    
}; // cap_pxb_csr_dhs_itr_wrbfr2_t
    
class cap_pxb_csr_dhs_itr_wrbfr1_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_wrbfr1_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_wrbfr1_entry_t(string name = "cap_pxb_csr_dhs_itr_wrbfr1_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_wrbfr1_entry_t();
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
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_wrbfr1_entry_t
    
class cap_pxb_csr_dhs_itr_wrbfr1_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_wrbfr1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_wrbfr1_t(string name = "cap_pxb_csr_dhs_itr_wrbfr1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_wrbfr1_t();
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
    
        #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_itr_wrbfr1_entry_t, 1024> entry;
        #else 
        cap_pxb_csr_dhs_itr_wrbfr1_entry_t entry[1024];
        #endif
        int get_depth_entry() { return 1024; }
    
}; // cap_pxb_csr_dhs_itr_wrbfr1_t
    
class cap_pxb_csr_dhs_itr_wrbfr0_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_wrbfr0_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_wrbfr0_entry_t(string name = "cap_pxb_csr_dhs_itr_wrbfr0_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_wrbfr0_entry_t();
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
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_wrbfr0_entry_t
    
class cap_pxb_csr_dhs_itr_wrbfr0_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_wrbfr0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_wrbfr0_t(string name = "cap_pxb_csr_dhs_itr_wrbfr0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_wrbfr0_t();
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
    
        #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_itr_wrbfr0_entry_t, 1024> entry;
        #else 
        cap_pxb_csr_dhs_itr_wrbfr0_entry_t entry[1024];
        #endif
        int get_depth_entry() { return 1024; }
    
}; // cap_pxb_csr_dhs_itr_wrbfr0_t
    
class cap_pxb_csr_dhs_tgt_aximst4_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_aximst4_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_aximst4_entry_t(string name = "cap_pxb_csr_dhs_tgt_aximst4_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_aximst4_entry_t();
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
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_tgt_aximst4_entry_t
    
class cap_pxb_csr_dhs_tgt_aximst4_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_aximst4_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_aximst4_t(string name = "cap_pxb_csr_dhs_tgt_aximst4_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_aximst4_t();
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
    
        #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_tgt_aximst4_entry_t, 128> entry;
        #else 
        cap_pxb_csr_dhs_tgt_aximst4_entry_t entry[128];
        #endif
        int get_depth_entry() { return 128; }
    
}; // cap_pxb_csr_dhs_tgt_aximst4_t
    
class cap_pxb_csr_dhs_tgt_aximst3_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_aximst3_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_aximst3_entry_t(string name = "cap_pxb_csr_dhs_tgt_aximst3_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_aximst3_entry_t();
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
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_tgt_aximst3_entry_t
    
class cap_pxb_csr_dhs_tgt_aximst3_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_aximst3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_aximst3_t(string name = "cap_pxb_csr_dhs_tgt_aximst3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_aximst3_t();
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
    
        #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_tgt_aximst3_entry_t, 128> entry;
        #else 
        cap_pxb_csr_dhs_tgt_aximst3_entry_t entry[128];
        #endif
        int get_depth_entry() { return 128; }
    
}; // cap_pxb_csr_dhs_tgt_aximst3_t
    
class cap_pxb_csr_dhs_tgt_aximst2_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_aximst2_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_aximst2_entry_t(string name = "cap_pxb_csr_dhs_tgt_aximst2_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_aximst2_entry_t();
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
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_tgt_aximst2_entry_t
    
class cap_pxb_csr_dhs_tgt_aximst2_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_aximst2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_aximst2_t(string name = "cap_pxb_csr_dhs_tgt_aximst2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_aximst2_t();
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
    
        #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_tgt_aximst2_entry_t, 128> entry;
        #else 
        cap_pxb_csr_dhs_tgt_aximst2_entry_t entry[128];
        #endif
        int get_depth_entry() { return 128; }
    
}; // cap_pxb_csr_dhs_tgt_aximst2_t
    
class cap_pxb_csr_dhs_tgt_aximst1_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_aximst1_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_aximst1_entry_t(string name = "cap_pxb_csr_dhs_tgt_aximst1_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_aximst1_entry_t();
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
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_tgt_aximst1_entry_t
    
class cap_pxb_csr_dhs_tgt_aximst1_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_aximst1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_aximst1_t(string name = "cap_pxb_csr_dhs_tgt_aximst1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_aximst1_t();
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
    
        #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_tgt_aximst1_entry_t, 128> entry;
        #else 
        cap_pxb_csr_dhs_tgt_aximst1_entry_t entry[128];
        #endif
        int get_depth_entry() { return 128; }
    
}; // cap_pxb_csr_dhs_tgt_aximst1_t
    
class cap_pxb_csr_dhs_tgt_aximst0_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_aximst0_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_aximst0_entry_t(string name = "cap_pxb_csr_dhs_tgt_aximst0_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_aximst0_entry_t();
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
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_tgt_aximst0_entry_t
    
class cap_pxb_csr_dhs_tgt_aximst0_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_aximst0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_aximst0_t(string name = "cap_pxb_csr_dhs_tgt_aximst0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_aximst0_t();
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
    
        #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_tgt_aximst0_entry_t, 128> entry;
        #else 
        cap_pxb_csr_dhs_tgt_aximst0_entry_t entry[128];
        #endif
        int get_depth_entry() { return 128; }
    
}; // cap_pxb_csr_dhs_tgt_aximst0_t
    
class cap_pxb_csr_dhs_tgt_romask_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_romask_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_romask_entry_t(string name = "cap_pxb_csr_dhs_tgt_romask_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_romask_entry_t();
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
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_tgt_romask_entry_t
    
class cap_pxb_csr_dhs_tgt_romask_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_romask_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_romask_t(string name = "cap_pxb_csr_dhs_tgt_romask_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_romask_t();
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
    
        #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_tgt_romask_entry_t, 128> entry;
        #else 
        cap_pxb_csr_dhs_tgt_romask_entry_t entry[128];
        #endif
        int get_depth_entry() { return 128; }
    
}; // cap_pxb_csr_dhs_tgt_romask_t
    
class cap_pxb_csr_dhs_tgt_cplst_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_cplst_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_cplst_entry_t(string name = "cap_pxb_csr_dhs_tgt_cplst_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_cplst_entry_t();
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
    
        typedef pu_cpp_int< 100 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 8 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_tgt_cplst_entry_t
    
class cap_pxb_csr_dhs_tgt_cplst_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_cplst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_cplst_t(string name = "cap_pxb_csr_dhs_tgt_cplst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_cplst_t();
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
    
        #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_tgt_cplst_entry_t, 128> entry;
        #else 
        cap_pxb_csr_dhs_tgt_cplst_entry_t entry[128];
        #endif
        int get_depth_entry() { return 128; }
    
}; // cap_pxb_csr_dhs_tgt_cplst_t
    
class cap_pxb_csr_dhs_tgt_rxinfo_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_rxinfo_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_rxinfo_entry_t(string name = "cap_pxb_csr_dhs_tgt_rxinfo_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_rxinfo_entry_t();
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
    
        typedef pu_cpp_int< 16 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 6 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_tgt_rxinfo_entry_t
    
class cap_pxb_csr_dhs_tgt_rxinfo_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_rxinfo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_rxinfo_t(string name = "cap_pxb_csr_dhs_tgt_rxinfo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_rxinfo_t();
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
    
        #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_tgt_rxinfo_entry_t, 1024> entry;
        #else 
        cap_pxb_csr_dhs_tgt_rxinfo_entry_t entry[1024];
        #endif
        int get_depth_entry() { return 1024; }
    
}; // cap_pxb_csr_dhs_tgt_rxinfo_t
    
class cap_pxb_csr_dhs_tgt_rxcrbfr3_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_rxcrbfr3_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_rxcrbfr3_entry_t(string name = "cap_pxb_csr_dhs_tgt_rxcrbfr3_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_rxcrbfr3_entry_t();
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
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_tgt_rxcrbfr3_entry_t
    
class cap_pxb_csr_dhs_tgt_rxcrbfr3_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_rxcrbfr3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_rxcrbfr3_t(string name = "cap_pxb_csr_dhs_tgt_rxcrbfr3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_rxcrbfr3_t();
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
    
        #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_tgt_rxcrbfr3_entry_t, 1024> entry;
        #else 
        cap_pxb_csr_dhs_tgt_rxcrbfr3_entry_t entry[1024];
        #endif
        int get_depth_entry() { return 1024; }
    
}; // cap_pxb_csr_dhs_tgt_rxcrbfr3_t
    
class cap_pxb_csr_dhs_tgt_rxcrbfr2_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_rxcrbfr2_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_rxcrbfr2_entry_t(string name = "cap_pxb_csr_dhs_tgt_rxcrbfr2_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_rxcrbfr2_entry_t();
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
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_tgt_rxcrbfr2_entry_t
    
class cap_pxb_csr_dhs_tgt_rxcrbfr2_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_rxcrbfr2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_rxcrbfr2_t(string name = "cap_pxb_csr_dhs_tgt_rxcrbfr2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_rxcrbfr2_t();
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
    
        #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_tgt_rxcrbfr2_entry_t, 1024> entry;
        #else 
        cap_pxb_csr_dhs_tgt_rxcrbfr2_entry_t entry[1024];
        #endif
        int get_depth_entry() { return 1024; }
    
}; // cap_pxb_csr_dhs_tgt_rxcrbfr2_t
    
class cap_pxb_csr_dhs_tgt_rxcrbfr1_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_rxcrbfr1_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_rxcrbfr1_entry_t(string name = "cap_pxb_csr_dhs_tgt_rxcrbfr1_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_rxcrbfr1_entry_t();
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
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_tgt_rxcrbfr1_entry_t
    
class cap_pxb_csr_dhs_tgt_rxcrbfr1_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_rxcrbfr1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_rxcrbfr1_t(string name = "cap_pxb_csr_dhs_tgt_rxcrbfr1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_rxcrbfr1_t();
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
    
        #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_tgt_rxcrbfr1_entry_t, 1024> entry;
        #else 
        cap_pxb_csr_dhs_tgt_rxcrbfr1_entry_t entry[1024];
        #endif
        int get_depth_entry() { return 1024; }
    
}; // cap_pxb_csr_dhs_tgt_rxcrbfr1_t
    
class cap_pxb_csr_dhs_tgt_rxcrbfr0_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_rxcrbfr0_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_rxcrbfr0_entry_t(string name = "cap_pxb_csr_dhs_tgt_rxcrbfr0_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_rxcrbfr0_entry_t();
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
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_tgt_rxcrbfr0_entry_t
    
class cap_pxb_csr_dhs_tgt_rxcrbfr0_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_rxcrbfr0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_rxcrbfr0_t(string name = "cap_pxb_csr_dhs_tgt_rxcrbfr0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_rxcrbfr0_t();
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
    
        #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_tgt_rxcrbfr0_entry_t, 1024> entry;
        #else 
        cap_pxb_csr_dhs_tgt_rxcrbfr0_entry_t entry[1024];
        #endif
        int get_depth_entry() { return 1024; }
    
}; // cap_pxb_csr_dhs_tgt_rxcrbfr0_t
    
class cap_pxb_csr_dhs_tgt_prt_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_prt_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_prt_entry_t(string name = "cap_pxb_csr_dhs_tgt_prt_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_prt_entry_t();
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
    
        typedef pu_cpp_int< 76 > entry_data_cpp_int_t;
        cpp_int int_var__entry_data;
        void entry_data (const cpp_int  & l__val);
        cpp_int entry_data() const;
    
        typedef pu_cpp_int< 8 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_tgt_prt_entry_t
    
class cap_pxb_csr_dhs_tgt_prt_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_prt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_prt_t(string name = "cap_pxb_csr_dhs_tgt_prt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_prt_t();
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
    
        #if 4096 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_tgt_prt_entry_t, 4096> entry;
        #else 
        cap_pxb_csr_dhs_tgt_prt_entry_t entry[4096];
        #endif
        int get_depth_entry() { return 4096; }
    
}; // cap_pxb_csr_dhs_tgt_prt_t
    
class cap_pxb_csr_dhs_tgt_pmr_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_pmr_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_pmr_entry_t(string name = "cap_pxb_csr_dhs_tgt_pmr_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_pmr_entry_t();
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
    
        typedef pu_cpp_int< 110 > entry_data_cpp_int_t;
        cpp_int int_var__entry_data;
        void entry_data (const cpp_int  & l__val);
        cpp_int entry_data() const;
    
        typedef pu_cpp_int< 8 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_tgt_pmr_entry_t
    
class cap_pxb_csr_dhs_tgt_pmr_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_pmr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_pmr_t(string name = "cap_pxb_csr_dhs_tgt_pmr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_pmr_t();
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
    
        #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_tgt_pmr_entry_t, 1024> entry;
        #else 
        cap_pxb_csr_dhs_tgt_pmr_entry_t entry[1024];
        #endif
        int get_depth_entry() { return 1024; }
    
}; // cap_pxb_csr_dhs_tgt_pmr_t
    
class cap_pxb_csr_dhs_tgt_pmt_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_pmt_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_pmt_entry_t(string name = "cap_pxb_csr_dhs_tgt_pmt_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_pmt_entry_t();
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
    
        typedef pu_cpp_int< 64 > x_data_cpp_int_t;
        cpp_int int_var__x_data;
        void x_data (const cpp_int  & l__val);
        cpp_int x_data() const;
    
        typedef pu_cpp_int< 64 > y_data_cpp_int_t;
        cpp_int int_var__y_data;
        void y_data (const cpp_int  & l__val);
        cpp_int y_data() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        cpp_int int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
}; // cap_pxb_csr_dhs_tgt_pmt_entry_t
    
class cap_pxb_csr_dhs_tgt_pmt_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_tgt_pmt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_tgt_pmt_t(string name = "cap_pxb_csr_dhs_tgt_pmt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_tgt_pmt_t();
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
    
        #if 1024 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_tgt_pmt_entry_t, 1024> entry;
        #else 
        cap_pxb_csr_dhs_tgt_pmt_entry_t entry[1024];
        #endif
        int get_depth_entry() { return 1024; }
    
}; // cap_pxb_csr_dhs_tgt_pmt_t
    
class cap_pxb_csr_dhs_itr_portmap_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_portmap_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_portmap_entry_t(string name = "cap_pxb_csr_dhs_itr_portmap_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_portmap_entry_t();
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
    
        typedef pu_cpp_int< 48 > dest_cpp_int_t;
        cpp_int int_var__dest;
        void dest (const cpp_int  & l__val);
        cpp_int dest() const;
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_portmap_entry_t
    
class cap_pxb_csr_dhs_itr_portmap_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_portmap_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_portmap_t(string name = "cap_pxb_csr_dhs_itr_portmap_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_portmap_t();
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
    
        #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_itr_portmap_entry_t, 128> entry;
        #else 
        cap_pxb_csr_dhs_itr_portmap_entry_t entry[128];
        #endif
        int get_depth_entry() { return 128; }
    
}; // cap_pxb_csr_dhs_itr_portmap_t
    
class cap_pxb_csr_dhs_itr_pcihdrt_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_pcihdrt_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_pcihdrt_entry_t(string name = "cap_pxb_csr_dhs_itr_pcihdrt_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_pcihdrt_entry_t();
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
    
        typedef pu_cpp_int< 16 > bdf_cpp_int_t;
        cpp_int int_var__bdf;
        void bdf (const cpp_int  & l__val);
        cpp_int bdf() const;
    
        typedef pu_cpp_int< 1 > td_cpp_int_t;
        cpp_int int_var__td;
        void td (const cpp_int  & l__val);
        cpp_int td() const;
    
        typedef pu_cpp_int< 1 > pasid_en_cpp_int_t;
        cpp_int int_var__pasid_en;
        void pasid_en (const cpp_int  & l__val);
        cpp_int pasid_en() const;
    
        typedef pu_cpp_int< 2 > pasid_source_cpp_int_t;
        cpp_int int_var__pasid_source;
        void pasid_source (const cpp_int  & l__val);
        cpp_int pasid_source() const;
    
        typedef pu_cpp_int< 20 > pasid_value_cpp_int_t;
        cpp_int int_var__pasid_value;
        void pasid_value (const cpp_int  & l__val);
        cpp_int pasid_value() const;
    
        typedef pu_cpp_int< 1 > pasid_exe_cpp_int_t;
        cpp_int int_var__pasid_exe;
        void pasid_exe (const cpp_int  & l__val);
        cpp_int pasid_exe() const;
    
        typedef pu_cpp_int< 1 > pasid_privl_cpp_int_t;
        cpp_int int_var__pasid_privl;
        void pasid_privl (const cpp_int  & l__val);
        cpp_int pasid_privl() const;
    
        typedef pu_cpp_int< 2 > attr2_1_rd_cpp_int_t;
        cpp_int int_var__attr2_1_rd;
        void attr2_1_rd (const cpp_int  & l__val);
        cpp_int attr2_1_rd() const;
    
        typedef pu_cpp_int< 2 > attr2_1_wr_cpp_int_t;
        cpp_int int_var__attr2_1_wr;
        void attr2_1_wr (const cpp_int  & l__val);
        cpp_int attr2_1_wr() const;
    
        typedef pu_cpp_int< 1 > rc_cfg1_cpp_int_t;
        cpp_int int_var__rc_cfg1;
        void rc_cfg1 (const cpp_int  & l__val);
        cpp_int rc_cfg1() const;
    
        typedef pu_cpp_int< 1 > attr0_rd_cpp_int_t;
        cpp_int int_var__attr0_rd;
        void attr0_rd (const cpp_int  & l__val);
        cpp_int attr0_rd() const;
    
        typedef pu_cpp_int< 1 > attr0_wr_cpp_int_t;
        cpp_int int_var__attr0_wr;
        void attr0_wr (const cpp_int  & l__val);
        cpp_int attr0_wr() const;
    
        typedef pu_cpp_int< 2 > ats_at_wr_cpp_int_t;
        cpp_int int_var__ats_at_wr;
        void ats_at_wr (const cpp_int  & l__val);
        cpp_int ats_at_wr() const;
    
        typedef pu_cpp_int< 2 > ats_at_rd_cpp_int_t;
        cpp_int int_var__ats_at_rd;
        void ats_at_rd (const cpp_int  & l__val);
        cpp_int ats_at_rd() const;
    
        typedef pu_cpp_int< 3 > tc_cpp_int_t;
        cpp_int int_var__tc;
        void tc (const cpp_int  & l__val);
        cpp_int tc() const;
    
        typedef pu_cpp_int< 1 > ln_wr_cpp_int_t;
        cpp_int int_var__ln_wr;
        void ln_wr (const cpp_int  & l__val);
        cpp_int ln_wr() const;
    
        typedef pu_cpp_int< 1 > ln_rd_cpp_int_t;
        cpp_int int_var__ln_rd;
        void ln_rd (const cpp_int  & l__val);
        cpp_int ln_rd() const;
    
        typedef pu_cpp_int< 13 > rsvd1_cpp_int_t;
        cpp_int int_var__rsvd1;
        void rsvd1 (const cpp_int  & l__val);
        cpp_int rsvd1() const;
    
        typedef pu_cpp_int< 8 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_pcihdrt_entry_t
    
class cap_pxb_csr_dhs_itr_pcihdrt_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_dhs_itr_pcihdrt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_dhs_itr_pcihdrt_t(string name = "cap_pxb_csr_dhs_itr_pcihdrt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_dhs_itr_pcihdrt_t();
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
    
        #if 2048 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_dhs_itr_pcihdrt_entry_t, 2048> entry;
        #else 
        cap_pxb_csr_dhs_itr_pcihdrt_entry_t entry[2048];
        #endif
        int get_depth_entry() { return 2048; }
    
}; // cap_pxb_csr_dhs_itr_pcihdrt_t
    
class cap_pxb_csr_cnt_axi_bw_mon_wr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cnt_axi_bw_mon_wr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cnt_axi_bw_mon_wr_t(string name = "cap_pxb_csr_cnt_axi_bw_mon_wr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cnt_axi_bw_mon_wr_t();
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
    
}; // cap_pxb_csr_cnt_axi_bw_mon_wr_t
    
class cap_pxb_csr_sta_axi_bw_mon_wr_transactions_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_axi_bw_mon_wr_transactions_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_axi_bw_mon_wr_transactions_t(string name = "cap_pxb_csr_sta_axi_bw_mon_wr_transactions_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_axi_bw_mon_wr_transactions_t();
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
    
        typedef pu_cpp_int< 8 > outstanding_cpp_int_t;
        cpp_int int_var__outstanding;
        void outstanding (const cpp_int  & l__val);
        cpp_int outstanding() const;
    
        typedef pu_cpp_int< 16 > dess_rdy_cpp_int_t;
        cpp_int int_var__dess_rdy;
        void dess_rdy (const cpp_int  & l__val);
        cpp_int dess_rdy() const;
    
}; // cap_pxb_csr_sta_axi_bw_mon_wr_transactions_t
    
class cap_pxb_csr_sta_axi_bw_mon_wr_bandwidth_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_axi_bw_mon_wr_bandwidth_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_axi_bw_mon_wr_bandwidth_t(string name = "cap_pxb_csr_sta_axi_bw_mon_wr_bandwidth_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_axi_bw_mon_wr_bandwidth_t();
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
    
        typedef pu_cpp_int< 16 > maxv_cpp_int_t;
        cpp_int int_var__maxv;
        void maxv (const cpp_int  & l__val);
        cpp_int maxv() const;
    
        typedef pu_cpp_int< 16 > avrg_cpp_int_t;
        cpp_int int_var__avrg;
        void avrg (const cpp_int  & l__val);
        cpp_int avrg() const;
    
}; // cap_pxb_csr_sta_axi_bw_mon_wr_bandwidth_t
    
class cap_pxb_csr_sta_axi_bw_mon_wr_latency_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_axi_bw_mon_wr_latency_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_axi_bw_mon_wr_latency_t(string name = "cap_pxb_csr_sta_axi_bw_mon_wr_latency_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_axi_bw_mon_wr_latency_t();
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
    
        typedef pu_cpp_int< 14 > maxv_cpp_int_t;
        cpp_int int_var__maxv;
        void maxv (const cpp_int  & l__val);
        cpp_int maxv() const;
    
        typedef pu_cpp_int< 14 > avrg_cpp_int_t;
        cpp_int int_var__avrg;
        void avrg (const cpp_int  & l__val);
        cpp_int avrg() const;
    
}; // cap_pxb_csr_sta_axi_bw_mon_wr_latency_t
    
class cap_pxb_csr_cnt_axi_bw_mon_rd_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cnt_axi_bw_mon_rd_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cnt_axi_bw_mon_rd_t(string name = "cap_pxb_csr_cnt_axi_bw_mon_rd_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cnt_axi_bw_mon_rd_t();
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
    
}; // cap_pxb_csr_cnt_axi_bw_mon_rd_t
    
class cap_pxb_csr_sta_axi_bw_mon_rd_transactions_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_axi_bw_mon_rd_transactions_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_axi_bw_mon_rd_transactions_t(string name = "cap_pxb_csr_sta_axi_bw_mon_rd_transactions_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_axi_bw_mon_rd_transactions_t();
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
    
        typedef pu_cpp_int< 8 > outstanding_cpp_int_t;
        cpp_int int_var__outstanding;
        void outstanding (const cpp_int  & l__val);
        cpp_int outstanding() const;
    
        typedef pu_cpp_int< 16 > dess_rdy_cpp_int_t;
        cpp_int int_var__dess_rdy;
        void dess_rdy (const cpp_int  & l__val);
        cpp_int dess_rdy() const;
    
}; // cap_pxb_csr_sta_axi_bw_mon_rd_transactions_t
    
class cap_pxb_csr_sta_axi_bw_mon_rd_bandwidth_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_axi_bw_mon_rd_bandwidth_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_axi_bw_mon_rd_bandwidth_t(string name = "cap_pxb_csr_sta_axi_bw_mon_rd_bandwidth_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_axi_bw_mon_rd_bandwidth_t();
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
    
        typedef pu_cpp_int< 16 > maxv_cpp_int_t;
        cpp_int int_var__maxv;
        void maxv (const cpp_int  & l__val);
        cpp_int maxv() const;
    
        typedef pu_cpp_int< 16 > avrg_cpp_int_t;
        cpp_int int_var__avrg;
        void avrg (const cpp_int  & l__val);
        cpp_int avrg() const;
    
}; // cap_pxb_csr_sta_axi_bw_mon_rd_bandwidth_t
    
class cap_pxb_csr_sta_axi_bw_mon_rd_latency_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_axi_bw_mon_rd_latency_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_axi_bw_mon_rd_latency_t(string name = "cap_pxb_csr_sta_axi_bw_mon_rd_latency_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_axi_bw_mon_rd_latency_t();
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
    
        typedef pu_cpp_int< 14 > maxv_cpp_int_t;
        cpp_int int_var__maxv;
        void maxv (const cpp_int  & l__val);
        cpp_int maxv() const;
    
        typedef pu_cpp_int< 14 > avrg_cpp_int_t;
        cpp_int int_var__avrg;
        void avrg (const cpp_int  & l__val);
        cpp_int avrg() const;
    
}; // cap_pxb_csr_sta_axi_bw_mon_rd_latency_t
    
class cap_pxb_csr_cfg_axi_bw_mon_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_axi_bw_mon_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_axi_bw_mon_t(string name = "cap_pxb_csr_cfg_axi_bw_mon_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_axi_bw_mon_t();
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
    
        typedef pu_cpp_int< 8 > alpha_cpp_int_t;
        cpp_int int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
        typedef pu_cpp_int< 16 > cycle_cpp_int_t;
        cpp_int int_var__cycle;
        void cycle (const cpp_int  & l__val);
        cpp_int cycle() const;
    
        typedef pu_cpp_int< 4 > rcache_cpp_int_t;
        cpp_int int_var__rcache;
        void rcache (const cpp_int  & l__val);
        cpp_int rcache() const;
    
        typedef pu_cpp_int< 4 > rcache_msk_cpp_int_t;
        cpp_int int_var__rcache_msk;
        void rcache_msk (const cpp_int  & l__val);
        cpp_int rcache_msk() const;
    
        typedef pu_cpp_int< 4 > wcache_cpp_int_t;
        cpp_int int_var__wcache;
        void wcache (const cpp_int  & l__val);
        cpp_int wcache() const;
    
        typedef pu_cpp_int< 4 > wcache_msk_cpp_int_t;
        cpp_int int_var__wcache_msk;
        void wcache_msk (const cpp_int  & l__val);
        cpp_int wcache_msk() const;
    
}; // cap_pxb_csr_cfg_axi_bw_mon_t
    
class cap_pxb_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t(string name = "cap_pxb_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t();
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
    
        typedef pu_cpp_int< 40 > msk_cpp_int_t;
        cpp_int int_var__msk;
        void msk (const cpp_int  & l__val);
        cpp_int msk() const;
    
}; // cap_pxb_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t
    
class cap_pxb_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t(string name = "cap_pxb_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t();
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
    
        typedef pu_cpp_int< 40 > msk_cpp_int_t;
        cpp_int int_var__msk;
        void msk (const cpp_int  & l__val);
        cpp_int msk() const;
    
}; // cap_pxb_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t
    
class cap_pxb_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t(string name = "cap_pxb_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t();
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
    
        typedef pu_cpp_int< 40 > msk_cpp_int_t;
        cpp_int int_var__msk;
        void msk (const cpp_int  & l__val);
        cpp_int msk() const;
    
}; // cap_pxb_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t
    
class cap_pxb_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t(string name = "cap_pxb_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t();
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
    
        typedef pu_cpp_int< 40 > msk_cpp_int_t;
        cpp_int int_var__msk;
        void msk (const cpp_int  & l__val);
        cpp_int msk() const;
    
}; // cap_pxb_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t
    
class cap_pxb_csr_cfg_uid2sidLL_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_uid2sidLL_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_uid2sidLL_t(string name = "cap_pxb_csr_cfg_uid2sidLL_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_uid2sidLL_t();
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
    
        typedef pu_cpp_int< 7 > spare_cpp_int_t;
        cpp_int int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
        typedef pu_cpp_int< 2 > mode_cpp_int_t;
        cpp_int int_var__mode;
        void mode (const cpp_int  & l__val);
        cpp_int mode() const;
    
        typedef pu_cpp_int< 7 > base_cpp_int_t;
        cpp_int int_var__base;
        void base (const cpp_int  & l__val);
        cpp_int base() const;
    
}; // cap_pxb_csr_cfg_uid2sidLL_t
    
class cap_pxb_csr_cfg_filter_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_filter_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_filter_t(string name = "cap_pxb_csr_cfg_filter_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_filter_t();
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
    
        typedef pu_cpp_int< 4 > awcache_mask_cpp_int_t;
        cpp_int int_var__awcache_mask;
        void awcache_mask (const cpp_int  & l__val);
        cpp_int awcache_mask() const;
    
        typedef pu_cpp_int< 4 > awcache_match_cpp_int_t;
        cpp_int int_var__awcache_match;
        void awcache_match (const cpp_int  & l__val);
        cpp_int awcache_match() const;
    
        typedef pu_cpp_int< 4 > arcache_mask_cpp_int_t;
        cpp_int int_var__arcache_mask;
        void arcache_mask (const cpp_int  & l__val);
        cpp_int arcache_mask() const;
    
        typedef pu_cpp_int< 4 > arcache_match_cpp_int_t;
        cpp_int int_var__arcache_match;
        void arcache_match (const cpp_int  & l__val);
        cpp_int arcache_match() const;
    
}; // cap_pxb_csr_cfg_filter_t
    
class cap_pxb_csr_cfg_pxb_spare3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_pxb_spare3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_pxb_spare3_t(string name = "cap_pxb_csr_cfg_pxb_spare3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_pxb_spare3_t();
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
    
}; // cap_pxb_csr_cfg_pxb_spare3_t
    
class cap_pxb_csr_cfg_pxb_spare2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_pxb_spare2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_pxb_spare2_t(string name = "cap_pxb_csr_cfg_pxb_spare2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_pxb_spare2_t();
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
    
}; // cap_pxb_csr_cfg_pxb_spare2_t
    
class cap_pxb_csr_cfg_pxb_spare1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_pxb_spare1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_pxb_spare1_t(string name = "cap_pxb_csr_cfg_pxb_spare1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_pxb_spare1_t();
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
    
}; // cap_pxb_csr_cfg_pxb_spare1_t
    
class cap_pxb_csr_cfg_pxb_spare0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_pxb_spare0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_pxb_spare0_t(string name = "cap_pxb_csr_cfg_pxb_spare0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_pxb_spare0_t();
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
    
}; // cap_pxb_csr_cfg_pxb_spare0_t
    
class cap_pxb_csr_csr_intr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_csr_intr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_csr_intr_t(string name = "cap_pxb_csr_csr_intr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_csr_intr_t();
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
    
}; // cap_pxb_csr_csr_intr_t
    
class cap_pxb_csr_sta_tcam_bist_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_tcam_bist_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_tcam_bist_t(string name = "cap_pxb_csr_sta_tcam_bist_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_tcam_bist_t();
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
    
        typedef pu_cpp_int< 1 > done_pass_cpp_int_t;
        cpp_int int_var__done_pass;
        void done_pass (const cpp_int  & l__val);
        cpp_int done_pass() const;
    
        typedef pu_cpp_int< 1 > done_fail_cpp_int_t;
        cpp_int int_var__done_fail;
        void done_fail (const cpp_int  & l__val);
        cpp_int done_fail() const;
    
}; // cap_pxb_csr_sta_tcam_bist_t
    
class cap_pxb_csr_cfg_tcam_bist_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_tcam_bist_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_tcam_bist_t(string name = "cap_pxb_csr_cfg_tcam_bist_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_tcam_bist_t();
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
    
}; // cap_pxb_csr_cfg_tcam_bist_t
    
class cap_pxb_csr_sta_sram_bist_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_sram_bist_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_sram_bist_t(string name = "cap_pxb_csr_sta_sram_bist_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_sram_bist_t();
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
    
        typedef pu_cpp_int< 38 > done_pass_cpp_int_t;
        cpp_int int_var__done_pass;
        void done_pass (const cpp_int  & l__val);
        cpp_int done_pass() const;
    
        typedef pu_cpp_int< 38 > done_fail_cpp_int_t;
        cpp_int int_var__done_fail;
        void done_fail (const cpp_int  & l__val);
        cpp_int done_fail() const;
    
}; // cap_pxb_csr_sta_sram_bist_t
    
class cap_pxb_csr_cfg_sram_bist_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_sram_bist_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_sram_bist_t(string name = "cap_pxb_csr_cfg_sram_bist_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_sram_bist_t();
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
    
        typedef pu_cpp_int< 38 > run_cpp_int_t;
        cpp_int int_var__run;
        void run (const cpp_int  & l__val);
        cpp_int run() const;
    
}; // cap_pxb_csr_cfg_sram_bist_t
    
class cap_pxb_csr_sat_itr_rdlat3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sat_itr_rdlat3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sat_itr_rdlat3_t(string name = "cap_pxb_csr_sat_itr_rdlat3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sat_itr_rdlat3_t();
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
    
        typedef pu_cpp_int< 40 > num_samples_cpp_int_t;
        cpp_int int_var__num_samples;
        void num_samples (const cpp_int  & l__val);
        cpp_int num_samples() const;
    
}; // cap_pxb_csr_sat_itr_rdlat3_t
    
class cap_pxb_csr_sat_itr_rdlat2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sat_itr_rdlat2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sat_itr_rdlat2_t(string name = "cap_pxb_csr_sat_itr_rdlat2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sat_itr_rdlat2_t();
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
    
        typedef pu_cpp_int< 40 > num_samples_cpp_int_t;
        cpp_int int_var__num_samples;
        void num_samples (const cpp_int  & l__val);
        cpp_int num_samples() const;
    
}; // cap_pxb_csr_sat_itr_rdlat2_t
    
class cap_pxb_csr_sat_itr_rdlat1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sat_itr_rdlat1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sat_itr_rdlat1_t(string name = "cap_pxb_csr_sat_itr_rdlat1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sat_itr_rdlat1_t();
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
    
        typedef pu_cpp_int< 40 > num_samples_cpp_int_t;
        cpp_int int_var__num_samples;
        void num_samples (const cpp_int  & l__val);
        cpp_int num_samples() const;
    
}; // cap_pxb_csr_sat_itr_rdlat1_t
    
class cap_pxb_csr_sat_itr_rdlat0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sat_itr_rdlat0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sat_itr_rdlat0_t(string name = "cap_pxb_csr_sat_itr_rdlat0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sat_itr_rdlat0_t();
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
    
        typedef pu_cpp_int< 40 > num_samples_cpp_int_t;
        cpp_int int_var__num_samples;
        void num_samples (const cpp_int  & l__val);
        cpp_int num_samples() const;
    
}; // cap_pxb_csr_sat_itr_rdlat0_t
    
class cap_pxb_csr_cfg_itr_rdlat_measure_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_itr_rdlat_measure_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_itr_rdlat_measure_t(string name = "cap_pxb_csr_cfg_itr_rdlat_measure_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_itr_rdlat_measure_t();
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
    
        typedef pu_cpp_int< 16 > thres2_cpp_int_t;
        cpp_int int_var__thres2;
        void thres2 (const cpp_int  & l__val);
        cpp_int thres2() const;
    
        typedef pu_cpp_int< 16 > thres1_cpp_int_t;
        cpp_int int_var__thres1;
        void thres1 (const cpp_int  & l__val);
        cpp_int thres1() const;
    
        typedef pu_cpp_int< 16 > thres0_cpp_int_t;
        cpp_int int_var__thres0;
        void thres0 (const cpp_int  & l__val);
        cpp_int thres0() const;
    
        typedef pu_cpp_int< 3 > portid_cpp_int_t;
        cpp_int int_var__portid;
        void portid (const cpp_int  & l__val);
        cpp_int portid() const;
    
}; // cap_pxb_csr_cfg_itr_rdlat_measure_t
    
class cap_pxb_csr_sta_diag_spare1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_diag_spare1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_diag_spare1_t(string name = "cap_pxb_csr_sta_diag_spare1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_diag_spare1_t();
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
    
}; // cap_pxb_csr_sta_diag_spare1_t
    
class cap_pxb_csr_cfg_diag_spare1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_diag_spare1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_diag_spare1_t(string name = "cap_pxb_csr_cfg_diag_spare1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_diag_spare1_t();
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
    
}; // cap_pxb_csr_cfg_diag_spare1_t
    
class cap_pxb_csr_sta_diag_spare0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_diag_spare0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_diag_spare0_t(string name = "cap_pxb_csr_sta_diag_spare0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_diag_spare0_t();
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
    
}; // cap_pxb_csr_sta_diag_spare0_t
    
class cap_pxb_csr_cfg_diag_spare0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_diag_spare0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_diag_spare0_t(string name = "cap_pxb_csr_cfg_diag_spare0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_diag_spare0_t();
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
    
}; // cap_pxb_csr_cfg_diag_spare0_t
    
class cap_pxb_csr_cfg_debug_port_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_debug_port_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_debug_port_t(string name = "cap_pxb_csr_cfg_debug_port_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_debug_port_t();
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
    
        typedef pu_cpp_int< 4 > select_cpp_int_t;
        cpp_int int_var__select;
        void select (const cpp_int  & l__val);
        cpp_int select() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        cpp_int int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
}; // cap_pxb_csr_cfg_debug_port_t
    
class cap_pxb_csr_cfg_tgt_axi_attr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_tgt_axi_attr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_tgt_axi_attr_t(string name = "cap_pxb_csr_cfg_tgt_axi_attr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_tgt_axi_attr_t();
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
    
        typedef pu_cpp_int< 4 > arcache_cpp_int_t;
        cpp_int int_var__arcache;
        void arcache (const cpp_int  & l__val);
        cpp_int arcache() const;
    
        typedef pu_cpp_int< 4 > awcache_cpp_int_t;
        cpp_int int_var__awcache;
        void awcache (const cpp_int  & l__val);
        cpp_int awcache() const;
    
        typedef pu_cpp_int< 3 > prot_cpp_int_t;
        cpp_int int_var__prot;
        void prot (const cpp_int  & l__val);
        cpp_int prot() const;
    
        typedef pu_cpp_int< 4 > qos_cpp_int_t;
        cpp_int int_var__qos;
        void qos (const cpp_int  & l__val);
        cpp_int qos() const;
    
        typedef pu_cpp_int< 1 > lock_cpp_int_t;
        cpp_int int_var__lock;
        void lock (const cpp_int  & l__val);
        cpp_int lock() const;
    
}; // cap_pxb_csr_cfg_tgt_axi_attr_t
    
class cap_pxb_csr_sat_tgt_rsp_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sat_tgt_rsp_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sat_tgt_rsp_err_t(string name = "cap_pxb_csr_sat_tgt_rsp_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sat_tgt_rsp_err_t();
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
    
        typedef pu_cpp_int< 8 > ind_cnxt_mismatch_cpp_int_t;
        cpp_int int_var__ind_cnxt_mismatch;
        void ind_cnxt_mismatch (const cpp_int  & l__val);
        cpp_int ind_cnxt_mismatch() const;
    
        typedef pu_cpp_int< 8 > rresp_err_cpp_int_t;
        cpp_int int_var__rresp_err;
        void rresp_err (const cpp_int  & l__val);
        cpp_int rresp_err() const;
    
        typedef pu_cpp_int< 8 > bresp_err_cpp_int_t;
        cpp_int int_var__bresp_err;
        void bresp_err (const cpp_int  & l__val);
        cpp_int bresp_err() const;
    
}; // cap_pxb_csr_sat_tgt_rsp_err_t
    
class cap_pxb_csr_sat_itr_req_portgate_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sat_itr_req_portgate_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sat_itr_req_portgate_t(string name = "cap_pxb_csr_sat_itr_req_portgate_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sat_itr_req_portgate_t();
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
    
        typedef pu_cpp_int< 16 > closed_cpp_int_t;
        cpp_int int_var__closed;
        void closed (const cpp_int  & l__val);
        cpp_int closed() const;
    
}; // cap_pxb_csr_sat_itr_req_portgate_t
    
class cap_pxb_csr_sat_itr_rsp_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sat_itr_rsp_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sat_itr_rsp_err_t(string name = "cap_pxb_csr_sat_itr_rsp_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sat_itr_rsp_err_t();
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
    
        typedef pu_cpp_int< 8 > axi_cpp_int_t;
        cpp_int int_var__axi;
        void axi (const cpp_int  & l__val);
        cpp_int axi() const;
    
        typedef pu_cpp_int< 8 > cpl_timeout_cpp_int_t;
        cpp_int int_var__cpl_timeout;
        void cpl_timeout (const cpp_int  & l__val);
        cpp_int cpl_timeout() const;
    
}; // cap_pxb_csr_sat_itr_rsp_err_t
    
class cap_pxb_csr_sat_itr_cpl_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sat_itr_cpl_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sat_itr_cpl_err_t(string name = "cap_pxb_csr_sat_itr_cpl_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sat_itr_cpl_err_t();
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
    
        typedef pu_cpp_int< 8 > ecrc_cpp_int_t;
        cpp_int int_var__ecrc;
        void ecrc (const cpp_int  & l__val);
        cpp_int ecrc() const;
    
        typedef pu_cpp_int< 8 > rxbuf_ecc_cpp_int_t;
        cpp_int int_var__rxbuf_ecc;
        void rxbuf_ecc (const cpp_int  & l__val);
        cpp_int rxbuf_ecc() const;
    
        typedef pu_cpp_int< 8 > cpl_stat_cpp_int_t;
        cpp_int int_var__cpl_stat;
        void cpl_stat (const cpp_int  & l__val);
        cpp_int cpl_stat() const;
    
        typedef pu_cpp_int< 8 > unexpected_cpp_int_t;
        cpp_int int_var__unexpected;
        void unexpected (const cpp_int  & l__val);
        cpp_int unexpected() const;
    
}; // cap_pxb_csr_sat_itr_cpl_err_t
    
class cap_pxb_csr_sat_itr_xfer_unexpected_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sat_itr_xfer_unexpected_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sat_itr_xfer_unexpected_t(string name = "cap_pxb_csr_sat_itr_xfer_unexpected_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sat_itr_xfer_unexpected_t();
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
    
        typedef pu_cpp_int< 8 > wr256x_cpp_int_t;
        cpp_int int_var__wr256x;
        void wr256x (const cpp_int  & l__val);
        cpp_int wr256x() const;
    
        typedef pu_cpp_int< 8 > rd256x_cpp_int_t;
        cpp_int int_var__rd256x;
        void rd256x (const cpp_int  & l__val);
        cpp_int rd256x() const;
    
        typedef pu_cpp_int< 8 > wr_narrow_cpp_int_t;
        cpp_int int_var__wr_narrow;
        void wr_narrow (const cpp_int  & l__val);
        cpp_int wr_narrow() const;
    
        typedef pu_cpp_int< 8 > rd_narrow_cpp_int_t;
        cpp_int int_var__rd_narrow;
        void rd_narrow (const cpp_int  & l__val);
        cpp_int rd_narrow() const;
    
}; // cap_pxb_csr_sat_itr_xfer_unexpected_t
    
class cap_pxb_csr_sat_itr_req_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sat_itr_req_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sat_itr_req_err_t(string name = "cap_pxb_csr_sat_itr_req_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sat_itr_req_err_t();
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
    
        typedef pu_cpp_int< 8 > unsupp_wr_cpp_int_t;
        cpp_int int_var__unsupp_wr;
        void unsupp_wr (const cpp_int  & l__val);
        cpp_int unsupp_wr() const;
    
        typedef pu_cpp_int< 8 > unsupp_rd_cpp_int_t;
        cpp_int int_var__unsupp_rd;
        void unsupp_rd (const cpp_int  & l__val);
        cpp_int unsupp_rd() const;
    
        typedef pu_cpp_int< 8 > pcihdrt_miss_cpp_int_t;
        cpp_int int_var__pcihdrt_miss;
        void pcihdrt_miss (const cpp_int  & l__val);
        cpp_int pcihdrt_miss() const;
    
        typedef pu_cpp_int< 8 > bus_master_dis_cpp_int_t;
        cpp_int int_var__bus_master_dis;
        void bus_master_dis (const cpp_int  & l__val);
        cpp_int bus_master_dis() const;
    
}; // cap_pxb_csr_sat_itr_req_err_t
    
class cap_pxb_csr_sat_tgt_ind_reason_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sat_tgt_ind_reason_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sat_tgt_ind_reason_t(string name = "cap_pxb_csr_sat_tgt_ind_reason_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sat_tgt_ind_reason_t();
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
    
        typedef pu_cpp_int< 8 > pmr_force_cpp_int_t;
        cpp_int int_var__pmr_force;
        void pmr_force (const cpp_int  & l__val);
        cpp_int pmr_force() const;
    
        typedef pu_cpp_int< 8 > prt_force_cpp_int_t;
        cpp_int int_var__prt_force;
        void prt_force (const cpp_int  & l__val);
        cpp_int prt_force() const;
    
        typedef pu_cpp_int< 8 > msg_cpp_int_t;
        cpp_int int_var__msg;
        void msg (const cpp_int  & l__val);
        cpp_int msg() const;
    
        typedef pu_cpp_int< 8 > atomic_cpp_int_t;
        cpp_int int_var__atomic;
        void atomic (const cpp_int  & l__val);
        cpp_int atomic() const;
    
        typedef pu_cpp_int< 8 > poisoned_cpp_int_t;
        cpp_int int_var__poisoned;
        void poisoned (const cpp_int  & l__val);
        cpp_int poisoned() const;
    
        typedef pu_cpp_int< 8 > unsupp_cpp_int_t;
        cpp_int int_var__unsupp;
        void unsupp (const cpp_int  & l__val);
        cpp_int unsupp() const;
    
        typedef pu_cpp_int< 8 > pmv_cpp_int_t;
        cpp_int int_var__pmv;
        void pmv (const cpp_int  & l__val);
        cpp_int pmv() const;
    
        typedef pu_cpp_int< 8 > db_pmv_cpp_int_t;
        cpp_int int_var__db_pmv;
        void db_pmv (const cpp_int  & l__val);
        cpp_int db_pmv() const;
    
        typedef pu_cpp_int< 8 > pmt_miss_cpp_int_t;
        cpp_int int_var__pmt_miss;
        void pmt_miss (const cpp_int  & l__val);
        cpp_int pmt_miss() const;
    
        typedef pu_cpp_int< 8 > rc_vfid_miss_cpp_int_t;
        cpp_int int_var__rc_vfid_miss;
        void rc_vfid_miss (const cpp_int  & l__val);
        cpp_int rc_vfid_miss() const;
    
        typedef pu_cpp_int< 8 > pmr_prt_miss_cpp_int_t;
        cpp_int int_var__pmr_prt_miss;
        void pmr_prt_miss (const cpp_int  & l__val);
        cpp_int pmr_prt_miss() const;
    
        typedef pu_cpp_int< 8 > prt_oor_cpp_int_t;
        cpp_int int_var__prt_oor;
        void prt_oor (const cpp_int  & l__val);
        cpp_int prt_oor() const;
    
        typedef pu_cpp_int< 8 > bdf_wcard_oor_cpp_int_t;
        cpp_int int_var__bdf_wcard_oor;
        void bdf_wcard_oor (const cpp_int  & l__val);
        cpp_int bdf_wcard_oor() const;
    
        typedef pu_cpp_int< 8 > vfid_oor_cpp_int_t;
        cpp_int int_var__vfid_oor;
        void vfid_oor (const cpp_int  & l__val);
        cpp_int vfid_oor() const;
    
}; // cap_pxb_csr_sat_tgt_ind_reason_t
    
class cap_pxb_csr_cfg_itr_axi_resp_order_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_itr_axi_resp_order_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_itr_axi_resp_order_t(string name = "cap_pxb_csr_cfg_itr_axi_resp_order_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_itr_axi_resp_order_t();
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
    
        typedef pu_cpp_int< 1 > rd_strict_cpp_int_t;
        cpp_int int_var__rd_strict;
        void rd_strict (const cpp_int  & l__val);
        cpp_int rd_strict() const;
    
        typedef pu_cpp_int< 1 > wr_strict_cpp_int_t;
        cpp_int int_var__wr_strict;
        void wr_strict (const cpp_int  & l__val);
        cpp_int wr_strict() const;
    
        typedef pu_cpp_int< 8 > rd_id_limit_cpp_int_t;
        cpp_int int_var__rd_id_limit;
        void rd_id_limit (const cpp_int  & l__val);
        cpp_int rd_id_limit() const;
    
}; // cap_pxb_csr_cfg_itr_axi_resp_order_t
    
class cap_pxb_csr_cfg_itr_atomic_seq_cnt_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_itr_atomic_seq_cnt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_itr_atomic_seq_cnt_t(string name = "cap_pxb_csr_cfg_itr_atomic_seq_cnt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_itr_atomic_seq_cnt_t();
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
    
        typedef pu_cpp_int< 32 > rst_mask_cpp_int_t;
        cpp_int int_var__rst_mask;
        void rst_mask (const cpp_int  & l__val);
        cpp_int rst_mask() const;
    
}; // cap_pxb_csr_cfg_itr_atomic_seq_cnt_t
    
class cap_pxb_csr_sta_itr_cnxt_pending_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_itr_cnxt_pending_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_itr_cnxt_pending_t(string name = "cap_pxb_csr_sta_itr_cnxt_pending_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_itr_cnxt_pending_t();
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
    
        typedef pu_cpp_int< 8 > num_cpp_int_t;
        cpp_int int_var__num;
        void num (const cpp_int  & l__val);
        cpp_int num() const;
    
}; // cap_pxb_csr_sta_itr_cnxt_pending_t
    
class cap_pxb_csr_sta_itr_atomic_seq_cnt_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_itr_atomic_seq_cnt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_itr_atomic_seq_cnt_t(string name = "cap_pxb_csr_sta_itr_atomic_seq_cnt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_itr_atomic_seq_cnt_t();
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
    
        typedef pu_cpp_int< 32 > err_mask_cpp_int_t;
        cpp_int int_var__err_mask;
        void err_mask (const cpp_int  & l__val);
        cpp_int err_mask() const;
    
}; // cap_pxb_csr_sta_itr_atomic_seq_cnt_t
    
class cap_pxb_csr_sta_itr_portfifo_depth_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_itr_portfifo_depth_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_itr_portfifo_depth_t(string name = "cap_pxb_csr_sta_itr_portfifo_depth_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_itr_portfifo_depth_t();
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
    
        typedef pu_cpp_int< 8 > port0_wr_cpp_int_t;
        cpp_int int_var__port0_wr;
        void port0_wr (const cpp_int  & l__val);
        cpp_int port0_wr() const;
    
        typedef pu_cpp_int< 8 > port0_rd_cpp_int_t;
        cpp_int int_var__port0_rd;
        void port0_rd (const cpp_int  & l__val);
        cpp_int port0_rd() const;
    
        typedef pu_cpp_int< 8 > port1_wr_cpp_int_t;
        cpp_int int_var__port1_wr;
        void port1_wr (const cpp_int  & l__val);
        cpp_int port1_wr() const;
    
        typedef pu_cpp_int< 8 > port1_rd_cpp_int_t;
        cpp_int int_var__port1_rd;
        void port1_rd (const cpp_int  & l__val);
        cpp_int port1_rd() const;
    
        typedef pu_cpp_int< 8 > port2_wr_cpp_int_t;
        cpp_int int_var__port2_wr;
        void port2_wr (const cpp_int  & l__val);
        cpp_int port2_wr() const;
    
        typedef pu_cpp_int< 8 > port2_rd_cpp_int_t;
        cpp_int int_var__port2_rd;
        void port2_rd (const cpp_int  & l__val);
        cpp_int port2_rd() const;
    
        typedef pu_cpp_int< 8 > port3_wr_cpp_int_t;
        cpp_int int_var__port3_wr;
        void port3_wr (const cpp_int  & l__val);
        cpp_int port3_wr() const;
    
        typedef pu_cpp_int< 8 > port3_rd_cpp_int_t;
        cpp_int int_var__port3_rd;
        void port3_rd (const cpp_int  & l__val);
        cpp_int port3_rd() const;
    
        typedef pu_cpp_int< 8 > port4_wr_cpp_int_t;
        cpp_int int_var__port4_wr;
        void port4_wr (const cpp_int  & l__val);
        cpp_int port4_wr() const;
    
        typedef pu_cpp_int< 8 > port4_rd_cpp_int_t;
        cpp_int int_var__port4_rd;
        void port4_rd (const cpp_int  & l__val);
        cpp_int port4_rd() const;
    
        typedef pu_cpp_int< 8 > port5_wr_cpp_int_t;
        cpp_int int_var__port5_wr;
        void port5_wr (const cpp_int  & l__val);
        cpp_int port5_wr() const;
    
        typedef pu_cpp_int< 8 > port5_rd_cpp_int_t;
        cpp_int int_var__port5_rd;
        void port5_rd (const cpp_int  & l__val);
        cpp_int port5_rd() const;
    
        typedef pu_cpp_int< 8 > port6_wr_cpp_int_t;
        cpp_int int_var__port6_wr;
        void port6_wr (const cpp_int  & l__val);
        cpp_int port6_wr() const;
    
        typedef pu_cpp_int< 8 > port6_rd_cpp_int_t;
        cpp_int int_var__port6_rd;
        void port6_rd (const cpp_int  & l__val);
        cpp_int port6_rd() const;
    
        typedef pu_cpp_int< 8 > port7_wr_cpp_int_t;
        cpp_int int_var__port7_wr;
        void port7_wr (const cpp_int  & l__val);
        cpp_int port7_wr() const;
    
        typedef pu_cpp_int< 8 > port7_rd_cpp_int_t;
        cpp_int int_var__port7_rd;
        void port7_rd (const cpp_int  & l__val);
        cpp_int port7_rd() const;
    
}; // cap_pxb_csr_sta_itr_portfifo_depth_t
    
class cap_pxb_csr_sta_itr_tags_pending_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_itr_tags_pending_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_itr_tags_pending_t(string name = "cap_pxb_csr_sta_itr_tags_pending_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_itr_tags_pending_t();
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
    
        typedef pu_cpp_int< 8 > port0_cpp_int_t;
        cpp_int int_var__port0;
        void port0 (const cpp_int  & l__val);
        cpp_int port0() const;
    
        typedef pu_cpp_int< 8 > port1_cpp_int_t;
        cpp_int int_var__port1;
        void port1 (const cpp_int  & l__val);
        cpp_int port1() const;
    
        typedef pu_cpp_int< 8 > port2_cpp_int_t;
        cpp_int int_var__port2;
        void port2 (const cpp_int  & l__val);
        cpp_int port2() const;
    
        typedef pu_cpp_int< 8 > port3_cpp_int_t;
        cpp_int int_var__port3;
        void port3 (const cpp_int  & l__val);
        cpp_int port3() const;
    
        typedef pu_cpp_int< 8 > port4_cpp_int_t;
        cpp_int int_var__port4;
        void port4 (const cpp_int  & l__val);
        cpp_int port4() const;
    
        typedef pu_cpp_int< 8 > port5_cpp_int_t;
        cpp_int int_var__port5;
        void port5 (const cpp_int  & l__val);
        cpp_int port5() const;
    
        typedef pu_cpp_int< 8 > port6_cpp_int_t;
        cpp_int int_var__port6;
        void port6 (const cpp_int  & l__val);
        cpp_int port6() const;
    
        typedef pu_cpp_int< 8 > port7_cpp_int_t;
        cpp_int int_var__port7;
        void port7 (const cpp_int  & l__val);
        cpp_int port7() const;
    
}; // cap_pxb_csr_sta_itr_tags_pending_t
    
class cap_pxb_csr_sta_itr_raw_tlp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_itr_raw_tlp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_itr_raw_tlp_t(string name = "cap_pxb_csr_sta_itr_raw_tlp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_itr_raw_tlp_t();
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
    
        typedef pu_cpp_int< 1 > resp_rdy_cpp_int_t;
        cpp_int int_var__resp_rdy;
        void resp_rdy (const cpp_int  & l__val);
        cpp_int resp_rdy() const;
    
        typedef pu_cpp_int< 3 > cpl_stat_cpp_int_t;
        cpp_int int_var__cpl_stat;
        void cpl_stat (const cpp_int  & l__val);
        cpp_int cpl_stat() const;
    
        typedef pu_cpp_int< 1 > cpl_data_err_cpp_int_t;
        cpp_int int_var__cpl_data_err;
        void cpl_data_err (const cpp_int  & l__val);
        cpp_int cpl_data_err() const;
    
        typedef pu_cpp_int< 1 > cpl_timeout_err_cpp_int_t;
        cpp_int int_var__cpl_timeout_err;
        void cpl_timeout_err (const cpp_int  & l__val);
        cpp_int cpl_timeout_err() const;
    
        typedef pu_cpp_int< 1 > req_err_cpp_int_t;
        cpp_int int_var__req_err;
        void req_err (const cpp_int  & l__val);
        cpp_int req_err() const;
    
}; // cap_pxb_csr_sta_itr_raw_tlp_t
    
class cap_pxb_csr_sta_itr_raw_tlp_data_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_itr_raw_tlp_data_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_itr_raw_tlp_data_t(string name = "cap_pxb_csr_sta_itr_raw_tlp_data_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_itr_raw_tlp_data_t();
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
    
        typedef pu_cpp_int< 32 > resp0_cpp_int_t;
        cpp_int int_var__resp0;
        void resp0 (const cpp_int  & l__val);
        cpp_int resp0() const;
    
        typedef pu_cpp_int< 32 > resp1_cpp_int_t;
        cpp_int int_var__resp1;
        void resp1 (const cpp_int  & l__val);
        cpp_int resp1() const;
    
        typedef pu_cpp_int< 32 > resp2_cpp_int_t;
        cpp_int int_var__resp2;
        void resp2 (const cpp_int  & l__val);
        cpp_int resp2() const;
    
        typedef pu_cpp_int< 32 > resp3_cpp_int_t;
        cpp_int int_var__resp3;
        void resp3 (const cpp_int  & l__val);
        cpp_int resp3() const;
    
        typedef pu_cpp_int< 32 > resp4_cpp_int_t;
        cpp_int int_var__resp4;
        void resp4 (const cpp_int  & l__val);
        cpp_int resp4() const;
    
        typedef pu_cpp_int< 32 > resp5_cpp_int_t;
        cpp_int int_var__resp5;
        void resp5 (const cpp_int  & l__val);
        cpp_int resp5() const;
    
        typedef pu_cpp_int< 32 > resp6_cpp_int_t;
        cpp_int int_var__resp6;
        void resp6 (const cpp_int  & l__val);
        cpp_int resp6() const;
    
        typedef pu_cpp_int< 32 > resp7_cpp_int_t;
        cpp_int int_var__resp7;
        void resp7 (const cpp_int  & l__val);
        cpp_int resp7() const;
    
}; // cap_pxb_csr_sta_itr_raw_tlp_data_t
    
class cap_pxb_csr_cfg_itr_raw_tlp_cmd_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_itr_raw_tlp_cmd_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_itr_raw_tlp_cmd_t(string name = "cap_pxb_csr_cfg_itr_raw_tlp_cmd_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_itr_raw_tlp_cmd_t();
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
    
        typedef pu_cpp_int< 4 > dw_cnt_cpp_int_t;
        cpp_int int_var__dw_cnt;
        void dw_cnt (const cpp_int  & l__val);
        cpp_int dw_cnt() const;
    
        typedef pu_cpp_int< 3 > port_id_cpp_int_t;
        cpp_int int_var__port_id;
        void port_id (const cpp_int  & l__val);
        cpp_int port_id() const;
    
        typedef pu_cpp_int< 1 > cmd_go_cpp_int_t;
        cpp_int int_var__cmd_go;
        void cmd_go (const cpp_int  & l__val);
        cpp_int cmd_go() const;
    
}; // cap_pxb_csr_cfg_itr_raw_tlp_cmd_t
    
class cap_pxb_csr_cfg_itr_raw_tlp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_itr_raw_tlp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_itr_raw_tlp_t(string name = "cap_pxb_csr_cfg_itr_raw_tlp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_itr_raw_tlp_t();
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
    
        typedef pu_cpp_int< 32 > dw0_cpp_int_t;
        cpp_int int_var__dw0;
        void dw0 (const cpp_int  & l__val);
        cpp_int dw0() const;
    
        typedef pu_cpp_int< 32 > dw1_cpp_int_t;
        cpp_int int_var__dw1;
        void dw1 (const cpp_int  & l__val);
        cpp_int dw1() const;
    
        typedef pu_cpp_int< 32 > dw2_cpp_int_t;
        cpp_int int_var__dw2;
        void dw2 (const cpp_int  & l__val);
        cpp_int dw2() const;
    
        typedef pu_cpp_int< 32 > dw3_cpp_int_t;
        cpp_int int_var__dw3;
        void dw3 (const cpp_int  & l__val);
        cpp_int dw3() const;
    
        typedef pu_cpp_int< 32 > dw4_cpp_int_t;
        cpp_int int_var__dw4;
        void dw4 (const cpp_int  & l__val);
        cpp_int dw4() const;
    
        typedef pu_cpp_int< 32 > dw5_cpp_int_t;
        cpp_int int_var__dw5;
        void dw5 (const cpp_int  & l__val);
        cpp_int dw5() const;
    
        typedef pu_cpp_int< 32 > dw6_cpp_int_t;
        cpp_int int_var__dw6;
        void dw6 (const cpp_int  & l__val);
        cpp_int dw6() const;
    
        typedef pu_cpp_int< 32 > dw7_cpp_int_t;
        cpp_int int_var__dw7;
        void dw7 (const cpp_int  & l__val);
        cpp_int dw7() const;
    
        typedef pu_cpp_int< 32 > dw8_cpp_int_t;
        cpp_int int_var__dw8;
        void dw8 (const cpp_int  & l__val);
        cpp_int dw8() const;
    
        typedef pu_cpp_int< 32 > dw9_cpp_int_t;
        cpp_int int_var__dw9;
        void dw9 (const cpp_int  & l__val);
        cpp_int dw9() const;
    
        typedef pu_cpp_int< 32 > dw10_cpp_int_t;
        cpp_int int_var__dw10;
        void dw10 (const cpp_int  & l__val);
        cpp_int dw10() const;
    
        typedef pu_cpp_int< 32 > dw11_cpp_int_t;
        cpp_int int_var__dw11;
        void dw11 (const cpp_int  & l__val);
        cpp_int dw11() const;
    
}; // cap_pxb_csr_cfg_itr_raw_tlp_t
    
class cap_pxb_csr_sta_tgt_rxcrbfr_debug_7_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_tgt_rxcrbfr_debug_7_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_tgt_rxcrbfr_debug_7_t(string name = "cap_pxb_csr_sta_tgt_rxcrbfr_debug_7_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_tgt_rxcrbfr_debug_7_t();
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
    
        typedef pu_cpp_int< 10 > waddr_cpp_int_t;
        cpp_int int_var__waddr;
        void waddr (const cpp_int  & l__val);
        cpp_int waddr() const;
    
        typedef pu_cpp_int< 10 > raddr_cpp_int_t;
        cpp_int int_var__raddr;
        void raddr (const cpp_int  & l__val);
        cpp_int raddr() const;
    
        typedef pu_cpp_int< 10 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pxb_csr_sta_tgt_rxcrbfr_debug_7_t
    
class cap_pxb_csr_sta_tgt_rxcrbfr_debug_6_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_tgt_rxcrbfr_debug_6_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_tgt_rxcrbfr_debug_6_t(string name = "cap_pxb_csr_sta_tgt_rxcrbfr_debug_6_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_tgt_rxcrbfr_debug_6_t();
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
    
        typedef pu_cpp_int< 10 > waddr_cpp_int_t;
        cpp_int int_var__waddr;
        void waddr (const cpp_int  & l__val);
        cpp_int waddr() const;
    
        typedef pu_cpp_int< 10 > raddr_cpp_int_t;
        cpp_int int_var__raddr;
        void raddr (const cpp_int  & l__val);
        cpp_int raddr() const;
    
        typedef pu_cpp_int< 10 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pxb_csr_sta_tgt_rxcrbfr_debug_6_t
    
class cap_pxb_csr_sta_tgt_rxcrbfr_debug_5_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_tgt_rxcrbfr_debug_5_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_tgt_rxcrbfr_debug_5_t(string name = "cap_pxb_csr_sta_tgt_rxcrbfr_debug_5_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_tgt_rxcrbfr_debug_5_t();
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
    
        typedef pu_cpp_int< 10 > waddr_cpp_int_t;
        cpp_int int_var__waddr;
        void waddr (const cpp_int  & l__val);
        cpp_int waddr() const;
    
        typedef pu_cpp_int< 10 > raddr_cpp_int_t;
        cpp_int int_var__raddr;
        void raddr (const cpp_int  & l__val);
        cpp_int raddr() const;
    
        typedef pu_cpp_int< 10 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pxb_csr_sta_tgt_rxcrbfr_debug_5_t
    
class cap_pxb_csr_sta_tgt_rxcrbfr_debug_4_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_tgt_rxcrbfr_debug_4_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_tgt_rxcrbfr_debug_4_t(string name = "cap_pxb_csr_sta_tgt_rxcrbfr_debug_4_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_tgt_rxcrbfr_debug_4_t();
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
    
        typedef pu_cpp_int< 10 > waddr_cpp_int_t;
        cpp_int int_var__waddr;
        void waddr (const cpp_int  & l__val);
        cpp_int waddr() const;
    
        typedef pu_cpp_int< 10 > raddr_cpp_int_t;
        cpp_int int_var__raddr;
        void raddr (const cpp_int  & l__val);
        cpp_int raddr() const;
    
        typedef pu_cpp_int< 10 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pxb_csr_sta_tgt_rxcrbfr_debug_4_t
    
class cap_pxb_csr_sta_tgt_rxcrbfr_debug_3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_tgt_rxcrbfr_debug_3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_tgt_rxcrbfr_debug_3_t(string name = "cap_pxb_csr_sta_tgt_rxcrbfr_debug_3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_tgt_rxcrbfr_debug_3_t();
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
    
        typedef pu_cpp_int< 10 > waddr_cpp_int_t;
        cpp_int int_var__waddr;
        void waddr (const cpp_int  & l__val);
        cpp_int waddr() const;
    
        typedef pu_cpp_int< 10 > raddr_cpp_int_t;
        cpp_int int_var__raddr;
        void raddr (const cpp_int  & l__val);
        cpp_int raddr() const;
    
        typedef pu_cpp_int< 10 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pxb_csr_sta_tgt_rxcrbfr_debug_3_t
    
class cap_pxb_csr_sta_tgt_rxcrbfr_debug_2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_tgt_rxcrbfr_debug_2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_tgt_rxcrbfr_debug_2_t(string name = "cap_pxb_csr_sta_tgt_rxcrbfr_debug_2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_tgt_rxcrbfr_debug_2_t();
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
    
        typedef pu_cpp_int< 10 > waddr_cpp_int_t;
        cpp_int int_var__waddr;
        void waddr (const cpp_int  & l__val);
        cpp_int waddr() const;
    
        typedef pu_cpp_int< 10 > raddr_cpp_int_t;
        cpp_int int_var__raddr;
        void raddr (const cpp_int  & l__val);
        cpp_int raddr() const;
    
        typedef pu_cpp_int< 10 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pxb_csr_sta_tgt_rxcrbfr_debug_2_t
    
class cap_pxb_csr_sta_tgt_rxcrbfr_debug_1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_tgt_rxcrbfr_debug_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_tgt_rxcrbfr_debug_1_t(string name = "cap_pxb_csr_sta_tgt_rxcrbfr_debug_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_tgt_rxcrbfr_debug_1_t();
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
    
        typedef pu_cpp_int< 10 > waddr_cpp_int_t;
        cpp_int int_var__waddr;
        void waddr (const cpp_int  & l__val);
        cpp_int waddr() const;
    
        typedef pu_cpp_int< 10 > raddr_cpp_int_t;
        cpp_int int_var__raddr;
        void raddr (const cpp_int  & l__val);
        cpp_int raddr() const;
    
        typedef pu_cpp_int< 10 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pxb_csr_sta_tgt_rxcrbfr_debug_1_t
    
class cap_pxb_csr_sta_tgt_rxcrbfr_debug_0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_tgt_rxcrbfr_debug_0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_tgt_rxcrbfr_debug_0_t(string name = "cap_pxb_csr_sta_tgt_rxcrbfr_debug_0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_tgt_rxcrbfr_debug_0_t();
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
    
        typedef pu_cpp_int< 10 > waddr_cpp_int_t;
        cpp_int int_var__waddr;
        void waddr (const cpp_int  & l__val);
        cpp_int waddr() const;
    
        typedef pu_cpp_int< 10 > raddr_cpp_int_t;
        cpp_int int_var__raddr;
        void raddr (const cpp_int  & l__val);
        cpp_int raddr() const;
    
        typedef pu_cpp_int< 10 > depth_cpp_int_t;
        cpp_int int_var__depth;
        void depth (const cpp_int  & l__val);
        cpp_int depth() const;
    
}; // cap_pxb_csr_sta_tgt_rxcrbfr_debug_0_t
    
class cap_pxb_csr_sta_itr_req_debug_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_itr_req_debug_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_itr_req_debug_t(string name = "cap_pxb_csr_sta_itr_req_debug_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_itr_req_debug_t();
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
    
        typedef pu_cpp_int< 1 > stg0_vld_cpp_int_t;
        cpp_int int_var__stg0_vld;
        void stg0_vld (const cpp_int  & l__val);
        cpp_int stg0_vld() const;
    
        typedef pu_cpp_int< 1 > stg1_vld_cpp_int_t;
        cpp_int int_var__stg1_vld;
        void stg1_vld (const cpp_int  & l__val);
        cpp_int stg1_vld() const;
    
        typedef pu_cpp_int< 1 > stg2_vld_cpp_int_t;
        cpp_int int_var__stg2_vld;
        void stg2_vld (const cpp_int  & l__val);
        cpp_int stg2_vld() const;
    
        typedef pu_cpp_int< 1 > stg3_vld_cpp_int_t;
        cpp_int int_var__stg3_vld;
        void stg3_vld (const cpp_int  & l__val);
        cpp_int stg3_vld() const;
    
        typedef pu_cpp_int< 1 > stg4_vld_cpp_int_t;
        cpp_int int_var__stg4_vld;
        void stg4_vld (const cpp_int  & l__val);
        cpp_int stg4_vld() const;
    
        typedef pu_cpp_int< 1 > stg5_vld_cpp_int_t;
        cpp_int int_var__stg5_vld;
        void stg5_vld (const cpp_int  & l__val);
        cpp_int stg5_vld() const;
    
        typedef pu_cpp_int< 1 > s0_rdy_cpp_int_t;
        cpp_int int_var__s0_rdy;
        void s0_rdy (const cpp_int  & l__val);
        cpp_int s0_rdy() const;
    
        typedef pu_cpp_int< 8 > s0_port_space_avl_vec_cpp_int_t;
        cpp_int int_var__s0_port_space_avl_vec;
        void s0_port_space_avl_vec (const cpp_int  & l__val);
        cpp_int s0_port_space_avl_vec() const;
    
        typedef pu_cpp_int< 8 > s0_tag_avl_vec_cpp_int_t;
        cpp_int int_var__s0_tag_avl_vec;
        void s0_tag_avl_vec (const cpp_int  & l__val);
        cpp_int s0_tag_avl_vec() const;
    
        typedef pu_cpp_int< 1 > itr_cnxt_free_avl0_cpp_int_t;
        cpp_int int_var__itr_cnxt_free_avl0;
        void itr_cnxt_free_avl0 (const cpp_int  & l__val);
        cpp_int itr_cnxt_free_avl0() const;
    
        typedef pu_cpp_int< 1 > itr_cnxt_free_avl1_cpp_int_t;
        cpp_int int_var__itr_cnxt_free_avl1;
        void itr_cnxt_free_avl1 (const cpp_int  & l__val);
        cpp_int itr_cnxt_free_avl1() const;
    
        typedef pu_cpp_int< 3 > s5_req_fifo_depth_cpp_int_t;
        cpp_int int_var__s5_req_fifo_depth;
        void s5_req_fifo_depth (const cpp_int  & l__val);
        cpp_int s5_req_fifo_depth() const;
    
        typedef pu_cpp_int< 3 > s5_seg_state_cpp_int_t;
        cpp_int int_var__s5_seg_state;
        void s5_seg_state (const cpp_int  & l__val);
        cpp_int s5_seg_state() const;
    
        typedef pu_cpp_int< 1 > tx_wrcnt_req_stall_cpp_int_t;
        cpp_int int_var__tx_wrcnt_req_stall;
        void tx_wrcnt_req_stall (const cpp_int  & l__val);
        cpp_int tx_wrcnt_req_stall() const;
    
        typedef pu_cpp_int< 1 > tx_wrcnt_cpl_stall_cpp_int_t;
        cpp_int int_var__tx_wrcnt_cpl_stall;
        void tx_wrcnt_cpl_stall (const cpp_int  & l__val);
        cpp_int tx_wrcnt_cpl_stall() const;
    
        typedef pu_cpp_int< 1 > tx_wrcnt_raw_stall_cpp_int_t;
        cpp_int int_var__tx_wrcnt_raw_stall;
        void tx_wrcnt_raw_stall (const cpp_int  & l__val);
        cpp_int tx_wrcnt_raw_stall() const;
    
        typedef pu_cpp_int< 1 > s5_is_wr_cpp_int_t;
        cpp_int int_var__s5_is_wr;
        void s5_is_wr (const cpp_int  & l__val);
        cpp_int s5_is_wr() const;
    
        typedef pu_cpp_int< 1 > s5_is_msg_cpp_int_t;
        cpp_int int_var__s5_is_msg;
        void s5_is_msg (const cpp_int  & l__val);
        cpp_int s5_is_msg() const;
    
        typedef pu_cpp_int< 1 > cpl_eop_pnd_cpp_int_t;
        cpp_int int_var__cpl_eop_pnd;
        void cpl_eop_pnd (const cpp_int  & l__val);
        cpp_int cpl_eop_pnd() const;
    
        typedef pu_cpp_int< 1 > req_eop_pnd_cpp_int_t;
        cpp_int int_var__req_eop_pnd;
        void req_eop_pnd (const cpp_int  & l__val);
        cpp_int req_eop_pnd() const;
    
        typedef pu_cpp_int< 1 > raw_cmd_pnd_cpp_int_t;
        cpp_int int_var__raw_cmd_pnd;
        void raw_cmd_pnd (const cpp_int  & l__val);
        cpp_int raw_cmd_pnd() const;
    
        typedef pu_cpp_int< 1 > cpl_tx_vld_cpp_int_t;
        cpp_int int_var__cpl_tx_vld;
        void cpl_tx_vld (const cpp_int  & l__val);
        cpp_int cpl_tx_vld() const;
    
        typedef pu_cpp_int< 1 > cpl_tx_rdy_cpp_int_t;
        cpp_int int_var__cpl_tx_rdy;
        void cpl_tx_rdy (const cpp_int  & l__val);
        cpp_int cpl_tx_rdy() const;
    
        typedef pu_cpp_int< 1 > bvalid_cpp_int_t;
        cpp_int int_var__bvalid;
        void bvalid (const cpp_int  & l__val);
        cpp_int bvalid() const;
    
        typedef pu_cpp_int< 1 > bready_cpp_int_t;
        cpp_int int_var__bready;
        void bready (const cpp_int  & l__val);
        cpp_int bready() const;
    
        typedef pu_cpp_int< 1 > awvalid_cpp_int_t;
        cpp_int int_var__awvalid;
        void awvalid (const cpp_int  & l__val);
        cpp_int awvalid() const;
    
        typedef pu_cpp_int< 1 > awready_cpp_int_t;
        cpp_int int_var__awready;
        void awready (const cpp_int  & l__val);
        cpp_int awready() const;
    
        typedef pu_cpp_int< 1 > wvalid_cpp_int_t;
        cpp_int int_var__wvalid;
        void wvalid (const cpp_int  & l__val);
        cpp_int wvalid() const;
    
        typedef pu_cpp_int< 1 > wready_cpp_int_t;
        cpp_int int_var__wready;
        void wready (const cpp_int  & l__val);
        cpp_int wready() const;
    
        typedef pu_cpp_int< 1 > arvalid_cpp_int_t;
        cpp_int int_var__arvalid;
        void arvalid (const cpp_int  & l__val);
        cpp_int arvalid() const;
    
        typedef pu_cpp_int< 1 > arready_cpp_int_t;
        cpp_int int_var__arready;
        void arready (const cpp_int  & l__val);
        cpp_int arready() const;
    
        typedef pu_cpp_int< 1 > rvalid_cpp_int_t;
        cpp_int int_var__rvalid;
        void rvalid (const cpp_int  & l__val);
        cpp_int rvalid() const;
    
        typedef pu_cpp_int< 1 > rready_cpp_int_t;
        cpp_int int_var__rready;
        void rready (const cpp_int  & l__val);
        cpp_int rready() const;
    
}; // cap_pxb_csr_sta_itr_req_debug_t
    
class cap_pxb_csr_sta_tgt_req_debug_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_tgt_req_debug_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_tgt_req_debug_t(string name = "cap_pxb_csr_sta_tgt_req_debug_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_tgt_req_debug_t();
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
    
        typedef pu_cpp_int< 1 > stg0_vld_cpp_int_t;
        cpp_int int_var__stg0_vld;
        void stg0_vld (const cpp_int  & l__val);
        cpp_int stg0_vld() const;
    
        typedef pu_cpp_int< 1 > stg1_vld_cpp_int_t;
        cpp_int int_var__stg1_vld;
        void stg1_vld (const cpp_int  & l__val);
        cpp_int stg1_vld() const;
    
        typedef pu_cpp_int< 1 > stg2_vld_cpp_int_t;
        cpp_int int_var__stg2_vld;
        void stg2_vld (const cpp_int  & l__val);
        cpp_int stg2_vld() const;
    
        typedef pu_cpp_int< 1 > stg3_vld_cpp_int_t;
        cpp_int int_var__stg3_vld;
        void stg3_vld (const cpp_int  & l__val);
        cpp_int stg3_vld() const;
    
        typedef pu_cpp_int< 1 > stg4_vld_cpp_int_t;
        cpp_int int_var__stg4_vld;
        void stg4_vld (const cpp_int  & l__val);
        cpp_int stg4_vld() const;
    
        typedef pu_cpp_int< 1 > stg5_vld_cpp_int_t;
        cpp_int int_var__stg5_vld;
        void stg5_vld (const cpp_int  & l__val);
        cpp_int stg5_vld() const;
    
        typedef pu_cpp_int< 1 > stg6_vld_cpp_int_t;
        cpp_int int_var__stg6_vld;
        void stg6_vld (const cpp_int  & l__val);
        cpp_int stg6_vld() const;
    
        typedef pu_cpp_int< 1 > stg7_vld_cpp_int_t;
        cpp_int int_var__stg7_vld;
        void stg7_vld (const cpp_int  & l__val);
        cpp_int stg7_vld() const;
    
        typedef pu_cpp_int< 1 > stg8_vld_cpp_int_t;
        cpp_int int_var__stg8_vld;
        void stg8_vld (const cpp_int  & l__val);
        cpp_int stg8_vld() const;
    
        typedef pu_cpp_int< 1 > stg9_vld_cpp_int_t;
        cpp_int int_var__stg9_vld;
        void stg9_vld (const cpp_int  & l__val);
        cpp_int stg9_vld() const;
    
        typedef pu_cpp_int< 1 > stg0_rdy_cpp_int_t;
        cpp_int int_var__stg0_rdy;
        void stg0_rdy (const cpp_int  & l__val);
        cpp_int stg0_rdy() const;
    
        typedef pu_cpp_int< 1 > stg1_rdy_cpp_int_t;
        cpp_int int_var__stg1_rdy;
        void stg1_rdy (const cpp_int  & l__val);
        cpp_int stg1_rdy() const;
    
        typedef pu_cpp_int< 1 > stg2_rdy_cpp_int_t;
        cpp_int int_var__stg2_rdy;
        void stg2_rdy (const cpp_int  & l__val);
        cpp_int stg2_rdy() const;
    
        typedef pu_cpp_int< 1 > stg3_rdy_cpp_int_t;
        cpp_int int_var__stg3_rdy;
        void stg3_rdy (const cpp_int  & l__val);
        cpp_int stg3_rdy() const;
    
        typedef pu_cpp_int< 1 > stg4_rdy_cpp_int_t;
        cpp_int int_var__stg4_rdy;
        void stg4_rdy (const cpp_int  & l__val);
        cpp_int stg4_rdy() const;
    
        typedef pu_cpp_int< 1 > stg5_rdy_cpp_int_t;
        cpp_int int_var__stg5_rdy;
        void stg5_rdy (const cpp_int  & l__val);
        cpp_int stg5_rdy() const;
    
        typedef pu_cpp_int< 1 > stg6_rdy_cpp_int_t;
        cpp_int int_var__stg6_rdy;
        void stg6_rdy (const cpp_int  & l__val);
        cpp_int stg6_rdy() const;
    
        typedef pu_cpp_int< 1 > stg7_rdy_cpp_int_t;
        cpp_int int_var__stg7_rdy;
        void stg7_rdy (const cpp_int  & l__val);
        cpp_int stg7_rdy() const;
    
        typedef pu_cpp_int< 1 > stg8_rdy_cpp_int_t;
        cpp_int int_var__stg8_rdy;
        void stg8_rdy (const cpp_int  & l__val);
        cpp_int stg8_rdy() const;
    
        typedef pu_cpp_int< 1 > stg9_rdy_cpp_int_t;
        cpp_int int_var__stg9_rdy;
        void stg9_rdy (const cpp_int  & l__val);
        cpp_int stg9_rdy() const;
    
        typedef pu_cpp_int< 8 > tgt_req_vld_cpp_int_t;
        cpp_int int_var__tgt_req_vld;
        void tgt_req_vld (const cpp_int  & l__val);
        cpp_int tgt_req_vld() const;
    
        typedef pu_cpp_int< 8 > axi_fifo_rdy_cpp_int_t;
        cpp_int int_var__axi_fifo_rdy;
        void axi_fifo_rdy (const cpp_int  & l__val);
        cpp_int axi_fifo_rdy() const;
    
        typedef pu_cpp_int< 8 > axi_fifo_notempty_cpp_int_t;
        cpp_int int_var__axi_fifo_notempty;
        void axi_fifo_notempty (const cpp_int  & l__val);
        cpp_int axi_fifo_notempty() const;
    
        typedef pu_cpp_int< 8 > notify_full_cpp_int_t;
        cpp_int int_var__notify_full;
        void notify_full (const cpp_int  & l__val);
        cpp_int notify_full() const;
    
        typedef pu_cpp_int< 8 > tgt_cnxt_xn_pnd_cpp_int_t;
        cpp_int int_var__tgt_cnxt_xn_pnd;
        void tgt_cnxt_xn_pnd (const cpp_int  & l__val);
        cpp_int tgt_cnxt_xn_pnd() const;
    
        typedef pu_cpp_int< 1 > rxcfg0_vld_cpp_int_t;
        cpp_int int_var__rxcfg0_vld;
        void rxcfg0_vld (const cpp_int  & l__val);
        cpp_int rxcfg0_vld() const;
    
        typedef pu_cpp_int< 1 > hw_rsp_rdy_cpp_int_t;
        cpp_int int_var__hw_rsp_rdy;
        void hw_rsp_rdy (const cpp_int  & l__val);
        cpp_int hw_rsp_rdy() const;
    
        typedef pu_cpp_int< 1 > m_axi_awvalid_cpp_int_t;
        cpp_int int_var__m_axi_awvalid;
        void m_axi_awvalid (const cpp_int  & l__val);
        cpp_int m_axi_awvalid() const;
    
        typedef pu_cpp_int< 1 > m_axi_arvalid_cpp_int_t;
        cpp_int int_var__m_axi_arvalid;
        void m_axi_arvalid (const cpp_int  & l__val);
        cpp_int m_axi_arvalid() const;
    
        typedef pu_cpp_int< 1 > m_axi_wvalid_cpp_int_t;
        cpp_int int_var__m_axi_wvalid;
        void m_axi_wvalid (const cpp_int  & l__val);
        cpp_int m_axi_wvalid() const;
    
        typedef pu_cpp_int< 1 > m_axi_awready_cpp_int_t;
        cpp_int int_var__m_axi_awready;
        void m_axi_awready (const cpp_int  & l__val);
        cpp_int m_axi_awready() const;
    
        typedef pu_cpp_int< 1 > m_axi_arready_cpp_int_t;
        cpp_int int_var__m_axi_arready;
        void m_axi_arready (const cpp_int  & l__val);
        cpp_int m_axi_arready() const;
    
        typedef pu_cpp_int< 1 > m_axi_wready_cpp_int_t;
        cpp_int int_var__m_axi_wready;
        void m_axi_wready (const cpp_int  & l__val);
        cpp_int m_axi_wready() const;
    
        typedef pu_cpp_int< 1 > m_axi_rvalid_cpp_int_t;
        cpp_int int_var__m_axi_rvalid;
        void m_axi_rvalid (const cpp_int  & l__val);
        cpp_int m_axi_rvalid() const;
    
        typedef pu_cpp_int< 1 > m_axi_rready_cpp_int_t;
        cpp_int int_var__m_axi_rready;
        void m_axi_rready (const cpp_int  & l__val);
        cpp_int m_axi_rready() const;
    
        typedef pu_cpp_int< 1 > m_axi_bvalid_cpp_int_t;
        cpp_int int_var__m_axi_bvalid;
        void m_axi_bvalid (const cpp_int  & l__val);
        cpp_int m_axi_bvalid() const;
    
        typedef pu_cpp_int< 1 > m_axi_bready_cpp_int_t;
        cpp_int int_var__m_axi_bready;
        void m_axi_bready (const cpp_int  & l__val);
        cpp_int m_axi_bready() const;
    
        typedef pu_cpp_int< 1 > tgt_rsp_tx_rdy_cpp_int_t;
        cpp_int int_var__tgt_rsp_tx_rdy;
        void tgt_rsp_tx_rdy (const cpp_int  & l__val);
        cpp_int tgt_rsp_tx_rdy() const;
    
        typedef pu_cpp_int< 4 > s7_ax_state0_cpp_int_t;
        cpp_int int_var__s7_ax_state0;
        void s7_ax_state0 (const cpp_int  & l__val);
        cpp_int s7_ax_state0() const;
    
        typedef pu_cpp_int< 4 > s7_ax_state1_cpp_int_t;
        cpp_int int_var__s7_ax_state1;
        void s7_ax_state1 (const cpp_int  & l__val);
        cpp_int s7_ax_state1() const;
    
        typedef pu_cpp_int< 4 > s7_ax_state2_cpp_int_t;
        cpp_int int_var__s7_ax_state2;
        void s7_ax_state2 (const cpp_int  & l__val);
        cpp_int s7_ax_state2() const;
    
        typedef pu_cpp_int< 4 > s7_ax_state3_cpp_int_t;
        cpp_int int_var__s7_ax_state3;
        void s7_ax_state3 (const cpp_int  & l__val);
        cpp_int s7_ax_state3() const;
    
        typedef pu_cpp_int< 4 > s7_ax_state4_cpp_int_t;
        cpp_int int_var__s7_ax_state4;
        void s7_ax_state4 (const cpp_int  & l__val);
        cpp_int s7_ax_state4() const;
    
        typedef pu_cpp_int< 4 > s7_ax_state5_cpp_int_t;
        cpp_int int_var__s7_ax_state5;
        void s7_ax_state5 (const cpp_int  & l__val);
        cpp_int s7_ax_state5() const;
    
        typedef pu_cpp_int< 4 > s7_ax_state6_cpp_int_t;
        cpp_int int_var__s7_ax_state6;
        void s7_ax_state6 (const cpp_int  & l__val);
        cpp_int s7_ax_state6() const;
    
        typedef pu_cpp_int< 4 > s7_ax_state7_cpp_int_t;
        cpp_int int_var__s7_ax_state7;
        void s7_ax_state7 (const cpp_int  & l__val);
        cpp_int s7_ax_state7() const;
    
}; // cap_pxb_csr_sta_tgt_req_debug_t
    
class cap_pxb_csr_sta_tgt_marker_rx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_tgt_marker_rx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_tgt_marker_rx_t(string name = "cap_pxb_csr_sta_tgt_marker_rx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_tgt_marker_rx_t();
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
    
        typedef pu_cpp_int< 1 > port0_cpp_int_t;
        cpp_int int_var__port0;
        void port0 (const cpp_int  & l__val);
        cpp_int port0() const;
    
        typedef pu_cpp_int< 1 > port1_cpp_int_t;
        cpp_int int_var__port1;
        void port1 (const cpp_int  & l__val);
        cpp_int port1() const;
    
        typedef pu_cpp_int< 1 > port2_cpp_int_t;
        cpp_int int_var__port2;
        void port2 (const cpp_int  & l__val);
        cpp_int port2() const;
    
        typedef pu_cpp_int< 1 > port3_cpp_int_t;
        cpp_int int_var__port3;
        void port3 (const cpp_int  & l__val);
        cpp_int port3() const;
    
        typedef pu_cpp_int< 1 > port4_cpp_int_t;
        cpp_int int_var__port4;
        void port4 (const cpp_int  & l__val);
        cpp_int port4() const;
    
        typedef pu_cpp_int< 1 > port5_cpp_int_t;
        cpp_int int_var__port5;
        void port5 (const cpp_int  & l__val);
        cpp_int port5() const;
    
        typedef pu_cpp_int< 1 > port6_cpp_int_t;
        cpp_int int_var__port6;
        void port6 (const cpp_int  & l__val);
        cpp_int port6() const;
    
        typedef pu_cpp_int< 1 > port7_cpp_int_t;
        cpp_int int_var__port7;
        void port7 (const cpp_int  & l__val);
        cpp_int port7() const;
    
}; // cap_pxb_csr_sta_tgt_marker_rx_t
    
class cap_pxb_csr_cnt_itr_tot_atomic_req_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cnt_itr_tot_atomic_req_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cnt_itr_tot_atomic_req_t(string name = "cap_pxb_csr_cnt_itr_tot_atomic_req_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cnt_itr_tot_atomic_req_t();
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
    
        typedef pu_cpp_int< 40 > sent_cpp_int_t;
        cpp_int int_var__sent;
        void sent (const cpp_int  & l__val);
        cpp_int sent() const;
    
}; // cap_pxb_csr_cnt_itr_tot_atomic_req_t
    
class cap_pxb_csr_cnt_itr_intx_assert_msg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cnt_itr_intx_assert_msg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cnt_itr_intx_assert_msg_t(string name = "cap_pxb_csr_cnt_itr_intx_assert_msg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cnt_itr_intx_assert_msg_t();
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
    
        typedef pu_cpp_int< 40 > sent_cpp_int_t;
        cpp_int int_var__sent;
        void sent (const cpp_int  & l__val);
        cpp_int sent() const;
    
}; // cap_pxb_csr_cnt_itr_intx_assert_msg_t
    
class cap_pxb_csr_cnt_itr_tot_msg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cnt_itr_tot_msg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cnt_itr_tot_msg_t(string name = "cap_pxb_csr_cnt_itr_tot_msg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cnt_itr_tot_msg_t();
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
    
        typedef pu_cpp_int< 40 > sent_cpp_int_t;
        cpp_int int_var__sent;
        void sent (const cpp_int  & l__val);
        cpp_int sent() const;
    
}; // cap_pxb_csr_cnt_itr_tot_msg_t
    
class cap_pxb_csr_sta_itr_axi_rd_num_ids_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_itr_axi_rd_num_ids_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_itr_axi_rd_num_ids_t(string name = "cap_pxb_csr_sta_itr_axi_rd_num_ids_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_itr_axi_rd_num_ids_t();
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
    
        typedef pu_cpp_int< 8 > pending_cpp_int_t;
        cpp_int int_var__pending;
        void pending (const cpp_int  & l__val);
        cpp_int pending() const;
    
}; // cap_pxb_csr_sta_itr_axi_rd_num_ids_t
    
class cap_pxb_csr_sta_itr_axi_wr_num_ids_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_itr_axi_wr_num_ids_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_itr_axi_wr_num_ids_t(string name = "cap_pxb_csr_sta_itr_axi_wr_num_ids_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_itr_axi_wr_num_ids_t();
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
    
        typedef pu_cpp_int< 8 > pending_cpp_int_t;
        cpp_int int_var__pending;
        void pending (const cpp_int  & l__val);
        cpp_int pending() const;
    
}; // cap_pxb_csr_sta_itr_axi_wr_num_ids_t
    
class cap_pxb_csr_sta_tgt_tot_axi_rd_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_tgt_tot_axi_rd_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_tgt_tot_axi_rd_t(string name = "cap_pxb_csr_sta_tgt_tot_axi_rd_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_tgt_tot_axi_rd_t();
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
    
        typedef pu_cpp_int< 50 > num_bytes_cpp_int_t;
        cpp_int int_var__num_bytes;
        void num_bytes (const cpp_int  & l__val);
        cpp_int num_bytes() const;
    
}; // cap_pxb_csr_sta_tgt_tot_axi_rd_t
    
class cap_pxb_csr_sta_tgt_tot_axi_wr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_tgt_tot_axi_wr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_tgt_tot_axi_wr_t(string name = "cap_pxb_csr_sta_tgt_tot_axi_wr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_tgt_tot_axi_wr_t();
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
    
        typedef pu_cpp_int< 50 > num_bytes_cpp_int_t;
        cpp_int int_var__num_bytes;
        void num_bytes (const cpp_int  & l__val);
        cpp_int num_bytes() const;
    
}; // cap_pxb_csr_sta_tgt_tot_axi_wr_t
    
class cap_pxb_csr_cnt_tgt_axi_rd64_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cnt_tgt_axi_rd64_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cnt_tgt_axi_rd64_t(string name = "cap_pxb_csr_cnt_tgt_axi_rd64_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cnt_tgt_axi_rd64_t();
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
    
        typedef pu_cpp_int< 40 > req_cpp_int_t;
        cpp_int int_var__req;
        void req (const cpp_int  & l__val);
        cpp_int req() const;
    
}; // cap_pxb_csr_cnt_tgt_axi_rd64_t
    
class cap_pxb_csr_cnt_tgt_tot_axi_rd_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cnt_tgt_tot_axi_rd_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cnt_tgt_tot_axi_rd_t(string name = "cap_pxb_csr_cnt_tgt_tot_axi_rd_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cnt_tgt_tot_axi_rd_t();
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
    
        typedef pu_cpp_int< 40 > req_cpp_int_t;
        cpp_int int_var__req;
        void req (const cpp_int  & l__val);
        cpp_int req() const;
    
}; // cap_pxb_csr_cnt_tgt_tot_axi_rd_t
    
class cap_pxb_csr_cnt_tgt_rsp_ca_ur_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cnt_tgt_rsp_ca_ur_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cnt_tgt_rsp_ca_ur_t(string name = "cap_pxb_csr_cnt_tgt_rsp_ca_ur_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cnt_tgt_rsp_ca_ur_t();
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
    
        typedef pu_cpp_int< 40 > cnt_cpp_int_t;
        cpp_int int_var__cnt;
        void cnt (const cpp_int  & l__val);
        cpp_int cnt() const;
    
}; // cap_pxb_csr_cnt_tgt_rsp_ca_ur_t
    
class cap_pxb_csr_cnt_tgt_db32_axi_wr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cnt_tgt_db32_axi_wr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cnt_tgt_db32_axi_wr_t(string name = "cap_pxb_csr_cnt_tgt_db32_axi_wr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cnt_tgt_db32_axi_wr_t();
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
    
        typedef pu_cpp_int< 40 > req_cpp_int_t;
        cpp_int int_var__req;
        void req (const cpp_int  & l__val);
        cpp_int req() const;
    
}; // cap_pxb_csr_cnt_tgt_db32_axi_wr_t
    
class cap_pxb_csr_cnt_tgt_db64_axi_wr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cnt_tgt_db64_axi_wr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cnt_tgt_db64_axi_wr_t(string name = "cap_pxb_csr_cnt_tgt_db64_axi_wr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cnt_tgt_db64_axi_wr_t();
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
    
        typedef pu_cpp_int< 40 > req_cpp_int_t;
        cpp_int int_var__req;
        void req (const cpp_int  & l__val);
        cpp_int req() const;
    
}; // cap_pxb_csr_cnt_tgt_db64_axi_wr_t
    
class cap_pxb_csr_cnt_tgt_axi_wr64_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cnt_tgt_axi_wr64_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cnt_tgt_axi_wr64_t(string name = "cap_pxb_csr_cnt_tgt_axi_wr64_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cnt_tgt_axi_wr64_t();
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
    
        typedef pu_cpp_int< 40 > req_cpp_int_t;
        cpp_int int_var__req;
        void req (const cpp_int  & l__val);
        cpp_int req() const;
    
}; // cap_pxb_csr_cnt_tgt_axi_wr64_t
    
class cap_pxb_csr_cnt_tgt_tot_axi_wr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cnt_tgt_tot_axi_wr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cnt_tgt_tot_axi_wr_t(string name = "cap_pxb_csr_cnt_tgt_tot_axi_wr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cnt_tgt_tot_axi_wr_t();
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
    
        typedef pu_cpp_int< 40 > req_cpp_int_t;
        cpp_int int_var__req;
        void req (const cpp_int  & l__val);
        cpp_int req() const;
    
}; // cap_pxb_csr_cnt_tgt_tot_axi_wr_t
    
class cap_pxb_csr_cnt_itr_axi_rd256_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cnt_itr_axi_rd256_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cnt_itr_axi_rd256_t(string name = "cap_pxb_csr_cnt_itr_axi_rd256_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cnt_itr_axi_rd256_t();
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
    
        typedef pu_cpp_int< 40 > req_cpp_int_t;
        cpp_int int_var__req;
        void req (const cpp_int  & l__val);
        cpp_int req() const;
    
}; // cap_pxb_csr_cnt_itr_axi_rd256_t
    
class cap_pxb_csr_cnt_itr_axi_rd64_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cnt_itr_axi_rd64_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cnt_itr_axi_rd64_t(string name = "cap_pxb_csr_cnt_itr_axi_rd64_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cnt_itr_axi_rd64_t();
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
    
        typedef pu_cpp_int< 40 > req_cpp_int_t;
        cpp_int int_var__req;
        void req (const cpp_int  & l__val);
        cpp_int req() const;
    
}; // cap_pxb_csr_cnt_itr_axi_rd64_t
    
class cap_pxb_csr_cnt_itr_tot_axi_rd_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cnt_itr_tot_axi_rd_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cnt_itr_tot_axi_rd_t(string name = "cap_pxb_csr_cnt_itr_tot_axi_rd_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cnt_itr_tot_axi_rd_t();
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
    
        typedef pu_cpp_int< 40 > req_cpp_int_t;
        cpp_int int_var__req;
        void req (const cpp_int  & l__val);
        cpp_int req() const;
    
}; // cap_pxb_csr_cnt_itr_tot_axi_rd_t
    
class cap_pxb_csr_sta_itr_tot_axi_rd_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_itr_tot_axi_rd_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_itr_tot_axi_rd_t(string name = "cap_pxb_csr_sta_itr_tot_axi_rd_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_itr_tot_axi_rd_t();
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
    
        typedef pu_cpp_int< 50 > num_bytes_cpp_int_t;
        cpp_int int_var__num_bytes;
        void num_bytes (const cpp_int  & l__val);
        cpp_int num_bytes() const;
    
}; // cap_pxb_csr_sta_itr_tot_axi_rd_t
    
class cap_pxb_csr_sta_itr_tot_axi_wr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_itr_tot_axi_wr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_itr_tot_axi_wr_t(string name = "cap_pxb_csr_sta_itr_tot_axi_wr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_itr_tot_axi_wr_t();
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
    
        typedef pu_cpp_int< 50 > num_bytes_cpp_int_t;
        cpp_int int_var__num_bytes;
        void num_bytes (const cpp_int  & l__val);
        cpp_int num_bytes() const;
    
}; // cap_pxb_csr_sta_itr_tot_axi_wr_t
    
class cap_pxb_csr_cnt_itr_axi_wr256_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cnt_itr_axi_wr256_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cnt_itr_axi_wr256_t(string name = "cap_pxb_csr_cnt_itr_axi_wr256_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cnt_itr_axi_wr256_t();
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
    
        typedef pu_cpp_int< 40 > req_cpp_int_t;
        cpp_int int_var__req;
        void req (const cpp_int  & l__val);
        cpp_int req() const;
    
}; // cap_pxb_csr_cnt_itr_axi_wr256_t
    
class cap_pxb_csr_cnt_itr_axi_wr64_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cnt_itr_axi_wr64_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cnt_itr_axi_wr64_t(string name = "cap_pxb_csr_cnt_itr_axi_wr64_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cnt_itr_axi_wr64_t();
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
    
        typedef pu_cpp_int< 40 > req_cpp_int_t;
        cpp_int int_var__req;
        void req (const cpp_int  & l__val);
        cpp_int req() const;
    
}; // cap_pxb_csr_cnt_itr_axi_wr64_t
    
class cap_pxb_csr_cnt_itr_tot_axi_wr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cnt_itr_tot_axi_wr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cnt_itr_tot_axi_wr_t(string name = "cap_pxb_csr_cnt_itr_tot_axi_wr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cnt_itr_tot_axi_wr_t();
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
    
        typedef pu_cpp_int< 40 > req_cpp_int_t;
        cpp_int int_var__req;
        void req (const cpp_int  & l__val);
        cpp_int req() const;
    
}; // cap_pxb_csr_cnt_itr_tot_axi_wr_t
    
class cap_pxb_csr_sta_tgt_ind_info_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_tgt_ind_info_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_tgt_ind_info_t(string name = "cap_pxb_csr_sta_tgt_ind_info_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_tgt_ind_info_t();
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
    
        typedef pu_cpp_int< 1 > pending_cpp_int_t;
        cpp_int int_var__pending;
        void pending (const cpp_int  & l__val);
        cpp_int pending() const;
    
        typedef pu_cpp_int< 7 > ramaddr_cpp_int_t;
        cpp_int int_var__ramaddr;
        void ramaddr (const cpp_int  & l__val);
        cpp_int ramaddr() const;
    
}; // cap_pxb_csr_sta_tgt_ind_info_t
    
class cap_pxb_csr_sta_tgt_axi_pending_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_tgt_axi_pending_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_tgt_axi_pending_t(string name = "cap_pxb_csr_sta_tgt_axi_pending_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_tgt_axi_pending_t();
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
    
        typedef pu_cpp_int< 8 > ids_p0_cpp_int_t;
        cpp_int int_var__ids_p0;
        void ids_p0 (const cpp_int  & l__val);
        cpp_int ids_p0() const;
    
        typedef pu_cpp_int< 8 > ids_p1_cpp_int_t;
        cpp_int int_var__ids_p1;
        void ids_p1 (const cpp_int  & l__val);
        cpp_int ids_p1() const;
    
        typedef pu_cpp_int< 8 > ids_p2_cpp_int_t;
        cpp_int int_var__ids_p2;
        void ids_p2 (const cpp_int  & l__val);
        cpp_int ids_p2() const;
    
        typedef pu_cpp_int< 8 > ids_p3_cpp_int_t;
        cpp_int int_var__ids_p3;
        void ids_p3 (const cpp_int  & l__val);
        cpp_int ids_p3() const;
    
        typedef pu_cpp_int< 8 > ids_p4_cpp_int_t;
        cpp_int int_var__ids_p4;
        void ids_p4 (const cpp_int  & l__val);
        cpp_int ids_p4() const;
    
        typedef pu_cpp_int< 8 > ids_p5_cpp_int_t;
        cpp_int int_var__ids_p5;
        void ids_p5 (const cpp_int  & l__val);
        cpp_int ids_p5() const;
    
        typedef pu_cpp_int< 8 > ids_p6_cpp_int_t;
        cpp_int int_var__ids_p6;
        void ids_p6 (const cpp_int  & l__val);
        cpp_int ids_p6() const;
    
        typedef pu_cpp_int< 8 > ids_p7_cpp_int_t;
        cpp_int int_var__ids_p7;
        void ids_p7 (const cpp_int  & l__val);
        cpp_int ids_p7() const;
    
}; // cap_pxb_csr_sta_tgt_axi_pending_t
    
class cap_pxb_csr_cfg_tgt_vf_cfg_stride_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_tgt_vf_cfg_stride_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_tgt_vf_cfg_stride_t(string name = "cap_pxb_csr_cfg_tgt_vf_cfg_stride_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_tgt_vf_cfg_stride_t();
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
    
        typedef pu_cpp_int< 5 > p_shift_cpp_int_t;
        cpp_int int_var__p_shift;
        void p_shift (const cpp_int  & l__val);
        cpp_int p_shift() const;
    
        typedef pu_cpp_int< 5 > b_shift_cpp_int_t;
        cpp_int int_var__b_shift;
        void b_shift (const cpp_int  & l__val);
        cpp_int b_shift() const;
    
        typedef pu_cpp_int< 5 > d_shift_cpp_int_t;
        cpp_int int_var__d_shift;
        void d_shift (const cpp_int  & l__val);
        cpp_int d_shift() const;
    
        typedef pu_cpp_int< 5 > f_shift_cpp_int_t;
        cpp_int int_var__f_shift;
        void f_shift (const cpp_int  & l__val);
        cpp_int f_shift() const;
    
        typedef pu_cpp_int< 10 > resource_dwsize_cpp_int_t;
        cpp_int int_var__resource_dwsize;
        void resource_dwsize (const cpp_int  & l__val);
        cpp_int resource_dwsize() const;
    
}; // cap_pxb_csr_cfg_tgt_vf_cfg_stride_t
    
class cap_pxb_csr_sat_tgt_rx_drop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sat_tgt_rx_drop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sat_tgt_rx_drop_t(string name = "cap_pxb_csr_sat_tgt_rx_drop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sat_tgt_rx_drop_t();
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
    
        typedef pu_cpp_int< 8 > port0_cpp_int_t;
        cpp_int int_var__port0;
        void port0 (const cpp_int  & l__val);
        cpp_int port0() const;
    
        typedef pu_cpp_int< 8 > port1_cpp_int_t;
        cpp_int int_var__port1;
        void port1 (const cpp_int  & l__val);
        cpp_int port1() const;
    
        typedef pu_cpp_int< 8 > port2_cpp_int_t;
        cpp_int int_var__port2;
        void port2 (const cpp_int  & l__val);
        cpp_int port2() const;
    
        typedef pu_cpp_int< 8 > port3_cpp_int_t;
        cpp_int int_var__port3;
        void port3 (const cpp_int  & l__val);
        cpp_int port3() const;
    
        typedef pu_cpp_int< 8 > port4_cpp_int_t;
        cpp_int int_var__port4;
        void port4 (const cpp_int  & l__val);
        cpp_int port4() const;
    
        typedef pu_cpp_int< 8 > port5_cpp_int_t;
        cpp_int int_var__port5;
        void port5 (const cpp_int  & l__val);
        cpp_int port5() const;
    
        typedef pu_cpp_int< 8 > port6_cpp_int_t;
        cpp_int int_var__port6;
        void port6 (const cpp_int  & l__val);
        cpp_int port6() const;
    
        typedef pu_cpp_int< 8 > port7_cpp_int_t;
        cpp_int int_var__port7;
        void port7 (const cpp_int  & l__val);
        cpp_int port7() const;
    
}; // cap_pxb_csr_sat_tgt_rx_drop_t
    
class cap_pxb_csr_sta_tgt_ecc_romask_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_tgt_ecc_romask_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_tgt_ecc_romask_t(string name = "cap_pxb_csr_sta_tgt_ecc_romask_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_tgt_ecc_romask_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 7 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 7 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_tgt_ecc_romask_t
    
class cap_pxb_csr_sta_tgt_ecc_prt_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_tgt_ecc_prt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_tgt_ecc_prt_t(string name = "cap_pxb_csr_sta_tgt_ecc_prt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_tgt_ecc_prt_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 8 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 12 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_tgt_ecc_prt_t
    
class cap_pxb_csr_sta_tgt_ecc_pmr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_tgt_ecc_pmr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_tgt_ecc_pmr_t(string name = "cap_pxb_csr_sta_tgt_ecc_pmr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_tgt_ecc_pmr_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 8 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 10 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_tgt_ecc_pmr_t
    
class cap_pxb_csr_sta_tgt_ecc_cplst_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_tgt_ecc_cplst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_tgt_ecc_cplst_t(string name = "cap_pxb_csr_sta_tgt_ecc_cplst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_tgt_ecc_cplst_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 8 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 7 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_tgt_ecc_cplst_t
    
class cap_pxb_csr_sta_tgt_ecc_rxinfo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_tgt_ecc_rxinfo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_tgt_ecc_rxinfo_t(string name = "cap_pxb_csr_sta_tgt_ecc_rxinfo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_tgt_ecc_rxinfo_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 6 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 10 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_tgt_ecc_rxinfo_t
    
class cap_pxb_csr_sta_tgt_ecc_aximst_4_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_tgt_ecc_aximst_4_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_tgt_ecc_aximst_4_t(string name = "cap_pxb_csr_sta_tgt_ecc_aximst_4_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_tgt_ecc_aximst_4_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 9 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 7 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_tgt_ecc_aximst_4_t
    
class cap_pxb_csr_sta_tgt_ecc_aximst_3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_tgt_ecc_aximst_3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_tgt_ecc_aximst_3_t(string name = "cap_pxb_csr_sta_tgt_ecc_aximst_3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_tgt_ecc_aximst_3_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 9 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 7 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_tgt_ecc_aximst_3_t
    
class cap_pxb_csr_sta_tgt_ecc_aximst_2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_tgt_ecc_aximst_2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_tgt_ecc_aximst_2_t(string name = "cap_pxb_csr_sta_tgt_ecc_aximst_2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_tgt_ecc_aximst_2_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 9 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 7 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_tgt_ecc_aximst_2_t
    
class cap_pxb_csr_sta_tgt_ecc_aximst_1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_tgt_ecc_aximst_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_tgt_ecc_aximst_1_t(string name = "cap_pxb_csr_sta_tgt_ecc_aximst_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_tgt_ecc_aximst_1_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 9 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 7 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_tgt_ecc_aximst_1_t
    
class cap_pxb_csr_sta_tgt_ecc_aximst_0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_tgt_ecc_aximst_0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_tgt_ecc_aximst_0_t(string name = "cap_pxb_csr_sta_tgt_ecc_aximst_0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_tgt_ecc_aximst_0_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 9 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 7 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_tgt_ecc_aximst_0_t
    
class cap_pxb_csr_sta_tgt_ecc_rxcrbfr_3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_tgt_ecc_rxcrbfr_3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_tgt_ecc_rxcrbfr_3_t(string name = "cap_pxb_csr_sta_tgt_ecc_rxcrbfr_3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_tgt_ecc_rxcrbfr_3_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 9 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 10 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_tgt_ecc_rxcrbfr_3_t
    
class cap_pxb_csr_sta_tgt_ecc_rxcrbfr_2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_tgt_ecc_rxcrbfr_2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_tgt_ecc_rxcrbfr_2_t(string name = "cap_pxb_csr_sta_tgt_ecc_rxcrbfr_2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_tgt_ecc_rxcrbfr_2_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 9 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 10 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_tgt_ecc_rxcrbfr_2_t
    
class cap_pxb_csr_sta_tgt_ecc_rxcrbfr_1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_tgt_ecc_rxcrbfr_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_tgt_ecc_rxcrbfr_1_t(string name = "cap_pxb_csr_sta_tgt_ecc_rxcrbfr_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_tgt_ecc_rxcrbfr_1_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 9 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 10 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_tgt_ecc_rxcrbfr_1_t
    
class cap_pxb_csr_sta_tgt_ecc_rxcrbfr_0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_tgt_ecc_rxcrbfr_0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_tgt_ecc_rxcrbfr_0_t(string name = "cap_pxb_csr_sta_tgt_ecc_rxcrbfr_0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_tgt_ecc_rxcrbfr_0_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 9 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 10 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_tgt_ecc_rxcrbfr_0_t
    
class cap_pxb_csr_sta_itr_ecc_cplbfr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_itr_ecc_cplbfr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_itr_ecc_cplbfr_t(string name = "cap_pxb_csr_sta_itr_ecc_cplbfr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_itr_ecc_cplbfr_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 7 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 13 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_itr_ecc_cplbfr_t
    
class cap_pxb_csr_sta_itr_ecc_rdhdr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_itr_ecc_rdhdr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_itr_ecc_rdhdr_t(string name = "cap_pxb_csr_sta_itr_ecc_rdhdr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_itr_ecc_rdhdr_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 8 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 7 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_itr_ecc_rdhdr_t
    
class cap_pxb_csr_sta_itr_ecc_wrhdr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_itr_ecc_wrhdr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_itr_ecc_wrhdr_t(string name = "cap_pxb_csr_sta_itr_ecc_wrhdr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_itr_ecc_wrhdr_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 8 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 7 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_itr_ecc_wrhdr_t
    
class cap_pxb_csr_sta_itr_ecc_portmap_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_itr_ecc_portmap_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_itr_ecc_portmap_t(string name = "cap_pxb_csr_sta_itr_ecc_portmap_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_itr_ecc_portmap_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 7 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 7 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_itr_ecc_portmap_t
    
class cap_pxb_csr_sta_itr_ecc_pcihdrt_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_itr_ecc_pcihdrt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_itr_ecc_pcihdrt_t(string name = "cap_pxb_csr_sta_itr_ecc_pcihdrt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_itr_ecc_pcihdrt_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 8 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 11 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_itr_ecc_pcihdrt_t
    
class cap_pxb_csr_sta_itr_ecc_wrbfr_3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_itr_ecc_wrbfr_3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_itr_ecc_wrbfr_3_t(string name = "cap_pxb_csr_sta_itr_ecc_wrbfr_3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_itr_ecc_wrbfr_3_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 9 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 10 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_itr_ecc_wrbfr_3_t
    
class cap_pxb_csr_sta_itr_ecc_wrbfr_2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_itr_ecc_wrbfr_2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_itr_ecc_wrbfr_2_t(string name = "cap_pxb_csr_sta_itr_ecc_wrbfr_2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_itr_ecc_wrbfr_2_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 9 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 10 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_itr_ecc_wrbfr_2_t
    
class cap_pxb_csr_sta_itr_ecc_wrbfr_1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_itr_ecc_wrbfr_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_itr_ecc_wrbfr_1_t(string name = "cap_pxb_csr_sta_itr_ecc_wrbfr_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_itr_ecc_wrbfr_1_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 9 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 10 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_itr_ecc_wrbfr_1_t
    
class cap_pxb_csr_sta_itr_ecc_wrbfr_0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_itr_ecc_wrbfr_0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_itr_ecc_wrbfr_0_t(string name = "cap_pxb_csr_sta_itr_ecc_wrbfr_0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_itr_ecc_wrbfr_0_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 9 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 10 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_itr_ecc_wrbfr_0_t
    
class cap_pxb_csr_cfg_tgt_ecc_disable_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_tgt_ecc_disable_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_tgt_ecc_disable_t(string name = "cap_pxb_csr_cfg_tgt_ecc_disable_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_tgt_ecc_disable_t();
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
    
        typedef pu_cpp_int< 1 > rxinfo_cor_cpp_int_t;
        cpp_int int_var__rxinfo_cor;
        void rxinfo_cor (const cpp_int  & l__val);
        cpp_int rxinfo_cor() const;
    
        typedef pu_cpp_int< 1 > rxinfo_det_cpp_int_t;
        cpp_int int_var__rxinfo_det;
        void rxinfo_det (const cpp_int  & l__val);
        cpp_int rxinfo_det() const;
    
        typedef pu_cpp_int< 1 > rxinfo_dhs_cpp_int_t;
        cpp_int int_var__rxinfo_dhs;
        void rxinfo_dhs (const cpp_int  & l__val);
        cpp_int rxinfo_dhs() const;
    
        typedef pu_cpp_int< 1 > pmr_cor_cpp_int_t;
        cpp_int int_var__pmr_cor;
        void pmr_cor (const cpp_int  & l__val);
        cpp_int pmr_cor() const;
    
        typedef pu_cpp_int< 1 > pmr_det_cpp_int_t;
        cpp_int int_var__pmr_det;
        void pmr_det (const cpp_int  & l__val);
        cpp_int pmr_det() const;
    
        typedef pu_cpp_int< 1 > pmr_dhs_cpp_int_t;
        cpp_int int_var__pmr_dhs;
        void pmr_dhs (const cpp_int  & l__val);
        cpp_int pmr_dhs() const;
    
        typedef pu_cpp_int< 1 > prt_cor_cpp_int_t;
        cpp_int int_var__prt_cor;
        void prt_cor (const cpp_int  & l__val);
        cpp_int prt_cor() const;
    
        typedef pu_cpp_int< 1 > prt_det_cpp_int_t;
        cpp_int int_var__prt_det;
        void prt_det (const cpp_int  & l__val);
        cpp_int prt_det() const;
    
        typedef pu_cpp_int< 1 > prt_dhs_cpp_int_t;
        cpp_int int_var__prt_dhs;
        void prt_dhs (const cpp_int  & l__val);
        cpp_int prt_dhs() const;
    
        typedef pu_cpp_int< 1 > cplst_cor_cpp_int_t;
        cpp_int int_var__cplst_cor;
        void cplst_cor (const cpp_int  & l__val);
        cpp_int cplst_cor() const;
    
        typedef pu_cpp_int< 1 > cplst_det_cpp_int_t;
        cpp_int int_var__cplst_det;
        void cplst_det (const cpp_int  & l__val);
        cpp_int cplst_det() const;
    
        typedef pu_cpp_int< 1 > cplst_dhs_cpp_int_t;
        cpp_int int_var__cplst_dhs;
        void cplst_dhs (const cpp_int  & l__val);
        cpp_int cplst_dhs() const;
    
        typedef pu_cpp_int< 1 > aximst_cor_cpp_int_t;
        cpp_int int_var__aximst_cor;
        void aximst_cor (const cpp_int  & l__val);
        cpp_int aximst_cor() const;
    
        typedef pu_cpp_int< 1 > aximst_det_cpp_int_t;
        cpp_int int_var__aximst_det;
        void aximst_det (const cpp_int  & l__val);
        cpp_int aximst_det() const;
    
        typedef pu_cpp_int< 1 > aximst_dhs_cpp_int_t;
        cpp_int int_var__aximst_dhs;
        void aximst_dhs (const cpp_int  & l__val);
        cpp_int aximst_dhs() const;
    
        typedef pu_cpp_int< 1 > romask_cor_cpp_int_t;
        cpp_int int_var__romask_cor;
        void romask_cor (const cpp_int  & l__val);
        cpp_int romask_cor() const;
    
        typedef pu_cpp_int< 1 > romask_det_cpp_int_t;
        cpp_int int_var__romask_det;
        void romask_det (const cpp_int  & l__val);
        cpp_int romask_det() const;
    
        typedef pu_cpp_int< 1 > romask_dhs_cpp_int_t;
        cpp_int int_var__romask_dhs;
        void romask_dhs (const cpp_int  & l__val);
        cpp_int romask_dhs() const;
    
        typedef pu_cpp_int< 4 > rxcrbfr_cor_cpp_int_t;
        cpp_int int_var__rxcrbfr_cor;
        void rxcrbfr_cor (const cpp_int  & l__val);
        cpp_int rxcrbfr_cor() const;
    
        typedef pu_cpp_int< 4 > rxcrbfr_det_cpp_int_t;
        cpp_int int_var__rxcrbfr_det;
        void rxcrbfr_det (const cpp_int  & l__val);
        cpp_int rxcrbfr_det() const;
    
        typedef pu_cpp_int< 4 > rxcrbfr_dhs_cpp_int_t;
        cpp_int int_var__rxcrbfr_dhs;
        void rxcrbfr_dhs (const cpp_int  & l__val);
        cpp_int rxcrbfr_dhs() const;
    
}; // cap_pxb_csr_cfg_tgt_ecc_disable_t
    
class cap_pxb_csr_cfg_itr_ecc_disable_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_itr_ecc_disable_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_itr_ecc_disable_t(string name = "cap_pxb_csr_cfg_itr_ecc_disable_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_itr_ecc_disable_t();
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
    
        typedef pu_cpp_int< 1 > pcihdrt_cor_cpp_int_t;
        cpp_int int_var__pcihdrt_cor;
        void pcihdrt_cor (const cpp_int  & l__val);
        cpp_int pcihdrt_cor() const;
    
        typedef pu_cpp_int< 1 > pcihdrt_det_cpp_int_t;
        cpp_int int_var__pcihdrt_det;
        void pcihdrt_det (const cpp_int  & l__val);
        cpp_int pcihdrt_det() const;
    
        typedef pu_cpp_int< 1 > pcihdrt_dhs_cpp_int_t;
        cpp_int int_var__pcihdrt_dhs;
        void pcihdrt_dhs (const cpp_int  & l__val);
        cpp_int pcihdrt_dhs() const;
    
        typedef pu_cpp_int< 1 > portmap_cor_cpp_int_t;
        cpp_int int_var__portmap_cor;
        void portmap_cor (const cpp_int  & l__val);
        cpp_int portmap_cor() const;
    
        typedef pu_cpp_int< 1 > portmap_det_cpp_int_t;
        cpp_int int_var__portmap_det;
        void portmap_det (const cpp_int  & l__val);
        cpp_int portmap_det() const;
    
        typedef pu_cpp_int< 1 > portmap_dhs_cpp_int_t;
        cpp_int int_var__portmap_dhs;
        void portmap_dhs (const cpp_int  & l__val);
        cpp_int portmap_dhs() const;
    
        typedef pu_cpp_int< 1 > wrbfr_cor_cpp_int_t;
        cpp_int int_var__wrbfr_cor;
        void wrbfr_cor (const cpp_int  & l__val);
        cpp_int wrbfr_cor() const;
    
        typedef pu_cpp_int< 1 > wrbfr_det_cpp_int_t;
        cpp_int int_var__wrbfr_det;
        void wrbfr_det (const cpp_int  & l__val);
        cpp_int wrbfr_det() const;
    
        typedef pu_cpp_int< 1 > wrbfr_dhs_cpp_int_t;
        cpp_int int_var__wrbfr_dhs;
        void wrbfr_dhs (const cpp_int  & l__val);
        cpp_int wrbfr_dhs() const;
    
        typedef pu_cpp_int< 1 > wrhdr_cor_cpp_int_t;
        cpp_int int_var__wrhdr_cor;
        void wrhdr_cor (const cpp_int  & l__val);
        cpp_int wrhdr_cor() const;
    
        typedef pu_cpp_int< 1 > wrhdr_det_cpp_int_t;
        cpp_int int_var__wrhdr_det;
        void wrhdr_det (const cpp_int  & l__val);
        cpp_int wrhdr_det() const;
    
        typedef pu_cpp_int< 1 > wrhdr_dhs_cpp_int_t;
        cpp_int int_var__wrhdr_dhs;
        void wrhdr_dhs (const cpp_int  & l__val);
        cpp_int wrhdr_dhs() const;
    
        typedef pu_cpp_int< 1 > rdhdr_cor_cpp_int_t;
        cpp_int int_var__rdhdr_cor;
        void rdhdr_cor (const cpp_int  & l__val);
        cpp_int rdhdr_cor() const;
    
        typedef pu_cpp_int< 1 > rdhdr_det_cpp_int_t;
        cpp_int int_var__rdhdr_det;
        void rdhdr_det (const cpp_int  & l__val);
        cpp_int rdhdr_det() const;
    
        typedef pu_cpp_int< 1 > rdhdr_dhs_cpp_int_t;
        cpp_int int_var__rdhdr_dhs;
        void rdhdr_dhs (const cpp_int  & l__val);
        cpp_int rdhdr_dhs() const;
    
        typedef pu_cpp_int< 1 > cplbfr_cor_cpp_int_t;
        cpp_int int_var__cplbfr_cor;
        void cplbfr_cor (const cpp_int  & l__val);
        cpp_int cplbfr_cor() const;
    
        typedef pu_cpp_int< 1 > cplbfr_det_cpp_int_t;
        cpp_int int_var__cplbfr_det;
        void cplbfr_det (const cpp_int  & l__val);
        cpp_int cplbfr_det() const;
    
        typedef pu_cpp_int< 1 > cplbfr_dhs_cpp_int_t;
        cpp_int int_var__cplbfr_dhs;
        void cplbfr_dhs (const cpp_int  & l__val);
        cpp_int cplbfr_dhs() const;
    
}; // cap_pxb_csr_cfg_itr_ecc_disable_t
    
class cap_pxb_csr_sta_tgt_pmt_ind_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_sta_tgt_pmt_ind_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_sta_tgt_pmt_ind_t(string name = "cap_pxb_csr_sta_tgt_pmt_ind_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_sta_tgt_pmt_ind_t();
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
    
        typedef pu_cpp_int< 1 > hit_cpp_int_t;
        cpp_int int_var__hit;
        void hit (const cpp_int  & l__val);
        cpp_int hit() const;
    
        typedef pu_cpp_int< 10 > hit_addr_cpp_int_t;
        cpp_int int_var__hit_addr;
        void hit_addr (const cpp_int  & l__val);
        cpp_int hit_addr() const;
    
}; // cap_pxb_csr_sta_tgt_pmt_ind_t
    
class cap_pxb_csr_cfg_tgt_pmt_ind_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_tgt_pmt_ind_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_tgt_pmt_ind_t(string name = "cap_pxb_csr_cfg_tgt_pmt_ind_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_tgt_pmt_ind_t();
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
    
        typedef pu_cpp_int< 64 > key_cpp_int_t;
        cpp_int int_var__key;
        void key (const cpp_int  & l__val);
        cpp_int key() const;
    
        typedef pu_cpp_int< 64 > mask_cpp_int_t;
        cpp_int int_var__mask;
        void mask (const cpp_int  & l__val);
        cpp_int mask() const;
    
}; // cap_pxb_csr_cfg_tgt_pmt_ind_t
    
class cap_pxb_csr_cfg_tgt_pmt_grst_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_tgt_pmt_grst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_tgt_pmt_grst_t(string name = "cap_pxb_csr_cfg_tgt_pmt_grst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_tgt_pmt_grst_t();
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
    
        typedef pu_cpp_int< 1 > vld_cpp_int_t;
        cpp_int int_var__vld;
        void vld (const cpp_int  & l__val);
        cpp_int vld() const;
    
}; // cap_pxb_csr_cfg_tgt_pmt_grst_t
    
class cap_pxb_csr_cfg_tgt_rc_legacy_int_data_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_tgt_rc_legacy_int_data_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_tgt_rc_legacy_int_data_t(string name = "cap_pxb_csr_cfg_tgt_rc_legacy_int_data_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_tgt_rc_legacy_int_data_t();
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
    
}; // cap_pxb_csr_cfg_tgt_rc_legacy_int_data_t
    
class cap_pxb_csr_cfg_tgt_rc_legacy_int_addr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_tgt_rc_legacy_int_addr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_tgt_rc_legacy_int_addr_t(string name = "cap_pxb_csr_cfg_tgt_rc_legacy_int_addr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_tgt_rc_legacy_int_addr_t();
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
    
        typedef pu_cpp_int< 34 > base_35_2_cpp_int_t;
        cpp_int int_var__base_35_2;
        void base_35_2 (const cpp_int  & l__val);
        cpp_int base_35_2() const;
    
}; // cap_pxb_csr_cfg_tgt_rc_legacy_int_addr_t
    
class cap_pxb_csr_cfg_pcie_local_memaddr_decode_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_pcie_local_memaddr_decode_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_pcie_local_memaddr_decode_t(string name = "cap_pxb_csr_cfg_pcie_local_memaddr_decode_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_pcie_local_memaddr_decode_t();
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
    
        typedef pu_cpp_int< 3 > region0_cpp_int_t;
        cpp_int int_var__region0;
        void region0 (const cpp_int  & l__val);
        cpp_int region0() const;
    
        typedef pu_cpp_int< 3 > region1_cpp_int_t;
        cpp_int int_var__region1;
        void region1 (const cpp_int  & l__val);
        cpp_int region1() const;
    
        typedef pu_cpp_int< 3 > region2_cpp_int_t;
        cpp_int int_var__region2;
        void region2 (const cpp_int  & l__val);
        cpp_int region2() const;
    
        typedef pu_cpp_int< 3 > region3_cpp_int_t;
        cpp_int int_var__region3;
        void region3 (const cpp_int  & l__val);
        cpp_int region3() const;
    
        typedef pu_cpp_int< 3 > region4_cpp_int_t;
        cpp_int int_var__region4;
        void region4 (const cpp_int  & l__val);
        cpp_int region4() const;
    
        typedef pu_cpp_int< 3 > region5_cpp_int_t;
        cpp_int int_var__region5;
        void region5 (const cpp_int  & l__val);
        cpp_int region5() const;
    
        typedef pu_cpp_int< 3 > region6_cpp_int_t;
        cpp_int int_var__region6;
        void region6 (const cpp_int  & l__val);
        cpp_int region6() const;
    
        typedef pu_cpp_int< 3 > region7_cpp_int_t;
        cpp_int int_var__region7;
        void region7 (const cpp_int  & l__val);
        cpp_int region7() const;
    
        typedef pu_cpp_int< 3 > region8_cpp_int_t;
        cpp_int int_var__region8;
        void region8 (const cpp_int  & l__val);
        cpp_int region8() const;
    
        typedef pu_cpp_int< 3 > region9_cpp_int_t;
        cpp_int int_var__region9;
        void region9 (const cpp_int  & l__val);
        cpp_int region9() const;
    
        typedef pu_cpp_int< 3 > region10_cpp_int_t;
        cpp_int int_var__region10;
        void region10 (const cpp_int  & l__val);
        cpp_int region10() const;
    
        typedef pu_cpp_int< 3 > region11_cpp_int_t;
        cpp_int int_var__region11;
        void region11 (const cpp_int  & l__val);
        cpp_int region11() const;
    
        typedef pu_cpp_int< 3 > region12_cpp_int_t;
        cpp_int int_var__region12;
        void region12 (const cpp_int  & l__val);
        cpp_int region12() const;
    
        typedef pu_cpp_int< 3 > region13_cpp_int_t;
        cpp_int int_var__region13;
        void region13 (const cpp_int  & l__val);
        cpp_int region13() const;
    
        typedef pu_cpp_int< 3 > region14_cpp_int_t;
        cpp_int int_var__region14;
        void region14 (const cpp_int  & l__val);
        cpp_int region14() const;
    
        typedef pu_cpp_int< 3 > region15_cpp_int_t;
        cpp_int int_var__region15;
        void region15 (const cpp_int  & l__val);
        cpp_int region15() const;
    
        typedef pu_cpp_int< 4 > atomic_cpp_int_t;
        cpp_int int_var__atomic;
        void atomic (const cpp_int  & l__val);
        cpp_int atomic() const;
    
        typedef pu_cpp_int< 4 > rc_cfg_cpp_int_t;
        cpp_int int_var__rc_cfg;
        void rc_cfg (const cpp_int  & l__val);
        cpp_int rc_cfg() const;
    
        typedef pu_cpp_int< 19 > atomic_page_id_cpp_int_t;
        cpp_int int_var__atomic_page_id;
        void atomic_page_id (const cpp_int  & l__val);
        cpp_int atomic_page_id() const;
    
        typedef pu_cpp_int< 19 > rc_cfg_page_id_cpp_int_t;
        cpp_int int_var__rc_cfg_page_id;
        void rc_cfg_page_id (const cpp_int  & l__val);
        cpp_int rc_cfg_page_id() const;
    
        typedef pu_cpp_int< 19 > rc_io64k_cpp_int_t;
        cpp_int int_var__rc_io64k;
        void rc_io64k (const cpp_int  & l__val);
        cpp_int rc_io64k() const;
    
}; // cap_pxb_csr_cfg_pcie_local_memaddr_decode_t
    
class cap_pxb_csr_cfg_tgt_rc_key_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_tgt_rc_key_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_tgt_rc_key_t(string name = "cap_pxb_csr_cfg_tgt_rc_key_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_tgt_rc_key_t();
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
    
        typedef pu_cpp_int< 2 > sel_cpp_int_t;
        cpp_int int_var__sel;
        void sel (const cpp_int  & l__val);
        cpp_int sel() const;
    
}; // cap_pxb_csr_cfg_tgt_rc_key_t
    
class cap_pxb_csr_cfg_pmt_tbl_id_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_pmt_tbl_id_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_pmt_tbl_id_t(string name = "cap_pxb_csr_cfg_pmt_tbl_id_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_pmt_tbl_id_t();
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
    
        typedef pu_cpp_int< 2 > memio_tbl_id0_cpp_int_t;
        cpp_int int_var__memio_tbl_id0;
        void memio_tbl_id0 (const cpp_int  & l__val);
        cpp_int memio_tbl_id0() const;
    
        typedef pu_cpp_int< 2 > cfg_tbl_id0_cpp_int_t;
        cpp_int int_var__cfg_tbl_id0;
        void cfg_tbl_id0 (const cpp_int  & l__val);
        cpp_int cfg_tbl_id0() const;
    
        typedef pu_cpp_int< 2 > rc_tbl_id0_cpp_int_t;
        cpp_int int_var__rc_tbl_id0;
        void rc_tbl_id0 (const cpp_int  & l__val);
        cpp_int rc_tbl_id0() const;
    
        typedef pu_cpp_int< 2 > default_tbl_id0_cpp_int_t;
        cpp_int int_var__default_tbl_id0;
        void default_tbl_id0 (const cpp_int  & l__val);
        cpp_int default_tbl_id0() const;
    
        typedef pu_cpp_int< 2 > memio_tbl_id1_cpp_int_t;
        cpp_int int_var__memio_tbl_id1;
        void memio_tbl_id1 (const cpp_int  & l__val);
        cpp_int memio_tbl_id1() const;
    
        typedef pu_cpp_int< 2 > cfg_tbl_id1_cpp_int_t;
        cpp_int int_var__cfg_tbl_id1;
        void cfg_tbl_id1 (const cpp_int  & l__val);
        cpp_int cfg_tbl_id1() const;
    
        typedef pu_cpp_int< 2 > rc_tbl_id1_cpp_int_t;
        cpp_int int_var__rc_tbl_id1;
        void rc_tbl_id1 (const cpp_int  & l__val);
        cpp_int rc_tbl_id1() const;
    
        typedef pu_cpp_int< 2 > default_tbl_id1_cpp_int_t;
        cpp_int int_var__default_tbl_id1;
        void default_tbl_id1 (const cpp_int  & l__val);
        cpp_int default_tbl_id1() const;
    
        typedef pu_cpp_int< 2 > memio_tbl_id2_cpp_int_t;
        cpp_int int_var__memio_tbl_id2;
        void memio_tbl_id2 (const cpp_int  & l__val);
        cpp_int memio_tbl_id2() const;
    
        typedef pu_cpp_int< 2 > cfg_tbl_id2_cpp_int_t;
        cpp_int int_var__cfg_tbl_id2;
        void cfg_tbl_id2 (const cpp_int  & l__val);
        cpp_int cfg_tbl_id2() const;
    
        typedef pu_cpp_int< 2 > rc_tbl_id2_cpp_int_t;
        cpp_int int_var__rc_tbl_id2;
        void rc_tbl_id2 (const cpp_int  & l__val);
        cpp_int rc_tbl_id2() const;
    
        typedef pu_cpp_int< 2 > default_tbl_id2_cpp_int_t;
        cpp_int int_var__default_tbl_id2;
        void default_tbl_id2 (const cpp_int  & l__val);
        cpp_int default_tbl_id2() const;
    
        typedef pu_cpp_int< 2 > memio_tbl_id3_cpp_int_t;
        cpp_int int_var__memio_tbl_id3;
        void memio_tbl_id3 (const cpp_int  & l__val);
        cpp_int memio_tbl_id3() const;
    
        typedef pu_cpp_int< 2 > cfg_tbl_id3_cpp_int_t;
        cpp_int int_var__cfg_tbl_id3;
        void cfg_tbl_id3 (const cpp_int  & l__val);
        cpp_int cfg_tbl_id3() const;
    
        typedef pu_cpp_int< 2 > rc_tbl_id3_cpp_int_t;
        cpp_int int_var__rc_tbl_id3;
        void rc_tbl_id3 (const cpp_int  & l__val);
        cpp_int rc_tbl_id3() const;
    
        typedef pu_cpp_int< 2 > default_tbl_id3_cpp_int_t;
        cpp_int int_var__default_tbl_id3;
        void default_tbl_id3 (const cpp_int  & l__val);
        cpp_int default_tbl_id3() const;
    
        typedef pu_cpp_int< 2 > memio_tbl_id4_cpp_int_t;
        cpp_int int_var__memio_tbl_id4;
        void memio_tbl_id4 (const cpp_int  & l__val);
        cpp_int memio_tbl_id4() const;
    
        typedef pu_cpp_int< 2 > cfg_tbl_id4_cpp_int_t;
        cpp_int int_var__cfg_tbl_id4;
        void cfg_tbl_id4 (const cpp_int  & l__val);
        cpp_int cfg_tbl_id4() const;
    
        typedef pu_cpp_int< 2 > rc_tbl_id4_cpp_int_t;
        cpp_int int_var__rc_tbl_id4;
        void rc_tbl_id4 (const cpp_int  & l__val);
        cpp_int rc_tbl_id4() const;
    
        typedef pu_cpp_int< 2 > default_tbl_id4_cpp_int_t;
        cpp_int int_var__default_tbl_id4;
        void default_tbl_id4 (const cpp_int  & l__val);
        cpp_int default_tbl_id4() const;
    
        typedef pu_cpp_int< 2 > memio_tbl_id5_cpp_int_t;
        cpp_int int_var__memio_tbl_id5;
        void memio_tbl_id5 (const cpp_int  & l__val);
        cpp_int memio_tbl_id5() const;
    
        typedef pu_cpp_int< 2 > cfg_tbl_id5_cpp_int_t;
        cpp_int int_var__cfg_tbl_id5;
        void cfg_tbl_id5 (const cpp_int  & l__val);
        cpp_int cfg_tbl_id5() const;
    
        typedef pu_cpp_int< 2 > rc_tbl_id5_cpp_int_t;
        cpp_int int_var__rc_tbl_id5;
        void rc_tbl_id5 (const cpp_int  & l__val);
        cpp_int rc_tbl_id5() const;
    
        typedef pu_cpp_int< 2 > default_tbl_id5_cpp_int_t;
        cpp_int int_var__default_tbl_id5;
        void default_tbl_id5 (const cpp_int  & l__val);
        cpp_int default_tbl_id5() const;
    
        typedef pu_cpp_int< 2 > memio_tbl_id6_cpp_int_t;
        cpp_int int_var__memio_tbl_id6;
        void memio_tbl_id6 (const cpp_int  & l__val);
        cpp_int memio_tbl_id6() const;
    
        typedef pu_cpp_int< 2 > cfg_tbl_id6_cpp_int_t;
        cpp_int int_var__cfg_tbl_id6;
        void cfg_tbl_id6 (const cpp_int  & l__val);
        cpp_int cfg_tbl_id6() const;
    
        typedef pu_cpp_int< 2 > rc_tbl_id6_cpp_int_t;
        cpp_int int_var__rc_tbl_id6;
        void rc_tbl_id6 (const cpp_int  & l__val);
        cpp_int rc_tbl_id6() const;
    
        typedef pu_cpp_int< 2 > default_tbl_id6_cpp_int_t;
        cpp_int int_var__default_tbl_id6;
        void default_tbl_id6 (const cpp_int  & l__val);
        cpp_int default_tbl_id6() const;
    
        typedef pu_cpp_int< 2 > memio_tbl_id7_cpp_int_t;
        cpp_int int_var__memio_tbl_id7;
        void memio_tbl_id7 (const cpp_int  & l__val);
        cpp_int memio_tbl_id7() const;
    
        typedef pu_cpp_int< 2 > cfg_tbl_id7_cpp_int_t;
        cpp_int int_var__cfg_tbl_id7;
        void cfg_tbl_id7 (const cpp_int  & l__val);
        cpp_int cfg_tbl_id7() const;
    
        typedef pu_cpp_int< 2 > rc_tbl_id7_cpp_int_t;
        cpp_int int_var__rc_tbl_id7;
        void rc_tbl_id7 (const cpp_int  & l__val);
        cpp_int rc_tbl_id7() const;
    
        typedef pu_cpp_int< 2 > default_tbl_id7_cpp_int_t;
        cpp_int int_var__default_tbl_id7;
        void default_tbl_id7 (const cpp_int  & l__val);
        cpp_int default_tbl_id7() const;
    
}; // cap_pxb_csr_cfg_pmt_tbl_id_t
    
class cap_pxb_csr_cfg_tgt_notify_en_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_tgt_notify_en_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_tgt_notify_en_t(string name = "cap_pxb_csr_cfg_tgt_notify_en_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_tgt_notify_en_t();
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
    
        typedef pu_cpp_int< 1 > msg_cpp_int_t;
        cpp_int int_var__msg;
        void msg (const cpp_int  & l__val);
        cpp_int msg() const;
    
        typedef pu_cpp_int< 1 > pmv_cpp_int_t;
        cpp_int int_var__pmv;
        void pmv (const cpp_int  & l__val);
        cpp_int pmv() const;
    
        typedef pu_cpp_int< 1 > db_pmv_cpp_int_t;
        cpp_int int_var__db_pmv;
        void db_pmv (const cpp_int  & l__val);
        cpp_int db_pmv() const;
    
        typedef pu_cpp_int< 1 > unsupp_cpp_int_t;
        cpp_int int_var__unsupp;
        void unsupp (const cpp_int  & l__val);
        cpp_int unsupp() const;
    
        typedef pu_cpp_int< 1 > atomic_cpp_int_t;
        cpp_int int_var__atomic;
        void atomic (const cpp_int  & l__val);
        cpp_int atomic() const;
    
        typedef pu_cpp_int< 1 > pmt_miss_cpp_int_t;
        cpp_int int_var__pmt_miss;
        void pmt_miss (const cpp_int  & l__val);
        cpp_int pmt_miss() const;
    
        typedef pu_cpp_int< 1 > pmr_invalid_cpp_int_t;
        cpp_int int_var__pmr_invalid;
        void pmr_invalid (const cpp_int  & l__val);
        cpp_int pmr_invalid() const;
    
        typedef pu_cpp_int< 1 > prt_invalid_cpp_int_t;
        cpp_int int_var__prt_invalid;
        void prt_invalid (const cpp_int  & l__val);
        cpp_int prt_invalid() const;
    
        typedef pu_cpp_int< 1 > rc_vfid_miss_cpp_int_t;
        cpp_int int_var__rc_vfid_miss;
        void rc_vfid_miss (const cpp_int  & l__val);
        cpp_int rc_vfid_miss() const;
    
        typedef pu_cpp_int< 1 > prt_oor_cpp_int_t;
        cpp_int int_var__prt_oor;
        void prt_oor (const cpp_int  & l__val);
        cpp_int prt_oor() const;
    
        typedef pu_cpp_int< 1 > vfid_oor_cpp_int_t;
        cpp_int int_var__vfid_oor;
        void vfid_oor (const cpp_int  & l__val);
        cpp_int vfid_oor() const;
    
        typedef pu_cpp_int< 1 > cfg_bdf_oor_cpp_int_t;
        cpp_int int_var__cfg_bdf_oor;
        void cfg_bdf_oor (const cpp_int  & l__val);
        cpp_int cfg_bdf_oor() const;
    
        typedef pu_cpp_int< 1 > pmr_ecc_err_cpp_int_t;
        cpp_int int_var__pmr_ecc_err;
        void pmr_ecc_err (const cpp_int  & l__val);
        cpp_int pmr_ecc_err() const;
    
        typedef pu_cpp_int< 1 > prt_ecc_err_cpp_int_t;
        cpp_int int_var__prt_ecc_err;
        void prt_ecc_err (const cpp_int  & l__val);
        cpp_int prt_ecc_err() const;
    
}; // cap_pxb_csr_cfg_tgt_notify_en_t
    
class cap_pxb_csr_cfg_tgt_ind_en_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_tgt_ind_en_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_tgt_ind_en_t(string name = "cap_pxb_csr_cfg_tgt_ind_en_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_tgt_ind_en_t();
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
    
        typedef pu_cpp_int< 1 > msg_cpp_int_t;
        cpp_int int_var__msg;
        void msg (const cpp_int  & l__val);
        cpp_int msg() const;
    
        typedef pu_cpp_int< 1 > pmv_cpp_int_t;
        cpp_int int_var__pmv;
        void pmv (const cpp_int  & l__val);
        cpp_int pmv() const;
    
        typedef pu_cpp_int< 1 > db_pmv_cpp_int_t;
        cpp_int int_var__db_pmv;
        void db_pmv (const cpp_int  & l__val);
        cpp_int db_pmv() const;
    
        typedef pu_cpp_int< 1 > unsupp_cpp_int_t;
        cpp_int int_var__unsupp;
        void unsupp (const cpp_int  & l__val);
        cpp_int unsupp() const;
    
        typedef pu_cpp_int< 1 > atomic_cpp_int_t;
        cpp_int int_var__atomic;
        void atomic (const cpp_int  & l__val);
        cpp_int atomic() const;
    
        typedef pu_cpp_int< 1 > pmt_miss_cpp_int_t;
        cpp_int int_var__pmt_miss;
        void pmt_miss (const cpp_int  & l__val);
        cpp_int pmt_miss() const;
    
        typedef pu_cpp_int< 1 > pmr_invalid_cpp_int_t;
        cpp_int int_var__pmr_invalid;
        void pmr_invalid (const cpp_int  & l__val);
        cpp_int pmr_invalid() const;
    
        typedef pu_cpp_int< 1 > prt_invalid_cpp_int_t;
        cpp_int int_var__prt_invalid;
        void prt_invalid (const cpp_int  & l__val);
        cpp_int prt_invalid() const;
    
        typedef pu_cpp_int< 1 > rc_vfid_miss_cpp_int_t;
        cpp_int int_var__rc_vfid_miss;
        void rc_vfid_miss (const cpp_int  & l__val);
        cpp_int rc_vfid_miss() const;
    
        typedef pu_cpp_int< 1 > prt_oor_cpp_int_t;
        cpp_int int_var__prt_oor;
        void prt_oor (const cpp_int  & l__val);
        cpp_int prt_oor() const;
    
        typedef pu_cpp_int< 1 > vfid_oor_cpp_int_t;
        cpp_int int_var__vfid_oor;
        void vfid_oor (const cpp_int  & l__val);
        cpp_int vfid_oor() const;
    
        typedef pu_cpp_int< 1 > cfg_bdf_oor_cpp_int_t;
        cpp_int int_var__cfg_bdf_oor;
        void cfg_bdf_oor (const cpp_int  & l__val);
        cpp_int cfg_bdf_oor() const;
    
        typedef pu_cpp_int< 1 > pmr_ecc_err_cpp_int_t;
        cpp_int int_var__pmr_ecc_err;
        void pmr_ecc_err (const cpp_int  & l__val);
        cpp_int pmr_ecc_err() const;
    
        typedef pu_cpp_int< 1 > prt_ecc_err_cpp_int_t;
        cpp_int int_var__prt_ecc_err;
        void prt_ecc_err (const cpp_int  & l__val);
        cpp_int prt_ecc_err() const;
    
}; // cap_pxb_csr_cfg_tgt_ind_en_t
    
class cap_pxb_csr_cfg_tgt_rx_credit_bfr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_tgt_rx_credit_bfr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_tgt_rx_credit_bfr_t(string name = "cap_pxb_csr_cfg_tgt_rx_credit_bfr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_tgt_rx_credit_bfr_t();
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
    
        typedef pu_cpp_int< 10 > adr_base0_cpp_int_t;
        cpp_int int_var__adr_base0;
        void adr_base0 (const cpp_int  & l__val);
        cpp_int adr_base0() const;
    
        typedef pu_cpp_int< 10 > adr_limit0_cpp_int_t;
        cpp_int int_var__adr_limit0;
        void adr_limit0 (const cpp_int  & l__val);
        cpp_int adr_limit0() const;
    
        typedef pu_cpp_int< 1 > update0_cpp_int_t;
        cpp_int int_var__update0;
        void update0 (const cpp_int  & l__val);
        cpp_int update0() const;
    
        typedef pu_cpp_int< 1 > rst_rxfifo0_cpp_int_t;
        cpp_int int_var__rst_rxfifo0;
        void rst_rxfifo0 (const cpp_int  & l__val);
        cpp_int rst_rxfifo0() const;
    
        typedef pu_cpp_int< 10 > adr_base1_cpp_int_t;
        cpp_int int_var__adr_base1;
        void adr_base1 (const cpp_int  & l__val);
        cpp_int adr_base1() const;
    
        typedef pu_cpp_int< 10 > adr_limit1_cpp_int_t;
        cpp_int int_var__adr_limit1;
        void adr_limit1 (const cpp_int  & l__val);
        cpp_int adr_limit1() const;
    
        typedef pu_cpp_int< 1 > update1_cpp_int_t;
        cpp_int int_var__update1;
        void update1 (const cpp_int  & l__val);
        cpp_int update1() const;
    
        typedef pu_cpp_int< 1 > rst_rxfifo1_cpp_int_t;
        cpp_int int_var__rst_rxfifo1;
        void rst_rxfifo1 (const cpp_int  & l__val);
        cpp_int rst_rxfifo1() const;
    
        typedef pu_cpp_int< 10 > adr_base2_cpp_int_t;
        cpp_int int_var__adr_base2;
        void adr_base2 (const cpp_int  & l__val);
        cpp_int adr_base2() const;
    
        typedef pu_cpp_int< 10 > adr_limit2_cpp_int_t;
        cpp_int int_var__adr_limit2;
        void adr_limit2 (const cpp_int  & l__val);
        cpp_int adr_limit2() const;
    
        typedef pu_cpp_int< 1 > update2_cpp_int_t;
        cpp_int int_var__update2;
        void update2 (const cpp_int  & l__val);
        cpp_int update2() const;
    
        typedef pu_cpp_int< 1 > rst_rxfifo2_cpp_int_t;
        cpp_int int_var__rst_rxfifo2;
        void rst_rxfifo2 (const cpp_int  & l__val);
        cpp_int rst_rxfifo2() const;
    
        typedef pu_cpp_int< 10 > adr_base3_cpp_int_t;
        cpp_int int_var__adr_base3;
        void adr_base3 (const cpp_int  & l__val);
        cpp_int adr_base3() const;
    
        typedef pu_cpp_int< 10 > adr_limit3_cpp_int_t;
        cpp_int int_var__adr_limit3;
        void adr_limit3 (const cpp_int  & l__val);
        cpp_int adr_limit3() const;
    
        typedef pu_cpp_int< 1 > update3_cpp_int_t;
        cpp_int int_var__update3;
        void update3 (const cpp_int  & l__val);
        cpp_int update3() const;
    
        typedef pu_cpp_int< 1 > rst_rxfifo3_cpp_int_t;
        cpp_int int_var__rst_rxfifo3;
        void rst_rxfifo3 (const cpp_int  & l__val);
        cpp_int rst_rxfifo3() const;
    
        typedef pu_cpp_int< 10 > adr_base4_cpp_int_t;
        cpp_int int_var__adr_base4;
        void adr_base4 (const cpp_int  & l__val);
        cpp_int adr_base4() const;
    
        typedef pu_cpp_int< 10 > adr_limit4_cpp_int_t;
        cpp_int int_var__adr_limit4;
        void adr_limit4 (const cpp_int  & l__val);
        cpp_int adr_limit4() const;
    
        typedef pu_cpp_int< 1 > update4_cpp_int_t;
        cpp_int int_var__update4;
        void update4 (const cpp_int  & l__val);
        cpp_int update4() const;
    
        typedef pu_cpp_int< 1 > rst_rxfifo4_cpp_int_t;
        cpp_int int_var__rst_rxfifo4;
        void rst_rxfifo4 (const cpp_int  & l__val);
        cpp_int rst_rxfifo4() const;
    
        typedef pu_cpp_int< 10 > adr_base5_cpp_int_t;
        cpp_int int_var__adr_base5;
        void adr_base5 (const cpp_int  & l__val);
        cpp_int adr_base5() const;
    
        typedef pu_cpp_int< 10 > adr_limit5_cpp_int_t;
        cpp_int int_var__adr_limit5;
        void adr_limit5 (const cpp_int  & l__val);
        cpp_int adr_limit5() const;
    
        typedef pu_cpp_int< 1 > update5_cpp_int_t;
        cpp_int int_var__update5;
        void update5 (const cpp_int  & l__val);
        cpp_int update5() const;
    
        typedef pu_cpp_int< 1 > rst_rxfifo5_cpp_int_t;
        cpp_int int_var__rst_rxfifo5;
        void rst_rxfifo5 (const cpp_int  & l__val);
        cpp_int rst_rxfifo5() const;
    
        typedef pu_cpp_int< 10 > adr_base6_cpp_int_t;
        cpp_int int_var__adr_base6;
        void adr_base6 (const cpp_int  & l__val);
        cpp_int adr_base6() const;
    
        typedef pu_cpp_int< 10 > adr_limit6_cpp_int_t;
        cpp_int int_var__adr_limit6;
        void adr_limit6 (const cpp_int  & l__val);
        cpp_int adr_limit6() const;
    
        typedef pu_cpp_int< 1 > update6_cpp_int_t;
        cpp_int int_var__update6;
        void update6 (const cpp_int  & l__val);
        cpp_int update6() const;
    
        typedef pu_cpp_int< 1 > rst_rxfifo6_cpp_int_t;
        cpp_int int_var__rst_rxfifo6;
        void rst_rxfifo6 (const cpp_int  & l__val);
        cpp_int rst_rxfifo6() const;
    
        typedef pu_cpp_int< 10 > adr_base7_cpp_int_t;
        cpp_int int_var__adr_base7;
        void adr_base7 (const cpp_int  & l__val);
        cpp_int adr_base7() const;
    
        typedef pu_cpp_int< 10 > adr_limit7_cpp_int_t;
        cpp_int int_var__adr_limit7;
        void adr_limit7 (const cpp_int  & l__val);
        cpp_int adr_limit7() const;
    
        typedef pu_cpp_int< 1 > update7_cpp_int_t;
        cpp_int int_var__update7;
        void update7 (const cpp_int  & l__val);
        cpp_int update7() const;
    
        typedef pu_cpp_int< 1 > rst_rxfifo7_cpp_int_t;
        cpp_int int_var__rst_rxfifo7;
        void rst_rxfifo7 (const cpp_int  & l__val);
        cpp_int rst_rxfifo7() const;
    
}; // cap_pxb_csr_cfg_tgt_rx_credit_bfr_t
    
class cap_pxb_csr_cfg_tgt_fence_ignore_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_tgt_fence_ignore_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_tgt_fence_ignore_t(string name = "cap_pxb_csr_cfg_tgt_fence_ignore_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_tgt_fence_ignore_t();
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
    
        typedef pu_cpp_int< 8 > from_mask_cpp_int_t;
        cpp_int int_var__from_mask;
        void from_mask (const cpp_int  & l__val);
        cpp_int from_mask() const;
    
}; // cap_pxb_csr_cfg_tgt_fence_ignore_t
    
class cap_pxb_csr_cfg_tgt_msg_filter_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_tgt_msg_filter_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_tgt_msg_filter_t(string name = "cap_pxb_csr_cfg_tgt_msg_filter_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_tgt_msg_filter_t();
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
    
        typedef pu_cpp_int< 8 > msg_code0_cpp_int_t;
        cpp_int int_var__msg_code0;
        void msg_code0 (const cpp_int  & l__val);
        cpp_int msg_code0() const;
    
        typedef pu_cpp_int< 1 > drop_en0_cpp_int_t;
        cpp_int int_var__drop_en0;
        void drop_en0 (const cpp_int  & l__val);
        cpp_int drop_en0() const;
    
        typedef pu_cpp_int< 8 > msg_code1_cpp_int_t;
        cpp_int int_var__msg_code1;
        void msg_code1 (const cpp_int  & l__val);
        cpp_int msg_code1() const;
    
        typedef pu_cpp_int< 1 > drop_en1_cpp_int_t;
        cpp_int int_var__drop_en1;
        void drop_en1 (const cpp_int  & l__val);
        cpp_int drop_en1() const;
    
        typedef pu_cpp_int< 8 > msg_code2_cpp_int_t;
        cpp_int int_var__msg_code2;
        void msg_code2 (const cpp_int  & l__val);
        cpp_int msg_code2() const;
    
        typedef pu_cpp_int< 1 > drop_en2_cpp_int_t;
        cpp_int int_var__drop_en2;
        void drop_en2 (const cpp_int  & l__val);
        cpp_int drop_en2() const;
    
}; // cap_pxb_csr_cfg_tgt_msg_filter_t
    
class cap_pxb_csr_cfg_tgt_doorbell_base_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_tgt_doorbell_base_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_tgt_doorbell_base_t(string name = "cap_pxb_csr_cfg_tgt_doorbell_base_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_tgt_doorbell_base_t();
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
    
        typedef pu_cpp_int< 10 > addr_33_24_cpp_int_t;
        cpp_int int_var__addr_33_24;
        void addr_33_24 (const cpp_int  & l__val);
        cpp_int addr_33_24() const;
    
        typedef pu_cpp_int< 2 > db_host_sel_cpp_int_t;
        cpp_int int_var__db_host_sel;
        void db_host_sel (const cpp_int  & l__val);
        cpp_int db_host_sel() const;
    
        typedef pu_cpp_int< 2 > db_32b_sel_cpp_int_t;
        cpp_int int_var__db_32b_sel;
        void db_32b_sel (const cpp_int  & l__val);
        cpp_int db_32b_sel() const;
    
}; // cap_pxb_csr_cfg_tgt_doorbell_base_t
    
class cap_pxb_csr_cfg_tgt_req_indirect_int_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_tgt_req_indirect_int_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_tgt_req_indirect_int_t(string name = "cap_pxb_csr_cfg_tgt_req_indirect_int_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_tgt_req_indirect_int_t();
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
    
        typedef pu_cpp_int< 32 > intr_data_cpp_int_t;
        cpp_int int_var__intr_data;
        void intr_data (const cpp_int  & l__val);
        cpp_int intr_data() const;
    
        typedef pu_cpp_int< 34 > base_addr_35_2_cpp_int_t;
        cpp_int int_var__base_addr_35_2;
        void base_addr_35_2 (const cpp_int  & l__val);
        cpp_int base_addr_35_2() const;
    
}; // cap_pxb_csr_cfg_tgt_req_indirect_int_t
    
class cap_pxb_csr_cfg_tgt_req_notify_ring_size_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_tgt_req_notify_ring_size_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_tgt_req_notify_ring_size_t(string name = "cap_pxb_csr_cfg_tgt_req_notify_ring_size_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_tgt_req_notify_ring_size_t();
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
    
        typedef pu_cpp_int< 16 > mask_cpp_int_t;
        cpp_int int_var__mask;
        void mask (const cpp_int  & l__val);
        cpp_int mask() const;
    
}; // cap_pxb_csr_cfg_tgt_req_notify_ring_size_t
    
class cap_pxb_csr_cfg_tgt_req_notify_int_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_tgt_req_notify_int_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_tgt_req_notify_int_t(string name = "cap_pxb_csr_cfg_tgt_req_notify_int_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_tgt_req_notify_int_t();
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
    
        typedef pu_cpp_int< 32 > intr_data_cpp_int_t;
        cpp_int int_var__intr_data;
        void intr_data (const cpp_int  & l__val);
        cpp_int intr_data() const;
    
        typedef pu_cpp_int< 34 > base_addr_35_2_cpp_int_t;
        cpp_int int_var__base_addr_35_2;
        void base_addr_35_2 (const cpp_int  & l__val);
        cpp_int base_addr_35_2() const;
    
}; // cap_pxb_csr_cfg_tgt_req_notify_int_t
    
class cap_pxb_csr_cfg_tgt_req_notify_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_tgt_req_notify_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_tgt_req_notify_t(string name = "cap_pxb_csr_cfg_tgt_req_notify_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_tgt_req_notify_t();
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
    
        typedef pu_cpp_int< 20 > base_addr_35_16_cpp_int_t;
        cpp_int int_var__base_addr_35_16;
        void base_addr_35_16 (const cpp_int  & l__val);
        cpp_int base_addr_35_16() const;
    
}; // cap_pxb_csr_cfg_tgt_req_notify_t
    
class cap_pxb_csr_cfg_itr_tx_req_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_itr_tx_req_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_itr_tx_req_t(string name = "cap_pxb_csr_cfg_itr_tx_req_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_itr_tx_req_t();
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
    
        typedef pu_cpp_int< 5 > macfifo_thres_cpp_int_t;
        cpp_int int_var__macfifo_thres;
        void macfifo_thres (const cpp_int  & l__val);
        cpp_int macfifo_thres() const;
    
        typedef pu_cpp_int< 1 > rd_sgl_pnd_cpp_int_t;
        cpp_int int_var__rd_sgl_pnd;
        void rd_sgl_pnd (const cpp_int  & l__val);
        cpp_int rd_sgl_pnd() const;
    
        typedef pu_cpp_int< 3 > tag_avl_guardband_cpp_int_t;
        cpp_int int_var__tag_avl_guardband;
        void tag_avl_guardband (const cpp_int  & l__val);
        cpp_int tag_avl_guardband() const;
    
        typedef pu_cpp_int< 3 > cnxt_avl_guardband_cpp_int_t;
        cpp_int int_var__cnxt_avl_guardband;
        void cnxt_avl_guardband (const cpp_int  & l__val);
        cpp_int cnxt_avl_guardband() const;
    
}; // cap_pxb_csr_cfg_itr_tx_req_t
    
class cap_pxb_csr_cfg_tgt_port_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_tgt_port_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_tgt_port_t(string name = "cap_pxb_csr_cfg_tgt_port_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_tgt_port_t();
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
    
        typedef pu_cpp_int< 8 > id_limit_cpp_int_t;
        cpp_int int_var__id_limit;
        void id_limit (const cpp_int  & l__val);
        cpp_int id_limit() const;
    
        typedef pu_cpp_int< 1 > bus_adjust_cpp_int_t;
        cpp_int int_var__bus_adjust;
        void bus_adjust (const cpp_int  & l__val);
        cpp_int bus_adjust() const;
    
        typedef pu_cpp_int< 1 > halt_cpp_int_t;
        cpp_int int_var__halt;
        void halt (const cpp_int  & l__val);
        cpp_int halt() const;
    
        typedef pu_cpp_int< 1 > single_pnd_cpp_int_t;
        cpp_int int_var__single_pnd;
        void single_pnd (const cpp_int  & l__val);
        cpp_int single_pnd() const;
    
        typedef pu_cpp_int< 1 > td_miss_cpp_int_t;
        cpp_int int_var__td_miss;
        void td_miss (const cpp_int  & l__val);
        cpp_int td_miss() const;
    
        typedef pu_cpp_int< 4 > depth_thres_cpp_int_t;
        cpp_int int_var__depth_thres;
        void depth_thres (const cpp_int  & l__val);
        cpp_int depth_thres() const;
    
        typedef pu_cpp_int< 1 > skip_notify_if_qfull_cpp_int_t;
        cpp_int int_var__skip_notify_if_qfull;
        void skip_notify_if_qfull (const cpp_int  & l__val);
        cpp_int skip_notify_if_qfull() const;
    
        typedef pu_cpp_int< 1 > fbe_holes_allow_cpp_int_t;
        cpp_int int_var__fbe_holes_allow;
        void fbe_holes_allow (const cpp_int  & l__val);
        cpp_int fbe_holes_allow() const;
    
        typedef pu_cpp_int< 1 > fence_dis_cpp_int_t;
        cpp_int int_var__fence_dis;
        void fence_dis (const cpp_int  & l__val);
        cpp_int fence_dis() const;
    
}; // cap_pxb_csr_cfg_tgt_port_t
    
class cap_pxb_csr_cfg_itr_port_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_itr_port_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_itr_port_t(string name = "cap_pxb_csr_cfg_itr_port_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_itr_port_t();
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
    
        typedef pu_cpp_int< 8 > rdreq_limit_cpp_int_t;
        cpp_int int_var__rdreq_limit;
        void rdreq_limit (const cpp_int  & l__val);
        cpp_int rdreq_limit() const;
    
        typedef pu_cpp_int< 16 > timer_tick_cpp_int_t;
        cpp_int int_var__timer_tick;
        void timer_tick (const cpp_int  & l__val);
        cpp_int timer_tick() const;
    
        typedef pu_cpp_int< 10 > timer_limit_cpp_int_t;
        cpp_int int_var__timer_limit;
        void timer_limit (const cpp_int  & l__val);
        cpp_int timer_limit() const;
    
        typedef pu_cpp_int< 1 > bus_adjust_cpp_int_t;
        cpp_int int_var__bus_adjust;
        void bus_adjust (const cpp_int  & l__val);
        cpp_int bus_adjust() const;
    
        typedef pu_cpp_int< 5 > arb_cr_hdr_cpp_int_t;
        cpp_int int_var__arb_cr_hdr;
        void arb_cr_hdr (const cpp_int  & l__val);
        cpp_int arb_cr_hdr() const;
    
        typedef pu_cpp_int< 6 > arb_cr_data_cpp_int_t;
        cpp_int int_var__arb_cr_data;
        void arb_cr_data (const cpp_int  & l__val);
        cpp_int arb_cr_data() const;
    
        typedef pu_cpp_int< 1 > arb_cr_dis_wr_cpp_int_t;
        cpp_int int_var__arb_cr_dis_wr;
        void arb_cr_dis_wr (const cpp_int  & l__val);
        cpp_int arb_cr_dis_wr() const;
    
        typedef pu_cpp_int< 1 > arb_cr_dis_rd_cpp_int_t;
        cpp_int int_var__arb_cr_dis_rd;
        void arb_cr_dis_rd (const cpp_int  & l__val);
        cpp_int arb_cr_dis_rd() const;
    
        typedef pu_cpp_int< 1 > arb_cnxtcnt_chk_dis_cpp_int_t;
        cpp_int int_var__arb_cnxtcnt_chk_dis;
        void arb_cnxtcnt_chk_dis (const cpp_int  & l__val);
        cpp_int arb_cnxtcnt_chk_dis() const;
    
        typedef pu_cpp_int< 1 > arb_tagcnt_chk_dis_cpp_int_t;
        cpp_int int_var__arb_tagcnt_chk_dis;
        void arb_tagcnt_chk_dis (const cpp_int  & l__val);
        cpp_int arb_tagcnt_chk_dis() const;
    
        typedef pu_cpp_int< 5 > arb_txfifocnt_cpp_int_t;
        cpp_int int_var__arb_txfifocnt;
        void arb_txfifocnt (const cpp_int  & l__val);
        cpp_int arb_txfifocnt() const;
    
        typedef pu_cpp_int< 1 > adr_extn_en_cpp_int_t;
        cpp_int int_var__adr_extn_en;
        void adr_extn_en (const cpp_int  & l__val);
        cpp_int adr_extn_en() const;
    
}; // cap_pxb_csr_cfg_itr_port_t
    
class cap_pxb_csr_cfg_pc_port_type_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_cfg_pc_port_type_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_cfg_pc_port_type_t(string name = "cap_pxb_csr_cfg_pc_port_type_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_cfg_pc_port_type_t();
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
    
        typedef pu_cpp_int< 1 > host_or_rc0_cpp_int_t;
        cpp_int int_var__host_or_rc0;
        void host_or_rc0 (const cpp_int  & l__val);
        cpp_int host_or_rc0() const;
    
        typedef pu_cpp_int< 1 > host_or_rc1_cpp_int_t;
        cpp_int int_var__host_or_rc1;
        void host_or_rc1 (const cpp_int  & l__val);
        cpp_int host_or_rc1() const;
    
        typedef pu_cpp_int< 1 > host_or_rc2_cpp_int_t;
        cpp_int int_var__host_or_rc2;
        void host_or_rc2 (const cpp_int  & l__val);
        cpp_int host_or_rc2() const;
    
        typedef pu_cpp_int< 1 > host_or_rc3_cpp_int_t;
        cpp_int int_var__host_or_rc3;
        void host_or_rc3 (const cpp_int  & l__val);
        cpp_int host_or_rc3() const;
    
        typedef pu_cpp_int< 1 > host_or_rc4_cpp_int_t;
        cpp_int int_var__host_or_rc4;
        void host_or_rc4 (const cpp_int  & l__val);
        cpp_int host_or_rc4() const;
    
        typedef pu_cpp_int< 1 > host_or_rc5_cpp_int_t;
        cpp_int int_var__host_or_rc5;
        void host_or_rc5 (const cpp_int  & l__val);
        cpp_int host_or_rc5() const;
    
        typedef pu_cpp_int< 1 > host_or_rc6_cpp_int_t;
        cpp_int int_var__host_or_rc6;
        void host_or_rc6 (const cpp_int  & l__val);
        cpp_int host_or_rc6() const;
    
        typedef pu_cpp_int< 1 > host_or_rc7_cpp_int_t;
        cpp_int int_var__host_or_rc7;
        void host_or_rc7 (const cpp_int  & l__val);
        cpp_int host_or_rc7() const;
    
}; // cap_pxb_csr_cfg_pc_port_type_t
    
class cap_pxb_csr_rdintr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_rdintr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_rdintr_t(string name = "cap_pxb_csr_rdintr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_rdintr_t();
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
    
        typedef pu_cpp_int< 32 > ireg_cpp_int_t;
        cpp_int int_var__ireg;
        void ireg (const cpp_int  & l__val);
        cpp_int ireg() const;
    
}; // cap_pxb_csr_rdintr_t
    
class cap_pxb_csr_base_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_base_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_base_t(string name = "cap_pxb_csr_base_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_base_t();
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
    
}; // cap_pxb_csr_base_t
    
class cap_pxb_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pxb_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pxb_csr_t(string name = "cap_pxb_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pxb_csr_t();
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
    
        cap_pxb_csr_base_t base;
    
        cap_pxb_csr_rdintr_t rdintr;
    
        cap_pxb_csr_cfg_pc_port_type_t cfg_pc_port_type;
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_cfg_itr_port_t, 8> cfg_itr_port;
        #else 
        cap_pxb_csr_cfg_itr_port_t cfg_itr_port[8];
        #endif
        int get_depth_cfg_itr_port() { return 8; }
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_cfg_tgt_port_t, 8> cfg_tgt_port;
        #else 
        cap_pxb_csr_cfg_tgt_port_t cfg_tgt_port[8];
        #endif
        int get_depth_cfg_tgt_port() { return 8; }
    
        cap_pxb_csr_cfg_itr_tx_req_t cfg_itr_tx_req;
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_cfg_tgt_req_notify_t, 8> cfg_tgt_req_notify;
        #else 
        cap_pxb_csr_cfg_tgt_req_notify_t cfg_tgt_req_notify[8];
        #endif
        int get_depth_cfg_tgt_req_notify() { return 8; }
    
        cap_pxb_csr_cfg_tgt_req_notify_int_t cfg_tgt_req_notify_int;
    
        cap_pxb_csr_cfg_tgt_req_notify_ring_size_t cfg_tgt_req_notify_ring_size;
    
        cap_pxb_csr_cfg_tgt_req_indirect_int_t cfg_tgt_req_indirect_int;
    
        cap_pxb_csr_cfg_tgt_doorbell_base_t cfg_tgt_doorbell_base;
    
        cap_pxb_csr_cfg_tgt_msg_filter_t cfg_tgt_msg_filter;
    
        cap_pxb_csr_cfg_tgt_fence_ignore_t cfg_tgt_fence_ignore;
    
        cap_pxb_csr_cfg_tgt_rx_credit_bfr_t cfg_tgt_rx_credit_bfr;
    
        cap_pxb_csr_cfg_tgt_ind_en_t cfg_tgt_ind_en;
    
        cap_pxb_csr_cfg_tgt_notify_en_t cfg_tgt_notify_en;
    
        cap_pxb_csr_cfg_pmt_tbl_id_t cfg_pmt_tbl_id;
    
        cap_pxb_csr_cfg_tgt_rc_key_t cfg_tgt_rc_key;
    
        cap_pxb_csr_cfg_pcie_local_memaddr_decode_t cfg_pcie_local_memaddr_decode;
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_cfg_tgt_rc_legacy_int_addr_t, 8> cfg_tgt_rc_legacy_int_addr;
        #else 
        cap_pxb_csr_cfg_tgt_rc_legacy_int_addr_t cfg_tgt_rc_legacy_int_addr[8];
        #endif
        int get_depth_cfg_tgt_rc_legacy_int_addr() { return 8; }
    
        cap_pxb_csr_cfg_tgt_rc_legacy_int_data_t cfg_tgt_rc_legacy_int_data;
    
        cap_pxb_csr_cfg_tgt_pmt_grst_t cfg_tgt_pmt_grst;
    
        cap_pxb_csr_cfg_tgt_pmt_ind_t cfg_tgt_pmt_ind;
    
        cap_pxb_csr_sta_tgt_pmt_ind_t sta_tgt_pmt_ind;
    
        cap_pxb_csr_cfg_itr_ecc_disable_t cfg_itr_ecc_disable;
    
        cap_pxb_csr_cfg_tgt_ecc_disable_t cfg_tgt_ecc_disable;
    
        cap_pxb_csr_sta_itr_ecc_wrbfr_0_t sta_itr_ecc_wrbfr_0;
    
        cap_pxb_csr_sta_itr_ecc_wrbfr_1_t sta_itr_ecc_wrbfr_1;
    
        cap_pxb_csr_sta_itr_ecc_wrbfr_2_t sta_itr_ecc_wrbfr_2;
    
        cap_pxb_csr_sta_itr_ecc_wrbfr_3_t sta_itr_ecc_wrbfr_3;
    
        cap_pxb_csr_sta_itr_ecc_pcihdrt_t sta_itr_ecc_pcihdrt;
    
        cap_pxb_csr_sta_itr_ecc_portmap_t sta_itr_ecc_portmap;
    
        cap_pxb_csr_sta_itr_ecc_wrhdr_t sta_itr_ecc_wrhdr;
    
        cap_pxb_csr_sta_itr_ecc_rdhdr_t sta_itr_ecc_rdhdr;
    
        cap_pxb_csr_sta_itr_ecc_cplbfr_t sta_itr_ecc_cplbfr;
    
        cap_pxb_csr_sta_tgt_ecc_rxcrbfr_0_t sta_tgt_ecc_rxcrbfr_0;
    
        cap_pxb_csr_sta_tgt_ecc_rxcrbfr_1_t sta_tgt_ecc_rxcrbfr_1;
    
        cap_pxb_csr_sta_tgt_ecc_rxcrbfr_2_t sta_tgt_ecc_rxcrbfr_2;
    
        cap_pxb_csr_sta_tgt_ecc_rxcrbfr_3_t sta_tgt_ecc_rxcrbfr_3;
    
        cap_pxb_csr_sta_tgt_ecc_aximst_0_t sta_tgt_ecc_aximst_0;
    
        cap_pxb_csr_sta_tgt_ecc_aximst_1_t sta_tgt_ecc_aximst_1;
    
        cap_pxb_csr_sta_tgt_ecc_aximst_2_t sta_tgt_ecc_aximst_2;
    
        cap_pxb_csr_sta_tgt_ecc_aximst_3_t sta_tgt_ecc_aximst_3;
    
        cap_pxb_csr_sta_tgt_ecc_aximst_4_t sta_tgt_ecc_aximst_4;
    
        cap_pxb_csr_sta_tgt_ecc_rxinfo_t sta_tgt_ecc_rxinfo;
    
        cap_pxb_csr_sta_tgt_ecc_cplst_t sta_tgt_ecc_cplst;
    
        cap_pxb_csr_sta_tgt_ecc_pmr_t sta_tgt_ecc_pmr;
    
        cap_pxb_csr_sta_tgt_ecc_prt_t sta_tgt_ecc_prt;
    
        cap_pxb_csr_sta_tgt_ecc_romask_t sta_tgt_ecc_romask;
    
        cap_pxb_csr_sat_tgt_rx_drop_t sat_tgt_rx_drop;
    
        #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_cfg_tgt_vf_cfg_stride_t, 16> cfg_tgt_vf_cfg_stride;
        #else 
        cap_pxb_csr_cfg_tgt_vf_cfg_stride_t cfg_tgt_vf_cfg_stride[16];
        #endif
        int get_depth_cfg_tgt_vf_cfg_stride() { return 16; }
    
        cap_pxb_csr_sta_tgt_axi_pending_t sta_tgt_axi_pending;
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pxb_csr_sta_tgt_ind_info_t, 8> sta_tgt_ind_info;
        #else 
        cap_pxb_csr_sta_tgt_ind_info_t sta_tgt_ind_info[8];
        #endif
        int get_depth_sta_tgt_ind_info() { return 8; }
    
        cap_pxb_csr_cnt_itr_tot_axi_wr_t cnt_itr_tot_axi_wr;
    
        cap_pxb_csr_cnt_itr_axi_wr64_t cnt_itr_axi_wr64;
    
        cap_pxb_csr_cnt_itr_axi_wr256_t cnt_itr_axi_wr256;
    
        cap_pxb_csr_sta_itr_tot_axi_wr_t sta_itr_tot_axi_wr;
    
        cap_pxb_csr_sta_itr_tot_axi_rd_t sta_itr_tot_axi_rd;
    
        cap_pxb_csr_cnt_itr_tot_axi_rd_t cnt_itr_tot_axi_rd;
    
        cap_pxb_csr_cnt_itr_axi_rd64_t cnt_itr_axi_rd64;
    
        cap_pxb_csr_cnt_itr_axi_rd256_t cnt_itr_axi_rd256;
    
        cap_pxb_csr_cnt_tgt_tot_axi_wr_t cnt_tgt_tot_axi_wr;
    
        cap_pxb_csr_cnt_tgt_axi_wr64_t cnt_tgt_axi_wr64;
    
        cap_pxb_csr_cnt_tgt_db64_axi_wr_t cnt_tgt_db64_axi_wr;
    
        cap_pxb_csr_cnt_tgt_db32_axi_wr_t cnt_tgt_db32_axi_wr;
    
        cap_pxb_csr_cnt_tgt_rsp_ca_ur_t cnt_tgt_rsp_ca_ur;
    
        cap_pxb_csr_cnt_tgt_tot_axi_rd_t cnt_tgt_tot_axi_rd;
    
        cap_pxb_csr_cnt_tgt_axi_rd64_t cnt_tgt_axi_rd64;
    
        cap_pxb_csr_sta_tgt_tot_axi_wr_t sta_tgt_tot_axi_wr;
    
        cap_pxb_csr_sta_tgt_tot_axi_rd_t sta_tgt_tot_axi_rd;
    
        cap_pxb_csr_sta_itr_axi_wr_num_ids_t sta_itr_axi_wr_num_ids;
    
        cap_pxb_csr_sta_itr_axi_rd_num_ids_t sta_itr_axi_rd_num_ids;
    
        cap_pxb_csr_cnt_itr_tot_msg_t cnt_itr_tot_msg;
    
        cap_pxb_csr_cnt_itr_intx_assert_msg_t cnt_itr_intx_assert_msg;
    
        cap_pxb_csr_cnt_itr_tot_atomic_req_t cnt_itr_tot_atomic_req;
    
        cap_pxb_csr_sta_tgt_marker_rx_t sta_tgt_marker_rx;
    
        cap_pxb_csr_sta_tgt_req_debug_t sta_tgt_req_debug;
    
        cap_pxb_csr_sta_itr_req_debug_t sta_itr_req_debug;
    
        cap_pxb_csr_sta_tgt_rxcrbfr_debug_0_t sta_tgt_rxcrbfr_debug_0;
    
        cap_pxb_csr_sta_tgt_rxcrbfr_debug_1_t sta_tgt_rxcrbfr_debug_1;
    
        cap_pxb_csr_sta_tgt_rxcrbfr_debug_2_t sta_tgt_rxcrbfr_debug_2;
    
        cap_pxb_csr_sta_tgt_rxcrbfr_debug_3_t sta_tgt_rxcrbfr_debug_3;
    
        cap_pxb_csr_sta_tgt_rxcrbfr_debug_4_t sta_tgt_rxcrbfr_debug_4;
    
        cap_pxb_csr_sta_tgt_rxcrbfr_debug_5_t sta_tgt_rxcrbfr_debug_5;
    
        cap_pxb_csr_sta_tgt_rxcrbfr_debug_6_t sta_tgt_rxcrbfr_debug_6;
    
        cap_pxb_csr_sta_tgt_rxcrbfr_debug_7_t sta_tgt_rxcrbfr_debug_7;
    
        cap_pxb_csr_cfg_itr_raw_tlp_t cfg_itr_raw_tlp;
    
        cap_pxb_csr_cfg_itr_raw_tlp_cmd_t cfg_itr_raw_tlp_cmd;
    
        cap_pxb_csr_sta_itr_raw_tlp_data_t sta_itr_raw_tlp_data;
    
        cap_pxb_csr_sta_itr_raw_tlp_t sta_itr_raw_tlp;
    
        cap_pxb_csr_sta_itr_tags_pending_t sta_itr_tags_pending;
    
        cap_pxb_csr_sta_itr_portfifo_depth_t sta_itr_portfifo_depth;
    
        cap_pxb_csr_sta_itr_atomic_seq_cnt_t sta_itr_atomic_seq_cnt;
    
        cap_pxb_csr_sta_itr_cnxt_pending_t sta_itr_cnxt_pending;
    
        cap_pxb_csr_cfg_itr_atomic_seq_cnt_t cfg_itr_atomic_seq_cnt;
    
        cap_pxb_csr_cfg_itr_axi_resp_order_t cfg_itr_axi_resp_order;
    
        cap_pxb_csr_sat_tgt_ind_reason_t sat_tgt_ind_reason;
    
        cap_pxb_csr_sat_itr_req_err_t sat_itr_req_err;
    
        cap_pxb_csr_sat_itr_xfer_unexpected_t sat_itr_xfer_unexpected;
    
        cap_pxb_csr_sat_itr_cpl_err_t sat_itr_cpl_err;
    
        cap_pxb_csr_sat_itr_rsp_err_t sat_itr_rsp_err;
    
        cap_pxb_csr_sat_itr_req_portgate_t sat_itr_req_portgate;
    
        cap_pxb_csr_sat_tgt_rsp_err_t sat_tgt_rsp_err;
    
        cap_pxb_csr_cfg_tgt_axi_attr_t cfg_tgt_axi_attr;
    
        cap_pxb_csr_cfg_debug_port_t cfg_debug_port;
    
        cap_pxb_csr_cfg_diag_spare0_t cfg_diag_spare0;
    
        cap_pxb_csr_sta_diag_spare0_t sta_diag_spare0;
    
        cap_pxb_csr_cfg_diag_spare1_t cfg_diag_spare1;
    
        cap_pxb_csr_sta_diag_spare1_t sta_diag_spare1;
    
        cap_pxb_csr_cfg_itr_rdlat_measure_t cfg_itr_rdlat_measure;
    
        cap_pxb_csr_sat_itr_rdlat0_t sat_itr_rdlat0;
    
        cap_pxb_csr_sat_itr_rdlat1_t sat_itr_rdlat1;
    
        cap_pxb_csr_sat_itr_rdlat2_t sat_itr_rdlat2;
    
        cap_pxb_csr_sat_itr_rdlat3_t sat_itr_rdlat3;
    
        cap_pxb_csr_cfg_sram_bist_t cfg_sram_bist;
    
        cap_pxb_csr_sta_sram_bist_t sta_sram_bist;
    
        cap_pxb_csr_cfg_tcam_bist_t cfg_tcam_bist;
    
        cap_pxb_csr_sta_tcam_bist_t sta_tcam_bist;
    
        cap_pxb_csr_csr_intr_t csr_intr;
    
        cap_pxb_csr_cfg_pxb_spare0_t cfg_pxb_spare0;
    
        cap_pxb_csr_cfg_pxb_spare1_t cfg_pxb_spare1;
    
        cap_pxb_csr_cfg_pxb_spare2_t cfg_pxb_spare2;
    
        cap_pxb_csr_cfg_pxb_spare3_t cfg_pxb_spare3;
    
        cap_pxb_csr_cfg_filter_t cfg_filter;
    
        cap_pxb_csr_cfg_uid2sidLL_t cfg_uid2sidLL;
    
        cap_pxb_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t cfg_uid2sidLL_hbm_hash_msk_bit0;
    
        cap_pxb_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t cfg_uid2sidLL_hbm_hash_msk_bit1;
    
        cap_pxb_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t cfg_uid2sidLL_hbm_hash_msk_bit2;
    
        cap_pxb_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t cfg_uid2sidLL_hbm_hash_msk_bit3;
    
        cap_pxb_csr_cfg_axi_bw_mon_t cfg_axi_bw_mon;
    
        cap_pxb_csr_sta_axi_bw_mon_rd_latency_t sta_axi_bw_mon_rd_latency;
    
        cap_pxb_csr_sta_axi_bw_mon_rd_bandwidth_t sta_axi_bw_mon_rd_bandwidth;
    
        cap_pxb_csr_sta_axi_bw_mon_rd_transactions_t sta_axi_bw_mon_rd_transactions;
    
        cap_pxb_csr_cnt_axi_bw_mon_rd_t cnt_axi_bw_mon_rd;
    
        cap_pxb_csr_sta_axi_bw_mon_wr_latency_t sta_axi_bw_mon_wr_latency;
    
        cap_pxb_csr_sta_axi_bw_mon_wr_bandwidth_t sta_axi_bw_mon_wr_bandwidth;
    
        cap_pxb_csr_sta_axi_bw_mon_wr_transactions_t sta_axi_bw_mon_wr_transactions;
    
        cap_pxb_csr_cnt_axi_bw_mon_wr_t cnt_axi_bw_mon_wr;
    
        cap_pxb_csr_dhs_itr_pcihdrt_t dhs_itr_pcihdrt;
    
        cap_pxb_csr_dhs_itr_portmap_t dhs_itr_portmap;
    
        cap_pxb_csr_dhs_tgt_pmt_t dhs_tgt_pmt;
    
        cap_pxb_csr_dhs_tgt_pmr_t dhs_tgt_pmr;
    
        cap_pxb_csr_dhs_tgt_prt_t dhs_tgt_prt;
    
        cap_pxb_csr_dhs_tgt_rxcrbfr0_t dhs_tgt_rxcrbfr0;
    
        cap_pxb_csr_dhs_tgt_rxcrbfr1_t dhs_tgt_rxcrbfr1;
    
        cap_pxb_csr_dhs_tgt_rxcrbfr2_t dhs_tgt_rxcrbfr2;
    
        cap_pxb_csr_dhs_tgt_rxcrbfr3_t dhs_tgt_rxcrbfr3;
    
        cap_pxb_csr_dhs_tgt_rxinfo_t dhs_tgt_rxinfo;
    
        cap_pxb_csr_dhs_tgt_cplst_t dhs_tgt_cplst;
    
        cap_pxb_csr_dhs_tgt_romask_t dhs_tgt_romask;
    
        cap_pxb_csr_dhs_tgt_aximst0_t dhs_tgt_aximst0;
    
        cap_pxb_csr_dhs_tgt_aximst1_t dhs_tgt_aximst1;
    
        cap_pxb_csr_dhs_tgt_aximst2_t dhs_tgt_aximst2;
    
        cap_pxb_csr_dhs_tgt_aximst3_t dhs_tgt_aximst3;
    
        cap_pxb_csr_dhs_tgt_aximst4_t dhs_tgt_aximst4;
    
        cap_pxb_csr_dhs_itr_wrbfr0_t dhs_itr_wrbfr0;
    
        cap_pxb_csr_dhs_itr_wrbfr1_t dhs_itr_wrbfr1;
    
        cap_pxb_csr_dhs_itr_wrbfr2_t dhs_itr_wrbfr2;
    
        cap_pxb_csr_dhs_itr_wrbfr3_t dhs_itr_wrbfr3;
    
        cap_pxb_csr_dhs_itr_wrhdr_t dhs_itr_wrhdr;
    
        cap_pxb_csr_dhs_itr_rdhdr_t dhs_itr_rdhdr;
    
        cap_pxb_csr_dhs_itr_rdcontext_t dhs_itr_rdcontext;
    
        cap_pxb_csr_dhs_tgt_rdcontext_t dhs_tgt_rdcontext;
    
        cap_pxb_csr_dhs_tgt_rc_bdfmap_t dhs_tgt_rc_bdfmap;
    
        cap_pxb_csr_dhs_itr_cplbfr0_t dhs_itr_cplbfr0;
    
        cap_pxb_csr_dhs_itr_cplbfr1_t dhs_itr_cplbfr1;
    
        cap_pxb_csr_dhs_itr_cplbfr2_t dhs_itr_cplbfr2;
    
        cap_pxb_csr_dhs_itr_cplbfr3_t dhs_itr_cplbfr3;
    
        cap_pxb_csr_dhs_itr_cplbfr4_t dhs_itr_cplbfr4;
    
        cap_pxb_csr_dhs_itr_cplbfr5_t dhs_itr_cplbfr5;
    
        cap_pxb_csr_dhs_itr_cplbfr6_t dhs_itr_cplbfr6;
    
        cap_pxb_csr_dhs_itr_cplbfr7_t dhs_itr_cplbfr7;
    
        cap_pxb_csr_dhs_itr_cplbfr8_t dhs_itr_cplbfr8;
    
        cap_pxb_csr_dhs_itr_cplbfr9_t dhs_itr_cplbfr9;
    
        cap_pxb_csr_dhs_itr_cplbfr10_t dhs_itr_cplbfr10;
    
        cap_pxb_csr_dhs_itr_cplbfr11_t dhs_itr_cplbfr11;
    
        cap_pxb_csr_dhs_itr_cplbfr12_t dhs_itr_cplbfr12;
    
        cap_pxb_csr_dhs_itr_cplbfr13_t dhs_itr_cplbfr13;
    
        cap_pxb_csr_dhs_itr_cplbfr14_t dhs_itr_cplbfr14;
    
        cap_pxb_csr_dhs_itr_cplbfr15_t dhs_itr_cplbfr15;
    
        cap_pxb_csr_dhs_tgt_notify_t dhs_tgt_notify;
    
        cap_pxb_csr_dhs_tgt_ind_rsp_t dhs_tgt_ind_rsp;
    
        cap_pxb_csr_dhs_tgt_pmt_ind_t dhs_tgt_pmt_ind;
    
        cap_pxb_csr_filter_addr_lo_t filter_addr_lo;
    
        cap_pxb_csr_filter_addr_hi_t filter_addr_hi;
    
        cap_pxb_csr_filter_addr_ctl_t filter_addr_ctl;
    
        cap_pxb_csr_sta_inval_cam_t sta_inval_cam;
    
        cap_pxb_csr_intgrp_status_t int_groups;
    
        cap_pxb_csr_int_itr_ecc_t int_itr_ecc;
    
        cap_pxb_csr_int_tgt_ecc_t int_tgt_ecc;
    
        cap_pxb_csr_int_err_t int_err;
    
}; // cap_pxb_csr_t
    
#endif // CAP_PXB_CSR_H
        