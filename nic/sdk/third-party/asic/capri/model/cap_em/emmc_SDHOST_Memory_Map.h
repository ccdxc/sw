
#ifndef EMMC_SDHOST_MEMORY_MAP_H
#define EMMC_SDHOST_MEMORY_MAP_H

#include "cap_csr_base.h" 

using namespace std;
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_sdmasysaddrlo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_sdmasysaddrlo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_sdmasysaddrlo_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_sdmasysaddrlo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_sdmasysaddrlo_t();
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
    
        typedef pu_cpp_int< 16 > sdma_sysaddress_cpp_int_t;
        cpp_int int_var__sdma_sysaddress;
        void sdma_sysaddress (const cpp_int  & l__val);
        cpp_int sdma_sysaddress() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_sdmasysaddrlo_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_sdmasysaddrhi_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_sdmasysaddrhi_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_sdmasysaddrhi_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_sdmasysaddrhi_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_sdmasysaddrhi_t();
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
    
        typedef pu_cpp_int< 16 > sdma_sysaddress_cpp_int_t;
        cpp_int int_var__sdma_sysaddress;
        void sdma_sysaddress (const cpp_int  & l__val);
        cpp_int sdma_sysaddress() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_sdmasysaddrhi_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_blocksize_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_blocksize_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_blocksize_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_blocksize_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_blocksize_t();
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
    
        typedef pu_cpp_int< 12 > xfer_blocksize_cpp_int_t;
        cpp_int int_var__xfer_blocksize;
        void xfer_blocksize (const cpp_int  & l__val);
        cpp_int xfer_blocksize() const;
    
        typedef pu_cpp_int< 3 > sdma_bufboundary_cpp_int_t;
        cpp_int int_var__sdma_bufboundary;
        void sdma_bufboundary (const cpp_int  & l__val);
        cpp_int sdma_bufboundary() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_blocksize_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_blockcount_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_blockcount_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_blockcount_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_blockcount_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_blockcount_t();
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
    
        typedef pu_cpp_int< 16 > xfer_blockcount_cpp_int_t;
        cpp_int int_var__xfer_blockcount;
        void xfer_blockcount (const cpp_int  & l__val);
        cpp_int xfer_blockcount() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_blockcount_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_argument1lo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_argument1lo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_argument1lo_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_argument1lo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_argument1lo_t();
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
    
        typedef pu_cpp_int< 16 > command_argument1_cpp_int_t;
        cpp_int int_var__command_argument1;
        void command_argument1 (const cpp_int  & l__val);
        cpp_int command_argument1() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_argument1lo_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_argument1hi_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_argument1hi_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_argument1hi_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_argument1hi_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_argument1hi_t();
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
    
        typedef pu_cpp_int< 16 > command_argument1_cpp_int_t;
        cpp_int int_var__command_argument1;
        void command_argument1 (const cpp_int  & l__val);
        cpp_int command_argument1() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_argument1hi_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_transfermode_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_transfermode_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_transfermode_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_transfermode_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_transfermode_t();
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
    
        typedef pu_cpp_int< 1 > xfermode_dmaenable_cpp_int_t;
        cpp_int int_var__xfermode_dmaenable;
        void xfermode_dmaenable (const cpp_int  & l__val);
        cpp_int xfermode_dmaenable() const;
    
        typedef pu_cpp_int< 1 > xfermode_blkcntena_cpp_int_t;
        cpp_int int_var__xfermode_blkcntena;
        void xfermode_blkcntena (const cpp_int  & l__val);
        cpp_int xfermode_blkcntena() const;
    
        typedef pu_cpp_int< 2 > xfermode_autocmdena_cpp_int_t;
        cpp_int int_var__xfermode_autocmdena;
        void xfermode_autocmdena (const cpp_int  & l__val);
        cpp_int xfermode_autocmdena() const;
    
        typedef pu_cpp_int< 1 > xfermode_dataxferdir_cpp_int_t;
        cpp_int int_var__xfermode_dataxferdir;
        void xfermode_dataxferdir (const cpp_int  & l__val);
        cpp_int xfermode_dataxferdir() const;
    
        typedef pu_cpp_int< 1 > xfermode_multiblksel_cpp_int_t;
        cpp_int int_var__xfermode_multiblksel;
        void xfermode_multiblksel (const cpp_int  & l__val);
        cpp_int xfermode_multiblksel() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_transfermode_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_command_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_command_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_command_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_command_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_command_t();
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
    
        typedef pu_cpp_int< 2 > command_responsetype_cpp_int_t;
        cpp_int int_var__command_responsetype;
        void command_responsetype (const cpp_int  & l__val);
        cpp_int command_responsetype() const;
    
        typedef pu_cpp_int< 1 > rsvd_2_2_cpp_int_t;
        cpp_int int_var__rsvd_2_2;
        void rsvd_2_2 (const cpp_int  & l__val);
        cpp_int rsvd_2_2() const;
    
        typedef pu_cpp_int< 1 > command_crcchkena_cpp_int_t;
        cpp_int int_var__command_crcchkena;
        void command_crcchkena (const cpp_int  & l__val);
        cpp_int command_crcchkena() const;
    
        typedef pu_cpp_int< 1 > command_indexchkena_cpp_int_t;
        cpp_int int_var__command_indexchkena;
        void command_indexchkena (const cpp_int  & l__val);
        cpp_int command_indexchkena() const;
    
        typedef pu_cpp_int< 1 > command_datapresent_cpp_int_t;
        cpp_int int_var__command_datapresent;
        void command_datapresent (const cpp_int  & l__val);
        cpp_int command_datapresent() const;
    
        typedef pu_cpp_int< 2 > command_cmdtype_cpp_int_t;
        cpp_int int_var__command_cmdtype;
        void command_cmdtype (const cpp_int  & l__val);
        cpp_int command_cmdtype() const;
    
        typedef pu_cpp_int< 6 > command_cmdindex_cpp_int_t;
        cpp_int int_var__command_cmdindex;
        void command_cmdindex (const cpp_int  & l__val);
        cpp_int command_cmdindex() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_command_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response0_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response0_t();
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
    
        typedef pu_cpp_int< 16 > command_response_cpp_int_t;
        cpp_int int_var__command_response;
        void command_response (const cpp_int  & l__val);
        cpp_int command_response() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response0_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response1_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response1_t();
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
    
        typedef pu_cpp_int< 16 > command_response_cpp_int_t;
        cpp_int int_var__command_response;
        void command_response (const cpp_int  & l__val);
        cpp_int command_response() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response1_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response2_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response2_t();
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
    
        typedef pu_cpp_int< 16 > command_response_cpp_int_t;
        cpp_int int_var__command_response;
        void command_response (const cpp_int  & l__val);
        cpp_int command_response() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response2_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response3_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response3_t();
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
    
        typedef pu_cpp_int< 16 > command_response_cpp_int_t;
        cpp_int int_var__command_response;
        void command_response (const cpp_int  & l__val);
        cpp_int command_response() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response3_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response4_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response4_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response4_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response4_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response4_t();
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
    
        typedef pu_cpp_int< 16 > command_response_cpp_int_t;
        cpp_int int_var__command_response;
        void command_response (const cpp_int  & l__val);
        cpp_int command_response() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response4_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response5_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response5_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response5_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response5_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response5_t();
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
    
        typedef pu_cpp_int< 16 > command_response_cpp_int_t;
        cpp_int int_var__command_response;
        void command_response (const cpp_int  & l__val);
        cpp_int command_response() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response5_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response6_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response6_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response6_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response6_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response6_t();
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
    
        typedef pu_cpp_int< 16 > command_response_cpp_int_t;
        cpp_int int_var__command_response;
        void command_response (const cpp_int  & l__val);
        cpp_int command_response() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response6_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response7_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response7_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response7_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response7_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response7_t();
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
    
        typedef pu_cpp_int< 16 > command_response_cpp_int_t;
        cpp_int int_var__command_response;
        void command_response (const cpp_int  & l__val);
        cpp_int command_response() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response7_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_dataport_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_dataport_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_dataport_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_dataport_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_dataport_t();
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
    
        typedef pu_cpp_int< 32 > sdhcdmactrl_piobufrddata_cpp_int_t;
        cpp_int int_var__sdhcdmactrl_piobufrddata;
        void sdhcdmactrl_piobufrddata (const cpp_int  & l__val);
        cpp_int sdhcdmactrl_piobufrddata() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_dataport_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presentstate_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presentstate_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presentstate_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presentstate_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presentstate_t();
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
    
        typedef pu_cpp_int< 1 > presentstate_inhibitcmd_cpp_int_t;
        cpp_int int_var__presentstate_inhibitcmd;
        void presentstate_inhibitcmd (const cpp_int  & l__val);
        cpp_int presentstate_inhibitcmd() const;
    
        typedef pu_cpp_int< 1 > presentstate_inhibitdat_cpp_int_t;
        cpp_int int_var__presentstate_inhibitdat;
        void presentstate_inhibitdat (const cpp_int  & l__val);
        cpp_int presentstate_inhibitdat() const;
    
        typedef pu_cpp_int< 1 > sdhcdmactrl_datalineactive_cpp_int_t;
        cpp_int int_var__sdhcdmactrl_datalineactive;
        void sdhcdmactrl_datalineactive (const cpp_int  & l__val);
        cpp_int sdhcdmactrl_datalineactive() const;
    
        typedef pu_cpp_int< 1 > sdhcsdctrl_retuningreq_dsync_cpp_int_t;
        cpp_int int_var__sdhcsdctrl_retuningreq_dsync;
        void sdhcsdctrl_retuningreq_dsync (const cpp_int  & l__val);
        cpp_int sdhcsdctrl_retuningreq_dsync() const;
    
        typedef pu_cpp_int< 4 > rsvd_7_4_cpp_int_t;
        cpp_int int_var__rsvd_7_4;
        void rsvd_7_4 (const cpp_int  & l__val);
        cpp_int rsvd_7_4() const;
    
        typedef pu_cpp_int< 1 > sdhcdmactrl_wrxferactive_cpp_int_t;
        cpp_int int_var__sdhcdmactrl_wrxferactive;
        void sdhcdmactrl_wrxferactive (const cpp_int  & l__val);
        cpp_int sdhcdmactrl_wrxferactive() const;
    
        typedef pu_cpp_int< 1 > sdhcdmactrl_rdxferactive_cpp_int_t;
        cpp_int int_var__sdhcdmactrl_rdxferactive;
        void sdhcdmactrl_rdxferactive (const cpp_int  & l__val);
        cpp_int sdhcdmactrl_rdxferactive() const;
    
        typedef pu_cpp_int< 1 > sdhcdmactrl_piobufwrena_cpp_int_t;
        cpp_int int_var__sdhcdmactrl_piobufwrena;
        void sdhcdmactrl_piobufwrena (const cpp_int  & l__val);
        cpp_int sdhcdmactrl_piobufwrena() const;
    
        typedef pu_cpp_int< 1 > sdhcdmactrl_piobufrdena_cpp_int_t;
        cpp_int int_var__sdhcdmactrl_piobufrdena;
        void sdhcdmactrl_piobufrdena (const cpp_int  & l__val);
        cpp_int sdhcdmactrl_piobufrdena() const;
    
        typedef pu_cpp_int< 4 > rsvd_15_12_cpp_int_t;
        cpp_int int_var__rsvd_15_12;
        void rsvd_15_12 (const cpp_int  & l__val);
        cpp_int rsvd_15_12() const;
    
        typedef pu_cpp_int< 1 > sdhccarddet_inserted_dsync_cpp_int_t;
        cpp_int int_var__sdhccarddet_inserted_dsync;
        void sdhccarddet_inserted_dsync (const cpp_int  & l__val);
        cpp_int sdhccarddet_inserted_dsync() const;
    
        typedef pu_cpp_int< 1 > sdhccarddet_statestable_dsync_cpp_int_t;
        cpp_int int_var__sdhccarddet_statestable_dsync;
        void sdhccarddet_statestable_dsync (const cpp_int  & l__val);
        cpp_int sdhccarddet_statestable_dsync() const;
    
        typedef pu_cpp_int< 1 > sdif_cd_n_dsync_cpp_int_t;
        cpp_int int_var__sdif_cd_n_dsync;
        void sdif_cd_n_dsync (const cpp_int  & l__val);
        cpp_int sdif_cd_n_dsync() const;
    
        typedef pu_cpp_int< 1 > sdif_wp_dsync_cpp_int_t;
        cpp_int int_var__sdif_wp_dsync;
        void sdif_wp_dsync (const cpp_int  & l__val);
        cpp_int sdif_wp_dsync() const;
    
        typedef pu_cpp_int< 1 > sdif_dat0in_dsync_cpp_int_t;
        cpp_int int_var__sdif_dat0in_dsync;
        void sdif_dat0in_dsync (const cpp_int  & l__val);
        cpp_int sdif_dat0in_dsync() const;
    
        typedef pu_cpp_int< 1 > sdif_dat1in_dsync_cpp_int_t;
        cpp_int int_var__sdif_dat1in_dsync;
        void sdif_dat1in_dsync (const cpp_int  & l__val);
        cpp_int sdif_dat1in_dsync() const;
    
        typedef pu_cpp_int< 1 > sdif_dat2in_dsync_cpp_int_t;
        cpp_int int_var__sdif_dat2in_dsync;
        void sdif_dat2in_dsync (const cpp_int  & l__val);
        cpp_int sdif_dat2in_dsync() const;
    
        typedef pu_cpp_int< 1 > sdif_dat3in_dsync_cpp_int_t;
        cpp_int int_var__sdif_dat3in_dsync;
        void sdif_dat3in_dsync (const cpp_int  & l__val);
        cpp_int sdif_dat3in_dsync() const;
    
        typedef pu_cpp_int< 1 > sdif_cmdin_dsync_cpp_int_t;
        cpp_int int_var__sdif_cmdin_dsync;
        void sdif_cmdin_dsync (const cpp_int  & l__val);
        cpp_int sdif_cmdin_dsync() const;
    
        typedef pu_cpp_int< 1 > sdif_dat4in_dsync_cpp_int_t;
        cpp_int int_var__sdif_dat4in_dsync;
        void sdif_dat4in_dsync (const cpp_int  & l__val);
        cpp_int sdif_dat4in_dsync() const;
    
        typedef pu_cpp_int< 1 > sdif_dat5in_dsync_cpp_int_t;
        cpp_int int_var__sdif_dat5in_dsync;
        void sdif_dat5in_dsync (const cpp_int  & l__val);
        cpp_int sdif_dat5in_dsync() const;
    
        typedef pu_cpp_int< 1 > sdif_dat6in_dsync_cpp_int_t;
        cpp_int int_var__sdif_dat6in_dsync;
        void sdif_dat6in_dsync (const cpp_int  & l__val);
        cpp_int sdif_dat6in_dsync() const;
    
        typedef pu_cpp_int< 1 > sdif_dat7in_dsync_cpp_int_t;
        cpp_int int_var__sdif_dat7in_dsync;
        void sdif_dat7in_dsync (const cpp_int  & l__val);
        cpp_int sdif_dat7in_dsync() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presentstate_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_hostcontrol1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_hostcontrol1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_hostcontrol1_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_hostcontrol1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_hostcontrol1_t();
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
    
        typedef pu_cpp_int< 1 > hostctrl1_ledcontrol_cpp_int_t;
        cpp_int int_var__hostctrl1_ledcontrol;
        void hostctrl1_ledcontrol (const cpp_int  & l__val);
        cpp_int hostctrl1_ledcontrol() const;
    
        typedef pu_cpp_int< 1 > hostctrl1_datawidth_cpp_int_t;
        cpp_int int_var__hostctrl1_datawidth;
        void hostctrl1_datawidth (const cpp_int  & l__val);
        cpp_int hostctrl1_datawidth() const;
    
        typedef pu_cpp_int< 1 > hostctrl1_highspeedena_cpp_int_t;
        cpp_int int_var__hostctrl1_highspeedena;
        void hostctrl1_highspeedena (const cpp_int  & l__val);
        cpp_int hostctrl1_highspeedena() const;
    
        typedef pu_cpp_int< 2 > hostctrl1_dmaselect_cpp_int_t;
        cpp_int int_var__hostctrl1_dmaselect;
        void hostctrl1_dmaselect (const cpp_int  & l__val);
        cpp_int hostctrl1_dmaselect() const;
    
        typedef pu_cpp_int< 1 > hostctrl1_extdatawidth_cpp_int_t;
        cpp_int int_var__hostctrl1_extdatawidth;
        void hostctrl1_extdatawidth (const cpp_int  & l__val);
        cpp_int hostctrl1_extdatawidth() const;
    
        typedef pu_cpp_int< 1 > hostctrl1_cdtestlevel_cpp_int_t;
        cpp_int int_var__hostctrl1_cdtestlevel;
        void hostctrl1_cdtestlevel (const cpp_int  & l__val);
        cpp_int hostctrl1_cdtestlevel() const;
    
        typedef pu_cpp_int< 1 > hostctrl1_cdsigselect_cpp_int_t;
        cpp_int int_var__hostctrl1_cdsigselect;
        void hostctrl1_cdsigselect (const cpp_int  & l__val);
        cpp_int hostctrl1_cdsigselect() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_hostcontrol1_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_powercontrol_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_powercontrol_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_powercontrol_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_powercontrol_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_powercontrol_t();
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
    
        typedef pu_cpp_int< 1 > pwrctrl_sdbuspower_cpp_int_t;
        cpp_int int_var__pwrctrl_sdbuspower;
        void pwrctrl_sdbuspower (const cpp_int  & l__val);
        cpp_int pwrctrl_sdbuspower() const;
    
        typedef pu_cpp_int< 3 > pwrctrl_sdbusvoltage_cpp_int_t;
        cpp_int int_var__pwrctrl_sdbusvoltage;
        void pwrctrl_sdbusvoltage (const cpp_int  & l__val);
        cpp_int pwrctrl_sdbusvoltage() const;
    
        typedef pu_cpp_int< 1 > emmc_hwreset_cpp_int_t;
        cpp_int int_var__emmc_hwreset;
        void emmc_hwreset (const cpp_int  & l__val);
        cpp_int emmc_hwreset() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_powercontrol_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_blockgapcontrol_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_blockgapcontrol_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_blockgapcontrol_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_blockgapcontrol_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_blockgapcontrol_t();
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
    
        typedef pu_cpp_int< 1 > blkgapctrl_stopatblkgap_cpp_int_t;
        cpp_int int_var__blkgapctrl_stopatblkgap;
        void blkgapctrl_stopatblkgap (const cpp_int  & l__val);
        cpp_int blkgapctrl_stopatblkgap() const;
    
        typedef pu_cpp_int< 1 > blkgapctrl_continue_cpp_int_t;
        cpp_int int_var__blkgapctrl_continue;
        void blkgapctrl_continue (const cpp_int  & l__val);
        cpp_int blkgapctrl_continue() const;
    
        typedef pu_cpp_int< 1 > blkgapctrl_rdwaitctrl_cpp_int_t;
        cpp_int int_var__blkgapctrl_rdwaitctrl;
        void blkgapctrl_rdwaitctrl (const cpp_int  & l__val);
        cpp_int blkgapctrl_rdwaitctrl() const;
    
        typedef pu_cpp_int< 1 > blkgapctrl_interrupt_cpp_int_t;
        cpp_int int_var__blkgapctrl_interrupt;
        void blkgapctrl_interrupt (const cpp_int  & l__val);
        cpp_int blkgapctrl_interrupt() const;
    
        typedef pu_cpp_int< 1 > blkgapctrl_spimode_cpp_int_t;
        cpp_int int_var__blkgapctrl_spimode;
        void blkgapctrl_spimode (const cpp_int  & l__val);
        cpp_int blkgapctrl_spimode() const;
    
        typedef pu_cpp_int< 1 > blkgapctrl_bootenable_cpp_int_t;
        cpp_int int_var__blkgapctrl_bootenable;
        void blkgapctrl_bootenable (const cpp_int  & l__val);
        cpp_int blkgapctrl_bootenable() const;
    
        typedef pu_cpp_int< 1 > blkgapctrl_altbootmode_cpp_int_t;
        cpp_int int_var__blkgapctrl_altbootmode;
        void blkgapctrl_altbootmode (const cpp_int  & l__val);
        cpp_int blkgapctrl_altbootmode() const;
    
        typedef pu_cpp_int< 1 > blkgapctrl_bootackena_cpp_int_t;
        cpp_int int_var__blkgapctrl_bootackena;
        void blkgapctrl_bootackena (const cpp_int  & l__val);
        cpp_int blkgapctrl_bootackena() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_blockgapcontrol_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_wakeupcontrol_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_wakeupcontrol_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_wakeupcontrol_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_wakeupcontrol_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_wakeupcontrol_t();
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
    
        typedef pu_cpp_int< 1 > wkupctrl_cardinterrupt_cpp_int_t;
        cpp_int int_var__wkupctrl_cardinterrupt;
        void wkupctrl_cardinterrupt (const cpp_int  & l__val);
        cpp_int wkupctrl_cardinterrupt() const;
    
        typedef pu_cpp_int< 1 > wkupctrl_cardinsertion_cpp_int_t;
        cpp_int int_var__wkupctrl_cardinsertion;
        void wkupctrl_cardinsertion (const cpp_int  & l__val);
        cpp_int wkupctrl_cardinsertion() const;
    
        typedef pu_cpp_int< 1 > wkupctrl_cardremoval_cpp_int_t;
        cpp_int int_var__wkupctrl_cardremoval;
        void wkupctrl_cardremoval (const cpp_int  & l__val);
        cpp_int wkupctrl_cardremoval() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_wakeupcontrol_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_clockcontrol_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_clockcontrol_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_clockcontrol_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_clockcontrol_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_clockcontrol_t();
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
    
        typedef pu_cpp_int< 1 > clkctrl_intclkena_cpp_int_t;
        cpp_int int_var__clkctrl_intclkena;
        void clkctrl_intclkena (const cpp_int  & l__val);
        cpp_int clkctrl_intclkena() const;
    
        typedef pu_cpp_int< 1 > sdhcclkgen_intclkstable_dsync_cpp_int_t;
        cpp_int int_var__sdhcclkgen_intclkstable_dsync;
        void sdhcclkgen_intclkstable_dsync (const cpp_int  & l__val);
        cpp_int sdhcclkgen_intclkstable_dsync() const;
    
        typedef pu_cpp_int< 1 > clkctrl_sdclkena_cpp_int_t;
        cpp_int int_var__clkctrl_sdclkena;
        void clkctrl_sdclkena (const cpp_int  & l__val);
        cpp_int clkctrl_sdclkena() const;
    
        typedef pu_cpp_int< 2 > rsvd_4_3_cpp_int_t;
        cpp_int int_var__rsvd_4_3;
        void rsvd_4_3 (const cpp_int  & l__val);
        cpp_int rsvd_4_3() const;
    
        typedef pu_cpp_int< 1 > clkctrl_clkgensel_cpp_int_t;
        cpp_int int_var__clkctrl_clkgensel;
        void clkctrl_clkgensel (const cpp_int  & l__val);
        cpp_int clkctrl_clkgensel() const;
    
        typedef pu_cpp_int< 2 > clkctrl_sdclkfreqsel_upperbits_cpp_int_t;
        cpp_int int_var__clkctrl_sdclkfreqsel_upperbits;
        void clkctrl_sdclkfreqsel_upperbits (const cpp_int  & l__val);
        cpp_int clkctrl_sdclkfreqsel_upperbits() const;
    
        typedef pu_cpp_int< 8 > clkctrl_sdclkfreqsel_cpp_int_t;
        cpp_int int_var__clkctrl_sdclkfreqsel;
        void clkctrl_sdclkfreqsel (const cpp_int  & l__val);
        cpp_int clkctrl_sdclkfreqsel() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_clockcontrol_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_timeoutcontrol_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_timeoutcontrol_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_timeoutcontrol_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_timeoutcontrol_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_timeoutcontrol_t();
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
    
        typedef pu_cpp_int< 4 > timeout_ctrvalue_cpp_int_t;
        cpp_int int_var__timeout_ctrvalue;
        void timeout_ctrvalue (const cpp_int  & l__val);
        cpp_int timeout_ctrvalue() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_timeoutcontrol_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_softwarereset_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_softwarereset_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_softwarereset_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_softwarereset_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_softwarereset_t();
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
    
        typedef pu_cpp_int< 1 > swreset_for_all_cpp_int_t;
        cpp_int int_var__swreset_for_all;
        void swreset_for_all (const cpp_int  & l__val);
        cpp_int swreset_for_all() const;
    
        typedef pu_cpp_int< 1 > swreset_for_cmd_cpp_int_t;
        cpp_int int_var__swreset_for_cmd;
        void swreset_for_cmd (const cpp_int  & l__val);
        cpp_int swreset_for_cmd() const;
    
        typedef pu_cpp_int< 1 > swreset_for_dat_cpp_int_t;
        cpp_int int_var__swreset_for_dat;
        void swreset_for_dat (const cpp_int  & l__val);
        cpp_int swreset_for_dat() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_softwarereset_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_normalintrsts_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_normalintrsts_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_normalintrsts_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_normalintrsts_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_normalintrsts_t();
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
    
        typedef pu_cpp_int< 1 > normalintrsts_cmdcomplete_cpp_int_t;
        cpp_int int_var__normalintrsts_cmdcomplete;
        void normalintrsts_cmdcomplete (const cpp_int  & l__val);
        cpp_int normalintrsts_cmdcomplete() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_xfercomplete_cpp_int_t;
        cpp_int int_var__normalintrsts_xfercomplete;
        void normalintrsts_xfercomplete (const cpp_int  & l__val);
        cpp_int normalintrsts_xfercomplete() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_blkgapevent_cpp_int_t;
        cpp_int int_var__normalintrsts_blkgapevent;
        void normalintrsts_blkgapevent (const cpp_int  & l__val);
        cpp_int normalintrsts_blkgapevent() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_dmainterrupt_cpp_int_t;
        cpp_int int_var__normalintrsts_dmainterrupt;
        void normalintrsts_dmainterrupt (const cpp_int  & l__val);
        cpp_int normalintrsts_dmainterrupt() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_bufwrready_cpp_int_t;
        cpp_int int_var__normalintrsts_bufwrready;
        void normalintrsts_bufwrready (const cpp_int  & l__val);
        cpp_int normalintrsts_bufwrready() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_bufrdready_cpp_int_t;
        cpp_int int_var__normalintrsts_bufrdready;
        void normalintrsts_bufrdready (const cpp_int  & l__val);
        cpp_int normalintrsts_bufrdready() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_cardinssts_cpp_int_t;
        cpp_int int_var__normalintrsts_cardinssts;
        void normalintrsts_cardinssts (const cpp_int  & l__val);
        cpp_int normalintrsts_cardinssts() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_cardremsts_cpp_int_t;
        cpp_int int_var__normalintrsts_cardremsts;
        void normalintrsts_cardremsts (const cpp_int  & l__val);
        cpp_int normalintrsts_cardremsts() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_cardintsts_cpp_int_t;
        cpp_int int_var__normalintrsts_cardintsts;
        void normalintrsts_cardintsts (const cpp_int  & l__val);
        cpp_int normalintrsts_cardintsts() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_inta_cpp_int_t;
        cpp_int int_var__normalintrsts_inta;
        void normalintrsts_inta (const cpp_int  & l__val);
        cpp_int normalintrsts_inta() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_intb_cpp_int_t;
        cpp_int int_var__normalintrsts_intb;
        void normalintrsts_intb (const cpp_int  & l__val);
        cpp_int normalintrsts_intb() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_intc_cpp_int_t;
        cpp_int int_var__normalintrsts_intc;
        void normalintrsts_intc (const cpp_int  & l__val);
        cpp_int normalintrsts_intc() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_retuningevent_cpp_int_t;
        cpp_int int_var__normalintrsts_retuningevent;
        void normalintrsts_retuningevent (const cpp_int  & l__val);
        cpp_int normalintrsts_retuningevent() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_rcvbootack_cpp_int_t;
        cpp_int int_var__normalintrsts_rcvbootack;
        void normalintrsts_rcvbootack (const cpp_int  & l__val);
        cpp_int normalintrsts_rcvbootack() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_bootcomplete_cpp_int_t;
        cpp_int int_var__normalintrsts_bootcomplete;
        void normalintrsts_bootcomplete (const cpp_int  & l__val);
        cpp_int normalintrsts_bootcomplete() const;
    
        typedef pu_cpp_int< 1 > reg_errorintrsts_cpp_int_t;
        cpp_int int_var__reg_errorintrsts;
        void reg_errorintrsts (const cpp_int  & l__val);
        cpp_int reg_errorintrsts() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_normalintrsts_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_errorintrsts_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_errorintrsts_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_errorintrsts_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_errorintrsts_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_errorintrsts_t();
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
    
        typedef pu_cpp_int< 1 > errorintrsts_cmdtimeouterror_cpp_int_t;
        cpp_int int_var__errorintrsts_cmdtimeouterror;
        void errorintrsts_cmdtimeouterror (const cpp_int  & l__val);
        cpp_int errorintrsts_cmdtimeouterror() const;
    
        typedef pu_cpp_int< 1 > errorintrsts_cmdcrcerror_cpp_int_t;
        cpp_int int_var__errorintrsts_cmdcrcerror;
        void errorintrsts_cmdcrcerror (const cpp_int  & l__val);
        cpp_int errorintrsts_cmdcrcerror() const;
    
        typedef pu_cpp_int< 1 > errorintrsts_cmdendbiterror_cpp_int_t;
        cpp_int int_var__errorintrsts_cmdendbiterror;
        void errorintrsts_cmdendbiterror (const cpp_int  & l__val);
        cpp_int errorintrsts_cmdendbiterror() const;
    
        typedef pu_cpp_int< 1 > errorintrsts_cmdindexerror_cpp_int_t;
        cpp_int int_var__errorintrsts_cmdindexerror;
        void errorintrsts_cmdindexerror (const cpp_int  & l__val);
        cpp_int errorintrsts_cmdindexerror() const;
    
        typedef pu_cpp_int< 1 > errorintrsts_datatimeouterror_cpp_int_t;
        cpp_int int_var__errorintrsts_datatimeouterror;
        void errorintrsts_datatimeouterror (const cpp_int  & l__val);
        cpp_int errorintrsts_datatimeouterror() const;
    
        typedef pu_cpp_int< 1 > errorintrsts_datacrcerror_cpp_int_t;
        cpp_int int_var__errorintrsts_datacrcerror;
        void errorintrsts_datacrcerror (const cpp_int  & l__val);
        cpp_int errorintrsts_datacrcerror() const;
    
        typedef pu_cpp_int< 1 > errorintrsts_dataendbiterror_cpp_int_t;
        cpp_int int_var__errorintrsts_dataendbiterror;
        void errorintrsts_dataendbiterror (const cpp_int  & l__val);
        cpp_int errorintrsts_dataendbiterror() const;
    
        typedef pu_cpp_int< 1 > errorintrsts_currlimiterror_cpp_int_t;
        cpp_int int_var__errorintrsts_currlimiterror;
        void errorintrsts_currlimiterror (const cpp_int  & l__val);
        cpp_int errorintrsts_currlimiterror() const;
    
        typedef pu_cpp_int< 1 > errorintrsts_autocmderror_cpp_int_t;
        cpp_int int_var__errorintrsts_autocmderror;
        void errorintrsts_autocmderror (const cpp_int  & l__val);
        cpp_int errorintrsts_autocmderror() const;
    
        typedef pu_cpp_int< 1 > errorintrsts_admaerror_cpp_int_t;
        cpp_int int_var__errorintrsts_admaerror;
        void errorintrsts_admaerror (const cpp_int  & l__val);
        cpp_int errorintrsts_admaerror() const;
    
        typedef pu_cpp_int< 2 > rsvd_11_10_cpp_int_t;
        cpp_int int_var__rsvd_11_10;
        void rsvd_11_10 (const cpp_int  & l__val);
        cpp_int rsvd_11_10() const;
    
        typedef pu_cpp_int< 1 > errorintrsts_hosterror_cpp_int_t;
        cpp_int int_var__errorintrsts_hosterror;
        void errorintrsts_hosterror (const cpp_int  & l__val);
        cpp_int errorintrsts_hosterror() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_errorintrsts_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_normalintrstsena_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_normalintrstsena_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_normalintrstsena_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_normalintrstsena_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_normalintrstsena_t();
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
    
        typedef pu_cpp_int< 1 > normalintrsts_enableregbit0_cpp_int_t;
        cpp_int int_var__normalintrsts_enableregbit0;
        void normalintrsts_enableregbit0 (const cpp_int  & l__val);
        cpp_int normalintrsts_enableregbit0() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_enableregbit1_cpp_int_t;
        cpp_int int_var__normalintrsts_enableregbit1;
        void normalintrsts_enableregbit1 (const cpp_int  & l__val);
        cpp_int normalintrsts_enableregbit1() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_enableregbit2_cpp_int_t;
        cpp_int int_var__normalintrsts_enableregbit2;
        void normalintrsts_enableregbit2 (const cpp_int  & l__val);
        cpp_int normalintrsts_enableregbit2() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_enableregbit3_cpp_int_t;
        cpp_int int_var__normalintrsts_enableregbit3;
        void normalintrsts_enableregbit3 (const cpp_int  & l__val);
        cpp_int normalintrsts_enableregbit3() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_enableregbit4_cpp_int_t;
        cpp_int int_var__normalintrsts_enableregbit4;
        void normalintrsts_enableregbit4 (const cpp_int  & l__val);
        cpp_int normalintrsts_enableregbit4() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_enableregbit5_cpp_int_t;
        cpp_int int_var__normalintrsts_enableregbit5;
        void normalintrsts_enableregbit5 (const cpp_int  & l__val);
        cpp_int normalintrsts_enableregbit5() const;
    
        typedef pu_cpp_int< 1 > sdhcregset_cardinsstsena_cpp_int_t;
        cpp_int int_var__sdhcregset_cardinsstsena;
        void sdhcregset_cardinsstsena (const cpp_int  & l__val);
        cpp_int sdhcregset_cardinsstsena() const;
    
        typedef pu_cpp_int< 1 > sdhcregset_cardremstsena_cpp_int_t;
        cpp_int int_var__sdhcregset_cardremstsena;
        void sdhcregset_cardremstsena (const cpp_int  & l__val);
        cpp_int sdhcregset_cardremstsena() const;
    
        typedef pu_cpp_int< 1 > sdhcregset_cardintstsena_cpp_int_t;
        cpp_int int_var__sdhcregset_cardintstsena;
        void sdhcregset_cardintstsena (const cpp_int  & l__val);
        cpp_int sdhcregset_cardintstsena() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_enableregbit9_cpp_int_t;
        cpp_int int_var__normalintrsts_enableregbit9;
        void normalintrsts_enableregbit9 (const cpp_int  & l__val);
        cpp_int normalintrsts_enableregbit9() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_enableregbit10_cpp_int_t;
        cpp_int int_var__normalintrsts_enableregbit10;
        void normalintrsts_enableregbit10 (const cpp_int  & l__val);
        cpp_int normalintrsts_enableregbit10() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_enableregbit11_cpp_int_t;
        cpp_int int_var__normalintrsts_enableregbit11;
        void normalintrsts_enableregbit11 (const cpp_int  & l__val);
        cpp_int normalintrsts_enableregbit11() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_enableregbit12_cpp_int_t;
        cpp_int int_var__normalintrsts_enableregbit12;
        void normalintrsts_enableregbit12 (const cpp_int  & l__val);
        cpp_int normalintrsts_enableregbit12() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_enableregbit13_cpp_int_t;
        cpp_int int_var__normalintrsts_enableregbit13;
        void normalintrsts_enableregbit13 (const cpp_int  & l__val);
        cpp_int normalintrsts_enableregbit13() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_enableregbit14_cpp_int_t;
        cpp_int int_var__normalintrsts_enableregbit14;
        void normalintrsts_enableregbit14 (const cpp_int  & l__val);
        cpp_int normalintrsts_enableregbit14() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_enableregbit15_cpp_int_t;
        cpp_int int_var__normalintrsts_enableregbit15;
        void normalintrsts_enableregbit15 (const cpp_int  & l__val);
        cpp_int normalintrsts_enableregbit15() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_normalintrstsena_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_errorintrstsena_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_errorintrstsena_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_errorintrstsena_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_errorintrstsena_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_errorintrstsena_t();
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
    
        typedef pu_cpp_int< 1 > errorintrsts_enableregbit0_cpp_int_t;
        cpp_int int_var__errorintrsts_enableregbit0;
        void errorintrsts_enableregbit0 (const cpp_int  & l__val);
        cpp_int errorintrsts_enableregbit0() const;
    
        typedef pu_cpp_int< 1 > errorintrsts_enableregbit1_cpp_int_t;
        cpp_int int_var__errorintrsts_enableregbit1;
        void errorintrsts_enableregbit1 (const cpp_int  & l__val);
        cpp_int errorintrsts_enableregbit1() const;
    
        typedef pu_cpp_int< 1 > errorintrsts_enableregbit2_cpp_int_t;
        cpp_int int_var__errorintrsts_enableregbit2;
        void errorintrsts_enableregbit2 (const cpp_int  & l__val);
        cpp_int errorintrsts_enableregbit2() const;
    
        typedef pu_cpp_int< 1 > errorintrsts_enableregbit3_cpp_int_t;
        cpp_int int_var__errorintrsts_enableregbit3;
        void errorintrsts_enableregbit3 (const cpp_int  & l__val);
        cpp_int errorintrsts_enableregbit3() const;
    
        typedef pu_cpp_int< 1 > errorintrsts_enableregbit4_cpp_int_t;
        cpp_int int_var__errorintrsts_enableregbit4;
        void errorintrsts_enableregbit4 (const cpp_int  & l__val);
        cpp_int errorintrsts_enableregbit4() const;
    
        typedef pu_cpp_int< 1 > errorintrsts_enableregbit5_cpp_int_t;
        cpp_int int_var__errorintrsts_enableregbit5;
        void errorintrsts_enableregbit5 (const cpp_int  & l__val);
        cpp_int errorintrsts_enableregbit5() const;
    
        typedef pu_cpp_int< 1 > errorintrsts_enableregbit6_cpp_int_t;
        cpp_int int_var__errorintrsts_enableregbit6;
        void errorintrsts_enableregbit6 (const cpp_int  & l__val);
        cpp_int errorintrsts_enableregbit6() const;
    
        typedef pu_cpp_int< 1 > errorintrsts_enableregbit7_cpp_int_t;
        cpp_int int_var__errorintrsts_enableregbit7;
        void errorintrsts_enableregbit7 (const cpp_int  & l__val);
        cpp_int errorintrsts_enableregbit7() const;
    
        typedef pu_cpp_int< 1 > errorintrsts_enableregbit8_cpp_int_t;
        cpp_int int_var__errorintrsts_enableregbit8;
        void errorintrsts_enableregbit8 (const cpp_int  & l__val);
        cpp_int errorintrsts_enableregbit8() const;
    
        typedef pu_cpp_int< 1 > errorintrsts_enableregbit9_cpp_int_t;
        cpp_int int_var__errorintrsts_enableregbit9;
        void errorintrsts_enableregbit9 (const cpp_int  & l__val);
        cpp_int errorintrsts_enableregbit9() const;
    
        typedef pu_cpp_int< 1 > errorintrsts_enableregbit10_cpp_int_t;
        cpp_int int_var__errorintrsts_enableregbit10;
        void errorintrsts_enableregbit10 (const cpp_int  & l__val);
        cpp_int errorintrsts_enableregbit10() const;
    
        typedef pu_cpp_int< 1 > rsvd_11_11_cpp_int_t;
        cpp_int int_var__rsvd_11_11;
        void rsvd_11_11 (const cpp_int  & l__val);
        cpp_int rsvd_11_11() const;
    
        typedef pu_cpp_int< 1 > errorintrsts_enableregbit12_cpp_int_t;
        cpp_int int_var__errorintrsts_enableregbit12;
        void errorintrsts_enableregbit12 (const cpp_int  & l__val);
        cpp_int errorintrsts_enableregbit12() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_errorintrstsena_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_normalintrsigena_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_normalintrsigena_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_normalintrsigena_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_normalintrsigena_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_normalintrsigena_t();
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
    
        typedef pu_cpp_int< 1 > normalintrsts_enableregbit0_cpp_int_t;
        cpp_int int_var__normalintrsts_enableregbit0;
        void normalintrsts_enableregbit0 (const cpp_int  & l__val);
        cpp_int normalintrsts_enableregbit0() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_enableregbit1_cpp_int_t;
        cpp_int int_var__normalintrsts_enableregbit1;
        void normalintrsts_enableregbit1 (const cpp_int  & l__val);
        cpp_int normalintrsts_enableregbit1() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_enableregbit2_cpp_int_t;
        cpp_int int_var__normalintrsts_enableregbit2;
        void normalintrsts_enableregbit2 (const cpp_int  & l__val);
        cpp_int normalintrsts_enableregbit2() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_enableregbit3_cpp_int_t;
        cpp_int int_var__normalintrsts_enableregbit3;
        void normalintrsts_enableregbit3 (const cpp_int  & l__val);
        cpp_int normalintrsts_enableregbit3() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_enableregbit4_cpp_int_t;
        cpp_int int_var__normalintrsts_enableregbit4;
        void normalintrsts_enableregbit4 (const cpp_int  & l__val);
        cpp_int normalintrsts_enableregbit4() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_enableregbit5_cpp_int_t;
        cpp_int int_var__normalintrsts_enableregbit5;
        void normalintrsts_enableregbit5 (const cpp_int  & l__val);
        cpp_int normalintrsts_enableregbit5() const;
    
        typedef pu_cpp_int< 1 > sdhcregset_cardinsstsena_cpp_int_t;
        cpp_int int_var__sdhcregset_cardinsstsena;
        void sdhcregset_cardinsstsena (const cpp_int  & l__val);
        cpp_int sdhcregset_cardinsstsena() const;
    
        typedef pu_cpp_int< 1 > sdhcregset_cardremstsena_cpp_int_t;
        cpp_int int_var__sdhcregset_cardremstsena;
        void sdhcregset_cardremstsena (const cpp_int  & l__val);
        cpp_int sdhcregset_cardremstsena() const;
    
        typedef pu_cpp_int< 1 > sdhcregset_cardintstsena_cpp_int_t;
        cpp_int int_var__sdhcregset_cardintstsena;
        void sdhcregset_cardintstsena (const cpp_int  & l__val);
        cpp_int sdhcregset_cardintstsena() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_enableregbit9_cpp_int_t;
        cpp_int int_var__normalintrsts_enableregbit9;
        void normalintrsts_enableregbit9 (const cpp_int  & l__val);
        cpp_int normalintrsts_enableregbit9() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_enableregbit10_cpp_int_t;
        cpp_int int_var__normalintrsts_enableregbit10;
        void normalintrsts_enableregbit10 (const cpp_int  & l__val);
        cpp_int normalintrsts_enableregbit10() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_enableregbit11_cpp_int_t;
        cpp_int int_var__normalintrsts_enableregbit11;
        void normalintrsts_enableregbit11 (const cpp_int  & l__val);
        cpp_int normalintrsts_enableregbit11() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_enableregbit12_cpp_int_t;
        cpp_int int_var__normalintrsts_enableregbit12;
        void normalintrsts_enableregbit12 (const cpp_int  & l__val);
        cpp_int normalintrsts_enableregbit12() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_enableregbit13_cpp_int_t;
        cpp_int int_var__normalintrsts_enableregbit13;
        void normalintrsts_enableregbit13 (const cpp_int  & l__val);
        cpp_int normalintrsts_enableregbit13() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_enableregbit14_cpp_int_t;
        cpp_int int_var__normalintrsts_enableregbit14;
        void normalintrsts_enableregbit14 (const cpp_int  & l__val);
        cpp_int normalintrsts_enableregbit14() const;
    
        typedef pu_cpp_int< 1 > normalintrsts_enableregbit15_cpp_int_t;
        cpp_int int_var__normalintrsts_enableregbit15;
        void normalintrsts_enableregbit15 (const cpp_int  & l__val);
        cpp_int normalintrsts_enableregbit15() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_normalintrsigena_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_errorintrsigena_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_errorintrsigena_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_errorintrsigena_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_errorintrsigena_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_errorintrsigena_t();
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
    
        typedef pu_cpp_int< 1 > errorintrsig_enableregbit0_cpp_int_t;
        cpp_int int_var__errorintrsig_enableregbit0;
        void errorintrsig_enableregbit0 (const cpp_int  & l__val);
        cpp_int errorintrsig_enableregbit0() const;
    
        typedef pu_cpp_int< 1 > errorintrsig_enableregbit1_cpp_int_t;
        cpp_int int_var__errorintrsig_enableregbit1;
        void errorintrsig_enableregbit1 (const cpp_int  & l__val);
        cpp_int errorintrsig_enableregbit1() const;
    
        typedef pu_cpp_int< 1 > errorintrsig_enableregbit2_cpp_int_t;
        cpp_int int_var__errorintrsig_enableregbit2;
        void errorintrsig_enableregbit2 (const cpp_int  & l__val);
        cpp_int errorintrsig_enableregbit2() const;
    
        typedef pu_cpp_int< 1 > errorintrsig_enableregbit3_cpp_int_t;
        cpp_int int_var__errorintrsig_enableregbit3;
        void errorintrsig_enableregbit3 (const cpp_int  & l__val);
        cpp_int errorintrsig_enableregbit3() const;
    
        typedef pu_cpp_int< 1 > errorintrsig_enableregbit4_cpp_int_t;
        cpp_int int_var__errorintrsig_enableregbit4;
        void errorintrsig_enableregbit4 (const cpp_int  & l__val);
        cpp_int errorintrsig_enableregbit4() const;
    
        typedef pu_cpp_int< 1 > errorintrsig_enableregbit5_cpp_int_t;
        cpp_int int_var__errorintrsig_enableregbit5;
        void errorintrsig_enableregbit5 (const cpp_int  & l__val);
        cpp_int errorintrsig_enableregbit5() const;
    
        typedef pu_cpp_int< 1 > errorintrsig_enableregbit6_cpp_int_t;
        cpp_int int_var__errorintrsig_enableregbit6;
        void errorintrsig_enableregbit6 (const cpp_int  & l__val);
        cpp_int errorintrsig_enableregbit6() const;
    
        typedef pu_cpp_int< 1 > errorintrsig_enableregbit7_cpp_int_t;
        cpp_int int_var__errorintrsig_enableregbit7;
        void errorintrsig_enableregbit7 (const cpp_int  & l__val);
        cpp_int errorintrsig_enableregbit7() const;
    
        typedef pu_cpp_int< 1 > errorintrsig_enableregbit8_cpp_int_t;
        cpp_int int_var__errorintrsig_enableregbit8;
        void errorintrsig_enableregbit8 (const cpp_int  & l__val);
        cpp_int errorintrsig_enableregbit8() const;
    
        typedef pu_cpp_int< 1 > errorintrsig_enableregbit9_cpp_int_t;
        cpp_int int_var__errorintrsig_enableregbit9;
        void errorintrsig_enableregbit9 (const cpp_int  & l__val);
        cpp_int errorintrsig_enableregbit9() const;
    
        typedef pu_cpp_int< 1 > errorintrsig_enableregbit10_cpp_int_t;
        cpp_int int_var__errorintrsig_enableregbit10;
        void errorintrsig_enableregbit10 (const cpp_int  & l__val);
        cpp_int errorintrsig_enableregbit10() const;
    
        typedef pu_cpp_int< 1 > rsvd_11_11_cpp_int_t;
        cpp_int int_var__rsvd_11_11;
        void rsvd_11_11 (const cpp_int  & l__val);
        cpp_int rsvd_11_11() const;
    
        typedef pu_cpp_int< 1 > errorintrsig_enableregbit12_cpp_int_t;
        cpp_int int_var__errorintrsig_enableregbit12;
        void errorintrsig_enableregbit12 (const cpp_int  & l__val);
        cpp_int errorintrsig_enableregbit12() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_errorintrsigena_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_autocmderrsts_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_autocmderrsts_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_autocmderrsts_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_autocmderrsts_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_autocmderrsts_t();
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
    
        typedef pu_cpp_int< 1 > autocmderrsts_notexecerror_cpp_int_t;
        cpp_int int_var__autocmderrsts_notexecerror;
        void autocmderrsts_notexecerror (const cpp_int  & l__val);
        cpp_int autocmderrsts_notexecerror() const;
    
        typedef pu_cpp_int< 1 > autocmderrsts_timeouterror_cpp_int_t;
        cpp_int int_var__autocmderrsts_timeouterror;
        void autocmderrsts_timeouterror (const cpp_int  & l__val);
        cpp_int autocmderrsts_timeouterror() const;
    
        typedef pu_cpp_int< 1 > autocmderrsts_crcerror_cpp_int_t;
        cpp_int int_var__autocmderrsts_crcerror;
        void autocmderrsts_crcerror (const cpp_int  & l__val);
        cpp_int autocmderrsts_crcerror() const;
    
        typedef pu_cpp_int< 1 > autocmderrsts_endbiterror_cpp_int_t;
        cpp_int int_var__autocmderrsts_endbiterror;
        void autocmderrsts_endbiterror (const cpp_int  & l__val);
        cpp_int autocmderrsts_endbiterror() const;
    
        typedef pu_cpp_int< 1 > autocmderrsts_indexerror_cpp_int_t;
        cpp_int int_var__autocmderrsts_indexerror;
        void autocmderrsts_indexerror (const cpp_int  & l__val);
        cpp_int autocmderrsts_indexerror() const;
    
        typedef pu_cpp_int< 2 > rsvd_6_5_cpp_int_t;
        cpp_int int_var__rsvd_6_5;
        void rsvd_6_5 (const cpp_int  & l__val);
        cpp_int rsvd_6_5() const;
    
        typedef pu_cpp_int< 1 > autocmderrsts_nexterror_cpp_int_t;
        cpp_int int_var__autocmderrsts_nexterror;
        void autocmderrsts_nexterror (const cpp_int  & l__val);
        cpp_int autocmderrsts_nexterror() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_autocmderrsts_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_hostcontrol2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_hostcontrol2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_hostcontrol2_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_hostcontrol2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_hostcontrol2_t();
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
    
        typedef pu_cpp_int< 3 > hostctrl2_uhsmodeselect_cpp_int_t;
        cpp_int int_var__hostctrl2_uhsmodeselect;
        void hostctrl2_uhsmodeselect (const cpp_int  & l__val);
        cpp_int hostctrl2_uhsmodeselect() const;
    
        typedef pu_cpp_int< 1 > hostctrl2_1p8vsignallingena_cpp_int_t;
        cpp_int int_var__hostctrl2_1p8vsignallingena;
        void hostctrl2_1p8vsignallingena (const cpp_int  & l__val);
        cpp_int hostctrl2_1p8vsignallingena() const;
    
        typedef pu_cpp_int< 2 > hostctrl2_driverstrength_cpp_int_t;
        cpp_int int_var__hostctrl2_driverstrength;
        void hostctrl2_driverstrength (const cpp_int  & l__val);
        cpp_int hostctrl2_driverstrength() const;
    
        typedef pu_cpp_int< 1 > hostctrl2_executetuning_cpp_int_t;
        cpp_int int_var__hostctrl2_executetuning;
        void hostctrl2_executetuning (const cpp_int  & l__val);
        cpp_int hostctrl2_executetuning() const;
    
        typedef pu_cpp_int< 1 > hostctrl2_samplingclkselect_cpp_int_t;
        cpp_int int_var__hostctrl2_samplingclkselect;
        void hostctrl2_samplingclkselect (const cpp_int  & l__val);
        cpp_int hostctrl2_samplingclkselect() const;
    
        typedef pu_cpp_int< 1 > rsvd_8_8_cpp_int_t;
        cpp_int int_var__rsvd_8_8;
        void rsvd_8_8 (const cpp_int  & l__val);
        cpp_int rsvd_8_8() const;
    
        typedef pu_cpp_int< 1 > hostctrl2_driverstrength_bit2_cpp_int_t;
        cpp_int int_var__hostctrl2_driverstrength_bit2;
        void hostctrl2_driverstrength_bit2 (const cpp_int  & l__val);
        cpp_int hostctrl2_driverstrength_bit2() const;
    
        typedef pu_cpp_int< 4 > rsvd_13_10_cpp_int_t;
        cpp_int int_var__rsvd_13_10;
        void rsvd_13_10 (const cpp_int  & l__val);
        cpp_int rsvd_13_10() const;
    
        typedef pu_cpp_int< 1 > hostctrl2_asynchintrenable_cpp_int_t;
        cpp_int int_var__hostctrl2_asynchintrenable;
        void hostctrl2_asynchintrenable (const cpp_int  & l__val);
        cpp_int hostctrl2_asynchintrenable() const;
    
        typedef pu_cpp_int< 1 > hostctrl2_presetvalueenable_cpp_int_t;
        cpp_int int_var__hostctrl2_presetvalueenable;
        void hostctrl2_presetvalueenable (const cpp_int  & l__val);
        cpp_int hostctrl2_presetvalueenable() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_hostcontrol2_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_capabilities_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_capabilities_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_capabilities_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_capabilities_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_capabilities_t();
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
    
        typedef pu_cpp_int< 6 > corecfg_timeoutclkfreq_cpp_int_t;
        cpp_int int_var__corecfg_timeoutclkfreq;
        void corecfg_timeoutclkfreq (const cpp_int  & l__val);
        cpp_int corecfg_timeoutclkfreq() const;
    
        typedef pu_cpp_int< 1 > rsvd_6_6_cpp_int_t;
        cpp_int int_var__rsvd_6_6;
        void rsvd_6_6 (const cpp_int  & l__val);
        cpp_int rsvd_6_6() const;
    
        typedef pu_cpp_int< 1 > corecfg_timeoutclkunit_cpp_int_t;
        cpp_int int_var__corecfg_timeoutclkunit;
        void corecfg_timeoutclkunit (const cpp_int  & l__val);
        cpp_int corecfg_timeoutclkunit() const;
    
        typedef pu_cpp_int< 8 > corecfg_baseclkfreq_cpp_int_t;
        cpp_int int_var__corecfg_baseclkfreq;
        void corecfg_baseclkfreq (const cpp_int  & l__val);
        cpp_int corecfg_baseclkfreq() const;
    
        typedef pu_cpp_int< 2 > corecfg_maxblklength_cpp_int_t;
        cpp_int int_var__corecfg_maxblklength;
        void corecfg_maxblklength (const cpp_int  & l__val);
        cpp_int corecfg_maxblklength() const;
    
        typedef pu_cpp_int< 1 > corecfg_8bitsupport_cpp_int_t;
        cpp_int int_var__corecfg_8bitsupport;
        void corecfg_8bitsupport (const cpp_int  & l__val);
        cpp_int corecfg_8bitsupport() const;
    
        typedef pu_cpp_int< 1 > corecfg_adma2support_cpp_int_t;
        cpp_int int_var__corecfg_adma2support;
        void corecfg_adma2support (const cpp_int  & l__val);
        cpp_int corecfg_adma2support() const;
    
        typedef pu_cpp_int< 1 > rsvd_20_20_cpp_int_t;
        cpp_int int_var__rsvd_20_20;
        void rsvd_20_20 (const cpp_int  & l__val);
        cpp_int rsvd_20_20() const;
    
        typedef pu_cpp_int< 1 > corecfg_highspeedsupport_cpp_int_t;
        cpp_int int_var__corecfg_highspeedsupport;
        void corecfg_highspeedsupport (const cpp_int  & l__val);
        cpp_int corecfg_highspeedsupport() const;
    
        typedef pu_cpp_int< 1 > corecfg_sdmasupport_cpp_int_t;
        cpp_int int_var__corecfg_sdmasupport;
        void corecfg_sdmasupport (const cpp_int  & l__val);
        cpp_int corecfg_sdmasupport() const;
    
        typedef pu_cpp_int< 1 > corecfg_suspressupport_cpp_int_t;
        cpp_int int_var__corecfg_suspressupport;
        void corecfg_suspressupport (const cpp_int  & l__val);
        cpp_int corecfg_suspressupport() const;
    
        typedef pu_cpp_int< 1 > corecfg_3p3voltsupport_cpp_int_t;
        cpp_int int_var__corecfg_3p3voltsupport;
        void corecfg_3p3voltsupport (const cpp_int  & l__val);
        cpp_int corecfg_3p3voltsupport() const;
    
        typedef pu_cpp_int< 1 > corecfg_3p0voltsupport_cpp_int_t;
        cpp_int int_var__corecfg_3p0voltsupport;
        void corecfg_3p0voltsupport (const cpp_int  & l__val);
        cpp_int corecfg_3p0voltsupport() const;
    
        typedef pu_cpp_int< 1 > corecfg_1p8voltsupport_cpp_int_t;
        cpp_int int_var__corecfg_1p8voltsupport;
        void corecfg_1p8voltsupport (const cpp_int  & l__val);
        cpp_int corecfg_1p8voltsupport() const;
    
        typedef pu_cpp_int< 1 > rsvd_27_27_cpp_int_t;
        cpp_int int_var__rsvd_27_27;
        void rsvd_27_27 (const cpp_int  & l__val);
        cpp_int rsvd_27_27() const;
    
        typedef pu_cpp_int< 1 > corecfg_64bitsupport_cpp_int_t;
        cpp_int int_var__corecfg_64bitsupport;
        void corecfg_64bitsupport (const cpp_int  & l__val);
        cpp_int corecfg_64bitsupport() const;
    
        typedef pu_cpp_int< 1 > corecfg_asynchintrsupport_cpp_int_t;
        cpp_int int_var__corecfg_asynchintrsupport;
        void corecfg_asynchintrsupport (const cpp_int  & l__val);
        cpp_int corecfg_asynchintrsupport() const;
    
        typedef pu_cpp_int< 2 > corecfg_slottype_cpp_int_t;
        cpp_int int_var__corecfg_slottype;
        void corecfg_slottype (const cpp_int  & l__val);
        cpp_int corecfg_slottype() const;
    
        typedef pu_cpp_int< 1 > corecfg_sdr50support_cpp_int_t;
        cpp_int int_var__corecfg_sdr50support;
        void corecfg_sdr50support (const cpp_int  & l__val);
        cpp_int corecfg_sdr50support() const;
    
        typedef pu_cpp_int< 1 > corecfg_sdr104support_cpp_int_t;
        cpp_int int_var__corecfg_sdr104support;
        void corecfg_sdr104support (const cpp_int  & l__val);
        cpp_int corecfg_sdr104support() const;
    
        typedef pu_cpp_int< 1 > corecfg_ddr50support_cpp_int_t;
        cpp_int int_var__corecfg_ddr50support;
        void corecfg_ddr50support (const cpp_int  & l__val);
        cpp_int corecfg_ddr50support() const;
    
        typedef pu_cpp_int< 1 > rsvd_35_35_cpp_int_t;
        cpp_int int_var__rsvd_35_35;
        void rsvd_35_35 (const cpp_int  & l__val);
        cpp_int rsvd_35_35() const;
    
        typedef pu_cpp_int< 1 > corecfg_adriversupport_cpp_int_t;
        cpp_int int_var__corecfg_adriversupport;
        void corecfg_adriversupport (const cpp_int  & l__val);
        cpp_int corecfg_adriversupport() const;
    
        typedef pu_cpp_int< 1 > corecfg_cdriversupport_cpp_int_t;
        cpp_int int_var__corecfg_cdriversupport;
        void corecfg_cdriversupport (const cpp_int  & l__val);
        cpp_int corecfg_cdriversupport() const;
    
        typedef pu_cpp_int< 1 > corecfg_ddriversupport_cpp_int_t;
        cpp_int int_var__corecfg_ddriversupport;
        void corecfg_ddriversupport (const cpp_int  & l__val);
        cpp_int corecfg_ddriversupport() const;
    
        typedef pu_cpp_int< 1 > corecfg_type4support_cpp_int_t;
        cpp_int int_var__corecfg_type4support;
        void corecfg_type4support (const cpp_int  & l__val);
        cpp_int corecfg_type4support() const;
    
        typedef pu_cpp_int< 4 > corecfg_retuningtimercnt_cpp_int_t;
        cpp_int int_var__corecfg_retuningtimercnt;
        void corecfg_retuningtimercnt (const cpp_int  & l__val);
        cpp_int corecfg_retuningtimercnt() const;
    
        typedef pu_cpp_int< 1 > rsvd_44_44_cpp_int_t;
        cpp_int int_var__rsvd_44_44;
        void rsvd_44_44 (const cpp_int  & l__val);
        cpp_int rsvd_44_44() const;
    
        typedef pu_cpp_int< 1 > corecfg_tuningforsdr50_cpp_int_t;
        cpp_int int_var__corecfg_tuningforsdr50;
        void corecfg_tuningforsdr50 (const cpp_int  & l__val);
        cpp_int corecfg_tuningforsdr50() const;
    
        typedef pu_cpp_int< 2 > corecfg_retuningmodes_cpp_int_t;
        cpp_int int_var__corecfg_retuningmodes;
        void corecfg_retuningmodes (const cpp_int  & l__val);
        cpp_int corecfg_retuningmodes() const;
    
        typedef pu_cpp_int< 8 > corecfg_clockmultiplier_cpp_int_t;
        cpp_int int_var__corecfg_clockmultiplier;
        void corecfg_clockmultiplier (const cpp_int  & l__val);
        cpp_int corecfg_clockmultiplier() const;
    
        typedef pu_cpp_int< 1 > corecfg_spisupport_cpp_int_t;
        cpp_int int_var__corecfg_spisupport;
        void corecfg_spisupport (const cpp_int  & l__val);
        cpp_int corecfg_spisupport() const;
    
        typedef pu_cpp_int< 1 > corecfg_spiblkmode_cpp_int_t;
        cpp_int int_var__corecfg_spiblkmode;
        void corecfg_spiblkmode (const cpp_int  & l__val);
        cpp_int corecfg_spiblkmode() const;
    
        typedef pu_cpp_int< 5 > rsvd_62_58_cpp_int_t;
        cpp_int int_var__rsvd_62_58;
        void rsvd_62_58 (const cpp_int  & l__val);
        cpp_int rsvd_62_58() const;
    
        typedef pu_cpp_int< 1 > corecfg_hs400support_cpp_int_t;
        cpp_int int_var__corecfg_hs400support;
        void corecfg_hs400support (const cpp_int  & l__val);
        cpp_int corecfg_hs400support() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_capabilities_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_maxcurrentcap_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_maxcurrentcap_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_maxcurrentcap_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_maxcurrentcap_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_maxcurrentcap_t();
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
    
        typedef pu_cpp_int< 8 > corecfg_maxcurrent3p3v_cpp_int_t;
        cpp_int int_var__corecfg_maxcurrent3p3v;
        void corecfg_maxcurrent3p3v (const cpp_int  & l__val);
        cpp_int corecfg_maxcurrent3p3v() const;
    
        typedef pu_cpp_int< 8 > corecfg_maxcurrent3p0v_cpp_int_t;
        cpp_int int_var__corecfg_maxcurrent3p0v;
        void corecfg_maxcurrent3p0v (const cpp_int  & l__val);
        cpp_int corecfg_maxcurrent3p0v() const;
    
        typedef pu_cpp_int< 8 > corecfg_maxcurrent1p8v_cpp_int_t;
        cpp_int int_var__corecfg_maxcurrent1p8v;
        void corecfg_maxcurrent1p8v (const cpp_int  & l__val);
        cpp_int corecfg_maxcurrent1p8v() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_maxcurrentcap_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_ForceEventforAUTOCMDErrorStatus_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_ForceEventforAUTOCMDErrorStatus_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_ForceEventforAUTOCMDErrorStatus_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_ForceEventforAUTOCMDErrorStatus_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_ForceEventforAUTOCMDErrorStatus_t();
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
    
        typedef pu_cpp_int< 1 > forceautocmdnotexec_cpp_int_t;
        cpp_int int_var__forceautocmdnotexec;
        void forceautocmdnotexec (const cpp_int  & l__val);
        cpp_int forceautocmdnotexec() const;
    
        typedef pu_cpp_int< 1 > forceautocmdtimeouterr_cpp_int_t;
        cpp_int int_var__forceautocmdtimeouterr;
        void forceautocmdtimeouterr (const cpp_int  & l__val);
        cpp_int forceautocmdtimeouterr() const;
    
        typedef pu_cpp_int< 1 > forceautocmdcrcerr_cpp_int_t;
        cpp_int int_var__forceautocmdcrcerr;
        void forceautocmdcrcerr (const cpp_int  & l__val);
        cpp_int forceautocmdcrcerr() const;
    
        typedef pu_cpp_int< 1 > forceautocmdendbiterr_cpp_int_t;
        cpp_int int_var__forceautocmdendbiterr;
        void forceautocmdendbiterr (const cpp_int  & l__val);
        cpp_int forceautocmdendbiterr() const;
    
        typedef pu_cpp_int< 1 > forceautocmdindexerr_cpp_int_t;
        cpp_int int_var__forceautocmdindexerr;
        void forceautocmdindexerr (const cpp_int  & l__val);
        cpp_int forceautocmdindexerr() const;
    
        typedef pu_cpp_int< 2 > rsvd_6_5_cpp_int_t;
        cpp_int int_var__rsvd_6_5;
        void rsvd_6_5 (const cpp_int  & l__val);
        cpp_int rsvd_6_5() const;
    
        typedef pu_cpp_int< 1 > forcecmdnotissuedbyautocmd12err_cpp_int_t;
        cpp_int int_var__forcecmdnotissuedbyautocmd12err;
        void forcecmdnotissuedbyautocmd12err (const cpp_int  & l__val);
        cpp_int forcecmdnotissuedbyautocmd12err() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_ForceEventforAUTOCMDErrorStatus_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_forceeventforerrintsts_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_forceeventforerrintsts_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_forceeventforerrintsts_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_forceeventforerrintsts_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_forceeventforerrintsts_t();
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
    
        typedef pu_cpp_int< 1 > forcecmdtimeouterr_cpp_int_t;
        cpp_int int_var__forcecmdtimeouterr;
        void forcecmdtimeouterr (const cpp_int  & l__val);
        cpp_int forcecmdtimeouterr() const;
    
        typedef pu_cpp_int< 1 > forcecmdcrcerr_cpp_int_t;
        cpp_int int_var__forcecmdcrcerr;
        void forcecmdcrcerr (const cpp_int  & l__val);
        cpp_int forcecmdcrcerr() const;
    
        typedef pu_cpp_int< 1 > forcecmdendbiterr_cpp_int_t;
        cpp_int int_var__forcecmdendbiterr;
        void forcecmdendbiterr (const cpp_int  & l__val);
        cpp_int forcecmdendbiterr() const;
    
        typedef pu_cpp_int< 1 > forcecmdindexerr_cpp_int_t;
        cpp_int int_var__forcecmdindexerr;
        void forcecmdindexerr (const cpp_int  & l__val);
        cpp_int forcecmdindexerr() const;
    
        typedef pu_cpp_int< 1 > forcedattimeouterr_cpp_int_t;
        cpp_int int_var__forcedattimeouterr;
        void forcedattimeouterr (const cpp_int  & l__val);
        cpp_int forcedattimeouterr() const;
    
        typedef pu_cpp_int< 1 > forcedatcrcerr_cpp_int_t;
        cpp_int int_var__forcedatcrcerr;
        void forcedatcrcerr (const cpp_int  & l__val);
        cpp_int forcedatcrcerr() const;
    
        typedef pu_cpp_int< 1 > forcedatendbiterr_cpp_int_t;
        cpp_int int_var__forcedatendbiterr;
        void forcedatendbiterr (const cpp_int  & l__val);
        cpp_int forcedatendbiterr() const;
    
        typedef pu_cpp_int< 1 > forcecurrlimerr_cpp_int_t;
        cpp_int int_var__forcecurrlimerr;
        void forcecurrlimerr (const cpp_int  & l__val);
        cpp_int forcecurrlimerr() const;
    
        typedef pu_cpp_int< 1 > forceautocmderr_cpp_int_t;
        cpp_int int_var__forceautocmderr;
        void forceautocmderr (const cpp_int  & l__val);
        cpp_int forceautocmderr() const;
    
        typedef pu_cpp_int< 1 > forceadmaerr_cpp_int_t;
        cpp_int int_var__forceadmaerr;
        void forceadmaerr (const cpp_int  & l__val);
        cpp_int forceadmaerr() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_forceeventforerrintsts_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admaerrsts_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admaerrsts_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admaerrsts_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admaerrsts_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admaerrsts_t();
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
    
        typedef pu_cpp_int< 2 > admaerrsts_admaerrorstate_cpp_int_t;
        cpp_int int_var__admaerrsts_admaerrorstate;
        void admaerrsts_admaerrorstate (const cpp_int  & l__val);
        cpp_int admaerrsts_admaerrorstate() const;
    
        typedef pu_cpp_int< 1 > admaerrsts_admalenmismatcherr_cpp_int_t;
        cpp_int int_var__admaerrsts_admalenmismatcherr;
        void admaerrsts_admalenmismatcherr (const cpp_int  & l__val);
        cpp_int admaerrsts_admalenmismatcherr() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admaerrsts_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admasysaddr0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admasysaddr0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admasysaddr0_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admasysaddr0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admasysaddr0_t();
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
    
        typedef pu_cpp_int< 16 > adma_sysaddress0_cpp_int_t;
        cpp_int int_var__adma_sysaddress0;
        void adma_sysaddress0 (const cpp_int  & l__val);
        cpp_int adma_sysaddress0() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admasysaddr0_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admasysaddr1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admasysaddr1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admasysaddr1_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admasysaddr1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admasysaddr1_t();
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
    
        typedef pu_cpp_int< 16 > adma_sysaddress1_cpp_int_t;
        cpp_int int_var__adma_sysaddress1;
        void adma_sysaddress1 (const cpp_int  & l__val);
        cpp_int adma_sysaddress1() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admasysaddr1_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admasysaddr2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admasysaddr2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admasysaddr2_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admasysaddr2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admasysaddr2_t();
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
    
        typedef pu_cpp_int< 16 > adma_sysaddress2_cpp_int_t;
        cpp_int int_var__adma_sysaddress2;
        void adma_sysaddress2 (const cpp_int  & l__val);
        cpp_int adma_sysaddress2() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admasysaddr2_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admasysaddr3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admasysaddr3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admasysaddr3_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admasysaddr3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admasysaddr3_t();
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
    
        typedef pu_cpp_int< 16 > adma_sysaddress3_cpp_int_t;
        cpp_int int_var__adma_sysaddress3;
        void adma_sysaddress3 (const cpp_int  & l__val);
        cpp_int adma_sysaddress3() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admasysaddr3_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue0_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue0_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue0_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue0_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue0_t();
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
    
        typedef pu_cpp_int< 10 > SDCLKFrequencySelectValue_cpp_int_t;
        cpp_int int_var__SDCLKFrequencySelectValue;
        void SDCLKFrequencySelectValue (const cpp_int  & l__val);
        cpp_int SDCLKFrequencySelectValue() const;
    
        typedef pu_cpp_int< 1 > ClockGeneratorSelectValue_cpp_int_t;
        cpp_int int_var__ClockGeneratorSelectValue;
        void ClockGeneratorSelectValue (const cpp_int  & l__val);
        cpp_int ClockGeneratorSelectValue() const;
    
        typedef pu_cpp_int< 3 > rsvd_13_11_cpp_int_t;
        cpp_int int_var__rsvd_13_11;
        void rsvd_13_11 (const cpp_int  & l__val);
        cpp_int rsvd_13_11() const;
    
        typedef pu_cpp_int< 2 > DriverStrengthSelectValue_cpp_int_t;
        cpp_int int_var__DriverStrengthSelectValue;
        void DriverStrengthSelectValue (const cpp_int  & l__val);
        cpp_int DriverStrengthSelectValue() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue0_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue1_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue1_t();
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
    
        typedef pu_cpp_int< 10 > SDCLKFrequencySelectValue_cpp_int_t;
        cpp_int int_var__SDCLKFrequencySelectValue;
        void SDCLKFrequencySelectValue (const cpp_int  & l__val);
        cpp_int SDCLKFrequencySelectValue() const;
    
        typedef pu_cpp_int< 1 > ClockGeneratorSelectValue_cpp_int_t;
        cpp_int int_var__ClockGeneratorSelectValue;
        void ClockGeneratorSelectValue (const cpp_int  & l__val);
        cpp_int ClockGeneratorSelectValue() const;
    
        typedef pu_cpp_int< 3 > rsvd_13_11_cpp_int_t;
        cpp_int int_var__rsvd_13_11;
        void rsvd_13_11 (const cpp_int  & l__val);
        cpp_int rsvd_13_11() const;
    
        typedef pu_cpp_int< 2 > DriverStrengthSelectValue_cpp_int_t;
        cpp_int int_var__DriverStrengthSelectValue;
        void DriverStrengthSelectValue (const cpp_int  & l__val);
        cpp_int DriverStrengthSelectValue() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue1_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue2_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue2_t();
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
    
        typedef pu_cpp_int< 10 > SDCLKFrequencySelectValue_cpp_int_t;
        cpp_int int_var__SDCLKFrequencySelectValue;
        void SDCLKFrequencySelectValue (const cpp_int  & l__val);
        cpp_int SDCLKFrequencySelectValue() const;
    
        typedef pu_cpp_int< 1 > ClockGeneratorSelectValue_cpp_int_t;
        cpp_int int_var__ClockGeneratorSelectValue;
        void ClockGeneratorSelectValue (const cpp_int  & l__val);
        cpp_int ClockGeneratorSelectValue() const;
    
        typedef pu_cpp_int< 3 > rsvd_13_11_cpp_int_t;
        cpp_int int_var__rsvd_13_11;
        void rsvd_13_11 (const cpp_int  & l__val);
        cpp_int rsvd_13_11() const;
    
        typedef pu_cpp_int< 2 > DriverStrengthSelectValue_cpp_int_t;
        cpp_int int_var__DriverStrengthSelectValue;
        void DriverStrengthSelectValue (const cpp_int  & l__val);
        cpp_int DriverStrengthSelectValue() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue2_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue3_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue3_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue3_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue3_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue3_t();
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
    
        typedef pu_cpp_int< 10 > SDCLKFrequencySelectValue_cpp_int_t;
        cpp_int int_var__SDCLKFrequencySelectValue;
        void SDCLKFrequencySelectValue (const cpp_int  & l__val);
        cpp_int SDCLKFrequencySelectValue() const;
    
        typedef pu_cpp_int< 1 > ClockGeneratorSelectValue_cpp_int_t;
        cpp_int int_var__ClockGeneratorSelectValue;
        void ClockGeneratorSelectValue (const cpp_int  & l__val);
        cpp_int ClockGeneratorSelectValue() const;
    
        typedef pu_cpp_int< 3 > rsvd_13_11_cpp_int_t;
        cpp_int int_var__rsvd_13_11;
        void rsvd_13_11 (const cpp_int  & l__val);
        cpp_int rsvd_13_11() const;
    
        typedef pu_cpp_int< 2 > DriverStrengthSelectValue_cpp_int_t;
        cpp_int int_var__DriverStrengthSelectValue;
        void DriverStrengthSelectValue (const cpp_int  & l__val);
        cpp_int DriverStrengthSelectValue() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue3_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue4_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue4_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue4_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue4_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue4_t();
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
    
        typedef pu_cpp_int< 10 > SDCLKFrequencySelectValue_cpp_int_t;
        cpp_int int_var__SDCLKFrequencySelectValue;
        void SDCLKFrequencySelectValue (const cpp_int  & l__val);
        cpp_int SDCLKFrequencySelectValue() const;
    
        typedef pu_cpp_int< 1 > ClockGeneratorSelectValue_cpp_int_t;
        cpp_int int_var__ClockGeneratorSelectValue;
        void ClockGeneratorSelectValue (const cpp_int  & l__val);
        cpp_int ClockGeneratorSelectValue() const;
    
        typedef pu_cpp_int< 3 > rsvd_13_11_cpp_int_t;
        cpp_int int_var__rsvd_13_11;
        void rsvd_13_11 (const cpp_int  & l__val);
        cpp_int rsvd_13_11() const;
    
        typedef pu_cpp_int< 2 > DriverStrengthSelectValue_cpp_int_t;
        cpp_int int_var__DriverStrengthSelectValue;
        void DriverStrengthSelectValue (const cpp_int  & l__val);
        cpp_int DriverStrengthSelectValue() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue4_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue5_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue5_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue5_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue5_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue5_t();
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
    
        typedef pu_cpp_int< 10 > SDCLKFrequencySelectValue_cpp_int_t;
        cpp_int int_var__SDCLKFrequencySelectValue;
        void SDCLKFrequencySelectValue (const cpp_int  & l__val);
        cpp_int SDCLKFrequencySelectValue() const;
    
        typedef pu_cpp_int< 1 > ClockGeneratorSelectValue_cpp_int_t;
        cpp_int int_var__ClockGeneratorSelectValue;
        void ClockGeneratorSelectValue (const cpp_int  & l__val);
        cpp_int ClockGeneratorSelectValue() const;
    
        typedef pu_cpp_int< 3 > rsvd_13_11_cpp_int_t;
        cpp_int int_var__rsvd_13_11;
        void rsvd_13_11 (const cpp_int  & l__val);
        cpp_int rsvd_13_11() const;
    
        typedef pu_cpp_int< 2 > DriverStrengthSelectValue_cpp_int_t;
        cpp_int int_var__DriverStrengthSelectValue;
        void DriverStrengthSelectValue (const cpp_int  & l__val);
        cpp_int DriverStrengthSelectValue() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue5_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue6_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue6_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue6_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue6_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue6_t();
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
    
        typedef pu_cpp_int< 10 > SDCLKFrequencySelectValue_cpp_int_t;
        cpp_int int_var__SDCLKFrequencySelectValue;
        void SDCLKFrequencySelectValue (const cpp_int  & l__val);
        cpp_int SDCLKFrequencySelectValue() const;
    
        typedef pu_cpp_int< 1 > ClockGeneratorSelectValue_cpp_int_t;
        cpp_int int_var__ClockGeneratorSelectValue;
        void ClockGeneratorSelectValue (const cpp_int  & l__val);
        cpp_int ClockGeneratorSelectValue() const;
    
        typedef pu_cpp_int< 3 > rsvd_13_11_cpp_int_t;
        cpp_int int_var__rsvd_13_11;
        void rsvd_13_11 (const cpp_int  & l__val);
        cpp_int rsvd_13_11() const;
    
        typedef pu_cpp_int< 2 > DriverStrengthSelectValue_cpp_int_t;
        cpp_int int_var__DriverStrengthSelectValue;
        void DriverStrengthSelectValue (const cpp_int  & l__val);
        cpp_int DriverStrengthSelectValue() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue6_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue7_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue7_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue7_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue7_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue7_t();
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
    
        typedef pu_cpp_int< 10 > SDCLKFrequencySelectValue_cpp_int_t;
        cpp_int int_var__SDCLKFrequencySelectValue;
        void SDCLKFrequencySelectValue (const cpp_int  & l__val);
        cpp_int SDCLKFrequencySelectValue() const;
    
        typedef pu_cpp_int< 1 > ClockGeneratorSelectValue_cpp_int_t;
        cpp_int int_var__ClockGeneratorSelectValue;
        void ClockGeneratorSelectValue (const cpp_int  & l__val);
        cpp_int ClockGeneratorSelectValue() const;
    
        typedef pu_cpp_int< 3 > rsvd_13_11_cpp_int_t;
        cpp_int int_var__rsvd_13_11;
        void rsvd_13_11 (const cpp_int  & l__val);
        cpp_int rsvd_13_11() const;
    
        typedef pu_cpp_int< 2 > DriverStrengthSelectValue_cpp_int_t;
        cpp_int int_var__DriverStrengthSelectValue;
        void DriverStrengthSelectValue (const cpp_int  & l__val);
        cpp_int DriverStrengthSelectValue() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue7_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_boottimeoutcnt_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_boottimeoutcnt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_boottimeoutcnt_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_boottimeoutcnt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_boottimeoutcnt_t();
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
    
        typedef pu_cpp_int< 32 > boot_timeoutcnt_cpp_int_t;
        cpp_int int_var__boot_timeoutcnt;
        void boot_timeoutcnt (const cpp_int  & l__val);
        cpp_int boot_timeoutcnt() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_boottimeoutcnt_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue8_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue8_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue8_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue8_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue8_t();
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
    
        typedef pu_cpp_int< 10 > SDCLKFrequencySelectValue_cpp_int_t;
        cpp_int int_var__SDCLKFrequencySelectValue;
        void SDCLKFrequencySelectValue (const cpp_int  & l__val);
        cpp_int SDCLKFrequencySelectValue() const;
    
        typedef pu_cpp_int< 1 > ClockGeneratorSelectValue_cpp_int_t;
        cpp_int int_var__ClockGeneratorSelectValue;
        void ClockGeneratorSelectValue (const cpp_int  & l__val);
        cpp_int ClockGeneratorSelectValue() const;
    
        typedef pu_cpp_int< 3 > rsvd_13_11_cpp_int_t;
        cpp_int int_var__rsvd_13_11;
        void rsvd_13_11 (const cpp_int  & l__val);
        cpp_int rsvd_13_11() const;
    
        typedef pu_cpp_int< 2 > DriverStrengthSelectValue_cpp_int_t;
        cpp_int int_var__DriverStrengthSelectValue;
        void DriverStrengthSelectValue (const cpp_int  & l__val);
        cpp_int DriverStrengthSelectValue() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue8_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_vendorregister_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_vendorregister_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_vendorregister_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_vendorregister_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_vendorregister_t();
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
    
        typedef pu_cpp_int< 1 > vendor_enhancedstrobe_cpp_int_t;
        cpp_int int_var__vendor_enhancedstrobe;
        void vendor_enhancedstrobe (const cpp_int  & l__val);
        cpp_int vendor_enhancedstrobe() const;
    
        typedef pu_cpp_int< 1 > autogate_sdclk_cpp_int_t;
        cpp_int int_var__autogate_sdclk;
        void autogate_sdclk (const cpp_int  & l__val);
        cpp_int autogate_sdclk() const;
    
        typedef pu_cpp_int< 1 > delay_cmdin_datin_dis_cpp_int_t;
        cpp_int int_var__delay_cmdin_datin_dis;
        void delay_cmdin_datin_dis (const cpp_int  & l__val);
        cpp_int delay_cmdin_datin_dis() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_vendorregister_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_slotintrsts_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_slotintrsts_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_slotintrsts_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_slotintrsts_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_slotintrsts_t();
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
    
        typedef pu_cpp_int< 8 > sdhchostif_slotintrsts_cpp_int_t;
        cpp_int int_var__sdhchostif_slotintrsts;
        void sdhchostif_slotintrsts (const cpp_int  & l__val);
        cpp_int sdhchostif_slotintrsts() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_slotintrsts_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_hostcontrollerver_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_hostcontrollerver_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_hostcontrollerver_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_hostcontrollerver_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_hostcontrollerver_t();
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
    
        typedef pu_cpp_int< 8 > SpecificationVersionNumber_cpp_int_t;
        cpp_int int_var__SpecificationVersionNumber;
        void SpecificationVersionNumber (const cpp_int  & l__val);
        cpp_int SpecificationVersionNumber() const;
    
        typedef pu_cpp_int< 8 > SDHC_VENVERNUM_cpp_int_t;
        cpp_int int_var__SDHC_VENVERNUM;
        void SDHC_VENVERNUM (const cpp_int  & l__val);
        cpp_int SDHC_VENVERNUM() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_hostcontrollerver_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqversion_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqversion_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqversion_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqversion_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqversion_t();
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
    
        typedef pu_cpp_int< 4 > eMMC_Version_Suffix_cpp_int_t;
        cpp_int int_var__eMMC_Version_Suffix;
        void eMMC_Version_Suffix (const cpp_int  & l__val);
        cpp_int eMMC_Version_Suffix() const;
    
        typedef pu_cpp_int< 4 > eMMC_Minor_Version_Number_cpp_int_t;
        cpp_int int_var__eMMC_Minor_Version_Number;
        void eMMC_Minor_Version_Number (const cpp_int  & l__val);
        cpp_int eMMC_Minor_Version_Number() const;
    
        typedef pu_cpp_int< 4 > eMMC_Major_Version_Number_cpp_int_t;
        cpp_int int_var__eMMC_Major_Version_Number;
        void eMMC_Major_Version_Number (const cpp_int  & l__val);
        cpp_int eMMC_Major_Version_Number() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqversion_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqcapabilities_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqcapabilities_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqcapabilities_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqcapabilities_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqcapabilities_t();
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
    
        typedef pu_cpp_int< 10 > corecfg_cqfval_cpp_int_t;
        cpp_int int_var__corecfg_cqfval;
        void corecfg_cqfval (const cpp_int  & l__val);
        cpp_int corecfg_cqfval() const;
    
        typedef pu_cpp_int< 2 > rsvd_11_10_cpp_int_t;
        cpp_int int_var__rsvd_11_10;
        void rsvd_11_10 (const cpp_int  & l__val);
        cpp_int rsvd_11_10() const;
    
        typedef pu_cpp_int< 4 > corecfg_cqfmul_cpp_int_t;
        cpp_int int_var__corecfg_cqfmul;
        void corecfg_cqfmul (const cpp_int  & l__val);
        cpp_int corecfg_cqfmul() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqcapabilities_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqconfig_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqconfig_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqconfig_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqconfig_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqconfig_t();
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
    
        typedef pu_cpp_int< 1 > cqcfg_cqenable_cpp_int_t;
        cpp_int int_var__cqcfg_cqenable;
        void cqcfg_cqenable (const cpp_int  & l__val);
        cpp_int cqcfg_cqenable() const;
    
        typedef pu_cpp_int< 7 > rsvd_7_1_cpp_int_t;
        cpp_int int_var__rsvd_7_1;
        void rsvd_7_1 (const cpp_int  & l__val);
        cpp_int rsvd_7_1() const;
    
        typedef pu_cpp_int< 1 > cqcfg_taskdescsize_cpp_int_t;
        cpp_int int_var__cqcfg_taskdescsize;
        void cqcfg_taskdescsize (const cpp_int  & l__val);
        cpp_int cqcfg_taskdescsize() const;
    
        typedef pu_cpp_int< 3 > rsvd_11_9_cpp_int_t;
        cpp_int int_var__rsvd_11_9;
        void rsvd_11_9 (const cpp_int  & l__val);
        cpp_int rsvd_11_9() const;
    
        typedef pu_cpp_int< 1 > cqcfg_dcmdenable_cpp_int_t;
        cpp_int int_var__cqcfg_dcmdenable;
        void cqcfg_dcmdenable (const cpp_int  & l__val);
        cpp_int cqcfg_dcmdenable() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqconfig_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqcontrol_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqcontrol_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqcontrol_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqcontrol_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqcontrol_t();
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
    
        typedef pu_cpp_int< 1 > cqctrl_haltbit_cpp_int_t;
        cpp_int int_var__cqctrl_haltbit;
        void cqctrl_haltbit (const cpp_int  & l__val);
        cpp_int cqctrl_haltbit() const;
    
        typedef pu_cpp_int< 7 > rsvd_7_1_cpp_int_t;
        cpp_int int_var__rsvd_7_1;
        void rsvd_7_1 (const cpp_int  & l__val);
        cpp_int rsvd_7_1() const;
    
        typedef pu_cpp_int< 1 > cqctrl_clearalltasks_cpp_int_t;
        cpp_int int_var__cqctrl_clearalltasks;
        void cqctrl_clearalltasks (const cpp_int  & l__val);
        cpp_int cqctrl_clearalltasks() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqcontrol_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqintrsts_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqintrsts_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqintrsts_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqintrsts_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqintrsts_t();
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
    
        typedef pu_cpp_int< 1 > cqintrsts_haltcomplete_cpp_int_t;
        cpp_int int_var__cqintrsts_haltcomplete;
        void cqintrsts_haltcomplete (const cpp_int  & l__val);
        cpp_int cqintrsts_haltcomplete() const;
    
        typedef pu_cpp_int< 1 > cqintrsts_taskcomplete_cpp_int_t;
        cpp_int int_var__cqintrsts_taskcomplete;
        void cqintrsts_taskcomplete (const cpp_int  & l__val);
        cpp_int cqintrsts_taskcomplete() const;
    
        typedef pu_cpp_int< 1 > cqintrsts_resperrdet_cpp_int_t;
        cpp_int int_var__cqintrsts_resperrdet;
        void cqintrsts_resperrdet (const cpp_int  & l__val);
        cpp_int cqintrsts_resperrdet() const;
    
        typedef pu_cpp_int< 1 > cqintrsts_taskcleared_cpp_int_t;
        cpp_int int_var__cqintrsts_taskcleared;
        void cqintrsts_taskcleared (const cpp_int  & l__val);
        cpp_int cqintrsts_taskcleared() const;
    
        typedef pu_cpp_int< 1 > cqintrsts_taskerror_cpp_int_t;
        cpp_int int_var__cqintrsts_taskerror;
        void cqintrsts_taskerror (const cpp_int  & l__val);
        cpp_int cqintrsts_taskerror() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqintrsts_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqintrstsena_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqintrstsena_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqintrstsena_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqintrstsena_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqintrstsena_t();
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
    
        typedef pu_cpp_int< 1 > reg_cqintrstsena0_cpp_int_t;
        cpp_int int_var__reg_cqintrstsena0;
        void reg_cqintrstsena0 (const cpp_int  & l__val);
        cpp_int reg_cqintrstsena0() const;
    
        typedef pu_cpp_int< 1 > reg_cqintrstsena1_cpp_int_t;
        cpp_int int_var__reg_cqintrstsena1;
        void reg_cqintrstsena1 (const cpp_int  & l__val);
        cpp_int reg_cqintrstsena1() const;
    
        typedef pu_cpp_int< 1 > reg_cqintrstsena2_cpp_int_t;
        cpp_int int_var__reg_cqintrstsena2;
        void reg_cqintrstsena2 (const cpp_int  & l__val);
        cpp_int reg_cqintrstsena2() const;
    
        typedef pu_cpp_int< 1 > reg_cqintrstsena3_cpp_int_t;
        cpp_int int_var__reg_cqintrstsena3;
        void reg_cqintrstsena3 (const cpp_int  & l__val);
        cpp_int reg_cqintrstsena3() const;
    
        typedef pu_cpp_int< 1 > reg_cqintrstsena4_cpp_int_t;
        cpp_int int_var__reg_cqintrstsena4;
        void reg_cqintrstsena4 (const cpp_int  & l__val);
        cpp_int reg_cqintrstsena4() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqintrstsena_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqintrsigena_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqintrsigena_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqintrsigena_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqintrsigena_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqintrsigena_t();
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
    
        typedef pu_cpp_int< 1 > cqintrsig_enablereg0_cpp_int_t;
        cpp_int int_var__cqintrsig_enablereg0;
        void cqintrsig_enablereg0 (const cpp_int  & l__val);
        cpp_int cqintrsig_enablereg0() const;
    
        typedef pu_cpp_int< 1 > cqintrsig_enablereg1_cpp_int_t;
        cpp_int int_var__cqintrsig_enablereg1;
        void cqintrsig_enablereg1 (const cpp_int  & l__val);
        cpp_int cqintrsig_enablereg1() const;
    
        typedef pu_cpp_int< 1 > cqintrsig_enablereg2_cpp_int_t;
        cpp_int int_var__cqintrsig_enablereg2;
        void cqintrsig_enablereg2 (const cpp_int  & l__val);
        cpp_int cqintrsig_enablereg2() const;
    
        typedef pu_cpp_int< 1 > cqintrsig_enablereg3_cpp_int_t;
        cpp_int int_var__cqintrsig_enablereg3;
        void cqintrsig_enablereg3 (const cpp_int  & l__val);
        cpp_int cqintrsig_enablereg3() const;
    
        typedef pu_cpp_int< 1 > cqintrsig_enablereg4_cpp_int_t;
        cpp_int int_var__cqintrsig_enablereg4;
        void cqintrsig_enablereg4 (const cpp_int  & l__val);
        cpp_int cqintrsig_enablereg4() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqintrsigena_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqintrcoalescing_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqintrcoalescing_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqintrcoalescing_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqintrcoalescing_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqintrcoalescing_t();
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
    
        typedef pu_cpp_int< 7 > cqintrcoalescing_timeoutvalue_cpp_int_t;
        cpp_int int_var__cqintrcoalescing_timeoutvalue;
        void cqintrcoalescing_timeoutvalue (const cpp_int  & l__val);
        cpp_int cqintrcoalescing_timeoutvalue() const;
    
        typedef pu_cpp_int< 1 > rsvd_7_7_cpp_int_t;
        cpp_int int_var__rsvd_7_7;
        void rsvd_7_7 (const cpp_int  & l__val);
        cpp_int rsvd_7_7() const;
    
        typedef pu_cpp_int< 5 > cqintrcoalescing_ctrthreshold_cpp_int_t;
        cpp_int int_var__cqintrcoalescing_ctrthreshold;
        void cqintrcoalescing_ctrthreshold (const cpp_int  & l__val);
        cpp_int cqintrcoalescing_ctrthreshold() const;
    
        typedef pu_cpp_int< 7 > rsvd_19_13_cpp_int_t;
        cpp_int int_var__rsvd_19_13;
        void rsvd_19_13 (const cpp_int  & l__val);
        cpp_int rsvd_19_13() const;
    
        typedef pu_cpp_int< 1 > sdhccqctrl_icstatus_cpp_int_t;
        cpp_int int_var__sdhccqctrl_icstatus;
        void sdhccqctrl_icstatus (const cpp_int  & l__val);
        cpp_int sdhccqctrl_icstatus() const;
    
        typedef pu_cpp_int< 10 > rsvd_30_21_cpp_int_t;
        cpp_int int_var__rsvd_30_21;
        void rsvd_30_21 (const cpp_int  & l__val);
        cpp_int rsvd_30_21() const;
    
        typedef pu_cpp_int< 1 > cqintrcoalescing_enable_cpp_int_t;
        cpp_int int_var__cqintrcoalescing_enable;
        void cqintrcoalescing_enable (const cpp_int  & l__val);
        cpp_int cqintrcoalescing_enable() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqintrcoalescing_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtdlbaseaddresslo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtdlbaseaddresslo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtdlbaseaddresslo_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtdlbaseaddresslo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtdlbaseaddresslo_t();
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
    
        typedef pu_cpp_int< 32 > cqtdl_baseaddress_cpp_int_t;
        cpp_int int_var__cqtdl_baseaddress;
        void cqtdl_baseaddress (const cpp_int  & l__val);
        cpp_int cqtdl_baseaddress() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtdlbaseaddresslo_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtdlbaseaddresshi_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtdlbaseaddresshi_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtdlbaseaddresshi_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtdlbaseaddresshi_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtdlbaseaddresshi_t();
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
    
        typedef pu_cpp_int< 32 > cqtdl_baseaddress_cpp_int_t;
        cpp_int int_var__cqtdl_baseaddress;
        void cqtdl_baseaddress (const cpp_int  & l__val);
        cpp_int cqtdl_baseaddress() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtdlbaseaddresshi_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtaskdoorbell_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtaskdoorbell_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtaskdoorbell_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtaskdoorbell_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtaskdoorbell_t();
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
    
        typedef pu_cpp_int< 32 > cqtask_doorbell_cpp_int_t;
        cpp_int int_var__cqtask_doorbell;
        void cqtask_doorbell (const cpp_int  & l__val);
        cpp_int cqtask_doorbell() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtaskdoorbell_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtaskcplnotif_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtaskcplnotif_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtaskcplnotif_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtaskcplnotif_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtaskcplnotif_t();
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
    
        typedef pu_cpp_int< 32 > cqtask_cplnotif_cpp_int_t;
        cpp_int int_var__cqtask_cplnotif;
        void cqtask_cplnotif (const cpp_int  & l__val);
        cpp_int cqtask_cplnotif() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtaskcplnotif_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqdevqueuestatus_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqdevqueuestatus_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqdevqueuestatus_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqdevqueuestatus_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqdevqueuestatus_t();
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
    
        typedef pu_cpp_int< 32 > sdhccqctrl_devqueuestatus_cpp_int_t;
        cpp_int int_var__sdhccqctrl_devqueuestatus;
        void sdhccqctrl_devqueuestatus (const cpp_int  & l__val);
        cpp_int sdhccqctrl_devqueuestatus() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqdevqueuestatus_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqdevpendingtasks_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqdevpendingtasks_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqdevpendingtasks_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqdevpendingtasks_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqdevpendingtasks_t();
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
    
        typedef pu_cpp_int< 32 > sdhccqctrl_devpendingtasks_cpp_int_t;
        cpp_int int_var__sdhccqctrl_devpendingtasks;
        void sdhccqctrl_devpendingtasks (const cpp_int  & l__val);
        cpp_int sdhccqctrl_devpendingtasks() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqdevpendingtasks_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtaskclear_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtaskclear_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtaskclear_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtaskclear_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtaskclear_t();
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
    
        typedef pu_cpp_int< 32 > cqtask_clear_cpp_int_t;
        cpp_int int_var__cqtask_clear;
        void cqtask_clear (const cpp_int  & l__val);
        cpp_int cqtask_clear() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtaskclear_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqsendstsconfig1_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqsendstsconfig1_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqsendstsconfig1_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqsendstsconfig1_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqsendstsconfig1_t();
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
    
        typedef pu_cpp_int< 16 > cqsendsts_timer_cpp_int_t;
        cpp_int int_var__cqsendsts_timer;
        void cqsendsts_timer (const cpp_int  & l__val);
        cpp_int cqsendsts_timer() const;
    
        typedef pu_cpp_int< 4 > cqsendsts_blkcnt_cpp_int_t;
        cpp_int int_var__cqsendsts_blkcnt;
        void cqsendsts_blkcnt (const cpp_int  & l__val);
        cpp_int cqsendsts_blkcnt() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqsendstsconfig1_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqsendstsconfig2_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqsendstsconfig2_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqsendstsconfig2_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqsendstsconfig2_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqsendstsconfig2_t();
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
    
        typedef pu_cpp_int< 16 > cqsendsts_rca_cpp_int_t;
        cpp_int int_var__cqsendsts_rca;
        void cqsendsts_rca (const cpp_int  & l__val);
        cpp_int cqsendsts_rca() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqsendstsconfig2_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqdcmdresponse_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqdcmdresponse_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqdcmdresponse_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqdcmdresponse_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqdcmdresponse_t();
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
    
        typedef pu_cpp_int< 32 > sdhccqctrl_dcmdresponse_cpp_int_t;
        cpp_int int_var__sdhccqctrl_dcmdresponse;
        void sdhccqctrl_dcmdresponse (const cpp_int  & l__val);
        cpp_int sdhccqctrl_dcmdresponse() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqdcmdresponse_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqresperrmask_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqresperrmask_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqresperrmask_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqresperrmask_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqresperrmask_t();
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
    
        typedef pu_cpp_int< 32 > cqresp_errmask_cpp_int_t;
        cpp_int int_var__cqresp_errmask;
        void cqresp_errmask (const cpp_int  & l__val);
        cpp_int cqresp_errmask() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqresperrmask_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtaskerrinfo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtaskerrinfo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtaskerrinfo_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtaskerrinfo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtaskerrinfo_t();
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
    
        typedef pu_cpp_int< 6 > sdhccqctrl_cmderrcmdindex_cpp_int_t;
        cpp_int int_var__sdhccqctrl_cmderrcmdindex;
        void sdhccqctrl_cmderrcmdindex (const cpp_int  & l__val);
        cpp_int sdhccqctrl_cmderrcmdindex() const;
    
        typedef pu_cpp_int< 2 > rsvd_7_6_cpp_int_t;
        cpp_int int_var__rsvd_7_6;
        void rsvd_7_6 (const cpp_int  & l__val);
        cpp_int rsvd_7_6() const;
    
        typedef pu_cpp_int< 5 > sdhccqctrl_cmderrtaskid_cpp_int_t;
        cpp_int int_var__sdhccqctrl_cmderrtaskid;
        void sdhccqctrl_cmderrtaskid (const cpp_int  & l__val);
        cpp_int sdhccqctrl_cmderrtaskid() const;
    
        typedef pu_cpp_int< 2 > rsvd_14_13_cpp_int_t;
        cpp_int int_var__rsvd_14_13;
        void rsvd_14_13 (const cpp_int  & l__val);
        cpp_int rsvd_14_13() const;
    
        typedef pu_cpp_int< 1 > sdhccqctrl_cmderrvalid_cpp_int_t;
        cpp_int int_var__sdhccqctrl_cmderrvalid;
        void sdhccqctrl_cmderrvalid (const cpp_int  & l__val);
        cpp_int sdhccqctrl_cmderrvalid() const;
    
        typedef pu_cpp_int< 6 > sdhccqctrl_daterrcmdindex_cpp_int_t;
        cpp_int int_var__sdhccqctrl_daterrcmdindex;
        void sdhccqctrl_daterrcmdindex (const cpp_int  & l__val);
        cpp_int sdhccqctrl_daterrcmdindex() const;
    
        typedef pu_cpp_int< 2 > rsvd_23_22_cpp_int_t;
        cpp_int int_var__rsvd_23_22;
        void rsvd_23_22 (const cpp_int  & l__val);
        cpp_int rsvd_23_22() const;
    
        typedef pu_cpp_int< 5 > sdhccqctrl_daterrtaskid_cpp_int_t;
        cpp_int int_var__sdhccqctrl_daterrtaskid;
        void sdhccqctrl_daterrtaskid (const cpp_int  & l__val);
        cpp_int sdhccqctrl_daterrtaskid() const;
    
        typedef pu_cpp_int< 2 > rsvd_30_29_cpp_int_t;
        cpp_int int_var__rsvd_30_29;
        void rsvd_30_29 (const cpp_int  & l__val);
        cpp_int rsvd_30_29() const;
    
        typedef pu_cpp_int< 1 > sdhccqctrl_daterrvalid_cpp_int_t;
        cpp_int int_var__sdhccqctrl_daterrvalid;
        void sdhccqctrl_daterrvalid (const cpp_int  & l__val);
        cpp_int sdhccqctrl_daterrvalid() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtaskerrinfo_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqlastcmdindex_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqlastcmdindex_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqlastcmdindex_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqlastcmdindex_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqlastcmdindex_t();
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
    
        typedef pu_cpp_int< 6 > sdhccqctrl_lastcmdindex_cpp_int_t;
        cpp_int int_var__sdhccqctrl_lastcmdindex;
        void sdhccqctrl_lastcmdindex (const cpp_int  & l__val);
        cpp_int sdhccqctrl_lastcmdindex() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqlastcmdindex_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqlastcmdresponse_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqlastcmdresponse_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqlastcmdresponse_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqlastcmdresponse_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqlastcmdresponse_t();
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
    
        typedef pu_cpp_int< 32 > sdhccqctrl_lastcmdresponse_cpp_int_t;
        cpp_int int_var__sdhccqctrl_lastcmdresponse;
        void sdhccqctrl_lastcmdresponse (const cpp_int  & l__val);
        cpp_int sdhccqctrl_lastcmdresponse() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqlastcmdresponse_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqerrortaskid_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqerrortaskid_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqerrortaskid_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqerrortaskid_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqerrortaskid_t();
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
    
        typedef pu_cpp_int< 5 > sdhccqctrl_taskerrid_cpp_int_t;
        cpp_int int_var__sdhccqctrl_taskerrid;
        void sdhccqctrl_taskerrid (const cpp_int  & l__val);
        cpp_int sdhccqctrl_taskerrid() const;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqerrortaskid_t
    
class emmc_SDHOST_Memory_Map_SDHOST_Address_Block_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_t(string name = "emmc_SDHOST_Memory_Map_SDHOST_Address_Block_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_SDHOST_Address_Block_t();
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
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_sdmasysaddrlo_t reg_sdmasysaddrlo;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_sdmasysaddrhi_t reg_sdmasysaddrhi;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_blocksize_t reg_blocksize;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_blockcount_t reg_blockcount;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_argument1lo_t reg_argument1lo;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_argument1hi_t reg_argument1hi;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_transfermode_t reg_transfermode;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_command_t reg_command;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response0_t reg_response0;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response1_t reg_response1;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response2_t reg_response2;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response3_t reg_response3;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response4_t reg_response4;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response5_t reg_response5;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response6_t reg_response6;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_response7_t reg_response7;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_dataport_t reg_dataport;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presentstate_t reg_presentstate;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_hostcontrol1_t reg_hostcontrol1;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_powercontrol_t reg_powercontrol;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_blockgapcontrol_t reg_blockgapcontrol;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_wakeupcontrol_t reg_wakeupcontrol;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_clockcontrol_t reg_clockcontrol;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_timeoutcontrol_t reg_timeoutcontrol;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_softwarereset_t reg_softwarereset;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_normalintrsts_t reg_normalintrsts;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_errorintrsts_t reg_errorintrsts;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_normalintrstsena_t reg_normalintrstsena;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_errorintrstsena_t reg_errorintrstsena;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_normalintrsigena_t reg_normalintrsigena;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_errorintrsigena_t reg_errorintrsigena;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_autocmderrsts_t reg_autocmderrsts;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_hostcontrol2_t reg_hostcontrol2;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_capabilities_t reg_capabilities;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_maxcurrentcap_t reg_maxcurrentcap;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_ForceEventforAUTOCMDErrorStatus_t reg_ForceEventforAUTOCMDErrorStatus;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_forceeventforerrintsts_t reg_forceeventforerrintsts;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admaerrsts_t reg_admaerrsts;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admasysaddr0_t reg_admasysaddr0;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admasysaddr1_t reg_admasysaddr1;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admasysaddr2_t reg_admasysaddr2;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_admasysaddr3_t reg_admasysaddr3;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue0_t reg_presetvalue0;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue1_t reg_presetvalue1;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue2_t reg_presetvalue2;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue3_t reg_presetvalue3;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue4_t reg_presetvalue4;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue5_t reg_presetvalue5;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue6_t reg_presetvalue6;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue7_t reg_presetvalue7;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_boottimeoutcnt_t reg_boottimeoutcnt;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_presetvalue8_t reg_presetvalue8;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_vendorregister_t reg_vendorregister;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_slotintrsts_t reg_slotintrsts;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_hostcontrollerver_t reg_hostcontrollerver;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqversion_t reg_cqversion;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqcapabilities_t reg_cqcapabilities;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqconfig_t reg_cqconfig;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqcontrol_t reg_cqcontrol;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqintrsts_t reg_cqintrsts;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqintrstsena_t reg_cqintrstsena;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqintrsigena_t reg_cqintrsigena;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqintrcoalescing_t reg_cqintrcoalescing;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtdlbaseaddresslo_t reg_cqtdlbaseaddresslo;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtdlbaseaddresshi_t reg_cqtdlbaseaddresshi;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtaskdoorbell_t reg_cqtaskdoorbell;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtaskcplnotif_t reg_cqtaskcplnotif;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqdevqueuestatus_t reg_cqdevqueuestatus;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqdevpendingtasks_t reg_cqdevpendingtasks;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtaskclear_t reg_cqtaskclear;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqsendstsconfig1_t reg_cqsendstsconfig1;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqsendstsconfig2_t reg_cqsendstsconfig2;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqdcmdresponse_t reg_cqdcmdresponse;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqresperrmask_t reg_cqresperrmask;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqtaskerrinfo_t reg_cqtaskerrinfo;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqlastcmdindex_t reg_cqlastcmdindex;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqlastcmdresponse_t reg_cqlastcmdresponse;
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_reg_cqerrortaskid_t reg_cqerrortaskid;
    
}; // emmc_SDHOST_Memory_Map_SDHOST_Address_Block_t
    
class emmc_SDHOST_Memory_Map_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        emmc_SDHOST_Memory_Map_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        emmc_SDHOST_Memory_Map_t(string name = "emmc_SDHOST_Memory_Map_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~emmc_SDHOST_Memory_Map_t();
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
    
        emmc_SDHOST_Memory_Map_SDHOST_Address_Block_t SDHOST_Address_Block;
    
}; // emmc_SDHOST_Memory_Map_t
    
#endif // EMMC_SDHOST_MEMORY_MAP_H
        