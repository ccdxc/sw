
#ifndef CAP_PPA_CSR_H
#define CAP_PPA_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_ppa_csr_int_sw_phv_mem_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_int_sw_phv_mem_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_int_sw_phv_mem_int_enable_clear_t(string name = "cap_ppa_csr_int_sw_phv_mem_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_int_sw_phv_mem_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_int_sw_phv_mem_int_enable_clear_t
    
class cap_ppa_csr_int_sw_phv_mem_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_int_sw_phv_mem_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_int_sw_phv_mem_int_test_set_t(string name = "cap_ppa_csr_int_sw_phv_mem_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_int_sw_phv_mem_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_int_sw_phv_mem_int_test_set_t
    
class cap_ppa_csr_int_sw_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_int_sw_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_int_sw_phv_mem_t(string name = "cap_ppa_csr_int_sw_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_int_sw_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_ppa_csr_int_sw_phv_mem_int_test_set_t intreg;
    
        cap_ppa_csr_int_sw_phv_mem_int_test_set_t int_test_set;
    
        cap_ppa_csr_int_sw_phv_mem_int_enable_clear_t int_enable_set;
    
        cap_ppa_csr_int_sw_phv_mem_int_enable_clear_t int_enable_clear;
    
}; // cap_ppa_csr_int_sw_phv_mem_t
    
class cap_ppa_csr_int_intf_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_int_intf_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_int_intf_int_enable_clear_t(string name = "cap_ppa_csr_int_intf_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_int_intf_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 1 > dp_err_enable_cpp_int_t;
        cpp_int int_var__dp_err_enable;
        void dp_err_enable (const cpp_int  & l__val);
        cpp_int dp_err_enable() const;
    
        typedef pu_cpp_int< 1 > dp_sop_err_enable_cpp_int_t;
        cpp_int int_var__dp_sop_err_enable;
        void dp_sop_err_enable (const cpp_int  & l__val);
        cpp_int dp_sop_err_enable() const;
    
        typedef pu_cpp_int< 1 > dp_eop_err_enable_cpp_int_t;
        cpp_int int_var__dp_eop_err_enable;
        void dp_eop_err_enable (const cpp_int  & l__val);
        cpp_int dp_eop_err_enable() const;
    
        typedef pu_cpp_int< 1 > ohi_err_enable_cpp_int_t;
        cpp_int int_var__ohi_err_enable;
        void ohi_err_enable (const cpp_int  & l__val);
        cpp_int ohi_err_enable() const;
    
        typedef pu_cpp_int< 1 > ohi_sop_err_enable_cpp_int_t;
        cpp_int int_var__ohi_sop_err_enable;
        void ohi_sop_err_enable (const cpp_int  & l__val);
        cpp_int ohi_sop_err_enable() const;
    
        typedef pu_cpp_int< 1 > ohi_eop_err_enable_cpp_int_t;
        cpp_int int_var__ohi_eop_err_enable;
        void ohi_eop_err_enable (const cpp_int  & l__val);
        cpp_int ohi_eop_err_enable() const;
    
        typedef pu_cpp_int< 1 > sw_phv_err_enable_cpp_int_t;
        cpp_int int_var__sw_phv_err_enable;
        void sw_phv_err_enable (const cpp_int  & l__val);
        cpp_int sw_phv_err_enable() const;
    
        typedef pu_cpp_int< 1 > sw_phv_sop_err_enable_cpp_int_t;
        cpp_int int_var__sw_phv_sop_err_enable;
        void sw_phv_sop_err_enable (const cpp_int  & l__val);
        cpp_int sw_phv_sop_err_enable() const;
    
        typedef pu_cpp_int< 1 > sw_phv_eop_err_enable_cpp_int_t;
        cpp_int int_var__sw_phv_eop_err_enable;
        void sw_phv_eop_err_enable (const cpp_int  & l__val);
        cpp_int sw_phv_eop_err_enable() const;
    
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
    
}; // cap_ppa_csr_int_intf_int_enable_clear_t
    
class cap_ppa_csr_int_intf_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_int_intf_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_int_intf_int_test_set_t(string name = "cap_ppa_csr_int_intf_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_int_intf_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 1 > dp_err_interrupt_cpp_int_t;
        cpp_int int_var__dp_err_interrupt;
        void dp_err_interrupt (const cpp_int  & l__val);
        cpp_int dp_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > dp_sop_err_interrupt_cpp_int_t;
        cpp_int int_var__dp_sop_err_interrupt;
        void dp_sop_err_interrupt (const cpp_int  & l__val);
        cpp_int dp_sop_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > dp_eop_err_interrupt_cpp_int_t;
        cpp_int int_var__dp_eop_err_interrupt;
        void dp_eop_err_interrupt (const cpp_int  & l__val);
        cpp_int dp_eop_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > ohi_err_interrupt_cpp_int_t;
        cpp_int int_var__ohi_err_interrupt;
        void ohi_err_interrupt (const cpp_int  & l__val);
        cpp_int ohi_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > ohi_sop_err_interrupt_cpp_int_t;
        cpp_int int_var__ohi_sop_err_interrupt;
        void ohi_sop_err_interrupt (const cpp_int  & l__val);
        cpp_int ohi_sop_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > ohi_eop_err_interrupt_cpp_int_t;
        cpp_int int_var__ohi_eop_err_interrupt;
        void ohi_eop_err_interrupt (const cpp_int  & l__val);
        cpp_int ohi_eop_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > sw_phv_err_interrupt_cpp_int_t;
        cpp_int int_var__sw_phv_err_interrupt;
        void sw_phv_err_interrupt (const cpp_int  & l__val);
        cpp_int sw_phv_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > sw_phv_sop_err_interrupt_cpp_int_t;
        cpp_int int_var__sw_phv_sop_err_interrupt;
        void sw_phv_sop_err_interrupt (const cpp_int  & l__val);
        cpp_int sw_phv_sop_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > sw_phv_eop_err_interrupt_cpp_int_t;
        cpp_int int_var__sw_phv_eop_err_interrupt;
        void sw_phv_eop_err_interrupt (const cpp_int  & l__val);
        cpp_int sw_phv_eop_err_interrupt() const;
    
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
    
}; // cap_ppa_csr_int_intf_int_test_set_t
    
class cap_ppa_csr_int_intf_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_int_intf_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_int_intf_t(string name = "cap_ppa_csr_int_intf_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_int_intf_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_ppa_csr_int_intf_int_test_set_t intreg;
    
        cap_ppa_csr_int_intf_int_test_set_t int_test_set;
    
        cap_ppa_csr_int_intf_int_enable_clear_t int_enable_set;
    
        cap_ppa_csr_int_intf_int_enable_clear_t int_enable_clear;
    
}; // cap_ppa_csr_int_intf_t
    
class cap_ppa_csr_int_pa_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_int_pa_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_int_pa_int_enable_clear_t(string name = "cap_ppa_csr_int_pa_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_int_pa_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > reorder_phv_not_sop_enable_cpp_int_t;
        cpp_int int_var__reorder_phv_not_sop_enable;
        void reorder_phv_not_sop_enable (const cpp_int  & l__val);
        cpp_int reorder_phv_not_sop_enable() const;
    
        typedef pu_cpp_int< 1 > csum0_not_enuf_bytes_enable_cpp_int_t;
        cpp_int int_var__csum0_not_enuf_bytes_enable;
        void csum0_not_enuf_bytes_enable (const cpp_int  & l__val);
        cpp_int csum0_not_enuf_bytes_enable() const;
    
        typedef pu_cpp_int< 1 > csum0_start_more_than_end_offset_enable_cpp_int_t;
        cpp_int int_var__csum0_start_more_than_end_offset_enable;
        void csum0_start_more_than_end_offset_enable (const cpp_int  & l__val);
        cpp_int csum0_start_more_than_end_offset_enable() const;
    
        typedef pu_cpp_int< 1 > csum1_not_enuf_bytes_enable_cpp_int_t;
        cpp_int int_var__csum1_not_enuf_bytes_enable;
        void csum1_not_enuf_bytes_enable (const cpp_int  & l__val);
        cpp_int csum1_not_enuf_bytes_enable() const;
    
        typedef pu_cpp_int< 1 > csum1_start_more_than_end_offset_enable_cpp_int_t;
        cpp_int int_var__csum1_start_more_than_end_offset_enable;
        void csum1_start_more_than_end_offset_enable (const cpp_int  & l__val);
        cpp_int csum1_start_more_than_end_offset_enable() const;
    
        typedef pu_cpp_int< 1 > csum2_not_enuf_bytes_enable_cpp_int_t;
        cpp_int int_var__csum2_not_enuf_bytes_enable;
        void csum2_not_enuf_bytes_enable (const cpp_int  & l__val);
        cpp_int csum2_not_enuf_bytes_enable() const;
    
        typedef pu_cpp_int< 1 > csum2_start_more_than_end_offset_enable_cpp_int_t;
        cpp_int int_var__csum2_start_more_than_end_offset_enable;
        void csum2_start_more_than_end_offset_enable (const cpp_int  & l__val);
        cpp_int csum2_start_more_than_end_offset_enable() const;
    
        typedef pu_cpp_int< 1 > csum3_not_enuf_bytes_enable_cpp_int_t;
        cpp_int int_var__csum3_not_enuf_bytes_enable;
        void csum3_not_enuf_bytes_enable (const cpp_int  & l__val);
        cpp_int csum3_not_enuf_bytes_enable() const;
    
        typedef pu_cpp_int< 1 > csum3_start_more_than_end_offset_enable_cpp_int_t;
        cpp_int int_var__csum3_start_more_than_end_offset_enable;
        void csum3_start_more_than_end_offset_enable (const cpp_int  & l__val);
        cpp_int csum3_start_more_than_end_offset_enable() const;
    
        typedef pu_cpp_int< 1 > csum4_not_enuf_bytes_enable_cpp_int_t;
        cpp_int int_var__csum4_not_enuf_bytes_enable;
        void csum4_not_enuf_bytes_enable (const cpp_int  & l__val);
        cpp_int csum4_not_enuf_bytes_enable() const;
    
        typedef pu_cpp_int< 1 > csum4_start_more_than_end_offset_enable_cpp_int_t;
        cpp_int int_var__csum4_start_more_than_end_offset_enable;
        void csum4_start_more_than_end_offset_enable (const cpp_int  & l__val);
        cpp_int csum4_start_more_than_end_offset_enable() const;
    
        typedef pu_cpp_int< 1 > crc_not_enuf_bytes_enable_cpp_int_t;
        cpp_int int_var__crc_not_enuf_bytes_enable;
        void crc_not_enuf_bytes_enable (const cpp_int  & l__val);
        cpp_int crc_not_enuf_bytes_enable() const;
    
        typedef pu_cpp_int< 1 > crc_start_more_than_end_offset_enable_cpp_int_t;
        cpp_int int_var__crc_start_more_than_end_offset_enable;
        void crc_start_more_than_end_offset_enable (const cpp_int  & l__val);
        cpp_int crc_start_more_than_end_offset_enable() const;
    
        typedef pu_cpp_int< 1 > chkr_phv_single_flit_enable_cpp_int_t;
        cpp_int int_var__chkr_phv_single_flit_enable;
        void chkr_phv_single_flit_enable (const cpp_int  & l__val);
        cpp_int chkr_phv_single_flit_enable() const;
    
        typedef pu_cpp_int< 1 > chkr_phv_not_sop_enable_cpp_int_t;
        cpp_int int_var__chkr_phv_not_sop_enable;
        void chkr_phv_not_sop_enable (const cpp_int  & l__val);
        cpp_int chkr_phv_not_sop_enable() const;
    
        typedef pu_cpp_int< 1 > chkr_pkt_not_sop_enable_cpp_int_t;
        cpp_int int_var__chkr_pkt_not_sop_enable;
        void chkr_pkt_not_sop_enable (const cpp_int  & l__val);
        cpp_int chkr_pkt_not_sop_enable() const;
    
        typedef pu_cpp_int< 1 > chkr_seq_id_enable_cpp_int_t;
        cpp_int int_var__chkr_seq_id_enable;
        void chkr_seq_id_enable (const cpp_int  & l__val);
        cpp_int chkr_seq_id_enable() const;
    
        typedef pu_cpp_int< 1 > preparse_line0_not_sop_enable_cpp_int_t;
        cpp_int int_var__preparse_line0_not_sop_enable;
        void preparse_line0_not_sop_enable (const cpp_int  & l__val);
        cpp_int preparse_line0_not_sop_enable() const;
    
        typedef pu_cpp_int< 1 > preparse_dff_not_sop_enable_cpp_int_t;
        cpp_int int_var__preparse_dff_not_sop_enable;
        void preparse_dff_not_sop_enable (const cpp_int  & l__val);
        cpp_int preparse_dff_not_sop_enable() const;
    
        typedef pu_cpp_int< 1 > preparse_dff_ovflow_enable_cpp_int_t;
        cpp_int int_var__preparse_dff_ovflow_enable;
        void preparse_dff_ovflow_enable (const cpp_int  & l__val);
        cpp_int preparse_dff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > preparse_eff_ovflow_enable_cpp_int_t;
        cpp_int int_var__preparse_eff_ovflow_enable;
        void preparse_eff_ovflow_enable (const cpp_int  & l__val);
        cpp_int preparse_eff_ovflow_enable() const;
    
}; // cap_ppa_csr_int_pa_int_enable_clear_t
    
class cap_ppa_csr_int_pa_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_int_pa_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_int_pa_int_test_set_t(string name = "cap_ppa_csr_int_pa_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_int_pa_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > reorder_phv_not_sop_interrupt_cpp_int_t;
        cpp_int int_var__reorder_phv_not_sop_interrupt;
        void reorder_phv_not_sop_interrupt (const cpp_int  & l__val);
        cpp_int reorder_phv_not_sop_interrupt() const;
    
        typedef pu_cpp_int< 1 > csum0_not_enuf_bytes_interrupt_cpp_int_t;
        cpp_int int_var__csum0_not_enuf_bytes_interrupt;
        void csum0_not_enuf_bytes_interrupt (const cpp_int  & l__val);
        cpp_int csum0_not_enuf_bytes_interrupt() const;
    
        typedef pu_cpp_int< 1 > csum0_start_more_than_end_offset_interrupt_cpp_int_t;
        cpp_int int_var__csum0_start_more_than_end_offset_interrupt;
        void csum0_start_more_than_end_offset_interrupt (const cpp_int  & l__val);
        cpp_int csum0_start_more_than_end_offset_interrupt() const;
    
        typedef pu_cpp_int< 1 > csum1_not_enuf_bytes_interrupt_cpp_int_t;
        cpp_int int_var__csum1_not_enuf_bytes_interrupt;
        void csum1_not_enuf_bytes_interrupt (const cpp_int  & l__val);
        cpp_int csum1_not_enuf_bytes_interrupt() const;
    
        typedef pu_cpp_int< 1 > csum1_start_more_than_end_offset_interrupt_cpp_int_t;
        cpp_int int_var__csum1_start_more_than_end_offset_interrupt;
        void csum1_start_more_than_end_offset_interrupt (const cpp_int  & l__val);
        cpp_int csum1_start_more_than_end_offset_interrupt() const;
    
        typedef pu_cpp_int< 1 > csum2_not_enuf_bytes_interrupt_cpp_int_t;
        cpp_int int_var__csum2_not_enuf_bytes_interrupt;
        void csum2_not_enuf_bytes_interrupt (const cpp_int  & l__val);
        cpp_int csum2_not_enuf_bytes_interrupt() const;
    
        typedef pu_cpp_int< 1 > csum2_start_more_than_end_offset_interrupt_cpp_int_t;
        cpp_int int_var__csum2_start_more_than_end_offset_interrupt;
        void csum2_start_more_than_end_offset_interrupt (const cpp_int  & l__val);
        cpp_int csum2_start_more_than_end_offset_interrupt() const;
    
        typedef pu_cpp_int< 1 > csum3_not_enuf_bytes_interrupt_cpp_int_t;
        cpp_int int_var__csum3_not_enuf_bytes_interrupt;
        void csum3_not_enuf_bytes_interrupt (const cpp_int  & l__val);
        cpp_int csum3_not_enuf_bytes_interrupt() const;
    
        typedef pu_cpp_int< 1 > csum3_start_more_than_end_offset_interrupt_cpp_int_t;
        cpp_int int_var__csum3_start_more_than_end_offset_interrupt;
        void csum3_start_more_than_end_offset_interrupt (const cpp_int  & l__val);
        cpp_int csum3_start_more_than_end_offset_interrupt() const;
    
        typedef pu_cpp_int< 1 > csum4_not_enuf_bytes_interrupt_cpp_int_t;
        cpp_int int_var__csum4_not_enuf_bytes_interrupt;
        void csum4_not_enuf_bytes_interrupt (const cpp_int  & l__val);
        cpp_int csum4_not_enuf_bytes_interrupt() const;
    
        typedef pu_cpp_int< 1 > csum4_start_more_than_end_offset_interrupt_cpp_int_t;
        cpp_int int_var__csum4_start_more_than_end_offset_interrupt;
        void csum4_start_more_than_end_offset_interrupt (const cpp_int  & l__val);
        cpp_int csum4_start_more_than_end_offset_interrupt() const;
    
        typedef pu_cpp_int< 1 > crc_not_enuf_bytes_interrupt_cpp_int_t;
        cpp_int int_var__crc_not_enuf_bytes_interrupt;
        void crc_not_enuf_bytes_interrupt (const cpp_int  & l__val);
        cpp_int crc_not_enuf_bytes_interrupt() const;
    
        typedef pu_cpp_int< 1 > crc_start_more_than_end_offset_interrupt_cpp_int_t;
        cpp_int int_var__crc_start_more_than_end_offset_interrupt;
        void crc_start_more_than_end_offset_interrupt (const cpp_int  & l__val);
        cpp_int crc_start_more_than_end_offset_interrupt() const;
    
        typedef pu_cpp_int< 1 > chkr_phv_single_flit_interrupt_cpp_int_t;
        cpp_int int_var__chkr_phv_single_flit_interrupt;
        void chkr_phv_single_flit_interrupt (const cpp_int  & l__val);
        cpp_int chkr_phv_single_flit_interrupt() const;
    
        typedef pu_cpp_int< 1 > chkr_phv_not_sop_interrupt_cpp_int_t;
        cpp_int int_var__chkr_phv_not_sop_interrupt;
        void chkr_phv_not_sop_interrupt (const cpp_int  & l__val);
        cpp_int chkr_phv_not_sop_interrupt() const;
    
        typedef pu_cpp_int< 1 > chkr_pkt_not_sop_interrupt_cpp_int_t;
        cpp_int int_var__chkr_pkt_not_sop_interrupt;
        void chkr_pkt_not_sop_interrupt (const cpp_int  & l__val);
        cpp_int chkr_pkt_not_sop_interrupt() const;
    
        typedef pu_cpp_int< 1 > chkr_seq_id_interrupt_cpp_int_t;
        cpp_int int_var__chkr_seq_id_interrupt;
        void chkr_seq_id_interrupt (const cpp_int  & l__val);
        cpp_int chkr_seq_id_interrupt() const;
    
        typedef pu_cpp_int< 1 > preparse_line0_not_sop_interrupt_cpp_int_t;
        cpp_int int_var__preparse_line0_not_sop_interrupt;
        void preparse_line0_not_sop_interrupt (const cpp_int  & l__val);
        cpp_int preparse_line0_not_sop_interrupt() const;
    
        typedef pu_cpp_int< 1 > preparse_dff_not_sop_interrupt_cpp_int_t;
        cpp_int int_var__preparse_dff_not_sop_interrupt;
        void preparse_dff_not_sop_interrupt (const cpp_int  & l__val);
        cpp_int preparse_dff_not_sop_interrupt() const;
    
        typedef pu_cpp_int< 1 > preparse_dff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__preparse_dff_ovflow_interrupt;
        void preparse_dff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int preparse_dff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > preparse_eff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__preparse_eff_ovflow_interrupt;
        void preparse_eff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int preparse_eff_ovflow_interrupt() const;
    
}; // cap_ppa_csr_int_pa_int_test_set_t
    
class cap_ppa_csr_int_pa_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_int_pa_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_int_pa_t(string name = "cap_ppa_csr_int_pa_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_int_pa_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_ppa_csr_int_pa_int_test_set_t intreg;
    
        cap_ppa_csr_int_pa_int_test_set_t int_test_set;
    
        cap_ppa_csr_int_pa_int_enable_clear_t int_enable_set;
    
        cap_ppa_csr_int_pa_int_enable_clear_t int_enable_clear;
    
}; // cap_ppa_csr_int_pa_t
    
class cap_ppa_csr_int_pe9_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_int_pe9_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_int_pe9_int_enable_clear_t(string name = "cap_ppa_csr_int_pe9_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_int_pe9_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > qctl_skid_depth_not_zero_enable_cpp_int_t;
        cpp_int int_var__qctl_skid_depth_not_zero_enable;
        void qctl_skid_depth_not_zero_enable (const cpp_int  & l__val);
        cpp_int qctl_skid_depth_not_zero_enable() const;
    
        typedef pu_cpp_int< 1 > qctl_not_sop_after_pkt_release_enable_cpp_int_t;
        cpp_int int_var__qctl_not_sop_after_pkt_release_enable;
        void qctl_not_sop_after_pkt_release_enable (const cpp_int  & l__val);
        cpp_int qctl_not_sop_after_pkt_release_enable() const;
    
        typedef pu_cpp_int< 1 > unexpected_non_sop_enable_cpp_int_t;
        cpp_int int_var__unexpected_non_sop_enable;
        void unexpected_non_sop_enable (const cpp_int  & l__val);
        cpp_int unexpected_non_sop_enable() const;
    
        typedef pu_cpp_int< 1 > exceed_phv_flit_cnt_enable_cpp_int_t;
        cpp_int int_var__exceed_phv_flit_cnt_enable;
        void exceed_phv_flit_cnt_enable (const cpp_int  & l__val);
        cpp_int exceed_phv_flit_cnt_enable() const;
    
        typedef pu_cpp_int< 1 > exceed_parse_loop_cnt_enable_cpp_int_t;
        cpp_int int_var__exceed_parse_loop_cnt_enable;
        void exceed_parse_loop_cnt_enable (const cpp_int  & l__val);
        cpp_int exceed_parse_loop_cnt_enable() const;
    
        typedef pu_cpp_int< 1 > offset_out_of_range_enable_cpp_int_t;
        cpp_int int_var__offset_out_of_range_enable;
        void offset_out_of_range_enable (const cpp_int  & l__val);
        cpp_int offset_out_of_range_enable() const;
    
        typedef pu_cpp_int< 1 > pkt_size_chk_enable_cpp_int_t;
        cpp_int int_var__pkt_size_chk_enable;
        void pkt_size_chk_enable (const cpp_int  & l__val);
        cpp_int pkt_size_chk_enable() const;
    
        typedef pu_cpp_int< 1 > offset_jump_chk_enable_cpp_int_t;
        cpp_int int_var__offset_jump_chk_enable;
        void offset_jump_chk_enable (const cpp_int  & l__val);
        cpp_int offset_jump_chk_enable() const;
    
        typedef pu_cpp_int< 1 > d1_not_valid_enable_cpp_int_t;
        cpp_int int_var__d1_not_valid_enable;
        void d1_not_valid_enable (const cpp_int  & l__val);
        cpp_int d1_not_valid_enable() const;
    
        typedef pu_cpp_int< 1 > phv_upr_idx_less_enable_cpp_int_t;
        cpp_int int_var__phv_upr_idx_less_enable;
        void phv_upr_idx_less_enable (const cpp_int  & l__val);
        cpp_int phv_upr_idx_less_enable() const;
    
}; // cap_ppa_csr_int_pe9_int_enable_clear_t
    
class cap_ppa_csr_int_pe9_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_int_pe9_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_int_pe9_int_test_set_t(string name = "cap_ppa_csr_int_pe9_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_int_pe9_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > qctl_skid_depth_not_zero_interrupt_cpp_int_t;
        cpp_int int_var__qctl_skid_depth_not_zero_interrupt;
        void qctl_skid_depth_not_zero_interrupt (const cpp_int  & l__val);
        cpp_int qctl_skid_depth_not_zero_interrupt() const;
    
        typedef pu_cpp_int< 1 > qctl_not_sop_after_pkt_release_interrupt_cpp_int_t;
        cpp_int int_var__qctl_not_sop_after_pkt_release_interrupt;
        void qctl_not_sop_after_pkt_release_interrupt (const cpp_int  & l__val);
        cpp_int qctl_not_sop_after_pkt_release_interrupt() const;
    
        typedef pu_cpp_int< 1 > unexpected_non_sop_interrupt_cpp_int_t;
        cpp_int int_var__unexpected_non_sop_interrupt;
        void unexpected_non_sop_interrupt (const cpp_int  & l__val);
        cpp_int unexpected_non_sop_interrupt() const;
    
        typedef pu_cpp_int< 1 > exceed_phv_flit_cnt_interrupt_cpp_int_t;
        cpp_int int_var__exceed_phv_flit_cnt_interrupt;
        void exceed_phv_flit_cnt_interrupt (const cpp_int  & l__val);
        cpp_int exceed_phv_flit_cnt_interrupt() const;
    
        typedef pu_cpp_int< 1 > exceed_parse_loop_cnt_interrupt_cpp_int_t;
        cpp_int int_var__exceed_parse_loop_cnt_interrupt;
        void exceed_parse_loop_cnt_interrupt (const cpp_int  & l__val);
        cpp_int exceed_parse_loop_cnt_interrupt() const;
    
        typedef pu_cpp_int< 1 > offset_out_of_range_interrupt_cpp_int_t;
        cpp_int int_var__offset_out_of_range_interrupt;
        void offset_out_of_range_interrupt (const cpp_int  & l__val);
        cpp_int offset_out_of_range_interrupt() const;
    
        typedef pu_cpp_int< 1 > pkt_size_chk_interrupt_cpp_int_t;
        cpp_int int_var__pkt_size_chk_interrupt;
        void pkt_size_chk_interrupt (const cpp_int  & l__val);
        cpp_int pkt_size_chk_interrupt() const;
    
        typedef pu_cpp_int< 1 > offset_jump_chk_interrupt_cpp_int_t;
        cpp_int int_var__offset_jump_chk_interrupt;
        void offset_jump_chk_interrupt (const cpp_int  & l__val);
        cpp_int offset_jump_chk_interrupt() const;
    
        typedef pu_cpp_int< 1 > d1_not_valid_interrupt_cpp_int_t;
        cpp_int int_var__d1_not_valid_interrupt;
        void d1_not_valid_interrupt (const cpp_int  & l__val);
        cpp_int d1_not_valid_interrupt() const;
    
        typedef pu_cpp_int< 1 > phv_upr_idx_less_interrupt_cpp_int_t;
        cpp_int int_var__phv_upr_idx_less_interrupt;
        void phv_upr_idx_less_interrupt (const cpp_int  & l__val);
        cpp_int phv_upr_idx_less_interrupt() const;
    
}; // cap_ppa_csr_int_pe9_int_test_set_t
    
class cap_ppa_csr_int_pe9_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_int_pe9_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_int_pe9_t(string name = "cap_ppa_csr_int_pe9_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_int_pe9_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_ppa_csr_int_pe9_int_test_set_t intreg;
    
        cap_ppa_csr_int_pe9_int_test_set_t int_test_set;
    
        cap_ppa_csr_int_pe9_int_enable_clear_t int_enable_set;
    
        cap_ppa_csr_int_pe9_int_enable_clear_t int_enable_clear;
    
}; // cap_ppa_csr_int_pe9_t
    
class cap_ppa_csr_int_fifo2_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_int_fifo2_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_int_fifo2_int_enable_clear_t(string name = "cap_ppa_csr_int_fifo2_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_int_fifo2_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > dp_if_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__dp_if_ff_ovflow_enable;
        void dp_if_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int dp_if_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > ohi_if_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__ohi_if_ff_ovflow_enable;
        void ohi_if_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int ohi_if_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > ma_if_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__ma_if_ff_ovflow_enable;
        void ma_if_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int ma_if_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pb_if_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pb_if_ff_ovflow_enable;
        void pb_if_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pb_if_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pkt_mtu_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pkt_mtu_ff_ovflow_enable;
        void pkt_mtu_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pkt_mtu_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > outphv_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__outphv_ff_ovflow_enable;
        void outphv_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int outphv_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > outohi_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__outohi_ff_ovflow_enable;
        void outohi_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int outohi_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > chk_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__chk_ff_ovflow_enable;
        void chk_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int chk_ff_ovflow_enable() const;
    
}; // cap_ppa_csr_int_fifo2_int_enable_clear_t
    
class cap_ppa_csr_int_fifo2_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_int_fifo2_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_int_fifo2_int_test_set_t(string name = "cap_ppa_csr_int_fifo2_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_int_fifo2_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > dp_if_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__dp_if_ff_ovflow_interrupt;
        void dp_if_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int dp_if_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > ohi_if_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__ohi_if_ff_ovflow_interrupt;
        void ohi_if_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int ohi_if_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > ma_if_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__ma_if_ff_ovflow_interrupt;
        void ma_if_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int ma_if_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pb_if_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pb_if_ff_ovflow_interrupt;
        void pb_if_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pb_if_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pkt_mtu_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pkt_mtu_ff_ovflow_interrupt;
        void pkt_mtu_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pkt_mtu_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > outphv_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__outphv_ff_ovflow_interrupt;
        void outphv_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int outphv_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > outohi_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__outohi_ff_ovflow_interrupt;
        void outohi_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int outohi_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > chk_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__chk_ff_ovflow_interrupt;
        void chk_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int chk_ff_ovflow_interrupt() const;
    
}; // cap_ppa_csr_int_fifo2_int_test_set_t
    
class cap_ppa_csr_int_fifo2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_int_fifo2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_int_fifo2_t(string name = "cap_ppa_csr_int_fifo2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_int_fifo2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_ppa_csr_int_fifo2_int_test_set_t intreg;
    
        cap_ppa_csr_int_fifo2_int_test_set_t int_test_set;
    
        cap_ppa_csr_int_fifo2_int_enable_clear_t int_enable_set;
    
        cap_ppa_csr_int_fifo2_int_enable_clear_t int_enable_clear;
    
}; // cap_ppa_csr_int_fifo2_t
    
class cap_ppa_csr_int_fifo1_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_int_fifo1_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_int_fifo1_int_enable_clear_t(string name = "cap_ppa_csr_int_fifo1_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_int_fifo1_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > pe0_qctl_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pe0_qctl_ff_ovflow_enable;
        void pe0_qctl_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pe0_qctl_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pe1_qctl_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pe1_qctl_ff_ovflow_enable;
        void pe1_qctl_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pe1_qctl_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pe2_qctl_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pe2_qctl_ff_ovflow_enable;
        void pe2_qctl_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pe2_qctl_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pe3_qctl_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pe3_qctl_ff_ovflow_enable;
        void pe3_qctl_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pe3_qctl_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pe4_qctl_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pe4_qctl_ff_ovflow_enable;
        void pe4_qctl_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pe4_qctl_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pe5_qctl_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pe5_qctl_ff_ovflow_enable;
        void pe5_qctl_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pe5_qctl_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pe6_qctl_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pe6_qctl_ff_ovflow_enable;
        void pe6_qctl_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pe6_qctl_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pe7_qctl_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pe7_qctl_ff_ovflow_enable;
        void pe7_qctl_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pe7_qctl_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pe8_qctl_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pe8_qctl_ff_ovflow_enable;
        void pe8_qctl_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pe8_qctl_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pe9_qctl_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pe9_qctl_ff_ovflow_enable;
        void pe9_qctl_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pe9_qctl_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pe0_phv_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pe0_phv_ff_ovflow_enable;
        void pe0_phv_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pe0_phv_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pe1_phv_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pe1_phv_ff_ovflow_enable;
        void pe1_phv_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pe1_phv_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pe2_phv_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pe2_phv_ff_ovflow_enable;
        void pe2_phv_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pe2_phv_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pe3_phv_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pe3_phv_ff_ovflow_enable;
        void pe3_phv_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pe3_phv_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pe4_phv_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pe4_phv_ff_ovflow_enable;
        void pe4_phv_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pe4_phv_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pe5_phv_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pe5_phv_ff_ovflow_enable;
        void pe5_phv_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pe5_phv_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pe6_phv_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pe6_phv_ff_ovflow_enable;
        void pe6_phv_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pe6_phv_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pe7_phv_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pe7_phv_ff_ovflow_enable;
        void pe7_phv_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pe7_phv_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pe8_phv_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pe8_phv_ff_ovflow_enable;
        void pe8_phv_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pe8_phv_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pe9_phv_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pe9_phv_ff_ovflow_enable;
        void pe9_phv_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pe9_phv_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pe0_ohi_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pe0_ohi_ff_ovflow_enable;
        void pe0_ohi_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pe0_ohi_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pe1_ohi_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pe1_ohi_ff_ovflow_enable;
        void pe1_ohi_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pe1_ohi_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pe2_ohi_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pe2_ohi_ff_ovflow_enable;
        void pe2_ohi_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pe2_ohi_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pe3_ohi_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pe3_ohi_ff_ovflow_enable;
        void pe3_ohi_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pe3_ohi_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pe4_ohi_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pe4_ohi_ff_ovflow_enable;
        void pe4_ohi_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pe4_ohi_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pe5_ohi_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pe5_ohi_ff_ovflow_enable;
        void pe5_ohi_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pe5_ohi_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pe6_ohi_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pe6_ohi_ff_ovflow_enable;
        void pe6_ohi_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pe6_ohi_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pe7_ohi_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pe7_ohi_ff_ovflow_enable;
        void pe7_ohi_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pe7_ohi_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pe8_ohi_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pe8_ohi_ff_ovflow_enable;
        void pe8_ohi_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pe8_ohi_ff_ovflow_enable() const;
    
        typedef pu_cpp_int< 1 > pe9_ohi_ff_ovflow_enable_cpp_int_t;
        cpp_int int_var__pe9_ohi_ff_ovflow_enable;
        void pe9_ohi_ff_ovflow_enable (const cpp_int  & l__val);
        cpp_int pe9_ohi_ff_ovflow_enable() const;
    
}; // cap_ppa_csr_int_fifo1_int_enable_clear_t
    
class cap_ppa_csr_int_fifo1_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_int_fifo1_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_int_fifo1_int_test_set_t(string name = "cap_ppa_csr_int_fifo1_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_int_fifo1_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > pe0_qctl_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pe0_qctl_ff_ovflow_interrupt;
        void pe0_qctl_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pe0_qctl_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pe1_qctl_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pe1_qctl_ff_ovflow_interrupt;
        void pe1_qctl_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pe1_qctl_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pe2_qctl_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pe2_qctl_ff_ovflow_interrupt;
        void pe2_qctl_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pe2_qctl_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pe3_qctl_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pe3_qctl_ff_ovflow_interrupt;
        void pe3_qctl_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pe3_qctl_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pe4_qctl_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pe4_qctl_ff_ovflow_interrupt;
        void pe4_qctl_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pe4_qctl_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pe5_qctl_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pe5_qctl_ff_ovflow_interrupt;
        void pe5_qctl_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pe5_qctl_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pe6_qctl_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pe6_qctl_ff_ovflow_interrupt;
        void pe6_qctl_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pe6_qctl_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pe7_qctl_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pe7_qctl_ff_ovflow_interrupt;
        void pe7_qctl_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pe7_qctl_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pe8_qctl_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pe8_qctl_ff_ovflow_interrupt;
        void pe8_qctl_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pe8_qctl_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pe9_qctl_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pe9_qctl_ff_ovflow_interrupt;
        void pe9_qctl_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pe9_qctl_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pe0_phv_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pe0_phv_ff_ovflow_interrupt;
        void pe0_phv_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pe0_phv_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pe1_phv_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pe1_phv_ff_ovflow_interrupt;
        void pe1_phv_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pe1_phv_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pe2_phv_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pe2_phv_ff_ovflow_interrupt;
        void pe2_phv_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pe2_phv_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pe3_phv_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pe3_phv_ff_ovflow_interrupt;
        void pe3_phv_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pe3_phv_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pe4_phv_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pe4_phv_ff_ovflow_interrupt;
        void pe4_phv_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pe4_phv_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pe5_phv_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pe5_phv_ff_ovflow_interrupt;
        void pe5_phv_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pe5_phv_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pe6_phv_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pe6_phv_ff_ovflow_interrupt;
        void pe6_phv_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pe6_phv_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pe7_phv_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pe7_phv_ff_ovflow_interrupt;
        void pe7_phv_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pe7_phv_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pe8_phv_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pe8_phv_ff_ovflow_interrupt;
        void pe8_phv_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pe8_phv_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pe9_phv_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pe9_phv_ff_ovflow_interrupt;
        void pe9_phv_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pe9_phv_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pe0_ohi_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pe0_ohi_ff_ovflow_interrupt;
        void pe0_ohi_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pe0_ohi_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pe1_ohi_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pe1_ohi_ff_ovflow_interrupt;
        void pe1_ohi_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pe1_ohi_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pe2_ohi_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pe2_ohi_ff_ovflow_interrupt;
        void pe2_ohi_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pe2_ohi_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pe3_ohi_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pe3_ohi_ff_ovflow_interrupt;
        void pe3_ohi_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pe3_ohi_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pe4_ohi_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pe4_ohi_ff_ovflow_interrupt;
        void pe4_ohi_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pe4_ohi_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pe5_ohi_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pe5_ohi_ff_ovflow_interrupt;
        void pe5_ohi_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pe5_ohi_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pe6_ohi_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pe6_ohi_ff_ovflow_interrupt;
        void pe6_ohi_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pe6_ohi_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pe7_ohi_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pe7_ohi_ff_ovflow_interrupt;
        void pe7_ohi_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pe7_ohi_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pe8_ohi_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pe8_ohi_ff_ovflow_interrupt;
        void pe8_ohi_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pe8_ohi_ff_ovflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > pe9_ohi_ff_ovflow_interrupt_cpp_int_t;
        cpp_int int_var__pe9_ohi_ff_ovflow_interrupt;
        void pe9_ohi_ff_ovflow_interrupt (const cpp_int  & l__val);
        cpp_int pe9_ohi_ff_ovflow_interrupt() const;
    
}; // cap_ppa_csr_int_fifo1_int_test_set_t
    
class cap_ppa_csr_int_fifo1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_int_fifo1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_int_fifo1_t(string name = "cap_ppa_csr_int_fifo1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_int_fifo1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_ppa_csr_int_fifo1_int_test_set_t intreg;
    
        cap_ppa_csr_int_fifo1_int_test_set_t int_test_set;
    
        cap_ppa_csr_int_fifo1_int_enable_clear_t int_enable_set;
    
        cap_ppa_csr_int_fifo1_int_enable_clear_t int_enable_clear;
    
}; // cap_ppa_csr_int_fifo1_t
    
class cap_ppa_csr_int_ecc_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_int_ecc_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_int_ecc_int_enable_clear_t(string name = "cap_ppa_csr_int_ecc_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_int_ecc_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > pkt_mem_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__pkt_mem_uncorrectable_enable;
        void pkt_mem_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int pkt_mem_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > pkt_mem_correctable_enable_cpp_int_t;
        cpp_int int_var__pkt_mem_correctable_enable;
        void pkt_mem_correctable_enable (const cpp_int  & l__val);
        cpp_int pkt_mem_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > bndl0_state_lkp_sram_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__bndl0_state_lkp_sram_uncorrectable_enable;
        void bndl0_state_lkp_sram_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int bndl0_state_lkp_sram_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > bndl0_state_lkp_sram_correctable_enable_cpp_int_t;
        cpp_int int_var__bndl0_state_lkp_sram_correctable_enable;
        void bndl0_state_lkp_sram_correctable_enable (const cpp_int  & l__val);
        cpp_int bndl0_state_lkp_sram_correctable_enable() const;
    
        typedef pu_cpp_int< 1 > bndl1_state_lkp_sram_uncorrectable_enable_cpp_int_t;
        cpp_int int_var__bndl1_state_lkp_sram_uncorrectable_enable;
        void bndl1_state_lkp_sram_uncorrectable_enable (const cpp_int  & l__val);
        cpp_int bndl1_state_lkp_sram_uncorrectable_enable() const;
    
        typedef pu_cpp_int< 1 > bndl1_state_lkp_sram_correctable_enable_cpp_int_t;
        cpp_int int_var__bndl1_state_lkp_sram_correctable_enable;
        void bndl1_state_lkp_sram_correctable_enable (const cpp_int  & l__val);
        cpp_int bndl1_state_lkp_sram_correctable_enable() const;
    
}; // cap_ppa_csr_int_ecc_int_enable_clear_t
    
class cap_ppa_csr_int_ecc_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_int_ecc_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_int_ecc_int_test_set_t(string name = "cap_ppa_csr_int_ecc_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_int_ecc_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > pkt_mem_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__pkt_mem_uncorrectable_interrupt;
        void pkt_mem_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int pkt_mem_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > pkt_mem_correctable_interrupt_cpp_int_t;
        cpp_int int_var__pkt_mem_correctable_interrupt;
        void pkt_mem_correctable_interrupt (const cpp_int  & l__val);
        cpp_int pkt_mem_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > bndl0_state_lkp_sram_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__bndl0_state_lkp_sram_uncorrectable_interrupt;
        void bndl0_state_lkp_sram_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int bndl0_state_lkp_sram_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > bndl0_state_lkp_sram_correctable_interrupt_cpp_int_t;
        cpp_int int_var__bndl0_state_lkp_sram_correctable_interrupt;
        void bndl0_state_lkp_sram_correctable_interrupt (const cpp_int  & l__val);
        cpp_int bndl0_state_lkp_sram_correctable_interrupt() const;
    
        typedef pu_cpp_int< 1 > bndl1_state_lkp_sram_uncorrectable_interrupt_cpp_int_t;
        cpp_int int_var__bndl1_state_lkp_sram_uncorrectable_interrupt;
        void bndl1_state_lkp_sram_uncorrectable_interrupt (const cpp_int  & l__val);
        cpp_int bndl1_state_lkp_sram_uncorrectable_interrupt() const;
    
        typedef pu_cpp_int< 1 > bndl1_state_lkp_sram_correctable_interrupt_cpp_int_t;
        cpp_int int_var__bndl1_state_lkp_sram_correctable_interrupt;
        void bndl1_state_lkp_sram_correctable_interrupt (const cpp_int  & l__val);
        cpp_int bndl1_state_lkp_sram_correctable_interrupt() const;
    
}; // cap_ppa_csr_int_ecc_int_test_set_t
    
class cap_ppa_csr_int_ecc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_int_ecc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_int_ecc_t(string name = "cap_ppa_csr_int_ecc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_int_ecc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_ppa_csr_int_ecc_int_test_set_t intreg;
    
        cap_ppa_csr_int_ecc_int_test_set_t int_test_set;
    
        cap_ppa_csr_int_ecc_int_enable_clear_t int_enable_set;
    
        cap_ppa_csr_int_ecc_int_enable_clear_t int_enable_clear;
    
}; // cap_ppa_csr_int_ecc_t
    
class cap_ppa_csr_int_bndl1_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_int_bndl1_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_int_bndl1_int_enable_clear_t(string name = "cap_ppa_csr_int_bndl1_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_int_bndl1_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > state_lkp_srch_hit_addr_range_err_enable_cpp_int_t;
        cpp_int int_var__state_lkp_srch_hit_addr_range_err_enable;
        void state_lkp_srch_hit_addr_range_err_enable (const cpp_int  & l__val);
        cpp_int state_lkp_srch_hit_addr_range_err_enable() const;
    
        typedef pu_cpp_int< 1 > state_lkp_srch_miss_enable_cpp_int_t;
        cpp_int int_var__state_lkp_srch_miss_enable;
        void state_lkp_srch_miss_enable (const cpp_int  & l__val);
        cpp_int state_lkp_srch_miss_enable() const;
    
        typedef pu_cpp_int< 1 > state_lkp_srch_req_not_rdy_enable_cpp_int_t;
        cpp_int int_var__state_lkp_srch_req_not_rdy_enable;
        void state_lkp_srch_req_not_rdy_enable (const cpp_int  & l__val);
        cpp_int state_lkp_srch_req_not_rdy_enable() const;
    
}; // cap_ppa_csr_int_bndl1_int_enable_clear_t
    
class cap_ppa_csr_int_bndl0_intreg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_int_bndl0_intreg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_int_bndl0_intreg_t(string name = "cap_ppa_csr_int_bndl0_intreg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_int_bndl0_intreg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > state_lkp_srch_hit_addr_range_err_interrupt_cpp_int_t;
        cpp_int int_var__state_lkp_srch_hit_addr_range_err_interrupt;
        void state_lkp_srch_hit_addr_range_err_interrupt (const cpp_int  & l__val);
        cpp_int state_lkp_srch_hit_addr_range_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > state_lkp_srch_miss_interrupt_cpp_int_t;
        cpp_int int_var__state_lkp_srch_miss_interrupt;
        void state_lkp_srch_miss_interrupt (const cpp_int  & l__val);
        cpp_int state_lkp_srch_miss_interrupt() const;
    
        typedef pu_cpp_int< 1 > state_lkp_srch_req_not_rdy_interrupt_cpp_int_t;
        cpp_int int_var__state_lkp_srch_req_not_rdy_interrupt;
        void state_lkp_srch_req_not_rdy_interrupt (const cpp_int  & l__val);
        cpp_int state_lkp_srch_req_not_rdy_interrupt() const;
    
}; // cap_ppa_csr_int_bndl0_intreg_t
    
class cap_ppa_csr_int_bndl0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_int_bndl0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_int_bndl0_t(string name = "cap_ppa_csr_int_bndl0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_int_bndl0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_ppa_csr_int_bndl0_intreg_t intreg;
    
        cap_ppa_csr_int_bndl0_intreg_t int_test_set;
    
        cap_ppa_csr_int_bndl1_int_enable_clear_t int_enable_set;
    
        cap_ppa_csr_int_bndl1_int_enable_clear_t int_enable_clear;
    
}; // cap_ppa_csr_int_bndl0_t
    
class cap_ppa_csr_intreg_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_intreg_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_intreg_status_t(string name = "cap_ppa_csr_intreg_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_intreg_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > int_bndl0_interrupt_cpp_int_t;
        cpp_int int_var__int_bndl0_interrupt;
        void int_bndl0_interrupt (const cpp_int  & l__val);
        cpp_int int_bndl0_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_bndl1_interrupt_cpp_int_t;
        cpp_int int_var__int_bndl1_interrupt;
        void int_bndl1_interrupt (const cpp_int  & l__val);
        cpp_int int_bndl1_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_ecc_interrupt_cpp_int_t;
        cpp_int int_var__int_ecc_interrupt;
        void int_ecc_interrupt (const cpp_int  & l__val);
        cpp_int int_ecc_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_fifo1_interrupt_cpp_int_t;
        cpp_int int_var__int_fifo1_interrupt;
        void int_fifo1_interrupt (const cpp_int  & l__val);
        cpp_int int_fifo1_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_fifo2_interrupt_cpp_int_t;
        cpp_int int_var__int_fifo2_interrupt;
        void int_fifo2_interrupt (const cpp_int  & l__val);
        cpp_int int_fifo2_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_pe0_interrupt_cpp_int_t;
        cpp_int int_var__int_pe0_interrupt;
        void int_pe0_interrupt (const cpp_int  & l__val);
        cpp_int int_pe0_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_pe1_interrupt_cpp_int_t;
        cpp_int int_var__int_pe1_interrupt;
        void int_pe1_interrupt (const cpp_int  & l__val);
        cpp_int int_pe1_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_pe2_interrupt_cpp_int_t;
        cpp_int int_var__int_pe2_interrupt;
        void int_pe2_interrupt (const cpp_int  & l__val);
        cpp_int int_pe2_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_pe3_interrupt_cpp_int_t;
        cpp_int int_var__int_pe3_interrupt;
        void int_pe3_interrupt (const cpp_int  & l__val);
        cpp_int int_pe3_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_pe4_interrupt_cpp_int_t;
        cpp_int int_var__int_pe4_interrupt;
        void int_pe4_interrupt (const cpp_int  & l__val);
        cpp_int int_pe4_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_pe5_interrupt_cpp_int_t;
        cpp_int int_var__int_pe5_interrupt;
        void int_pe5_interrupt (const cpp_int  & l__val);
        cpp_int int_pe5_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_pe6_interrupt_cpp_int_t;
        cpp_int int_var__int_pe6_interrupt;
        void int_pe6_interrupt (const cpp_int  & l__val);
        cpp_int int_pe6_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_pe7_interrupt_cpp_int_t;
        cpp_int int_var__int_pe7_interrupt;
        void int_pe7_interrupt (const cpp_int  & l__val);
        cpp_int int_pe7_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_pe8_interrupt_cpp_int_t;
        cpp_int int_var__int_pe8_interrupt;
        void int_pe8_interrupt (const cpp_int  & l__val);
        cpp_int int_pe8_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_pe9_interrupt_cpp_int_t;
        cpp_int int_var__int_pe9_interrupt;
        void int_pe9_interrupt (const cpp_int  & l__val);
        cpp_int int_pe9_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_pa_interrupt_cpp_int_t;
        cpp_int int_var__int_pa_interrupt;
        void int_pa_interrupt (const cpp_int  & l__val);
        cpp_int int_pa_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_sw_phv_mem_interrupt_cpp_int_t;
        cpp_int int_var__int_sw_phv_mem_interrupt;
        void int_sw_phv_mem_interrupt (const cpp_int  & l__val);
        cpp_int int_sw_phv_mem_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_intf_interrupt_cpp_int_t;
        cpp_int int_var__int_intf_interrupt;
        void int_intf_interrupt (const cpp_int  & l__val);
        cpp_int int_intf_interrupt() const;
    
}; // cap_ppa_csr_intreg_status_t
    
class cap_ppa_csr_int_groups_int_enable_rw_reg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_int_groups_int_enable_rw_reg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_int_groups_int_enable_rw_reg_t(string name = "cap_ppa_csr_int_groups_int_enable_rw_reg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_int_groups_int_enable_rw_reg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > int_bndl0_enable_cpp_int_t;
        cpp_int int_var__int_bndl0_enable;
        void int_bndl0_enable (const cpp_int  & l__val);
        cpp_int int_bndl0_enable() const;
    
        typedef pu_cpp_int< 1 > int_bndl1_enable_cpp_int_t;
        cpp_int int_var__int_bndl1_enable;
        void int_bndl1_enable (const cpp_int  & l__val);
        cpp_int int_bndl1_enable() const;
    
        typedef pu_cpp_int< 1 > int_ecc_enable_cpp_int_t;
        cpp_int int_var__int_ecc_enable;
        void int_ecc_enable (const cpp_int  & l__val);
        cpp_int int_ecc_enable() const;
    
        typedef pu_cpp_int< 1 > int_fifo1_enable_cpp_int_t;
        cpp_int int_var__int_fifo1_enable;
        void int_fifo1_enable (const cpp_int  & l__val);
        cpp_int int_fifo1_enable() const;
    
        typedef pu_cpp_int< 1 > int_fifo2_enable_cpp_int_t;
        cpp_int int_var__int_fifo2_enable;
        void int_fifo2_enable (const cpp_int  & l__val);
        cpp_int int_fifo2_enable() const;
    
        typedef pu_cpp_int< 1 > int_pe0_enable_cpp_int_t;
        cpp_int int_var__int_pe0_enable;
        void int_pe0_enable (const cpp_int  & l__val);
        cpp_int int_pe0_enable() const;
    
        typedef pu_cpp_int< 1 > int_pe1_enable_cpp_int_t;
        cpp_int int_var__int_pe1_enable;
        void int_pe1_enable (const cpp_int  & l__val);
        cpp_int int_pe1_enable() const;
    
        typedef pu_cpp_int< 1 > int_pe2_enable_cpp_int_t;
        cpp_int int_var__int_pe2_enable;
        void int_pe2_enable (const cpp_int  & l__val);
        cpp_int int_pe2_enable() const;
    
        typedef pu_cpp_int< 1 > int_pe3_enable_cpp_int_t;
        cpp_int int_var__int_pe3_enable;
        void int_pe3_enable (const cpp_int  & l__val);
        cpp_int int_pe3_enable() const;
    
        typedef pu_cpp_int< 1 > int_pe4_enable_cpp_int_t;
        cpp_int int_var__int_pe4_enable;
        void int_pe4_enable (const cpp_int  & l__val);
        cpp_int int_pe4_enable() const;
    
        typedef pu_cpp_int< 1 > int_pe5_enable_cpp_int_t;
        cpp_int int_var__int_pe5_enable;
        void int_pe5_enable (const cpp_int  & l__val);
        cpp_int int_pe5_enable() const;
    
        typedef pu_cpp_int< 1 > int_pe6_enable_cpp_int_t;
        cpp_int int_var__int_pe6_enable;
        void int_pe6_enable (const cpp_int  & l__val);
        cpp_int int_pe6_enable() const;
    
        typedef pu_cpp_int< 1 > int_pe7_enable_cpp_int_t;
        cpp_int int_var__int_pe7_enable;
        void int_pe7_enable (const cpp_int  & l__val);
        cpp_int int_pe7_enable() const;
    
        typedef pu_cpp_int< 1 > int_pe8_enable_cpp_int_t;
        cpp_int int_var__int_pe8_enable;
        void int_pe8_enable (const cpp_int  & l__val);
        cpp_int int_pe8_enable() const;
    
        typedef pu_cpp_int< 1 > int_pe9_enable_cpp_int_t;
        cpp_int int_var__int_pe9_enable;
        void int_pe9_enable (const cpp_int  & l__val);
        cpp_int int_pe9_enable() const;
    
        typedef pu_cpp_int< 1 > int_pa_enable_cpp_int_t;
        cpp_int int_var__int_pa_enable;
        void int_pa_enable (const cpp_int  & l__val);
        cpp_int int_pa_enable() const;
    
        typedef pu_cpp_int< 1 > int_sw_phv_mem_enable_cpp_int_t;
        cpp_int int_var__int_sw_phv_mem_enable;
        void int_sw_phv_mem_enable (const cpp_int  & l__val);
        cpp_int int_sw_phv_mem_enable() const;
    
        typedef pu_cpp_int< 1 > int_intf_enable_cpp_int_t;
        cpp_int int_var__int_intf_enable;
        void int_intf_enable (const cpp_int  & l__val);
        cpp_int int_intf_enable() const;
    
}; // cap_ppa_csr_int_groups_int_enable_rw_reg_t
    
class cap_ppa_csr_intgrp_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_intgrp_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_intgrp_status_t(string name = "cap_ppa_csr_intgrp_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_intgrp_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_ppa_csr_intreg_status_t intreg;
    
        cap_ppa_csr_int_groups_int_enable_rw_reg_t int_enable_rw_reg;
    
        cap_ppa_csr_intreg_status_t int_rw_reg;
    
}; // cap_ppa_csr_intgrp_status_t
    
class cap_ppa_csr_dhs_dbg_mem_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_dhs_dbg_mem_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_dhs_dbg_mem_entry_t(string name = "cap_ppa_csr_dhs_dbg_mem_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_dhs_dbg_mem_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 546 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // cap_ppa_csr_dhs_dbg_mem_entry_t
    
class cap_ppa_csr_dhs_dbg_mem_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_dhs_dbg_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_dhs_dbg_mem_t(string name = "cap_ppa_csr_dhs_dbg_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_dhs_dbg_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        #if 256 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_csr_dhs_dbg_mem_entry_t, 256> entry;
        #else 
        cap_ppa_csr_dhs_dbg_mem_entry_t entry[256];
        #endif
        int get_depth_entry() { return 256; }
    
}; // cap_ppa_csr_dhs_dbg_mem_t
    
class cap_ppa_csr_dhs_sw_phv_mem_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_dhs_sw_phv_mem_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_dhs_sw_phv_mem_entry_t(string name = "cap_ppa_csr_dhs_sw_phv_mem_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_dhs_sw_phv_mem_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_dhs_sw_phv_mem_entry_t
    
class cap_ppa_csr_dhs_sw_phv_mem_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_dhs_sw_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_dhs_sw_phv_mem_t(string name = "cap_ppa_csr_dhs_sw_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_dhs_sw_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
        cap_csr_large_array_wrapper<cap_ppa_csr_dhs_sw_phv_mem_entry_t, 16> entry;
        #else 
        cap_ppa_csr_dhs_sw_phv_mem_entry_t entry[16];
        #endif
        int get_depth_entry() { return 16; }
    
}; // cap_ppa_csr_dhs_sw_phv_mem_t
    
class cap_ppa_csr_dhs_bndl1_state_lkp_sram_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_dhs_bndl1_state_lkp_sram_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_dhs_bndl1_state_lkp_sram_entry_t(string name = "cap_ppa_csr_dhs_bndl1_state_lkp_sram_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_dhs_bndl1_state_lkp_sram_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 763 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 1 > rsvd_cpp_int_t;
        cpp_int int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
        typedef pu_cpp_int< 54 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_ppa_csr_dhs_bndl1_state_lkp_sram_entry_t
    
class cap_ppa_csr_dhs_bndl1_state_lkp_sram_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_dhs_bndl1_state_lkp_sram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_dhs_bndl1_state_lkp_sram_t(string name = "cap_ppa_csr_dhs_bndl1_state_lkp_sram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_dhs_bndl1_state_lkp_sram_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        #if 288 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_csr_dhs_bndl1_state_lkp_sram_entry_t, 288> entry;
        #else 
        cap_ppa_csr_dhs_bndl1_state_lkp_sram_entry_t entry[288];
        #endif
        int get_depth_entry() { return 288; }
    
}; // cap_ppa_csr_dhs_bndl1_state_lkp_sram_t
    
class cap_ppa_csr_dhs_bndl1_state_lkp_tcam_ind_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_dhs_bndl1_state_lkp_tcam_ind_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_dhs_bndl1_state_lkp_tcam_ind_entry_t(string name = "cap_ppa_csr_dhs_bndl1_state_lkp_tcam_ind_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_dhs_bndl1_state_lkp_tcam_ind_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_dhs_bndl1_state_lkp_tcam_ind_entry_t
    
class cap_ppa_csr_dhs_bndl1_state_lkp_tcam_ind_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_dhs_bndl1_state_lkp_tcam_ind_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_dhs_bndl1_state_lkp_tcam_ind_t(string name = "cap_ppa_csr_dhs_bndl1_state_lkp_tcam_ind_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_dhs_bndl1_state_lkp_tcam_ind_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_ppa_csr_dhs_bndl1_state_lkp_tcam_ind_entry_t entry;
    
}; // cap_ppa_csr_dhs_bndl1_state_lkp_tcam_ind_t
    
class cap_ppa_csr_dhs_bndl1_state_lkp_tcam_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_dhs_bndl1_state_lkp_tcam_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_dhs_bndl1_state_lkp_tcam_entry_t(string name = "cap_ppa_csr_dhs_bndl1_state_lkp_tcam_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_dhs_bndl1_state_lkp_tcam_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_dhs_bndl1_state_lkp_tcam_entry_t
    
class cap_ppa_csr_dhs_bndl1_state_lkp_tcam_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_dhs_bndl1_state_lkp_tcam_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_dhs_bndl1_state_lkp_tcam_t(string name = "cap_ppa_csr_dhs_bndl1_state_lkp_tcam_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_dhs_bndl1_state_lkp_tcam_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        #if 288 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_csr_dhs_bndl1_state_lkp_tcam_entry_t, 288> entry;
        #else 
        cap_ppa_csr_dhs_bndl1_state_lkp_tcam_entry_t entry[288];
        #endif
        int get_depth_entry() { return 288; }
    
}; // cap_ppa_csr_dhs_bndl1_state_lkp_tcam_t
    
class cap_ppa_csr_dhs_bndl0_state_lkp_sram_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_dhs_bndl0_state_lkp_sram_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_dhs_bndl0_state_lkp_sram_entry_t(string name = "cap_ppa_csr_dhs_bndl0_state_lkp_sram_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_dhs_bndl0_state_lkp_sram_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 763 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 1 > rsvd_cpp_int_t;
        cpp_int int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
        typedef pu_cpp_int< 54 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_ppa_csr_dhs_bndl0_state_lkp_sram_entry_t
    
class cap_ppa_csr_dhs_bndl0_state_lkp_sram_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_dhs_bndl0_state_lkp_sram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_dhs_bndl0_state_lkp_sram_t(string name = "cap_ppa_csr_dhs_bndl0_state_lkp_sram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_dhs_bndl0_state_lkp_sram_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        #if 288 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_csr_dhs_bndl0_state_lkp_sram_entry_t, 288> entry;
        #else 
        cap_ppa_csr_dhs_bndl0_state_lkp_sram_entry_t entry[288];
        #endif
        int get_depth_entry() { return 288; }
    
}; // cap_ppa_csr_dhs_bndl0_state_lkp_sram_t
    
class cap_ppa_csr_dhs_bndl0_state_lkp_tcam_ind_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_dhs_bndl0_state_lkp_tcam_ind_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_dhs_bndl0_state_lkp_tcam_ind_entry_t(string name = "cap_ppa_csr_dhs_bndl0_state_lkp_tcam_ind_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_dhs_bndl0_state_lkp_tcam_ind_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_dhs_bndl0_state_lkp_tcam_ind_entry_t
    
class cap_ppa_csr_dhs_bndl0_state_lkp_tcam_ind_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_dhs_bndl0_state_lkp_tcam_ind_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_dhs_bndl0_state_lkp_tcam_ind_t(string name = "cap_ppa_csr_dhs_bndl0_state_lkp_tcam_ind_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_dhs_bndl0_state_lkp_tcam_ind_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_ppa_csr_dhs_bndl0_state_lkp_tcam_ind_entry_t entry;
    
}; // cap_ppa_csr_dhs_bndl0_state_lkp_tcam_ind_t
    
class cap_ppa_csr_dhs_bndl0_state_lkp_tcam_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_dhs_bndl0_state_lkp_tcam_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_dhs_bndl0_state_lkp_tcam_entry_t(string name = "cap_ppa_csr_dhs_bndl0_state_lkp_tcam_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_dhs_bndl0_state_lkp_tcam_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_dhs_bndl0_state_lkp_tcam_entry_t
    
class cap_ppa_csr_dhs_bndl0_state_lkp_tcam_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_dhs_bndl0_state_lkp_tcam_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_dhs_bndl0_state_lkp_tcam_t(string name = "cap_ppa_csr_dhs_bndl0_state_lkp_tcam_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_dhs_bndl0_state_lkp_tcam_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        #if 288 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_csr_dhs_bndl0_state_lkp_tcam_entry_t, 288> entry;
        #else 
        cap_ppa_csr_dhs_bndl0_state_lkp_tcam_entry_t entry[288];
        #endif
        int get_depth_entry() { return 288; }
    
}; // cap_ppa_csr_dhs_bndl0_state_lkp_tcam_t
    
class cap_ppa_csr_CNT_preparse_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_preparse_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_preparse_t(string name = "cap_ppa_csr_CNT_preparse_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_preparse_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > dff_sop_cpp_int_t;
        cpp_int int_var__dff_sop;
        void dff_sop (const cpp_int  & l__val);
        cpp_int dff_sop() const;
    
        typedef pu_cpp_int< 40 > dff_eop_cpp_int_t;
        cpp_int int_var__dff_eop;
        void dff_eop (const cpp_int  & l__val);
        cpp_int dff_eop() const;
    
        typedef pu_cpp_int< 40 > eff_cpp_int_t;
        cpp_int int_var__eff;
        void eff (const cpp_int  & l__val);
        cpp_int eff() const;
    
}; // cap_ppa_csr_CNT_preparse_t
    
class cap_ppa_csr_sta_preparse_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_preparse_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_preparse_t(string name = "cap_ppa_csr_sta_preparse_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_preparse_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 1 > dq_state_cpp_int_t;
        cpp_int int_var__dq_state;
        void dq_state (const cpp_int  & l__val);
        cpp_int dq_state() const;
    
        typedef pu_cpp_int< 1 > dff_empty_cpp_int_t;
        cpp_int int_var__dff_empty;
        void dff_empty (const cpp_int  & l__val);
        cpp_int dff_empty() const;
    
        typedef pu_cpp_int< 1 > dff_full_cpp_int_t;
        cpp_int int_var__dff_full;
        void dff_full (const cpp_int  & l__val);
        cpp_int dff_full() const;
    
        typedef pu_cpp_int< 1 > eff_empty_cpp_int_t;
        cpp_int int_var__eff_empty;
        void eff_empty (const cpp_int  & l__val);
        cpp_int eff_empty() const;
    
        typedef pu_cpp_int< 1 > eff_full_cpp_int_t;
        cpp_int int_var__eff_full;
        void eff_full (const cpp_int  & l__val);
        cpp_int eff_full() const;
    
}; // cap_ppa_csr_sta_preparse_t
    
class cap_ppa_csr_cfg_preparse_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_preparse_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_preparse_t(string name = "cap_ppa_csr_cfg_preparse_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_preparse_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 15 > spare_cpp_int_t;
        cpp_int int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
        typedef pu_cpp_int< 1 > bypass_cpp_int_t;
        cpp_int int_var__bypass;
        void bypass (const cpp_int  & l__val);
        cpp_int bypass() const;
    
        typedef pu_cpp_int< 16 > tm_iport_enc_en_cpp_int_t;
        cpp_int int_var__tm_iport_enc_en;
        void tm_iport_enc_en (const cpp_int  & l__val);
        cpp_int tm_iport_enc_en() const;
    
        typedef pu_cpp_int< 16 > udp_dstport_roce_val0_cpp_int_t;
        cpp_int int_var__udp_dstport_roce_val0;
        void udp_dstport_roce_val0 (const cpp_int  & l__val);
        cpp_int udp_dstport_roce_val0() const;
    
        typedef pu_cpp_int< 16 > udp_dstport_roce_val1_cpp_int_t;
        cpp_int int_var__udp_dstport_roce_val1;
        void udp_dstport_roce_val1 (const cpp_int  & l__val);
        cpp_int udp_dstport_roce_val1() const;
    
        typedef pu_cpp_int< 16 > udp_dstport_vxlan_val0_cpp_int_t;
        cpp_int int_var__udp_dstport_vxlan_val0;
        void udp_dstport_vxlan_val0 (const cpp_int  & l__val);
        cpp_int udp_dstport_vxlan_val0() const;
    
        typedef pu_cpp_int< 16 > udp_dstport_vxlan_val1_cpp_int_t;
        cpp_int int_var__udp_dstport_vxlan_val1;
        void udp_dstport_vxlan_val1 (const cpp_int  & l__val);
        cpp_int udp_dstport_vxlan_val1() const;
    
        typedef pu_cpp_int< 8 > vxlan_flag_mask_cpp_int_t;
        cpp_int int_var__vxlan_flag_mask;
        void vxlan_flag_mask (const cpp_int  & l__val);
        cpp_int vxlan_flag_mask() const;
    
        typedef pu_cpp_int< 8 > vxlan_flag_val_cpp_int_t;
        cpp_int int_var__vxlan_flag_val;
        void vxlan_flag_val (const cpp_int  & l__val);
        cpp_int vxlan_flag_val() const;
    
}; // cap_ppa_csr_cfg_preparse_t
    
class cap_ppa_csr_cfg_spare_dbg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_spare_dbg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_spare_dbg_t(string name = "cap_ppa_csr_cfg_spare_dbg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_spare_dbg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_cfg_spare_dbg_t
    
class cap_ppa_csr_cfg_spare_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_spare_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_spare_t(string name = "cap_ppa_csr_cfg_spare_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_spare_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 128 > fld0_inst_cpp_int_t;
        cpp_int int_var__fld0_inst;
        void fld0_inst (const cpp_int  & l__val);
        cpp_int fld0_inst() const;
    
        typedef pu_cpp_int< 128 > fld1_inst_cpp_int_t;
        cpp_int int_var__fld1_inst;
        void fld1_inst (const cpp_int  & l__val);
        cpp_int fld1_inst() const;
    
        typedef pu_cpp_int< 128 > fld2_inst_cpp_int_t;
        cpp_int int_var__fld2_inst;
        void fld2_inst (const cpp_int  & l__val);
        cpp_int fld2_inst() const;
    
        typedef pu_cpp_int< 128 > fld3_inst_cpp_int_t;
        cpp_int int_var__fld3_inst;
        void fld3_inst (const cpp_int  & l__val);
        cpp_int fld3_inst() const;
    
}; // cap_ppa_csr_cfg_spare_t
    
class cap_ppa_csr_cfg_debug_port_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_debug_port_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_debug_port_t(string name = "cap_ppa_csr_cfg_debug_port_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_debug_port_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_cfg_debug_port_t
    
class cap_ppa_csr_cfg_debug_ctrl_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_debug_ctrl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_debug_ctrl_t(string name = "cap_ppa_csr_cfg_debug_ctrl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_debug_ctrl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 4 > dbg_bndl_sel_cpp_int_t;
        cpp_int int_var__dbg_bndl_sel;
        void dbg_bndl_sel (const cpp_int  & l__val);
        cpp_int dbg_bndl_sel() const;
    
        typedef pu_cpp_int< 4 > dbg_bus_sel_cpp_int_t;
        cpp_int int_var__dbg_bus_sel;
        void dbg_bus_sel (const cpp_int  & l__val);
        cpp_int dbg_bus_sel() const;
    
        typedef pu_cpp_int< 1 > eccbypass_cpp_int_t;
        cpp_int int_var__eccbypass;
        void eccbypass (const cpp_int  & l__val);
        cpp_int eccbypass() const;
    
}; // cap_ppa_csr_cfg_debug_ctrl_t
    
class cap_ppa_csr_sta_pe_alloc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_pe_alloc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_pe_alloc_t(string name = "cap_ppa_csr_sta_pe_alloc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_pe_alloc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_sta_pe_alloc_t
    
class cap_ppa_csr_sta_bndl1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_bndl1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_bndl1_t(string name = "cap_ppa_csr_sta_bndl1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_bndl1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 100 > pkt_queue_cpp_int_t;
        cpp_int int_var__pkt_queue;
        void pkt_queue (const cpp_int  & l__val);
        cpp_int pkt_queue() const;
    
}; // cap_ppa_csr_sta_bndl1_t
    
class cap_ppa_csr_sta_bndl0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_bndl0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_bndl0_t(string name = "cap_ppa_csr_sta_bndl0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_bndl0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 100 > pkt_queue_cpp_int_t;
        cpp_int int_var__pkt_queue;
        void pkt_queue (const cpp_int  & l__val);
        cpp_int pkt_queue() const;
    
}; // cap_ppa_csr_sta_bndl0_t
    
class cap_ppa_csr_sta_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_fifo_t(string name = "cap_ppa_csr_sta_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_fifo_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > dp_if_ff_full_cpp_int_t;
        cpp_int int_var__dp_if_ff_full;
        void dp_if_ff_full (const cpp_int  & l__val);
        cpp_int dp_if_ff_full() const;
    
        typedef pu_cpp_int< 1 > dp_if_ff_empty_cpp_int_t;
        cpp_int int_var__dp_if_ff_empty;
        void dp_if_ff_empty (const cpp_int  & l__val);
        cpp_int dp_if_ff_empty() const;
    
        typedef pu_cpp_int< 1 > ohi_if_ff_full_cpp_int_t;
        cpp_int int_var__ohi_if_ff_full;
        void ohi_if_ff_full (const cpp_int  & l__val);
        cpp_int ohi_if_ff_full() const;
    
        typedef pu_cpp_int< 1 > ohi_if_ff_empty_cpp_int_t;
        cpp_int int_var__ohi_if_ff_empty;
        void ohi_if_ff_empty (const cpp_int  & l__val);
        cpp_int ohi_if_ff_empty() const;
    
        typedef pu_cpp_int< 1 > ma_if_ff_full_cpp_int_t;
        cpp_int int_var__ma_if_ff_full;
        void ma_if_ff_full (const cpp_int  & l__val);
        cpp_int ma_if_ff_full() const;
    
        typedef pu_cpp_int< 1 > ma_if_ff_empty_cpp_int_t;
        cpp_int int_var__ma_if_ff_empty;
        void ma_if_ff_empty (const cpp_int  & l__val);
        cpp_int ma_if_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pb_if_ff_full_cpp_int_t;
        cpp_int int_var__pb_if_ff_full;
        void pb_if_ff_full (const cpp_int  & l__val);
        cpp_int pb_if_ff_full() const;
    
        typedef pu_cpp_int< 1 > pb_if_ff_empty_cpp_int_t;
        cpp_int int_var__pb_if_ff_empty;
        void pb_if_ff_empty (const cpp_int  & l__val);
        cpp_int pb_if_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pkt_mtu_ff_full_cpp_int_t;
        cpp_int int_var__pkt_mtu_ff_full;
        void pkt_mtu_ff_full (const cpp_int  & l__val);
        cpp_int pkt_mtu_ff_full() const;
    
        typedef pu_cpp_int< 1 > pkt_mtu_ff_empty_cpp_int_t;
        cpp_int int_var__pkt_mtu_ff_empty;
        void pkt_mtu_ff_empty (const cpp_int  & l__val);
        cpp_int pkt_mtu_ff_empty() const;
    
        typedef pu_cpp_int< 1 > outphv_ff_full_cpp_int_t;
        cpp_int int_var__outphv_ff_full;
        void outphv_ff_full (const cpp_int  & l__val);
        cpp_int outphv_ff_full() const;
    
        typedef pu_cpp_int< 1 > outphv_ff_empty_cpp_int_t;
        cpp_int int_var__outphv_ff_empty;
        void outphv_ff_empty (const cpp_int  & l__val);
        cpp_int outphv_ff_empty() const;
    
        typedef pu_cpp_int< 1 > outohi_ff_full_cpp_int_t;
        cpp_int int_var__outohi_ff_full;
        void outohi_ff_full (const cpp_int  & l__val);
        cpp_int outohi_ff_full() const;
    
        typedef pu_cpp_int< 1 > outohi_ff_empty_cpp_int_t;
        cpp_int int_var__outohi_ff_empty;
        void outohi_ff_empty (const cpp_int  & l__val);
        cpp_int outohi_ff_empty() const;
    
        typedef pu_cpp_int< 1 > chk_ff_full_cpp_int_t;
        cpp_int int_var__chk_ff_full;
        void chk_ff_full (const cpp_int  & l__val);
        cpp_int chk_ff_full() const;
    
        typedef pu_cpp_int< 1 > chk_ff_empty_cpp_int_t;
        cpp_int int_var__chk_ff_empty;
        void chk_ff_empty (const cpp_int  & l__val);
        cpp_int chk_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pe0_qctl_ff_full_cpp_int_t;
        cpp_int int_var__pe0_qctl_ff_full;
        void pe0_qctl_ff_full (const cpp_int  & l__val);
        cpp_int pe0_qctl_ff_full() const;
    
        typedef pu_cpp_int< 1 > pe0_qctl_ff_empty_cpp_int_t;
        cpp_int int_var__pe0_qctl_ff_empty;
        void pe0_qctl_ff_empty (const cpp_int  & l__val);
        cpp_int pe0_qctl_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pe0_phv_ff_full_cpp_int_t;
        cpp_int int_var__pe0_phv_ff_full;
        void pe0_phv_ff_full (const cpp_int  & l__val);
        cpp_int pe0_phv_ff_full() const;
    
        typedef pu_cpp_int< 1 > pe0_phv_ff_empty_cpp_int_t;
        cpp_int int_var__pe0_phv_ff_empty;
        void pe0_phv_ff_empty (const cpp_int  & l__val);
        cpp_int pe0_phv_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pe0_ohi_ff_full_cpp_int_t;
        cpp_int int_var__pe0_ohi_ff_full;
        void pe0_ohi_ff_full (const cpp_int  & l__val);
        cpp_int pe0_ohi_ff_full() const;
    
        typedef pu_cpp_int< 1 > pe0_ohi_ff_empty_cpp_int_t;
        cpp_int int_var__pe0_ohi_ff_empty;
        void pe0_ohi_ff_empty (const cpp_int  & l__val);
        cpp_int pe0_ohi_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pe1_qctl_ff_full_cpp_int_t;
        cpp_int int_var__pe1_qctl_ff_full;
        void pe1_qctl_ff_full (const cpp_int  & l__val);
        cpp_int pe1_qctl_ff_full() const;
    
        typedef pu_cpp_int< 1 > pe1_qctl_ff_empty_cpp_int_t;
        cpp_int int_var__pe1_qctl_ff_empty;
        void pe1_qctl_ff_empty (const cpp_int  & l__val);
        cpp_int pe1_qctl_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pe1_phv_ff_full_cpp_int_t;
        cpp_int int_var__pe1_phv_ff_full;
        void pe1_phv_ff_full (const cpp_int  & l__val);
        cpp_int pe1_phv_ff_full() const;
    
        typedef pu_cpp_int< 1 > pe1_phv_ff_empty_cpp_int_t;
        cpp_int int_var__pe1_phv_ff_empty;
        void pe1_phv_ff_empty (const cpp_int  & l__val);
        cpp_int pe1_phv_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pe1_ohi_ff_full_cpp_int_t;
        cpp_int int_var__pe1_ohi_ff_full;
        void pe1_ohi_ff_full (const cpp_int  & l__val);
        cpp_int pe1_ohi_ff_full() const;
    
        typedef pu_cpp_int< 1 > pe1_ohi_ff_empty_cpp_int_t;
        cpp_int int_var__pe1_ohi_ff_empty;
        void pe1_ohi_ff_empty (const cpp_int  & l__val);
        cpp_int pe1_ohi_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pe2_qctl_ff_full_cpp_int_t;
        cpp_int int_var__pe2_qctl_ff_full;
        void pe2_qctl_ff_full (const cpp_int  & l__val);
        cpp_int pe2_qctl_ff_full() const;
    
        typedef pu_cpp_int< 1 > pe2_qctl_ff_empty_cpp_int_t;
        cpp_int int_var__pe2_qctl_ff_empty;
        void pe2_qctl_ff_empty (const cpp_int  & l__val);
        cpp_int pe2_qctl_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pe2_phv_ff_full_cpp_int_t;
        cpp_int int_var__pe2_phv_ff_full;
        void pe2_phv_ff_full (const cpp_int  & l__val);
        cpp_int pe2_phv_ff_full() const;
    
        typedef pu_cpp_int< 1 > pe2_phv_ff_empty_cpp_int_t;
        cpp_int int_var__pe2_phv_ff_empty;
        void pe2_phv_ff_empty (const cpp_int  & l__val);
        cpp_int pe2_phv_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pe2_ohi_ff_full_cpp_int_t;
        cpp_int int_var__pe2_ohi_ff_full;
        void pe2_ohi_ff_full (const cpp_int  & l__val);
        cpp_int pe2_ohi_ff_full() const;
    
        typedef pu_cpp_int< 1 > pe2_ohi_ff_empty_cpp_int_t;
        cpp_int int_var__pe2_ohi_ff_empty;
        void pe2_ohi_ff_empty (const cpp_int  & l__val);
        cpp_int pe2_ohi_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pe3_qctl_ff_full_cpp_int_t;
        cpp_int int_var__pe3_qctl_ff_full;
        void pe3_qctl_ff_full (const cpp_int  & l__val);
        cpp_int pe3_qctl_ff_full() const;
    
        typedef pu_cpp_int< 1 > pe3_qctl_ff_empty_cpp_int_t;
        cpp_int int_var__pe3_qctl_ff_empty;
        void pe3_qctl_ff_empty (const cpp_int  & l__val);
        cpp_int pe3_qctl_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pe3_phv_ff_full_cpp_int_t;
        cpp_int int_var__pe3_phv_ff_full;
        void pe3_phv_ff_full (const cpp_int  & l__val);
        cpp_int pe3_phv_ff_full() const;
    
        typedef pu_cpp_int< 1 > pe3_phv_ff_empty_cpp_int_t;
        cpp_int int_var__pe3_phv_ff_empty;
        void pe3_phv_ff_empty (const cpp_int  & l__val);
        cpp_int pe3_phv_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pe3_ohi_ff_full_cpp_int_t;
        cpp_int int_var__pe3_ohi_ff_full;
        void pe3_ohi_ff_full (const cpp_int  & l__val);
        cpp_int pe3_ohi_ff_full() const;
    
        typedef pu_cpp_int< 1 > pe3_ohi_ff_empty_cpp_int_t;
        cpp_int int_var__pe3_ohi_ff_empty;
        void pe3_ohi_ff_empty (const cpp_int  & l__val);
        cpp_int pe3_ohi_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pe4_qctl_ff_full_cpp_int_t;
        cpp_int int_var__pe4_qctl_ff_full;
        void pe4_qctl_ff_full (const cpp_int  & l__val);
        cpp_int pe4_qctl_ff_full() const;
    
        typedef pu_cpp_int< 1 > pe4_qctl_ff_empty_cpp_int_t;
        cpp_int int_var__pe4_qctl_ff_empty;
        void pe4_qctl_ff_empty (const cpp_int  & l__val);
        cpp_int pe4_qctl_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pe4_phv_ff_full_cpp_int_t;
        cpp_int int_var__pe4_phv_ff_full;
        void pe4_phv_ff_full (const cpp_int  & l__val);
        cpp_int pe4_phv_ff_full() const;
    
        typedef pu_cpp_int< 1 > pe4_phv_ff_empty_cpp_int_t;
        cpp_int int_var__pe4_phv_ff_empty;
        void pe4_phv_ff_empty (const cpp_int  & l__val);
        cpp_int pe4_phv_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pe4_ohi_ff_full_cpp_int_t;
        cpp_int int_var__pe4_ohi_ff_full;
        void pe4_ohi_ff_full (const cpp_int  & l__val);
        cpp_int pe4_ohi_ff_full() const;
    
        typedef pu_cpp_int< 1 > pe4_ohi_ff_empty_cpp_int_t;
        cpp_int int_var__pe4_ohi_ff_empty;
        void pe4_ohi_ff_empty (const cpp_int  & l__val);
        cpp_int pe4_ohi_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pe5_qctl_ff_full_cpp_int_t;
        cpp_int int_var__pe5_qctl_ff_full;
        void pe5_qctl_ff_full (const cpp_int  & l__val);
        cpp_int pe5_qctl_ff_full() const;
    
        typedef pu_cpp_int< 1 > pe5_qctl_ff_empty_cpp_int_t;
        cpp_int int_var__pe5_qctl_ff_empty;
        void pe5_qctl_ff_empty (const cpp_int  & l__val);
        cpp_int pe5_qctl_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pe5_phv_ff_full_cpp_int_t;
        cpp_int int_var__pe5_phv_ff_full;
        void pe5_phv_ff_full (const cpp_int  & l__val);
        cpp_int pe5_phv_ff_full() const;
    
        typedef pu_cpp_int< 1 > pe5_phv_ff_empty_cpp_int_t;
        cpp_int int_var__pe5_phv_ff_empty;
        void pe5_phv_ff_empty (const cpp_int  & l__val);
        cpp_int pe5_phv_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pe5_ohi_ff_full_cpp_int_t;
        cpp_int int_var__pe5_ohi_ff_full;
        void pe5_ohi_ff_full (const cpp_int  & l__val);
        cpp_int pe5_ohi_ff_full() const;
    
        typedef pu_cpp_int< 1 > pe5_ohi_ff_empty_cpp_int_t;
        cpp_int int_var__pe5_ohi_ff_empty;
        void pe5_ohi_ff_empty (const cpp_int  & l__val);
        cpp_int pe5_ohi_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pe6_qctl_ff_full_cpp_int_t;
        cpp_int int_var__pe6_qctl_ff_full;
        void pe6_qctl_ff_full (const cpp_int  & l__val);
        cpp_int pe6_qctl_ff_full() const;
    
        typedef pu_cpp_int< 1 > pe6_qctl_ff_empty_cpp_int_t;
        cpp_int int_var__pe6_qctl_ff_empty;
        void pe6_qctl_ff_empty (const cpp_int  & l__val);
        cpp_int pe6_qctl_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pe6_phv_ff_full_cpp_int_t;
        cpp_int int_var__pe6_phv_ff_full;
        void pe6_phv_ff_full (const cpp_int  & l__val);
        cpp_int pe6_phv_ff_full() const;
    
        typedef pu_cpp_int< 1 > pe6_phv_ff_empty_cpp_int_t;
        cpp_int int_var__pe6_phv_ff_empty;
        void pe6_phv_ff_empty (const cpp_int  & l__val);
        cpp_int pe6_phv_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pe6_ohi_ff_full_cpp_int_t;
        cpp_int int_var__pe6_ohi_ff_full;
        void pe6_ohi_ff_full (const cpp_int  & l__val);
        cpp_int pe6_ohi_ff_full() const;
    
        typedef pu_cpp_int< 1 > pe6_ohi_ff_empty_cpp_int_t;
        cpp_int int_var__pe6_ohi_ff_empty;
        void pe6_ohi_ff_empty (const cpp_int  & l__val);
        cpp_int pe6_ohi_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pe7_qctl_ff_full_cpp_int_t;
        cpp_int int_var__pe7_qctl_ff_full;
        void pe7_qctl_ff_full (const cpp_int  & l__val);
        cpp_int pe7_qctl_ff_full() const;
    
        typedef pu_cpp_int< 1 > pe7_qctl_ff_empty_cpp_int_t;
        cpp_int int_var__pe7_qctl_ff_empty;
        void pe7_qctl_ff_empty (const cpp_int  & l__val);
        cpp_int pe7_qctl_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pe7_phv_ff_full_cpp_int_t;
        cpp_int int_var__pe7_phv_ff_full;
        void pe7_phv_ff_full (const cpp_int  & l__val);
        cpp_int pe7_phv_ff_full() const;
    
        typedef pu_cpp_int< 1 > pe7_phv_ff_empty_cpp_int_t;
        cpp_int int_var__pe7_phv_ff_empty;
        void pe7_phv_ff_empty (const cpp_int  & l__val);
        cpp_int pe7_phv_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pe7_ohi_ff_full_cpp_int_t;
        cpp_int int_var__pe7_ohi_ff_full;
        void pe7_ohi_ff_full (const cpp_int  & l__val);
        cpp_int pe7_ohi_ff_full() const;
    
        typedef pu_cpp_int< 1 > pe7_ohi_ff_empty_cpp_int_t;
        cpp_int int_var__pe7_ohi_ff_empty;
        void pe7_ohi_ff_empty (const cpp_int  & l__val);
        cpp_int pe7_ohi_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pe8_qctl_ff_full_cpp_int_t;
        cpp_int int_var__pe8_qctl_ff_full;
        void pe8_qctl_ff_full (const cpp_int  & l__val);
        cpp_int pe8_qctl_ff_full() const;
    
        typedef pu_cpp_int< 1 > pe8_qctl_ff_empty_cpp_int_t;
        cpp_int int_var__pe8_qctl_ff_empty;
        void pe8_qctl_ff_empty (const cpp_int  & l__val);
        cpp_int pe8_qctl_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pe8_phv_ff_full_cpp_int_t;
        cpp_int int_var__pe8_phv_ff_full;
        void pe8_phv_ff_full (const cpp_int  & l__val);
        cpp_int pe8_phv_ff_full() const;
    
        typedef pu_cpp_int< 1 > pe8_phv_ff_empty_cpp_int_t;
        cpp_int int_var__pe8_phv_ff_empty;
        void pe8_phv_ff_empty (const cpp_int  & l__val);
        cpp_int pe8_phv_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pe8_ohi_ff_full_cpp_int_t;
        cpp_int int_var__pe8_ohi_ff_full;
        void pe8_ohi_ff_full (const cpp_int  & l__val);
        cpp_int pe8_ohi_ff_full() const;
    
        typedef pu_cpp_int< 1 > pe8_ohi_ff_empty_cpp_int_t;
        cpp_int int_var__pe8_ohi_ff_empty;
        void pe8_ohi_ff_empty (const cpp_int  & l__val);
        cpp_int pe8_ohi_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pe9_qctl_ff_full_cpp_int_t;
        cpp_int int_var__pe9_qctl_ff_full;
        void pe9_qctl_ff_full (const cpp_int  & l__val);
        cpp_int pe9_qctl_ff_full() const;
    
        typedef pu_cpp_int< 1 > pe9_qctl_ff_empty_cpp_int_t;
        cpp_int int_var__pe9_qctl_ff_empty;
        void pe9_qctl_ff_empty (const cpp_int  & l__val);
        cpp_int pe9_qctl_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pe9_phv_ff_full_cpp_int_t;
        cpp_int int_var__pe9_phv_ff_full;
        void pe9_phv_ff_full (const cpp_int  & l__val);
        cpp_int pe9_phv_ff_full() const;
    
        typedef pu_cpp_int< 1 > pe9_phv_ff_empty_cpp_int_t;
        cpp_int int_var__pe9_phv_ff_empty;
        void pe9_phv_ff_empty (const cpp_int  & l__val);
        cpp_int pe9_phv_ff_empty() const;
    
        typedef pu_cpp_int< 1 > pe9_ohi_ff_full_cpp_int_t;
        cpp_int int_var__pe9_ohi_ff_full;
        void pe9_ohi_ff_full (const cpp_int  & l__val);
        cpp_int pe9_ohi_ff_full() const;
    
        typedef pu_cpp_int< 1 > pe9_ohi_ff_empty_cpp_int_t;
        cpp_int int_var__pe9_ohi_ff_empty;
        void pe9_ohi_ff_empty (const cpp_int  & l__val);
        cpp_int pe9_ohi_ff_empty() const;
    
}; // cap_ppa_csr_sta_fifo_t
    
class cap_ppa_csr_sta_bndl1_state_lkp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_bndl1_state_lkp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_bndl1_state_lkp_t(string name = "cap_ppa_csr_sta_bndl1_state_lkp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_bndl1_state_lkp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 13 > err_log_cpp_int_t;
        cpp_int int_var__err_log;
        void err_log (const cpp_int  & l__val);
        cpp_int err_log() const;
    
}; // cap_ppa_csr_sta_bndl1_state_lkp_t
    
class cap_ppa_csr_sta_bndl0_state_lkp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_bndl0_state_lkp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_bndl0_state_lkp_t(string name = "cap_ppa_csr_sta_bndl0_state_lkp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_bndl0_state_lkp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 13 > err_log_cpp_int_t;
        cpp_int int_var__err_log;
        void err_log (const cpp_int  & l__val);
        cpp_int err_log() const;
    
}; // cap_ppa_csr_sta_bndl0_state_lkp_t
    
class cap_ppa_csr_sta_pe9_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_pe9_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_pe9_t(string name = "cap_ppa_csr_sta_pe9_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_pe9_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > err_vec_cpp_int_t;
        cpp_int int_var__err_vec;
        void err_vec (const cpp_int  & l__val);
        cpp_int err_vec() const;
    
        typedef pu_cpp_int< 105 > err_log_cpp_int_t;
        cpp_int int_var__err_log;
        void err_log (const cpp_int  & l__val);
        cpp_int err_log() const;
    
        typedef pu_cpp_int< 4 > pe_state_cpp_int_t;
        cpp_int int_var__pe_state;
        void pe_state (const cpp_int  & l__val);
        cpp_int pe_state() const;
    
        typedef pu_cpp_int< 2 > phv_state_cpp_int_t;
        cpp_int int_var__phv_state;
        void phv_state (const cpp_int  & l__val);
        cpp_int phv_state() const;
    
        typedef pu_cpp_int< 1 > pkt_state_cpp_int_t;
        cpp_int int_var__pkt_state;
        void pkt_state (const cpp_int  & l__val);
        cpp_int pkt_state() const;
    
}; // cap_ppa_csr_sta_pe9_t
    
class cap_ppa_csr_sta_pe8_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_pe8_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_pe8_t(string name = "cap_ppa_csr_sta_pe8_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_pe8_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > err_vec_cpp_int_t;
        cpp_int int_var__err_vec;
        void err_vec (const cpp_int  & l__val);
        cpp_int err_vec() const;
    
        typedef pu_cpp_int< 105 > err_log_cpp_int_t;
        cpp_int int_var__err_log;
        void err_log (const cpp_int  & l__val);
        cpp_int err_log() const;
    
        typedef pu_cpp_int< 4 > pe_state_cpp_int_t;
        cpp_int int_var__pe_state;
        void pe_state (const cpp_int  & l__val);
        cpp_int pe_state() const;
    
        typedef pu_cpp_int< 2 > phv_state_cpp_int_t;
        cpp_int int_var__phv_state;
        void phv_state (const cpp_int  & l__val);
        cpp_int phv_state() const;
    
        typedef pu_cpp_int< 1 > pkt_state_cpp_int_t;
        cpp_int int_var__pkt_state;
        void pkt_state (const cpp_int  & l__val);
        cpp_int pkt_state() const;
    
}; // cap_ppa_csr_sta_pe8_t
    
class cap_ppa_csr_sta_pe7_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_pe7_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_pe7_t(string name = "cap_ppa_csr_sta_pe7_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_pe7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > err_vec_cpp_int_t;
        cpp_int int_var__err_vec;
        void err_vec (const cpp_int  & l__val);
        cpp_int err_vec() const;
    
        typedef pu_cpp_int< 105 > err_log_cpp_int_t;
        cpp_int int_var__err_log;
        void err_log (const cpp_int  & l__val);
        cpp_int err_log() const;
    
        typedef pu_cpp_int< 4 > pe_state_cpp_int_t;
        cpp_int int_var__pe_state;
        void pe_state (const cpp_int  & l__val);
        cpp_int pe_state() const;
    
        typedef pu_cpp_int< 2 > phv_state_cpp_int_t;
        cpp_int int_var__phv_state;
        void phv_state (const cpp_int  & l__val);
        cpp_int phv_state() const;
    
        typedef pu_cpp_int< 1 > pkt_state_cpp_int_t;
        cpp_int int_var__pkt_state;
        void pkt_state (const cpp_int  & l__val);
        cpp_int pkt_state() const;
    
}; // cap_ppa_csr_sta_pe7_t
    
class cap_ppa_csr_sta_pe6_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_pe6_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_pe6_t(string name = "cap_ppa_csr_sta_pe6_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_pe6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > err_vec_cpp_int_t;
        cpp_int int_var__err_vec;
        void err_vec (const cpp_int  & l__val);
        cpp_int err_vec() const;
    
        typedef pu_cpp_int< 105 > err_log_cpp_int_t;
        cpp_int int_var__err_log;
        void err_log (const cpp_int  & l__val);
        cpp_int err_log() const;
    
        typedef pu_cpp_int< 4 > pe_state_cpp_int_t;
        cpp_int int_var__pe_state;
        void pe_state (const cpp_int  & l__val);
        cpp_int pe_state() const;
    
        typedef pu_cpp_int< 2 > phv_state_cpp_int_t;
        cpp_int int_var__phv_state;
        void phv_state (const cpp_int  & l__val);
        cpp_int phv_state() const;
    
        typedef pu_cpp_int< 1 > pkt_state_cpp_int_t;
        cpp_int int_var__pkt_state;
        void pkt_state (const cpp_int  & l__val);
        cpp_int pkt_state() const;
    
}; // cap_ppa_csr_sta_pe6_t
    
class cap_ppa_csr_sta_pe5_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_pe5_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_pe5_t(string name = "cap_ppa_csr_sta_pe5_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_pe5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > err_vec_cpp_int_t;
        cpp_int int_var__err_vec;
        void err_vec (const cpp_int  & l__val);
        cpp_int err_vec() const;
    
        typedef pu_cpp_int< 105 > err_log_cpp_int_t;
        cpp_int int_var__err_log;
        void err_log (const cpp_int  & l__val);
        cpp_int err_log() const;
    
        typedef pu_cpp_int< 4 > pe_state_cpp_int_t;
        cpp_int int_var__pe_state;
        void pe_state (const cpp_int  & l__val);
        cpp_int pe_state() const;
    
        typedef pu_cpp_int< 2 > phv_state_cpp_int_t;
        cpp_int int_var__phv_state;
        void phv_state (const cpp_int  & l__val);
        cpp_int phv_state() const;
    
        typedef pu_cpp_int< 1 > pkt_state_cpp_int_t;
        cpp_int int_var__pkt_state;
        void pkt_state (const cpp_int  & l__val);
        cpp_int pkt_state() const;
    
}; // cap_ppa_csr_sta_pe5_t
    
class cap_ppa_csr_sta_pe4_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_pe4_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_pe4_t(string name = "cap_ppa_csr_sta_pe4_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_pe4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > err_vec_cpp_int_t;
        cpp_int int_var__err_vec;
        void err_vec (const cpp_int  & l__val);
        cpp_int err_vec() const;
    
        typedef pu_cpp_int< 105 > err_log_cpp_int_t;
        cpp_int int_var__err_log;
        void err_log (const cpp_int  & l__val);
        cpp_int err_log() const;
    
        typedef pu_cpp_int< 4 > pe_state_cpp_int_t;
        cpp_int int_var__pe_state;
        void pe_state (const cpp_int  & l__val);
        cpp_int pe_state() const;
    
        typedef pu_cpp_int< 2 > phv_state_cpp_int_t;
        cpp_int int_var__phv_state;
        void phv_state (const cpp_int  & l__val);
        cpp_int phv_state() const;
    
        typedef pu_cpp_int< 1 > pkt_state_cpp_int_t;
        cpp_int int_var__pkt_state;
        void pkt_state (const cpp_int  & l__val);
        cpp_int pkt_state() const;
    
}; // cap_ppa_csr_sta_pe4_t
    
class cap_ppa_csr_sta_pe3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_pe3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_pe3_t(string name = "cap_ppa_csr_sta_pe3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_pe3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > err_vec_cpp_int_t;
        cpp_int int_var__err_vec;
        void err_vec (const cpp_int  & l__val);
        cpp_int err_vec() const;
    
        typedef pu_cpp_int< 105 > err_log_cpp_int_t;
        cpp_int int_var__err_log;
        void err_log (const cpp_int  & l__val);
        cpp_int err_log() const;
    
        typedef pu_cpp_int< 4 > pe_state_cpp_int_t;
        cpp_int int_var__pe_state;
        void pe_state (const cpp_int  & l__val);
        cpp_int pe_state() const;
    
        typedef pu_cpp_int< 2 > phv_state_cpp_int_t;
        cpp_int int_var__phv_state;
        void phv_state (const cpp_int  & l__val);
        cpp_int phv_state() const;
    
        typedef pu_cpp_int< 1 > pkt_state_cpp_int_t;
        cpp_int int_var__pkt_state;
        void pkt_state (const cpp_int  & l__val);
        cpp_int pkt_state() const;
    
}; // cap_ppa_csr_sta_pe3_t
    
class cap_ppa_csr_sta_pe2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_pe2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_pe2_t(string name = "cap_ppa_csr_sta_pe2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_pe2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > err_vec_cpp_int_t;
        cpp_int int_var__err_vec;
        void err_vec (const cpp_int  & l__val);
        cpp_int err_vec() const;
    
        typedef pu_cpp_int< 105 > err_log_cpp_int_t;
        cpp_int int_var__err_log;
        void err_log (const cpp_int  & l__val);
        cpp_int err_log() const;
    
        typedef pu_cpp_int< 4 > pe_state_cpp_int_t;
        cpp_int int_var__pe_state;
        void pe_state (const cpp_int  & l__val);
        cpp_int pe_state() const;
    
        typedef pu_cpp_int< 2 > phv_state_cpp_int_t;
        cpp_int int_var__phv_state;
        void phv_state (const cpp_int  & l__val);
        cpp_int phv_state() const;
    
        typedef pu_cpp_int< 1 > pkt_state_cpp_int_t;
        cpp_int int_var__pkt_state;
        void pkt_state (const cpp_int  & l__val);
        cpp_int pkt_state() const;
    
}; // cap_ppa_csr_sta_pe2_t
    
class cap_ppa_csr_sta_pe1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_pe1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_pe1_t(string name = "cap_ppa_csr_sta_pe1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_pe1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > err_vec_cpp_int_t;
        cpp_int int_var__err_vec;
        void err_vec (const cpp_int  & l__val);
        cpp_int err_vec() const;
    
        typedef pu_cpp_int< 105 > err_log_cpp_int_t;
        cpp_int int_var__err_log;
        void err_log (const cpp_int  & l__val);
        cpp_int err_log() const;
    
        typedef pu_cpp_int< 4 > pe_state_cpp_int_t;
        cpp_int int_var__pe_state;
        void pe_state (const cpp_int  & l__val);
        cpp_int pe_state() const;
    
        typedef pu_cpp_int< 2 > phv_state_cpp_int_t;
        cpp_int int_var__phv_state;
        void phv_state (const cpp_int  & l__val);
        cpp_int phv_state() const;
    
        typedef pu_cpp_int< 1 > pkt_state_cpp_int_t;
        cpp_int int_var__pkt_state;
        void pkt_state (const cpp_int  & l__val);
        cpp_int pkt_state() const;
    
}; // cap_ppa_csr_sta_pe1_t
    
class cap_ppa_csr_sta_pe0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_pe0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_pe0_t(string name = "cap_ppa_csr_sta_pe0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_pe0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > err_vec_cpp_int_t;
        cpp_int int_var__err_vec;
        void err_vec (const cpp_int  & l__val);
        cpp_int err_vec() const;
    
        typedef pu_cpp_int< 105 > err_log_cpp_int_t;
        cpp_int int_var__err_log;
        void err_log (const cpp_int  & l__val);
        cpp_int err_log() const;
    
        typedef pu_cpp_int< 4 > pe_state_cpp_int_t;
        cpp_int int_var__pe_state;
        void pe_state (const cpp_int  & l__val);
        cpp_int pe_state() const;
    
        typedef pu_cpp_int< 2 > phv_state_cpp_int_t;
        cpp_int int_var__phv_state;
        void phv_state (const cpp_int  & l__val);
        cpp_int phv_state() const;
    
        typedef pu_cpp_int< 1 > pkt_state_cpp_int_t;
        cpp_int int_var__pkt_state;
        void pkt_state (const cpp_int  & l__val);
        cpp_int pkt_state() const;
    
}; // cap_ppa_csr_sta_pe0_t
    
class cap_ppa_csr_sta_chkr_seq_id_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_chkr_seq_id_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_chkr_seq_id_t(string name = "cap_ppa_csr_sta_chkr_seq_id_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_chkr_seq_id_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 7 > ohi_cpp_int_t;
        cpp_int int_var__ohi;
        void ohi (const cpp_int  & l__val);
        cpp_int ohi() const;
    
        typedef pu_cpp_int< 7 > pkt_cpp_int_t;
        cpp_int int_var__pkt;
        void pkt (const cpp_int  & l__val);
        cpp_int pkt() const;
    
}; // cap_ppa_csr_sta_chkr_seq_id_t
    
class cap_ppa_csr_sta_csum4_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_csum4_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_csum4_t(string name = "cap_ppa_csr_sta_csum4_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_csum4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > err_vec_cpp_int_t;
        cpp_int int_var__err_vec;
        void err_vec (const cpp_int  & l__val);
        cpp_int err_vec() const;
    
}; // cap_ppa_csr_sta_csum4_t
    
class cap_ppa_csr_sta_csum3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_csum3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_csum3_t(string name = "cap_ppa_csr_sta_csum3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_csum3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > err_vec_cpp_int_t;
        cpp_int int_var__err_vec;
        void err_vec (const cpp_int  & l__val);
        cpp_int err_vec() const;
    
}; // cap_ppa_csr_sta_csum3_t
    
class cap_ppa_csr_sta_csum2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_csum2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_csum2_t(string name = "cap_ppa_csr_sta_csum2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_csum2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > err_vec_cpp_int_t;
        cpp_int int_var__err_vec;
        void err_vec (const cpp_int  & l__val);
        cpp_int err_vec() const;
    
}; // cap_ppa_csr_sta_csum2_t
    
class cap_ppa_csr_sta_csum1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_csum1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_csum1_t(string name = "cap_ppa_csr_sta_csum1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_csum1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > err_vec_cpp_int_t;
        cpp_int int_var__err_vec;
        void err_vec (const cpp_int  & l__val);
        cpp_int err_vec() const;
    
}; // cap_ppa_csr_sta_csum1_t
    
class cap_ppa_csr_sta_csum0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_csum0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_csum0_t(string name = "cap_ppa_csr_sta_csum0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_csum0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > err_vec_cpp_int_t;
        cpp_int int_var__err_vec;
        void err_vec (const cpp_int  & l__val);
        cpp_int err_vec() const;
    
}; // cap_ppa_csr_sta_csum0_t
    
class cap_ppa_csr_sta_reorder_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_reorder_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_reorder_t(string name = "cap_ppa_csr_sta_reorder_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_reorder_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 2 > seq_state_cpp_int_t;
        cpp_int int_var__seq_state;
        void seq_state (const cpp_int  & l__val);
        cpp_int seq_state() const;
    
        typedef pu_cpp_int< 7 > seq_num_cpp_int_t;
        cpp_int int_var__seq_num;
        void seq_num (const cpp_int  & l__val);
        cpp_int seq_num() const;
    
        typedef pu_cpp_int< 10 > ohi_srdy_vec_cpp_int_t;
        cpp_int int_var__ohi_srdy_vec;
        void ohi_srdy_vec (const cpp_int  & l__val);
        cpp_int ohi_srdy_vec() const;
    
        typedef pu_cpp_int< 10 > phv_srdy_vec_cpp_int_t;
        cpp_int int_var__phv_srdy_vec;
        void phv_srdy_vec (const cpp_int  & l__val);
        cpp_int phv_srdy_vec() const;
    
        typedef pu_cpp_int< 10 > match_vec_cpp_int_t;
        cpp_int int_var__match_vec;
        void match_vec (const cpp_int  & l__val);
        cpp_int match_vec() const;
    
        typedef pu_cpp_int< 10 > eop_vec_cpp_int_t;
        cpp_int int_var__eop_vec;
        void eop_vec (const cpp_int  & l__val);
        cpp_int eop_vec() const;
    
        typedef pu_cpp_int< 10 > drdy_vec_d_cpp_int_t;
        cpp_int int_var__drdy_vec_d;
        void drdy_vec_d (const cpp_int  & l__val);
        cpp_int drdy_vec_d() const;
    
}; // cap_ppa_csr_sta_reorder_t
    
class cap_ppa_csr_sta_reorder_phv_not_sop_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_reorder_phv_not_sop_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_reorder_phv_not_sop_err_t(string name = "cap_ppa_csr_sta_reorder_phv_not_sop_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_reorder_phv_not_sop_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > match_vec_cpp_int_t;
        cpp_int int_var__match_vec;
        void match_vec (const cpp_int  & l__val);
        cpp_int match_vec() const;
    
        typedef pu_cpp_int< 10 > phv_sop_vec_cpp_int_t;
        cpp_int int_var__phv_sop_vec;
        void phv_sop_vec (const cpp_int  & l__val);
        cpp_int phv_sop_vec() const;
    
}; // cap_ppa_csr_sta_reorder_phv_not_sop_err_t
    
class cap_ppa_csr_sta_sw_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_sw_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_sw_phv_mem_t(string name = "cap_ppa_csr_sta_sw_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_sw_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_sta_sw_phv_mem_t
    
class cap_ppa_csr_cfg_sw_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_sw_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_sw_phv_mem_t(string name = "cap_ppa_csr_cfg_sw_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_sw_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_cfg_sw_phv_mem_t
    
class cap_ppa_csr_sta_sw_phv_state_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_sw_phv_state_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_sw_phv_state_t(string name = "cap_ppa_csr_sta_sw_phv_state_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_sw_phv_state_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_sta_sw_phv_state_t
    
class cap_ppa_csr_cfg_sw_phv_config_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_sw_phv_config_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_sw_phv_config_t(string name = "cap_ppa_csr_cfg_sw_phv_config_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_sw_phv_config_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_cfg_sw_phv_config_t
    
class cap_ppa_csr_cfg_sw_phv_control_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_sw_phv_control_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_sw_phv_control_t(string name = "cap_ppa_csr_cfg_sw_phv_control_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_sw_phv_control_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_cfg_sw_phv_control_t
    
class cap_ppa_csr_cfg_sw_phv_global_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_sw_phv_global_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_sw_phv_global_t(string name = "cap_ppa_csr_cfg_sw_phv_global_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_sw_phv_global_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_cfg_sw_phv_global_t
    
class cap_ppa_csr_sta_pe9_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_pe9_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_pe9_phv_mem_t(string name = "cap_ppa_csr_sta_pe9_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_pe9_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_sta_pe9_phv_mem_t
    
class cap_ppa_csr_cfg_pe9_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_pe9_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_pe9_phv_mem_t(string name = "cap_ppa_csr_cfg_pe9_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_pe9_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_cfg_pe9_phv_mem_t
    
class cap_ppa_csr_sta_pe8_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_pe8_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_pe8_phv_mem_t(string name = "cap_ppa_csr_sta_pe8_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_pe8_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_sta_pe8_phv_mem_t
    
class cap_ppa_csr_cfg_pe8_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_pe8_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_pe8_phv_mem_t(string name = "cap_ppa_csr_cfg_pe8_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_pe8_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_cfg_pe8_phv_mem_t
    
class cap_ppa_csr_sta_pe7_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_pe7_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_pe7_phv_mem_t(string name = "cap_ppa_csr_sta_pe7_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_pe7_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_sta_pe7_phv_mem_t
    
class cap_ppa_csr_cfg_pe7_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_pe7_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_pe7_phv_mem_t(string name = "cap_ppa_csr_cfg_pe7_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_pe7_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_cfg_pe7_phv_mem_t
    
class cap_ppa_csr_sta_pe6_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_pe6_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_pe6_phv_mem_t(string name = "cap_ppa_csr_sta_pe6_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_pe6_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_sta_pe6_phv_mem_t
    
class cap_ppa_csr_cfg_pe6_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_pe6_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_pe6_phv_mem_t(string name = "cap_ppa_csr_cfg_pe6_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_pe6_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_cfg_pe6_phv_mem_t
    
class cap_ppa_csr_sta_pe5_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_pe5_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_pe5_phv_mem_t(string name = "cap_ppa_csr_sta_pe5_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_pe5_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_sta_pe5_phv_mem_t
    
class cap_ppa_csr_cfg_pe5_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_pe5_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_pe5_phv_mem_t(string name = "cap_ppa_csr_cfg_pe5_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_pe5_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_cfg_pe5_phv_mem_t
    
class cap_ppa_csr_sta_pe4_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_pe4_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_pe4_phv_mem_t(string name = "cap_ppa_csr_sta_pe4_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_pe4_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_sta_pe4_phv_mem_t
    
class cap_ppa_csr_cfg_pe4_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_pe4_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_pe4_phv_mem_t(string name = "cap_ppa_csr_cfg_pe4_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_pe4_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_cfg_pe4_phv_mem_t
    
class cap_ppa_csr_sta_pe3_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_pe3_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_pe3_phv_mem_t(string name = "cap_ppa_csr_sta_pe3_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_pe3_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_sta_pe3_phv_mem_t
    
class cap_ppa_csr_cfg_pe3_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_pe3_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_pe3_phv_mem_t(string name = "cap_ppa_csr_cfg_pe3_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_pe3_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_cfg_pe3_phv_mem_t
    
class cap_ppa_csr_sta_pe2_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_pe2_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_pe2_phv_mem_t(string name = "cap_ppa_csr_sta_pe2_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_pe2_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_sta_pe2_phv_mem_t
    
class cap_ppa_csr_cfg_pe2_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_pe2_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_pe2_phv_mem_t(string name = "cap_ppa_csr_cfg_pe2_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_pe2_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_cfg_pe2_phv_mem_t
    
class cap_ppa_csr_sta_pe1_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_pe1_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_pe1_phv_mem_t(string name = "cap_ppa_csr_sta_pe1_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_pe1_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_sta_pe1_phv_mem_t
    
class cap_ppa_csr_cfg_pe1_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_pe1_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_pe1_phv_mem_t(string name = "cap_ppa_csr_cfg_pe1_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_pe1_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_cfg_pe1_phv_mem_t
    
class cap_ppa_csr_sta_pe0_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_pe0_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_pe0_phv_mem_t(string name = "cap_ppa_csr_sta_pe0_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_pe0_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_sta_pe0_phv_mem_t
    
class cap_ppa_csr_cfg_pe0_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_pe0_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_pe0_phv_mem_t(string name = "cap_ppa_csr_cfg_pe0_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_pe0_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_cfg_pe0_phv_mem_t
    
class cap_ppa_csr_sta_bndl1_state_lkp_tcam_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_bndl1_state_lkp_tcam_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_bndl1_state_lkp_tcam_t(string name = "cap_ppa_csr_sta_bndl1_state_lkp_tcam_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_bndl1_state_lkp_tcam_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_sta_bndl1_state_lkp_tcam_t
    
class cap_ppa_csr_cfg_bndl1_state_lkp_tcam_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_bndl1_state_lkp_tcam_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_bndl1_state_lkp_tcam_t(string name = "cap_ppa_csr_cfg_bndl1_state_lkp_tcam_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_bndl1_state_lkp_tcam_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_cfg_bndl1_state_lkp_tcam_t
    
class cap_ppa_csr_sta_bndl0_state_lkp_tcam_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_bndl0_state_lkp_tcam_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_bndl0_state_lkp_tcam_t(string name = "cap_ppa_csr_sta_bndl0_state_lkp_tcam_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_bndl0_state_lkp_tcam_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_sta_bndl0_state_lkp_tcam_t
    
class cap_ppa_csr_cfg_bndl0_state_lkp_tcam_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_bndl0_state_lkp_tcam_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_bndl0_state_lkp_tcam_t(string name = "cap_ppa_csr_cfg_bndl0_state_lkp_tcam_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_bndl0_state_lkp_tcam_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_cfg_bndl0_state_lkp_tcam_t
    
class cap_ppa_csr_sta_outq_ohi_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_outq_ohi_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_outq_ohi_mem_t(string name = "cap_ppa_csr_sta_outq_ohi_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_outq_ohi_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_sta_outq_ohi_mem_t
    
class cap_ppa_csr_cfg_outq_ohi_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_outq_ohi_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_outq_ohi_mem_t(string name = "cap_ppa_csr_cfg_outq_ohi_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_outq_ohi_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_cfg_outq_ohi_mem_t
    
class cap_ppa_csr_sta_outq_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_outq_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_outq_phv_mem_t(string name = "cap_ppa_csr_sta_outq_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_outq_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_sta_outq_phv_mem_t
    
class cap_ppa_csr_cfg_outq_phv_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_outq_phv_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_outq_phv_mem_t(string name = "cap_ppa_csr_cfg_outq_phv_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_outq_phv_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_cfg_outq_phv_mem_t
    
class cap_ppa_csr_sta_bndl1_inq_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_bndl1_inq_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_bndl1_inq_mem_t(string name = "cap_ppa_csr_sta_bndl1_inq_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_bndl1_inq_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_sta_bndl1_inq_mem_t
    
class cap_ppa_csr_cfg_bndl1_inq_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_bndl1_inq_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_bndl1_inq_mem_t(string name = "cap_ppa_csr_cfg_bndl1_inq_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_bndl1_inq_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_cfg_bndl1_inq_mem_t
    
class cap_ppa_csr_sta_bndl0_inq_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_bndl0_inq_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_bndl0_inq_mem_t(string name = "cap_ppa_csr_sta_bndl0_inq_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_bndl0_inq_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_sta_bndl0_inq_mem_t
    
class cap_ppa_csr_cfg_bndl0_inq_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_bndl0_inq_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_bndl0_inq_mem_t(string name = "cap_ppa_csr_cfg_bndl0_inq_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_bndl0_inq_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_cfg_bndl0_inq_mem_t
    
class cap_ppa_csr_sta_bndl1_state_lkp_sram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_bndl1_state_lkp_sram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_bndl1_state_lkp_sram_t(string name = "cap_ppa_csr_sta_bndl1_state_lkp_sram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_bndl1_state_lkp_sram_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 54 > syndrome_cpp_int_t;
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
    
}; // cap_ppa_csr_sta_bndl1_state_lkp_sram_t
    
class cap_ppa_csr_cfg_bndl1_state_lkp_sram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_bndl1_state_lkp_sram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_bndl1_state_lkp_sram_t(string name = "cap_ppa_csr_cfg_bndl1_state_lkp_sram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_bndl1_state_lkp_sram_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 1 > dhs_ecc_bypass_cpp_int_t;
        cpp_int int_var__dhs_ecc_bypass;
        void dhs_ecc_bypass (const cpp_int  & l__val);
        cpp_int dhs_ecc_bypass() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_ppa_csr_cfg_bndl1_state_lkp_sram_t
    
class cap_ppa_csr_sta_bndl0_state_lkp_sram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_bndl0_state_lkp_sram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_bndl0_state_lkp_sram_t(string name = "cap_ppa_csr_sta_bndl0_state_lkp_sram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_bndl0_state_lkp_sram_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 54 > syndrome_cpp_int_t;
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
    
}; // cap_ppa_csr_sta_bndl0_state_lkp_sram_t
    
class cap_ppa_csr_cfg_bndl0_state_lkp_sram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_bndl0_state_lkp_sram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_bndl0_state_lkp_sram_t(string name = "cap_ppa_csr_cfg_bndl0_state_lkp_sram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_bndl0_state_lkp_sram_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 1 > dhs_ecc_bypass_cpp_int_t;
        cpp_int int_var__dhs_ecc_bypass;
        void dhs_ecc_bypass (const cpp_int  & l__val);
        cpp_int dhs_ecc_bypass() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_ppa_csr_cfg_bndl0_state_lkp_sram_t
    
class cap_ppa_csr_sta_pkt_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_pkt_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_pkt_mem_t(string name = "cap_ppa_csr_sta_pkt_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_pkt_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_sta_pkt_mem_t
    
class cap_ppa_csr_cfg_pkt_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_pkt_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_pkt_mem_t(string name = "cap_ppa_csr_cfg_pkt_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_pkt_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_cfg_pkt_mem_t
    
class cap_ppa_csr_csr_intr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_csr_intr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_csr_intr_t(string name = "cap_ppa_csr_csr_intr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_csr_intr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_csr_intr_t
    
class cap_ppa_csr_CNT_ppa_pe9_phv_out_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_pe9_phv_out_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_pe9_phv_out_t(string name = "cap_ppa_csr_CNT_ppa_pe9_phv_out_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_pe9_phv_out_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        cpp_int int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        cpp_int int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe9_phv_out_t
    
class cap_ppa_csr_CNT_ppa_pe9_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_pe9_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_pe9_t(string name = "cap_ppa_csr_CNT_ppa_pe9_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_pe9_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > pkt_release_cpp_int_t;
        cpp_int int_var__pkt_release;
        void pkt_release (const cpp_int  & l__val);
        cpp_int pkt_release() const;
    
        typedef pu_cpp_int< 32 > ohi_out_cpp_int_t;
        cpp_int int_var__ohi_out;
        void ohi_out (const cpp_int  & l__val);
        cpp_int ohi_out() const;
    
}; // cap_ppa_csr_CNT_ppa_pe9_t
    
class cap_ppa_csr_CNT_ppa_pe9_pkt_in_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_pe9_pkt_in_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_pe9_pkt_in_t(string name = "cap_ppa_csr_CNT_ppa_pe9_pkt_in_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_pe9_pkt_in_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        cpp_int int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        cpp_int int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe9_pkt_in_t
    
class cap_ppa_csr_CNT_ppa_pe8_phv_out_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_pe8_phv_out_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_pe8_phv_out_t(string name = "cap_ppa_csr_CNT_ppa_pe8_phv_out_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_pe8_phv_out_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        cpp_int int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        cpp_int int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe8_phv_out_t
    
class cap_ppa_csr_CNT_ppa_pe8_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_pe8_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_pe8_t(string name = "cap_ppa_csr_CNT_ppa_pe8_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_pe8_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > pkt_release_cpp_int_t;
        cpp_int int_var__pkt_release;
        void pkt_release (const cpp_int  & l__val);
        cpp_int pkt_release() const;
    
        typedef pu_cpp_int< 32 > ohi_out_cpp_int_t;
        cpp_int int_var__ohi_out;
        void ohi_out (const cpp_int  & l__val);
        cpp_int ohi_out() const;
    
}; // cap_ppa_csr_CNT_ppa_pe8_t
    
class cap_ppa_csr_CNT_ppa_pe8_pkt_in_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_pe8_pkt_in_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_pe8_pkt_in_t(string name = "cap_ppa_csr_CNT_ppa_pe8_pkt_in_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_pe8_pkt_in_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        cpp_int int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        cpp_int int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe8_pkt_in_t
    
class cap_ppa_csr_CNT_ppa_pe7_phv_out_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_pe7_phv_out_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_pe7_phv_out_t(string name = "cap_ppa_csr_CNT_ppa_pe7_phv_out_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_pe7_phv_out_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        cpp_int int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        cpp_int int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe7_phv_out_t
    
class cap_ppa_csr_CNT_ppa_pe7_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_pe7_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_pe7_t(string name = "cap_ppa_csr_CNT_ppa_pe7_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_pe7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > pkt_release_cpp_int_t;
        cpp_int int_var__pkt_release;
        void pkt_release (const cpp_int  & l__val);
        cpp_int pkt_release() const;
    
        typedef pu_cpp_int< 32 > ohi_out_cpp_int_t;
        cpp_int int_var__ohi_out;
        void ohi_out (const cpp_int  & l__val);
        cpp_int ohi_out() const;
    
}; // cap_ppa_csr_CNT_ppa_pe7_t
    
class cap_ppa_csr_CNT_ppa_pe7_pkt_in_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_pe7_pkt_in_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_pe7_pkt_in_t(string name = "cap_ppa_csr_CNT_ppa_pe7_pkt_in_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_pe7_pkt_in_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        cpp_int int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        cpp_int int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe7_pkt_in_t
    
class cap_ppa_csr_CNT_ppa_pe6_phv_out_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_pe6_phv_out_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_pe6_phv_out_t(string name = "cap_ppa_csr_CNT_ppa_pe6_phv_out_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_pe6_phv_out_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        cpp_int int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        cpp_int int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe6_phv_out_t
    
class cap_ppa_csr_CNT_ppa_pe6_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_pe6_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_pe6_t(string name = "cap_ppa_csr_CNT_ppa_pe6_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_pe6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > pkt_release_cpp_int_t;
        cpp_int int_var__pkt_release;
        void pkt_release (const cpp_int  & l__val);
        cpp_int pkt_release() const;
    
        typedef pu_cpp_int< 32 > ohi_out_cpp_int_t;
        cpp_int int_var__ohi_out;
        void ohi_out (const cpp_int  & l__val);
        cpp_int ohi_out() const;
    
}; // cap_ppa_csr_CNT_ppa_pe6_t
    
class cap_ppa_csr_CNT_ppa_pe6_pkt_in_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_pe6_pkt_in_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_pe6_pkt_in_t(string name = "cap_ppa_csr_CNT_ppa_pe6_pkt_in_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_pe6_pkt_in_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        cpp_int int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        cpp_int int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe6_pkt_in_t
    
class cap_ppa_csr_CNT_ppa_pe5_phv_out_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_pe5_phv_out_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_pe5_phv_out_t(string name = "cap_ppa_csr_CNT_ppa_pe5_phv_out_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_pe5_phv_out_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        cpp_int int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        cpp_int int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe5_phv_out_t
    
class cap_ppa_csr_CNT_ppa_pe5_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_pe5_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_pe5_t(string name = "cap_ppa_csr_CNT_ppa_pe5_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_pe5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > pkt_release_cpp_int_t;
        cpp_int int_var__pkt_release;
        void pkt_release (const cpp_int  & l__val);
        cpp_int pkt_release() const;
    
        typedef pu_cpp_int< 32 > ohi_out_cpp_int_t;
        cpp_int int_var__ohi_out;
        void ohi_out (const cpp_int  & l__val);
        cpp_int ohi_out() const;
    
}; // cap_ppa_csr_CNT_ppa_pe5_t
    
class cap_ppa_csr_CNT_ppa_pe5_pkt_in_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_pe5_pkt_in_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_pe5_pkt_in_t(string name = "cap_ppa_csr_CNT_ppa_pe5_pkt_in_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_pe5_pkt_in_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        cpp_int int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        cpp_int int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe5_pkt_in_t
    
class cap_ppa_csr_CNT_ppa_pe4_phv_out_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_pe4_phv_out_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_pe4_phv_out_t(string name = "cap_ppa_csr_CNT_ppa_pe4_phv_out_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_pe4_phv_out_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        cpp_int int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        cpp_int int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe4_phv_out_t
    
class cap_ppa_csr_CNT_ppa_pe4_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_pe4_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_pe4_t(string name = "cap_ppa_csr_CNT_ppa_pe4_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_pe4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > pkt_release_cpp_int_t;
        cpp_int int_var__pkt_release;
        void pkt_release (const cpp_int  & l__val);
        cpp_int pkt_release() const;
    
        typedef pu_cpp_int< 32 > ohi_out_cpp_int_t;
        cpp_int int_var__ohi_out;
        void ohi_out (const cpp_int  & l__val);
        cpp_int ohi_out() const;
    
}; // cap_ppa_csr_CNT_ppa_pe4_t
    
class cap_ppa_csr_CNT_ppa_pe4_pkt_in_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_pe4_pkt_in_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_pe4_pkt_in_t(string name = "cap_ppa_csr_CNT_ppa_pe4_pkt_in_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_pe4_pkt_in_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        cpp_int int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        cpp_int int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe4_pkt_in_t
    
class cap_ppa_csr_CNT_ppa_pe3_phv_out_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_pe3_phv_out_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_pe3_phv_out_t(string name = "cap_ppa_csr_CNT_ppa_pe3_phv_out_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_pe3_phv_out_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        cpp_int int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        cpp_int int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe3_phv_out_t
    
class cap_ppa_csr_CNT_ppa_pe3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_pe3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_pe3_t(string name = "cap_ppa_csr_CNT_ppa_pe3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_pe3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > pkt_release_cpp_int_t;
        cpp_int int_var__pkt_release;
        void pkt_release (const cpp_int  & l__val);
        cpp_int pkt_release() const;
    
        typedef pu_cpp_int< 32 > ohi_out_cpp_int_t;
        cpp_int int_var__ohi_out;
        void ohi_out (const cpp_int  & l__val);
        cpp_int ohi_out() const;
    
}; // cap_ppa_csr_CNT_ppa_pe3_t
    
class cap_ppa_csr_CNT_ppa_pe3_pkt_in_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_pe3_pkt_in_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_pe3_pkt_in_t(string name = "cap_ppa_csr_CNT_ppa_pe3_pkt_in_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_pe3_pkt_in_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        cpp_int int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        cpp_int int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe3_pkt_in_t
    
class cap_ppa_csr_CNT_ppa_pe2_phv_out_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_pe2_phv_out_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_pe2_phv_out_t(string name = "cap_ppa_csr_CNT_ppa_pe2_phv_out_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_pe2_phv_out_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        cpp_int int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        cpp_int int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe2_phv_out_t
    
class cap_ppa_csr_CNT_ppa_pe2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_pe2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_pe2_t(string name = "cap_ppa_csr_CNT_ppa_pe2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_pe2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > pkt_release_cpp_int_t;
        cpp_int int_var__pkt_release;
        void pkt_release (const cpp_int  & l__val);
        cpp_int pkt_release() const;
    
        typedef pu_cpp_int< 32 > ohi_out_cpp_int_t;
        cpp_int int_var__ohi_out;
        void ohi_out (const cpp_int  & l__val);
        cpp_int ohi_out() const;
    
}; // cap_ppa_csr_CNT_ppa_pe2_t
    
class cap_ppa_csr_CNT_ppa_pe2_pkt_in_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_pe2_pkt_in_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_pe2_pkt_in_t(string name = "cap_ppa_csr_CNT_ppa_pe2_pkt_in_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_pe2_pkt_in_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        cpp_int int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        cpp_int int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe2_pkt_in_t
    
class cap_ppa_csr_CNT_ppa_pe1_phv_out_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_pe1_phv_out_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_pe1_phv_out_t(string name = "cap_ppa_csr_CNT_ppa_pe1_phv_out_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_pe1_phv_out_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        cpp_int int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        cpp_int int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe1_phv_out_t
    
class cap_ppa_csr_CNT_ppa_pe1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_pe1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_pe1_t(string name = "cap_ppa_csr_CNT_ppa_pe1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_pe1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > pkt_release_cpp_int_t;
        cpp_int int_var__pkt_release;
        void pkt_release (const cpp_int  & l__val);
        cpp_int pkt_release() const;
    
        typedef pu_cpp_int< 32 > ohi_out_cpp_int_t;
        cpp_int int_var__ohi_out;
        void ohi_out (const cpp_int  & l__val);
        cpp_int ohi_out() const;
    
}; // cap_ppa_csr_CNT_ppa_pe1_t
    
class cap_ppa_csr_CNT_ppa_pe1_pkt_in_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_pe1_pkt_in_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_pe1_pkt_in_t(string name = "cap_ppa_csr_CNT_ppa_pe1_pkt_in_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_pe1_pkt_in_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        cpp_int int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        cpp_int int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe1_pkt_in_t
    
class cap_ppa_csr_CNT_ppa_pe0_phv_out_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_pe0_phv_out_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_pe0_phv_out_t(string name = "cap_ppa_csr_CNT_ppa_pe0_phv_out_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_pe0_phv_out_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        cpp_int int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        cpp_int int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe0_phv_out_t
    
class cap_ppa_csr_CNT_ppa_pe0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_pe0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_pe0_t(string name = "cap_ppa_csr_CNT_ppa_pe0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_pe0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > pkt_release_cpp_int_t;
        cpp_int int_var__pkt_release;
        void pkt_release (const cpp_int  & l__val);
        cpp_int pkt_release() const;
    
        typedef pu_cpp_int< 32 > ohi_out_cpp_int_t;
        cpp_int int_var__ohi_out;
        void ohi_out (const cpp_int  & l__val);
        cpp_int ohi_out() const;
    
}; // cap_ppa_csr_CNT_ppa_pe0_t
    
class cap_ppa_csr_CNT_ppa_pe0_pkt_in_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_pe0_pkt_in_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_pe0_pkt_in_t(string name = "cap_ppa_csr_CNT_ppa_pe0_pkt_in_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_pe0_pkt_in_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        cpp_int int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        cpp_int int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe0_pkt_in_t
    
class cap_ppa_csr_CNT_ppa_ohi_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_ohi_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_ohi_t(string name = "cap_ppa_csr_CNT_ppa_ohi_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_ohi_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 40 > eop_cpp_int_t;
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
    
}; // cap_ppa_csr_CNT_ppa_ohi_t
    
class cap_ppa_csr_CNT_ppa_sw_phv_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_sw_phv_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_sw_phv_t(string name = "cap_ppa_csr_CNT_ppa_sw_phv_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_sw_phv_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > sop_cpp_int_t;
        cpp_int int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 16 > eop_cpp_int_t;
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
    
}; // cap_ppa_csr_CNT_ppa_sw_phv_t
    
class cap_ppa_csr_CNT_ppa_ma_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_ma_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_ma_t(string name = "cap_ppa_csr_CNT_ppa_ma_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_ma_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 40 > eop_cpp_int_t;
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
    
}; // cap_ppa_csr_CNT_ppa_ma_t
    
class cap_ppa_csr_CNT_ppa_dp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_dp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_dp_t(string name = "cap_ppa_csr_CNT_ppa_dp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_dp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 40 > eop_cpp_int_t;
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
    
}; // cap_ppa_csr_CNT_ppa_dp_t
    
class cap_ppa_csr_CNT_ppa_pb_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_CNT_ppa_pb_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_CNT_ppa_pb_t(string name = "cap_ppa_csr_CNT_ppa_pb_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_CNT_ppa_pb_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 40 > eop_cpp_int_t;
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
    
}; // cap_ppa_csr_CNT_ppa_pb_t
    
class cap_ppa_csr_cfg_align_chk_profile_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_align_chk_profile_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_align_chk_profile_t(string name = "cap_ppa_csr_cfg_align_chk_profile_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_align_chk_profile_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > addsub_start_cpp_int_t;
        cpp_int int_var__addsub_start;
        void addsub_start (const cpp_int  & l__val);
        cpp_int addsub_start() const;
    
        typedef pu_cpp_int< 6 > start_adj_cpp_int_t;
        cpp_int int_var__start_adj;
        void start_adj (const cpp_int  & l__val);
        cpp_int start_adj() const;
    
        typedef pu_cpp_int< 8 > mod_log2_cpp_int_t;
        cpp_int int_var__mod_log2;
        void mod_log2 (const cpp_int  & l__val);
        cpp_int mod_log2() const;
    
}; // cap_ppa_csr_cfg_align_chk_profile_t
    
class cap_ppa_csr_cfg_len_chk_profile_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_len_chk_profile_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_len_chk_profile_t(string name = "cap_ppa_csr_cfg_len_chk_profile_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_len_chk_profile_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 14 > len_mask_cpp_int_t;
        cpp_int int_var__len_mask;
        void len_mask (const cpp_int  & l__val);
        cpp_int len_mask() const;
    
        typedef pu_cpp_int< 1 > len_shift_left_cpp_int_t;
        cpp_int int_var__len_shift_left;
        void len_shift_left (const cpp_int  & l__val);
        cpp_int len_shift_left() const;
    
        typedef pu_cpp_int< 3 > len_shift_val_cpp_int_t;
        cpp_int int_var__len_shift_val;
        void len_shift_val (const cpp_int  & l__val);
        cpp_int len_shift_val() const;
    
        typedef pu_cpp_int< 1 > addsub_start_cpp_int_t;
        cpp_int int_var__addsub_start;
        void addsub_start (const cpp_int  & l__val);
        cpp_int addsub_start() const;
    
        typedef pu_cpp_int< 6 > start_adj_cpp_int_t;
        cpp_int int_var__start_adj;
        void start_adj (const cpp_int  & l__val);
        cpp_int start_adj() const;
    
}; // cap_ppa_csr_cfg_len_chk_profile_t
    
class cap_ppa_csr_cfg_csum_phdr_profile_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_csum_phdr_profile_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_csum_phdr_profile_t(string name = "cap_ppa_csr_cfg_csum_phdr_profile_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_csum_phdr_profile_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > fld0_fld_en_cpp_int_t;
        cpp_int int_var__fld0_fld_en;
        void fld0_fld_en (const cpp_int  & l__val);
        cpp_int fld0_fld_en() const;
    
        typedef pu_cpp_int< 1 > fld0_fld_align_cpp_int_t;
        cpp_int int_var__fld0_fld_align;
        void fld0_fld_align (const cpp_int  & l__val);
        cpp_int fld0_fld_align() const;
    
        typedef pu_cpp_int< 6 > fld0_fld_start_cpp_int_t;
        cpp_int int_var__fld0_fld_start;
        void fld0_fld_start (const cpp_int  & l__val);
        cpp_int fld0_fld_start() const;
    
        typedef pu_cpp_int< 6 > fld0_fld_end_cpp_int_t;
        cpp_int int_var__fld0_fld_end;
        void fld0_fld_end (const cpp_int  & l__val);
        cpp_int fld0_fld_end() const;
    
        typedef pu_cpp_int< 1 > fld0_add_len_cpp_int_t;
        cpp_int int_var__fld0_add_len;
        void fld0_add_len (const cpp_int  & l__val);
        cpp_int fld0_add_len() const;
    
        typedef pu_cpp_int< 1 > fld1_fld_en_cpp_int_t;
        cpp_int int_var__fld1_fld_en;
        void fld1_fld_en (const cpp_int  & l__val);
        cpp_int fld1_fld_en() const;
    
        typedef pu_cpp_int< 1 > fld1_fld_align_cpp_int_t;
        cpp_int int_var__fld1_fld_align;
        void fld1_fld_align (const cpp_int  & l__val);
        cpp_int fld1_fld_align() const;
    
        typedef pu_cpp_int< 6 > fld1_fld_start_cpp_int_t;
        cpp_int int_var__fld1_fld_start;
        void fld1_fld_start (const cpp_int  & l__val);
        cpp_int fld1_fld_start() const;
    
        typedef pu_cpp_int< 6 > fld1_fld_end_cpp_int_t;
        cpp_int int_var__fld1_fld_end;
        void fld1_fld_end (const cpp_int  & l__val);
        cpp_int fld1_fld_end() const;
    
        typedef pu_cpp_int< 1 > fld1_add_len_cpp_int_t;
        cpp_int int_var__fld1_add_len;
        void fld1_add_len (const cpp_int  & l__val);
        cpp_int fld1_add_len() const;
    
        typedef pu_cpp_int< 1 > fld2_fld_en_cpp_int_t;
        cpp_int int_var__fld2_fld_en;
        void fld2_fld_en (const cpp_int  & l__val);
        cpp_int fld2_fld_en() const;
    
        typedef pu_cpp_int< 1 > fld2_fld_align_cpp_int_t;
        cpp_int int_var__fld2_fld_align;
        void fld2_fld_align (const cpp_int  & l__val);
        cpp_int fld2_fld_align() const;
    
        typedef pu_cpp_int< 6 > fld2_fld_start_cpp_int_t;
        cpp_int int_var__fld2_fld_start;
        void fld2_fld_start (const cpp_int  & l__val);
        cpp_int fld2_fld_start() const;
    
        typedef pu_cpp_int< 6 > fld2_fld_end_cpp_int_t;
        cpp_int int_var__fld2_fld_end;
        void fld2_fld_end (const cpp_int  & l__val);
        cpp_int fld2_fld_end() const;
    
        typedef pu_cpp_int< 1 > fld2_add_len_cpp_int_t;
        cpp_int int_var__fld2_add_len;
        void fld2_add_len (const cpp_int  & l__val);
        cpp_int fld2_add_len() const;
    
        typedef pu_cpp_int< 1 > fld3_fld_en_cpp_int_t;
        cpp_int int_var__fld3_fld_en;
        void fld3_fld_en (const cpp_int  & l__val);
        cpp_int fld3_fld_en() const;
    
        typedef pu_cpp_int< 1 > fld3_fld_align_cpp_int_t;
        cpp_int int_var__fld3_fld_align;
        void fld3_fld_align (const cpp_int  & l__val);
        cpp_int fld3_fld_align() const;
    
        typedef pu_cpp_int< 6 > fld3_fld_start_cpp_int_t;
        cpp_int int_var__fld3_fld_start;
        void fld3_fld_start (const cpp_int  & l__val);
        cpp_int fld3_fld_start() const;
    
        typedef pu_cpp_int< 6 > fld3_fld_end_cpp_int_t;
        cpp_int int_var__fld3_fld_end;
        void fld3_fld_end (const cpp_int  & l__val);
        cpp_int fld3_fld_end() const;
    
        typedef pu_cpp_int< 1 > fld3_add_len_cpp_int_t;
        cpp_int int_var__fld3_add_len;
        void fld3_add_len (const cpp_int  & l__val);
        cpp_int fld3_add_len() const;
    
}; // cap_ppa_csr_cfg_csum_phdr_profile_t
    
class cap_ppa_csr_cfg_csum_profile_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_csum_profile_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_csum_profile_t(string name = "cap_ppa_csr_cfg_csum_profile_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_csum_profile_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > csum_loc_use_phdr_ohi_cpp_int_t;
        cpp_int int_var__csum_loc_use_phdr_ohi;
        void csum_loc_use_phdr_ohi (const cpp_int  & l__val);
        cpp_int csum_loc_use_phdr_ohi() const;
    
        typedef pu_cpp_int< 1 > csum_8b_cpp_int_t;
        cpp_int int_var__csum_8b;
        void csum_8b (const cpp_int  & l__val);
        cpp_int csum_8b() const;
    
        typedef pu_cpp_int< 4 > phv_csum_flit_num_cpp_int_t;
        cpp_int int_var__phv_csum_flit_num;
        void phv_csum_flit_num (const cpp_int  & l__val);
        cpp_int phv_csum_flit_num() const;
    
        typedef pu_cpp_int< 1 > end_eop_cpp_int_t;
        cpp_int int_var__end_eop;
        void end_eop (const cpp_int  & l__val);
        cpp_int end_eop() const;
    
        typedef pu_cpp_int< 14 > len_mask_cpp_int_t;
        cpp_int int_var__len_mask;
        void len_mask (const cpp_int  & l__val);
        cpp_int len_mask() const;
    
        typedef pu_cpp_int< 1 > len_shift_left_cpp_int_t;
        cpp_int int_var__len_shift_left;
        void len_shift_left (const cpp_int  & l__val);
        cpp_int len_shift_left() const;
    
        typedef pu_cpp_int< 3 > len_shift_val_cpp_int_t;
        cpp_int int_var__len_shift_val;
        void len_shift_val (const cpp_int  & l__val);
        cpp_int len_shift_val() const;
    
        typedef pu_cpp_int< 1 > addsub_start_cpp_int_t;
        cpp_int int_var__addsub_start;
        void addsub_start (const cpp_int  & l__val);
        cpp_int addsub_start() const;
    
        typedef pu_cpp_int< 6 > start_adj_cpp_int_t;
        cpp_int int_var__start_adj;
        void start_adj (const cpp_int  & l__val);
        cpp_int start_adj() const;
    
        typedef pu_cpp_int< 1 > addsub_end_cpp_int_t;
        cpp_int int_var__addsub_end;
        void addsub_end (const cpp_int  & l__val);
        cpp_int addsub_end() const;
    
        typedef pu_cpp_int< 6 > end_adj_cpp_int_t;
        cpp_int int_var__end_adj;
        void end_adj (const cpp_int  & l__val);
        cpp_int end_adj() const;
    
        typedef pu_cpp_int< 1 > addsub_phdr_cpp_int_t;
        cpp_int int_var__addsub_phdr;
        void addsub_phdr (const cpp_int  & l__val);
        cpp_int addsub_phdr() const;
    
        typedef pu_cpp_int< 6 > phdr_adj_cpp_int_t;
        cpp_int int_var__phdr_adj;
        void phdr_adj (const cpp_int  & l__val);
        cpp_int phdr_adj() const;
    
        typedef pu_cpp_int< 1 > addsub_csum_loc_cpp_int_t;
        cpp_int int_var__addsub_csum_loc;
        void addsub_csum_loc (const cpp_int  & l__val);
        cpp_int addsub_csum_loc() const;
    
        typedef pu_cpp_int< 6 > csum_loc_adj_cpp_int_t;
        cpp_int int_var__csum_loc_adj;
        void csum_loc_adj (const cpp_int  & l__val);
        cpp_int csum_loc_adj() const;
    
        typedef pu_cpp_int< 1 > align_cpp_int_t;
        cpp_int int_var__align;
        void align (const cpp_int  & l__val);
        cpp_int align() const;
    
        typedef pu_cpp_int< 16 > add_val_cpp_int_t;
        cpp_int int_var__add_val;
        void add_val (const cpp_int  & l__val);
        cpp_int add_val() const;
    
}; // cap_ppa_csr_cfg_csum_profile_t
    
class cap_ppa_csr_cfg_crc_mask_profile_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_crc_mask_profile_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_crc_mask_profile_t(string name = "cap_ppa_csr_cfg_crc_mask_profile_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_crc_mask_profile_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > fld0_mask_en_cpp_int_t;
        cpp_int int_var__fld0_mask_en;
        void fld0_mask_en (const cpp_int  & l__val);
        cpp_int fld0_mask_en() const;
    
        typedef pu_cpp_int< 1 > fld0_use_ohi_cpp_int_t;
        cpp_int int_var__fld0_use_ohi;
        void fld0_use_ohi (const cpp_int  & l__val);
        cpp_int fld0_use_ohi() const;
    
        typedef pu_cpp_int< 6 > fld0_start_adj_cpp_int_t;
        cpp_int int_var__fld0_start_adj;
        void fld0_start_adj (const cpp_int  & l__val);
        cpp_int fld0_start_adj() const;
    
        typedef pu_cpp_int< 6 > fld0_end_adj_cpp_int_t;
        cpp_int int_var__fld0_end_adj;
        void fld0_end_adj (const cpp_int  & l__val);
        cpp_int fld0_end_adj() const;
    
        typedef pu_cpp_int< 1 > fld0_fill_cpp_int_t;
        cpp_int int_var__fld0_fill;
        void fld0_fill (const cpp_int  & l__val);
        cpp_int fld0_fill() const;
    
        typedef pu_cpp_int< 1 > fld0_skip_first_nibble_cpp_int_t;
        cpp_int int_var__fld0_skip_first_nibble;
        void fld0_skip_first_nibble (const cpp_int  & l__val);
        cpp_int fld0_skip_first_nibble() const;
    
        typedef pu_cpp_int< 1 > fld1_mask_en_cpp_int_t;
        cpp_int int_var__fld1_mask_en;
        void fld1_mask_en (const cpp_int  & l__val);
        cpp_int fld1_mask_en() const;
    
        typedef pu_cpp_int< 1 > fld1_use_ohi_cpp_int_t;
        cpp_int int_var__fld1_use_ohi;
        void fld1_use_ohi (const cpp_int  & l__val);
        cpp_int fld1_use_ohi() const;
    
        typedef pu_cpp_int< 6 > fld1_start_adj_cpp_int_t;
        cpp_int int_var__fld1_start_adj;
        void fld1_start_adj (const cpp_int  & l__val);
        cpp_int fld1_start_adj() const;
    
        typedef pu_cpp_int< 6 > fld1_end_adj_cpp_int_t;
        cpp_int int_var__fld1_end_adj;
        void fld1_end_adj (const cpp_int  & l__val);
        cpp_int fld1_end_adj() const;
    
        typedef pu_cpp_int< 1 > fld1_fill_cpp_int_t;
        cpp_int int_var__fld1_fill;
        void fld1_fill (const cpp_int  & l__val);
        cpp_int fld1_fill() const;
    
        typedef pu_cpp_int< 1 > fld1_skip_first_nibble_cpp_int_t;
        cpp_int int_var__fld1_skip_first_nibble;
        void fld1_skip_first_nibble (const cpp_int  & l__val);
        cpp_int fld1_skip_first_nibble() const;
    
        typedef pu_cpp_int< 1 > fld2_mask_en_cpp_int_t;
        cpp_int int_var__fld2_mask_en;
        void fld2_mask_en (const cpp_int  & l__val);
        cpp_int fld2_mask_en() const;
    
        typedef pu_cpp_int< 1 > fld2_use_ohi_cpp_int_t;
        cpp_int int_var__fld2_use_ohi;
        void fld2_use_ohi (const cpp_int  & l__val);
        cpp_int fld2_use_ohi() const;
    
        typedef pu_cpp_int< 6 > fld2_start_adj_cpp_int_t;
        cpp_int int_var__fld2_start_adj;
        void fld2_start_adj (const cpp_int  & l__val);
        cpp_int fld2_start_adj() const;
    
        typedef pu_cpp_int< 6 > fld2_end_adj_cpp_int_t;
        cpp_int int_var__fld2_end_adj;
        void fld2_end_adj (const cpp_int  & l__val);
        cpp_int fld2_end_adj() const;
    
        typedef pu_cpp_int< 1 > fld2_fill_cpp_int_t;
        cpp_int int_var__fld2_fill;
        void fld2_fill (const cpp_int  & l__val);
        cpp_int fld2_fill() const;
    
        typedef pu_cpp_int< 1 > fld2_skip_first_nibble_cpp_int_t;
        cpp_int int_var__fld2_skip_first_nibble;
        void fld2_skip_first_nibble (const cpp_int  & l__val);
        cpp_int fld2_skip_first_nibble() const;
    
        typedef pu_cpp_int< 1 > fld3_mask_en_cpp_int_t;
        cpp_int int_var__fld3_mask_en;
        void fld3_mask_en (const cpp_int  & l__val);
        cpp_int fld3_mask_en() const;
    
        typedef pu_cpp_int< 1 > fld3_use_ohi_cpp_int_t;
        cpp_int int_var__fld3_use_ohi;
        void fld3_use_ohi (const cpp_int  & l__val);
        cpp_int fld3_use_ohi() const;
    
        typedef pu_cpp_int< 6 > fld3_start_adj_cpp_int_t;
        cpp_int int_var__fld3_start_adj;
        void fld3_start_adj (const cpp_int  & l__val);
        cpp_int fld3_start_adj() const;
    
        typedef pu_cpp_int< 6 > fld3_end_adj_cpp_int_t;
        cpp_int int_var__fld3_end_adj;
        void fld3_end_adj (const cpp_int  & l__val);
        cpp_int fld3_end_adj() const;
    
        typedef pu_cpp_int< 1 > fld3_fill_cpp_int_t;
        cpp_int int_var__fld3_fill;
        void fld3_fill (const cpp_int  & l__val);
        cpp_int fld3_fill() const;
    
        typedef pu_cpp_int< 1 > fld3_skip_first_nibble_cpp_int_t;
        cpp_int int_var__fld3_skip_first_nibble;
        void fld3_skip_first_nibble (const cpp_int  & l__val);
        cpp_int fld3_skip_first_nibble() const;
    
        typedef pu_cpp_int< 1 > fld4_mask_en_cpp_int_t;
        cpp_int int_var__fld4_mask_en;
        void fld4_mask_en (const cpp_int  & l__val);
        cpp_int fld4_mask_en() const;
    
        typedef pu_cpp_int< 1 > fld4_use_ohi_cpp_int_t;
        cpp_int int_var__fld4_use_ohi;
        void fld4_use_ohi (const cpp_int  & l__val);
        cpp_int fld4_use_ohi() const;
    
        typedef pu_cpp_int< 6 > fld4_start_adj_cpp_int_t;
        cpp_int int_var__fld4_start_adj;
        void fld4_start_adj (const cpp_int  & l__val);
        cpp_int fld4_start_adj() const;
    
        typedef pu_cpp_int< 6 > fld4_end_adj_cpp_int_t;
        cpp_int int_var__fld4_end_adj;
        void fld4_end_adj (const cpp_int  & l__val);
        cpp_int fld4_end_adj() const;
    
        typedef pu_cpp_int< 1 > fld4_fill_cpp_int_t;
        cpp_int int_var__fld4_fill;
        void fld4_fill (const cpp_int  & l__val);
        cpp_int fld4_fill() const;
    
        typedef pu_cpp_int< 1 > fld4_skip_first_nibble_cpp_int_t;
        cpp_int int_var__fld4_skip_first_nibble;
        void fld4_skip_first_nibble (const cpp_int  & l__val);
        cpp_int fld4_skip_first_nibble() const;
    
        typedef pu_cpp_int< 1 > fld5_mask_en_cpp_int_t;
        cpp_int int_var__fld5_mask_en;
        void fld5_mask_en (const cpp_int  & l__val);
        cpp_int fld5_mask_en() const;
    
        typedef pu_cpp_int< 1 > fld5_use_ohi_cpp_int_t;
        cpp_int int_var__fld5_use_ohi;
        void fld5_use_ohi (const cpp_int  & l__val);
        cpp_int fld5_use_ohi() const;
    
        typedef pu_cpp_int< 6 > fld5_start_adj_cpp_int_t;
        cpp_int int_var__fld5_start_adj;
        void fld5_start_adj (const cpp_int  & l__val);
        cpp_int fld5_start_adj() const;
    
        typedef pu_cpp_int< 6 > fld5_end_adj_cpp_int_t;
        cpp_int int_var__fld5_end_adj;
        void fld5_end_adj (const cpp_int  & l__val);
        cpp_int fld5_end_adj() const;
    
        typedef pu_cpp_int< 1 > fld5_fill_cpp_int_t;
        cpp_int int_var__fld5_fill;
        void fld5_fill (const cpp_int  & l__val);
        cpp_int fld5_fill() const;
    
        typedef pu_cpp_int< 1 > fld5_skip_first_nibble_cpp_int_t;
        cpp_int int_var__fld5_skip_first_nibble;
        void fld5_skip_first_nibble (const cpp_int  & l__val);
        cpp_int fld5_skip_first_nibble() const;
    
}; // cap_ppa_csr_cfg_crc_mask_profile_t
    
class cap_ppa_csr_cfg_crc_profile_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_crc_profile_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_crc_profile_t(string name = "cap_ppa_csr_cfg_crc_profile_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_crc_profile_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 14 > len_mask_cpp_int_t;
        cpp_int int_var__len_mask;
        void len_mask (const cpp_int  & l__val);
        cpp_int len_mask() const;
    
        typedef pu_cpp_int< 1 > len_shift_left_cpp_int_t;
        cpp_int int_var__len_shift_left;
        void len_shift_left (const cpp_int  & l__val);
        cpp_int len_shift_left() const;
    
        typedef pu_cpp_int< 3 > len_shift_val_cpp_int_t;
        cpp_int int_var__len_shift_val;
        void len_shift_val (const cpp_int  & l__val);
        cpp_int len_shift_val() const;
    
        typedef pu_cpp_int< 1 > addsub_start_cpp_int_t;
        cpp_int int_var__addsub_start;
        void addsub_start (const cpp_int  & l__val);
        cpp_int addsub_start() const;
    
        typedef pu_cpp_int< 6 > start_adj_cpp_int_t;
        cpp_int int_var__start_adj;
        void start_adj (const cpp_int  & l__val);
        cpp_int start_adj() const;
    
        typedef pu_cpp_int< 1 > addsub_end_cpp_int_t;
        cpp_int int_var__addsub_end;
        void addsub_end (const cpp_int  & l__val);
        cpp_int addsub_end() const;
    
        typedef pu_cpp_int< 6 > end_adj_cpp_int_t;
        cpp_int int_var__end_adj;
        void end_adj (const cpp_int  & l__val);
        cpp_int end_adj() const;
    
        typedef pu_cpp_int< 1 > addsub_mask_cpp_int_t;
        cpp_int int_var__addsub_mask;
        void addsub_mask (const cpp_int  & l__val);
        cpp_int addsub_mask() const;
    
        typedef pu_cpp_int< 6 > mask_adj_cpp_int_t;
        cpp_int int_var__mask_adj;
        void mask_adj (const cpp_int  & l__val);
        cpp_int mask_adj() const;
    
        typedef pu_cpp_int< 1 > end_eop_cpp_int_t;
        cpp_int int_var__end_eop;
        void end_eop (const cpp_int  & l__val);
        cpp_int end_eop() const;
    
        typedef pu_cpp_int< 2 > mask_prof_sel_cpp_int_t;
        cpp_int int_var__mask_prof_sel;
        void mask_prof_sel (const cpp_int  & l__val);
        cpp_int mask_prof_sel() const;
    
}; // cap_ppa_csr_cfg_crc_profile_t
    
class cap_ppa_csr_sta_bndl1_state_lkp_tcam_ind_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_bndl1_state_lkp_tcam_ind_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_bndl1_state_lkp_tcam_ind_t(string name = "cap_ppa_csr_sta_bndl1_state_lkp_tcam_ind_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_bndl1_state_lkp_tcam_ind_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 9 > hit_addr_cpp_int_t;
        cpp_int int_var__hit_addr;
        void hit_addr (const cpp_int  & l__val);
        cpp_int hit_addr() const;
    
}; // cap_ppa_csr_sta_bndl1_state_lkp_tcam_ind_t
    
class cap_ppa_csr_cfg_bndl1_state_lkp_tcam_ind_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_bndl1_state_lkp_tcam_ind_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_bndl1_state_lkp_tcam_ind_t(string name = "cap_ppa_csr_cfg_bndl1_state_lkp_tcam_ind_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_bndl1_state_lkp_tcam_ind_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_cfg_bndl1_state_lkp_tcam_ind_t
    
class cap_ppa_csr_cfg_bndl1_state_lkp_tcam_grst_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_bndl1_state_lkp_tcam_grst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_bndl1_state_lkp_tcam_grst_t(string name = "cap_ppa_csr_cfg_bndl1_state_lkp_tcam_grst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_bndl1_state_lkp_tcam_grst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_cfg_bndl1_state_lkp_tcam_grst_t
    
class cap_ppa_csr_sta_bndl0_state_lkp_tcam_ind_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_sta_bndl0_state_lkp_tcam_ind_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_sta_bndl0_state_lkp_tcam_ind_t(string name = "cap_ppa_csr_sta_bndl0_state_lkp_tcam_ind_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_sta_bndl0_state_lkp_tcam_ind_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 9 > hit_addr_cpp_int_t;
        cpp_int int_var__hit_addr;
        void hit_addr (const cpp_int  & l__val);
        cpp_int hit_addr() const;
    
}; // cap_ppa_csr_sta_bndl0_state_lkp_tcam_ind_t
    
class cap_ppa_csr_cfg_bndl0_state_lkp_tcam_ind_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_bndl0_state_lkp_tcam_ind_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_bndl0_state_lkp_tcam_ind_t(string name = "cap_ppa_csr_cfg_bndl0_state_lkp_tcam_ind_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_bndl0_state_lkp_tcam_ind_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_cfg_bndl0_state_lkp_tcam_ind_t
    
class cap_ppa_csr_cfg_bndl0_state_lkp_tcam_grst_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_bndl0_state_lkp_tcam_grst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_bndl0_state_lkp_tcam_grst_t(string name = "cap_ppa_csr_cfg_bndl0_state_lkp_tcam_grst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_bndl0_state_lkp_tcam_grst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_cfg_bndl0_state_lkp_tcam_grst_t
    
class cap_ppa_csr_cfg_init_profile_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_init_profile_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_init_profile_t(string name = "cap_ppa_csr_cfg_init_profile_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_init_profile_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 6 > curr_offset_cpp_int_t;
        cpp_int int_var__curr_offset;
        void curr_offset (const cpp_int  & l__val);
        cpp_int curr_offset() const;
    
        typedef pu_cpp_int< 9 > state_cpp_int_t;
        cpp_int int_var__state;
        void state (const cpp_int  & l__val);
        cpp_int state() const;
    
        typedef pu_cpp_int< 6 > lkp_val_pkt_idx0_cpp_int_t;
        cpp_int int_var__lkp_val_pkt_idx0;
        void lkp_val_pkt_idx0 (const cpp_int  & l__val);
        cpp_int lkp_val_pkt_idx0() const;
    
        typedef pu_cpp_int< 6 > lkp_val_pkt_idx1_cpp_int_t;
        cpp_int int_var__lkp_val_pkt_idx1;
        void lkp_val_pkt_idx1 (const cpp_int  & l__val);
        cpp_int lkp_val_pkt_idx1() const;
    
        typedef pu_cpp_int< 6 > lkp_val_pkt_idx2_cpp_int_t;
        cpp_int int_var__lkp_val_pkt_idx2;
        void lkp_val_pkt_idx2 (const cpp_int  & l__val);
        cpp_int lkp_val_pkt_idx2() const;
    
}; // cap_ppa_csr_cfg_init_profile_t
    
class cap_ppa_csr_cfg_ctrl_thresh_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_ctrl_thresh_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_ctrl_thresh_t(string name = "cap_ppa_csr_cfg_ctrl_thresh_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_ctrl_thresh_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > outphv_ff_cpp_int_t;
        cpp_int int_var__outphv_ff;
        void outphv_ff (const cpp_int  & l__val);
        cpp_int outphv_ff() const;
    
        typedef pu_cpp_int< 5 > phv_ff_cpp_int_t;
        cpp_int int_var__phv_ff;
        void phv_ff (const cpp_int  & l__val);
        cpp_int phv_ff() const;
    
        typedef pu_cpp_int< 6 > chk_ff_cpp_int_t;
        cpp_int int_var__chk_ff;
        void chk_ff (const cpp_int  & l__val);
        cpp_int chk_ff() const;
    
}; // cap_ppa_csr_cfg_ctrl_thresh_t
    
class cap_ppa_csr_cfg_ctrl_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_cfg_ctrl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_cfg_ctrl_t(string name = "cap_ppa_csr_cfg_ctrl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_cfg_ctrl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 10 > pe_enable_cpp_int_t;
        cpp_int int_var__pe_enable;
        void pe_enable (const cpp_int  & l__val);
        cpp_int pe_enable() const;
    
        typedef pu_cpp_int< 9 > parse_loop_cnt_cpp_int_t;
        cpp_int int_var__parse_loop_cnt;
        void parse_loop_cnt (const cpp_int  & l__val);
        cpp_int parse_loop_cnt() const;
    
        typedef pu_cpp_int< 3 > num_phv_flit_cpp_int_t;
        cpp_int int_var__num_phv_flit;
        void num_phv_flit (const cpp_int  & l__val);
        cpp_int num_phv_flit() const;
    
        typedef pu_cpp_int< 9 > state_lkp_catchall_entry_cpp_int_t;
        cpp_int int_var__state_lkp_catchall_entry;
        void state_lkp_catchall_entry (const cpp_int  & l__val);
        cpp_int state_lkp_catchall_entry() const;
    
        typedef pu_cpp_int< 1 > phv_set_hw_err_en_cpp_int_t;
        cpp_int int_var__phv_set_hw_err_en;
        void phv_set_hw_err_en (const cpp_int  & l__val);
        cpp_int phv_set_hw_err_en() const;
    
        typedef pu_cpp_int< 1 > phv_set_parser_err_en_cpp_int_t;
        cpp_int int_var__phv_set_parser_err_en;
        void phv_set_parser_err_en (const cpp_int  & l__val);
        cpp_int phv_set_parser_err_en() const;
    
        typedef pu_cpp_int< 1 > gso_csum_en_cpp_int_t;
        cpp_int int_var__gso_csum_en;
        void gso_csum_en (const cpp_int  & l__val);
        cpp_int gso_csum_en() const;
    
        typedef pu_cpp_int< 7 > pe_err_vec_en_cpp_int_t;
        cpp_int int_var__pe_err_vec_en;
        void pe_err_vec_en (const cpp_int  & l__val);
        cpp_int pe_err_vec_en() const;
    
        typedef pu_cpp_int< 1 > end_offset_en_cpp_int_t;
        cpp_int int_var__end_offset_en;
        void end_offset_en (const cpp_int  & l__val);
        cpp_int end_offset_en() const;
    
        typedef pu_cpp_int< 4 > end_offset_flit_num_cpp_int_t;
        cpp_int int_var__end_offset_flit_num;
        void end_offset_flit_num (const cpp_int  & l__val);
        cpp_int end_offset_flit_num() const;
    
        typedef pu_cpp_int< 7 > tcam_key_ctl_cpp_int_t;
        cpp_int int_var__tcam_key_ctl;
        void tcam_key_ctl (const cpp_int  & l__val);
        cpp_int tcam_key_ctl() const;
    
        typedef pu_cpp_int< 1 > crc_start_offset_err_en_cpp_int_t;
        cpp_int int_var__crc_start_offset_err_en;
        void crc_start_offset_err_en (const cpp_int  & l__val);
        cpp_int crc_start_offset_err_en() const;
    
        typedef pu_cpp_int< 1 > csum_start_offset_err_en_cpp_int_t;
        cpp_int int_var__csum_start_offset_err_en;
        void csum_start_offset_err_en (const cpp_int  & l__val);
        cpp_int csum_start_offset_err_en() const;
    
}; // cap_ppa_csr_cfg_ctrl_t
    
class cap_ppa_csr_rdintr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_rdintr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_rdintr_t(string name = "cap_ppa_csr_rdintr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_rdintr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_rdintr_t
    
class cap_ppa_csr_base_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_base_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_base_t(string name = "cap_ppa_csr_base_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_base_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_csr_base_t
    
class cap_ppa_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_csr_t(string name = "cap_ppa_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_ppa_csr_base_t base;
    
        cap_ppa_csr_rdintr_t rdintr;
    
        cap_ppa_csr_cfg_ctrl_t cfg_ctrl;
    
        cap_ppa_csr_cfg_ctrl_thresh_t cfg_ctrl_thresh;
    
        #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_csr_cfg_init_profile_t, 16> cfg_init_profile;
        #else 
        cap_ppa_csr_cfg_init_profile_t cfg_init_profile[16];
        #endif
        int get_depth_cfg_init_profile() { return 16; }
    
        cap_ppa_csr_cfg_bndl0_state_lkp_tcam_grst_t cfg_bndl0_state_lkp_tcam_grst;
    
        cap_ppa_csr_cfg_bndl0_state_lkp_tcam_ind_t cfg_bndl0_state_lkp_tcam_ind;
    
        cap_ppa_csr_sta_bndl0_state_lkp_tcam_ind_t sta_bndl0_state_lkp_tcam_ind;
    
        cap_ppa_csr_cfg_bndl1_state_lkp_tcam_grst_t cfg_bndl1_state_lkp_tcam_grst;
    
        cap_ppa_csr_cfg_bndl1_state_lkp_tcam_ind_t cfg_bndl1_state_lkp_tcam_ind;
    
        cap_ppa_csr_sta_bndl1_state_lkp_tcam_ind_t sta_bndl1_state_lkp_tcam_ind;
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_csr_cfg_crc_profile_t, 8> cfg_crc_profile;
        #else 
        cap_ppa_csr_cfg_crc_profile_t cfg_crc_profile[8];
        #endif
        int get_depth_cfg_crc_profile() { return 8; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_csr_cfg_crc_mask_profile_t, 4> cfg_crc_mask_profile;
        #else 
        cap_ppa_csr_cfg_crc_mask_profile_t cfg_crc_mask_profile[4];
        #endif
        int get_depth_cfg_crc_mask_profile() { return 4; }
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_csr_cfg_csum_profile_t, 8> cfg_csum_profile;
        #else 
        cap_ppa_csr_cfg_csum_profile_t cfg_csum_profile[8];
        #endif
        int get_depth_cfg_csum_profile() { return 8; }
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_csr_cfg_csum_phdr_profile_t, 8> cfg_csum_phdr_profile;
        #else 
        cap_ppa_csr_cfg_csum_phdr_profile_t cfg_csum_phdr_profile[8];
        #endif
        int get_depth_cfg_csum_phdr_profile() { return 8; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_csr_cfg_len_chk_profile_t, 4> cfg_len_chk_profile;
        #else 
        cap_ppa_csr_cfg_len_chk_profile_t cfg_len_chk_profile[4];
        #endif
        int get_depth_cfg_len_chk_profile() { return 4; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_csr_cfg_align_chk_profile_t, 4> cfg_align_chk_profile;
        #else 
        cap_ppa_csr_cfg_align_chk_profile_t cfg_align_chk_profile[4];
        #endif
        int get_depth_cfg_align_chk_profile() { return 4; }
    
        cap_ppa_csr_CNT_ppa_pb_t CNT_ppa_pb;
    
        cap_ppa_csr_CNT_ppa_dp_t CNT_ppa_dp;
    
        cap_ppa_csr_CNT_ppa_ma_t CNT_ppa_ma;
    
        cap_ppa_csr_CNT_ppa_sw_phv_t CNT_ppa_sw_phv;
    
        cap_ppa_csr_CNT_ppa_ohi_t CNT_ppa_ohi;
    
        cap_ppa_csr_CNT_ppa_pe0_pkt_in_t CNT_ppa_pe0_pkt_in;
    
        cap_ppa_csr_CNT_ppa_pe0_t CNT_ppa_pe0;
    
        cap_ppa_csr_CNT_ppa_pe0_phv_out_t CNT_ppa_pe0_phv_out;
    
        cap_ppa_csr_CNT_ppa_pe1_pkt_in_t CNT_ppa_pe1_pkt_in;
    
        cap_ppa_csr_CNT_ppa_pe1_t CNT_ppa_pe1;
    
        cap_ppa_csr_CNT_ppa_pe1_phv_out_t CNT_ppa_pe1_phv_out;
    
        cap_ppa_csr_CNT_ppa_pe2_pkt_in_t CNT_ppa_pe2_pkt_in;
    
        cap_ppa_csr_CNT_ppa_pe2_t CNT_ppa_pe2;
    
        cap_ppa_csr_CNT_ppa_pe2_phv_out_t CNT_ppa_pe2_phv_out;
    
        cap_ppa_csr_CNT_ppa_pe3_pkt_in_t CNT_ppa_pe3_pkt_in;
    
        cap_ppa_csr_CNT_ppa_pe3_t CNT_ppa_pe3;
    
        cap_ppa_csr_CNT_ppa_pe3_phv_out_t CNT_ppa_pe3_phv_out;
    
        cap_ppa_csr_CNT_ppa_pe4_pkt_in_t CNT_ppa_pe4_pkt_in;
    
        cap_ppa_csr_CNT_ppa_pe4_t CNT_ppa_pe4;
    
        cap_ppa_csr_CNT_ppa_pe4_phv_out_t CNT_ppa_pe4_phv_out;
    
        cap_ppa_csr_CNT_ppa_pe5_pkt_in_t CNT_ppa_pe5_pkt_in;
    
        cap_ppa_csr_CNT_ppa_pe5_t CNT_ppa_pe5;
    
        cap_ppa_csr_CNT_ppa_pe5_phv_out_t CNT_ppa_pe5_phv_out;
    
        cap_ppa_csr_CNT_ppa_pe6_pkt_in_t CNT_ppa_pe6_pkt_in;
    
        cap_ppa_csr_CNT_ppa_pe6_t CNT_ppa_pe6;
    
        cap_ppa_csr_CNT_ppa_pe6_phv_out_t CNT_ppa_pe6_phv_out;
    
        cap_ppa_csr_CNT_ppa_pe7_pkt_in_t CNT_ppa_pe7_pkt_in;
    
        cap_ppa_csr_CNT_ppa_pe7_t CNT_ppa_pe7;
    
        cap_ppa_csr_CNT_ppa_pe7_phv_out_t CNT_ppa_pe7_phv_out;
    
        cap_ppa_csr_CNT_ppa_pe8_pkt_in_t CNT_ppa_pe8_pkt_in;
    
        cap_ppa_csr_CNT_ppa_pe8_t CNT_ppa_pe8;
    
        cap_ppa_csr_CNT_ppa_pe8_phv_out_t CNT_ppa_pe8_phv_out;
    
        cap_ppa_csr_CNT_ppa_pe9_pkt_in_t CNT_ppa_pe9_pkt_in;
    
        cap_ppa_csr_CNT_ppa_pe9_t CNT_ppa_pe9;
    
        cap_ppa_csr_CNT_ppa_pe9_phv_out_t CNT_ppa_pe9_phv_out;
    
        cap_ppa_csr_csr_intr_t csr_intr;
    
        cap_ppa_csr_cfg_pkt_mem_t cfg_pkt_mem;
    
        cap_ppa_csr_sta_pkt_mem_t sta_pkt_mem;
    
        cap_ppa_csr_cfg_bndl0_state_lkp_sram_t cfg_bndl0_state_lkp_sram;
    
        cap_ppa_csr_sta_bndl0_state_lkp_sram_t sta_bndl0_state_lkp_sram;
    
        cap_ppa_csr_cfg_bndl1_state_lkp_sram_t cfg_bndl1_state_lkp_sram;
    
        cap_ppa_csr_sta_bndl1_state_lkp_sram_t sta_bndl1_state_lkp_sram;
    
        cap_ppa_csr_cfg_bndl0_inq_mem_t cfg_bndl0_inq_mem;
    
        cap_ppa_csr_sta_bndl0_inq_mem_t sta_bndl0_inq_mem;
    
        cap_ppa_csr_cfg_bndl1_inq_mem_t cfg_bndl1_inq_mem;
    
        cap_ppa_csr_sta_bndl1_inq_mem_t sta_bndl1_inq_mem;
    
        cap_ppa_csr_cfg_outq_phv_mem_t cfg_outq_phv_mem;
    
        cap_ppa_csr_sta_outq_phv_mem_t sta_outq_phv_mem;
    
        cap_ppa_csr_cfg_outq_ohi_mem_t cfg_outq_ohi_mem;
    
        cap_ppa_csr_sta_outq_ohi_mem_t sta_outq_ohi_mem;
    
        cap_ppa_csr_cfg_bndl0_state_lkp_tcam_t cfg_bndl0_state_lkp_tcam;
    
        cap_ppa_csr_sta_bndl0_state_lkp_tcam_t sta_bndl0_state_lkp_tcam;
    
        cap_ppa_csr_cfg_bndl1_state_lkp_tcam_t cfg_bndl1_state_lkp_tcam;
    
        cap_ppa_csr_sta_bndl1_state_lkp_tcam_t sta_bndl1_state_lkp_tcam;
    
        cap_ppa_csr_cfg_pe0_phv_mem_t cfg_pe0_phv_mem;
    
        cap_ppa_csr_sta_pe0_phv_mem_t sta_pe0_phv_mem;
    
        cap_ppa_csr_cfg_pe1_phv_mem_t cfg_pe1_phv_mem;
    
        cap_ppa_csr_sta_pe1_phv_mem_t sta_pe1_phv_mem;
    
        cap_ppa_csr_cfg_pe2_phv_mem_t cfg_pe2_phv_mem;
    
        cap_ppa_csr_sta_pe2_phv_mem_t sta_pe2_phv_mem;
    
        cap_ppa_csr_cfg_pe3_phv_mem_t cfg_pe3_phv_mem;
    
        cap_ppa_csr_sta_pe3_phv_mem_t sta_pe3_phv_mem;
    
        cap_ppa_csr_cfg_pe4_phv_mem_t cfg_pe4_phv_mem;
    
        cap_ppa_csr_sta_pe4_phv_mem_t sta_pe4_phv_mem;
    
        cap_ppa_csr_cfg_pe5_phv_mem_t cfg_pe5_phv_mem;
    
        cap_ppa_csr_sta_pe5_phv_mem_t sta_pe5_phv_mem;
    
        cap_ppa_csr_cfg_pe6_phv_mem_t cfg_pe6_phv_mem;
    
        cap_ppa_csr_sta_pe6_phv_mem_t sta_pe6_phv_mem;
    
        cap_ppa_csr_cfg_pe7_phv_mem_t cfg_pe7_phv_mem;
    
        cap_ppa_csr_sta_pe7_phv_mem_t sta_pe7_phv_mem;
    
        cap_ppa_csr_cfg_pe8_phv_mem_t cfg_pe8_phv_mem;
    
        cap_ppa_csr_sta_pe8_phv_mem_t sta_pe8_phv_mem;
    
        cap_ppa_csr_cfg_pe9_phv_mem_t cfg_pe9_phv_mem;
    
        cap_ppa_csr_sta_pe9_phv_mem_t sta_pe9_phv_mem;
    
        cap_ppa_csr_cfg_sw_phv_global_t cfg_sw_phv_global;
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_csr_cfg_sw_phv_control_t, 8> cfg_sw_phv_control;
        #else 
        cap_ppa_csr_cfg_sw_phv_control_t cfg_sw_phv_control[8];
        #endif
        int get_depth_cfg_sw_phv_control() { return 8; }
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_csr_cfg_sw_phv_config_t, 8> cfg_sw_phv_config;
        #else 
        cap_ppa_csr_cfg_sw_phv_config_t cfg_sw_phv_config[8];
        #endif
        int get_depth_cfg_sw_phv_config() { return 8; }
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_csr_sta_sw_phv_state_t, 8> sta_sw_phv_state;
        #else 
        cap_ppa_csr_sta_sw_phv_state_t sta_sw_phv_state[8];
        #endif
        int get_depth_sta_sw_phv_state() { return 8; }
    
        cap_ppa_csr_cfg_sw_phv_mem_t cfg_sw_phv_mem;
    
        cap_ppa_csr_sta_sw_phv_mem_t sta_sw_phv_mem;
    
        cap_ppa_csr_sta_reorder_phv_not_sop_err_t sta_reorder_phv_not_sop_err;
    
        cap_ppa_csr_sta_reorder_t sta_reorder;
    
        cap_ppa_csr_sta_csum0_t sta_csum0;
    
        cap_ppa_csr_sta_csum1_t sta_csum1;
    
        cap_ppa_csr_sta_csum2_t sta_csum2;
    
        cap_ppa_csr_sta_csum3_t sta_csum3;
    
        cap_ppa_csr_sta_csum4_t sta_csum4;
    
        cap_ppa_csr_sta_chkr_seq_id_t sta_chkr_seq_id;
    
        cap_ppa_csr_sta_pe0_t sta_pe0;
    
        cap_ppa_csr_sta_pe1_t sta_pe1;
    
        cap_ppa_csr_sta_pe2_t sta_pe2;
    
        cap_ppa_csr_sta_pe3_t sta_pe3;
    
        cap_ppa_csr_sta_pe4_t sta_pe4;
    
        cap_ppa_csr_sta_pe5_t sta_pe5;
    
        cap_ppa_csr_sta_pe6_t sta_pe6;
    
        cap_ppa_csr_sta_pe7_t sta_pe7;
    
        cap_ppa_csr_sta_pe8_t sta_pe8;
    
        cap_ppa_csr_sta_pe9_t sta_pe9;
    
        cap_ppa_csr_sta_bndl0_state_lkp_t sta_bndl0_state_lkp;
    
        cap_ppa_csr_sta_bndl1_state_lkp_t sta_bndl1_state_lkp;
    
        cap_ppa_csr_sta_fifo_t sta_fifo;
    
        cap_ppa_csr_sta_bndl0_t sta_bndl0;
    
        cap_ppa_csr_sta_bndl1_t sta_bndl1;
    
        cap_ppa_csr_sta_pe_alloc_t sta_pe_alloc;
    
        cap_ppa_csr_cfg_debug_ctrl_t cfg_debug_ctrl;
    
        cap_ppa_csr_cfg_debug_port_t cfg_debug_port;
    
        cap_ppa_csr_cfg_spare_t cfg_spare;
    
        cap_ppa_csr_cfg_spare_dbg_t cfg_spare_dbg;
    
        cap_ppa_csr_cfg_preparse_t cfg_preparse;
    
        cap_ppa_csr_sta_preparse_t sta_preparse;
    
        cap_ppa_csr_CNT_preparse_t CNT_preparse;
    
        cap_ppa_csr_dhs_bndl0_state_lkp_tcam_t dhs_bndl0_state_lkp_tcam;
    
        cap_ppa_csr_dhs_bndl0_state_lkp_tcam_ind_t dhs_bndl0_state_lkp_tcam_ind;
    
        cap_ppa_csr_dhs_bndl0_state_lkp_sram_t dhs_bndl0_state_lkp_sram;
    
        cap_ppa_csr_dhs_bndl1_state_lkp_tcam_t dhs_bndl1_state_lkp_tcam;
    
        cap_ppa_csr_dhs_bndl1_state_lkp_tcam_ind_t dhs_bndl1_state_lkp_tcam_ind;
    
        cap_ppa_csr_dhs_bndl1_state_lkp_sram_t dhs_bndl1_state_lkp_sram;
    
        cap_ppa_csr_dhs_sw_phv_mem_t dhs_sw_phv_mem;
    
        cap_ppa_csr_dhs_dbg_mem_t dhs_dbg_mem;
    
        cap_ppa_csr_intgrp_status_t int_groups;
    
        cap_ppa_csr_int_bndl0_t int_bndl0;
    
        cap_ppa_csr_int_bndl0_t int_bndl1;
    
        cap_ppa_csr_int_ecc_t int_ecc;
    
        cap_ppa_csr_int_fifo1_t int_fifo1;
    
        cap_ppa_csr_int_fifo2_t int_fifo2;
    
        cap_ppa_csr_int_pe9_t int_pe0;
    
        cap_ppa_csr_int_pe9_t int_pe1;
    
        cap_ppa_csr_int_pe9_t int_pe2;
    
        cap_ppa_csr_int_pe9_t int_pe3;
    
        cap_ppa_csr_int_pe9_t int_pe4;
    
        cap_ppa_csr_int_pe9_t int_pe5;
    
        cap_ppa_csr_int_pe9_t int_pe6;
    
        cap_ppa_csr_int_pe9_t int_pe7;
    
        cap_ppa_csr_int_pe9_t int_pe8;
    
        cap_ppa_csr_int_pe9_t int_pe9;
    
        cap_ppa_csr_int_pa_t int_pa;
    
        cap_ppa_csr_int_intf_t int_intf;
    
        cap_ppa_csr_int_sw_phv_mem_t int_sw_phv_mem;
    
}; // cap_ppa_csr_t
    
#endif // CAP_PPA_CSR_H
        