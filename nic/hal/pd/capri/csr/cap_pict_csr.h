
#ifndef CAP_PICT_CSR_H
#define CAP_PICT_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_pict_csr_dhs_tcam_ind_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pict_csr_dhs_tcam_ind_entry_t(string name = "cap_pict_csr_dhs_tcam_ind_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pict_csr_dhs_tcam_ind_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 2 > cmd_cpp_int_t;
        cmd_cpp_int_t int_var__cmd;
        void cmd (const cpp_int  & l__val);
        cpp_int cmd() const;
    
}; // cap_pict_csr_dhs_tcam_ind_entry_t
    
class cap_pict_csr_dhs_tcam_ind_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pict_csr_dhs_tcam_ind_t(string name = "cap_pict_csr_dhs_tcam_ind_t", cap_csr_base *parent = 0);
        virtual ~cap_pict_csr_dhs_tcam_ind_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pict_csr_dhs_tcam_ind_entry_t entry[8];
        int get_depth_entry() { return 8; }
    
}; // cap_pict_csr_dhs_tcam_ind_t
    
class cap_pict_csr_dhs_tcam_xy_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pict_csr_dhs_tcam_xy_entry_t(string name = "cap_pict_csr_dhs_tcam_xy_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pict_csr_dhs_tcam_xy_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        valid_cpp_int_t int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 128 > x_cpp_int_t;
        x_cpp_int_t int_var__x;
        void x (const cpp_int  & l__val);
        cpp_int x() const;
    
        typedef pu_cpp_int< 128 > y_cpp_int_t;
        y_cpp_int_t int_var__y;
        void y (const cpp_int  & l__val);
        cpp_int y() const;
    
}; // cap_pict_csr_dhs_tcam_xy_entry_t
    
class cap_pict_csr_dhs_tcam_xy_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pict_csr_dhs_tcam_xy_t(string name = "cap_pict_csr_dhs_tcam_xy_t", cap_csr_base *parent = 0);
        virtual ~cap_pict_csr_dhs_tcam_xy_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pict_csr_dhs_tcam_xy_entry_t entry[8192];
        int get_depth_entry() { return 8192; }
    
}; // cap_pict_csr_dhs_tcam_xy_t
    
class cap_pict_csr_sta_tcam_ind_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pict_csr_sta_tcam_ind_t(string name = "cap_pict_csr_sta_tcam_ind_t", cap_csr_base *parent = 0);
        virtual ~cap_pict_csr_sta_tcam_ind_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > hit_cpp_int_t;
        hit_cpp_int_t int_var__hit;
        void hit (const cpp_int  & l__val);
        cpp_int hit() const;
    
        typedef pu_cpp_int< 10 > hit_addr_cpp_int_t;
        hit_addr_cpp_int_t int_var__hit_addr;
        void hit_addr (const cpp_int  & l__val);
        cpp_int hit_addr() const;
    
}; // cap_pict_csr_sta_tcam_ind_t
    
class cap_pict_csr_cfg_tcam_ind_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pict_csr_cfg_tcam_ind_t(string name = "cap_pict_csr_cfg_tcam_ind_t", cap_csr_base *parent = 0);
        virtual ~cap_pict_csr_cfg_tcam_ind_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > grst_pulse_cpp_int_t;
        grst_pulse_cpp_int_t int_var__grst_pulse;
        void grst_pulse (const cpp_int  & l__val);
        cpp_int grst_pulse() const;
    
        typedef pu_cpp_int< 128 > key_cpp_int_t;
        key_cpp_int_t int_var__key;
        void key (const cpp_int  & l__val);
        cpp_int key() const;
    
        typedef pu_cpp_int< 128 > mask_cpp_int_t;
        mask_cpp_int_t int_var__mask;
        void mask (const cpp_int  & l__val);
        cpp_int mask() const;
    
}; // cap_pict_csr_cfg_tcam_ind_t
    
class cap_pict_csr_cfg_tcam_table_profile_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pict_csr_cfg_tcam_table_profile_t(string name = "cap_pict_csr_cfg_tcam_table_profile_t", cap_csr_base *parent = 0);
        virtual ~cap_pict_csr_cfg_tcam_table_profile_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 17 > start_addr_cpp_int_t;
        start_addr_cpp_int_t int_var__start_addr;
        void start_addr (const cpp_int  & l__val);
        cpp_int start_addr() const;
    
        typedef pu_cpp_int< 17 > end_addr_cpp_int_t;
        end_addr_cpp_int_t int_var__end_addr;
        void end_addr (const cpp_int  & l__val);
        cpp_int end_addr() const;
    
        typedef pu_cpp_int< 6 > width_cpp_int_t;
        width_cpp_int_t int_var__width;
        void width (const cpp_int  & l__val);
        cpp_int width() const;
    
        typedef pu_cpp_int< 1 > en_tbid_cpp_int_t;
        en_tbid_cpp_int_t int_var__en_tbid;
        void en_tbid (const cpp_int  & l__val);
        cpp_int en_tbid() const;
    
        typedef pu_cpp_int< 7 > tbid_cpp_int_t;
        tbid_cpp_int_t int_var__tbid;
        void tbid (const cpp_int  & l__val);
        cpp_int tbid() const;
    
        typedef pu_cpp_int< 3 > bkts_cpp_int_t;
        bkts_cpp_int_t int_var__bkts;
        void bkts (const cpp_int  & l__val);
        cpp_int bkts() const;
    
}; // cap_pict_csr_cfg_tcam_table_profile_t
    
class cap_pict_csr_cfg_global_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pict_csr_cfg_global_t(string name = "cap_pict_csr_cfg_global_t", cap_csr_base *parent = 0);
        virtual ~cap_pict_csr_cfg_global_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > sw_rst_cpp_int_t;
        sw_rst_cpp_int_t int_var__sw_rst;
        void sw_rst (const cpp_int  & l__val);
        cpp_int sw_rst() const;
    
}; // cap_pict_csr_cfg_global_t
    
class cap_pict_csr_base_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pict_csr_base_t(string name = "cap_pict_csr_base_t", cap_csr_base *parent = 0);
        virtual ~cap_pict_csr_base_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > scratch_reg_cpp_int_t;
        scratch_reg_cpp_int_t int_var__scratch_reg;
        void scratch_reg (const cpp_int  & l__val);
        cpp_int scratch_reg() const;
    
}; // cap_pict_csr_base_t
    
class cap_pict_csr_t : public cap_block_base {
    public:
        cpp_int_helper hlp;
    
        cap_pict_csr_t(string name = "cap_pict_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_pict_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pict_csr_base_t base;
    
        cap_pict_csr_cfg_global_t cfg_global;
    
        cap_pict_csr_cfg_tcam_table_profile_t cfg_tcam_table_profile[128];
        int get_depth_cfg_tcam_table_profile() { return 128; }
    
        cap_pict_csr_cfg_tcam_ind_t cfg_tcam_ind[8];
        int get_depth_cfg_tcam_ind() { return 8; }
    
        cap_pict_csr_sta_tcam_ind_t sta_tcam_ind[8];
        int get_depth_sta_tcam_ind() { return 8; }
    
        cap_pict_csr_dhs_tcam_xy_t dhs_tcam_xy;
    
        cap_pict_csr_dhs_tcam_ind_t dhs_tcam_ind;
    
}; // cap_pict_csr_t
    
#endif // CAP_PICT_CSR_H
        