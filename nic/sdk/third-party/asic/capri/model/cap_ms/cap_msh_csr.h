
#ifndef CAP_MSH_CSR_H
#define CAP_MSH_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_msh_csr_sta_bist_qspi_ram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msh_csr_sta_bist_qspi_ram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msh_csr_sta_bist_qspi_ram_t(string name = "cap_msh_csr_sta_bist_qspi_ram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msh_csr_sta_bist_qspi_ram_t();
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
    
        typedef pu_cpp_int< 1 > done_fail_cpp_int_t;
        cpp_int int_var__done_fail;
        void done_fail (const cpp_int  & l__val);
        cpp_int done_fail() const;
    
        typedef pu_cpp_int< 1 > done_pass_cpp_int_t;
        cpp_int int_var__done_pass;
        void done_pass (const cpp_int  & l__val);
        cpp_int done_pass() const;
    
}; // cap_msh_csr_sta_bist_qspi_ram_t
    
class cap_msh_csr_cfg_bist_qspi_ram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msh_csr_cfg_bist_qspi_ram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msh_csr_cfg_bist_qspi_ram_t(string name = "cap_msh_csr_cfg_bist_qspi_ram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msh_csr_cfg_bist_qspi_ram_t();
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
    
        typedef pu_cpp_int< 1 > run_cpp_int_t;
        cpp_int int_var__run;
        void run (const cpp_int  & l__val);
        cpp_int run() const;
    
}; // cap_msh_csr_cfg_bist_qspi_ram_t
    
class cap_msh_csr_sta_esecure_m0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msh_csr_sta_esecure_m0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msh_csr_sta_esecure_m0_t(string name = "cap_msh_csr_sta_esecure_m0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msh_csr_sta_esecure_m0_t();
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
    
        typedef pu_cpp_int< 1 > dbgrestarted_cpp_int_t;
        cpp_int int_var__dbgrestarted;
        void dbgrestarted (const cpp_int  & l__val);
        cpp_int dbgrestarted() const;
    
        typedef pu_cpp_int< 1 > halted_cpp_int_t;
        cpp_int int_var__halted;
        void halted (const cpp_int  & l__val);
        cpp_int halted() const;
    
        typedef pu_cpp_int< 1 > sleepdeep_cpp_int_t;
        cpp_int int_var__sleepdeep;
        void sleepdeep (const cpp_int  & l__val);
        cpp_int sleepdeep() const;
    
        typedef pu_cpp_int< 1 > wakeup_cpp_int_t;
        cpp_int int_var__wakeup;
        void wakeup (const cpp_int  & l__val);
        cpp_int wakeup() const;
    
        typedef pu_cpp_int< 34 > wicsense_cpp_int_t;
        cpp_int int_var__wicsense;
        void wicsense (const cpp_int  & l__val);
        cpp_int wicsense() const;
    
}; // cap_msh_csr_sta_esecure_m0_t
    
class cap_msh_csr_cfg_esecure_m0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msh_csr_cfg_esecure_m0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msh_csr_cfg_esecure_m0_t(string name = "cap_msh_csr_cfg_esecure_m0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msh_csr_cfg_esecure_m0_t();
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
    
        typedef pu_cpp_int< 1 > dbgrestart_cpp_int_t;
        cpp_int int_var__dbgrestart;
        void dbgrestart (const cpp_int  & l__val);
        cpp_int dbgrestart() const;
    
        typedef pu_cpp_int< 1 > edbgrq_cpp_int_t;
        cpp_int int_var__edbgrq;
        void edbgrq (const cpp_int  & l__val);
        cpp_int edbgrq() const;
    
        typedef pu_cpp_int< 8 > irqlatency_cpp_int_t;
        cpp_int int_var__irqlatency;
        void irqlatency (const cpp_int  & l__val);
        cpp_int irqlatency() const;
    
        typedef pu_cpp_int< 1 > rstbypass_cpp_int_t;
        cpp_int int_var__rstbypass;
        void rstbypass (const cpp_int  & l__val);
        cpp_int rstbypass() const;
    
        typedef pu_cpp_int< 1 > stclken_cpp_int_t;
        cpp_int int_var__stclken;
        void stclken (const cpp_int  & l__val);
        cpp_int stclken() const;
    
}; // cap_msh_csr_cfg_esecure_m0_t
    
class cap_msh_csr_sta_esecure_esystem_ram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msh_csr_sta_esecure_esystem_ram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msh_csr_sta_esecure_esystem_ram_t(string name = "cap_msh_csr_sta_esecure_esystem_ram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msh_csr_sta_esecure_esystem_ram_t();
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
    
        typedef pu_cpp_int< 1 > ecc_correctable_cpp_int_t;
        cpp_int int_var__ecc_correctable;
        void ecc_correctable (const cpp_int  & l__val);
        cpp_int ecc_correctable() const;
    
        typedef pu_cpp_int< 1 > ecc_uncorrectable_cpp_int_t;
        cpp_int int_var__ecc_uncorrectable;
        void ecc_uncorrectable (const cpp_int  & l__val);
        cpp_int ecc_uncorrectable() const;
    
        typedef pu_cpp_int< 14 > ecc_addr_cpp_int_t;
        cpp_int int_var__ecc_addr;
        void ecc_addr (const cpp_int  & l__val);
        cpp_int ecc_addr() const;
    
        typedef pu_cpp_int< 5 > ecc_syndrome_cpp_int_t;
        cpp_int int_var__ecc_syndrome;
        void ecc_syndrome (const cpp_int  & l__val);
        cpp_int ecc_syndrome() const;
    
        typedef pu_cpp_int< 1 > esystem_ram_fail_cpp_int_t;
        cpp_int int_var__esystem_ram_fail;
        void esystem_ram_fail (const cpp_int  & l__val);
        cpp_int esystem_ram_fail() const;
    
        typedef pu_cpp_int< 1 > esystem_ram_pass_cpp_int_t;
        cpp_int int_var__esystem_ram_pass;
        void esystem_ram_pass (const cpp_int  & l__val);
        cpp_int esystem_ram_pass() const;
    
}; // cap_msh_csr_sta_esecure_esystem_ram_t
    
class cap_msh_csr_cfg_esecure_esystem_ram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msh_csr_cfg_esecure_esystem_ram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msh_csr_cfg_esecure_esystem_ram_t(string name = "cap_msh_csr_cfg_esecure_esystem_ram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msh_csr_cfg_esecure_esystem_ram_t();
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
    
        typedef pu_cpp_int< 1 > disable_cor_cpp_int_t;
        cpp_int int_var__disable_cor;
        void disable_cor (const cpp_int  & l__val);
        cpp_int disable_cor() const;
    
        typedef pu_cpp_int< 1 > disable_det_cpp_int_t;
        cpp_int int_var__disable_det;
        void disable_det (const cpp_int  & l__val);
        cpp_int disable_det() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_msh_csr_cfg_esecure_esystem_ram_t
    
class cap_msh_csr_sta_esecure_data_ram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msh_csr_sta_esecure_data_ram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msh_csr_sta_esecure_data_ram_t(string name = "cap_msh_csr_sta_esecure_data_ram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msh_csr_sta_esecure_data_ram_t();
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
    
        typedef pu_cpp_int< 1 > ecc_correctable_cpp_int_t;
        cpp_int int_var__ecc_correctable;
        void ecc_correctable (const cpp_int  & l__val);
        cpp_int ecc_correctable() const;
    
        typedef pu_cpp_int< 1 > ecc_uncorrectable_cpp_int_t;
        cpp_int int_var__ecc_uncorrectable;
        void ecc_uncorrectable (const cpp_int  & l__val);
        cpp_int ecc_uncorrectable() const;
    
        typedef pu_cpp_int< 9 > ecc_addr_cpp_int_t;
        cpp_int int_var__ecc_addr;
        void ecc_addr (const cpp_int  & l__val);
        cpp_int ecc_addr() const;
    
        typedef pu_cpp_int< 18 > ecc_syndrome_cpp_int_t;
        cpp_int int_var__ecc_syndrome;
        void ecc_syndrome (const cpp_int  & l__val);
        cpp_int ecc_syndrome() const;
    
        typedef pu_cpp_int< 1 > data_ram_fail_cpp_int_t;
        cpp_int int_var__data_ram_fail;
        void data_ram_fail (const cpp_int  & l__val);
        cpp_int data_ram_fail() const;
    
        typedef pu_cpp_int< 1 > data_ram_pass_cpp_int_t;
        cpp_int int_var__data_ram_pass;
        void data_ram_pass (const cpp_int  & l__val);
        cpp_int data_ram_pass() const;
    
}; // cap_msh_csr_sta_esecure_data_ram_t
    
class cap_msh_csr_sta_esecure_sys_rom_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msh_csr_sta_esecure_sys_rom_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msh_csr_sta_esecure_sys_rom_t(string name = "cap_msh_csr_sta_esecure_sys_rom_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msh_csr_sta_esecure_sys_rom_t();
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
    
        typedef pu_cpp_int< 1 > bist_fail_cpp_int_t;
        cpp_int int_var__bist_fail;
        void bist_fail (const cpp_int  & l__val);
        cpp_int bist_fail() const;
    
        typedef pu_cpp_int< 1 > bist_pass_cpp_int_t;
        cpp_int int_var__bist_pass;
        void bist_pass (const cpp_int  & l__val);
        cpp_int bist_pass() const;
    
}; // cap_msh_csr_sta_esecure_sys_rom_t
    
class cap_msh_csr_sta_esecure_pk_rom_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msh_csr_sta_esecure_pk_rom_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msh_csr_sta_esecure_pk_rom_t(string name = "cap_msh_csr_sta_esecure_pk_rom_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msh_csr_sta_esecure_pk_rom_t();
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
    
        typedef pu_cpp_int< 1 > bist_fail_cpp_int_t;
        cpp_int int_var__bist_fail;
        void bist_fail (const cpp_int  & l__val);
        cpp_int bist_fail() const;
    
        typedef pu_cpp_int< 1 > bist_pass_cpp_int_t;
        cpp_int int_var__bist_pass;
        void bist_pass (const cpp_int  & l__val);
        cpp_int bist_pass() const;
    
}; // cap_msh_csr_sta_esecure_pk_rom_t
    
class cap_msh_csr_sta_esecure_puf_ram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msh_csr_sta_esecure_puf_ram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msh_csr_sta_esecure_puf_ram_t(string name = "cap_msh_csr_sta_esecure_puf_ram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msh_csr_sta_esecure_puf_ram_t();
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
    
        typedef pu_cpp_int< 1 > bist_fail_cpp_int_t;
        cpp_int int_var__bist_fail;
        void bist_fail (const cpp_int  & l__val);
        cpp_int bist_fail() const;
    
        typedef pu_cpp_int< 1 > bist_pass_cpp_int_t;
        cpp_int int_var__bist_pass;
        void bist_pass (const cpp_int  & l__val);
        cpp_int bist_pass() const;
    
}; // cap_msh_csr_sta_esecure_puf_ram_t
    
class cap_msh_csr_cfg_esecure_data_ram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msh_csr_cfg_esecure_data_ram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msh_csr_cfg_esecure_data_ram_t(string name = "cap_msh_csr_cfg_esecure_data_ram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msh_csr_cfg_esecure_data_ram_t();
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
    
        typedef pu_cpp_int< 1 > disable_cor_cpp_int_t;
        cpp_int int_var__disable_cor;
        void disable_cor (const cpp_int  & l__val);
        cpp_int disable_cor() const;
    
        typedef pu_cpp_int< 1 > disable_det_cpp_int_t;
        cpp_int int_var__disable_det;
        void disable_det (const cpp_int  & l__val);
        cpp_int disable_det() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_msh_csr_cfg_esecure_data_ram_t
    
class cap_msh_csr_cfg_esecure_sys_rom_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msh_csr_cfg_esecure_sys_rom_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msh_csr_cfg_esecure_sys_rom_t(string name = "cap_msh_csr_cfg_esecure_sys_rom_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msh_csr_cfg_esecure_sys_rom_t();
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
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_msh_csr_cfg_esecure_sys_rom_t
    
class cap_msh_csr_cfg_esecure_pk_rom_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msh_csr_cfg_esecure_pk_rom_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msh_csr_cfg_esecure_pk_rom_t(string name = "cap_msh_csr_cfg_esecure_pk_rom_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msh_csr_cfg_esecure_pk_rom_t();
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
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_msh_csr_cfg_esecure_pk_rom_t
    
class cap_msh_csr_sta_ms_qbist_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msh_csr_sta_ms_qbist_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msh_csr_sta_ms_qbist_t(string name = "cap_msh_csr_sta_ms_qbist_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msh_csr_sta_ms_qbist_t();
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
    
        typedef pu_cpp_int< 1 > active_cpp_int_t;
        cpp_int int_var__active;
        void active (const cpp_int  & l__val);
        cpp_int active() const;
    
        typedef pu_cpp_int< 1 > ok_cpp_int_t;
        cpp_int int_var__ok;
        void ok (const cpp_int  & l__val);
        cpp_int ok() const;
    
        typedef pu_cpp_int< 1 > running_cpp_int_t;
        cpp_int int_var__running;
        void running (const cpp_int  & l__val);
        cpp_int running() const;
    
        typedef pu_cpp_int< 1 > error_cpp_int_t;
        cpp_int int_var__error;
        void error (const cpp_int  & l__val);
        cpp_int error() const;
    
        typedef pu_cpp_int< 1 > puf_busy_cpp_int_t;
        cpp_int int_var__puf_busy;
        void puf_busy (const cpp_int  & l__val);
        cpp_int puf_busy() const;
    
        typedef pu_cpp_int< 1 > puf_error_cpp_int_t;
        cpp_int int_var__puf_error;
        void puf_error (const cpp_int  & l__val);
        cpp_int puf_error() const;
    
}; // cap_msh_csr_sta_ms_qbist_t
    
class cap_msh_csr_cfg_ms_qbist_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msh_csr_cfg_ms_qbist_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msh_csr_cfg_ms_qbist_t(string name = "cap_msh_csr_cfg_ms_qbist_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msh_csr_cfg_ms_qbist_t();
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
    
        typedef pu_cpp_int< 1 > bist_ovr_cpp_int_t;
        cpp_int int_var__bist_ovr;
        void bist_ovr (const cpp_int  & l__val);
        cpp_int bist_ovr() const;
    
        typedef pu_cpp_int< 1 > bist_enable_ovr_cpp_int_t;
        cpp_int int_var__bist_enable_ovr;
        void bist_enable_ovr (const cpp_int  & l__val);
        cpp_int bist_enable_ovr() const;
    
        typedef pu_cpp_int< 1 > scanmode_ovr_cpp_int_t;
        cpp_int int_var__scanmode_ovr;
        void scanmode_ovr (const cpp_int  & l__val);
        cpp_int scanmode_ovr() const;
    
}; // cap_msh_csr_cfg_ms_qbist_t
    
class cap_msh_csr_cfg_esecure_puf_ram_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msh_csr_cfg_esecure_puf_ram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msh_csr_cfg_esecure_puf_ram_t(string name = "cap_msh_csr_cfg_esecure_puf_ram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msh_csr_cfg_esecure_puf_ram_t();
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
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_msh_csr_cfg_esecure_puf_ram_t
    
class cap_msh_csr_sta_zeroize_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msh_csr_sta_zeroize_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msh_csr_sta_zeroize_t(string name = "cap_msh_csr_sta_zeroize_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msh_csr_sta_zeroize_t();
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
    
        typedef pu_cpp_int< 1 > tamper_pin_cpp_int_t;
        cpp_int int_var__tamper_pin;
        void tamper_pin (const cpp_int  & l__val);
        cpp_int tamper_pin() const;
    
        typedef pu_cpp_int< 1 > esecure_cpp_int_t;
        cpp_int int_var__esecure;
        void esecure (const cpp_int  & l__val);
        cpp_int esecure() const;
    
}; // cap_msh_csr_sta_zeroize_t
    
class cap_msh_csr_sta_tamper_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msh_csr_sta_tamper_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msh_csr_sta_tamper_t(string name = "cap_msh_csr_sta_tamper_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msh_csr_sta_tamper_t();
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
    
        typedef pu_cpp_int< 1 > clr_cpp_int_t;
        cpp_int int_var__clr;
        void clr (const cpp_int  & l__val);
        cpp_int clr() const;
    
        typedef pu_cpp_int< 1 > irq_cpp_int_t;
        cpp_int int_var__irq;
        void irq (const cpp_int  & l__val);
        cpp_int irq() const;
    
        typedef pu_cpp_int< 1 > rst_cpp_int_t;
        cpp_int int_var__rst;
        void rst (const cpp_int  & l__val);
        cpp_int rst() const;
    
}; // cap_msh_csr_sta_tamper_t
    
class cap_msh_csr_cfg_tamper_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msh_csr_cfg_tamper_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msh_csr_cfg_tamper_t(string name = "cap_msh_csr_cfg_tamper_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msh_csr_cfg_tamper_t();
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
    
        typedef pu_cpp_int< 1 > clrack_cpp_int_t;
        cpp_int int_var__clrack;
        void clrack (const cpp_int  & l__val);
        cpp_int clrack() const;
    
}; // cap_msh_csr_cfg_tamper_t
    
class cap_msh_csr_sta_esecure_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msh_csr_sta_esecure_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msh_csr_sta_esecure_t(string name = "cap_msh_csr_sta_esecure_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msh_csr_sta_esecure_t();
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
    
        typedef pu_cpp_int< 1 > rst_sec_cpp_int_t;
        cpp_int int_var__rst_sec;
        void rst_sec (const cpp_int  & l__val);
        cpp_int rst_sec() const;
    
        typedef pu_cpp_int< 1 > rst_host_cpp_int_t;
        cpp_int int_var__rst_host;
        void rst_host (const cpp_int  & l__val);
        cpp_int rst_host() const;
    
        typedef pu_cpp_int< 1 > rst_host_cpu_cpp_int_t;
        cpp_int int_var__rst_host_cpu;
        void rst_host_cpu (const cpp_int  & l__val);
        cpp_int rst_host_cpu() const;
    
}; // cap_msh_csr_sta_esecure_t
    
class cap_msh_csr_cfg_esecure_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msh_csr_cfg_esecure_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msh_csr_cfg_esecure_t(string name = "cap_msh_csr_cfg_esecure_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msh_csr_cfg_esecure_t();
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
    
        typedef pu_cpp_int< 1 > AesDisableCM_cpp_int_t;
        cpp_int int_var__AesDisableCM;
        void AesDisableCM (const cpp_int  & l__val);
        cpp_int AesDisableCM() const;
    
        typedef pu_cpp_int< 1 > PKDisableCM_cpp_int_t;
        cpp_int int_var__PKDisableCM;
        void PKDisableCM (const cpp_int  & l__val);
        cpp_int PKDisableCM() const;
    
}; // cap_msh_csr_cfg_esecure_t
    
class cap_msh_csr_dci_grant_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msh_csr_dci_grant_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msh_csr_dci_grant_t(string name = "cap_msh_csr_dci_grant_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msh_csr_dci_grant_t();
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
    
        typedef pu_cpp_int< 16 > sta_GRANT_cpp_int_t;
        cpp_int int_var__sta_GRANT;
        void sta_GRANT (const cpp_int  & l__val);
        cpp_int sta_GRANT() const;
    
        typedef pu_cpp_int< 1 > sta_WACK_cpp_int_t;
        cpp_int int_var__sta_WACK;
        void sta_WACK (const cpp_int  & l__val);
        cpp_int sta_WACK() const;
    
        typedef pu_cpp_int< 1 > sta_RREQ_cpp_int_t;
        cpp_int int_var__sta_RREQ;
        void sta_RREQ (const cpp_int  & l__val);
        cpp_int sta_RREQ() const;
    
}; // cap_msh_csr_dci_grant_t
    
class cap_msh_csr_dci_stat_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msh_csr_dci_stat_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msh_csr_dci_stat_t(string name = "cap_msh_csr_dci_stat_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msh_csr_dci_stat_t();
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
    
        typedef pu_cpp_int< 32 > sta_RDATA_cpp_int_t;
        cpp_int int_var__sta_RDATA;
        void sta_RDATA (const cpp_int  & l__val);
        cpp_int sta_RDATA() const;
    
}; // cap_msh_csr_dci_stat_t
    
class cap_msh_csr_dci_req_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msh_csr_dci_req_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msh_csr_dci_req_t(string name = "cap_msh_csr_dci_req_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msh_csr_dci_req_t();
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
    
        typedef pu_cpp_int< 1 > cfg_WREQ_cpp_int_t;
        cpp_int int_var__cfg_WREQ;
        void cfg_WREQ (const cpp_int  & l__val);
        cpp_int cfg_WREQ() const;
    
        typedef pu_cpp_int< 1 > cfg_RACK_cpp_int_t;
        cpp_int int_var__cfg_RACK;
        void cfg_RACK (const cpp_int  & l__val);
        cpp_int cfg_RACK() const;
    
        typedef pu_cpp_int< 1 > cfg_DISCONNECT_cpp_int_t;
        cpp_int int_var__cfg_DISCONNECT;
        void cfg_DISCONNECT (const cpp_int  & l__val);
        cpp_int cfg_DISCONNECT() const;
    
}; // cap_msh_csr_dci_req_t
    
class cap_msh_csr_dci_wdata_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msh_csr_dci_wdata_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msh_csr_dci_wdata_t(string name = "cap_msh_csr_dci_wdata_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msh_csr_dci_wdata_t();
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
    
        typedef pu_cpp_int< 32 > cfg_WDATA_cpp_int_t;
        cpp_int int_var__cfg_WDATA;
        void cfg_WDATA (const cpp_int  & l__val);
        cpp_int cfg_WDATA() const;
    
}; // cap_msh_csr_dci_wdata_t
    
class cap_msh_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_msh_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_msh_csr_t(string name = "cap_msh_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_msh_csr_t();
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
    
        cap_msh_csr_dci_wdata_t dci_wdata;
    
        cap_msh_csr_dci_req_t dci_req;
    
        cap_msh_csr_dci_stat_t dci_stat;
    
        cap_msh_csr_dci_grant_t dci_grant;
    
        cap_msh_csr_cfg_esecure_t cfg_esecure;
    
        cap_msh_csr_sta_esecure_t sta_esecure;
    
        cap_msh_csr_cfg_tamper_t cfg_tamper;
    
        cap_msh_csr_sta_tamper_t sta_tamper;
    
        cap_msh_csr_sta_zeroize_t sta_zeroize;
    
        cap_msh_csr_cfg_esecure_puf_ram_t cfg_esecure_puf_ram;
    
        cap_msh_csr_cfg_ms_qbist_t cfg_ms_qbist;
    
        cap_msh_csr_sta_ms_qbist_t sta_ms_qbist;
    
        cap_msh_csr_cfg_esecure_pk_rom_t cfg_esecure_pk_rom;
    
        cap_msh_csr_cfg_esecure_sys_rom_t cfg_esecure_sys_rom;
    
        cap_msh_csr_cfg_esecure_data_ram_t cfg_esecure_data_ram;
    
        cap_msh_csr_sta_esecure_puf_ram_t sta_esecure_puf_ram;
    
        cap_msh_csr_sta_esecure_pk_rom_t sta_esecure_pk_rom;
    
        cap_msh_csr_sta_esecure_sys_rom_t sta_esecure_sys_rom;
    
        cap_msh_csr_sta_esecure_data_ram_t sta_esecure_data_ram;
    
        cap_msh_csr_cfg_esecure_esystem_ram_t cfg_esecure_esystem_ram;
    
        cap_msh_csr_sta_esecure_esystem_ram_t sta_esecure_esystem_ram;
    
        cap_msh_csr_cfg_esecure_m0_t cfg_esecure_m0;
    
        cap_msh_csr_sta_esecure_m0_t sta_esecure_m0;
    
        cap_msh_csr_cfg_bist_qspi_ram_t cfg_bist_qspi_ram;
    
        cap_msh_csr_sta_bist_qspi_ram_t sta_bist_qspi_ram;
    
}; // cap_msh_csr_t
    
#endif // CAP_MSH_CSR_H
        