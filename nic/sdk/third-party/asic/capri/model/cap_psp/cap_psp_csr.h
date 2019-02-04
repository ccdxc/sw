
#ifndef CAP_PSP_CSR_H
#define CAP_PSP_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_psp_csr_int_sw_phv_mem_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_int_sw_phv_mem_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_int_sw_phv_mem_int_enable_clear_t(string name = "cap_psp_csr_int_sw_phv_mem_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_int_sw_phv_mem_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > ecc_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__ecc_uncorrectable_enable;
        void ecc_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int ecc_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > ecc_correctable_enable_cpp_int_t;
        cpp_int int_var__ecc_correctable_enable;
        void ecc_correctable_enable (const cpp_int  & l__val);
        cpp_int ecc_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > phv_invalid_sram_enable_cpp_int_t;
        cpp_int int_var__phv_invalid_sram_enable;
        void phv_invalid_sram_enable (const cpp_int  & l__val);
        cpp_int phv_invalid_sram_enable() const;
    
        typedef pu_cpp_int< 1 > phv_invalid_data_enable_cpp_int_t;
        cpp_int int_var__phv_invalid_data_enable;
        void phv_invalid_data_enable (const cpp_int  & l__val);
        cpp_int phv_invalid_data_enable() const;
    
        typedef pu_cpp_int< 1 > phv_done0_enable_cpp_int_t;
        cpp_int int_var__phv_done0_enable;
        void phv_done0_enable (const cpp_int  & l__val);
        cpp_int phv_done0_enable() const;
    
        typedef pu_cpp_int< 1 > phv_done1_enable_cpp_int_t;
        cpp_int int_var__phv_done1_enable;
        void phv_done1_enable (const cpp_int  & l__val);
        cpp_int phv_done1_enable() const;
    
        typedef pu_cpp_int< 1 > phv_done2_enable_cpp_int_t;
        cpp_int int_var__phv_done2_enable;
        void phv_done2_enable (const cpp_int  & l__val);
        cpp_int phv_done2_enable() const;
    
        typedef pu_cpp_int< 1 > phv_done3_enable_cpp_int_t;
        cpp_int int_var__phv_done3_enable;
        void phv_done3_enable (const cpp_int  & l__val);
        cpp_int phv_done3_enable() const;
    
        typedef pu_cpp_int< 1 > phv_done4_enable_cpp_int_t;
        cpp_int int_var__phv_done4_enable;
        void phv_done4_enable (const cpp_int  & l__val);
        cpp_int phv_done4_enable() const;
    
        typedef pu_cpp_int< 1 > phv_done5_enable_cpp_int_t;
        cpp_int int_var__phv_done5_enable;
        void phv_done5_enable (const cpp_int  & l__val);
        cpp_int phv_done5_enable() const;
    
        typedef pu_cpp_int< 1 > phv_done6_enable_cpp_int_t;
        cpp_int int_var__phv_done6_enable;
        void phv_done6_enable (const cpp_int  & l__val);
        cpp_int phv_done6_enable() const;
    
        typedef pu_cpp_int< 1 > phv_done7_enable_cpp_int_t;
        cpp_int int_var__phv_done7_enable;
        void phv_done7_enable (const cpp_int  & l__val);
        cpp_int phv_done7_enable() const;
    
}; // cap_psp_csr_int_sw_phv_mem_int_enable_clear_t
    
class cap_psp_csr_int_sw_phv_mem_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_int_sw_phv_mem_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_int_sw_phv_mem_int_test_set_t(string name = "cap_psp_csr_int_sw_phv_mem_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_int_sw_phv_mem_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > ecc_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__ecc_uncorrectable_interrupt;
        void ecc_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int ecc_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > ecc_correctable_interrupt_cpp_int_t;
        cpp_int int_var__ecc_correctable_interrupt;
        void ecc_correctable_interrupt (const cpp_int  & l__val);
        cpp_int ecc_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > phv_invalid_sram_interrupt_cpp_int_t;
        cpp_int int_var__phv_invalid_sram_interrupt;
        void phv_invalid_sram_interrupt (const cpp_int  & l__val);
        cpp_int phv_invalid_sram_interrupt() const;
    
        typedef pu_cpp_int< 1 > phv_invalid_data_interrupt_cpp_int_t;
        cpp_int int_var__phv_invalid_data_interrupt;
        void phv_invalid_data_interrupt (const cpp_int  & l__val);
        cpp_int phv_invalid_data_interrupt() const;
    
        typedef pu_cpp_int< 1 > phv_done0_interrupt_cpp_int_t;
        cpp_int int_var__phv_done0_interrupt;
        void phv_done0_interrupt (const cpp_int  & l__val);
        cpp_int phv_done0_interrupt() const;
    
        typedef pu_cpp_int< 1 > phv_done1_interrupt_cpp_int_t;
        cpp_int int_var__phv_done1_interrupt;
        void phv_done1_interrupt (const cpp_int  & l__val);
        cpp_int phv_done1_interrupt() const;
    
        typedef pu_cpp_int< 1 > phv_done2_interrupt_cpp_int_t;
        cpp_int int_var__phv_done2_interrupt;
        void phv_done2_interrupt (const cpp_int  & l__val);
        cpp_int phv_done2_interrupt() const;
    
        typedef pu_cpp_int< 1 > phv_done3_interrupt_cpp_int_t;
        cpp_int int_var__phv_done3_interrupt;
        void phv_done3_interrupt (const cpp_int  & l__val);
        cpp_int phv_done3_interrupt() const;
    
        typedef pu_cpp_int< 1 > phv_done4_interrupt_cpp_int_t;
        cpp_int int_var__phv_done4_interrupt;
        void phv_done4_interrupt (const cpp_int  & l__val);
        cpp_int phv_done4_interrupt() const;
    
        typedef pu_cpp_int< 1 > phv_done5_interrupt_cpp_int_t;
        cpp_int int_var__phv_done5_interrupt;
        void phv_done5_interrupt (const cpp_int  & l__val);
        cpp_int phv_done5_interrupt() const;
    
        typedef pu_cpp_int< 1 > phv_done6_interrupt_cpp_int_t;
        cpp_int int_var__phv_done6_interrupt;
        void phv_done6_interrupt (const cpp_int  & l__val);
        cpp_int phv_done6_interrupt() const;
    
        typedef pu_cpp_int< 1 > phv_done7_interrupt_cpp_int_t;
        cpp_int int_var__phv_done7_interrupt;
        void phv_done7_interrupt (const cpp_int  & l__val);
        cpp_int phv_done7_interrupt() const;
    
}; // cap_psp_csr_int_sw_phv_mem_int_test_set_t
    
class cap_psp_csr_int_sw_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_int_sw_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_int_sw_phv_mem_t(string name = "cap_psp_csr_int_sw_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_int_sw_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_psp_csr_int_sw_phv_mem_int_test_set_t intreg;
    
        cap_psp_csr_int_sw_phv_mem_int_test_set_t int_test_set;
    
        cap_psp_csr_int_sw_phv_mem_int_enable_clear_t int_enable_set;
    
        cap_psp_csr_int_sw_phv_mem_int_enable_clear_t int_enable_clear;
    
}; // cap_psp_csr_int_sw_phv_mem_t
    
class cap_psp_csr_int_lif_qstate_map_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_int_lif_qstate_map_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_int_lif_qstate_map_int_enable_clear_t(string name = "cap_psp_csr_int_lif_qstate_map_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_int_lif_qstate_map_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > ecc_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__ecc_uncorrectable_enable;
        void ecc_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int ecc_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > ecc_correctable_enable_cpp_int_t;
        cpp_int int_var__ecc_correctable_enable;
        void ecc_correctable_enable (const cpp_int  & l__val);
        cpp_int ecc_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > qid_invalid_enable_cpp_int_t;
        cpp_int int_var__qid_invalid_enable;
        void qid_invalid_enable (const cpp_int  & l__val);
        cpp_int qid_invalid_enable() const;
    
}; // cap_psp_csr_int_lif_qstate_map_int_enable_clear_t
    
class cap_psp_csr_int_lif_qstate_map_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_int_lif_qstate_map_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_int_lif_qstate_map_int_test_set_t(string name = "cap_psp_csr_int_lif_qstate_map_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_int_lif_qstate_map_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > ecc_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__ecc_uncorrectable_interrupt;
        void ecc_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int ecc_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > ecc_correctable_interrupt_cpp_int_t;
        cpp_int int_var__ecc_correctable_interrupt;
        void ecc_correctable_interrupt (const cpp_int  & l__val);
        cpp_int ecc_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > qid_invalid_interrupt_cpp_int_t;
        cpp_int int_var__qid_invalid_interrupt;
        void qid_invalid_interrupt (const cpp_int  & l__val);
        cpp_int qid_invalid_interrupt() const;
    
}; // cap_psp_csr_int_lif_qstate_map_int_test_set_t
    
class cap_psp_csr_int_lif_qstate_map_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_int_lif_qstate_map_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_int_lif_qstate_map_t(string name = "cap_psp_csr_int_lif_qstate_map_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_int_lif_qstate_map_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_psp_csr_int_lif_qstate_map_int_test_set_t intreg;
    
        cap_psp_csr_int_lif_qstate_map_int_test_set_t int_test_set;
    
        cap_psp_csr_int_lif_qstate_map_int_enable_clear_t int_enable_set;
    
        cap_psp_csr_int_lif_qstate_map_int_enable_clear_t int_enable_clear;
    
}; // cap_psp_csr_int_lif_qstate_map_t
    
class cap_psp_csr_int_fatal_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_int_fatal_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_int_fatal_int_enable_clear_t(string name = "cap_psp_csr_int_fatal_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_int_fatal_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > pb_pbus_sop_err_enable_cpp_int_t;
        cpp_int int_var__pb_pbus_sop_err_enable;
        void pb_pbus_sop_err_enable (const cpp_int  & l__val);
        cpp_int pb_pbus_sop_err_enable() const;
    
        typedef pu_cpp_int< 1 > pb_pbus_eop_err_enable_cpp_int_t;
        cpp_int int_var__pb_pbus_eop_err_enable;
        void pb_pbus_eop_err_enable (const cpp_int  & l__val);
        cpp_int pb_pbus_eop_err_enable() const;
    
        typedef pu_cpp_int< 1 > ma_sop_err_enable_cpp_int_t;
        cpp_int int_var__ma_sop_err_enable;
        void ma_sop_err_enable (const cpp_int  & l__val);
        cpp_int ma_sop_err_enable() const;
    
        typedef pu_cpp_int< 1 > ma_eop_err_enable_cpp_int_t;
        cpp_int int_var__ma_eop_err_enable;
        void ma_eop_err_enable (const cpp_int  & l__val);
        cpp_int ma_eop_err_enable() const;
    
        typedef pu_cpp_int< 1 > pr_pbus_sop_err_enable_cpp_int_t;
        cpp_int int_var__pr_pbus_sop_err_enable;
        void pr_pbus_sop_err_enable (const cpp_int  & l__val);
        cpp_int pr_pbus_sop_err_enable() const;
    
        typedef pu_cpp_int< 1 > pr_pbus_eop_err_enable_cpp_int_t;
        cpp_int int_var__pr_pbus_eop_err_enable;
        void pr_pbus_eop_err_enable (const cpp_int  & l__val);
        cpp_int pr_pbus_eop_err_enable() const;
    
        typedef pu_cpp_int< 1 > pr_resub_pbus_sop_err_enable_cpp_int_t;
        cpp_int int_var__pr_resub_pbus_sop_err_enable;
        void pr_resub_pbus_sop_err_enable (const cpp_int  & l__val);
        cpp_int pr_resub_pbus_sop_err_enable() const;
    
        typedef pu_cpp_int< 1 > pr_resub_pbus_eop_err_enable_cpp_int_t;
        cpp_int int_var__pr_resub_pbus_eop_err_enable;
        void pr_resub_pbus_eop_err_enable (const cpp_int  & l__val);
        cpp_int pr_resub_pbus_eop_err_enable() const;
    
        typedef pu_cpp_int< 1 > pr_resub_sop_err_enable_cpp_int_t;
        cpp_int int_var__pr_resub_sop_err_enable;
        void pr_resub_sop_err_enable (const cpp_int  & l__val);
        cpp_int pr_resub_sop_err_enable() const;
    
        typedef pu_cpp_int< 1 > pr_resub_eop_err_enable_cpp_int_t;
        cpp_int int_var__pr_resub_eop_err_enable;
        void pr_resub_eop_err_enable (const cpp_int  & l__val);
        cpp_int pr_resub_eop_err_enable() const;
    
        typedef pu_cpp_int< 1 > pb_pbus_fsm0_no_data_err_enable_cpp_int_t;
        cpp_int int_var__pb_pbus_fsm0_no_data_err_enable;
        void pb_pbus_fsm0_no_data_err_enable (const cpp_int  & l__val);
        cpp_int pb_pbus_fsm0_no_data_err_enable() const;
    
        typedef pu_cpp_int< 1 > pb_pbus_fsm1_no_data_err_enable_cpp_int_t;
        cpp_int int_var__pb_pbus_fsm1_no_data_err_enable;
        void pb_pbus_fsm1_no_data_err_enable (const cpp_int  & l__val);
        cpp_int pb_pbus_fsm1_no_data_err_enable() const;
    
        typedef pu_cpp_int< 1 > pb_pbus_fsm2_no_data_err_enable_cpp_int_t;
        cpp_int int_var__pb_pbus_fsm2_no_data_err_enable;
        void pb_pbus_fsm2_no_data_err_enable (const cpp_int  & l__val);
        cpp_int pb_pbus_fsm2_no_data_err_enable() const;
    
        typedef pu_cpp_int< 1 > pb_pbus_fsm3_no_data_err_enable_cpp_int_t;
        cpp_int int_var__pb_pbus_fsm3_no_data_err_enable;
        void pb_pbus_fsm3_no_data_err_enable (const cpp_int  & l__val);
        cpp_int pb_pbus_fsm3_no_data_err_enable() const;
    
        typedef pu_cpp_int< 1 > pb_pbus_fsm4_no_data_err_enable_cpp_int_t;
        cpp_int int_var__pb_pbus_fsm4_no_data_err_enable;
        void pb_pbus_fsm4_no_data_err_enable (const cpp_int  & l__val);
        cpp_int pb_pbus_fsm4_no_data_err_enable() const;
    
        typedef pu_cpp_int< 1 > pb_pbus_fsm5_pkt_too_small_err_enable_cpp_int_t;
        cpp_int int_var__pb_pbus_fsm5_pkt_too_small_err_enable;
        void pb_pbus_fsm5_pkt_too_small_err_enable (const cpp_int  & l__val);
        cpp_int pb_pbus_fsm5_pkt_too_small_err_enable() const;
    
        typedef pu_cpp_int< 1 > lif_ind_table_rsp_err_enable_cpp_int_t;
        cpp_int int_var__lif_ind_table_rsp_err_enable;
        void lif_ind_table_rsp_err_enable (const cpp_int  & l__val);
        cpp_int lif_ind_table_rsp_err_enable() const;
    
        typedef pu_cpp_int< 1 > flit_cnt_oflow_err_enable_cpp_int_t;
        cpp_int int_var__flit_cnt_oflow_err_enable;
        void flit_cnt_oflow_err_enable (const cpp_int  & l__val);
        cpp_int flit_cnt_oflow_err_enable() const;
    
        typedef pu_cpp_int< 1 > lb_phv_sop_err_enable_cpp_int_t;
        cpp_int int_var__lb_phv_sop_err_enable;
        void lb_phv_sop_err_enable (const cpp_int  & l__val);
        cpp_int lb_phv_sop_err_enable() const;
    
        typedef pu_cpp_int< 1 > dc_phv_sop_err_enable_cpp_int_t;
        cpp_int int_var__dc_phv_sop_err_enable;
        void dc_phv_sop_err_enable (const cpp_int  & l__val);
        cpp_int dc_phv_sop_err_enable() const;
    
        typedef pu_cpp_int< 1 > lb_pkt_sop_err_enable_cpp_int_t;
        cpp_int int_var__lb_pkt_sop_err_enable;
        void lb_pkt_sop_err_enable (const cpp_int  & l__val);
        cpp_int lb_pkt_sop_err_enable() const;
    
        typedef pu_cpp_int< 1 > dc_pkt_sop_err_enable_cpp_int_t;
        cpp_int int_var__dc_pkt_sop_err_enable;
        void dc_pkt_sop_err_enable (const cpp_int  & l__val);
        cpp_int dc_pkt_sop_err_enable() const;
    
        typedef pu_cpp_int< 1 > sw_phv_jabber_err_enable_cpp_int_t;
        cpp_int int_var__sw_phv_jabber_err_enable;
        void sw_phv_jabber_err_enable (const cpp_int  & l__val);
        cpp_int sw_phv_jabber_err_enable() const;
    
}; // cap_psp_csr_int_fatal_int_enable_clear_t
    
class cap_psp_csr_int_fatal_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_int_fatal_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_int_fatal_int_test_set_t(string name = "cap_psp_csr_int_fatal_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_int_fatal_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > pb_pbus_sop_err_interrupt_cpp_int_t;
        cpp_int int_var__pb_pbus_sop_err_interrupt;
        void pb_pbus_sop_err_interrupt (const cpp_int  & l__val);
        cpp_int pb_pbus_sop_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > pb_pbus_eop_err_interrupt_cpp_int_t;
        cpp_int int_var__pb_pbus_eop_err_interrupt;
        void pb_pbus_eop_err_interrupt (const cpp_int  & l__val);
        cpp_int pb_pbus_eop_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > ma_sop_err_interrupt_cpp_int_t;
        cpp_int int_var__ma_sop_err_interrupt;
        void ma_sop_err_interrupt (const cpp_int  & l__val);
        cpp_int ma_sop_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > ma_eop_err_interrupt_cpp_int_t;
        cpp_int int_var__ma_eop_err_interrupt;
        void ma_eop_err_interrupt (const cpp_int  & l__val);
        cpp_int ma_eop_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > pr_pbus_sop_err_interrupt_cpp_int_t;
        cpp_int int_var__pr_pbus_sop_err_interrupt;
        void pr_pbus_sop_err_interrupt (const cpp_int  & l__val);
        cpp_int pr_pbus_sop_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > pr_pbus_eop_err_interrupt_cpp_int_t;
        cpp_int int_var__pr_pbus_eop_err_interrupt;
        void pr_pbus_eop_err_interrupt (const cpp_int  & l__val);
        cpp_int pr_pbus_eop_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > pr_resub_pbus_sop_err_interrupt_cpp_int_t;
        cpp_int int_var__pr_resub_pbus_sop_err_interrupt;
        void pr_resub_pbus_sop_err_interrupt (const cpp_int  & l__val);
        cpp_int pr_resub_pbus_sop_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > pr_resub_pbus_eop_err_interrupt_cpp_int_t;
        cpp_int int_var__pr_resub_pbus_eop_err_interrupt;
        void pr_resub_pbus_eop_err_interrupt (const cpp_int  & l__val);
        cpp_int pr_resub_pbus_eop_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > pr_resub_sop_err_interrupt_cpp_int_t;
        cpp_int int_var__pr_resub_sop_err_interrupt;
        void pr_resub_sop_err_interrupt (const cpp_int  & l__val);
        cpp_int pr_resub_sop_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > pr_resub_eop_err_interrupt_cpp_int_t;
        cpp_int int_var__pr_resub_eop_err_interrupt;
        void pr_resub_eop_err_interrupt (const cpp_int  & l__val);
        cpp_int pr_resub_eop_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > pb_pbus_fsm0_no_data_err_interrupt_cpp_int_t;
        cpp_int int_var__pb_pbus_fsm0_no_data_err_interrupt;
        void pb_pbus_fsm0_no_data_err_interrupt (const cpp_int  & l__val);
        cpp_int pb_pbus_fsm0_no_data_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > pb_pbus_fsm1_no_data_err_interrupt_cpp_int_t;
        cpp_int int_var__pb_pbus_fsm1_no_data_err_interrupt;
        void pb_pbus_fsm1_no_data_err_interrupt (const cpp_int  & l__val);
        cpp_int pb_pbus_fsm1_no_data_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > pb_pbus_fsm2_no_data_err_interrupt_cpp_int_t;
        cpp_int int_var__pb_pbus_fsm2_no_data_err_interrupt;
        void pb_pbus_fsm2_no_data_err_interrupt (const cpp_int  & l__val);
        cpp_int pb_pbus_fsm2_no_data_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > pb_pbus_fsm3_no_data_err_interrupt_cpp_int_t;
        cpp_int int_var__pb_pbus_fsm3_no_data_err_interrupt;
        void pb_pbus_fsm3_no_data_err_interrupt (const cpp_int  & l__val);
        cpp_int pb_pbus_fsm3_no_data_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > pb_pbus_fsm4_no_data_err_interrupt_cpp_int_t;
        cpp_int int_var__pb_pbus_fsm4_no_data_err_interrupt;
        void pb_pbus_fsm4_no_data_err_interrupt (const cpp_int  & l__val);
        cpp_int pb_pbus_fsm4_no_data_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > pb_pbus_fsm5_pkt_too_small_err_interrupt_cpp_int_t;
        cpp_int int_var__pb_pbus_fsm5_pkt_too_small_err_interrupt;
        void pb_pbus_fsm5_pkt_too_small_err_interrupt (const cpp_int  & l__val);
        cpp_int pb_pbus_fsm5_pkt_too_small_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > lif_ind_table_rsp_err_interrupt_cpp_int_t;
        cpp_int int_var__lif_ind_table_rsp_err_interrupt;
        void lif_ind_table_rsp_err_interrupt (const cpp_int  & l__val);
        cpp_int lif_ind_table_rsp_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > flit_cnt_oflow_err_interrupt_cpp_int_t;
        cpp_int int_var__flit_cnt_oflow_err_interrupt;
        void flit_cnt_oflow_err_interrupt (const cpp_int  & l__val);
        cpp_int flit_cnt_oflow_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > lb_phv_sop_err_interrupt_cpp_int_t;
        cpp_int int_var__lb_phv_sop_err_interrupt;
        void lb_phv_sop_err_interrupt (const cpp_int  & l__val);
        cpp_int lb_phv_sop_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > dc_phv_sop_err_interrupt_cpp_int_t;
        cpp_int int_var__dc_phv_sop_err_interrupt;
        void dc_phv_sop_err_interrupt (const cpp_int  & l__val);
        cpp_int dc_phv_sop_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > lb_pkt_sop_err_interrupt_cpp_int_t;
        cpp_int int_var__lb_pkt_sop_err_interrupt;
        void lb_pkt_sop_err_interrupt (const cpp_int  & l__val);
        cpp_int lb_pkt_sop_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > dc_pkt_sop_err_interrupt_cpp_int_t;
        cpp_int int_var__dc_pkt_sop_err_interrupt;
        void dc_pkt_sop_err_interrupt (const cpp_int  & l__val);
        cpp_int dc_pkt_sop_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > sw_phv_jabber_err_interrupt_cpp_int_t;
        cpp_int int_var__sw_phv_jabber_err_interrupt;
        void sw_phv_jabber_err_interrupt (const cpp_int  & l__val);
        cpp_int sw_phv_jabber_err_interrupt() const;
    
}; // cap_psp_csr_int_fatal_int_test_set_t
    
class cap_psp_csr_int_fatal_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_int_fatal_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_int_fatal_t(string name = "cap_psp_csr_int_fatal_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_int_fatal_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_psp_csr_int_fatal_int_test_set_t intreg;
    
        cap_psp_csr_int_fatal_int_test_set_t int_test_set;
    
        cap_psp_csr_int_fatal_int_enable_clear_t int_enable_set;
    
        cap_psp_csr_int_fatal_int_enable_clear_t int_enable_clear;
    
}; // cap_psp_csr_int_fatal_t
    
class cap_psp_csr_int_err_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_int_err_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_int_err_int_enable_clear_t(string name = "cap_psp_csr_int_err_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_int_err_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > pb_pbus_err_enable_cpp_int_t;
        cpp_int int_var__pb_pbus_err_enable;
        void pb_pbus_err_enable (const cpp_int  & l__val);
        cpp_int pb_pbus_err_enable() const;
    
        typedef pu_cpp_int< 1 > pr_pbus_err_enable_cpp_int_t;
        cpp_int int_var__pr_pbus_err_enable;
        void pr_pbus_err_enable (const cpp_int  & l__val);
        cpp_int pr_pbus_err_enable() const;
    
        typedef pu_cpp_int< 1 > pr_resub_pbus_err_enable_cpp_int_t;
        cpp_int int_var__pr_resub_pbus_err_enable;
        void pr_resub_pbus_err_enable (const cpp_int  & l__val);
        cpp_int pr_resub_pbus_err_enable() const;
    
        typedef pu_cpp_int< 1 > psp_csr_read_access_err_enable_cpp_int_t;
        cpp_int int_var__psp_csr_read_access_err_enable;
        void psp_csr_read_access_err_enable (const cpp_int  & l__val);
        cpp_int psp_csr_read_access_err_enable() const;
    
        typedef pu_cpp_int< 1 > psp_csr_write_access_err_enable_cpp_int_t;
        cpp_int int_var__psp_csr_write_access_err_enable;
        void psp_csr_write_access_err_enable (const cpp_int  & l__val);
        cpp_int psp_csr_write_access_err_enable() const;
    
        typedef pu_cpp_int< 1 > psp_phv_larger_than_twelve_flits_err_enable_cpp_int_t;
        cpp_int int_var__psp_phv_larger_than_twelve_flits_err_enable;
        void psp_phv_larger_than_twelve_flits_err_enable (const cpp_int  & l__val);
        cpp_int psp_phv_larger_than_twelve_flits_err_enable() const;
    
        typedef pu_cpp_int< 1 > psp_phv_sop_offset_err_enable_cpp_int_t;
        cpp_int int_var__psp_phv_sop_offset_err_enable;
        void psp_phv_sop_offset_err_enable (const cpp_int  & l__val);
        cpp_int psp_phv_sop_offset_err_enable() const;
    
        typedef pu_cpp_int< 1 > psp_phv_eop_offset_err_enable_cpp_int_t;
        cpp_int int_var__psp_phv_eop_offset_err_enable;
        void psp_phv_eop_offset_err_enable (const cpp_int  & l__val);
        cpp_int psp_phv_eop_offset_err_enable() const;
    
        typedef pu_cpp_int< 1 > max_recir_count_err_enable_cpp_int_t;
        cpp_int int_var__max_recir_count_err_enable;
        void max_recir_count_err_enable (const cpp_int  & l__val);
        cpp_int max_recir_count_err_enable() const;
    
        typedef pu_cpp_int< 1 > pb_pbus_dummy_err_enable_cpp_int_t;
        cpp_int int_var__pb_pbus_dummy_err_enable;
        void pb_pbus_dummy_err_enable (const cpp_int  & l__val);
        cpp_int pb_pbus_dummy_err_enable() const;
    
        typedef pu_cpp_int< 1 > pb_pbus_nopkt_err_enable_cpp_int_t;
        cpp_int int_var__pb_pbus_nopkt_err_enable;
        void pb_pbus_nopkt_err_enable (const cpp_int  & l__val);
        cpp_int pb_pbus_nopkt_err_enable() const;
    
        typedef pu_cpp_int< 1 > pb_pbus_min_err_enable_cpp_int_t;
        cpp_int int_var__pb_pbus_min_err_enable;
        void pb_pbus_min_err_enable (const cpp_int  & l__val);
        cpp_int pb_pbus_min_err_enable() const;
    
        typedef pu_cpp_int< 1 > lb_phv_almost_full_timeout_err_enable_cpp_int_t;
        cpp_int int_var__lb_phv_almost_full_timeout_err_enable;
        void lb_phv_almost_full_timeout_err_enable (const cpp_int  & l__val);
        cpp_int lb_phv_almost_full_timeout_err_enable() const;
    
        typedef pu_cpp_int< 1 > lb_pkt_almost_full_timeout_err_enable_cpp_int_t;
        cpp_int int_var__lb_pkt_almost_full_timeout_err_enable;
        void lb_pkt_almost_full_timeout_err_enable (const cpp_int  & l__val);
        cpp_int lb_pkt_almost_full_timeout_err_enable() const;
    
}; // cap_psp_csr_int_err_int_enable_clear_t
    
class cap_psp_csr_int_err_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_int_err_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_int_err_int_test_set_t(string name = "cap_psp_csr_int_err_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_int_err_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > pb_pbus_err_interrupt_cpp_int_t;
        cpp_int int_var__pb_pbus_err_interrupt;
        void pb_pbus_err_interrupt (const cpp_int  & l__val);
        cpp_int pb_pbus_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > pr_pbus_err_interrupt_cpp_int_t;
        cpp_int int_var__pr_pbus_err_interrupt;
        void pr_pbus_err_interrupt (const cpp_int  & l__val);
        cpp_int pr_pbus_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > pr_resub_pbus_err_interrupt_cpp_int_t;
        cpp_int int_var__pr_resub_pbus_err_interrupt;
        void pr_resub_pbus_err_interrupt (const cpp_int  & l__val);
        cpp_int pr_resub_pbus_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > psp_csr_read_access_err_interrupt_cpp_int_t;
        cpp_int int_var__psp_csr_read_access_err_interrupt;
        void psp_csr_read_access_err_interrupt (const cpp_int  & l__val);
        cpp_int psp_csr_read_access_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > psp_csr_write_access_err_interrupt_cpp_int_t;
        cpp_int int_var__psp_csr_write_access_err_interrupt;
        void psp_csr_write_access_err_interrupt (const cpp_int  & l__val);
        cpp_int psp_csr_write_access_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > psp_phv_larger_than_twelve_flits_err_interrupt_cpp_int_t;
        cpp_int int_var__psp_phv_larger_than_twelve_flits_err_interrupt;
        void psp_phv_larger_than_twelve_flits_err_interrupt (const cpp_int  & l__val);
        cpp_int psp_phv_larger_than_twelve_flits_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > psp_phv_sop_offset_err_interrupt_cpp_int_t;
        cpp_int int_var__psp_phv_sop_offset_err_interrupt;
        void psp_phv_sop_offset_err_interrupt (const cpp_int  & l__val);
        cpp_int psp_phv_sop_offset_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > psp_phv_eop_offset_err_interrupt_cpp_int_t;
        cpp_int int_var__psp_phv_eop_offset_err_interrupt;
        void psp_phv_eop_offset_err_interrupt (const cpp_int  & l__val);
        cpp_int psp_phv_eop_offset_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > max_recir_count_err_interrupt_cpp_int_t;
        cpp_int int_var__max_recir_count_err_interrupt;
        void max_recir_count_err_interrupt (const cpp_int  & l__val);
        cpp_int max_recir_count_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > pb_pbus_dummy_err_interrupt_cpp_int_t;
        cpp_int int_var__pb_pbus_dummy_err_interrupt;
        void pb_pbus_dummy_err_interrupt (const cpp_int  & l__val);
        cpp_int pb_pbus_dummy_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > pb_pbus_nopkt_err_interrupt_cpp_int_t;
        cpp_int int_var__pb_pbus_nopkt_err_interrupt;
        void pb_pbus_nopkt_err_interrupt (const cpp_int  & l__val);
        cpp_int pb_pbus_nopkt_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > pb_pbus_min_err_interrupt_cpp_int_t;
        cpp_int int_var__pb_pbus_min_err_interrupt;
        void pb_pbus_min_err_interrupt (const cpp_int  & l__val);
        cpp_int pb_pbus_min_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > lb_phv_almost_full_timeout_err_interrupt_cpp_int_t;
        cpp_int int_var__lb_phv_almost_full_timeout_err_interrupt;
        void lb_phv_almost_full_timeout_err_interrupt (const cpp_int  & l__val);
        cpp_int lb_phv_almost_full_timeout_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > lb_pkt_almost_full_timeout_err_interrupt_cpp_int_t;
        cpp_int int_var__lb_pkt_almost_full_timeout_err_interrupt;
        void lb_pkt_almost_full_timeout_err_interrupt (const cpp_int  & l__val);
        cpp_int lb_pkt_almost_full_timeout_err_interrupt() const;
    
}; // cap_psp_csr_int_err_int_test_set_t
    
class cap_psp_csr_int_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_int_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_int_err_t(string name = "cap_psp_csr_int_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_int_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_psp_csr_int_err_int_test_set_t intreg;
    
        cap_psp_csr_int_err_int_test_set_t int_test_set;
    
        cap_psp_csr_int_err_int_enable_clear_t int_enable_set;
    
        cap_psp_csr_int_err_int_enable_clear_t int_enable_clear;
    
}; // cap_psp_csr_int_err_t
    
class cap_psp_csr_int_info_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_int_info_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_int_info_int_enable_clear_t(string name = "cap_psp_csr_int_info_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_int_info_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > pr_pkt_ff_almost_full_enable_cpp_int_t;
        cpp_int int_var__pr_pkt_ff_almost_full_enable;
        void pr_pkt_ff_almost_full_enable (const cpp_int  & l__val);
        cpp_int pr_pkt_ff_almost_full_enable() const;
    
        typedef pu_cpp_int< 1 > ptd_npv_phv_full_enable_cpp_int_t;
        cpp_int int_var__ptd_npv_phv_full_enable;
        void ptd_npv_phv_full_enable (const cpp_int  & l__val);
        cpp_int ptd_npv_phv_full_enable() const;
    
        typedef pu_cpp_int< 1 > phv_srdy_collapsed_enable_cpp_int_t;
        cpp_int int_var__phv_srdy_collapsed_enable;
        void phv_srdy_collapsed_enable (const cpp_int  & l__val);
        cpp_int phv_srdy_collapsed_enable() const;
    
        typedef pu_cpp_int< 1 > pr_resub_pkt_before_phv_enable_cpp_int_t;
        cpp_int int_var__pr_resub_pkt_before_phv_enable;
        void pr_resub_pkt_before_phv_enable (const cpp_int  & l__val);
        cpp_int pr_resub_pkt_before_phv_enable() const;
    
        typedef pu_cpp_int< 1 > pb_pbus_fsm0_no_data_enable_cpp_int_t;
        cpp_int int_var__pb_pbus_fsm0_no_data_enable;
        void pb_pbus_fsm0_no_data_enable (const cpp_int  & l__val);
        cpp_int pb_pbus_fsm0_no_data_enable() const;
    
        typedef pu_cpp_int< 1 > pb_pbus_fsm1_no_data_enable_cpp_int_t;
        cpp_int int_var__pb_pbus_fsm1_no_data_enable;
        void pb_pbus_fsm1_no_data_enable (const cpp_int  & l__val);
        cpp_int pb_pbus_fsm1_no_data_enable() const;
    
        typedef pu_cpp_int< 1 > pb_pbus_fsm2_no_data_enable_cpp_int_t;
        cpp_int int_var__pb_pbus_fsm2_no_data_enable;
        void pb_pbus_fsm2_no_data_enable (const cpp_int  & l__val);
        cpp_int pb_pbus_fsm2_no_data_enable() const;
    
        typedef pu_cpp_int< 1 > pb_pbus_fsm3_no_data_enable_cpp_int_t;
        cpp_int int_var__pb_pbus_fsm3_no_data_enable;
        void pb_pbus_fsm3_no_data_enable (const cpp_int  & l__val);
        cpp_int pb_pbus_fsm3_no_data_enable() const;
    
        typedef pu_cpp_int< 1 > pb_pbus_fsm4_no_data_enable_cpp_int_t;
        cpp_int int_var__pb_pbus_fsm4_no_data_enable;
        void pb_pbus_fsm4_no_data_enable (const cpp_int  & l__val);
        cpp_int pb_pbus_fsm4_no_data_enable() const;
    
}; // cap_psp_csr_int_info_int_enable_clear_t
    
class cap_psp_csr_int_info_intreg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_int_info_intreg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_int_info_intreg_t(string name = "cap_psp_csr_int_info_intreg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_int_info_intreg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > pr_pkt_ff_almost_full_interrupt_cpp_int_t;
        cpp_int int_var__pr_pkt_ff_almost_full_interrupt;
        void pr_pkt_ff_almost_full_interrupt (const cpp_int  & l__val);
        cpp_int pr_pkt_ff_almost_full_interrupt() const;
    
        typedef pu_cpp_int< 1 > ptd_npv_phv_full_interrupt_cpp_int_t;
        cpp_int int_var__ptd_npv_phv_full_interrupt;
        void ptd_npv_phv_full_interrupt (const cpp_int  & l__val);
        cpp_int ptd_npv_phv_full_interrupt() const;
    
        typedef pu_cpp_int< 1 > phv_srdy_collapsed_interrupt_cpp_int_t;
        cpp_int int_var__phv_srdy_collapsed_interrupt;
        void phv_srdy_collapsed_interrupt (const cpp_int  & l__val);
        cpp_int phv_srdy_collapsed_interrupt() const;
    
        typedef pu_cpp_int< 1 > pr_resub_pkt_before_phv_interrupt_cpp_int_t;
        cpp_int int_var__pr_resub_pkt_before_phv_interrupt;
        void pr_resub_pkt_before_phv_interrupt (const cpp_int  & l__val);
        cpp_int pr_resub_pkt_before_phv_interrupt() const;
    
        typedef pu_cpp_int< 1 > pb_pbus_fsm0_no_data_interrupt_cpp_int_t;
        cpp_int int_var__pb_pbus_fsm0_no_data_interrupt;
        void pb_pbus_fsm0_no_data_interrupt (const cpp_int  & l__val);
        cpp_int pb_pbus_fsm0_no_data_interrupt() const;
    
        typedef pu_cpp_int< 1 > pb_pbus_fsm1_no_data_interrupt_cpp_int_t;
        cpp_int int_var__pb_pbus_fsm1_no_data_interrupt;
        void pb_pbus_fsm1_no_data_interrupt (const cpp_int  & l__val);
        cpp_int pb_pbus_fsm1_no_data_interrupt() const;
    
        typedef pu_cpp_int< 1 > pb_pbus_fsm2_no_data_interrupt_cpp_int_t;
        cpp_int int_var__pb_pbus_fsm2_no_data_interrupt;
        void pb_pbus_fsm2_no_data_interrupt (const cpp_int  & l__val);
        cpp_int pb_pbus_fsm2_no_data_interrupt() const;
    
        typedef pu_cpp_int< 1 > pb_pbus_fsm3_no_data_interrupt_cpp_int_t;
        cpp_int int_var__pb_pbus_fsm3_no_data_interrupt;
        void pb_pbus_fsm3_no_data_interrupt (const cpp_int  & l__val);
        cpp_int pb_pbus_fsm3_no_data_interrupt() const;
    
        typedef pu_cpp_int< 1 > pb_pbus_fsm4_no_data_interrupt_cpp_int_t;
        cpp_int int_var__pb_pbus_fsm4_no_data_interrupt;
        void pb_pbus_fsm4_no_data_interrupt (const cpp_int  & l__val);
        cpp_int pb_pbus_fsm4_no_data_interrupt() const;
    
}; // cap_psp_csr_int_info_intreg_t
    
class cap_psp_csr_int_info_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_int_info_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_int_info_t(string name = "cap_psp_csr_int_info_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_int_info_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_psp_csr_int_info_intreg_t intreg;
    
        cap_psp_csr_int_info_intreg_t int_test_set;
    
        cap_psp_csr_int_info_int_enable_clear_t int_enable_set;
    
        cap_psp_csr_int_info_int_enable_clear_t int_enable_clear;
    
}; // cap_psp_csr_int_info_t
    
class cap_psp_csr_intreg_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_intreg_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_intreg_status_t(string name = "cap_psp_csr_intreg_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_intreg_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > int_info_interrupt_cpp_int_t;
        cpp_int int_var__int_info_interrupt;
        void int_info_interrupt (const cpp_int  & l__val);
        cpp_int int_info_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_err_interrupt_cpp_int_t;
        cpp_int int_var__int_err_interrupt;
        void int_err_interrupt (const cpp_int  & l__val);
        cpp_int int_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_fatal_interrupt_cpp_int_t;
        cpp_int int_var__int_fatal_interrupt;
        void int_fatal_interrupt (const cpp_int  & l__val);
        cpp_int int_fatal_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_lif_qstate_map_interrupt_cpp_int_t;
        cpp_int int_var__int_lif_qstate_map_interrupt;
        void int_lif_qstate_map_interrupt (const cpp_int  & l__val);
        cpp_int int_lif_qstate_map_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_sw_phv_mem_interrupt_cpp_int_t;
        cpp_int int_var__int_sw_phv_mem_interrupt;
        void int_sw_phv_mem_interrupt (const cpp_int  & l__val);
        cpp_int int_sw_phv_mem_interrupt() const;
    
}; // cap_psp_csr_intreg_status_t
    
class cap_psp_csr_int_groups_int_enable_rw_reg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_int_groups_int_enable_rw_reg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_int_groups_int_enable_rw_reg_t(string name = "cap_psp_csr_int_groups_int_enable_rw_reg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_int_groups_int_enable_rw_reg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > int_info_enable_cpp_int_t;
        cpp_int int_var__int_info_enable;
        void int_info_enable (const cpp_int  & l__val);
        cpp_int int_info_enable() const;
    
        typedef pu_cpp_int< 1 > int_err_enable_cpp_int_t;
        cpp_int int_var__int_err_enable;
        void int_err_enable (const cpp_int  & l__val);
        cpp_int int_err_enable() const;
    
        typedef pu_cpp_int< 1 > int_fatal_enable_cpp_int_t;
        cpp_int int_var__int_fatal_enable;
        void int_fatal_enable (const cpp_int  & l__val);
        cpp_int int_fatal_enable() const;
    
        typedef pu_cpp_int< 1 > int_lif_qstate_map_enable_cpp_int_t;
        cpp_int int_var__int_lif_qstate_map_enable;
        void int_lif_qstate_map_enable (const cpp_int  & l__val);
        cpp_int int_lif_qstate_map_enable() const;
    
        typedef pu_cpp_int< 1 > int_sw_phv_mem_enable_cpp_int_t;
        cpp_int int_var__int_sw_phv_mem_enable;
        void int_sw_phv_mem_enable (const cpp_int  & l__val);
        cpp_int int_sw_phv_mem_enable() const;
    
}; // cap_psp_csr_int_groups_int_enable_rw_reg_t
    
class cap_psp_csr_intgrp_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_intgrp_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_intgrp_status_t(string name = "cap_psp_csr_intgrp_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_intgrp_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_psp_csr_intreg_status_t intreg;
    
        cap_psp_csr_int_groups_int_enable_rw_reg_t int_enable_rw_reg;
    
        cap_psp_csr_intreg_status_t int_rw_reg;
    
}; // cap_psp_csr_intgrp_status_t
    
class cap_psp_csr_dhs_debug_trace_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_dhs_debug_trace_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_dhs_debug_trace_entry_t(string name = "cap_psp_csr_dhs_debug_trace_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_dhs_debug_trace_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > armed_cpp_int_t;
        cpp_int int_var__armed;
        void armed (const cpp_int  & l__val);
        cpp_int armed() const;
    
}; // cap_psp_csr_dhs_debug_trace_entry_t
    
class cap_psp_csr_dhs_debug_trace_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_dhs_debug_trace_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_dhs_debug_trace_t(string name = "cap_psp_csr_dhs_debug_trace_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_dhs_debug_trace_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_psp_csr_dhs_debug_trace_entry_t entry;
    
}; // cap_psp_csr_dhs_debug_trace_t
    
class cap_psp_csr_dhs_sw_phv_mem_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_dhs_sw_phv_mem_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_dhs_sw_phv_mem_entry_t(string name = "cap_psp_csr_dhs_sw_phv_mem_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_dhs_sw_phv_mem_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 512 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 3 > spare_cpp_int_t;
        cpp_int int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
        typedef pu_cpp_int< 20 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_psp_csr_dhs_sw_phv_mem_entry_t
    
class cap_psp_csr_dhs_sw_phv_mem_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_dhs_sw_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_dhs_sw_phv_mem_t(string name = "cap_psp_csr_dhs_sw_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_dhs_sw_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_psp_csr_dhs_sw_phv_mem_entry_t, 16> entry;
        #else 
        cap_psp_csr_dhs_sw_phv_mem_entry_t entry[16];
        #endif
        int get_depth_entry() { return 16; }
    
}; // cap_psp_csr_dhs_sw_phv_mem_t
    
class cap_psp_csr_dhs_lif_qstate_map_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_dhs_lif_qstate_map_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_dhs_lif_qstate_map_entry_t(string name = "cap_psp_csr_dhs_lif_qstate_map_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_dhs_lif_qstate_map_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 22 > qstate_base_cpp_int_t;
        cpp_int int_var__qstate_base;
        void qstate_base (const cpp_int  & l__val);
        cpp_int qstate_base() const;
    
        typedef pu_cpp_int< 5 > length0_cpp_int_t;
        cpp_int int_var__length0;
        void length0 (const cpp_int  & l__val);
        cpp_int length0() const;
    
        typedef pu_cpp_int< 3 > size0_cpp_int_t;
        cpp_int int_var__size0;
        void size0 (const cpp_int  & l__val);
        cpp_int size0() const;
    
        typedef pu_cpp_int< 5 > length1_cpp_int_t;
        cpp_int int_var__length1;
        void length1 (const cpp_int  & l__val);
        cpp_int length1() const;
    
        typedef pu_cpp_int< 3 > size1_cpp_int_t;
        cpp_int int_var__size1;
        void size1 (const cpp_int  & l__val);
        cpp_int size1() const;
    
        typedef pu_cpp_int< 5 > length2_cpp_int_t;
        cpp_int int_var__length2;
        void length2 (const cpp_int  & l__val);
        cpp_int length2() const;
    
        typedef pu_cpp_int< 3 > size2_cpp_int_t;
        cpp_int int_var__size2;
        void size2 (const cpp_int  & l__val);
        cpp_int size2() const;
    
        typedef pu_cpp_int< 5 > length3_cpp_int_t;
        cpp_int int_var__length3;
        void length3 (const cpp_int  & l__val);
        cpp_int length3() const;
    
        typedef pu_cpp_int< 3 > size3_cpp_int_t;
        cpp_int int_var__size3;
        void size3 (const cpp_int  & l__val);
        cpp_int size3() const;
    
        typedef pu_cpp_int< 5 > length4_cpp_int_t;
        cpp_int int_var__length4;
        void length4 (const cpp_int  & l__val);
        cpp_int length4() const;
    
        typedef pu_cpp_int< 3 > size4_cpp_int_t;
        cpp_int int_var__size4;
        void size4 (const cpp_int  & l__val);
        cpp_int size4() const;
    
        typedef pu_cpp_int< 5 > length5_cpp_int_t;
        cpp_int int_var__length5;
        void length5 (const cpp_int  & l__val);
        cpp_int length5() const;
    
        typedef pu_cpp_int< 3 > size5_cpp_int_t;
        cpp_int int_var__size5;
        void size5 (const cpp_int  & l__val);
        cpp_int size5() const;
    
        typedef pu_cpp_int< 5 > length6_cpp_int_t;
        cpp_int int_var__length6;
        void length6 (const cpp_int  & l__val);
        cpp_int length6() const;
    
        typedef pu_cpp_int< 3 > size6_cpp_int_t;
        cpp_int int_var__size6;
        void size6 (const cpp_int  & l__val);
        cpp_int size6() const;
    
        typedef pu_cpp_int< 5 > length7_cpp_int_t;
        cpp_int int_var__length7;
        void length7 (const cpp_int  & l__val);
        cpp_int length7() const;
    
        typedef pu_cpp_int< 3 > size7_cpp_int_t;
        cpp_int int_var__size7;
        void size7 (const cpp_int  & l__val);
        cpp_int size7() const;
    
        typedef pu_cpp_int< 1 > sched_hint_en_cpp_int_t;
        cpp_int int_var__sched_hint_en;
        void sched_hint_en (const cpp_int  & l__val);
        cpp_int sched_hint_en() const;
    
        typedef pu_cpp_int< 4 > sched_hint_cos_cpp_int_t;
        cpp_int int_var__sched_hint_cos;
        void sched_hint_cos (const cpp_int  & l__val);
        cpp_int sched_hint_cos() const;
    
        typedef pu_cpp_int< 4 > spare_cpp_int_t;
        cpp_int int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
        typedef pu_cpp_int< 8 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_psp_csr_dhs_lif_qstate_map_entry_t
    
class cap_psp_csr_dhs_lif_qstate_map_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_dhs_lif_qstate_map_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_dhs_lif_qstate_map_t(string name = "cap_psp_csr_dhs_lif_qstate_map_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_dhs_lif_qstate_map_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
        cap_csr_large_array_wrapper<cap_psp_csr_dhs_lif_qstate_map_entry_t, 2048> entry;
        #else 
        cap_psp_csr_dhs_lif_qstate_map_entry_t entry[2048];
        #endif
        int get_depth_entry() { return 2048; }
    
}; // cap_psp_csr_dhs_lif_qstate_map_t
    
class cap_psp_csr_dhs_psp_pkt_mem_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_dhs_psp_pkt_mem_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_dhs_psp_pkt_mem_entry_t(string name = "cap_psp_csr_dhs_psp_pkt_mem_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_dhs_psp_pkt_mem_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 518 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 1 > sop_cpp_int_t;
        cpp_int int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 1 > eop_cpp_int_t;
        cpp_int int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
        typedef pu_cpp_int< 1 > err_cpp_int_t;
        cpp_int int_var__err;
        void err (const cpp_int  & l__val);
        cpp_int err() const;
    
}; // cap_psp_csr_dhs_psp_pkt_mem_entry_t
    
class cap_psp_csr_dhs_psp_pkt_mem_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_dhs_psp_pkt_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_dhs_psp_pkt_mem_t(string name = "cap_psp_csr_dhs_psp_pkt_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_dhs_psp_pkt_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_psp_csr_dhs_psp_pkt_mem_entry_t, 16> entry;
        #else 
        cap_psp_csr_dhs_psp_pkt_mem_entry_t entry[16];
        #endif
        int get_depth_entry() { return 16; }
    
}; // cap_psp_csr_dhs_psp_pkt_mem_t
    
class cap_psp_csr_dhs_psp_out_mem_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_dhs_psp_out_mem_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_dhs_psp_out_mem_entry_t(string name = "cap_psp_csr_dhs_psp_out_mem_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_dhs_psp_out_mem_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 512 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 1 > sop_cpp_int_t;
        cpp_int int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 1 > eop_cpp_int_t;
        cpp_int int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
        typedef pu_cpp_int< 1 > err_cpp_int_t;
        cpp_int int_var__err;
        void err (const cpp_int  & l__val);
        cpp_int err() const;
    
}; // cap_psp_csr_dhs_psp_out_mem_entry_t
    
class cap_psp_csr_dhs_psp_out_mem_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_dhs_psp_out_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_dhs_psp_out_mem_t(string name = "cap_psp_csr_dhs_psp_out_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_dhs_psp_out_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_psp_csr_dhs_psp_out_mem_entry_t, 16> entry;
        #else 
        cap_psp_csr_dhs_psp_out_mem_entry_t entry[16];
        #endif
        int get_depth_entry() { return 16; }
    
}; // cap_psp_csr_dhs_psp_out_mem_t
    
class cap_psp_csr_dhs_psp_phv_mem_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_dhs_psp_phv_mem_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_dhs_psp_phv_mem_entry_t(string name = "cap_psp_csr_dhs_psp_phv_mem_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_dhs_psp_phv_mem_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 512 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 1 > sop_cpp_int_t;
        cpp_int int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 1 > eop_cpp_int_t;
        cpp_int int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
        typedef pu_cpp_int< 1 > err_cpp_int_t;
        cpp_int int_var__err;
        void err (const cpp_int  & l__val);
        cpp_int err() const;
    
}; // cap_psp_csr_dhs_psp_phv_mem_entry_t
    
class cap_psp_csr_dhs_psp_phv_mem_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_dhs_psp_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_dhs_psp_phv_mem_t(string name = "cap_psp_csr_dhs_psp_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_dhs_psp_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        #if 48 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_psp_csr_dhs_psp_phv_mem_entry_t, 48> entry;
        #else 
        cap_psp_csr_dhs_psp_phv_mem_entry_t entry[48];
        #endif
        int get_depth_entry() { return 48; }
    
}; // cap_psp_csr_dhs_psp_phv_mem_t
    
class cap_psp_csr_cnt_pb_pbus_min_phv_drop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_pb_pbus_min_phv_drop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_pb_pbus_min_phv_drop_t(string name = "cap_psp_csr_cnt_pb_pbus_min_phv_drop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_pb_pbus_min_phv_drop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_pb_pbus_min_phv_drop_t
    
class cap_psp_csr_cnt_pb_pbus_no_data_hw_reset_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_pb_pbus_no_data_hw_reset_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_pb_pbus_no_data_hw_reset_t(string name = "cap_psp_csr_cnt_pb_pbus_no_data_hw_reset_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_pb_pbus_no_data_hw_reset_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_pb_pbus_no_data_hw_reset_t
    
class cap_psp_csr_cnt_pb_pbus_no_data_hw_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_pb_pbus_no_data_hw_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_pb_pbus_no_data_hw_set_t(string name = "cap_psp_csr_cnt_pb_pbus_no_data_hw_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_pb_pbus_no_data_hw_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_pb_pbus_no_data_hw_set_t
    
class cap_psp_csr_sta_debug_trace_pr_pkt_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sta_debug_trace_pr_pkt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sta_debug_trace_pr_pkt_t(string name = "cap_psp_csr_sta_debug_trace_pr_pkt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sta_debug_trace_pr_pkt_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 512 > sv21_pr_pkt_data_line_cpp_int_t;
        cpp_int int_var__sv21_pr_pkt_data_line;
        void sv21_pr_pkt_data_line (const cpp_int  & l__val);
        cpp_int sv21_pr_pkt_data_line() const;
    
        typedef pu_cpp_int< 6 > sv21_pr_pkt_data_size_cpp_int_t;
        cpp_int int_var__sv21_pr_pkt_data_size;
        void sv21_pr_pkt_data_size (const cpp_int  & l__val);
        cpp_int sv21_pr_pkt_data_size() const;
    
        typedef pu_cpp_int< 14 > sv21_pr_pkt_packet_len_cpp_int_t;
        cpp_int int_var__sv21_pr_pkt_packet_len;
        void sv21_pr_pkt_packet_len (const cpp_int  & l__val);
        cpp_int sv21_pr_pkt_packet_len() const;
    
        typedef pu_cpp_int< 5 > sv21_pr_pkt_tm_oq_cpp_int_t;
        cpp_int int_var__sv21_pr_pkt_tm_oq;
        void sv21_pr_pkt_tm_oq (const cpp_int  & l__val);
        cpp_int sv21_pr_pkt_tm_oq() const;
    
        typedef pu_cpp_int< 1 > pr_pkt_ff_almost_full_dly_cpp_int_t;
        cpp_int int_var__pr_pkt_ff_almost_full_dly;
        void pr_pkt_ff_almost_full_dly (const cpp_int  & l__val);
        cpp_int pr_pkt_ff_almost_full_dly() const;
    
        typedef pu_cpp_int< 1 > ptd_npv_phv_full_dly_cpp_int_t;
        cpp_int int_var__ptd_npv_phv_full_dly;
        void ptd_npv_phv_full_dly (const cpp_int  & l__val);
        cpp_int ptd_npv_phv_full_dly() const;
    
        typedef pu_cpp_int< 1 > psp_prd_phv_valid_dly_cpp_int_t;
        cpp_int int_var__psp_prd_phv_valid_dly;
        void psp_prd_phv_valid_dly (const cpp_int  & l__val);
        cpp_int psp_prd_phv_valid_dly() const;
    
        typedef pu_cpp_int< 1 > sv21_pr_pkt_recirc_cpp_int_t;
        cpp_int int_var__sv21_pr_pkt_recirc;
        void sv21_pr_pkt_recirc (const cpp_int  & l__val);
        cpp_int sv21_pr_pkt_recirc() const;
    
        typedef pu_cpp_int< 1 > sv21_pr_pkt_data_err_cpp_int_t;
        cpp_int int_var__sv21_pr_pkt_data_err;
        void sv21_pr_pkt_data_err (const cpp_int  & l__val);
        cpp_int sv21_pr_pkt_data_err() const;
    
        typedef pu_cpp_int< 1 > sv21_pr_pkt_data_eop_cpp_int_t;
        cpp_int int_var__sv21_pr_pkt_data_eop;
        void sv21_pr_pkt_data_eop (const cpp_int  & l__val);
        cpp_int sv21_pr_pkt_data_eop() const;
    
        typedef pu_cpp_int< 1 > sv21_pr_pkt_data_sop_cpp_int_t;
        cpp_int int_var__sv21_pr_pkt_data_sop;
        void sv21_pr_pkt_data_sop (const cpp_int  & l__val);
        cpp_int sv21_pr_pkt_data_sop() const;
    
}; // cap_psp_csr_sta_debug_trace_pr_pkt_t
    
class cap_psp_csr_sta_debug_trace_ma_phv_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sta_debug_trace_ma_phv_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sta_debug_trace_ma_phv_t(string name = "cap_psp_csr_sta_debug_trace_ma_phv_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sta_debug_trace_ma_phv_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 512 > sv21_ma_phv_data_phv_cpp_int_t;
        cpp_int int_var__sv21_ma_phv_data_phv;
        void sv21_ma_phv_data_phv (const cpp_int  & l__val);
        cpp_int sv21_ma_phv_data_phv() const;
    
        typedef pu_cpp_int< 1 > sv21_ma_phv_data_eop_cpp_int_t;
        cpp_int int_var__sv21_ma_phv_data_eop;
        void sv21_ma_phv_data_eop (const cpp_int  & l__val);
        cpp_int sv21_ma_phv_data_eop() const;
    
        typedef pu_cpp_int< 1 > sv21_ma_phv_data_sop_cpp_int_t;
        cpp_int int_var__sv21_ma_phv_data_sop;
        void sv21_ma_phv_data_sop (const cpp_int  & l__val);
        cpp_int sv21_ma_phv_data_sop() const;
    
}; // cap_psp_csr_sta_debug_trace_ma_phv_t
    
class cap_psp_csr_sta_debug_trace_lb_pkt_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sta_debug_trace_lb_pkt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sta_debug_trace_lb_pkt_t(string name = "cap_psp_csr_sta_debug_trace_lb_pkt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sta_debug_trace_lb_pkt_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 512 > sv13_lb_pkt_data_line_cpp_int_t;
        cpp_int int_var__sv13_lb_pkt_data_line;
        void sv13_lb_pkt_data_line (const cpp_int  & l__val);
        cpp_int sv13_lb_pkt_data_line() const;
    
        typedef pu_cpp_int< 6 > sv13_lb_pkt_data_size_cpp_int_t;
        cpp_int int_var__sv13_lb_pkt_data_size;
        void sv13_lb_pkt_data_size (const cpp_int  & l__val);
        cpp_int sv13_lb_pkt_data_size() const;
    
        typedef pu_cpp_int< 1 > sv13_lb_pkt_data_err_cpp_int_t;
        cpp_int int_var__sv13_lb_pkt_data_err;
        void sv13_lb_pkt_data_err (const cpp_int  & l__val);
        cpp_int sv13_lb_pkt_data_err() const;
    
        typedef pu_cpp_int< 1 > sv13_lb_pkt_data_eop_cpp_int_t;
        cpp_int int_var__sv13_lb_pkt_data_eop;
        void sv13_lb_pkt_data_eop (const cpp_int  & l__val);
        cpp_int sv13_lb_pkt_data_eop() const;
    
        typedef pu_cpp_int< 1 > sv13_lb_pkt_data_sop_cpp_int_t;
        cpp_int int_var__sv13_lb_pkt_data_sop;
        void sv13_lb_pkt_data_sop (const cpp_int  & l__val);
        cpp_int sv13_lb_pkt_data_sop() const;
    
}; // cap_psp_csr_sta_debug_trace_lb_pkt_t
    
class cap_psp_csr_sta_debug_trace_lb_phv_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sta_debug_trace_lb_phv_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sta_debug_trace_lb_phv_t(string name = "cap_psp_csr_sta_debug_trace_lb_phv_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sta_debug_trace_lb_phv_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 512 > sv13_lb_phv_data_phv_cpp_int_t;
        cpp_int int_var__sv13_lb_phv_data_phv;
        void sv13_lb_phv_data_phv (const cpp_int  & l__val);
        cpp_int sv13_lb_phv_data_phv() const;
    
        typedef pu_cpp_int< 1 > sv13_lb_phv_data_eop_cpp_int_t;
        cpp_int int_var__sv13_lb_phv_data_eop;
        void sv13_lb_phv_data_eop (const cpp_int  & l__val);
        cpp_int sv13_lb_phv_data_eop() const;
    
        typedef pu_cpp_int< 1 > sv13_lb_phv_data_sop_cpp_int_t;
        cpp_int int_var__sv13_lb_phv_data_sop;
        void sv13_lb_phv_data_sop (const cpp_int  & l__val);
        cpp_int sv13_lb_phv_data_sop() const;
    
}; // cap_psp_csr_sta_debug_trace_lb_phv_t
    
class cap_psp_csr_sta_debug_trace_pb_pkt_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sta_debug_trace_pb_pkt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sta_debug_trace_pb_pkt_t(string name = "cap_psp_csr_sta_debug_trace_pb_pkt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sta_debug_trace_pb_pkt_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 512 > sv01_pb_pkt_data_line_cpp_int_t;
        cpp_int int_var__sv01_pb_pkt_data_line;
        void sv01_pb_pkt_data_line (const cpp_int  & l__val);
        cpp_int sv01_pb_pkt_data_line() const;
    
        typedef pu_cpp_int< 6 > sv01_pb_pkt_data_size_cpp_int_t;
        cpp_int int_var__sv01_pb_pkt_data_size;
        void sv01_pb_pkt_data_size (const cpp_int  & l__val);
        cpp_int sv01_pb_pkt_data_size() const;
    
        typedef pu_cpp_int< 1 > sv01_pb_pkt_data_err_cpp_int_t;
        cpp_int int_var__sv01_pb_pkt_data_err;
        void sv01_pb_pkt_data_err (const cpp_int  & l__val);
        cpp_int sv01_pb_pkt_data_err() const;
    
        typedef pu_cpp_int< 1 > sv01_pb_pkt_data_eop_cpp_int_t;
        cpp_int int_var__sv01_pb_pkt_data_eop;
        void sv01_pb_pkt_data_eop (const cpp_int  & l__val);
        cpp_int sv01_pb_pkt_data_eop() const;
    
        typedef pu_cpp_int< 1 > sv01_pb_pkt_data_sop_cpp_int_t;
        cpp_int int_var__sv01_pb_pkt_data_sop;
        void sv01_pb_pkt_data_sop (const cpp_int  & l__val);
        cpp_int sv01_pb_pkt_data_sop() const;
    
}; // cap_psp_csr_sta_debug_trace_pb_pkt_t
    
class cap_psp_csr_sta_debug_trace_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sta_debug_trace_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sta_debug_trace_t(string name = "cap_psp_csr_sta_debug_trace_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sta_debug_trace_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 48 > pb_pkt_timestamp_cpp_int_t;
        cpp_int int_var__pb_pkt_timestamp;
        void pb_pkt_timestamp (const cpp_int  & l__val);
        cpp_int pb_pkt_timestamp() const;
    
        typedef pu_cpp_int< 1 > pb_pkt_captured_cpp_int_t;
        cpp_int int_var__pb_pkt_captured;
        void pb_pkt_captured (const cpp_int  & l__val);
        cpp_int pb_pkt_captured() const;
    
        typedef pu_cpp_int< 1 > lb_phv_captured_cpp_int_t;
        cpp_int int_var__lb_phv_captured;
        void lb_phv_captured (const cpp_int  & l__val);
        cpp_int lb_phv_captured() const;
    
        typedef pu_cpp_int< 1 > lb_pkt_captured_cpp_int_t;
        cpp_int int_var__lb_pkt_captured;
        void lb_pkt_captured (const cpp_int  & l__val);
        cpp_int lb_pkt_captured() const;
    
        typedef pu_cpp_int< 1 > ma_phv_captured_cpp_int_t;
        cpp_int int_var__ma_phv_captured;
        void ma_phv_captured (const cpp_int  & l__val);
        cpp_int ma_phv_captured() const;
    
        typedef pu_cpp_int< 1 > pr_pkt_captured_cpp_int_t;
        cpp_int int_var__pr_pkt_captured;
        void pr_pkt_captured (const cpp_int  & l__val);
        cpp_int pr_pkt_captured() const;
    
}; // cap_psp_csr_sta_debug_trace_t
    
class cap_psp_csr_sta_fsm_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sta_fsm_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sta_fsm_t(string name = "cap_psp_csr_sta_fsm_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sta_fsm_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 3 > split_state_cpp_int_t;
        cpp_int int_var__split_state;
        void split_state (const cpp_int  & l__val);
        cpp_int split_state() const;
    
        typedef pu_cpp_int< 2 > phv_state_cpp_int_t;
        cpp_int int_var__phv_state;
        void phv_state (const cpp_int  & l__val);
        cpp_int phv_state() const;
    
        typedef pu_cpp_int< 2 > pkt_state_cpp_int_t;
        cpp_int int_var__pkt_state;
        void pkt_state (const cpp_int  & l__val);
        cpp_int pkt_state() const;
    
        typedef pu_cpp_int< 1 > sv01_pb_pkt_in_pkt_cpp_int_t;
        cpp_int int_var__sv01_pb_pkt_in_pkt;
        void sv01_pb_pkt_in_pkt (const cpp_int  & l__val);
        cpp_int sv01_pb_pkt_in_pkt() const;
    
        typedef pu_cpp_int< 1 > sv03_lt_req_in_progress_cpp_int_t;
        cpp_int int_var__sv03_lt_req_in_progress;
        void sv03_lt_req_in_progress (const cpp_int  & l__val);
        cpp_int sv03_lt_req_in_progress() const;
    
        typedef pu_cpp_int< 1 > sv03_sw_req_in_pkt_cpp_int_t;
        cpp_int int_var__sv03_sw_req_in_pkt;
        void sv03_sw_req_in_pkt (const cpp_int  & l__val);
        cpp_int sv03_sw_req_in_pkt() const;
    
        typedef pu_cpp_int< 1 > sv03_sw_req_in_progress_cpp_int_t;
        cpp_int int_var__sv03_sw_req_in_progress;
        void sv03_sw_req_in_progress (const cpp_int  & l__val);
        cpp_int sv03_sw_req_in_progress() const;
    
        typedef pu_cpp_int< 1 > sv13_lb_phv_in_pkt_cpp_int_t;
        cpp_int int_var__sv13_lb_phv_in_pkt;
        void sv13_lb_phv_in_pkt (const cpp_int  & l__val);
        cpp_int sv13_lb_phv_in_pkt() const;
    
        typedef pu_cpp_int< 1 > sv13_lb_pkt_in_pkt_cpp_int_t;
        cpp_int int_var__sv13_lb_pkt_in_pkt;
        void sv13_lb_pkt_in_pkt (const cpp_int  & l__val);
        cpp_int sv13_lb_pkt_in_pkt() const;
    
        typedef pu_cpp_int< 1 > sv15_pr_pkt_in_pkt_cpp_int_t;
        cpp_int int_var__sv15_pr_pkt_in_pkt;
        void sv15_pr_pkt_in_pkt (const cpp_int  & l__val);
        cpp_int sv15_pr_pkt_in_pkt() const;
    
        typedef pu_cpp_int< 1 > sv20_ma_phv_in_pkt_cpp_int_t;
        cpp_int int_var__sv20_ma_phv_in_pkt;
        void sv20_ma_phv_in_pkt (const cpp_int  & l__val);
        cpp_int sv20_ma_phv_in_pkt() const;
    
}; // cap_psp_csr_sta_fsm_t
    
class cap_psp_csr_sta_flow_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sta_flow_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sta_flow_t(string name = "cap_psp_csr_sta_flow_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sta_flow_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > ma_drdy_cpp_int_t;
        cpp_int int_var__ma_drdy;
        void ma_drdy (const cpp_int  & l__val);
        cpp_int ma_drdy() const;
    
        typedef pu_cpp_int< 1 > pb_pbus_drdy_cpp_int_t;
        cpp_int int_var__pb_pbus_drdy;
        void pb_pbus_drdy (const cpp_int  & l__val);
        cpp_int pb_pbus_drdy() const;
    
        typedef pu_cpp_int< 1 > pkt_order_push_drdy_cpp_int_t;
        cpp_int int_var__pkt_order_push_drdy;
        void pkt_order_push_drdy (const cpp_int  & l__val);
        cpp_int pkt_order_push_drdy() const;
    
        typedef pu_cpp_int< 1 > pr_resub_drdy_cpp_int_t;
        cpp_int int_var__pr_resub_drdy;
        void pr_resub_drdy (const cpp_int  & l__val);
        cpp_int pr_resub_drdy() const;
    
        typedef pu_cpp_int< 1 > pr_resub_pbus_drdy_cpp_int_t;
        cpp_int int_var__pr_resub_pbus_drdy;
        void pr_resub_pbus_drdy (const cpp_int  & l__val);
        cpp_int pr_resub_pbus_drdy() const;
    
        typedef pu_cpp_int< 1 > sv01_pb_pkt_drdy_cpp_int_t;
        cpp_int int_var__sv01_pb_pkt_drdy;
        void sv01_pb_pkt_drdy (const cpp_int  & l__val);
        cpp_int sv01_pb_pkt_drdy() const;
    
        typedef pu_cpp_int< 1 > sv01_lb_phv_drdy_cpp_int_t;
        cpp_int int_var__sv01_lb_phv_drdy;
        void sv01_lb_phv_drdy (const cpp_int  & l__val);
        cpp_int sv01_lb_phv_drdy() const;
    
        typedef pu_cpp_int< 1 > sv02_lb_pkt_drdy_cpp_int_t;
        cpp_int int_var__sv02_lb_pkt_drdy;
        void sv02_lb_pkt_drdy (const cpp_int  & l__val);
        cpp_int sv02_lb_pkt_drdy() const;
    
        typedef pu_cpp_int< 1 > sv04_lt_ovr_drdy_cpp_int_t;
        cpp_int int_var__sv04_lt_ovr_drdy;
        void sv04_lt_ovr_drdy (const cpp_int  & l__val);
        cpp_int sv04_lt_ovr_drdy() const;
    
        typedef pu_cpp_int< 1 > sv04_lt_rsp_drdy_cpp_int_t;
        cpp_int int_var__sv04_lt_rsp_drdy;
        void sv04_lt_rsp_drdy (const cpp_int  & l__val);
        cpp_int sv04_lt_rsp_drdy() const;
    
        typedef pu_cpp_int< 1 > sv11_in_phv_drdy_cpp_int_t;
        cpp_int int_var__sv11_in_phv_drdy;
        void sv11_in_phv_drdy (const cpp_int  & l__val);
        cpp_int sv11_in_phv_drdy() const;
    
        typedef pu_cpp_int< 1 > sv11_in_pkt_drdy_cpp_int_t;
        cpp_int int_var__sv11_in_pkt_drdy;
        void sv11_in_pkt_drdy (const cpp_int  & l__val);
        cpp_int sv11_in_pkt_drdy() const;
    
        typedef pu_cpp_int< 1 > sv14_ma_phv_drdy_cpp_int_t;
        cpp_int int_var__sv14_ma_phv_drdy;
        void sv14_ma_phv_drdy (const cpp_int  & l__val);
        cpp_int sv14_ma_phv_drdy() const;
    
        typedef pu_cpp_int< 1 > sv15_pr_pkt_drdy_cpp_int_t;
        cpp_int int_var__sv15_pr_pkt_drdy;
        void sv15_pr_pkt_drdy (const cpp_int  & l__val);
        cpp_int sv15_pr_pkt_drdy() const;
    
        typedef pu_cpp_int< 1 > sv20_ma_phv_drdy_cpp_int_t;
        cpp_int int_var__sv20_ma_phv_drdy;
        void sv20_ma_phv_drdy (const cpp_int  & l__val);
        cpp_int sv20_ma_phv_drdy() const;
    
        typedef pu_cpp_int< 1 > sv21_ma_phv_drdy_cpp_int_t;
        cpp_int int_var__sv21_ma_phv_drdy;
        void sv21_ma_phv_drdy (const cpp_int  & l__val);
        cpp_int sv21_ma_phv_drdy() const;
    
        typedef pu_cpp_int< 1 > pr_pkt_ff_almost_full_cpp_int_t;
        cpp_int int_var__pr_pkt_ff_almost_full;
        void pr_pkt_ff_almost_full (const cpp_int  & l__val);
        cpp_int pr_pkt_ff_almost_full() const;
    
        typedef pu_cpp_int< 1 > ptd_npv_phv_full_cpp_int_t;
        cpp_int int_var__ptd_npv_phv_full;
        void ptd_npv_phv_full (const cpp_int  & l__val);
        cpp_int ptd_npv_phv_full() const;
    
}; // cap_psp_csr_sta_flow_t
    
class cap_psp_csr_sta_sw_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sta_sw_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sta_sw_phv_mem_t(string name = "cap_psp_csr_sta_sw_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sta_sw_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > ecc_correctable_cpp_int_t;
        cpp_int int_var__ecc_correctable;
        void ecc_correctable (const cpp_int  & l__val);
        cpp_int ecc_correctable() const;
    
        typedef pu_cpp_int< 1 > ecc_uncorrectable_cpp_int_t;
        cpp_int int_var__ecc_uncorrectable;
        void ecc_uncorrectable (const cpp_int  & l__val);
        cpp_int ecc_uncorrectable() const;
    
        typedef pu_cpp_int< 20 > ecc_syndrome_cpp_int_t;
        cpp_int int_var__ecc_syndrome;
        void ecc_syndrome (const cpp_int  & l__val);
        cpp_int ecc_syndrome() const;
    
        typedef pu_cpp_int< 4 > ecc_addr_cpp_int_t;
        cpp_int int_var__ecc_addr;
        void ecc_addr (const cpp_int  & l__val);
        cpp_int ecc_addr() const;
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
}; // cap_psp_csr_sta_sw_phv_mem_t
    
class cap_psp_csr_cfg_sw_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cfg_sw_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cfg_sw_phv_mem_t(string name = "cap_psp_csr_cfg_sw_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cfg_sw_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > ecc_detect_cpp_int_t;
        cpp_int int_var__ecc_detect;
        void ecc_detect (const cpp_int  & l__val);
        cpp_int ecc_detect() const;
    
        typedef pu_cpp_int< 1 > ecc_correct_cpp_int_t;
        cpp_int int_var__ecc_correct;
        void ecc_correct (const cpp_int  & l__val);
        cpp_int ecc_correct() const;
    
        typedef pu_cpp_int< 1 > ecc_bypass_cpp_int_t;
        cpp_int int_var__ecc_bypass;
        void ecc_bypass (const cpp_int  & l__val);
        cpp_int ecc_bypass() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_psp_csr_cfg_sw_phv_mem_t
    
class cap_psp_csr_sta_sw_phv_state_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sta_sw_phv_state_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sta_sw_phv_state_t(string name = "cap_psp_csr_sta_sw_phv_state_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sta_sw_phv_state_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > done_cpp_int_t;
        cpp_int int_var__done;
        void done (const cpp_int  & l__val);
        cpp_int done() const;
    
        typedef pu_cpp_int< 42 > current_insertion_period_cpp_int_t;
        cpp_int int_var__current_insertion_period;
        void current_insertion_period (const cpp_int  & l__val);
        cpp_int current_insertion_period() const;
    
        typedef pu_cpp_int< 14 > current_counter_cpp_int_t;
        cpp_int int_var__current_counter;
        void current_counter (const cpp_int  & l__val);
        cpp_int current_counter() const;
    
        typedef pu_cpp_int< 4 > current_flit_cpp_int_t;
        cpp_int int_var__current_flit;
        void current_flit (const cpp_int  & l__val);
        cpp_int current_flit() const;
    
        typedef pu_cpp_int< 24 > current_qid_cpp_int_t;
        cpp_int int_var__current_qid;
        void current_qid (const cpp_int  & l__val);
        cpp_int current_qid() const;
    
}; // cap_psp_csr_sta_sw_phv_state_t
    
class cap_psp_csr_cfg_sw_phv_config_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cfg_sw_phv_config_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cfg_sw_phv_config_t(string name = "cap_psp_csr_cfg_sw_phv_config_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cfg_sw_phv_config_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 4 > start_addr_cpp_int_t;
        cpp_int int_var__start_addr;
        void start_addr (const cpp_int  & l__val);
        cpp_int start_addr() const;
    
        typedef pu_cpp_int< 4 > num_flits_cpp_int_t;
        cpp_int int_var__num_flits;
        void num_flits (const cpp_int  & l__val);
        cpp_int num_flits() const;
    
        typedef pu_cpp_int< 42 > insertion_period_clocks_cpp_int_t;
        cpp_int int_var__insertion_period_clocks;
        void insertion_period_clocks (const cpp_int  & l__val);
        cpp_int insertion_period_clocks() const;
    
        typedef pu_cpp_int< 14 > counter_max_cpp_int_t;
        cpp_int int_var__counter_max;
        void counter_max (const cpp_int  & l__val);
        cpp_int counter_max() const;
    
        typedef pu_cpp_int< 24 > qid_min_cpp_int_t;
        cpp_int int_var__qid_min;
        void qid_min (const cpp_int  & l__val);
        cpp_int qid_min() const;
    
        typedef pu_cpp_int< 24 > qid_max_cpp_int_t;
        cpp_int int_var__qid_max;
        void qid_max (const cpp_int  & l__val);
        cpp_int qid_max() const;
    
}; // cap_psp_csr_cfg_sw_phv_config_t
    
class cap_psp_csr_cfg_sw_phv_control_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cfg_sw_phv_control_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cfg_sw_phv_control_t(string name = "cap_psp_csr_cfg_sw_phv_control_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cfg_sw_phv_control_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > start_enable_cpp_int_t;
        cpp_int int_var__start_enable;
        void start_enable (const cpp_int  & l__val);
        cpp_int start_enable() const;
    
        typedef pu_cpp_int< 1 > counter_repeat_enable_cpp_int_t;
        cpp_int int_var__counter_repeat_enable;
        void counter_repeat_enable (const cpp_int  & l__val);
        cpp_int counter_repeat_enable() const;
    
        typedef pu_cpp_int< 1 > qid_repeat_enable_cpp_int_t;
        cpp_int int_var__qid_repeat_enable;
        void qid_repeat_enable (const cpp_int  & l__val);
        cpp_int qid_repeat_enable() const;
    
        typedef pu_cpp_int< 1 > localtime_enable_cpp_int_t;
        cpp_int int_var__localtime_enable;
        void localtime_enable (const cpp_int  & l__val);
        cpp_int localtime_enable() const;
    
        typedef pu_cpp_int< 1 > frame_size_enable_cpp_int_t;
        cpp_int int_var__frame_size_enable;
        void frame_size_enable (const cpp_int  & l__val);
        cpp_int frame_size_enable() const;
    
        typedef pu_cpp_int< 1 > packet_len_enable_cpp_int_t;
        cpp_int int_var__packet_len_enable;
        void packet_len_enable (const cpp_int  & l__val);
        cpp_int packet_len_enable() const;
    
        typedef pu_cpp_int< 1 > qid_enable_cpp_int_t;
        cpp_int int_var__qid_enable;
        void qid_enable (const cpp_int  & l__val);
        cpp_int qid_enable() const;
    
}; // cap_psp_csr_cfg_sw_phv_control_t
    
class cap_psp_csr_cfg_sw_phv_global_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cfg_sw_phv_global_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cfg_sw_phv_global_t(string name = "cap_psp_csr_cfg_sw_phv_global_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cfg_sw_phv_global_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > start_enable_cpp_int_t;
        cpp_int int_var__start_enable;
        void start_enable (const cpp_int  & l__val);
        cpp_int start_enable() const;
    
        typedef pu_cpp_int< 1 > err_enable_cpp_int_t;
        cpp_int int_var__err_enable;
        void err_enable (const cpp_int  & l__val);
        cpp_int err_enable() const;
    
}; // cap_psp_csr_cfg_sw_phv_global_t
    
class cap_psp_csr_sta_lif_table_sram_bist_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sta_lif_table_sram_bist_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sta_lif_table_sram_bist_t(string name = "cap_psp_csr_sta_lif_table_sram_bist_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sta_lif_table_sram_bist_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_psp_csr_sta_lif_table_sram_bist_t
    
class cap_psp_csr_cfg_lif_table_sram_bist_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cfg_lif_table_sram_bist_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cfg_lif_table_sram_bist_t(string name = "cap_psp_csr_cfg_lif_table_sram_bist_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cfg_lif_table_sram_bist_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_psp_csr_cfg_lif_table_sram_bist_t
    
class cap_psp_csr_sta_ecc_lif_qstate_map_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sta_ecc_lif_qstate_map_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sta_ecc_lif_qstate_map_t(string name = "cap_psp_csr_sta_ecc_lif_qstate_map_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sta_ecc_lif_qstate_map_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_psp_csr_sta_ecc_lif_qstate_map_t
    
class cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t(string name = "cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > cor_cpp_int_t;
        cpp_int int_var__cor;
        void cor (const cpp_int  & l__val);
        cpp_int cor() const;
    
        typedef pu_cpp_int< 1 > det_cpp_int_t;
        cpp_int int_var__det;
        void det (const cpp_int  & l__val);
        cpp_int det() const;
    
        typedef pu_cpp_int< 1 > dhs_cpp_int_t;
        cpp_int int_var__dhs;
        void dhs (const cpp_int  & l__val);
        cpp_int dhs() const;
    
}; // cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t
    
class cap_psp_csr_csr_intr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_csr_intr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_csr_intr_t(string name = "cap_psp_csr_csr_intr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_csr_intr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_psp_csr_csr_intr_t
    
class cap_psp_csr_cnt_ptd_npv_phv_full_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_ptd_npv_phv_full_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_ptd_npv_phv_full_t(string name = "cap_psp_csr_cnt_ptd_npv_phv_full_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_ptd_npv_phv_full_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_ptd_npv_phv_full_t
    
class cap_psp_csr_cnt_psp_prd_phv_valid_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_psp_prd_phv_valid_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_psp_prd_phv_valid_t(string name = "cap_psp_csr_cnt_psp_prd_phv_valid_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_psp_prd_phv_valid_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_psp_prd_phv_valid_t
    
class cap_psp_csr_sat_psp_csr_write_access_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sat_psp_csr_write_access_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sat_psp_csr_write_access_err_t(string name = "cap_psp_csr_sat_psp_csr_write_access_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sat_psp_csr_write_access_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_sat_psp_csr_write_access_err_t
    
class cap_psp_csr_sat_psp_csr_read_access_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sat_psp_csr_read_access_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sat_psp_csr_read_access_err_t(string name = "cap_psp_csr_sat_psp_csr_read_access_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sat_psp_csr_read_access_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_sat_psp_csr_read_access_err_t
    
class cap_psp_csr_cnt_psp_csr_write_access_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_psp_csr_write_access_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_psp_csr_write_access_t(string name = "cap_psp_csr_cnt_psp_csr_write_access_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_psp_csr_write_access_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_psp_csr_write_access_t
    
class cap_psp_csr_cnt_psp_csr_read_access_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_psp_csr_read_access_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_psp_csr_read_access_t(string name = "cap_psp_csr_cnt_psp_csr_read_access_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_psp_csr_read_access_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_psp_csr_read_access_t
    
class cap_psp_csr_sat_pr_resub_eop_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sat_pr_resub_eop_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sat_pr_resub_eop_err_t(string name = "cap_psp_csr_sat_pr_resub_eop_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sat_pr_resub_eop_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_sat_pr_resub_eop_err_t
    
class cap_psp_csr_sat_pr_resub_sop_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sat_pr_resub_sop_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sat_pr_resub_sop_err_t(string name = "cap_psp_csr_sat_pr_resub_sop_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sat_pr_resub_sop_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_sat_pr_resub_sop_err_t
    
class cap_psp_csr_cnt_pr_resub_no_data_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_pr_resub_no_data_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_pr_resub_no_data_t(string name = "cap_psp_csr_cnt_pr_resub_no_data_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_pr_resub_no_data_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_pr_resub_no_data_t
    
class cap_psp_csr_cnt_pr_resub_eop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_pr_resub_eop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_pr_resub_eop_t(string name = "cap_psp_csr_cnt_pr_resub_eop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_pr_resub_eop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_pr_resub_eop_t
    
class cap_psp_csr_cnt_pr_resub_sop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_pr_resub_sop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_pr_resub_sop_t(string name = "cap_psp_csr_cnt_pr_resub_sop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_pr_resub_sop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_pr_resub_sop_t
    
class cap_psp_csr_cnt_pr_resub_xns_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_pr_resub_xns_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_pr_resub_xns_t(string name = "cap_psp_csr_cnt_pr_resub_xns_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_pr_resub_xns_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_pr_resub_xns_t
    
class cap_psp_csr_sat_pr_resub_pbus_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sat_pr_resub_pbus_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sat_pr_resub_pbus_err_t(string name = "cap_psp_csr_sat_pr_resub_pbus_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sat_pr_resub_pbus_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_sat_pr_resub_pbus_err_t
    
class cap_psp_csr_sat_pr_resub_pbus_eop_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sat_pr_resub_pbus_eop_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sat_pr_resub_pbus_eop_err_t(string name = "cap_psp_csr_sat_pr_resub_pbus_eop_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sat_pr_resub_pbus_eop_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_sat_pr_resub_pbus_eop_err_t
    
class cap_psp_csr_sat_pr_resub_pbus_sop_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sat_pr_resub_pbus_sop_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sat_pr_resub_pbus_sop_err_t(string name = "cap_psp_csr_sat_pr_resub_pbus_sop_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sat_pr_resub_pbus_sop_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_sat_pr_resub_pbus_sop_err_t
    
class cap_psp_csr_cnt_pr_resub_pbus_eop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_pr_resub_pbus_eop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_pr_resub_pbus_eop_t(string name = "cap_psp_csr_cnt_pr_resub_pbus_eop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_pr_resub_pbus_eop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_pr_resub_pbus_eop_t
    
class cap_psp_csr_cnt_pr_resub_pbus_sop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_pr_resub_pbus_sop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_pr_resub_pbus_sop_t(string name = "cap_psp_csr_cnt_pr_resub_pbus_sop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_pr_resub_pbus_sop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_pr_resub_pbus_sop_t
    
class cap_psp_csr_cnt_pr_resub_pbus_xns_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_pr_resub_pbus_xns_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_pr_resub_pbus_xns_t(string name = "cap_psp_csr_cnt_pr_resub_pbus_xns_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_pr_resub_pbus_xns_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_pr_resub_pbus_xns_t
    
class cap_psp_csr_cnt_pr_pkt_ff_almost_full_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_pr_pkt_ff_almost_full_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_pr_pkt_ff_almost_full_t(string name = "cap_psp_csr_cnt_pr_pkt_ff_almost_full_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_pr_pkt_ff_almost_full_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_pr_pkt_ff_almost_full_t
    
class cap_psp_csr_sat_pr_pbus_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sat_pr_pbus_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sat_pr_pbus_err_t(string name = "cap_psp_csr_sat_pr_pbus_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sat_pr_pbus_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_sat_pr_pbus_err_t
    
class cap_psp_csr_sat_pr_pbus_eop_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sat_pr_pbus_eop_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sat_pr_pbus_eop_err_t(string name = "cap_psp_csr_sat_pr_pbus_eop_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sat_pr_pbus_eop_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_sat_pr_pbus_eop_err_t
    
class cap_psp_csr_sat_pr_pbus_sop_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sat_pr_pbus_sop_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sat_pr_pbus_sop_err_t(string name = "cap_psp_csr_sat_pr_pbus_sop_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sat_pr_pbus_sop_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_sat_pr_pbus_sop_err_t
    
class cap_psp_csr_cnt_pr_pbus_recirc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_pr_pbus_recirc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_pr_pbus_recirc_t(string name = "cap_psp_csr_cnt_pr_pbus_recirc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_pr_pbus_recirc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_pr_pbus_recirc_t
    
class cap_psp_csr_cnt_pr_pbus_eop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_pr_pbus_eop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_pr_pbus_eop_t(string name = "cap_psp_csr_cnt_pr_pbus_eop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_pr_pbus_eop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_pr_pbus_eop_t
    
class cap_psp_csr_cnt_pr_pbus_sop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_pr_pbus_sop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_pr_pbus_sop_t(string name = "cap_psp_csr_cnt_pr_pbus_sop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_pr_pbus_sop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_pr_pbus_sop_t
    
class cap_psp_csr_cnt_pr_pbus_xns_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_pr_pbus_xns_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_pr_pbus_xns_t(string name = "cap_psp_csr_cnt_pr_pbus_xns_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_pr_pbus_xns_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_pr_pbus_xns_t
    
class cap_psp_csr_sat_ma_eop_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sat_ma_eop_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sat_ma_eop_err_t(string name = "cap_psp_csr_sat_ma_eop_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sat_ma_eop_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_sat_ma_eop_err_t
    
class cap_psp_csr_sat_ma_sop_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sat_ma_sop_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sat_ma_sop_err_t(string name = "cap_psp_csr_sat_ma_sop_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sat_ma_sop_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_sat_ma_sop_err_t
    
class cap_psp_csr_cnt_ma_drop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_ma_drop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_ma_drop_t(string name = "cap_psp_csr_cnt_ma_drop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_ma_drop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_ma_drop_t
    
class cap_psp_csr_cnt_ma_collapsed_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_ma_collapsed_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_ma_collapsed_t(string name = "cap_psp_csr_cnt_ma_collapsed_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_ma_collapsed_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_ma_collapsed_t
    
class cap_psp_csr_cnt_ma_no_data_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_ma_no_data_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_ma_no_data_t(string name = "cap_psp_csr_cnt_ma_no_data_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_ma_no_data_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_ma_no_data_t
    
class cap_psp_csr_cnt_ma_recirc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_ma_recirc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_ma_recirc_t(string name = "cap_psp_csr_cnt_ma_recirc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_ma_recirc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_ma_recirc_t
    
class cap_psp_csr_cnt_ma_eop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_ma_eop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_ma_eop_t(string name = "cap_psp_csr_cnt_ma_eop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_ma_eop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_ma_eop_t
    
class cap_psp_csr_cnt_ma_sop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_ma_sop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_ma_sop_t(string name = "cap_psp_csr_cnt_ma_sop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_ma_sop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_ma_sop_t
    
class cap_psp_csr_cnt_ma_xns_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_ma_xns_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_ma_xns_t(string name = "cap_psp_csr_cnt_ma_xns_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_ma_xns_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_ma_xns_t
    
class cap_psp_csr_sat_pb_pbus_nopkt_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sat_pb_pbus_nopkt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sat_pb_pbus_nopkt_t(string name = "cap_psp_csr_sat_pb_pbus_nopkt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sat_pb_pbus_nopkt_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_sat_pb_pbus_nopkt_t
    
class cap_psp_csr_sat_pb_pbus_dummy_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sat_pb_pbus_dummy_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sat_pb_pbus_dummy_t(string name = "cap_psp_csr_sat_pb_pbus_dummy_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sat_pb_pbus_dummy_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_sat_pb_pbus_dummy_t
    
class cap_psp_csr_sat_pb_pbus_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sat_pb_pbus_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sat_pb_pbus_err_t(string name = "cap_psp_csr_sat_pb_pbus_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sat_pb_pbus_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_sat_pb_pbus_err_t
    
class cap_psp_csr_sat_pb_pbus_eop_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sat_pb_pbus_eop_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sat_pb_pbus_eop_err_t(string name = "cap_psp_csr_sat_pb_pbus_eop_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sat_pb_pbus_eop_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_sat_pb_pbus_eop_err_t
    
class cap_psp_csr_sat_pb_pbus_sop_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sat_pb_pbus_sop_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sat_pb_pbus_sop_err_t(string name = "cap_psp_csr_sat_pb_pbus_sop_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sat_pb_pbus_sop_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_sat_pb_pbus_sop_err_t
    
class cap_psp_csr_cnt_pb_pbus_no_data_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_pb_pbus_no_data_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_pb_pbus_no_data_t(string name = "cap_psp_csr_cnt_pb_pbus_no_data_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_pb_pbus_no_data_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_pb_pbus_no_data_t
    
class cap_psp_csr_cnt_pb_pbus_eop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_pb_pbus_eop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_pb_pbus_eop_t(string name = "cap_psp_csr_cnt_pb_pbus_eop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_pb_pbus_eop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_pb_pbus_eop_t
    
class cap_psp_csr_cnt_pb_pbus_sop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_pb_pbus_sop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_pb_pbus_sop_t(string name = "cap_psp_csr_cnt_pb_pbus_sop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_pb_pbus_sop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_pb_pbus_sop_t
    
class cap_psp_csr_cnt_pb_pbus_xns_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_pb_pbus_xns_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_pb_pbus_xns_t(string name = "cap_psp_csr_cnt_pb_pbus_xns_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_pb_pbus_xns_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_pb_pbus_xns_t
    
class cap_psp_csr_cnt_sw_eop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_sw_eop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_sw_eop_t(string name = "cap_psp_csr_cnt_sw_eop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_sw_eop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_sw_eop_t
    
class cap_psp_csr_cnt_sw_sop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_sw_sop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_sw_sop_t(string name = "cap_psp_csr_cnt_sw_sop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_sw_sop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_sw_sop_t
    
class cap_psp_csr_cnt_sw_xns_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cnt_sw_xns_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cnt_sw_xns_t(string name = "cap_psp_csr_cnt_sw_xns_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cnt_sw_xns_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_cnt_sw_xns_t
    
class cap_psp_csr_sat_lif_table_qid_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sat_lif_table_qid_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sat_lif_table_qid_err_t(string name = "cap_psp_csr_sat_lif_table_qid_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sat_lif_table_qid_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_sat_lif_table_qid_err_t
    
class cap_psp_csr_sat_lif_table_ecc_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sat_lif_table_ecc_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sat_lif_table_ecc_err_t(string name = "cap_psp_csr_sat_lif_table_ecc_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sat_lif_table_ecc_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_psp_csr_sat_lif_table_ecc_err_t
    
class cap_psp_csr_cfw_psp_pkt_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cfw_psp_pkt_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cfw_psp_pkt_mem_t(string name = "cap_psp_csr_cfw_psp_pkt_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cfw_psp_pkt_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > offline_cpp_int_t;
        cpp_int int_var__offline;
        void offline (const cpp_int  & l__val);
        cpp_int offline() const;
    
        typedef pu_cpp_int< 1 > flush_cpp_int_t;
        cpp_int int_var__flush;
        void flush (const cpp_int  & l__val);
        cpp_int flush() const;
    
        typedef pu_cpp_int< 4 > rptr_cpp_int_t;
        cpp_int int_var__rptr;
        void rptr (const cpp_int  & l__val);
        cpp_int rptr() const;
    
        typedef pu_cpp_int< 4 > wptr_cpp_int_t;
        cpp_int int_var__wptr;
        void wptr (const cpp_int  & l__val);
        cpp_int wptr() const;
    
}; // cap_psp_csr_cfw_psp_pkt_mem_t
    
class cap_psp_csr_sta_psp_pkt_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sta_psp_pkt_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sta_psp_pkt_mem_t(string name = "cap_psp_csr_sta_psp_pkt_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sta_psp_pkt_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 4 > rptr_cpp_int_t;
        cpp_int int_var__rptr;
        void rptr (const cpp_int  & l__val);
        cpp_int rptr() const;
    
        typedef pu_cpp_int< 4 > wptr_cpp_int_t;
        cpp_int int_var__wptr;
        void wptr (const cpp_int  & l__val);
        cpp_int wptr() const;
    
}; // cap_psp_csr_sta_psp_pkt_mem_t
    
class cap_psp_csr_cfg_psp_pkt_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cfg_psp_pkt_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cfg_psp_pkt_mem_t(string name = "cap_psp_csr_cfg_psp_pkt_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cfg_psp_pkt_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > ecc_bypass_cpp_int_t;
        cpp_int int_var__ecc_bypass;
        void ecc_bypass (const cpp_int  & l__val);
        cpp_int ecc_bypass() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_psp_csr_cfg_psp_pkt_mem_t
    
class cap_psp_csr_cfw_psp_out_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cfw_psp_out_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cfw_psp_out_mem_t(string name = "cap_psp_csr_cfw_psp_out_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cfw_psp_out_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > offline_cpp_int_t;
        cpp_int int_var__offline;
        void offline (const cpp_int  & l__val);
        cpp_int offline() const;
    
        typedef pu_cpp_int< 1 > flush_cpp_int_t;
        cpp_int int_var__flush;
        void flush (const cpp_int  & l__val);
        cpp_int flush() const;
    
        typedef pu_cpp_int< 4 > rptr_cpp_int_t;
        cpp_int int_var__rptr;
        void rptr (const cpp_int  & l__val);
        cpp_int rptr() const;
    
        typedef pu_cpp_int< 4 > wptr_cpp_int_t;
        cpp_int int_var__wptr;
        void wptr (const cpp_int  & l__val);
        cpp_int wptr() const;
    
}; // cap_psp_csr_cfw_psp_out_mem_t
    
class cap_psp_csr_sta_psp_out_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sta_psp_out_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sta_psp_out_mem_t(string name = "cap_psp_csr_sta_psp_out_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sta_psp_out_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 4 > rptr_cpp_int_t;
        cpp_int int_var__rptr;
        void rptr (const cpp_int  & l__val);
        cpp_int rptr() const;
    
        typedef pu_cpp_int< 4 > wptr_cpp_int_t;
        cpp_int int_var__wptr;
        void wptr (const cpp_int  & l__val);
        cpp_int wptr() const;
    
}; // cap_psp_csr_sta_psp_out_mem_t
    
class cap_psp_csr_cfg_psp_out_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cfg_psp_out_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cfg_psp_out_mem_t(string name = "cap_psp_csr_cfg_psp_out_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cfg_psp_out_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > ecc_bypass_cpp_int_t;
        cpp_int int_var__ecc_bypass;
        void ecc_bypass (const cpp_int  & l__val);
        cpp_int ecc_bypass() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_psp_csr_cfg_psp_out_mem_t
    
class cap_psp_csr_cfw_psp_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cfw_psp_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cfw_psp_phv_mem_t(string name = "cap_psp_csr_cfw_psp_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cfw_psp_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > offline_cpp_int_t;
        cpp_int int_var__offline;
        void offline (const cpp_int  & l__val);
        cpp_int offline() const;
    
        typedef pu_cpp_int< 1 > flush_cpp_int_t;
        cpp_int int_var__flush;
        void flush (const cpp_int  & l__val);
        cpp_int flush() const;
    
        typedef pu_cpp_int< 6 > rptr_cpp_int_t;
        cpp_int int_var__rptr;
        void rptr (const cpp_int  & l__val);
        cpp_int rptr() const;
    
        typedef pu_cpp_int< 6 > wptr_cpp_int_t;
        cpp_int int_var__wptr;
        void wptr (const cpp_int  & l__val);
        cpp_int wptr() const;
    
}; // cap_psp_csr_cfw_psp_phv_mem_t
    
class cap_psp_csr_sta_psp_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sta_psp_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sta_psp_phv_mem_t(string name = "cap_psp_csr_sta_psp_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sta_psp_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 6 > rptr_cpp_int_t;
        cpp_int int_var__rptr;
        void rptr (const cpp_int  & l__val);
        cpp_int rptr() const;
    
        typedef pu_cpp_int< 6 > wptr_cpp_int_t;
        cpp_int int_var__wptr;
        void wptr (const cpp_int  & l__val);
        cpp_int wptr() const;
    
}; // cap_psp_csr_sta_psp_phv_mem_t
    
class cap_psp_csr_cfg_psp_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cfg_psp_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cfg_psp_phv_mem_t(string name = "cap_psp_csr_cfg_psp_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cfg_psp_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > ecc_bypass_cpp_int_t;
        cpp_int int_var__ecc_bypass;
        void ecc_bypass (const cpp_int  & l__val);
        cpp_int ecc_bypass() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_psp_csr_cfg_psp_phv_mem_t
    
class cap_psp_csr_cfg_npv_flit_override_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cfg_npv_flit_override_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cfg_npv_flit_override_t(string name = "cap_psp_csr_cfg_npv_flit_override_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cfg_npv_flit_override_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 512 > phv_cpp_int_t;
        cpp_int int_var__phv;
        void phv (const cpp_int  & l__val);
        cpp_int phv() const;
    
}; // cap_psp_csr_cfg_npv_flit_override_t
    
class cap_psp_csr_cfg_npv_flit_values_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cfg_npv_flit_values_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cfg_npv_flit_values_t(string name = "cap_psp_csr_cfg_npv_flit_values_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cfg_npv_flit_values_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 512 > phv_cpp_int_t;
        cpp_int int_var__phv;
        void phv (const cpp_int  & l__val);
        cpp_int phv() const;
    
}; // cap_psp_csr_cfg_npv_flit_values_t
    
class cap_psp_csr_cfg_npv_cos_to_tm_oq_map_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cfg_npv_cos_to_tm_oq_map_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cfg_npv_cos_to_tm_oq_map_t(string name = "cap_psp_csr_cfg_npv_cos_to_tm_oq_map_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cfg_npv_cos_to_tm_oq_map_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 5 > tm_oq_cpp_int_t;
        cpp_int int_var__tm_oq;
        void tm_oq (const cpp_int  & l__val);
        cpp_int tm_oq() const;
    
}; // cap_psp_csr_cfg_npv_cos_to_tm_oq_map_t
    
class cap_psp_csr_cfg_npv_cos_to_tm_iq_map_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cfg_npv_cos_to_tm_iq_map_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cfg_npv_cos_to_tm_iq_map_t(string name = "cap_psp_csr_cfg_npv_cos_to_tm_iq_map_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cfg_npv_cos_to_tm_iq_map_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 5 > tm_iq_cpp_int_t;
        cpp_int int_var__tm_iq;
        void tm_iq (const cpp_int  & l__val);
        cpp_int tm_iq() const;
    
}; // cap_psp_csr_cfg_npv_cos_to_tm_iq_map_t
    
class cap_psp_csr_cfg_npv_values_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cfg_npv_values_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cfg_npv_values_t(string name = "cap_psp_csr_cfg_npv_values_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cfg_npv_values_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > localtime_enable_cpp_int_t;
        cpp_int int_var__localtime_enable;
        void localtime_enable (const cpp_int  & l__val);
        cpp_int localtime_enable() const;
    
        typedef pu_cpp_int< 1 > tm_fields_enable_cpp_int_t;
        cpp_int int_var__tm_fields_enable;
        void tm_fields_enable (const cpp_int  & l__val);
        cpp_int tm_fields_enable() const;
    
        typedef pu_cpp_int< 8 > tm_span_session_cpp_int_t;
        cpp_int int_var__tm_span_session;
        void tm_span_session (const cpp_int  & l__val);
        cpp_int tm_span_session() const;
    
        typedef pu_cpp_int< 1 > tm_cpu_cpp_int_t;
        cpp_int int_var__tm_cpu;
        void tm_cpu (const cpp_int  & l__val);
        cpp_int tm_cpu() const;
    
        typedef pu_cpp_int< 4 > tm_oport_cpp_int_t;
        cpp_int int_var__tm_oport;
        void tm_oport (const cpp_int  & l__val);
        cpp_int tm_oport() const;
    
        typedef pu_cpp_int< 5 > tm_oq_cpp_int_t;
        cpp_int int_var__tm_oq;
        void tm_oq (const cpp_int  & l__val);
        cpp_int tm_oq() const;
    
        typedef pu_cpp_int< 1 > dma_fields_enable_cpp_int_t;
        cpp_int int_var__dma_fields_enable;
        void dma_fields_enable (const cpp_int  & l__val);
        cpp_int dma_fields_enable() const;
    
        typedef pu_cpp_int< 6 > dma_cmd_ptr_cpp_int_t;
        cpp_int int_var__dma_cmd_ptr;
        void dma_cmd_ptr (const cpp_int  & l__val);
        cpp_int dma_cmd_ptr() const;
    
        typedef pu_cpp_int< 1 > debug_trace_cpp_int_t;
        cpp_int int_var__debug_trace;
        void debug_trace (const cpp_int  & l__val);
        cpp_int debug_trace() const;
    
        typedef pu_cpp_int< 1 > tm_iq_map_enable_cpp_int_t;
        cpp_int int_var__tm_iq_map_enable;
        void tm_iq_map_enable (const cpp_int  & l__val);
        cpp_int tm_iq_map_enable() const;
    
        typedef pu_cpp_int< 1 > tm_oq_map_enable_cpp_int_t;
        cpp_int int_var__tm_oq_map_enable;
        void tm_oq_map_enable (const cpp_int  & l__val);
        cpp_int tm_oq_map_enable() const;
    
}; // cap_psp_csr_cfg_npv_values_t
    
class cap_psp_csr_cfg_qstate_map_rsp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cfg_qstate_map_rsp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cfg_qstate_map_rsp_t(string name = "cap_psp_csr_cfg_qstate_map_rsp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cfg_qstate_map_rsp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 1 > qid_enable_cpp_int_t;
        cpp_int int_var__qid_enable;
        void qid_enable (const cpp_int  & l__val);
        cpp_int qid_enable() const;
    
        typedef pu_cpp_int< 1 > qtype_enable_cpp_int_t;
        cpp_int int_var__qtype_enable;
        void qtype_enable (const cpp_int  & l__val);
        cpp_int qtype_enable() const;
    
        typedef pu_cpp_int< 1 > addr_enable_cpp_int_t;
        cpp_int int_var__addr_enable;
        void addr_enable (const cpp_int  & l__val);
        cpp_int addr_enable() const;
    
        typedef pu_cpp_int< 1 > no_data_enable_cpp_int_t;
        cpp_int int_var__no_data_enable;
        void no_data_enable (const cpp_int  & l__val);
        cpp_int no_data_enable() const;
    
        typedef pu_cpp_int< 1 > err_enable_cpp_int_t;
        cpp_int int_var__err_enable;
        void err_enable (const cpp_int  & l__val);
        cpp_int err_enable() const;
    
}; // cap_psp_csr_cfg_qstate_map_rsp_t
    
class cap_psp_csr_cfg_qstate_map_req_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cfg_qstate_map_req_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cfg_qstate_map_req_t(string name = "cap_psp_csr_cfg_qstate_map_req_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cfg_qstate_map_req_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > lif_override_enable_cpp_int_t;
        cpp_int int_var__lif_override_enable;
        void lif_override_enable (const cpp_int  & l__val);
        cpp_int lif_override_enable() const;
    
        typedef pu_cpp_int< 11 > lif_cpp_int_t;
        cpp_int int_var__lif;
        void lif (const cpp_int  & l__val);
        cpp_int lif() const;
    
        typedef pu_cpp_int< 1 > qtype_override_enable_cpp_int_t;
        cpp_int int_var__qtype_override_enable;
        void qtype_override_enable (const cpp_int  & l__val);
        cpp_int qtype_override_enable() const;
    
        typedef pu_cpp_int< 3 > qtype_cpp_int_t;
        cpp_int int_var__qtype;
        void qtype (const cpp_int  & l__val);
        cpp_int qtype() const;
    
        typedef pu_cpp_int< 1 > qid_override_enable_cpp_int_t;
        cpp_int int_var__qid_override_enable;
        void qid_override_enable (const cpp_int  & l__val);
        cpp_int qid_override_enable() const;
    
        typedef pu_cpp_int< 24 > qid_cpp_int_t;
        cpp_int int_var__qid;
        void qid (const cpp_int  & l__val);
        cpp_int qid() const;
    
}; // cap_psp_csr_cfg_qstate_map_req_t
    
class cap_psp_csr_sta_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sta_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sta_fifo_t(string name = "cap_psp_csr_sta_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sta_fifo_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > bus_pkt_empty_cpp_int_t;
        cpp_int int_var__bus_pkt_empty;
        void bus_pkt_empty (const cpp_int  & l__val);
        cpp_int bus_pkt_empty() const;
    
        typedef pu_cpp_int< 1 > sv10_ff_pkt_srdy_cpp_int_t;
        cpp_int int_var__sv10_ff_pkt_srdy;
        void sv10_ff_pkt_srdy (const cpp_int  & l__val);
        cpp_int sv10_ff_pkt_srdy() const;
    
        typedef pu_cpp_int< 1 > sv10_ff_rsp_srdy_cpp_int_t;
        cpp_int int_var__sv10_ff_rsp_srdy;
        void sv10_ff_rsp_srdy (const cpp_int  & l__val);
        cpp_int sv10_ff_rsp_srdy() const;
    
        typedef pu_cpp_int< 1 > sv12_dc_phv_srdy_cpp_int_t;
        cpp_int int_var__sv12_dc_phv_srdy;
        void sv12_dc_phv_srdy (const cpp_int  & l__val);
        cpp_int sv12_dc_phv_srdy() const;
    
        typedef pu_cpp_int< 1 > sv12_dc_pkt_srdy_cpp_int_t;
        cpp_int int_var__sv12_dc_pkt_srdy;
        void sv12_dc_pkt_srdy (const cpp_int  & l__val);
        cpp_int sv12_dc_pkt_srdy() const;
    
        typedef pu_cpp_int< 1 > lpbk_phv_ff_empty_cpp_int_t;
        cpp_int int_var__lpbk_phv_ff_empty;
        void lpbk_phv_ff_empty (const cpp_int  & l__val);
        cpp_int lpbk_phv_ff_empty() const;
    
        typedef pu_cpp_int< 1 > sv13_lb_pkt_srdy_cpp_int_t;
        cpp_int int_var__sv13_lb_pkt_srdy;
        void sv13_lb_pkt_srdy (const cpp_int  & l__val);
        cpp_int sv13_lb_pkt_srdy() const;
    
        typedef pu_cpp_int< 1 > out_phv_empty_cpp_int_t;
        cpp_int int_var__out_phv_empty;
        void out_phv_empty (const cpp_int  & l__val);
        cpp_int out_phv_empty() const;
    
        typedef pu_cpp_int< 1 > sv21_ma_phv_srdy_cpp_int_t;
        cpp_int int_var__sv21_ma_phv_srdy;
        void sv21_ma_phv_srdy (const cpp_int  & l__val);
        cpp_int sv21_ma_phv_srdy() const;
    
        typedef pu_cpp_int< 1 > pkt_order_pop_srdy_cpp_int_t;
        cpp_int int_var__pkt_order_pop_srdy;
        void pkt_order_pop_srdy (const cpp_int  & l__val);
        cpp_int pkt_order_pop_srdy() const;
    
        typedef pu_cpp_int< 1 > sv21_pr_pkt_srdy_cpp_int_t;
        cpp_int int_var__sv21_pr_pkt_srdy;
        void sv21_pr_pkt_srdy (const cpp_int  & l__val);
        cpp_int sv21_pr_pkt_srdy() const;
    
}; // cap_psp_csr_sta_fifo_t
    
class cap_psp_csr_sta_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_sta_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_sta_t(string name = "cap_psp_csr_sta_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_sta_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > empty_cpp_int_t;
        cpp_int int_var__empty;
        void empty (const cpp_int  & l__val);
        cpp_int empty() const;
    
        typedef pu_cpp_int< 1 > pkt_empty_cpp_int_t;
        cpp_int int_var__pkt_empty;
        void pkt_empty (const cpp_int  & l__val);
        cpp_int pkt_empty() const;
    
        typedef pu_cpp_int< 1 > phv_empty_cpp_int_t;
        cpp_int int_var__phv_empty;
        void phv_empty (const cpp_int  & l__val);
        cpp_int phv_empty() const;
    
        typedef pu_cpp_int< 16 > pkt_inflight_cpp_int_t;
        cpp_int int_var__pkt_inflight;
        void pkt_inflight (const cpp_int  & l__val);
        cpp_int pkt_inflight() const;
    
        typedef pu_cpp_int< 16 > phv_inflight_cpp_int_t;
        cpp_int int_var__phv_inflight;
        void phv_inflight (const cpp_int  & l__val);
        cpp_int phv_inflight() const;
    
}; // cap_psp_csr_sta_t
    
class cap_psp_csr_cfg_profile_almost_full_timer_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cfg_profile_almost_full_timer_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cfg_profile_almost_full_timer_t(string name = "cap_psp_csr_cfg_profile_almost_full_timer_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cfg_profile_almost_full_timer_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > max_cpp_int_t;
        cpp_int int_var__max;
        void max (const cpp_int  & l__val);
        cpp_int max() const;
    
}; // cap_psp_csr_cfg_profile_almost_full_timer_t
    
class cap_psp_csr_cfg_profile_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cfg_profile_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cfg_profile_t(string name = "cap_psp_csr_cfg_profile_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cfg_profile_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > ptd_npv_phv_full_enable_cpp_int_t;
        cpp_int int_var__ptd_npv_phv_full_enable;
        void ptd_npv_phv_full_enable (const cpp_int  & l__val);
        cpp_int ptd_npv_phv_full_enable() const;
    
        typedef pu_cpp_int< 3 > recirc_max_cpp_int_t;
        cpp_int int_var__recirc_max;
        void recirc_max (const cpp_int  & l__val);
        cpp_int recirc_max() const;
    
        typedef pu_cpp_int< 1 > recirc_max_enable_cpp_int_t;
        cpp_int int_var__recirc_max_enable;
        void recirc_max_enable (const cpp_int  & l__val);
        cpp_int recirc_max_enable() const;
    
        typedef pu_cpp_int< 1 > recirc_inc_enable_cpp_int_t;
        cpp_int int_var__recirc_inc_enable;
        void recirc_inc_enable (const cpp_int  & l__val);
        cpp_int recirc_inc_enable() const;
    
        typedef pu_cpp_int< 1 > recirc_rst_enable_cpp_int_t;
        cpp_int int_var__recirc_rst_enable;
        void recirc_rst_enable (const cpp_int  & l__val);
        cpp_int recirc_rst_enable() const;
    
        typedef pu_cpp_int< 1 > input_enable_cpp_int_t;
        cpp_int int_var__input_enable;
        void input_enable (const cpp_int  & l__val);
        cpp_int input_enable() const;
    
        typedef pu_cpp_int< 1 > flush_enable_cpp_int_t;
        cpp_int int_var__flush_enable;
        void flush_enable (const cpp_int  & l__val);
        cpp_int flush_enable() const;
    
        typedef pu_cpp_int< 1 > swphv_enable_cpp_int_t;
        cpp_int int_var__swphv_enable;
        void swphv_enable (const cpp_int  & l__val);
        cpp_int swphv_enable() const;
    
        typedef pu_cpp_int< 1 > error_enable_cpp_int_t;
        cpp_int int_var__error_enable;
        void error_enable (const cpp_int  & l__val);
        cpp_int error_enable() const;
    
        typedef pu_cpp_int< 1 > collapse_enable_cpp_int_t;
        cpp_int int_var__collapse_enable;
        void collapse_enable (const cpp_int  & l__val);
        cpp_int collapse_enable() const;
    
        typedef pu_cpp_int< 1 > collapse_onfull_enable_cpp_int_t;
        cpp_int int_var__collapse_onfull_enable;
        void collapse_onfull_enable (const cpp_int  & l__val);
        cpp_int collapse_onfull_enable() const;
    
        typedef pu_cpp_int< 1 > pb_err_enable_cpp_int_t;
        cpp_int int_var__pb_err_enable;
        void pb_err_enable (const cpp_int  & l__val);
        cpp_int pb_err_enable() const;
    
        typedef pu_cpp_int< 1 > ma_err_enable_cpp_int_t;
        cpp_int int_var__ma_err_enable;
        void ma_err_enable (const cpp_int  & l__val);
        cpp_int ma_err_enable() const;
    
        typedef pu_cpp_int< 1 > fsm_err_enable_cpp_int_t;
        cpp_int int_var__fsm_err_enable;
        void fsm_err_enable (const cpp_int  & l__val);
        cpp_int fsm_err_enable() const;
    
        typedef pu_cpp_int< 5 > phv_out_fifo_max_depth_cpp_int_t;
        cpp_int int_var__phv_out_fifo_max_depth;
        void phv_out_fifo_max_depth (const cpp_int  & l__val);
        cpp_int phv_out_fifo_max_depth() const;
    
        typedef pu_cpp_int< 1 > pr_frame_size_enable_cpp_int_t;
        cpp_int int_var__pr_frame_size_enable;
        void pr_frame_size_enable (const cpp_int  & l__val);
        cpp_int pr_frame_size_enable() const;
    
        typedef pu_cpp_int< 1 > dummy_enable_cpp_int_t;
        cpp_int int_var__dummy_enable;
        void dummy_enable (const cpp_int  & l__val);
        cpp_int dummy_enable() const;
    
        typedef pu_cpp_int< 1 > nopkt_enable_cpp_int_t;
        cpp_int int_var__nopkt_enable;
        void nopkt_enable (const cpp_int  & l__val);
        cpp_int nopkt_enable() const;
    
        typedef pu_cpp_int< 6 > phv_min_cpp_int_t;
        cpp_int int_var__phv_min;
        void phv_min (const cpp_int  & l__val);
        cpp_int phv_min() const;
    
        typedef pu_cpp_int< 1 > phv_min_drop_enable_cpp_int_t;
        cpp_int int_var__phv_min_drop_enable;
        void phv_min_drop_enable (const cpp_int  & l__val);
        cpp_int phv_min_drop_enable() const;
    
}; // cap_psp_csr_cfg_profile_t
    
class cap_psp_csr_cfg_debug_port_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_cfg_debug_port_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_cfg_debug_port_t(string name = "cap_psp_csr_cfg_debug_port_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_cfg_debug_port_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 3 > select_cpp_int_t;
        cpp_int int_var__select;
        void select (const cpp_int  & l__val);
        cpp_int select() const;
    
        typedef pu_cpp_int< 1 > elam_enable_cpp_int_t;
        cpp_int int_var__elam_enable;
        void elam_enable (const cpp_int  & l__val);
        cpp_int elam_enable() const;
    
        typedef pu_cpp_int< 1 > elam_pb_pkt_cpp_int_t;
        cpp_int int_var__elam_pb_pkt;
        void elam_pb_pkt (const cpp_int  & l__val);
        cpp_int elam_pb_pkt() const;
    
        typedef pu_cpp_int< 1 > elam_lb_phv_cpp_int_t;
        cpp_int int_var__elam_lb_phv;
        void elam_lb_phv (const cpp_int  & l__val);
        cpp_int elam_lb_phv() const;
    
        typedef pu_cpp_int< 1 > elam_lb_pkt_cpp_int_t;
        cpp_int int_var__elam_lb_pkt;
        void elam_lb_pkt (const cpp_int  & l__val);
        cpp_int elam_lb_pkt() const;
    
        typedef pu_cpp_int< 1 > elam_ma_phv_cpp_int_t;
        cpp_int int_var__elam_ma_phv;
        void elam_ma_phv (const cpp_int  & l__val);
        cpp_int elam_ma_phv() const;
    
        typedef pu_cpp_int< 1 > elam_pr_pkt_cpp_int_t;
        cpp_int int_var__elam_pr_pkt;
        void elam_pr_pkt (const cpp_int  & l__val);
        cpp_int elam_pr_pkt() const;
    
        typedef pu_cpp_int< 1 > elam_trace_cpp_int_t;
        cpp_int int_var__elam_trace;
        void elam_trace (const cpp_int  & l__val);
        cpp_int elam_trace() const;
    
        typedef pu_cpp_int< 1 > fix_error_bits_pb_cpp_int_t;
        cpp_int int_var__fix_error_bits_pb;
        void fix_error_bits_pb (const cpp_int  & l__val);
        cpp_int fix_error_bits_pb() const;
    
        typedef pu_cpp_int< 1 > fix_no_data_pb_cpp_int_t;
        cpp_int int_var__fix_no_data_pb;
        void fix_no_data_pb (const cpp_int  & l__val);
        cpp_int fix_no_data_pb() const;
    
        typedef pu_cpp_int< 1 > fix_no_data_recirc_cpp_int_t;
        cpp_int int_var__fix_no_data_recirc;
        void fix_no_data_recirc (const cpp_int  & l__val);
        cpp_int fix_no_data_recirc() const;
    
        typedef pu_cpp_int< 1 > fix_no_data_int_cpp_int_t;
        cpp_int int_var__fix_no_data_int;
        void fix_no_data_int (const cpp_int  & l__val);
        cpp_int fix_no_data_int() const;
    
        typedef pu_cpp_int< 1 > srst_cpp_int_t;
        cpp_int int_var__srst;
        void srst (const cpp_int  & l__val);
        cpp_int srst() const;
    
}; // cap_psp_csr_cfg_debug_port_t
    
class cap_psp_csr_base_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_base_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_base_t(string name = "cap_psp_csr_base_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_base_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_psp_csr_base_t
    
class cap_psp_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_psp_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_psp_csr_t(string name = "cap_psp_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_psp_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_psp_csr_base_t base;
    
        cap_psp_csr_cfg_debug_port_t cfg_debug_port;
    
        cap_psp_csr_cfg_profile_t cfg_profile;
    
        cap_psp_csr_cfg_profile_almost_full_timer_t cfg_profile_almost_full_timer;
    
        cap_psp_csr_sta_t sta;
    
        cap_psp_csr_sta_fifo_t sta_fifo;
    
        cap_psp_csr_cfg_qstate_map_req_t cfg_qstate_map_req;
    
        cap_psp_csr_cfg_qstate_map_rsp_t cfg_qstate_map_rsp;
    
        cap_psp_csr_cfg_npv_values_t cfg_npv_values;
    
        #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_psp_csr_cfg_npv_cos_to_tm_iq_map_t, 16> cfg_npv_cos_to_tm_iq_map;
        #else 
        cap_psp_csr_cfg_npv_cos_to_tm_iq_map_t cfg_npv_cos_to_tm_iq_map[16];
        #endif
        int get_depth_cfg_npv_cos_to_tm_iq_map() { return 16; }
    
        #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_psp_csr_cfg_npv_cos_to_tm_oq_map_t, 16> cfg_npv_cos_to_tm_oq_map;
        #else 
        cap_psp_csr_cfg_npv_cos_to_tm_oq_map_t cfg_npv_cos_to_tm_oq_map[16];
        #endif
        int get_depth_cfg_npv_cos_to_tm_oq_map() { return 16; }
    
        cap_psp_csr_cfg_npv_flit_values_t cfg_npv_flit_values;
    
        cap_psp_csr_cfg_npv_flit_override_t cfg_npv_flit_override;
    
        cap_psp_csr_cfg_psp_phv_mem_t cfg_psp_phv_mem;
    
        cap_psp_csr_sta_psp_phv_mem_t sta_psp_phv_mem;
    
        cap_psp_csr_cfw_psp_phv_mem_t cfw_psp_phv_mem;
    
        cap_psp_csr_cfg_psp_out_mem_t cfg_psp_out_mem;
    
        cap_psp_csr_sta_psp_out_mem_t sta_psp_out_mem;
    
        cap_psp_csr_cfw_psp_out_mem_t cfw_psp_out_mem;
    
        cap_psp_csr_cfg_psp_pkt_mem_t cfg_psp_pkt_mem;
    
        cap_psp_csr_sta_psp_pkt_mem_t sta_psp_pkt_mem;
    
        cap_psp_csr_cfw_psp_pkt_mem_t cfw_psp_pkt_mem;
    
        cap_psp_csr_sat_lif_table_ecc_err_t sat_lif_table_ecc_err;
    
        cap_psp_csr_sat_lif_table_qid_err_t sat_lif_table_qid_err;
    
        cap_psp_csr_cnt_sw_xns_t cnt_sw_xns;
    
        cap_psp_csr_cnt_sw_sop_t cnt_sw_sop;
    
        cap_psp_csr_cnt_sw_eop_t cnt_sw_eop;
    
        cap_psp_csr_cnt_pb_pbus_xns_t cnt_pb_pbus_xns;
    
        cap_psp_csr_cnt_pb_pbus_sop_t cnt_pb_pbus_sop;
    
        cap_psp_csr_cnt_pb_pbus_eop_t cnt_pb_pbus_eop;
    
        cap_psp_csr_cnt_pb_pbus_no_data_t cnt_pb_pbus_no_data;
    
        cap_psp_csr_sat_pb_pbus_sop_err_t sat_pb_pbus_sop_err;
    
        cap_psp_csr_sat_pb_pbus_eop_err_t sat_pb_pbus_eop_err;
    
        cap_psp_csr_sat_pb_pbus_err_t sat_pb_pbus_err;
    
        cap_psp_csr_sat_pb_pbus_dummy_t sat_pb_pbus_dummy;
    
        cap_psp_csr_sat_pb_pbus_nopkt_t sat_pb_pbus_nopkt;
    
        cap_psp_csr_cnt_ma_xns_t cnt_ma_xns;
    
        cap_psp_csr_cnt_ma_sop_t cnt_ma_sop;
    
        cap_psp_csr_cnt_ma_eop_t cnt_ma_eop;
    
        cap_psp_csr_cnt_ma_recirc_t cnt_ma_recirc;
    
        cap_psp_csr_cnt_ma_no_data_t cnt_ma_no_data;
    
        cap_psp_csr_cnt_ma_collapsed_t cnt_ma_collapsed;
    
        cap_psp_csr_cnt_ma_drop_t cnt_ma_drop;
    
        cap_psp_csr_sat_ma_sop_err_t sat_ma_sop_err;
    
        cap_psp_csr_sat_ma_eop_err_t sat_ma_eop_err;
    
        cap_psp_csr_cnt_pr_pbus_xns_t cnt_pr_pbus_xns;
    
        cap_psp_csr_cnt_pr_pbus_sop_t cnt_pr_pbus_sop;
    
        cap_psp_csr_cnt_pr_pbus_eop_t cnt_pr_pbus_eop;
    
        cap_psp_csr_cnt_pr_pbus_recirc_t cnt_pr_pbus_recirc;
    
        cap_psp_csr_sat_pr_pbus_sop_err_t sat_pr_pbus_sop_err;
    
        cap_psp_csr_sat_pr_pbus_eop_err_t sat_pr_pbus_eop_err;
    
        cap_psp_csr_sat_pr_pbus_err_t sat_pr_pbus_err;
    
        cap_psp_csr_cnt_pr_pkt_ff_almost_full_t cnt_pr_pkt_ff_almost_full;
    
        cap_psp_csr_cnt_pr_resub_pbus_xns_t cnt_pr_resub_pbus_xns;
    
        cap_psp_csr_cnt_pr_resub_pbus_sop_t cnt_pr_resub_pbus_sop;
    
        cap_psp_csr_cnt_pr_resub_pbus_eop_t cnt_pr_resub_pbus_eop;
    
        cap_psp_csr_sat_pr_resub_pbus_sop_err_t sat_pr_resub_pbus_sop_err;
    
        cap_psp_csr_sat_pr_resub_pbus_eop_err_t sat_pr_resub_pbus_eop_err;
    
        cap_psp_csr_sat_pr_resub_pbus_err_t sat_pr_resub_pbus_err;
    
        cap_psp_csr_cnt_pr_resub_xns_t cnt_pr_resub_xns;
    
        cap_psp_csr_cnt_pr_resub_sop_t cnt_pr_resub_sop;
    
        cap_psp_csr_cnt_pr_resub_eop_t cnt_pr_resub_eop;
    
        cap_psp_csr_cnt_pr_resub_no_data_t cnt_pr_resub_no_data;
    
        cap_psp_csr_sat_pr_resub_sop_err_t sat_pr_resub_sop_err;
    
        cap_psp_csr_sat_pr_resub_eop_err_t sat_pr_resub_eop_err;
    
        cap_psp_csr_cnt_psp_csr_read_access_t cnt_psp_csr_read_access;
    
        cap_psp_csr_cnt_psp_csr_write_access_t cnt_psp_csr_write_access;
    
        cap_psp_csr_sat_psp_csr_read_access_err_t sat_psp_csr_read_access_err;
    
        cap_psp_csr_sat_psp_csr_write_access_err_t sat_psp_csr_write_access_err;
    
        cap_psp_csr_cnt_psp_prd_phv_valid_t cnt_psp_prd_phv_valid;
    
        cap_psp_csr_cnt_ptd_npv_phv_full_t cnt_ptd_npv_phv_full;
    
        cap_psp_csr_csr_intr_t csr_intr;
    
        cap_psp_csr_cfg_ecc_disable_lif_qstate_map_t cfg_ecc_disable_lif_qstate_map;
    
        cap_psp_csr_sta_ecc_lif_qstate_map_t sta_ecc_lif_qstate_map;
    
        cap_psp_csr_cfg_lif_table_sram_bist_t cfg_lif_table_sram_bist;
    
        cap_psp_csr_sta_lif_table_sram_bist_t sta_lif_table_sram_bist;
    
        cap_psp_csr_cfg_sw_phv_global_t cfg_sw_phv_global;
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_psp_csr_cfg_sw_phv_control_t, 8> cfg_sw_phv_control;
        #else 
        cap_psp_csr_cfg_sw_phv_control_t cfg_sw_phv_control[8];
        #endif
        int get_depth_cfg_sw_phv_control() { return 8; }
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_psp_csr_cfg_sw_phv_config_t, 8> cfg_sw_phv_config;
        #else 
        cap_psp_csr_cfg_sw_phv_config_t cfg_sw_phv_config[8];
        #endif
        int get_depth_cfg_sw_phv_config() { return 8; }
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_psp_csr_sta_sw_phv_state_t, 8> sta_sw_phv_state;
        #else 
        cap_psp_csr_sta_sw_phv_state_t sta_sw_phv_state[8];
        #endif
        int get_depth_sta_sw_phv_state() { return 8; }
    
        cap_psp_csr_cfg_sw_phv_mem_t cfg_sw_phv_mem;
    
        cap_psp_csr_sta_sw_phv_mem_t sta_sw_phv_mem;
    
        cap_psp_csr_sta_flow_t sta_flow;
    
        cap_psp_csr_sta_fsm_t sta_fsm;
    
        cap_psp_csr_sta_debug_trace_t sta_debug_trace;
    
        cap_psp_csr_sta_debug_trace_pb_pkt_t sta_debug_trace_pb_pkt;
    
        cap_psp_csr_sta_debug_trace_lb_phv_t sta_debug_trace_lb_phv;
    
        cap_psp_csr_sta_debug_trace_lb_pkt_t sta_debug_trace_lb_pkt;
    
        cap_psp_csr_sta_debug_trace_ma_phv_t sta_debug_trace_ma_phv;
    
        cap_psp_csr_sta_debug_trace_pr_pkt_t sta_debug_trace_pr_pkt;
    
        cap_psp_csr_cnt_pb_pbus_no_data_hw_set_t cnt_pb_pbus_no_data_hw_set;
    
        cap_psp_csr_cnt_pb_pbus_no_data_hw_reset_t cnt_pb_pbus_no_data_hw_reset;
    
        cap_psp_csr_cnt_pb_pbus_min_phv_drop_t cnt_pb_pbus_min_phv_drop;
    
        cap_psp_csr_dhs_psp_phv_mem_t dhs_psp_phv_mem;
    
        cap_psp_csr_dhs_psp_out_mem_t dhs_psp_out_mem;
    
        cap_psp_csr_dhs_psp_pkt_mem_t dhs_psp_pkt_mem;
    
        cap_psp_csr_dhs_lif_qstate_map_t dhs_lif_qstate_map;
    
        cap_psp_csr_dhs_sw_phv_mem_t dhs_sw_phv_mem;
    
        cap_psp_csr_dhs_debug_trace_t dhs_debug_trace;
    
        cap_psp_csr_intgrp_status_t int_groups;
    
        cap_psp_csr_int_info_t int_info;
    
        cap_psp_csr_int_err_t int_err;
    
        cap_psp_csr_int_fatal_t int_fatal;
    
        cap_psp_csr_int_lif_qstate_map_t int_lif_qstate_map;
    
        cap_psp_csr_int_sw_phv_mem_t int_sw_phv_mem;
    
}; // cap_psp_csr_t
    
#endif // CAP_PSP_CSR_H
        