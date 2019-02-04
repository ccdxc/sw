
#ifndef CAP_BX_CSR_H
#define CAP_BX_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_bx_csr_int_mac_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_bx_csr_int_mac_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_bx_csr_int_mac_int_enable_clear_t(string name = "cap_bx_csr_int_mac_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_bx_csr_int_mac_int_enable_clear_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > lane_sbe_enable_cpp_int_t;
        cpp_int int_var__lane_sbe_enable;
        void lane_sbe_enable (const cpp_int  & l__val);
        cpp_int lane_sbe_enable() const;
    
        typedef pu_cpp_int< 1 > lane_dbe_enable_cpp_int_t;
        cpp_int int_var__lane_dbe_enable;
        void lane_dbe_enable (const cpp_int  & l__val);
        cpp_int lane_dbe_enable() const;
    
        typedef pu_cpp_int< 1 > xdmac10_intr_enable_cpp_int_t;
        cpp_int int_var__xdmac10_intr_enable;
        void xdmac10_intr_enable (const cpp_int  & l__val);
        cpp_int xdmac10_intr_enable() const;
    
        typedef pu_cpp_int< 1 > xdmac10_pslverr_enable_cpp_int_t;
        cpp_int int_var__xdmac10_pslverr_enable;
        void xdmac10_pslverr_enable (const cpp_int  & l__val);
        cpp_int xdmac10_pslverr_enable() const;
    
        typedef pu_cpp_int< 1 > bxpb_pbus_drdy_enable_cpp_int_t;
        cpp_int int_var__bxpb_pbus_drdy_enable;
        void bxpb_pbus_drdy_enable (const cpp_int  & l__val);
        cpp_int bxpb_pbus_drdy_enable() const;
    
        typedef pu_cpp_int< 1 > rx_missing_sof_enable_cpp_int_t;
        cpp_int int_var__rx_missing_sof_enable;
        void rx_missing_sof_enable (const cpp_int  & l__val);
        cpp_int rx_missing_sof_enable() const;
    
        typedef pu_cpp_int< 1 > rx_missing_eof_enable_cpp_int_t;
        cpp_int int_var__rx_missing_eof_enable;
        void rx_missing_eof_enable (const cpp_int  & l__val);
        cpp_int rx_missing_eof_enable() const;
    
        typedef pu_cpp_int< 1 > rx_timeout_err_enable_cpp_int_t;
        cpp_int int_var__rx_timeout_err_enable;
        void rx_timeout_err_enable (const cpp_int  & l__val);
        cpp_int rx_timeout_err_enable() const;
    
        typedef pu_cpp_int< 1 > rx_min_size_err_enable_cpp_int_t;
        cpp_int int_var__rx_min_size_err_enable;
        void rx_min_size_err_enable (const cpp_int  & l__val);
        cpp_int rx_min_size_err_enable() const;
    
}; // cap_bx_csr_int_mac_int_enable_clear_t
    
class cap_bx_csr_intreg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_bx_csr_intreg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_bx_csr_intreg_t(string name = "cap_bx_csr_intreg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_bx_csr_intreg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > lane_sbe_interrupt_cpp_int_t;
        cpp_int int_var__lane_sbe_interrupt;
        void lane_sbe_interrupt (const cpp_int  & l__val);
        cpp_int lane_sbe_interrupt() const;
    
        typedef pu_cpp_int< 1 > lane_dbe_interrupt_cpp_int_t;
        cpp_int int_var__lane_dbe_interrupt;
        void lane_dbe_interrupt (const cpp_int  & l__val);
        cpp_int lane_dbe_interrupt() const;
    
        typedef pu_cpp_int< 1 > xdmac10_intr_interrupt_cpp_int_t;
        cpp_int int_var__xdmac10_intr_interrupt;
        void xdmac10_intr_interrupt (const cpp_int  & l__val);
        cpp_int xdmac10_intr_interrupt() const;
    
        typedef pu_cpp_int< 1 > xdmac10_pslverr_interrupt_cpp_int_t;
        cpp_int int_var__xdmac10_pslverr_interrupt;
        void xdmac10_pslverr_interrupt (const cpp_int  & l__val);
        cpp_int xdmac10_pslverr_interrupt() const;
    
        typedef pu_cpp_int< 1 > bxpb_pbus_drdy_interrupt_cpp_int_t;
        cpp_int int_var__bxpb_pbus_drdy_interrupt;
        void bxpb_pbus_drdy_interrupt (const cpp_int  & l__val);
        cpp_int bxpb_pbus_drdy_interrupt() const;
    
        typedef pu_cpp_int< 1 > rx_missing_sof_interrupt_cpp_int_t;
        cpp_int int_var__rx_missing_sof_interrupt;
        void rx_missing_sof_interrupt (const cpp_int  & l__val);
        cpp_int rx_missing_sof_interrupt() const;
    
        typedef pu_cpp_int< 1 > rx_missing_eof_interrupt_cpp_int_t;
        cpp_int int_var__rx_missing_eof_interrupt;
        void rx_missing_eof_interrupt (const cpp_int  & l__val);
        cpp_int rx_missing_eof_interrupt() const;
    
        typedef pu_cpp_int< 1 > rx_timeout_err_interrupt_cpp_int_t;
        cpp_int int_var__rx_timeout_err_interrupt;
        void rx_timeout_err_interrupt (const cpp_int  & l__val);
        cpp_int rx_timeout_err_interrupt() const;
    
        typedef pu_cpp_int< 1 > rx_min_size_err_interrupt_cpp_int_t;
        cpp_int int_var__rx_min_size_err_interrupt;
        void rx_min_size_err_interrupt (const cpp_int  & l__val);
        cpp_int rx_min_size_err_interrupt() const;
    
}; // cap_bx_csr_intreg_t
    
class cap_bx_csr_intgrp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_bx_csr_intgrp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_bx_csr_intgrp_t(string name = "cap_bx_csr_intgrp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_bx_csr_intgrp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_bx_csr_intreg_t intreg;
    
        cap_bx_csr_intreg_t int_test_set;
    
        cap_bx_csr_int_mac_int_enable_clear_t int_enable_set;
    
        cap_bx_csr_int_mac_int_enable_clear_t int_enable_clear;
    
}; // cap_bx_csr_intgrp_t
    
class cap_bx_csr_intreg_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_bx_csr_intreg_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_bx_csr_intreg_status_t(string name = "cap_bx_csr_intreg_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_bx_csr_intreg_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_bx_csr_intreg_status_t
    
class cap_bx_csr_int_groups_int_enable_rw_reg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_bx_csr_int_groups_int_enable_rw_reg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_bx_csr_int_groups_int_enable_rw_reg_t(string name = "cap_bx_csr_int_groups_int_enable_rw_reg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_bx_csr_int_groups_int_enable_rw_reg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_bx_csr_int_groups_int_enable_rw_reg_t
    
class cap_bx_csr_intgrp_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_bx_csr_intgrp_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_bx_csr_intgrp_status_t(string name = "cap_bx_csr_intgrp_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_bx_csr_intgrp_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_bx_csr_intreg_status_t intreg;
    
        cap_bx_csr_int_groups_int_enable_rw_reg_t int_enable_rw_reg;
    
        cap_bx_csr_intreg_status_t int_rw_reg;
    
}; // cap_bx_csr_intgrp_status_t
    
class cap_bx_csr_dhs_apb_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_bx_csr_dhs_apb_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_bx_csr_dhs_apb_entry_t(string name = "cap_bx_csr_dhs_apb_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_bx_csr_dhs_apb_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_bx_csr_dhs_apb_entry_t
    
class cap_bx_csr_dhs_apb_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_bx_csr_dhs_apb_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_bx_csr_dhs_apb_t(string name = "cap_bx_csr_dhs_apb_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_bx_csr_dhs_apb_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
        cap_csr_large_array_wrapper<cap_bx_csr_dhs_apb_entry_t, 65536> entry;
        #else 
        cap_bx_csr_dhs_apb_entry_t entry[65536];
        #endif
        int get_depth_entry() { return 65536; }
    
}; // cap_bx_csr_dhs_apb_t
    
class cap_bx_csr_dhs_mac_stats_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_bx_csr_dhs_mac_stats_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_bx_csr_dhs_mac_stats_entry_t(string name = "cap_bx_csr_dhs_mac_stats_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_bx_csr_dhs_mac_stats_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_bx_csr_dhs_mac_stats_entry_t
    
class cap_bx_csr_dhs_mac_stats_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_bx_csr_dhs_mac_stats_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_bx_csr_dhs_mac_stats_t(string name = "cap_bx_csr_dhs_mac_stats_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_bx_csr_dhs_mac_stats_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
        cap_csr_large_array_wrapper<cap_bx_csr_dhs_mac_stats_entry_t, 32> entry;
        #else 
        cap_bx_csr_dhs_mac_stats_entry_t entry[32];
        #endif
        int get_depth_entry() { return 32; }
    
}; // cap_bx_csr_dhs_mac_stats_t
    
class cap_bx_csr_sta_stats_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_bx_csr_sta_stats_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_bx_csr_sta_stats_mem_t(string name = "cap_bx_csr_sta_stats_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_bx_csr_sta_stats_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_bx_csr_sta_stats_mem_t
    
class cap_bx_csr_cfg_stats_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_bx_csr_cfg_stats_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_bx_csr_cfg_stats_mem_t(string name = "cap_bx_csr_cfg_stats_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_bx_csr_cfg_stats_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_bx_csr_cfg_stats_mem_t
    
class cap_bx_csr_sta_ff_rxfifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_bx_csr_sta_ff_rxfifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_bx_csr_sta_ff_rxfifo_t(string name = "cap_bx_csr_sta_ff_rxfifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_bx_csr_sta_ff_rxfifo_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_bx_csr_sta_ff_rxfifo_t
    
class cap_bx_csr_cfg_ff_rxfifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_bx_csr_cfg_ff_rxfifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_bx_csr_cfg_ff_rxfifo_t(string name = "cap_bx_csr_cfg_ff_rxfifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_bx_csr_cfg_ff_rxfifo_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_bx_csr_cfg_ff_rxfifo_t
    
class cap_bx_csr_sta_ff_txfifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_bx_csr_sta_ff_txfifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_bx_csr_sta_ff_txfifo_t(string name = "cap_bx_csr_sta_ff_txfifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_bx_csr_sta_ff_txfifo_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_bx_csr_sta_ff_txfifo_t
    
class cap_bx_csr_cfg_ff_txfifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_bx_csr_cfg_ff_txfifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_bx_csr_cfg_ff_txfifo_t(string name = "cap_bx_csr_cfg_ff_txfifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_bx_csr_cfg_ff_txfifo_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_bx_csr_cfg_ff_txfifo_t
    
class cap_bx_csr_cfg_debug_port_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_bx_csr_cfg_debug_port_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_bx_csr_cfg_debug_port_t(string name = "cap_bx_csr_cfg_debug_port_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_bx_csr_cfg_debug_port_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_bx_csr_cfg_debug_port_t
    
class cap_bx_csr_cfg_fixer_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_bx_csr_cfg_fixer_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_bx_csr_cfg_fixer_t(string name = "cap_bx_csr_cfg_fixer_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_bx_csr_cfg_fixer_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_bx_csr_cfg_fixer_t
    
class cap_bx_csr_csr_intr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_bx_csr_csr_intr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_bx_csr_csr_intr_t(string name = "cap_bx_csr_csr_intr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_bx_csr_csr_intr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_bx_csr_csr_intr_t
    
class cap_bx_csr_sta_mac_sd_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_bx_csr_sta_mac_sd_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_bx_csr_sta_mac_sd_status_t(string name = "cap_bx_csr_sta_mac_sd_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_bx_csr_sta_mac_sd_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_bx_csr_sta_mac_sd_status_t
    
class cap_bx_csr_sta_mac_sd_interrupt_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_bx_csr_sta_mac_sd_interrupt_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_bx_csr_sta_mac_sd_interrupt_status_t(string name = "cap_bx_csr_sta_mac_sd_interrupt_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_bx_csr_sta_mac_sd_interrupt_status_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_bx_csr_sta_mac_sd_interrupt_status_t
    
class cap_bx_csr_cfg_mac_sd_core_to_cntl_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_bx_csr_cfg_mac_sd_core_to_cntl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_bx_csr_cfg_mac_sd_core_to_cntl_t(string name = "cap_bx_csr_cfg_mac_sd_core_to_cntl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_bx_csr_cfg_mac_sd_core_to_cntl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_bx_csr_cfg_mac_sd_core_to_cntl_t
    
class cap_bx_csr_cfg_mac_sd_interrupt_request_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_bx_csr_cfg_mac_sd_interrupt_request_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_bx_csr_cfg_mac_sd_interrupt_request_t(string name = "cap_bx_csr_cfg_mac_sd_interrupt_request_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_bx_csr_cfg_mac_sd_interrupt_request_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > lane_cpp_int_t;
        cpp_int int_var__lane;
        void lane (const cpp_int  & l__val);
        cpp_int lane() const;
    
}; // cap_bx_csr_cfg_mac_sd_interrupt_request_t
    
class cap_bx_csr_cfg_mac_sd_interrupt_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_bx_csr_cfg_mac_sd_interrupt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_bx_csr_cfg_mac_sd_interrupt_t(string name = "cap_bx_csr_cfg_mac_sd_interrupt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_bx_csr_cfg_mac_sd_interrupt_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_bx_csr_cfg_mac_sd_interrupt_t
    
class cap_bx_csr_cfg_mac_xoff_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_bx_csr_cfg_mac_xoff_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_bx_csr_cfg_mac_xoff_t(string name = "cap_bx_csr_cfg_mac_xoff_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_bx_csr_cfg_mac_xoff_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > ff_txfcxoff_i_cpp_int_t;
        cpp_int int_var__ff_txfcxoff_i;
        void ff_txfcxoff_i (const cpp_int  & l__val);
        cpp_int ff_txfcxoff_i() const;
    
        typedef pu_cpp_int< 8 > ff_txpfcxoff_i_cpp_int_t;
        cpp_int int_var__ff_txpfcxoff_i;
        void ff_txpfcxoff_i (const cpp_int  & l__val);
        cpp_int ff_txpfcxoff_i() const;
    
}; // cap_bx_csr_cfg_mac_xoff_t
    
class cap_bx_csr_sta_mac_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_bx_csr_sta_mac_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_bx_csr_sta_mac_t(string name = "cap_bx_csr_sta_mac_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_bx_csr_sta_mac_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > ff_txafull_o_cpp_int_t;
        cpp_int int_var__ff_txafull_o;
        void ff_txafull_o (const cpp_int  & l__val);
        cpp_int ff_txafull_o() const;
    
        typedef pu_cpp_int< 1 > ff_serdessigok_o_cpp_int_t;
        cpp_int int_var__ff_serdessigok_o;
        void ff_serdessigok_o (const cpp_int  & l__val);
        cpp_int ff_serdessigok_o() const;
    
        typedef pu_cpp_int< 1 > ff_txidle_o_cpp_int_t;
        cpp_int int_var__ff_txidle_o;
        void ff_txidle_o (const cpp_int  & l__val);
        cpp_int ff_txidle_o() const;
    
        typedef pu_cpp_int< 1 > ff_txgood_o_cpp_int_t;
        cpp_int int_var__ff_txgood_o;
        void ff_txgood_o (const cpp_int  & l__val);
        cpp_int ff_txgood_o() const;
    
        typedef pu_cpp_int< 1 > ff_txread_o_cpp_int_t;
        cpp_int int_var__ff_txread_o;
        void ff_txread_o (const cpp_int  & l__val);
        cpp_int ff_txread_o() const;
    
        typedef pu_cpp_int< 1 > ff_rxsync_o_cpp_int_t;
        cpp_int int_var__ff_rxsync_o;
        void ff_rxsync_o (const cpp_int  & l__val);
        cpp_int ff_rxsync_o() const;
    
        typedef pu_cpp_int< 8 > ff_rxpfcxoff_o_cpp_int_t;
        cpp_int int_var__ff_rxpfcxoff_o;
        void ff_rxpfcxoff_o (const cpp_int  & l__val);
        cpp_int ff_rxpfcxoff_o() const;
    
        typedef pu_cpp_int< 56 > ff_rxpreamble_o_cpp_int_t;
        cpp_int int_var__ff_rxpreamble_o;
        void ff_rxpreamble_o (const cpp_int  & l__val);
        cpp_int ff_rxpreamble_o() const;
    
}; // cap_bx_csr_sta_mac_t
    
class cap_bx_csr_cfg_serdes_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_bx_csr_cfg_serdes_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_bx_csr_cfg_serdes_t(string name = "cap_bx_csr_cfg_serdes_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_bx_csr_cfg_serdes_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > rxsigok_i_sel_cpp_int_t;
        cpp_int int_var__rxsigok_i_sel;
        void rxsigok_i_sel (const cpp_int  & l__val);
        cpp_int rxsigok_i_sel() const;
    
        typedef pu_cpp_int< 1 > rxsigok_i_cpp_int_t;
        cpp_int int_var__rxsigok_i;
        void rxsigok_i (const cpp_int  & l__val);
        cpp_int rxsigok_i() const;
    
        typedef pu_cpp_int< 1 > tx_override_in_cpp_int_t;
        cpp_int int_var__tx_override_in;
        void tx_override_in (const cpp_int  & l__val);
        cpp_int tx_override_in() const;
    
}; // cap_bx_csr_cfg_serdes_t
    
class cap_bx_csr_cfg_mac_gbl_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_bx_csr_cfg_mac_gbl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_bx_csr_cfg_mac_gbl_t(string name = "cap_bx_csr_cfg_mac_gbl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_bx_csr_cfg_mac_gbl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 56 > ff_txpreamble_i_cpp_int_t;
        cpp_int int_var__ff_txpreamble_i;
        void ff_txpreamble_i (const cpp_int  & l__val);
        cpp_int ff_txpreamble_i() const;
    
        typedef pu_cpp_int< 1 > rx_err_gen_enable_cpp_int_t;
        cpp_int int_var__rx_err_gen_enable;
        void rx_err_gen_enable (const cpp_int  & l__val);
        cpp_int rx_err_gen_enable() const;
    
        typedef pu_cpp_int< 7 > rx_err_mask_cpp_int_t;
        cpp_int int_var__rx_err_mask;
        void rx_err_mask (const cpp_int  & l__val);
        cpp_int rx_err_mask() const;
    
        typedef pu_cpp_int< 1 > umacreg_rxdisfcschk_cpp_int_t;
        cpp_int int_var__umacreg_rxdisfcschk;
        void umacreg_rxdisfcschk (const cpp_int  & l__val);
        cpp_int umacreg_rxdisfcschk() const;
    
}; // cap_bx_csr_cfg_mac_gbl_t
    
class cap_bx_csr_base_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_bx_csr_base_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_bx_csr_base_t(string name = "cap_bx_csr_base_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_bx_csr_base_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_bx_csr_base_t
    
class cap_bx_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_bx_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_bx_csr_t(string name = "cap_bx_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_bx_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_bx_csr_base_t base;
    
        cap_bx_csr_cfg_mac_gbl_t cfg_mac_gbl;
    
        cap_bx_csr_cfg_serdes_t cfg_serdes;
    
        cap_bx_csr_sta_mac_t sta_mac;
    
        cap_bx_csr_cfg_mac_xoff_t cfg_mac_xoff;
    
        cap_bx_csr_cfg_mac_sd_interrupt_t cfg_mac_sd_interrupt;
    
        cap_bx_csr_cfg_mac_sd_interrupt_request_t cfg_mac_sd_interrupt_request;
    
        cap_bx_csr_cfg_mac_sd_core_to_cntl_t cfg_mac_sd_core_to_cntl;
    
        cap_bx_csr_sta_mac_sd_interrupt_status_t sta_mac_sd_interrupt_status;
    
        cap_bx_csr_sta_mac_sd_status_t sta_mac_sd_status;
    
        cap_bx_csr_csr_intr_t csr_intr;
    
        cap_bx_csr_cfg_fixer_t cfg_fixer;
    
        cap_bx_csr_cfg_debug_port_t cfg_debug_port;
    
        cap_bx_csr_cfg_ff_txfifo_t cfg_ff_txfifo;
    
        cap_bx_csr_sta_ff_txfifo_t sta_ff_txfifo;
    
        cap_bx_csr_cfg_ff_rxfifo_t cfg_ff_rxfifo;
    
        cap_bx_csr_sta_ff_rxfifo_t sta_ff_rxfifo;
    
        cap_bx_csr_cfg_stats_mem_t cfg_stats_mem;
    
        cap_bx_csr_sta_stats_mem_t sta_stats_mem;
    
        cap_bx_csr_dhs_mac_stats_t dhs_mac_stats;
    
        cap_bx_csr_dhs_apb_t dhs_apb;
    
        cap_bx_csr_intgrp_status_t int_groups;
    
        cap_bx_csr_intgrp_t int_mac;
    
}; // cap_bx_csr_t
    
#endif // CAP_BX_CSR_H
        