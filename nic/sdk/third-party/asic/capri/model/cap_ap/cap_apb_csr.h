
#ifndef CAP_APB_CSR_H
#define CAP_APB_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_apb_csr_dhs_bypass_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_apb_csr_dhs_bypass_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_apb_csr_dhs_bypass_entry_t(string name = "cap_apb_csr_dhs_bypass_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_apb_csr_dhs_bypass_entry_t();
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
    
}; // cap_apb_csr_dhs_bypass_entry_t
    
class cap_apb_csr_dhs_bypass_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_apb_csr_dhs_bypass_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_apb_csr_dhs_bypass_t(string name = "cap_apb_csr_dhs_bypass_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_apb_csr_dhs_bypass_t();
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
    
        #if 65536 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_apb_csr_dhs_bypass_entry_t, 65536> entry;
        #else 
        cap_apb_csr_dhs_bypass_entry_t entry[65536];
        #endif
        int get_depth_entry() { return 65536; }
    
}; // cap_apb_csr_dhs_bypass_t
    
class cap_apb_csr_dhs_psys_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_apb_csr_dhs_psys_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_apb_csr_dhs_psys_entry_t(string name = "cap_apb_csr_dhs_psys_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_apb_csr_dhs_psys_entry_t();
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
    
}; // cap_apb_csr_dhs_psys_entry_t
    
class cap_apb_csr_dhs_psys_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_apb_csr_dhs_psys_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_apb_csr_dhs_psys_t(string name = "cap_apb_csr_dhs_psys_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_apb_csr_dhs_psys_t();
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
    
        #if 65536 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_apb_csr_dhs_psys_entry_t, 65536> entry;
        #else 
        cap_apb_csr_dhs_psys_entry_t entry[65536];
        #endif
        int get_depth_entry() { return 65536; }
    
}; // cap_apb_csr_dhs_psys_t
    
class cap_apb_csr_cfg_dap_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_apb_csr_cfg_dap_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_apb_csr_cfg_dap_t(string name = "cap_apb_csr_cfg_dap_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_apb_csr_cfg_dap_t();
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
    
        typedef pu_cpp_int< 1 > PCLKENSYS_cpp_int_t;
        cpp_int int_var__PCLKENSYS;
        void PCLKENSYS (const cpp_int  & l__val);
        cpp_int PCLKENSYS() const;
    
        typedef pu_cpp_int< 1 > PRESETSYSn_cpp_int_t;
        cpp_int int_var__PRESETSYSn;
        void PRESETSYSn (const cpp_int  & l__val);
        cpp_int PRESETSYSn() const;
    
        typedef pu_cpp_int< 1 > PRESETDBGn_cpp_int_t;
        cpp_int int_var__PRESETDBGn;
        void PRESETDBGn (const cpp_int  & l__val);
        cpp_int PRESETDBGn() const;
    
        typedef pu_cpp_int< 1 > CDBGPWRUPACK_cpp_int_t;
        cpp_int int_var__CDBGPWRUPACK;
        void CDBGPWRUPACK (const cpp_int  & l__val);
        cpp_int CDBGPWRUPACK() const;
    
        typedef pu_cpp_int< 1 > CSYSPWRUPACK_cpp_int_t;
        cpp_int int_var__CSYSPWRUPACK;
        void CSYSPWRUPACK (const cpp_int  & l__val);
        cpp_int CSYSPWRUPACK() const;
    
        typedef pu_cpp_int< 1 > CDBGRSTACK_cpp_int_t;
        cpp_int int_var__CDBGRSTACK;
        void CDBGRSTACK (const cpp_int  & l__val);
        cpp_int CDBGRSTACK() const;
    
        typedef pu_cpp_int< 1 > nCDBGPWRDN_cpp_int_t;
        cpp_int int_var__nCDBGPWRDN;
        void nCDBGPWRDN (const cpp_int  & l__val);
        cpp_int nCDBGPWRDN() const;
    
        typedef pu_cpp_int< 1 > nCSOCPWRDN_cpp_int_t;
        cpp_int int_var__nCSOCPWRDN;
        void nCSOCPWRDN (const cpp_int  & l__val);
        cpp_int nCSOCPWRDN() const;
    
        typedef pu_cpp_int< 1 > DEVICEEN_cpp_int_t;
        cpp_int int_var__DEVICEEN;
        void DEVICEEN (const cpp_int  & l__val);
        cpp_int DEVICEEN() const;
    
        typedef pu_cpp_int< 14 > psys_paddr_indir_cpp_int_t;
        cpp_int int_var__psys_paddr_indir;
        void psys_paddr_indir (const cpp_int  & l__val);
        cpp_int psys_paddr_indir() const;
    
        typedef pu_cpp_int< 1 > PCLKENDBG_cpp_int_t;
        cpp_int int_var__PCLKENDBG;
        void PCLKENDBG (const cpp_int  & l__val);
        cpp_int PCLKENDBG() const;
    
        typedef pu_cpp_int< 1 > nPOTRST_cpp_int_t;
        cpp_int int_var__nPOTRST;
        void nPOTRST (const cpp_int  & l__val);
        cpp_int nPOTRST() const;
    
        typedef pu_cpp_int< 32 > PADDRDBG_mask_cpp_int_t;
        cpp_int int_var__PADDRDBG_mask;
        void PADDRDBG_mask (const cpp_int  & l__val);
        cpp_int PADDRDBG_mask() const;
    
        typedef pu_cpp_int< 32 > PADDRDBG_or_cpp_int_t;
        cpp_int int_var__PADDRDBG_or;
        void PADDRDBG_or (const cpp_int  & l__val);
        cpp_int PADDRDBG_or() const;
    
        typedef pu_cpp_int< 1 > bypass_debug_cpp_int_t;
        cpp_int int_var__bypass_debug;
        void bypass_debug (const cpp_int  & l__val);
        cpp_int bypass_debug() const;
    
}; // cap_apb_csr_cfg_dap_t
    
class cap_apb_csr_sta_dap_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_apb_csr_sta_dap_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_apb_csr_sta_dap_t(string name = "cap_apb_csr_sta_dap_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_apb_csr_sta_dap_t();
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
    
        typedef pu_cpp_int< 1 > CDBGPWRUPREQ_cpp_int_t;
        cpp_int int_var__CDBGPWRUPREQ;
        void CDBGPWRUPREQ (const cpp_int  & l__val);
        cpp_int CDBGPWRUPREQ() const;
    
        typedef pu_cpp_int< 1 > CDBGRSTREQ_cpp_int_t;
        cpp_int int_var__CDBGRSTREQ;
        void CDBGRSTREQ (const cpp_int  & l__val);
        cpp_int CDBGRSTREQ() const;
    
        typedef pu_cpp_int< 1 > CSYSPWRUPREQ_cpp_int_t;
        cpp_int int_var__CSYSPWRUPREQ;
        void CSYSPWRUPREQ (const cpp_int  & l__val);
        cpp_int CSYSPWRUPREQ() const;
    
        typedef pu_cpp_int< 1 > DBGSWENABLE_cpp_int_t;
        cpp_int int_var__DBGSWENABLE;
        void DBGSWENABLE (const cpp_int  & l__val);
        cpp_int DBGSWENABLE() const;
    
        typedef pu_cpp_int< 1 > JTAGNSW_cpp_int_t;
        cpp_int int_var__JTAGNSW;
        void JTAGNSW (const cpp_int  & l__val);
        cpp_int JTAGNSW() const;
    
        typedef pu_cpp_int< 1 > JTAGTOP_cpp_int_t;
        cpp_int int_var__JTAGTOP;
        void JTAGTOP (const cpp_int  & l__val);
        cpp_int JTAGTOP() const;
    
        typedef pu_cpp_int< 1 > PSLVERRSYS_cpp_int_t;
        cpp_int int_var__PSLVERRSYS;
        void PSLVERRSYS (const cpp_int  & l__val);
        cpp_int PSLVERRSYS() const;
    
        typedef pu_cpp_int< 1 > PSLVERRDBG_cpp_int_t;
        cpp_int int_var__PSLVERRDBG;
        void PSLVERRDBG (const cpp_int  & l__val);
        cpp_int PSLVERRDBG() const;
    
}; // cap_apb_csr_sta_dap_t
    
class cap_apb_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_apb_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_apb_csr_t(string name = "cap_apb_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_apb_csr_t();
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
    
        cap_apb_csr_sta_dap_t sta_dap;
    
        cap_apb_csr_cfg_dap_t cfg_dap;
    
        cap_apb_csr_dhs_psys_t dhs_psys;
    
        cap_apb_csr_dhs_bypass_t dhs_bypass;
    
}; // cap_apb_csr_t
    
#endif // CAP_APB_CSR_H
        