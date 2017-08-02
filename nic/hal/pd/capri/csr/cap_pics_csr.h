
#ifndef CAP_PICS_CSR_H
#define CAP_PICS_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_pics_csr_dhs_table_data_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pics_csr_dhs_table_data_entry_t(string name = "cap_pics_csr_dhs_table_data_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pics_csr_dhs_table_data_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 512 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // cap_pics_csr_dhs_table_data_entry_t
    
class cap_pics_csr_dhs_table_data_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pics_csr_dhs_table_data_t(string name = "cap_pics_csr_dhs_table_data_t", cap_csr_base *parent = 0);
        virtual ~cap_pics_csr_dhs_table_data_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pics_csr_dhs_table_data_entry_t entry;
    
}; // cap_pics_csr_dhs_table_data_t
    
class cap_pics_csr_dhs_table_address_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pics_csr_dhs_table_address_entry_t(string name = "cap_pics_csr_dhs_table_address_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pics_csr_dhs_table_address_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 64 > address_cpp_int_t;
        address_cpp_int_t int_var__address;
        void address (const cpp_int  & l__val);
        cpp_int address() const;
    
}; // cap_pics_csr_dhs_table_address_entry_t
    
class cap_pics_csr_dhs_table_address_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pics_csr_dhs_table_address_t(string name = "cap_pics_csr_dhs_table_address_t", cap_csr_base *parent = 0);
        virtual ~cap_pics_csr_dhs_table_address_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pics_csr_dhs_table_address_entry_t entry;
    
}; // cap_pics_csr_dhs_table_address_t
    
class cap_pics_csr_dhs_sram_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pics_csr_dhs_sram_entry_t(string name = "cap_pics_csr_dhs_sram_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pics_csr_dhs_sram_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pics_csr_dhs_sram_entry_t
    
class cap_pics_csr_dhs_sram_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pics_csr_dhs_sram_t(string name = "cap_pics_csr_dhs_sram_t", cap_csr_base *parent = 0);
        virtual ~cap_pics_csr_dhs_sram_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pics_csr_dhs_sram_entry_t entry[40960];
        int get_depth_entry() { return 40960; }
    
}; // cap_pics_csr_dhs_sram_t
    
class cap_pics_csr_cfg_meter_sram_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pics_csr_cfg_meter_sram_t(string name = "cap_pics_csr_cfg_meter_sram_t", cap_csr_base *parent = 0);
        virtual ~cap_pics_csr_cfg_meter_sram_t();
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
    
        typedef pu_cpp_int< 12 > start_cpp_int_t;
        start_cpp_int_t int_var__start;
        void start (const cpp_int  & l__val);
        cpp_int start() const;
    
        typedef pu_cpp_int< 12 > end_cpp_int_t;
        end_cpp_int_t int_var__end;
        void end (const cpp_int  & l__val);
        cpp_int end() const;
    
        typedef pu_cpp_int< 16 > scale_cpp_int_t;
        scale_cpp_int_t int_var__scale;
        void scale (const cpp_int  & l__val);
        cpp_int scale() const;
    
        typedef pu_cpp_int< 9 > opcode_cpp_int_t;
        opcode_cpp_int_t int_var__opcode;
        void opcode (const cpp_int  & l__val);
        cpp_int opcode() const;
    
}; // cap_pics_csr_cfg_meter_sram_t
    
class cap_pics_csr_cfg_meter_access_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pics_csr_cfg_meter_access_t(string name = "cap_pics_csr_cfg_meter_access_t", cap_csr_base *parent = 0);
        virtual ~cap_pics_csr_cfg_meter_access_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > max_cycle_cpp_int_t;
        max_cycle_cpp_int_t int_var__max_cycle;
        void max_cycle (const cpp_int  & l__val);
        cpp_int max_cycle() const;
    
        typedef pu_cpp_int< 12 > start_addr_cpp_int_t;
        start_addr_cpp_int_t int_var__start_addr;
        void start_addr (const cpp_int  & l__val);
        cpp_int start_addr() const;
    
        typedef pu_cpp_int< 12 > end_addr_cpp_int_t;
        end_addr_cpp_int_t int_var__end_addr;
        void end_addr (const cpp_int  & l__val);
        cpp_int end_addr() const;
    
}; // cap_pics_csr_cfg_meter_access_t
    
class cap_pics_csr_cfg_meter_timer_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pics_csr_cfg_meter_timer_t(string name = "cap_pics_csr_cfg_meter_timer_t", cap_csr_base *parent = 0);
        virtual ~cap_pics_csr_cfg_meter_timer_t();
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
    
        typedef pu_cpp_int< 32 > cycle_cpp_int_t;
        cycle_cpp_int_t int_var__cycle;
        void cycle (const cpp_int  & l__val);
        cpp_int cycle() const;
    
}; // cap_pics_csr_cfg_meter_timer_t
    
class cap_pics_csr_cfg_update_profile_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pics_csr_cfg_update_profile_t(string name = "cap_pics_csr_cfg_update_profile_t", cap_csr_base *parent = 0);
        virtual ~cap_pics_csr_cfg_update_profile_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 6 > oprd1_sel_cpp_int_t;
        oprd1_sel_cpp_int_t int_var__oprd1_sel;
        void oprd1_sel (const cpp_int  & l__val);
        cpp_int oprd1_sel() const;
    
        typedef pu_cpp_int< 6 > oprd2_sel_cpp_int_t;
        oprd2_sel_cpp_int_t int_var__oprd2_sel;
        void oprd2_sel (const cpp_int  & l__val);
        cpp_int oprd2_sel() const;
    
        typedef pu_cpp_int< 6 > oprd3_sel_cpp_int_t;
        oprd3_sel_cpp_int_t int_var__oprd3_sel;
        void oprd3_sel (const cpp_int  & l__val);
        cpp_int oprd3_sel() const;
    
        typedef pu_cpp_int< 5 > oprd1_wid_cpp_int_t;
        oprd1_wid_cpp_int_t int_var__oprd1_wid;
        void oprd1_wid (const cpp_int  & l__val);
        cpp_int oprd1_wid() const;
    
        typedef pu_cpp_int< 5 > oprd2_wid_cpp_int_t;
        oprd2_wid_cpp_int_t int_var__oprd2_wid;
        void oprd2_wid (const cpp_int  & l__val);
        cpp_int oprd2_wid() const;
    
        typedef pu_cpp_int< 5 > oprd3_wid_cpp_int_t;
        oprd3_wid_cpp_int_t int_var__oprd3_wid;
        void oprd3_wid (const cpp_int  & l__val);
        cpp_int oprd3_wid() const;
    
}; // cap_pics_csr_cfg_update_profile_t
    
class cap_pics_csr_cfg_table_profile_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pics_csr_cfg_table_profile_t(string name = "cap_pics_csr_cfg_table_profile_t", cap_csr_base *parent = 0);
        virtual ~cap_pics_csr_cfg_table_profile_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 20 > start_addr_cpp_int_t;
        start_addr_cpp_int_t int_var__start_addr;
        void start_addr (const cpp_int  & l__val);
        cpp_int start_addr() const;
    
        typedef pu_cpp_int< 20 > end_addr_cpp_int_t;
        end_addr_cpp_int_t int_var__end_addr;
        void end_addr (const cpp_int  & l__val);
        cpp_int end_addr() const;
    
        typedef pu_cpp_int< 6 > width_cpp_int_t;
        width_cpp_int_t int_var__width;
        void width (const cpp_int  & l__val);
        cpp_int width() const;
    
        typedef pu_cpp_int< 1 > hash_cpp_int_t;
        hash_cpp_int_t int_var__hash;
        void hash (const cpp_int  & l__val);
        cpp_int hash() const;
    
        typedef pu_cpp_int< 9 > opcode_cpp_int_t;
        opcode_cpp_int_t int_var__opcode;
        void opcode (const cpp_int  & l__val);
        cpp_int opcode() const;
    
        typedef pu_cpp_int< 1 > log2bkts_cpp_int_t;
        log2bkts_cpp_int_t int_var__log2bkts;
        void log2bkts (const cpp_int  & l__val);
        cpp_int log2bkts() const;
    
}; // cap_pics_csr_cfg_table_profile_t
    
class cap_pics_csr_cfg_sram_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pics_csr_cfg_sram_t(string name = "cap_pics_csr_cfg_sram_t", cap_csr_base *parent = 0);
        virtual ~cap_pics_csr_cfg_sram_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        ecc_disable_cor_cpp_int_t int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        ecc_disable_det_cpp_int_t int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
}; // cap_pics_csr_cfg_sram_t
    
class cap_pics_csr_base_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pics_csr_base_t(string name = "cap_pics_csr_base_t", cap_csr_base *parent = 0);
        virtual ~cap_pics_csr_base_t();
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
    
}; // cap_pics_csr_base_t
    
class cap_pics_csr_t : public cap_block_base {
    public:
        cpp_int_helper hlp;
    
        cap_pics_csr_t(string name = "cap_pics_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_pics_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pics_csr_base_t base;
    
        cap_pics_csr_cfg_sram_t cfg_sram;
    
        cap_pics_csr_cfg_table_profile_t cfg_table_profile[128];
        int get_depth_cfg_table_profile() { return 128; }
    
        cap_pics_csr_cfg_update_profile_t cfg_update_profile[16];
        int get_depth_cfg_update_profile() { return 16; }
    
        cap_pics_csr_cfg_meter_timer_t cfg_meter_timer;
    
        cap_pics_csr_cfg_meter_access_t cfg_meter_access;
    
        cap_pics_csr_cfg_meter_sram_t cfg_meter_sram[40];
        int get_depth_cfg_meter_sram() { return 40; }
    
        cap_pics_csr_dhs_sram_t dhs_sram;
    
        cap_pics_csr_dhs_table_address_t dhs_table_address;
    
        cap_pics_csr_dhs_table_data_t dhs_table_data;
    
}; // cap_pics_csr_t
    
#endif // CAP_PICS_CSR_H
        