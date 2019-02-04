
#ifndef CAP_DPR_CSR_H
#define CAP_DPR_CSR_H

#include "cap_csr_base.h" 
#include "cap_dprmem_csr.h" 
#include "cap_dprstats_csr.h" 
#include "cap_dprcfg_csr.h" 
#include "cap_dprhdrfld_csr.h" 

using namespace std;
class cap_dpr_csr_int_spare_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_int_spare_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_int_spare_int_enable_clear_t(string name = "cap_dpr_csr_int_spare_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_int_spare_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dpr_csr_int_spare_int_enable_clear_t
    
class cap_dpr_csr_int_spare_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_int_spare_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_int_spare_int_test_set_t(string name = "cap_dpr_csr_int_spare_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_int_spare_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dpr_csr_int_spare_int_test_set_t
    
class cap_dpr_csr_int_spare_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_int_spare_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_int_spare_t(string name = "cap_dpr_csr_int_spare_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_int_spare_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_dpr_csr_int_spare_int_test_set_t intreg;
    
        cap_dpr_csr_int_spare_int_test_set_t int_test_set;
    
        cap_dpr_csr_int_spare_int_enable_clear_t int_enable_set;
    
        cap_dpr_csr_int_spare_int_enable_clear_t int_enable_clear;
    
}; // cap_dpr_csr_int_spare_t
    
class cap_dpr_csr_int_credit_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_int_credit_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_int_credit_int_enable_clear_t(string name = "cap_dpr_csr_int_credit_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_int_credit_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > egress_credit_ovflow_enable_cpp_int_t;
        cpp_int int_var__egress_credit_ovflow_enable;
        void egress_credit_ovflow_enable (const cpp_int  & l__val);
        cpp_int egress_credit_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > egress_credit_undflow_enable_cpp_int_t;
        cpp_int int_var__egress_credit_undflow_enable;
        void egress_credit_undflow_enable (const cpp_int  & l__val);
        cpp_int egress_credit_undflow_enable() const;
    
        typedef pu_cpp_int< 1 > pktout_credit_ovflow_enable_cpp_int_t;
        cpp_int int_var__pktout_credit_ovflow_enable;
        void pktout_credit_ovflow_enable (const cpp_int  & l__val);
        cpp_int pktout_credit_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pktout_credit_undflow_enable_cpp_int_t;
        cpp_int int_var__pktout_credit_undflow_enable;
        void pktout_credit_undflow_enable (const cpp_int  & l__val);
        cpp_int pktout_credit_undflow_enable() const;
    
}; // cap_dpr_csr_int_credit_int_enable_clear_t
    
class cap_dpr_csr_int_credit_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_int_credit_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_int_credit_int_test_set_t(string name = "cap_dpr_csr_int_credit_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_int_credit_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > egress_credit_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__egress_credit_ovflow_interrupt;
        void egress_credit_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int egress_credit_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > egress_credit_undflow_interrupt_cpp_int_t;
        cpp_int int_var__egress_credit_undflow_interrupt;
        void egress_credit_undflow_interrupt (const cpp_int  & l__val);
        cpp_int egress_credit_undflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pktout_credit_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pktout_credit_ovflow_interrupt;
        void pktout_credit_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pktout_credit_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pktout_credit_undflow_interrupt_cpp_int_t;
        cpp_int int_var__pktout_credit_undflow_interrupt;
        void pktout_credit_undflow_interrupt (const cpp_int  & l__val);
        cpp_int pktout_credit_undflow_interrupt() const;
    
}; // cap_dpr_csr_int_credit_int_test_set_t
    
class cap_dpr_csr_int_credit_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_int_credit_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_int_credit_t(string name = "cap_dpr_csr_int_credit_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_int_credit_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_dpr_csr_int_credit_int_test_set_t intreg;
    
        cap_dpr_csr_int_credit_int_test_set_t int_test_set;
    
        cap_dpr_csr_int_credit_int_enable_clear_t int_enable_set;
    
        cap_dpr_csr_int_credit_int_enable_clear_t int_enable_clear;
    
}; // cap_dpr_csr_int_credit_t
    
class cap_dpr_csr_int_flop_fifo_1_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_int_flop_fifo_1_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_int_flop_fifo_1_int_enable_clear_t(string name = "cap_dpr_csr_int_flop_fifo_1_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_int_flop_fifo_1_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > data_mux_force_bypass_crc_flop_ff_undflow_enable_cpp_int_t;
        cpp_int int_var__data_mux_force_bypass_crc_flop_ff_undflow_enable;
        void data_mux_force_bypass_crc_flop_ff_undflow_enable (const cpp_int  & l__val);
        cpp_int data_mux_force_bypass_crc_flop_ff_undflow_enable() const;
    
        typedef pu_cpp_int< 1 > dpr_crc_info_flop_ff_undflow_enable_cpp_int_t;
        cpp_int int_var__dpr_crc_info_flop_ff_undflow_enable;
        void dpr_crc_info_flop_ff_undflow_enable (const cpp_int  & l__val);
        cpp_int dpr_crc_info_flop_ff_undflow_enable() const;
    
        typedef pu_cpp_int< 1 > dpr_crc_update_info_flop_ff_undflow_enable_cpp_int_t;
        cpp_int int_var__dpr_crc_update_info_flop_ff_undflow_enable;
        void dpr_crc_update_info_flop_ff_undflow_enable (const cpp_int  & l__val);
        cpp_int dpr_crc_update_info_flop_ff_undflow_enable() const;
    
        typedef pu_cpp_int< 1 > dpr_csum_info_flop_ff_undflow_enable_cpp_int_t;
        cpp_int int_var__dpr_csum_info_flop_ff_undflow_enable;
        void dpr_csum_info_flop_ff_undflow_enable (const cpp_int  & l__val);
        cpp_int dpr_csum_info_flop_ff_undflow_enable() const;
    
        typedef pu_cpp_int< 1 > data_mux_force_bypass_csum_flop_ff_undflow_enable_cpp_int_t;
        cpp_int int_var__data_mux_force_bypass_csum_flop_ff_undflow_enable;
        void data_mux_force_bypass_csum_flop_ff_undflow_enable (const cpp_int  & l__val);
        cpp_int data_mux_force_bypass_csum_flop_ff_undflow_enable() const;
    
        typedef pu_cpp_int< 1 > dpr_csum_update_info_flop_ff_undflow_enable_cpp_int_t;
        cpp_int int_var__dpr_csum_update_info_flop_ff_undflow_enable;
        void dpr_csum_update_info_flop_ff_undflow_enable (const cpp_int  & l__val);
        cpp_int dpr_csum_update_info_flop_ff_undflow_enable() const;
    
        typedef pu_cpp_int< 1 > ptr_early_pkt_eop_info_flop_ff_undflow_enable_cpp_int_t;
        cpp_int int_var__ptr_early_pkt_eop_info_flop_ff_undflow_enable;
        void ptr_early_pkt_eop_info_flop_ff_undflow_enable (const cpp_int  & l__val);
        cpp_int ptr_early_pkt_eop_info_flop_ff_undflow_enable() const;
    
        typedef pu_cpp_int< 1 > data_mux_eop_err_flop_ff_undflow_enable_cpp_int_t;
        cpp_int int_var__data_mux_eop_err_flop_ff_undflow_enable;
        void data_mux_eop_err_flop_ff_undflow_enable (const cpp_int  & l__val);
        cpp_int data_mux_eop_err_flop_ff_undflow_enable() const;
    
        typedef pu_cpp_int< 1 > pktin_eop_err_flop_ff_undflow_enable_cpp_int_t;
        cpp_int int_var__pktin_eop_err_flop_ff_undflow_enable;
        void pktin_eop_err_flop_ff_undflow_enable (const cpp_int  & l__val);
        cpp_int pktin_eop_err_flop_ff_undflow_enable() const;
    
        typedef pu_cpp_int< 1 > csum_err_flop_ff_undflow_enable_cpp_int_t;
        cpp_int int_var__csum_err_flop_ff_undflow_enable;
        void csum_err_flop_ff_undflow_enable (const cpp_int  & l__val);
        cpp_int csum_err_flop_ff_undflow_enable() const;
    
        typedef pu_cpp_int< 1 > crc_err_flop_ff_undflow_enable_cpp_int_t;
        cpp_int int_var__crc_err_flop_ff_undflow_enable;
        void crc_err_flop_ff_undflow_enable (const cpp_int  & l__val);
        cpp_int crc_err_flop_ff_undflow_enable() const;
    
        typedef pu_cpp_int< 1 > data_mux_drop_flop_ff_undflow_enable_cpp_int_t;
        cpp_int int_var__data_mux_drop_flop_ff_undflow_enable;
        void data_mux_drop_flop_ff_undflow_enable (const cpp_int  & l__val);
        cpp_int data_mux_drop_flop_ff_undflow_enable() const;
    
        typedef pu_cpp_int< 1 > phv_pkt_data_flop_ff_undflow_enable_cpp_int_t;
        cpp_int int_var__phv_pkt_data_flop_ff_undflow_enable;
        void phv_pkt_data_flop_ff_undflow_enable (const cpp_int  & l__val);
        cpp_int phv_pkt_data_flop_ff_undflow_enable() const;
    
        typedef pu_cpp_int< 1 > pktout_len_cell_flop_ff_undflow_enable_cpp_int_t;
        cpp_int int_var__pktout_len_cell_flop_ff_undflow_enable;
        void pktout_len_cell_flop_ff_undflow_enable (const cpp_int  & l__val);
        cpp_int pktout_len_cell_flop_ff_undflow_enable() const;
    
        typedef pu_cpp_int< 1 > padding_size_flop_ff_undflow_enable_cpp_int_t;
        cpp_int int_var__padding_size_flop_ff_undflow_enable;
        void padding_size_flop_ff_undflow_enable (const cpp_int  & l__val);
        cpp_int padding_size_flop_ff_undflow_enable() const;
    
        typedef pu_cpp_int< 1 > pktin_err_flop_ff_undflow_enable_cpp_int_t;
        cpp_int int_var__pktin_err_flop_ff_undflow_enable;
        void pktin_err_flop_ff_undflow_enable (const cpp_int  & l__val);
        cpp_int pktin_err_flop_ff_undflow_enable() const;
    
        typedef pu_cpp_int< 1 > phv_no_data_flop_ff_undflow_enable_cpp_int_t;
        cpp_int int_var__phv_no_data_flop_ff_undflow_enable;
        void phv_no_data_flop_ff_undflow_enable (const cpp_int  & l__val);
        cpp_int phv_no_data_flop_ff_undflow_enable() const;
    
        typedef pu_cpp_int< 1 > ptr_lookahaed_flop_ff_undflow_enable_cpp_int_t;
        cpp_int int_var__ptr_lookahaed_flop_ff_undflow_enable;
        void ptr_lookahaed_flop_ff_undflow_enable (const cpp_int  & l__val);
        cpp_int ptr_lookahaed_flop_ff_undflow_enable() const;
    
        typedef pu_cpp_int< 1 > eop_vld_flop_ff_undflow_enable_cpp_int_t;
        cpp_int int_var__eop_vld_flop_ff_undflow_enable;
        void eop_vld_flop_ff_undflow_enable (const cpp_int  & l__val);
        cpp_int eop_vld_flop_ff_undflow_enable() const;
    
        typedef pu_cpp_int< 1 > csum_cal_vld_flop_ff_undflow_enable_cpp_int_t;
        cpp_int int_var__csum_cal_vld_flop_ff_undflow_enable;
        void csum_cal_vld_flop_ff_undflow_enable (const cpp_int  & l__val);
        cpp_int csum_cal_vld_flop_ff_undflow_enable() const;
    
}; // cap_dpr_csr_int_flop_fifo_1_int_enable_clear_t
    
class cap_dpr_csr_int_flop_fifo_1_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_int_flop_fifo_1_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_int_flop_fifo_1_int_test_set_t(string name = "cap_dpr_csr_int_flop_fifo_1_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_int_flop_fifo_1_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > data_mux_force_bypass_crc_flop_ff_undflow_interrupt_cpp_int_t;
        cpp_int int_var__data_mux_force_bypass_crc_flop_ff_undflow_interrupt;
        void data_mux_force_bypass_crc_flop_ff_undflow_interrupt (const cpp_int  & l__val);
        cpp_int data_mux_force_bypass_crc_flop_ff_undflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > dpr_crc_info_flop_ff_undflow_interrupt_cpp_int_t;
        cpp_int int_var__dpr_crc_info_flop_ff_undflow_interrupt;
        void dpr_crc_info_flop_ff_undflow_interrupt (const cpp_int  & l__val);
        cpp_int dpr_crc_info_flop_ff_undflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > dpr_crc_update_info_flop_ff_undflow_interrupt_cpp_int_t;
        cpp_int int_var__dpr_crc_update_info_flop_ff_undflow_interrupt;
        void dpr_crc_update_info_flop_ff_undflow_interrupt (const cpp_int  & l__val);
        cpp_int dpr_crc_update_info_flop_ff_undflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > dpr_csum_info_flop_ff_undflow_interrupt_cpp_int_t;
        cpp_int int_var__dpr_csum_info_flop_ff_undflow_interrupt;
        void dpr_csum_info_flop_ff_undflow_interrupt (const cpp_int  & l__val);
        cpp_int dpr_csum_info_flop_ff_undflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > data_mux_force_bypass_csum_flop_ff_undflow_interrupt_cpp_int_t;
        cpp_int int_var__data_mux_force_bypass_csum_flop_ff_undflow_interrupt;
        void data_mux_force_bypass_csum_flop_ff_undflow_interrupt (const cpp_int  & l__val);
        cpp_int data_mux_force_bypass_csum_flop_ff_undflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > dpr_csum_update_info_flop_ff_undflow_interrupt_cpp_int_t;
        cpp_int int_var__dpr_csum_update_info_flop_ff_undflow_interrupt;
        void dpr_csum_update_info_flop_ff_undflow_interrupt (const cpp_int  & l__val);
        cpp_int dpr_csum_update_info_flop_ff_undflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > ptr_early_pkt_eop_info_flop_ff_undflow_interrupt_cpp_int_t;
        cpp_int int_var__ptr_early_pkt_eop_info_flop_ff_undflow_interrupt;
        void ptr_early_pkt_eop_info_flop_ff_undflow_interrupt (const cpp_int  & l__val);
        cpp_int ptr_early_pkt_eop_info_flop_ff_undflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > data_mux_eop_err_flop_ff_undflow_interrupt_cpp_int_t;
        cpp_int int_var__data_mux_eop_err_flop_ff_undflow_interrupt;
        void data_mux_eop_err_flop_ff_undflow_interrupt (const cpp_int  & l__val);
        cpp_int data_mux_eop_err_flop_ff_undflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pktin_eop_err_flop_ff_undflow_interrupt_cpp_int_t;
        cpp_int int_var__pktin_eop_err_flop_ff_undflow_interrupt;
        void pktin_eop_err_flop_ff_undflow_interrupt (const cpp_int  & l__val);
        cpp_int pktin_eop_err_flop_ff_undflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > csum_err_flop_ff_undflow_interrupt_cpp_int_t;
        cpp_int int_var__csum_err_flop_ff_undflow_interrupt;
        void csum_err_flop_ff_undflow_interrupt (const cpp_int  & l__val);
        cpp_int csum_err_flop_ff_undflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > crc_err_flop_ff_undflow_interrupt_cpp_int_t;
        cpp_int int_var__crc_err_flop_ff_undflow_interrupt;
        void crc_err_flop_ff_undflow_interrupt (const cpp_int  & l__val);
        cpp_int crc_err_flop_ff_undflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > data_mux_drop_flop_ff_undflow_interrupt_cpp_int_t;
        cpp_int int_var__data_mux_drop_flop_ff_undflow_interrupt;
        void data_mux_drop_flop_ff_undflow_interrupt (const cpp_int  & l__val);
        cpp_int data_mux_drop_flop_ff_undflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > phv_pkt_data_flop_ff_undflow_interrupt_cpp_int_t;
        cpp_int int_var__phv_pkt_data_flop_ff_undflow_interrupt;
        void phv_pkt_data_flop_ff_undflow_interrupt (const cpp_int  & l__val);
        cpp_int phv_pkt_data_flop_ff_undflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pktout_len_cell_flop_ff_undflow_interrupt_cpp_int_t;
        cpp_int int_var__pktout_len_cell_flop_ff_undflow_interrupt;
        void pktout_len_cell_flop_ff_undflow_interrupt (const cpp_int  & l__val);
        cpp_int pktout_len_cell_flop_ff_undflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > padding_size_flop_ff_undflow_interrupt_cpp_int_t;
        cpp_int int_var__padding_size_flop_ff_undflow_interrupt;
        void padding_size_flop_ff_undflow_interrupt (const cpp_int  & l__val);
        cpp_int padding_size_flop_ff_undflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pktin_err_flop_ff_undflow_interrupt_cpp_int_t;
        cpp_int int_var__pktin_err_flop_ff_undflow_interrupt;
        void pktin_err_flop_ff_undflow_interrupt (const cpp_int  & l__val);
        cpp_int pktin_err_flop_ff_undflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > phv_no_data_flop_ff_undflow_interrupt_cpp_int_t;
        cpp_int int_var__phv_no_data_flop_ff_undflow_interrupt;
        void phv_no_data_flop_ff_undflow_interrupt (const cpp_int  & l__val);
        cpp_int phv_no_data_flop_ff_undflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > ptr_lookahaed_flop_ff_undflow_interrupt_cpp_int_t;
        cpp_int int_var__ptr_lookahaed_flop_ff_undflow_interrupt;
        void ptr_lookahaed_flop_ff_undflow_interrupt (const cpp_int  & l__val);
        cpp_int ptr_lookahaed_flop_ff_undflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_vld_flop_ff_undflow_interrupt_cpp_int_t;
        cpp_int int_var__eop_vld_flop_ff_undflow_interrupt;
        void eop_vld_flop_ff_undflow_interrupt (const cpp_int  & l__val);
        cpp_int eop_vld_flop_ff_undflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > csum_cal_vld_flop_ff_undflow_interrupt_cpp_int_t;
        cpp_int int_var__csum_cal_vld_flop_ff_undflow_interrupt;
        void csum_cal_vld_flop_ff_undflow_interrupt (const cpp_int  & l__val);
        cpp_int csum_cal_vld_flop_ff_undflow_interrupt() const;
    
}; // cap_dpr_csr_int_flop_fifo_1_int_test_set_t
    
class cap_dpr_csr_int_flop_fifo_1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_int_flop_fifo_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_int_flop_fifo_1_t(string name = "cap_dpr_csr_int_flop_fifo_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_int_flop_fifo_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_dpr_csr_int_flop_fifo_1_int_test_set_t intreg;
    
        cap_dpr_csr_int_flop_fifo_1_int_test_set_t int_test_set;
    
        cap_dpr_csr_int_flop_fifo_1_int_enable_clear_t int_enable_set;
    
        cap_dpr_csr_int_flop_fifo_1_int_enable_clear_t int_enable_clear;
    
}; // cap_dpr_csr_int_flop_fifo_1_t
    
class cap_dpr_csr_int_flop_fifo_0_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_int_flop_fifo_0_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_int_flop_fifo_0_int_enable_clear_t(string name = "cap_dpr_csr_int_flop_fifo_0_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_int_flop_fifo_0_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > data_mux_force_bypass_crc_flop_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__data_mux_force_bypass_crc_flop_ff_ovflow_enable;
        void data_mux_force_bypass_crc_flop_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int data_mux_force_bypass_crc_flop_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > dpr_crc_info_flop_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__dpr_crc_info_flop_ff_ovflow_enable;
        void dpr_crc_info_flop_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int dpr_crc_info_flop_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > dpr_crc_update_info_flop_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__dpr_crc_update_info_flop_ff_ovflow_enable;
        void dpr_crc_update_info_flop_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int dpr_crc_update_info_flop_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > dpr_csum_info_flop_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__dpr_csum_info_flop_ff_ovflow_enable;
        void dpr_csum_info_flop_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int dpr_csum_info_flop_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > data_mux_force_bypass_csum_flop_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__data_mux_force_bypass_csum_flop_ff_ovflow_enable;
        void data_mux_force_bypass_csum_flop_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int data_mux_force_bypass_csum_flop_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > dpr_csum_update_info_flop_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__dpr_csum_update_info_flop_ff_ovflow_enable;
        void dpr_csum_update_info_flop_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int dpr_csum_update_info_flop_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > ptr_early_pkt_eop_info_flop_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__ptr_early_pkt_eop_info_flop_ff_ovflow_enable;
        void ptr_early_pkt_eop_info_flop_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int ptr_early_pkt_eop_info_flop_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > data_mux_eop_err_flop_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__data_mux_eop_err_flop_ff_ovflow_enable;
        void data_mux_eop_err_flop_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int data_mux_eop_err_flop_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pktin_eop_err_flop_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pktin_eop_err_flop_ff_ovflow_enable;
        void pktin_eop_err_flop_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pktin_eop_err_flop_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > csum_err_flop_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__csum_err_flop_ff_ovflow_enable;
        void csum_err_flop_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int csum_err_flop_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > crc_err_flop_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__crc_err_flop_ff_ovflow_enable;
        void crc_err_flop_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int crc_err_flop_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > data_mux_drop_flop_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__data_mux_drop_flop_ff_ovflow_enable;
        void data_mux_drop_flop_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int data_mux_drop_flop_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > phv_pkt_data_flop_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__phv_pkt_data_flop_ff_ovflow_enable;
        void phv_pkt_data_flop_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int phv_pkt_data_flop_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pktout_len_cell_flop_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pktout_len_cell_flop_ff_ovflow_enable;
        void pktout_len_cell_flop_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pktout_len_cell_flop_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > padding_size_flop_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__padding_size_flop_ff_ovflow_enable;
        void padding_size_flop_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int padding_size_flop_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pktin_err_flop_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pktin_err_flop_ff_ovflow_enable;
        void pktin_err_flop_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pktin_err_flop_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > phv_no_data_flop_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__phv_no_data_flop_ff_ovflow_enable;
        void phv_no_data_flop_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int phv_no_data_flop_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > ptr_lookahaed_flop_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__ptr_lookahaed_flop_ff_ovflow_enable;
        void ptr_lookahaed_flop_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int ptr_lookahaed_flop_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > eop_vld_flop_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__eop_vld_flop_ff_ovflow_enable;
        void eop_vld_flop_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int eop_vld_flop_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > csum_cal_vld_flop_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__csum_cal_vld_flop_ff_ovflow_enable;
        void csum_cal_vld_flop_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int csum_cal_vld_flop_ff_ovflow_enable() const;
    
}; // cap_dpr_csr_int_flop_fifo_0_int_enable_clear_t
    
class cap_dpr_csr_int_flop_fifo_0_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_int_flop_fifo_0_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_int_flop_fifo_0_int_test_set_t(string name = "cap_dpr_csr_int_flop_fifo_0_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_int_flop_fifo_0_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > data_mux_force_bypass_crc_flop_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__data_mux_force_bypass_crc_flop_ff_ovflow_interrupt;
        void data_mux_force_bypass_crc_flop_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int data_mux_force_bypass_crc_flop_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > dpr_crc_info_flop_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__dpr_crc_info_flop_ff_ovflow_interrupt;
        void dpr_crc_info_flop_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int dpr_crc_info_flop_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > dpr_crc_update_info_flop_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__dpr_crc_update_info_flop_ff_ovflow_interrupt;
        void dpr_crc_update_info_flop_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int dpr_crc_update_info_flop_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > dpr_csum_info_flop_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__dpr_csum_info_flop_ff_ovflow_interrupt;
        void dpr_csum_info_flop_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int dpr_csum_info_flop_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > data_mux_force_bypass_csum_flop_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__data_mux_force_bypass_csum_flop_ff_ovflow_interrupt;
        void data_mux_force_bypass_csum_flop_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int data_mux_force_bypass_csum_flop_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > dpr_csum_update_info_flop_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__dpr_csum_update_info_flop_ff_ovflow_interrupt;
        void dpr_csum_update_info_flop_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int dpr_csum_update_info_flop_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > ptr_early_pkt_eop_info_flop_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__ptr_early_pkt_eop_info_flop_ff_ovflow_interrupt;
        void ptr_early_pkt_eop_info_flop_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int ptr_early_pkt_eop_info_flop_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > data_mux_eop_err_flop_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__data_mux_eop_err_flop_ff_ovflow_interrupt;
        void data_mux_eop_err_flop_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int data_mux_eop_err_flop_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pktin_eop_err_flop_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pktin_eop_err_flop_ff_ovflow_interrupt;
        void pktin_eop_err_flop_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pktin_eop_err_flop_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > csum_err_flop_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__csum_err_flop_ff_ovflow_interrupt;
        void csum_err_flop_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int csum_err_flop_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > crc_err_flop_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__crc_err_flop_ff_ovflow_interrupt;
        void crc_err_flop_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int crc_err_flop_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > data_mux_drop_flop_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__data_mux_drop_flop_ff_ovflow_interrupt;
        void data_mux_drop_flop_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int data_mux_drop_flop_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > phv_pkt_data_flop_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__phv_pkt_data_flop_ff_ovflow_interrupt;
        void phv_pkt_data_flop_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int phv_pkt_data_flop_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pktout_len_cell_flop_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pktout_len_cell_flop_ff_ovflow_interrupt;
        void pktout_len_cell_flop_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pktout_len_cell_flop_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > padding_size_flop_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__padding_size_flop_ff_ovflow_interrupt;
        void padding_size_flop_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int padding_size_flop_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pktin_err_flop_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pktin_err_flop_ff_ovflow_interrupt;
        void pktin_err_flop_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pktin_err_flop_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > phv_no_data_flop_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__phv_no_data_flop_ff_ovflow_interrupt;
        void phv_no_data_flop_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int phv_no_data_flop_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > ptr_lookahaed_flop_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__ptr_lookahaed_flop_ff_ovflow_interrupt;
        void ptr_lookahaed_flop_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int ptr_lookahaed_flop_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_vld_flop_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__eop_vld_flop_ff_ovflow_interrupt;
        void eop_vld_flop_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int eop_vld_flop_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > csum_cal_vld_flop_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__csum_cal_vld_flop_ff_ovflow_interrupt;
        void csum_cal_vld_flop_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int csum_cal_vld_flop_ff_ovflow_interrupt() const;
    
}; // cap_dpr_csr_int_flop_fifo_0_int_test_set_t
    
class cap_dpr_csr_int_flop_fifo_0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_int_flop_fifo_0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_int_flop_fifo_0_t(string name = "cap_dpr_csr_int_flop_fifo_0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_int_flop_fifo_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_dpr_csr_int_flop_fifo_0_int_test_set_t intreg;
    
        cap_dpr_csr_int_flop_fifo_0_int_test_set_t int_test_set;
    
        cap_dpr_csr_int_flop_fifo_0_int_enable_clear_t int_enable_set;
    
        cap_dpr_csr_int_flop_fifo_0_int_enable_clear_t int_enable_clear;
    
}; // cap_dpr_csr_int_flop_fifo_0_t
    
class cap_dpr_csr_int_fifo_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_int_fifo_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_int_fifo_int_enable_clear_t(string name = "cap_dpr_csr_int_fifo_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_int_fifo_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > phv_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__phv_ff_ovflow_enable;
        void phv_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int phv_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > ohi_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__ohi_ff_ovflow_enable;
        void ohi_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int ohi_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pktin_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pktin_ff_ovflow_enable;
        void pktin_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pktin_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pktout_ff_undflow_enable_cpp_int_t;
        cpp_int int_var__pktout_ff_undflow_enable;
        void pktout_ff_undflow_enable (const cpp_int  & l__val);
        cpp_int pktout_ff_undflow_enable() const;
    
        typedef pu_cpp_int< 1 > csum_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__csum_ff_ovflow_enable;
        void csum_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int csum_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > ptr_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__ptr_ff_ovflow_enable;
        void ptr_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int ptr_ff_ovflow_enable() const;
    
}; // cap_dpr_csr_int_fifo_int_enable_clear_t
    
class cap_dpr_csr_int_fifo_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_int_fifo_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_int_fifo_int_test_set_t(string name = "cap_dpr_csr_int_fifo_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_int_fifo_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > phv_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__phv_ff_ovflow_interrupt;
        void phv_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int phv_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > ohi_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__ohi_ff_ovflow_interrupt;
        void ohi_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int ohi_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pktin_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pktin_ff_ovflow_interrupt;
        void pktin_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pktin_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pktout_ff_undflow_interrupt_cpp_int_t;
        cpp_int int_var__pktout_ff_undflow_interrupt;
        void pktout_ff_undflow_interrupt (const cpp_int  & l__val);
        cpp_int pktout_ff_undflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > csum_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__csum_ff_ovflow_interrupt;
        void csum_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int csum_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > ptr_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__ptr_ff_ovflow_interrupt;
        void ptr_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int ptr_ff_ovflow_interrupt() const;
    
}; // cap_dpr_csr_int_fifo_int_test_set_t
    
class cap_dpr_csr_int_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_int_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_int_fifo_t(string name = "cap_dpr_csr_int_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_int_fifo_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_dpr_csr_int_fifo_int_test_set_t intreg;
    
        cap_dpr_csr_int_fifo_int_test_set_t int_test_set;
    
        cap_dpr_csr_int_fifo_int_enable_clear_t int_enable_set;
    
        cap_dpr_csr_int_fifo_int_enable_clear_t int_enable_clear;
    
}; // cap_dpr_csr_int_fifo_t
    
class cap_dpr_csr_int_reg2_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_int_reg2_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_int_reg2_int_enable_clear_t(string name = "cap_dpr_csr_int_reg2_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_int_reg2_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 1 > err_csum_start_gt_end_4_enable_cpp_int_t;
        cpp_int int_var__err_csum_start_gt_end_4_enable;
        void err_csum_start_gt_end_4_enable (const cpp_int  & l__val);
        cpp_int err_csum_start_gt_end_4_enable() const;
    
        typedef pu_cpp_int< 1 > err_csum_start_gt_end_3_enable_cpp_int_t;
        cpp_int int_var__err_csum_start_gt_end_3_enable;
        void err_csum_start_gt_end_3_enable (const cpp_int  & l__val);
        cpp_int err_csum_start_gt_end_3_enable() const;
    
        typedef pu_cpp_int< 1 > err_csum_start_gt_end_2_enable_cpp_int_t;
        cpp_int int_var__err_csum_start_gt_end_2_enable;
        void err_csum_start_gt_end_2_enable (const cpp_int  & l__val);
        cpp_int err_csum_start_gt_end_2_enable() const;
    
        typedef pu_cpp_int< 1 > err_csum_start_gt_end_1_enable_cpp_int_t;
        cpp_int int_var__err_csum_start_gt_end_1_enable;
        void err_csum_start_gt_end_1_enable (const cpp_int  & l__val);
        cpp_int err_csum_start_gt_end_1_enable() const;
    
        typedef pu_cpp_int< 1 > err_csum_start_gt_end_0_enable_cpp_int_t;
        cpp_int int_var__err_csum_start_gt_end_0_enable;
        void err_csum_start_gt_end_0_enable (const cpp_int  & l__val);
        cpp_int err_csum_start_gt_end_0_enable() const;
    
        typedef pu_cpp_int< 1 > err_crc_start_gt_end_enable_cpp_int_t;
        cpp_int int_var__err_crc_start_gt_end_enable;
        void err_crc_start_gt_end_enable (const cpp_int  & l__val);
        cpp_int err_crc_start_gt_end_enable() const;
    
}; // cap_dpr_csr_int_reg2_int_enable_clear_t
    
class cap_dpr_csr_int_reg2_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_int_reg2_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_int_reg2_int_test_set_t(string name = "cap_dpr_csr_int_reg2_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_int_reg2_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 1 > err_csum_start_gt_end_4_interrupt_cpp_int_t;
        cpp_int int_var__err_csum_start_gt_end_4_interrupt;
        void err_csum_start_gt_end_4_interrupt (const cpp_int  & l__val);
        cpp_int err_csum_start_gt_end_4_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_csum_start_gt_end_3_interrupt_cpp_int_t;
        cpp_int int_var__err_csum_start_gt_end_3_interrupt;
        void err_csum_start_gt_end_3_interrupt (const cpp_int  & l__val);
        cpp_int err_csum_start_gt_end_3_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_csum_start_gt_end_2_interrupt_cpp_int_t;
        cpp_int int_var__err_csum_start_gt_end_2_interrupt;
        void err_csum_start_gt_end_2_interrupt (const cpp_int  & l__val);
        cpp_int err_csum_start_gt_end_2_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_csum_start_gt_end_1_interrupt_cpp_int_t;
        cpp_int int_var__err_csum_start_gt_end_1_interrupt;
        void err_csum_start_gt_end_1_interrupt (const cpp_int  & l__val);
        cpp_int err_csum_start_gt_end_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_csum_start_gt_end_0_interrupt_cpp_int_t;
        cpp_int int_var__err_csum_start_gt_end_0_interrupt;
        void err_csum_start_gt_end_0_interrupt (const cpp_int  & l__val);
        cpp_int err_csum_start_gt_end_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_crc_start_gt_end_interrupt_cpp_int_t;
        cpp_int int_var__err_crc_start_gt_end_interrupt;
        void err_crc_start_gt_end_interrupt (const cpp_int  & l__val);
        cpp_int err_crc_start_gt_end_interrupt() const;
    
}; // cap_dpr_csr_int_reg2_int_test_set_t
    
class cap_dpr_csr_int_reg2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_int_reg2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_int_reg2_t(string name = "cap_dpr_csr_int_reg2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_int_reg2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_dpr_csr_int_reg2_int_test_set_t intreg;
    
        cap_dpr_csr_int_reg2_int_test_set_t int_test_set;
    
        cap_dpr_csr_int_reg2_int_enable_clear_t int_enable_set;
    
        cap_dpr_csr_int_reg2_int_enable_clear_t int_enable_clear;
    
}; // cap_dpr_csr_int_reg2_t
    
class cap_dpr_csr_int_reg1_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_int_reg1_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_int_reg1_int_enable_clear_t(string name = "cap_dpr_csr_int_reg1_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_int_reg1_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 1 > err_pktin_sop_no_eop_enable_cpp_int_t;
        cpp_int int_var__err_pktin_sop_no_eop_enable;
        void err_pktin_sop_no_eop_enable (const cpp_int  & l__val);
        cpp_int err_pktin_sop_no_eop_enable() const;
    
        typedef pu_cpp_int< 1 > err_pktin_eop_no_sop_enable_cpp_int_t;
        cpp_int int_var__err_pktin_eop_no_sop_enable;
        void err_pktin_eop_no_sop_enable (const cpp_int  & l__val);
        cpp_int err_pktin_eop_no_sop_enable() const;
    
        typedef pu_cpp_int< 1 > err_csum_offset_gt_pkt_size_4_enable_cpp_int_t;
        cpp_int int_var__err_csum_offset_gt_pkt_size_4_enable;
        void err_csum_offset_gt_pkt_size_4_enable (const cpp_int  & l__val);
        cpp_int err_csum_offset_gt_pkt_size_4_enable() const;
    
        typedef pu_cpp_int< 1 > err_csum_offset_gt_pkt_size_3_enable_cpp_int_t;
        cpp_int int_var__err_csum_offset_gt_pkt_size_3_enable;
        void err_csum_offset_gt_pkt_size_3_enable (const cpp_int  & l__val);
        cpp_int err_csum_offset_gt_pkt_size_3_enable() const;
    
        typedef pu_cpp_int< 1 > err_csum_offset_gt_pkt_size_2_enable_cpp_int_t;
        cpp_int int_var__err_csum_offset_gt_pkt_size_2_enable;
        void err_csum_offset_gt_pkt_size_2_enable (const cpp_int  & l__val);
        cpp_int err_csum_offset_gt_pkt_size_2_enable() const;
    
        typedef pu_cpp_int< 1 > err_csum_offset_gt_pkt_size_1_enable_cpp_int_t;
        cpp_int int_var__err_csum_offset_gt_pkt_size_1_enable;
        void err_csum_offset_gt_pkt_size_1_enable (const cpp_int  & l__val);
        cpp_int err_csum_offset_gt_pkt_size_1_enable() const;
    
        typedef pu_cpp_int< 1 > err_csum_offset_gt_pkt_size_0_enable_cpp_int_t;
        cpp_int int_var__err_csum_offset_gt_pkt_size_0_enable;
        void err_csum_offset_gt_pkt_size_0_enable (const cpp_int  & l__val);
        cpp_int err_csum_offset_gt_pkt_size_0_enable() const;
    
        typedef pu_cpp_int< 1 > err_csum_phdr_offset_gt_pkt_size_4_enable_cpp_int_t;
        cpp_int int_var__err_csum_phdr_offset_gt_pkt_size_4_enable;
        void err_csum_phdr_offset_gt_pkt_size_4_enable (const cpp_int  & l__val);
        cpp_int err_csum_phdr_offset_gt_pkt_size_4_enable() const;
    
        typedef pu_cpp_int< 1 > err_csum_phdr_offset_gt_pkt_size_3_enable_cpp_int_t;
        cpp_int int_var__err_csum_phdr_offset_gt_pkt_size_3_enable;
        void err_csum_phdr_offset_gt_pkt_size_3_enable (const cpp_int  & l__val);
        cpp_int err_csum_phdr_offset_gt_pkt_size_3_enable() const;
    
        typedef pu_cpp_int< 1 > err_csum_phdr_offset_gt_pkt_size_2_enable_cpp_int_t;
        cpp_int int_var__err_csum_phdr_offset_gt_pkt_size_2_enable;
        void err_csum_phdr_offset_gt_pkt_size_2_enable (const cpp_int  & l__val);
        cpp_int err_csum_phdr_offset_gt_pkt_size_2_enable() const;
    
        typedef pu_cpp_int< 1 > err_csum_phdr_offset_gt_pkt_size_1_enable_cpp_int_t;
        cpp_int int_var__err_csum_phdr_offset_gt_pkt_size_1_enable;
        void err_csum_phdr_offset_gt_pkt_size_1_enable (const cpp_int  & l__val);
        cpp_int err_csum_phdr_offset_gt_pkt_size_1_enable() const;
    
        typedef pu_cpp_int< 1 > err_csum_phdr_offset_gt_pkt_size_0_enable_cpp_int_t;
        cpp_int int_var__err_csum_phdr_offset_gt_pkt_size_0_enable;
        void err_csum_phdr_offset_gt_pkt_size_0_enable (const cpp_int  & l__val);
        cpp_int err_csum_phdr_offset_gt_pkt_size_0_enable() const;
    
        typedef pu_cpp_int< 1 > err_csum_loc_gt_pkt_size_4_enable_cpp_int_t;
        cpp_int int_var__err_csum_loc_gt_pkt_size_4_enable;
        void err_csum_loc_gt_pkt_size_4_enable (const cpp_int  & l__val);
        cpp_int err_csum_loc_gt_pkt_size_4_enable() const;
    
        typedef pu_cpp_int< 1 > err_csum_loc_gt_pkt_size_3_enable_cpp_int_t;
        cpp_int int_var__err_csum_loc_gt_pkt_size_3_enable;
        void err_csum_loc_gt_pkt_size_3_enable (const cpp_int  & l__val);
        cpp_int err_csum_loc_gt_pkt_size_3_enable() const;
    
        typedef pu_cpp_int< 1 > err_csum_loc_gt_pkt_size_2_enable_cpp_int_t;
        cpp_int int_var__err_csum_loc_gt_pkt_size_2_enable;
        void err_csum_loc_gt_pkt_size_2_enable (const cpp_int  & l__val);
        cpp_int err_csum_loc_gt_pkt_size_2_enable() const;
    
        typedef pu_cpp_int< 1 > err_csum_loc_gt_pkt_size_1_enable_cpp_int_t;
        cpp_int int_var__err_csum_loc_gt_pkt_size_1_enable;
        void err_csum_loc_gt_pkt_size_1_enable (const cpp_int  & l__val);
        cpp_int err_csum_loc_gt_pkt_size_1_enable() const;
    
        typedef pu_cpp_int< 1 > err_csum_loc_gt_pkt_size_0_enable_cpp_int_t;
        cpp_int int_var__err_csum_loc_gt_pkt_size_0_enable;
        void err_csum_loc_gt_pkt_size_0_enable (const cpp_int  & l__val);
        cpp_int err_csum_loc_gt_pkt_size_0_enable() const;
    
        typedef pu_cpp_int< 1 > err_crc_offset_gt_pkt_size_enable_cpp_int_t;
        cpp_int int_var__err_crc_offset_gt_pkt_size_enable;
        void err_crc_offset_gt_pkt_size_enable (const cpp_int  & l__val);
        cpp_int err_crc_offset_gt_pkt_size_enable() const;
    
        typedef pu_cpp_int< 1 > err_crc_loc_gt_pkt_size_enable_cpp_int_t;
        cpp_int int_var__err_crc_loc_gt_pkt_size_enable;
        void err_crc_loc_gt_pkt_size_enable (const cpp_int  & l__val);
        cpp_int err_crc_loc_gt_pkt_size_enable() const;
    
        typedef pu_cpp_int< 1 > err_crc_mask_offset_gt_pkt_size_enable_cpp_int_t;
        cpp_int int_var__err_crc_mask_offset_gt_pkt_size_enable;
        void err_crc_mask_offset_gt_pkt_size_enable (const cpp_int  & l__val);
        cpp_int err_crc_mask_offset_gt_pkt_size_enable() const;
    
        typedef pu_cpp_int< 1 > err_pkt_eop_early_enable_cpp_int_t;
        cpp_int int_var__err_pkt_eop_early_enable;
        void err_pkt_eop_early_enable (const cpp_int  & l__val);
        cpp_int err_pkt_eop_early_enable() const;
    
        typedef pu_cpp_int< 1 > err_ptr_ff_overflow_enable_cpp_int_t;
        cpp_int int_var__err_ptr_ff_overflow_enable;
        void err_ptr_ff_overflow_enable (const cpp_int  & l__val);
        cpp_int err_ptr_ff_overflow_enable() const;
    
        typedef pu_cpp_int< 1 > err_csum_ff_overflow_enable_cpp_int_t;
        cpp_int int_var__err_csum_ff_overflow_enable;
        void err_csum_ff_overflow_enable (const cpp_int  & l__val);
        cpp_int err_csum_ff_overflow_enable() const;
    
        typedef pu_cpp_int< 1 > err_pktout_ff_overflow_enable_cpp_int_t;
        cpp_int int_var__err_pktout_ff_overflow_enable;
        void err_pktout_ff_overflow_enable (const cpp_int  & l__val);
        cpp_int err_pktout_ff_overflow_enable() const;
    
        typedef pu_cpp_int< 1 > err_ptr_from_cfg_overflow_enable_cpp_int_t;
        cpp_int int_var__err_ptr_from_cfg_overflow_enable;
        void err_ptr_from_cfg_overflow_enable (const cpp_int  & l__val);
        cpp_int err_ptr_from_cfg_overflow_enable() const;
    
}; // cap_dpr_csr_int_reg1_int_enable_clear_t
    
class cap_dpr_csr_int_reg1_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_int_reg1_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_int_reg1_int_test_set_t(string name = "cap_dpr_csr_int_reg1_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_int_reg1_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 1 > err_pktin_sop_no_eop_interrupt_cpp_int_t;
        cpp_int int_var__err_pktin_sop_no_eop_interrupt;
        void err_pktin_sop_no_eop_interrupt (const cpp_int  & l__val);
        cpp_int err_pktin_sop_no_eop_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_pktin_eop_no_sop_interrupt_cpp_int_t;
        cpp_int int_var__err_pktin_eop_no_sop_interrupt;
        void err_pktin_eop_no_sop_interrupt (const cpp_int  & l__val);
        cpp_int err_pktin_eop_no_sop_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_csum_offset_gt_pkt_size_4_interrupt_cpp_int_t;
        cpp_int int_var__err_csum_offset_gt_pkt_size_4_interrupt;
        void err_csum_offset_gt_pkt_size_4_interrupt (const cpp_int  & l__val);
        cpp_int err_csum_offset_gt_pkt_size_4_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_csum_offset_gt_pkt_size_3_interrupt_cpp_int_t;
        cpp_int int_var__err_csum_offset_gt_pkt_size_3_interrupt;
        void err_csum_offset_gt_pkt_size_3_interrupt (const cpp_int  & l__val);
        cpp_int err_csum_offset_gt_pkt_size_3_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_csum_offset_gt_pkt_size_2_interrupt_cpp_int_t;
        cpp_int int_var__err_csum_offset_gt_pkt_size_2_interrupt;
        void err_csum_offset_gt_pkt_size_2_interrupt (const cpp_int  & l__val);
        cpp_int err_csum_offset_gt_pkt_size_2_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_csum_offset_gt_pkt_size_1_interrupt_cpp_int_t;
        cpp_int int_var__err_csum_offset_gt_pkt_size_1_interrupt;
        void err_csum_offset_gt_pkt_size_1_interrupt (const cpp_int  & l__val);
        cpp_int err_csum_offset_gt_pkt_size_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_csum_offset_gt_pkt_size_0_interrupt_cpp_int_t;
        cpp_int int_var__err_csum_offset_gt_pkt_size_0_interrupt;
        void err_csum_offset_gt_pkt_size_0_interrupt (const cpp_int  & l__val);
        cpp_int err_csum_offset_gt_pkt_size_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_csum_phdr_offset_gt_pkt_size_4_interrupt_cpp_int_t;
        cpp_int int_var__err_csum_phdr_offset_gt_pkt_size_4_interrupt;
        void err_csum_phdr_offset_gt_pkt_size_4_interrupt (const cpp_int  & l__val);
        cpp_int err_csum_phdr_offset_gt_pkt_size_4_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_csum_phdr_offset_gt_pkt_size_3_interrupt_cpp_int_t;
        cpp_int int_var__err_csum_phdr_offset_gt_pkt_size_3_interrupt;
        void err_csum_phdr_offset_gt_pkt_size_3_interrupt (const cpp_int  & l__val);
        cpp_int err_csum_phdr_offset_gt_pkt_size_3_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_csum_phdr_offset_gt_pkt_size_2_interrupt_cpp_int_t;
        cpp_int int_var__err_csum_phdr_offset_gt_pkt_size_2_interrupt;
        void err_csum_phdr_offset_gt_pkt_size_2_interrupt (const cpp_int  & l__val);
        cpp_int err_csum_phdr_offset_gt_pkt_size_2_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_csum_phdr_offset_gt_pkt_size_1_interrupt_cpp_int_t;
        cpp_int int_var__err_csum_phdr_offset_gt_pkt_size_1_interrupt;
        void err_csum_phdr_offset_gt_pkt_size_1_interrupt (const cpp_int  & l__val);
        cpp_int err_csum_phdr_offset_gt_pkt_size_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_csum_phdr_offset_gt_pkt_size_0_interrupt_cpp_int_t;
        cpp_int int_var__err_csum_phdr_offset_gt_pkt_size_0_interrupt;
        void err_csum_phdr_offset_gt_pkt_size_0_interrupt (const cpp_int  & l__val);
        cpp_int err_csum_phdr_offset_gt_pkt_size_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_csum_loc_gt_pkt_size_4_interrupt_cpp_int_t;
        cpp_int int_var__err_csum_loc_gt_pkt_size_4_interrupt;
        void err_csum_loc_gt_pkt_size_4_interrupt (const cpp_int  & l__val);
        cpp_int err_csum_loc_gt_pkt_size_4_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_csum_loc_gt_pkt_size_3_interrupt_cpp_int_t;
        cpp_int int_var__err_csum_loc_gt_pkt_size_3_interrupt;
        void err_csum_loc_gt_pkt_size_3_interrupt (const cpp_int  & l__val);
        cpp_int err_csum_loc_gt_pkt_size_3_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_csum_loc_gt_pkt_size_2_interrupt_cpp_int_t;
        cpp_int int_var__err_csum_loc_gt_pkt_size_2_interrupt;
        void err_csum_loc_gt_pkt_size_2_interrupt (const cpp_int  & l__val);
        cpp_int err_csum_loc_gt_pkt_size_2_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_csum_loc_gt_pkt_size_1_interrupt_cpp_int_t;
        cpp_int int_var__err_csum_loc_gt_pkt_size_1_interrupt;
        void err_csum_loc_gt_pkt_size_1_interrupt (const cpp_int  & l__val);
        cpp_int err_csum_loc_gt_pkt_size_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_csum_loc_gt_pkt_size_0_interrupt_cpp_int_t;
        cpp_int int_var__err_csum_loc_gt_pkt_size_0_interrupt;
        void err_csum_loc_gt_pkt_size_0_interrupt (const cpp_int  & l__val);
        cpp_int err_csum_loc_gt_pkt_size_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_crc_offset_gt_pkt_size_interrupt_cpp_int_t;
        cpp_int int_var__err_crc_offset_gt_pkt_size_interrupt;
        void err_crc_offset_gt_pkt_size_interrupt (const cpp_int  & l__val);
        cpp_int err_crc_offset_gt_pkt_size_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_crc_loc_gt_pkt_size_interrupt_cpp_int_t;
        cpp_int int_var__err_crc_loc_gt_pkt_size_interrupt;
        void err_crc_loc_gt_pkt_size_interrupt (const cpp_int  & l__val);
        cpp_int err_crc_loc_gt_pkt_size_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_crc_mask_offset_gt_pkt_size_interrupt_cpp_int_t;
        cpp_int int_var__err_crc_mask_offset_gt_pkt_size_interrupt;
        void err_crc_mask_offset_gt_pkt_size_interrupt (const cpp_int  & l__val);
        cpp_int err_crc_mask_offset_gt_pkt_size_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_pkt_eop_early_interrupt_cpp_int_t;
        cpp_int int_var__err_pkt_eop_early_interrupt;
        void err_pkt_eop_early_interrupt (const cpp_int  & l__val);
        cpp_int err_pkt_eop_early_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_ptr_ff_overflow_interrupt_cpp_int_t;
        cpp_int int_var__err_ptr_ff_overflow_interrupt;
        void err_ptr_ff_overflow_interrupt (const cpp_int  & l__val);
        cpp_int err_ptr_ff_overflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_csum_ff_overflow_interrupt_cpp_int_t;
        cpp_int int_var__err_csum_ff_overflow_interrupt;
        void err_csum_ff_overflow_interrupt (const cpp_int  & l__val);
        cpp_int err_csum_ff_overflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_pktout_ff_overflow_interrupt_cpp_int_t;
        cpp_int int_var__err_pktout_ff_overflow_interrupt;
        void err_pktout_ff_overflow_interrupt (const cpp_int  & l__val);
        cpp_int err_pktout_ff_overflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > err_ptr_from_cfg_overflow_interrupt_cpp_int_t;
        cpp_int int_var__err_ptr_from_cfg_overflow_interrupt;
        void err_ptr_from_cfg_overflow_interrupt (const cpp_int  & l__val);
        cpp_int err_ptr_from_cfg_overflow_interrupt() const;
    
}; // cap_dpr_csr_int_reg1_int_test_set_t
    
class cap_dpr_csr_int_reg1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_int_reg1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_int_reg1_t(string name = "cap_dpr_csr_int_reg1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_int_reg1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_dpr_csr_int_reg1_int_test_set_t intreg;
    
        cap_dpr_csr_int_reg1_int_test_set_t int_test_set;
    
        cap_dpr_csr_int_reg1_int_enable_clear_t int_enable_set;
    
        cap_dpr_csr_int_reg1_int_enable_clear_t int_enable_clear;
    
}; // cap_dpr_csr_int_reg1_t
    
class cap_dpr_csr_intreg_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_intreg_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_intreg_status_t(string name = "cap_dpr_csr_intreg_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_intreg_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 1 > int_flop_fifo_0_interrupt_cpp_int_t;
        cpp_int int_var__int_flop_fifo_0_interrupt;
        void int_flop_fifo_0_interrupt (const cpp_int  & l__val);
        cpp_int int_flop_fifo_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_flop_fifo_1_interrupt_cpp_int_t;
        cpp_int int_var__int_flop_fifo_1_interrupt;
        void int_flop_fifo_1_interrupt (const cpp_int  & l__val);
        cpp_int int_flop_fifo_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_spare_interrupt_cpp_int_t;
        cpp_int int_var__int_spare_interrupt;
        void int_spare_interrupt (const cpp_int  & l__val);
        cpp_int int_spare_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_credit_interrupt_cpp_int_t;
        cpp_int int_var__int_credit_interrupt;
        void int_credit_interrupt (const cpp_int  & l__val);
        cpp_int int_credit_interrupt() const;
    
}; // cap_dpr_csr_intreg_status_t
    
class cap_dpr_csr_int_groups_int_enable_rw_reg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_int_groups_int_enable_rw_reg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_int_groups_int_enable_rw_reg_t(string name = "cap_dpr_csr_int_groups_int_enable_rw_reg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_int_groups_int_enable_rw_reg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 1 > int_flop_fifo_0_enable_cpp_int_t;
        cpp_int int_var__int_flop_fifo_0_enable;
        void int_flop_fifo_0_enable (const cpp_int  & l__val);
        cpp_int int_flop_fifo_0_enable() const;
    
        typedef pu_cpp_int< 1 > int_flop_fifo_1_enable_cpp_int_t;
        cpp_int int_var__int_flop_fifo_1_enable;
        void int_flop_fifo_1_enable (const cpp_int  & l__val);
        cpp_int int_flop_fifo_1_enable() const;
    
        typedef pu_cpp_int< 1 > int_spare_enable_cpp_int_t;
        cpp_int int_var__int_spare_enable;
        void int_spare_enable (const cpp_int  & l__val);
        cpp_int int_spare_enable() const;
    
        typedef pu_cpp_int< 1 > int_credit_enable_cpp_int_t;
        cpp_int int_var__int_credit_enable;
        void int_credit_enable (const cpp_int  & l__val);
        cpp_int int_credit_enable() const;
    
}; // cap_dpr_csr_int_groups_int_enable_rw_reg_t
    
class cap_dpr_csr_intgrp_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_intgrp_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_intgrp_status_t(string name = "cap_dpr_csr_intgrp_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_intgrp_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_dpr_csr_intreg_status_t intreg;
    
        cap_dpr_csr_int_groups_int_enable_rw_reg_t int_enable_rw_reg;
    
        cap_dpr_csr_intreg_status_t int_rw_reg;
    
}; // cap_dpr_csr_intgrp_status_t
    
class cap_dpr_csr_int_srams_ecc_int_enable_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_int_srams_ecc_int_enable_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_int_srams_ecc_int_enable_set_t(string name = "cap_dpr_csr_int_srams_ecc_int_enable_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_int_srams_ecc_int_enable_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > dpr_pktin_fifo_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__dpr_pktin_fifo_uncorrectable_enable;
        void dpr_pktin_fifo_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int dpr_pktin_fifo_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > dpr_pktin_fifo_correctable_enable_cpp_int_t;
        cpp_int int_var__dpr_pktin_fifo_correctable_enable;
        void dpr_pktin_fifo_correctable_enable (const cpp_int  & l__val);
        cpp_int dpr_pktin_fifo_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > dpr_csum_fifo_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__dpr_csum_fifo_uncorrectable_enable;
        void dpr_csum_fifo_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int dpr_csum_fifo_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > dpr_csum_fifo_correctable_enable_cpp_int_t;
        cpp_int int_var__dpr_csum_fifo_correctable_enable;
        void dpr_csum_fifo_correctable_enable (const cpp_int  & l__val);
        cpp_int dpr_csum_fifo_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > dpr_phv_fifo_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__dpr_phv_fifo_uncorrectable_enable;
        void dpr_phv_fifo_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int dpr_phv_fifo_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > dpr_phv_fifo_correctable_enable_cpp_int_t;
        cpp_int int_var__dpr_phv_fifo_correctable_enable;
        void dpr_phv_fifo_correctable_enable (const cpp_int  & l__val);
        cpp_int dpr_phv_fifo_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > dpr_ohi_fifo_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__dpr_ohi_fifo_uncorrectable_enable;
        void dpr_ohi_fifo_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int dpr_ohi_fifo_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > dpr_ohi_fifo_correctable_enable_cpp_int_t;
        cpp_int int_var__dpr_ohi_fifo_correctable_enable;
        void dpr_ohi_fifo_correctable_enable (const cpp_int  & l__val);
        cpp_int dpr_ohi_fifo_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > dpr_ptr_fifo_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__dpr_ptr_fifo_uncorrectable_enable;
        void dpr_ptr_fifo_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int dpr_ptr_fifo_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > dpr_ptr_fifo_correctable_enable_cpp_int_t;
        cpp_int int_var__dpr_ptr_fifo_correctable_enable;
        void dpr_ptr_fifo_correctable_enable (const cpp_int  & l__val);
        cpp_int dpr_ptr_fifo_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > dpr_pktout_fifo_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__dpr_pktout_fifo_uncorrectable_enable;
        void dpr_pktout_fifo_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int dpr_pktout_fifo_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > dpr_pktout_fifo_correctable_enable_cpp_int_t;
        cpp_int int_var__dpr_pktout_fifo_correctable_enable;
        void dpr_pktout_fifo_correctable_enable (const cpp_int  & l__val);
        cpp_int dpr_pktout_fifo_correctable_enable() const;
    
}; // cap_dpr_csr_int_srams_ecc_int_enable_set_t
    
class cap_dpr_csr_int_srams_ecc_intreg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_int_srams_ecc_intreg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_int_srams_ecc_intreg_t(string name = "cap_dpr_csr_int_srams_ecc_intreg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_int_srams_ecc_intreg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > dpr_pktin_fifo_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__dpr_pktin_fifo_uncorrectable_interrupt;
        void dpr_pktin_fifo_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int dpr_pktin_fifo_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > dpr_pktin_fifo_correctable_interrupt_cpp_int_t;
        cpp_int int_var__dpr_pktin_fifo_correctable_interrupt;
        void dpr_pktin_fifo_correctable_interrupt (const cpp_int  & l__val);
        cpp_int dpr_pktin_fifo_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > dpr_csum_fifo_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__dpr_csum_fifo_uncorrectable_interrupt;
        void dpr_csum_fifo_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int dpr_csum_fifo_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > dpr_csum_fifo_correctable_interrupt_cpp_int_t;
        cpp_int int_var__dpr_csum_fifo_correctable_interrupt;
        void dpr_csum_fifo_correctable_interrupt (const cpp_int  & l__val);
        cpp_int dpr_csum_fifo_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > dpr_phv_fifo_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__dpr_phv_fifo_uncorrectable_interrupt;
        void dpr_phv_fifo_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int dpr_phv_fifo_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > dpr_phv_fifo_correctable_interrupt_cpp_int_t;
        cpp_int int_var__dpr_phv_fifo_correctable_interrupt;
        void dpr_phv_fifo_correctable_interrupt (const cpp_int  & l__val);
        cpp_int dpr_phv_fifo_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > dpr_ohi_fifo_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__dpr_ohi_fifo_uncorrectable_interrupt;
        void dpr_ohi_fifo_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int dpr_ohi_fifo_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > dpr_ohi_fifo_correctable_interrupt_cpp_int_t;
        cpp_int int_var__dpr_ohi_fifo_correctable_interrupt;
        void dpr_ohi_fifo_correctable_interrupt (const cpp_int  & l__val);
        cpp_int dpr_ohi_fifo_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > dpr_ptr_fifo_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__dpr_ptr_fifo_uncorrectable_interrupt;
        void dpr_ptr_fifo_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int dpr_ptr_fifo_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > dpr_ptr_fifo_correctable_interrupt_cpp_int_t;
        cpp_int int_var__dpr_ptr_fifo_correctable_interrupt;
        void dpr_ptr_fifo_correctable_interrupt (const cpp_int  & l__val);
        cpp_int dpr_ptr_fifo_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > dpr_pktout_fifo_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__dpr_pktout_fifo_uncorrectable_interrupt;
        void dpr_pktout_fifo_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int dpr_pktout_fifo_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > dpr_pktout_fifo_correctable_interrupt_cpp_int_t;
        cpp_int int_var__dpr_pktout_fifo_correctable_interrupt;
        void dpr_pktout_fifo_correctable_interrupt (const cpp_int  & l__val);
        cpp_int dpr_pktout_fifo_correctable_interrupt() const;
    
}; // cap_dpr_csr_int_srams_ecc_intreg_t
    
class cap_dpr_csr_int_srams_ecc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_int_srams_ecc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_int_srams_ecc_t(string name = "cap_dpr_csr_int_srams_ecc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_int_srams_ecc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_dpr_csr_int_srams_ecc_intreg_t intreg;
    
        cap_dpr_csr_int_srams_ecc_intreg_t int_test_set;
    
        cap_dpr_csr_int_srams_ecc_int_enable_set_t int_enable_set;
    
        cap_dpr_csr_int_srams_ecc_int_enable_set_t int_enable_clear;
    
}; // cap_dpr_csr_int_srams_ecc_t
    
class cap_dpr_csr_cfw_dpr_spare_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_cfw_dpr_spare_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_cfw_dpr_spare_t(string name = "cap_dpr_csr_cfw_dpr_spare_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_cfw_dpr_spare_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dpr_csr_cfw_dpr_spare_t
    
class cap_dpr_csr_cfg_spare_csr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_cfg_spare_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_cfg_spare_csr_t(string name = "cap_dpr_csr_cfg_spare_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_cfg_spare_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dpr_csr_cfg_spare_csr_t
    
class cap_dpr_csr_csr_intr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_csr_intr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_csr_intr_t(string name = "cap_dpr_csr_csr_intr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_csr_intr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dpr_csr_csr_intr_t
    
class cap_dpr_csr_cfg_interrupt_fifo_mask_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_cfg_interrupt_fifo_mask_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_cfg_interrupt_fifo_mask_t(string name = "cap_dpr_csr_cfg_interrupt_fifo_mask_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_cfg_interrupt_fifo_mask_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > phv_ff_cpp_int_t;
        cpp_int int_var__phv_ff;
        void phv_ff (const cpp_int  & l__val);
        cpp_int phv_ff() const;
    
        typedef pu_cpp_int< 1 > ohi_ff_cpp_int_t;
        cpp_int int_var__ohi_ff;
        void ohi_ff (const cpp_int  & l__val);
        cpp_int ohi_ff() const;
    
        typedef pu_cpp_int< 1 > pktin_ff_cpp_int_t;
        cpp_int int_var__pktin_ff;
        void pktin_ff (const cpp_int  & l__val);
        cpp_int pktin_ff() const;
    
        typedef pu_cpp_int< 1 > pktout_ff_cpp_int_t;
        cpp_int int_var__pktout_ff;
        void pktout_ff (const cpp_int  & l__val);
        cpp_int pktout_ff() const;
    
        typedef pu_cpp_int< 1 > csum_ff_cpp_int_t;
        cpp_int int_var__csum_ff;
        void csum_ff (const cpp_int  & l__val);
        cpp_int csum_ff() const;
    
        typedef pu_cpp_int< 1 > ptr_ff_cpp_int_t;
        cpp_int int_var__ptr_ff;
        void ptr_ff (const cpp_int  & l__val);
        cpp_int ptr_ff() const;
    
        typedef pu_cpp_int< 1 > egress_credit_ovflow_cpp_int_t;
        cpp_int int_var__egress_credit_ovflow;
        void egress_credit_ovflow (const cpp_int  & l__val);
        cpp_int egress_credit_ovflow() const;
    
        typedef pu_cpp_int< 1 > egress_credit_undflow_cpp_int_t;
        cpp_int int_var__egress_credit_undflow;
        void egress_credit_undflow (const cpp_int  & l__val);
        cpp_int egress_credit_undflow() const;
    
        typedef pu_cpp_int< 1 > pktout_credit_ovflow_cpp_int_t;
        cpp_int int_var__pktout_credit_ovflow;
        void pktout_credit_ovflow (const cpp_int  & l__val);
        cpp_int pktout_credit_ovflow() const;
    
        typedef pu_cpp_int< 1 > pktout_credit_undflow_cpp_int_t;
        cpp_int int_var__pktout_credit_undflow;
        void pktout_credit_undflow (const cpp_int  & l__val);
        cpp_int pktout_credit_undflow() const;
    
}; // cap_dpr_csr_cfg_interrupt_fifo_mask_t
    
class cap_dpr_csr_cfg_interrupt_flop_fifo_mask_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_cfg_interrupt_flop_fifo_mask_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_cfg_interrupt_flop_fifo_mask_t(string name = "cap_dpr_csr_cfg_interrupt_flop_fifo_mask_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_cfg_interrupt_flop_fifo_mask_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > data_mux_force_bypass_crc_cpp_int_t;
        cpp_int int_var__data_mux_force_bypass_crc;
        void data_mux_force_bypass_crc (const cpp_int  & l__val);
        cpp_int data_mux_force_bypass_crc() const;
    
        typedef pu_cpp_int< 1 > dpr_crc_info_cpp_int_t;
        cpp_int int_var__dpr_crc_info;
        void dpr_crc_info (const cpp_int  & l__val);
        cpp_int dpr_crc_info() const;
    
        typedef pu_cpp_int< 1 > dpr_crc_update_info_cpp_int_t;
        cpp_int int_var__dpr_crc_update_info;
        void dpr_crc_update_info (const cpp_int  & l__val);
        cpp_int dpr_crc_update_info() const;
    
        typedef pu_cpp_int< 1 > dpr_csum_info_cpp_int_t;
        cpp_int int_var__dpr_csum_info;
        void dpr_csum_info (const cpp_int  & l__val);
        cpp_int dpr_csum_info() const;
    
        typedef pu_cpp_int< 1 > data_mux_force_bypass_csum_cpp_int_t;
        cpp_int int_var__data_mux_force_bypass_csum;
        void data_mux_force_bypass_csum (const cpp_int  & l__val);
        cpp_int data_mux_force_bypass_csum() const;
    
        typedef pu_cpp_int< 1 > dpr_csum_update_info_cpp_int_t;
        cpp_int int_var__dpr_csum_update_info;
        void dpr_csum_update_info (const cpp_int  & l__val);
        cpp_int dpr_csum_update_info() const;
    
        typedef pu_cpp_int< 1 > ptr_early_pkt_eop_info_cpp_int_t;
        cpp_int int_var__ptr_early_pkt_eop_info;
        void ptr_early_pkt_eop_info (const cpp_int  & l__val);
        cpp_int ptr_early_pkt_eop_info() const;
    
        typedef pu_cpp_int< 1 > data_mux_eop_err_cpp_int_t;
        cpp_int int_var__data_mux_eop_err;
        void data_mux_eop_err (const cpp_int  & l__val);
        cpp_int data_mux_eop_err() const;
    
        typedef pu_cpp_int< 1 > pktin_eop_err_cpp_int_t;
        cpp_int int_var__pktin_eop_err;
        void pktin_eop_err (const cpp_int  & l__val);
        cpp_int pktin_eop_err() const;
    
        typedef pu_cpp_int< 1 > csum_err_cpp_int_t;
        cpp_int int_var__csum_err;
        void csum_err (const cpp_int  & l__val);
        cpp_int csum_err() const;
    
        typedef pu_cpp_int< 1 > crc_err_cpp_int_t;
        cpp_int int_var__crc_err;
        void crc_err (const cpp_int  & l__val);
        cpp_int crc_err() const;
    
        typedef pu_cpp_int< 1 > data_mux_drop_cpp_int_t;
        cpp_int int_var__data_mux_drop;
        void data_mux_drop (const cpp_int  & l__val);
        cpp_int data_mux_drop() const;
    
        typedef pu_cpp_int< 1 > phv_pkt_data_cpp_int_t;
        cpp_int int_var__phv_pkt_data;
        void phv_pkt_data (const cpp_int  & l__val);
        cpp_int phv_pkt_data() const;
    
        typedef pu_cpp_int< 1 > pktout_len_cell_cpp_int_t;
        cpp_int int_var__pktout_len_cell;
        void pktout_len_cell (const cpp_int  & l__val);
        cpp_int pktout_len_cell() const;
    
        typedef pu_cpp_int< 1 > padding_size_cpp_int_t;
        cpp_int int_var__padding_size;
        void padding_size (const cpp_int  & l__val);
        cpp_int padding_size() const;
    
        typedef pu_cpp_int< 1 > pktin_size_cpp_int_t;
        cpp_int int_var__pktin_size;
        void pktin_size (const cpp_int  & l__val);
        cpp_int pktin_size() const;
    
        typedef pu_cpp_int< 1 > pktin_err_cpp_int_t;
        cpp_int int_var__pktin_err;
        void pktin_err (const cpp_int  & l__val);
        cpp_int pktin_err() const;
    
        typedef pu_cpp_int< 1 > phv_no_data_cpp_int_t;
        cpp_int int_var__phv_no_data;
        void phv_no_data (const cpp_int  & l__val);
        cpp_int phv_no_data() const;
    
        typedef pu_cpp_int< 1 > ptr_lookahaed_cpp_int_t;
        cpp_int int_var__ptr_lookahaed;
        void ptr_lookahaed (const cpp_int  & l__val);
        cpp_int ptr_lookahaed() const;
    
        typedef pu_cpp_int< 1 > eop_vld_cpp_int_t;
        cpp_int int_var__eop_vld;
        void eop_vld (const cpp_int  & l__val);
        cpp_int eop_vld() const;
    
        typedef pu_cpp_int< 1 > csum_cal_vld_cpp_int_t;
        cpp_int int_var__csum_cal_vld;
        void csum_cal_vld (const cpp_int  & l__val);
        cpp_int csum_cal_vld() const;
    
}; // cap_dpr_csr_cfg_interrupt_flop_fifo_mask_t
    
class cap_dpr_csr_cfg_interrupt_mask_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_cfg_interrupt_mask_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_cfg_interrupt_mask_t(string name = "cap_dpr_csr_cfg_interrupt_mask_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_cfg_interrupt_mask_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 1 > err_pktin_sop_no_eop_cpp_int_t;
        cpp_int int_var__err_pktin_sop_no_eop;
        void err_pktin_sop_no_eop (const cpp_int  & l__val);
        cpp_int err_pktin_sop_no_eop() const;
    
        typedef pu_cpp_int< 1 > err_pktin_eop_no_sop_cpp_int_t;
        cpp_int int_var__err_pktin_eop_no_sop;
        void err_pktin_eop_no_sop (const cpp_int  & l__val);
        cpp_int err_pktin_eop_no_sop() const;
    
        typedef pu_cpp_int< 1 > err_csum_offset_gt_pkt_size_cpp_int_t;
        cpp_int int_var__err_csum_offset_gt_pkt_size;
        void err_csum_offset_gt_pkt_size (const cpp_int  & l__val);
        cpp_int err_csum_offset_gt_pkt_size() const;
    
        typedef pu_cpp_int< 1 > err_csum_phdr_offset_gt_pkt_size_cpp_int_t;
        cpp_int int_var__err_csum_phdr_offset_gt_pkt_size;
        void err_csum_phdr_offset_gt_pkt_size (const cpp_int  & l__val);
        cpp_int err_csum_phdr_offset_gt_pkt_size() const;
    
        typedef pu_cpp_int< 1 > err_csum_loc_gt_pkt_size_cpp_int_t;
        cpp_int int_var__err_csum_loc_gt_pkt_size;
        void err_csum_loc_gt_pkt_size (const cpp_int  & l__val);
        cpp_int err_csum_loc_gt_pkt_size() const;
    
        typedef pu_cpp_int< 1 > err_csum_start_gt_end_cpp_int_t;
        cpp_int int_var__err_csum_start_gt_end;
        void err_csum_start_gt_end (const cpp_int  & l__val);
        cpp_int err_csum_start_gt_end() const;
    
        typedef pu_cpp_int< 1 > err_crc_offset_gt_pkt_size_cpp_int_t;
        cpp_int int_var__err_crc_offset_gt_pkt_size;
        void err_crc_offset_gt_pkt_size (const cpp_int  & l__val);
        cpp_int err_crc_offset_gt_pkt_size() const;
    
        typedef pu_cpp_int< 1 > err_crc_loc_gt_pkt_size_cpp_int_t;
        cpp_int int_var__err_crc_loc_gt_pkt_size;
        void err_crc_loc_gt_pkt_size (const cpp_int  & l__val);
        cpp_int err_crc_loc_gt_pkt_size() const;
    
        typedef pu_cpp_int< 1 > err_crc_mask_offset_gt_pkt_size_cpp_int_t;
        cpp_int int_var__err_crc_mask_offset_gt_pkt_size;
        void err_crc_mask_offset_gt_pkt_size (const cpp_int  & l__val);
        cpp_int err_crc_mask_offset_gt_pkt_size() const;
    
        typedef pu_cpp_int< 1 > err_crc_start_gt_end_cpp_int_t;
        cpp_int int_var__err_crc_start_gt_end;
        void err_crc_start_gt_end (const cpp_int  & l__val);
        cpp_int err_crc_start_gt_end() const;
    
        typedef pu_cpp_int< 1 > err_pkt_eop_early_cpp_int_t;
        cpp_int int_var__err_pkt_eop_early;
        void err_pkt_eop_early (const cpp_int  & l__val);
        cpp_int err_pkt_eop_early() const;
    
        typedef pu_cpp_int< 1 > err_ptr_ff_overflow_cpp_int_t;
        cpp_int int_var__err_ptr_ff_overflow;
        void err_ptr_ff_overflow (const cpp_int  & l__val);
        cpp_int err_ptr_ff_overflow() const;
    
        typedef pu_cpp_int< 1 > err_csum_ff_overflow_cpp_int_t;
        cpp_int int_var__err_csum_ff_overflow;
        void err_csum_ff_overflow (const cpp_int  & l__val);
        cpp_int err_csum_ff_overflow() const;
    
        typedef pu_cpp_int< 1 > err_pktout_ff_overflow_cpp_int_t;
        cpp_int int_var__err_pktout_ff_overflow;
        void err_pktout_ff_overflow (const cpp_int  & l__val);
        cpp_int err_pktout_ff_overflow() const;
    
        typedef pu_cpp_int< 1 > err_ptr_from_cfg_overflow_cpp_int_t;
        cpp_int int_var__err_ptr_from_cfg_overflow;
        void err_ptr_from_cfg_overflow (const cpp_int  & l__val);
        cpp_int err_ptr_from_cfg_overflow() const;
    
        typedef pu_cpp_int< 32 > rsvd_cpp_int_t;
        cpp_int int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
}; // cap_dpr_csr_cfg_interrupt_mask_t
    
class cap_dpr_csr_cfg_error_mask_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_cfg_error_mask_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_cfg_error_mask_t(string name = "cap_dpr_csr_cfg_error_mask_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_cfg_error_mask_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 1 > err_pktin_sop_no_eop_cpp_int_t;
        cpp_int int_var__err_pktin_sop_no_eop;
        void err_pktin_sop_no_eop (const cpp_int  & l__val);
        cpp_int err_pktin_sop_no_eop() const;
    
        typedef pu_cpp_int< 1 > err_pktin_eop_no_sop_cpp_int_t;
        cpp_int int_var__err_pktin_eop_no_sop;
        void err_pktin_eop_no_sop (const cpp_int  & l__val);
        cpp_int err_pktin_eop_no_sop() const;
    
        typedef pu_cpp_int< 1 > err_csum_offset_gt_pkt_size_cpp_int_t;
        cpp_int int_var__err_csum_offset_gt_pkt_size;
        void err_csum_offset_gt_pkt_size (const cpp_int  & l__val);
        cpp_int err_csum_offset_gt_pkt_size() const;
    
        typedef pu_cpp_int< 1 > err_csum_phdr_offset_gt_pkt_size_cpp_int_t;
        cpp_int int_var__err_csum_phdr_offset_gt_pkt_size;
        void err_csum_phdr_offset_gt_pkt_size (const cpp_int  & l__val);
        cpp_int err_csum_phdr_offset_gt_pkt_size() const;
    
        typedef pu_cpp_int< 1 > err_csum_loc_gt_pkt_size_cpp_int_t;
        cpp_int int_var__err_csum_loc_gt_pkt_size;
        void err_csum_loc_gt_pkt_size (const cpp_int  & l__val);
        cpp_int err_csum_loc_gt_pkt_size() const;
    
        typedef pu_cpp_int< 1 > err_csum_start_gt_end_cpp_int_t;
        cpp_int int_var__err_csum_start_gt_end;
        void err_csum_start_gt_end (const cpp_int  & l__val);
        cpp_int err_csum_start_gt_end() const;
    
        typedef pu_cpp_int< 1 > err_crc_offset_gt_pkt_size_cpp_int_t;
        cpp_int int_var__err_crc_offset_gt_pkt_size;
        void err_crc_offset_gt_pkt_size (const cpp_int  & l__val);
        cpp_int err_crc_offset_gt_pkt_size() const;
    
        typedef pu_cpp_int< 1 > err_crc_loc_gt_pkt_size_cpp_int_t;
        cpp_int int_var__err_crc_loc_gt_pkt_size;
        void err_crc_loc_gt_pkt_size (const cpp_int  & l__val);
        cpp_int err_crc_loc_gt_pkt_size() const;
    
        typedef pu_cpp_int< 1 > err_crc_mask_offset_gt_pkt_size_cpp_int_t;
        cpp_int int_var__err_crc_mask_offset_gt_pkt_size;
        void err_crc_mask_offset_gt_pkt_size (const cpp_int  & l__val);
        cpp_int err_crc_mask_offset_gt_pkt_size() const;
    
        typedef pu_cpp_int< 1 > err_crc_start_gt_end_cpp_int_t;
        cpp_int int_var__err_crc_start_gt_end;
        void err_crc_start_gt_end (const cpp_int  & l__val);
        cpp_int err_crc_start_gt_end() const;
    
        typedef pu_cpp_int< 1 > err_pkt_eop_early_cpp_int_t;
        cpp_int int_var__err_pkt_eop_early;
        void err_pkt_eop_early (const cpp_int  & l__val);
        cpp_int err_pkt_eop_early() const;
    
        typedef pu_cpp_int< 1 > err_ptr_ff_overflow_cpp_int_t;
        cpp_int int_var__err_ptr_ff_overflow;
        void err_ptr_ff_overflow (const cpp_int  & l__val);
        cpp_int err_ptr_ff_overflow() const;
    
        typedef pu_cpp_int< 1 > err_csum_ff_overflow_cpp_int_t;
        cpp_int int_var__err_csum_ff_overflow;
        void err_csum_ff_overflow (const cpp_int  & l__val);
        cpp_int err_csum_ff_overflow() const;
    
        typedef pu_cpp_int< 1 > err_pktout_ff_overflow_cpp_int_t;
        cpp_int int_var__err_pktout_ff_overflow;
        void err_pktout_ff_overflow (const cpp_int  & l__val);
        cpp_int err_pktout_ff_overflow() const;
    
        typedef pu_cpp_int< 1 > err_ptr_from_cfg_overflow_cpp_int_t;
        cpp_int int_var__err_ptr_from_cfg_overflow;
        void err_ptr_from_cfg_overflow (const cpp_int  & l__val);
        cpp_int err_ptr_from_cfg_overflow() const;
    
        typedef pu_cpp_int< 32 > rsvd_cpp_int_t;
        cpp_int int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
}; // cap_dpr_csr_cfg_error_mask_t
    
class cap_dpr_csr_cfg_pkt_truncation_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_cfg_pkt_truncation_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_cfg_pkt_truncation_t(string name = "cap_dpr_csr_cfg_pkt_truncation_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_cfg_pkt_truncation_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > truncation_profile_sel_cpp_int_t;
        cpp_int int_var__truncation_profile_sel;
        void truncation_profile_sel (const cpp_int  & l__val);
        cpp_int truncation_profile_sel() const;
    
        typedef pu_cpp_int< 64 > max_size_cpp_int_t;
        cpp_int int_var__max_size;
        void max_size (const cpp_int  & l__val);
        cpp_int max_size() const;
    
}; // cap_dpr_csr_cfg_pkt_truncation_t
    
class cap_dpr_csr_cfg_pkt_padding_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_cfg_pkt_padding_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_cfg_pkt_padding_t(string name = "cap_dpr_csr_cfg_pkt_padding_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_cfg_pkt_padding_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > padding_profile_sel_cpp_int_t;
        cpp_int int_var__padding_profile_sel;
        void padding_profile_sel (const cpp_int  & l__val);
        cpp_int padding_profile_sel() const;
    
        typedef pu_cpp_int< 32 > min_size_cpp_int_t;
        cpp_int int_var__min_size;
        void min_size (const cpp_int  & l__val);
        cpp_int min_size() const;
    
}; // cap_dpr_csr_cfg_pkt_padding_t
    
class cap_dpr_csr_cfg_global_err_code_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_cfg_global_err_code_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_cfg_global_err_code_t(string name = "cap_dpr_csr_cfg_global_err_code_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_cfg_global_err_code_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 1 > rw_error_cpp_int_t;
        cpp_int int_var__rw_error;
        void rw_error (const cpp_int  & l__val);
        cpp_int rw_error() const;
    
        typedef pu_cpp_int< 8 > err_code_0_cpp_int_t;
        cpp_int int_var__err_code_0;
        void err_code_0 (const cpp_int  & l__val);
        cpp_int err_code_0() const;
    
        typedef pu_cpp_int< 8 > err_code_1_cpp_int_t;
        cpp_int int_var__err_code_1;
        void err_code_1 (const cpp_int  & l__val);
        cpp_int err_code_1() const;
    
        typedef pu_cpp_int< 8 > err_code_2_cpp_int_t;
        cpp_int int_var__err_code_2;
        void err_code_2 (const cpp_int  & l__val);
        cpp_int err_code_2() const;
    
        typedef pu_cpp_int< 8 > err_code_3_cpp_int_t;
        cpp_int int_var__err_code_3;
        void err_code_3 (const cpp_int  & l__val);
        cpp_int err_code_3() const;
    
        typedef pu_cpp_int< 8 > err_code_4_cpp_int_t;
        cpp_int int_var__err_code_4;
        void err_code_4 (const cpp_int  & l__val);
        cpp_int err_code_4() const;
    
        typedef pu_cpp_int< 8 > err_code_5_cpp_int_t;
        cpp_int int_var__err_code_5;
        void err_code_5 (const cpp_int  & l__val);
        cpp_int err_code_5() const;
    
        typedef pu_cpp_int< 8 > err_code_6_cpp_int_t;
        cpp_int int_var__err_code_6;
        void err_code_6 (const cpp_int  & l__val);
        cpp_int err_code_6() const;
    
        typedef pu_cpp_int< 8 > err_code_7_cpp_int_t;
        cpp_int int_var__err_code_7;
        void err_code_7 (const cpp_int  & l__val);
        cpp_int err_code_7() const;
    
}; // cap_dpr_csr_cfg_global_err_code_t
    
class cap_dpr_csr_cfg_global_2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_cfg_global_2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_cfg_global_2_t(string name = "cap_dpr_csr_cfg_global_2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_cfg_global_2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > dump_drop_no_data_phv_cpp_int_t;
        cpp_int int_var__dump_drop_no_data_phv;
        void dump_drop_no_data_phv (const cpp_int  & l__val);
        cpp_int dump_drop_no_data_phv() const;
    
        typedef pu_cpp_int< 1 > increment_recirc_cnt_en_cpp_int_t;
        cpp_int int_var__increment_recirc_cnt_en;
        void increment_recirc_cnt_en (const cpp_int  & l__val);
        cpp_int increment_recirc_cnt_en() const;
    
        typedef pu_cpp_int< 3 > max_recirc_cnt_cpp_int_t;
        cpp_int int_var__max_recirc_cnt;
        void max_recirc_cnt (const cpp_int  & l__val);
        cpp_int max_recirc_cnt() const;
    
        typedef pu_cpp_int< 1 > drop_max_recirc_cnt_cpp_int_t;
        cpp_int int_var__drop_max_recirc_cnt;
        void drop_max_recirc_cnt (const cpp_int  & l__val);
        cpp_int drop_max_recirc_cnt() const;
    
        typedef pu_cpp_int< 4 > recirc_oport_cpp_int_t;
        cpp_int int_var__recirc_oport;
        void recirc_oport (const cpp_int  & l__val);
        cpp_int recirc_oport() const;
    
        typedef pu_cpp_int< 1 > clear_recirc_bit_en_cpp_int_t;
        cpp_int int_var__clear_recirc_bit_en;
        void clear_recirc_bit_en (const cpp_int  & l__val);
        cpp_int clear_recirc_bit_en() const;
    
        typedef pu_cpp_int< 16 > recirc_rw_bm_cpp_int_t;
        cpp_int int_var__recirc_rw_bm;
        void recirc_rw_bm (const cpp_int  & l__val);
        cpp_int recirc_rw_bm() const;
    
        typedef pu_cpp_int< 16 > frame_size_rw_bm_cpp_int_t;
        cpp_int int_var__frame_size_rw_bm;
        void frame_size_rw_bm (const cpp_int  & l__val);
        cpp_int frame_size_rw_bm() const;
    
        typedef pu_cpp_int< 27 > rsvd_cpp_int_t;
        cpp_int int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
}; // cap_dpr_csr_cfg_global_2_t
    
class cap_dpr_csr_cfg_global_hw_1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_cfg_global_hw_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_cfg_global_hw_1_t(string name = "cap_dpr_csr_cfg_global_hw_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_cfg_global_hw_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > freeze_on_err_cpp_int_t;
        cpp_int int_var__freeze_on_err;
        void freeze_on_err (const cpp_int  & l__val);
        cpp_int freeze_on_err() const;
    
        typedef pu_cpp_int< 1 > freeze_on_err_release_cpp_int_t;
        cpp_int int_var__freeze_on_err_release;
        void freeze_on_err_release (const cpp_int  & l__val);
        cpp_int freeze_on_err_release() const;
    
        typedef pu_cpp_int< 1 > freeze_on_capture_cpp_int_t;
        cpp_int int_var__freeze_on_capture;
        void freeze_on_capture (const cpp_int  & l__val);
        cpp_int freeze_on_capture() const;
    
        typedef pu_cpp_int< 1 > freeze_on_capture_release_cpp_int_t;
        cpp_int int_var__freeze_on_capture_release;
        void freeze_on_capture_release (const cpp_int  & l__val);
        cpp_int freeze_on_capture_release() const;
    
        typedef pu_cpp_int< 1 > disable_ptr_lookahead_cpp_int_t;
        cpp_int int_var__disable_ptr_lookahead;
        void disable_ptr_lookahead (const cpp_int  & l__val);
        cpp_int disable_ptr_lookahead() const;
    
        typedef pu_cpp_int< 1 > debug_port_enable_cpp_int_t;
        cpp_int int_var__debug_port_enable;
        void debug_port_enable (const cpp_int  & l__val);
        cpp_int debug_port_enable() const;
    
        typedef pu_cpp_int< 1 > eop_err_det_disable_cpp_int_t;
        cpp_int int_var__eop_err_det_disable;
        void eop_err_det_disable (const cpp_int  & l__val);
        cpp_int eop_err_det_disable() const;
    
        typedef pu_cpp_int< 1 > eop_err_set_disable_cpp_int_t;
        cpp_int int_var__eop_err_set_disable;
        void eop_err_set_disable (const cpp_int  & l__val);
        cpp_int eop_err_set_disable() const;
    
        typedef pu_cpp_int< 1 > swap_crc_byte_cpp_int_t;
        cpp_int int_var__swap_crc_byte;
        void swap_crc_byte (const cpp_int  & l__val);
        cpp_int swap_crc_byte() const;
    
}; // cap_dpr_csr_cfg_global_hw_1_t
    
class cap_dpr_csr_cfg_global_1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_cfg_global_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_cfg_global_1_t(string name = "cap_dpr_csr_cfg_global_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_cfg_global_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > intrinsic_len_rw_en_cpp_int_t;
        cpp_int int_var__intrinsic_len_rw_en;
        void intrinsic_len_rw_en (const cpp_int  & l__val);
        cpp_int intrinsic_len_rw_en() const;
    
        typedef pu_cpp_int< 1 > intrinsic_drop_rw_en_cpp_int_t;
        cpp_int int_var__intrinsic_drop_rw_en;
        void intrinsic_drop_rw_en (const cpp_int  & l__val);
        cpp_int intrinsic_drop_rw_en() const;
    
        typedef pu_cpp_int< 1 > padding_en_cpp_int_t;
        cpp_int int_var__padding_en;
        void padding_en (const cpp_int  & l__val);
        cpp_int padding_en() const;
    
        typedef pu_cpp_int< 16 > pkt_min_size_cpp_int_t;
        cpp_int int_var__pkt_min_size;
        void pkt_min_size (const cpp_int  & l__val);
        cpp_int pkt_min_size() const;
    
        typedef pu_cpp_int< 16 > max_pkt_size_cpp_int_t;
        cpp_int int_var__max_pkt_size;
        void max_pkt_size (const cpp_int  & l__val);
        cpp_int max_pkt_size() const;
    
        typedef pu_cpp_int< 1 > pkt_truncation_en_cpp_int_t;
        cpp_int int_var__pkt_truncation_en;
        void pkt_truncation_en (const cpp_int  & l__val);
        cpp_int pkt_truncation_en() const;
    
        typedef pu_cpp_int< 16 > ignore_hdrfld_size_0_cpp_int_t;
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
    
        typedef pu_cpp_int< 1 > no_csum_update_on_csum_err_cpp_int_t;
        cpp_int int_var__no_csum_update_on_csum_err;
        void no_csum_update_on_csum_err (const cpp_int  & l__val);
        cpp_int no_csum_update_on_csum_err() const;
    
        typedef pu_cpp_int< 1 > no_crc_update_on_crc_err_cpp_int_t;
        cpp_int int_var__no_crc_update_on_crc_err;
        void no_crc_update_on_crc_err (const cpp_int  & l__val);
        cpp_int no_crc_update_on_crc_err() const;
    
        typedef pu_cpp_int< 1 > no_sf_cpp_int_t;
        cpp_int int_var__no_sf;
        void no_sf (const cpp_int  & l__val);
        cpp_int no_sf() const;
    
}; // cap_dpr_csr_cfg_global_1_t
    
class cap_dpr_csr_cfg_global_hw_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_cfg_global_hw_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_cfg_global_hw_t(string name = "cap_dpr_csr_cfg_global_hw_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_cfg_global_hw_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 3 > pktout_fc_threshold_cpp_int_t;
        cpp_int int_var__pktout_fc_threshold;
        void pktout_fc_threshold (const cpp_int  & l__val);
        cpp_int pktout_fc_threshold() const;
    
        typedef pu_cpp_int< 10 > csum_ff_fc_threshold_cpp_int_t;
        cpp_int int_var__csum_ff_fc_threshold;
        void csum_ff_fc_threshold (const cpp_int  & l__val);
        cpp_int csum_ff_fc_threshold() const;
    
        typedef pu_cpp_int< 8 > phv_fifo_almost_full_threshold_cpp_int_t;
        cpp_int int_var__phv_fifo_almost_full_threshold;
        void phv_fifo_almost_full_threshold (const cpp_int  & l__val);
        cpp_int phv_fifo_almost_full_threshold() const;
    
        typedef pu_cpp_int< 4 > ohi_fifo_almost_full_threshold_cpp_int_t;
        cpp_int int_var__ohi_fifo_almost_full_threshold;
        void ohi_fifo_almost_full_threshold (const cpp_int  & l__val);
        cpp_int ohi_fifo_almost_full_threshold() const;
    
        typedef pu_cpp_int< 10 > pktin_fifo_almost_full_threshold_cpp_int_t;
        cpp_int int_var__pktin_fifo_almost_full_threshold;
        void pktin_fifo_almost_full_threshold (const cpp_int  & l__val);
        cpp_int pktin_fifo_almost_full_threshold() const;
    
        typedef pu_cpp_int< 5 > ptr_fifo_fc_thr_cpp_int_t;
        cpp_int int_var__ptr_fifo_fc_thr;
        void ptr_fifo_fc_thr (const cpp_int  & l__val);
        cpp_int ptr_fifo_fc_thr() const;
    
        typedef pu_cpp_int< 1 > csum_ff_credit_en_cpp_int_t;
        cpp_int int_var__csum_ff_credit_en;
        void csum_ff_credit_en (const cpp_int  & l__val);
        cpp_int csum_ff_credit_en() const;
    
        typedef pu_cpp_int< 5 > csum_ff_credit_cpp_int_t;
        cpp_int int_var__csum_ff_credit;
        void csum_ff_credit (const cpp_int  & l__val);
        cpp_int csum_ff_credit() const;
    
        typedef pu_cpp_int< 1 > pktout_ff_credit_en_cpp_int_t;
        cpp_int int_var__pktout_ff_credit_en;
        void pktout_ff_credit_en (const cpp_int  & l__val);
        cpp_int pktout_ff_credit_en() const;
    
        typedef pu_cpp_int< 5 > pktout_ff_credit_cpp_int_t;
        cpp_int int_var__pktout_ff_credit;
        void pktout_ff_credit (const cpp_int  & l__val);
        cpp_int pktout_ff_credit() const;
    
}; // cap_dpr_csr_cfg_global_hw_t
    
class cap_dpr_csr_cfg_global_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_cfg_global_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_cfg_global_t(string name = "cap_dpr_csr_cfg_global_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_cfg_global_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > rsvd_cpp_int_t;
        cpp_int int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
}; // cap_dpr_csr_cfg_global_t
    
class cap_dpr_csr_base_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_base_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_base_t(string name = "cap_dpr_csr_base_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_base_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dpr_csr_base_t
    
class cap_dpr_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dpr_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dpr_csr_t(string name = "cap_dpr_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dpr_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_dpr_csr_base_t base;
    
        cap_dpr_csr_cfg_global_t cfg_global;
    
        cap_dpr_csr_cfg_global_hw_t cfg_global_hw;
    
        cap_dpr_csr_cfg_global_1_t cfg_global_1;
    
        cap_dpr_csr_cfg_global_hw_1_t cfg_global_hw_1;
    
        cap_dpr_csr_cfg_global_2_t cfg_global_2;
    
        cap_dpr_csr_cfg_global_err_code_t cfg_global_err_code;
    
        cap_dpr_csr_cfg_pkt_padding_t cfg_pkt_padding;
    
        cap_dpr_csr_cfg_pkt_truncation_t cfg_pkt_truncation;
    
        cap_dpr_csr_cfg_error_mask_t cfg_error_mask;
    
        cap_dpr_csr_cfg_interrupt_mask_t cfg_interrupt_mask;
    
        cap_dpr_csr_cfg_interrupt_flop_fifo_mask_t cfg_interrupt_flop_fifo_mask;
    
        cap_dpr_csr_cfg_interrupt_fifo_mask_t cfg_interrupt_fifo_mask;
    
        cap_dpr_csr_csr_intr_t csr_intr;
    
        #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_dpr_csr_cfg_spare_csr_t, 16> cfg_spare_csr;
        #else 
        cap_dpr_csr_cfg_spare_csr_t cfg_spare_csr[16];
        #endif
        int get_depth_cfg_spare_csr() { return 16; }
    
        cap_dpr_csr_cfw_dpr_spare_t cfw_dpr_spare;
    
        cap_dprhdrfld_csr_t hdrfld;
    
        cap_dprcfg_csr_t cfg;
    
        cap_dprstats_csr_t stats;
    
        cap_dprmem_csr_t mem;
    
        cap_dpr_csr_int_srams_ecc_t int_srams_ecc;
    
        cap_dpr_csr_intgrp_status_t int_groups;
    
        cap_dpr_csr_int_reg1_t int_reg1;
    
        cap_dpr_csr_int_reg2_t int_reg2;
    
        cap_dpr_csr_int_fifo_t int_fifo;
    
        cap_dpr_csr_int_flop_fifo_0_t int_flop_fifo_0;
    
        cap_dpr_csr_int_flop_fifo_1_t int_flop_fifo_1;
    
        cap_dpr_csr_int_credit_t int_credit;
    
        cap_dpr_csr_int_spare_t int_spare;
    
}; // cap_dpr_csr_t
    
#endif // CAP_DPR_CSR_H
        