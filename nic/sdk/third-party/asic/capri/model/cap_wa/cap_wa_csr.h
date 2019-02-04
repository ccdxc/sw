
#ifndef CAP_WA_CSR_H
#define CAP_WA_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_wa_csr_int_lif_qstate_map_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_int_lif_qstate_map_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_int_lif_qstate_map_int_enable_clear_t(string name = "cap_wa_csr_int_lif_qstate_map_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_int_lif_qstate_map_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_wa_csr_int_lif_qstate_map_int_enable_clear_t
    
class cap_wa_csr_int_lif_qstate_map_int_test_set_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_int_lif_qstate_map_int_test_set_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_int_lif_qstate_map_int_test_set_t(string name = "cap_wa_csr_int_lif_qstate_map_int_test_set_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_int_lif_qstate_map_int_test_set_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_wa_csr_int_lif_qstate_map_int_test_set_t
    
class cap_wa_csr_int_lif_qstate_map_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_int_lif_qstate_map_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_int_lif_qstate_map_t(string name = "cap_wa_csr_int_lif_qstate_map_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_int_lif_qstate_map_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_wa_csr_int_lif_qstate_map_int_test_set_t intreg;
    
        cap_wa_csr_int_lif_qstate_map_int_test_set_t int_test_set;
    
        cap_wa_csr_int_lif_qstate_map_int_enable_clear_t int_enable_set;
    
        cap_wa_csr_int_lif_qstate_map_int_enable_clear_t int_enable_clear;
    
}; // cap_wa_csr_int_lif_qstate_map_t
    
class cap_wa_csr_int_db_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_int_db_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_int_db_int_enable_clear_t(string name = "cap_wa_csr_int_db_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_int_db_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > db_cam_conflict_enable_cpp_int_t;
        cpp_int int_var__db_cam_conflict_enable;
        void db_cam_conflict_enable (const cpp_int  & l__val);
        cpp_int db_cam_conflict_enable() const;
    
        typedef pu_cpp_int< 1 > db_pid_chk_fail_enable_cpp_int_t;
        cpp_int int_var__db_pid_chk_fail_enable;
        void db_pid_chk_fail_enable (const cpp_int  & l__val);
        cpp_int db_pid_chk_fail_enable() const;
    
        typedef pu_cpp_int< 1 > db_qid_overflow_enable_cpp_int_t;
        cpp_int int_var__db_qid_overflow_enable;
        void db_qid_overflow_enable (const cpp_int  & l__val);
        cpp_int db_qid_overflow_enable() const;
    
        typedef pu_cpp_int< 1 > host_ring_access_err_enable_cpp_int_t;
        cpp_int int_var__host_ring_access_err_enable;
        void host_ring_access_err_enable (const cpp_int  & l__val);
        cpp_int host_ring_access_err_enable() const;
    
        typedef pu_cpp_int< 1 > total_ring_access_err_enable_cpp_int_t;
        cpp_int int_var__total_ring_access_err_enable;
        void total_ring_access_err_enable (const cpp_int  & l__val);
        cpp_int total_ring_access_err_enable() const;
    
        typedef pu_cpp_int< 1 > rresp_err_enable_cpp_int_t;
        cpp_int int_var__rresp_err_enable;
        void rresp_err_enable (const cpp_int  & l__val);
        cpp_int rresp_err_enable() const;
    
        typedef pu_cpp_int< 1 > bresp_err_enable_cpp_int_t;
        cpp_int int_var__bresp_err_enable;
        void bresp_err_enable (const cpp_int  & l__val);
        cpp_int bresp_err_enable() const;
    
}; // cap_wa_csr_int_db_int_enable_clear_t
    
class cap_wa_csr_int_db_intreg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_int_db_intreg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_int_db_intreg_t(string name = "cap_wa_csr_int_db_intreg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_int_db_intreg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > db_cam_conflict_interrupt_cpp_int_t;
        cpp_int int_var__db_cam_conflict_interrupt;
        void db_cam_conflict_interrupt (const cpp_int  & l__val);
        cpp_int db_cam_conflict_interrupt() const;
    
        typedef pu_cpp_int< 1 > db_pid_chk_fail_interrupt_cpp_int_t;
        cpp_int int_var__db_pid_chk_fail_interrupt;
        void db_pid_chk_fail_interrupt (const cpp_int  & l__val);
        cpp_int db_pid_chk_fail_interrupt() const;
    
        typedef pu_cpp_int< 1 > db_qid_overflow_interrupt_cpp_int_t;
        cpp_int int_var__db_qid_overflow_interrupt;
        void db_qid_overflow_interrupt (const cpp_int  & l__val);
        cpp_int db_qid_overflow_interrupt() const;
    
        typedef pu_cpp_int< 1 > host_ring_access_err_interrupt_cpp_int_t;
        cpp_int int_var__host_ring_access_err_interrupt;
        void host_ring_access_err_interrupt (const cpp_int  & l__val);
        cpp_int host_ring_access_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > total_ring_access_err_interrupt_cpp_int_t;
        cpp_int int_var__total_ring_access_err_interrupt;
        void total_ring_access_err_interrupt (const cpp_int  & l__val);
        cpp_int total_ring_access_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > rresp_err_interrupt_cpp_int_t;
        cpp_int int_var__rresp_err_interrupt;
        void rresp_err_interrupt (const cpp_int  & l__val);
        cpp_int rresp_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > bresp_err_interrupt_cpp_int_t;
        cpp_int int_var__bresp_err_interrupt;
        void bresp_err_interrupt (const cpp_int  & l__val);
        cpp_int bresp_err_interrupt() const;
    
}; // cap_wa_csr_int_db_intreg_t
    
class cap_wa_csr_int_db_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_int_db_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_int_db_t(string name = "cap_wa_csr_int_db_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_int_db_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_wa_csr_int_db_intreg_t intreg;
    
        cap_wa_csr_int_db_intreg_t int_test_set;
    
        cap_wa_csr_int_db_int_enable_clear_t int_enable_set;
    
        cap_wa_csr_int_db_int_enable_clear_t int_enable_clear;
    
}; // cap_wa_csr_int_db_t
    
class cap_wa_csr_intreg_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_intreg_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_intreg_status_t(string name = "cap_wa_csr_intreg_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_intreg_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > int_lif_qstate_map_interrupt_cpp_int_t;
        cpp_int int_var__int_lif_qstate_map_interrupt;
        void int_lif_qstate_map_interrupt (const cpp_int  & l__val);
        cpp_int int_lif_qstate_map_interrupt() const;
    
        typedef pu_cpp_int< 1 > int_db_interrupt_cpp_int_t;
        cpp_int int_var__int_db_interrupt;
        void int_db_interrupt (const cpp_int  & l__val);
        cpp_int int_db_interrupt() const;
    
}; // cap_wa_csr_intreg_status_t
    
class cap_wa_csr_int_groups_int_enable_rw_reg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_int_groups_int_enable_rw_reg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_int_groups_int_enable_rw_reg_t(string name = "cap_wa_csr_int_groups_int_enable_rw_reg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_int_groups_int_enable_rw_reg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > int_lif_qstate_map_enable_cpp_int_t;
        cpp_int int_var__int_lif_qstate_map_enable;
        void int_lif_qstate_map_enable (const cpp_int  & l__val);
        cpp_int int_lif_qstate_map_enable() const;
    
        typedef pu_cpp_int< 1 > int_db_enable_cpp_int_t;
        cpp_int int_var__int_db_enable;
        void int_db_enable (const cpp_int  & l__val);
        cpp_int int_db_enable() const;
    
}; // cap_wa_csr_int_groups_int_enable_rw_reg_t
    
class cap_wa_csr_intgrp_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_intgrp_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_intgrp_status_t(string name = "cap_wa_csr_intgrp_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_intgrp_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_wa_csr_intreg_status_t intreg;
    
        cap_wa_csr_int_groups_int_enable_rw_reg_t int_enable_rw_reg;
    
        cap_wa_csr_intreg_status_t int_rw_reg;
    
}; // cap_wa_csr_intgrp_status_t
    
class cap_wa_csr_sta_inval_cam_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_sta_inval_cam_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_sta_inval_cam_entry_t(string name = "cap_wa_csr_sta_inval_cam_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_sta_inval_cam_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_wa_csr_sta_inval_cam_entry_t
    
class cap_wa_csr_sta_inval_cam_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_sta_inval_cam_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_sta_inval_cam_t(string name = "cap_wa_csr_sta_inval_cam_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_sta_inval_cam_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
        cap_csr_large_array_wrapper<cap_wa_csr_sta_inval_cam_entry_t, 64> entry;
        #else 
        cap_wa_csr_sta_inval_cam_entry_t entry[64];
        #endif
        int get_depth_entry() { return 64; }
    
}; // cap_wa_csr_sta_inval_cam_t
    
class cap_wa_csr_filter_addr_ctl_value_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_filter_addr_ctl_value_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_filter_addr_ctl_value_t(string name = "cap_wa_csr_filter_addr_ctl_value_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_filter_addr_ctl_value_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_wa_csr_filter_addr_ctl_value_t
    
class cap_wa_csr_filter_addr_ctl_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_filter_addr_ctl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_filter_addr_ctl_t(string name = "cap_wa_csr_filter_addr_ctl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_filter_addr_ctl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
        cap_csr_large_array_wrapper<cap_wa_csr_filter_addr_ctl_value_t, 8> value;
        #else 
        cap_wa_csr_filter_addr_ctl_value_t value[8];
        #endif
        int get_depth_value() { return 8; }
    
}; // cap_wa_csr_filter_addr_ctl_t
    
class cap_wa_csr_filter_addr_hi_data_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_filter_addr_hi_data_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_filter_addr_hi_data_t(string name = "cap_wa_csr_filter_addr_hi_data_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_filter_addr_hi_data_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_wa_csr_filter_addr_hi_data_t
    
class cap_wa_csr_filter_addr_hi_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_filter_addr_hi_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_filter_addr_hi_t(string name = "cap_wa_csr_filter_addr_hi_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_filter_addr_hi_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
        cap_csr_large_array_wrapper<cap_wa_csr_filter_addr_hi_data_t, 8> data;
        #else 
        cap_wa_csr_filter_addr_hi_data_t data[8];
        #endif
        int get_depth_data() { return 8; }
    
}; // cap_wa_csr_filter_addr_hi_t
    
class cap_wa_csr_filter_addr_lo_data_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_filter_addr_lo_data_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_filter_addr_lo_data_t(string name = "cap_wa_csr_filter_addr_lo_data_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_filter_addr_lo_data_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_wa_csr_filter_addr_lo_data_t
    
class cap_wa_csr_filter_addr_lo_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_filter_addr_lo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_filter_addr_lo_t(string name = "cap_wa_csr_filter_addr_lo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_filter_addr_lo_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
        cap_csr_large_array_wrapper<cap_wa_csr_filter_addr_lo_data_t, 8> data;
        #else 
        cap_wa_csr_filter_addr_lo_data_t data[8];
        #endif
        int get_depth_data() { return 8; }
    
}; // cap_wa_csr_filter_addr_lo_t
    
class cap_wa_csr_dhs_lif_qstate_map_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_dhs_lif_qstate_map_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_dhs_lif_qstate_map_entry_t(string name = "cap_wa_csr_dhs_lif_qstate_map_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_dhs_lif_qstate_map_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_wa_csr_dhs_lif_qstate_map_entry_t
    
class cap_wa_csr_dhs_lif_qstate_map_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_dhs_lif_qstate_map_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_dhs_lif_qstate_map_t(string name = "cap_wa_csr_dhs_lif_qstate_map_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_dhs_lif_qstate_map_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
        cap_csr_large_array_wrapper<cap_wa_csr_dhs_lif_qstate_map_entry_t, 2048> entry;
        #else 
        cap_wa_csr_dhs_lif_qstate_map_entry_t entry[2048];
        #endif
        int get_depth_entry() { return 2048; }
    
}; // cap_wa_csr_dhs_lif_qstate_map_t
    
class cap_wa_csr_dhs_doorbell_err_activity_log_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_dhs_doorbell_err_activity_log_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_dhs_doorbell_err_activity_log_entry_t(string name = "cap_wa_csr_dhs_doorbell_err_activity_log_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_dhs_doorbell_err_activity_log_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 29 > qstateaddr_or_qid_cpp_int_t;
        cpp_int int_var__qstateaddr_or_qid;
        void qstateaddr_or_qid (const cpp_int  & l__val);
        cpp_int qstateaddr_or_qid() const;
    
        typedef pu_cpp_int< 16 > pid_or_lif_type_cpp_int_t;
        cpp_int int_var__pid_or_lif_type;
        void pid_or_lif_type (const cpp_int  & l__val);
        cpp_int pid_or_lif_type() const;
    
        typedef pu_cpp_int< 12 > cnt_cpp_int_t;
        cpp_int int_var__cnt;
        void cnt (const cpp_int  & l__val);
        cpp_int cnt() const;
    
        typedef pu_cpp_int< 1 > doorbell_merged_cpp_int_t;
        cpp_int int_var__doorbell_merged;
        void doorbell_merged (const cpp_int  & l__val);
        cpp_int doorbell_merged() const;
    
        typedef pu_cpp_int< 1 > addr_conflict_cpp_int_t;
        cpp_int int_var__addr_conflict;
        void addr_conflict (const cpp_int  & l__val);
        cpp_int addr_conflict() const;
    
        typedef pu_cpp_int< 1 > tot_ring_err_cpp_int_t;
        cpp_int int_var__tot_ring_err;
        void tot_ring_err (const cpp_int  & l__val);
        cpp_int tot_ring_err() const;
    
        typedef pu_cpp_int< 1 > host_ring_err_cpp_int_t;
        cpp_int int_var__host_ring_err;
        void host_ring_err (const cpp_int  & l__val);
        cpp_int host_ring_err() const;
    
        typedef pu_cpp_int< 1 > pid_fail_cpp_int_t;
        cpp_int int_var__pid_fail;
        void pid_fail (const cpp_int  & l__val);
        cpp_int pid_fail() const;
    
        typedef pu_cpp_int< 1 > qid_ovflow_cpp_int_t;
        cpp_int int_var__qid_ovflow;
        void qid_ovflow (const cpp_int  & l__val);
        cpp_int qid_ovflow() const;
    
}; // cap_wa_csr_dhs_doorbell_err_activity_log_entry_t
    
class cap_wa_csr_dhs_doorbell_err_activity_log_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_dhs_doorbell_err_activity_log_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_dhs_doorbell_err_activity_log_t(string name = "cap_wa_csr_dhs_doorbell_err_activity_log_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_dhs_doorbell_err_activity_log_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
        cap_csr_large_array_wrapper<cap_wa_csr_dhs_doorbell_err_activity_log_entry_t, 16> entry;
        #else 
        cap_wa_csr_dhs_doorbell_err_activity_log_entry_t entry[16];
        #endif
        int get_depth_entry() { return 16; }
    
}; // cap_wa_csr_dhs_doorbell_err_activity_log_t
    
class cap_wa_csr_dhs_page4k_remap_tbl_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_dhs_page4k_remap_tbl_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_dhs_page4k_remap_tbl_entry_t(string name = "cap_wa_csr_dhs_page4k_remap_tbl_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_dhs_page4k_remap_tbl_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > upd_vec_cpp_int_t;
        cpp_int int_var__upd_vec;
        void upd_vec (const cpp_int  & l__val);
        cpp_int upd_vec() const;
    
        typedef pu_cpp_int< 11 > lif_cpp_int_t;
        cpp_int int_var__lif;
        void lif (const cpp_int  & l__val);
        cpp_int lif() const;
    
        typedef pu_cpp_int< 16 > pid_cpp_int_t;
        cpp_int int_var__pid;
        void pid (const cpp_int  & l__val);
        cpp_int pid() const;
    
}; // cap_wa_csr_dhs_page4k_remap_tbl_entry_t
    
class cap_wa_csr_dhs_page4k_remap_tbl_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_dhs_page4k_remap_tbl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_dhs_page4k_remap_tbl_t(string name = "cap_wa_csr_dhs_page4k_remap_tbl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_dhs_page4k_remap_tbl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_wa_csr_dhs_page4k_remap_tbl_entry_t, 32> entry;
        #else 
        cap_wa_csr_dhs_page4k_remap_tbl_entry_t entry[32];
        #endif
        int get_depth_entry() { return 32; }
    
}; // cap_wa_csr_dhs_page4k_remap_tbl_t
    
class cap_wa_csr_dhs_page4k_remap_db_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_dhs_page4k_remap_db_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_dhs_page4k_remap_db_entry_t(string name = "cap_wa_csr_dhs_page4k_remap_db_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_dhs_page4k_remap_db_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > index_cpp_int_t;
        cpp_int int_var__index;
        void index (const cpp_int  & l__val);
        cpp_int index() const;
    
        typedef pu_cpp_int< 3 > ring_cpp_int_t;
        cpp_int int_var__ring;
        void ring (const cpp_int  & l__val);
        cpp_int ring() const;
    
        typedef pu_cpp_int< 5 > rsvd_cpp_int_t;
        cpp_int int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
        typedef pu_cpp_int< 24 > qid_cpp_int_t;
        cpp_int int_var__qid;
        void qid (const cpp_int  & l__val);
        cpp_int qid() const;
    
        typedef pu_cpp_int< 16 > rsvd4pid_cpp_int_t;
        cpp_int int_var__rsvd4pid;
        void rsvd4pid (const cpp_int  & l__val);
        cpp_int rsvd4pid() const;
    
}; // cap_wa_csr_dhs_page4k_remap_db_entry_t
    
class cap_wa_csr_dhs_page4k_remap_db_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_dhs_page4k_remap_db_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_dhs_page4k_remap_db_t(string name = "cap_wa_csr_dhs_page4k_remap_db_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_dhs_page4k_remap_db_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        #if 16384 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_wa_csr_dhs_page4k_remap_db_entry_t, 16384> entry;
        #else 
        cap_wa_csr_dhs_page4k_remap_db_entry_t entry[16384];
        #endif
        int get_depth_entry() { return 16384; }
    
}; // cap_wa_csr_dhs_page4k_remap_db_t
    
class cap_wa_csr_dhs_32b_doorbell_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_dhs_32b_doorbell_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_dhs_32b_doorbell_entry_t(string name = "cap_wa_csr_dhs_32b_doorbell_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_dhs_32b_doorbell_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > index_cpp_int_t;
        cpp_int int_var__index;
        void index (const cpp_int  & l__val);
        cpp_int index() const;
    
        typedef pu_cpp_int< 16 > qid_cpp_int_t;
        cpp_int int_var__qid;
        void qid (const cpp_int  & l__val);
        cpp_int qid() const;
    
}; // cap_wa_csr_dhs_32b_doorbell_entry_t
    
class cap_wa_csr_dhs_32b_doorbell_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_dhs_32b_doorbell_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_dhs_32b_doorbell_t(string name = "cap_wa_csr_dhs_32b_doorbell_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_dhs_32b_doorbell_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        #if 524288 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_wa_csr_dhs_32b_doorbell_entry_t, 524288> entry;
        #else 
        cap_wa_csr_dhs_32b_doorbell_entry_t entry[524288];
        #endif
        int get_depth_entry() { return 524288; }
    
}; // cap_wa_csr_dhs_32b_doorbell_t
    
class cap_wa_csr_dhs_local_doorbell_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_dhs_local_doorbell_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_dhs_local_doorbell_entry_t(string name = "cap_wa_csr_dhs_local_doorbell_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_dhs_local_doorbell_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > index_cpp_int_t;
        cpp_int int_var__index;
        void index (const cpp_int  & l__val);
        cpp_int index() const;
    
        typedef pu_cpp_int< 3 > ring_cpp_int_t;
        cpp_int int_var__ring;
        void ring (const cpp_int  & l__val);
        cpp_int ring() const;
    
        typedef pu_cpp_int< 5 > rsvd_cpp_int_t;
        cpp_int int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
        typedef pu_cpp_int< 24 > qid_cpp_int_t;
        cpp_int int_var__qid;
        void qid (const cpp_int  & l__val);
        cpp_int qid() const;
    
        typedef pu_cpp_int< 16 > pid_cpp_int_t;
        cpp_int int_var__pid;
        void pid (const cpp_int  & l__val);
        cpp_int pid() const;
    
}; // cap_wa_csr_dhs_local_doorbell_entry_t
    
class cap_wa_csr_dhs_local_doorbell_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_dhs_local_doorbell_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_dhs_local_doorbell_t(string name = "cap_wa_csr_dhs_local_doorbell_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_dhs_local_doorbell_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        #if 524288 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_wa_csr_dhs_local_doorbell_entry_t, 524288> entry;
        #else 
        cap_wa_csr_dhs_local_doorbell_entry_t entry[524288];
        #endif
        int get_depth_entry() { return 524288; }
    
}; // cap_wa_csr_dhs_local_doorbell_t
    
class cap_wa_csr_dhs_host_doorbell_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_dhs_host_doorbell_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_dhs_host_doorbell_entry_t(string name = "cap_wa_csr_dhs_host_doorbell_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_dhs_host_doorbell_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > index_cpp_int_t;
        cpp_int int_var__index;
        void index (const cpp_int  & l__val);
        cpp_int index() const;
    
        typedef pu_cpp_int< 3 > ring_cpp_int_t;
        cpp_int int_var__ring;
        void ring (const cpp_int  & l__val);
        cpp_int ring() const;
    
        typedef pu_cpp_int< 5 > rsvd_cpp_int_t;
        cpp_int int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
        typedef pu_cpp_int< 24 > qid_cpp_int_t;
        cpp_int int_var__qid;
        void qid (const cpp_int  & l__val);
        cpp_int qid() const;
    
        typedef pu_cpp_int< 16 > pid_cpp_int_t;
        cpp_int int_var__pid;
        void pid (const cpp_int  & l__val);
        cpp_int pid() const;
    
}; // cap_wa_csr_dhs_host_doorbell_entry_t
    
class cap_wa_csr_dhs_host_doorbell_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_dhs_host_doorbell_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_dhs_host_doorbell_t(string name = "cap_wa_csr_dhs_host_doorbell_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_dhs_host_doorbell_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        #if 524288 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_wa_csr_dhs_host_doorbell_entry_t, 524288> entry;
        #else 
        cap_wa_csr_dhs_host_doorbell_entry_t entry[524288];
        #endif
        int get_depth_entry() { return 524288; }
    
}; // cap_wa_csr_dhs_host_doorbell_t
    
class cap_wa_csr_cfg_filter_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_cfg_filter_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_cfg_filter_t(string name = "cap_wa_csr_cfg_filter_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_cfg_filter_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_wa_csr_cfg_filter_t
    
class cap_wa_csr_sta_lif_table_sram_bist_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_sta_lif_table_sram_bist_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_sta_lif_table_sram_bist_t(string name = "cap_wa_csr_sta_lif_table_sram_bist_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_sta_lif_table_sram_bist_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_wa_csr_sta_lif_table_sram_bist_t
    
class cap_wa_csr_cfg_lif_table_sram_bist_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_cfg_lif_table_sram_bist_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_cfg_lif_table_sram_bist_t(string name = "cap_wa_csr_cfg_lif_table_sram_bist_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_cfg_lif_table_sram_bist_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_wa_csr_cfg_lif_table_sram_bist_t
    
class cap_wa_csr_sta_ecc_lif_qstate_map_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_sta_ecc_lif_qstate_map_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_sta_ecc_lif_qstate_map_t(string name = "cap_wa_csr_sta_ecc_lif_qstate_map_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_sta_ecc_lif_qstate_map_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_wa_csr_sta_ecc_lif_qstate_map_t
    
class cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t(string name = "cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t
    
class cap_wa_csr_csr_intr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_csr_intr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_csr_intr_t(string name = "cap_wa_csr_csr_intr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_csr_intr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_wa_csr_csr_intr_t
    
class cap_wa_csr_cfg_debug_port_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_cfg_debug_port_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_cfg_debug_port_t(string name = "cap_wa_csr_cfg_debug_port_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_cfg_debug_port_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 2 > select_cpp_int_t;
        cpp_int int_var__select;
        void select (const cpp_int  & l__val);
        cpp_int select() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        cpp_int int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
}; // cap_wa_csr_cfg_debug_port_t
    
class cap_wa_csr_cfg_doorbell_axi_attr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_cfg_doorbell_axi_attr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_cfg_doorbell_axi_attr_t(string name = "cap_wa_csr_cfg_doorbell_axi_attr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_cfg_doorbell_axi_attr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_wa_csr_cfg_doorbell_axi_attr_t
    
class cap_wa_csr_cnt_wa_sched_out_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_cnt_wa_sched_out_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_cnt_wa_sched_out_t(string name = "cap_wa_csr_cnt_wa_sched_out_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_cnt_wa_sched_out_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_wa_csr_cnt_wa_sched_out_t
    
class cap_wa_csr_cnt_wa_arm4k_doorbells_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_cnt_wa_arm4k_doorbells_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_cnt_wa_arm4k_doorbells_t(string name = "cap_wa_csr_cnt_wa_arm4k_doorbells_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_cnt_wa_arm4k_doorbells_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_wa_csr_cnt_wa_arm4k_doorbells_t
    
class cap_wa_csr_cnt_wa_timer_doorbells_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_cnt_wa_timer_doorbells_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_cnt_wa_timer_doorbells_t(string name = "cap_wa_csr_cnt_wa_timer_doorbells_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_cnt_wa_timer_doorbells_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_wa_csr_cnt_wa_timer_doorbells_t
    
class cap_wa_csr_cnt_wa_32b_doorbells_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_cnt_wa_32b_doorbells_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_cnt_wa_32b_doorbells_t(string name = "cap_wa_csr_cnt_wa_32b_doorbells_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_cnt_wa_32b_doorbells_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_wa_csr_cnt_wa_32b_doorbells_t
    
class cap_wa_csr_cnt_wa_local_doorbells_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_cnt_wa_local_doorbells_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_cnt_wa_local_doorbells_t(string name = "cap_wa_csr_cnt_wa_local_doorbells_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_cnt_wa_local_doorbells_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_wa_csr_cnt_wa_local_doorbells_t
    
class cap_wa_csr_cnt_wa_host_doorbells_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_cnt_wa_host_doorbells_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_cnt_wa_host_doorbells_t(string name = "cap_wa_csr_cnt_wa_host_doorbells_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_cnt_wa_host_doorbells_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_wa_csr_cnt_wa_host_doorbells_t
    
class cap_wa_csr_sat_wa_axi_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_sat_wa_axi_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_sat_wa_axi_err_t(string name = "cap_wa_csr_sat_wa_axi_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_sat_wa_axi_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > rresp_cpp_int_t;
        cpp_int int_var__rresp;
        void rresp (const cpp_int  & l__val);
        cpp_int rresp() const;
    
        typedef pu_cpp_int< 8 > bresp_cpp_int_t;
        cpp_int int_var__bresp;
        void bresp (const cpp_int  & l__val);
        cpp_int bresp() const;
    
}; // cap_wa_csr_sat_wa_axi_err_t
    
class cap_wa_csr_sat_wa_pid_chkfail_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_sat_wa_pid_chkfail_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_sat_wa_pid_chkfail_t(string name = "cap_wa_csr_sat_wa_pid_chkfail_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_sat_wa_pid_chkfail_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > cnt_cpp_int_t;
        cpp_int int_var__cnt;
        void cnt (const cpp_int  & l__val);
        cpp_int cnt() const;
    
}; // cap_wa_csr_sat_wa_pid_chkfail_t
    
class cap_wa_csr_sat_wa_qid_overflow_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_sat_wa_qid_overflow_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_sat_wa_qid_overflow_t(string name = "cap_wa_csr_sat_wa_qid_overflow_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_sat_wa_qid_overflow_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > cnt_cpp_int_t;
        cpp_int int_var__cnt;
        void cnt (const cpp_int  & l__val);
        cpp_int cnt() const;
    
}; // cap_wa_csr_sat_wa_qid_overflow_t
    
class cap_wa_csr_sat_wa_merged_pre_axi_read_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_sat_wa_merged_pre_axi_read_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_sat_wa_merged_pre_axi_read_t(string name = "cap_wa_csr_sat_wa_merged_pre_axi_read_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_sat_wa_merged_pre_axi_read_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > cnt_cpp_int_t;
        cpp_int int_var__cnt;
        void cnt (const cpp_int  & l__val);
        cpp_int cnt() const;
    
}; // cap_wa_csr_sat_wa_merged_pre_axi_read_t
    
class cap_wa_csr_sat_wa_merged_inflight_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_sat_wa_merged_inflight_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_sat_wa_merged_inflight_t(string name = "cap_wa_csr_sat_wa_merged_inflight_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_sat_wa_merged_inflight_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > cnt_cpp_int_t;
        cpp_int int_var__cnt;
        void cnt (const cpp_int  & l__val);
        cpp_int cnt() const;
    
}; // cap_wa_csr_sat_wa_merged_inflight_t
    
class cap_wa_csr_sat_wa_qaddr_cam_conflict_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_sat_wa_qaddr_cam_conflict_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_sat_wa_qaddr_cam_conflict_t(string name = "cap_wa_csr_sat_wa_qaddr_cam_conflict_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_sat_wa_qaddr_cam_conflict_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > cnt_cpp_int_t;
        cpp_int int_var__cnt;
        void cnt (const cpp_int  & l__val);
        cpp_int cnt() const;
    
}; // cap_wa_csr_sat_wa_qaddr_cam_conflict_t
    
class cap_wa_csr_sat_wa_ring_access_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_sat_wa_ring_access_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_sat_wa_ring_access_err_t(string name = "cap_wa_csr_sat_wa_ring_access_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_sat_wa_ring_access_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > cnt_cpp_int_t;
        cpp_int int_var__cnt;
        void cnt (const cpp_int  & l__val);
        cpp_int cnt() const;
    
}; // cap_wa_csr_sat_wa_ring_access_err_t
    
class cap_wa_csr_sat_wa_host_access_err_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_sat_wa_host_access_err_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_sat_wa_host_access_err_t(string name = "cap_wa_csr_sat_wa_host_access_err_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_sat_wa_host_access_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > cnt_cpp_int_t;
        cpp_int int_var__cnt;
        void cnt (const cpp_int  & l__val);
        cpp_int cnt() const;
    
}; // cap_wa_csr_sat_wa_host_access_err_t
    
class cap_wa_csr_sta_wa_axi_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_sta_wa_axi_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_sta_wa_axi_t(string name = "cap_wa_csr_sta_wa_axi_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_sta_wa_axi_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > num_ids_cpp_int_t;
        cpp_int int_var__num_ids;
        void num_ids (const cpp_int  & l__val);
        cpp_int num_ids() const;
    
}; // cap_wa_csr_sta_wa_axi_t
    
class cap_wa_csr_cfg_wa_merge_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_cfg_wa_merge_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_cfg_wa_merge_t(string name = "cap_wa_csr_cfg_wa_merge_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_cfg_wa_merge_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > inflight_en_cpp_int_t;
        cpp_int int_var__inflight_en;
        void inflight_en (const cpp_int  & l__val);
        cpp_int inflight_en() const;
    
        typedef pu_cpp_int< 1 > pre_axi_read_en_cpp_int_t;
        cpp_int int_var__pre_axi_read_en;
        void pre_axi_read_en (const cpp_int  & l__val);
        cpp_int pre_axi_read_en() const;
    
}; // cap_wa_csr_cfg_wa_merge_t
    
class cap_wa_csr_cfg_wa_sched_hint_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_cfg_wa_sched_hint_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_cfg_wa_sched_hint_t(string name = "cap_wa_csr_cfg_wa_sched_hint_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_cfg_wa_sched_hint_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 5 > enable_src_mask_cpp_int_t;
        cpp_int int_var__enable_src_mask;
        void enable_src_mask (const cpp_int  & l__val);
        cpp_int enable_src_mask() const;
    
}; // cap_wa_csr_cfg_wa_sched_hint_t
    
class cap_wa_csr_cfg_wa_axi_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_cfg_wa_axi_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_cfg_wa_axi_t(string name = "cap_wa_csr_cfg_wa_axi_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_cfg_wa_axi_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > num_ids_cpp_int_t;
        cpp_int int_var__num_ids;
        void num_ids (const cpp_int  & l__val);
        cpp_int num_ids() const;
    
}; // cap_wa_csr_cfg_wa_axi_t
    
class cap_wa_csr_rdintr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_rdintr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_rdintr_t(string name = "cap_wa_csr_rdintr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_rdintr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_wa_csr_rdintr_t
    
class cap_wa_csr_base_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_base_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_base_t(string name = "cap_wa_csr_base_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_base_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_wa_csr_base_t
    
class cap_wa_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_wa_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_wa_csr_t(string name = "cap_wa_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_wa_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_wa_csr_base_t base;
    
        cap_wa_csr_rdintr_t rdintr;
    
        cap_wa_csr_cfg_wa_axi_t cfg_wa_axi;
    
        cap_wa_csr_cfg_wa_sched_hint_t cfg_wa_sched_hint;
    
        cap_wa_csr_cfg_wa_merge_t cfg_wa_merge;
    
        cap_wa_csr_sta_wa_axi_t sta_wa_axi;
    
        cap_wa_csr_sat_wa_host_access_err_t sat_wa_host_access_err;
    
        cap_wa_csr_sat_wa_ring_access_err_t sat_wa_ring_access_err;
    
        cap_wa_csr_sat_wa_qaddr_cam_conflict_t sat_wa_qaddr_cam_conflict;
    
        cap_wa_csr_sat_wa_merged_inflight_t sat_wa_merged_inflight;
    
        cap_wa_csr_sat_wa_merged_pre_axi_read_t sat_wa_merged_pre_axi_read;
    
        cap_wa_csr_sat_wa_qid_overflow_t sat_wa_qid_overflow;
    
        cap_wa_csr_sat_wa_pid_chkfail_t sat_wa_pid_chkfail;
    
        cap_wa_csr_sat_wa_axi_err_t sat_wa_axi_err;
    
        cap_wa_csr_cnt_wa_host_doorbells_t cnt_wa_host_doorbells;
    
        cap_wa_csr_cnt_wa_local_doorbells_t cnt_wa_local_doorbells;
    
        cap_wa_csr_cnt_wa_32b_doorbells_t cnt_wa_32b_doorbells;
    
        cap_wa_csr_cnt_wa_timer_doorbells_t cnt_wa_timer_doorbells;
    
        cap_wa_csr_cnt_wa_arm4k_doorbells_t cnt_wa_arm4k_doorbells;
    
        cap_wa_csr_cnt_wa_sched_out_t cnt_wa_sched_out;
    
        cap_wa_csr_cfg_doorbell_axi_attr_t cfg_doorbell_axi_attr;
    
        cap_wa_csr_cfg_debug_port_t cfg_debug_port;
    
        cap_wa_csr_csr_intr_t csr_intr;
    
        cap_wa_csr_cfg_ecc_disable_lif_qstate_map_t cfg_ecc_disable_lif_qstate_map;
    
        cap_wa_csr_sta_ecc_lif_qstate_map_t sta_ecc_lif_qstate_map;
    
        cap_wa_csr_cfg_lif_table_sram_bist_t cfg_lif_table_sram_bist;
    
        cap_wa_csr_sta_lif_table_sram_bist_t sta_lif_table_sram_bist;
    
        cap_wa_csr_cfg_filter_t cfg_filter;
    
        cap_wa_csr_dhs_host_doorbell_t dhs_host_doorbell;
    
        cap_wa_csr_dhs_local_doorbell_t dhs_local_doorbell;
    
        cap_wa_csr_dhs_32b_doorbell_t dhs_32b_doorbell;
    
        cap_wa_csr_dhs_page4k_remap_db_t dhs_page4k_remap_db;
    
        cap_wa_csr_dhs_page4k_remap_tbl_t dhs_page4k_remap_tbl;
    
        cap_wa_csr_dhs_doorbell_err_activity_log_t dhs_doorbell_err_activity_log;
    
        cap_wa_csr_dhs_lif_qstate_map_t dhs_lif_qstate_map;
    
        cap_wa_csr_filter_addr_lo_t filter_addr_lo;
    
        cap_wa_csr_filter_addr_hi_t filter_addr_hi;
    
        cap_wa_csr_filter_addr_ctl_t filter_addr_ctl;
    
        cap_wa_csr_sta_inval_cam_t sta_inval_cam;
    
        cap_wa_csr_intgrp_status_t int_groups;
    
        cap_wa_csr_int_db_t int_db;
    
        cap_wa_csr_int_lif_qstate_map_t int_lif_qstate_map;
    
}; // cap_wa_csr_t
    
#endif // CAP_WA_CSR_H
        