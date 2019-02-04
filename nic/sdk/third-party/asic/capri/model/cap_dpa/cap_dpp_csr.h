
#ifndef CAP_DPP_CSR_H
#define CAP_DPP_CSR_H

#include "cap_csr_base.h" 
#include "cap_dppmem_csr.h" 
#include "cap_dppstats_csr.h" 
#include "cap_dppcsum_csr.h" 
#include "cap_dpphdrfld_csr.h" 
#include "cap_dpphdr_csr.h" 

using namespace std;
class cap_dpp_csr_int_spare_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_int_spare_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_int_spare_int_enable_clear_t(string name = "cap_dpp_csr_int_spare_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_int_spare_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > spare_0_enable_cpp_int_t;
        cpp_int int_var__spare_0_enable;
        void spare_0_enable (const cpp_int  & l__val);
        cpp_int spare_0_enable() const;
    
        typedef pu_cpp_int< 1 > spare_1_enable_cpp_int_t;
        cpp_int int_var__spare_1_enable;
        void spare_1_enable (const cpp_int  & l__val);
        cpp_int spare_1_enable() const;
    
        typedef pu_cpp_int< 1 > spare_2_enable_cpp_int_t;
        cpp_int int_var__spare_2_enable;
        void spare_2_enable (const cpp_int  & l__val);
        cpp_int spare_2_enable() const;
    
        typedef pu_cpp_int< 1 > spare_3_enable_cpp_int_t;
        cpp_int int_var__spare_3_enable;
        void spare_3_enable (const cpp_int  & l__val);
        cpp_int spare_3_enable() const;
    
        typedef pu_cpp_int< 1 > spare_4_enable_cpp_int_t;
        cpp_int int_var__spare_4_enable;
        void spare_4_enable (const cpp_int  & l__val);
        cpp_int spare_4_enable() const;
    
        typedef pu_cpp_int< 1 > spare_5_enable_cpp_int_t;
        cpp_int int_var__spare_5_enable;
        void spare_5_enable (const cpp_int  & l__val);
        cpp_int spare_5_enable() const;
    
        typedef pu_cpp_int< 1 > spare_6_enable_cpp_int_t;
        cpp_int int_var__spare_6_enable;
        void spare_6_enable (const cpp_int  & l__val);
        cpp_int spare_6_enable() const;
    
        typedef pu_cpp_int< 1 > spare_7_enable_cpp_int_t;
        cpp_int int_var__spare_7_enable;
        void spare_7_enable (const cpp_int  & l__val);
        cpp_int spare_7_enable() const;
    
        typedef pu_cpp_int< 1 > spare_8_enable_cpp_int_t;
        cpp_int int_var__spare_8_enable;
        void spare_8_enable (const cpp_int  & l__val);
        cpp_int spare_8_enable() const;
    
        typedef pu_cpp_int< 1 > spare_9_enable_cpp_int_t;
        cpp_int int_var__spare_9_enable;
        void spare_9_enable (const cpp_int  & l__val);
        cpp_int spare_9_enable() const;
    
        typedef pu_cpp_int< 1 > spare_10_enable_cpp_int_t;
        cpp_int int_var__spare_10_enable;
        void spare_10_enable (const cpp_int  & l__val);
        cpp_int spare_10_enable() const;
    
        typedef pu_cpp_int< 1 > spare_11_enable_cpp_int_t;
        cpp_int int_var__spare_11_enable;
        void spare_11_enable (const cpp_int  & l__val);
        cpp_int spare_11_enable() const;
    
        typedef pu_cpp_int< 1 > spare_12_enable_cpp_int_t;
        cpp_int int_var__spare_12_enable;
        void spare_12_enable (const cpp_int  & l__val);
        cpp_int spare_12_enable() const;
    
        typedef pu_cpp_int< 1 > spare_13_enable_cpp_int_t;
        cpp_int int_var__spare_13_enable;
        void spare_13_enable (const cpp_int  & l__val);
        cpp_int spare_13_enable() const;
    
        typedef pu_cpp_int< 1 > spare_14_enable_cpp_int_t;
        cpp_int int_var__spare_14_enable;
        void spare_14_enable (const cpp_int  & l__val);
        cpp_int spare_14_enable() const;
    
        typedef pu_cpp_int< 1 > spare_15_enable_cpp_int_t;
        cpp_int int_var__spare_15_enable;
        void spare_15_enable (const cpp_int  & l__val);
        cpp_int spare_15_enable() const;
    
        typedef pu_cpp_int< 1 > spare_16_enable_cpp_int_t;
        cpp_int int_var__spare_16_enable;
        void spare_16_enable (const cpp_int  & l__val);
        cpp_int spare_16_enable() const;
    
        typedef pu_cpp_int< 1 > spare_17_enable_cpp_int_t;
        cpp_int int_var__spare_17_enable;
        void spare_17_enable (const cpp_int  & l__val);
        cpp_int spare_17_enable() const;
    
        typedef pu_cpp_int< 1 > spare_18_enable_cpp_int_t;
        cpp_int int_var__spare_18_enable;
        void spare_18_enable (const cpp_int  & l__val);
        cpp_int spare_18_enable() const;
    
        typedef pu_cpp_int< 1 > spare_19_enable_cpp_int_t;
        cpp_int int_var__spare_19_enable;
        void spare_19_enable (const cpp_int  & l__val);
        cpp_int spare_19_enable() const;
    
        typedef pu_cpp_int< 1 > spare_20_enable_cpp_int_t;
        cpp_int int_var__spare_20_enable;
        void spare_20_enable (const cpp_int  & l__val);
        cpp_int spare_20_enable() const;
    
        typedef pu_cpp_int< 1 > spare_21_enable_cpp_int_t;
        cpp_int int_var__spare_21_enable;
        void spare_21_enable (const cpp_int  & l__val);
        cpp_int spare_21_enable() const;
    
        typedef pu_cpp_int< 1 > spare_22_enable_cpp_int_t;
        cpp_int int_var__spare_22_enable;
        void spare_22_enable (const cpp_int  & l__val);
        cpp_int spare_22_enable() const;
    
        typedef pu_cpp_int< 1 > spare_23_enable_cpp_int_t;
        cpp_int int_var__spare_23_enable;
        void spare_23_enable (const cpp_int  & l__val);
        cpp_int spare_23_enable() const;
    
        typedef pu_cpp_int< 1 > spare_24_enable_cpp_int_t;
        cpp_int int_var__spare_24_enable;
        void spare_24_enable (const cpp_int  & l__val);
        cpp_int spare_24_enable() const;
    
        typedef pu_cpp_int< 1 > spare_25_enable_cpp_int_t;
        cpp_int int_var__spare_25_enable;
        void spare_25_enable (const cpp_int  & l__val);
        cpp_int spare_25_enable() const;
    
        typedef pu_cpp_int< 1 > spare_26_enable_cpp_int_t;
        cpp_int int_var__spare_26_enable;
        void spare_26_enable (const cpp_int  & l__val);
        cpp_int spare_26_enable() const;
    
        typedef pu_cpp_int< 1 > spare_27_enable_cpp_int_t;
        cpp_int int_var__spare_27_enable;
        void spare_27_enable (const cpp_int  & l__val);
        cpp_int spare_27_enable() const;
    
        typedef pu_cpp_int< 1 > spare_28_enable_cpp_int_t;
        cpp_int int_var__spare_28_enable;
        void spare_28_enable (const cpp_int  & l__val);
        cpp_int spare_28_enable() const;
    
        typedef pu_cpp_int< 1 > spare_29_enable_cpp_int_t;
        cpp_int int_var__spare_29_enable;
        void spare_29_enable (const cpp_int  & l__val);
        cpp_int spare_29_enable() const;
    
        typedef pu_cpp_int< 1 > spare_30_enable_cpp_int_t;
        cpp_int int_var__spare_30_enable;
        void spare_30_enable (const cpp_int  & l__val);
        cpp_int spare_30_enable() const;
    
        typedef pu_cpp_int< 1 > spare_31_enable_cpp_int_t;
        cpp_int int_var__spare_31_enable;
        void spare_31_enable (const cpp_int  & l__val);
        cpp_int spare_31_enable() const;
    
}; // cap_dpp_csr_int_spare_int_enable_clear_t
    
class cap_dpp_csr_int_spare_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_int_spare_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_int_spare_int_test_set_t(string name = "cap_dpp_csr_int_spare_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_int_spare_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > spare_0_interrupt_cpp_int_t;
        cpp_int int_var__spare_0_interrupt;
        void spare_0_interrupt (const cpp_int  & l__val);
        cpp_int spare_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > spare_1_interrupt_cpp_int_t;
        cpp_int int_var__spare_1_interrupt;
        void spare_1_interrupt (const cpp_int  & l__val);
        cpp_int spare_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > spare_2_interrupt_cpp_int_t;
        cpp_int int_var__spare_2_interrupt;
        void spare_2_interrupt (const cpp_int  & l__val);
        cpp_int spare_2_interrupt() const;
    
        typedef pu_cpp_int< 1 > spare_3_interrupt_cpp_int_t;
        cpp_int int_var__spare_3_interrupt;
        void spare_3_interrupt (const cpp_int  & l__val);
        cpp_int spare_3_interrupt() const;
    
        typedef pu_cpp_int< 1 > spare_4_interrupt_cpp_int_t;
        cpp_int int_var__spare_4_interrupt;
        void spare_4_interrupt (const cpp_int  & l__val);
        cpp_int spare_4_interrupt() const;
    
        typedef pu_cpp_int< 1 > spare_5_interrupt_cpp_int_t;
        cpp_int int_var__spare_5_interrupt;
        void spare_5_interrupt (const cpp_int  & l__val);
        cpp_int spare_5_interrupt() const;
    
        typedef pu_cpp_int< 1 > spare_6_interrupt_cpp_int_t;
        cpp_int int_var__spare_6_interrupt;
        void spare_6_interrupt (const cpp_int  & l__val);
        cpp_int spare_6_interrupt() const;
    
        typedef pu_cpp_int< 1 > spare_7_interrupt_cpp_int_t;
        cpp_int int_var__spare_7_interrupt;
        void spare_7_interrupt (const cpp_int  & l__val);
        cpp_int spare_7_interrupt() const;
    
        typedef pu_cpp_int< 1 > spare_8_interrupt_cpp_int_t;
        cpp_int int_var__spare_8_interrupt;
        void spare_8_interrupt (const cpp_int  & l__val);
        cpp_int spare_8_interrupt() const;
    
        typedef pu_cpp_int< 1 > spare_9_interrupt_cpp_int_t;
        cpp_int int_var__spare_9_interrupt;
        void spare_9_interrupt (const cpp_int  & l__val);
        cpp_int spare_9_interrupt() const;
    
        typedef pu_cpp_int< 1 > spare_10_interrupt_cpp_int_t;
        cpp_int int_var__spare_10_interrupt;
        void spare_10_interrupt (const cpp_int  & l__val);
        cpp_int spare_10_interrupt() const;
    
        typedef pu_cpp_int< 1 > spare_11_interrupt_cpp_int_t;
        cpp_int int_var__spare_11_interrupt;
        void spare_11_interrupt (const cpp_int  & l__val);
        cpp_int spare_11_interrupt() const;
    
        typedef pu_cpp_int< 1 > spare_12_interrupt_cpp_int_t;
        cpp_int int_var__spare_12_interrupt;
        void spare_12_interrupt (const cpp_int  & l__val);
        cpp_int spare_12_interrupt() const;
    
        typedef pu_cpp_int< 1 > spare_13_interrupt_cpp_int_t;
        cpp_int int_var__spare_13_interrupt;
        void spare_13_interrupt (const cpp_int  & l__val);
        cpp_int spare_13_interrupt() const;
    
        typedef pu_cpp_int< 1 > spare_14_interrupt_cpp_int_t;
        cpp_int int_var__spare_14_interrupt;
        void spare_14_interrupt (const cpp_int  & l__val);
        cpp_int spare_14_interrupt() const;
    
        typedef pu_cpp_int< 1 > spare_15_interrupt_cpp_int_t;
        cpp_int int_var__spare_15_interrupt;
        void spare_15_interrupt (const cpp_int  & l__val);
        cpp_int spare_15_interrupt() const;
    
        typedef pu_cpp_int< 1 > spare_16_interrupt_cpp_int_t;
        cpp_int int_var__spare_16_interrupt;
        void spare_16_interrupt (const cpp_int  & l__val);
        cpp_int spare_16_interrupt() const;
    
        typedef pu_cpp_int< 1 > spare_17_interrupt_cpp_int_t;
        cpp_int int_var__spare_17_interrupt;
        void spare_17_interrupt (const cpp_int  & l__val);
        cpp_int spare_17_interrupt() const;
    
        typedef pu_cpp_int< 1 > spare_18_interrupt_cpp_int_t;
        cpp_int int_var__spare_18_interrupt;
        void spare_18_interrupt (const cpp_int  & l__val);
        cpp_int spare_18_interrupt() const;
    
        typedef pu_cpp_int< 1 > spare_19_interrupt_cpp_int_t;
        cpp_int int_var__spare_19_interrupt;
        void spare_19_interrupt (const cpp_int  & l__val);
        cpp_int spare_19_interrupt() const;
    
        typedef pu_cpp_int< 1 > spare_20_interrupt_cpp_int_t;
        cpp_int int_var__spare_20_interrupt;
        void spare_20_interrupt (const cpp_int  & l__val);
        cpp_int spare_20_interrupt() const;
    
        typedef pu_cpp_int< 1 > spare_21_interrupt_cpp_int_t;
        cpp_int int_var__spare_21_interrupt;
        void spare_21_interrupt (const cpp_int  & l__val);
        cpp_int spare_21_interrupt() const;
    
        typedef pu_cpp_int< 1 > spare_22_interrupt_cpp_int_t;
        cpp_int int_var__spare_22_interrupt;
        void spare_22_interrupt (const cpp_int  & l__val);
        cpp_int spare_22_interrupt() const;
    
        typedef pu_cpp_int< 1 > spare_23_interrupt_cpp_int_t;
        cpp_int int_var__spare_23_interrupt;
        void spare_23_interrupt (const cpp_int  & l__val);
        cpp_int spare_23_interrupt() const;
    
        typedef pu_cpp_int< 1 > spare_24_interrupt_cpp_int_t;
        cpp_int int_var__spare_24_interrupt;
        void spare_24_interrupt (const cpp_int  & l__val);
        cpp_int spare_24_interrupt() const;
    
        typedef pu_cpp_int< 1 > spare_25_interrupt_cpp_int_t;
        cpp_int int_var__spare_25_interrupt;
        void spare_25_interrupt (const cpp_int  & l__val);
        cpp_int spare_25_interrupt() const;
    
        typedef pu_cpp_int< 1 > spare_26_interrupt_cpp_int_t;
        cpp_int int_var__spare_26_interrupt;
        void spare_26_interrupt (const cpp_int  & l__val);
        cpp_int spare_26_interrupt() const;
    
        typedef pu_cpp_int< 1 > spare_27_interrupt_cpp_int_t;
        cpp_int int_var__spare_27_interrupt;
        void spare_27_interrupt (const cpp_int  & l__val);
        cpp_int spare_27_interrupt() const;
    
        typedef pu_cpp_int< 1 > spare_28_interrupt_cpp_int_t;
        cpp_int int_var__spare_28_interrupt;
        void spare_28_interrupt (const cpp_int  & l__val);
        cpp_int spare_28_interrupt() const;
    
        typedef pu_cpp_int< 1 > spare_29_interrupt_cpp_int_t;
        cpp_int int_var__spare_29_interrupt;
        void spare_29_interrupt (const cpp_int  & l__val);
        cpp_int spare_29_interrupt() const;
    
        typedef pu_cpp_int< 1 > spare_30_interrupt_cpp_int_t;
        cpp_int int_var__spare_30_interrupt;
        void spare_30_interrupt (const cpp_int  & l__val);
        cpp_int spare_30_interrupt() const;
    
        typedef pu_cpp_int< 1 > spare_31_interrupt_cpp_int_t;
        cpp_int int_var__spare_31_interrupt;
        void spare_31_interrupt (const cpp_int  & l__val);
        cpp_int spare_31_interrupt() const;
    
}; // cap_dpp_csr_int_spare_int_test_set_t
    
class cap_dpp_csr_int_spare_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_int_spare_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_int_spare_t(string name = "cap_dpp_csr_int_spare_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_int_spare_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_dpp_csr_int_spare_int_test_set_t intreg;
    
        cap_dpp_csr_int_spare_int_test_set_t int_test_set;
    
        cap_dpp_csr_int_spare_int_enable_clear_t int_enable_set;
    
        cap_dpp_csr_int_spare_int_enable_clear_t int_enable_clear;
    
}; // cap_dpp_csr_int_spare_t
    
class cap_dpp_csr_int_credit_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_int_credit_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_int_credit_int_enable_clear_t(string name = "cap_dpp_csr_int_credit_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_int_credit_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > ptr_credit_ovflow_enable_cpp_int_t;
        cpp_int int_var__ptr_credit_ovflow_enable;
        void ptr_credit_ovflow_enable (const cpp_int  & l__val);
        cpp_int ptr_credit_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > ptr_credit_undflow_enable_cpp_int_t;
        cpp_int int_var__ptr_credit_undflow_enable;
        void ptr_credit_undflow_enable (const cpp_int  & l__val);
        cpp_int ptr_credit_undflow_enable() const;
    
        typedef pu_cpp_int< 1 > pkt_credit_ovflow_enable_cpp_int_t;
        cpp_int int_var__pkt_credit_ovflow_enable;
        void pkt_credit_ovflow_enable (const cpp_int  & l__val);
        cpp_int pkt_credit_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pkt_credit_undflow_enable_cpp_int_t;
        cpp_int int_var__pkt_credit_undflow_enable;
        void pkt_credit_undflow_enable (const cpp_int  & l__val);
        cpp_int pkt_credit_undflow_enable() const;
    
        typedef pu_cpp_int< 1 > framer_credit_ovflow_enable_cpp_int_t;
        cpp_int int_var__framer_credit_ovflow_enable;
        void framer_credit_ovflow_enable (const cpp_int  & l__val);
        cpp_int framer_credit_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > framer_credit_undflow_enable_cpp_int_t;
        cpp_int int_var__framer_credit_undflow_enable;
        void framer_credit_undflow_enable (const cpp_int  & l__val);
        cpp_int framer_credit_undflow_enable() const;
    
        typedef pu_cpp_int< 1 > framer_hdrfld_vld_ovfl_enable_cpp_int_t;
        cpp_int int_var__framer_hdrfld_vld_ovfl_enable;
        void framer_hdrfld_vld_ovfl_enable (const cpp_int  & l__val);
        cpp_int framer_hdrfld_vld_ovfl_enable() const;
    
        typedef pu_cpp_int< 1 > framer_hdrfld_offset_ovfl_enable_cpp_int_t;
        cpp_int int_var__framer_hdrfld_offset_ovfl_enable;
        void framer_hdrfld_offset_ovfl_enable (const cpp_int  & l__val);
        cpp_int framer_hdrfld_offset_ovfl_enable() const;
    
        typedef pu_cpp_int< 1 > err_framer_hdrsize_zero_ovfl_enable_cpp_int_t;
        cpp_int int_var__err_framer_hdrsize_zero_ovfl_enable;
        void err_framer_hdrsize_zero_ovfl_enable (const cpp_int  & l__val);
        cpp_int err_framer_hdrsize_zero_ovfl_enable() const;
    
}; // cap_dpp_csr_int_credit_int_enable_clear_t
    
class cap_dpp_csr_int_credit_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_int_credit_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_int_credit_int_test_set_t(string name = "cap_dpp_csr_int_credit_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_int_credit_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > ptr_credit_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__ptr_credit_ovflow_interrupt;
        void ptr_credit_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int ptr_credit_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > ptr_credit_undflow_interrupt_cpp_int_t;
        cpp_int int_var__ptr_credit_undflow_interrupt;
        void ptr_credit_undflow_interrupt (const cpp_int  & l__val);
        cpp_int ptr_credit_undflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pkt_credit_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pkt_credit_ovflow_interrupt;
        void pkt_credit_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pkt_credit_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pkt_credit_undflow_interrupt_cpp_int_t;
        cpp_int int_var__pkt_credit_undflow_interrupt;
        void pkt_credit_undflow_interrupt (const cpp_int  & l__val);
        cpp_int pkt_credit_undflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > framer_credit_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__framer_credit_ovflow_interrupt;
        void framer_credit_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int framer_credit_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > framer_credit_undflow_interrupt_cpp_int_t;
        cpp_int int_var__framer_credit_undflow_interrupt;
        void framer_credit_undflow_interrupt (const cpp_int  & l__val);
        cpp_int framer_credit_undflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > framer_hdrfld_vld_ovfl_interrupt_cpp_int_t;
        cpp_int int_var__framer_hdrfld_vld_ovfl_interrupt;
        void framer_hdrfld_vld_ovfl_interrupt (const cpp_int  & l__val);
        cpp_int framer_hdrfld_vld_ovfl_interrupt() const;
    
        typedef pu_cpp_int< 1 > framer_hdrfld_offset_ovfl_interrupt_cpp_int_t;
        cpp_int int_var__framer_hdrfld_offset_ovfl_interrupt;
        void framer_hdrfld_offset_ovfl_interrupt (const cpp_int  & l__val);
        cpp_int framer_hdrfld_offset_ovfl_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_framer_hdrsize_zero_ovfl_interrupt_cpp_int_t;
        cpp_int int_var__err_framer_hdrsize_zero_ovfl_interrupt;
        void err_framer_hdrsize_zero_ovfl_interrupt (const cpp_int  & l__val);
        cpp_int err_framer_hdrsize_zero_ovfl_interrupt() const;
    
}; // cap_dpp_csr_int_credit_int_test_set_t
    
class cap_dpp_csr_int_credit_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_int_credit_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_int_credit_t(string name = "cap_dpp_csr_int_credit_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_int_credit_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_dpp_csr_int_credit_int_test_set_t intreg;
    
        cap_dpp_csr_int_credit_int_test_set_t int_test_set;
    
        cap_dpp_csr_int_credit_int_enable_clear_t int_enable_set;
    
        cap_dpp_csr_int_credit_int_enable_clear_t int_enable_clear;
    
}; // cap_dpp_csr_int_credit_t
    
class cap_dpp_csr_int_fifo_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_int_fifo_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_int_fifo_int_enable_clear_t(string name = "cap_dpp_csr_int_fifo_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_int_fifo_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > phv_ff_overflow_enable_cpp_int_t;
        cpp_int int_var__phv_ff_overflow_enable;
        void phv_ff_overflow_enable (const cpp_int  & l__val);
        cpp_int phv_ff_overflow_enable() const;
    
        typedef pu_cpp_int< 1 > ohi_ff_overflow_enable_cpp_int_t;
        cpp_int int_var__ohi_ff_overflow_enable;
        void ohi_ff_overflow_enable (const cpp_int  & l__val);
        cpp_int ohi_ff_overflow_enable() const;
    
        typedef pu_cpp_int< 1 > pkt_size_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pkt_size_ff_ovflow_enable;
        void pkt_size_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pkt_size_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pkt_size_ff_undflow_enable_cpp_int_t;
        cpp_int int_var__pkt_size_ff_undflow_enable;
        void pkt_size_ff_undflow_enable (const cpp_int  & l__val);
        cpp_int pkt_size_ff_undflow_enable() const;
    
        typedef pu_cpp_int< 1 > csum_phv_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__csum_phv_ff_ovflow_enable;
        void csum_phv_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int csum_phv_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > csum_phv_ff_undflow_enable_cpp_int_t;
        cpp_int int_var__csum_phv_ff_undflow_enable;
        void csum_phv_ff_undflow_enable (const cpp_int  & l__val);
        cpp_int csum_phv_ff_undflow_enable() const;
    
}; // cap_dpp_csr_int_fifo_int_enable_clear_t
    
class cap_dpp_csr_int_fifo_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_int_fifo_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_int_fifo_int_test_set_t(string name = "cap_dpp_csr_int_fifo_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_int_fifo_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > phv_ff_overflow_interrupt_cpp_int_t;
        cpp_int int_var__phv_ff_overflow_interrupt;
        void phv_ff_overflow_interrupt (const cpp_int  & l__val);
        cpp_int phv_ff_overflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > ohi_ff_overflow_interrupt_cpp_int_t;
        cpp_int int_var__ohi_ff_overflow_interrupt;
        void ohi_ff_overflow_interrupt (const cpp_int  & l__val);
        cpp_int ohi_ff_overflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pkt_size_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pkt_size_ff_ovflow_interrupt;
        void pkt_size_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pkt_size_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pkt_size_ff_undflow_interrupt_cpp_int_t;
        cpp_int int_var__pkt_size_ff_undflow_interrupt;
        void pkt_size_ff_undflow_interrupt (const cpp_int  & l__val);
        cpp_int pkt_size_ff_undflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > csum_phv_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__csum_phv_ff_ovflow_interrupt;
        void csum_phv_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int csum_phv_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > csum_phv_ff_undflow_interrupt_cpp_int_t;
        cpp_int int_var__csum_phv_ff_undflow_interrupt;
        void csum_phv_ff_undflow_interrupt (const cpp_int  & l__val);
        cpp_int csum_phv_ff_undflow_interrupt() const;
    
}; // cap_dpp_csr_int_fifo_int_test_set_t
    
class cap_dpp_csr_int_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_int_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_int_fifo_t(string name = "cap_dpp_csr_int_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_int_fifo_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_dpp_csr_int_fifo_int_test_set_t intreg;
    
        cap_dpp_csr_int_fifo_int_test_set_t int_test_set;
    
        cap_dpp_csr_int_fifo_int_enable_clear_t int_enable_set;
    
        cap_dpp_csr_int_fifo_int_enable_clear_t int_enable_clear;
    
}; // cap_dpp_csr_int_fifo_t
    
class cap_dpp_csr_int_reg2_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_int_reg2_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_int_reg2_int_enable_clear_t(string name = "cap_dpp_csr_int_reg2_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_int_reg2_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > fieldC_enable_cpp_int_t;
        cpp_int int_var__fieldC_enable;
        void fieldC_enable (const cpp_int  & l__val);
        cpp_int fieldC_enable() const;
    
        typedef pu_cpp_int< 1 > fieldD_enable_cpp_int_t;
        cpp_int int_var__fieldD_enable;
        void fieldD_enable (const cpp_int  & l__val);
        cpp_int fieldD_enable() const;
    
}; // cap_dpp_csr_int_reg2_int_enable_clear_t
    
class cap_dpp_csr_int_reg2_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_int_reg2_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_int_reg2_int_test_set_t(string name = "cap_dpp_csr_int_reg2_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_int_reg2_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > fieldC_interrupt_cpp_int_t;
        cpp_int int_var__fieldC_interrupt;
        void fieldC_interrupt (const cpp_int  & l__val);
        cpp_int fieldC_interrupt() const;
    
        typedef pu_cpp_int< 1 > fieldD_interrupt_cpp_int_t;
        cpp_int int_var__fieldD_interrupt;
        void fieldD_interrupt (const cpp_int  & l__val);
        cpp_int fieldD_interrupt() const;
    
}; // cap_dpp_csr_int_reg2_int_test_set_t
    
class cap_dpp_csr_int_reg2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_int_reg2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_int_reg2_t(string name = "cap_dpp_csr_int_reg2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_int_reg2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_dpp_csr_int_reg2_int_test_set_t intreg;
    
        cap_dpp_csr_int_reg2_int_test_set_t int_test_set;
    
        cap_dpp_csr_int_reg2_int_enable_clear_t int_enable_set;
    
        cap_dpp_csr_int_reg2_int_enable_clear_t int_enable_clear;
    
}; // cap_dpp_csr_int_reg2_t
    
class cap_dpp_csr_int_reg1_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_int_reg1_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_int_reg1_int_enable_clear_t(string name = "cap_dpp_csr_int_reg1_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_int_reg1_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > err_phv_sop_no_eop_enable_cpp_int_t;
        cpp_int int_var__err_phv_sop_no_eop_enable;
        void err_phv_sop_no_eop_enable (const cpp_int  & l__val);
        cpp_int err_phv_sop_no_eop_enable() const;
    
        typedef pu_cpp_int< 1 > err_phv_eop_no_sop_enable_cpp_int_t;
        cpp_int int_var__err_phv_eop_no_sop_enable;
        void err_phv_eop_no_sop_enable (const cpp_int  & l__val);
        cpp_int err_phv_eop_no_sop_enable() const;
    
        typedef pu_cpp_int< 1 > err_ohi_sop_no_eop_enable_cpp_int_t;
        cpp_int int_var__err_ohi_sop_no_eop_enable;
        void err_ohi_sop_no_eop_enable (const cpp_int  & l__val);
        cpp_int err_ohi_sop_no_eop_enable() const;
    
        typedef pu_cpp_int< 1 > err_ohi_eop_no_sop_enable_cpp_int_t;
        cpp_int int_var__err_ohi_eop_no_sop_enable;
        void err_ohi_eop_no_sop_enable (const cpp_int  & l__val);
        cpp_int err_ohi_eop_no_sop_enable() const;
    
        typedef pu_cpp_int< 1 > err_framer_credit_overrun_enable_cpp_int_t;
        cpp_int int_var__err_framer_credit_overrun_enable;
        void err_framer_credit_overrun_enable (const cpp_int  & l__val);
        cpp_int err_framer_credit_overrun_enable() const;
    
        typedef pu_cpp_int< 1 > err_packets_in_flight_credit_overrun_enable_cpp_int_t;
        cpp_int int_var__err_packets_in_flight_credit_overrun_enable;
        void err_packets_in_flight_credit_overrun_enable (const cpp_int  & l__val);
        cpp_int err_packets_in_flight_credit_overrun_enable() const;
    
        typedef pu_cpp_int< 1 > err_null_hdr_vld_enable_cpp_int_t;
        cpp_int int_var__err_null_hdr_vld_enable;
        void err_null_hdr_vld_enable (const cpp_int  & l__val);
        cpp_int err_null_hdr_vld_enable() const;
    
        typedef pu_cpp_int< 1 > err_null_hdrfld_vld_enable_cpp_int_t;
        cpp_int int_var__err_null_hdrfld_vld_enable;
        void err_null_hdrfld_vld_enable (const cpp_int  & l__val);
        cpp_int err_null_hdrfld_vld_enable() const;
    
        typedef pu_cpp_int< 1 > err_max_pkt_size_enable_cpp_int_t;
        cpp_int int_var__err_max_pkt_size_enable;
        void err_max_pkt_size_enable (const cpp_int  & l__val);
        cpp_int err_max_pkt_size_enable() const;
    
        typedef pu_cpp_int< 1 > err_max_active_hdrs_enable_cpp_int_t;
        cpp_int int_var__err_max_active_hdrs_enable;
        void err_max_active_hdrs_enable (const cpp_int  & l__val);
        cpp_int err_max_active_hdrs_enable() const;
    
        typedef pu_cpp_int< 1 > err_phv_no_data_reference_ohi_enable_cpp_int_t;
        cpp_int int_var__err_phv_no_data_reference_ohi_enable;
        void err_phv_no_data_reference_ohi_enable (const cpp_int  & l__val);
        cpp_int err_phv_no_data_reference_ohi_enable() const;
    
        typedef pu_cpp_int< 1 > err_csum_multiple_hdr_enable_cpp_int_t;
        cpp_int int_var__err_csum_multiple_hdr_enable;
        void err_csum_multiple_hdr_enable (const cpp_int  & l__val);
        cpp_int err_csum_multiple_hdr_enable() const;
    
        typedef pu_cpp_int< 1 > err_csum_multiple_hdr_copy_enable_cpp_int_t;
        cpp_int int_var__err_csum_multiple_hdr_copy_enable;
        void err_csum_multiple_hdr_copy_enable (const cpp_int  & l__val);
        cpp_int err_csum_multiple_hdr_copy_enable() const;
    
        typedef pu_cpp_int< 1 > err_crc_multiple_hdr_enable_cpp_int_t;
        cpp_int int_var__err_crc_multiple_hdr_enable;
        void err_crc_multiple_hdr_enable (const cpp_int  & l__val);
        cpp_int err_crc_multiple_hdr_enable() const;
    
        typedef pu_cpp_int< 1 > err_ptr_fifo_credit_overrun_enable_cpp_int_t;
        cpp_int int_var__err_ptr_fifo_credit_overrun_enable;
        void err_ptr_fifo_credit_overrun_enable (const cpp_int  & l__val);
        cpp_int err_ptr_fifo_credit_overrun_enable() const;
    
        typedef pu_cpp_int< 1 > err_clip_max_pkt_size_enable_cpp_int_t;
        cpp_int int_var__err_clip_max_pkt_size_enable;
        void err_clip_max_pkt_size_enable (const cpp_int  & l__val);
        cpp_int err_clip_max_pkt_size_enable() const;
    
        typedef pu_cpp_int< 1 > err_min_pkt_size_enable_cpp_int_t;
        cpp_int int_var__err_min_pkt_size_enable;
        void err_min_pkt_size_enable (const cpp_int  & l__val);
        cpp_int err_min_pkt_size_enable() const;
    
}; // cap_dpp_csr_int_reg1_int_enable_clear_t
    
class cap_dpp_csr_int_reg1_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_int_reg1_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_int_reg1_int_test_set_t(string name = "cap_dpp_csr_int_reg1_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_int_reg1_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > err_phv_sop_no_eop_interrupt_cpp_int_t;
        cpp_int int_var__err_phv_sop_no_eop_interrupt;
        void err_phv_sop_no_eop_interrupt (const cpp_int  & l__val);
        cpp_int err_phv_sop_no_eop_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_phv_eop_no_sop_interrupt_cpp_int_t;
        cpp_int int_var__err_phv_eop_no_sop_interrupt;
        void err_phv_eop_no_sop_interrupt (const cpp_int  & l__val);
        cpp_int err_phv_eop_no_sop_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_ohi_sop_no_eop_interrupt_cpp_int_t;
        cpp_int int_var__err_ohi_sop_no_eop_interrupt;
        void err_ohi_sop_no_eop_interrupt (const cpp_int  & l__val);
        cpp_int err_ohi_sop_no_eop_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_ohi_eop_no_sop_interrupt_cpp_int_t;
        cpp_int int_var__err_ohi_eop_no_sop_interrupt;
        void err_ohi_eop_no_sop_interrupt (const cpp_int  & l__val);
        cpp_int err_ohi_eop_no_sop_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_framer_credit_overrun_interrupt_cpp_int_t;
        cpp_int int_var__err_framer_credit_overrun_interrupt;
        void err_framer_credit_overrun_interrupt (const cpp_int  & l__val);
        cpp_int err_framer_credit_overrun_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_packets_in_flight_credit_overrun_interrupt_cpp_int_t;
        cpp_int int_var__err_packets_in_flight_credit_overrun_interrupt;
        void err_packets_in_flight_credit_overrun_interrupt (const cpp_int  & l__val);
        cpp_int err_packets_in_flight_credit_overrun_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_null_hdr_vld_interrupt_cpp_int_t;
        cpp_int int_var__err_null_hdr_vld_interrupt;
        void err_null_hdr_vld_interrupt (const cpp_int  & l__val);
        cpp_int err_null_hdr_vld_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_null_hdrfld_vld_interrupt_cpp_int_t;
        cpp_int int_var__err_null_hdrfld_vld_interrupt;
        void err_null_hdrfld_vld_interrupt (const cpp_int  & l__val);
        cpp_int err_null_hdrfld_vld_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_max_pkt_size_interrupt_cpp_int_t;
        cpp_int int_var__err_max_pkt_size_interrupt;
        void err_max_pkt_size_interrupt (const cpp_int  & l__val);
        cpp_int err_max_pkt_size_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_max_active_hdrs_interrupt_cpp_int_t;
        cpp_int int_var__err_max_active_hdrs_interrupt;
        void err_max_active_hdrs_interrupt (const cpp_int  & l__val);
        cpp_int err_max_active_hdrs_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_phv_no_data_reference_ohi_interrupt_cpp_int_t;
        cpp_int int_var__err_phv_no_data_reference_ohi_interrupt;
        void err_phv_no_data_reference_ohi_interrupt (const cpp_int  & l__val);
        cpp_int err_phv_no_data_reference_ohi_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_csum_multiple_hdr_interrupt_cpp_int_t;
        cpp_int int_var__err_csum_multiple_hdr_interrupt;
        void err_csum_multiple_hdr_interrupt (const cpp_int  & l__val);
        cpp_int err_csum_multiple_hdr_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_csum_multiple_hdr_copy_interrupt_cpp_int_t;
        cpp_int int_var__err_csum_multiple_hdr_copy_interrupt;
        void err_csum_multiple_hdr_copy_interrupt (const cpp_int  & l__val);
        cpp_int err_csum_multiple_hdr_copy_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_crc_multiple_hdr_interrupt_cpp_int_t;
        cpp_int int_var__err_crc_multiple_hdr_interrupt;
        void err_crc_multiple_hdr_interrupt (const cpp_int  & l__val);
        cpp_int err_crc_multiple_hdr_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_ptr_fifo_credit_overrun_interrupt_cpp_int_t;
        cpp_int int_var__err_ptr_fifo_credit_overrun_interrupt;
        void err_ptr_fifo_credit_overrun_interrupt (const cpp_int  & l__val);
        cpp_int err_ptr_fifo_credit_overrun_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_clip_max_pkt_size_interrupt_cpp_int_t;
        cpp_int int_var__err_clip_max_pkt_size_interrupt;
        void err_clip_max_pkt_size_interrupt (const cpp_int  & l__val);
        cpp_int err_clip_max_pkt_size_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_min_pkt_size_interrupt_cpp_int_t;
        cpp_int int_var__err_min_pkt_size_interrupt;
        void err_min_pkt_size_interrupt (const cpp_int  & l__val);
        cpp_int err_min_pkt_size_interrupt() const;
    
}; // cap_dpp_csr_int_reg1_int_test_set_t
    
class cap_dpp_csr_int_reg1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_int_reg1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_int_reg1_t(string name = "cap_dpp_csr_int_reg1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_int_reg1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_dpp_csr_int_reg1_int_test_set_t intreg;
    
        cap_dpp_csr_int_reg1_int_test_set_t int_test_set;
    
        cap_dpp_csr_int_reg1_int_enable_clear_t int_enable_set;
    
        cap_dpp_csr_int_reg1_int_enable_clear_t int_enable_clear;
    
}; // cap_dpp_csr_int_reg1_t
    
class cap_dpp_csr_intreg_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_intreg_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_intreg_status_t(string name = "cap_dpp_csr_intreg_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_intreg_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > int_reg1_interrupt_cpp_int_t;
        cpp_int int_var__int_reg1_interrupt;
        void int_reg1_interrupt (const cpp_int  & l__val);
        cpp_int int_reg1_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_reg2_interrupt_cpp_int_t;
        cpp_int int_var__int_reg2_interrupt;
        void int_reg2_interrupt (const cpp_int  & l__val);
        cpp_int int_reg2_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_srams_ecc_interrupt_cpp_int_t;
        cpp_int int_var__int_srams_ecc_interrupt;
        void int_srams_ecc_interrupt (const cpp_int  & l__val);
        cpp_int int_srams_ecc_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_fifo_interrupt_cpp_int_t;
        cpp_int int_var__int_fifo_interrupt;
        void int_fifo_interrupt (const cpp_int  & l__val);
        cpp_int int_fifo_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_spare_interrupt_cpp_int_t;
        cpp_int int_var__int_spare_interrupt;
        void int_spare_interrupt (const cpp_int  & l__val);
        cpp_int int_spare_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_credit_interrupt_cpp_int_t;
        cpp_int int_var__int_credit_interrupt;
        void int_credit_interrupt (const cpp_int  & l__val);
        cpp_int int_credit_interrupt() const;
    
}; // cap_dpp_csr_intreg_status_t
    
class cap_dpp_csr_int_groups_int_enable_rw_reg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_int_groups_int_enable_rw_reg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_int_groups_int_enable_rw_reg_t(string name = "cap_dpp_csr_int_groups_int_enable_rw_reg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_int_groups_int_enable_rw_reg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > int_reg1_enable_cpp_int_t;
        cpp_int int_var__int_reg1_enable;
        void int_reg1_enable (const cpp_int  & l__val);
        cpp_int int_reg1_enable() const;
    
        typedef pu_cpp_int< 1 > int_reg2_enable_cpp_int_t;
        cpp_int int_var__int_reg2_enable;
        void int_reg2_enable (const cpp_int  & l__val);
        cpp_int int_reg2_enable() const;
    
        typedef pu_cpp_int< 1 > int_srams_ecc_enable_cpp_int_t;
        cpp_int int_var__int_srams_ecc_enable;
        void int_srams_ecc_enable (const cpp_int  & l__val);
        cpp_int int_srams_ecc_enable() const;
    
        typedef pu_cpp_int< 1 > int_fifo_enable_cpp_int_t;
        cpp_int int_var__int_fifo_enable;
        void int_fifo_enable (const cpp_int  & l__val);
        cpp_int int_fifo_enable() const;
    
        typedef pu_cpp_int< 1 > int_spare_enable_cpp_int_t;
        cpp_int int_var__int_spare_enable;
        void int_spare_enable (const cpp_int  & l__val);
        cpp_int int_spare_enable() const;
    
        typedef pu_cpp_int< 1 > int_credit_enable_cpp_int_t;
        cpp_int int_var__int_credit_enable;
        void int_credit_enable (const cpp_int  & l__val);
        cpp_int int_credit_enable() const;
    
}; // cap_dpp_csr_int_groups_int_enable_rw_reg_t
    
class cap_dpp_csr_intgrp_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_intgrp_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_intgrp_status_t(string name = "cap_dpp_csr_intgrp_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_intgrp_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_dpp_csr_intreg_status_t intreg;
    
        cap_dpp_csr_int_groups_int_enable_rw_reg_t int_enable_rw_reg;
    
        cap_dpp_csr_intreg_status_t int_rw_reg;
    
}; // cap_dpp_csr_intgrp_status_t
    
class cap_dpp_csr_int_srams_ecc_int_enable_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_int_srams_ecc_int_enable_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_int_srams_ecc_int_enable_set_t(string name = "cap_dpp_csr_int_srams_ecc_int_enable_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_int_srams_ecc_int_enable_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > dpp_phv_fifo_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__dpp_phv_fifo_uncorrectable_enable;
        void dpp_phv_fifo_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int dpp_phv_fifo_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > dpp_phv_fifo_correctable_enable_cpp_int_t;
        cpp_int int_var__dpp_phv_fifo_correctable_enable;
        void dpp_phv_fifo_correctable_enable (const cpp_int  & l__val);
        cpp_int dpp_phv_fifo_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > dpp_ohi_fifo_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__dpp_ohi_fifo_uncorrectable_enable;
        void dpp_ohi_fifo_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int dpp_ohi_fifo_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > dpp_ohi_fifo_correctable_enable_cpp_int_t;
        cpp_int int_var__dpp_ohi_fifo_correctable_enable;
        void dpp_ohi_fifo_correctable_enable (const cpp_int  & l__val);
        cpp_int dpp_ohi_fifo_correctable_enable() const;
    
}; // cap_dpp_csr_int_srams_ecc_int_enable_set_t
    
class cap_dpp_csr_int_srams_ecc_intreg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_int_srams_ecc_intreg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_int_srams_ecc_intreg_t(string name = "cap_dpp_csr_int_srams_ecc_intreg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_int_srams_ecc_intreg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > dpp_phv_fifo_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__dpp_phv_fifo_uncorrectable_interrupt;
        void dpp_phv_fifo_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int dpp_phv_fifo_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > dpp_phv_fifo_correctable_interrupt_cpp_int_t;
        cpp_int int_var__dpp_phv_fifo_correctable_interrupt;
        void dpp_phv_fifo_correctable_interrupt (const cpp_int  & l__val);
        cpp_int dpp_phv_fifo_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > dpp_ohi_fifo_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__dpp_ohi_fifo_uncorrectable_interrupt;
        void dpp_ohi_fifo_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int dpp_ohi_fifo_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > dpp_ohi_fifo_correctable_interrupt_cpp_int_t;
        cpp_int int_var__dpp_ohi_fifo_correctable_interrupt;
        void dpp_ohi_fifo_correctable_interrupt (const cpp_int  & l__val);
        cpp_int dpp_ohi_fifo_correctable_interrupt() const;
    
}; // cap_dpp_csr_int_srams_ecc_intreg_t
    
class cap_dpp_csr_int_srams_ecc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_int_srams_ecc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_int_srams_ecc_t(string name = "cap_dpp_csr_int_srams_ecc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_int_srams_ecc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_dpp_csr_int_srams_ecc_intreg_t intreg;
    
        cap_dpp_csr_int_srams_ecc_intreg_t int_test_set;
    
        cap_dpp_csr_int_srams_ecc_int_enable_set_t int_enable_set;
    
        cap_dpp_csr_int_srams_ecc_int_enable_set_t int_enable_clear;
    
}; // cap_dpp_csr_int_srams_ecc_t
    
class cap_dpp_csr_cfw_dpp_spare_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_cfw_dpp_spare_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_cfw_dpp_spare_t(string name = "cap_dpp_csr_cfw_dpp_spare_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_cfw_dpp_spare_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > pulse_0_cpp_int_t;
        cpp_int int_var__pulse_0;
        void pulse_0 (const cpp_int  & l__val);
        cpp_int pulse_0() const;
    
        typedef pu_cpp_int< 1 > pulse_1_cpp_int_t;
        cpp_int int_var__pulse_1;
        void pulse_1 (const cpp_int  & l__val);
        cpp_int pulse_1() const;
    
        typedef pu_cpp_int< 1 > pulse_2_cpp_int_t;
        cpp_int int_var__pulse_2;
        void pulse_2 (const cpp_int  & l__val);
        cpp_int pulse_2() const;
    
        typedef pu_cpp_int< 1 > pulse_3_cpp_int_t;
        cpp_int int_var__pulse_3;
        void pulse_3 (const cpp_int  & l__val);
        cpp_int pulse_3() const;
    
        typedef pu_cpp_int< 1 > pulse_4_cpp_int_t;
        cpp_int int_var__pulse_4;
        void pulse_4 (const cpp_int  & l__val);
        cpp_int pulse_4() const;
    
        typedef pu_cpp_int< 1 > pulse_5_cpp_int_t;
        cpp_int int_var__pulse_5;
        void pulse_5 (const cpp_int  & l__val);
        cpp_int pulse_5() const;
    
        typedef pu_cpp_int< 1 > pulse_6_cpp_int_t;
        cpp_int int_var__pulse_6;
        void pulse_6 (const cpp_int  & l__val);
        cpp_int pulse_6() const;
    
        typedef pu_cpp_int< 1 > pulse_7_cpp_int_t;
        cpp_int int_var__pulse_7;
        void pulse_7 (const cpp_int  & l__val);
        cpp_int pulse_7() const;
    
        typedef pu_cpp_int< 1 > pulse_8_cpp_int_t;
        cpp_int int_var__pulse_8;
        void pulse_8 (const cpp_int  & l__val);
        cpp_int pulse_8() const;
    
        typedef pu_cpp_int< 1 > pulse_9_cpp_int_t;
        cpp_int int_var__pulse_9;
        void pulse_9 (const cpp_int  & l__val);
        cpp_int pulse_9() const;
    
        typedef pu_cpp_int< 1 > pulse_10_cpp_int_t;
        cpp_int int_var__pulse_10;
        void pulse_10 (const cpp_int  & l__val);
        cpp_int pulse_10() const;
    
        typedef pu_cpp_int< 1 > pulse_11_cpp_int_t;
        cpp_int int_var__pulse_11;
        void pulse_11 (const cpp_int  & l__val);
        cpp_int pulse_11() const;
    
        typedef pu_cpp_int< 1 > pulse_12_cpp_int_t;
        cpp_int int_var__pulse_12;
        void pulse_12 (const cpp_int  & l__val);
        cpp_int pulse_12() const;
    
        typedef pu_cpp_int< 1 > pulse_13_cpp_int_t;
        cpp_int int_var__pulse_13;
        void pulse_13 (const cpp_int  & l__val);
        cpp_int pulse_13() const;
    
        typedef pu_cpp_int< 1 > pulse_14_cpp_int_t;
        cpp_int int_var__pulse_14;
        void pulse_14 (const cpp_int  & l__val);
        cpp_int pulse_14() const;
    
        typedef pu_cpp_int< 1 > pulse_15_cpp_int_t;
        cpp_int int_var__pulse_15;
        void pulse_15 (const cpp_int  & l__val);
        cpp_int pulse_15() const;
    
        typedef pu_cpp_int< 1 > pulse_16_cpp_int_t;
        cpp_int int_var__pulse_16;
        void pulse_16 (const cpp_int  & l__val);
        cpp_int pulse_16() const;
    
        typedef pu_cpp_int< 1 > pulse_17_cpp_int_t;
        cpp_int int_var__pulse_17;
        void pulse_17 (const cpp_int  & l__val);
        cpp_int pulse_17() const;
    
        typedef pu_cpp_int< 1 > pulse_18_cpp_int_t;
        cpp_int int_var__pulse_18;
        void pulse_18 (const cpp_int  & l__val);
        cpp_int pulse_18() const;
    
        typedef pu_cpp_int< 1 > pulse_19_cpp_int_t;
        cpp_int int_var__pulse_19;
        void pulse_19 (const cpp_int  & l__val);
        cpp_int pulse_19() const;
    
        typedef pu_cpp_int< 1 > pulse_20_cpp_int_t;
        cpp_int int_var__pulse_20;
        void pulse_20 (const cpp_int  & l__val);
        cpp_int pulse_20() const;
    
        typedef pu_cpp_int< 1 > pulse_21_cpp_int_t;
        cpp_int int_var__pulse_21;
        void pulse_21 (const cpp_int  & l__val);
        cpp_int pulse_21() const;
    
        typedef pu_cpp_int< 1 > pulse_22_cpp_int_t;
        cpp_int int_var__pulse_22;
        void pulse_22 (const cpp_int  & l__val);
        cpp_int pulse_22() const;
    
        typedef pu_cpp_int< 1 > pulse_23_cpp_int_t;
        cpp_int int_var__pulse_23;
        void pulse_23 (const cpp_int  & l__val);
        cpp_int pulse_23() const;
    
        typedef pu_cpp_int< 1 > pulse_24_cpp_int_t;
        cpp_int int_var__pulse_24;
        void pulse_24 (const cpp_int  & l__val);
        cpp_int pulse_24() const;
    
        typedef pu_cpp_int< 1 > pulse_25_cpp_int_t;
        cpp_int int_var__pulse_25;
        void pulse_25 (const cpp_int  & l__val);
        cpp_int pulse_25() const;
    
        typedef pu_cpp_int< 1 > pulse_26_cpp_int_t;
        cpp_int int_var__pulse_26;
        void pulse_26 (const cpp_int  & l__val);
        cpp_int pulse_26() const;
    
        typedef pu_cpp_int< 1 > pulse_27_cpp_int_t;
        cpp_int int_var__pulse_27;
        void pulse_27 (const cpp_int  & l__val);
        cpp_int pulse_27() const;
    
        typedef pu_cpp_int< 1 > pulse_28_cpp_int_t;
        cpp_int int_var__pulse_28;
        void pulse_28 (const cpp_int  & l__val);
        cpp_int pulse_28() const;
    
        typedef pu_cpp_int< 1 > pulse_29_cpp_int_t;
        cpp_int int_var__pulse_29;
        void pulse_29 (const cpp_int  & l__val);
        cpp_int pulse_29() const;
    
        typedef pu_cpp_int< 1 > pulse_30_cpp_int_t;
        cpp_int int_var__pulse_30;
        void pulse_30 (const cpp_int  & l__val);
        cpp_int pulse_30() const;
    
        typedef pu_cpp_int< 1 > pulse_31_cpp_int_t;
        cpp_int int_var__pulse_31;
        void pulse_31 (const cpp_int  & l__val);
        cpp_int pulse_31() const;
    
}; // cap_dpp_csr_cfw_dpp_spare_t
    
class cap_dpp_csr_cfg_spare_csr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_cfg_spare_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_cfg_spare_csr_t(string name = "cap_dpp_csr_cfg_spare_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_cfg_spare_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dpp_csr_cfg_spare_csr_t
    
class cap_dpp_csr_csr_intr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_csr_intr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_csr_intr_t(string name = "cap_dpp_csr_csr_intr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_csr_intr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dpp_csr_csr_intr_t
    
class cap_dpp_csr_cfg_ohi_payload_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_cfg_ohi_payload_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_cfg_ohi_payload_t(string name = "cap_dpp_csr_cfg_ohi_payload_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_cfg_ohi_payload_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 64 > ohi_slot_payload_ptr_bm_cpp_int_t;
        cpp_int int_var__ohi_slot_payload_ptr_bm;
        void ohi_slot_payload_ptr_bm (const cpp_int  & l__val);
        cpp_int ohi_slot_payload_ptr_bm() const;
    
}; // cap_dpp_csr_cfg_ohi_payload_t
    
class cap_dpp_csr_cfg_interrupt_spare_mask_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_cfg_interrupt_spare_mask_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_cfg_interrupt_spare_mask_t(string name = "cap_dpp_csr_cfg_interrupt_spare_mask_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_cfg_interrupt_spare_mask_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > spare_0_cpp_int_t;
        cpp_int int_var__spare_0;
        void spare_0 (const cpp_int  & l__val);
        cpp_int spare_0() const;
    
        typedef pu_cpp_int< 1 > spare_1_cpp_int_t;
        cpp_int int_var__spare_1;
        void spare_1 (const cpp_int  & l__val);
        cpp_int spare_1() const;
    
        typedef pu_cpp_int< 1 > spare_2_cpp_int_t;
        cpp_int int_var__spare_2;
        void spare_2 (const cpp_int  & l__val);
        cpp_int spare_2() const;
    
        typedef pu_cpp_int< 1 > spare_3_cpp_int_t;
        cpp_int int_var__spare_3;
        void spare_3 (const cpp_int  & l__val);
        cpp_int spare_3() const;
    
        typedef pu_cpp_int< 1 > spare_4_cpp_int_t;
        cpp_int int_var__spare_4;
        void spare_4 (const cpp_int  & l__val);
        cpp_int spare_4() const;
    
        typedef pu_cpp_int< 1 > spare_5_cpp_int_t;
        cpp_int int_var__spare_5;
        void spare_5 (const cpp_int  & l__val);
        cpp_int spare_5() const;
    
        typedef pu_cpp_int< 1 > spare_6_cpp_int_t;
        cpp_int int_var__spare_6;
        void spare_6 (const cpp_int  & l__val);
        cpp_int spare_6() const;
    
        typedef pu_cpp_int< 1 > spare_7_cpp_int_t;
        cpp_int int_var__spare_7;
        void spare_7 (const cpp_int  & l__val);
        cpp_int spare_7() const;
    
        typedef pu_cpp_int< 1 > spare_8_cpp_int_t;
        cpp_int int_var__spare_8;
        void spare_8 (const cpp_int  & l__val);
        cpp_int spare_8() const;
    
        typedef pu_cpp_int< 1 > spare_9_cpp_int_t;
        cpp_int int_var__spare_9;
        void spare_9 (const cpp_int  & l__val);
        cpp_int spare_9() const;
    
        typedef pu_cpp_int< 1 > spare_10_cpp_int_t;
        cpp_int int_var__spare_10;
        void spare_10 (const cpp_int  & l__val);
        cpp_int spare_10() const;
    
        typedef pu_cpp_int< 1 > spare_11_cpp_int_t;
        cpp_int int_var__spare_11;
        void spare_11 (const cpp_int  & l__val);
        cpp_int spare_11() const;
    
        typedef pu_cpp_int< 1 > spare_12_cpp_int_t;
        cpp_int int_var__spare_12;
        void spare_12 (const cpp_int  & l__val);
        cpp_int spare_12() const;
    
        typedef pu_cpp_int< 1 > spare_13_cpp_int_t;
        cpp_int int_var__spare_13;
        void spare_13 (const cpp_int  & l__val);
        cpp_int spare_13() const;
    
        typedef pu_cpp_int< 1 > spare_14_cpp_int_t;
        cpp_int int_var__spare_14;
        void spare_14 (const cpp_int  & l__val);
        cpp_int spare_14() const;
    
        typedef pu_cpp_int< 1 > spare_15_cpp_int_t;
        cpp_int int_var__spare_15;
        void spare_15 (const cpp_int  & l__val);
        cpp_int spare_15() const;
    
        typedef pu_cpp_int< 1 > spare_16_cpp_int_t;
        cpp_int int_var__spare_16;
        void spare_16 (const cpp_int  & l__val);
        cpp_int spare_16() const;
    
        typedef pu_cpp_int< 1 > spare_17_cpp_int_t;
        cpp_int int_var__spare_17;
        void spare_17 (const cpp_int  & l__val);
        cpp_int spare_17() const;
    
        typedef pu_cpp_int< 1 > spare_18_cpp_int_t;
        cpp_int int_var__spare_18;
        void spare_18 (const cpp_int  & l__val);
        cpp_int spare_18() const;
    
        typedef pu_cpp_int< 1 > spare_19_cpp_int_t;
        cpp_int int_var__spare_19;
        void spare_19 (const cpp_int  & l__val);
        cpp_int spare_19() const;
    
        typedef pu_cpp_int< 1 > spare_20_cpp_int_t;
        cpp_int int_var__spare_20;
        void spare_20 (const cpp_int  & l__val);
        cpp_int spare_20() const;
    
        typedef pu_cpp_int< 1 > spare_21_cpp_int_t;
        cpp_int int_var__spare_21;
        void spare_21 (const cpp_int  & l__val);
        cpp_int spare_21() const;
    
        typedef pu_cpp_int< 1 > spare_22_cpp_int_t;
        cpp_int int_var__spare_22;
        void spare_22 (const cpp_int  & l__val);
        cpp_int spare_22() const;
    
        typedef pu_cpp_int< 1 > spare_23_cpp_int_t;
        cpp_int int_var__spare_23;
        void spare_23 (const cpp_int  & l__val);
        cpp_int spare_23() const;
    
        typedef pu_cpp_int< 1 > spare_24_cpp_int_t;
        cpp_int int_var__spare_24;
        void spare_24 (const cpp_int  & l__val);
        cpp_int spare_24() const;
    
        typedef pu_cpp_int< 1 > spare_25_cpp_int_t;
        cpp_int int_var__spare_25;
        void spare_25 (const cpp_int  & l__val);
        cpp_int spare_25() const;
    
        typedef pu_cpp_int< 1 > spare_26_cpp_int_t;
        cpp_int int_var__spare_26;
        void spare_26 (const cpp_int  & l__val);
        cpp_int spare_26() const;
    
        typedef pu_cpp_int< 1 > spare_27_cpp_int_t;
        cpp_int int_var__spare_27;
        void spare_27 (const cpp_int  & l__val);
        cpp_int spare_27() const;
    
        typedef pu_cpp_int< 1 > spare_28_cpp_int_t;
        cpp_int int_var__spare_28;
        void spare_28 (const cpp_int  & l__val);
        cpp_int spare_28() const;
    
        typedef pu_cpp_int< 1 > spare_29_cpp_int_t;
        cpp_int int_var__spare_29;
        void spare_29 (const cpp_int  & l__val);
        cpp_int spare_29() const;
    
        typedef pu_cpp_int< 1 > spare_30_cpp_int_t;
        cpp_int int_var__spare_30;
        void spare_30 (const cpp_int  & l__val);
        cpp_int spare_30() const;
    
        typedef pu_cpp_int< 1 > spare_31_cpp_int_t;
        cpp_int int_var__spare_31;
        void spare_31 (const cpp_int  & l__val);
        cpp_int spare_31() const;
    
}; // cap_dpp_csr_cfg_interrupt_spare_mask_t
    
class cap_dpp_csr_cfg_interrupt_mask_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_cfg_interrupt_mask_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_cfg_interrupt_mask_t(string name = "cap_dpp_csr_cfg_interrupt_mask_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_cfg_interrupt_mask_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > err_phv_sop_no_eop_cpp_int_t;
        cpp_int int_var__err_phv_sop_no_eop;
        void err_phv_sop_no_eop (const cpp_int  & l__val);
        cpp_int err_phv_sop_no_eop() const;
    
        typedef pu_cpp_int< 1 > err_phv_eop_no_sop_cpp_int_t;
        cpp_int int_var__err_phv_eop_no_sop;
        void err_phv_eop_no_sop (const cpp_int  & l__val);
        cpp_int err_phv_eop_no_sop() const;
    
        typedef pu_cpp_int< 1 > err_ohi_sop_no_eop_cpp_int_t;
        cpp_int int_var__err_ohi_sop_no_eop;
        void err_ohi_sop_no_eop (const cpp_int  & l__val);
        cpp_int err_ohi_sop_no_eop() const;
    
        typedef pu_cpp_int< 1 > err_ohi_eop_no_sop_cpp_int_t;
        cpp_int int_var__err_ohi_eop_no_sop;
        void err_ohi_eop_no_sop (const cpp_int  & l__val);
        cpp_int err_ohi_eop_no_sop() const;
    
        typedef pu_cpp_int< 1 > err_framer_credit_overrun_cpp_int_t;
        cpp_int int_var__err_framer_credit_overrun;
        void err_framer_credit_overrun (const cpp_int  & l__val);
        cpp_int err_framer_credit_overrun() const;
    
        typedef pu_cpp_int< 1 > err_packets_in_flight_credit_overrun_cpp_int_t;
        cpp_int int_var__err_packets_in_flight_credit_overrun;
        void err_packets_in_flight_credit_overrun (const cpp_int  & l__val);
        cpp_int err_packets_in_flight_credit_overrun() const;
    
        typedef pu_cpp_int< 1 > err_null_hdr_vld_cpp_int_t;
        cpp_int int_var__err_null_hdr_vld;
        void err_null_hdr_vld (const cpp_int  & l__val);
        cpp_int err_null_hdr_vld() const;
    
        typedef pu_cpp_int< 1 > err_null_hdrfld_vld_cpp_int_t;
        cpp_int int_var__err_null_hdrfld_vld;
        void err_null_hdrfld_vld (const cpp_int  & l__val);
        cpp_int err_null_hdrfld_vld() const;
    
        typedef pu_cpp_int< 1 > err_max_pkt_size_cpp_int_t;
        cpp_int int_var__err_max_pkt_size;
        void err_max_pkt_size (const cpp_int  & l__val);
        cpp_int err_max_pkt_size() const;
    
        typedef pu_cpp_int< 1 > err_max_active_hdrs_cpp_int_t;
        cpp_int int_var__err_max_active_hdrs;
        void err_max_active_hdrs (const cpp_int  & l__val);
        cpp_int err_max_active_hdrs() const;
    
        typedef pu_cpp_int< 1 > err_phv_no_data_reference_ohi_cpp_int_t;
        cpp_int int_var__err_phv_no_data_reference_ohi;
        void err_phv_no_data_reference_ohi (const cpp_int  & l__val);
        cpp_int err_phv_no_data_reference_ohi() const;
    
        typedef pu_cpp_int< 1 > err_csum_multiple_hdr_cpp_int_t;
        cpp_int int_var__err_csum_multiple_hdr;
        void err_csum_multiple_hdr (const cpp_int  & l__val);
        cpp_int err_csum_multiple_hdr() const;
    
        typedef pu_cpp_int< 1 > err_csum_multiple_hdr_copy_cpp_int_t;
        cpp_int int_var__err_csum_multiple_hdr_copy;
        void err_csum_multiple_hdr_copy (const cpp_int  & l__val);
        cpp_int err_csum_multiple_hdr_copy() const;
    
        typedef pu_cpp_int< 1 > err_crc_multiple_hdr_cpp_int_t;
        cpp_int int_var__err_crc_multiple_hdr;
        void err_crc_multiple_hdr (const cpp_int  & l__val);
        cpp_int err_crc_multiple_hdr() const;
    
        typedef pu_cpp_int< 1 > err_ptr_fifo_credit_overrun_cpp_int_t;
        cpp_int int_var__err_ptr_fifo_credit_overrun;
        void err_ptr_fifo_credit_overrun (const cpp_int  & l__val);
        cpp_int err_ptr_fifo_credit_overrun() const;
    
        typedef pu_cpp_int< 1 > err_clip_max_pkt_size_cpp_int_t;
        cpp_int int_var__err_clip_max_pkt_size;
        void err_clip_max_pkt_size (const cpp_int  & l__val);
        cpp_int err_clip_max_pkt_size() const;
    
        typedef pu_cpp_int< 1 > err_min_pkt_size_cpp_int_t;
        cpp_int int_var__err_min_pkt_size;
        void err_min_pkt_size (const cpp_int  & l__val);
        cpp_int err_min_pkt_size() const;
    
        typedef pu_cpp_int< 32 > rsvd_cpp_int_t;
        cpp_int int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
}; // cap_dpp_csr_cfg_interrupt_mask_t
    
class cap_dpp_csr_cfg_error_spare_mask_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_cfg_error_spare_mask_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_cfg_error_spare_mask_t(string name = "cap_dpp_csr_cfg_error_spare_mask_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_cfg_error_spare_mask_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > spare_0_cpp_int_t;
        cpp_int int_var__spare_0;
        void spare_0 (const cpp_int  & l__val);
        cpp_int spare_0() const;
    
        typedef pu_cpp_int< 1 > spare_1_cpp_int_t;
        cpp_int int_var__spare_1;
        void spare_1 (const cpp_int  & l__val);
        cpp_int spare_1() const;
    
        typedef pu_cpp_int< 1 > spare_2_cpp_int_t;
        cpp_int int_var__spare_2;
        void spare_2 (const cpp_int  & l__val);
        cpp_int spare_2() const;
    
        typedef pu_cpp_int< 1 > spare_3_cpp_int_t;
        cpp_int int_var__spare_3;
        void spare_3 (const cpp_int  & l__val);
        cpp_int spare_3() const;
    
        typedef pu_cpp_int< 1 > spare_4_cpp_int_t;
        cpp_int int_var__spare_4;
        void spare_4 (const cpp_int  & l__val);
        cpp_int spare_4() const;
    
        typedef pu_cpp_int< 1 > spare_5_cpp_int_t;
        cpp_int int_var__spare_5;
        void spare_5 (const cpp_int  & l__val);
        cpp_int spare_5() const;
    
        typedef pu_cpp_int< 1 > spare_6_cpp_int_t;
        cpp_int int_var__spare_6;
        void spare_6 (const cpp_int  & l__val);
        cpp_int spare_6() const;
    
        typedef pu_cpp_int< 1 > spare_7_cpp_int_t;
        cpp_int int_var__spare_7;
        void spare_7 (const cpp_int  & l__val);
        cpp_int spare_7() const;
    
        typedef pu_cpp_int< 1 > spare_8_cpp_int_t;
        cpp_int int_var__spare_8;
        void spare_8 (const cpp_int  & l__val);
        cpp_int spare_8() const;
    
        typedef pu_cpp_int< 1 > spare_9_cpp_int_t;
        cpp_int int_var__spare_9;
        void spare_9 (const cpp_int  & l__val);
        cpp_int spare_9() const;
    
        typedef pu_cpp_int< 1 > spare_10_cpp_int_t;
        cpp_int int_var__spare_10;
        void spare_10 (const cpp_int  & l__val);
        cpp_int spare_10() const;
    
        typedef pu_cpp_int< 1 > spare_11_cpp_int_t;
        cpp_int int_var__spare_11;
        void spare_11 (const cpp_int  & l__val);
        cpp_int spare_11() const;
    
        typedef pu_cpp_int< 1 > spare_12_cpp_int_t;
        cpp_int int_var__spare_12;
        void spare_12 (const cpp_int  & l__val);
        cpp_int spare_12() const;
    
        typedef pu_cpp_int< 1 > spare_13_cpp_int_t;
        cpp_int int_var__spare_13;
        void spare_13 (const cpp_int  & l__val);
        cpp_int spare_13() const;
    
        typedef pu_cpp_int< 1 > spare_14_cpp_int_t;
        cpp_int int_var__spare_14;
        void spare_14 (const cpp_int  & l__val);
        cpp_int spare_14() const;
    
        typedef pu_cpp_int< 1 > spare_15_cpp_int_t;
        cpp_int int_var__spare_15;
        void spare_15 (const cpp_int  & l__val);
        cpp_int spare_15() const;
    
        typedef pu_cpp_int< 1 > spare_16_cpp_int_t;
        cpp_int int_var__spare_16;
        void spare_16 (const cpp_int  & l__val);
        cpp_int spare_16() const;
    
        typedef pu_cpp_int< 1 > spare_17_cpp_int_t;
        cpp_int int_var__spare_17;
        void spare_17 (const cpp_int  & l__val);
        cpp_int spare_17() const;
    
        typedef pu_cpp_int< 1 > spare_18_cpp_int_t;
        cpp_int int_var__spare_18;
        void spare_18 (const cpp_int  & l__val);
        cpp_int spare_18() const;
    
        typedef pu_cpp_int< 1 > spare_19_cpp_int_t;
        cpp_int int_var__spare_19;
        void spare_19 (const cpp_int  & l__val);
        cpp_int spare_19() const;
    
        typedef pu_cpp_int< 1 > spare_20_cpp_int_t;
        cpp_int int_var__spare_20;
        void spare_20 (const cpp_int  & l__val);
        cpp_int spare_20() const;
    
        typedef pu_cpp_int< 1 > spare_21_cpp_int_t;
        cpp_int int_var__spare_21;
        void spare_21 (const cpp_int  & l__val);
        cpp_int spare_21() const;
    
        typedef pu_cpp_int< 1 > spare_22_cpp_int_t;
        cpp_int int_var__spare_22;
        void spare_22 (const cpp_int  & l__val);
        cpp_int spare_22() const;
    
        typedef pu_cpp_int< 1 > spare_23_cpp_int_t;
        cpp_int int_var__spare_23;
        void spare_23 (const cpp_int  & l__val);
        cpp_int spare_23() const;
    
        typedef pu_cpp_int< 1 > spare_24_cpp_int_t;
        cpp_int int_var__spare_24;
        void spare_24 (const cpp_int  & l__val);
        cpp_int spare_24() const;
    
        typedef pu_cpp_int< 1 > spare_25_cpp_int_t;
        cpp_int int_var__spare_25;
        void spare_25 (const cpp_int  & l__val);
        cpp_int spare_25() const;
    
        typedef pu_cpp_int< 1 > spare_26_cpp_int_t;
        cpp_int int_var__spare_26;
        void spare_26 (const cpp_int  & l__val);
        cpp_int spare_26() const;
    
        typedef pu_cpp_int< 1 > spare_27_cpp_int_t;
        cpp_int int_var__spare_27;
        void spare_27 (const cpp_int  & l__val);
        cpp_int spare_27() const;
    
        typedef pu_cpp_int< 1 > spare_28_cpp_int_t;
        cpp_int int_var__spare_28;
        void spare_28 (const cpp_int  & l__val);
        cpp_int spare_28() const;
    
        typedef pu_cpp_int< 1 > spare_29_cpp_int_t;
        cpp_int int_var__spare_29;
        void spare_29 (const cpp_int  & l__val);
        cpp_int spare_29() const;
    
        typedef pu_cpp_int< 1 > spare_30_cpp_int_t;
        cpp_int int_var__spare_30;
        void spare_30 (const cpp_int  & l__val);
        cpp_int spare_30() const;
    
        typedef pu_cpp_int< 1 > spare_31_cpp_int_t;
        cpp_int int_var__spare_31;
        void spare_31 (const cpp_int  & l__val);
        cpp_int spare_31() const;
    
}; // cap_dpp_csr_cfg_error_spare_mask_t
    
class cap_dpp_csr_cfg_error_mask_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_cfg_error_mask_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_cfg_error_mask_t(string name = "cap_dpp_csr_cfg_error_mask_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_cfg_error_mask_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > err_phv_sop_no_eop_cpp_int_t;
        cpp_int int_var__err_phv_sop_no_eop;
        void err_phv_sop_no_eop (const cpp_int  & l__val);
        cpp_int err_phv_sop_no_eop() const;
    
        typedef pu_cpp_int< 1 > err_phv_eop_no_sop_cpp_int_t;
        cpp_int int_var__err_phv_eop_no_sop;
        void err_phv_eop_no_sop (const cpp_int  & l__val);
        cpp_int err_phv_eop_no_sop() const;
    
        typedef pu_cpp_int< 1 > err_ohi_sop_no_eop_cpp_int_t;
        cpp_int int_var__err_ohi_sop_no_eop;
        void err_ohi_sop_no_eop (const cpp_int  & l__val);
        cpp_int err_ohi_sop_no_eop() const;
    
        typedef pu_cpp_int< 1 > err_ohi_eop_no_sop_cpp_int_t;
        cpp_int int_var__err_ohi_eop_no_sop;
        void err_ohi_eop_no_sop (const cpp_int  & l__val);
        cpp_int err_ohi_eop_no_sop() const;
    
        typedef pu_cpp_int< 1 > err_framer_credit_overrun_cpp_int_t;
        cpp_int int_var__err_framer_credit_overrun;
        void err_framer_credit_overrun (const cpp_int  & l__val);
        cpp_int err_framer_credit_overrun() const;
    
        typedef pu_cpp_int< 1 > err_packets_in_flight_credit_overrun_cpp_int_t;
        cpp_int int_var__err_packets_in_flight_credit_overrun;
        void err_packets_in_flight_credit_overrun (const cpp_int  & l__val);
        cpp_int err_packets_in_flight_credit_overrun() const;
    
        typedef pu_cpp_int< 1 > err_null_hdr_vld_cpp_int_t;
        cpp_int int_var__err_null_hdr_vld;
        void err_null_hdr_vld (const cpp_int  & l__val);
        cpp_int err_null_hdr_vld() const;
    
        typedef pu_cpp_int< 1 > err_null_hdrfld_vld_cpp_int_t;
        cpp_int int_var__err_null_hdrfld_vld;
        void err_null_hdrfld_vld (const cpp_int  & l__val);
        cpp_int err_null_hdrfld_vld() const;
    
        typedef pu_cpp_int< 1 > err_max_pkt_size_cpp_int_t;
        cpp_int int_var__err_max_pkt_size;
        void err_max_pkt_size (const cpp_int  & l__val);
        cpp_int err_max_pkt_size() const;
    
        typedef pu_cpp_int< 1 > err_max_active_hdrs_cpp_int_t;
        cpp_int int_var__err_max_active_hdrs;
        void err_max_active_hdrs (const cpp_int  & l__val);
        cpp_int err_max_active_hdrs() const;
    
        typedef pu_cpp_int< 1 > err_phv_no_data_reference_ohi_cpp_int_t;
        cpp_int int_var__err_phv_no_data_reference_ohi;
        void err_phv_no_data_reference_ohi (const cpp_int  & l__val);
        cpp_int err_phv_no_data_reference_ohi() const;
    
        typedef pu_cpp_int< 1 > err_csum_multiple_hdr_cpp_int_t;
        cpp_int int_var__err_csum_multiple_hdr;
        void err_csum_multiple_hdr (const cpp_int  & l__val);
        cpp_int err_csum_multiple_hdr() const;
    
        typedef pu_cpp_int< 1 > err_csum_multiple_hdr_copy_cpp_int_t;
        cpp_int int_var__err_csum_multiple_hdr_copy;
        void err_csum_multiple_hdr_copy (const cpp_int  & l__val);
        cpp_int err_csum_multiple_hdr_copy() const;
    
        typedef pu_cpp_int< 1 > err_crc_multiple_hdr_cpp_int_t;
        cpp_int int_var__err_crc_multiple_hdr;
        void err_crc_multiple_hdr (const cpp_int  & l__val);
        cpp_int err_crc_multiple_hdr() const;
    
        typedef pu_cpp_int< 1 > err_ptr_fifo_credit_overrun_cpp_int_t;
        cpp_int int_var__err_ptr_fifo_credit_overrun;
        void err_ptr_fifo_credit_overrun (const cpp_int  & l__val);
        cpp_int err_ptr_fifo_credit_overrun() const;
    
        typedef pu_cpp_int< 1 > err_clip_max_pkt_size_cpp_int_t;
        cpp_int int_var__err_clip_max_pkt_size;
        void err_clip_max_pkt_size (const cpp_int  & l__val);
        cpp_int err_clip_max_pkt_size() const;
    
        typedef pu_cpp_int< 1 > err_min_pkt_size_cpp_int_t;
        cpp_int int_var__err_min_pkt_size;
        void err_min_pkt_size (const cpp_int  & l__val);
        cpp_int err_min_pkt_size() const;
    
        typedef pu_cpp_int< 32 > rsvd_cpp_int_t;
        cpp_int int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
}; // cap_dpp_csr_cfg_error_mask_t
    
class cap_dpp_csr_cfg_global_err_code_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_cfg_global_err_code_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_cfg_global_err_code_t(string name = "cap_dpp_csr_cfg_global_err_code_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_cfg_global_err_code_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 3 > hw_err_cpp_int_t;
        cpp_int int_var__hw_err;
        void hw_err (const cpp_int  & l__val);
        cpp_int hw_err() const;
    
        typedef pu_cpp_int< 3 > cfg_err_cpp_int_t;
        cpp_int int_var__cfg_err;
        void cfg_err (const cpp_int  & l__val);
        cpp_int cfg_err() const;
    
        typedef pu_cpp_int< 3 > mtu_err_cpp_int_t;
        cpp_int int_var__mtu_err;
        void mtu_err (const cpp_int  & l__val);
        cpp_int mtu_err() const;
    
}; // cap_dpp_csr_cfg_global_err_code_t
    
class cap_dpp_csr_cfg_global_2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_cfg_global_2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_cfg_global_2_t(string name = "cap_dpp_csr_cfg_global_2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_cfg_global_2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > dump_drop_no_data_phv_0_cpp_int_t;
        cpp_int int_var__dump_drop_no_data_phv_0;
        void dump_drop_no_data_phv_0 (const cpp_int  & l__val);
        cpp_int dump_drop_no_data_phv_0() const;
    
        typedef pu_cpp_int< 1 > dump_drop_no_data_phv_1_cpp_int_t;
        cpp_int int_var__dump_drop_no_data_phv_1;
        void dump_drop_no_data_phv_1 (const cpp_int  & l__val);
        cpp_int dump_drop_no_data_phv_1() const;
    
        typedef pu_cpp_int< 31 > rsvd_cpp_int_t;
        cpp_int int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
}; // cap_dpp_csr_cfg_global_2_t
    
class cap_dpp_csr_cfg_global_hw_1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_cfg_global_hw_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_cfg_global_hw_1_t(string name = "cap_dpp_csr_cfg_global_hw_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_cfg_global_hw_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 5 > ptr_fifo_fc_thr_cpp_int_t;
        cpp_int int_var__ptr_fifo_fc_thr;
        void ptr_fifo_fc_thr (const cpp_int  & l__val);
        cpp_int ptr_fifo_fc_thr() const;
    
        typedef pu_cpp_int< 10 > phv_fifo_almost_full_threshold_cpp_int_t;
        cpp_int int_var__phv_fifo_almost_full_threshold;
        void phv_fifo_almost_full_threshold (const cpp_int  & l__val);
        cpp_int phv_fifo_almost_full_threshold() const;
    
        typedef pu_cpp_int< 10 > ohi_fifo_almost_full_threshold_cpp_int_t;
        cpp_int int_var__ohi_fifo_almost_full_threshold;
        void ohi_fifo_almost_full_threshold (const cpp_int  & l__val);
        cpp_int ohi_fifo_almost_full_threshold() const;
    
        typedef pu_cpp_int< 1 > debug_port_enable_cpp_int_t;
        cpp_int int_var__debug_port_enable;
        void debug_port_enable (const cpp_int  & l__val);
        cpp_int debug_port_enable() const;
    
}; // cap_dpp_csr_cfg_global_hw_1_t
    
class cap_dpp_csr_cfg_global_1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_cfg_global_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_cfg_global_1_t(string name = "cap_dpp_csr_cfg_global_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_cfg_global_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > max_hdrfld_size_cpp_int_t;
        cpp_int int_var__max_hdrfld_size;
        void max_hdrfld_size (const cpp_int  & l__val);
        cpp_int max_hdrfld_size() const;
    
        typedef pu_cpp_int< 16 > max_pkt_size_cpp_int_t;
        cpp_int int_var__max_pkt_size;
        void max_pkt_size (const cpp_int  & l__val);
        cpp_int max_pkt_size() const;
    
        typedef pu_cpp_int< 1 > pkt_truncation_en_cpp_int_t;
        cpp_int int_var__pkt_truncation_en;
        void pkt_truncation_en (const cpp_int  & l__val);
        cpp_int pkt_truncation_en() const;
    
        typedef pu_cpp_int< 1 > ignore_hdrfld_size_0_cpp_int_t;
        cpp_int int_var__ignore_hdrfld_size_0;
        void ignore_hdrfld_size_0 (const cpp_int  & l__val);
        cpp_int ignore_hdrfld_size_0() const;
    
        typedef pu_cpp_int< 1 > bypass_mode_cpp_int_t;
        cpp_int int_var__bypass_mode;
        void bypass_mode (const cpp_int  & l__val);
        cpp_int bypass_mode() const;
    
        typedef pu_cpp_int< 1 > phv_drop_bypass_en_cpp_int_t;
        cpp_int int_var__phv_drop_bypass_en;
        void phv_drop_bypass_en (const cpp_int  & l__val);
        cpp_int phv_drop_bypass_en() const;
    
        typedef pu_cpp_int< 16 > max_bypass_size_cpp_int_t;
        cpp_int int_var__max_bypass_size;
        void max_bypass_size (const cpp_int  & l__val);
        cpp_int max_bypass_size() const;
    
}; // cap_dpp_csr_cfg_global_1_t
    
class cap_dpp_csr_cfg_global_hw_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_cfg_global_hw_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_cfg_global_hw_t(string name = "cap_dpp_csr_cfg_global_hw_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_cfg_global_hw_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 4 > input_pacer_timer_cpp_int_t;
        cpp_int int_var__input_pacer_timer;
        void input_pacer_timer (const cpp_int  & l__val);
        cpp_int input_pacer_timer() const;
    
        typedef pu_cpp_int< 4 > input_pacer_release_cpp_int_t;
        cpp_int int_var__input_pacer_release;
        void input_pacer_release (const cpp_int  & l__val);
        cpp_int input_pacer_release() const;
    
        typedef pu_cpp_int< 4 > packet_in_flight_credit_cpp_int_t;
        cpp_int int_var__packet_in_flight_credit;
        void packet_in_flight_credit (const cpp_int  & l__val);
        cpp_int packet_in_flight_credit() const;
    
        typedef pu_cpp_int< 1 > packet_in_flight_credit_en_cpp_int_t;
        cpp_int int_var__packet_in_flight_credit_en;
        void packet_in_flight_credit_en (const cpp_int  & l__val);
        cpp_int packet_in_flight_credit_en() const;
    
        typedef pu_cpp_int< 2 > phv_framer_credit_cpp_int_t;
        cpp_int int_var__phv_framer_credit;
        void phv_framer_credit (const cpp_int  & l__val);
        cpp_int phv_framer_credit() const;
    
        typedef pu_cpp_int< 1 > phv_framer_credit_en_cpp_int_t;
        cpp_int int_var__phv_framer_credit_en;
        void phv_framer_credit_en (const cpp_int  & l__val);
        cpp_int phv_framer_credit_en() const;
    
        typedef pu_cpp_int< 6 > framer_ptr_fifo_credit_cpp_int_t;
        cpp_int int_var__framer_ptr_fifo_credit;
        void framer_ptr_fifo_credit (const cpp_int  & l__val);
        cpp_int framer_ptr_fifo_credit() const;
    
        typedef pu_cpp_int< 1 > framer_ptr_fifo_credit_en_cpp_int_t;
        cpp_int int_var__framer_ptr_fifo_credit_en;
        void framer_ptr_fifo_credit_en (const cpp_int  & l__val);
        cpp_int framer_ptr_fifo_credit_en() const;
    
        typedef pu_cpp_int< 4 > framer_pacer_timer_cpp_int_t;
        cpp_int int_var__framer_pacer_timer;
        void framer_pacer_timer (const cpp_int  & l__val);
        cpp_int framer_pacer_timer() const;
    
        typedef pu_cpp_int< 1 > framer_pacer_timer_en_cpp_int_t;
        cpp_int int_var__framer_pacer_timer_en;
        void framer_pacer_timer_en (const cpp_int  & l__val);
        cpp_int framer_pacer_timer_en() const;
    
        typedef pu_cpp_int< 4 > framer_eop_pacer_timer_cpp_int_t;
        cpp_int int_var__framer_eop_pacer_timer;
        void framer_eop_pacer_timer (const cpp_int  & l__val);
        cpp_int framer_eop_pacer_timer() const;
    
        typedef pu_cpp_int< 1 > framer_eop_pacer_timer_en_cpp_int_t;
        cpp_int int_var__framer_eop_pacer_timer_en;
        void framer_eop_pacer_timer_en (const cpp_int  & l__val);
        cpp_int framer_eop_pacer_timer_en() const;
    
}; // cap_dpp_csr_cfg_global_hw_t
    
class cap_dpp_csr_cfg_global_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_cfg_global_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_cfg_global_t(string name = "cap_dpp_csr_cfg_global_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_cfg_global_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 7 > hdr_vld_phv_mask_cpp_int_t;
        cpp_int int_var__hdr_vld_phv_mask;
        void hdr_vld_phv_mask (const cpp_int  & l__val);
        cpp_int hdr_vld_phv_mask() const;
    
}; // cap_dpp_csr_cfg_global_t
    
class cap_dpp_csr_base_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_base_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_base_t(string name = "cap_dpp_csr_base_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_base_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dpp_csr_base_t
    
class cap_dpp_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpp_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpp_csr_t(string name = "cap_dpp_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpp_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_dpp_csr_base_t base;
    
        cap_dpp_csr_cfg_global_t cfg_global;
    
        cap_dpp_csr_cfg_global_hw_t cfg_global_hw;
    
        cap_dpp_csr_cfg_global_1_t cfg_global_1;
    
        cap_dpp_csr_cfg_global_hw_1_t cfg_global_hw_1;
    
        cap_dpp_csr_cfg_global_2_t cfg_global_2;
    
        cap_dpp_csr_cfg_global_err_code_t cfg_global_err_code;
    
        cap_dpp_csr_cfg_error_mask_t cfg_error_mask;
    
        cap_dpp_csr_cfg_error_spare_mask_t cfg_error_spare_mask;
    
        cap_dpp_csr_cfg_interrupt_mask_t cfg_interrupt_mask;
    
        cap_dpp_csr_cfg_interrupt_spare_mask_t cfg_interrupt_spare_mask;
    
        cap_dpp_csr_cfg_ohi_payload_t cfg_ohi_payload;
    
        cap_dpp_csr_csr_intr_t csr_intr;
    
        #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_dpp_csr_cfg_spare_csr_t, 32> cfg_spare_csr;
        #else 
        cap_dpp_csr_cfg_spare_csr_t cfg_spare_csr[32];
        #endif
        int get_depth_cfg_spare_csr() { return 32; }
    
        cap_dpp_csr_cfw_dpp_spare_t cfw_dpp_spare;
    
        cap_dpphdr_csr_t hdr;
    
        cap_dpphdrfld_csr_t hdrfld;
    
        cap_dppcsum_csr_t csum;
    
        cap_dppstats_csr_t stats;
    
        cap_dppmem_csr_t mem;
    
        cap_dpp_csr_int_srams_ecc_t int_srams_ecc;
    
        cap_dpp_csr_intgrp_status_t int_groups;
    
        cap_dpp_csr_int_reg1_t int_reg1;
    
        cap_dpp_csr_int_reg2_t int_reg2;
    
        cap_dpp_csr_int_fifo_t int_fifo;
    
        cap_dpp_csr_int_credit_t int_credit;
    
        cap_dpp_csr_int_spare_t int_spare;
    
}; // cap_dpp_csr_t
    
#endif // CAP_DPP_CSR_H
        