
#ifndef CAP_PBCHBMETH_CSR_H
#define CAP_PBCHBMETH_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_pbchbmeth_csr_cfg_hbm_read_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmeth_csr_cfg_hbm_read_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmeth_csr_cfg_hbm_read_fifo_t(string name = "cap_pbchbmeth_csr_cfg_hbm_read_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmeth_csr_cfg_hbm_read_fifo_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 8 > max_packets_cpp_int_t;
        cpp_int int_var__max_packets;
        void max_packets (const cpp_int  & l__val);
        cpp_int max_packets() const;
    
        typedef pu_cpp_int< 9 > max_flits_cpp_int_t;
        cpp_int int_var__max_flits;
        void max_flits (const cpp_int  & l__val);
        cpp_int max_flits() const;
    
        typedef pu_cpp_int< 12 > max_cells_cpp_int_t;
        cpp_int int_var__max_cells;
        void max_cells (const cpp_int  & l__val);
        cpp_int max_cells() const;
    
}; // cap_pbchbmeth_csr_cfg_hbm_read_fifo_t
    
class cap_pbchbmeth_csr_cfg_hbm_parser_cam_cos_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmeth_csr_cfg_hbm_parser_cam_cos_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmeth_csr_cfg_hbm_parser_cam_cos_t(string name = "cap_pbchbmeth_csr_cfg_hbm_parser_cam_cos_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmeth_csr_cfg_hbm_parser_cam_cos_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 3 > entry_0_cpp_int_t;
        cpp_int int_var__entry_0;
        void entry_0 (const cpp_int  & l__val);
        cpp_int entry_0() const;
    
        typedef pu_cpp_int< 3 > entry_1_cpp_int_t;
        cpp_int int_var__entry_1;
        void entry_1 (const cpp_int  & l__val);
        cpp_int entry_1() const;
    
        typedef pu_cpp_int< 3 > entry_2_cpp_int_t;
        cpp_int int_var__entry_2;
        void entry_2 (const cpp_int  & l__val);
        cpp_int entry_2() const;
    
        typedef pu_cpp_int< 3 > entry_3_cpp_int_t;
        cpp_int int_var__entry_3;
        void entry_3 (const cpp_int  & l__val);
        cpp_int entry_3() const;
    
}; // cap_pbchbmeth_csr_cfg_hbm_parser_cam_cos_t
    
class cap_pbchbmeth_csr_cfg_hbm_parser_cam_enable_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmeth_csr_cfg_hbm_parser_cam_enable_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmeth_csr_cfg_hbm_parser_cam_enable_t(string name = "cap_pbchbmeth_csr_cfg_hbm_parser_cam_enable_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmeth_csr_cfg_hbm_parser_cam_enable_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 2 > entry_0_cpp_int_t;
        cpp_int int_var__entry_0;
        void entry_0 (const cpp_int  & l__val);
        cpp_int entry_0() const;
    
        typedef pu_cpp_int< 2 > entry_1_cpp_int_t;
        cpp_int int_var__entry_1;
        void entry_1 (const cpp_int  & l__val);
        cpp_int entry_1() const;
    
        typedef pu_cpp_int< 2 > entry_2_cpp_int_t;
        cpp_int int_var__entry_2;
        void entry_2 (const cpp_int  & l__val);
        cpp_int entry_2() const;
    
        typedef pu_cpp_int< 2 > entry_3_cpp_int_t;
        cpp_int int_var__entry_3;
        void entry_3 (const cpp_int  & l__val);
        cpp_int entry_3() const;
    
}; // cap_pbchbmeth_csr_cfg_hbm_parser_cam_enable_t
    
class cap_pbchbmeth_csr_cfg_hbm_parser_cam_type_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmeth_csr_cfg_hbm_parser_cam_type_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmeth_csr_cfg_hbm_parser_cam_type_t(string name = "cap_pbchbmeth_csr_cfg_hbm_parser_cam_type_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmeth_csr_cfg_hbm_parser_cam_type_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > entry_0_cpp_int_t;
        cpp_int int_var__entry_0;
        void entry_0 (const cpp_int  & l__val);
        cpp_int entry_0() const;
    
        typedef pu_cpp_int< 16 > entry_1_cpp_int_t;
        cpp_int int_var__entry_1;
        void entry_1 (const cpp_int  & l__val);
        cpp_int entry_1() const;
    
        typedef pu_cpp_int< 16 > entry_2_cpp_int_t;
        cpp_int int_var__entry_2;
        void entry_2 (const cpp_int  & l__val);
        cpp_int entry_2() const;
    
        typedef pu_cpp_int< 16 > entry_3_cpp_int_t;
        cpp_int int_var__entry_3;
        void entry_3 (const cpp_int  & l__val);
        cpp_int entry_3() const;
    
}; // cap_pbchbmeth_csr_cfg_hbm_parser_cam_type_t
    
class cap_pbchbmeth_csr_cfg_hbm_parser_cam_da_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmeth_csr_cfg_hbm_parser_cam_da_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmeth_csr_cfg_hbm_parser_cam_da_t(string name = "cap_pbchbmeth_csr_cfg_hbm_parser_cam_da_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmeth_csr_cfg_hbm_parser_cam_da_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 48 > entry_0_cpp_int_t;
        cpp_int int_var__entry_0;
        void entry_0 (const cpp_int  & l__val);
        cpp_int entry_0() const;
    
        typedef pu_cpp_int< 48 > entry_1_cpp_int_t;
        cpp_int int_var__entry_1;
        void entry_1 (const cpp_int  & l__val);
        cpp_int entry_1() const;
    
        typedef pu_cpp_int< 48 > entry_2_cpp_int_t;
        cpp_int int_var__entry_2;
        void entry_2 (const cpp_int  & l__val);
        cpp_int entry_2() const;
    
        typedef pu_cpp_int< 48 > entry_3_cpp_int_t;
        cpp_int int_var__entry_3;
        void entry_3 (const cpp_int  & l__val);
        cpp_int entry_3() const;
    
}; // cap_pbchbmeth_csr_cfg_hbm_parser_cam_da_t
    
class cap_pbchbmeth_csr_cfg_hbm_eth_ctrl_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmeth_csr_cfg_hbm_eth_ctrl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmeth_csr_cfg_hbm_eth_ctrl_t(string name = "cap_pbchbmeth_csr_cfg_hbm_eth_ctrl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmeth_csr_cfg_hbm_eth_ctrl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 216 > base_cpp_int_t;
        cpp_int int_var__base;
        void base (const cpp_int  & l__val);
        cpp_int base() const;
    
        typedef pu_cpp_int< 184 > mem_sz_cpp_int_t;
        cpp_int int_var__mem_sz;
        void mem_sz (const cpp_int  & l__val);
        cpp_int mem_sz() const;
    
}; // cap_pbchbmeth_csr_cfg_hbm_eth_ctrl_t
    
class cap_pbchbmeth_csr_cfg_hbm_eth_payload_occupancy_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmeth_csr_cfg_hbm_eth_payload_occupancy_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmeth_csr_cfg_hbm_eth_payload_occupancy_t(string name = "cap_pbchbmeth_csr_cfg_hbm_eth_payload_occupancy_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmeth_csr_cfg_hbm_eth_payload_occupancy_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 152 > threshold_cpp_int_t;
        cpp_int int_var__threshold;
        void threshold (const cpp_int  & l__val);
        cpp_int threshold() const;
    
}; // cap_pbchbmeth_csr_cfg_hbm_eth_payload_occupancy_t
    
class cap_pbchbmeth_csr_cfg_hbm_eth_payload_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmeth_csr_cfg_hbm_eth_payload_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmeth_csr_cfg_hbm_eth_payload_t(string name = "cap_pbchbmeth_csr_cfg_hbm_eth_payload_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmeth_csr_cfg_hbm_eth_payload_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 216 > base_cpp_int_t;
        cpp_int int_var__base;
        void base (const cpp_int  & l__val);
        cpp_int base() const;
    
        typedef pu_cpp_int< 184 > mem_sz_cpp_int_t;
        cpp_int int_var__mem_sz;
        void mem_sz (const cpp_int  & l__val);
        cpp_int mem_sz() const;
    
}; // cap_pbchbmeth_csr_cfg_hbm_eth_payload_t
    
class cap_pbchbmeth_csr_cnt_hbm_cut_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmeth_csr_cnt_hbm_cut_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmeth_csr_cnt_hbm_cut_t(string name = "cap_pbchbmeth_csr_cnt_hbm_cut_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmeth_csr_cnt_hbm_cut_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > thru_cpp_int_t;
        cpp_int int_var__thru;
        void thru (const cpp_int  & l__val);
        cpp_int thru() const;
    
}; // cap_pbchbmeth_csr_cnt_hbm_cut_t
    
class cap_pbchbmeth_csr_cnt_hbm_truncate_no_drop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmeth_csr_cnt_hbm_truncate_no_drop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmeth_csr_cnt_hbm_truncate_no_drop_t(string name = "cap_pbchbmeth_csr_cnt_hbm_truncate_no_drop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmeth_csr_cnt_hbm_truncate_no_drop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > count_cpp_int_t;
        cpp_int int_var__count;
        void count (const cpp_int  & l__val);
        cpp_int count() const;
    
}; // cap_pbchbmeth_csr_cnt_hbm_truncate_no_drop_t
    
class cap_pbchbmeth_csr_sat_hbm_ctrl_full_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmeth_csr_sat_hbm_ctrl_full_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmeth_csr_sat_hbm_ctrl_full_t(string name = "cap_pbchbmeth_csr_sat_hbm_ctrl_full_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmeth_csr_sat_hbm_ctrl_full_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbmeth_csr_sat_hbm_ctrl_full_t
    
class cap_pbchbmeth_csr_cnt_hbm_truncate_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmeth_csr_cnt_hbm_truncate_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmeth_csr_cnt_hbm_truncate_t(string name = "cap_pbchbmeth_csr_cnt_hbm_truncate_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmeth_csr_cnt_hbm_truncate_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbmeth_csr_cnt_hbm_truncate_t
    
class cap_pbchbmeth_csr_cnt_hbm_write_ack_full_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmeth_csr_cnt_hbm_write_ack_full_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmeth_csr_cnt_hbm_write_ack_full_t(string name = "cap_pbchbmeth_csr_cnt_hbm_write_ack_full_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmeth_csr_cnt_hbm_write_ack_full_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbmeth_csr_cnt_hbm_write_ack_full_t
    
class cap_pbchbmeth_csr_cnt_hbm_write_ack_filling_up_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmeth_csr_cnt_hbm_write_ack_filling_up_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmeth_csr_cnt_hbm_write_ack_filling_up_t(string name = "cap_pbchbmeth_csr_cnt_hbm_write_ack_filling_up_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmeth_csr_cnt_hbm_write_ack_filling_up_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbmeth_csr_cnt_hbm_write_ack_filling_up_t
    
class cap_pbchbmeth_csr_cnt_hbm_emergency_stop_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmeth_csr_cnt_hbm_emergency_stop_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmeth_csr_cnt_hbm_emergency_stop_t(string name = "cap_pbchbmeth_csr_cnt_hbm_emergency_stop_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmeth_csr_cnt_hbm_emergency_stop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
}; // cap_pbchbmeth_csr_cnt_hbm_emergency_stop_t
    
class cap_pbchbmeth_csr_cfg_hbm_addr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmeth_csr_cfg_hbm_addr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmeth_csr_cfg_hbm_addr_t(string name = "cap_pbchbmeth_csr_cfg_hbm_addr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmeth_csr_cfg_hbm_addr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > swizzle_cpp_int_t;
        cpp_int int_var__swizzle;
        void swizzle (const cpp_int  & l__val);
        cpp_int swizzle() const;
    
}; // cap_pbchbmeth_csr_cfg_hbm_addr_t
    
class cap_pbchbmeth_csr_cnt_hbm_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmeth_csr_cnt_hbm_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmeth_csr_cnt_hbm_t(string name = "cap_pbchbmeth_csr_cnt_hbm_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmeth_csr_cnt_hbm_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 32 > flits_sop_in_cpp_int_t;
        cpp_int int_var__flits_sop_in;
        void flits_sop_in (const cpp_int  & l__val);
        cpp_int flits_sop_in() const;
    
        typedef pu_cpp_int< 32 > flits_eop_in_cpp_int_t;
        cpp_int int_var__flits_eop_in;
        void flits_eop_in (const cpp_int  & l__val);
        cpp_int flits_eop_in() const;
    
        typedef pu_cpp_int< 32 > flits_sop_out_cpp_int_t;
        cpp_int int_var__flits_sop_out;
        void flits_sop_out (const cpp_int  & l__val);
        cpp_int flits_sop_out() const;
    
        typedef pu_cpp_int< 32 > flits_eop_out_cpp_int_t;
        cpp_int int_var__flits_eop_out;
        void flits_eop_out (const cpp_int  & l__val);
        cpp_int flits_eop_out() const;
    
        typedef pu_cpp_int< 32 > occupancy_drop_cpp_int_t;
        cpp_int int_var__occupancy_drop;
        void occupancy_drop (const cpp_int  & l__val);
        cpp_int occupancy_drop() const;
    
}; // cap_pbchbmeth_csr_cnt_hbm_t
    
class cap_pbchbmeth_csr_cfg_hbm_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmeth_csr_cfg_hbm_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmeth_csr_cfg_hbm_t(string name = "cap_pbchbmeth_csr_cfg_hbm_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmeth_csr_cfg_hbm_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 4 > rate_limiter_cpp_int_t;
        cpp_int int_var__rate_limiter;
        void rate_limiter (const cpp_int  & l__val);
        cpp_int rate_limiter() const;
    
        typedef pu_cpp_int< 1 > rewrite_cpp_int_t;
        cpp_int int_var__rewrite;
        void rewrite (const cpp_int  & l__val);
        cpp_int rewrite() const;
    
        typedef pu_cpp_int< 7 > depth_shift_cpp_int_t;
        cpp_int int_var__depth_shift;
        void depth_shift (const cpp_int  & l__val);
        cpp_int depth_shift() const;
    
        typedef pu_cpp_int< 1 > max_pop_size_enable_cpp_int_t;
        cpp_int int_var__max_pop_size_enable;
        void max_pop_size_enable (const cpp_int  & l__val);
        cpp_int max_pop_size_enable() const;
    
        typedef pu_cpp_int< 1 > enable_multiple_reads_cpp_int_t;
        cpp_int int_var__enable_multiple_reads;
        void enable_multiple_reads (const cpp_int  & l__val);
        cpp_int enable_multiple_reads() const;
    
        typedef pu_cpp_int< 1 > enable_drdy_cpp_int_t;
        cpp_int int_var__enable_drdy;
        void enable_drdy (const cpp_int  & l__val);
        cpp_int enable_drdy() const;
    
}; // cap_pbchbmeth_csr_cfg_hbm_t
    
class cap_pbchbmeth_csr_cfg_hbm_parser_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmeth_csr_cfg_hbm_parser_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmeth_csr_cfg_hbm_parser_t(string name = "cap_pbchbmeth_csr_cfg_hbm_parser_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmeth_csr_cfg_hbm_parser_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 16 > dot1q_type_cpp_int_t;
        cpp_int int_var__dot1q_type;
        void dot1q_type (const cpp_int  & l__val);
        cpp_int dot1q_type() const;
    
        typedef pu_cpp_int< 16 > ipv4_type_cpp_int_t;
        cpp_int int_var__ipv4_type;
        void ipv4_type (const cpp_int  & l__val);
        cpp_int ipv4_type() const;
    
        typedef pu_cpp_int< 16 > ipv6_type_cpp_int_t;
        cpp_int int_var__ipv6_type;
        void ipv6_type (const cpp_int  & l__val);
        cpp_int ipv6_type() const;
    
        typedef pu_cpp_int< 1 > use_ip_cpp_int_t;
        cpp_int int_var__use_ip;
        void use_ip (const cpp_int  & l__val);
        cpp_int use_ip() const;
    
        typedef pu_cpp_int< 1 > use_dot1q_cpp_int_t;
        cpp_int int_var__use_dot1q;
        void use_dot1q (const cpp_int  & l__val);
        cpp_int use_dot1q() const;
    
        typedef pu_cpp_int< 3 > default_cos_cpp_int_t;
        cpp_int int_var__default_cos;
        void default_cos (const cpp_int  & l__val);
        cpp_int default_cos() const;
    
        typedef pu_cpp_int< 192 > dscp_map_cpp_int_t;
        cpp_int int_var__dscp_map;
        void dscp_map (const cpp_int  & l__val);
        cpp_int dscp_map() const;
    
}; // cap_pbchbmeth_csr_cfg_hbm_parser_t
    
class cap_pbchbmeth_csr_cfg_hbm_context_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmeth_csr_cfg_hbm_context_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmeth_csr_cfg_hbm_context_t(string name = "cap_pbchbmeth_csr_cfg_hbm_context_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmeth_csr_cfg_hbm_context_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 5 > rsvd_cpp_int_t;
        cpp_int int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
        typedef pu_cpp_int< 8 > enable_cpp_int_t;
        cpp_int int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 8 > no_drop_cpp_int_t;
        cpp_int int_var__no_drop;
        void no_drop (const cpp_int  & l__val);
        cpp_int no_drop() const;
    
        typedef pu_cpp_int< 5 > base_cpp_int_t;
        cpp_int int_var__base;
        void base (const cpp_int  & l__val);
        cpp_int base() const;
    
}; // cap_pbchbmeth_csr_cfg_hbm_context_t
    
class cap_pbchbmeth_csr_cfg_hbm_mem_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmeth_csr_cfg_hbm_mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmeth_csr_cfg_hbm_mem_t(string name = "cap_pbchbmeth_csr_cfg_hbm_mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmeth_csr_cfg_hbm_mem_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        cpp_int int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 1 > dhs_selection_cpp_int_t;
        cpp_int int_var__dhs_selection;
        void dhs_selection (const cpp_int  & l__val);
        cpp_int dhs_selection() const;
    
        typedef pu_cpp_int< 2 > ecc_disable_cor_cpp_int_t;
        cpp_int int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 2 > ecc_disable_det_cpp_int_t;
        cpp_int int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 2 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_pbchbmeth_csr_cfg_hbm_mem_t
    
class cap_pbchbmeth_csr_cfg_hbm_tc_to_q_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmeth_csr_cfg_hbm_tc_to_q_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmeth_csr_cfg_hbm_tc_to_q_t(string name = "cap_pbchbmeth_csr_cfg_hbm_tc_to_q_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmeth_csr_cfg_hbm_tc_to_q_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_pbchbmeth_csr_cfg_hbm_tc_to_q_t
    
class cap_pbchbmeth_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pbchbmeth_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pbchbmeth_csr_t(string name = "cap_pbchbmeth_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pbchbmeth_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_pbchbmeth_csr_cfg_hbm_tc_to_q_t cfg_hbm_tc_to_q;
    
        cap_pbchbmeth_csr_cfg_hbm_mem_t cfg_hbm_mem;
    
        cap_pbchbmeth_csr_cfg_hbm_context_t cfg_hbm_context;
    
        cap_pbchbmeth_csr_cfg_hbm_parser_t cfg_hbm_parser;
    
        cap_pbchbmeth_csr_cfg_hbm_t cfg_hbm;
    
        cap_pbchbmeth_csr_cnt_hbm_t cnt_hbm;
    
        cap_pbchbmeth_csr_cfg_hbm_addr_t cfg_hbm_addr;
    
        cap_pbchbmeth_csr_cnt_hbm_emergency_stop_t cnt_hbm_emergency_stop;
    
        cap_pbchbmeth_csr_cnt_hbm_write_ack_filling_up_t cnt_hbm_write_ack_filling_up;
    
        cap_pbchbmeth_csr_cnt_hbm_write_ack_full_t cnt_hbm_write_ack_full;
    
        cap_pbchbmeth_csr_cnt_hbm_truncate_t cnt_hbm_truncate;
    
        cap_pbchbmeth_csr_sat_hbm_ctrl_full_t sat_hbm_ctrl_full;
    
        cap_pbchbmeth_csr_cnt_hbm_truncate_no_drop_t cnt_hbm_truncate_no_drop;
    
        cap_pbchbmeth_csr_cnt_hbm_cut_t cnt_hbm_cut;
    
        cap_pbchbmeth_csr_cfg_hbm_eth_payload_t cfg_hbm_eth_payload;
    
        cap_pbchbmeth_csr_cfg_hbm_eth_payload_occupancy_t cfg_hbm_eth_payload_occupancy;
    
        cap_pbchbmeth_csr_cfg_hbm_eth_ctrl_t cfg_hbm_eth_ctrl;
    
        cap_pbchbmeth_csr_cfg_hbm_parser_cam_da_t cfg_hbm_parser_cam_da;
    
        cap_pbchbmeth_csr_cfg_hbm_parser_cam_type_t cfg_hbm_parser_cam_type;
    
        cap_pbchbmeth_csr_cfg_hbm_parser_cam_enable_t cfg_hbm_parser_cam_enable;
    
        cap_pbchbmeth_csr_cfg_hbm_parser_cam_cos_t cfg_hbm_parser_cam_cos;
    
        cap_pbchbmeth_csr_cfg_hbm_read_fifo_t cfg_hbm_read_fifo;
    
}; // cap_pbchbmeth_csr_t
    
#endif // CAP_PBCHBMETH_CSR_H
        