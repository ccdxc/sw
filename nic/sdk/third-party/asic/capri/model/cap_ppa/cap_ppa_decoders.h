
#ifndef CAP_PPA_DECODERS_H
#define CAP_PPA_DECODERS_H

#include "cap_csr_base.h" 

using namespace std;
class cap_ppa_decoders_mux_idx_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_mux_idx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_mux_idx_t(string name = "cap_ppa_decoders_mux_idx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_mux_idx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 2 > sel_cpp_int_t;
        cpp_int int_var__sel;
        void sel (const cpp_int  & l__val);
        cpp_int sel() const;
    
        typedef pu_cpp_int< 2 > lkpsel_cpp_int_t;
        cpp_int int_var__lkpsel;
        void lkpsel (const cpp_int  & l__val);
        cpp_int lkpsel() const;
    
        typedef pu_cpp_int< 6 > idx_cpp_int_t;
        cpp_int int_var__idx;
        void idx (const cpp_int  & l__val);
        cpp_int idx() const;
    
        typedef pu_cpp_int< 1 > load_stored_lkp_cpp_int_t;
        cpp_int int_var__load_stored_lkp;
        void load_stored_lkp (const cpp_int  & l__val);
        cpp_int load_stored_lkp() const;
    
}; // cap_ppa_decoders_mux_idx_t
    
class cap_ppa_decoders_mux_inst_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_mux_inst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_mux_inst_t(string name = "cap_ppa_decoders_mux_inst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_mux_inst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > sel_cpp_int_t;
        cpp_int int_var__sel;
        void sel (const cpp_int  & l__val);
        cpp_int sel() const;
    
        typedef pu_cpp_int< 2 > muxsel_cpp_int_t;
        cpp_int int_var__muxsel;
        void muxsel (const cpp_int  & l__val);
        cpp_int muxsel() const;
    
        typedef pu_cpp_int< 16 > mask_val_cpp_int_t;
        cpp_int int_var__mask_val;
        void mask_val (const cpp_int  & l__val);
        cpp_int mask_val() const;
    
        typedef pu_cpp_int< 4 > shift_val_cpp_int_t;
        cpp_int int_var__shift_val;
        void shift_val (const cpp_int  & l__val);
        cpp_int shift_val() const;
    
        typedef pu_cpp_int< 8 > addsub_val_cpp_int_t;
        cpp_int int_var__addsub_val;
        void addsub_val (const cpp_int  & l__val);
        cpp_int addsub_val() const;
    
        typedef pu_cpp_int< 1 > shift_left_cpp_int_t;
        cpp_int int_var__shift_left;
        void shift_left (const cpp_int  & l__val);
        cpp_int shift_left() const;
    
        typedef pu_cpp_int< 1 > addsub_cpp_int_t;
        cpp_int int_var__addsub;
        void addsub (const cpp_int  & l__val);
        cpp_int addsub() const;
    
        typedef pu_cpp_int< 2 > lkpsel_cpp_int_t;
        cpp_int int_var__lkpsel;
        void lkpsel (const cpp_int  & l__val);
        cpp_int lkpsel() const;
    
        typedef pu_cpp_int< 1 > lkp_addsub_cpp_int_t;
        cpp_int int_var__lkp_addsub;
        void lkp_addsub (const cpp_int  & l__val);
        cpp_int lkp_addsub() const;
    
        typedef pu_cpp_int< 1 > load_mux_pkt_cpp_int_t;
        cpp_int int_var__load_mux_pkt;
        void load_mux_pkt (const cpp_int  & l__val);
        cpp_int load_mux_pkt() const;
    
}; // cap_ppa_decoders_mux_inst_t
    
class cap_ppa_decoders_offset_inst_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_offset_inst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_offset_inst_t(string name = "cap_ppa_decoders_offset_inst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_offset_inst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > sel_cpp_int_t;
        cpp_int int_var__sel;
        void sel (const cpp_int  & l__val);
        cpp_int sel() const;
    
        typedef pu_cpp_int< 2 > muxsel_cpp_int_t;
        cpp_int int_var__muxsel;
        void muxsel (const cpp_int  & l__val);
        cpp_int muxsel() const;
    
        typedef pu_cpp_int< 14 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
}; // cap_ppa_decoders_offset_inst_t
    
class cap_ppa_decoders_lkp_val_inst_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_lkp_val_inst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_lkp_val_inst_t(string name = "cap_ppa_decoders_lkp_val_inst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_lkp_val_inst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 2 > sel_cpp_int_t;
        cpp_int int_var__sel;
        void sel (const cpp_int  & l__val);
        cpp_int sel() const;
    
        typedef pu_cpp_int< 2 > muxsel_cpp_int_t;
        cpp_int int_var__muxsel;
        void muxsel (const cpp_int  & l__val);
        cpp_int muxsel() const;
    
        typedef pu_cpp_int< 1 > store_en_cpp_int_t;
        cpp_int int_var__store_en;
        void store_en (const cpp_int  & l__val);
        cpp_int store_en() const;
    
}; // cap_ppa_decoders_lkp_val_inst_t
    
class cap_ppa_decoders_extract_inst_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_extract_inst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_extract_inst_t(string name = "cap_ppa_decoders_extract_inst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_extract_inst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 6 > pkt_idx_cpp_int_t;
        cpp_int int_var__pkt_idx;
        void pkt_idx (const cpp_int  & l__val);
        cpp_int pkt_idx() const;
    
        typedef pu_cpp_int< 3 > len_cpp_int_t;
        cpp_int int_var__len;
        void len (const cpp_int  & l__val);
        cpp_int len() const;
    
        typedef pu_cpp_int< 7 > phv_idx_cpp_int_t;
        cpp_int int_var__phv_idx;
        void phv_idx (const cpp_int  & l__val);
        cpp_int phv_idx() const;
    
}; // cap_ppa_decoders_extract_inst_t
    
class cap_ppa_decoders_meta_inst_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_meta_inst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_meta_inst_t(string name = "cap_ppa_decoders_meta_inst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_meta_inst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 7 > phv_idx_cpp_int_t;
        cpp_int int_var__phv_idx;
        void phv_idx (const cpp_int  & l__val);
        cpp_int phv_idx() const;
    
        typedef pu_cpp_int< 8 > val_cpp_int_t;
        cpp_int int_var__val;
        void val (const cpp_int  & l__val);
        cpp_int val() const;
    
        typedef pu_cpp_int< 3 > sel_cpp_int_t;
        cpp_int int_var__sel;
        void sel (const cpp_int  & l__val);
        cpp_int sel() const;
    
}; // cap_ppa_decoders_meta_inst_t
    
class cap_ppa_decoders_ohi_inst_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_ohi_inst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_ohi_inst_t(string name = "cap_ppa_decoders_ohi_inst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_ohi_inst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 3 > sel_cpp_int_t;
        cpp_int int_var__sel;
        void sel (const cpp_int  & l__val);
        cpp_int sel() const;
    
        typedef pu_cpp_int< 2 > muxsel_cpp_int_t;
        cpp_int int_var__muxsel;
        void muxsel (const cpp_int  & l__val);
        cpp_int muxsel() const;
    
        typedef pu_cpp_int< 16 > idx_val_cpp_int_t;
        cpp_int int_var__idx_val;
        void idx_val (const cpp_int  & l__val);
        cpp_int idx_val() const;
    
        typedef pu_cpp_int< 6 > slot_num_cpp_int_t;
        cpp_int int_var__slot_num;
        void slot_num (const cpp_int  & l__val);
        cpp_int slot_num() const;
    
}; // cap_ppa_decoders_ohi_inst_t
    
class cap_ppa_decoders_crc_inst_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_crc_inst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_crc_inst_t(string name = "cap_ppa_decoders_crc_inst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_crc_inst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > en_cpp_int_t;
        cpp_int int_var__en;
        void en (const cpp_int  & l__val);
        cpp_int en() const;
    
        typedef pu_cpp_int< 1 > prof_sel_en_cpp_int_t;
        cpp_int int_var__prof_sel_en;
        void prof_sel_en (const cpp_int  & l__val);
        cpp_int prof_sel_en() const;
    
        typedef pu_cpp_int< 3 > prof_sel_cpp_int_t;
        cpp_int int_var__prof_sel;
        void prof_sel (const cpp_int  & l__val);
        cpp_int prof_sel() const;
    
        typedef pu_cpp_int< 6 > ohi_start_sel_cpp_int_t;
        cpp_int int_var__ohi_start_sel;
        void ohi_start_sel (const cpp_int  & l__val);
        cpp_int ohi_start_sel() const;
    
        typedef pu_cpp_int< 6 > ohi_len_sel_cpp_int_t;
        cpp_int int_var__ohi_len_sel;
        void ohi_len_sel (const cpp_int  & l__val);
        cpp_int ohi_len_sel() const;
    
        typedef pu_cpp_int< 6 > ohi_mask_sel_cpp_int_t;
        cpp_int int_var__ohi_mask_sel;
        void ohi_mask_sel (const cpp_int  & l__val);
        cpp_int ohi_mask_sel() const;
    
}; // cap_ppa_decoders_crc_inst_t
    
class cap_ppa_decoders_csum_inst_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_csum_inst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_csum_inst_t(string name = "cap_ppa_decoders_csum_inst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_csum_inst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > en_cpp_int_t;
        cpp_int int_var__en;
        void en (const cpp_int  & l__val);
        cpp_int en() const;
    
        typedef pu_cpp_int< 3 > unit_sel_cpp_int_t;
        cpp_int int_var__unit_sel;
        void unit_sel (const cpp_int  & l__val);
        cpp_int unit_sel() const;
    
        typedef pu_cpp_int< 3 > prof_sel_cpp_int_t;
        cpp_int int_var__prof_sel;
        void prof_sel (const cpp_int  & l__val);
        cpp_int prof_sel() const;
    
        typedef pu_cpp_int< 6 > ohi_start_sel_cpp_int_t;
        cpp_int int_var__ohi_start_sel;
        void ohi_start_sel (const cpp_int  & l__val);
        cpp_int ohi_start_sel() const;
    
        typedef pu_cpp_int< 6 > ohi_len_sel_cpp_int_t;
        cpp_int int_var__ohi_len_sel;
        void ohi_len_sel (const cpp_int  & l__val);
        cpp_int ohi_len_sel() const;
    
        typedef pu_cpp_int< 1 > phdr_en_cpp_int_t;
        cpp_int int_var__phdr_en;
        void phdr_en (const cpp_int  & l__val);
        cpp_int phdr_en() const;
    
        typedef pu_cpp_int< 3 > phdr_sel_cpp_int_t;
        cpp_int int_var__phdr_sel;
        void phdr_sel (const cpp_int  & l__val);
        cpp_int phdr_sel() const;
    
        typedef pu_cpp_int< 6 > phdr_ohi_sel_cpp_int_t;
        cpp_int int_var__phdr_ohi_sel;
        void phdr_ohi_sel (const cpp_int  & l__val);
        cpp_int phdr_ohi_sel() const;
    
        typedef pu_cpp_int< 1 > dis_zero_cpp_int_t;
        cpp_int int_var__dis_zero;
        void dis_zero (const cpp_int  & l__val);
        cpp_int dis_zero() const;
    
        typedef pu_cpp_int< 1 > load_phdr_prof_en_cpp_int_t;
        cpp_int int_var__load_phdr_prof_en;
        void load_phdr_prof_en (const cpp_int  & l__val);
        cpp_int load_phdr_prof_en() const;
    
}; // cap_ppa_decoders_csum_inst_t
    
class cap_ppa_decoders_len_chk_inst_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_len_chk_inst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_len_chk_inst_t(string name = "cap_ppa_decoders_len_chk_inst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_len_chk_inst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > en_cpp_int_t;
        cpp_int int_var__en;
        void en (const cpp_int  & l__val);
        cpp_int en() const;
    
        typedef pu_cpp_int< 2 > unit_sel_cpp_int_t;
        cpp_int int_var__unit_sel;
        void unit_sel (const cpp_int  & l__val);
        cpp_int unit_sel() const;
    
        typedef pu_cpp_int< 2 > prof_sel_cpp_int_t;
        cpp_int int_var__prof_sel;
        void prof_sel (const cpp_int  & l__val);
        cpp_int prof_sel() const;
    
        typedef pu_cpp_int< 6 > ohi_start_sel_cpp_int_t;
        cpp_int int_var__ohi_start_sel;
        void ohi_start_sel (const cpp_int  & l__val);
        cpp_int ohi_start_sel() const;
    
        typedef pu_cpp_int< 6 > ohi_len_sel_cpp_int_t;
        cpp_int int_var__ohi_len_sel;
        void ohi_len_sel (const cpp_int  & l__val);
        cpp_int ohi_len_sel() const;
    
        typedef pu_cpp_int< 1 > exact_cpp_int_t;
        cpp_int int_var__exact;
        void exact (const cpp_int  & l__val);
        cpp_int exact() const;
    
}; // cap_ppa_decoders_len_chk_inst_t
    
class cap_ppa_decoders_align_chk_inst_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_align_chk_inst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_align_chk_inst_t(string name = "cap_ppa_decoders_align_chk_inst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_align_chk_inst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > en_cpp_int_t;
        cpp_int int_var__en;
        void en (const cpp_int  & l__val);
        cpp_int en() const;
    
        typedef pu_cpp_int< 1 > prof_sel_cpp_int_t;
        cpp_int int_var__prof_sel;
        void prof_sel (const cpp_int  & l__val);
        cpp_int prof_sel() const;
    
        typedef pu_cpp_int< 6 > ohi_start_sel_cpp_int_t;
        cpp_int int_var__ohi_start_sel;
        void ohi_start_sel (const cpp_int  & l__val);
        cpp_int ohi_start_sel() const;
    
}; // cap_ppa_decoders_align_chk_inst_t
    
class cap_ppa_decoders_ppa_lkp_sram_entry_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_ppa_lkp_sram_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_ppa_lkp_sram_entry_t(string name = "cap_ppa_decoders_ppa_lkp_sram_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_ppa_lkp_sram_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > action_cpp_int_t;
        cpp_int int_var__action;
        void action (const cpp_int  & l__val);
        cpp_int action() const;
    
        typedef pu_cpp_int< 9 > nxt_state_cpp_int_t;
        cpp_int int_var__nxt_state;
        void nxt_state (const cpp_int  & l__val);
        cpp_int nxt_state() const;
    
        cap_ppa_decoders_offset_inst_t offset_inst;
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_decoders_mux_idx_t, 4> mux_idx;
        #else 
        cap_ppa_decoders_mux_idx_t mux_idx[4];
        #endif
        int get_depth_mux_idx() { return 4; }
    
        #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_decoders_mux_inst_t, 3> mux_inst;
        #else 
        cap_ppa_decoders_mux_inst_t mux_inst[3];
        #endif
        int get_depth_mux_inst() { return 3; }
    
        #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_decoders_lkp_val_inst_t, 3> lkp_val_inst;
        #else 
        cap_ppa_decoders_lkp_val_inst_t lkp_val_inst[3];
        #endif
        int get_depth_lkp_val_inst() { return 3; }
    
        typedef pu_cpp_int< 3 > phv_idx_upr_cpp_int_t;
        cpp_int int_var__phv_idx_upr;
        void phv_idx_upr (const cpp_int  & l__val);
        cpp_int phv_idx_upr() const;
    
        #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_decoders_extract_inst_t, 16> extract_inst;
        #else 
        cap_ppa_decoders_extract_inst_t extract_inst[16];
        #endif
        int get_depth_extract_inst() { return 16; }
    
        #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_decoders_meta_inst_t, 3> meta_inst;
        #else 
        cap_ppa_decoders_meta_inst_t meta_inst[3];
        #endif
        int get_depth_meta_inst() { return 3; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_decoders_ohi_inst_t, 4> ohi_inst;
        #else 
        cap_ppa_decoders_ohi_inst_t ohi_inst[4];
        #endif
        int get_depth_ohi_inst() { return 4; }
    
        cap_ppa_decoders_crc_inst_t crc_inst;
    
        #if 2 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_decoders_csum_inst_t, 2> csum_inst;
        #else 
        cap_ppa_decoders_csum_inst_t csum_inst[2];
        #endif
        int get_depth_csum_inst() { return 2; }
    
        #if 2 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_decoders_len_chk_inst_t, 2> len_chk_inst;
        #else 
        cap_ppa_decoders_len_chk_inst_t len_chk_inst[2];
        #endif
        int get_depth_len_chk_inst() { return 2; }
    
        #if 2 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_decoders_align_chk_inst_t, 2> align_chk_inst;
        #else 
        cap_ppa_decoders_align_chk_inst_t align_chk_inst[2];
        #endif
        int get_depth_align_chk_inst() { return 2; }
    
        typedef pu_cpp_int< 1 > offset_jump_chk_en_cpp_int_t;
        cpp_int int_var__offset_jump_chk_en;
        void offset_jump_chk_en (const cpp_int  & l__val);
        cpp_int offset_jump_chk_en() const;
    
        typedef pu_cpp_int< 1 > pkt_size_chk_en_cpp_int_t;
        cpp_int int_var__pkt_size_chk_en;
        void pkt_size_chk_en (const cpp_int  & l__val);
        cpp_int pkt_size_chk_en() const;
    
        typedef pu_cpp_int< 6 > pkt_size_cpp_int_t;
        cpp_int int_var__pkt_size;
        void pkt_size (const cpp_int  & l__val);
        cpp_int pkt_size() const;
    
}; // cap_ppa_decoders_ppa_lkp_sram_entry_t
    
class cap_ppa_decoders_chk_ctl_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_chk_ctl_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_chk_ctl_t(string name = "cap_ppa_decoders_chk_ctl_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_chk_ctl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 14 > last_offset_cpp_int_t;
        cpp_int int_var__last_offset;
        void last_offset (const cpp_int  & l__val);
        cpp_int last_offset() const;
    
        typedef pu_cpp_int< 7 > seq_id_cpp_int_t;
        cpp_int int_var__seq_id;
        void seq_id (const cpp_int  & l__val);
        cpp_int seq_id() const;
    
        typedef pu_cpp_int< 7 > err_vec_cpp_int_t;
        cpp_int int_var__err_vec;
        void err_vec (const cpp_int  & l__val);
        cpp_int err_vec() const;
    
        cap_ppa_decoders_crc_inst_t crc_inst;
    
        #if 5 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_decoders_csum_inst_t, 5> csum_inst;
        #else 
        cap_ppa_decoders_csum_inst_t csum_inst[5];
        #endif
        int get_depth_csum_inst() { return 5; }
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_decoders_len_chk_inst_t, 4> len_chk_inst;
        #else 
        cap_ppa_decoders_len_chk_inst_t len_chk_inst[4];
        #endif
        int get_depth_len_chk_inst() { return 4; }
    
        #if 2 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_decoders_align_chk_inst_t, 2> align_chk_inst;
        #else 
        cap_ppa_decoders_align_chk_inst_t align_chk_inst[2];
        #endif
        int get_depth_align_chk_inst() { return 2; }
    
}; // cap_ppa_decoders_chk_ctl_t
    
class cap_ppa_decoders_ppa_lkp_tcam_key_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_ppa_lkp_tcam_key_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_ppa_lkp_tcam_key_t(string name = "cap_ppa_decoders_ppa_lkp_tcam_key_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_ppa_lkp_tcam_key_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 7 > control_cpp_int_t;
        cpp_int int_var__control;
        void control (const cpp_int  & l__val);
        cpp_int control() const;
    
        typedef pu_cpp_int< 9 > state_cpp_int_t;
        cpp_int int_var__state;
        void state (const cpp_int  & l__val);
        cpp_int state() const;
    
        typedef pu_cpp_int < 16 > lkp_val_cpp_int_t;
        cpp_int int_var__lkp_val[3];
        int get_depth_lkp_val() { return 3; }
        void lkp_val ( const cpp_int & _val, int _idx);
        cpp_int lkp_val(int _idx) const;
    
}; // cap_ppa_decoders_ppa_lkp_tcam_key_t
    
class cap_ppa_decoders_ppa_lkp_tcam_entry_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_ppa_lkp_tcam_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_ppa_lkp_tcam_entry_t(string name = "cap_ppa_decoders_ppa_lkp_tcam_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_ppa_lkp_tcam_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_ppa_decoders_ppa_lkp_tcam_key_t key;
    
        cap_ppa_decoders_ppa_lkp_tcam_key_t mask;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        cpp_int int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
}; // cap_ppa_decoders_ppa_lkp_tcam_entry_t
    
class cap_ppa_decoders_crc_prof_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_crc_prof_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_crc_prof_t(string name = "cap_ppa_decoders_crc_prof_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_crc_prof_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_decoders_crc_prof_t
    
class cap_ppa_decoders_crc_mask_prof_inst_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_crc_mask_prof_inst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_crc_mask_prof_inst_t(string name = "cap_ppa_decoders_crc_mask_prof_inst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_crc_mask_prof_inst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > mask_en_cpp_int_t;
        cpp_int int_var__mask_en;
        void mask_en (const cpp_int  & l__val);
        cpp_int mask_en() const;
    
        typedef pu_cpp_int< 1 > use_ohi_cpp_int_t;
        cpp_int int_var__use_ohi;
        void use_ohi (const cpp_int  & l__val);
        cpp_int use_ohi() const;
    
        typedef pu_cpp_int< 6 > start_adj_cpp_int_t;
        cpp_int int_var__start_adj;
        void start_adj (const cpp_int  & l__val);
        cpp_int start_adj() const;
    
        typedef pu_cpp_int< 6 > end_adj_cpp_int_t;
        cpp_int int_var__end_adj;
        void end_adj (const cpp_int  & l__val);
        cpp_int end_adj() const;
    
        typedef pu_cpp_int< 1 > fill_cpp_int_t;
        cpp_int int_var__fill;
        void fill (const cpp_int  & l__val);
        cpp_int fill() const;
    
        typedef pu_cpp_int< 1 > skip_first_nibble_cpp_int_t;
        cpp_int int_var__skip_first_nibble;
        void skip_first_nibble (const cpp_int  & l__val);
        cpp_int skip_first_nibble() const;
    
}; // cap_ppa_decoders_crc_mask_prof_inst_t
    
class cap_ppa_decoders_crc_mask_prof_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_crc_mask_prof_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_crc_mask_prof_t(string name = "cap_ppa_decoders_crc_mask_prof_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_crc_mask_prof_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        #if 6 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_decoders_crc_mask_prof_inst_t, 6> fld;
        #else 
        cap_ppa_decoders_crc_mask_prof_inst_t fld[6];
        #endif
        int get_depth_fld() { return 6; }
    
}; // cap_ppa_decoders_crc_mask_prof_t
    
class cap_ppa_decoders_csum_prof_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_csum_prof_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_csum_prof_t(string name = "cap_ppa_decoders_csum_prof_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_csum_prof_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
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
    
}; // cap_ppa_decoders_csum_prof_t
    
class cap_ppa_decoders_csum_phdr_prof_inst_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_csum_phdr_prof_inst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_csum_phdr_prof_inst_t(string name = "cap_ppa_decoders_csum_phdr_prof_inst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_csum_phdr_prof_inst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > fld_en_cpp_int_t;
        cpp_int int_var__fld_en;
        void fld_en (const cpp_int  & l__val);
        cpp_int fld_en() const;
    
        typedef pu_cpp_int< 1 > fld_align_cpp_int_t;
        cpp_int int_var__fld_align;
        void fld_align (const cpp_int  & l__val);
        cpp_int fld_align() const;
    
        typedef pu_cpp_int< 6 > fld_start_cpp_int_t;
        cpp_int int_var__fld_start;
        void fld_start (const cpp_int  & l__val);
        cpp_int fld_start() const;
    
        typedef pu_cpp_int< 6 > fld_end_cpp_int_t;
        cpp_int int_var__fld_end;
        void fld_end (const cpp_int  & l__val);
        cpp_int fld_end() const;
    
        typedef pu_cpp_int< 1 > add_len_cpp_int_t;
        cpp_int int_var__add_len;
        void add_len (const cpp_int  & l__val);
        cpp_int add_len() const;
    
}; // cap_ppa_decoders_csum_phdr_prof_inst_t
    
class cap_ppa_decoders_csum_phdr_prof_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_csum_phdr_prof_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_csum_phdr_prof_t(string name = "cap_ppa_decoders_csum_phdr_prof_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_csum_phdr_prof_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_decoders_csum_phdr_prof_inst_t, 4> fld;
        #else 
        cap_ppa_decoders_csum_phdr_prof_inst_t fld[4];
        #endif
        int get_depth_fld() { return 4; }
    
}; // cap_ppa_decoders_csum_phdr_prof_t
    
class cap_ppa_decoders_csum_all_fld_inst_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_csum_all_fld_inst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_csum_all_fld_inst_t(string name = "cap_ppa_decoders_csum_all_fld_inst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_csum_all_fld_inst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > fld_en_cpp_int_t;
        cpp_int int_var__fld_en;
        void fld_en (const cpp_int  & l__val);
        cpp_int fld_en() const;
    
        typedef pu_cpp_int< 1 > fld_align_cpp_int_t;
        cpp_int int_var__fld_align;
        void fld_align (const cpp_int  & l__val);
        cpp_int fld_align() const;
    
        typedef pu_cpp_int< 16 > fld_start_cpp_int_t;
        cpp_int int_var__fld_start;
        void fld_start (const cpp_int  & l__val);
        cpp_int fld_start() const;
    
        typedef pu_cpp_int< 16 > fld_end_cpp_int_t;
        cpp_int int_var__fld_end;
        void fld_end (const cpp_int  & l__val);
        cpp_int fld_end() const;
    
        typedef pu_cpp_int< 1 > add_len_cpp_int_t;
        cpp_int int_var__add_len;
        void add_len (const cpp_int  & l__val);
        cpp_int add_len() const;
    
        typedef pu_cpp_int< 1 > end_eop_cpp_int_t;
        cpp_int int_var__end_eop;
        void end_eop (const cpp_int  & l__val);
        cpp_int end_eop() const;
    
        typedef pu_cpp_int< 4 > phv_csum_flit_num_cpp_int_t;
        cpp_int int_var__phv_csum_flit_num;
        void phv_csum_flit_num (const cpp_int  & l__val);
        cpp_int phv_csum_flit_num() const;
    
        typedef pu_cpp_int< 1 > csum_8b_cpp_int_t;
        cpp_int int_var__csum_8b;
        void csum_8b (const cpp_int  & l__val);
        cpp_int csum_8b() const;
    
        typedef pu_cpp_int< 16 > add_val_cpp_int_t;
        cpp_int int_var__add_val;
        void add_val (const cpp_int  & l__val);
        cpp_int add_val() const;
    
}; // cap_ppa_decoders_csum_all_fld_inst_t
    
class cap_ppa_decoders_csum_all_prof_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_csum_all_prof_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_csum_all_prof_t(string name = "cap_ppa_decoders_csum_all_prof_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_csum_all_prof_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        #if 5 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_decoders_csum_all_fld_inst_t, 5> fld;
        #else 
        cap_ppa_decoders_csum_all_fld_inst_t fld[5];
        #endif
        int get_depth_fld() { return 5; }
    
}; // cap_ppa_decoders_csum_all_prof_t
    
class cap_ppa_decoders_len_chk_prof_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_len_chk_prof_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_len_chk_prof_t(string name = "cap_ppa_decoders_len_chk_prof_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_len_chk_prof_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_decoders_len_chk_prof_t
    
class cap_ppa_decoders_pkt_ff_data_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_pkt_ff_data_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_pkt_ff_data_t(string name = "cap_ppa_decoders_pkt_ff_data_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_pkt_ff_data_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
        typedef pu_cpp_int< 6 > sz_cpp_int_t;
        cpp_int int_var__sz;
        void sz (const cpp_int  & l__val);
        cpp_int sz() const;
    
        typedef pu_cpp_int< 1 > err_cpp_int_t;
        cpp_int int_var__err;
        void err (const cpp_int  & l__val);
        cpp_int err() const;
    
        typedef pu_cpp_int< 1 > eop_cpp_int_t;
        cpp_int int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
        typedef pu_cpp_int< 1 > sop_cpp_int_t;
        cpp_int int_var__sop;
        void sop (const cpp_int  & l__val);
        cpp_int sop() const;
    
        typedef pu_cpp_int< 5 > seq_id_cpp_int_t;
        cpp_int int_var__seq_id;
        void seq_id (const cpp_int  & l__val);
        cpp_int seq_id() const;
    
}; // cap_ppa_decoders_pkt_ff_data_t
    
class cap_ppa_decoders_align_chk_prof_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_align_chk_prof_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_align_chk_prof_t(string name = "cap_ppa_decoders_align_chk_prof_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_align_chk_prof_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_ppa_decoders_align_chk_prof_t
    
class cap_ppa_decoders_err_vec_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_err_vec_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_err_vec_t(string name = "cap_ppa_decoders_err_vec_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_err_vec_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > offset_jump_chk_cpp_int_t;
        cpp_int int_var__offset_jump_chk;
        void offset_jump_chk (const cpp_int  & l__val);
        cpp_int offset_jump_chk() const;
    
        typedef pu_cpp_int< 1 > pkt_size_chk_cpp_int_t;
        cpp_int int_var__pkt_size_chk;
        void pkt_size_chk (const cpp_int  & l__val);
        cpp_int pkt_size_chk() const;
    
        typedef pu_cpp_int< 1 > offset_out_of_range_cpp_int_t;
        cpp_int int_var__offset_out_of_range;
        void offset_out_of_range (const cpp_int  & l__val);
        cpp_int offset_out_of_range() const;
    
        typedef pu_cpp_int< 1 > exceed_parse_loop_cnt_cpp_int_t;
        cpp_int int_var__exceed_parse_loop_cnt;
        void exceed_parse_loop_cnt (const cpp_int  & l__val);
        cpp_int exceed_parse_loop_cnt() const;
    
        typedef pu_cpp_int< 1 > exceed_phv_flit_cnt_cpp_int_t;
        cpp_int int_var__exceed_phv_flit_cnt;
        void exceed_phv_flit_cnt (const cpp_int  & l__val);
        cpp_int exceed_phv_flit_cnt() const;
    
        typedef pu_cpp_int< 1 > phv_upr_idx_less_cpp_int_t;
        cpp_int int_var__phv_upr_idx_less;
        void phv_upr_idx_less (const cpp_int  & l__val);
        cpp_int phv_upr_idx_less() const;
    
        typedef pu_cpp_int< 1 > init_lkp_idx_cpp_int_t;
        cpp_int int_var__init_lkp_idx;
        void init_lkp_idx (const cpp_int  & l__val);
        cpp_int init_lkp_idx() const;
    
}; // cap_ppa_decoders_err_vec_t
    
class cap_ppa_decoders_si_mux_pkt_rslt_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_si_mux_pkt_rslt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_si_mux_pkt_rslt_t(string name = "cap_ppa_decoders_si_mux_pkt_rslt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_si_mux_pkt_rslt_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_ppa_decoders_mux_idx_t inst;
    
        typedef pu_cpp_int< 16 > mux_pkt_cpp_int_t;
        cpp_int int_var__mux_pkt;
        void mux_pkt (const cpp_int  & l__val);
        cpp_int mux_pkt() const;
    
}; // cap_ppa_decoders_si_mux_pkt_rslt_t
    
class cap_ppa_decoders_si_mux_inst_rslt_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_si_mux_inst_rslt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_si_mux_inst_rslt_t(string name = "cap_ppa_decoders_si_mux_inst_rslt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_si_mux_inst_rslt_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_ppa_decoders_mux_inst_t inst;
    
        typedef pu_cpp_int< 16 > mux_pkt_inst_cpp_int_t;
        cpp_int int_var__mux_pkt_inst;
        void mux_pkt_inst (const cpp_int  & l__val);
        cpp_int mux_pkt_inst() const;
    
}; // cap_ppa_decoders_si_mux_inst_rslt_t
    
class cap_ppa_decoders_si_extract_rslt_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_si_extract_rslt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_si_extract_rslt_t(string name = "cap_ppa_decoders_si_extract_rslt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_si_extract_rslt_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_ppa_decoders_extract_inst_t inst;
    
        typedef pu_cpp_int< 32 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // cap_ppa_decoders_si_extract_rslt_t
    
class cap_ppa_decoders_si_meta_rslt_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_si_meta_rslt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_si_meta_rslt_t(string name = "cap_ppa_decoders_si_meta_rslt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_si_meta_rslt_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_ppa_decoders_meta_inst_t inst;
    
        typedef pu_cpp_int< 32 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // cap_ppa_decoders_si_meta_rslt_t
    
class cap_ppa_decoders_si_state_rslt_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_si_state_rslt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_si_state_rslt_t(string name = "cap_ppa_decoders_si_state_rslt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_si_state_rslt_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        #if 4 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_decoders_si_mux_pkt_rslt_t, 4> mux_pkt;
        #else 
        cap_ppa_decoders_si_mux_pkt_rslt_t mux_pkt[4];
        #endif
        int get_depth_mux_pkt() { return 4; }
    
        #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_decoders_si_mux_inst_rslt_t, 3> mux_inst;
        #else 
        cap_ppa_decoders_si_mux_inst_rslt_t mux_inst[3];
        #endif
        int get_depth_mux_inst() { return 3; }
    
        #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_decoders_si_extract_rslt_t, 16> extract_inst;
        #else 
        cap_ppa_decoders_si_extract_rslt_t extract_inst[16];
        #endif
        int get_depth_extract_inst() { return 16; }
    
        #if 3 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_ppa_decoders_si_meta_rslt_t, 3> meta_inst;
        #else 
        cap_ppa_decoders_si_meta_rslt_t meta_inst[3];
        #endif
        int get_depth_meta_inst() { return 3; }
    
}; // cap_ppa_decoders_si_state_rslt_t
    
class cap_ppa_decoders_si_state_info_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_si_state_info_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_si_state_info_t(string name = "cap_ppa_decoders_si_state_info_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_si_state_info_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        cap_ppa_decoders_ppa_lkp_tcam_key_t lkp_key;
    
        typedef pu_cpp_int< 1 > tcam_hit_cpp_int_t;
        cpp_int int_var__tcam_hit;
        void tcam_hit (const cpp_int  & l__val);
        cpp_int tcam_hit() const;
    
        typedef pu_cpp_int< 10 > tcam_hit_idx_cpp_int_t;
        cpp_int int_var__tcam_hit_idx;
        void tcam_hit_idx (const cpp_int  & l__val);
        cpp_int tcam_hit_idx() const;
    
        cap_ppa_decoders_si_state_rslt_t state_rslt;
    
        typedef pu_cpp_int< 3 > flit_idx_cpp_int_t;
        cpp_int int_var__flit_idx;
        void flit_idx (const cpp_int  & l__val);
        cpp_int flit_idx() const;
    
}; // cap_ppa_decoders_si_state_info_t
    
class cap_ppa_decoders_si_pe_step_info_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_si_pe_step_info_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_si_pe_step_info_t(string name = "cap_ppa_decoders_si_pe_step_info_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_si_pe_step_info_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 4 > init_profile_idx_cpp_int_t;
        cpp_int int_var__init_profile_idx;
        void init_profile_idx (const cpp_int  & l__val);
        cpp_int init_profile_idx() const;
    
        typedef pu_cpp_int< 5 > phv_global_csum_err_cpp_int_t;
        cpp_int int_var__phv_global_csum_err;
        void phv_global_csum_err (const cpp_int  & l__val);
        cpp_int phv_global_csum_err() const;
    
        typedef pu_cpp_int < 9 > lkp_cur_state_cpp_int_t;
        cpp_int int_var__lkp_cur_state[100];
        int get_depth_lkp_cur_state() { return 100; }
        void lkp_cur_state ( const cpp_int & _val, int _idx);
        cpp_int lkp_cur_state(int _idx) const;
    
        typedef pu_cpp_int < 16 > stored_lkp_val0_cpp_int_t;
        cpp_int int_var__stored_lkp_val0[100];
        int get_depth_stored_lkp_val0() { return 100; }
        void stored_lkp_val0 ( const cpp_int & _val, int _idx);
        cpp_int stored_lkp_val0(int _idx) const;
    
        typedef pu_cpp_int < 16 > lkp_val0_cpp_int_t;
        cpp_int int_var__lkp_val0[100];
        int get_depth_lkp_val0() { return 100; }
        void lkp_val0 ( const cpp_int & _val, int _idx);
        cpp_int lkp_val0(int _idx) const;
    
        typedef pu_cpp_int < 16 > stored_lkp_val1_cpp_int_t;
        cpp_int int_var__stored_lkp_val1[100];
        int get_depth_stored_lkp_val1() { return 100; }
        void stored_lkp_val1 ( const cpp_int & _val, int _idx);
        cpp_int stored_lkp_val1(int _idx) const;
    
        typedef pu_cpp_int < 16 > lkp_val1_cpp_int_t;
        cpp_int int_var__lkp_val1[100];
        int get_depth_lkp_val1() { return 100; }
        void lkp_val1 ( const cpp_int & _val, int _idx);
        cpp_int lkp_val1(int _idx) const;
    
        typedef pu_cpp_int < 16 > stored_lkp_val2_cpp_int_t;
        cpp_int int_var__stored_lkp_val2[100];
        int get_depth_stored_lkp_val2() { return 100; }
        void stored_lkp_val2 ( const cpp_int & _val, int _idx);
        cpp_int stored_lkp_val2(int _idx) const;
    
        typedef pu_cpp_int < 16 > lkp_val2_cpp_int_t;
        cpp_int int_var__lkp_val2[100];
        int get_depth_lkp_val2() { return 100; }
        void lkp_val2 ( const cpp_int & _val, int _idx);
        cpp_int lkp_val2(int _idx) const;
    
        typedef pu_cpp_int < 1 > tcam_hit_cpp_int_t;
        cpp_int int_var__tcam_hit[100];
        int get_depth_tcam_hit() { return 100; }
        void tcam_hit ( const cpp_int & _val, int _idx);
        cpp_int tcam_hit(int _idx) const;
    
        typedef pu_cpp_int < 10 > tcam_hit_idx_cpp_int_t;
        cpp_int int_var__tcam_hit_idx[100];
        int get_depth_tcam_hit_idx() { return 100; }
        void tcam_hit_idx ( const cpp_int & _val, int _idx);
        cpp_int tcam_hit_idx(int _idx) const;
    
        typedef pu_cpp_int < 3 > cur_flit_idx_cpp_int_t;
        cpp_int int_var__cur_flit_idx[100];
        int get_depth_cur_flit_idx() { return 100; }
        void cur_flit_idx ( const cpp_int & _val, int _idx);
        cpp_int cur_flit_idx(int _idx) const;
    
        typedef pu_cpp_int < 16 > mux_pkt0_cpp_int_t;
        cpp_int int_var__mux_pkt0[100];
        int get_depth_mux_pkt0() { return 100; }
        void mux_pkt0 ( const cpp_int & _val, int _idx);
        cpp_int mux_pkt0(int _idx) const;
    
        typedef pu_cpp_int < 16 > mux_pkt1_cpp_int_t;
        cpp_int int_var__mux_pkt1[100];
        int get_depth_mux_pkt1() { return 100; }
        void mux_pkt1 ( const cpp_int & _val, int _idx);
        cpp_int mux_pkt1(int _idx) const;
    
        typedef pu_cpp_int < 16 > mux_pkt2_cpp_int_t;
        cpp_int int_var__mux_pkt2[100];
        int get_depth_mux_pkt2() { return 100; }
        void mux_pkt2 ( const cpp_int & _val, int _idx);
        cpp_int mux_pkt2(int _idx) const;
    
        typedef pu_cpp_int < 16 > mux_pkt3_cpp_int_t;
        cpp_int int_var__mux_pkt3[100];
        int get_depth_mux_pkt3() { return 100; }
        void mux_pkt3 ( const cpp_int & _val, int _idx);
        cpp_int mux_pkt3(int _idx) const;
    
        typedef pu_cpp_int < 16 > mux_inst_data0_cpp_int_t;
        cpp_int int_var__mux_inst_data0[100];
        int get_depth_mux_inst_data0() { return 100; }
        void mux_inst_data0 ( const cpp_int & _val, int _idx);
        cpp_int mux_inst_data0(int _idx) const;
    
        typedef pu_cpp_int < 16 > mux_inst_data1_cpp_int_t;
        cpp_int int_var__mux_inst_data1[100];
        int get_depth_mux_inst_data1() { return 100; }
        void mux_inst_data1 ( const cpp_int & _val, int _idx);
        cpp_int mux_inst_data1(int _idx) const;
    
        typedef pu_cpp_int < 16 > mux_inst_data2_cpp_int_t;
        cpp_int int_var__mux_inst_data2[100];
        int get_depth_mux_inst_data2() { return 100; }
        void mux_inst_data2 ( const cpp_int & _val, int _idx);
        cpp_int mux_inst_data2(int _idx) const;
    
        typedef pu_cpp_int < 1024 > extract_phv_data_cpp_int_t;
        cpp_int int_var__extract_phv_data[100];
        int get_depth_extract_phv_data() { return 100; }
        void extract_phv_data ( const cpp_int & _val, int _idx);
        cpp_int extract_phv_data(int _idx) const;
    
}; // cap_ppa_decoders_si_pe_step_info_t
    
class cap_ppa_decoders_si_non_pe_step_info_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ppa_decoders_si_non_pe_step_info_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ppa_decoders_si_non_pe_step_info_t(string name = "cap_ppa_decoders_si_non_pe_step_info_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ppa_decoders_si_non_pe_step_info_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 5 > preparser_csum_err_out_cpp_int_t;
        cpp_int int_var__preparser_csum_err_out;
        void preparser_csum_err_out (const cpp_int  & l__val);
        cpp_int preparser_csum_err_out() const;
    
        typedef pu_cpp_int< 16 > parse_end_ptr_cpp_int_t;
        cpp_int int_var__parse_end_ptr;
        void parse_end_ptr (const cpp_int  & l__val);
        cpp_int parse_end_ptr() const;
    
        typedef pu_cpp_int< 6 > err_vec_cpp_int_t;
        cpp_int int_var__err_vec;
        void err_vec (const cpp_int  & l__val);
        cpp_int err_vec() const;
    
        typedef pu_cpp_int< 6 > err_code_cpp_int_t;
        cpp_int int_var__err_code;
        void err_code (const cpp_int  & l__val);
        cpp_int err_code() const;
    
        typedef pu_cpp_int< 32 > crc_val_cpp_int_t;
        cpp_int int_var__crc_val;
        void crc_val (const cpp_int  & l__val);
        cpp_int crc_val() const;
    
        typedef pu_cpp_int< 1 > crc_err_cpp_int_t;
        cpp_int int_var__crc_err;
        void crc_err (const cpp_int  & l__val);
        cpp_int crc_err() const;
    
        typedef pu_cpp_int < 32 > csum_val_cpp_int_t;
        cpp_int int_var__csum_val[5];
        int get_depth_csum_val() { return 5; }
        void csum_val ( const cpp_int & _val, int _idx);
        cpp_int csum_val(int _idx) const;
    
        typedef pu_cpp_int < 1 > csum_err_cpp_int_t;
        cpp_int int_var__csum_err[5];
        int get_depth_csum_err() { return 5; }
        void csum_err ( const cpp_int & _val, int _idx);
        cpp_int csum_err(int _idx) const;
    
        typedef pu_cpp_int< 32 > len_chk_val0_cpp_int_t;
        cpp_int int_var__len_chk_val0;
        void len_chk_val0 (const cpp_int  & l__val);
        cpp_int len_chk_val0() const;
    
        typedef pu_cpp_int< 1 > len_chk_err0_cpp_int_t;
        cpp_int int_var__len_chk_err0;
        void len_chk_err0 (const cpp_int  & l__val);
        cpp_int len_chk_err0() const;
    
        typedef pu_cpp_int< 32 > len_chk_val1_cpp_int_t;
        cpp_int int_var__len_chk_val1;
        void len_chk_val1 (const cpp_int  & l__val);
        cpp_int len_chk_val1() const;
    
        typedef pu_cpp_int< 1 > len_chk_err1_cpp_int_t;
        cpp_int int_var__len_chk_err1;
        void len_chk_err1 (const cpp_int  & l__val);
        cpp_int len_chk_err1() const;
    
        typedef pu_cpp_int< 32 > len_chk_val2_cpp_int_t;
        cpp_int int_var__len_chk_val2;
        void len_chk_val2 (const cpp_int  & l__val);
        cpp_int len_chk_val2() const;
    
        typedef pu_cpp_int< 1 > len_chk_err2_cpp_int_t;
        cpp_int int_var__len_chk_err2;
        void len_chk_err2 (const cpp_int  & l__val);
        cpp_int len_chk_err2() const;
    
        typedef pu_cpp_int< 32 > len_chk_val3_cpp_int_t;
        cpp_int int_var__len_chk_val3;
        void len_chk_val3 (const cpp_int  & l__val);
        cpp_int len_chk_val3() const;
    
        typedef pu_cpp_int< 1 > len_chk_err3_cpp_int_t;
        cpp_int int_var__len_chk_err3;
        void len_chk_err3 (const cpp_int  & l__val);
        cpp_int len_chk_err3() const;
    
        typedef pu_cpp_int< 32 > align_chk_val0_cpp_int_t;
        cpp_int int_var__align_chk_val0;
        void align_chk_val0 (const cpp_int  & l__val);
        cpp_int align_chk_val0() const;
    
        typedef pu_cpp_int< 1 > align_chk_err0_cpp_int_t;
        cpp_int int_var__align_chk_err0;
        void align_chk_err0 (const cpp_int  & l__val);
        cpp_int align_chk_err0() const;
    
        typedef pu_cpp_int< 32 > align_chk_val1_cpp_int_t;
        cpp_int int_var__align_chk_val1;
        void align_chk_val1 (const cpp_int  & l__val);
        cpp_int align_chk_val1() const;
    
        typedef pu_cpp_int< 1 > align_chk_err1_cpp_int_t;
        cpp_int int_var__align_chk_err1;
        void align_chk_err1 (const cpp_int  & l__val);
        cpp_int align_chk_err1() const;
    
}; // cap_ppa_decoders_si_non_pe_step_info_t
    
#endif // CAP_PPA_DECODERS_H
        