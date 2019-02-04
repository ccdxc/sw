
#ifndef CAP_DPPCSUM_CSR_H
#define CAP_DPPCSUM_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_dppcsum_csr_cfg_spare_csum_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppcsum_csr_cfg_spare_csum_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppcsum_csr_cfg_spare_csum_t(string name = "cap_dppcsum_csr_cfg_spare_csum_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppcsum_csr_cfg_spare_csum_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
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
    
}; // cap_dppcsum_csr_cfg_spare_csum_t
    
class cap_dppcsum_csr_cfg_crc_mask_profile_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppcsum_csr_cfg_crc_mask_profile_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppcsum_csr_cfg_crc_mask_profile_t(string name = "cap_dppcsum_csr_cfg_crc_mask_profile_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppcsum_csr_cfg_crc_mask_profile_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > fld_en_0_cpp_int_t;
        cpp_int int_var__fld_en_0;
        void fld_en_0 (const cpp_int  & l__val);
        cpp_int fld_en_0() const;
    
        typedef pu_cpp_int< 1 > fld_start_sub_0_cpp_int_t;
        cpp_int int_var__fld_start_sub_0;
        void fld_start_sub_0 (const cpp_int  & l__val);
        cpp_int fld_start_sub_0() const;
    
        typedef pu_cpp_int< 14 > fld_start_0_cpp_int_t;
        cpp_int int_var__fld_start_0;
        void fld_start_0 (const cpp_int  & l__val);
        cpp_int fld_start_0() const;
    
        typedef pu_cpp_int< 1 > fld_end_sub_0_cpp_int_t;
        cpp_int int_var__fld_end_sub_0;
        void fld_end_sub_0 (const cpp_int  & l__val);
        cpp_int fld_end_sub_0() const;
    
        typedef pu_cpp_int< 14 > fld_end_0_cpp_int_t;
        cpp_int int_var__fld_end_0;
        void fld_end_0 (const cpp_int  & l__val);
        cpp_int fld_end_0() const;
    
        typedef pu_cpp_int< 1 > skip_first_nibble_0_cpp_int_t;
        cpp_int int_var__skip_first_nibble_0;
        void skip_first_nibble_0 (const cpp_int  & l__val);
        cpp_int skip_first_nibble_0() const;
    
        typedef pu_cpp_int< 1 > fld_en_1_cpp_int_t;
        cpp_int int_var__fld_en_1;
        void fld_en_1 (const cpp_int  & l__val);
        cpp_int fld_en_1() const;
    
        typedef pu_cpp_int< 1 > fld_start_sub_1_cpp_int_t;
        cpp_int int_var__fld_start_sub_1;
        void fld_start_sub_1 (const cpp_int  & l__val);
        cpp_int fld_start_sub_1() const;
    
        typedef pu_cpp_int< 14 > fld_start_1_cpp_int_t;
        cpp_int int_var__fld_start_1;
        void fld_start_1 (const cpp_int  & l__val);
        cpp_int fld_start_1() const;
    
        typedef pu_cpp_int< 1 > fld_end_sub_1_cpp_int_t;
        cpp_int int_var__fld_end_sub_1;
        void fld_end_sub_1 (const cpp_int  & l__val);
        cpp_int fld_end_sub_1() const;
    
        typedef pu_cpp_int< 14 > fld_end_1_cpp_int_t;
        cpp_int int_var__fld_end_1;
        void fld_end_1 (const cpp_int  & l__val);
        cpp_int fld_end_1() const;
    
        typedef pu_cpp_int< 1 > skip_first_nibble_1_cpp_int_t;
        cpp_int int_var__skip_first_nibble_1;
        void skip_first_nibble_1 (const cpp_int  & l__val);
        cpp_int skip_first_nibble_1() const;
    
        typedef pu_cpp_int< 1 > fld_en_2_cpp_int_t;
        cpp_int int_var__fld_en_2;
        void fld_en_2 (const cpp_int  & l__val);
        cpp_int fld_en_2() const;
    
        typedef pu_cpp_int< 1 > fld_start_sub_2_cpp_int_t;
        cpp_int int_var__fld_start_sub_2;
        void fld_start_sub_2 (const cpp_int  & l__val);
        cpp_int fld_start_sub_2() const;
    
        typedef pu_cpp_int< 14 > fld_start_2_cpp_int_t;
        cpp_int int_var__fld_start_2;
        void fld_start_2 (const cpp_int  & l__val);
        cpp_int fld_start_2() const;
    
        typedef pu_cpp_int< 1 > fld_end_sub_2_cpp_int_t;
        cpp_int int_var__fld_end_sub_2;
        void fld_end_sub_2 (const cpp_int  & l__val);
        cpp_int fld_end_sub_2() const;
    
        typedef pu_cpp_int< 14 > fld_end_2_cpp_int_t;
        cpp_int int_var__fld_end_2;
        void fld_end_2 (const cpp_int  & l__val);
        cpp_int fld_end_2() const;
    
        typedef pu_cpp_int< 1 > skip_first_nibble_2_cpp_int_t;
        cpp_int int_var__skip_first_nibble_2;
        void skip_first_nibble_2 (const cpp_int  & l__val);
        cpp_int skip_first_nibble_2() const;
    
        typedef pu_cpp_int< 1 > fld_en_3_cpp_int_t;
        cpp_int int_var__fld_en_3;
        void fld_en_3 (const cpp_int  & l__val);
        cpp_int fld_en_3() const;
    
        typedef pu_cpp_int< 1 > fld_start_sub_3_cpp_int_t;
        cpp_int int_var__fld_start_sub_3;
        void fld_start_sub_3 (const cpp_int  & l__val);
        cpp_int fld_start_sub_3() const;
    
        typedef pu_cpp_int< 14 > fld_start_3_cpp_int_t;
        cpp_int int_var__fld_start_3;
        void fld_start_3 (const cpp_int  & l__val);
        cpp_int fld_start_3() const;
    
        typedef pu_cpp_int< 1 > fld_end_sub_3_cpp_int_t;
        cpp_int int_var__fld_end_sub_3;
        void fld_end_sub_3 (const cpp_int  & l__val);
        cpp_int fld_end_sub_3() const;
    
        typedef pu_cpp_int< 14 > fld_end_3_cpp_int_t;
        cpp_int int_var__fld_end_3;
        void fld_end_3 (const cpp_int  & l__val);
        cpp_int fld_end_3() const;
    
        typedef pu_cpp_int< 1 > skip_first_nibble_3_cpp_int_t;
        cpp_int int_var__skip_first_nibble_3;
        void skip_first_nibble_3 (const cpp_int  & l__val);
        cpp_int skip_first_nibble_3() const;
    
        typedef pu_cpp_int< 1 > fld_en_4_cpp_int_t;
        cpp_int int_var__fld_en_4;
        void fld_en_4 (const cpp_int  & l__val);
        cpp_int fld_en_4() const;
    
        typedef pu_cpp_int< 1 > fld_start_sub_4_cpp_int_t;
        cpp_int int_var__fld_start_sub_4;
        void fld_start_sub_4 (const cpp_int  & l__val);
        cpp_int fld_start_sub_4() const;
    
        typedef pu_cpp_int< 14 > fld_start_4_cpp_int_t;
        cpp_int int_var__fld_start_4;
        void fld_start_4 (const cpp_int  & l__val);
        cpp_int fld_start_4() const;
    
        typedef pu_cpp_int< 1 > fld_end_sub_4_cpp_int_t;
        cpp_int int_var__fld_end_sub_4;
        void fld_end_sub_4 (const cpp_int  & l__val);
        cpp_int fld_end_sub_4() const;
    
        typedef pu_cpp_int< 14 > fld_end_4_cpp_int_t;
        cpp_int int_var__fld_end_4;
        void fld_end_4 (const cpp_int  & l__val);
        cpp_int fld_end_4() const;
    
        typedef pu_cpp_int< 1 > skip_first_nibble_4_cpp_int_t;
        cpp_int int_var__skip_first_nibble_4;
        void skip_first_nibble_4 (const cpp_int  & l__val);
        cpp_int skip_first_nibble_4() const;
    
        typedef pu_cpp_int< 1 > fill_cpp_int_t;
        cpp_int int_var__fill;
        void fill (const cpp_int  & l__val);
        cpp_int fill() const;
    
}; // cap_dppcsum_csr_cfg_crc_mask_profile_t
    
class cap_dppcsum_csr_cfg_crc_profile_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppcsum_csr_cfg_crc_profile_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppcsum_csr_cfg_crc_profile_t(string name = "cap_dppcsum_csr_cfg_crc_profile_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppcsum_csr_cfg_crc_profile_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > use_phv_len_cpp_int_t;
        cpp_int int_var__use_phv_len;
        void use_phv_len (const cpp_int  & l__val);
        cpp_int use_phv_len() const;
    
        typedef pu_cpp_int< 6 > phv_len_sel_cpp_int_t;
        cpp_int int_var__phv_len_sel;
        void phv_len_sel (const cpp_int  & l__val);
        cpp_int phv_len_sel() const;
    
        typedef pu_cpp_int< 16 > len_mask_cpp_int_t;
        cpp_int int_var__len_mask;
        void len_mask (const cpp_int  & l__val);
        cpp_int len_mask() const;
    
        typedef pu_cpp_int< 1 > len_shift_left_cpp_int_t;
        cpp_int int_var__len_shift_left;
        void len_shift_left (const cpp_int  & l__val);
        cpp_int len_shift_left() const;
    
        typedef pu_cpp_int< 4 > len_shift_val_cpp_int_t;
        cpp_int int_var__len_shift_val;
        void len_shift_val (const cpp_int  & l__val);
        cpp_int len_shift_val() const;
    
        typedef pu_cpp_int< 1 > start_sop_cpp_int_t;
        cpp_int int_var__start_sop;
        void start_sop (const cpp_int  & l__val);
        cpp_int start_sop() const;
    
        typedef pu_cpp_int< 1 > end_eop_cpp_int_t;
        cpp_int int_var__end_eop;
        void end_eop (const cpp_int  & l__val);
        cpp_int end_eop() const;
    
        typedef pu_cpp_int< 1 > start_adj_sub_cpp_int_t;
        cpp_int int_var__start_adj_sub;
        void start_adj_sub (const cpp_int  & l__val);
        cpp_int start_adj_sub() const;
    
        typedef pu_cpp_int< 14 > start_adj_cpp_int_t;
        cpp_int int_var__start_adj;
        void start_adj (const cpp_int  & l__val);
        cpp_int start_adj() const;
    
        typedef pu_cpp_int< 1 > end_adj_sub_cpp_int_t;
        cpp_int int_var__end_adj_sub;
        void end_adj_sub (const cpp_int  & l__val);
        cpp_int end_adj_sub() const;
    
        typedef pu_cpp_int< 14 > end_adj_cpp_int_t;
        cpp_int int_var__end_adj;
        void end_adj (const cpp_int  & l__val);
        cpp_int end_adj() const;
    
        typedef pu_cpp_int< 1 > loc_adj_sub_cpp_int_t;
        cpp_int int_var__loc_adj_sub;
        void loc_adj_sub (const cpp_int  & l__val);
        cpp_int loc_adj_sub() const;
    
        typedef pu_cpp_int< 14 > loc_adj_cpp_int_t;
        cpp_int int_var__loc_adj;
        void loc_adj (const cpp_int  & l__val);
        cpp_int loc_adj() const;
    
        typedef pu_cpp_int< 1 > crc_unit_include_bm_cpp_int_t;
        cpp_int int_var__crc_unit_include_bm;
        void crc_unit_include_bm (const cpp_int  & l__val);
        cpp_int crc_unit_include_bm() const;
    
        typedef pu_cpp_int< 1 > add_fix_mask_cpp_int_t;
        cpp_int int_var__add_fix_mask;
        void add_fix_mask (const cpp_int  & l__val);
        cpp_int add_fix_mask() const;
    
}; // cap_dppcsum_csr_cfg_crc_profile_t
    
class cap_dppcsum_csr_cfg_crc_hdrs_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppcsum_csr_cfg_crc_hdrs_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppcsum_csr_cfg_crc_hdrs_t(string name = "cap_dppcsum_csr_cfg_crc_hdrs_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppcsum_csr_cfg_crc_hdrs_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 7 > hdr_num_cpp_int_t;
        cpp_int int_var__hdr_num;
        void hdr_num (const cpp_int  & l__val);
        cpp_int hdr_num() const;
    
        typedef pu_cpp_int< 1 > crc_vld_cpp_int_t;
        cpp_int int_var__crc_vld;
        void crc_vld (const cpp_int  & l__val);
        cpp_int crc_vld() const;
    
        typedef pu_cpp_int< 1 > crc_unit_cpp_int_t;
        cpp_int int_var__crc_unit;
        void crc_unit (const cpp_int  & l__val);
        cpp_int crc_unit() const;
    
        typedef pu_cpp_int< 4 > crc_profile_cpp_int_t;
        cpp_int int_var__crc_profile;
        void crc_profile (const cpp_int  & l__val);
        cpp_int crc_profile() const;
    
        typedef pu_cpp_int< 1 > mask_vld_cpp_int_t;
        cpp_int int_var__mask_vld;
        void mask_vld (const cpp_int  & l__val);
        cpp_int mask_vld() const;
    
        typedef pu_cpp_int< 1 > mask_unit_cpp_int_t;
        cpp_int int_var__mask_unit;
        void mask_unit (const cpp_int  & l__val);
        cpp_int mask_unit() const;
    
        typedef pu_cpp_int< 4 > mask_profile_cpp_int_t;
        cpp_int int_var__mask_profile;
        void mask_profile (const cpp_int  & l__val);
        cpp_int mask_profile() const;
    
        typedef pu_cpp_int< 8 > hdrfld_start_cpp_int_t;
        cpp_int int_var__hdrfld_start;
        void hdrfld_start (const cpp_int  & l__val);
        cpp_int hdrfld_start() const;
    
        typedef pu_cpp_int< 8 > hdrfld_end_cpp_int_t;
        cpp_int int_var__hdrfld_end;
        void hdrfld_end (const cpp_int  & l__val);
        cpp_int hdrfld_end() const;
    
}; // cap_dppcsum_csr_cfg_crc_hdrs_t
    
class cap_dppcsum_csr_cfg_csum_phdr_profile_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppcsum_csr_cfg_csum_phdr_profile_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppcsum_csr_cfg_csum_phdr_profile_t(string name = "cap_dppcsum_csr_cfg_csum_phdr_profile_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppcsum_csr_cfg_csum_phdr_profile_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > fld_en_0_cpp_int_t;
        cpp_int int_var__fld_en_0;
        void fld_en_0 (const cpp_int  & l__val);
        cpp_int fld_en_0() const;
    
        typedef pu_cpp_int< 14 > fld_start_0_cpp_int_t;
        cpp_int int_var__fld_start_0;
        void fld_start_0 (const cpp_int  & l__val);
        cpp_int fld_start_0() const;
    
        typedef pu_cpp_int< 14 > fld_end_0_cpp_int_t;
        cpp_int int_var__fld_end_0;
        void fld_end_0 (const cpp_int  & l__val);
        cpp_int fld_end_0() const;
    
        typedef pu_cpp_int< 1 > fld_align_0_cpp_int_t;
        cpp_int int_var__fld_align_0;
        void fld_align_0 (const cpp_int  & l__val);
        cpp_int fld_align_0() const;
    
        typedef pu_cpp_int< 1 > fld_en_1_cpp_int_t;
        cpp_int int_var__fld_en_1;
        void fld_en_1 (const cpp_int  & l__val);
        cpp_int fld_en_1() const;
    
        typedef pu_cpp_int< 14 > fld_start_1_cpp_int_t;
        cpp_int int_var__fld_start_1;
        void fld_start_1 (const cpp_int  & l__val);
        cpp_int fld_start_1() const;
    
        typedef pu_cpp_int< 14 > fld_end_1_cpp_int_t;
        cpp_int int_var__fld_end_1;
        void fld_end_1 (const cpp_int  & l__val);
        cpp_int fld_end_1() const;
    
        typedef pu_cpp_int< 1 > fld_align_1_cpp_int_t;
        cpp_int int_var__fld_align_1;
        void fld_align_1 (const cpp_int  & l__val);
        cpp_int fld_align_1() const;
    
        typedef pu_cpp_int< 1 > fld_en_2_cpp_int_t;
        cpp_int int_var__fld_en_2;
        void fld_en_2 (const cpp_int  & l__val);
        cpp_int fld_en_2() const;
    
        typedef pu_cpp_int< 14 > fld_start_2_cpp_int_t;
        cpp_int int_var__fld_start_2;
        void fld_start_2 (const cpp_int  & l__val);
        cpp_int fld_start_2() const;
    
        typedef pu_cpp_int< 14 > fld_end_2_cpp_int_t;
        cpp_int int_var__fld_end_2;
        void fld_end_2 (const cpp_int  & l__val);
        cpp_int fld_end_2() const;
    
        typedef pu_cpp_int< 1 > fld_align_2_cpp_int_t;
        cpp_int int_var__fld_align_2;
        void fld_align_2 (const cpp_int  & l__val);
        cpp_int fld_align_2() const;
    
        typedef pu_cpp_int< 1 > fld_en_3_cpp_int_t;
        cpp_int int_var__fld_en_3;
        void fld_en_3 (const cpp_int  & l__val);
        cpp_int fld_en_3() const;
    
        typedef pu_cpp_int< 14 > fld_start_3_cpp_int_t;
        cpp_int int_var__fld_start_3;
        void fld_start_3 (const cpp_int  & l__val);
        cpp_int fld_start_3() const;
    
        typedef pu_cpp_int< 14 > fld_end_3_cpp_int_t;
        cpp_int int_var__fld_end_3;
        void fld_end_3 (const cpp_int  & l__val);
        cpp_int fld_end_3() const;
    
        typedef pu_cpp_int< 1 > fld_align_3_cpp_int_t;
        cpp_int int_var__fld_align_3;
        void fld_align_3 (const cpp_int  & l__val);
        cpp_int fld_align_3() const;
    
        typedef pu_cpp_int< 1 > add_len_cpp_int_t;
        cpp_int int_var__add_len;
        void add_len (const cpp_int  & l__val);
        cpp_int add_len() const;
    
}; // cap_dppcsum_csr_cfg_csum_phdr_profile_t
    
class cap_dppcsum_csr_cfg_csum_profile_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppcsum_csr_cfg_csum_profile_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppcsum_csr_cfg_csum_profile_t(string name = "cap_dppcsum_csr_cfg_csum_profile_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppcsum_csr_cfg_csum_profile_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 1 > use_phv_len_cpp_int_t;
        cpp_int int_var__use_phv_len;
        void use_phv_len (const cpp_int  & l__val);
        cpp_int use_phv_len() const;
    
        typedef pu_cpp_int< 6 > phv_len_sel_cpp_int_t;
        cpp_int int_var__phv_len_sel;
        void phv_len_sel (const cpp_int  & l__val);
        cpp_int phv_len_sel() const;
    
        typedef pu_cpp_int< 16 > len_mask_cpp_int_t;
        cpp_int int_var__len_mask;
        void len_mask (const cpp_int  & l__val);
        cpp_int len_mask() const;
    
        typedef pu_cpp_int< 1 > len_shift_left_cpp_int_t;
        cpp_int int_var__len_shift_left;
        void len_shift_left (const cpp_int  & l__val);
        cpp_int len_shift_left() const;
    
        typedef pu_cpp_int< 4 > len_shift_val_cpp_int_t;
        cpp_int int_var__len_shift_val;
        void len_shift_val (const cpp_int  & l__val);
        cpp_int len_shift_val() const;
    
        typedef pu_cpp_int< 14 > start_adj_cpp_int_t;
        cpp_int int_var__start_adj;
        void start_adj (const cpp_int  & l__val);
        cpp_int start_adj() const;
    
        typedef pu_cpp_int< 14 > end_adj_cpp_int_t;
        cpp_int int_var__end_adj;
        void end_adj (const cpp_int  & l__val);
        cpp_int end_adj() const;
    
        typedef pu_cpp_int< 14 > loc_adj_cpp_int_t;
        cpp_int int_var__loc_adj;
        void loc_adj (const cpp_int  & l__val);
        cpp_int loc_adj() const;
    
        typedef pu_cpp_int< 1 > add_len_cpp_int_t;
        cpp_int int_var__add_len;
        void add_len (const cpp_int  & l__val);
        cpp_int add_len() const;
    
        typedef pu_cpp_int< 1 > eight_bit_cpp_int_t;
        cpp_int int_var__eight_bit;
        void eight_bit (const cpp_int  & l__val);
        cpp_int eight_bit() const;
    
        typedef pu_cpp_int< 1 > invert_zero_cpp_int_t;
        cpp_int int_var__invert_zero;
        void invert_zero (const cpp_int  & l__val);
        cpp_int invert_zero() const;
    
        typedef pu_cpp_int< 1 > no_csum_rw_cpp_int_t;
        cpp_int int_var__no_csum_rw;
        void no_csum_rw (const cpp_int  & l__val);
        cpp_int no_csum_rw() const;
    
        typedef pu_cpp_int< 16 > phdr_next_hdr_cpp_int_t;
        cpp_int int_var__phdr_next_hdr;
        void phdr_next_hdr (const cpp_int  & l__val);
        cpp_int phdr_next_hdr() const;
    
}; // cap_dppcsum_csr_cfg_csum_profile_t
    
class cap_dppcsum_csr_cfg_csum_hdrs_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppcsum_csr_cfg_csum_hdrs_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppcsum_csr_cfg_csum_hdrs_t(string name = "cap_dppcsum_csr_cfg_csum_hdrs_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppcsum_csr_cfg_csum_hdrs_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        typedef pu_cpp_int< 7 > hdr_num_cpp_int_t;
        cpp_int int_var__hdr_num;
        void hdr_num (const cpp_int  & l__val);
        cpp_int hdr_num() const;
    
        typedef pu_cpp_int< 1 > csum_vld_cpp_int_t;
        cpp_int int_var__csum_vld;
        void csum_vld (const cpp_int  & l__val);
        cpp_int csum_vld() const;
    
        typedef pu_cpp_int< 1 > csum_copy_vld_cpp_int_t;
        cpp_int int_var__csum_copy_vld;
        void csum_copy_vld (const cpp_int  & l__val);
        cpp_int csum_copy_vld() const;
    
        typedef pu_cpp_int< 3 > csum_unit_cpp_int_t;
        cpp_int int_var__csum_unit;
        void csum_unit (const cpp_int  & l__val);
        cpp_int csum_unit() const;
    
        typedef pu_cpp_int< 4 > csum_profile_cpp_int_t;
        cpp_int int_var__csum_profile;
        void csum_profile (const cpp_int  & l__val);
        cpp_int csum_profile() const;
    
        typedef pu_cpp_int< 1 > phdr_vld_cpp_int_t;
        cpp_int int_var__phdr_vld;
        void phdr_vld (const cpp_int  & l__val);
        cpp_int phdr_vld() const;
    
        typedef pu_cpp_int< 3 > phdr_unit_cpp_int_t;
        cpp_int int_var__phdr_unit;
        void phdr_unit (const cpp_int  & l__val);
        cpp_int phdr_unit() const;
    
        typedef pu_cpp_int< 4 > phdr_profile_cpp_int_t;
        cpp_int int_var__phdr_profile;
        void phdr_profile (const cpp_int  & l__val);
        cpp_int phdr_profile() const;
    
        typedef pu_cpp_int< 5 > csum_unit_include_bm_cpp_int_t;
        cpp_int int_var__csum_unit_include_bm;
        void csum_unit_include_bm (const cpp_int  & l__val);
        cpp_int csum_unit_include_bm() const;
    
        typedef pu_cpp_int< 1 > crc_include_bm_cpp_int_t;
        cpp_int int_var__crc_include_bm;
        void crc_include_bm (const cpp_int  & l__val);
        cpp_int crc_include_bm() const;
    
        typedef pu_cpp_int< 8 > hdrfld_start_cpp_int_t;
        cpp_int int_var__hdrfld_start;
        void hdrfld_start (const cpp_int  & l__val);
        cpp_int hdrfld_start() const;
    
        typedef pu_cpp_int< 8 > hdrfld_end_cpp_int_t;
        cpp_int int_var__hdrfld_end;
        void hdrfld_end (const cpp_int  & l__val);
        cpp_int hdrfld_end() const;
    
}; // cap_dppcsum_csr_cfg_csum_hdrs_t
    
class cap_dppcsum_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dppcsum_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dppcsum_csr_t(string name = "cap_dppcsum_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dppcsum_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        #ifndef CSR_NO_SHOW_IMPL 
        virtual void show();
        #endif
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
        #ifndef EXCLUDE_PER_FIELD_CNTRL
            virtual bool get_field_val(const char *, cpp_int & , int level=1);
            virtual bool set_field_val(const char *, cpp_int & , int level=1);
            std::vector<string> get_fields(int level=1) const;
        #endif        
        virtual void clear();
    
        #if 24 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_dppcsum_csr_cfg_csum_hdrs_t, 24> cfg_csum_hdrs;
        #else 
        cap_dppcsum_csr_cfg_csum_hdrs_t cfg_csum_hdrs[24];
        #endif
        int get_depth_cfg_csum_hdrs() { return 24; }
    
        #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_dppcsum_csr_cfg_csum_profile_t, 16> cfg_csum_profile;
        #else 
        cap_dppcsum_csr_cfg_csum_profile_t cfg_csum_profile[16];
        #endif
        int get_depth_cfg_csum_profile() { return 16; }
    
        #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_dppcsum_csr_cfg_csum_phdr_profile_t, 16> cfg_csum_phdr_profile;
        #else 
        cap_dppcsum_csr_cfg_csum_phdr_profile_t cfg_csum_phdr_profile[16];
        #endif
        int get_depth_cfg_csum_phdr_profile() { return 16; }
    
        #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_dppcsum_csr_cfg_crc_hdrs_t, 16> cfg_crc_hdrs;
        #else 
        cap_dppcsum_csr_cfg_crc_hdrs_t cfg_crc_hdrs[16];
        #endif
        int get_depth_cfg_crc_hdrs() { return 16; }
    
        #if 12 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_dppcsum_csr_cfg_crc_profile_t, 12> cfg_crc_profile;
        #else 
        cap_dppcsum_csr_cfg_crc_profile_t cfg_crc_profile[12];
        #endif
        int get_depth_cfg_crc_profile() { return 12; }
    
        #if 12 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_dppcsum_csr_cfg_crc_mask_profile_t, 12> cfg_crc_mask_profile;
        #else 
        cap_dppcsum_csr_cfg_crc_mask_profile_t cfg_crc_mask_profile[12];
        #endif
        int get_depth_cfg_crc_mask_profile() { return 12; }
    
        #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_dppcsum_csr_cfg_spare_csum_t, 32> cfg_spare_csum;
        #else 
        cap_dppcsum_csr_cfg_spare_csum_t cfg_spare_csum[32];
        #endif
        int get_depth_cfg_spare_csum() { return 32; }
    
}; // cap_dppcsum_csr_t
    
#endif // CAP_DPPCSUM_CSR_H
        