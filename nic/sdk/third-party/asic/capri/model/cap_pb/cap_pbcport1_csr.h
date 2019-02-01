
#ifndef CAP_PBCPORT1_CSR_H
#define CAP_PBCPORT1_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_pbcport1_csr_dhs_oq_flow_control_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_dhs_oq_flow_control_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_dhs_oq_flow_control_entry_t(string name = "cap_pbcport1_csr_dhs_oq_flow_control_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_dhs_oq_flow_control_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 14 > entry_cpp_int_t;
        cpp_int int_var__entry;
        void entry (const cpp_int  & l__val);
        cpp_int entry() const;
    
}; // cap_pbcport1_csr_dhs_oq_flow_control_entry_t
    
class cap_pbcport1_csr_dhs_oq_flow_control_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_dhs_oq_flow_control_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_dhs_oq_flow_control_t(string name = "cap_pbcport1_csr_dhs_oq_flow_control_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_dhs_oq_flow_control_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
        cap_csr_large_array_wrapper<cap_pbcport1_csr_dhs_oq_flow_control_entry_t, 32> entry;
        #else 
        cap_pbcport1_csr_dhs_oq_flow_control_entry_t entry[32];
        #endif
        int get_depth_entry() { return 32; }
    
}; // cap_pbcport1_csr_dhs_oq_flow_control_t
    
class cap_pbcport1_csr_cfg_mac_xoff_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_cfg_mac_xoff_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_cfg_mac_xoff_t(string name = "cap_pbcport1_csr_cfg_mac_xoff_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_cfg_mac_xoff_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > enable_cpp_int_t;
        cpp_int int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
}; // cap_pbcport1_csr_cfg_mac_xoff_t
    
class cap_pbcport1_csr_cfg_account_pause_timer_enable_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_cfg_account_pause_timer_enable_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_cfg_account_pause_timer_enable_t(string name = "cap_pbcport1_csr_cfg_account_pause_timer_enable_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_cfg_account_pause_timer_enable_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > iq_cpp_int_t;
        cpp_int int_var__iq;
        void iq (const cpp_int  & l__val);
        cpp_int iq() const;
    
}; // cap_pbcport1_csr_cfg_account_pause_timer_enable_t
    
class cap_pbcport1_csr_cfg_oq_arb_l2_strict_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_cfg_oq_arb_l2_strict_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_cfg_oq_arb_l2_strict_t(string name = "cap_pbcport1_csr_cfg_oq_arb_l2_strict_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_cfg_oq_arb_l2_strict_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > priority_cpp_int_t;
        cpp_int int_var__priority;
        void priority (const cpp_int  & l__val);
        cpp_int priority() const;
    
}; // cap_pbcport1_csr_cfg_oq_arb_l2_strict_t
    
class cap_pbcport1_csr_cfg_oq_arb_l2_selection_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_cfg_oq_arb_l2_selection_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_cfg_oq_arb_l2_selection_t(string name = "cap_pbcport1_csr_cfg_oq_arb_l2_selection_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_cfg_oq_arb_l2_selection_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > node_0_cpp_int_t;
        cpp_int int_var__node_0;
        void node_0 (const cpp_int  & l__val);
        cpp_int node_0() const;
    
        typedef pu_cpp_int< 16 > node_1_cpp_int_t;
        cpp_int int_var__node_1;
        void node_1 (const cpp_int  & l__val);
        cpp_int node_1() const;
    
        typedef pu_cpp_int< 16 > node_2_cpp_int_t;
        cpp_int int_var__node_2;
        void node_2 (const cpp_int  & l__val);
        cpp_int node_2() const;
    
        typedef pu_cpp_int< 16 > node_3_cpp_int_t;
        cpp_int int_var__node_3;
        void node_3 (const cpp_int  & l__val);
        cpp_int node_3() const;
    
}; // cap_pbcport1_csr_cfg_oq_arb_l2_selection_t
    
class cap_pbcport1_csr_cfg_oq_arb_l1_strict_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_cfg_oq_arb_l1_strict_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_cfg_oq_arb_l1_strict_t(string name = "cap_pbcport1_csr_cfg_oq_arb_l1_strict_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_cfg_oq_arb_l1_strict_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > priority_cpp_int_t;
        cpp_int int_var__priority;
        void priority (const cpp_int  & l__val);
        cpp_int priority() const;
    
}; // cap_pbcport1_csr_cfg_oq_arb_l1_strict_t
    
class cap_pbcport1_csr_cfg_oq_arb_l1_selection_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_cfg_oq_arb_l1_selection_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_cfg_oq_arb_l1_selection_t(string name = "cap_pbcport1_csr_cfg_oq_arb_l1_selection_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_cfg_oq_arb_l1_selection_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > node_0_cpp_int_t;
        cpp_int int_var__node_0;
        void node_0 (const cpp_int  & l__val);
        cpp_int node_0() const;
    
        typedef pu_cpp_int< 32 > node_1_cpp_int_t;
        cpp_int int_var__node_1;
        void node_1 (const cpp_int  & l__val);
        cpp_int node_1() const;
    
        typedef pu_cpp_int< 32 > node_2_cpp_int_t;
        cpp_int int_var__node_2;
        void node_2 (const cpp_int  & l__val);
        cpp_int node_2() const;
    
        typedef pu_cpp_int< 32 > node_3_cpp_int_t;
        cpp_int int_var__node_3;
        void node_3 (const cpp_int  & l__val);
        cpp_int node_3() const;
    
        typedef pu_cpp_int< 32 > node_4_cpp_int_t;
        cpp_int int_var__node_4;
        void node_4 (const cpp_int  & l__val);
        cpp_int node_4() const;
    
        typedef pu_cpp_int< 32 > node_5_cpp_int_t;
        cpp_int int_var__node_5;
        void node_5 (const cpp_int  & l__val);
        cpp_int node_5() const;
    
        typedef pu_cpp_int< 32 > node_6_cpp_int_t;
        cpp_int int_var__node_6;
        void node_6 (const cpp_int  & l__val);
        cpp_int node_6() const;
    
        typedef pu_cpp_int< 32 > node_7_cpp_int_t;
        cpp_int int_var__node_7;
        void node_7 (const cpp_int  & l__val);
        cpp_int node_7() const;
    
        typedef pu_cpp_int< 32 > node_8_cpp_int_t;
        cpp_int int_var__node_8;
        void node_8 (const cpp_int  & l__val);
        cpp_int node_8() const;
    
        typedef pu_cpp_int< 32 > node_9_cpp_int_t;
        cpp_int int_var__node_9;
        void node_9 (const cpp_int  & l__val);
        cpp_int node_9() const;
    
        typedef pu_cpp_int< 32 > node_10_cpp_int_t;
        cpp_int int_var__node_10;
        void node_10 (const cpp_int  & l__val);
        cpp_int node_10() const;
    
        typedef pu_cpp_int< 32 > node_11_cpp_int_t;
        cpp_int int_var__node_11;
        void node_11 (const cpp_int  & l__val);
        cpp_int node_11() const;
    
        typedef pu_cpp_int< 32 > node_12_cpp_int_t;
        cpp_int int_var__node_12;
        void node_12 (const cpp_int  & l__val);
        cpp_int node_12() const;
    
        typedef pu_cpp_int< 32 > node_13_cpp_int_t;
        cpp_int int_var__node_13;
        void node_13 (const cpp_int  & l__val);
        cpp_int node_13() const;
    
        typedef pu_cpp_int< 32 > node_14_cpp_int_t;
        cpp_int int_var__node_14;
        void node_14 (const cpp_int  & l__val);
        cpp_int node_14() const;
    
        typedef pu_cpp_int< 32 > node_15_cpp_int_t;
        cpp_int int_var__node_15;
        void node_15 (const cpp_int  & l__val);
        cpp_int node_15() const;
    
}; // cap_pbcport1_csr_cfg_oq_arb_l1_selection_t
    
class cap_pbcport1_csr_cfg_oq_queue_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_cfg_oq_queue_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_cfg_oq_queue_t(string name = "cap_pbcport1_csr_cfg_oq_queue_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_cfg_oq_queue_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > enable_cpp_int_t;
        cpp_int int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 32 > flush_cpp_int_t;
        cpp_int int_var__flush;
        void flush (const cpp_int  & l__val);
        cpp_int flush() const;
    
}; // cap_pbcport1_csr_cfg_oq_queue_t
    
class cap_pbcport1_csr_cfg_oq_xoff2oq_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_cfg_oq_xoff2oq_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_cfg_oq_xoff2oq_t(string name = "cap_pbcport1_csr_cfg_oq_xoff2oq_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_cfg_oq_xoff2oq_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 96 > map_cpp_int_t;
        cpp_int int_var__map;
        void map (const cpp_int  & l__val);
        cpp_int map() const;
    
}; // cap_pbcport1_csr_cfg_oq_xoff2oq_t
    
class cap_pbcport1_csr_cfg_oq_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_cfg_oq_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_cfg_oq_t(string name = "cap_pbcport1_csr_cfg_oq_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_cfg_oq_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 1 > flush_cpp_int_t;
        cpp_int int_var__flush;
        void flush (const cpp_int  & l__val);
        cpp_int flush() const;
    
        typedef pu_cpp_int< 7 > num_hdr_bytes_cpp_int_t;
        cpp_int int_var__num_hdr_bytes;
        void num_hdr_bytes (const cpp_int  & l__val);
        cpp_int num_hdr_bytes() const;
    
        typedef pu_cpp_int< 1 > eg_ts_enable_cpp_int_t;
        cpp_int int_var__eg_ts_enable;
        void eg_ts_enable (const cpp_int  & l__val);
        cpp_int eg_ts_enable() const;
    
        typedef pu_cpp_int< 1 > flush_hw_error_cpp_int_t;
        cpp_int int_var__flush_hw_error;
        void flush_hw_error (const cpp_int  & l__val);
        cpp_int flush_hw_error() const;
    
        typedef pu_cpp_int< 1 > hw_error_to_pbus_cpp_int_t;
        cpp_int int_var__hw_error_to_pbus;
        void hw_error_to_pbus (const cpp_int  & l__val);
        cpp_int hw_error_to_pbus() const;
    
        typedef pu_cpp_int< 1 > rewrite_enable_cpp_int_t;
        cpp_int int_var__rewrite_enable;
        void rewrite_enable (const cpp_int  & l__val);
        cpp_int rewrite_enable() const;
    
        typedef pu_cpp_int< 5 > ipg_bytes_cpp_int_t;
        cpp_int int_var__ipg_bytes;
        void ipg_bytes (const cpp_int  & l__val);
        cpp_int ipg_bytes() const;
    
        typedef pu_cpp_int< 1 > flow_control_enable_xoff_cpp_int_t;
        cpp_int int_var__flow_control_enable_xoff;
        void flow_control_enable_xoff (const cpp_int  & l__val);
        cpp_int flow_control_enable_xoff() const;
    
}; // cap_pbcport1_csr_cfg_oq_t
    
class cap_pbcport1_csr_cfg_account_pg_7_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_cfg_account_pg_7_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_cfg_account_pg_7_t(string name = "cap_pbcport1_csr_cfg_account_pg_7_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_cfg_account_pg_7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 12 > reserved_min_cpp_int_t;
        cpp_int int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        cpp_int int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        cpp_int int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        cpp_int int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 3 > alpha_cpp_int_t;
        cpp_int int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbcport1_csr_cfg_account_pg_7_t
    
class cap_pbcport1_csr_cfg_account_pg_6_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_cfg_account_pg_6_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_cfg_account_pg_6_t(string name = "cap_pbcport1_csr_cfg_account_pg_6_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_cfg_account_pg_6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 12 > reserved_min_cpp_int_t;
        cpp_int int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        cpp_int int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        cpp_int int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        cpp_int int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 3 > alpha_cpp_int_t;
        cpp_int int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbcport1_csr_cfg_account_pg_6_t
    
class cap_pbcport1_csr_cfg_account_pg_5_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_cfg_account_pg_5_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_cfg_account_pg_5_t(string name = "cap_pbcport1_csr_cfg_account_pg_5_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_cfg_account_pg_5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 12 > reserved_min_cpp_int_t;
        cpp_int int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        cpp_int int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        cpp_int int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        cpp_int int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 3 > alpha_cpp_int_t;
        cpp_int int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbcport1_csr_cfg_account_pg_5_t
    
class cap_pbcport1_csr_cfg_account_pg_4_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_cfg_account_pg_4_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_cfg_account_pg_4_t(string name = "cap_pbcport1_csr_cfg_account_pg_4_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_cfg_account_pg_4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 12 > reserved_min_cpp_int_t;
        cpp_int int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        cpp_int int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        cpp_int int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        cpp_int int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 3 > alpha_cpp_int_t;
        cpp_int int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbcport1_csr_cfg_account_pg_4_t
    
class cap_pbcport1_csr_cfg_account_pg_3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_cfg_account_pg_3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_cfg_account_pg_3_t(string name = "cap_pbcport1_csr_cfg_account_pg_3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_cfg_account_pg_3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 12 > reserved_min_cpp_int_t;
        cpp_int int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        cpp_int int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        cpp_int int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        cpp_int int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 3 > alpha_cpp_int_t;
        cpp_int int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbcport1_csr_cfg_account_pg_3_t
    
class cap_pbcport1_csr_cfg_account_pg_2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_cfg_account_pg_2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_cfg_account_pg_2_t(string name = "cap_pbcport1_csr_cfg_account_pg_2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_cfg_account_pg_2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 12 > reserved_min_cpp_int_t;
        cpp_int int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        cpp_int int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        cpp_int int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        cpp_int int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 3 > alpha_cpp_int_t;
        cpp_int int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbcport1_csr_cfg_account_pg_2_t
    
class cap_pbcport1_csr_cfg_account_pg_1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_cfg_account_pg_1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_cfg_account_pg_1_t(string name = "cap_pbcport1_csr_cfg_account_pg_1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_cfg_account_pg_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 12 > reserved_min_cpp_int_t;
        cpp_int int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        cpp_int int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        cpp_int int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        cpp_int int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 3 > alpha_cpp_int_t;
        cpp_int int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbcport1_csr_cfg_account_pg_1_t
    
class cap_pbcport1_csr_cfg_account_pg_0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_cfg_account_pg_0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_cfg_account_pg_0_t(string name = "cap_pbcport1_csr_cfg_account_pg_0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_cfg_account_pg_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 12 > reserved_min_cpp_int_t;
        cpp_int int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        cpp_int int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        cpp_int int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        cpp_int int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 3 > alpha_cpp_int_t;
        cpp_int int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbcport1_csr_cfg_account_pg_0_t
    
class cap_pbcport1_csr_cfg_account_mtu_table_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_cfg_account_mtu_table_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_cfg_account_mtu_table_t(string name = "cap_pbcport1_csr_cfg_account_mtu_table_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_cfg_account_mtu_table_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 5 > pg0_cpp_int_t;
        cpp_int int_var__pg0;
        void pg0 (const cpp_int  & l__val);
        cpp_int pg0() const;
    
        typedef pu_cpp_int< 5 > pg1_cpp_int_t;
        cpp_int int_var__pg1;
        void pg1 (const cpp_int  & l__val);
        cpp_int pg1() const;
    
        typedef pu_cpp_int< 5 > pg2_cpp_int_t;
        cpp_int int_var__pg2;
        void pg2 (const cpp_int  & l__val);
        cpp_int pg2() const;
    
        typedef pu_cpp_int< 5 > pg3_cpp_int_t;
        cpp_int int_var__pg3;
        void pg3 (const cpp_int  & l__val);
        cpp_int pg3() const;
    
        typedef pu_cpp_int< 5 > pg4_cpp_int_t;
        cpp_int int_var__pg4;
        void pg4 (const cpp_int  & l__val);
        cpp_int pg4() const;
    
        typedef pu_cpp_int< 5 > pg5_cpp_int_t;
        cpp_int int_var__pg5;
        void pg5 (const cpp_int  & l__val);
        cpp_int pg5() const;
    
        typedef pu_cpp_int< 5 > pg6_cpp_int_t;
        cpp_int int_var__pg6;
        void pg6 (const cpp_int  & l__val);
        cpp_int pg6() const;
    
        typedef pu_cpp_int< 5 > pg7_cpp_int_t;
        cpp_int int_var__pg7;
        void pg7 (const cpp_int  & l__val);
        cpp_int pg7() const;
    
}; // cap_pbcport1_csr_cfg_account_mtu_table_t
    
class cap_pbcport1_csr_cfg_account_pause_timer_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_cfg_account_pause_timer_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_cfg_account_pause_timer_t(string name = "cap_pbcport1_csr_cfg_account_pause_timer_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_cfg_account_pause_timer_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 27 > xoff_timeout_cpp_int_t;
        cpp_int int_var__xoff_timeout;
        void xoff_timeout (const cpp_int  & l__val);
        cpp_int xoff_timeout() const;
    
        typedef pu_cpp_int< 1 > xoff_timeout_auto_xon_cpp_int_t;
        cpp_int int_var__xoff_timeout_auto_xon;
        void xoff_timeout_auto_xon (const cpp_int  & l__val);
        cpp_int xoff_timeout_auto_xon() const;
    
        typedef pu_cpp_int< 1 > xoff_timeout_auto_clear_cpp_int_t;
        cpp_int int_var__xoff_timeout_auto_clear;
        void xoff_timeout_auto_clear (const cpp_int  & l__val);
        cpp_int xoff_timeout_auto_clear() const;
    
}; // cap_pbcport1_csr_cfg_account_pause_timer_t
    
class cap_pbcport1_csr_sta_account_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_sta_account_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_sta_account_t(string name = "cap_pbcport1_csr_sta_account_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_sta_account_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 14 > occupancy_0_cpp_int_t;
        cpp_int int_var__occupancy_0;
        void occupancy_0 (const cpp_int  & l__val);
        cpp_int occupancy_0() const;
    
        typedef pu_cpp_int< 14 > sp_held_0_cpp_int_t;
        cpp_int int_var__sp_held_0;
        void sp_held_0 (const cpp_int  & l__val);
        cpp_int sp_held_0() const;
    
        typedef pu_cpp_int< 14 > occupancy_1_cpp_int_t;
        cpp_int int_var__occupancy_1;
        void occupancy_1 (const cpp_int  & l__val);
        cpp_int occupancy_1() const;
    
        typedef pu_cpp_int< 14 > sp_held_1_cpp_int_t;
        cpp_int int_var__sp_held_1;
        void sp_held_1 (const cpp_int  & l__val);
        cpp_int sp_held_1() const;
    
        typedef pu_cpp_int< 14 > occupancy_2_cpp_int_t;
        cpp_int int_var__occupancy_2;
        void occupancy_2 (const cpp_int  & l__val);
        cpp_int occupancy_2() const;
    
        typedef pu_cpp_int< 14 > sp_held_2_cpp_int_t;
        cpp_int int_var__sp_held_2;
        void sp_held_2 (const cpp_int  & l__val);
        cpp_int sp_held_2() const;
    
        typedef pu_cpp_int< 14 > occupancy_3_cpp_int_t;
        cpp_int int_var__occupancy_3;
        void occupancy_3 (const cpp_int  & l__val);
        cpp_int occupancy_3() const;
    
        typedef pu_cpp_int< 14 > sp_held_3_cpp_int_t;
        cpp_int int_var__sp_held_3;
        void sp_held_3 (const cpp_int  & l__val);
        cpp_int sp_held_3() const;
    
        typedef pu_cpp_int< 14 > occupancy_4_cpp_int_t;
        cpp_int int_var__occupancy_4;
        void occupancy_4 (const cpp_int  & l__val);
        cpp_int occupancy_4() const;
    
        typedef pu_cpp_int< 14 > sp_held_4_cpp_int_t;
        cpp_int int_var__sp_held_4;
        void sp_held_4 (const cpp_int  & l__val);
        cpp_int sp_held_4() const;
    
        typedef pu_cpp_int< 14 > occupancy_5_cpp_int_t;
        cpp_int int_var__occupancy_5;
        void occupancy_5 (const cpp_int  & l__val);
        cpp_int occupancy_5() const;
    
        typedef pu_cpp_int< 14 > sp_held_5_cpp_int_t;
        cpp_int int_var__sp_held_5;
        void sp_held_5 (const cpp_int  & l__val);
        cpp_int sp_held_5() const;
    
        typedef pu_cpp_int< 14 > occupancy_6_cpp_int_t;
        cpp_int int_var__occupancy_6;
        void occupancy_6 (const cpp_int  & l__val);
        cpp_int occupancy_6() const;
    
        typedef pu_cpp_int< 14 > sp_held_6_cpp_int_t;
        cpp_int int_var__sp_held_6;
        void sp_held_6 (const cpp_int  & l__val);
        cpp_int sp_held_6() const;
    
        typedef pu_cpp_int< 14 > occupancy_7_cpp_int_t;
        cpp_int int_var__occupancy_7;
        void occupancy_7 (const cpp_int  & l__val);
        cpp_int occupancy_7() const;
    
        typedef pu_cpp_int< 14 > sp_held_7_cpp_int_t;
        cpp_int int_var__sp_held_7;
        void sp_held_7 (const cpp_int  & l__val);
        cpp_int sp_held_7() const;
    
}; // cap_pbcport1_csr_sta_account_t
    
class cap_pbcport1_csr_cfg_account_tc_to_pg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_cfg_account_tc_to_pg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_cfg_account_tc_to_pg_t(string name = "cap_pbcport1_csr_cfg_account_tc_to_pg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_cfg_account_tc_to_pg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 24 > table_cpp_int_t;
        cpp_int int_var__table;
        void table (const cpp_int  & l__val);
        cpp_int table() const;
    
}; // cap_pbcport1_csr_cfg_account_tc_to_pg_t
    
class cap_pbcport1_csr_cfg_account_pg_to_sp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_cfg_account_pg_to_sp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_cfg_account_pg_to_sp_t(string name = "cap_pbcport1_csr_cfg_account_pg_to_sp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_cfg_account_pg_to_sp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > map_cpp_int_t;
        cpp_int int_var__map;
        void map (const cpp_int  & l__val);
        cpp_int map() const;
    
}; // cap_pbcport1_csr_cfg_account_pg_to_sp_t
    
class cap_pbcport1_csr_cfg_account_sp_fetch_threshold_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_cfg_account_sp_fetch_threshold_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_cfg_account_sp_fetch_threshold_t(string name = "cap_pbcport1_csr_cfg_account_sp_fetch_threshold_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_cfg_account_sp_fetch_threshold_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 6 > pg0_cpp_int_t;
        cpp_int int_var__pg0;
        void pg0 (const cpp_int  & l__val);
        cpp_int pg0() const;
    
        typedef pu_cpp_int< 6 > pg1_cpp_int_t;
        cpp_int int_var__pg1;
        void pg1 (const cpp_int  & l__val);
        cpp_int pg1() const;
    
        typedef pu_cpp_int< 6 > pg2_cpp_int_t;
        cpp_int int_var__pg2;
        void pg2 (const cpp_int  & l__val);
        cpp_int pg2() const;
    
        typedef pu_cpp_int< 6 > pg3_cpp_int_t;
        cpp_int int_var__pg3;
        void pg3 (const cpp_int  & l__val);
        cpp_int pg3() const;
    
        typedef pu_cpp_int< 6 > pg4_cpp_int_t;
        cpp_int int_var__pg4;
        void pg4 (const cpp_int  & l__val);
        cpp_int pg4() const;
    
        typedef pu_cpp_int< 6 > pg5_cpp_int_t;
        cpp_int int_var__pg5;
        void pg5 (const cpp_int  & l__val);
        cpp_int pg5() const;
    
        typedef pu_cpp_int< 6 > pg6_cpp_int_t;
        cpp_int int_var__pg6;
        void pg6 (const cpp_int  & l__val);
        cpp_int pg6() const;
    
        typedef pu_cpp_int< 6 > pg7_cpp_int_t;
        cpp_int int_var__pg7;
        void pg7 (const cpp_int  & l__val);
        cpp_int pg7() const;
    
}; // cap_pbcport1_csr_cfg_account_sp_fetch_threshold_t
    
class cap_pbcport1_csr_cfg_account_sp_fetch_quanta_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_cfg_account_sp_fetch_quanta_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_cfg_account_sp_fetch_quanta_t(string name = "cap_pbcport1_csr_cfg_account_sp_fetch_quanta_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_cfg_account_sp_fetch_quanta_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 4 > pg0_cpp_int_t;
        cpp_int int_var__pg0;
        void pg0 (const cpp_int  & l__val);
        cpp_int pg0() const;
    
        typedef pu_cpp_int< 4 > pg1_cpp_int_t;
        cpp_int int_var__pg1;
        void pg1 (const cpp_int  & l__val);
        cpp_int pg1() const;
    
        typedef pu_cpp_int< 4 > pg2_cpp_int_t;
        cpp_int int_var__pg2;
        void pg2 (const cpp_int  & l__val);
        cpp_int pg2() const;
    
        typedef pu_cpp_int< 4 > pg3_cpp_int_t;
        cpp_int int_var__pg3;
        void pg3 (const cpp_int  & l__val);
        cpp_int pg3() const;
    
        typedef pu_cpp_int< 4 > pg4_cpp_int_t;
        cpp_int int_var__pg4;
        void pg4 (const cpp_int  & l__val);
        cpp_int pg4() const;
    
        typedef pu_cpp_int< 4 > pg5_cpp_int_t;
        cpp_int int_var__pg5;
        void pg5 (const cpp_int  & l__val);
        cpp_int pg5() const;
    
        typedef pu_cpp_int< 4 > pg6_cpp_int_t;
        cpp_int int_var__pg6;
        void pg6 (const cpp_int  & l__val);
        cpp_int pg6() const;
    
        typedef pu_cpp_int< 4 > pg7_cpp_int_t;
        cpp_int int_var__pg7;
        void pg7 (const cpp_int  & l__val);
        cpp_int pg7() const;
    
}; // cap_pbcport1_csr_cfg_account_sp_fetch_quanta_t
    
class cap_pbcport1_csr_cfg_account_clear_xoff_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_cfg_account_clear_xoff_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_cfg_account_clear_xoff_t(string name = "cap_pbcport1_csr_cfg_account_clear_xoff_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_cfg_account_clear_xoff_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > bitmap_cpp_int_t;
        cpp_int int_var__bitmap;
        void bitmap (const cpp_int  & l__val);
        cpp_int bitmap() const;
    
}; // cap_pbcport1_csr_cfg_account_clear_xoff_t
    
class cap_pbcport1_csr_cfg_account_credit_return_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_cfg_account_credit_return_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_cfg_account_credit_return_t(string name = "cap_pbcport1_csr_cfg_account_credit_return_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_cfg_account_credit_return_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > enable_cpp_int_t;
        cpp_int int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
}; // cap_pbcport1_csr_cfg_account_credit_return_t
    
class cap_pbcport1_csr_cfg_account_control_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_cfg_account_control_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_cfg_account_control_t(string name = "cap_pbcport1_csr_cfg_account_control_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_cfg_account_control_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > sp_enable_cpp_int_t;
        cpp_int int_var__sp_enable;
        void sp_enable (const cpp_int  & l__val);
        cpp_int sp_enable() const;
    
        typedef pu_cpp_int< 1 > use_sp_as_wm_cpp_int_t;
        cpp_int int_var__use_sp_as_wm;
        void use_sp_as_wm (const cpp_int  & l__val);
        cpp_int use_sp_as_wm() const;
    
        typedef pu_cpp_int< 1 > clear_wm_enable_cpp_int_t;
        cpp_int int_var__clear_wm_enable;
        void clear_wm_enable (const cpp_int  & l__val);
        cpp_int clear_wm_enable() const;
    
        typedef pu_cpp_int< 8 > clear_wm_index_cpp_int_t;
        cpp_int int_var__clear_wm_index;
        void clear_wm_index (const cpp_int  & l__val);
        cpp_int clear_wm_index() const;
    
}; // cap_pbcport1_csr_cfg_account_control_t
    
class cap_pbcport1_csr_sta_write_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_sta_write_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_sta_write_t(string name = "cap_pbcport1_csr_sta_write_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_sta_write_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 2 > cache_entries_cpp_int_t;
        cpp_int int_var__cache_entries;
        void cache_entries (const cpp_int  & l__val);
        cpp_int cache_entries() const;
    
}; // cap_pbcport1_csr_sta_write_t
    
class cap_pbcport1_csr_cfg_write_control_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_cfg_write_control_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_cfg_write_control_t(string name = "cap_pbcport1_csr_cfg_write_control_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_cfg_write_control_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 5 > cut_thru_cpp_int_t;
        cpp_int int_var__cut_thru;
        void cut_thru (const cpp_int  & l__val);
        cpp_int cut_thru() const;
    
        typedef pu_cpp_int< 1 > packing_cpp_int_t;
        cpp_int int_var__packing;
        void packing (const cpp_int  & l__val);
        cpp_int packing() const;
    
        typedef pu_cpp_int< 1 > packing_same_pg_cpp_int_t;
        cpp_int int_var__packing_same_pg;
        void packing_same_pg (const cpp_int  & l__val);
        cpp_int packing_same_pg() const;
    
        typedef pu_cpp_int< 10 > packing_timeout_cycles_cpp_int_t;
        cpp_int int_var__packing_timeout_cycles;
        void packing_timeout_cycles (const cpp_int  & l__val);
        cpp_int packing_timeout_cycles() const;
    
        typedef pu_cpp_int< 1 > rewrite_cpp_int_t;
        cpp_int int_var__rewrite;
        void rewrite (const cpp_int  & l__val);
        cpp_int rewrite() const;
    
        typedef pu_cpp_int< 4 > rate_limiter_cpp_int_t;
        cpp_int int_var__rate_limiter;
        void rate_limiter (const cpp_int  & l__val);
        cpp_int rate_limiter() const;
    
        typedef pu_cpp_int< 6 > min_size_cpp_int_t;
        cpp_int int_var__min_size;
        void min_size (const cpp_int  & l__val);
        cpp_int min_size() const;
    
        typedef pu_cpp_int< 1 > drop_on_error_cpp_int_t;
        cpp_int int_var__drop_on_error;
        void drop_on_error (const cpp_int  & l__val);
        cpp_int drop_on_error() const;
    
        typedef pu_cpp_int< 1 > release_cells_cpp_int_t;
        cpp_int int_var__release_cells;
        void release_cells (const cpp_int  & l__val);
        cpp_int release_cells() const;
    
        typedef pu_cpp_int< 1 > span_pg_enable_cpp_int_t;
        cpp_int int_var__span_pg_enable;
        void span_pg_enable (const cpp_int  & l__val);
        cpp_int span_pg_enable() const;
    
        typedef pu_cpp_int< 3 > span_pg_index_cpp_int_t;
        cpp_int int_var__span_pg_index;
        void span_pg_index (const cpp_int  & l__val);
        cpp_int span_pg_index() const;
    
}; // cap_pbcport1_csr_cfg_write_control_t
    
class cap_pbcport1_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbcport1_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbcport1_csr_t(string name = "cap_pbcport1_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbcport1_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbcport1_csr_cfg_write_control_t cfg_write_control;
    
        cap_pbcport1_csr_sta_write_t sta_write;
    
        cap_pbcport1_csr_cfg_account_control_t cfg_account_control;
    
        cap_pbcport1_csr_cfg_account_credit_return_t cfg_account_credit_return;
    
        cap_pbcport1_csr_cfg_account_clear_xoff_t cfg_account_clear_xoff;
    
        cap_pbcport1_csr_cfg_account_sp_fetch_quanta_t cfg_account_sp_fetch_quanta;
    
        cap_pbcport1_csr_cfg_account_sp_fetch_threshold_t cfg_account_sp_fetch_threshold;
    
        cap_pbcport1_csr_cfg_account_pg_to_sp_t cfg_account_pg_to_sp;
    
        cap_pbcport1_csr_cfg_account_tc_to_pg_t cfg_account_tc_to_pg;
    
        cap_pbcport1_csr_sta_account_t sta_account;
    
        cap_pbcport1_csr_cfg_account_pause_timer_t cfg_account_pause_timer;
    
        cap_pbcport1_csr_cfg_account_mtu_table_t cfg_account_mtu_table;
    
        cap_pbcport1_csr_cfg_account_pg_0_t cfg_account_pg_0;
    
        cap_pbcport1_csr_cfg_account_pg_1_t cfg_account_pg_1;
    
        cap_pbcport1_csr_cfg_account_pg_2_t cfg_account_pg_2;
    
        cap_pbcport1_csr_cfg_account_pg_3_t cfg_account_pg_3;
    
        cap_pbcport1_csr_cfg_account_pg_4_t cfg_account_pg_4;
    
        cap_pbcport1_csr_cfg_account_pg_5_t cfg_account_pg_5;
    
        cap_pbcport1_csr_cfg_account_pg_6_t cfg_account_pg_6;
    
        cap_pbcport1_csr_cfg_account_pg_7_t cfg_account_pg_7;
    
        cap_pbcport1_csr_cfg_oq_t cfg_oq;
    
        cap_pbcport1_csr_cfg_oq_xoff2oq_t cfg_oq_xoff2oq;
    
        cap_pbcport1_csr_cfg_oq_queue_t cfg_oq_queue;
    
        cap_pbcport1_csr_cfg_oq_arb_l1_selection_t cfg_oq_arb_l1_selection;
    
        cap_pbcport1_csr_cfg_oq_arb_l1_strict_t cfg_oq_arb_l1_strict;
    
        cap_pbcport1_csr_cfg_oq_arb_l2_selection_t cfg_oq_arb_l2_selection;
    
        cap_pbcport1_csr_cfg_oq_arb_l2_strict_t cfg_oq_arb_l2_strict;
    
        cap_pbcport1_csr_cfg_account_pause_timer_enable_t cfg_account_pause_timer_enable;
    
        cap_pbcport1_csr_cfg_mac_xoff_t cfg_mac_xoff;
    
        cap_pbcport1_csr_dhs_oq_flow_control_t dhs_oq_flow_control;
    
}; // cap_pbcport1_csr_t
    
#endif // CAP_PBCPORT1_CSR_H
        