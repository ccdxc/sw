
#ifndef CAP_PICT_CSR_H
#define CAP_PICT_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_pict_csr_dhs_tcam_srch_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pict_csr_dhs_tcam_srch_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pict_csr_dhs_tcam_srch_entry_t(string name = "cap_pict_csr_dhs_tcam_srch_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pict_csr_dhs_tcam_srch_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > cmd_cpp_int_t;
        cpp_int int_var__cmd;
        void cmd (const cpp_int  & l__val);
        cpp_int cmd() const;
    
}; // cap_pict_csr_dhs_tcam_srch_entry_t
    
class cap_pict_csr_dhs_tcam_srch_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pict_csr_dhs_tcam_srch_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pict_csr_dhs_tcam_srch_t(string name = "cap_pict_csr_dhs_tcam_srch_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pict_csr_dhs_tcam_srch_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
        cap_csr_large_array_wrapper<cap_pict_csr_dhs_tcam_srch_entry_t, 8> entry;
        #else 
        cap_pict_csr_dhs_tcam_srch_entry_t entry[8];
        #endif
        int get_depth_entry() { return 8; }
    
}; // cap_pict_csr_dhs_tcam_srch_t
    
class cap_pict_csr_dhs_tcam_xy_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pict_csr_dhs_tcam_xy_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pict_csr_dhs_tcam_xy_entry_t(string name = "cap_pict_csr_dhs_tcam_xy_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pict_csr_dhs_tcam_xy_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 128 > x_cpp_int_t;
        cpp_int int_var__x;
        void x (const cpp_int  & l__val);
        cpp_int x() const;
    
        typedef pu_cpp_int< 128 > y_cpp_int_t;
        cpp_int int_var__y;
        void y (const cpp_int  & l__val);
        cpp_int y() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        cpp_int int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
}; // cap_pict_csr_dhs_tcam_xy_entry_t
    
class cap_pict_csr_dhs_tcam_xy_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pict_csr_dhs_tcam_xy_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pict_csr_dhs_tcam_xy_t(string name = "cap_pict_csr_dhs_tcam_xy_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pict_csr_dhs_tcam_xy_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        #if 8192 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pict_csr_dhs_tcam_xy_entry_t, 8192> entry;
        #else 
        cap_pict_csr_dhs_tcam_xy_entry_t entry[8192];
        #endif
        int get_depth_entry() { return 8192; }
    
}; // cap_pict_csr_dhs_tcam_xy_t
    
class cap_pict_csr_sta_tcam_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pict_csr_sta_tcam_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pict_csr_sta_tcam_t(string name = "cap_pict_csr_sta_tcam_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pict_csr_sta_tcam_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 8 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
}; // cap_pict_csr_sta_tcam_t
    
class cap_pict_csr_cfg_tcam_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pict_csr_cfg_tcam_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pict_csr_cfg_tcam_t(string name = "cap_pict_csr_cfg_tcam_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pict_csr_cfg_tcam_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_pict_csr_cfg_tcam_t
    
class cap_pict_csr_cnt_tcam_search_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pict_csr_cnt_tcam_search_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pict_csr_cnt_tcam_search_t(string name = "cap_pict_csr_cnt_tcam_search_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pict_csr_cnt_tcam_search_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_pict_csr_cnt_tcam_search_t
    
class cap_pict_csr_cnt_axi_pot_rdrsp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pict_csr_cnt_axi_pot_rdrsp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pict_csr_cnt_axi_pot_rdrsp_t(string name = "cap_pict_csr_cnt_axi_pot_rdrsp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pict_csr_cnt_axi_pot_rdrsp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_pict_csr_cnt_axi_pot_rdrsp_t
    
class cap_pict_csr_cnt_axi_pot_rdreq_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pict_csr_cnt_axi_pot_rdreq_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pict_csr_cnt_axi_pot_rdreq_t(string name = "cap_pict_csr_cnt_axi_pot_rdreq_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pict_csr_cnt_axi_pot_rdreq_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 40 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_pict_csr_cnt_axi_pot_rdreq_t
    
class cap_pict_csr_sta_tcam_srch_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pict_csr_sta_tcam_srch_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pict_csr_sta_tcam_srch_t(string name = "cap_pict_csr_sta_tcam_srch_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pict_csr_sta_tcam_srch_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_pict_csr_sta_tcam_srch_t
    
class cap_pict_csr_cfg_tcam_srch_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pict_csr_cfg_tcam_srch_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pict_csr_cfg_tcam_srch_t(string name = "cap_pict_csr_cfg_tcam_srch_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pict_csr_cfg_tcam_srch_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 128 > key_cpp_int_t;
        cpp_int int_var__key;
        void key (const cpp_int  & l__val);
        cpp_int key() const;
    
        typedef pu_cpp_int< 128 > mask_cpp_int_t;
        cpp_int int_var__mask;
        void mask (const cpp_int  & l__val);
        cpp_int mask() const;
    
        typedef pu_cpp_int< 16 > tbl_mask_cpp_int_t;
        cpp_int int_var__tbl_mask;
        void tbl_mask (const cpp_int  & l__val);
        cpp_int tbl_mask() const;
    
}; // cap_pict_csr_cfg_tcam_srch_t
    
class cap_pict_csr_cfg_tcam_reset_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pict_csr_cfg_tcam_reset_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pict_csr_cfg_tcam_reset_t(string name = "cap_pict_csr_cfg_tcam_reset_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pict_csr_cfg_tcam_reset_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 8 > vec_cpp_int_t;
        cpp_int int_var__vec;
        void vec (const cpp_int  & l__val);
        cpp_int vec() const;
    
}; // cap_pict_csr_cfg_tcam_reset_t
    
class cap_pict_csr_cfg_tcam_table_partition_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pict_csr_cfg_tcam_table_partition_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pict_csr_cfg_tcam_table_partition_t(string name = "cap_pict_csr_cfg_tcam_table_partition_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pict_csr_cfg_tcam_table_partition_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > vbe_cpp_int_t;
        cpp_int int_var__vbe;
        void vbe (const cpp_int  & l__val);
        cpp_int vbe() const;
    
}; // cap_pict_csr_cfg_tcam_table_partition_t
    
class cap_pict_csr_cfg_tcam_table_profile_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pict_csr_cfg_tcam_table_profile_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pict_csr_cfg_tcam_table_profile_t(string name = "cap_pict_csr_cfg_tcam_table_profile_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pict_csr_cfg_tcam_table_profile_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 17 > start_addr_cpp_int_t;
        cpp_int int_var__start_addr;
        void start_addr (const cpp_int  & l__val);
        cpp_int start_addr() const;
    
        typedef pu_cpp_int< 17 > end_addr_cpp_int_t;
        cpp_int int_var__end_addr;
        void end_addr (const cpp_int  & l__val);
        cpp_int end_addr() const;
    
        typedef pu_cpp_int< 6 > width_cpp_int_t;
        cpp_int int_var__width;
        void width (const cpp_int  & l__val);
        cpp_int width() const;
    
        typedef pu_cpp_int< 1 > en_tbid_cpp_int_t;
        cpp_int int_var__en_tbid;
        void en_tbid (const cpp_int  & l__val);
        cpp_int en_tbid() const;
    
        typedef pu_cpp_int< 7 > tbid_cpp_int_t;
        cpp_int int_var__tbid;
        void tbid (const cpp_int  & l__val);
        cpp_int tbid() const;
    
        typedef pu_cpp_int< 3 > bkts_cpp_int_t;
        cpp_int int_var__bkts;
        void bkts (const cpp_int  & l__val);
        cpp_int bkts() const;
    
        typedef pu_cpp_int< 5 > keyshift_cpp_int_t;
        cpp_int int_var__keyshift;
        void keyshift (const cpp_int  & l__val);
        cpp_int keyshift() const;
    
}; // cap_pict_csr_cfg_tcam_table_profile_t
    
class cap_pict_csr_base_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pict_csr_base_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pict_csr_base_t(string name = "cap_pict_csr_base_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pict_csr_base_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_pict_csr_base_t
    
class cap_pict_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pict_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pict_csr_t(string name = "cap_pict_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pict_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pict_csr_base_t base;
    
        #if 128 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pict_csr_cfg_tcam_table_profile_t, 128> cfg_tcam_table_profile;
        #else 
        cap_pict_csr_cfg_tcam_table_profile_t cfg_tcam_table_profile[128];
        #endif
        int get_depth_cfg_tcam_table_profile() { return 128; }
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pict_csr_cfg_tcam_table_partition_t, 8> cfg_tcam_table_partition;
        #else 
        cap_pict_csr_cfg_tcam_table_partition_t cfg_tcam_table_partition[8];
        #endif
        int get_depth_cfg_tcam_table_partition() { return 8; }
    
        cap_pict_csr_cfg_tcam_reset_t cfg_tcam_reset;
    
        cap_pict_csr_cfg_tcam_srch_t cfg_tcam_srch;
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pict_csr_sta_tcam_srch_t, 8> sta_tcam_srch;
        #else 
        cap_pict_csr_sta_tcam_srch_t sta_tcam_srch[8];
        #endif
        int get_depth_sta_tcam_srch() { return 8; }
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pict_csr_cnt_axi_pot_rdreq_t, 8> cnt_axi_pot_rdreq;
        #else 
        cap_pict_csr_cnt_axi_pot_rdreq_t cnt_axi_pot_rdreq[8];
        #endif
        int get_depth_cnt_axi_pot_rdreq() { return 8; }
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pict_csr_cnt_axi_pot_rdrsp_t, 8> cnt_axi_pot_rdrsp;
        #else 
        cap_pict_csr_cnt_axi_pot_rdrsp_t cnt_axi_pot_rdrsp[8];
        #endif
        int get_depth_cnt_axi_pot_rdrsp() { return 8; }
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_pict_csr_cnt_tcam_search_t, 8> cnt_tcam_search;
        #else 
        cap_pict_csr_cnt_tcam_search_t cnt_tcam_search[8];
        #endif
        int get_depth_cnt_tcam_search() { return 8; }
    
        cap_pict_csr_cfg_tcam_t cfg_tcam;
    
        cap_pict_csr_sta_tcam_t sta_tcam;
    
        cap_pict_csr_dhs_tcam_xy_t dhs_tcam_xy;
    
        cap_pict_csr_dhs_tcam_srch_t dhs_tcam_srch;
    
}; // cap_pict_csr_t
    
#endif // CAP_PICT_CSR_H
        