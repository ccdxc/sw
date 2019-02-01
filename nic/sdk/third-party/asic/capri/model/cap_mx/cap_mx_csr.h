
#ifndef CAP_MX_CSR_H
#define CAP_MX_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_mx_csr_int_ecc_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_int_ecc_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_int_ecc_int_enable_clear_t(string name = "cap_mx_csr_int_ecc_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_int_ecc_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_mx_csr_int_ecc_int_enable_clear_t
    
class cap_mx_csr_int_ecc_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_int_ecc_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_int_ecc_int_test_set_t(string name = "cap_mx_csr_int_ecc_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_int_ecc_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_mx_csr_int_ecc_int_test_set_t
    
class cap_mx_csr_int_ecc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_int_ecc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_int_ecc_t(string name = "cap_mx_csr_int_ecc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_int_ecc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_mx_csr_int_ecc_int_test_set_t intreg;
    
        cap_mx_csr_int_ecc_int_test_set_t int_test_set;
    
        cap_mx_csr_int_ecc_int_enable_clear_t int_enable_set;
    
        cap_mx_csr_int_ecc_int_enable_clear_t int_enable_clear;
    
}; // cap_mx_csr_int_ecc_t
    
class cap_mx_csr_int_mac_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_int_mac_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_int_mac_int_enable_clear_t(string name = "cap_mx_csr_int_mac_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_int_mac_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > lane0_sbe_enable_cpp_int_t;
        cpp_int int_var__lane0_sbe_enable;
        void lane0_sbe_enable (const cpp_int  & l__val);
        cpp_int lane0_sbe_enable() const;
    
        typedef pu_cpp_int< 1 > lane0_dbe_enable_cpp_int_t;
        cpp_int int_var__lane0_dbe_enable;
        void lane0_dbe_enable (const cpp_int  & l__val);
        cpp_int lane0_dbe_enable() const;
    
        typedef pu_cpp_int< 1 > lane1_sbe_enable_cpp_int_t;
        cpp_int int_var__lane1_sbe_enable;
        void lane1_sbe_enable (const cpp_int  & l__val);
        cpp_int lane1_sbe_enable() const;
    
        typedef pu_cpp_int< 1 > lane1_dbe_enable_cpp_int_t;
        cpp_int int_var__lane1_dbe_enable;
        void lane1_dbe_enable (const cpp_int  & l__val);
        cpp_int lane1_dbe_enable() const;
    
        typedef pu_cpp_int< 1 > lane2_sbe_enable_cpp_int_t;
        cpp_int int_var__lane2_sbe_enable;
        void lane2_sbe_enable (const cpp_int  & l__val);
        cpp_int lane2_sbe_enable() const;
    
        typedef pu_cpp_int< 1 > lane2_dbe_enable_cpp_int_t;
        cpp_int int_var__lane2_dbe_enable;
        void lane2_dbe_enable (const cpp_int  & l__val);
        cpp_int lane2_dbe_enable() const;
    
        typedef pu_cpp_int< 1 > lane3_sbe_enable_cpp_int_t;
        cpp_int int_var__lane3_sbe_enable;
        void lane3_sbe_enable (const cpp_int  & l__val);
        cpp_int lane3_sbe_enable() const;
    
        typedef pu_cpp_int< 1 > lane3_dbe_enable_cpp_int_t;
        cpp_int int_var__lane3_dbe_enable;
        void lane3_dbe_enable (const cpp_int  & l__val);
        cpp_int lane3_dbe_enable() const;
    
        typedef pu_cpp_int< 1 > umac_CFG3_intr_enable_cpp_int_t;
        cpp_int int_var__umac_CFG3_intr_enable;
        void umac_CFG3_intr_enable (const cpp_int  & l__val);
        cpp_int umac_CFG3_intr_enable() const;
    
        typedef pu_cpp_int< 1 > umac_CFG3_pslverr_enable_cpp_int_t;
        cpp_int int_var__umac_CFG3_pslverr_enable;
        void umac_CFG3_pslverr_enable (const cpp_int  & l__val);
        cpp_int umac_CFG3_pslverr_enable() const;
    
        typedef pu_cpp_int< 1 > m0pb_pbus_drdy_enable_cpp_int_t;
        cpp_int int_var__m0pb_pbus_drdy_enable;
        void m0pb_pbus_drdy_enable (const cpp_int  & l__val);
        cpp_int m0pb_pbus_drdy_enable() const;
    
        typedef pu_cpp_int< 1 > m1pb_pbus_drdy_enable_cpp_int_t;
        cpp_int int_var__m1pb_pbus_drdy_enable;
        void m1pb_pbus_drdy_enable (const cpp_int  & l__val);
        cpp_int m1pb_pbus_drdy_enable() const;
    
        typedef pu_cpp_int< 1 > m2pb_pbus_drdy_enable_cpp_int_t;
        cpp_int int_var__m2pb_pbus_drdy_enable;
        void m2pb_pbus_drdy_enable (const cpp_int  & l__val);
        cpp_int m2pb_pbus_drdy_enable() const;
    
        typedef pu_cpp_int< 1 > m3pb_pbus_drdy_enable_cpp_int_t;
        cpp_int int_var__m3pb_pbus_drdy_enable;
        void m3pb_pbus_drdy_enable (const cpp_int  & l__val);
        cpp_int m3pb_pbus_drdy_enable() const;
    
        typedef pu_cpp_int< 1 > rx0_missing_sof_enable_cpp_int_t;
        cpp_int int_var__rx0_missing_sof_enable;
        void rx0_missing_sof_enable (const cpp_int  & l__val);
        cpp_int rx0_missing_sof_enable() const;
    
        typedef pu_cpp_int< 1 > rx0_missing_eof_enable_cpp_int_t;
        cpp_int int_var__rx0_missing_eof_enable;
        void rx0_missing_eof_enable (const cpp_int  & l__val);
        cpp_int rx0_missing_eof_enable() const;
    
        typedef pu_cpp_int< 1 > rx0_timeout_err_enable_cpp_int_t;
        cpp_int int_var__rx0_timeout_err_enable;
        void rx0_timeout_err_enable (const cpp_int  & l__val);
        cpp_int rx0_timeout_err_enable() const;
    
        typedef pu_cpp_int< 1 > rx0_min_size_err_enable_cpp_int_t;
        cpp_int int_var__rx0_min_size_err_enable;
        void rx0_min_size_err_enable (const cpp_int  & l__val);
        cpp_int rx0_min_size_err_enable() const;
    
        typedef pu_cpp_int< 1 > rx1_missing_sof_enable_cpp_int_t;
        cpp_int int_var__rx1_missing_sof_enable;
        void rx1_missing_sof_enable (const cpp_int  & l__val);
        cpp_int rx1_missing_sof_enable() const;
    
        typedef pu_cpp_int< 1 > rx1_missing_eof_enable_cpp_int_t;
        cpp_int int_var__rx1_missing_eof_enable;
        void rx1_missing_eof_enable (const cpp_int  & l__val);
        cpp_int rx1_missing_eof_enable() const;
    
        typedef pu_cpp_int< 1 > rx1_timeout_err_enable_cpp_int_t;
        cpp_int int_var__rx1_timeout_err_enable;
        void rx1_timeout_err_enable (const cpp_int  & l__val);
        cpp_int rx1_timeout_err_enable() const;
    
        typedef pu_cpp_int< 1 > rx1_min_size_err_enable_cpp_int_t;
        cpp_int int_var__rx1_min_size_err_enable;
        void rx1_min_size_err_enable (const cpp_int  & l__val);
        cpp_int rx1_min_size_err_enable() const;
    
        typedef pu_cpp_int< 1 > rx2_missing_sof_enable_cpp_int_t;
        cpp_int int_var__rx2_missing_sof_enable;
        void rx2_missing_sof_enable (const cpp_int  & l__val);
        cpp_int rx2_missing_sof_enable() const;
    
        typedef pu_cpp_int< 1 > rx2_missing_eof_enable_cpp_int_t;
        cpp_int int_var__rx2_missing_eof_enable;
        void rx2_missing_eof_enable (const cpp_int  & l__val);
        cpp_int rx2_missing_eof_enable() const;
    
        typedef pu_cpp_int< 1 > rx2_timeout_err_enable_cpp_int_t;
        cpp_int int_var__rx2_timeout_err_enable;
        void rx2_timeout_err_enable (const cpp_int  & l__val);
        cpp_int rx2_timeout_err_enable() const;
    
        typedef pu_cpp_int< 1 > rx2_min_size_err_enable_cpp_int_t;
        cpp_int int_var__rx2_min_size_err_enable;
        void rx2_min_size_err_enable (const cpp_int  & l__val);
        cpp_int rx2_min_size_err_enable() const;
    
        typedef pu_cpp_int< 1 > rx3_missing_sof_enable_cpp_int_t;
        cpp_int int_var__rx3_missing_sof_enable;
        void rx3_missing_sof_enable (const cpp_int  & l__val);
        cpp_int rx3_missing_sof_enable() const;
    
        typedef pu_cpp_int< 1 > rx3_missing_eof_enable_cpp_int_t;
        cpp_int int_var__rx3_missing_eof_enable;
        void rx3_missing_eof_enable (const cpp_int  & l__val);
        cpp_int rx3_missing_eof_enable() const;
    
        typedef pu_cpp_int< 1 > rx3_timeout_err_enable_cpp_int_t;
        cpp_int int_var__rx3_timeout_err_enable;
        void rx3_timeout_err_enable (const cpp_int  & l__val);
        cpp_int rx3_timeout_err_enable() const;
    
        typedef pu_cpp_int< 1 > rx3_min_size_err_enable_cpp_int_t;
        cpp_int int_var__rx3_min_size_err_enable;
        void rx3_min_size_err_enable (const cpp_int  & l__val);
        cpp_int rx3_min_size_err_enable() const;
    
}; // cap_mx_csr_int_mac_int_enable_clear_t
    
class cap_mx_csr_int_mac_intreg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_int_mac_intreg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_int_mac_intreg_t(string name = "cap_mx_csr_int_mac_intreg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_int_mac_intreg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > lane0_sbe_interrupt_cpp_int_t;
        cpp_int int_var__lane0_sbe_interrupt;
        void lane0_sbe_interrupt (const cpp_int  & l__val);
        cpp_int lane0_sbe_interrupt() const;
    
        typedef pu_cpp_int< 1 > lane0_dbe_interrupt_cpp_int_t;
        cpp_int int_var__lane0_dbe_interrupt;
        void lane0_dbe_interrupt (const cpp_int  & l__val);
        cpp_int lane0_dbe_interrupt() const;
    
        typedef pu_cpp_int< 1 > lane1_sbe_interrupt_cpp_int_t;
        cpp_int int_var__lane1_sbe_interrupt;
        void lane1_sbe_interrupt (const cpp_int  & l__val);
        cpp_int lane1_sbe_interrupt() const;
    
        typedef pu_cpp_int< 1 > lane1_dbe_interrupt_cpp_int_t;
        cpp_int int_var__lane1_dbe_interrupt;
        void lane1_dbe_interrupt (const cpp_int  & l__val);
        cpp_int lane1_dbe_interrupt() const;
    
        typedef pu_cpp_int< 1 > lane2_sbe_interrupt_cpp_int_t;
        cpp_int int_var__lane2_sbe_interrupt;
        void lane2_sbe_interrupt (const cpp_int  & l__val);
        cpp_int lane2_sbe_interrupt() const;
    
        typedef pu_cpp_int< 1 > lane2_dbe_interrupt_cpp_int_t;
        cpp_int int_var__lane2_dbe_interrupt;
        void lane2_dbe_interrupt (const cpp_int  & l__val);
        cpp_int lane2_dbe_interrupt() const;
    
        typedef pu_cpp_int< 1 > lane3_sbe_interrupt_cpp_int_t;
        cpp_int int_var__lane3_sbe_interrupt;
        void lane3_sbe_interrupt (const cpp_int  & l__val);
        cpp_int lane3_sbe_interrupt() const;
    
        typedef pu_cpp_int< 1 > lane3_dbe_interrupt_cpp_int_t;
        cpp_int int_var__lane3_dbe_interrupt;
        void lane3_dbe_interrupt (const cpp_int  & l__val);
        cpp_int lane3_dbe_interrupt() const;
    
        typedef pu_cpp_int< 1 > umac_CFG3_intr_interrupt_cpp_int_t;
        cpp_int int_var__umac_CFG3_intr_interrupt;
        void umac_CFG3_intr_interrupt (const cpp_int  & l__val);
        cpp_int umac_CFG3_intr_interrupt() const;
    
        typedef pu_cpp_int< 1 > umac_CFG3_pslverr_interrupt_cpp_int_t;
        cpp_int int_var__umac_CFG3_pslverr_interrupt;
        void umac_CFG3_pslverr_interrupt (const cpp_int  & l__val);
        cpp_int umac_CFG3_pslverr_interrupt() const;
    
        typedef pu_cpp_int< 1 > m0pb_pbus_drdy_interrupt_cpp_int_t;
        cpp_int int_var__m0pb_pbus_drdy_interrupt;
        void m0pb_pbus_drdy_interrupt (const cpp_int  & l__val);
        cpp_int m0pb_pbus_drdy_interrupt() const;
    
        typedef pu_cpp_int< 1 > m1pb_pbus_drdy_interrupt_cpp_int_t;
        cpp_int int_var__m1pb_pbus_drdy_interrupt;
        void m1pb_pbus_drdy_interrupt (const cpp_int  & l__val);
        cpp_int m1pb_pbus_drdy_interrupt() const;
    
        typedef pu_cpp_int< 1 > m2pb_pbus_drdy_interrupt_cpp_int_t;
        cpp_int int_var__m2pb_pbus_drdy_interrupt;
        void m2pb_pbus_drdy_interrupt (const cpp_int  & l__val);
        cpp_int m2pb_pbus_drdy_interrupt() const;
    
        typedef pu_cpp_int< 1 > m3pb_pbus_drdy_interrupt_cpp_int_t;
        cpp_int int_var__m3pb_pbus_drdy_interrupt;
        void m3pb_pbus_drdy_interrupt (const cpp_int  & l__val);
        cpp_int m3pb_pbus_drdy_interrupt() const;
    
        typedef pu_cpp_int< 1 > rx0_missing_sof_interrupt_cpp_int_t;
        cpp_int int_var__rx0_missing_sof_interrupt;
        void rx0_missing_sof_interrupt (const cpp_int  & l__val);
        cpp_int rx0_missing_sof_interrupt() const;
    
        typedef pu_cpp_int< 1 > rx0_missing_eof_interrupt_cpp_int_t;
        cpp_int int_var__rx0_missing_eof_interrupt;
        void rx0_missing_eof_interrupt (const cpp_int  & l__val);
        cpp_int rx0_missing_eof_interrupt() const;
    
        typedef pu_cpp_int< 1 > rx0_timeout_err_interrupt_cpp_int_t;
        cpp_int int_var__rx0_timeout_err_interrupt;
        void rx0_timeout_err_interrupt (const cpp_int  & l__val);
        cpp_int rx0_timeout_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > rx0_min_size_err_interrupt_cpp_int_t;
        cpp_int int_var__rx0_min_size_err_interrupt;
        void rx0_min_size_err_interrupt (const cpp_int  & l__val);
        cpp_int rx0_min_size_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > rx1_missing_sof_interrupt_cpp_int_t;
        cpp_int int_var__rx1_missing_sof_interrupt;
        void rx1_missing_sof_interrupt (const cpp_int  & l__val);
        cpp_int rx1_missing_sof_interrupt() const;
    
        typedef pu_cpp_int< 1 > rx1_missing_eof_interrupt_cpp_int_t;
        cpp_int int_var__rx1_missing_eof_interrupt;
        void rx1_missing_eof_interrupt (const cpp_int  & l__val);
        cpp_int rx1_missing_eof_interrupt() const;
    
        typedef pu_cpp_int< 1 > rx1_timeout_err_interrupt_cpp_int_t;
        cpp_int int_var__rx1_timeout_err_interrupt;
        void rx1_timeout_err_interrupt (const cpp_int  & l__val);
        cpp_int rx1_timeout_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > rx1_min_size_err_interrupt_cpp_int_t;
        cpp_int int_var__rx1_min_size_err_interrupt;
        void rx1_min_size_err_interrupt (const cpp_int  & l__val);
        cpp_int rx1_min_size_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > rx2_missing_sof_interrupt_cpp_int_t;
        cpp_int int_var__rx2_missing_sof_interrupt;
        void rx2_missing_sof_interrupt (const cpp_int  & l__val);
        cpp_int rx2_missing_sof_interrupt() const;
    
        typedef pu_cpp_int< 1 > rx2_missing_eof_interrupt_cpp_int_t;
        cpp_int int_var__rx2_missing_eof_interrupt;
        void rx2_missing_eof_interrupt (const cpp_int  & l__val);
        cpp_int rx2_missing_eof_interrupt() const;
    
        typedef pu_cpp_int< 1 > rx2_timeout_err_interrupt_cpp_int_t;
        cpp_int int_var__rx2_timeout_err_interrupt;
        void rx2_timeout_err_interrupt (const cpp_int  & l__val);
        cpp_int rx2_timeout_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > rx2_min_size_err_interrupt_cpp_int_t;
        cpp_int int_var__rx2_min_size_err_interrupt;
        void rx2_min_size_err_interrupt (const cpp_int  & l__val);
        cpp_int rx2_min_size_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > rx3_missing_sof_interrupt_cpp_int_t;
        cpp_int int_var__rx3_missing_sof_interrupt;
        void rx3_missing_sof_interrupt (const cpp_int  & l__val);
        cpp_int rx3_missing_sof_interrupt() const;
    
        typedef pu_cpp_int< 1 > rx3_missing_eof_interrupt_cpp_int_t;
        cpp_int int_var__rx3_missing_eof_interrupt;
        void rx3_missing_eof_interrupt (const cpp_int  & l__val);
        cpp_int rx3_missing_eof_interrupt() const;
    
        typedef pu_cpp_int< 1 > rx3_timeout_err_interrupt_cpp_int_t;
        cpp_int int_var__rx3_timeout_err_interrupt;
        void rx3_timeout_err_interrupt (const cpp_int  & l__val);
        cpp_int rx3_timeout_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > rx3_min_size_err_interrupt_cpp_int_t;
        cpp_int int_var__rx3_min_size_err_interrupt;
        void rx3_min_size_err_interrupt (const cpp_int  & l__val);
        cpp_int rx3_min_size_err_interrupt() const;
    
}; // cap_mx_csr_int_mac_intreg_t
    
class cap_mx_csr_int_mac_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_int_mac_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_int_mac_t(string name = "cap_mx_csr_int_mac_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_int_mac_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_mx_csr_int_mac_intreg_t intreg;
    
        cap_mx_csr_int_mac_intreg_t int_test_set;
    
        cap_mx_csr_int_mac_int_enable_clear_t int_enable_set;
    
        cap_mx_csr_int_mac_int_enable_clear_t int_enable_clear;
    
}; // cap_mx_csr_int_mac_t
    
class cap_mx_csr_intreg_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_intreg_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_intreg_status_t(string name = "cap_mx_csr_intreg_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_intreg_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > int_mac_interrupt_cpp_int_t;
        cpp_int int_var__int_mac_interrupt;
        void int_mac_interrupt (const cpp_int  & l__val);
        cpp_int int_mac_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_ecc_interrupt_cpp_int_t;
        cpp_int int_var__int_ecc_interrupt;
        void int_ecc_interrupt (const cpp_int  & l__val);
        cpp_int int_ecc_interrupt() const;
    
}; // cap_mx_csr_intreg_status_t
    
class cap_mx_csr_int_groups_int_enable_rw_reg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_int_groups_int_enable_rw_reg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_int_groups_int_enable_rw_reg_t(string name = "cap_mx_csr_int_groups_int_enable_rw_reg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_int_groups_int_enable_rw_reg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > int_mac_enable_cpp_int_t;
        cpp_int int_var__int_mac_enable;
        void int_mac_enable (const cpp_int  & l__val);
        cpp_int int_mac_enable() const;
    
        typedef pu_cpp_int< 1 > int_ecc_enable_cpp_int_t;
        cpp_int int_var__int_ecc_enable;
        void int_ecc_enable (const cpp_int  & l__val);
        cpp_int int_ecc_enable() const;
    
}; // cap_mx_csr_int_groups_int_enable_rw_reg_t
    
class cap_mx_csr_intgrp_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_intgrp_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_intgrp_status_t(string name = "cap_mx_csr_intgrp_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_intgrp_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_mx_csr_intreg_status_t intreg;
    
        cap_mx_csr_int_groups_int_enable_rw_reg_t int_enable_rw_reg;
    
        cap_mx_csr_intreg_status_t int_rw_reg;
    
}; // cap_mx_csr_intgrp_status_t
    
class cap_mx_csr_dhs_mac_stats_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_dhs_mac_stats_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_dhs_mac_stats_entry_t(string name = "cap_mx_csr_dhs_mac_stats_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_dhs_mac_stats_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_mx_csr_dhs_mac_stats_entry_t
    
class cap_mx_csr_dhs_mac_stats_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_dhs_mac_stats_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_dhs_mac_stats_t(string name = "cap_mx_csr_dhs_mac_stats_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_dhs_mac_stats_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
        cap_csr_large_array_wrapper<cap_mx_csr_dhs_mac_stats_entry_t, 512> entry;
        #else 
        cap_mx_csr_dhs_mac_stats_entry_t entry[512];
        #endif
        int get_depth_entry() { return 512; }
    
}; // cap_mx_csr_dhs_mac_stats_t
    
class cap_mx_csr_dhs_apb_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_dhs_apb_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_dhs_apb_entry_t(string name = "cap_mx_csr_dhs_apb_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_dhs_apb_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_mx_csr_dhs_apb_entry_t
    
class cap_mx_csr_dhs_apb_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_dhs_apb_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_dhs_apb_t(string name = "cap_mx_csr_dhs_apb_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_dhs_apb_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        #if 65536 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mx_csr_dhs_apb_entry_t, 65536> entry;
        #else 
        cap_mx_csr_dhs_apb_entry_t entry[65536];
        #endif
        int get_depth_entry() { return 65536; }
    
}; // cap_mx_csr_dhs_apb_t
    
class cap_mx_csr_sta_ecc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_sta_ecc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_sta_ecc_t(string name = "cap_mx_csr_sta_ecc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_sta_ecc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_mx_csr_sta_ecc_t
    
class cap_mx_csr_sta_fec_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_sta_fec_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_sta_fec_mem_t(string name = "cap_mx_csr_sta_fec_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_sta_fec_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_mx_csr_sta_fec_mem_t
    
class cap_mx_csr_cfg_fec_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_cfg_fec_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_cfg_fec_mem_t(string name = "cap_mx_csr_cfg_fec_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_cfg_fec_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        cpp_int int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        cpp_int int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
}; // cap_mx_csr_cfg_fec_mem_t
    
class cap_mx_csr_sta_stats_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_sta_stats_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_sta_stats_mem_t(string name = "cap_mx_csr_sta_stats_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_sta_stats_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_mx_csr_sta_stats_mem_t
    
class cap_mx_csr_cfg_stats_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_cfg_stats_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_cfg_stats_mem_t(string name = "cap_mx_csr_cfg_stats_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_cfg_stats_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        cpp_int int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        cpp_int int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
}; // cap_mx_csr_cfg_stats_mem_t
    
class cap_mx_csr_sta_rxfifo_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_sta_rxfifo_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_sta_rxfifo_mem_t(string name = "cap_mx_csr_sta_rxfifo_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_sta_rxfifo_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_mx_csr_sta_rxfifo_mem_t
    
class cap_mx_csr_cfg_rxfifo_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_cfg_rxfifo_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_cfg_rxfifo_mem_t(string name = "cap_mx_csr_cfg_rxfifo_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_cfg_rxfifo_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_mx_csr_cfg_rxfifo_mem_t
    
class cap_mx_csr_sta_txfifo_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_sta_txfifo_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_sta_txfifo_mem_t(string name = "cap_mx_csr_sta_txfifo_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_sta_txfifo_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_mx_csr_sta_txfifo_mem_t
    
class cap_mx_csr_cfg_txfifo_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_cfg_txfifo_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_cfg_txfifo_mem_t(string name = "cap_mx_csr_cfg_txfifo_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_cfg_txfifo_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_mx_csr_cfg_txfifo_mem_t
    
class cap_mx_csr_cfg_debug_port_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_cfg_debug_port_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_cfg_debug_port_t(string name = "cap_mx_csr_cfg_debug_port_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_cfg_debug_port_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_mx_csr_cfg_debug_port_t
    
class cap_mx_csr_cfg_fixer_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_cfg_fixer_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_cfg_fixer_t(string name = "cap_mx_csr_cfg_fixer_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_cfg_fixer_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 16 > timeout_cpp_int_t;
        cpp_int int_var__timeout;
        void timeout (const cpp_int  & l__val);
        cpp_int timeout() const;
    
}; // cap_mx_csr_cfg_fixer_t
    
class cap_mx_csr_csr_intr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_csr_intr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_csr_intr_t(string name = "cap_mx_csr_csr_intr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_csr_intr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_mx_csr_csr_intr_t
    
class cap_mx_csr_sta_mac_sd_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_sta_mac_sd_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_sta_mac_sd_status_t(string name = "cap_mx_csr_sta_mac_sd_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_sta_mac_sd_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > rx_idle_detect_cpp_int_t;
        cpp_int int_var__rx_idle_detect;
        void rx_idle_detect (const cpp_int  & l__val);
        cpp_int rx_idle_detect() const;
    
        typedef pu_cpp_int< 1 > rx_rdy_cpp_int_t;
        cpp_int int_var__rx_rdy;
        void rx_rdy (const cpp_int  & l__val);
        cpp_int rx_rdy() const;
    
        typedef pu_cpp_int< 1 > tx_rdy_cpp_int_t;
        cpp_int int_var__tx_rdy;
        void tx_rdy (const cpp_int  & l__val);
        cpp_int tx_rdy() const;
    
        typedef pu_cpp_int< 32 > core_status_cpp_int_t;
        cpp_int int_var__core_status;
        void core_status (const cpp_int  & l__val);
        cpp_int core_status() const;
    
}; // cap_mx_csr_sta_mac_sd_status_t
    
class cap_mx_csr_sta_mac_sd_interrupt_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_sta_mac_sd_interrupt_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_sta_mac_sd_interrupt_status_t(string name = "cap_mx_csr_sta_mac_sd_interrupt_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_sta_mac_sd_interrupt_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > in_progress_cpp_int_t;
        cpp_int int_var__in_progress;
        void in_progress (const cpp_int  & l__val);
        cpp_int in_progress() const;
    
        typedef pu_cpp_int< 16 > data_out_cpp_int_t;
        cpp_int int_var__data_out;
        void data_out (const cpp_int  & l__val);
        cpp_int data_out() const;
    
}; // cap_mx_csr_sta_mac_sd_interrupt_status_t
    
class cap_mx_csr_cfg_mac_sd_core_to_cntl_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_cfg_mac_sd_core_to_cntl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_cfg_mac_sd_core_to_cntl_t(string name = "cap_mx_csr_cfg_mac_sd_core_to_cntl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_cfg_mac_sd_core_to_cntl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > value_cpp_int_t;
        cpp_int int_var__value;
        void value (const cpp_int  & l__val);
        cpp_int value() const;
    
}; // cap_mx_csr_cfg_mac_sd_core_to_cntl_t
    
class cap_mx_csr_cfg_mac_sd_interrupt_request_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_cfg_mac_sd_interrupt_request_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_cfg_mac_sd_interrupt_request_t(string name = "cap_mx_csr_cfg_mac_sd_interrupt_request_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_cfg_mac_sd_interrupt_request_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_mx_csr_cfg_mac_sd_interrupt_request_t
    
class cap_mx_csr_cfg_mac_sd_interrupt_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_cfg_mac_sd_interrupt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_cfg_mac_sd_interrupt_t(string name = "cap_mx_csr_cfg_mac_sd_interrupt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_cfg_mac_sd_interrupt_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > code_cpp_int_t;
        cpp_int int_var__code;
        void code (const cpp_int  & l__val);
        cpp_int code() const;
    
        typedef pu_cpp_int< 16 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // cap_mx_csr_cfg_mac_sd_interrupt_t
    
class cap_mx_csr_cfg_mac_xoff_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_cfg_mac_xoff_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_cfg_mac_xoff_t(string name = "cap_mx_csr_cfg_mac_xoff_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_cfg_mac_xoff_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > ff_tx0fcxoff_i_cpp_int_t;
        cpp_int int_var__ff_tx0fcxoff_i;
        void ff_tx0fcxoff_i (const cpp_int  & l__val);
        cpp_int ff_tx0fcxoff_i() const;
    
        typedef pu_cpp_int< 8 > ff_tx0pfcxoff_i_cpp_int_t;
        cpp_int int_var__ff_tx0pfcxoff_i;
        void ff_tx0pfcxoff_i (const cpp_int  & l__val);
        cpp_int ff_tx0pfcxoff_i() const;
    
        typedef pu_cpp_int< 1 > ff_tx1fcxoff_i_cpp_int_t;
        cpp_int int_var__ff_tx1fcxoff_i;
        void ff_tx1fcxoff_i (const cpp_int  & l__val);
        cpp_int ff_tx1fcxoff_i() const;
    
        typedef pu_cpp_int< 8 > ff_tx1pfcxoff_i_cpp_int_t;
        cpp_int int_var__ff_tx1pfcxoff_i;
        void ff_tx1pfcxoff_i (const cpp_int  & l__val);
        cpp_int ff_tx1pfcxoff_i() const;
    
        typedef pu_cpp_int< 1 > ff_tx2fcxoff_i_cpp_int_t;
        cpp_int int_var__ff_tx2fcxoff_i;
        void ff_tx2fcxoff_i (const cpp_int  & l__val);
        cpp_int ff_tx2fcxoff_i() const;
    
        typedef pu_cpp_int< 8 > ff_tx2pfcxoff_i_cpp_int_t;
        cpp_int int_var__ff_tx2pfcxoff_i;
        void ff_tx2pfcxoff_i (const cpp_int  & l__val);
        cpp_int ff_tx2pfcxoff_i() const;
    
        typedef pu_cpp_int< 1 > ff_tx3fcxoff_i_cpp_int_t;
        cpp_int int_var__ff_tx3fcxoff_i;
        void ff_tx3fcxoff_i (const cpp_int  & l__val);
        cpp_int ff_tx3fcxoff_i() const;
    
        typedef pu_cpp_int< 8 > ff_tx3pfcxoff_i_cpp_int_t;
        cpp_int int_var__ff_tx3pfcxoff_i;
        void ff_tx3pfcxoff_i (const cpp_int  & l__val);
        cpp_int ff_tx3pfcxoff_i() const;
    
}; // cap_mx_csr_cfg_mac_xoff_t
    
class cap_mx_csr_sta_mac_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_sta_mac_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_sta_mac_t(string name = "cap_mx_csr_sta_mac_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_sta_mac_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 2 > serdes_rxcorecontrol0_o_cpp_int_t;
        cpp_int int_var__serdes_rxcorecontrol0_o;
        void serdes_rxcorecontrol0_o (const cpp_int  & l__val);
        cpp_int serdes_rxcorecontrol0_o() const;
    
        typedef pu_cpp_int< 6 > serdes_txcorecontrol0_o_cpp_int_t;
        cpp_int int_var__serdes_txcorecontrol0_o;
        void serdes_txcorecontrol0_o (const cpp_int  & l__val);
        cpp_int serdes_txcorecontrol0_o() const;
    
        typedef pu_cpp_int< 1 > ff_txafull0_o_cpp_int_t;
        cpp_int int_var__ff_txafull0_o;
        void ff_txafull0_o (const cpp_int  & l__val);
        cpp_int ff_txafull0_o() const;
    
        typedef pu_cpp_int< 1 > ff_serdes0sigok_o_cpp_int_t;
        cpp_int int_var__ff_serdes0sigok_o;
        void ff_serdes0sigok_o (const cpp_int  & l__val);
        cpp_int ff_serdes0sigok_o() const;
    
        typedef pu_cpp_int< 1 > ff_rx0sync_o_cpp_int_t;
        cpp_int int_var__ff_rx0sync_o;
        void ff_rx0sync_o (const cpp_int  & l__val);
        cpp_int ff_rx0sync_o() const;
    
        typedef pu_cpp_int< 1 > ff_tx0idle_o_cpp_int_t;
        cpp_int int_var__ff_tx0idle_o;
        void ff_tx0idle_o (const cpp_int  & l__val);
        cpp_int ff_tx0idle_o() const;
    
        typedef pu_cpp_int< 1 > ff_rx0idle_o_cpp_int_t;
        cpp_int int_var__ff_rx0idle_o;
        void ff_rx0idle_o (const cpp_int  & l__val);
        cpp_int ff_rx0idle_o() const;
    
        typedef pu_cpp_int< 1 > ff_tx0good_o_cpp_int_t;
        cpp_int int_var__ff_tx0good_o;
        void ff_tx0good_o (const cpp_int  & l__val);
        cpp_int ff_tx0good_o() const;
    
        typedef pu_cpp_int< 1 > serdes_txquiet_ts_0_o_cpp_int_t;
        cpp_int int_var__serdes_txquiet_ts_0_o;
        void serdes_txquiet_ts_0_o (const cpp_int  & l__val);
        cpp_int serdes_txquiet_ts_0_o() const;
    
        typedef pu_cpp_int< 1 > serdes_rxquiet_rs_0_o_cpp_int_t;
        cpp_int int_var__serdes_rxquiet_rs_0_o;
        void serdes_rxquiet_rs_0_o (const cpp_int  & l__val);
        cpp_int serdes_rxquiet_rs_0_o() const;
    
        typedef pu_cpp_int< 1 > ff_tx0read_o_cpp_int_t;
        cpp_int int_var__ff_tx0read_o;
        void ff_tx0read_o (const cpp_int  & l__val);
        cpp_int ff_tx0read_o() const;
    
        typedef pu_cpp_int< 1 > ff_txlpi_0_o_cpp_int_t;
        cpp_int int_var__ff_txlpi_0_o;
        void ff_txlpi_0_o (const cpp_int  & l__val);
        cpp_int ff_txlpi_0_o() const;
    
        typedef pu_cpp_int< 1 > ff_rxlpi_0_o_cpp_int_t;
        cpp_int int_var__ff_rxlpi_0_o;
        void ff_rxlpi_0_o (const cpp_int  & l__val);
        cpp_int ff_rxlpi_0_o() const;
    
        typedef pu_cpp_int< 1 > serdes_rxlpiactive_rs_0_o_cpp_int_t;
        cpp_int int_var__serdes_rxlpiactive_rs_0_o;
        void serdes_rxlpiactive_rs_0_o (const cpp_int  & l__val);
        cpp_int serdes_rxlpiactive_rs_0_o() const;
    
        typedef pu_cpp_int< 1 > serdes_txalert_ts_0_o_cpp_int_t;
        cpp_int int_var__serdes_txalert_ts_0_o;
        void serdes_txalert_ts_0_o (const cpp_int  & l__val);
        cpp_int serdes_txalert_ts_0_o() const;
    
        typedef pu_cpp_int< 2 > serdes_rxcorecontrol1_o_cpp_int_t;
        cpp_int int_var__serdes_rxcorecontrol1_o;
        void serdes_rxcorecontrol1_o (const cpp_int  & l__val);
        cpp_int serdes_rxcorecontrol1_o() const;
    
        typedef pu_cpp_int< 6 > serdes_txcorecontrol1_o_cpp_int_t;
        cpp_int int_var__serdes_txcorecontrol1_o;
        void serdes_txcorecontrol1_o (const cpp_int  & l__val);
        cpp_int serdes_txcorecontrol1_o() const;
    
        typedef pu_cpp_int< 1 > ff_txafull1_o_cpp_int_t;
        cpp_int int_var__ff_txafull1_o;
        void ff_txafull1_o (const cpp_int  & l__val);
        cpp_int ff_txafull1_o() const;
    
        typedef pu_cpp_int< 1 > ff_serdes1sigok_o_cpp_int_t;
        cpp_int int_var__ff_serdes1sigok_o;
        void ff_serdes1sigok_o (const cpp_int  & l__val);
        cpp_int ff_serdes1sigok_o() const;
    
        typedef pu_cpp_int< 1 > ff_rx1sync_o_cpp_int_t;
        cpp_int int_var__ff_rx1sync_o;
        void ff_rx1sync_o (const cpp_int  & l__val);
        cpp_int ff_rx1sync_o() const;
    
        typedef pu_cpp_int< 1 > ff_tx1idle_o_cpp_int_t;
        cpp_int int_var__ff_tx1idle_o;
        void ff_tx1idle_o (const cpp_int  & l__val);
        cpp_int ff_tx1idle_o() const;
    
        typedef pu_cpp_int< 1 > ff_rx1idle_o_cpp_int_t;
        cpp_int int_var__ff_rx1idle_o;
        void ff_rx1idle_o (const cpp_int  & l__val);
        cpp_int ff_rx1idle_o() const;
    
        typedef pu_cpp_int< 1 > ff_tx1good_o_cpp_int_t;
        cpp_int int_var__ff_tx1good_o;
        void ff_tx1good_o (const cpp_int  & l__val);
        cpp_int ff_tx1good_o() const;
    
        typedef pu_cpp_int< 1 > serdes_txquiet_ts_1_o_cpp_int_t;
        cpp_int int_var__serdes_txquiet_ts_1_o;
        void serdes_txquiet_ts_1_o (const cpp_int  & l__val);
        cpp_int serdes_txquiet_ts_1_o() const;
    
        typedef pu_cpp_int< 1 > serdes_rxquiet_rs_1_o_cpp_int_t;
        cpp_int int_var__serdes_rxquiet_rs_1_o;
        void serdes_rxquiet_rs_1_o (const cpp_int  & l__val);
        cpp_int serdes_rxquiet_rs_1_o() const;
    
        typedef pu_cpp_int< 1 > ff_tx1read_o_cpp_int_t;
        cpp_int int_var__ff_tx1read_o;
        void ff_tx1read_o (const cpp_int  & l__val);
        cpp_int ff_tx1read_o() const;
    
        typedef pu_cpp_int< 1 > ff_txlpi_1_o_cpp_int_t;
        cpp_int int_var__ff_txlpi_1_o;
        void ff_txlpi_1_o (const cpp_int  & l__val);
        cpp_int ff_txlpi_1_o() const;
    
        typedef pu_cpp_int< 1 > ff_rxlpi_1_o_cpp_int_t;
        cpp_int int_var__ff_rxlpi_1_o;
        void ff_rxlpi_1_o (const cpp_int  & l__val);
        cpp_int ff_rxlpi_1_o() const;
    
        typedef pu_cpp_int< 1 > serdes_rxlpiactive_rs_1_o_cpp_int_t;
        cpp_int int_var__serdes_rxlpiactive_rs_1_o;
        void serdes_rxlpiactive_rs_1_o (const cpp_int  & l__val);
        cpp_int serdes_rxlpiactive_rs_1_o() const;
    
        typedef pu_cpp_int< 1 > serdes_txalert_ts_1_o_cpp_int_t;
        cpp_int int_var__serdes_txalert_ts_1_o;
        void serdes_txalert_ts_1_o (const cpp_int  & l__val);
        cpp_int serdes_txalert_ts_1_o() const;
    
        typedef pu_cpp_int< 2 > serdes_rxcorecontrol2_o_cpp_int_t;
        cpp_int int_var__serdes_rxcorecontrol2_o;
        void serdes_rxcorecontrol2_o (const cpp_int  & l__val);
        cpp_int serdes_rxcorecontrol2_o() const;
    
        typedef pu_cpp_int< 6 > serdes_txcorecontrol2_o_cpp_int_t;
        cpp_int int_var__serdes_txcorecontrol2_o;
        void serdes_txcorecontrol2_o (const cpp_int  & l__val);
        cpp_int serdes_txcorecontrol2_o() const;
    
        typedef pu_cpp_int< 1 > ff_txafull2_o_cpp_int_t;
        cpp_int int_var__ff_txafull2_o;
        void ff_txafull2_o (const cpp_int  & l__val);
        cpp_int ff_txafull2_o() const;
    
        typedef pu_cpp_int< 1 > ff_serdes2sigok_o_cpp_int_t;
        cpp_int int_var__ff_serdes2sigok_o;
        void ff_serdes2sigok_o (const cpp_int  & l__val);
        cpp_int ff_serdes2sigok_o() const;
    
        typedef pu_cpp_int< 1 > ff_rx2sync_o_cpp_int_t;
        cpp_int int_var__ff_rx2sync_o;
        void ff_rx2sync_o (const cpp_int  & l__val);
        cpp_int ff_rx2sync_o() const;
    
        typedef pu_cpp_int< 1 > ff_tx2idle_o_cpp_int_t;
        cpp_int int_var__ff_tx2idle_o;
        void ff_tx2idle_o (const cpp_int  & l__val);
        cpp_int ff_tx2idle_o() const;
    
        typedef pu_cpp_int< 1 > ff_rx2idle_o_cpp_int_t;
        cpp_int int_var__ff_rx2idle_o;
        void ff_rx2idle_o (const cpp_int  & l__val);
        cpp_int ff_rx2idle_o() const;
    
        typedef pu_cpp_int< 1 > ff_tx2good_o_cpp_int_t;
        cpp_int int_var__ff_tx2good_o;
        void ff_tx2good_o (const cpp_int  & l__val);
        cpp_int ff_tx2good_o() const;
    
        typedef pu_cpp_int< 1 > serdes_txquiet_ts_2_o_cpp_int_t;
        cpp_int int_var__serdes_txquiet_ts_2_o;
        void serdes_txquiet_ts_2_o (const cpp_int  & l__val);
        cpp_int serdes_txquiet_ts_2_o() const;
    
        typedef pu_cpp_int< 1 > serdes_rxquiet_rs_2_o_cpp_int_t;
        cpp_int int_var__serdes_rxquiet_rs_2_o;
        void serdes_rxquiet_rs_2_o (const cpp_int  & l__val);
        cpp_int serdes_rxquiet_rs_2_o() const;
    
        typedef pu_cpp_int< 1 > ff_tx2read_o_cpp_int_t;
        cpp_int int_var__ff_tx2read_o;
        void ff_tx2read_o (const cpp_int  & l__val);
        cpp_int ff_tx2read_o() const;
    
        typedef pu_cpp_int< 1 > ff_txlpi_2_o_cpp_int_t;
        cpp_int int_var__ff_txlpi_2_o;
        void ff_txlpi_2_o (const cpp_int  & l__val);
        cpp_int ff_txlpi_2_o() const;
    
        typedef pu_cpp_int< 1 > ff_rxlpi_2_o_cpp_int_t;
        cpp_int int_var__ff_rxlpi_2_o;
        void ff_rxlpi_2_o (const cpp_int  & l__val);
        cpp_int ff_rxlpi_2_o() const;
    
        typedef pu_cpp_int< 1 > serdes_rxlpiactive_rs_2_o_cpp_int_t;
        cpp_int int_var__serdes_rxlpiactive_rs_2_o;
        void serdes_rxlpiactive_rs_2_o (const cpp_int  & l__val);
        cpp_int serdes_rxlpiactive_rs_2_o() const;
    
        typedef pu_cpp_int< 1 > serdes_txalert_ts_2_o_cpp_int_t;
        cpp_int int_var__serdes_txalert_ts_2_o;
        void serdes_txalert_ts_2_o (const cpp_int  & l__val);
        cpp_int serdes_txalert_ts_2_o() const;
    
        typedef pu_cpp_int< 2 > serdes_rxcorecontrol3_o_cpp_int_t;
        cpp_int int_var__serdes_rxcorecontrol3_o;
        void serdes_rxcorecontrol3_o (const cpp_int  & l__val);
        cpp_int serdes_rxcorecontrol3_o() const;
    
        typedef pu_cpp_int< 6 > serdes_txcorecontrol3_o_cpp_int_t;
        cpp_int int_var__serdes_txcorecontrol3_o;
        void serdes_txcorecontrol3_o (const cpp_int  & l__val);
        cpp_int serdes_txcorecontrol3_o() const;
    
        typedef pu_cpp_int< 1 > ff_txafull3_o_cpp_int_t;
        cpp_int int_var__ff_txafull3_o;
        void ff_txafull3_o (const cpp_int  & l__val);
        cpp_int ff_txafull3_o() const;
    
        typedef pu_cpp_int< 1 > ff_serdes3sigok_o_cpp_int_t;
        cpp_int int_var__ff_serdes3sigok_o;
        void ff_serdes3sigok_o (const cpp_int  & l__val);
        cpp_int ff_serdes3sigok_o() const;
    
        typedef pu_cpp_int< 1 > ff_rx3sync_o_cpp_int_t;
        cpp_int int_var__ff_rx3sync_o;
        void ff_rx3sync_o (const cpp_int  & l__val);
        cpp_int ff_rx3sync_o() const;
    
        typedef pu_cpp_int< 1 > ff_tx3idle_o_cpp_int_t;
        cpp_int int_var__ff_tx3idle_o;
        void ff_tx3idle_o (const cpp_int  & l__val);
        cpp_int ff_tx3idle_o() const;
    
        typedef pu_cpp_int< 1 > ff_rx3idle_o_cpp_int_t;
        cpp_int int_var__ff_rx3idle_o;
        void ff_rx3idle_o (const cpp_int  & l__val);
        cpp_int ff_rx3idle_o() const;
    
        typedef pu_cpp_int< 1 > ff_tx3good_o_cpp_int_t;
        cpp_int int_var__ff_tx3good_o;
        void ff_tx3good_o (const cpp_int  & l__val);
        cpp_int ff_tx3good_o() const;
    
        typedef pu_cpp_int< 1 > serdes_txquiet_ts_3_o_cpp_int_t;
        cpp_int int_var__serdes_txquiet_ts_3_o;
        void serdes_txquiet_ts_3_o (const cpp_int  & l__val);
        cpp_int serdes_txquiet_ts_3_o() const;
    
        typedef pu_cpp_int< 1 > serdes_rxquiet_rs_3_o_cpp_int_t;
        cpp_int int_var__serdes_rxquiet_rs_3_o;
        void serdes_rxquiet_rs_3_o (const cpp_int  & l__val);
        cpp_int serdes_rxquiet_rs_3_o() const;
    
        typedef pu_cpp_int< 1 > ff_tx3read_o_cpp_int_t;
        cpp_int int_var__ff_tx3read_o;
        void ff_tx3read_o (const cpp_int  & l__val);
        cpp_int ff_tx3read_o() const;
    
        typedef pu_cpp_int< 1 > ff_txlpi_3_o_cpp_int_t;
        cpp_int int_var__ff_txlpi_3_o;
        void ff_txlpi_3_o (const cpp_int  & l__val);
        cpp_int ff_txlpi_3_o() const;
    
        typedef pu_cpp_int< 1 > ff_rxlpi_3_o_cpp_int_t;
        cpp_int int_var__ff_rxlpi_3_o;
        void ff_rxlpi_3_o (const cpp_int  & l__val);
        cpp_int ff_rxlpi_3_o() const;
    
        typedef pu_cpp_int< 1 > serdes_rxlpiactive_rs_3_o_cpp_int_t;
        cpp_int int_var__serdes_rxlpiactive_rs_3_o;
        void serdes_rxlpiactive_rs_3_o (const cpp_int  & l__val);
        cpp_int serdes_rxlpiactive_rs_3_o() const;
    
        typedef pu_cpp_int< 1 > serdes_txalert_ts_3_o_cpp_int_t;
        cpp_int int_var__serdes_txalert_ts_3_o;
        void serdes_txalert_ts_3_o (const cpp_int  & l__val);
        cpp_int serdes_txalert_ts_3_o() const;
    
}; // cap_mx_csr_sta_mac_t
    
class cap_mx_csr_cfg_mac_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_cfg_mac_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_cfg_mac_t(string name = "cap_mx_csr_cfg_mac_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_cfg_mac_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > mac_rxsigok_i_sel_cpp_int_t;
        cpp_int int_var__mac_rxsigok_i_sel;
        void mac_rxsigok_i_sel (const cpp_int  & l__val);
        cpp_int mac_rxsigok_i_sel() const;
    
        typedef pu_cpp_int< 1 > mac_rxenergydetect_rs_i_sel_cpp_int_t;
        cpp_int int_var__mac_rxenergydetect_rs_i_sel;
        void mac_rxenergydetect_rs_i_sel (const cpp_int  & l__val);
        cpp_int mac_rxenergydetect_rs_i_sel() const;
    
        typedef pu_cpp_int< 1 > mac_tx_corestatus_i_sel_cpp_int_t;
        cpp_int int_var__mac_tx_corestatus_i_sel;
        void mac_tx_corestatus_i_sel (const cpp_int  & l__val);
        cpp_int mac_tx_corestatus_i_sel() const;
    
        typedef pu_cpp_int< 1 > mac_rx_corestatus_i_sel_cpp_int_t;
        cpp_int int_var__mac_rx_corestatus_i_sel;
        void mac_rx_corestatus_i_sel (const cpp_int  & l__val);
        cpp_int mac_rx_corestatus_i_sel() const;
    
        typedef pu_cpp_int< 1 > core_to_cntl_7_5_sel_cpp_int_t;
        cpp_int int_var__core_to_cntl_7_5_sel;
        void core_to_cntl_7_5_sel (const cpp_int  & l__val);
        cpp_int core_to_cntl_7_5_sel() const;
    
        typedef pu_cpp_int< 1 > core_to_cntl_9_8_sel_cpp_int_t;
        cpp_int int_var__core_to_cntl_9_8_sel;
        void core_to_cntl_9_8_sel (const cpp_int  & l__val);
        cpp_int core_to_cntl_9_8_sel() const;
    
        typedef pu_cpp_int< 1 > core_to_cntl_15_10_sel_cpp_int_t;
        cpp_int int_var__core_to_cntl_15_10_sel;
        void core_to_cntl_15_10_sel (const cpp_int  & l__val);
        cpp_int core_to_cntl_15_10_sel() const;
    
        typedef pu_cpp_int< 1 > serdes0_rxsigok_i_cpp_int_t;
        cpp_int int_var__serdes0_rxsigok_i;
        void serdes0_rxsigok_i (const cpp_int  & l__val);
        cpp_int serdes0_rxsigok_i() const;
    
        typedef pu_cpp_int< 1 > serdes0_tx_override_in_cpp_int_t;
        cpp_int int_var__serdes0_tx_override_in;
        void serdes0_tx_override_in (const cpp_int  & l__val);
        cpp_int serdes0_tx_override_in() const;
    
        typedef pu_cpp_int< 1 > serdes_rxenergydetect_rs_0_i_cpp_int_t;
        cpp_int int_var__serdes_rxenergydetect_rs_0_i;
        void serdes_rxenergydetect_rs_0_i (const cpp_int  & l__val);
        cpp_int serdes_rxenergydetect_rs_0_i() const;
    
        typedef pu_cpp_int< 2 > serdes_txcorestatus_0_i_cpp_int_t;
        cpp_int int_var__serdes_txcorestatus_0_i;
        void serdes_txcorestatus_0_i (const cpp_int  & l__val);
        cpp_int serdes_txcorestatus_0_i() const;
    
        typedef pu_cpp_int< 6 > serdes_rxcorestatus_0_i_cpp_int_t;
        cpp_int int_var__serdes_rxcorestatus_0_i;
        void serdes_rxcorestatus_0_i (const cpp_int  & l__val);
        cpp_int serdes_rxcorestatus_0_i() const;
    
        typedef pu_cpp_int< 1 > serdes1_rxsigok_i_cpp_int_t;
        cpp_int int_var__serdes1_rxsigok_i;
        void serdes1_rxsigok_i (const cpp_int  & l__val);
        cpp_int serdes1_rxsigok_i() const;
    
        typedef pu_cpp_int< 1 > serdes1_tx_override_in_cpp_int_t;
        cpp_int int_var__serdes1_tx_override_in;
        void serdes1_tx_override_in (const cpp_int  & l__val);
        cpp_int serdes1_tx_override_in() const;
    
        typedef pu_cpp_int< 1 > serdes_rxenergydetect_rs_1_i_cpp_int_t;
        cpp_int int_var__serdes_rxenergydetect_rs_1_i;
        void serdes_rxenergydetect_rs_1_i (const cpp_int  & l__val);
        cpp_int serdes_rxenergydetect_rs_1_i() const;
    
        typedef pu_cpp_int< 2 > serdes_txcorestatus_1_i_cpp_int_t;
        cpp_int int_var__serdes_txcorestatus_1_i;
        void serdes_txcorestatus_1_i (const cpp_int  & l__val);
        cpp_int serdes_txcorestatus_1_i() const;
    
        typedef pu_cpp_int< 6 > serdes_rxcorestatus_1_i_cpp_int_t;
        cpp_int int_var__serdes_rxcorestatus_1_i;
        void serdes_rxcorestatus_1_i (const cpp_int  & l__val);
        cpp_int serdes_rxcorestatus_1_i() const;
    
        typedef pu_cpp_int< 1 > serdes2_rxsigok_i_cpp_int_t;
        cpp_int int_var__serdes2_rxsigok_i;
        void serdes2_rxsigok_i (const cpp_int  & l__val);
        cpp_int serdes2_rxsigok_i() const;
    
        typedef pu_cpp_int< 1 > serdes2_tx_override_in_cpp_int_t;
        cpp_int int_var__serdes2_tx_override_in;
        void serdes2_tx_override_in (const cpp_int  & l__val);
        cpp_int serdes2_tx_override_in() const;
    
        typedef pu_cpp_int< 1 > serdes_rxenergydetect_rs_2_i_cpp_int_t;
        cpp_int int_var__serdes_rxenergydetect_rs_2_i;
        void serdes_rxenergydetect_rs_2_i (const cpp_int  & l__val);
        cpp_int serdes_rxenergydetect_rs_2_i() const;
    
        typedef pu_cpp_int< 2 > serdes_txcorestatus_2_i_cpp_int_t;
        cpp_int int_var__serdes_txcorestatus_2_i;
        void serdes_txcorestatus_2_i (const cpp_int  & l__val);
        cpp_int serdes_txcorestatus_2_i() const;
    
        typedef pu_cpp_int< 6 > serdes_rxcorestatus_2_i_cpp_int_t;
        cpp_int int_var__serdes_rxcorestatus_2_i;
        void serdes_rxcorestatus_2_i (const cpp_int  & l__val);
        cpp_int serdes_rxcorestatus_2_i() const;
    
        typedef pu_cpp_int< 1 > serdes3_rxsigok_i_cpp_int_t;
        cpp_int int_var__serdes3_rxsigok_i;
        void serdes3_rxsigok_i (const cpp_int  & l__val);
        cpp_int serdes3_rxsigok_i() const;
    
        typedef pu_cpp_int< 1 > serdes3_tx_override_in_cpp_int_t;
        cpp_int int_var__serdes3_tx_override_in;
        void serdes3_tx_override_in (const cpp_int  & l__val);
        cpp_int serdes3_tx_override_in() const;
    
        typedef pu_cpp_int< 1 > serdes_rxenergydetect_rs_3_i_cpp_int_t;
        cpp_int int_var__serdes_rxenergydetect_rs_3_i;
        void serdes_rxenergydetect_rs_3_i (const cpp_int  & l__val);
        cpp_int serdes_rxenergydetect_rs_3_i() const;
    
        typedef pu_cpp_int< 2 > serdes_txcorestatus_3_i_cpp_int_t;
        cpp_int int_var__serdes_txcorestatus_3_i;
        void serdes_txcorestatus_3_i (const cpp_int  & l__val);
        cpp_int serdes_txcorestatus_3_i() const;
    
        typedef pu_cpp_int< 6 > serdes_rxcorestatus_3_i_cpp_int_t;
        cpp_int int_var__serdes_rxcorestatus_3_i;
        void serdes_rxcorestatus_3_i (const cpp_int  & l__val);
        cpp_int serdes_rxcorestatus_3_i() const;
    
        typedef pu_cpp_int< 1 > ff_txlpi_0_i_cpp_int_t;
        cpp_int int_var__ff_txlpi_0_i;
        void ff_txlpi_0_i (const cpp_int  & l__val);
        cpp_int ff_txlpi_0_i() const;
    
        typedef pu_cpp_int< 1 > ff_txlpi_1_i_cpp_int_t;
        cpp_int int_var__ff_txlpi_1_i;
        void ff_txlpi_1_i (const cpp_int  & l__val);
        cpp_int ff_txlpi_1_i() const;
    
        typedef pu_cpp_int< 1 > ff_txlpi_2_i_cpp_int_t;
        cpp_int int_var__ff_txlpi_2_i;
        void ff_txlpi_2_i (const cpp_int  & l__val);
        cpp_int ff_txlpi_2_i() const;
    
        typedef pu_cpp_int< 1 > ff_txlpi_3_i_cpp_int_t;
        cpp_int int_var__ff_txlpi_3_i;
        void ff_txlpi_3_i (const cpp_int  & l__val);
        cpp_int ff_txlpi_3_i() const;
    
}; // cap_mx_csr_cfg_mac_t
    
class cap_mx_csr_cfg_mac_tdm_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_cfg_mac_tdm_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_cfg_mac_tdm_t(string name = "cap_mx_csr_cfg_mac_tdm_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_cfg_mac_tdm_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 3 > slot0_cpp_int_t;
        cpp_int int_var__slot0;
        void slot0 (const cpp_int  & l__val);
        cpp_int slot0() const;
    
        typedef pu_cpp_int< 3 > slot1_cpp_int_t;
        cpp_int int_var__slot1;
        void slot1 (const cpp_int  & l__val);
        cpp_int slot1() const;
    
        typedef pu_cpp_int< 3 > slot2_cpp_int_t;
        cpp_int int_var__slot2;
        void slot2 (const cpp_int  & l__val);
        cpp_int slot2() const;
    
        typedef pu_cpp_int< 3 > slot3_cpp_int_t;
        cpp_int int_var__slot3;
        void slot3 (const cpp_int  & l__val);
        cpp_int slot3() const;
    
}; // cap_mx_csr_cfg_mac_tdm_t
    
class cap_mx_csr_cfg_mac_gbl_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_cfg_mac_gbl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_cfg_mac_gbl_t(string name = "cap_mx_csr_cfg_mac_gbl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_cfg_mac_gbl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > ff_txdispad_i_cpp_int_t;
        cpp_int int_var__ff_txdispad_i;
        void ff_txdispad_i (const cpp_int  & l__val);
        cpp_int ff_txdispad_i() const;
    
        typedef pu_cpp_int< 1 > pkt_fcs_enable_cpp_int_t;
        cpp_int int_var__pkt_fcs_enable;
        void pkt_fcs_enable (const cpp_int  & l__val);
        cpp_int pkt_fcs_enable() const;
    
        typedef pu_cpp_int< 8 > ff_tx_ipg_cpp_int_t;
        cpp_int int_var__ff_tx_ipg;
        void ff_tx_ipg (const cpp_int  & l__val);
        cpp_int ff_tx_ipg() const;
    
        typedef pu_cpp_int< 1 > ff_txtimestampena_i_cpp_int_t;
        cpp_int int_var__ff_txtimestampena_i;
        void ff_txtimestampena_i (const cpp_int  & l__val);
        cpp_int ff_txtimestampena_i() const;
    
        typedef pu_cpp_int< 2 > ff_txtimestampid_i_cpp_int_t;
        cpp_int int_var__ff_txtimestampid_i;
        void ff_txtimestampid_i (const cpp_int  & l__val);
        cpp_int ff_txtimestampid_i() const;
    
        typedef pu_cpp_int< 1 > cg_fec_enable_i_cpp_int_t;
        cpp_int int_var__cg_fec_enable_i;
        void cg_fec_enable_i (const cpp_int  & l__val);
        cpp_int cg_fec_enable_i() const;
    
        typedef pu_cpp_int< 56 > ff_txpreamble_i_cpp_int_t;
        cpp_int int_var__ff_txpreamble_i;
        void ff_txpreamble_i (const cpp_int  & l__val);
        cpp_int ff_txpreamble_i() const;
    
}; // cap_mx_csr_cfg_mac_gbl_t
    
class cap_mx_csr_base_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_base_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_base_t(string name = "cap_mx_csr_base_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_base_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_mx_csr_base_t
    
class cap_mx_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mx_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mx_csr_t(string name = "cap_mx_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mx_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_mx_csr_base_t base;
    
        cap_mx_csr_cfg_mac_gbl_t cfg_mac_gbl;
    
        cap_mx_csr_cfg_mac_tdm_t cfg_mac_tdm;
    
        cap_mx_csr_cfg_mac_t cfg_mac;
    
        cap_mx_csr_sta_mac_t sta_mac;
    
        cap_mx_csr_cfg_mac_xoff_t cfg_mac_xoff;
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mx_csr_cfg_mac_sd_interrupt_t, 4> cfg_mac_sd_interrupt;
        #else 
        cap_mx_csr_cfg_mac_sd_interrupt_t cfg_mac_sd_interrupt[4];
        #endif
        int get_depth_cfg_mac_sd_interrupt() { return 4; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mx_csr_cfg_mac_sd_interrupt_request_t, 4> cfg_mac_sd_interrupt_request;
        #else 
        cap_mx_csr_cfg_mac_sd_interrupt_request_t cfg_mac_sd_interrupt_request[4];
        #endif
        int get_depth_cfg_mac_sd_interrupt_request() { return 4; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mx_csr_cfg_mac_sd_core_to_cntl_t, 4> cfg_mac_sd_core_to_cntl;
        #else 
        cap_mx_csr_cfg_mac_sd_core_to_cntl_t cfg_mac_sd_core_to_cntl[4];
        #endif
        int get_depth_cfg_mac_sd_core_to_cntl() { return 4; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mx_csr_sta_mac_sd_interrupt_status_t, 4> sta_mac_sd_interrupt_status;
        #else 
        cap_mx_csr_sta_mac_sd_interrupt_status_t sta_mac_sd_interrupt_status[4];
        #endif
        int get_depth_sta_mac_sd_interrupt_status() { return 4; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_mx_csr_sta_mac_sd_status_t, 4> sta_mac_sd_status;
        #else 
        cap_mx_csr_sta_mac_sd_status_t sta_mac_sd_status[4];
        #endif
        int get_depth_sta_mac_sd_status() { return 4; }
    
        cap_mx_csr_csr_intr_t csr_intr;
    
        cap_mx_csr_cfg_fixer_t cfg_fixer;
    
        cap_mx_csr_cfg_debug_port_t cfg_debug_port;
    
        cap_mx_csr_cfg_txfifo_mem_t cfg_txfifo_mem;
    
        cap_mx_csr_sta_txfifo_mem_t sta_txfifo_mem;
    
        cap_mx_csr_cfg_rxfifo_mem_t cfg_rxfifo_mem;
    
        cap_mx_csr_sta_rxfifo_mem_t sta_rxfifo_mem;
    
        cap_mx_csr_cfg_stats_mem_t cfg_stats_mem;
    
        cap_mx_csr_sta_stats_mem_t sta_stats_mem;
    
        cap_mx_csr_cfg_fec_mem_t cfg_fec_mem;
    
        cap_mx_csr_sta_fec_mem_t sta_fec_mem;
    
        cap_mx_csr_sta_ecc_t sta_ecc;
    
        cap_mx_csr_dhs_apb_t dhs_apb;
    
        cap_mx_csr_dhs_mac_stats_t dhs_mac_stats;
    
        cap_mx_csr_intgrp_status_t int_groups;
    
        cap_mx_csr_int_mac_t int_mac;
    
        cap_mx_csr_int_ecc_t int_ecc;
    
}; // cap_mx_csr_t
    
#endif // CAP_MX_CSR_H
        