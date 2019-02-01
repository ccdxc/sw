
#ifndef CAP_MSS_CSR_H
#define CAP_MSS_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_mss_csr_cfg_mod_tx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mss_csr_cfg_mod_tx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mss_csr_cfg_mod_tx_t(string name = "cap_mss_csr_cfg_mod_tx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mss_csr_cfg_mod_tx_t();
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
    
        typedef pu_cpp_int< 1 > srst_cpp_int_t;
        cpp_int int_var__srst;
        void srst (const cpp_int  & l__val);
        cpp_int srst() const;
    
        typedef pu_cpp_int< 1 > hrst_cpp_int_t;
        cpp_int int_var__hrst;
        void hrst (const cpp_int  & l__val);
        cpp_int hrst() const;
    
        typedef pu_cpp_int< 1 > rst_override_cpp_int_t;
        cpp_int int_var__rst_override;
        void rst_override (const cpp_int  & l__val);
        cpp_int rst_override() const;
    
}; // cap_mss_csr_cfg_mod_tx_t
    
class cap_mss_csr_cfg_mod_em_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mss_csr_cfg_mod_em_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mss_csr_cfg_mod_em_t(string name = "cap_mss_csr_cfg_mod_em_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mss_csr_cfg_mod_em_t();
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
    
        typedef pu_cpp_int< 1 > srst_cpp_int_t;
        cpp_int int_var__srst;
        void srst (const cpp_int  & l__val);
        cpp_int srst() const;
    
        typedef pu_cpp_int< 1 > hrst_cpp_int_t;
        cpp_int int_var__hrst;
        void hrst (const cpp_int  & l__val);
        cpp_int hrst() const;
    
        typedef pu_cpp_int< 1 > rst_override_cpp_int_t;
        cpp_int int_var__rst_override;
        void rst_override (const cpp_int  & l__val);
        cpp_int rst_override() const;
    
}; // cap_mss_csr_cfg_mod_em_t
    
class cap_mss_csr_cfg_mod_ms_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mss_csr_cfg_mod_ms_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mss_csr_cfg_mod_ms_t(string name = "cap_mss_csr_cfg_mod_ms_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mss_csr_cfg_mod_ms_t();
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
    
        typedef pu_cpp_int< 1 > srst_cpp_int_t;
        cpp_int int_var__srst;
        void srst (const cpp_int  & l__val);
        cpp_int srst() const;
    
        typedef pu_cpp_int< 1 > hrst_cpp_int_t;
        cpp_int int_var__hrst;
        void hrst (const cpp_int  & l__val);
        cpp_int hrst() const;
    
        typedef pu_cpp_int< 1 > rst_override_cpp_int_t;
        cpp_int int_var__rst_override;
        void rst_override (const cpp_int  & l__val);
        cpp_int rst_override() const;
    
}; // cap_mss_csr_cfg_mod_ms_t
    
class cap_mss_csr_cfg_mod_bx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mss_csr_cfg_mod_bx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mss_csr_cfg_mod_bx_t(string name = "cap_mss_csr_cfg_mod_bx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mss_csr_cfg_mod_bx_t();
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
    
        typedef pu_cpp_int< 1 > srst_cpp_int_t;
        cpp_int int_var__srst;
        void srst (const cpp_int  & l__val);
        cpp_int srst() const;
    
        typedef pu_cpp_int< 1 > hrst_cpp_int_t;
        cpp_int int_var__hrst;
        void hrst (const cpp_int  & l__val);
        cpp_int hrst() const;
    
        typedef pu_cpp_int< 1 > rst_override_cpp_int_t;
        cpp_int int_var__rst_override;
        void rst_override (const cpp_int  & l__val);
        cpp_int rst_override() const;
    
}; // cap_mss_csr_cfg_mod_bx_t
    
class cap_mss_csr_cfg_mod_mx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mss_csr_cfg_mod_mx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mss_csr_cfg_mod_mx_t(string name = "cap_mss_csr_cfg_mod_mx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mss_csr_cfg_mod_mx_t();
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
    
        typedef pu_cpp_int< 1 > srst_cpp_int_t;
        cpp_int int_var__srst;
        void srst (const cpp_int  & l__val);
        cpp_int srst() const;
    
        typedef pu_cpp_int< 1 > hrst_cpp_int_t;
        cpp_int int_var__hrst;
        void hrst (const cpp_int  & l__val);
        cpp_int hrst() const;
    
        typedef pu_cpp_int< 1 > rst_override_cpp_int_t;
        cpp_int int_var__rst_override;
        void rst_override (const cpp_int  & l__val);
        cpp_int rst_override() const;
    
}; // cap_mss_csr_cfg_mod_mx_t
    
class cap_mss_csr_cfg_mod_dp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mss_csr_cfg_mod_dp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mss_csr_cfg_mod_dp_t(string name = "cap_mss_csr_cfg_mod_dp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mss_csr_cfg_mod_dp_t();
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
    
        typedef pu_cpp_int< 1 > srst_cpp_int_t;
        cpp_int int_var__srst;
        void srst (const cpp_int  & l__val);
        cpp_int srst() const;
    
        typedef pu_cpp_int< 1 > hrst_cpp_int_t;
        cpp_int int_var__hrst;
        void hrst (const cpp_int  & l__val);
        cpp_int hrst() const;
    
        typedef pu_cpp_int< 1 > rst_override_cpp_int_t;
        cpp_int int_var__rst_override;
        void rst_override (const cpp_int  & l__val);
        cpp_int rst_override() const;
    
}; // cap_mss_csr_cfg_mod_dp_t
    
class cap_mss_csr_cfg_mod_pa_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mss_csr_cfg_mod_pa_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mss_csr_cfg_mod_pa_t(string name = "cap_mss_csr_cfg_mod_pa_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mss_csr_cfg_mod_pa_t();
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
    
        typedef pu_cpp_int< 1 > srst_cpp_int_t;
        cpp_int int_var__srst;
        void srst (const cpp_int  & l__val);
        cpp_int srst() const;
    
        typedef pu_cpp_int< 1 > hrst_cpp_int_t;
        cpp_int int_var__hrst;
        void hrst (const cpp_int  & l__val);
        cpp_int hrst() const;
    
        typedef pu_cpp_int< 1 > rst_override_cpp_int_t;
        cpp_int int_var__rst_override;
        void rst_override (const cpp_int  & l__val);
        cpp_int rst_override() const;
    
}; // cap_mss_csr_cfg_mod_pa_t
    
class cap_mss_csr_cfg_mod_pm_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mss_csr_cfg_mod_pm_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mss_csr_cfg_mod_pm_t(string name = "cap_mss_csr_cfg_mod_pm_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mss_csr_cfg_mod_pm_t();
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
    
        typedef pu_cpp_int< 1 > srst_cpp_int_t;
        cpp_int int_var__srst;
        void srst (const cpp_int  & l__val);
        cpp_int srst() const;
    
        typedef pu_cpp_int< 1 > hrst_cpp_int_t;
        cpp_int int_var__hrst;
        void hrst (const cpp_int  & l__val);
        cpp_int hrst() const;
    
        typedef pu_cpp_int< 1 > rst_override_cpp_int_t;
        cpp_int int_var__rst_override;
        void rst_override (const cpp_int  & l__val);
        cpp_int rst_override() const;
    
}; // cap_mss_csr_cfg_mod_pm_t
    
class cap_mss_csr_cfg_mod_pb_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mss_csr_cfg_mod_pb_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mss_csr_cfg_mod_pb_t(string name = "cap_mss_csr_cfg_mod_pb_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mss_csr_cfg_mod_pb_t();
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
    
        typedef pu_cpp_int< 1 > srst_cpp_int_t;
        cpp_int int_var__srst;
        void srst (const cpp_int  & l__val);
        cpp_int srst() const;
    
        typedef pu_cpp_int< 1 > hrst_cpp_int_t;
        cpp_int int_var__hrst;
        void hrst (const cpp_int  & l__val);
        cpp_int hrst() const;
    
        typedef pu_cpp_int< 1 > rst_override_cpp_int_t;
        cpp_int int_var__rst_override;
        void rst_override (const cpp_int  & l__val);
        cpp_int rst_override() const;
    
}; // cap_mss_csr_cfg_mod_pb_t
    
class cap_mss_csr_cfg_mod_pr_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mss_csr_cfg_mod_pr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mss_csr_cfg_mod_pr_t(string name = "cap_mss_csr_cfg_mod_pr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mss_csr_cfg_mod_pr_t();
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
    
        typedef pu_cpp_int< 1 > srst_cpp_int_t;
        cpp_int int_var__srst;
        void srst (const cpp_int  & l__val);
        cpp_int srst() const;
    
        typedef pu_cpp_int< 1 > hrst_cpp_int_t;
        cpp_int int_var__hrst;
        void hrst (const cpp_int  & l__val);
        cpp_int hrst() const;
    
        typedef pu_cpp_int< 1 > rst_override_cpp_int_t;
        cpp_int int_var__rst_override;
        void rst_override (const cpp_int  & l__val);
        cpp_int rst_override() const;
    
}; // cap_mss_csr_cfg_mod_pr_t
    
class cap_mss_csr_cfg_mod_pt_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mss_csr_cfg_mod_pt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mss_csr_cfg_mod_pt_t(string name = "cap_mss_csr_cfg_mod_pt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mss_csr_cfg_mod_pt_t();
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
    
        typedef pu_cpp_int< 1 > srst_cpp_int_t;
        cpp_int int_var__srst;
        void srst (const cpp_int  & l__val);
        cpp_int srst() const;
    
        typedef pu_cpp_int< 1 > hrst_cpp_int_t;
        cpp_int int_var__hrst;
        void hrst (const cpp_int  & l__val);
        cpp_int hrst() const;
    
        typedef pu_cpp_int< 1 > rst_override_cpp_int_t;
        cpp_int int_var__rst_override;
        void rst_override (const cpp_int  & l__val);
        cpp_int rst_override() const;
    
}; // cap_mss_csr_cfg_mod_pt_t
    
class cap_mss_csr_cfg_mod_pg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mss_csr_cfg_mod_pg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mss_csr_cfg_mod_pg_t(string name = "cap_mss_csr_cfg_mod_pg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mss_csr_cfg_mod_pg_t();
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
    
        typedef pu_cpp_int< 1 > srst_cpp_int_t;
        cpp_int int_var__srst;
        void srst (const cpp_int  & l__val);
        cpp_int srst() const;
    
        typedef pu_cpp_int< 1 > hrst_cpp_int_t;
        cpp_int int_var__hrst;
        void hrst (const cpp_int  & l__val);
        cpp_int hrst() const;
    
        typedef pu_cpp_int< 1 > rst_override_cpp_int_t;
        cpp_int int_var__rst_override;
        void rst_override (const cpp_int  & l__val);
        cpp_int rst_override() const;
    
}; // cap_mss_csr_cfg_mod_pg_t
    
class cap_mss_csr_cfg_mod_sc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mss_csr_cfg_mod_sc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mss_csr_cfg_mod_sc_t(string name = "cap_mss_csr_cfg_mod_sc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mss_csr_cfg_mod_sc_t();
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
    
        typedef pu_cpp_int< 1 > srst_cpp_int_t;
        cpp_int int_var__srst;
        void srst (const cpp_int  & l__val);
        cpp_int srst() const;
    
        typedef pu_cpp_int< 1 > hrst_cpp_int_t;
        cpp_int int_var__hrst;
        void hrst (const cpp_int  & l__val);
        cpp_int hrst() const;
    
        typedef pu_cpp_int< 1 > rst_override_cpp_int_t;
        cpp_int int_var__rst_override;
        void rst_override (const cpp_int  & l__val);
        cpp_int rst_override() const;
    
}; // cap_mss_csr_cfg_mod_sc_t
    
class cap_mss_csr_cfg_mod_pc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mss_csr_cfg_mod_pc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mss_csr_cfg_mod_pc_t(string name = "cap_mss_csr_cfg_mod_pc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mss_csr_cfg_mod_pc_t();
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
    
        typedef pu_cpp_int< 1 > srst_cpp_int_t;
        cpp_int int_var__srst;
        void srst (const cpp_int  & l__val);
        cpp_int srst() const;
    
        typedef pu_cpp_int< 1 > hrst_cpp_int_t;
        cpp_int int_var__hrst;
        void hrst (const cpp_int  & l__val);
        cpp_int hrst() const;
    
        typedef pu_cpp_int< 1 > rst_override_cpp_int_t;
        cpp_int int_var__rst_override;
        void rst_override (const cpp_int  & l__val);
        cpp_int rst_override() const;
    
}; // cap_mss_csr_cfg_mod_pc_t
    
class cap_mss_csr_cfg_mod_sg_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mss_csr_cfg_mod_sg_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mss_csr_cfg_mod_sg_t(string name = "cap_mss_csr_cfg_mod_sg_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mss_csr_cfg_mod_sg_t();
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
    
        typedef pu_cpp_int< 1 > srst_cpp_int_t;
        cpp_int int_var__srst;
        void srst (const cpp_int  & l__val);
        cpp_int srst() const;
    
        typedef pu_cpp_int< 1 > hrst_cpp_int_t;
        cpp_int int_var__hrst;
        void hrst (const cpp_int  & l__val);
        cpp_int hrst() const;
    
        typedef pu_cpp_int< 1 > rst_override_cpp_int_t;
        cpp_int int_var__rst_override;
        void rst_override (const cpp_int  & l__val);
        cpp_int rst_override() const;
    
}; // cap_mss_csr_cfg_mod_sg_t
    
class cap_mss_csr_cfg_mod_nx_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mss_csr_cfg_mod_nx_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mss_csr_cfg_mod_nx_t(string name = "cap_mss_csr_cfg_mod_nx_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mss_csr_cfg_mod_nx_t();
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
    
        typedef pu_cpp_int< 1 > srst_cpp_int_t;
        cpp_int int_var__srst;
        void srst (const cpp_int  & l__val);
        cpp_int srst() const;
    
        typedef pu_cpp_int< 1 > hrst_cpp_int_t;
        cpp_int int_var__hrst;
        void hrst (const cpp_int  & l__val);
        cpp_int hrst() const;
    
        typedef pu_cpp_int< 1 > rst_override_cpp_int_t;
        cpp_int int_var__rst_override;
        void rst_override (const cpp_int  & l__val);
        cpp_int rst_override() const;
    
}; // cap_mss_csr_cfg_mod_nx_t
    
class cap_mss_csr_cfg_mod_he_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mss_csr_cfg_mod_he_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mss_csr_cfg_mod_he_t(string name = "cap_mss_csr_cfg_mod_he_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mss_csr_cfg_mod_he_t();
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
    
        typedef pu_cpp_int< 1 > srst_cpp_int_t;
        cpp_int int_var__srst;
        void srst (const cpp_int  & l__val);
        cpp_int srst() const;
    
        typedef pu_cpp_int< 1 > hrst_cpp_int_t;
        cpp_int int_var__hrst;
        void hrst (const cpp_int  & l__val);
        cpp_int hrst() const;
    
        typedef pu_cpp_int< 1 > rst_override_cpp_int_t;
        cpp_int int_var__rst_override;
        void rst_override (const cpp_int  & l__val);
        cpp_int rst_override() const;
    
}; // cap_mss_csr_cfg_mod_he_t
    
class cap_mss_csr_cfg_mod_md_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mss_csr_cfg_mod_md_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mss_csr_cfg_mod_md_t(string name = "cap_mss_csr_cfg_mod_md_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mss_csr_cfg_mod_md_t();
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
    
        typedef pu_cpp_int< 1 > srst_cpp_int_t;
        cpp_int int_var__srst;
        void srst (const cpp_int  & l__val);
        cpp_int srst() const;
    
        typedef pu_cpp_int< 1 > hrst_cpp_int_t;
        cpp_int int_var__hrst;
        void hrst (const cpp_int  & l__val);
        cpp_int hrst() const;
    
        typedef pu_cpp_int< 1 > rst_override_cpp_int_t;
        cpp_int int_var__rst_override;
        void rst_override (const cpp_int  & l__val);
        cpp_int rst_override() const;
    
}; // cap_mss_csr_cfg_mod_md_t
    
class cap_mss_csr_cfg_mod_mc_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mss_csr_cfg_mod_mc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mss_csr_cfg_mod_mc_t(string name = "cap_mss_csr_cfg_mod_mc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mss_csr_cfg_mod_mc_t();
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
    
        typedef pu_cpp_int< 1 > srst_cpp_int_t;
        cpp_int int_var__srst;
        void srst (const cpp_int  & l__val);
        cpp_int srst() const;
    
        typedef pu_cpp_int< 1 > hrst_cpp_int_t;
        cpp_int int_var__hrst;
        void hrst (const cpp_int  & l__val);
        cpp_int hrst() const;
    
        typedef pu_cpp_int< 1 > rst_override_cpp_int_t;
        cpp_int int_var__rst_override;
        void rst_override (const cpp_int  & l__val);
        cpp_int rst_override() const;
    
}; // cap_mss_csr_cfg_mod_mc_t
    
class cap_mss_csr_cfg_mod_ap_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mss_csr_cfg_mod_ap_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mss_csr_cfg_mod_ap_t(string name = "cap_mss_csr_cfg_mod_ap_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mss_csr_cfg_mod_ap_t();
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
    
        typedef pu_cpp_int< 1 > srst_cpp_int_t;
        cpp_int int_var__srst;
        void srst (const cpp_int  & l__val);
        cpp_int srst() const;
    
        typedef pu_cpp_int< 1 > hrst_cpp_int_t;
        cpp_int int_var__hrst;
        void hrst (const cpp_int  & l__val);
        cpp_int hrst() const;
    
        typedef pu_cpp_int< 1 > rst_override_cpp_int_t;
        cpp_int int_var__rst_override;
        void rst_override (const cpp_int  & l__val);
        cpp_int rst_override() const;
    
}; // cap_mss_csr_cfg_mod_ap_t
    
class cap_mss_csr_cfg_mod_ar_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mss_csr_cfg_mod_ar_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mss_csr_cfg_mod_ar_t(string name = "cap_mss_csr_cfg_mod_ar_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mss_csr_cfg_mod_ar_t();
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
    
        typedef pu_cpp_int< 1 > srst_cpp_int_t;
        cpp_int int_var__srst;
        void srst (const cpp_int  & l__val);
        cpp_int srst() const;
    
        typedef pu_cpp_int< 1 > hrst_cpp_int_t;
        cpp_int int_var__hrst;
        void hrst (const cpp_int  & l__val);
        cpp_int hrst() const;
    
        typedef pu_cpp_int< 1 > rst_override_cpp_int_t;
        cpp_int int_var__rst_override;
        void rst_override (const cpp_int  & l__val);
        cpp_int rst_override() const;
    
}; // cap_mss_csr_cfg_mod_ar_t
    
class cap_mss_csr_cfg_mod_px_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mss_csr_cfg_mod_px_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mss_csr_cfg_mod_px_t(string name = "cap_mss_csr_cfg_mod_px_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mss_csr_cfg_mod_px_t();
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
    
        typedef pu_cpp_int< 1 > srst_cpp_int_t;
        cpp_int int_var__srst;
        void srst (const cpp_int  & l__val);
        cpp_int srst() const;
    
        typedef pu_cpp_int< 1 > hrst_cpp_int_t;
        cpp_int int_var__hrst;
        void hrst (const cpp_int  & l__val);
        cpp_int hrst() const;
    
        typedef pu_cpp_int< 1 > rst_override_cpp_int_t;
        cpp_int int_var__rst_override;
        void rst_override (const cpp_int  & l__val);
        cpp_int rst_override() const;
    
}; // cap_mss_csr_cfg_mod_px_t
    
class cap_mss_csr_cfg_mod_pp_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mss_csr_cfg_mod_pp_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mss_csr_cfg_mod_pp_t(string name = "cap_mss_csr_cfg_mod_pp_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mss_csr_cfg_mod_pp_t();
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
    
        typedef pu_cpp_int< 1 > srst_cpp_int_t;
        cpp_int int_var__srst;
        void srst (const cpp_int  & l__val);
        cpp_int srst() const;
    
        typedef pu_cpp_int< 1 > hrst_cpp_int_t;
        cpp_int int_var__hrst;
        void hrst (const cpp_int  & l__val);
        cpp_int hrst() const;
    
        typedef pu_cpp_int< 1 > rst_override_cpp_int_t;
        cpp_int int_var__rst_override;
        void rst_override (const cpp_int  & l__val);
        cpp_int rst_override() const;
    
}; // cap_mss_csr_cfg_mod_pp_t
    
class cap_mss_csr_cfg_periph_rst_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mss_csr_cfg_periph_rst_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mss_csr_cfg_periph_rst_t(string name = "cap_mss_csr_cfg_periph_rst_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mss_csr_cfg_periph_rst_t();
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
    
        typedef pu_cpp_int< 1 > hw_qspi_cpp_int_t;
        cpp_int int_var__hw_qspi;
        void hw_qspi (const cpp_int  & l__val);
        cpp_int hw_qspi() const;
    
        typedef pu_cpp_int< 1 > hw_gpio_cpp_int_t;
        cpp_int int_var__hw_gpio;
        void hw_gpio (const cpp_int  & l__val);
        cpp_int hw_gpio() const;
    
        typedef pu_cpp_int< 1 > hw_wdt0_cpp_int_t;
        cpp_int int_var__hw_wdt0;
        void hw_wdt0 (const cpp_int  & l__val);
        cpp_int hw_wdt0() const;
    
        typedef pu_cpp_int< 1 > hw_wdt1_cpp_int_t;
        cpp_int int_var__hw_wdt1;
        void hw_wdt1 (const cpp_int  & l__val);
        cpp_int hw_wdt1() const;
    
        typedef pu_cpp_int< 1 > hw_wdt2_cpp_int_t;
        cpp_int int_var__hw_wdt2;
        void hw_wdt2 (const cpp_int  & l__val);
        cpp_int hw_wdt2() const;
    
        typedef pu_cpp_int< 1 > hw_wdt3_cpp_int_t;
        cpp_int int_var__hw_wdt3;
        void hw_wdt3 (const cpp_int  & l__val);
        cpp_int hw_wdt3() const;
    
        typedef pu_cpp_int< 1 > hw_i2c0_cpp_int_t;
        cpp_int int_var__hw_i2c0;
        void hw_i2c0 (const cpp_int  & l__val);
        cpp_int hw_i2c0() const;
    
        typedef pu_cpp_int< 1 > hw_i2c1_cpp_int_t;
        cpp_int int_var__hw_i2c1;
        void hw_i2c1 (const cpp_int  & l__val);
        cpp_int hw_i2c1() const;
    
        typedef pu_cpp_int< 1 > hw_i2c2_cpp_int_t;
        cpp_int int_var__hw_i2c2;
        void hw_i2c2 (const cpp_int  & l__val);
        cpp_int hw_i2c2() const;
    
        typedef pu_cpp_int< 1 > hw_i2c3_cpp_int_t;
        cpp_int int_var__hw_i2c3;
        void hw_i2c3 (const cpp_int  & l__val);
        cpp_int hw_i2c3() const;
    
        typedef pu_cpp_int< 1 > hw_uart0_cpp_int_t;
        cpp_int int_var__hw_uart0;
        void hw_uart0 (const cpp_int  & l__val);
        cpp_int hw_uart0() const;
    
        typedef pu_cpp_int< 1 > hw_uart1_cpp_int_t;
        cpp_int int_var__hw_uart1;
        void hw_uart1 (const cpp_int  & l__val);
        cpp_int hw_uart1() const;
    
        typedef pu_cpp_int< 1 > hw_ssi0_cpp_int_t;
        cpp_int int_var__hw_ssi0;
        void hw_ssi0 (const cpp_int  & l__val);
        cpp_int hw_ssi0() const;
    
        typedef pu_cpp_int< 1 > hw_ssi1_cpp_int_t;
        cpp_int int_var__hw_ssi1;
        void hw_ssi1 (const cpp_int  & l__val);
        cpp_int hw_ssi1() const;
    
}; // cap_mss_csr_cfg_periph_rst_t
    
class cap_mss_csr_base_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mss_csr_base_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mss_csr_base_t(string name = "cap_mss_csr_base_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mss_csr_base_t();
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
    
}; // cap_mss_csr_base_t
    
class cap_mss_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_mss_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_mss_csr_t(string name = "cap_mss_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_mss_csr_t();
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
    
        cap_mss_csr_base_t base;
    
        cap_mss_csr_cfg_periph_rst_t cfg_periph_rst;
    
        cap_mss_csr_cfg_mod_pp_t cfg_mod_pp;
    
        cap_mss_csr_cfg_mod_px_t cfg_mod_px;
    
        cap_mss_csr_cfg_mod_ar_t cfg_mod_ar;
    
        cap_mss_csr_cfg_mod_ap_t cfg_mod_ap;
    
        cap_mss_csr_cfg_mod_mc_t cfg_mod_mc;
    
        cap_mss_csr_cfg_mod_md_t cfg_mod_md;
    
        cap_mss_csr_cfg_mod_he_t cfg_mod_he;
    
        cap_mss_csr_cfg_mod_nx_t cfg_mod_nx;
    
        cap_mss_csr_cfg_mod_sg_t cfg_mod_sg;
    
        cap_mss_csr_cfg_mod_pc_t cfg_mod_pc;
    
        cap_mss_csr_cfg_mod_sc_t cfg_mod_sc;
    
        cap_mss_csr_cfg_mod_pg_t cfg_mod_pg;
    
        cap_mss_csr_cfg_mod_pt_t cfg_mod_pt;
    
        cap_mss_csr_cfg_mod_pr_t cfg_mod_pr;
    
        cap_mss_csr_cfg_mod_pb_t cfg_mod_pb;
    
        cap_mss_csr_cfg_mod_pm_t cfg_mod_pm;
    
        cap_mss_csr_cfg_mod_pa_t cfg_mod_pa;
    
        cap_mss_csr_cfg_mod_dp_t cfg_mod_dp;
    
        cap_mss_csr_cfg_mod_mx_t cfg_mod_mx;
    
        cap_mss_csr_cfg_mod_bx_t cfg_mod_bx;
    
        cap_mss_csr_cfg_mod_ms_t cfg_mod_ms;
    
        cap_mss_csr_cfg_mod_em_t cfg_mod_em;
    
        cap_mss_csr_cfg_mod_tx_t cfg_mod_tx;
    
}; // cap_mss_csr_t
    
#endif // CAP_MSS_CSR_H
        