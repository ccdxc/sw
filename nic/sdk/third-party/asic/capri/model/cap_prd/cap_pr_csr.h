
#ifndef CAP_PR_CSR_H
#define CAP_PR_CSR_H

#include "cap_csr_base.h" 
#include "cap_prd_csr.h" 
#include "cap_psp_csr.h" 

using namespace std;
class cap_pr_csr_int_reg1_int_enable_clear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pr_csr_int_reg1_int_enable_clear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pr_csr_int_reg1_int_enable_clear_t(string name = "cap_pr_csr_int_reg1_int_enable_clear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pr_csr_int_reg1_int_enable_clear_t();
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
    
        typedef pu_cpp_int< 1 > slave_prd_enable_cpp_int_t;
        cpp_int int_var__slave_prd_enable;
        void slave_prd_enable (const cpp_int  & l__val);
        cpp_int slave_prd_enable() const;
    
        typedef pu_cpp_int< 1 > slave_psp_enable_cpp_int_t;
        cpp_int int_var__slave_psp_enable;
        void slave_psp_enable (const cpp_int  & l__val);
        cpp_int slave_psp_enable() const;
    
}; // cap_pr_csr_int_reg1_int_enable_clear_t
    
class cap_pr_csr_intreg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pr_csr_intreg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pr_csr_intreg_t(string name = "cap_pr_csr_intreg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pr_csr_intreg_t();
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
    
        typedef pu_cpp_int< 1 > slave_prd_interrupt_cpp_int_t;
        cpp_int int_var__slave_prd_interrupt;
        void slave_prd_interrupt (const cpp_int  & l__val);
        cpp_int slave_prd_interrupt() const;
    
        typedef pu_cpp_int< 1 > slave_psp_interrupt_cpp_int_t;
        cpp_int int_var__slave_psp_interrupt;
        void slave_psp_interrupt (const cpp_int  & l__val);
        cpp_int slave_psp_interrupt() const;
    
}; // cap_pr_csr_intreg_t
    
class cap_pr_csr_intgrp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pr_csr_intgrp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pr_csr_intgrp_t(string name = "cap_pr_csr_intgrp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pr_csr_intgrp_t();
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
    
        cap_pr_csr_intreg_t intreg;
    
        cap_pr_csr_intreg_t int_test_set;
    
        cap_pr_csr_int_reg1_int_enable_clear_t int_enable_set;
    
        cap_pr_csr_int_reg1_int_enable_clear_t int_enable_clear;
    
}; // cap_pr_csr_intgrp_t
    
class cap_pr_csr_intreg_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pr_csr_intreg_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pr_csr_intreg_status_t(string name = "cap_pr_csr_intreg_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pr_csr_intreg_status_t();
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
    
        typedef pu_cpp_int< 1 > int_reg1_interrupt_cpp_int_t;
        cpp_int int_var__int_reg1_interrupt;
        void int_reg1_interrupt (const cpp_int  & l__val);
        cpp_int int_reg1_interrupt() const;
    
}; // cap_pr_csr_intreg_status_t
    
class cap_pr_csr_int_groups_int_enable_rw_reg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pr_csr_int_groups_int_enable_rw_reg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pr_csr_int_groups_int_enable_rw_reg_t(string name = "cap_pr_csr_int_groups_int_enable_rw_reg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pr_csr_int_groups_int_enable_rw_reg_t();
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
    
        typedef pu_cpp_int< 1 > int_reg1_enable_cpp_int_t;
        cpp_int int_var__int_reg1_enable;
        void int_reg1_enable (const cpp_int  & l__val);
        cpp_int int_reg1_enable() const;
    
}; // cap_pr_csr_int_groups_int_enable_rw_reg_t
    
class cap_pr_csr_intgrp_status_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pr_csr_intgrp_status_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pr_csr_intgrp_status_t(string name = "cap_pr_csr_intgrp_status_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pr_csr_intgrp_status_t();
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
    
        cap_pr_csr_intreg_status_t intreg;
    
        cap_pr_csr_int_groups_int_enable_rw_reg_t int_enable_rw_reg;
    
        cap_pr_csr_intreg_status_t int_rw_reg;
    
}; // cap_pr_csr_intgrp_status_t
    
class cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t(string name = "cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t();
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
    
        typedef pu_cpp_int< 40 > msk_cpp_int_t;
        cpp_int int_var__msk;
        void msk (const cpp_int  & l__val);
        cpp_int msk() const;
    
}; // cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t
    
class cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t(string name = "cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t();
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
    
        typedef pu_cpp_int< 40 > msk_cpp_int_t;
        cpp_int int_var__msk;
        void msk (const cpp_int  & l__val);
        cpp_int msk() const;
    
}; // cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t
    
class cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t(string name = "cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t();
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
    
        typedef pu_cpp_int< 40 > msk_cpp_int_t;
        cpp_int int_var__msk;
        void msk (const cpp_int  & l__val);
        cpp_int msk() const;
    
}; // cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t
    
class cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t(string name = "cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t();
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
    
        typedef pu_cpp_int< 40 > msk_cpp_int_t;
        cpp_int int_var__msk;
        void msk (const cpp_int  & l__val);
        cpp_int msk() const;
    
}; // cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t
    
class cap_pr_csr_cfg_uid2sidLL_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pr_csr_cfg_uid2sidLL_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pr_csr_cfg_uid2sidLL_t(string name = "cap_pr_csr_cfg_uid2sidLL_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pr_csr_cfg_uid2sidLL_t();
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
    
        typedef pu_cpp_int< 7 > spare_cpp_int_t;
        cpp_int int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
        typedef pu_cpp_int< 2 > mode_cpp_int_t;
        cpp_int int_var__mode;
        void mode (const cpp_int  & l__val);
        cpp_int mode() const;
    
        typedef pu_cpp_int< 7 > base_cpp_int_t;
        cpp_int int_var__base;
        void base (const cpp_int  & l__val);
        cpp_int base() const;
    
}; // cap_pr_csr_cfg_uid2sidLL_t
    
class cap_pr_csr_base_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pr_csr_base_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pr_csr_base_t(string name = "cap_pr_csr_base_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pr_csr_base_t();
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
    
}; // cap_pr_csr_base_t
    
class cap_pr_csr_csr_intr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pr_csr_csr_intr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pr_csr_csr_intr_t(string name = "cap_pr_csr_csr_intr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pr_csr_csr_intr_t();
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
    
}; // cap_pr_csr_csr_intr_t
    
class cap_pr_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pr_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pr_csr_t(string name = "cap_pr_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pr_csr_t();
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
    
        cap_pr_csr_csr_intr_t csr_intr;
    
        cap_pr_csr_base_t base;
    
        cap_pr_csr_cfg_uid2sidLL_t cfg_uid2sidLL;
    
        cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit0_t cfg_uid2sidLL_hbm_hash_msk_bit0;
    
        cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit1_t cfg_uid2sidLL_hbm_hash_msk_bit1;
    
        cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit2_t cfg_uid2sidLL_hbm_hash_msk_bit2;
    
        cap_pr_csr_cfg_uid2sidLL_hbm_hash_msk_bit3_t cfg_uid2sidLL_hbm_hash_msk_bit3;
    
        cap_psp_csr_t psp;
    
        cap_prd_csr_t prd;
    
        cap_pr_csr_intgrp_status_t int_groups;
    
        cap_pr_csr_intgrp_t int_reg1;
    
}; // cap_pr_csr_t
    
#endif // CAP_PR_CSR_H
        