
#ifndef CAP_DPRMEM_CSR_H
#define CAP_DPRMEM_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t(string name = "cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t();
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
    
        typedef pu_cpp_int< 521 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 36 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t
    
class cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_t(string name = "cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_t();
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
        cap_csr_large_array_wrapper<cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t, 16> entry;
        #else 
        cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t entry[16];
        #endif
        int get_depth_entry() { return 16; }
    
}; // cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_t
    
class cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t(string name = "cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t();
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
    
        typedef pu_cpp_int< 1218 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 72 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t
    
class cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_t(string name = "cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_t();
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
    
        #if 32 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t, 32> entry;
        #else 
        cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t entry[32];
        #endif
        int get_depth_entry() { return 32; }
    
}; // cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_t
    
class cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t(string name = "cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t();
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
    
        typedef pu_cpp_int< 515 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 20 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t
    
class cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_t(string name = "cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_t();
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
        cap_csr_large_array_wrapper<cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t, 16> entry;
        #else 
        cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t entry[16];
        #endif
        int get_depth_entry() { return 16; }
    
}; // cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_t
    
class cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t(string name = "cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t();
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
    
        typedef pu_cpp_int< 515 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 36 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t
    
class cap_dprmem_csr_dhs_dpr_phv_fifo_sram_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_dhs_dpr_phv_fifo_sram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_dhs_dpr_phv_fifo_sram_t(string name = "cap_dprmem_csr_dhs_dpr_phv_fifo_sram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_dhs_dpr_phv_fifo_sram_t();
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
    
        #if 64 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t, 64> entry;
        #else 
        cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t entry[64];
        #endif
        int get_depth_entry() { return 64; }
    
}; // cap_dprmem_csr_dhs_dpr_phv_fifo_sram_t
    
class cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t(string name = "cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t();
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
    
        typedef pu_cpp_int< 521 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 36 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t
    
class cap_dprmem_csr_dhs_dpr_csum_fifo_sram_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_dhs_dpr_csum_fifo_sram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_dhs_dpr_csum_fifo_sram_t(string name = "cap_dprmem_csr_dhs_dpr_csum_fifo_sram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_dhs_dpr_csum_fifo_sram_t();
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
    
        #if 512 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t, 512> entry;
        #else 
        cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t entry[512];
        #endif
        int get_depth_entry() { return 512; }
    
}; // cap_dprmem_csr_dhs_dpr_csum_fifo_sram_t
    
class cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t(string name = "cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t();
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
    
        typedef pu_cpp_int< 521 > data_cpp_int_t;
        cpp_int int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 36 > ecc_cpp_int_t;
        cpp_int int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t
    
class cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_t : public cap_memory_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_t(string name = "cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_t();
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
    
        #if 832 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t, 832> entry;
        #else 
        cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t entry[832];
        #endif
        int get_depth_entry() { return 832; }
    
}; // cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_t
    
class cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t(string name = "cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t();
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
    
        typedef pu_cpp_int< 1 > fifo_ptr_written_cpp_int_t;
        cpp_int int_var__fifo_ptr_written;
        void fifo_ptr_written (const cpp_int  & l__val);
        cpp_int fifo_ptr_written() const;
    
        typedef pu_cpp_int< 4 > fifo_ptr_wptr_cpp_int_t;
        cpp_int int_var__fifo_ptr_wptr;
        void fifo_ptr_wptr (const cpp_int  & l__val);
        cpp_int fifo_ptr_wptr() const;
    
        typedef pu_cpp_int< 4 > fifo_ptr_rptr_cpp_int_t;
        cpp_int int_var__fifo_ptr_rptr;
        void fifo_ptr_rptr (const cpp_int  & l__val);
        cpp_int fifo_ptr_rptr() const;
    
        typedef pu_cpp_int< 1 > fifo_offline_cpp_int_t;
        cpp_int int_var__fifo_offline;
        void fifo_offline (const cpp_int  & l__val);
        cpp_int fifo_offline() const;
    
        typedef pu_cpp_int< 1 > fifo_flush_cpp_int_t;
        cpp_int int_var__fifo_flush;
        void fifo_flush (const cpp_int  & l__val);
        cpp_int fifo_flush() const;
    
}; // cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t
    
class cap_dprmem_csr_sta_ff_ptr_dpr_pktout_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_sta_ff_ptr_dpr_pktout_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_sta_ff_ptr_dpr_pktout_fifo_t(string name = "cap_dprmem_csr_sta_ff_ptr_dpr_pktout_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_sta_ff_ptr_dpr_pktout_fifo_t();
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
    
        typedef pu_cpp_int< 4 > sta_ptr_wptr_cpp_int_t;
        cpp_int int_var__sta_ptr_wptr;
        void sta_ptr_wptr (const cpp_int  & l__val);
        cpp_int sta_ptr_wptr() const;
    
        typedef pu_cpp_int< 4 > sta_ptr_rptr_cpp_int_t;
        cpp_int int_var__sta_ptr_rptr;
        void sta_ptr_rptr (const cpp_int  & l__val);
        cpp_int sta_ptr_rptr() const;
    
        typedef pu_cpp_int< 1 > sta_empty_cpp_int_t;
        cpp_int int_var__sta_empty;
        void sta_empty (const cpp_int  & l__val);
        cpp_int sta_empty() const;
    
        typedef pu_cpp_int< 1 > sta_full_cpp_int_t;
        cpp_int int_var__sta_full;
        void sta_full (const cpp_int  & l__val);
        cpp_int sta_full() const;
    
}; // cap_dprmem_csr_sta_ff_ptr_dpr_pktout_fifo_t
    
class cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t(string name = "cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 36 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 4 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
}; // cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t
    
class cap_dprmem_csr_cfg_dpr_pktout_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_cfg_dpr_pktout_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_cfg_dpr_pktout_fifo_t(string name = "cap_dprmem_csr_cfg_dpr_pktout_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_cfg_dpr_pktout_fifo_t();
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
    
        typedef pu_cpp_int< 1 > ecc_detect_cpp_int_t;
        cpp_int int_var__ecc_detect;
        void ecc_detect (const cpp_int  & l__val);
        cpp_int ecc_detect() const;
    
        typedef pu_cpp_int< 1 > ecc_correct_cpp_int_t;
        cpp_int int_var__ecc_correct;
        void ecc_correct (const cpp_int  & l__val);
        cpp_int ecc_correct() const;
    
        typedef pu_cpp_int< 1 > ecc_bypass_cpp_int_t;
        cpp_int int_var__ecc_bypass;
        void ecc_bypass (const cpp_int  & l__val);
        cpp_int ecc_bypass() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_dprmem_csr_cfg_dpr_pktout_fifo_t
    
class cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t(string name = "cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t();
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
    
        typedef pu_cpp_int< 1 > fifo_ptr_written_cpp_int_t;
        cpp_int int_var__fifo_ptr_written;
        void fifo_ptr_written (const cpp_int  & l__val);
        cpp_int fifo_ptr_written() const;
    
        typedef pu_cpp_int< 5 > fifo_ptr_wptr_cpp_int_t;
        cpp_int int_var__fifo_ptr_wptr;
        void fifo_ptr_wptr (const cpp_int  & l__val);
        cpp_int fifo_ptr_wptr() const;
    
        typedef pu_cpp_int< 5 > fifo_ptr_rptr_cpp_int_t;
        cpp_int int_var__fifo_ptr_rptr;
        void fifo_ptr_rptr (const cpp_int  & l__val);
        cpp_int fifo_ptr_rptr() const;
    
        typedef pu_cpp_int< 1 > fifo_offline_cpp_int_t;
        cpp_int int_var__fifo_offline;
        void fifo_offline (const cpp_int  & l__val);
        cpp_int fifo_offline() const;
    
        typedef pu_cpp_int< 1 > fifo_flush_cpp_int_t;
        cpp_int int_var__fifo_flush;
        void fifo_flush (const cpp_int  & l__val);
        cpp_int fifo_flush() const;
    
}; // cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t
    
class cap_dprmem_csr_sta_ff_ptr_dpr_ptr_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_sta_ff_ptr_dpr_ptr_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_sta_ff_ptr_dpr_ptr_fifo_t(string name = "cap_dprmem_csr_sta_ff_ptr_dpr_ptr_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_sta_ff_ptr_dpr_ptr_fifo_t();
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
    
        typedef pu_cpp_int< 5 > sta_ptr_wptr_cpp_int_t;
        cpp_int int_var__sta_ptr_wptr;
        void sta_ptr_wptr (const cpp_int  & l__val);
        cpp_int sta_ptr_wptr() const;
    
        typedef pu_cpp_int< 5 > sta_ptr_rptr_cpp_int_t;
        cpp_int int_var__sta_ptr_rptr;
        void sta_ptr_rptr (const cpp_int  & l__val);
        cpp_int sta_ptr_rptr() const;
    
        typedef pu_cpp_int< 1 > sta_empty_cpp_int_t;
        cpp_int int_var__sta_empty;
        void sta_empty (const cpp_int  & l__val);
        cpp_int sta_empty() const;
    
        typedef pu_cpp_int< 1 > sta_full_cpp_int_t;
        cpp_int int_var__sta_full;
        void sta_full (const cpp_int  & l__val);
        cpp_int sta_full() const;
    
}; // cap_dprmem_csr_sta_ff_ptr_dpr_ptr_fifo_t
    
class cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t(string name = "cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 72 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 5 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
}; // cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t
    
class cap_dprmem_csr_cfg_dpr_ptr_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_cfg_dpr_ptr_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_cfg_dpr_ptr_fifo_t(string name = "cap_dprmem_csr_cfg_dpr_ptr_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_cfg_dpr_ptr_fifo_t();
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
    
        typedef pu_cpp_int< 1 > ecc_detect_cpp_int_t;
        cpp_int int_var__ecc_detect;
        void ecc_detect (const cpp_int  & l__val);
        cpp_int ecc_detect() const;
    
        typedef pu_cpp_int< 1 > ecc_correct_cpp_int_t;
        cpp_int int_var__ecc_correct;
        void ecc_correct (const cpp_int  & l__val);
        cpp_int ecc_correct() const;
    
        typedef pu_cpp_int< 1 > ecc_bypass_cpp_int_t;
        cpp_int int_var__ecc_bypass;
        void ecc_bypass (const cpp_int  & l__val);
        cpp_int ecc_bypass() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_dprmem_csr_cfg_dpr_ptr_fifo_t
    
class cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t(string name = "cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t();
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
    
        typedef pu_cpp_int< 1 > fifo_ptr_written_cpp_int_t;
        cpp_int int_var__fifo_ptr_written;
        void fifo_ptr_written (const cpp_int  & l__val);
        cpp_int fifo_ptr_written() const;
    
        typedef pu_cpp_int< 4 > fifo_ptr_wptr_cpp_int_t;
        cpp_int int_var__fifo_ptr_wptr;
        void fifo_ptr_wptr (const cpp_int  & l__val);
        cpp_int fifo_ptr_wptr() const;
    
        typedef pu_cpp_int< 4 > fifo_ptr_rptr_cpp_int_t;
        cpp_int int_var__fifo_ptr_rptr;
        void fifo_ptr_rptr (const cpp_int  & l__val);
        cpp_int fifo_ptr_rptr() const;
    
        typedef pu_cpp_int< 1 > fifo_offline_cpp_int_t;
        cpp_int int_var__fifo_offline;
        void fifo_offline (const cpp_int  & l__val);
        cpp_int fifo_offline() const;
    
        typedef pu_cpp_int< 1 > fifo_flush_cpp_int_t;
        cpp_int int_var__fifo_flush;
        void fifo_flush (const cpp_int  & l__val);
        cpp_int fifo_flush() const;
    
}; // cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t
    
class cap_dprmem_csr_sta_ff_ptr_dpr_ohi_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_sta_ff_ptr_dpr_ohi_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_sta_ff_ptr_dpr_ohi_fifo_t(string name = "cap_dprmem_csr_sta_ff_ptr_dpr_ohi_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_sta_ff_ptr_dpr_ohi_fifo_t();
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
    
        typedef pu_cpp_int< 4 > sta_ptr_wptr_cpp_int_t;
        cpp_int int_var__sta_ptr_wptr;
        void sta_ptr_wptr (const cpp_int  & l__val);
        cpp_int sta_ptr_wptr() const;
    
        typedef pu_cpp_int< 4 > sta_ptr_rptr_cpp_int_t;
        cpp_int int_var__sta_ptr_rptr;
        void sta_ptr_rptr (const cpp_int  & l__val);
        cpp_int sta_ptr_rptr() const;
    
        typedef pu_cpp_int< 1 > sta_empty_cpp_int_t;
        cpp_int int_var__sta_empty;
        void sta_empty (const cpp_int  & l__val);
        cpp_int sta_empty() const;
    
        typedef pu_cpp_int< 1 > sta_full_cpp_int_t;
        cpp_int int_var__sta_full;
        void sta_full (const cpp_int  & l__val);
        cpp_int sta_full() const;
    
}; // cap_dprmem_csr_sta_ff_ptr_dpr_ohi_fifo_t
    
class cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t(string name = "cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 20 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 4 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
}; // cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t
    
class cap_dprmem_csr_cfg_dpr_ohi_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_cfg_dpr_ohi_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_cfg_dpr_ohi_fifo_t(string name = "cap_dprmem_csr_cfg_dpr_ohi_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_cfg_dpr_ohi_fifo_t();
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
    
        typedef pu_cpp_int< 1 > ecc_detect_cpp_int_t;
        cpp_int int_var__ecc_detect;
        void ecc_detect (const cpp_int  & l__val);
        cpp_int ecc_detect() const;
    
        typedef pu_cpp_int< 1 > ecc_correct_cpp_int_t;
        cpp_int int_var__ecc_correct;
        void ecc_correct (const cpp_int  & l__val);
        cpp_int ecc_correct() const;
    
        typedef pu_cpp_int< 1 > ecc_bypass_cpp_int_t;
        cpp_int int_var__ecc_bypass;
        void ecc_bypass (const cpp_int  & l__val);
        cpp_int ecc_bypass() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_dprmem_csr_cfg_dpr_ohi_fifo_t
    
class cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t(string name = "cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t();
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
    
        typedef pu_cpp_int< 1 > fifo_ptr_written_cpp_int_t;
        cpp_int int_var__fifo_ptr_written;
        void fifo_ptr_written (const cpp_int  & l__val);
        cpp_int fifo_ptr_written() const;
    
        typedef pu_cpp_int< 6 > fifo_ptr_wptr_cpp_int_t;
        cpp_int int_var__fifo_ptr_wptr;
        void fifo_ptr_wptr (const cpp_int  & l__val);
        cpp_int fifo_ptr_wptr() const;
    
        typedef pu_cpp_int< 6 > fifo_ptr_rptr_cpp_int_t;
        cpp_int int_var__fifo_ptr_rptr;
        void fifo_ptr_rptr (const cpp_int  & l__val);
        cpp_int fifo_ptr_rptr() const;
    
        typedef pu_cpp_int< 1 > fifo_offline_cpp_int_t;
        cpp_int int_var__fifo_offline;
        void fifo_offline (const cpp_int  & l__val);
        cpp_int fifo_offline() const;
    
        typedef pu_cpp_int< 1 > fifo_flush_cpp_int_t;
        cpp_int int_var__fifo_flush;
        void fifo_flush (const cpp_int  & l__val);
        cpp_int fifo_flush() const;
    
}; // cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t
    
class cap_dprmem_csr_sta_ff_ptr_dpr_phv_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_sta_ff_ptr_dpr_phv_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_sta_ff_ptr_dpr_phv_fifo_t(string name = "cap_dprmem_csr_sta_ff_ptr_dpr_phv_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_sta_ff_ptr_dpr_phv_fifo_t();
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
    
        typedef pu_cpp_int< 6 > sta_ptr_wptr_cpp_int_t;
        cpp_int int_var__sta_ptr_wptr;
        void sta_ptr_wptr (const cpp_int  & l__val);
        cpp_int sta_ptr_wptr() const;
    
        typedef pu_cpp_int< 6 > sta_ptr_rptr_cpp_int_t;
        cpp_int int_var__sta_ptr_rptr;
        void sta_ptr_rptr (const cpp_int  & l__val);
        cpp_int sta_ptr_rptr() const;
    
        typedef pu_cpp_int< 1 > sta_empty_cpp_int_t;
        cpp_int int_var__sta_empty;
        void sta_empty (const cpp_int  & l__val);
        cpp_int sta_empty() const;
    
        typedef pu_cpp_int< 1 > sta_full_cpp_int_t;
        cpp_int int_var__sta_full;
        void sta_full (const cpp_int  & l__val);
        cpp_int sta_full() const;
    
}; // cap_dprmem_csr_sta_ff_ptr_dpr_phv_fifo_t
    
class cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t(string name = "cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 36 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 6 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
}; // cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t
    
class cap_dprmem_csr_cfg_dpr_phv_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_cfg_dpr_phv_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_cfg_dpr_phv_fifo_t(string name = "cap_dprmem_csr_cfg_dpr_phv_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_cfg_dpr_phv_fifo_t();
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
    
        typedef pu_cpp_int< 1 > ecc_detect_cpp_int_t;
        cpp_int int_var__ecc_detect;
        void ecc_detect (const cpp_int  & l__val);
        cpp_int ecc_detect() const;
    
        typedef pu_cpp_int< 1 > ecc_correct_cpp_int_t;
        cpp_int int_var__ecc_correct;
        void ecc_correct (const cpp_int  & l__val);
        cpp_int ecc_correct() const;
    
        typedef pu_cpp_int< 1 > ecc_bypass_cpp_int_t;
        cpp_int int_var__ecc_bypass;
        void ecc_bypass (const cpp_int  & l__val);
        cpp_int ecc_bypass() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_dprmem_csr_cfg_dpr_phv_fifo_t
    
class cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t(string name = "cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t();
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
    
        typedef pu_cpp_int< 1 > fifo_ptr_written_cpp_int_t;
        cpp_int int_var__fifo_ptr_written;
        void fifo_ptr_written (const cpp_int  & l__val);
        cpp_int fifo_ptr_written() const;
    
        typedef pu_cpp_int< 9 > fifo_ptr_wptr_cpp_int_t;
        cpp_int int_var__fifo_ptr_wptr;
        void fifo_ptr_wptr (const cpp_int  & l__val);
        cpp_int fifo_ptr_wptr() const;
    
        typedef pu_cpp_int< 9 > fifo_ptr_rptr_cpp_int_t;
        cpp_int int_var__fifo_ptr_rptr;
        void fifo_ptr_rptr (const cpp_int  & l__val);
        cpp_int fifo_ptr_rptr() const;
    
        typedef pu_cpp_int< 1 > fifo_offline_cpp_int_t;
        cpp_int int_var__fifo_offline;
        void fifo_offline (const cpp_int  & l__val);
        cpp_int fifo_offline() const;
    
        typedef pu_cpp_int< 1 > fifo_flush_cpp_int_t;
        cpp_int int_var__fifo_flush;
        void fifo_flush (const cpp_int  & l__val);
        cpp_int fifo_flush() const;
    
}; // cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t
    
class cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t(string name = "cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t();
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
    
        typedef pu_cpp_int< 9 > sta_ptr_wptr_cpp_int_t;
        cpp_int int_var__sta_ptr_wptr;
        void sta_ptr_wptr (const cpp_int  & l__val);
        cpp_int sta_ptr_wptr() const;
    
        typedef pu_cpp_int< 9 > sta_ptr_rptr_cpp_int_t;
        cpp_int int_var__sta_ptr_rptr;
        void sta_ptr_rptr (const cpp_int  & l__val);
        cpp_int sta_ptr_rptr() const;
    
        typedef pu_cpp_int< 1 > sta_empty_cpp_int_t;
        cpp_int int_var__sta_empty;
        void sta_empty (const cpp_int  & l__val);
        cpp_int sta_empty() const;
    
        typedef pu_cpp_int< 1 > sta_full_cpp_int_t;
        cpp_int int_var__sta_full;
        void sta_full (const cpp_int  & l__val);
        cpp_int sta_full() const;
    
}; // cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t
    
class cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t(string name = "cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 36 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 9 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
}; // cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t
    
class cap_dprmem_csr_cfg_dpr_csum_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_cfg_dpr_csum_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_cfg_dpr_csum_fifo_t(string name = "cap_dprmem_csr_cfg_dpr_csum_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_cfg_dpr_csum_fifo_t();
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
    
        typedef pu_cpp_int< 1 > ecc_detect_cpp_int_t;
        cpp_int int_var__ecc_detect;
        void ecc_detect (const cpp_int  & l__val);
        cpp_int ecc_detect() const;
    
        typedef pu_cpp_int< 1 > ecc_correct_cpp_int_t;
        cpp_int int_var__ecc_correct;
        void ecc_correct (const cpp_int  & l__val);
        cpp_int ecc_correct() const;
    
        typedef pu_cpp_int< 1 > ecc_bypass_cpp_int_t;
        cpp_int int_var__ecc_bypass;
        void ecc_bypass (const cpp_int  & l__val);
        cpp_int ecc_bypass() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_dprmem_csr_cfg_dpr_csum_fifo_t
    
class cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t(string name = "cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t();
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
    
        typedef pu_cpp_int< 1 > fifo_ptr_written_cpp_int_t;
        cpp_int int_var__fifo_ptr_written;
        void fifo_ptr_written (const cpp_int  & l__val);
        cpp_int fifo_ptr_written() const;
    
        typedef pu_cpp_int< 10 > fifo_ptr_wptr_cpp_int_t;
        cpp_int int_var__fifo_ptr_wptr;
        void fifo_ptr_wptr (const cpp_int  & l__val);
        cpp_int fifo_ptr_wptr() const;
    
        typedef pu_cpp_int< 10 > fifo_ptr_rptr_cpp_int_t;
        cpp_int int_var__fifo_ptr_rptr;
        void fifo_ptr_rptr (const cpp_int  & l__val);
        cpp_int fifo_ptr_rptr() const;
    
        typedef pu_cpp_int< 1 > fifo_offline_cpp_int_t;
        cpp_int int_var__fifo_offline;
        void fifo_offline (const cpp_int  & l__val);
        cpp_int fifo_offline() const;
    
        typedef pu_cpp_int< 1 > fifo_flush_cpp_int_t;
        cpp_int int_var__fifo_flush;
        void fifo_flush (const cpp_int  & l__val);
        cpp_int fifo_flush() const;
    
}; // cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t
    
class cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t(string name = "cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t();
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
    
        typedef pu_cpp_int< 10 > sta_ptr_wptr_cpp_int_t;
        cpp_int int_var__sta_ptr_wptr;
        void sta_ptr_wptr (const cpp_int  & l__val);
        cpp_int sta_ptr_wptr() const;
    
        typedef pu_cpp_int< 10 > sta_ptr_rptr_cpp_int_t;
        cpp_int int_var__sta_ptr_rptr;
        void sta_ptr_rptr (const cpp_int  & l__val);
        cpp_int sta_ptr_rptr() const;
    
        typedef pu_cpp_int< 1 > sta_empty_cpp_int_t;
        cpp_int int_var__sta_empty;
        void sta_empty (const cpp_int  & l__val);
        cpp_int sta_empty() const;
    
        typedef pu_cpp_int< 1 > sta_full_cpp_int_t;
        cpp_int int_var__sta_full;
        void sta_full (const cpp_int  & l__val);
        cpp_int sta_full() const;
    
}; // cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t
    
class cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t(string name = "cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t();
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
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        cpp_int int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        cpp_int int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 36 > syndrome_cpp_int_t;
        cpp_int int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 10 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 1 > bist_done_fail_cpp_int_t;
        cpp_int int_var__bist_done_fail;
        void bist_done_fail (const cpp_int  & l__val);
        cpp_int bist_done_fail() const;
    
        typedef pu_cpp_int< 1 > bist_done_pass_cpp_int_t;
        cpp_int int_var__bist_done_pass;
        void bist_done_pass (const cpp_int  & l__val);
        cpp_int bist_done_pass() const;
    
}; // cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t
    
class cap_dprmem_csr_cfg_dpr_pktin_fifo_t : public cap_register_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_cfg_dpr_pktin_fifo_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_cfg_dpr_pktin_fifo_t(string name = "cap_dprmem_csr_cfg_dpr_pktin_fifo_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_cfg_dpr_pktin_fifo_t();
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
    
        typedef pu_cpp_int< 1 > ecc_detect_cpp_int_t;
        cpp_int int_var__ecc_detect;
        void ecc_detect (const cpp_int  & l__val);
        cpp_int ecc_detect() const;
    
        typedef pu_cpp_int< 1 > ecc_correct_cpp_int_t;
        cpp_int int_var__ecc_correct;
        void ecc_correct (const cpp_int  & l__val);
        cpp_int ecc_correct() const;
    
        typedef pu_cpp_int< 1 > ecc_bypass_cpp_int_t;
        cpp_int int_var__ecc_bypass;
        void ecc_bypass (const cpp_int  & l__val);
        cpp_int ecc_bypass() const;
    
        typedef pu_cpp_int< 1 > bist_run_cpp_int_t;
        cpp_int int_var__bist_run;
        void bist_run (const cpp_int  & l__val);
        cpp_int bist_run() const;
    
}; // cap_dprmem_csr_cfg_dpr_pktin_fifo_t
    
class cap_dprmem_csr_t : public cap_block_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_dprmem_csr_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_dprmem_csr_t(string name = "cap_dprmem_csr_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_dprmem_csr_t();
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
    
        cap_dprmem_csr_cfg_dpr_pktin_fifo_t cfg_dpr_pktin_fifo;
    
        cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t sta_srams_ecc_dpr_pktin_fifo;
    
        cap_dprmem_csr_sta_ff_ptr_dpr_pktin_fifo_t sta_ff_ptr_dpr_pktin_fifo;
    
        cap_dprmem_csr_cfw_ff_dpr_pktin_fifo_t cfw_ff_dpr_pktin_fifo;
    
        cap_dprmem_csr_cfg_dpr_csum_fifo_t cfg_dpr_csum_fifo;
    
        cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t sta_srams_ecc_dpr_csum_fifo;
    
        cap_dprmem_csr_sta_ff_ptr_dpr_csum_fifo_t sta_ff_ptr_dpr_csum_fifo;
    
        cap_dprmem_csr_cfw_ff_dpr_csum_fifo_t cfw_ff_dpr_csum_fifo;
    
        cap_dprmem_csr_cfg_dpr_phv_fifo_t cfg_dpr_phv_fifo;
    
        cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t sta_srams_ecc_dpr_phv_fifo;
    
        cap_dprmem_csr_sta_ff_ptr_dpr_phv_fifo_t sta_ff_ptr_dpr_phv_fifo;
    
        cap_dprmem_csr_cfw_ff_dpr_phv_fifo_t cfw_ff_dpr_phv_fifo;
    
        cap_dprmem_csr_cfg_dpr_ohi_fifo_t cfg_dpr_ohi_fifo;
    
        cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t sta_srams_ecc_dpr_ohi_fifo;
    
        cap_dprmem_csr_sta_ff_ptr_dpr_ohi_fifo_t sta_ff_ptr_dpr_ohi_fifo;
    
        cap_dprmem_csr_cfw_ff_dpr_ohi_fifo_t cfw_ff_dpr_ohi_fifo;
    
        cap_dprmem_csr_cfg_dpr_ptr_fifo_t cfg_dpr_ptr_fifo;
    
        cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t sta_srams_ecc_dpr_ptr_fifo;
    
        cap_dprmem_csr_sta_ff_ptr_dpr_ptr_fifo_t sta_ff_ptr_dpr_ptr_fifo;
    
        cap_dprmem_csr_cfw_ff_dpr_ptr_fifo_t cfw_ff_dpr_ptr_fifo;
    
        cap_dprmem_csr_cfg_dpr_pktout_fifo_t cfg_dpr_pktout_fifo;
    
        cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t sta_srams_ecc_dpr_pktout_fifo;
    
        cap_dprmem_csr_sta_ff_ptr_dpr_pktout_fifo_t sta_ff_ptr_dpr_pktout_fifo;
    
        cap_dprmem_csr_cfw_ff_dpr_pktout_fifo_t cfw_ff_dpr_pktout_fifo;
    
        cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_t dhs_dpr_pktin_fifo_sram;
    
        cap_dprmem_csr_dhs_dpr_csum_fifo_sram_t dhs_dpr_csum_fifo_sram;
    
        cap_dprmem_csr_dhs_dpr_phv_fifo_sram_t dhs_dpr_phv_fifo_sram;
    
        cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_t dhs_dpr_ohi_fifo_sram;
    
        cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_t dhs_dpr_ptr_fifo_sram;
    
        cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_t dhs_dpr_pktout_fifo_sram;
    
}; // cap_dprmem_csr_t
    
#endif // CAP_DPRMEM_CSR_H
        