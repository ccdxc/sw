
#ifndef CAP_MSR_CSR_H
#define CAP_MSR_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_msr_csr_cfg_flash_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msr_csr_cfg_flash_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msr_csr_cfg_flash_t(string name = "cap_msr_csr_cfg_flash_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msr_csr_cfg_flash_t();
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
    
        typedef pu_cpp_int< 35 > reset_timer_cpp_int_t;
        cpp_int int_var__reset_timer;
        void reset_timer (const cpp_int  & l__val);
        cpp_int reset_timer() const;
    
}; // cap_msr_csr_cfg_flash_t
    
class cap_msr_csr_cfg_arm_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msr_csr_cfg_arm_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msr_csr_cfg_arm_t(string name = "cap_msr_csr_cfg_arm_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msr_csr_cfg_arm_t();
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
    
        typedef pu_cpp_int< 1 > ov_resRESETPOR_cpp_int_t;
        cpp_int int_var__ov_resRESETPOR;
        void ov_resRESETPOR (const cpp_int  & l__val);
        cpp_int ov_resRESETPOR() const;
    
        typedef pu_cpp_int< 1 > ov_resCPUPORESET_cpp_int_t;
        cpp_int int_var__ov_resCPUPORESET;
        void ov_resCPUPORESET (const cpp_int  & l__val);
        cpp_int ov_resCPUPORESET() const;
    
}; // cap_msr_csr_cfg_arm_t
    
class cap_msr_csr_c2p_clk_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msr_csr_c2p_clk_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msr_csr_c2p_clk_t(string name = "cap_msr_csr_c2p_clk_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msr_csr_c2p_clk_t();
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
    
        typedef pu_cpp_int< 4 > obs_sel_cpp_int_t;
        cpp_int int_var__obs_sel;
        void obs_sel (const cpp_int  & l__val);
        cpp_int obs_sel() const;
    
        typedef pu_cpp_int< 2 > obs_div_cpp_int_t;
        cpp_int int_var__obs_div;
        void obs_div (const cpp_int  & l__val);
        cpp_int obs_div() const;
    
}; // cap_msr_csr_c2p_clk_t
    
class cap_msr_csr_sta_cause_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msr_csr_sta_cause_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msr_csr_sta_cause_t(string name = "cap_msr_csr_sta_cause_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msr_csr_sta_cause_t();
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
    
        typedef pu_cpp_int< 3 > reason_type_cpp_int_t;
        cpp_int int_var__reason_type;
        void reason_type (const cpp_int  & l__val);
        cpp_int reason_type() const;
    
}; // cap_msr_csr_sta_cause_t
    
class cap_msr_csr_cfg_boot_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msr_csr_cfg_boot_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msr_csr_cfg_boot_t(string name = "cap_msr_csr_cfg_boot_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msr_csr_cfg_boot_t();
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
    
        typedef pu_cpp_int< 32 > mode_reg_cpp_int_t;
        cpp_int int_var__mode_reg;
        void mode_reg (const cpp_int  & l__val);
        cpp_int mode_reg() const;
    
}; // cap_msr_csr_cfg_boot_t
    
class cap_msr_csr_cfg_nonresettable_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msr_csr_cfg_nonresettable_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msr_csr_cfg_nonresettable_t(string name = "cap_msr_csr_cfg_nonresettable_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msr_csr_cfg_nonresettable_t();
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
    
        typedef pu_cpp_int< 32 > nr_reg_cpp_int_t;
        cpp_int int_var__nr_reg;
        void nr_reg (const cpp_int  & l__val);
        cpp_int nr_reg() const;
    
}; // cap_msr_csr_cfg_nonresettable_t
    
class cap_msr_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msr_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msr_csr_t(string name = "cap_msr_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msr_csr_t();
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
    
        #if 16 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_msr_csr_cfg_nonresettable_t, 16> cfg_nonresettable;
        #else 
        cap_msr_csr_cfg_nonresettable_t cfg_nonresettable[16];
        #endif
        int get_depth_cfg_nonresettable() { return 16; }
    
        cap_msr_csr_cfg_boot_t cfg_boot;
    
        cap_msr_csr_sta_cause_t sta_cause;
    
        cap_msr_csr_c2p_clk_t c2p_clk;
    
        cap_msr_csr_cfg_arm_t cfg_arm;
    
        cap_msr_csr_cfg_flash_t cfg_flash;
    
}; // cap_msr_csr_t
    
#endif // CAP_MSR_CSR_H
        