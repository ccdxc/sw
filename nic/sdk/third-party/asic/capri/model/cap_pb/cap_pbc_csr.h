
#ifndef CAP_PBC_CSR_H
#define CAP_PBC_CSR_H

#include "cap_csr_base.h" 
#include "cap_pbchbmtx_csr.h" 
#include "cap_pbchbmeth_csr.h" 
#include "cap_pbchbm_csr.h" 
#include "cap_pbcport11_csr.h" 
#include "cap_pbcport10_csr.h" 
#include "cap_pbcport9_csr.h" 
#include "cap_pbcport8_csr.h" 
#include "cap_pbcport7_csr.h" 
#include "cap_pbcport6_csr.h" 
#include "cap_pbcport5_csr.h" 
#include "cap_pbcport4_csr.h" 
#include "cap_pbcport3_csr.h" 
#include "cap_pbcport2_csr.h" 
#include "cap_pbcport1_csr.h" 
#include "cap_pbcport0_csr.h" 

using namespace std;
class cap_pbc_csr_int_credit_underflow_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_int_credit_underflow_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_int_credit_underflow_int_enable_clear_t(string name = "cap_pbc_csr_int_credit_underflow_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_int_credit_underflow_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > port_10_enable_cpp_int_t;
        cpp_int int_var__port_10_enable;
        void port_10_enable (const cpp_int  & l__val);
        cpp_int port_10_enable() const;
    
        typedef pu_cpp_int< 1 > port_11_enable_cpp_int_t;
        cpp_int int_var__port_11_enable;
        void port_11_enable (const cpp_int  & l__val);
        cpp_int port_11_enable() const;
    
}; // cap_pbc_csr_int_credit_underflow_int_enable_clear_t
    
class cap_pbc_csr_int_credit_underflow_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_int_credit_underflow_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_int_credit_underflow_int_test_set_t(string name = "cap_pbc_csr_int_credit_underflow_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_int_credit_underflow_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > port_10_interrupt_cpp_int_t;
        cpp_int int_var__port_10_interrupt;
        void port_10_interrupt (const cpp_int  & l__val);
        cpp_int port_10_interrupt() const;
    
        typedef pu_cpp_int< 1 > port_11_interrupt_cpp_int_t;
        cpp_int int_var__port_11_interrupt;
        void port_11_interrupt (const cpp_int  & l__val);
        cpp_int port_11_interrupt() const;
    
}; // cap_pbc_csr_int_credit_underflow_int_test_set_t
    
class cap_pbc_csr_int_credit_underflow_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_int_credit_underflow_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_int_credit_underflow_t(string name = "cap_pbc_csr_int_credit_underflow_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_int_credit_underflow_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbc_csr_int_credit_underflow_int_test_set_t intreg;
    
        cap_pbc_csr_int_credit_underflow_int_test_set_t int_test_set;
    
        cap_pbc_csr_int_credit_underflow_int_enable_clear_t int_enable_set;
    
        cap_pbc_csr_int_credit_underflow_int_enable_clear_t int_enable_clear;
    
}; // cap_pbc_csr_int_credit_underflow_t
    
class cap_pbc_csr_int_ecc_port_mon_out_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_int_ecc_port_mon_out_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_int_ecc_port_mon_out_int_enable_clear_t(string name = "cap_pbc_csr_int_ecc_port_mon_out_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_int_ecc_port_mon_out_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > uncorrectable_enable_cpp_int_t;
        cpp_int int_var__uncorrectable_enable;
        void uncorrectable_enable (const cpp_int  & l__val);
        cpp_int uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > correctable_enable_cpp_int_t;
        cpp_int int_var__correctable_enable;
        void correctable_enable (const cpp_int  & l__val);
        cpp_int correctable_enable() const;
    
}; // cap_pbc_csr_int_ecc_port_mon_out_int_enable_clear_t
    
class cap_pbc_csr_int_ecc_port_mon_out_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_int_ecc_port_mon_out_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_int_ecc_port_mon_out_int_test_set_t(string name = "cap_pbc_csr_int_ecc_port_mon_out_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_int_ecc_port_mon_out_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__uncorrectable_interrupt;
        void uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > correctable_interrupt_cpp_int_t;
        cpp_int int_var__correctable_interrupt;
        void correctable_interrupt (const cpp_int  & l__val);
        cpp_int correctable_interrupt() const;
    
}; // cap_pbc_csr_int_ecc_port_mon_out_int_test_set_t
    
class cap_pbc_csr_int_ecc_port_mon_out_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_int_ecc_port_mon_out_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_int_ecc_port_mon_out_t(string name = "cap_pbc_csr_int_ecc_port_mon_out_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_int_ecc_port_mon_out_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbc_csr_int_ecc_port_mon_out_int_test_set_t intreg;
    
        cap_pbc_csr_int_ecc_port_mon_out_int_test_set_t int_test_set;
    
        cap_pbc_csr_int_ecc_port_mon_out_int_enable_clear_t int_enable_set;
    
        cap_pbc_csr_int_ecc_port_mon_out_int_enable_clear_t int_enable_clear;
    
}; // cap_pbc_csr_int_ecc_port_mon_out_t
    
class cap_pbc_csr_int_rpl_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_int_rpl_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_int_rpl_int_enable_clear_t(string name = "cap_pbc_csr_int_rpl_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_int_rpl_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > memory_error_enable_cpp_int_t;
        cpp_int int_var__memory_error_enable;
        void memory_error_enable (const cpp_int  & l__val);
        cpp_int memory_error_enable() const;
    
        typedef pu_cpp_int< 1 > zero_last_error_enable_cpp_int_t;
        cpp_int int_var__zero_last_error_enable;
        void zero_last_error_enable (const cpp_int  & l__val);
        cpp_int zero_last_error_enable() const;
    
}; // cap_pbc_csr_int_rpl_int_enable_clear_t
    
class cap_pbc_csr_int_rpl_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_int_rpl_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_int_rpl_int_test_set_t(string name = "cap_pbc_csr_int_rpl_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_int_rpl_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > memory_error_interrupt_cpp_int_t;
        cpp_int int_var__memory_error_interrupt;
        void memory_error_interrupt (const cpp_int  & l__val);
        cpp_int memory_error_interrupt() const;
    
        typedef pu_cpp_int< 1 > zero_last_error_interrupt_cpp_int_t;
        cpp_int int_var__zero_last_error_interrupt;
        void zero_last_error_interrupt (const cpp_int  & l__val);
        cpp_int zero_last_error_interrupt() const;
    
}; // cap_pbc_csr_int_rpl_int_test_set_t
    
class cap_pbc_csr_int_rpl_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_int_rpl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_int_rpl_t(string name = "cap_pbc_csr_int_rpl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_int_rpl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbc_csr_int_rpl_int_test_set_t intreg;
    
        cap_pbc_csr_int_rpl_int_test_set_t int_test_set;
    
        cap_pbc_csr_int_rpl_int_enable_clear_t int_enable_set;
    
        cap_pbc_csr_int_rpl_int_enable_clear_t int_enable_clear;
    
}; // cap_pbc_csr_int_rpl_t
    
class cap_pbc_csr_int_ecc_sideband_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_int_ecc_sideband_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_int_ecc_sideband_int_enable_clear_t(string name = "cap_pbc_csr_int_ecc_sideband_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_int_ecc_sideband_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > pck_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__pck_uncorrectable_enable;
        void pck_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int pck_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > pck_correctable_enable_cpp_int_t;
        cpp_int int_var__pck_correctable_enable;
        void pck_correctable_enable (const cpp_int  & l__val);
        cpp_int pck_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > rwr_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__rwr_uncorrectable_enable;
        void rwr_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int rwr_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > rwr_correctable_enable_cpp_int_t;
        cpp_int int_var__rwr_correctable_enable;
        void rwr_correctable_enable (const cpp_int  & l__val);
        cpp_int rwr_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > rxd_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__rxd_uncorrectable_enable;
        void rxd_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int rxd_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > rxd_correctable_enable_cpp_int_t;
        cpp_int int_var__rxd_correctable_enable;
        void rxd_correctable_enable (const cpp_int  & l__val);
        cpp_int rxd_correctable_enable() const;
    
}; // cap_pbc_csr_int_ecc_sideband_int_enable_clear_t
    
class cap_pbc_csr_int_ecc_sideband_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_int_ecc_sideband_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_int_ecc_sideband_int_test_set_t(string name = "cap_pbc_csr_int_ecc_sideband_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_int_ecc_sideband_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > pck_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__pck_uncorrectable_interrupt;
        void pck_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int pck_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > pck_correctable_interrupt_cpp_int_t;
        cpp_int int_var__pck_correctable_interrupt;
        void pck_correctable_interrupt (const cpp_int  & l__val);
        cpp_int pck_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > rwr_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__rwr_uncorrectable_interrupt;
        void rwr_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int rwr_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > rwr_correctable_interrupt_cpp_int_t;
        cpp_int int_var__rwr_correctable_interrupt;
        void rwr_correctable_interrupt (const cpp_int  & l__val);
        cpp_int rwr_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > rxd_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__rxd_uncorrectable_interrupt;
        void rxd_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int rxd_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > rxd_correctable_interrupt_cpp_int_t;
        cpp_int int_var__rxd_correctable_interrupt;
        void rxd_correctable_interrupt (const cpp_int  & l__val);
        cpp_int rxd_correctable_interrupt() const;
    
}; // cap_pbc_csr_int_ecc_sideband_int_test_set_t
    
class cap_pbc_csr_int_ecc_sideband_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_int_ecc_sideband_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_int_ecc_sideband_t(string name = "cap_pbc_csr_int_ecc_sideband_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_int_ecc_sideband_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbc_csr_int_ecc_sideband_int_test_set_t intreg;
    
        cap_pbc_csr_int_ecc_sideband_int_test_set_t int_test_set;
    
        cap_pbc_csr_int_ecc_sideband_int_enable_clear_t int_enable_set;
    
        cap_pbc_csr_int_ecc_sideband_int_enable_clear_t int_enable_clear;
    
}; // cap_pbc_csr_int_ecc_sideband_t
    
class cap_pbc_csr_int_pbus_violation_out_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_int_pbus_violation_out_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_int_pbus_violation_out_int_enable_clear_t(string name = "cap_pbc_csr_int_pbus_violation_out_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_int_pbus_violation_out_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > sop_sop_out_0_enable_cpp_int_t;
        cpp_int int_var__sop_sop_out_0_enable;
        void sop_sop_out_0_enable (const cpp_int  & l__val);
        cpp_int sop_sop_out_0_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_1_enable_cpp_int_t;
        cpp_int int_var__sop_sop_out_1_enable;
        void sop_sop_out_1_enable (const cpp_int  & l__val);
        cpp_int sop_sop_out_1_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_2_enable_cpp_int_t;
        cpp_int int_var__sop_sop_out_2_enable;
        void sop_sop_out_2_enable (const cpp_int  & l__val);
        cpp_int sop_sop_out_2_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_3_enable_cpp_int_t;
        cpp_int int_var__sop_sop_out_3_enable;
        void sop_sop_out_3_enable (const cpp_int  & l__val);
        cpp_int sop_sop_out_3_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_4_enable_cpp_int_t;
        cpp_int int_var__sop_sop_out_4_enable;
        void sop_sop_out_4_enable (const cpp_int  & l__val);
        cpp_int sop_sop_out_4_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_5_enable_cpp_int_t;
        cpp_int int_var__sop_sop_out_5_enable;
        void sop_sop_out_5_enable (const cpp_int  & l__val);
        cpp_int sop_sop_out_5_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_6_enable_cpp_int_t;
        cpp_int int_var__sop_sop_out_6_enable;
        void sop_sop_out_6_enable (const cpp_int  & l__val);
        cpp_int sop_sop_out_6_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_7_enable_cpp_int_t;
        cpp_int int_var__sop_sop_out_7_enable;
        void sop_sop_out_7_enable (const cpp_int  & l__val);
        cpp_int sop_sop_out_7_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_8_enable_cpp_int_t;
        cpp_int int_var__sop_sop_out_8_enable;
        void sop_sop_out_8_enable (const cpp_int  & l__val);
        cpp_int sop_sop_out_8_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_9_enable_cpp_int_t;
        cpp_int int_var__sop_sop_out_9_enable;
        void sop_sop_out_9_enable (const cpp_int  & l__val);
        cpp_int sop_sop_out_9_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_10_enable_cpp_int_t;
        cpp_int int_var__sop_sop_out_10_enable;
        void sop_sop_out_10_enable (const cpp_int  & l__val);
        cpp_int sop_sop_out_10_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_11_enable_cpp_int_t;
        cpp_int int_var__sop_sop_out_11_enable;
        void sop_sop_out_11_enable (const cpp_int  & l__val);
        cpp_int sop_sop_out_11_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_0_enable_cpp_int_t;
        cpp_int int_var__eop_eop_out_0_enable;
        void eop_eop_out_0_enable (const cpp_int  & l__val);
        cpp_int eop_eop_out_0_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_1_enable_cpp_int_t;
        cpp_int int_var__eop_eop_out_1_enable;
        void eop_eop_out_1_enable (const cpp_int  & l__val);
        cpp_int eop_eop_out_1_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_2_enable_cpp_int_t;
        cpp_int int_var__eop_eop_out_2_enable;
        void eop_eop_out_2_enable (const cpp_int  & l__val);
        cpp_int eop_eop_out_2_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_3_enable_cpp_int_t;
        cpp_int int_var__eop_eop_out_3_enable;
        void eop_eop_out_3_enable (const cpp_int  & l__val);
        cpp_int eop_eop_out_3_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_4_enable_cpp_int_t;
        cpp_int int_var__eop_eop_out_4_enable;
        void eop_eop_out_4_enable (const cpp_int  & l__val);
        cpp_int eop_eop_out_4_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_5_enable_cpp_int_t;
        cpp_int int_var__eop_eop_out_5_enable;
        void eop_eop_out_5_enable (const cpp_int  & l__val);
        cpp_int eop_eop_out_5_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_6_enable_cpp_int_t;
        cpp_int int_var__eop_eop_out_6_enable;
        void eop_eop_out_6_enable (const cpp_int  & l__val);
        cpp_int eop_eop_out_6_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_7_enable_cpp_int_t;
        cpp_int int_var__eop_eop_out_7_enable;
        void eop_eop_out_7_enable (const cpp_int  & l__val);
        cpp_int eop_eop_out_7_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_8_enable_cpp_int_t;
        cpp_int int_var__eop_eop_out_8_enable;
        void eop_eop_out_8_enable (const cpp_int  & l__val);
        cpp_int eop_eop_out_8_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_9_enable_cpp_int_t;
        cpp_int int_var__eop_eop_out_9_enable;
        void eop_eop_out_9_enable (const cpp_int  & l__val);
        cpp_int eop_eop_out_9_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_10_enable_cpp_int_t;
        cpp_int int_var__eop_eop_out_10_enable;
        void eop_eop_out_10_enable (const cpp_int  & l__val);
        cpp_int eop_eop_out_10_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_11_enable_cpp_int_t;
        cpp_int int_var__eop_eop_out_11_enable;
        void eop_eop_out_11_enable (const cpp_int  & l__val);
        cpp_int eop_eop_out_11_enable() const;
    
}; // cap_pbc_csr_int_pbus_violation_out_int_enable_clear_t
    
class cap_pbc_csr_int_pbus_violation_out_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_int_pbus_violation_out_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_int_pbus_violation_out_int_test_set_t(string name = "cap_pbc_csr_int_pbus_violation_out_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_int_pbus_violation_out_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > sop_sop_out_0_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_out_0_interrupt;
        void sop_sop_out_0_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_out_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_1_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_out_1_interrupt;
        void sop_sop_out_1_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_out_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_2_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_out_2_interrupt;
        void sop_sop_out_2_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_out_2_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_3_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_out_3_interrupt;
        void sop_sop_out_3_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_out_3_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_4_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_out_4_interrupt;
        void sop_sop_out_4_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_out_4_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_5_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_out_5_interrupt;
        void sop_sop_out_5_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_out_5_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_6_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_out_6_interrupt;
        void sop_sop_out_6_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_out_6_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_7_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_out_7_interrupt;
        void sop_sop_out_7_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_out_7_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_8_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_out_8_interrupt;
        void sop_sop_out_8_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_out_8_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_9_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_out_9_interrupt;
        void sop_sop_out_9_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_out_9_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_10_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_out_10_interrupt;
        void sop_sop_out_10_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_out_10_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_out_11_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_out_11_interrupt;
        void sop_sop_out_11_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_out_11_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_0_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_out_0_interrupt;
        void eop_eop_out_0_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_out_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_1_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_out_1_interrupt;
        void eop_eop_out_1_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_out_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_2_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_out_2_interrupt;
        void eop_eop_out_2_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_out_2_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_3_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_out_3_interrupt;
        void eop_eop_out_3_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_out_3_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_4_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_out_4_interrupt;
        void eop_eop_out_4_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_out_4_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_5_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_out_5_interrupt;
        void eop_eop_out_5_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_out_5_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_6_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_out_6_interrupt;
        void eop_eop_out_6_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_out_6_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_7_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_out_7_interrupt;
        void eop_eop_out_7_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_out_7_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_8_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_out_8_interrupt;
        void eop_eop_out_8_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_out_8_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_9_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_out_9_interrupt;
        void eop_eop_out_9_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_out_9_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_10_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_out_10_interrupt;
        void eop_eop_out_10_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_out_10_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_out_11_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_out_11_interrupt;
        void eop_eop_out_11_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_out_11_interrupt() const;
    
}; // cap_pbc_csr_int_pbus_violation_out_int_test_set_t
    
class cap_pbc_csr_int_pbus_violation_out_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_int_pbus_violation_out_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_int_pbus_violation_out_t(string name = "cap_pbc_csr_int_pbus_violation_out_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_int_pbus_violation_out_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbc_csr_int_pbus_violation_out_int_test_set_t intreg;
    
        cap_pbc_csr_int_pbus_violation_out_int_test_set_t int_test_set;
    
        cap_pbc_csr_int_pbus_violation_out_int_enable_clear_t int_enable_set;
    
        cap_pbc_csr_int_pbus_violation_out_int_enable_clear_t int_enable_clear;
    
}; // cap_pbc_csr_int_pbus_violation_out_t
    
class cap_pbc_csr_int_pbus_violation_in_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_int_pbus_violation_in_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_int_pbus_violation_in_int_enable_clear_t(string name = "cap_pbc_csr_int_pbus_violation_in_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_int_pbus_violation_in_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > sop_sop_in_0_enable_cpp_int_t;
        cpp_int int_var__sop_sop_in_0_enable;
        void sop_sop_in_0_enable (const cpp_int  & l__val);
        cpp_int sop_sop_in_0_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_1_enable_cpp_int_t;
        cpp_int int_var__sop_sop_in_1_enable;
        void sop_sop_in_1_enable (const cpp_int  & l__val);
        cpp_int sop_sop_in_1_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_2_enable_cpp_int_t;
        cpp_int int_var__sop_sop_in_2_enable;
        void sop_sop_in_2_enable (const cpp_int  & l__val);
        cpp_int sop_sop_in_2_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_3_enable_cpp_int_t;
        cpp_int int_var__sop_sop_in_3_enable;
        void sop_sop_in_3_enable (const cpp_int  & l__val);
        cpp_int sop_sop_in_3_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_4_enable_cpp_int_t;
        cpp_int int_var__sop_sop_in_4_enable;
        void sop_sop_in_4_enable (const cpp_int  & l__val);
        cpp_int sop_sop_in_4_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_5_enable_cpp_int_t;
        cpp_int int_var__sop_sop_in_5_enable;
        void sop_sop_in_5_enable (const cpp_int  & l__val);
        cpp_int sop_sop_in_5_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_6_enable_cpp_int_t;
        cpp_int int_var__sop_sop_in_6_enable;
        void sop_sop_in_6_enable (const cpp_int  & l__val);
        cpp_int sop_sop_in_6_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_7_enable_cpp_int_t;
        cpp_int int_var__sop_sop_in_7_enable;
        void sop_sop_in_7_enable (const cpp_int  & l__val);
        cpp_int sop_sop_in_7_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_8_enable_cpp_int_t;
        cpp_int int_var__sop_sop_in_8_enable;
        void sop_sop_in_8_enable (const cpp_int  & l__val);
        cpp_int sop_sop_in_8_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_9_enable_cpp_int_t;
        cpp_int int_var__sop_sop_in_9_enable;
        void sop_sop_in_9_enable (const cpp_int  & l__val);
        cpp_int sop_sop_in_9_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_10_enable_cpp_int_t;
        cpp_int int_var__sop_sop_in_10_enable;
        void sop_sop_in_10_enable (const cpp_int  & l__val);
        cpp_int sop_sop_in_10_enable() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_11_enable_cpp_int_t;
        cpp_int int_var__sop_sop_in_11_enable;
        void sop_sop_in_11_enable (const cpp_int  & l__val);
        cpp_int sop_sop_in_11_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_0_enable_cpp_int_t;
        cpp_int int_var__eop_eop_in_0_enable;
        void eop_eop_in_0_enable (const cpp_int  & l__val);
        cpp_int eop_eop_in_0_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_1_enable_cpp_int_t;
        cpp_int int_var__eop_eop_in_1_enable;
        void eop_eop_in_1_enable (const cpp_int  & l__val);
        cpp_int eop_eop_in_1_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_2_enable_cpp_int_t;
        cpp_int int_var__eop_eop_in_2_enable;
        void eop_eop_in_2_enable (const cpp_int  & l__val);
        cpp_int eop_eop_in_2_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_3_enable_cpp_int_t;
        cpp_int int_var__eop_eop_in_3_enable;
        void eop_eop_in_3_enable (const cpp_int  & l__val);
        cpp_int eop_eop_in_3_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_4_enable_cpp_int_t;
        cpp_int int_var__eop_eop_in_4_enable;
        void eop_eop_in_4_enable (const cpp_int  & l__val);
        cpp_int eop_eop_in_4_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_5_enable_cpp_int_t;
        cpp_int int_var__eop_eop_in_5_enable;
        void eop_eop_in_5_enable (const cpp_int  & l__val);
        cpp_int eop_eop_in_5_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_6_enable_cpp_int_t;
        cpp_int int_var__eop_eop_in_6_enable;
        void eop_eop_in_6_enable (const cpp_int  & l__val);
        cpp_int eop_eop_in_6_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_7_enable_cpp_int_t;
        cpp_int int_var__eop_eop_in_7_enable;
        void eop_eop_in_7_enable (const cpp_int  & l__val);
        cpp_int eop_eop_in_7_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_8_enable_cpp_int_t;
        cpp_int int_var__eop_eop_in_8_enable;
        void eop_eop_in_8_enable (const cpp_int  & l__val);
        cpp_int eop_eop_in_8_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_9_enable_cpp_int_t;
        cpp_int int_var__eop_eop_in_9_enable;
        void eop_eop_in_9_enable (const cpp_int  & l__val);
        cpp_int eop_eop_in_9_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_10_enable_cpp_int_t;
        cpp_int int_var__eop_eop_in_10_enable;
        void eop_eop_in_10_enable (const cpp_int  & l__val);
        cpp_int eop_eop_in_10_enable() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_11_enable_cpp_int_t;
        cpp_int int_var__eop_eop_in_11_enable;
        void eop_eop_in_11_enable (const cpp_int  & l__val);
        cpp_int eop_eop_in_11_enable() const;
    
}; // cap_pbc_csr_int_pbus_violation_in_int_enable_clear_t
    
class cap_pbc_csr_int_pbus_violation_in_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_int_pbus_violation_in_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_int_pbus_violation_in_int_test_set_t(string name = "cap_pbc_csr_int_pbus_violation_in_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_int_pbus_violation_in_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > sop_sop_in_0_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_in_0_interrupt;
        void sop_sop_in_0_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_in_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_1_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_in_1_interrupt;
        void sop_sop_in_1_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_in_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_2_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_in_2_interrupt;
        void sop_sop_in_2_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_in_2_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_3_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_in_3_interrupt;
        void sop_sop_in_3_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_in_3_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_4_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_in_4_interrupt;
        void sop_sop_in_4_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_in_4_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_5_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_in_5_interrupt;
        void sop_sop_in_5_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_in_5_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_6_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_in_6_interrupt;
        void sop_sop_in_6_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_in_6_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_7_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_in_7_interrupt;
        void sop_sop_in_7_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_in_7_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_8_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_in_8_interrupt;
        void sop_sop_in_8_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_in_8_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_9_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_in_9_interrupt;
        void sop_sop_in_9_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_in_9_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_10_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_in_10_interrupt;
        void sop_sop_in_10_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_in_10_interrupt() const;
    
        typedef pu_cpp_int< 1 > sop_sop_in_11_interrupt_cpp_int_t;
        cpp_int int_var__sop_sop_in_11_interrupt;
        void sop_sop_in_11_interrupt (const cpp_int  & l__val);
        cpp_int sop_sop_in_11_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_0_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_in_0_interrupt;
        void eop_eop_in_0_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_in_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_1_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_in_1_interrupt;
        void eop_eop_in_1_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_in_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_2_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_in_2_interrupt;
        void eop_eop_in_2_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_in_2_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_3_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_in_3_interrupt;
        void eop_eop_in_3_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_in_3_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_4_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_in_4_interrupt;
        void eop_eop_in_4_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_in_4_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_5_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_in_5_interrupt;
        void eop_eop_in_5_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_in_5_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_6_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_in_6_interrupt;
        void eop_eop_in_6_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_in_6_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_7_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_in_7_interrupt;
        void eop_eop_in_7_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_in_7_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_8_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_in_8_interrupt;
        void eop_eop_in_8_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_in_8_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_9_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_in_9_interrupt;
        void eop_eop_in_9_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_in_9_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_10_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_in_10_interrupt;
        void eop_eop_in_10_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_in_10_interrupt() const;
    
        typedef pu_cpp_int< 1 > eop_eop_in_11_interrupt_cpp_int_t;
        cpp_int int_var__eop_eop_in_11_interrupt;
        void eop_eop_in_11_interrupt (const cpp_int  & l__val);
        cpp_int eop_eop_in_11_interrupt() const;
    
}; // cap_pbc_csr_int_pbus_violation_in_int_test_set_t
    
class cap_pbc_csr_int_pbus_violation_in_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_int_pbus_violation_in_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_int_pbus_violation_in_t(string name = "cap_pbc_csr_int_pbus_violation_in_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_int_pbus_violation_in_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbc_csr_int_pbus_violation_in_int_test_set_t intreg;
    
        cap_pbc_csr_int_pbus_violation_in_int_test_set_t int_test_set;
    
        cap_pbc_csr_int_pbus_violation_in_int_enable_clear_t int_enable_set;
    
        cap_pbc_csr_int_pbus_violation_in_int_enable_clear_t int_enable_clear;
    
}; // cap_pbc_csr_int_pbus_violation_in_t
    
class cap_pbc_csr_int_write_11_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_int_write_11_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_int_write_11_int_enable_clear_t(string name = "cap_pbc_csr_int_write_11_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_int_write_11_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > out_of_cells_enable_cpp_int_t;
        cpp_int int_var__out_of_cells_enable;
        void out_of_cells_enable (const cpp_int  & l__val);
        cpp_int out_of_cells_enable() const;
    
        typedef pu_cpp_int< 1 > out_of_credit_enable_cpp_int_t;
        cpp_int int_var__out_of_credit_enable;
        void out_of_credit_enable (const cpp_int  & l__val);
        cpp_int out_of_credit_enable() const;
    
        typedef pu_cpp_int< 1 > port_disabled_enable_cpp_int_t;
        cpp_int int_var__port_disabled_enable;
        void port_disabled_enable (const cpp_int  & l__val);
        cpp_int port_disabled_enable() const;
    
        typedef pu_cpp_int< 1 > truncation_enable_cpp_int_t;
        cpp_int int_var__truncation_enable;
        void truncation_enable (const cpp_int  & l__val);
        cpp_int truncation_enable() const;
    
        typedef pu_cpp_int< 1 > intrinsic_drop_enable_cpp_int_t;
        cpp_int int_var__intrinsic_drop_enable;
        void intrinsic_drop_enable (const cpp_int  & l__val);
        cpp_int intrinsic_drop_enable() const;
    
        typedef pu_cpp_int< 1 > out_of_cells1_enable_cpp_int_t;
        cpp_int int_var__out_of_cells1_enable;
        void out_of_cells1_enable (const cpp_int  & l__val);
        cpp_int out_of_cells1_enable() const;
    
        typedef pu_cpp_int< 1 > enq_err_enable_cpp_int_t;
        cpp_int int_var__enq_err_enable;
        void enq_err_enable (const cpp_int  & l__val);
        cpp_int enq_err_enable() const;
    
        typedef pu_cpp_int< 1 > tail_drop_cpu_enable_cpp_int_t;
        cpp_int int_var__tail_drop_cpu_enable;
        void tail_drop_cpu_enable (const cpp_int  & l__val);
        cpp_int tail_drop_cpu_enable() const;
    
        typedef pu_cpp_int< 1 > tail_drop_span_enable_cpp_int_t;
        cpp_int int_var__tail_drop_span_enable;
        void tail_drop_span_enable (const cpp_int  & l__val);
        cpp_int tail_drop_span_enable() const;
    
        typedef pu_cpp_int< 1 > min_size_viol_enable_cpp_int_t;
        cpp_int int_var__min_size_viol_enable;
        void min_size_viol_enable (const cpp_int  & l__val);
        cpp_int min_size_viol_enable() const;
    
        typedef pu_cpp_int< 1 > port_range_enable_cpp_int_t;
        cpp_int int_var__port_range_enable;
        void port_range_enable (const cpp_int  & l__val);
        cpp_int port_range_enable() const;
    
        typedef pu_cpp_int< 1 > credit_growth_error_enable_cpp_int_t;
        cpp_int int_var__credit_growth_error_enable;
        void credit_growth_error_enable (const cpp_int  & l__val);
        cpp_int credit_growth_error_enable() const;
    
        typedef pu_cpp_int< 1 > oq_range_enable_cpp_int_t;
        cpp_int int_var__oq_range_enable;
        void oq_range_enable (const cpp_int  & l__val);
        cpp_int oq_range_enable() const;
    
        typedef pu_cpp_int< 1 > xoff_timeout_enable_cpp_int_t;
        cpp_int int_var__xoff_timeout_enable;
        void xoff_timeout_enable (const cpp_int  & l__val);
        cpp_int xoff_timeout_enable() const;
    
}; // cap_pbc_csr_int_write_11_int_enable_clear_t
    
class cap_pbc_csr_int_write_0_intreg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_int_write_0_intreg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_int_write_0_intreg_t(string name = "cap_pbc_csr_int_write_0_intreg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_int_write_0_intreg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > out_of_cells_interrupt_cpp_int_t;
        cpp_int int_var__out_of_cells_interrupt;
        void out_of_cells_interrupt (const cpp_int  & l__val);
        cpp_int out_of_cells_interrupt() const;
    
        typedef pu_cpp_int< 1 > out_of_credit_interrupt_cpp_int_t;
        cpp_int int_var__out_of_credit_interrupt;
        void out_of_credit_interrupt (const cpp_int  & l__val);
        cpp_int out_of_credit_interrupt() const;
    
        typedef pu_cpp_int< 1 > port_disabled_interrupt_cpp_int_t;
        cpp_int int_var__port_disabled_interrupt;
        void port_disabled_interrupt (const cpp_int  & l__val);
        cpp_int port_disabled_interrupt() const;
    
        typedef pu_cpp_int< 1 > truncation_interrupt_cpp_int_t;
        cpp_int int_var__truncation_interrupt;
        void truncation_interrupt (const cpp_int  & l__val);
        cpp_int truncation_interrupt() const;
    
        typedef pu_cpp_int< 1 > intrinsic_drop_interrupt_cpp_int_t;
        cpp_int int_var__intrinsic_drop_interrupt;
        void intrinsic_drop_interrupt (const cpp_int  & l__val);
        cpp_int intrinsic_drop_interrupt() const;
    
        typedef pu_cpp_int< 1 > out_of_cells1_interrupt_cpp_int_t;
        cpp_int int_var__out_of_cells1_interrupt;
        void out_of_cells1_interrupt (const cpp_int  & l__val);
        cpp_int out_of_cells1_interrupt() const;
    
        typedef pu_cpp_int< 1 > enq_err_interrupt_cpp_int_t;
        cpp_int int_var__enq_err_interrupt;
        void enq_err_interrupt (const cpp_int  & l__val);
        cpp_int enq_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > tail_drop_cpu_interrupt_cpp_int_t;
        cpp_int int_var__tail_drop_cpu_interrupt;
        void tail_drop_cpu_interrupt (const cpp_int  & l__val);
        cpp_int tail_drop_cpu_interrupt() const;
    
        typedef pu_cpp_int< 1 > tail_drop_span_interrupt_cpp_int_t;
        cpp_int int_var__tail_drop_span_interrupt;
        void tail_drop_span_interrupt (const cpp_int  & l__val);
        cpp_int tail_drop_span_interrupt() const;
    
        typedef pu_cpp_int< 1 > min_size_viol_interrupt_cpp_int_t;
        cpp_int int_var__min_size_viol_interrupt;
        void min_size_viol_interrupt (const cpp_int  & l__val);
        cpp_int min_size_viol_interrupt() const;
    
        typedef pu_cpp_int< 1 > port_range_interrupt_cpp_int_t;
        cpp_int int_var__port_range_interrupt;
        void port_range_interrupt (const cpp_int  & l__val);
        cpp_int port_range_interrupt() const;
    
        typedef pu_cpp_int< 1 > credit_growth_error_interrupt_cpp_int_t;
        cpp_int int_var__credit_growth_error_interrupt;
        void credit_growth_error_interrupt (const cpp_int  & l__val);
        cpp_int credit_growth_error_interrupt() const;
    
        typedef pu_cpp_int< 1 > oq_range_interrupt_cpp_int_t;
        cpp_int int_var__oq_range_interrupt;
        void oq_range_interrupt (const cpp_int  & l__val);
        cpp_int oq_range_interrupt() const;
    
        typedef pu_cpp_int< 1 > xoff_timeout_interrupt_cpp_int_t;
        cpp_int int_var__xoff_timeout_interrupt;
        void xoff_timeout_interrupt (const cpp_int  & l__val);
        cpp_int xoff_timeout_interrupt() const;
    
}; // cap_pbc_csr_int_write_0_intreg_t
    
class cap_pbc_csr_int_write_0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_int_write_0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_int_write_0_t(string name = "cap_pbc_csr_int_write_0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_int_write_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbc_csr_int_write_0_intreg_t intreg;
    
        cap_pbc_csr_int_write_0_intreg_t int_test_set;
    
        cap_pbc_csr_int_write_11_int_enable_clear_t int_enable_set;
    
        cap_pbc_csr_int_write_11_int_enable_clear_t int_enable_clear;
    
}; // cap_pbc_csr_int_write_0_t
    
class cap_pbc_csr_intreg_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_intreg_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_intreg_status_t(string name = "cap_pbc_csr_intreg_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_intreg_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > int_ecc_fc_0_interrupt_cpp_int_t;
        cpp_int int_var__int_ecc_fc_0_interrupt;
        void int_ecc_fc_0_interrupt (const cpp_int  & l__val);
        cpp_int int_ecc_fc_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_ecc_ll_0_interrupt_cpp_int_t;
        cpp_int int_var__int_ecc_ll_0_interrupt;
        void int_ecc_ll_0_interrupt (const cpp_int  & l__val);
        cpp_int int_ecc_ll_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_ecc_desc_0_interrupt_cpp_int_t;
        cpp_int int_var__int_ecc_desc_0_interrupt;
        void int_ecc_desc_0_interrupt (const cpp_int  & l__val);
        cpp_int int_ecc_desc_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_ecc_fc_1_interrupt_cpp_int_t;
        cpp_int int_var__int_ecc_fc_1_interrupt;
        void int_ecc_fc_1_interrupt (const cpp_int  & l__val);
        cpp_int int_ecc_fc_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_ecc_ll_1_interrupt_cpp_int_t;
        cpp_int int_var__int_ecc_ll_1_interrupt;
        void int_ecc_ll_1_interrupt (const cpp_int  & l__val);
        cpp_int int_ecc_ll_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_ecc_desc_1_interrupt_cpp_int_t;
        cpp_int int_var__int_ecc_desc_1_interrupt;
        void int_ecc_desc_1_interrupt (const cpp_int  & l__val);
        cpp_int int_ecc_desc_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_write_0_interrupt_cpp_int_t;
        cpp_int int_var__int_write_0_interrupt;
        void int_write_0_interrupt (const cpp_int  & l__val);
        cpp_int int_write_0_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_write_1_interrupt_cpp_int_t;
        cpp_int int_var__int_write_1_interrupt;
        void int_write_1_interrupt (const cpp_int  & l__val);
        cpp_int int_write_1_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_write_2_interrupt_cpp_int_t;
        cpp_int int_var__int_write_2_interrupt;
        void int_write_2_interrupt (const cpp_int  & l__val);
        cpp_int int_write_2_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_write_3_interrupt_cpp_int_t;
        cpp_int int_var__int_write_3_interrupt;
        void int_write_3_interrupt (const cpp_int  & l__val);
        cpp_int int_write_3_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_write_4_interrupt_cpp_int_t;
        cpp_int int_var__int_write_4_interrupt;
        void int_write_4_interrupt (const cpp_int  & l__val);
        cpp_int int_write_4_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_write_5_interrupt_cpp_int_t;
        cpp_int int_var__int_write_5_interrupt;
        void int_write_5_interrupt (const cpp_int  & l__val);
        cpp_int int_write_5_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_write_6_interrupt_cpp_int_t;
        cpp_int int_var__int_write_6_interrupt;
        void int_write_6_interrupt (const cpp_int  & l__val);
        cpp_int int_write_6_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_write_7_interrupt_cpp_int_t;
        cpp_int int_var__int_write_7_interrupt;
        void int_write_7_interrupt (const cpp_int  & l__val);
        cpp_int int_write_7_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_write_8_interrupt_cpp_int_t;
        cpp_int int_var__int_write_8_interrupt;
        void int_write_8_interrupt (const cpp_int  & l__val);
        cpp_int int_write_8_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_write_9_interrupt_cpp_int_t;
        cpp_int int_var__int_write_9_interrupt;
        void int_write_9_interrupt (const cpp_int  & l__val);
        cpp_int int_write_9_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_write_10_interrupt_cpp_int_t;
        cpp_int int_var__int_write_10_interrupt;
        void int_write_10_interrupt (const cpp_int  & l__val);
        cpp_int int_write_10_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_write_11_interrupt_cpp_int_t;
        cpp_int int_var__int_write_11_interrupt;
        void int_write_11_interrupt (const cpp_int  & l__val);
        cpp_int int_write_11_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_ecc_rc_interrupt_cpp_int_t;
        cpp_int int_var__int_ecc_rc_interrupt;
        void int_ecc_rc_interrupt (const cpp_int  & l__val);
        cpp_int int_ecc_rc_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_ecc_pack_interrupt_cpp_int_t;
        cpp_int int_var__int_ecc_pack_interrupt;
        void int_ecc_pack_interrupt (const cpp_int  & l__val);
        cpp_int int_ecc_pack_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_ecc_rwr_interrupt_cpp_int_t;
        cpp_int int_var__int_ecc_rwr_interrupt;
        void int_ecc_rwr_interrupt (const cpp_int  & l__val);
        cpp_int int_ecc_rwr_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_ecc_sched_interrupt_cpp_int_t;
        cpp_int int_var__int_ecc_sched_interrupt;
        void int_ecc_sched_interrupt (const cpp_int  & l__val);
        cpp_int int_ecc_sched_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_ecc_sideband_interrupt_cpp_int_t;
        cpp_int int_var__int_ecc_sideband_interrupt;
        void int_ecc_sideband_interrupt (const cpp_int  & l__val);
        cpp_int int_ecc_sideband_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_ecc_nc_interrupt_cpp_int_t;
        cpp_int int_var__int_ecc_nc_interrupt;
        void int_ecc_nc_interrupt (const cpp_int  & l__val);
        cpp_int int_ecc_nc_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_ecc_uc_interrupt_cpp_int_t;
        cpp_int int_var__int_ecc_uc_interrupt;
        void int_ecc_uc_interrupt (const cpp_int  & l__val);
        cpp_int int_ecc_uc_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_ecc_port_mon_in_interrupt_cpp_int_t;
        cpp_int int_var__int_ecc_port_mon_in_interrupt;
        void int_ecc_port_mon_in_interrupt (const cpp_int  & l__val);
        cpp_int int_ecc_port_mon_in_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_ecc_port_mon_out_interrupt_cpp_int_t;
        cpp_int int_var__int_ecc_port_mon_out_interrupt;
        void int_ecc_port_mon_out_interrupt (const cpp_int  & l__val);
        cpp_int int_ecc_port_mon_out_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_rpl_interrupt_cpp_int_t;
        cpp_int int_var__int_rpl_interrupt;
        void int_rpl_interrupt (const cpp_int  & l__val);
        cpp_int int_rpl_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_credit_underflow_interrupt_cpp_int_t;
        cpp_int int_var__int_credit_underflow_interrupt;
        void int_credit_underflow_interrupt (const cpp_int  & l__val);
        cpp_int int_credit_underflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_pbus_violation_in_interrupt_cpp_int_t;
        cpp_int int_var__int_pbus_violation_in_interrupt;
        void int_pbus_violation_in_interrupt (const cpp_int  & l__val);
        cpp_int int_pbus_violation_in_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_pbus_violation_out_interrupt_cpp_int_t;
        cpp_int int_var__int_pbus_violation_out_interrupt;
        void int_pbus_violation_out_interrupt (const cpp_int  & l__val);
        cpp_int int_pbus_violation_out_interrupt() const;
    
}; // cap_pbc_csr_intreg_status_t
    
class cap_pbc_csr_int_groups_int_enable_rw_reg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_int_groups_int_enable_rw_reg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_int_groups_int_enable_rw_reg_t(string name = "cap_pbc_csr_int_groups_int_enable_rw_reg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_int_groups_int_enable_rw_reg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > int_ecc_fc_0_enable_cpp_int_t;
        cpp_int int_var__int_ecc_fc_0_enable;
        void int_ecc_fc_0_enable (const cpp_int  & l__val);
        cpp_int int_ecc_fc_0_enable() const;
    
        typedef pu_cpp_int< 1 > int_ecc_ll_0_enable_cpp_int_t;
        cpp_int int_var__int_ecc_ll_0_enable;
        void int_ecc_ll_0_enable (const cpp_int  & l__val);
        cpp_int int_ecc_ll_0_enable() const;
    
        typedef pu_cpp_int< 1 > int_ecc_desc_0_enable_cpp_int_t;
        cpp_int int_var__int_ecc_desc_0_enable;
        void int_ecc_desc_0_enable (const cpp_int  & l__val);
        cpp_int int_ecc_desc_0_enable() const;
    
        typedef pu_cpp_int< 1 > int_ecc_fc_1_enable_cpp_int_t;
        cpp_int int_var__int_ecc_fc_1_enable;
        void int_ecc_fc_1_enable (const cpp_int  & l__val);
        cpp_int int_ecc_fc_1_enable() const;
    
        typedef pu_cpp_int< 1 > int_ecc_ll_1_enable_cpp_int_t;
        cpp_int int_var__int_ecc_ll_1_enable;
        void int_ecc_ll_1_enable (const cpp_int  & l__val);
        cpp_int int_ecc_ll_1_enable() const;
    
        typedef pu_cpp_int< 1 > int_ecc_desc_1_enable_cpp_int_t;
        cpp_int int_var__int_ecc_desc_1_enable;
        void int_ecc_desc_1_enable (const cpp_int  & l__val);
        cpp_int int_ecc_desc_1_enable() const;
    
        typedef pu_cpp_int< 1 > int_write_0_enable_cpp_int_t;
        cpp_int int_var__int_write_0_enable;
        void int_write_0_enable (const cpp_int  & l__val);
        cpp_int int_write_0_enable() const;
    
        typedef pu_cpp_int< 1 > int_write_1_enable_cpp_int_t;
        cpp_int int_var__int_write_1_enable;
        void int_write_1_enable (const cpp_int  & l__val);
        cpp_int int_write_1_enable() const;
    
        typedef pu_cpp_int< 1 > int_write_2_enable_cpp_int_t;
        cpp_int int_var__int_write_2_enable;
        void int_write_2_enable (const cpp_int  & l__val);
        cpp_int int_write_2_enable() const;
    
        typedef pu_cpp_int< 1 > int_write_3_enable_cpp_int_t;
        cpp_int int_var__int_write_3_enable;
        void int_write_3_enable (const cpp_int  & l__val);
        cpp_int int_write_3_enable() const;
    
        typedef pu_cpp_int< 1 > int_write_4_enable_cpp_int_t;
        cpp_int int_var__int_write_4_enable;
        void int_write_4_enable (const cpp_int  & l__val);
        cpp_int int_write_4_enable() const;
    
        typedef pu_cpp_int< 1 > int_write_5_enable_cpp_int_t;
        cpp_int int_var__int_write_5_enable;
        void int_write_5_enable (const cpp_int  & l__val);
        cpp_int int_write_5_enable() const;
    
        typedef pu_cpp_int< 1 > int_write_6_enable_cpp_int_t;
        cpp_int int_var__int_write_6_enable;
        void int_write_6_enable (const cpp_int  & l__val);
        cpp_int int_write_6_enable() const;
    
        typedef pu_cpp_int< 1 > int_write_7_enable_cpp_int_t;
        cpp_int int_var__int_write_7_enable;
        void int_write_7_enable (const cpp_int  & l__val);
        cpp_int int_write_7_enable() const;
    
        typedef pu_cpp_int< 1 > int_write_8_enable_cpp_int_t;
        cpp_int int_var__int_write_8_enable;
        void int_write_8_enable (const cpp_int  & l__val);
        cpp_int int_write_8_enable() const;
    
        typedef pu_cpp_int< 1 > int_write_9_enable_cpp_int_t;
        cpp_int int_var__int_write_9_enable;
        void int_write_9_enable (const cpp_int  & l__val);
        cpp_int int_write_9_enable() const;
    
        typedef pu_cpp_int< 1 > int_write_10_enable_cpp_int_t;
        cpp_int int_var__int_write_10_enable;
        void int_write_10_enable (const cpp_int  & l__val);
        cpp_int int_write_10_enable() const;
    
        typedef pu_cpp_int< 1 > int_write_11_enable_cpp_int_t;
        cpp_int int_var__int_write_11_enable;
        void int_write_11_enable (const cpp_int  & l__val);
        cpp_int int_write_11_enable() const;
    
        typedef pu_cpp_int< 1 > int_ecc_rc_enable_cpp_int_t;
        cpp_int int_var__int_ecc_rc_enable;
        void int_ecc_rc_enable (const cpp_int  & l__val);
        cpp_int int_ecc_rc_enable() const;
    
        typedef pu_cpp_int< 1 > int_ecc_pack_enable_cpp_int_t;
        cpp_int int_var__int_ecc_pack_enable;
        void int_ecc_pack_enable (const cpp_int  & l__val);
        cpp_int int_ecc_pack_enable() const;
    
        typedef pu_cpp_int< 1 > int_ecc_rwr_enable_cpp_int_t;
        cpp_int int_var__int_ecc_rwr_enable;
        void int_ecc_rwr_enable (const cpp_int  & l__val);
        cpp_int int_ecc_rwr_enable() const;
    
        typedef pu_cpp_int< 1 > int_ecc_sched_enable_cpp_int_t;
        cpp_int int_var__int_ecc_sched_enable;
        void int_ecc_sched_enable (const cpp_int  & l__val);
        cpp_int int_ecc_sched_enable() const;
    
        typedef pu_cpp_int< 1 > int_ecc_sideband_enable_cpp_int_t;
        cpp_int int_var__int_ecc_sideband_enable;
        void int_ecc_sideband_enable (const cpp_int  & l__val);
        cpp_int int_ecc_sideband_enable() const;
    
        typedef pu_cpp_int< 1 > int_ecc_nc_enable_cpp_int_t;
        cpp_int int_var__int_ecc_nc_enable;
        void int_ecc_nc_enable (const cpp_int  & l__val);
        cpp_int int_ecc_nc_enable() const;
    
        typedef pu_cpp_int< 1 > int_ecc_uc_enable_cpp_int_t;
        cpp_int int_var__int_ecc_uc_enable;
        void int_ecc_uc_enable (const cpp_int  & l__val);
        cpp_int int_ecc_uc_enable() const;
    
        typedef pu_cpp_int< 1 > int_ecc_port_mon_in_enable_cpp_int_t;
        cpp_int int_var__int_ecc_port_mon_in_enable;
        void int_ecc_port_mon_in_enable (const cpp_int  & l__val);
        cpp_int int_ecc_port_mon_in_enable() const;
    
        typedef pu_cpp_int< 1 > int_ecc_port_mon_out_enable_cpp_int_t;
        cpp_int int_var__int_ecc_port_mon_out_enable;
        void int_ecc_port_mon_out_enable (const cpp_int  & l__val);
        cpp_int int_ecc_port_mon_out_enable() const;
    
        typedef pu_cpp_int< 1 > int_rpl_enable_cpp_int_t;
        cpp_int int_var__int_rpl_enable;
        void int_rpl_enable (const cpp_int  & l__val);
        cpp_int int_rpl_enable() const;
    
        typedef pu_cpp_int< 1 > int_credit_underflow_enable_cpp_int_t;
        cpp_int int_var__int_credit_underflow_enable;
        void int_credit_underflow_enable (const cpp_int  & l__val);
        cpp_int int_credit_underflow_enable() const;
    
        typedef pu_cpp_int< 1 > int_pbus_violation_in_enable_cpp_int_t;
        cpp_int int_var__int_pbus_violation_in_enable;
        void int_pbus_violation_in_enable (const cpp_int  & l__val);
        cpp_int int_pbus_violation_in_enable() const;
    
        typedef pu_cpp_int< 1 > int_pbus_violation_out_enable_cpp_int_t;
        cpp_int int_var__int_pbus_violation_out_enable;
        void int_pbus_violation_out_enable (const cpp_int  & l__val);
        cpp_int int_pbus_violation_out_enable() const;
    
}; // cap_pbc_csr_int_groups_int_enable_rw_reg_t
    
class cap_pbc_csr_intgrp_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_intgrp_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_intgrp_status_t(string name = "cap_pbc_csr_intgrp_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_intgrp_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbc_csr_intreg_status_t intreg;
    
        cap_pbc_csr_int_groups_int_enable_rw_reg_t int_enable_rw_reg;
    
        cap_pbc_csr_intreg_status_t int_rw_reg;
    
}; // cap_pbc_csr_intgrp_status_t
    
class cap_pbc_csr_dhs_fc_backdoor_1_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_fc_backdoor_1_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_fc_backdoor_1_entry_t(string name = "cap_pbc_csr_dhs_fc_backdoor_1_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_fc_backdoor_1_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 14 > cell_cpp_int_t;
        cpp_int int_var__cell;
        void cell (const cpp_int  & l__val);
        cpp_int cell() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        cpp_int int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
}; // cap_pbc_csr_dhs_fc_backdoor_1_entry_t
    
class cap_pbc_csr_dhs_fc_backdoor_1_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_fc_backdoor_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_fc_backdoor_1_t(string name = "cap_pbc_csr_dhs_fc_backdoor_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_fc_backdoor_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbc_csr_dhs_fc_backdoor_1_entry_t entry;
    
}; // cap_pbc_csr_dhs_fc_backdoor_1_t
    
class cap_pbc_csr_dhs_fc_backdoor_0_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_fc_backdoor_0_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_fc_backdoor_0_entry_t(string name = "cap_pbc_csr_dhs_fc_backdoor_0_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_fc_backdoor_0_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 14 > cell_cpp_int_t;
        cpp_int int_var__cell;
        void cell (const cpp_int  & l__val);
        cpp_int cell() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        cpp_int int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
}; // cap_pbc_csr_dhs_fc_backdoor_0_entry_t
    
class cap_pbc_csr_dhs_fc_backdoor_0_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_fc_backdoor_0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_fc_backdoor_0_t(string name = "cap_pbc_csr_dhs_fc_backdoor_0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_fc_backdoor_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbc_csr_dhs_fc_backdoor_0_entry_t entry;
    
}; // cap_pbc_csr_dhs_fc_backdoor_0_t
    
class cap_pbc_csr_dhs_port_mon_out_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_port_mon_out_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_port_mon_out_entry_t(string name = "cap_pbc_csr_dhs_port_mon_out_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_port_mon_out_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 2 > command_cpp_int_t;
        cpp_int int_var__command;
        void command (const cpp_int  & l__val);
        cpp_int command() const;
    
        typedef pu_cpp_int< 32 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_pbc_csr_dhs_port_mon_out_entry_t
    
class cap_pbc_csr_dhs_port_mon_out_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_port_mon_out_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_port_mon_out_t(string name = "cap_pbc_csr_dhs_port_mon_out_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_port_mon_out_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbc_csr_dhs_port_mon_out_entry_t entry;
    
}; // cap_pbc_csr_dhs_port_mon_out_t
    
class cap_pbc_csr_dhs_port_mon_in_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_port_mon_in_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_port_mon_in_entry_t(string name = "cap_pbc_csr_dhs_port_mon_in_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_port_mon_in_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 2 > command_cpp_int_t;
        cpp_int int_var__command;
        void command (const cpp_int  & l__val);
        cpp_int command() const;
    
        typedef pu_cpp_int< 32 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_pbc_csr_dhs_port_mon_in_entry_t
    
class cap_pbc_csr_dhs_port_mon_in_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_port_mon_in_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_port_mon_in_t(string name = "cap_pbc_csr_dhs_port_mon_in_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_port_mon_in_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbc_csr_dhs_port_mon_in_entry_t entry;
    
}; // cap_pbc_csr_dhs_port_mon_in_t
    
class cap_pbc_csr_dhs_uc_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_uc_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_uc_entry_t(string name = "cap_pbc_csr_dhs_uc_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_uc_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 14 > next_cell_cpp_int_t;
        cpp_int int_var__next_cell;
        void next_cell (const cpp_int  & l__val);
        cpp_int next_cell() const;
    
        typedef pu_cpp_int< 16 > rpl_ptr_cpp_int_t;
        cpp_int int_var__rpl_ptr;
        void rpl_ptr (const cpp_int  & l__val);
        cpp_int rpl_ptr() const;
    
        typedef pu_cpp_int< 1 > rpl_en_cpp_int_t;
        cpp_int int_var__rpl_en;
        void rpl_en (const cpp_int  & l__val);
        cpp_int rpl_en() const;
    
}; // cap_pbc_csr_dhs_uc_entry_t
    
class cap_pbc_csr_dhs_uc_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_uc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_uc_t(string name = "cap_pbc_csr_dhs_uc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_uc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbc_csr_dhs_uc_entry_t entry;
    
}; // cap_pbc_csr_dhs_uc_t
    
class cap_pbc_csr_dhs_nc_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_nc_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_nc_entry_t(string name = "cap_pbc_csr_dhs_nc_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_nc_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 5 > entry_cpp_int_t;
        cpp_int int_var__entry;
        void entry (const cpp_int  & l__val);
        cpp_int entry() const;
    
}; // cap_pbc_csr_dhs_nc_entry_t
    
class cap_pbc_csr_dhs_nc_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_nc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_nc_t(string name = "cap_pbc_csr_dhs_nc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_nc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbc_csr_dhs_nc_entry_t entry;
    
}; // cap_pbc_csr_dhs_nc_t
    
class cap_pbc_csr_dhs_sp1_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_sp1_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_sp1_entry_t(string name = "cap_pbc_csr_dhs_sp1_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_sp1_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 14 > credits_cpp_int_t;
        cpp_int int_var__credits;
        void credits (const cpp_int  & l__val);
        cpp_int credits() const;
    
}; // cap_pbc_csr_dhs_sp1_entry_t
    
class cap_pbc_csr_dhs_sp1_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_sp1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_sp1_t(string name = "cap_pbc_csr_dhs_sp1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_sp1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pbc_csr_dhs_sp1_entry_t, 4> entry;
        #else 
        cap_pbc_csr_dhs_sp1_entry_t entry[4];
        #endif
        int get_depth_entry() { return 4; }
    
}; // cap_pbc_csr_dhs_sp1_t
    
class cap_pbc_csr_dhs_sp0_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_sp0_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_sp0_entry_t(string name = "cap_pbc_csr_dhs_sp0_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_sp0_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 14 > credits_cpp_int_t;
        cpp_int int_var__credits;
        void credits (const cpp_int  & l__val);
        cpp_int credits() const;
    
}; // cap_pbc_csr_dhs_sp0_entry_t
    
class cap_pbc_csr_dhs_sp0_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_sp0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_sp0_t(string name = "cap_pbc_csr_dhs_sp0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_sp0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pbc_csr_dhs_sp0_entry_t, 4> entry;
        #else 
        cap_pbc_csr_dhs_sp0_entry_t entry[4];
        #endif
        int get_depth_entry() { return 4; }
    
}; // cap_pbc_csr_dhs_sp0_t
    
class cap_pbc_csr_dhs_sched_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_sched_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_sched_entry_t(string name = "cap_pbc_csr_dhs_sched_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_sched_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > command_cpp_int_t;
        cpp_int int_var__command;
        void command (const cpp_int  & l__val);
        cpp_int command() const;
    
        typedef pu_cpp_int< 32 > current_credit_cpp_int_t;
        cpp_int int_var__current_credit;
        void current_credit (const cpp_int  & l__val);
        cpp_int current_credit() const;
    
        typedef pu_cpp_int< 32 > quota_cpp_int_t;
        cpp_int int_var__quota;
        void quota (const cpp_int  & l__val);
        cpp_int quota() const;
    
}; // cap_pbc_csr_dhs_sched_entry_t
    
class cap_pbc_csr_dhs_sched_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_sched_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_sched_t(string name = "cap_pbc_csr_dhs_sched_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_sched_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
        cap_csr_large_array_wrapper<cap_pbc_csr_dhs_sched_entry_t, 512> entry;
        #else 
        cap_pbc_csr_dhs_sched_entry_t entry[512];
        #endif
        int get_depth_entry() { return 512; }
    
}; // cap_pbc_csr_dhs_sched_t
    
class cap_pbc_csr_dhs_rc_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_rc_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_rc_entry_t(string name = "cap_pbc_csr_dhs_rc_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_rc_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 3 > count0_cpp_int_t;
        cpp_int int_var__count0;
        void count0 (const cpp_int  & l__val);
        cpp_int count0() const;
    
        typedef pu_cpp_int< 3 > count1_cpp_int_t;
        cpp_int int_var__count1;
        void count1 (const cpp_int  & l__val);
        cpp_int count1() const;
    
        typedef pu_cpp_int< 3 > count2_cpp_int_t;
        cpp_int int_var__count2;
        void count2 (const cpp_int  & l__val);
        cpp_int count2() const;
    
        typedef pu_cpp_int< 3 > count3_cpp_int_t;
        cpp_int int_var__count3;
        void count3 (const cpp_int  & l__val);
        cpp_int count3() const;
    
        typedef pu_cpp_int< 3 > count4_cpp_int_t;
        cpp_int int_var__count4;
        void count4 (const cpp_int  & l__val);
        cpp_int count4() const;
    
        typedef pu_cpp_int< 3 > count5_cpp_int_t;
        cpp_int int_var__count5;
        void count5 (const cpp_int  & l__val);
        cpp_int count5() const;
    
        typedef pu_cpp_int< 3 > count6_cpp_int_t;
        cpp_int int_var__count6;
        void count6 (const cpp_int  & l__val);
        cpp_int count6() const;
    
        typedef pu_cpp_int< 3 > count7_cpp_int_t;
        cpp_int int_var__count7;
        void count7 (const cpp_int  & l__val);
        cpp_int count7() const;
    
}; // cap_pbc_csr_dhs_rc_entry_t
    
class cap_pbc_csr_dhs_rc_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_rc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_rc_t(string name = "cap_pbc_csr_dhs_rc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_rc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbc_csr_dhs_rc_entry_t entry;
    
}; // cap_pbc_csr_dhs_rc_t
    
class cap_pbc_csr_dhs_desc_1_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_desc_1_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_desc_1_entry_t(string name = "cap_pbc_csr_dhs_desc_1_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_desc_1_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 4 > src_port_cpp_int_t;
        cpp_int int_var__src_port;
        void src_port (const cpp_int  & l__val);
        cpp_int src_port() const;
    
        typedef pu_cpp_int< 3 > refcount_cpp_int_t;
        cpp_int int_var__refcount;
        void refcount (const cpp_int  & l__val);
        cpp_int refcount() const;
    
        typedef pu_cpp_int< 3 > input_queue_cpp_int_t;
        cpp_int int_var__input_queue;
        void input_queue (const cpp_int  & l__val);
        cpp_int input_queue() const;
    
        typedef pu_cpp_int< 5 > eop_pack_cpp_int_t;
        cpp_int int_var__eop_pack;
        void eop_pack (const cpp_int  & l__val);
        cpp_int eop_pack() const;
    
        typedef pu_cpp_int< 5 > err_pack_cpp_int_t;
        cpp_int int_var__err_pack;
        void err_pack (const cpp_int  & l__val);
        cpp_int err_pack() const;
    
}; // cap_pbc_csr_dhs_desc_1_entry_t
    
class cap_pbc_csr_dhs_desc_1_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_desc_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_desc_1_t(string name = "cap_pbc_csr_dhs_desc_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_desc_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbc_csr_dhs_desc_1_entry_t entry;
    
}; // cap_pbc_csr_dhs_desc_1_t
    
class cap_pbc_csr_dhs_ll_1_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_ll_1_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_ll_1_entry_t(string name = "cap_pbc_csr_dhs_ll_1_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_ll_1_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 14 > next_cell_cpp_int_t;
        cpp_int int_var__next_cell;
        void next_cell (const cpp_int  & l__val);
        cpp_int next_cell() const;
    
        typedef pu_cpp_int< 1 > eop_cpp_int_t;
        cpp_int int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
        typedef pu_cpp_int< 1 > full_cpp_int_t;
        cpp_int int_var__full;
        void full (const cpp_int  & l__val);
        cpp_int full() const;
    
        typedef pu_cpp_int< 1 > ge256B_cpp_int_t;
        cpp_int int_var__ge256B;
        void ge256B (const cpp_int  & l__val);
        cpp_int ge256B() const;
    
        typedef pu_cpp_int< 1 > err_cpp_int_t;
        cpp_int int_var__err;
        void err (const cpp_int  & l__val);
        cpp_int err() const;
    
}; // cap_pbc_csr_dhs_ll_1_entry_t
    
class cap_pbc_csr_dhs_ll_1_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_ll_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_ll_1_t(string name = "cap_pbc_csr_dhs_ll_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_ll_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbc_csr_dhs_ll_1_entry_t entry;
    
}; // cap_pbc_csr_dhs_ll_1_t
    
class cap_pbc_csr_dhs_fc_1_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_fc_1_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_fc_1_entry_t(string name = "cap_pbc_csr_dhs_fc_1_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_fc_1_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 14 > cell0_cpp_int_t;
        cpp_int int_var__cell0;
        void cell0 (const cpp_int  & l__val);
        cpp_int cell0() const;
    
        typedef pu_cpp_int< 14 > cell1_cpp_int_t;
        cpp_int int_var__cell1;
        void cell1 (const cpp_int  & l__val);
        cpp_int cell1() const;
    
}; // cap_pbc_csr_dhs_fc_1_entry_t
    
class cap_pbc_csr_dhs_fc_1_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_fc_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_fc_1_t(string name = "cap_pbc_csr_dhs_fc_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_fc_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbc_csr_dhs_fc_1_entry_t entry;
    
}; // cap_pbc_csr_dhs_fc_1_t
    
class cap_pbc_csr_dhs_desc_0_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_desc_0_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_desc_0_entry_t(string name = "cap_pbc_csr_dhs_desc_0_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_desc_0_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 4 > src_port_cpp_int_t;
        cpp_int int_var__src_port;
        void src_port (const cpp_int  & l__val);
        cpp_int src_port() const;
    
        typedef pu_cpp_int< 3 > refcount_cpp_int_t;
        cpp_int int_var__refcount;
        void refcount (const cpp_int  & l__val);
        cpp_int refcount() const;
    
        typedef pu_cpp_int< 3 > input_queue_cpp_int_t;
        cpp_int int_var__input_queue;
        void input_queue (const cpp_int  & l__val);
        cpp_int input_queue() const;
    
        typedef pu_cpp_int< 5 > eop_pack_cpp_int_t;
        cpp_int int_var__eop_pack;
        void eop_pack (const cpp_int  & l__val);
        cpp_int eop_pack() const;
    
        typedef pu_cpp_int< 5 > err_pack_cpp_int_t;
        cpp_int int_var__err_pack;
        void err_pack (const cpp_int  & l__val);
        cpp_int err_pack() const;
    
}; // cap_pbc_csr_dhs_desc_0_entry_t
    
class cap_pbc_csr_dhs_desc_0_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_desc_0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_desc_0_t(string name = "cap_pbc_csr_dhs_desc_0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_desc_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbc_csr_dhs_desc_0_entry_t entry;
    
}; // cap_pbc_csr_dhs_desc_0_t
    
class cap_pbc_csr_dhs_ll_0_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_ll_0_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_ll_0_entry_t(string name = "cap_pbc_csr_dhs_ll_0_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_ll_0_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 14 > next_cell_cpp_int_t;
        cpp_int int_var__next_cell;
        void next_cell (const cpp_int  & l__val);
        cpp_int next_cell() const;
    
        typedef pu_cpp_int< 1 > eop_cpp_int_t;
        cpp_int int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
        typedef pu_cpp_int< 1 > full_cpp_int_t;
        cpp_int int_var__full;
        void full (const cpp_int  & l__val);
        cpp_int full() const;
    
        typedef pu_cpp_int< 1 > ge256B_cpp_int_t;
        cpp_int int_var__ge256B;
        void ge256B (const cpp_int  & l__val);
        cpp_int ge256B() const;
    
        typedef pu_cpp_int< 1 > err_cpp_int_t;
        cpp_int int_var__err;
        void err (const cpp_int  & l__val);
        cpp_int err() const;
    
}; // cap_pbc_csr_dhs_ll_0_entry_t
    
class cap_pbc_csr_dhs_ll_0_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_ll_0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_ll_0_t(string name = "cap_pbc_csr_dhs_ll_0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_ll_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbc_csr_dhs_ll_0_entry_t entry;
    
}; // cap_pbc_csr_dhs_ll_0_t
    
class cap_pbc_csr_dhs_fc_0_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_fc_0_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_fc_0_entry_t(string name = "cap_pbc_csr_dhs_fc_0_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_fc_0_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 14 > cell0_cpp_int_t;
        cpp_int int_var__cell0;
        void cell0 (const cpp_int  & l__val);
        cpp_int cell0() const;
    
        typedef pu_cpp_int< 14 > cell1_cpp_int_t;
        cpp_int int_var__cell1;
        void cell1 (const cpp_int  & l__val);
        cpp_int cell1() const;
    
}; // cap_pbc_csr_dhs_fc_0_entry_t
    
class cap_pbc_csr_dhs_fc_0_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_dhs_fc_0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_dhs_fc_0_t(string name = "cap_pbc_csr_dhs_fc_0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_dhs_fc_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbc_csr_dhs_fc_0_entry_t entry;
    
}; // cap_pbc_csr_dhs_fc_0_t
    
class cap_pbc_csr_sta_ecc_rxd_sideband_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_sta_ecc_rxd_sideband_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_sta_ecc_rxd_sideband_t(string name = "cap_pbc_csr_sta_ecc_rxd_sideband_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_sta_ecc_rxd_sideband_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 14 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pbc_csr_sta_ecc_rxd_sideband_t
    
class cap_pbc_csr_sta_ecc_rwr_sideband_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_sta_ecc_rwr_sideband_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_sta_ecc_rwr_sideband_t(string name = "cap_pbc_csr_sta_ecc_rwr_sideband_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_sta_ecc_rwr_sideband_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 14 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pbc_csr_sta_ecc_rwr_sideband_t
    
class cap_pbc_csr_cfg_axi_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_axi_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_axi_t(string name = "cap_pbc_csr_cfg_axi_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_axi_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 64 > base_addr_cpp_int_t;
        cpp_int int_var__base_addr;
        void base_addr (const cpp_int  & l__val);
        cpp_int base_addr() const;
    
}; // cap_pbc_csr_cfg_axi_t
    
class cap_pbc_csr_sta_bist_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_sta_bist_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_sta_bist_t(string name = "cap_pbc_csr_sta_bist_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_sta_bist_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 2 > linked_list_done_fail_cpp_int_t;
        cpp_int int_var__linked_list_done_fail;
        void linked_list_done_fail (const cpp_int  & l__val);
        cpp_int linked_list_done_fail() const;
    
        typedef pu_cpp_int< 2 > linked_list_done_pass_cpp_int_t;
        cpp_int int_var__linked_list_done_pass;
        void linked_list_done_pass (const cpp_int  & l__val);
        cpp_int linked_list_done_pass() const;
    
        typedef pu_cpp_int< 2 > free_cell_done_fail_cpp_int_t;
        cpp_int int_var__free_cell_done_fail;
        void free_cell_done_fail (const cpp_int  & l__val);
        cpp_int free_cell_done_fail() const;
    
        typedef pu_cpp_int< 2 > free_cell_done_pass_cpp_int_t;
        cpp_int int_var__free_cell_done_pass;
        void free_cell_done_pass (const cpp_int  & l__val);
        cpp_int free_cell_done_pass() const;
    
        typedef pu_cpp_int< 1 > refcount_done_fail_cpp_int_t;
        cpp_int int_var__refcount_done_fail;
        void refcount_done_fail (const cpp_int  & l__val);
        cpp_int refcount_done_fail() const;
    
        typedef pu_cpp_int< 1 > refcount_done_pass_cpp_int_t;
        cpp_int int_var__refcount_done_pass;
        void refcount_done_pass (const cpp_int  & l__val);
        cpp_int refcount_done_pass() const;
    
        typedef pu_cpp_int< 1 > num_cells_done_fail_cpp_int_t;
        cpp_int int_var__num_cells_done_fail;
        void num_cells_done_fail (const cpp_int  & l__val);
        cpp_int num_cells_done_fail() const;
    
        typedef pu_cpp_int< 1 > num_cells_done_pass_cpp_int_t;
        cpp_int int_var__num_cells_done_pass;
        void num_cells_done_pass (const cpp_int  & l__val);
        cpp_int num_cells_done_pass() const;
    
        typedef pu_cpp_int< 8 > oq_pck_done_fail_cpp_int_t;
        cpp_int int_var__oq_pck_done_fail;
        void oq_pck_done_fail (const cpp_int  & l__val);
        cpp_int oq_pck_done_fail() const;
    
        typedef pu_cpp_int< 8 > oq_pck_done_pass_cpp_int_t;
        cpp_int int_var__oq_pck_done_pass;
        void oq_pck_done_pass (const cpp_int  & l__val);
        cpp_int oq_pck_done_pass() const;
    
        typedef pu_cpp_int< 1 > unicast_mgr_done_fail_cpp_int_t;
        cpp_int int_var__unicast_mgr_done_fail;
        void unicast_mgr_done_fail (const cpp_int  & l__val);
        cpp_int unicast_mgr_done_fail() const;
    
        typedef pu_cpp_int< 1 > unicast_mgr_done_pass_cpp_int_t;
        cpp_int int_var__unicast_mgr_done_pass;
        void unicast_mgr_done_pass (const cpp_int  & l__val);
        cpp_int unicast_mgr_done_pass() const;
    
        typedef pu_cpp_int< 4 > oq_rwr_done_fail_cpp_int_t;
        cpp_int int_var__oq_rwr_done_fail;
        void oq_rwr_done_fail (const cpp_int  & l__val);
        cpp_int oq_rwr_done_fail() const;
    
        typedef pu_cpp_int< 4 > oq_rwr_done_pass_cpp_int_t;
        cpp_int int_var__oq_rwr_done_pass;
        void oq_rwr_done_pass (const cpp_int  & l__val);
        cpp_int oq_rwr_done_pass() const;
    
        typedef pu_cpp_int< 2 > desc_done_fail_cpp_int_t;
        cpp_int int_var__desc_done_fail;
        void desc_done_fail (const cpp_int  & l__val);
        cpp_int desc_done_fail() const;
    
        typedef pu_cpp_int< 2 > desc_done_pass_cpp_int_t;
        cpp_int int_var__desc_done_pass;
        void desc_done_pass (const cpp_int  & l__val);
        cpp_int desc_done_pass() const;
    
        typedef pu_cpp_int< 2 > sched_done_fail_cpp_int_t;
        cpp_int int_var__sched_done_fail;
        void sched_done_fail (const cpp_int  & l__val);
        cpp_int sched_done_fail() const;
    
        typedef pu_cpp_int< 2 > sched_done_pass_cpp_int_t;
        cpp_int int_var__sched_done_pass;
        void sched_done_pass (const cpp_int  & l__val);
        cpp_int sched_done_pass() const;
    
}; // cap_pbc_csr_sta_bist_t
    
class cap_pbc_csr_sta_account_xoff_timeout_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_sta_account_xoff_timeout_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_sta_account_xoff_timeout_t(string name = "cap_pbc_csr_sta_account_xoff_timeout_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_sta_account_xoff_timeout_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 3 > pg0_cpp_int_t;
        cpp_int int_var__pg0;
        void pg0 (const cpp_int  & l__val);
        cpp_int pg0() const;
    
        typedef pu_cpp_int< 3 > pg1_cpp_int_t;
        cpp_int int_var__pg1;
        void pg1 (const cpp_int  & l__val);
        cpp_int pg1() const;
    
        typedef pu_cpp_int< 3 > pg2_cpp_int_t;
        cpp_int int_var__pg2;
        void pg2 (const cpp_int  & l__val);
        cpp_int pg2() const;
    
        typedef pu_cpp_int< 3 > pg3_cpp_int_t;
        cpp_int int_var__pg3;
        void pg3 (const cpp_int  & l__val);
        cpp_int pg3() const;
    
        typedef pu_cpp_int< 3 > pg4_cpp_int_t;
        cpp_int int_var__pg4;
        void pg4 (const cpp_int  & l__val);
        cpp_int pg4() const;
    
        typedef pu_cpp_int< 3 > pg5_cpp_int_t;
        cpp_int int_var__pg5;
        void pg5 (const cpp_int  & l__val);
        cpp_int pg5() const;
    
        typedef pu_cpp_int< 3 > pg6_cpp_int_t;
        cpp_int int_var__pg6;
        void pg6 (const cpp_int  & l__val);
        cpp_int pg6() const;
    
        typedef pu_cpp_int< 3 > pg7_cpp_int_t;
        cpp_int int_var__pg7;
        void pg7 (const cpp_int  & l__val);
        cpp_int pg7() const;
    
        typedef pu_cpp_int< 3 > pg8_cpp_int_t;
        cpp_int int_var__pg8;
        void pg8 (const cpp_int  & l__val);
        cpp_int pg8() const;
    
}; // cap_pbc_csr_sta_account_xoff_timeout_t
    
class cap_pbc_csr_cfg_credits_max_growth_11_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_credits_max_growth_11_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_credits_max_growth_11_t(string name = "cap_pbc_csr_cfg_credits_max_growth_11_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_credits_max_growth_11_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 160 > cells_cpp_int_t;
        cpp_int int_var__cells;
        void cells (const cpp_int  & l__val);
        cpp_int cells() const;
    
}; // cap_pbc_csr_cfg_credits_max_growth_11_t
    
class cap_pbc_csr_cfg_credits_max_growth_10_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_credits_max_growth_10_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_credits_max_growth_10_t(string name = "cap_pbc_csr_cfg_credits_max_growth_10_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_credits_max_growth_10_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 160 > cells_cpp_int_t;
        cpp_int int_var__cells;
        void cells (const cpp_int  & l__val);
        cpp_int cells() const;
    
}; // cap_pbc_csr_cfg_credits_max_growth_10_t
    
class cap_pbc_csr_sta_ecc_uc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_sta_ecc_uc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_sta_ecc_uc_t(string name = "cap_pbc_csr_sta_ecc_uc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_sta_ecc_uc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 14 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pbc_csr_sta_ecc_uc_t
    
class cap_pbc_csr_sta_port_mon_out_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_sta_port_mon_out_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_sta_port_mon_out_t(string name = "cap_pbc_csr_sta_port_mon_out_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_sta_port_mon_out_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 7 > ecc_addr_cpp_int_t;
        cpp_int int_var__ecc_addr;
        void ecc_addr (const cpp_int  & l__val);
        cpp_int ecc_addr() const;
    
        typedef pu_cpp_int< 1 > ecc_correctable_cpp_int_t;
        cpp_int int_var__ecc_correctable;
        void ecc_correctable (const cpp_int  & l__val);
        cpp_int ecc_correctable() const;
    
        typedef pu_cpp_int< 1 > ecc_uncorrectable_cpp_int_t;
        cpp_int int_var__ecc_uncorrectable;
        void ecc_uncorrectable (const cpp_int  & l__val);
        cpp_int ecc_uncorrectable() const;
    
        typedef pu_cpp_int< 3 > ecc_syndrome_cpp_int_t;
        cpp_int int_var__ecc_syndrome;
        void ecc_syndrome (const cpp_int  & l__val);
        cpp_int ecc_syndrome() const;
    
        typedef pu_cpp_int< 1 > init_done_cpp_int_t;
        cpp_int int_var__init_done;
        void init_done (const cpp_int  & l__val);
        cpp_int init_done() const;
    
}; // cap_pbc_csr_sta_port_mon_out_t
    
class cap_pbc_csr_cfg_port_mon_out_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_port_mon_out_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_port_mon_out_t(string name = "cap_pbc_csr_cfg_port_mon_out_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_port_mon_out_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 4 > port_0_cpp_int_t;
        cpp_int int_var__port_0;
        void port_0 (const cpp_int  & l__val);
        cpp_int port_0() const;
    
        typedef pu_cpp_int< 4 > port_1_cpp_int_t;
        cpp_int int_var__port_1;
        void port_1 (const cpp_int  & l__val);
        cpp_int port_1() const;
    
        typedef pu_cpp_int< 4 > port_2_cpp_int_t;
        cpp_int int_var__port_2;
        void port_2 (const cpp_int  & l__val);
        cpp_int port_2() const;
    
        typedef pu_cpp_int< 4 > port_3_cpp_int_t;
        cpp_int int_var__port_3;
        void port_3 (const cpp_int  & l__val);
        cpp_int port_3() const;
    
        typedef pu_cpp_int< 1 > dhs_selection_cpp_int_t;
        cpp_int int_var__dhs_selection;
        void dhs_selection (const cpp_int  & l__val);
        cpp_int dhs_selection() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        cpp_int int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        cpp_int int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        cpp_int int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > init_start_cpp_int_t;
        cpp_int int_var__init_start;
        void init_start (const cpp_int  & l__val);
        cpp_int init_start() const;
    
}; // cap_pbc_csr_cfg_port_mon_out_t
    
class cap_pbc_csr_sta_port_mon_in_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_sta_port_mon_in_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_sta_port_mon_in_t(string name = "cap_pbc_csr_sta_port_mon_in_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_sta_port_mon_in_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 7 > ecc_addr_cpp_int_t;
        cpp_int int_var__ecc_addr;
        void ecc_addr (const cpp_int  & l__val);
        cpp_int ecc_addr() const;
    
        typedef pu_cpp_int< 1 > ecc_correctable_cpp_int_t;
        cpp_int int_var__ecc_correctable;
        void ecc_correctable (const cpp_int  & l__val);
        cpp_int ecc_correctable() const;
    
        typedef pu_cpp_int< 1 > ecc_uncorrectable_cpp_int_t;
        cpp_int int_var__ecc_uncorrectable;
        void ecc_uncorrectable (const cpp_int  & l__val);
        cpp_int ecc_uncorrectable() const;
    
        typedef pu_cpp_int< 3 > ecc_syndrome_cpp_int_t;
        cpp_int int_var__ecc_syndrome;
        void ecc_syndrome (const cpp_int  & l__val);
        cpp_int ecc_syndrome() const;
    
        typedef pu_cpp_int< 1 > init_done_cpp_int_t;
        cpp_int int_var__init_done;
        void init_done (const cpp_int  & l__val);
        cpp_int init_done() const;
    
}; // cap_pbc_csr_sta_port_mon_in_t
    
class cap_pbc_csr_cfg_port_mon_in_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_port_mon_in_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_port_mon_in_t(string name = "cap_pbc_csr_cfg_port_mon_in_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_port_mon_in_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 4 > eth_cpp_int_t;
        cpp_int int_var__eth;
        void eth (const cpp_int  & l__val);
        cpp_int eth() const;
    
        typedef pu_cpp_int< 1 > dhs_selection_cpp_int_t;
        cpp_int int_var__dhs_selection;
        void dhs_selection (const cpp_int  & l__val);
        cpp_int dhs_selection() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        cpp_int int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        cpp_int int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        cpp_int int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > init_start_cpp_int_t;
        cpp_int int_var__init_start;
        void init_start (const cpp_int  & l__val);
        cpp_int init_start() const;
    
}; // cap_pbc_csr_cfg_port_mon_in_t
    
class cap_pbc_csr_cfg_uc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_uc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_uc_t(string name = "cap_pbc_csr_cfg_uc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_uc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        cpp_int int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_pbc_csr_cfg_uc_t
    
class cap_pbc_csr_sta_ecc_nc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_sta_ecc_nc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_sta_ecc_nc_t(string name = "cap_pbc_csr_sta_ecc_nc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_sta_ecc_nc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 5 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 14 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pbc_csr_sta_ecc_nc_t
    
class cap_pbc_csr_cfg_nc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_nc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_nc_t(string name = "cap_pbc_csr_cfg_nc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_nc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        cpp_int int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_pbc_csr_cfg_nc_t
    
class cap_pbc_csr_cnt_flush_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cnt_flush_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cnt_flush_t(string name = "cap_pbc_csr_cnt_flush_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cnt_flush_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > packets_cpp_int_t;
        cpp_int int_var__packets;
        void packets (const cpp_int  & l__val);
        cpp_int packets() const;
    
}; // cap_pbc_csr_cnt_flush_t
    
class cap_pbc_csr_cfg_eg_ts_ctrl_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_eg_ts_ctrl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_eg_ts_ctrl_t(string name = "cap_pbc_csr_cfg_eg_ts_ctrl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_eg_ts_ctrl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 243 > base_cpp_int_t;
        cpp_int int_var__base;
        void base (const cpp_int  & l__val);
        cpp_int base() const;
    
}; // cap_pbc_csr_cfg_eg_ts_ctrl_t
    
class cap_pbc_csr_cfg_src_port_to_lif_map_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_src_port_to_lif_map_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_src_port_to_lif_map_t(string name = "cap_pbc_csr_cfg_src_port_to_lif_map_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_src_port_to_lif_map_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 11 > entry_0_cpp_int_t;
        cpp_int int_var__entry_0;
        void entry_0 (const cpp_int  & l__val);
        cpp_int entry_0() const;
    
        typedef pu_cpp_int< 11 > entry_1_cpp_int_t;
        cpp_int int_var__entry_1;
        void entry_1 (const cpp_int  & l__val);
        cpp_int entry_1() const;
    
        typedef pu_cpp_int< 11 > entry_2_cpp_int_t;
        cpp_int int_var__entry_2;
        void entry_2 (const cpp_int  & l__val);
        cpp_int entry_2() const;
    
        typedef pu_cpp_int< 11 > entry_3_cpp_int_t;
        cpp_int int_var__entry_3;
        void entry_3 (const cpp_int  & l__val);
        cpp_int entry_3() const;
    
        typedef pu_cpp_int< 11 > entry_4_cpp_int_t;
        cpp_int int_var__entry_4;
        void entry_4 (const cpp_int  & l__val);
        cpp_int entry_4() const;
    
        typedef pu_cpp_int< 11 > entry_5_cpp_int_t;
        cpp_int int_var__entry_5;
        void entry_5 (const cpp_int  & l__val);
        cpp_int entry_5() const;
    
        typedef pu_cpp_int< 11 > entry_6_cpp_int_t;
        cpp_int int_var__entry_6;
        void entry_6 (const cpp_int  & l__val);
        cpp_int entry_6() const;
    
        typedef pu_cpp_int< 11 > entry_7_cpp_int_t;
        cpp_int int_var__entry_7;
        void entry_7 (const cpp_int  & l__val);
        cpp_int entry_7() const;
    
        typedef pu_cpp_int< 11 > entry_8_cpp_int_t;
        cpp_int int_var__entry_8;
        void entry_8 (const cpp_int  & l__val);
        cpp_int entry_8() const;
    
}; // cap_pbc_csr_cfg_src_port_to_lif_map_t
    
class cap_pbc_csr_cfg_tail_drop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_tail_drop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_tail_drop_t(string name = "cap_pbc_csr_cfg_tail_drop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_tail_drop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > cpu_threshold_cpp_int_t;
        cpp_int int_var__cpu_threshold;
        void cpu_threshold (const cpp_int  & l__val);
        cpp_int cpu_threshold() const;
    
        typedef pu_cpp_int< 16 > span_threshold_cpp_int_t;
        cpp_int int_var__span_threshold;
        void span_threshold (const cpp_int  & l__val);
        cpp_int span_threshold() const;
    
}; // cap_pbc_csr_cfg_tail_drop_t
    
class cap_pbc_csr_sta_rpl_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_sta_rpl_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_sta_rpl_err_t(string name = "cap_pbc_csr_sta_rpl_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_sta_rpl_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 5 > rid_cpp_int_t;
        cpp_int int_var__rid;
        void rid (const cpp_int  & l__val);
        cpp_int rid() const;
    
        typedef pu_cpp_int< 2 > rresp_cpp_int_t;
        cpp_int int_var__rresp;
        void rresp (const cpp_int  & l__val);
        cpp_int rresp() const;
    
        typedef pu_cpp_int< 16 > replication_ptr_cpp_int_t;
        cpp_int int_var__replication_ptr;
        void replication_ptr (const cpp_int  & l__val);
        cpp_int replication_ptr() const;
    
}; // cap_pbc_csr_sta_rpl_err_t
    
class cap_pbc_csr_sta_ecc_sideband_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_sta_ecc_sideband_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_sta_ecc_sideband_t(string name = "cap_pbc_csr_sta_ecc_sideband_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_sta_ecc_sideband_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 14 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pbc_csr_sta_ecc_sideband_t
    
class cap_pbc_csr_cfg_spare_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_spare_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_spare_t(string name = "cap_pbc_csr_cfg_spare_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_spare_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_pbc_csr_cfg_spare_t
    
class cap_pbc_csr_cfg_debug_port_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_debug_port_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_debug_port_t(string name = "cap_pbc_csr_cfg_debug_port_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_debug_port_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 2 > select_cpp_int_t;
        cpp_int int_var__select;
        void select (const cpp_int  & l__val);
        cpp_int select() const;
    
        typedef pu_cpp_int< 16 > param_cpp_int_t;
        cpp_int int_var__param;
        void param (const cpp_int  & l__val);
        cpp_int param() const;
    
}; // cap_pbc_csr_cfg_debug_port_t
    
class cap_pbc_csr_cfg_dhs_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_dhs_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_dhs_mem_t(string name = "cap_pbc_csr_cfg_dhs_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_dhs_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 14 > address_cpp_int_t;
        cpp_int int_var__address;
        void address (const cpp_int  & l__val);
        cpp_int address() const;
    
}; // cap_pbc_csr_cfg_dhs_mem_t
    
class cap_pbc_csr_sta_oq_12_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_sta_oq_12_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_sta_oq_12_t(string name = "cap_pbc_csr_sta_oq_12_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_sta_oq_12_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > depth_value_cpp_int_t;
        cpp_int int_var__depth_value;
        void depth_value (const cpp_int  & l__val);
        cpp_int depth_value() const;
    
}; // cap_pbc_csr_sta_oq_12_t
    
class cap_pbc_csr_sta_oq_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_sta_oq_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_sta_oq_t(string name = "cap_pbc_csr_sta_oq_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_sta_oq_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 512 > depth_value_cpp_int_t;
        cpp_int int_var__depth_value;
        void depth_value (const cpp_int  & l__val);
        cpp_int depth_value() const;
    
}; // cap_pbc_csr_sta_oq_t
    
class cap_pbc_csr_sat_write_error_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_sat_write_error_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_sat_write_error_t(string name = "cap_pbc_csr_sat_write_error_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_sat_write_error_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > discarded_cpp_int_t;
        cpp_int int_var__discarded;
        void discarded (const cpp_int  & l__val);
        cpp_int discarded() const;
    
        typedef pu_cpp_int< 16 > admitted_cpp_int_t;
        cpp_int int_var__admitted;
        void admitted (const cpp_int  & l__val);
        cpp_int admitted() const;
    
        typedef pu_cpp_int< 16 > intrinsic_drop_cpp_int_t;
        cpp_int int_var__intrinsic_drop;
        void intrinsic_drop (const cpp_int  & l__val);
        cpp_int intrinsic_drop() const;
    
        typedef pu_cpp_int< 8 > out_of_cells_cpp_int_t;
        cpp_int int_var__out_of_cells;
        void out_of_cells (const cpp_int  & l__val);
        cpp_int out_of_cells() const;
    
        typedef pu_cpp_int< 8 > out_of_credit_cpp_int_t;
        cpp_int int_var__out_of_credit;
        void out_of_credit (const cpp_int  & l__val);
        cpp_int out_of_credit() const;
    
        typedef pu_cpp_int< 8 > truncation_cpp_int_t;
        cpp_int int_var__truncation;
        void truncation (const cpp_int  & l__val);
        cpp_int truncation() const;
    
        typedef pu_cpp_int< 8 > port_disabled_cpp_int_t;
        cpp_int int_var__port_disabled;
        void port_disabled (const cpp_int  & l__val);
        cpp_int port_disabled() const;
    
        typedef pu_cpp_int< 16 > out_of_cells1_cpp_int_t;
        cpp_int int_var__out_of_cells1;
        void out_of_cells1 (const cpp_int  & l__val);
        cpp_int out_of_cells1() const;
    
        typedef pu_cpp_int< 16 > tail_drop_cpu_cpp_int_t;
        cpp_int int_var__tail_drop_cpu;
        void tail_drop_cpu (const cpp_int  & l__val);
        cpp_int tail_drop_cpu() const;
    
        typedef pu_cpp_int< 16 > tail_drop_span_cpp_int_t;
        cpp_int int_var__tail_drop_span;
        void tail_drop_span (const cpp_int  & l__val);
        cpp_int tail_drop_span() const;
    
        typedef pu_cpp_int< 8 > min_size_viol_cpp_int_t;
        cpp_int int_var__min_size_viol;
        void min_size_viol (const cpp_int  & l__val);
        cpp_int min_size_viol() const;
    
        typedef pu_cpp_int< 8 > enqueue_cpp_int_t;
        cpp_int int_var__enqueue;
        void enqueue (const cpp_int  & l__val);
        cpp_int enqueue() const;
    
        typedef pu_cpp_int< 8 > port_range_cpp_int_t;
        cpp_int int_var__port_range;
        void port_range (const cpp_int  & l__val);
        cpp_int port_range() const;
    
        typedef pu_cpp_int< 8 > oq_range_cpp_int_t;
        cpp_int int_var__oq_range;
        void oq_range (const cpp_int  & l__val);
        cpp_int oq_range() const;
    
}; // cap_pbc_csr_sat_write_error_t
    
class cap_pbc_csr_cnt_flits_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cnt_flits_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cnt_flits_t(string name = "cap_pbc_csr_cnt_flits_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cnt_flits_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > sop_in_cpp_int_t;
        cpp_int int_var__sop_in;
        void sop_in (const cpp_int  & l__val);
        cpp_int sop_in() const;
    
        typedef pu_cpp_int< 16 > eop_in_cpp_int_t;
        cpp_int int_var__eop_in;
        void eop_in (const cpp_int  & l__val);
        cpp_int eop_in() const;
    
        typedef pu_cpp_int< 16 > sop_out_cpp_int_t;
        cpp_int int_var__sop_out;
        void sop_out (const cpp_int  & l__val);
        cpp_int sop_out() const;
    
        typedef pu_cpp_int< 16 > eop_out_cpp_int_t;
        cpp_int int_var__eop_out;
        void eop_out (const cpp_int  & l__val);
        cpp_int eop_out() const;
    
}; // cap_pbc_csr_cnt_flits_t
    
class cap_pbc_csr_cfg_parser8_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_parser8_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_parser8_t(string name = "cap_pbc_csr_cfg_parser8_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_parser8_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 4 > default_port_cpp_int_t;
        cpp_int int_var__default_port;
        void default_port (const cpp_int  & l__val);
        cpp_int default_port() const;
    
        typedef pu_cpp_int< 3 > default_cos_cpp_int_t;
        cpp_int int_var__default_cos;
        void default_cos (const cpp_int  & l__val);
        cpp_int default_cos() const;
    
        typedef pu_cpp_int< 40 > oq_map_cpp_int_t;
        cpp_int int_var__oq_map;
        void oq_map (const cpp_int  & l__val);
        cpp_int oq_map() const;
    
}; // cap_pbc_csr_cfg_parser8_t
    
class cap_pbc_csr_cfg_parser7_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_parser7_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_parser7_t(string name = "cap_pbc_csr_cfg_parser7_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_parser7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 4 > default_port_cpp_int_t;
        cpp_int int_var__default_port;
        void default_port (const cpp_int  & l__val);
        cpp_int default_port() const;
    
        typedef pu_cpp_int< 3 > default_cos_cpp_int_t;
        cpp_int int_var__default_cos;
        void default_cos (const cpp_int  & l__val);
        cpp_int default_cos() const;
    
        typedef pu_cpp_int< 40 > oq_map_cpp_int_t;
        cpp_int int_var__oq_map;
        void oq_map (const cpp_int  & l__val);
        cpp_int oq_map() const;
    
}; // cap_pbc_csr_cfg_parser7_t
    
class cap_pbc_csr_cfg_parser6_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_parser6_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_parser6_t(string name = "cap_pbc_csr_cfg_parser6_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_parser6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 4 > default_port_cpp_int_t;
        cpp_int int_var__default_port;
        void default_port (const cpp_int  & l__val);
        cpp_int default_port() const;
    
        typedef pu_cpp_int< 3 > default_cos_cpp_int_t;
        cpp_int int_var__default_cos;
        void default_cos (const cpp_int  & l__val);
        cpp_int default_cos() const;
    
        typedef pu_cpp_int< 40 > oq_map_cpp_int_t;
        cpp_int int_var__oq_map;
        void oq_map (const cpp_int  & l__val);
        cpp_int oq_map() const;
    
}; // cap_pbc_csr_cfg_parser6_t
    
class cap_pbc_csr_cfg_parser5_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_parser5_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_parser5_t(string name = "cap_pbc_csr_cfg_parser5_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_parser5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 4 > default_port_cpp_int_t;
        cpp_int int_var__default_port;
        void default_port (const cpp_int  & l__val);
        cpp_int default_port() const;
    
        typedef pu_cpp_int< 3 > default_cos_cpp_int_t;
        cpp_int int_var__default_cos;
        void default_cos (const cpp_int  & l__val);
        cpp_int default_cos() const;
    
        typedef pu_cpp_int< 40 > oq_map_cpp_int_t;
        cpp_int int_var__oq_map;
        void oq_map (const cpp_int  & l__val);
        cpp_int oq_map() const;
    
}; // cap_pbc_csr_cfg_parser5_t
    
class cap_pbc_csr_cfg_parser4_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_parser4_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_parser4_t(string name = "cap_pbc_csr_cfg_parser4_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_parser4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 4 > default_port_cpp_int_t;
        cpp_int int_var__default_port;
        void default_port (const cpp_int  & l__val);
        cpp_int default_port() const;
    
        typedef pu_cpp_int< 3 > default_cos_cpp_int_t;
        cpp_int int_var__default_cos;
        void default_cos (const cpp_int  & l__val);
        cpp_int default_cos() const;
    
        typedef pu_cpp_int< 40 > oq_map_cpp_int_t;
        cpp_int int_var__oq_map;
        void oq_map (const cpp_int  & l__val);
        cpp_int oq_map() const;
    
}; // cap_pbc_csr_cfg_parser4_t
    
class cap_pbc_csr_cfg_parser3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_parser3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_parser3_t(string name = "cap_pbc_csr_cfg_parser3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_parser3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 4 > default_port_cpp_int_t;
        cpp_int int_var__default_port;
        void default_port (const cpp_int  & l__val);
        cpp_int default_port() const;
    
        typedef pu_cpp_int< 3 > default_cos_cpp_int_t;
        cpp_int int_var__default_cos;
        void default_cos (const cpp_int  & l__val);
        cpp_int default_cos() const;
    
        typedef pu_cpp_int< 40 > oq_map_cpp_int_t;
        cpp_int int_var__oq_map;
        void oq_map (const cpp_int  & l__val);
        cpp_int oq_map() const;
    
}; // cap_pbc_csr_cfg_parser3_t
    
class cap_pbc_csr_cfg_parser2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_parser2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_parser2_t(string name = "cap_pbc_csr_cfg_parser2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_parser2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 4 > default_port_cpp_int_t;
        cpp_int int_var__default_port;
        void default_port (const cpp_int  & l__val);
        cpp_int default_port() const;
    
        typedef pu_cpp_int< 3 > default_cos_cpp_int_t;
        cpp_int int_var__default_cos;
        void default_cos (const cpp_int  & l__val);
        cpp_int default_cos() const;
    
        typedef pu_cpp_int< 40 > oq_map_cpp_int_t;
        cpp_int int_var__oq_map;
        void oq_map (const cpp_int  & l__val);
        cpp_int oq_map() const;
    
}; // cap_pbc_csr_cfg_parser2_t
    
class cap_pbc_csr_cfg_parser1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_parser1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_parser1_t(string name = "cap_pbc_csr_cfg_parser1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_parser1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 4 > default_port_cpp_int_t;
        cpp_int int_var__default_port;
        void default_port (const cpp_int  & l__val);
        cpp_int default_port() const;
    
        typedef pu_cpp_int< 3 > default_cos_cpp_int_t;
        cpp_int int_var__default_cos;
        void default_cos (const cpp_int  & l__val);
        cpp_int default_cos() const;
    
        typedef pu_cpp_int< 40 > oq_map_cpp_int_t;
        cpp_int int_var__oq_map;
        void oq_map (const cpp_int  & l__val);
        cpp_int oq_map() const;
    
}; // cap_pbc_csr_cfg_parser1_t
    
class cap_pbc_csr_cfg_parser0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_parser0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_parser0_t(string name = "cap_pbc_csr_cfg_parser0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_parser0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 4 > default_port_cpp_int_t;
        cpp_int int_var__default_port;
        void default_port (const cpp_int  & l__val);
        cpp_int default_port() const;
    
        typedef pu_cpp_int< 3 > default_cos_cpp_int_t;
        cpp_int int_var__default_cos;
        void default_cos (const cpp_int  & l__val);
        cpp_int default_cos() const;
    
        typedef pu_cpp_int< 40 > oq_map_cpp_int_t;
        cpp_int int_var__oq_map;
        void oq_map (const cpp_int  & l__val);
        cpp_int oq_map() const;
    
}; // cap_pbc_csr_cfg_parser0_t
    
class cap_pbc_csr_cfg_rpl_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_rpl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_rpl_t(string name = "cap_pbc_csr_cfg_rpl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_rpl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 27 > base_cpp_int_t;
        cpp_int int_var__base;
        void base (const cpp_int  & l__val);
        cpp_int base() const;
    
        typedef pu_cpp_int< 2 > token_size_cpp_int_t;
        cpp_int int_var__token_size;
        void token_size (const cpp_int  & l__val);
        cpp_int token_size() const;
    
}; // cap_pbc_csr_cfg_rpl_t
    
class cap_pbc_csr_cfg_island_control_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_island_control_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_island_control_t(string name = "cap_pbc_csr_cfg_island_control_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_island_control_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > map_cpp_int_t;
        cpp_int int_var__map;
        void map (const cpp_int  & l__val);
        cpp_int map() const;
    
}; // cap_pbc_csr_cfg_island_control_t
    
class cap_pbc_csr_sta_sched_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_sta_sched_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_sta_sched_t(string name = "cap_pbc_csr_sta_sched_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_sta_sched_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > ecc_uncorrectable_cpp_int_t;
        cpp_int int_var__ecc_uncorrectable;
        void ecc_uncorrectable (const cpp_int  & l__val);
        cpp_int ecc_uncorrectable() const;
    
        typedef pu_cpp_int< 1 > ecc_correctable_cpp_int_t;
        cpp_int int_var__ecc_correctable;
        void ecc_correctable (const cpp_int  & l__val);
        cpp_int ecc_correctable() const;
    
        typedef pu_cpp_int< 8 > ecc_syndrome_cpp_int_t;
        cpp_int int_var__ecc_syndrome;
        void ecc_syndrome (const cpp_int  & l__val);
        cpp_int ecc_syndrome() const;
    
        typedef pu_cpp_int< 9 > ecc_addr_cpp_int_t;
        cpp_int int_var__ecc_addr;
        void ecc_addr (const cpp_int  & l__val);
        cpp_int ecc_addr() const;
    
}; // cap_pbc_csr_sta_sched_t
    
class cap_pbc_csr_cfg_sched_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_sched_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_sched_t(string name = "cap_pbc_csr_cfg_sched_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_sched_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > enable_wrr_cpp_int_t;
        cpp_int int_var__enable_wrr;
        void enable_wrr (const cpp_int  & l__val);
        cpp_int enable_wrr() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        cpp_int int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        cpp_int int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        cpp_int int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 3 > dhs_selection_cpp_int_t;
        cpp_int int_var__dhs_selection;
        void dhs_selection (const cpp_int  & l__val);
        cpp_int dhs_selection() const;
    
        typedef pu_cpp_int< 32 > timer_cpp_int_t;
        cpp_int int_var__timer;
        void timer (const cpp_int  & l__val);
        cpp_int timer() const;
    
        typedef pu_cpp_int< 2 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_pbc_csr_cfg_sched_t
    
class cap_pbc_csr_sta_rc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_sta_rc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_sta_rc_t(string name = "cap_pbc_csr_sta_rc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_sta_rc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > init_done_cpp_int_t;
        cpp_int int_var__init_done;
        void init_done (const cpp_int  & l__val);
        cpp_int init_done() const;
    
        typedef pu_cpp_int< 1 > ecc_uncorrectable_cpp_int_t;
        cpp_int int_var__ecc_uncorrectable;
        void ecc_uncorrectable (const cpp_int  & l__val);
        cpp_int ecc_uncorrectable() const;
    
        typedef pu_cpp_int< 1 > ecc_correctable_cpp_int_t;
        cpp_int int_var__ecc_correctable;
        void ecc_correctable (const cpp_int  & l__val);
        cpp_int ecc_correctable() const;
    
        typedef pu_cpp_int< 6 > ecc_syndrome_cpp_int_t;
        cpp_int int_var__ecc_syndrome;
        void ecc_syndrome (const cpp_int  & l__val);
        cpp_int ecc_syndrome() const;
    
        typedef pu_cpp_int< 11 > ecc_addr_cpp_int_t;
        cpp_int int_var__ecc_addr;
        void ecc_addr (const cpp_int  & l__val);
        cpp_int ecc_addr() const;
    
}; // cap_pbc_csr_sta_rc_t
    
class cap_pbc_csr_cfg_rc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_rc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_rc_t(string name = "cap_pbc_csr_cfg_rc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_rc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > init_start_cpp_int_t;
        cpp_int int_var__init_start;
        void init_start (const cpp_int  & l__val);
        cpp_int init_start() const;
    
        typedef pu_cpp_int< 1 > init_reset_cpp_int_t;
        cpp_int int_var__init_reset;
        void init_reset (const cpp_int  & l__val);
        cpp_int init_reset() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        cpp_int int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        cpp_int int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        cpp_int int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_pbc_csr_cfg_rc_t
    
class cap_pbc_csr_sta_ecc_desc_1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_sta_ecc_desc_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_sta_ecc_desc_1_t(string name = "cap_pbc_csr_sta_ecc_desc_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_sta_ecc_desc_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 14 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pbc_csr_sta_ecc_desc_1_t
    
class cap_pbc_csr_cfg_desc_1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_desc_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_desc_1_t(string name = "cap_pbc_csr_cfg_desc_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_desc_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        cpp_int int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_pbc_csr_cfg_desc_1_t
    
class cap_pbc_csr_sta_ecc_ll_1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_sta_ecc_ll_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_sta_ecc_ll_1_t(string name = "cap_pbc_csr_sta_ecc_ll_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_sta_ecc_ll_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 14 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pbc_csr_sta_ecc_ll_1_t
    
class cap_pbc_csr_cfg_ll_1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_ll_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_ll_1_t(string name = "cap_pbc_csr_cfg_ll_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_ll_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        cpp_int int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_pbc_csr_cfg_ll_1_t
    
class cap_pbc_csr_sta_ecc_fc_1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_sta_ecc_fc_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_sta_ecc_fc_1_t(string name = "cap_pbc_csr_sta_ecc_fc_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_sta_ecc_fc_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 12 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pbc_csr_sta_ecc_fc_1_t
    
class cap_pbc_csr_cfg_fc_mgr_1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_fc_mgr_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_fc_mgr_1_t(string name = "cap_pbc_csr_cfg_fc_mgr_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_fc_mgr_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > init_start_cpp_int_t;
        cpp_int int_var__init_start;
        void init_start (const cpp_int  & l__val);
        cpp_int init_start() const;
    
        typedef pu_cpp_int< 1 > init_reset_cpp_int_t;
        cpp_int int_var__init_reset;
        void init_reset (const cpp_int  & l__val);
        cpp_int init_reset() const;
    
        typedef pu_cpp_int< 12 > max_row_cpp_int_t;
        cpp_int int_var__max_row;
        void max_row (const cpp_int  & l__val);
        cpp_int max_row() const;
    
        typedef pu_cpp_int< 13 > min_cell_cpp_int_t;
        cpp_int int_var__min_cell;
        void min_cell (const cpp_int  & l__val);
        cpp_int min_cell() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        cpp_int int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        cpp_int int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        cpp_int int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_pbc_csr_cfg_fc_mgr_1_t
    
class cap_pbc_csr_sta_fc_mgr_1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_sta_fc_mgr_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_sta_fc_mgr_1_t(string name = "cap_pbc_csr_sta_fc_mgr_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_sta_fc_mgr_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 13 > cells_available_cpp_int_t;
        cpp_int int_var__cells_available;
        void cells_available (const cpp_int  & l__val);
        cpp_int cells_available() const;
    
        typedef pu_cpp_int< 1 > init_done_cpp_int_t;
        cpp_int int_var__init_done;
        void init_done (const cpp_int  & l__val);
        cpp_int init_done() const;
    
}; // cap_pbc_csr_sta_fc_mgr_1_t
    
class cap_pbc_csr_sta_ecc_desc_0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_sta_ecc_desc_0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_sta_ecc_desc_0_t(string name = "cap_pbc_csr_sta_ecc_desc_0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_sta_ecc_desc_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 14 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pbc_csr_sta_ecc_desc_0_t
    
class cap_pbc_csr_cfg_desc_0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_desc_0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_desc_0_t(string name = "cap_pbc_csr_cfg_desc_0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_desc_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        cpp_int int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_pbc_csr_cfg_desc_0_t
    
class cap_pbc_csr_sta_ecc_ll_0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_sta_ecc_ll_0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_sta_ecc_ll_0_t(string name = "cap_pbc_csr_sta_ecc_ll_0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_sta_ecc_ll_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 14 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pbc_csr_sta_ecc_ll_0_t
    
class cap_pbc_csr_cfg_ll_0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_ll_0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_ll_0_t(string name = "cap_pbc_csr_cfg_ll_0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_ll_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        cpp_int int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_pbc_csr_cfg_ll_0_t
    
class cap_pbc_csr_sta_ecc_fc_0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_sta_ecc_fc_0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_sta_ecc_fc_0_t(string name = "cap_pbc_csr_sta_ecc_fc_0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_sta_ecc_fc_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 12 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pbc_csr_sta_ecc_fc_0_t
    
class cap_pbc_csr_cfg_fc_mgr_0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_fc_mgr_0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_fc_mgr_0_t(string name = "cap_pbc_csr_cfg_fc_mgr_0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_fc_mgr_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > init_start_cpp_int_t;
        cpp_int int_var__init_start;
        void init_start (const cpp_int  & l__val);
        cpp_int init_start() const;
    
        typedef pu_cpp_int< 1 > init_reset_cpp_int_t;
        cpp_int int_var__init_reset;
        void init_reset (const cpp_int  & l__val);
        cpp_int init_reset() const;
    
        typedef pu_cpp_int< 12 > max_row_cpp_int_t;
        cpp_int int_var__max_row;
        void max_row (const cpp_int  & l__val);
        cpp_int max_row() const;
    
        typedef pu_cpp_int< 13 > min_cell_cpp_int_t;
        cpp_int int_var__min_cell;
        void min_cell (const cpp_int  & l__val);
        cpp_int min_cell() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        cpp_int int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        cpp_int int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        cpp_int int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_pbc_csr_cfg_fc_mgr_0_t
    
class cap_pbc_csr_sta_fc_mgr_0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_sta_fc_mgr_0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_sta_fc_mgr_0_t(string name = "cap_pbc_csr_sta_fc_mgr_0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_sta_fc_mgr_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 13 > cells_available_cpp_int_t;
        cpp_int int_var__cells_available;
        void cells_available (const cpp_int  & l__val);
        cpp_int cells_available() const;
    
        typedef pu_cpp_int< 1 > init_done_cpp_int_t;
        cpp_int int_var__init_done;
        void init_done (const cpp_int  & l__val);
        cpp_int init_done() const;
    
}; // cap_pbc_csr_sta_fc_mgr_0_t
    
class cap_pbc_csr_sta_ecc_rwr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_sta_ecc_rwr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_sta_ecc_rwr_t(string name = "cap_pbc_csr_sta_ecc_rwr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_sta_ecc_rwr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 18 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 11 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pbc_csr_sta_ecc_rwr_t
    
class cap_pbc_csr_sta_ecc_pack_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_sta_ecc_pack_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_sta_ecc_pack_t(string name = "cap_pbc_csr_sta_ecc_pack_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_sta_ecc_pack_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 16 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pbc_csr_sta_ecc_pack_t
    
class cap_pbc_csr_csr_intr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_csr_intr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_csr_intr_t(string name = "cap_pbc_csr_csr_intr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_csr_intr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_pbc_csr_csr_intr_t
    
class cap_pbc_csr_cfg_pbc_control_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_cfg_pbc_control_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_cfg_pbc_control_t(string name = "cap_pbc_csr_cfg_pbc_control_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_cfg_pbc_control_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > sw_reset_cpp_int_t;
        cpp_int int_var__sw_reset;
        void sw_reset (const cpp_int  & l__val);
        cpp_int sw_reset() const;
    
}; // cap_pbc_csr_cfg_pbc_control_t
    
class cap_pbc_csr_base_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_base_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_base_t(string name = "cap_pbc_csr_base_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_base_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_pbc_csr_base_t
    
class cap_pbc_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbc_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbc_csr_t(string name = "cap_pbc_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbc_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbc_csr_base_t base;
    
        cap_pbc_csr_cfg_pbc_control_t cfg_pbc_control;
    
        cap_pbc_csr_csr_intr_t csr_intr;
    
        cap_pbc_csr_sta_ecc_pack_t sta_ecc_pack;
    
        cap_pbc_csr_sta_ecc_rwr_t sta_ecc_rwr;
    
        cap_pbc_csr_sta_fc_mgr_0_t sta_fc_mgr_0;
    
        cap_pbc_csr_cfg_fc_mgr_0_t cfg_fc_mgr_0;
    
        cap_pbc_csr_sta_ecc_fc_0_t sta_ecc_fc_0;
    
        cap_pbc_csr_cfg_ll_0_t cfg_ll_0;
    
        cap_pbc_csr_sta_ecc_ll_0_t sta_ecc_ll_0;
    
        cap_pbc_csr_cfg_desc_0_t cfg_desc_0;
    
        cap_pbc_csr_sta_ecc_desc_0_t sta_ecc_desc_0;
    
        cap_pbc_csr_sta_fc_mgr_1_t sta_fc_mgr_1;
    
        cap_pbc_csr_cfg_fc_mgr_1_t cfg_fc_mgr_1;
    
        cap_pbc_csr_sta_ecc_fc_1_t sta_ecc_fc_1;
    
        cap_pbc_csr_cfg_ll_1_t cfg_ll_1;
    
        cap_pbc_csr_sta_ecc_ll_1_t sta_ecc_ll_1;
    
        cap_pbc_csr_cfg_desc_1_t cfg_desc_1;
    
        cap_pbc_csr_sta_ecc_desc_1_t sta_ecc_desc_1;
    
        cap_pbc_csr_cfg_rc_t cfg_rc;
    
        cap_pbc_csr_sta_rc_t sta_rc;
    
        cap_pbc_csr_cfg_sched_t cfg_sched;
    
        cap_pbc_csr_sta_sched_t sta_sched;
    
        cap_pbc_csr_cfg_island_control_t cfg_island_control;
    
        cap_pbc_csr_cfg_rpl_t cfg_rpl;
    
        cap_pbc_csr_cfg_parser0_t cfg_parser0;
    
        cap_pbc_csr_cfg_parser1_t cfg_parser1;
    
        cap_pbc_csr_cfg_parser2_t cfg_parser2;
    
        cap_pbc_csr_cfg_parser3_t cfg_parser3;
    
        cap_pbc_csr_cfg_parser4_t cfg_parser4;
    
        cap_pbc_csr_cfg_parser5_t cfg_parser5;
    
        cap_pbc_csr_cfg_parser6_t cfg_parser6;
    
        cap_pbc_csr_cfg_parser7_t cfg_parser7;
    
        cap_pbc_csr_cfg_parser8_t cfg_parser8;
    
        #if 12 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pbc_csr_cnt_flits_t, 12> cnt_flits;
        #else 
        cap_pbc_csr_cnt_flits_t cnt_flits[12];
        #endif
        int get_depth_cnt_flits() { return 12; }
    
        #if 12 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pbc_csr_sat_write_error_t, 12> sat_write_error;
        #else 
        cap_pbc_csr_sat_write_error_t sat_write_error[12];
        #endif
        int get_depth_sat_write_error() { return 12; }
    
        #if 12 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pbc_csr_sta_oq_t, 12> sta_oq;
        #else 
        cap_pbc_csr_sta_oq_t sta_oq[12];
        #endif
        int get_depth_sta_oq() { return 12; }
    
        cap_pbc_csr_sta_oq_12_t sta_oq_12;
    
        cap_pbc_csr_cfg_dhs_mem_t cfg_dhs_mem;
    
        cap_pbc_csr_cfg_debug_port_t cfg_debug_port;
    
        cap_pbc_csr_cfg_spare_t cfg_spare;
    
        cap_pbc_csr_sta_ecc_sideband_t sta_ecc_sideband;
    
        cap_pbc_csr_sta_rpl_err_t sta_rpl_err;
    
        cap_pbc_csr_cfg_tail_drop_t cfg_tail_drop;
    
        cap_pbc_csr_cfg_src_port_to_lif_map_t cfg_src_port_to_lif_map;
    
        cap_pbc_csr_cfg_eg_ts_ctrl_t cfg_eg_ts_ctrl;
    
        #if 12 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pbc_csr_cnt_flush_t, 12> cnt_flush;
        #else 
        cap_pbc_csr_cnt_flush_t cnt_flush[12];
        #endif
        int get_depth_cnt_flush() { return 12; }
    
        cap_pbc_csr_cfg_nc_t cfg_nc;
    
        cap_pbc_csr_sta_ecc_nc_t sta_ecc_nc;
    
        cap_pbc_csr_cfg_uc_t cfg_uc;
    
        cap_pbc_csr_cfg_port_mon_in_t cfg_port_mon_in;
    
        cap_pbc_csr_sta_port_mon_in_t sta_port_mon_in;
    
        cap_pbc_csr_cfg_port_mon_out_t cfg_port_mon_out;
    
        cap_pbc_csr_sta_port_mon_out_t sta_port_mon_out;
    
        cap_pbc_csr_sta_ecc_uc_t sta_ecc_uc;
    
        cap_pbc_csr_cfg_credits_max_growth_10_t cfg_credits_max_growth_10;
    
        cap_pbc_csr_cfg_credits_max_growth_11_t cfg_credits_max_growth_11;
    
        cap_pbc_csr_sta_account_xoff_timeout_t sta_account_xoff_timeout;
    
        cap_pbc_csr_sta_bist_t sta_bist;
    
        cap_pbc_csr_cfg_axi_t cfg_axi;
    
        cap_pbc_csr_sta_ecc_rwr_sideband_t sta_ecc_rwr_sideband;
    
        cap_pbc_csr_sta_ecc_rxd_sideband_t sta_ecc_rxd_sideband;
    
        cap_pbc_csr_dhs_fc_0_t dhs_fc_0;
    
        cap_pbc_csr_dhs_ll_0_t dhs_ll_0;
    
        cap_pbc_csr_dhs_desc_0_t dhs_desc_0;
    
        cap_pbc_csr_dhs_fc_1_t dhs_fc_1;
    
        cap_pbc_csr_dhs_ll_1_t dhs_ll_1;
    
        cap_pbc_csr_dhs_desc_1_t dhs_desc_1;
    
        cap_pbc_csr_dhs_rc_t dhs_rc;
    
        cap_pbc_csr_dhs_sched_t dhs_sched;
    
        cap_pbc_csr_dhs_sp0_t dhs_sp0;
    
        cap_pbc_csr_dhs_sp1_t dhs_sp1;
    
        cap_pbc_csr_dhs_nc_t dhs_nc;
    
        cap_pbc_csr_dhs_uc_t dhs_uc;
    
        cap_pbc_csr_dhs_port_mon_in_t dhs_port_mon_in;
    
        cap_pbc_csr_dhs_port_mon_out_t dhs_port_mon_out;
    
        cap_pbc_csr_dhs_fc_backdoor_0_t dhs_fc_backdoor_0;
    
        cap_pbc_csr_dhs_fc_backdoor_1_t dhs_fc_backdoor_1;
    
        cap_pbcport0_csr_t port_0;
    
        cap_pbcport1_csr_t port_1;
    
        cap_pbcport2_csr_t port_2;
    
        cap_pbcport3_csr_t port_3;
    
        cap_pbcport4_csr_t port_4;
    
        cap_pbcport5_csr_t port_5;
    
        cap_pbcport6_csr_t port_6;
    
        cap_pbcport7_csr_t port_7;
    
        cap_pbcport8_csr_t port_8;
    
        cap_pbcport9_csr_t port_9;
    
        cap_pbcport10_csr_t port_10;
    
        cap_pbcport11_csr_t port_11;
    
        cap_pbchbm_csr_t hbm;
    
        cap_pbc_csr_intgrp_status_t int_groups;
    
        cap_pbc_csr_int_write_0_t int_write_0;
    
        cap_pbc_csr_int_write_0_t int_write_1;
    
        cap_pbc_csr_int_write_0_t int_write_2;
    
        cap_pbc_csr_int_write_0_t int_write_3;
    
        cap_pbc_csr_int_write_0_t int_write_4;
    
        cap_pbc_csr_int_write_0_t int_write_5;
    
        cap_pbc_csr_int_write_0_t int_write_6;
    
        cap_pbc_csr_int_write_0_t int_write_7;
    
        cap_pbc_csr_int_write_0_t int_write_8;
    
        cap_pbc_csr_int_write_0_t int_write_9;
    
        cap_pbc_csr_int_write_0_t int_write_10;
    
        cap_pbc_csr_int_write_0_t int_write_11;
    
        cap_pbc_csr_int_ecc_port_mon_out_t int_ecc_pack;
    
        cap_pbc_csr_int_ecc_port_mon_out_t int_ecc_rwr;
    
        cap_pbc_csr_int_ecc_port_mon_out_t int_ecc_fc_0;
    
        cap_pbc_csr_int_ecc_port_mon_out_t int_ecc_ll_0;
    
        cap_pbc_csr_int_ecc_port_mon_out_t int_ecc_desc_0;
    
        cap_pbc_csr_int_ecc_port_mon_out_t int_ecc_fc_1;
    
        cap_pbc_csr_int_ecc_port_mon_out_t int_ecc_ll_1;
    
        cap_pbc_csr_int_ecc_port_mon_out_t int_ecc_desc_1;
    
        cap_pbc_csr_int_ecc_port_mon_out_t int_ecc_rc;
    
        cap_pbc_csr_int_ecc_port_mon_out_t int_ecc_sched;
    
        cap_pbc_csr_int_pbus_violation_in_t int_pbus_violation_in;
    
        cap_pbc_csr_int_pbus_violation_out_t int_pbus_violation_out;
    
        cap_pbc_csr_int_ecc_sideband_t int_ecc_sideband;
    
        cap_pbc_csr_int_rpl_t int_rpl;
    
        cap_pbc_csr_int_ecc_port_mon_out_t int_ecc_nc;
    
        cap_pbc_csr_int_ecc_port_mon_out_t int_ecc_uc;
    
        cap_pbc_csr_int_ecc_port_mon_out_t int_ecc_port_mon_in;
    
        cap_pbc_csr_int_ecc_port_mon_out_t int_ecc_port_mon_out;
    
        cap_pbc_csr_int_credit_underflow_t int_credit_underflow;
    
}; // cap_pbc_csr_t
    
#endif // CAP_PBC_CSR_H
        