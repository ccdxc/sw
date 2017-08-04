
#ifndef CAP_PPA_CSR_H
#define CAP_PPA_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_ppa_csr_dhs_bndl1_state_lkp_sram_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_dhs_bndl1_state_lkp_sram_entry_t(string name = "cap_ppa_csr_dhs_bndl1_state_lkp_sram_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_dhs_bndl1_state_lkp_sram_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 756 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 8 > rsvd_cpp_int_t;
        rsvd_cpp_int_t int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
        typedef pu_cpp_int< 54 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_ppa_csr_dhs_bndl1_state_lkp_sram_entry_t
    
class cap_ppa_csr_dhs_bndl1_state_lkp_sram_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_dhs_bndl1_state_lkp_sram_t(string name = "cap_ppa_csr_dhs_bndl1_state_lkp_sram_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_dhs_bndl1_state_lkp_sram_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_ppa_csr_dhs_bndl1_state_lkp_sram_entry_t entry[288];
        int get_depth_entry() { return 288; }
    
}; // cap_ppa_csr_dhs_bndl1_state_lkp_sram_t
    
class cap_ppa_csr_dhs_bndl1_state_lkp_tcam_ind_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_dhs_bndl1_state_lkp_tcam_ind_entry_t(string name = "cap_ppa_csr_dhs_bndl1_state_lkp_tcam_ind_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_dhs_bndl1_state_lkp_tcam_ind_entry_t();
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
    
}; // cap_ppa_csr_dhs_bndl1_state_lkp_tcam_ind_entry_t
    
class cap_ppa_csr_dhs_bndl1_state_lkp_tcam_ind_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_dhs_bndl1_state_lkp_tcam_ind_t(string name = "cap_ppa_csr_dhs_bndl1_state_lkp_tcam_ind_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_dhs_bndl1_state_lkp_tcam_ind_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_ppa_csr_dhs_bndl1_state_lkp_tcam_ind_entry_t entry;
    
}; // cap_ppa_csr_dhs_bndl1_state_lkp_tcam_ind_t
    
class cap_ppa_csr_dhs_bndl1_state_lkp_tcam_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_dhs_bndl1_state_lkp_tcam_entry_t(string name = "cap_ppa_csr_dhs_bndl1_state_lkp_tcam_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_dhs_bndl1_state_lkp_tcam_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 64 > x_data_cpp_int_t;
        x_data_cpp_int_t int_var__x_data;
        void x_data (const cpp_int  & l__val);
        cpp_int x_data() const;
    
        typedef pu_cpp_int< 64 > y_data_cpp_int_t;
        y_data_cpp_int_t int_var__y_data;
        void y_data (const cpp_int  & l__val);
        cpp_int y_data() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        valid_cpp_int_t int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
}; // cap_ppa_csr_dhs_bndl1_state_lkp_tcam_entry_t
    
class cap_ppa_csr_dhs_bndl1_state_lkp_tcam_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_dhs_bndl1_state_lkp_tcam_t(string name = "cap_ppa_csr_dhs_bndl1_state_lkp_tcam_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_dhs_bndl1_state_lkp_tcam_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_ppa_csr_dhs_bndl1_state_lkp_tcam_entry_t entry[288];
        int get_depth_entry() { return 288; }
    
}; // cap_ppa_csr_dhs_bndl1_state_lkp_tcam_t
    
class cap_ppa_csr_dhs_bndl0_state_lkp_sram_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_dhs_bndl0_state_lkp_sram_entry_t(string name = "cap_ppa_csr_dhs_bndl0_state_lkp_sram_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_dhs_bndl0_state_lkp_sram_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 756 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 8 > rsvd_cpp_int_t;
        rsvd_cpp_int_t int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
        typedef pu_cpp_int< 54 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_ppa_csr_dhs_bndl0_state_lkp_sram_entry_t
    
class cap_ppa_csr_dhs_bndl0_state_lkp_sram_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_dhs_bndl0_state_lkp_sram_t(string name = "cap_ppa_csr_dhs_bndl0_state_lkp_sram_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_dhs_bndl0_state_lkp_sram_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_ppa_csr_dhs_bndl0_state_lkp_sram_entry_t entry[288];
        int get_depth_entry() { return 288; }
    
}; // cap_ppa_csr_dhs_bndl0_state_lkp_sram_t
    
class cap_ppa_csr_dhs_bndl0_state_lkp_tcam_ind_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_dhs_bndl0_state_lkp_tcam_ind_entry_t(string name = "cap_ppa_csr_dhs_bndl0_state_lkp_tcam_ind_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_dhs_bndl0_state_lkp_tcam_ind_entry_t();
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
    
}; // cap_ppa_csr_dhs_bndl0_state_lkp_tcam_ind_entry_t
    
class cap_ppa_csr_dhs_bndl0_state_lkp_tcam_ind_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_dhs_bndl0_state_lkp_tcam_ind_t(string name = "cap_ppa_csr_dhs_bndl0_state_lkp_tcam_ind_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_dhs_bndl0_state_lkp_tcam_ind_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_ppa_csr_dhs_bndl0_state_lkp_tcam_ind_entry_t entry;
    
}; // cap_ppa_csr_dhs_bndl0_state_lkp_tcam_ind_t
    
class cap_ppa_csr_dhs_bndl0_state_lkp_tcam_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_dhs_bndl0_state_lkp_tcam_entry_t(string name = "cap_ppa_csr_dhs_bndl0_state_lkp_tcam_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_dhs_bndl0_state_lkp_tcam_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 64 > x_data_cpp_int_t;
        x_data_cpp_int_t int_var__x_data;
        void x_data (const cpp_int  & l__val);
        cpp_int x_data() const;
    
        typedef pu_cpp_int< 64 > y_data_cpp_int_t;
        y_data_cpp_int_t int_var__y_data;
        void y_data (const cpp_int  & l__val);
        cpp_int y_data() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        valid_cpp_int_t int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
}; // cap_ppa_csr_dhs_bndl0_state_lkp_tcam_entry_t
    
class cap_ppa_csr_dhs_bndl0_state_lkp_tcam_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_dhs_bndl0_state_lkp_tcam_t(string name = "cap_ppa_csr_dhs_bndl0_state_lkp_tcam_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_dhs_bndl0_state_lkp_tcam_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_ppa_csr_dhs_bndl0_state_lkp_tcam_entry_t entry[288];
        int get_depth_entry() { return 288; }
    
}; // cap_ppa_csr_dhs_bndl0_state_lkp_tcam_t
    
class cap_ppa_csr_cfg_spare_dbg_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_cfg_spare_dbg_t(string name = "cap_ppa_csr_cfg_spare_dbg_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_cfg_spare_dbg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // cap_ppa_csr_cfg_spare_dbg_t
    
class cap_ppa_csr_cfg_debug_port_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_cfg_debug_port_t(string name = "cap_ppa_csr_cfg_debug_port_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_cfg_debug_port_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        enable_cpp_int_t int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 1 > select_cpp_int_t;
        select_cpp_int_t int_var__select;
        void select (const cpp_int  & l__val);
        cpp_int select() const;
    
}; // cap_ppa_csr_cfg_debug_port_t
    
class cap_ppa_csr_cfg_spare_ecc_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_cfg_spare_ecc_t(string name = "cap_ppa_csr_cfg_spare_ecc_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_cfg_spare_ecc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 128 > fld0_inst_cpp_int_t;
        fld0_inst_cpp_int_t int_var__fld0_inst;
        void fld0_inst (const cpp_int  & l__val);
        cpp_int fld0_inst() const;
    
        typedef pu_cpp_int< 128 > fld1_inst_cpp_int_t;
        fld1_inst_cpp_int_t int_var__fld1_inst;
        void fld1_inst (const cpp_int  & l__val);
        cpp_int fld1_inst() const;
    
        typedef pu_cpp_int< 128 > fld2_inst_cpp_int_t;
        fld2_inst_cpp_int_t int_var__fld2_inst;
        void fld2_inst (const cpp_int  & l__val);
        cpp_int fld2_inst() const;
    
        typedef pu_cpp_int< 128 > fld3_inst_cpp_int_t;
        fld3_inst_cpp_int_t int_var__fld3_inst;
        void fld3_inst (const cpp_int  & l__val);
        cpp_int fld3_inst() const;
    
}; // cap_ppa_csr_cfg_spare_ecc_t
    
class cap_ppa_csr_cfg_spare_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_cfg_spare_t(string name = "cap_ppa_csr_cfg_spare_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_cfg_spare_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 128 > fld0_inst_cpp_int_t;
        fld0_inst_cpp_int_t int_var__fld0_inst;
        void fld0_inst (const cpp_int  & l__val);
        cpp_int fld0_inst() const;
    
        typedef pu_cpp_int< 128 > fld1_inst_cpp_int_t;
        fld1_inst_cpp_int_t int_var__fld1_inst;
        void fld1_inst (const cpp_int  & l__val);
        cpp_int fld1_inst() const;
    
        typedef pu_cpp_int< 128 > fld2_inst_cpp_int_t;
        fld2_inst_cpp_int_t int_var__fld2_inst;
        void fld2_inst (const cpp_int  & l__val);
        cpp_int fld2_inst() const;
    
        typedef pu_cpp_int< 128 > fld3_inst_cpp_int_t;
        fld3_inst_cpp_int_t int_var__fld3_inst;
        void fld3_inst (const cpp_int  & l__val);
        cpp_int fld3_inst() const;
    
}; // cap_ppa_csr_cfg_spare_t
    
class cap_ppa_csr_sta_qos_counter_15_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_sta_qos_counter_15_t(string name = "cap_ppa_csr_sta_qos_counter_15_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_sta_qos_counter_15_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_ppa_csr_sta_qos_counter_15_t
    
class cap_ppa_csr_sta_qos_counter_14_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_sta_qos_counter_14_t(string name = "cap_ppa_csr_sta_qos_counter_14_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_sta_qos_counter_14_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_ppa_csr_sta_qos_counter_14_t
    
class cap_ppa_csr_sta_qos_counter_13_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_sta_qos_counter_13_t(string name = "cap_ppa_csr_sta_qos_counter_13_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_sta_qos_counter_13_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_ppa_csr_sta_qos_counter_13_t
    
class cap_ppa_csr_sta_qos_counter_12_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_sta_qos_counter_12_t(string name = "cap_ppa_csr_sta_qos_counter_12_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_sta_qos_counter_12_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_ppa_csr_sta_qos_counter_12_t
    
class cap_ppa_csr_sta_qos_counter_11_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_sta_qos_counter_11_t(string name = "cap_ppa_csr_sta_qos_counter_11_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_sta_qos_counter_11_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_ppa_csr_sta_qos_counter_11_t
    
class cap_ppa_csr_sta_qos_counter_10_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_sta_qos_counter_10_t(string name = "cap_ppa_csr_sta_qos_counter_10_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_sta_qos_counter_10_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_ppa_csr_sta_qos_counter_10_t
    
class cap_ppa_csr_sta_qos_counter_9_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_sta_qos_counter_9_t(string name = "cap_ppa_csr_sta_qos_counter_9_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_sta_qos_counter_9_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_ppa_csr_sta_qos_counter_9_t
    
class cap_ppa_csr_sta_qos_counter_8_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_sta_qos_counter_8_t(string name = "cap_ppa_csr_sta_qos_counter_8_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_sta_qos_counter_8_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_ppa_csr_sta_qos_counter_8_t
    
class cap_ppa_csr_sta_qos_counter_7_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_sta_qos_counter_7_t(string name = "cap_ppa_csr_sta_qos_counter_7_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_sta_qos_counter_7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_ppa_csr_sta_qos_counter_7_t
    
class cap_ppa_csr_sta_qos_counter_6_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_sta_qos_counter_6_t(string name = "cap_ppa_csr_sta_qos_counter_6_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_sta_qos_counter_6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_ppa_csr_sta_qos_counter_6_t
    
class cap_ppa_csr_sta_qos_counter_5_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_sta_qos_counter_5_t(string name = "cap_ppa_csr_sta_qos_counter_5_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_sta_qos_counter_5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_ppa_csr_sta_qos_counter_5_t
    
class cap_ppa_csr_sta_qos_counter_4_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_sta_qos_counter_4_t(string name = "cap_ppa_csr_sta_qos_counter_4_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_sta_qos_counter_4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_ppa_csr_sta_qos_counter_4_t
    
class cap_ppa_csr_sta_qos_counter_3_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_sta_qos_counter_3_t(string name = "cap_ppa_csr_sta_qos_counter_3_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_sta_qos_counter_3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_ppa_csr_sta_qos_counter_3_t
    
class cap_ppa_csr_sta_qos_counter_2_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_sta_qos_counter_2_t(string name = "cap_ppa_csr_sta_qos_counter_2_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_sta_qos_counter_2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_ppa_csr_sta_qos_counter_2_t
    
class cap_ppa_csr_sta_qos_counter_1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_sta_qos_counter_1_t(string name = "cap_ppa_csr_sta_qos_counter_1_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_sta_qos_counter_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_ppa_csr_sta_qos_counter_1_t
    
class cap_ppa_csr_sta_qos_counter_0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_sta_qos_counter_0_t(string name = "cap_ppa_csr_sta_qos_counter_0_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_sta_qos_counter_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 64 > val_cpp_int_t;
        val_cpp_int_t int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_ppa_csr_sta_qos_counter_0_t
    
class cap_ppa_csr_CNT_ppa_pe9_phv_out_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_pe9_phv_out_t(string name = "cap_ppa_csr_CNT_ppa_pe9_phv_out_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_pe9_phv_out_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        sop_cpp_int_t int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        eop_cpp_int_t int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe9_phv_out_t
    
class cap_ppa_csr_CNT_ppa_pe9_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_pe9_t(string name = "cap_ppa_csr_CNT_ppa_pe9_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_pe9_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > pkt_release_cpp_int_t;
        pkt_release_cpp_int_t int_var__pkt_release;
        void pkt_release (const cpp_int  & l__val);
        cpp_int pkt_release() const;
    
        typedef pu_cpp_int< 32 > ohi_out_cpp_int_t;
        ohi_out_cpp_int_t int_var__ohi_out;
        void ohi_out (const cpp_int  & l__val);
        cpp_int ohi_out() const;
    
}; // cap_ppa_csr_CNT_ppa_pe9_t
    
class cap_ppa_csr_CNT_ppa_pe9_pkt_in_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_pe9_pkt_in_t(string name = "cap_ppa_csr_CNT_ppa_pe9_pkt_in_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_pe9_pkt_in_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        sop_cpp_int_t int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        eop_cpp_int_t int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe9_pkt_in_t
    
class cap_ppa_csr_CNT_ppa_pe8_phv_out_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_pe8_phv_out_t(string name = "cap_ppa_csr_CNT_ppa_pe8_phv_out_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_pe8_phv_out_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        sop_cpp_int_t int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        eop_cpp_int_t int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe8_phv_out_t
    
class cap_ppa_csr_CNT_ppa_pe8_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_pe8_t(string name = "cap_ppa_csr_CNT_ppa_pe8_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_pe8_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > pkt_release_cpp_int_t;
        pkt_release_cpp_int_t int_var__pkt_release;
        void pkt_release (const cpp_int  & l__val);
        cpp_int pkt_release() const;
    
        typedef pu_cpp_int< 32 > ohi_out_cpp_int_t;
        ohi_out_cpp_int_t int_var__ohi_out;
        void ohi_out (const cpp_int  & l__val);
        cpp_int ohi_out() const;
    
}; // cap_ppa_csr_CNT_ppa_pe8_t
    
class cap_ppa_csr_CNT_ppa_pe8_pkt_in_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_pe8_pkt_in_t(string name = "cap_ppa_csr_CNT_ppa_pe8_pkt_in_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_pe8_pkt_in_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        sop_cpp_int_t int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        eop_cpp_int_t int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe8_pkt_in_t
    
class cap_ppa_csr_CNT_ppa_pe7_phv_out_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_pe7_phv_out_t(string name = "cap_ppa_csr_CNT_ppa_pe7_phv_out_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_pe7_phv_out_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        sop_cpp_int_t int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        eop_cpp_int_t int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe7_phv_out_t
    
class cap_ppa_csr_CNT_ppa_pe7_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_pe7_t(string name = "cap_ppa_csr_CNT_ppa_pe7_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_pe7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > pkt_release_cpp_int_t;
        pkt_release_cpp_int_t int_var__pkt_release;
        void pkt_release (const cpp_int  & l__val);
        cpp_int pkt_release() const;
    
        typedef pu_cpp_int< 32 > ohi_out_cpp_int_t;
        ohi_out_cpp_int_t int_var__ohi_out;
        void ohi_out (const cpp_int  & l__val);
        cpp_int ohi_out() const;
    
}; // cap_ppa_csr_CNT_ppa_pe7_t
    
class cap_ppa_csr_CNT_ppa_pe7_pkt_in_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_pe7_pkt_in_t(string name = "cap_ppa_csr_CNT_ppa_pe7_pkt_in_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_pe7_pkt_in_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        sop_cpp_int_t int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        eop_cpp_int_t int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe7_pkt_in_t
    
class cap_ppa_csr_CNT_ppa_pe6_phv_out_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_pe6_phv_out_t(string name = "cap_ppa_csr_CNT_ppa_pe6_phv_out_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_pe6_phv_out_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        sop_cpp_int_t int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        eop_cpp_int_t int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe6_phv_out_t
    
class cap_ppa_csr_CNT_ppa_pe6_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_pe6_t(string name = "cap_ppa_csr_CNT_ppa_pe6_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_pe6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > pkt_release_cpp_int_t;
        pkt_release_cpp_int_t int_var__pkt_release;
        void pkt_release (const cpp_int  & l__val);
        cpp_int pkt_release() const;
    
        typedef pu_cpp_int< 32 > ohi_out_cpp_int_t;
        ohi_out_cpp_int_t int_var__ohi_out;
        void ohi_out (const cpp_int  & l__val);
        cpp_int ohi_out() const;
    
}; // cap_ppa_csr_CNT_ppa_pe6_t
    
class cap_ppa_csr_CNT_ppa_pe6_pkt_in_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_pe6_pkt_in_t(string name = "cap_ppa_csr_CNT_ppa_pe6_pkt_in_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_pe6_pkt_in_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        sop_cpp_int_t int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        eop_cpp_int_t int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe6_pkt_in_t
    
class cap_ppa_csr_CNT_ppa_pe5_phv_out_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_pe5_phv_out_t(string name = "cap_ppa_csr_CNT_ppa_pe5_phv_out_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_pe5_phv_out_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        sop_cpp_int_t int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        eop_cpp_int_t int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe5_phv_out_t
    
class cap_ppa_csr_CNT_ppa_pe5_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_pe5_t(string name = "cap_ppa_csr_CNT_ppa_pe5_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_pe5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > pkt_release_cpp_int_t;
        pkt_release_cpp_int_t int_var__pkt_release;
        void pkt_release (const cpp_int  & l__val);
        cpp_int pkt_release() const;
    
        typedef pu_cpp_int< 32 > ohi_out_cpp_int_t;
        ohi_out_cpp_int_t int_var__ohi_out;
        void ohi_out (const cpp_int  & l__val);
        cpp_int ohi_out() const;
    
}; // cap_ppa_csr_CNT_ppa_pe5_t
    
class cap_ppa_csr_CNT_ppa_pe5_pkt_in_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_pe5_pkt_in_t(string name = "cap_ppa_csr_CNT_ppa_pe5_pkt_in_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_pe5_pkt_in_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        sop_cpp_int_t int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        eop_cpp_int_t int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe5_pkt_in_t
    
class cap_ppa_csr_CNT_ppa_pe4_phv_out_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_pe4_phv_out_t(string name = "cap_ppa_csr_CNT_ppa_pe4_phv_out_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_pe4_phv_out_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        sop_cpp_int_t int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        eop_cpp_int_t int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe4_phv_out_t
    
class cap_ppa_csr_CNT_ppa_pe4_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_pe4_t(string name = "cap_ppa_csr_CNT_ppa_pe4_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_pe4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > pkt_release_cpp_int_t;
        pkt_release_cpp_int_t int_var__pkt_release;
        void pkt_release (const cpp_int  & l__val);
        cpp_int pkt_release() const;
    
        typedef pu_cpp_int< 32 > ohi_out_cpp_int_t;
        ohi_out_cpp_int_t int_var__ohi_out;
        void ohi_out (const cpp_int  & l__val);
        cpp_int ohi_out() const;
    
}; // cap_ppa_csr_CNT_ppa_pe4_t
    
class cap_ppa_csr_CNT_ppa_pe4_pkt_in_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_pe4_pkt_in_t(string name = "cap_ppa_csr_CNT_ppa_pe4_pkt_in_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_pe4_pkt_in_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        sop_cpp_int_t int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        eop_cpp_int_t int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe4_pkt_in_t
    
class cap_ppa_csr_CNT_ppa_pe3_phv_out_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_pe3_phv_out_t(string name = "cap_ppa_csr_CNT_ppa_pe3_phv_out_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_pe3_phv_out_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        sop_cpp_int_t int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        eop_cpp_int_t int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe3_phv_out_t
    
class cap_ppa_csr_CNT_ppa_pe3_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_pe3_t(string name = "cap_ppa_csr_CNT_ppa_pe3_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_pe3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > pkt_release_cpp_int_t;
        pkt_release_cpp_int_t int_var__pkt_release;
        void pkt_release (const cpp_int  & l__val);
        cpp_int pkt_release() const;
    
        typedef pu_cpp_int< 32 > ohi_out_cpp_int_t;
        ohi_out_cpp_int_t int_var__ohi_out;
        void ohi_out (const cpp_int  & l__val);
        cpp_int ohi_out() const;
    
}; // cap_ppa_csr_CNT_ppa_pe3_t
    
class cap_ppa_csr_CNT_ppa_pe3_pkt_in_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_pe3_pkt_in_t(string name = "cap_ppa_csr_CNT_ppa_pe3_pkt_in_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_pe3_pkt_in_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        sop_cpp_int_t int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        eop_cpp_int_t int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe3_pkt_in_t
    
class cap_ppa_csr_CNT_ppa_pe2_phv_out_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_pe2_phv_out_t(string name = "cap_ppa_csr_CNT_ppa_pe2_phv_out_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_pe2_phv_out_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        sop_cpp_int_t int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        eop_cpp_int_t int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe2_phv_out_t
    
class cap_ppa_csr_CNT_ppa_pe2_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_pe2_t(string name = "cap_ppa_csr_CNT_ppa_pe2_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_pe2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > pkt_release_cpp_int_t;
        pkt_release_cpp_int_t int_var__pkt_release;
        void pkt_release (const cpp_int  & l__val);
        cpp_int pkt_release() const;
    
        typedef pu_cpp_int< 32 > ohi_out_cpp_int_t;
        ohi_out_cpp_int_t int_var__ohi_out;
        void ohi_out (const cpp_int  & l__val);
        cpp_int ohi_out() const;
    
}; // cap_ppa_csr_CNT_ppa_pe2_t
    
class cap_ppa_csr_CNT_ppa_pe2_pkt_in_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_pe2_pkt_in_t(string name = "cap_ppa_csr_CNT_ppa_pe2_pkt_in_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_pe2_pkt_in_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        sop_cpp_int_t int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        eop_cpp_int_t int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe2_pkt_in_t
    
class cap_ppa_csr_CNT_ppa_pe1_phv_out_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_pe1_phv_out_t(string name = "cap_ppa_csr_CNT_ppa_pe1_phv_out_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_pe1_phv_out_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        sop_cpp_int_t int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        eop_cpp_int_t int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe1_phv_out_t
    
class cap_ppa_csr_CNT_ppa_pe1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_pe1_t(string name = "cap_ppa_csr_CNT_ppa_pe1_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_pe1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > pkt_release_cpp_int_t;
        pkt_release_cpp_int_t int_var__pkt_release;
        void pkt_release (const cpp_int  & l__val);
        cpp_int pkt_release() const;
    
        typedef pu_cpp_int< 32 > ohi_out_cpp_int_t;
        ohi_out_cpp_int_t int_var__ohi_out;
        void ohi_out (const cpp_int  & l__val);
        cpp_int ohi_out() const;
    
}; // cap_ppa_csr_CNT_ppa_pe1_t
    
class cap_ppa_csr_CNT_ppa_pe1_pkt_in_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_pe1_pkt_in_t(string name = "cap_ppa_csr_CNT_ppa_pe1_pkt_in_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_pe1_pkt_in_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        sop_cpp_int_t int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        eop_cpp_int_t int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe1_pkt_in_t
    
class cap_ppa_csr_CNT_ppa_pe0_phv_out_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_pe0_phv_out_t(string name = "cap_ppa_csr_CNT_ppa_pe0_phv_out_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_pe0_phv_out_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        sop_cpp_int_t int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        eop_cpp_int_t int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe0_phv_out_t
    
class cap_ppa_csr_CNT_ppa_pe0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_pe0_t(string name = "cap_ppa_csr_CNT_ppa_pe0_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_pe0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > pkt_release_cpp_int_t;
        pkt_release_cpp_int_t int_var__pkt_release;
        void pkt_release (const cpp_int  & l__val);
        cpp_int pkt_release() const;
    
        typedef pu_cpp_int< 32 > ohi_out_cpp_int_t;
        ohi_out_cpp_int_t int_var__ohi_out;
        void ohi_out (const cpp_int  & l__val);
        cpp_int ohi_out() const;
    
}; // cap_ppa_csr_CNT_ppa_pe0_t
    
class cap_ppa_csr_CNT_ppa_pe0_pkt_in_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_pe0_pkt_in_t(string name = "cap_ppa_csr_CNT_ppa_pe0_pkt_in_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_pe0_pkt_in_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        sop_cpp_int_t int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        eop_cpp_int_t int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pe0_pkt_in_t
    
class cap_ppa_csr_CNT_ppa_ohi_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_ohi_t(string name = "cap_ppa_csr_CNT_ppa_ohi_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_ohi_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        sop_cpp_int_t int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        eop_cpp_int_t int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_ohi_t
    
class cap_ppa_csr_CNT_ppa_ma_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_ma_t(string name = "cap_ppa_csr_CNT_ppa_ma_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_ma_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        sop_cpp_int_t int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        eop_cpp_int_t int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_ma_t
    
class cap_ppa_csr_CNT_ppa_dp_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_dp_t(string name = "cap_ppa_csr_CNT_ppa_dp_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_dp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        sop_cpp_int_t int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        eop_cpp_int_t int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_dp_t
    
class cap_ppa_csr_CNT_ppa_pb_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_CNT_ppa_pb_t(string name = "cap_ppa_csr_CNT_ppa_pb_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_CNT_ppa_pb_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sop_cpp_int_t;
        sop_cpp_int_t int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 32 > eop_cpp_int_t;
        eop_cpp_int_t int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
}; // cap_ppa_csr_CNT_ppa_pb_t
    
class cap_ppa_csr_cfg_free_cell_init_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_cfg_free_cell_init_t(string name = "cap_ppa_csr_cfg_free_cell_init_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_cfg_free_cell_init_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > vld_cpp_int_t;
        vld_cpp_int_t int_var__vld;
        void vld (const cpp_int  & l__val);
        cpp_int vld() const;
    
}; // cap_ppa_csr_cfg_free_cell_init_t
    
class cap_ppa_csr_cfg_align_chk_profile_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_cfg_align_chk_profile_t(string name = "cap_ppa_csr_cfg_align_chk_profile_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_cfg_align_chk_profile_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 15 > align_chk_prof_cpp_int_t;
        align_chk_prof_cpp_int_t int_var__align_chk_prof;
        void align_chk_prof (const cpp_int  & l__val);
        cpp_int align_chk_prof() const;
    
}; // cap_ppa_csr_cfg_align_chk_profile_t
    
class cap_ppa_csr_cfg_len_chk_profile_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_cfg_len_chk_profile_t(string name = "cap_ppa_csr_cfg_len_chk_profile_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_cfg_len_chk_profile_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 25 > len_chk_prof_cpp_int_t;
        len_chk_prof_cpp_int_t int_var__len_chk_prof;
        void len_chk_prof (const cpp_int  & l__val);
        cpp_int len_chk_prof() const;
    
}; // cap_ppa_csr_cfg_len_chk_profile_t
    
class cap_ppa_csr_cfg_csum_phdr_profile_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_cfg_csum_phdr_profile_t(string name = "cap_ppa_csr_cfg_csum_phdr_profile_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_cfg_csum_phdr_profile_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 15 > fld0_inst_cpp_int_t;
        fld0_inst_cpp_int_t int_var__fld0_inst;
        void fld0_inst (const cpp_int  & l__val);
        cpp_int fld0_inst() const;
    
        typedef pu_cpp_int< 15 > fld1_inst_cpp_int_t;
        fld1_inst_cpp_int_t int_var__fld1_inst;
        void fld1_inst (const cpp_int  & l__val);
        cpp_int fld1_inst() const;
    
        typedef pu_cpp_int< 15 > fld2_inst_cpp_int_t;
        fld2_inst_cpp_int_t int_var__fld2_inst;
        void fld2_inst (const cpp_int  & l__val);
        cpp_int fld2_inst() const;
    
        typedef pu_cpp_int< 15 > fld3_inst_cpp_int_t;
        fld3_inst_cpp_int_t int_var__fld3_inst;
        void fld3_inst (const cpp_int  & l__val);
        cpp_int fld3_inst() const;
    
}; // cap_ppa_csr_cfg_csum_phdr_profile_t
    
class cap_ppa_csr_cfg_csum_profile_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_cfg_csum_profile_t(string name = "cap_ppa_csr_cfg_csum_profile_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_cfg_csum_profile_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 47 > csum_prof_cpp_int_t;
        csum_prof_cpp_int_t int_var__csum_prof;
        void csum_prof (const cpp_int  & l__val);
        cpp_int csum_prof() const;
    
}; // cap_ppa_csr_cfg_csum_profile_t
    
class cap_ppa_csr_cfg_crc_mask_profile_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_cfg_crc_mask_profile_t(string name = "cap_ppa_csr_cfg_crc_mask_profile_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_cfg_crc_mask_profile_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > fld0_inst_cpp_int_t;
        fld0_inst_cpp_int_t int_var__fld0_inst;
        void fld0_inst (const cpp_int  & l__val);
        cpp_int fld0_inst() const;
    
        typedef pu_cpp_int< 16 > fld1_inst_cpp_int_t;
        fld1_inst_cpp_int_t int_var__fld1_inst;
        void fld1_inst (const cpp_int  & l__val);
        cpp_int fld1_inst() const;
    
        typedef pu_cpp_int< 16 > fld2_inst_cpp_int_t;
        fld2_inst_cpp_int_t int_var__fld2_inst;
        void fld2_inst (const cpp_int  & l__val);
        cpp_int fld2_inst() const;
    
        typedef pu_cpp_int< 16 > fld3_inst_cpp_int_t;
        fld3_inst_cpp_int_t int_var__fld3_inst;
        void fld3_inst (const cpp_int  & l__val);
        cpp_int fld3_inst() const;
    
        typedef pu_cpp_int< 16 > fld4_inst_cpp_int_t;
        fld4_inst_cpp_int_t int_var__fld4_inst;
        void fld4_inst (const cpp_int  & l__val);
        cpp_int fld4_inst() const;
    
}; // cap_ppa_csr_cfg_crc_mask_profile_t
    
class cap_ppa_csr_cfg_crc_profile_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_cfg_crc_profile_t(string name = "cap_ppa_csr_cfg_crc_profile_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_cfg_crc_profile_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 14 > len_mask_cpp_int_t;
        len_mask_cpp_int_t int_var__len_mask;
        void len_mask (const cpp_int  & l__val);
        cpp_int len_mask() const;
    
        typedef pu_cpp_int< 1 > len_shift_left_cpp_int_t;
        len_shift_left_cpp_int_t int_var__len_shift_left;
        void len_shift_left (const cpp_int  & l__val);
        cpp_int len_shift_left() const;
    
        typedef pu_cpp_int< 3 > len_shift_val_cpp_int_t;
        len_shift_val_cpp_int_t int_var__len_shift_val;
        void len_shift_val (const cpp_int  & l__val);
        cpp_int len_shift_val() const;
    
        typedef pu_cpp_int< 1 > addsub_start_cpp_int_t;
        addsub_start_cpp_int_t int_var__addsub_start;
        void addsub_start (const cpp_int  & l__val);
        cpp_int addsub_start() const;
    
        typedef pu_cpp_int< 6 > start_adj_cpp_int_t;
        start_adj_cpp_int_t int_var__start_adj;
        void start_adj (const cpp_int  & l__val);
        cpp_int start_adj() const;
    
        typedef pu_cpp_int< 1 > addsub_end_cpp_int_t;
        addsub_end_cpp_int_t int_var__addsub_end;
        void addsub_end (const cpp_int  & l__val);
        cpp_int addsub_end() const;
    
        typedef pu_cpp_int< 6 > end_adj_cpp_int_t;
        end_adj_cpp_int_t int_var__end_adj;
        void end_adj (const cpp_int  & l__val);
        cpp_int end_adj() const;
    
        typedef pu_cpp_int< 1 > addsub_mask_cpp_int_t;
        addsub_mask_cpp_int_t int_var__addsub_mask;
        void addsub_mask (const cpp_int  & l__val);
        cpp_int addsub_mask() const;
    
        typedef pu_cpp_int< 6 > mask_adj_cpp_int_t;
        mask_adj_cpp_int_t int_var__mask_adj;
        void mask_adj (const cpp_int  & l__val);
        cpp_int mask_adj() const;
    
        typedef pu_cpp_int< 1 > end_eop_cpp_int_t;
        end_eop_cpp_int_t int_var__end_eop;
        void end_eop (const cpp_int  & l__val);
        cpp_int end_eop() const;
    
        typedef pu_cpp_int< 2 > mask_prof_sel_cpp_int_t;
        mask_prof_sel_cpp_int_t int_var__mask_prof_sel;
        void mask_prof_sel (const cpp_int  & l__val);
        cpp_int mask_prof_sel() const;
    
}; // cap_ppa_csr_cfg_crc_profile_t
    
class cap_ppa_csr_sta_bndl1_state_lkp_tcam_ind_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_sta_bndl1_state_lkp_tcam_ind_t(string name = "cap_ppa_csr_sta_bndl1_state_lkp_tcam_ind_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_sta_bndl1_state_lkp_tcam_ind_t();
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
    
        typedef pu_cpp_int< 9 > hit_addr_cpp_int_t;
        hit_addr_cpp_int_t int_var__hit_addr;
        void hit_addr (const cpp_int  & l__val);
        cpp_int hit_addr() const;
    
}; // cap_ppa_csr_sta_bndl1_state_lkp_tcam_ind_t
    
class cap_ppa_csr_cfg_bndl1_state_lkp_tcam_ind_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_cfg_bndl1_state_lkp_tcam_ind_t(string name = "cap_ppa_csr_cfg_bndl1_state_lkp_tcam_ind_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_cfg_bndl1_state_lkp_tcam_ind_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 64 > key_cpp_int_t;
        key_cpp_int_t int_var__key;
        void key (const cpp_int  & l__val);
        cpp_int key() const;
    
        typedef pu_cpp_int< 64 > mask_cpp_int_t;
        mask_cpp_int_t int_var__mask;
        void mask (const cpp_int  & l__val);
        cpp_int mask() const;
    
}; // cap_ppa_csr_cfg_bndl1_state_lkp_tcam_ind_t
    
class cap_ppa_csr_cfg_bndl1_state_lkp_tcam_grst_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_cfg_bndl1_state_lkp_tcam_grst_t(string name = "cap_ppa_csr_cfg_bndl1_state_lkp_tcam_grst_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_cfg_bndl1_state_lkp_tcam_grst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > vld_cpp_int_t;
        vld_cpp_int_t int_var__vld;
        void vld (const cpp_int  & l__val);
        cpp_int vld() const;
    
}; // cap_ppa_csr_cfg_bndl1_state_lkp_tcam_grst_t
    
class cap_ppa_csr_sta_bndl0_state_lkp_tcam_ind_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_sta_bndl0_state_lkp_tcam_ind_t(string name = "cap_ppa_csr_sta_bndl0_state_lkp_tcam_ind_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_sta_bndl0_state_lkp_tcam_ind_t();
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
    
        typedef pu_cpp_int< 9 > hit_addr_cpp_int_t;
        hit_addr_cpp_int_t int_var__hit_addr;
        void hit_addr (const cpp_int  & l__val);
        cpp_int hit_addr() const;
    
}; // cap_ppa_csr_sta_bndl0_state_lkp_tcam_ind_t
    
class cap_ppa_csr_cfg_bndl0_state_lkp_tcam_ind_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_cfg_bndl0_state_lkp_tcam_ind_t(string name = "cap_ppa_csr_cfg_bndl0_state_lkp_tcam_ind_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_cfg_bndl0_state_lkp_tcam_ind_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 64 > key_cpp_int_t;
        key_cpp_int_t int_var__key;
        void key (const cpp_int  & l__val);
        cpp_int key() const;
    
        typedef pu_cpp_int< 64 > mask_cpp_int_t;
        mask_cpp_int_t int_var__mask;
        void mask (const cpp_int  & l__val);
        cpp_int mask() const;
    
}; // cap_ppa_csr_cfg_bndl0_state_lkp_tcam_ind_t
    
class cap_ppa_csr_cfg_bndl0_state_lkp_tcam_grst_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_cfg_bndl0_state_lkp_tcam_grst_t(string name = "cap_ppa_csr_cfg_bndl0_state_lkp_tcam_grst_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_cfg_bndl0_state_lkp_tcam_grst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > vld_cpp_int_t;
        vld_cpp_int_t int_var__vld;
        void vld (const cpp_int  & l__val);
        cpp_int vld() const;
    
}; // cap_ppa_csr_cfg_bndl0_state_lkp_tcam_grst_t
    
class cap_ppa_csr_cfg_init_profile_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_cfg_init_profile_t(string name = "cap_ppa_csr_cfg_init_profile_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_cfg_init_profile_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 6 > curr_offset_cpp_int_t;
        curr_offset_cpp_int_t int_var__curr_offset;
        void curr_offset (const cpp_int  & l__val);
        cpp_int curr_offset() const;
    
        typedef pu_cpp_int< 9 > state_cpp_int_t;
        state_cpp_int_t int_var__state;
        void state (const cpp_int  & l__val);
        cpp_int state() const;
    
        typedef pu_cpp_int< 6 > lkp_val_pkt_idx0_cpp_int_t;
        lkp_val_pkt_idx0_cpp_int_t int_var__lkp_val_pkt_idx0;
        void lkp_val_pkt_idx0 (const cpp_int  & l__val);
        cpp_int lkp_val_pkt_idx0() const;
    
        typedef pu_cpp_int< 6 > lkp_val_pkt_idx1_cpp_int_t;
        lkp_val_pkt_idx1_cpp_int_t int_var__lkp_val_pkt_idx1;
        void lkp_val_pkt_idx1 (const cpp_int  & l__val);
        cpp_int lkp_val_pkt_idx1() const;
    
        typedef pu_cpp_int< 6 > lkp_val_pkt_idx2_cpp_int_t;
        lkp_val_pkt_idx2_cpp_int_t int_var__lkp_val_pkt_idx2;
        void lkp_val_pkt_idx2 (const cpp_int  & l__val);
        cpp_int lkp_val_pkt_idx2() const;
    
}; // cap_ppa_csr_cfg_init_profile_t
    
class cap_ppa_csr_cfg_ctrl_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_cfg_ctrl_t(string name = "cap_ppa_csr_cfg_ctrl_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_cfg_ctrl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > pe_enable_cpp_int_t;
        pe_enable_cpp_int_t int_var__pe_enable;
        void pe_enable (const cpp_int  & l__val);
        cpp_int pe_enable() const;
    
        typedef pu_cpp_int< 9 > parse_loop_cnt_cpp_int_t;
        parse_loop_cnt_cpp_int_t int_var__parse_loop_cnt;
        void parse_loop_cnt (const cpp_int  & l__val);
        cpp_int parse_loop_cnt() const;
    
        typedef pu_cpp_int< 3 > num_phv_flit_cpp_int_t;
        num_phv_flit_cpp_int_t int_var__num_phv_flit;
        void num_phv_flit (const cpp_int  & l__val);
        cpp_int num_phv_flit() const;
    
        typedef pu_cpp_int< 9 > state_lkp_catchall_entry_cpp_int_t;
        state_lkp_catchall_entry_cpp_int_t int_var__state_lkp_catchall_entry;
        void state_lkp_catchall_entry (const cpp_int  & l__val);
        cpp_int state_lkp_catchall_entry() const;
    
        typedef pu_cpp_int< 3 > cpu_tcam_tdm_slot_cpp_int_t;
        cpu_tcam_tdm_slot_cpp_int_t int_var__cpu_tcam_tdm_slot;
        void cpu_tcam_tdm_slot (const cpp_int  & l__val);
        cpp_int cpu_tcam_tdm_slot() const;
    
}; // cap_ppa_csr_cfg_ctrl_t
    
class cap_ppa_csr_base_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_base_t(string name = "cap_ppa_csr_base_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_base_t();
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
    
}; // cap_ppa_csr_base_t
    
class cap_ppa_csr_t : public cap_block_base {
    public:
        cpp_int_helper hlp;
    
        cap_ppa_csr_t(string name = "cap_ppa_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_ppa_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_ppa_csr_base_t base;
    
        cap_ppa_csr_cfg_ctrl_t cfg_ctrl;
    
        cap_ppa_csr_cfg_init_profile_t cfg_init_profile[16];
        int get_depth_cfg_init_profile() { return 16; }
    
        cap_ppa_csr_cfg_bndl0_state_lkp_tcam_grst_t cfg_bndl0_state_lkp_tcam_grst;
    
        cap_ppa_csr_cfg_bndl0_state_lkp_tcam_ind_t cfg_bndl0_state_lkp_tcam_ind;
    
        cap_ppa_csr_sta_bndl0_state_lkp_tcam_ind_t sta_bndl0_state_lkp_tcam_ind;
    
        cap_ppa_csr_cfg_bndl1_state_lkp_tcam_grst_t cfg_bndl1_state_lkp_tcam_grst;
    
        cap_ppa_csr_cfg_bndl1_state_lkp_tcam_ind_t cfg_bndl1_state_lkp_tcam_ind;
    
        cap_ppa_csr_sta_bndl1_state_lkp_tcam_ind_t sta_bndl1_state_lkp_tcam_ind;
    
        cap_ppa_csr_cfg_crc_profile_t cfg_crc_profile[8];
        int get_depth_cfg_crc_profile() { return 8; }
    
        cap_ppa_csr_cfg_crc_mask_profile_t cfg_crc_mask_profile[4];
        int get_depth_cfg_crc_mask_profile() { return 4; }
    
        cap_ppa_csr_cfg_csum_profile_t cfg_csum_profile[8];
        int get_depth_cfg_csum_profile() { return 8; }
    
        cap_ppa_csr_cfg_csum_phdr_profile_t cfg_csum_phdr_profile[8];
        int get_depth_cfg_csum_phdr_profile() { return 8; }
    
        cap_ppa_csr_cfg_len_chk_profile_t cfg_len_chk_profile[4];
        int get_depth_cfg_len_chk_profile() { return 4; }
    
        cap_ppa_csr_cfg_align_chk_profile_t cfg_align_chk_profile[4];
        int get_depth_cfg_align_chk_profile() { return 4; }
    
        cap_ppa_csr_cfg_free_cell_init_t cfg_free_cell_init;
    
        cap_ppa_csr_CNT_ppa_pb_t CNT_ppa_pb;
    
        cap_ppa_csr_CNT_ppa_dp_t CNT_ppa_dp;
    
        cap_ppa_csr_CNT_ppa_ma_t CNT_ppa_ma;
    
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
    
        cap_ppa_csr_sta_qos_counter_0_t sta_qos_counter_0;
    
        cap_ppa_csr_sta_qos_counter_1_t sta_qos_counter_1;
    
        cap_ppa_csr_sta_qos_counter_2_t sta_qos_counter_2;
    
        cap_ppa_csr_sta_qos_counter_3_t sta_qos_counter_3;
    
        cap_ppa_csr_sta_qos_counter_4_t sta_qos_counter_4;
    
        cap_ppa_csr_sta_qos_counter_5_t sta_qos_counter_5;
    
        cap_ppa_csr_sta_qos_counter_6_t sta_qos_counter_6;
    
        cap_ppa_csr_sta_qos_counter_7_t sta_qos_counter_7;
    
        cap_ppa_csr_sta_qos_counter_8_t sta_qos_counter_8;
    
        cap_ppa_csr_sta_qos_counter_9_t sta_qos_counter_9;
    
        cap_ppa_csr_sta_qos_counter_10_t sta_qos_counter_10;
    
        cap_ppa_csr_sta_qos_counter_11_t sta_qos_counter_11;
    
        cap_ppa_csr_sta_qos_counter_12_t sta_qos_counter_12;
    
        cap_ppa_csr_sta_qos_counter_13_t sta_qos_counter_13;
    
        cap_ppa_csr_sta_qos_counter_14_t sta_qos_counter_14;
    
        cap_ppa_csr_sta_qos_counter_15_t sta_qos_counter_15;
    
        cap_ppa_csr_cfg_spare_t cfg_spare[4];
        int get_depth_cfg_spare() { return 4; }
    
        cap_ppa_csr_cfg_spare_ecc_t cfg_spare_ecc[2];
        int get_depth_cfg_spare_ecc() { return 2; }
    
        cap_ppa_csr_cfg_debug_port_t cfg_debug_port;
    
        cap_ppa_csr_cfg_spare_dbg_t cfg_spare_dbg;
    
        cap_ppa_csr_dhs_bndl0_state_lkp_tcam_t dhs_bndl0_state_lkp_tcam;
    
        cap_ppa_csr_dhs_bndl0_state_lkp_tcam_ind_t dhs_bndl0_state_lkp_tcam_ind;
    
        cap_ppa_csr_dhs_bndl0_state_lkp_sram_t dhs_bndl0_state_lkp_sram;
    
        cap_ppa_csr_dhs_bndl1_state_lkp_tcam_t dhs_bndl1_state_lkp_tcam;
    
        cap_ppa_csr_dhs_bndl1_state_lkp_tcam_ind_t dhs_bndl1_state_lkp_tcam_ind;
    
        cap_ppa_csr_dhs_bndl1_state_lkp_sram_t dhs_bndl1_state_lkp_sram;
    
}; // cap_ppa_csr_t
    
#endif // CAP_PPA_CSR_H
        