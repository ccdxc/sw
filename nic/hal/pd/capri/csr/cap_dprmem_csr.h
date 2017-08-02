
#ifndef CAP_DPRMEM_CSR_H
#define CAP_DPRMEM_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t(string name = "cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 521 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 36 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t
    
class cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_t(string name = "cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_t", cap_csr_base *parent = 0);
        virtual ~cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_entry_t entry[16];
        int get_depth_entry() { return 16; }
    
}; // cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_t
    
class cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t(string name = "cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1218 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 72 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t
    
class cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_t(string name = "cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_t", cap_csr_base *parent = 0);
        virtual ~cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_entry_t entry[20];
        int get_depth_entry() { return 20; }
    
}; // cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_t
    
class cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t(string name = "cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 515 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 20 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t
    
class cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_t(string name = "cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_t", cap_csr_base *parent = 0);
        virtual ~cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_entry_t entry[16];
        int get_depth_entry() { return 16; }
    
}; // cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_t
    
class cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t(string name = "cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 515 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 36 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t
    
class cap_dprmem_csr_dhs_dpr_phv_fifo_sram_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_dprmem_csr_dhs_dpr_phv_fifo_sram_t(string name = "cap_dprmem_csr_dhs_dpr_phv_fifo_sram_t", cap_csr_base *parent = 0);
        virtual ~cap_dprmem_csr_dhs_dpr_phv_fifo_sram_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_dprmem_csr_dhs_dpr_phv_fifo_sram_entry_t entry[64];
        int get_depth_entry() { return 64; }
    
}; // cap_dprmem_csr_dhs_dpr_phv_fifo_sram_t
    
class cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t(string name = "cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 521 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 36 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t
    
class cap_dprmem_csr_dhs_dpr_csum_fifo_sram_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_dprmem_csr_dhs_dpr_csum_fifo_sram_t(string name = "cap_dprmem_csr_dhs_dpr_csum_fifo_sram_t", cap_csr_base *parent = 0);
        virtual ~cap_dprmem_csr_dhs_dpr_csum_fifo_sram_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_dprmem_csr_dhs_dpr_csum_fifo_sram_entry_t entry[512];
        int get_depth_entry() { return 512; }
    
}; // cap_dprmem_csr_dhs_dpr_csum_fifo_sram_t
    
class cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t(string name = "cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 521 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 36 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t
    
class cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_t(string name = "cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_t", cap_csr_base *parent = 0);
        virtual ~cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_entry_t entry[832];
        int get_depth_entry() { return 832; }
    
}; // cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_t
    
class cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t(string name = "cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t", cap_csr_base *parent = 0);
        virtual ~cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        uncorrectable_cpp_int_t int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        correctable_cpp_int_t int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 11 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 4 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t
    
class cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t(string name = "cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t", cap_csr_base *parent = 0);
        virtual ~cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        uncorrectable_cpp_int_t int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        correctable_cpp_int_t int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 12 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 5 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t
    
class cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t(string name = "cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t", cap_csr_base *parent = 0);
        virtual ~cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        uncorrectable_cpp_int_t int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        correctable_cpp_int_t int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 11 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 4 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t
    
class cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t(string name = "cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t", cap_csr_base *parent = 0);
        virtual ~cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        uncorrectable_cpp_int_t int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        correctable_cpp_int_t int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 11 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 6 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t
    
class cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t(string name = "cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t", cap_csr_base *parent = 0);
        virtual ~cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        uncorrectable_cpp_int_t int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        correctable_cpp_int_t int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 11 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 9 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t
    
class cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t(string name = "cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t", cap_csr_base *parent = 0);
        virtual ~cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > uncorrectable_cpp_int_t;
        uncorrectable_cpp_int_t int_var__uncorrectable;
        void uncorrectable (const cpp_int  & l__val);
        cpp_int uncorrectable() const;
    
        typedef pu_cpp_int< 1 > correctable_cpp_int_t;
        correctable_cpp_int_t int_var__correctable;
        void correctable (const cpp_int  & l__val);
        cpp_int correctable() const;
    
        typedef pu_cpp_int< 11 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 10 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t
    
class cap_dprmem_csr_t : public cap_block_base {
    public:
        cpp_int_helper hlp;
    
        cap_dprmem_csr_t(string name = "cap_dprmem_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_dprmem_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_dprmem_csr_sta_srams_ecc_dpr_pktin_fifo_t sta_srams_ecc_dpr_pktin_fifo;
    
        cap_dprmem_csr_sta_srams_ecc_dpr_csum_fifo_t sta_srams_ecc_dpr_csum_fifo;
    
        cap_dprmem_csr_sta_srams_ecc_dpr_phv_fifo_t sta_srams_ecc_dpr_phv_fifo;
    
        cap_dprmem_csr_sta_srams_ecc_dpr_ohi_fifo_t sta_srams_ecc_dpr_ohi_fifo;
    
        cap_dprmem_csr_sta_srams_ecc_dpr_ptr_fifo_t sta_srams_ecc_dpr_ptr_fifo;
    
        cap_dprmem_csr_sta_srams_ecc_dpr_pktout_fifo_t sta_srams_ecc_dpr_pktout_fifo;
    
        cap_dprmem_csr_dhs_dpr_pktin_fifo_sram_t dhs_dpr_pktin_fifo_sram;
    
        cap_dprmem_csr_dhs_dpr_csum_fifo_sram_t dhs_dpr_csum_fifo_sram;
    
        cap_dprmem_csr_dhs_dpr_phv_fifo_sram_t dhs_dpr_phv_fifo_sram;
    
        cap_dprmem_csr_dhs_dpr_ohi_fifo_sram_t dhs_dpr_ohi_fifo_sram;
    
        cap_dprmem_csr_dhs_dpr_ptr_fifo_sram_t dhs_dpr_ptr_fifo_sram;
    
        cap_dprmem_csr_dhs_dpr_pktout_fifo_sram_t dhs_dpr_pktout_fifo_sram;
    
}; // cap_dprmem_csr_t
    
#endif // CAP_DPRMEM_CSR_H
        