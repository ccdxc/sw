
#ifndef CAP_PXB_CSR_H
#define CAP_PXB_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_pxb_csr_dhs_tgt_pmt_ind_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_pmt_ind_entry_t(string name = "cap_pxb_csr_dhs_tgt_pmt_ind_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_pmt_ind_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 2 > cmd_cpp_int_t;
        cmd_cpp_int_t int_var__cmd;
        void cmd (const cpp_int  & l__val);
        cpp_int cmd() const;
    
}; // cap_pxb_csr_dhs_tgt_pmt_ind_entry_t
    
class cap_pxb_csr_dhs_tgt_pmt_ind_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_pmt_ind_t(string name = "cap_pxb_csr_dhs_tgt_pmt_ind_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_pmt_ind_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_tgt_pmt_ind_entry_t entry;
    
}; // cap_pxb_csr_dhs_tgt_pmt_ind_t
    
class cap_pxb_csr_dhs_tgt_ind_rsp_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_ind_rsp_entry_t(string name = "cap_pxb_csr_dhs_tgt_ind_rsp_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_ind_rsp_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > data0_cpp_int_t;
        data0_cpp_int_t int_var__data0;
        void data0 (const cpp_int  & l__val);
        cpp_int data0() const;
    
        typedef pu_cpp_int< 32 > data1_cpp_int_t;
        data1_cpp_int_t int_var__data1;
        void data1 (const cpp_int  & l__val);
        cpp_int data1() const;
    
        typedef pu_cpp_int< 32 > data2_cpp_int_t;
        data2_cpp_int_t int_var__data2;
        void data2 (const cpp_int  & l__val);
        cpp_int data2() const;
    
        typedef pu_cpp_int< 32 > data3_cpp_int_t;
        data3_cpp_int_t int_var__data3;
        void data3 (const cpp_int  & l__val);
        cpp_int data3() const;
    
        typedef pu_cpp_int< 3 > cpl_stat_cpp_int_t;
        cpl_stat_cpp_int_t int_var__cpl_stat;
        void cpl_stat (const cpp_int  & l__val);
        cpp_int cpl_stat() const;
    
        typedef pu_cpp_int< 3 > port_id_cpp_int_t;
        port_id_cpp_int_t int_var__port_id;
        void port_id (const cpp_int  & l__val);
        cpp_int port_id() const;
    
        typedef pu_cpp_int< 7 > axi_id_cpp_int_t;
        axi_id_cpp_int_t int_var__axi_id;
        void axi_id (const cpp_int  & l__val);
        cpp_int axi_id() const;
    
        typedef pu_cpp_int< 1 > fetch_rsp_cpp_int_t;
        fetch_rsp_cpp_int_t int_var__fetch_rsp;
        void fetch_rsp (const cpp_int  & l__val);
        cpp_int fetch_rsp() const;
    
}; // cap_pxb_csr_dhs_tgt_ind_rsp_entry_t
    
class cap_pxb_csr_dhs_tgt_ind_rsp_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_ind_rsp_t(string name = "cap_pxb_csr_dhs_tgt_ind_rsp_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_ind_rsp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_tgt_ind_rsp_entry_t entry;
    
}; // cap_pxb_csr_dhs_tgt_ind_rsp_t
    
class cap_pxb_csr_dhs_tgt_notify_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_notify_entry_t(string name = "cap_pxb_csr_dhs_tgt_notify_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_notify_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > posted_cpp_int_t;
        posted_cpp_int_t int_var__posted;
        void posted (const cpp_int  & l__val);
        cpp_int posted() const;
    
        typedef pu_cpp_int< 10 > completed_cpp_int_t;
        completed_cpp_int_t int_var__completed;
        void completed (const cpp_int  & l__val);
        cpp_int completed() const;
    
}; // cap_pxb_csr_dhs_tgt_notify_entry_t
    
class cap_pxb_csr_dhs_tgt_notify_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_notify_t(string name = "cap_pxb_csr_dhs_tgt_notify_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_notify_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_tgt_notify_entry_t entry[8];
        int get_depth_entry() { return 8; }
    
}; // cap_pxb_csr_dhs_tgt_notify_t
    
class cap_pxb_csr_dhs_itr_cplbfr15_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr15_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr15_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr15_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr15_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr15_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr15_t(string name = "cap_pxb_csr_dhs_itr_cplbfr15_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr15_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_itr_cplbfr15_entry_t entry[512];
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr15_t
    
class cap_pxb_csr_dhs_itr_cplbfr14_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr14_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr14_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr14_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr14_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr14_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr14_t(string name = "cap_pxb_csr_dhs_itr_cplbfr14_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr14_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_itr_cplbfr14_entry_t entry[512];
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr14_t
    
class cap_pxb_csr_dhs_itr_cplbfr13_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr13_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr13_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr13_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr13_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr13_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr13_t(string name = "cap_pxb_csr_dhs_itr_cplbfr13_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr13_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_itr_cplbfr13_entry_t entry[512];
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr13_t
    
class cap_pxb_csr_dhs_itr_cplbfr12_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr12_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr12_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr12_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr12_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr12_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr12_t(string name = "cap_pxb_csr_dhs_itr_cplbfr12_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr12_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_itr_cplbfr12_entry_t entry[512];
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr12_t
    
class cap_pxb_csr_dhs_itr_cplbfr11_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr11_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr11_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr11_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr11_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr11_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr11_t(string name = "cap_pxb_csr_dhs_itr_cplbfr11_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr11_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_itr_cplbfr11_entry_t entry[512];
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr11_t
    
class cap_pxb_csr_dhs_itr_cplbfr10_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr10_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr10_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr10_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr10_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr10_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr10_t(string name = "cap_pxb_csr_dhs_itr_cplbfr10_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr10_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_itr_cplbfr10_entry_t entry[512];
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr10_t
    
class cap_pxb_csr_dhs_itr_cplbfr9_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr9_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr9_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr9_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr9_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr9_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr9_t(string name = "cap_pxb_csr_dhs_itr_cplbfr9_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr9_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_itr_cplbfr9_entry_t entry[512];
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr9_t
    
class cap_pxb_csr_dhs_itr_cplbfr8_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr8_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr8_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr8_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr8_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr8_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr8_t(string name = "cap_pxb_csr_dhs_itr_cplbfr8_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr8_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_itr_cplbfr8_entry_t entry[512];
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr8_t
    
class cap_pxb_csr_dhs_itr_cplbfr7_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr7_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr7_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr7_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr7_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr7_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr7_t(string name = "cap_pxb_csr_dhs_itr_cplbfr7_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_itr_cplbfr7_entry_t entry[512];
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr7_t
    
class cap_pxb_csr_dhs_itr_cplbfr6_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr6_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr6_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr6_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr6_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr6_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr6_t(string name = "cap_pxb_csr_dhs_itr_cplbfr6_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_itr_cplbfr6_entry_t entry[512];
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr6_t
    
class cap_pxb_csr_dhs_itr_cplbfr5_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr5_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr5_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr5_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr5_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr5_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr5_t(string name = "cap_pxb_csr_dhs_itr_cplbfr5_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_itr_cplbfr5_entry_t entry[512];
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr5_t
    
class cap_pxb_csr_dhs_itr_cplbfr4_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr4_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr4_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr4_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr4_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr4_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr4_t(string name = "cap_pxb_csr_dhs_itr_cplbfr4_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_itr_cplbfr4_entry_t entry[512];
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr4_t
    
class cap_pxb_csr_dhs_itr_cplbfr3_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr3_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr3_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr3_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr3_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr3_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr3_t(string name = "cap_pxb_csr_dhs_itr_cplbfr3_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_itr_cplbfr3_entry_t entry[512];
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr3_t
    
class cap_pxb_csr_dhs_itr_cplbfr2_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr2_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr2_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr2_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr2_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr2_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr2_t(string name = "cap_pxb_csr_dhs_itr_cplbfr2_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_itr_cplbfr2_entry_t entry[512];
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr2_t
    
class cap_pxb_csr_dhs_itr_cplbfr1_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr1_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr1_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr1_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr1_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr1_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr1_t(string name = "cap_pxb_csr_dhs_itr_cplbfr1_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_itr_cplbfr1_entry_t entry[512];
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr1_t
    
class cap_pxb_csr_dhs_itr_cplbfr0_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr0_entry_t(string name = "cap_pxb_csr_dhs_itr_cplbfr0_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr0_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_cplbfr0_entry_t
    
class cap_pxb_csr_dhs_itr_cplbfr0_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_cplbfr0_t(string name = "cap_pxb_csr_dhs_itr_cplbfr0_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_cplbfr0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_itr_cplbfr0_entry_t entry[512];
        int get_depth_entry() { return 512; }
    
}; // cap_pxb_csr_dhs_itr_cplbfr0_t
    
class cap_pxb_csr_dhs_tgt_rc_bdfmap_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_rc_bdfmap_entry_t(string name = "cap_pxb_csr_dhs_tgt_rc_bdfmap_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_rc_bdfmap_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        valid_cpp_int_t int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 16 > rc_bdf_cpp_int_t;
        rc_bdf_cpp_int_t int_var__rc_bdf;
        void rc_bdf (const cpp_int  & l__val);
        cpp_int rc_bdf() const;
    
        typedef pu_cpp_int< 3 > port_id_cpp_int_t;
        port_id_cpp_int_t int_var__port_id;
        void port_id (const cpp_int  & l__val);
        cpp_int port_id() const;
    
        typedef pu_cpp_int< 11 > VFID_cpp_int_t;
        VFID_cpp_int_t int_var__VFID;
        void VFID (const cpp_int  & l__val);
        cpp_int VFID() const;
    
}; // cap_pxb_csr_dhs_tgt_rc_bdfmap_entry_t
    
class cap_pxb_csr_dhs_tgt_rc_bdfmap_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_rc_bdfmap_t(string name = "cap_pxb_csr_dhs_tgt_rc_bdfmap_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_rc_bdfmap_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_tgt_rc_bdfmap_entry_t entry[64];
        int get_depth_entry() { return 64; }
    
}; // cap_pxb_csr_dhs_tgt_rc_bdfmap_t
    
class cap_pxb_csr_dhs_tgt_rdcontext_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_rdcontext_entry_t(string name = "cap_pxb_csr_dhs_tgt_rdcontext_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_rdcontext_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 7 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // cap_pxb_csr_dhs_tgt_rdcontext_entry_t
    
class cap_pxb_csr_dhs_tgt_rdcontext_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_rdcontext_t(string name = "cap_pxb_csr_dhs_tgt_rdcontext_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_rdcontext_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_tgt_rdcontext_entry_t entry[128];
        int get_depth_entry() { return 128; }
    
}; // cap_pxb_csr_dhs_tgt_rdcontext_t
    
class cap_pxb_csr_dhs_itr_rdcontext_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_rdcontext_entry_t(string name = "cap_pxb_csr_dhs_itr_rdcontext_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_rdcontext_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 100 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
}; // cap_pxb_csr_dhs_itr_rdcontext_entry_t
    
class cap_pxb_csr_dhs_itr_rdcontext_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_rdcontext_t(string name = "cap_pxb_csr_dhs_itr_rdcontext_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_rdcontext_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_itr_rdcontext_entry_t entry[128];
        int get_depth_entry() { return 128; }
    
}; // cap_pxb_csr_dhs_itr_rdcontext_t
    
class cap_pxb_csr_dhs_itr_rdhdr_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_rdhdr_entry_t(string name = "cap_pxb_csr_dhs_itr_rdhdr_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_rdhdr_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 80 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 8 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_rdhdr_entry_t
    
class cap_pxb_csr_dhs_itr_rdhdr_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_rdhdr_t(string name = "cap_pxb_csr_dhs_itr_rdhdr_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_rdhdr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_itr_rdhdr_entry_t entry[128];
        int get_depth_entry() { return 128; }
    
}; // cap_pxb_csr_dhs_itr_rdhdr_t
    
class cap_pxb_csr_dhs_itr_wrhdr_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_wrhdr_entry_t(string name = "cap_pxb_csr_dhs_itr_wrhdr_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_wrhdr_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 80 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 8 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_wrhdr_entry_t
    
class cap_pxb_csr_dhs_itr_wrhdr_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_wrhdr_t(string name = "cap_pxb_csr_dhs_itr_wrhdr_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_wrhdr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_itr_wrhdr_entry_t entry[128];
        int get_depth_entry() { return 128; }
    
}; // cap_pxb_csr_dhs_itr_wrhdr_t
    
class cap_pxb_csr_dhs_itr_wrbfr3_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_wrbfr3_entry_t(string name = "cap_pxb_csr_dhs_itr_wrbfr3_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_wrbfr3_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_wrbfr3_entry_t
    
class cap_pxb_csr_dhs_itr_wrbfr3_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_wrbfr3_t(string name = "cap_pxb_csr_dhs_itr_wrbfr3_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_wrbfr3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_itr_wrbfr3_entry_t entry[1024];
        int get_depth_entry() { return 1024; }
    
}; // cap_pxb_csr_dhs_itr_wrbfr3_t
    
class cap_pxb_csr_dhs_itr_wrbfr2_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_wrbfr2_entry_t(string name = "cap_pxb_csr_dhs_itr_wrbfr2_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_wrbfr2_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_wrbfr2_entry_t
    
class cap_pxb_csr_dhs_itr_wrbfr2_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_wrbfr2_t(string name = "cap_pxb_csr_dhs_itr_wrbfr2_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_wrbfr2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_itr_wrbfr2_entry_t entry[1024];
        int get_depth_entry() { return 1024; }
    
}; // cap_pxb_csr_dhs_itr_wrbfr2_t
    
class cap_pxb_csr_dhs_itr_wrbfr1_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_wrbfr1_entry_t(string name = "cap_pxb_csr_dhs_itr_wrbfr1_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_wrbfr1_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_wrbfr1_entry_t
    
class cap_pxb_csr_dhs_itr_wrbfr1_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_wrbfr1_t(string name = "cap_pxb_csr_dhs_itr_wrbfr1_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_wrbfr1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_itr_wrbfr1_entry_t entry[1024];
        int get_depth_entry() { return 1024; }
    
}; // cap_pxb_csr_dhs_itr_wrbfr1_t
    
class cap_pxb_csr_dhs_itr_wrbfr0_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_wrbfr0_entry_t(string name = "cap_pxb_csr_dhs_itr_wrbfr0_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_wrbfr0_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_wrbfr0_entry_t
    
class cap_pxb_csr_dhs_itr_wrbfr0_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_wrbfr0_t(string name = "cap_pxb_csr_dhs_itr_wrbfr0_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_wrbfr0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_itr_wrbfr0_entry_t entry[1024];
        int get_depth_entry() { return 1024; }
    
}; // cap_pxb_csr_dhs_itr_wrbfr0_t
    
class cap_pxb_csr_dhs_tgt_aximst4_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_aximst4_entry_t(string name = "cap_pxb_csr_dhs_tgt_aximst4_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_aximst4_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_tgt_aximst4_entry_t
    
class cap_pxb_csr_dhs_tgt_aximst4_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_aximst4_t(string name = "cap_pxb_csr_dhs_tgt_aximst4_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_aximst4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_tgt_aximst4_entry_t entry[128];
        int get_depth_entry() { return 128; }
    
}; // cap_pxb_csr_dhs_tgt_aximst4_t
    
class cap_pxb_csr_dhs_tgt_aximst3_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_aximst3_entry_t(string name = "cap_pxb_csr_dhs_tgt_aximst3_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_aximst3_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_tgt_aximst3_entry_t
    
class cap_pxb_csr_dhs_tgt_aximst3_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_aximst3_t(string name = "cap_pxb_csr_dhs_tgt_aximst3_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_aximst3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_tgt_aximst3_entry_t entry[128];
        int get_depth_entry() { return 128; }
    
}; // cap_pxb_csr_dhs_tgt_aximst3_t
    
class cap_pxb_csr_dhs_tgt_aximst2_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_aximst2_entry_t(string name = "cap_pxb_csr_dhs_tgt_aximst2_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_aximst2_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_tgt_aximst2_entry_t
    
class cap_pxb_csr_dhs_tgt_aximst2_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_aximst2_t(string name = "cap_pxb_csr_dhs_tgt_aximst2_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_aximst2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_tgt_aximst2_entry_t entry[128];
        int get_depth_entry() { return 128; }
    
}; // cap_pxb_csr_dhs_tgt_aximst2_t
    
class cap_pxb_csr_dhs_tgt_aximst1_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_aximst1_entry_t(string name = "cap_pxb_csr_dhs_tgt_aximst1_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_aximst1_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_tgt_aximst1_entry_t
    
class cap_pxb_csr_dhs_tgt_aximst1_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_aximst1_t(string name = "cap_pxb_csr_dhs_tgt_aximst1_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_aximst1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_tgt_aximst1_entry_t entry[128];
        int get_depth_entry() { return 128; }
    
}; // cap_pxb_csr_dhs_tgt_aximst1_t
    
class cap_pxb_csr_dhs_tgt_aximst0_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_aximst0_entry_t(string name = "cap_pxb_csr_dhs_tgt_aximst0_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_aximst0_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_tgt_aximst0_entry_t
    
class cap_pxb_csr_dhs_tgt_aximst0_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_aximst0_t(string name = "cap_pxb_csr_dhs_tgt_aximst0_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_aximst0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_tgt_aximst0_entry_t entry[128];
        int get_depth_entry() { return 128; }
    
}; // cap_pxb_csr_dhs_tgt_aximst0_t
    
class cap_pxb_csr_dhs_tgt_romask_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_romask_entry_t(string name = "cap_pxb_csr_dhs_tgt_romask_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_romask_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_tgt_romask_entry_t
    
class cap_pxb_csr_dhs_tgt_romask_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_romask_t(string name = "cap_pxb_csr_dhs_tgt_romask_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_romask_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_tgt_romask_entry_t entry[128];
        int get_depth_entry() { return 128; }
    
}; // cap_pxb_csr_dhs_tgt_romask_t
    
class cap_pxb_csr_dhs_tgt_cplst_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_cplst_entry_t(string name = "cap_pxb_csr_dhs_tgt_cplst_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_cplst_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 100 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 8 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_tgt_cplst_entry_t
    
class cap_pxb_csr_dhs_tgt_cplst_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_cplst_t(string name = "cap_pxb_csr_dhs_tgt_cplst_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_cplst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_tgt_cplst_entry_t entry[128];
        int get_depth_entry() { return 128; }
    
}; // cap_pxb_csr_dhs_tgt_cplst_t
    
class cap_pxb_csr_dhs_tgt_rxinfo_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_rxinfo_entry_t(string name = "cap_pxb_csr_dhs_tgt_rxinfo_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_rxinfo_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 6 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_tgt_rxinfo_entry_t
    
class cap_pxb_csr_dhs_tgt_rxinfo_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_rxinfo_t(string name = "cap_pxb_csr_dhs_tgt_rxinfo_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_rxinfo_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_tgt_rxinfo_entry_t entry[1024];
        int get_depth_entry() { return 1024; }
    
}; // cap_pxb_csr_dhs_tgt_rxinfo_t
    
class cap_pxb_csr_dhs_tgt_rxcrbfr3_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_rxcrbfr3_entry_t(string name = "cap_pxb_csr_dhs_tgt_rxcrbfr3_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_rxcrbfr3_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_tgt_rxcrbfr3_entry_t
    
class cap_pxb_csr_dhs_tgt_rxcrbfr3_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_rxcrbfr3_t(string name = "cap_pxb_csr_dhs_tgt_rxcrbfr3_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_rxcrbfr3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_tgt_rxcrbfr3_entry_t entry[1024];
        int get_depth_entry() { return 1024; }
    
}; // cap_pxb_csr_dhs_tgt_rxcrbfr3_t
    
class cap_pxb_csr_dhs_tgt_rxcrbfr2_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_rxcrbfr2_entry_t(string name = "cap_pxb_csr_dhs_tgt_rxcrbfr2_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_rxcrbfr2_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_tgt_rxcrbfr2_entry_t
    
class cap_pxb_csr_dhs_tgt_rxcrbfr2_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_rxcrbfr2_t(string name = "cap_pxb_csr_dhs_tgt_rxcrbfr2_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_rxcrbfr2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_tgt_rxcrbfr2_entry_t entry[1024];
        int get_depth_entry() { return 1024; }
    
}; // cap_pxb_csr_dhs_tgt_rxcrbfr2_t
    
class cap_pxb_csr_dhs_tgt_rxcrbfr1_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_rxcrbfr1_entry_t(string name = "cap_pxb_csr_dhs_tgt_rxcrbfr1_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_rxcrbfr1_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_tgt_rxcrbfr1_entry_t
    
class cap_pxb_csr_dhs_tgt_rxcrbfr1_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_rxcrbfr1_t(string name = "cap_pxb_csr_dhs_tgt_rxcrbfr1_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_rxcrbfr1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_tgt_rxcrbfr1_entry_t entry[1024];
        int get_depth_entry() { return 1024; }
    
}; // cap_pxb_csr_dhs_tgt_rxcrbfr1_t
    
class cap_pxb_csr_dhs_tgt_rxcrbfr0_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_rxcrbfr0_entry_t(string name = "cap_pxb_csr_dhs_tgt_rxcrbfr0_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_rxcrbfr0_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 128 > data_cpp_int_t;
        data_cpp_int_t int_var__data;
        void data (const cpp_int  & l__val);
        cpp_int data() const;
    
        typedef pu_cpp_int< 9 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_tgt_rxcrbfr0_entry_t
    
class cap_pxb_csr_dhs_tgt_rxcrbfr0_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_rxcrbfr0_t(string name = "cap_pxb_csr_dhs_tgt_rxcrbfr0_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_rxcrbfr0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_tgt_rxcrbfr0_entry_t entry[1024];
        int get_depth_entry() { return 1024; }
    
}; // cap_pxb_csr_dhs_tgt_rxcrbfr0_t
    
class cap_pxb_csr_dhs_tgt_prt_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_prt_entry_t(string name = "cap_pxb_csr_dhs_tgt_prt_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_prt_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 76 > entry_data_cpp_int_t;
        entry_data_cpp_int_t int_var__entry_data;
        void entry_data (const cpp_int  & l__val);
        cpp_int entry_data() const;
    
        typedef pu_cpp_int< 8 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_tgt_prt_entry_t
    
class cap_pxb_csr_dhs_tgt_prt_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_prt_t(string name = "cap_pxb_csr_dhs_tgt_prt_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_prt_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_tgt_prt_entry_t entry[4096];
        int get_depth_entry() { return 4096; }
    
}; // cap_pxb_csr_dhs_tgt_prt_t
    
class cap_pxb_csr_dhs_tgt_pmr_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_pmr_entry_t(string name = "cap_pxb_csr_dhs_tgt_pmr_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_pmr_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 110 > entry_data_cpp_int_t;
        entry_data_cpp_int_t int_var__entry_data;
        void entry_data (const cpp_int  & l__val);
        cpp_int entry_data() const;
    
        typedef pu_cpp_int< 8 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_tgt_pmr_entry_t
    
class cap_pxb_csr_dhs_tgt_pmr_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_pmr_t(string name = "cap_pxb_csr_dhs_tgt_pmr_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_pmr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_tgt_pmr_entry_t entry[1024];
        int get_depth_entry() { return 1024; }
    
}; // cap_pxb_csr_dhs_tgt_pmr_t
    
class cap_pxb_csr_dhs_tgt_pmt_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_pmt_entry_t(string name = "cap_pxb_csr_dhs_tgt_pmt_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_pmt_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 64 > x_data_cpp_int_t;
        x_data_cpp_int_t int_var__x_data;
        void x_data (const cpp_int  & l__val);
        cpp_int x_data() const;
    
        typedef pu_cpp_int< 64 > y_data_cpp_int_t;
        y_data_cpp_int_t int_var__y_data;
        void y_data (const cpp_int  & l__val);
        cpp_int y_data() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        valid_cpp_int_t int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
}; // cap_pxb_csr_dhs_tgt_pmt_entry_t
    
class cap_pxb_csr_dhs_tgt_pmt_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_tgt_pmt_t(string name = "cap_pxb_csr_dhs_tgt_pmt_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_tgt_pmt_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_tgt_pmt_entry_t entry[1024];
        int get_depth_entry() { return 1024; }
    
}; // cap_pxb_csr_dhs_tgt_pmt_t
    
class cap_pxb_csr_dhs_itr_portmap_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_portmap_entry_t(string name = "cap_pxb_csr_dhs_itr_portmap_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_portmap_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 48 > dest_cpp_int_t;
        dest_cpp_int_t int_var__dest;
        void dest (const cpp_int  & l__val);
        cpp_int dest() const;
    
        typedef pu_cpp_int< 7 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_portmap_entry_t
    
class cap_pxb_csr_dhs_itr_portmap_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_portmap_t(string name = "cap_pxb_csr_dhs_itr_portmap_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_portmap_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_itr_portmap_entry_t entry[128];
        int get_depth_entry() { return 128; }
    
}; // cap_pxb_csr_dhs_itr_portmap_t
    
class cap_pxb_csr_dhs_itr_pcihdrt_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_pcihdrt_entry_t(string name = "cap_pxb_csr_dhs_itr_pcihdrt_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_pcihdrt_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        valid_cpp_int_t int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
        typedef pu_cpp_int< 16 > bdf_cpp_int_t;
        bdf_cpp_int_t int_var__bdf;
        void bdf (const cpp_int  & l__val);
        cpp_int bdf() const;
    
        typedef pu_cpp_int< 1 > td_cpp_int_t;
        td_cpp_int_t int_var__td;
        void td (const cpp_int  & l__val);
        cpp_int td() const;
    
        typedef pu_cpp_int< 1 > pasid_en_cpp_int_t;
        pasid_en_cpp_int_t int_var__pasid_en;
        void pasid_en (const cpp_int  & l__val);
        cpp_int pasid_en() const;
    
        typedef pu_cpp_int< 2 > pasid_source_cpp_int_t;
        pasid_source_cpp_int_t int_var__pasid_source;
        void pasid_source (const cpp_int  & l__val);
        cpp_int pasid_source() const;
    
        typedef pu_cpp_int< 20 > pasid_value_cpp_int_t;
        pasid_value_cpp_int_t int_var__pasid_value;
        void pasid_value (const cpp_int  & l__val);
        cpp_int pasid_value() const;
    
        typedef pu_cpp_int< 1 > pasid_exe_cpp_int_t;
        pasid_exe_cpp_int_t int_var__pasid_exe;
        void pasid_exe (const cpp_int  & l__val);
        cpp_int pasid_exe() const;
    
        typedef pu_cpp_int< 1 > pasid_privl_cpp_int_t;
        pasid_privl_cpp_int_t int_var__pasid_privl;
        void pasid_privl (const cpp_int  & l__val);
        cpp_int pasid_privl() const;
    
        typedef pu_cpp_int< 2 > attr2_1_cpp_int_t;
        attr2_1_cpp_int_t int_var__attr2_1;
        void attr2_1 (const cpp_int  & l__val);
        cpp_int attr2_1() const;
    
        typedef pu_cpp_int< 2 > wr_ph_cpp_int_t;
        wr_ph_cpp_int_t int_var__wr_ph;
        void wr_ph (const cpp_int  & l__val);
        cpp_int wr_ph() const;
    
        typedef pu_cpp_int< 1 > wr_tph_en_cpp_int_t;
        wr_tph_en_cpp_int_t int_var__wr_tph_en;
        void wr_tph_en (const cpp_int  & l__val);
        cpp_int wr_tph_en() const;
    
        typedef pu_cpp_int< 2 > rd_ph_cpp_int_t;
        rd_ph_cpp_int_t int_var__rd_ph;
        void rd_ph (const cpp_int  & l__val);
        cpp_int rd_ph() const;
    
        typedef pu_cpp_int< 1 > rd_tph_en_cpp_int_t;
        rd_tph_en_cpp_int_t int_var__rd_tph_en;
        void rd_tph_en (const cpp_int  & l__val);
        cpp_int rd_tph_en() const;
    
        typedef pu_cpp_int< 8 > tph_dca_st_cpp_int_t;
        tph_dca_st_cpp_int_t int_var__tph_dca_st;
        void tph_dca_st (const cpp_int  & l__val);
        cpp_int tph_dca_st() const;
    
        typedef pu_cpp_int< 8 > msi_st_cpp_int_t;
        msi_st_cpp_int_t int_var__msi_st;
        void msi_st (const cpp_int  & l__val);
        cpp_int msi_st() const;
    
        typedef pu_cpp_int< 1 > rc_cfg1_cpp_int_t;
        rc_cfg1_cpp_int_t int_var__rc_cfg1;
        void rc_cfg1 (const cpp_int  & l__val);
        cpp_int rc_cfg1() const;
    
        typedef pu_cpp_int< 8 > ecc_cpp_int_t;
        ecc_cpp_int_t int_var__ecc;
        void ecc (const cpp_int  & l__val);
        cpp_int ecc() const;
    
}; // cap_pxb_csr_dhs_itr_pcihdrt_entry_t
    
class cap_pxb_csr_dhs_itr_pcihdrt_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_dhs_itr_pcihdrt_t(string name = "cap_pxb_csr_dhs_itr_pcihdrt_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_dhs_itr_pcihdrt_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_dhs_itr_pcihdrt_entry_t entry[2048];
        int get_depth_entry() { return 2048; }
    
}; // cap_pxb_csr_dhs_itr_pcihdrt_t
    
class cap_pxb_csr_cfg_debug_port_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_cfg_debug_port_t(string name = "cap_pxb_csr_cfg_debug_port_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_cfg_debug_port_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 4 > select_cpp_int_t;
        select_cpp_int_t int_var__select;
        void select (const cpp_int  & l__val);
        cpp_int select() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        enable_cpp_int_t int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
}; // cap_pxb_csr_cfg_debug_port_t
    
class cap_pxb_csr_cfg_tgt_axi_attr_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_cfg_tgt_axi_attr_t(string name = "cap_pxb_csr_cfg_tgt_axi_attr_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_cfg_tgt_axi_attr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 4 > arcache_cpp_int_t;
        arcache_cpp_int_t int_var__arcache;
        void arcache (const cpp_int  & l__val);
        cpp_int arcache() const;
    
        typedef pu_cpp_int< 4 > awcache_cpp_int_t;
        awcache_cpp_int_t int_var__awcache;
        void awcache (const cpp_int  & l__val);
        cpp_int awcache() const;
    
        typedef pu_cpp_int< 3 > prot_cpp_int_t;
        prot_cpp_int_t int_var__prot;
        void prot (const cpp_int  & l__val);
        cpp_int prot() const;
    
        typedef pu_cpp_int< 4 > qos_cpp_int_t;
        qos_cpp_int_t int_var__qos;
        void qos (const cpp_int  & l__val);
        cpp_int qos() const;
    
        typedef pu_cpp_int< 1 > lock_cpp_int_t;
        lock_cpp_int_t int_var__lock;
        void lock (const cpp_int  & l__val);
        cpp_int lock() const;
    
}; // cap_pxb_csr_cfg_tgt_axi_attr_t
    
class cap_pxb_csr_sat_itr_req_err_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sat_itr_req_err_t(string name = "cap_pxb_csr_sat_itr_req_err_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sat_itr_req_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > unsupp_wr_cpp_int_t;
        unsupp_wr_cpp_int_t int_var__unsupp_wr;
        void unsupp_wr (const cpp_int  & l__val);
        cpp_int unsupp_wr() const;
    
        typedef pu_cpp_int< 8 > unsupp_rd_cpp_int_t;
        unsupp_rd_cpp_int_t int_var__unsupp_rd;
        void unsupp_rd (const cpp_int  & l__val);
        cpp_int unsupp_rd() const;
    
        typedef pu_cpp_int< 8 > pcihdrt_miss_cpp_int_t;
        pcihdrt_miss_cpp_int_t int_var__pcihdrt_miss;
        void pcihdrt_miss (const cpp_int  & l__val);
        cpp_int pcihdrt_miss() const;
    
        typedef pu_cpp_int< 8 > bus_master_dis_cpp_int_t;
        bus_master_dis_cpp_int_t int_var__bus_master_dis;
        void bus_master_dis (const cpp_int  & l__val);
        cpp_int bus_master_dis() const;
    
}; // cap_pxb_csr_sat_itr_req_err_t
    
class cap_pxb_csr_cfg_itr_atomic_seq_cnt_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_cfg_itr_atomic_seq_cnt_t(string name = "cap_pxb_csr_cfg_itr_atomic_seq_cnt_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_cfg_itr_atomic_seq_cnt_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > rst_mask_cpp_int_t;
        rst_mask_cpp_int_t int_var__rst_mask;
        void rst_mask (const cpp_int  & l__val);
        cpp_int rst_mask() const;
    
}; // cap_pxb_csr_cfg_itr_atomic_seq_cnt_t
    
class cap_pxb_csr_sta_itr_atomic_seq_cnt_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_itr_atomic_seq_cnt_t(string name = "cap_pxb_csr_sta_itr_atomic_seq_cnt_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_itr_atomic_seq_cnt_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > err_mask_cpp_int_t;
        err_mask_cpp_int_t int_var__err_mask;
        void err_mask (const cpp_int  & l__val);
        cpp_int err_mask() const;
    
}; // cap_pxb_csr_sta_itr_atomic_seq_cnt_t
    
class cap_pxb_csr_sta_itr_portfifo_depth_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_itr_portfifo_depth_t(string name = "cap_pxb_csr_sta_itr_portfifo_depth_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_itr_portfifo_depth_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > port0_wr_cpp_int_t;
        port0_wr_cpp_int_t int_var__port0_wr;
        void port0_wr (const cpp_int  & l__val);
        cpp_int port0_wr() const;
    
        typedef pu_cpp_int< 8 > port0_rd_cpp_int_t;
        port0_rd_cpp_int_t int_var__port0_rd;
        void port0_rd (const cpp_int  & l__val);
        cpp_int port0_rd() const;
    
        typedef pu_cpp_int< 8 > port1_wr_cpp_int_t;
        port1_wr_cpp_int_t int_var__port1_wr;
        void port1_wr (const cpp_int  & l__val);
        cpp_int port1_wr() const;
    
        typedef pu_cpp_int< 8 > port1_rd_cpp_int_t;
        port1_rd_cpp_int_t int_var__port1_rd;
        void port1_rd (const cpp_int  & l__val);
        cpp_int port1_rd() const;
    
        typedef pu_cpp_int< 8 > port2_wr_cpp_int_t;
        port2_wr_cpp_int_t int_var__port2_wr;
        void port2_wr (const cpp_int  & l__val);
        cpp_int port2_wr() const;
    
        typedef pu_cpp_int< 8 > port2_rd_cpp_int_t;
        port2_rd_cpp_int_t int_var__port2_rd;
        void port2_rd (const cpp_int  & l__val);
        cpp_int port2_rd() const;
    
        typedef pu_cpp_int< 8 > port3_wr_cpp_int_t;
        port3_wr_cpp_int_t int_var__port3_wr;
        void port3_wr (const cpp_int  & l__val);
        cpp_int port3_wr() const;
    
        typedef pu_cpp_int< 8 > port3_rd_cpp_int_t;
        port3_rd_cpp_int_t int_var__port3_rd;
        void port3_rd (const cpp_int  & l__val);
        cpp_int port3_rd() const;
    
        typedef pu_cpp_int< 8 > port4_wr_cpp_int_t;
        port4_wr_cpp_int_t int_var__port4_wr;
        void port4_wr (const cpp_int  & l__val);
        cpp_int port4_wr() const;
    
        typedef pu_cpp_int< 8 > port4_rd_cpp_int_t;
        port4_rd_cpp_int_t int_var__port4_rd;
        void port4_rd (const cpp_int  & l__val);
        cpp_int port4_rd() const;
    
        typedef pu_cpp_int< 8 > port5_wr_cpp_int_t;
        port5_wr_cpp_int_t int_var__port5_wr;
        void port5_wr (const cpp_int  & l__val);
        cpp_int port5_wr() const;
    
        typedef pu_cpp_int< 8 > port5_rd_cpp_int_t;
        port5_rd_cpp_int_t int_var__port5_rd;
        void port5_rd (const cpp_int  & l__val);
        cpp_int port5_rd() const;
    
        typedef pu_cpp_int< 8 > port6_wr_cpp_int_t;
        port6_wr_cpp_int_t int_var__port6_wr;
        void port6_wr (const cpp_int  & l__val);
        cpp_int port6_wr() const;
    
        typedef pu_cpp_int< 8 > port6_rd_cpp_int_t;
        port6_rd_cpp_int_t int_var__port6_rd;
        void port6_rd (const cpp_int  & l__val);
        cpp_int port6_rd() const;
    
        typedef pu_cpp_int< 8 > port7_wr_cpp_int_t;
        port7_wr_cpp_int_t int_var__port7_wr;
        void port7_wr (const cpp_int  & l__val);
        cpp_int port7_wr() const;
    
        typedef pu_cpp_int< 8 > port7_rd_cpp_int_t;
        port7_rd_cpp_int_t int_var__port7_rd;
        void port7_rd (const cpp_int  & l__val);
        cpp_int port7_rd() const;
    
}; // cap_pxb_csr_sta_itr_portfifo_depth_t
    
class cap_pxb_csr_sta_itr_tags_pending_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_itr_tags_pending_t(string name = "cap_pxb_csr_sta_itr_tags_pending_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_itr_tags_pending_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > port0_cpp_int_t;
        port0_cpp_int_t int_var__port0;
        void port0 (const cpp_int  & l__val);
        cpp_int port0() const;
    
        typedef pu_cpp_int< 8 > port1_cpp_int_t;
        port1_cpp_int_t int_var__port1;
        void port1 (const cpp_int  & l__val);
        cpp_int port1() const;
    
        typedef pu_cpp_int< 8 > port2_cpp_int_t;
        port2_cpp_int_t int_var__port2;
        void port2 (const cpp_int  & l__val);
        cpp_int port2() const;
    
        typedef pu_cpp_int< 8 > port3_cpp_int_t;
        port3_cpp_int_t int_var__port3;
        void port3 (const cpp_int  & l__val);
        cpp_int port3() const;
    
        typedef pu_cpp_int< 8 > port4_cpp_int_t;
        port4_cpp_int_t int_var__port4;
        void port4 (const cpp_int  & l__val);
        cpp_int port4() const;
    
        typedef pu_cpp_int< 8 > port5_cpp_int_t;
        port5_cpp_int_t int_var__port5;
        void port5 (const cpp_int  & l__val);
        cpp_int port5() const;
    
        typedef pu_cpp_int< 8 > port6_cpp_int_t;
        port6_cpp_int_t int_var__port6;
        void port6 (const cpp_int  & l__val);
        cpp_int port6() const;
    
        typedef pu_cpp_int< 8 > port7_cpp_int_t;
        port7_cpp_int_t int_var__port7;
        void port7 (const cpp_int  & l__val);
        cpp_int port7() const;
    
}; // cap_pxb_csr_sta_itr_tags_pending_t
    
class cap_pxb_csr_sta_itr_raw_tlp_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_itr_raw_tlp_t(string name = "cap_pxb_csr_sta_itr_raw_tlp_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_itr_raw_tlp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > resp0_cpp_int_t;
        resp0_cpp_int_t int_var__resp0;
        void resp0 (const cpp_int  & l__val);
        cpp_int resp0() const;
    
        typedef pu_cpp_int< 32 > resp1_cpp_int_t;
        resp1_cpp_int_t int_var__resp1;
        void resp1 (const cpp_int  & l__val);
        cpp_int resp1() const;
    
        typedef pu_cpp_int< 32 > resp2_cpp_int_t;
        resp2_cpp_int_t int_var__resp2;
        void resp2 (const cpp_int  & l__val);
        cpp_int resp2() const;
    
        typedef pu_cpp_int< 32 > resp3_cpp_int_t;
        resp3_cpp_int_t int_var__resp3;
        void resp3 (const cpp_int  & l__val);
        cpp_int resp3() const;
    
        typedef pu_cpp_int< 32 > resp4_cpp_int_t;
        resp4_cpp_int_t int_var__resp4;
        void resp4 (const cpp_int  & l__val);
        cpp_int resp4() const;
    
        typedef pu_cpp_int< 32 > resp5_cpp_int_t;
        resp5_cpp_int_t int_var__resp5;
        void resp5 (const cpp_int  & l__val);
        cpp_int resp5() const;
    
        typedef pu_cpp_int< 32 > resp6_cpp_int_t;
        resp6_cpp_int_t int_var__resp6;
        void resp6 (const cpp_int  & l__val);
        cpp_int resp6() const;
    
        typedef pu_cpp_int< 32 > resp7_cpp_int_t;
        resp7_cpp_int_t int_var__resp7;
        void resp7 (const cpp_int  & l__val);
        cpp_int resp7() const;
    
        typedef pu_cpp_int< 1 > resp_rdy_cpp_int_t;
        resp_rdy_cpp_int_t int_var__resp_rdy;
        void resp_rdy (const cpp_int  & l__val);
        cpp_int resp_rdy() const;
    
        typedef pu_cpp_int< 3 > cpl_stat_cpp_int_t;
        cpl_stat_cpp_int_t int_var__cpl_stat;
        void cpl_stat (const cpp_int  & l__val);
        cpp_int cpl_stat() const;
    
        typedef pu_cpp_int< 1 > cpl_data_err_cpp_int_t;
        cpl_data_err_cpp_int_t int_var__cpl_data_err;
        void cpl_data_err (const cpp_int  & l__val);
        cpp_int cpl_data_err() const;
    
        typedef pu_cpp_int< 1 > cpl_timeout_err_cpp_int_t;
        cpl_timeout_err_cpp_int_t int_var__cpl_timeout_err;
        void cpl_timeout_err (const cpp_int  & l__val);
        cpp_int cpl_timeout_err() const;
    
        typedef pu_cpp_int< 1 > req_err_cpp_int_t;
        req_err_cpp_int_t int_var__req_err;
        void req_err (const cpp_int  & l__val);
        cpp_int req_err() const;
    
}; // cap_pxb_csr_sta_itr_raw_tlp_t
    
class cap_pxb_csr_cfg_itr_raw_tlp_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_cfg_itr_raw_tlp_t(string name = "cap_pxb_csr_cfg_itr_raw_tlp_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_cfg_itr_raw_tlp_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > dw0_cpp_int_t;
        dw0_cpp_int_t int_var__dw0;
        void dw0 (const cpp_int  & l__val);
        cpp_int dw0() const;
    
        typedef pu_cpp_int< 32 > dw1_cpp_int_t;
        dw1_cpp_int_t int_var__dw1;
        void dw1 (const cpp_int  & l__val);
        cpp_int dw1() const;
    
        typedef pu_cpp_int< 32 > dw2_cpp_int_t;
        dw2_cpp_int_t int_var__dw2;
        void dw2 (const cpp_int  & l__val);
        cpp_int dw2() const;
    
        typedef pu_cpp_int< 32 > dw3_cpp_int_t;
        dw3_cpp_int_t int_var__dw3;
        void dw3 (const cpp_int  & l__val);
        cpp_int dw3() const;
    
        typedef pu_cpp_int< 32 > dw4_cpp_int_t;
        dw4_cpp_int_t int_var__dw4;
        void dw4 (const cpp_int  & l__val);
        cpp_int dw4() const;
    
        typedef pu_cpp_int< 32 > dw5_cpp_int_t;
        dw5_cpp_int_t int_var__dw5;
        void dw5 (const cpp_int  & l__val);
        cpp_int dw5() const;
    
        typedef pu_cpp_int< 32 > dw6_cpp_int_t;
        dw6_cpp_int_t int_var__dw6;
        void dw6 (const cpp_int  & l__val);
        cpp_int dw6() const;
    
        typedef pu_cpp_int< 32 > dw7_cpp_int_t;
        dw7_cpp_int_t int_var__dw7;
        void dw7 (const cpp_int  & l__val);
        cpp_int dw7() const;
    
        typedef pu_cpp_int< 32 > dw8_cpp_int_t;
        dw8_cpp_int_t int_var__dw8;
        void dw8 (const cpp_int  & l__val);
        cpp_int dw8() const;
    
        typedef pu_cpp_int< 32 > dw9_cpp_int_t;
        dw9_cpp_int_t int_var__dw9;
        void dw9 (const cpp_int  & l__val);
        cpp_int dw9() const;
    
        typedef pu_cpp_int< 32 > dw10_cpp_int_t;
        dw10_cpp_int_t int_var__dw10;
        void dw10 (const cpp_int  & l__val);
        cpp_int dw10() const;
    
        typedef pu_cpp_int< 32 > dw11_cpp_int_t;
        dw11_cpp_int_t int_var__dw11;
        void dw11 (const cpp_int  & l__val);
        cpp_int dw11() const;
    
        typedef pu_cpp_int< 4 > dw_cnt_cpp_int_t;
        dw_cnt_cpp_int_t int_var__dw_cnt;
        void dw_cnt (const cpp_int  & l__val);
        cpp_int dw_cnt() const;
    
        typedef pu_cpp_int< 3 > port_id_cpp_int_t;
        port_id_cpp_int_t int_var__port_id;
        void port_id (const cpp_int  & l__val);
        cpp_int port_id() const;
    
        typedef pu_cpp_int< 1 > cmd_go_cpp_int_t;
        cmd_go_cpp_int_t int_var__cmd_go;
        void cmd_go (const cpp_int  & l__val);
        cpp_int cmd_go() const;
    
}; // cap_pxb_csr_cfg_itr_raw_tlp_t
    
class cap_pxb_csr_sta_tgt_marker_rx_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_tgt_marker_rx_t(string name = "cap_pxb_csr_sta_tgt_marker_rx_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_tgt_marker_rx_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > port0_cpp_int_t;
        port0_cpp_int_t int_var__port0;
        void port0 (const cpp_int  & l__val);
        cpp_int port0() const;
    
        typedef pu_cpp_int< 1 > port1_cpp_int_t;
        port1_cpp_int_t int_var__port1;
        void port1 (const cpp_int  & l__val);
        cpp_int port1() const;
    
        typedef pu_cpp_int< 1 > port2_cpp_int_t;
        port2_cpp_int_t int_var__port2;
        void port2 (const cpp_int  & l__val);
        cpp_int port2() const;
    
        typedef pu_cpp_int< 1 > port3_cpp_int_t;
        port3_cpp_int_t int_var__port3;
        void port3 (const cpp_int  & l__val);
        cpp_int port3() const;
    
        typedef pu_cpp_int< 1 > port4_cpp_int_t;
        port4_cpp_int_t int_var__port4;
        void port4 (const cpp_int  & l__val);
        cpp_int port4() const;
    
        typedef pu_cpp_int< 1 > port5_cpp_int_t;
        port5_cpp_int_t int_var__port5;
        void port5 (const cpp_int  & l__val);
        cpp_int port5() const;
    
        typedef pu_cpp_int< 1 > port6_cpp_int_t;
        port6_cpp_int_t int_var__port6;
        void port6 (const cpp_int  & l__val);
        cpp_int port6() const;
    
        typedef pu_cpp_int< 1 > port7_cpp_int_t;
        port7_cpp_int_t int_var__port7;
        void port7 (const cpp_int  & l__val);
        cpp_int port7() const;
    
}; // cap_pxb_csr_sta_tgt_marker_rx_t
    
class cap_pxb_csr_cfg_tgt_doorbell_base_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_cfg_tgt_doorbell_base_t(string name = "cap_pxb_csr_cfg_tgt_doorbell_base_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_cfg_tgt_doorbell_base_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > addr_33_24_cpp_int_t;
        addr_33_24_cpp_int_t int_var__addr_33_24;
        void addr_33_24 (const cpp_int  & l__val);
        cpp_int addr_33_24() const;
    
}; // cap_pxb_csr_cfg_tgt_doorbell_base_t
    
class cap_pxb_csr_cnt_itr_tot_atomic_req_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_cnt_itr_tot_atomic_req_t(string name = "cap_pxb_csr_cnt_itr_tot_atomic_req_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_cnt_itr_tot_atomic_req_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sent_cpp_int_t;
        sent_cpp_int_t int_var__sent;
        void sent (const cpp_int  & l__val);
        cpp_int sent() const;
    
}; // cap_pxb_csr_cnt_itr_tot_atomic_req_t
    
class cap_pxb_csr_cnt_itr_intx_assert_msg_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_cnt_itr_intx_assert_msg_t(string name = "cap_pxb_csr_cnt_itr_intx_assert_msg_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_cnt_itr_intx_assert_msg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sent_cpp_int_t;
        sent_cpp_int_t int_var__sent;
        void sent (const cpp_int  & l__val);
        cpp_int sent() const;
    
}; // cap_pxb_csr_cnt_itr_intx_assert_msg_t
    
class cap_pxb_csr_cnt_itr_tot_msg_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_cnt_itr_tot_msg_t(string name = "cap_pxb_csr_cnt_itr_tot_msg_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_cnt_itr_tot_msg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > sent_cpp_int_t;
        sent_cpp_int_t int_var__sent;
        void sent (const cpp_int  & l__val);
        cpp_int sent() const;
    
}; // cap_pxb_csr_cnt_itr_tot_msg_t
    
class cap_pxb_csr_sta_itr_axi_rd_num_ids_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_itr_axi_rd_num_ids_t(string name = "cap_pxb_csr_sta_itr_axi_rd_num_ids_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_itr_axi_rd_num_ids_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > pending_cpp_int_t;
        pending_cpp_int_t int_var__pending;
        void pending (const cpp_int  & l__val);
        cpp_int pending() const;
    
}; // cap_pxb_csr_sta_itr_axi_rd_num_ids_t
    
class cap_pxb_csr_sta_itr_axi_wr_num_ids_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_itr_axi_wr_num_ids_t(string name = "cap_pxb_csr_sta_itr_axi_wr_num_ids_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_itr_axi_wr_num_ids_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > pending_cpp_int_t;
        pending_cpp_int_t int_var__pending;
        void pending (const cpp_int  & l__val);
        cpp_int pending() const;
    
}; // cap_pxb_csr_sta_itr_axi_wr_num_ids_t
    
class cap_pxb_csr_cnt_itr_tot_axi_rd_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_cnt_itr_tot_axi_rd_t(string name = "cap_pxb_csr_cnt_itr_tot_axi_rd_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_cnt_itr_tot_axi_rd_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > req_cpp_int_t;
        req_cpp_int_t int_var__req;
        void req (const cpp_int  & l__val);
        cpp_int req() const;
    
}; // cap_pxb_csr_cnt_itr_tot_axi_rd_t
    
class cap_pxb_csr_cnt_itr_tot_axi_wr_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_cnt_itr_tot_axi_wr_t(string name = "cap_pxb_csr_cnt_itr_tot_axi_wr_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_cnt_itr_tot_axi_wr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > req_cpp_int_t;
        req_cpp_int_t int_var__req;
        void req (const cpp_int  & l__val);
        cpp_int req() const;
    
}; // cap_pxb_csr_cnt_itr_tot_axi_wr_t
    
class cap_pxb_csr_cfg_tgt_vf_cfg_stride_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_cfg_tgt_vf_cfg_stride_t(string name = "cap_pxb_csr_cfg_tgt_vf_cfg_stride_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_cfg_tgt_vf_cfg_stride_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 5 > p_shift_cpp_int_t;
        p_shift_cpp_int_t int_var__p_shift;
        void p_shift (const cpp_int  & l__val);
        cpp_int p_shift() const;
    
        typedef pu_cpp_int< 5 > b_shift_cpp_int_t;
        b_shift_cpp_int_t int_var__b_shift;
        void b_shift (const cpp_int  & l__val);
        cpp_int b_shift() const;
    
        typedef pu_cpp_int< 5 > d_shift_cpp_int_t;
        d_shift_cpp_int_t int_var__d_shift;
        void d_shift (const cpp_int  & l__val);
        cpp_int d_shift() const;
    
        typedef pu_cpp_int< 5 > f_shift_cpp_int_t;
        f_shift_cpp_int_t int_var__f_shift;
        void f_shift (const cpp_int  & l__val);
        cpp_int f_shift() const;
    
        typedef pu_cpp_int< 10 > resource_dwsize_cpp_int_t;
        resource_dwsize_cpp_int_t int_var__resource_dwsize;
        void resource_dwsize (const cpp_int  & l__val);
        cpp_int resource_dwsize() const;
    
}; // cap_pxb_csr_cfg_tgt_vf_cfg_stride_t
    
class cap_pxb_csr_sat_tgt_rx_drop_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sat_tgt_rx_drop_t(string name = "cap_pxb_csr_sat_tgt_rx_drop_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sat_tgt_rx_drop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > port0_cpp_int_t;
        port0_cpp_int_t int_var__port0;
        void port0 (const cpp_int  & l__val);
        cpp_int port0() const;
    
        typedef pu_cpp_int< 8 > port1_cpp_int_t;
        port1_cpp_int_t int_var__port1;
        void port1 (const cpp_int  & l__val);
        cpp_int port1() const;
    
        typedef pu_cpp_int< 8 > port2_cpp_int_t;
        port2_cpp_int_t int_var__port2;
        void port2 (const cpp_int  & l__val);
        cpp_int port2() const;
    
        typedef pu_cpp_int< 8 > port3_cpp_int_t;
        port3_cpp_int_t int_var__port3;
        void port3 (const cpp_int  & l__val);
        cpp_int port3() const;
    
        typedef pu_cpp_int< 8 > port4_cpp_int_t;
        port4_cpp_int_t int_var__port4;
        void port4 (const cpp_int  & l__val);
        cpp_int port4() const;
    
        typedef pu_cpp_int< 8 > port5_cpp_int_t;
        port5_cpp_int_t int_var__port5;
        void port5 (const cpp_int  & l__val);
        cpp_int port5() const;
    
        typedef pu_cpp_int< 8 > port6_cpp_int_t;
        port6_cpp_int_t int_var__port6;
        void port6 (const cpp_int  & l__val);
        cpp_int port6() const;
    
        typedef pu_cpp_int< 8 > port7_cpp_int_t;
        port7_cpp_int_t int_var__port7;
        void port7 (const cpp_int  & l__val);
        cpp_int port7() const;
    
}; // cap_pxb_csr_sat_tgt_rx_drop_t
    
class cap_pxb_csr_sta_tgt_ecc_romask_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_tgt_ecc_romask_t(string name = "cap_pxb_csr_sta_tgt_ecc_romask_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_tgt_ecc_romask_t();
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
    
        typedef pu_cpp_int< 7 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 7 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_tgt_ecc_romask_t
    
class cap_pxb_csr_sta_tgt_ecc_prt_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_tgt_ecc_prt_t(string name = "cap_pxb_csr_sta_tgt_ecc_prt_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_tgt_ecc_prt_t();
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
    
        typedef pu_cpp_int< 8 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 12 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_tgt_ecc_prt_t
    
class cap_pxb_csr_sta_tgt_ecc_pmr_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_tgt_ecc_pmr_t(string name = "cap_pxb_csr_sta_tgt_ecc_pmr_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_tgt_ecc_pmr_t();
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
    
        typedef pu_cpp_int< 8 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 10 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_tgt_ecc_pmr_t
    
class cap_pxb_csr_sta_tgt_ecc_cplst_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_tgt_ecc_cplst_t(string name = "cap_pxb_csr_sta_tgt_ecc_cplst_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_tgt_ecc_cplst_t();
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
    
        typedef pu_cpp_int< 8 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 7 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_tgt_ecc_cplst_t
    
class cap_pxb_csr_sta_tgt_ecc_rxinfo_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_tgt_ecc_rxinfo_t(string name = "cap_pxb_csr_sta_tgt_ecc_rxinfo_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_tgt_ecc_rxinfo_t();
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
    
        typedef pu_cpp_int< 6 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 10 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_tgt_ecc_rxinfo_t
    
class cap_pxb_csr_sta_tgt_ecc_aximst_4_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_tgt_ecc_aximst_4_t(string name = "cap_pxb_csr_sta_tgt_ecc_aximst_4_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_tgt_ecc_aximst_4_t();
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
    
        typedef pu_cpp_int< 9 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 7 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_tgt_ecc_aximst_4_t
    
class cap_pxb_csr_sta_tgt_ecc_aximst_3_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_tgt_ecc_aximst_3_t(string name = "cap_pxb_csr_sta_tgt_ecc_aximst_3_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_tgt_ecc_aximst_3_t();
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
    
        typedef pu_cpp_int< 9 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 7 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_tgt_ecc_aximst_3_t
    
class cap_pxb_csr_sta_tgt_ecc_aximst_2_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_tgt_ecc_aximst_2_t(string name = "cap_pxb_csr_sta_tgt_ecc_aximst_2_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_tgt_ecc_aximst_2_t();
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
    
        typedef pu_cpp_int< 9 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 7 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_tgt_ecc_aximst_2_t
    
class cap_pxb_csr_sta_tgt_ecc_aximst_1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_tgt_ecc_aximst_1_t(string name = "cap_pxb_csr_sta_tgt_ecc_aximst_1_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_tgt_ecc_aximst_1_t();
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
    
        typedef pu_cpp_int< 9 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 7 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_tgt_ecc_aximst_1_t
    
class cap_pxb_csr_sta_tgt_ecc_aximst_0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_tgt_ecc_aximst_0_t(string name = "cap_pxb_csr_sta_tgt_ecc_aximst_0_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_tgt_ecc_aximst_0_t();
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
    
        typedef pu_cpp_int< 9 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 7 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_tgt_ecc_aximst_0_t
    
class cap_pxb_csr_sta_tgt_ecc_rxcrbfr_3_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_tgt_ecc_rxcrbfr_3_t(string name = "cap_pxb_csr_sta_tgt_ecc_rxcrbfr_3_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_tgt_ecc_rxcrbfr_3_t();
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
    
        typedef pu_cpp_int< 9 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 10 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_tgt_ecc_rxcrbfr_3_t
    
class cap_pxb_csr_sta_tgt_ecc_rxcrbfr_2_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_tgt_ecc_rxcrbfr_2_t(string name = "cap_pxb_csr_sta_tgt_ecc_rxcrbfr_2_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_tgt_ecc_rxcrbfr_2_t();
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
    
        typedef pu_cpp_int< 9 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 10 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_tgt_ecc_rxcrbfr_2_t
    
class cap_pxb_csr_sta_tgt_ecc_rxcrbfr_1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_tgt_ecc_rxcrbfr_1_t(string name = "cap_pxb_csr_sta_tgt_ecc_rxcrbfr_1_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_tgt_ecc_rxcrbfr_1_t();
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
    
        typedef pu_cpp_int< 9 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 10 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_tgt_ecc_rxcrbfr_1_t
    
class cap_pxb_csr_sta_tgt_ecc_rxcrbfr_0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_tgt_ecc_rxcrbfr_0_t(string name = "cap_pxb_csr_sta_tgt_ecc_rxcrbfr_0_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_tgt_ecc_rxcrbfr_0_t();
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
    
        typedef pu_cpp_int< 9 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 10 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_tgt_ecc_rxcrbfr_0_t
    
class cap_pxb_csr_sta_itr_ecc_cplbfr_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_itr_ecc_cplbfr_t(string name = "cap_pxb_csr_sta_itr_ecc_cplbfr_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_itr_ecc_cplbfr_t();
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
    
        typedef pu_cpp_int< 7 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 9 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_itr_ecc_cplbfr_t
    
class cap_pxb_csr_sta_itr_ecc_rdhdr_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_itr_ecc_rdhdr_t(string name = "cap_pxb_csr_sta_itr_ecc_rdhdr_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_itr_ecc_rdhdr_t();
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
    
        typedef pu_cpp_int< 8 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 7 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_itr_ecc_rdhdr_t
    
class cap_pxb_csr_sta_itr_ecc_wrhdr_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_itr_ecc_wrhdr_t(string name = "cap_pxb_csr_sta_itr_ecc_wrhdr_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_itr_ecc_wrhdr_t();
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
    
        typedef pu_cpp_int< 8 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 7 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_itr_ecc_wrhdr_t
    
class cap_pxb_csr_sta_itr_ecc_portmap_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_itr_ecc_portmap_t(string name = "cap_pxb_csr_sta_itr_ecc_portmap_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_itr_ecc_portmap_t();
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
    
        typedef pu_cpp_int< 7 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 7 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_itr_ecc_portmap_t
    
class cap_pxb_csr_sta_itr_ecc_pcihdrt_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_itr_ecc_pcihdrt_t(string name = "cap_pxb_csr_sta_itr_ecc_pcihdrt_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_itr_ecc_pcihdrt_t();
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
    
        typedef pu_cpp_int< 8 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 11 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_itr_ecc_pcihdrt_t
    
class cap_pxb_csr_sta_itr_ecc_wrbfr_3_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_itr_ecc_wrbfr_3_t(string name = "cap_pxb_csr_sta_itr_ecc_wrbfr_3_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_itr_ecc_wrbfr_3_t();
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
    
        typedef pu_cpp_int< 9 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 10 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_itr_ecc_wrbfr_3_t
    
class cap_pxb_csr_sta_itr_ecc_wrbfr_2_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_itr_ecc_wrbfr_2_t(string name = "cap_pxb_csr_sta_itr_ecc_wrbfr_2_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_itr_ecc_wrbfr_2_t();
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
    
        typedef pu_cpp_int< 9 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 10 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_itr_ecc_wrbfr_2_t
    
class cap_pxb_csr_sta_itr_ecc_wrbfr_1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_itr_ecc_wrbfr_1_t(string name = "cap_pxb_csr_sta_itr_ecc_wrbfr_1_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_itr_ecc_wrbfr_1_t();
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
    
        typedef pu_cpp_int< 9 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 10 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_itr_ecc_wrbfr_1_t
    
class cap_pxb_csr_sta_itr_ecc_wrbfr_0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_itr_ecc_wrbfr_0_t(string name = "cap_pxb_csr_sta_itr_ecc_wrbfr_0_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_itr_ecc_wrbfr_0_t();
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
    
        typedef pu_cpp_int< 9 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 10 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pxb_csr_sta_itr_ecc_wrbfr_0_t
    
class cap_pxb_csr_cfg_tgt_ecc_disable_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_cfg_tgt_ecc_disable_t(string name = "cap_pxb_csr_cfg_tgt_ecc_disable_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_cfg_tgt_ecc_disable_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > rxcrbfr_cor_cpp_int_t;
        rxcrbfr_cor_cpp_int_t int_var__rxcrbfr_cor;
        void rxcrbfr_cor (const cpp_int  & l__val);
        cpp_int rxcrbfr_cor() const;
    
        typedef pu_cpp_int< 1 > rxcrbfr_det_cpp_int_t;
        rxcrbfr_det_cpp_int_t int_var__rxcrbfr_det;
        void rxcrbfr_det (const cpp_int  & l__val);
        cpp_int rxcrbfr_det() const;
    
        typedef pu_cpp_int< 1 > rxcrbfr_dhs_cpp_int_t;
        rxcrbfr_dhs_cpp_int_t int_var__rxcrbfr_dhs;
        void rxcrbfr_dhs (const cpp_int  & l__val);
        cpp_int rxcrbfr_dhs() const;
    
        typedef pu_cpp_int< 1 > rxinfo_cor_cpp_int_t;
        rxinfo_cor_cpp_int_t int_var__rxinfo_cor;
        void rxinfo_cor (const cpp_int  & l__val);
        cpp_int rxinfo_cor() const;
    
        typedef pu_cpp_int< 1 > rxinfo_det_cpp_int_t;
        rxinfo_det_cpp_int_t int_var__rxinfo_det;
        void rxinfo_det (const cpp_int  & l__val);
        cpp_int rxinfo_det() const;
    
        typedef pu_cpp_int< 1 > rxinfo_dhs_cpp_int_t;
        rxinfo_dhs_cpp_int_t int_var__rxinfo_dhs;
        void rxinfo_dhs (const cpp_int  & l__val);
        cpp_int rxinfo_dhs() const;
    
        typedef pu_cpp_int< 1 > pmr_cor_cpp_int_t;
        pmr_cor_cpp_int_t int_var__pmr_cor;
        void pmr_cor (const cpp_int  & l__val);
        cpp_int pmr_cor() const;
    
        typedef pu_cpp_int< 1 > pmr_det_cpp_int_t;
        pmr_det_cpp_int_t int_var__pmr_det;
        void pmr_det (const cpp_int  & l__val);
        cpp_int pmr_det() const;
    
        typedef pu_cpp_int< 1 > pmr_dhs_cpp_int_t;
        pmr_dhs_cpp_int_t int_var__pmr_dhs;
        void pmr_dhs (const cpp_int  & l__val);
        cpp_int pmr_dhs() const;
    
        typedef pu_cpp_int< 1 > prt_cor_cpp_int_t;
        prt_cor_cpp_int_t int_var__prt_cor;
        void prt_cor (const cpp_int  & l__val);
        cpp_int prt_cor() const;
    
        typedef pu_cpp_int< 1 > prt_det_cpp_int_t;
        prt_det_cpp_int_t int_var__prt_det;
        void prt_det (const cpp_int  & l__val);
        cpp_int prt_det() const;
    
        typedef pu_cpp_int< 1 > prt_dhs_cpp_int_t;
        prt_dhs_cpp_int_t int_var__prt_dhs;
        void prt_dhs (const cpp_int  & l__val);
        cpp_int prt_dhs() const;
    
        typedef pu_cpp_int< 1 > cplst_cor_cpp_int_t;
        cplst_cor_cpp_int_t int_var__cplst_cor;
        void cplst_cor (const cpp_int  & l__val);
        cpp_int cplst_cor() const;
    
        typedef pu_cpp_int< 1 > cplst_det_cpp_int_t;
        cplst_det_cpp_int_t int_var__cplst_det;
        void cplst_det (const cpp_int  & l__val);
        cpp_int cplst_det() const;
    
        typedef pu_cpp_int< 1 > cplst_dhs_cpp_int_t;
        cplst_dhs_cpp_int_t int_var__cplst_dhs;
        void cplst_dhs (const cpp_int  & l__val);
        cpp_int cplst_dhs() const;
    
        typedef pu_cpp_int< 1 > aximst_cor_cpp_int_t;
        aximst_cor_cpp_int_t int_var__aximst_cor;
        void aximst_cor (const cpp_int  & l__val);
        cpp_int aximst_cor() const;
    
        typedef pu_cpp_int< 1 > aximst_det_cpp_int_t;
        aximst_det_cpp_int_t int_var__aximst_det;
        void aximst_det (const cpp_int  & l__val);
        cpp_int aximst_det() const;
    
        typedef pu_cpp_int< 1 > aximst_dhs_cpp_int_t;
        aximst_dhs_cpp_int_t int_var__aximst_dhs;
        void aximst_dhs (const cpp_int  & l__val);
        cpp_int aximst_dhs() const;
    
        typedef pu_cpp_int< 1 > romask_cor_cpp_int_t;
        romask_cor_cpp_int_t int_var__romask_cor;
        void romask_cor (const cpp_int  & l__val);
        cpp_int romask_cor() const;
    
        typedef pu_cpp_int< 1 > romask_det_cpp_int_t;
        romask_det_cpp_int_t int_var__romask_det;
        void romask_det (const cpp_int  & l__val);
        cpp_int romask_det() const;
    
        typedef pu_cpp_int< 1 > romask_dhs_cpp_int_t;
        romask_dhs_cpp_int_t int_var__romask_dhs;
        void romask_dhs (const cpp_int  & l__val);
        cpp_int romask_dhs() const;
    
}; // cap_pxb_csr_cfg_tgt_ecc_disable_t
    
class cap_pxb_csr_cfg_itr_ecc_disable_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_cfg_itr_ecc_disable_t(string name = "cap_pxb_csr_cfg_itr_ecc_disable_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_cfg_itr_ecc_disable_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > pcihdrt_cor_cpp_int_t;
        pcihdrt_cor_cpp_int_t int_var__pcihdrt_cor;
        void pcihdrt_cor (const cpp_int  & l__val);
        cpp_int pcihdrt_cor() const;
    
        typedef pu_cpp_int< 1 > pcihdrt_det_cpp_int_t;
        pcihdrt_det_cpp_int_t int_var__pcihdrt_det;
        void pcihdrt_det (const cpp_int  & l__val);
        cpp_int pcihdrt_det() const;
    
        typedef pu_cpp_int< 1 > pcihdrt_dhs_cpp_int_t;
        pcihdrt_dhs_cpp_int_t int_var__pcihdrt_dhs;
        void pcihdrt_dhs (const cpp_int  & l__val);
        cpp_int pcihdrt_dhs() const;
    
        typedef pu_cpp_int< 1 > portmap_cor_cpp_int_t;
        portmap_cor_cpp_int_t int_var__portmap_cor;
        void portmap_cor (const cpp_int  & l__val);
        cpp_int portmap_cor() const;
    
        typedef pu_cpp_int< 1 > portmap_det_cpp_int_t;
        portmap_det_cpp_int_t int_var__portmap_det;
        void portmap_det (const cpp_int  & l__val);
        cpp_int portmap_det() const;
    
        typedef pu_cpp_int< 1 > portmap_dhs_cpp_int_t;
        portmap_dhs_cpp_int_t int_var__portmap_dhs;
        void portmap_dhs (const cpp_int  & l__val);
        cpp_int portmap_dhs() const;
    
        typedef pu_cpp_int< 1 > wrbfr_cor_cpp_int_t;
        wrbfr_cor_cpp_int_t int_var__wrbfr_cor;
        void wrbfr_cor (const cpp_int  & l__val);
        cpp_int wrbfr_cor() const;
    
        typedef pu_cpp_int< 1 > wrbfr_det_cpp_int_t;
        wrbfr_det_cpp_int_t int_var__wrbfr_det;
        void wrbfr_det (const cpp_int  & l__val);
        cpp_int wrbfr_det() const;
    
        typedef pu_cpp_int< 1 > wrbfr_dhs_cpp_int_t;
        wrbfr_dhs_cpp_int_t int_var__wrbfr_dhs;
        void wrbfr_dhs (const cpp_int  & l__val);
        cpp_int wrbfr_dhs() const;
    
        typedef pu_cpp_int< 1 > wrhdr_cor_cpp_int_t;
        wrhdr_cor_cpp_int_t int_var__wrhdr_cor;
        void wrhdr_cor (const cpp_int  & l__val);
        cpp_int wrhdr_cor() const;
    
        typedef pu_cpp_int< 1 > wrhdr_det_cpp_int_t;
        wrhdr_det_cpp_int_t int_var__wrhdr_det;
        void wrhdr_det (const cpp_int  & l__val);
        cpp_int wrhdr_det() const;
    
        typedef pu_cpp_int< 1 > wrhdr_dhs_cpp_int_t;
        wrhdr_dhs_cpp_int_t int_var__wrhdr_dhs;
        void wrhdr_dhs (const cpp_int  & l__val);
        cpp_int wrhdr_dhs() const;
    
        typedef pu_cpp_int< 1 > rdhdr_cor_cpp_int_t;
        rdhdr_cor_cpp_int_t int_var__rdhdr_cor;
        void rdhdr_cor (const cpp_int  & l__val);
        cpp_int rdhdr_cor() const;
    
        typedef pu_cpp_int< 1 > rdhdr_det_cpp_int_t;
        rdhdr_det_cpp_int_t int_var__rdhdr_det;
        void rdhdr_det (const cpp_int  & l__val);
        cpp_int rdhdr_det() const;
    
        typedef pu_cpp_int< 1 > rdhdr_dhs_cpp_int_t;
        rdhdr_dhs_cpp_int_t int_var__rdhdr_dhs;
        void rdhdr_dhs (const cpp_int  & l__val);
        cpp_int rdhdr_dhs() const;
    
        typedef pu_cpp_int< 1 > cplbfr_cor_cpp_int_t;
        cplbfr_cor_cpp_int_t int_var__cplbfr_cor;
        void cplbfr_cor (const cpp_int  & l__val);
        cpp_int cplbfr_cor() const;
    
        typedef pu_cpp_int< 1 > cplbfr_det_cpp_int_t;
        cplbfr_det_cpp_int_t int_var__cplbfr_det;
        void cplbfr_det (const cpp_int  & l__val);
        cpp_int cplbfr_det() const;
    
        typedef pu_cpp_int< 1 > cplbfr_dhs_cpp_int_t;
        cplbfr_dhs_cpp_int_t int_var__cplbfr_dhs;
        void cplbfr_dhs (const cpp_int  & l__val);
        cpp_int cplbfr_dhs() const;
    
}; // cap_pxb_csr_cfg_itr_ecc_disable_t
    
class cap_pxb_csr_sta_tgt_pmt_ind_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_tgt_pmt_ind_t(string name = "cap_pxb_csr_sta_tgt_pmt_ind_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_tgt_pmt_ind_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > hit_cpp_int_t;
        hit_cpp_int_t int_var__hit;
        void hit (const cpp_int  & l__val);
        cpp_int hit() const;
    
        typedef pu_cpp_int< 10 > hit_addr_cpp_int_t;
        hit_addr_cpp_int_t int_var__hit_addr;
        void hit_addr (const cpp_int  & l__val);
        cpp_int hit_addr() const;
    
}; // cap_pxb_csr_sta_tgt_pmt_ind_t
    
class cap_pxb_csr_cfg_tgt_pmt_ind_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_cfg_tgt_pmt_ind_t(string name = "cap_pxb_csr_cfg_tgt_pmt_ind_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_cfg_tgt_pmt_ind_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 64 > key_cpp_int_t;
        key_cpp_int_t int_var__key;
        void key (const cpp_int  & l__val);
        cpp_int key() const;
    
        typedef pu_cpp_int< 64 > mask_cpp_int_t;
        mask_cpp_int_t int_var__mask;
        void mask (const cpp_int  & l__val);
        cpp_int mask() const;
    
}; // cap_pxb_csr_cfg_tgt_pmt_ind_t
    
class cap_pxb_csr_cfg_tgt_pmt_grst_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_cfg_tgt_pmt_grst_t(string name = "cap_pxb_csr_cfg_tgt_pmt_grst_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_cfg_tgt_pmt_grst_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > vld_cpp_int_t;
        vld_cpp_int_t int_var__vld;
        void vld (const cpp_int  & l__val);
        cpp_int vld() const;
    
}; // cap_pxb_csr_cfg_tgt_pmt_grst_t
    
class cap_pxb_csr_cfg_pcie_local_memaddr_decode_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_cfg_pcie_local_memaddr_decode_t(string name = "cap_pxb_csr_cfg_pcie_local_memaddr_decode_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_cfg_pcie_local_memaddr_decode_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 3 > region0_cpp_int_t;
        region0_cpp_int_t int_var__region0;
        void region0 (const cpp_int  & l__val);
        cpp_int region0() const;
    
        typedef pu_cpp_int< 3 > region1_cpp_int_t;
        region1_cpp_int_t int_var__region1;
        void region1 (const cpp_int  & l__val);
        cpp_int region1() const;
    
        typedef pu_cpp_int< 3 > region2_cpp_int_t;
        region2_cpp_int_t int_var__region2;
        void region2 (const cpp_int  & l__val);
        cpp_int region2() const;
    
        typedef pu_cpp_int< 3 > region3_cpp_int_t;
        region3_cpp_int_t int_var__region3;
        void region3 (const cpp_int  & l__val);
        cpp_int region3() const;
    
        typedef pu_cpp_int< 3 > region4_cpp_int_t;
        region4_cpp_int_t int_var__region4;
        void region4 (const cpp_int  & l__val);
        cpp_int region4() const;
    
        typedef pu_cpp_int< 3 > region5_cpp_int_t;
        region5_cpp_int_t int_var__region5;
        void region5 (const cpp_int  & l__val);
        cpp_int region5() const;
    
        typedef pu_cpp_int< 3 > region6_cpp_int_t;
        region6_cpp_int_t int_var__region6;
        void region6 (const cpp_int  & l__val);
        cpp_int region6() const;
    
        typedef pu_cpp_int< 3 > region7_cpp_int_t;
        region7_cpp_int_t int_var__region7;
        void region7 (const cpp_int  & l__val);
        cpp_int region7() const;
    
        typedef pu_cpp_int< 3 > region8_cpp_int_t;
        region8_cpp_int_t int_var__region8;
        void region8 (const cpp_int  & l__val);
        cpp_int region8() const;
    
        typedef pu_cpp_int< 3 > region9_cpp_int_t;
        region9_cpp_int_t int_var__region9;
        void region9 (const cpp_int  & l__val);
        cpp_int region9() const;
    
        typedef pu_cpp_int< 3 > region10_cpp_int_t;
        region10_cpp_int_t int_var__region10;
        void region10 (const cpp_int  & l__val);
        cpp_int region10() const;
    
        typedef pu_cpp_int< 3 > region11_cpp_int_t;
        region11_cpp_int_t int_var__region11;
        void region11 (const cpp_int  & l__val);
        cpp_int region11() const;
    
        typedef pu_cpp_int< 3 > region12_cpp_int_t;
        region12_cpp_int_t int_var__region12;
        void region12 (const cpp_int  & l__val);
        cpp_int region12() const;
    
        typedef pu_cpp_int< 3 > region13_cpp_int_t;
        region13_cpp_int_t int_var__region13;
        void region13 (const cpp_int  & l__val);
        cpp_int region13() const;
    
        typedef pu_cpp_int< 3 > region14_cpp_int_t;
        region14_cpp_int_t int_var__region14;
        void region14 (const cpp_int  & l__val);
        cpp_int region14() const;
    
        typedef pu_cpp_int< 3 > region15_cpp_int_t;
        region15_cpp_int_t int_var__region15;
        void region15 (const cpp_int  & l__val);
        cpp_int region15() const;
    
        typedef pu_cpp_int< 4 > atomic_cpp_int_t;
        atomic_cpp_int_t int_var__atomic;
        void atomic (const cpp_int  & l__val);
        cpp_int atomic() const;
    
        typedef pu_cpp_int< 4 > rc_cfg_cpp_int_t;
        rc_cfg_cpp_int_t int_var__rc_cfg;
        void rc_cfg (const cpp_int  & l__val);
        cpp_int rc_cfg() const;
    
        typedef pu_cpp_int< 19 > atomic_page_id_cpp_int_t;
        atomic_page_id_cpp_int_t int_var__atomic_page_id;
        void atomic_page_id (const cpp_int  & l__val);
        cpp_int atomic_page_id() const;
    
        typedef pu_cpp_int< 19 > rc_cfg_page_id_cpp_int_t;
        rc_cfg_page_id_cpp_int_t int_var__rc_cfg_page_id;
        void rc_cfg_page_id (const cpp_int  & l__val);
        cpp_int rc_cfg_page_id() const;
    
        typedef pu_cpp_int< 19 > rc_io64k_cpp_int_t;
        rc_io64k_cpp_int_t int_var__rc_io64k;
        void rc_io64k (const cpp_int  & l__val);
        cpp_int rc_io64k() const;
    
}; // cap_pxb_csr_cfg_pcie_local_memaddr_decode_t
    
class cap_pxb_csr_cfg_tgt_rc_key_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_cfg_tgt_rc_key_t(string name = "cap_pxb_csr_cfg_tgt_rc_key_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_cfg_tgt_rc_key_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 2 > sel_cpp_int_t;
        sel_cpp_int_t int_var__sel;
        void sel (const cpp_int  & l__val);
        cpp_int sel() const;
    
}; // cap_pxb_csr_cfg_tgt_rc_key_t
    
class cap_pxb_csr_cfg_pmt_tbl_id_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_cfg_pmt_tbl_id_t(string name = "cap_pxb_csr_cfg_pmt_tbl_id_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_cfg_pmt_tbl_id_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 2 > memio_tbl_id0_cpp_int_t;
        memio_tbl_id0_cpp_int_t int_var__memio_tbl_id0;
        void memio_tbl_id0 (const cpp_int  & l__val);
        cpp_int memio_tbl_id0() const;
    
        typedef pu_cpp_int< 2 > cfg_tbl_id0_cpp_int_t;
        cfg_tbl_id0_cpp_int_t int_var__cfg_tbl_id0;
        void cfg_tbl_id0 (const cpp_int  & l__val);
        cpp_int cfg_tbl_id0() const;
    
        typedef pu_cpp_int< 2 > rc_tbl_id0_cpp_int_t;
        rc_tbl_id0_cpp_int_t int_var__rc_tbl_id0;
        void rc_tbl_id0 (const cpp_int  & l__val);
        cpp_int rc_tbl_id0() const;
    
        typedef pu_cpp_int< 2 > default_tbl_id0_cpp_int_t;
        default_tbl_id0_cpp_int_t int_var__default_tbl_id0;
        void default_tbl_id0 (const cpp_int  & l__val);
        cpp_int default_tbl_id0() const;
    
        typedef pu_cpp_int< 2 > memio_tbl_id1_cpp_int_t;
        memio_tbl_id1_cpp_int_t int_var__memio_tbl_id1;
        void memio_tbl_id1 (const cpp_int  & l__val);
        cpp_int memio_tbl_id1() const;
    
        typedef pu_cpp_int< 2 > cfg_tbl_id1_cpp_int_t;
        cfg_tbl_id1_cpp_int_t int_var__cfg_tbl_id1;
        void cfg_tbl_id1 (const cpp_int  & l__val);
        cpp_int cfg_tbl_id1() const;
    
        typedef pu_cpp_int< 2 > rc_tbl_id1_cpp_int_t;
        rc_tbl_id1_cpp_int_t int_var__rc_tbl_id1;
        void rc_tbl_id1 (const cpp_int  & l__val);
        cpp_int rc_tbl_id1() const;
    
        typedef pu_cpp_int< 2 > default_tbl_id1_cpp_int_t;
        default_tbl_id1_cpp_int_t int_var__default_tbl_id1;
        void default_tbl_id1 (const cpp_int  & l__val);
        cpp_int default_tbl_id1() const;
    
        typedef pu_cpp_int< 2 > memio_tbl_id2_cpp_int_t;
        memio_tbl_id2_cpp_int_t int_var__memio_tbl_id2;
        void memio_tbl_id2 (const cpp_int  & l__val);
        cpp_int memio_tbl_id2() const;
    
        typedef pu_cpp_int< 2 > cfg_tbl_id2_cpp_int_t;
        cfg_tbl_id2_cpp_int_t int_var__cfg_tbl_id2;
        void cfg_tbl_id2 (const cpp_int  & l__val);
        cpp_int cfg_tbl_id2() const;
    
        typedef pu_cpp_int< 2 > rc_tbl_id2_cpp_int_t;
        rc_tbl_id2_cpp_int_t int_var__rc_tbl_id2;
        void rc_tbl_id2 (const cpp_int  & l__val);
        cpp_int rc_tbl_id2() const;
    
        typedef pu_cpp_int< 2 > default_tbl_id2_cpp_int_t;
        default_tbl_id2_cpp_int_t int_var__default_tbl_id2;
        void default_tbl_id2 (const cpp_int  & l__val);
        cpp_int default_tbl_id2() const;
    
        typedef pu_cpp_int< 2 > memio_tbl_id3_cpp_int_t;
        memio_tbl_id3_cpp_int_t int_var__memio_tbl_id3;
        void memio_tbl_id3 (const cpp_int  & l__val);
        cpp_int memio_tbl_id3() const;
    
        typedef pu_cpp_int< 2 > cfg_tbl_id3_cpp_int_t;
        cfg_tbl_id3_cpp_int_t int_var__cfg_tbl_id3;
        void cfg_tbl_id3 (const cpp_int  & l__val);
        cpp_int cfg_tbl_id3() const;
    
        typedef pu_cpp_int< 2 > rc_tbl_id3_cpp_int_t;
        rc_tbl_id3_cpp_int_t int_var__rc_tbl_id3;
        void rc_tbl_id3 (const cpp_int  & l__val);
        cpp_int rc_tbl_id3() const;
    
        typedef pu_cpp_int< 2 > default_tbl_id3_cpp_int_t;
        default_tbl_id3_cpp_int_t int_var__default_tbl_id3;
        void default_tbl_id3 (const cpp_int  & l__val);
        cpp_int default_tbl_id3() const;
    
        typedef pu_cpp_int< 2 > memio_tbl_id4_cpp_int_t;
        memio_tbl_id4_cpp_int_t int_var__memio_tbl_id4;
        void memio_tbl_id4 (const cpp_int  & l__val);
        cpp_int memio_tbl_id4() const;
    
        typedef pu_cpp_int< 2 > cfg_tbl_id4_cpp_int_t;
        cfg_tbl_id4_cpp_int_t int_var__cfg_tbl_id4;
        void cfg_tbl_id4 (const cpp_int  & l__val);
        cpp_int cfg_tbl_id4() const;
    
        typedef pu_cpp_int< 2 > rc_tbl_id4_cpp_int_t;
        rc_tbl_id4_cpp_int_t int_var__rc_tbl_id4;
        void rc_tbl_id4 (const cpp_int  & l__val);
        cpp_int rc_tbl_id4() const;
    
        typedef pu_cpp_int< 2 > default_tbl_id4_cpp_int_t;
        default_tbl_id4_cpp_int_t int_var__default_tbl_id4;
        void default_tbl_id4 (const cpp_int  & l__val);
        cpp_int default_tbl_id4() const;
    
        typedef pu_cpp_int< 2 > memio_tbl_id5_cpp_int_t;
        memio_tbl_id5_cpp_int_t int_var__memio_tbl_id5;
        void memio_tbl_id5 (const cpp_int  & l__val);
        cpp_int memio_tbl_id5() const;
    
        typedef pu_cpp_int< 2 > cfg_tbl_id5_cpp_int_t;
        cfg_tbl_id5_cpp_int_t int_var__cfg_tbl_id5;
        void cfg_tbl_id5 (const cpp_int  & l__val);
        cpp_int cfg_tbl_id5() const;
    
        typedef pu_cpp_int< 2 > rc_tbl_id5_cpp_int_t;
        rc_tbl_id5_cpp_int_t int_var__rc_tbl_id5;
        void rc_tbl_id5 (const cpp_int  & l__val);
        cpp_int rc_tbl_id5() const;
    
        typedef pu_cpp_int< 2 > default_tbl_id5_cpp_int_t;
        default_tbl_id5_cpp_int_t int_var__default_tbl_id5;
        void default_tbl_id5 (const cpp_int  & l__val);
        cpp_int default_tbl_id5() const;
    
        typedef pu_cpp_int< 2 > memio_tbl_id6_cpp_int_t;
        memio_tbl_id6_cpp_int_t int_var__memio_tbl_id6;
        void memio_tbl_id6 (const cpp_int  & l__val);
        cpp_int memio_tbl_id6() const;
    
        typedef pu_cpp_int< 2 > cfg_tbl_id6_cpp_int_t;
        cfg_tbl_id6_cpp_int_t int_var__cfg_tbl_id6;
        void cfg_tbl_id6 (const cpp_int  & l__val);
        cpp_int cfg_tbl_id6() const;
    
        typedef pu_cpp_int< 2 > rc_tbl_id6_cpp_int_t;
        rc_tbl_id6_cpp_int_t int_var__rc_tbl_id6;
        void rc_tbl_id6 (const cpp_int  & l__val);
        cpp_int rc_tbl_id6() const;
    
        typedef pu_cpp_int< 2 > default_tbl_id6_cpp_int_t;
        default_tbl_id6_cpp_int_t int_var__default_tbl_id6;
        void default_tbl_id6 (const cpp_int  & l__val);
        cpp_int default_tbl_id6() const;
    
        typedef pu_cpp_int< 2 > memio_tbl_id7_cpp_int_t;
        memio_tbl_id7_cpp_int_t int_var__memio_tbl_id7;
        void memio_tbl_id7 (const cpp_int  & l__val);
        cpp_int memio_tbl_id7() const;
    
        typedef pu_cpp_int< 2 > cfg_tbl_id7_cpp_int_t;
        cfg_tbl_id7_cpp_int_t int_var__cfg_tbl_id7;
        void cfg_tbl_id7 (const cpp_int  & l__val);
        cpp_int cfg_tbl_id7() const;
    
        typedef pu_cpp_int< 2 > rc_tbl_id7_cpp_int_t;
        rc_tbl_id7_cpp_int_t int_var__rc_tbl_id7;
        void rc_tbl_id7 (const cpp_int  & l__val);
        cpp_int rc_tbl_id7() const;
    
        typedef pu_cpp_int< 2 > default_tbl_id7_cpp_int_t;
        default_tbl_id7_cpp_int_t int_var__default_tbl_id7;
        void default_tbl_id7 (const cpp_int  & l__val);
        cpp_int default_tbl_id7() const;
    
}; // cap_pxb_csr_cfg_pmt_tbl_id_t
    
class cap_pxb_csr_cfg_tgt_notify_en_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_cfg_tgt_notify_en_t(string name = "cap_pxb_csr_cfg_tgt_notify_en_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_cfg_tgt_notify_en_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > msg_cpp_int_t;
        msg_cpp_int_t int_var__msg;
        void msg (const cpp_int  & l__val);
        cpp_int msg() const;
    
        typedef pu_cpp_int< 1 > pmv_cpp_int_t;
        pmv_cpp_int_t int_var__pmv;
        void pmv (const cpp_int  & l__val);
        cpp_int pmv() const;
    
        typedef pu_cpp_int< 1 > db_pmv_cpp_int_t;
        db_pmv_cpp_int_t int_var__db_pmv;
        void db_pmv (const cpp_int  & l__val);
        cpp_int db_pmv() const;
    
        typedef pu_cpp_int< 1 > unsupp_cpp_int_t;
        unsupp_cpp_int_t int_var__unsupp;
        void unsupp (const cpp_int  & l__val);
        cpp_int unsupp() const;
    
        typedef pu_cpp_int< 1 > atomic_cpp_int_t;
        atomic_cpp_int_t int_var__atomic;
        void atomic (const cpp_int  & l__val);
        cpp_int atomic() const;
    
        typedef pu_cpp_int< 1 > pmt_miss_cpp_int_t;
        pmt_miss_cpp_int_t int_var__pmt_miss;
        void pmt_miss (const cpp_int  & l__val);
        cpp_int pmt_miss() const;
    
        typedef pu_cpp_int< 1 > pmr_invalid_cpp_int_t;
        pmr_invalid_cpp_int_t int_var__pmr_invalid;
        void pmr_invalid (const cpp_int  & l__val);
        cpp_int pmr_invalid() const;
    
        typedef pu_cpp_int< 1 > prt_invalid_cpp_int_t;
        prt_invalid_cpp_int_t int_var__prt_invalid;
        void prt_invalid (const cpp_int  & l__val);
        cpp_int prt_invalid() const;
    
        typedef pu_cpp_int< 1 > rc_vfid_miss_cpp_int_t;
        rc_vfid_miss_cpp_int_t int_var__rc_vfid_miss;
        void rc_vfid_miss (const cpp_int  & l__val);
        cpp_int rc_vfid_miss() const;
    
        typedef pu_cpp_int< 1 > prt_oor_cpp_int_t;
        prt_oor_cpp_int_t int_var__prt_oor;
        void prt_oor (const cpp_int  & l__val);
        cpp_int prt_oor() const;
    
        typedef pu_cpp_int< 1 > vfid_oor_cpp_int_t;
        vfid_oor_cpp_int_t int_var__vfid_oor;
        void vfid_oor (const cpp_int  & l__val);
        cpp_int vfid_oor() const;
    
        typedef pu_cpp_int< 1 > cfg_bdf_oor_cpp_int_t;
        cfg_bdf_oor_cpp_int_t int_var__cfg_bdf_oor;
        void cfg_bdf_oor (const cpp_int  & l__val);
        cpp_int cfg_bdf_oor() const;
    
        typedef pu_cpp_int< 1 > pmr_ecc_err_cpp_int_t;
        pmr_ecc_err_cpp_int_t int_var__pmr_ecc_err;
        void pmr_ecc_err (const cpp_int  & l__val);
        cpp_int pmr_ecc_err() const;
    
        typedef pu_cpp_int< 1 > prt_ecc_err_cpp_int_t;
        prt_ecc_err_cpp_int_t int_var__prt_ecc_err;
        void prt_ecc_err (const cpp_int  & l__val);
        cpp_int prt_ecc_err() const;
    
}; // cap_pxb_csr_cfg_tgt_notify_en_t
    
class cap_pxb_csr_cfg_tgt_ind_en_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_cfg_tgt_ind_en_t(string name = "cap_pxb_csr_cfg_tgt_ind_en_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_cfg_tgt_ind_en_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > msg_cpp_int_t;
        msg_cpp_int_t int_var__msg;
        void msg (const cpp_int  & l__val);
        cpp_int msg() const;
    
        typedef pu_cpp_int< 1 > pmv_cpp_int_t;
        pmv_cpp_int_t int_var__pmv;
        void pmv (const cpp_int  & l__val);
        cpp_int pmv() const;
    
        typedef pu_cpp_int< 1 > db_pmv_cpp_int_t;
        db_pmv_cpp_int_t int_var__db_pmv;
        void db_pmv (const cpp_int  & l__val);
        cpp_int db_pmv() const;
    
        typedef pu_cpp_int< 1 > unsupp_cpp_int_t;
        unsupp_cpp_int_t int_var__unsupp;
        void unsupp (const cpp_int  & l__val);
        cpp_int unsupp() const;
    
        typedef pu_cpp_int< 1 > atomic_cpp_int_t;
        atomic_cpp_int_t int_var__atomic;
        void atomic (const cpp_int  & l__val);
        cpp_int atomic() const;
    
        typedef pu_cpp_int< 1 > pmt_miss_cpp_int_t;
        pmt_miss_cpp_int_t int_var__pmt_miss;
        void pmt_miss (const cpp_int  & l__val);
        cpp_int pmt_miss() const;
    
        typedef pu_cpp_int< 1 > pmr_invalid_cpp_int_t;
        pmr_invalid_cpp_int_t int_var__pmr_invalid;
        void pmr_invalid (const cpp_int  & l__val);
        cpp_int pmr_invalid() const;
    
        typedef pu_cpp_int< 1 > prt_invalid_cpp_int_t;
        prt_invalid_cpp_int_t int_var__prt_invalid;
        void prt_invalid (const cpp_int  & l__val);
        cpp_int prt_invalid() const;
    
        typedef pu_cpp_int< 1 > rc_vfid_miss_cpp_int_t;
        rc_vfid_miss_cpp_int_t int_var__rc_vfid_miss;
        void rc_vfid_miss (const cpp_int  & l__val);
        cpp_int rc_vfid_miss() const;
    
        typedef pu_cpp_int< 1 > prt_oor_cpp_int_t;
        prt_oor_cpp_int_t int_var__prt_oor;
        void prt_oor (const cpp_int  & l__val);
        cpp_int prt_oor() const;
    
        typedef pu_cpp_int< 1 > vfid_oor_cpp_int_t;
        vfid_oor_cpp_int_t int_var__vfid_oor;
        void vfid_oor (const cpp_int  & l__val);
        cpp_int vfid_oor() const;
    
        typedef pu_cpp_int< 1 > cfg_bdf_oor_cpp_int_t;
        cfg_bdf_oor_cpp_int_t int_var__cfg_bdf_oor;
        void cfg_bdf_oor (const cpp_int  & l__val);
        cpp_int cfg_bdf_oor() const;
    
        typedef pu_cpp_int< 1 > pmr_ecc_err_cpp_int_t;
        pmr_ecc_err_cpp_int_t int_var__pmr_ecc_err;
        void pmr_ecc_err (const cpp_int  & l__val);
        cpp_int pmr_ecc_err() const;
    
        typedef pu_cpp_int< 1 > prt_ecc_err_cpp_int_t;
        prt_ecc_err_cpp_int_t int_var__prt_ecc_err;
        void prt_ecc_err (const cpp_int  & l__val);
        cpp_int prt_ecc_err() const;
    
}; // cap_pxb_csr_cfg_tgt_ind_en_t
    
class cap_pxb_csr_cfg_pc_port_type_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_cfg_pc_port_type_t(string name = "cap_pxb_csr_cfg_pc_port_type_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_cfg_pc_port_type_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > host_or_rc0_cpp_int_t;
        host_or_rc0_cpp_int_t int_var__host_or_rc0;
        void host_or_rc0 (const cpp_int  & l__val);
        cpp_int host_or_rc0() const;
    
        typedef pu_cpp_int< 1 > host_or_rc1_cpp_int_t;
        host_or_rc1_cpp_int_t int_var__host_or_rc1;
        void host_or_rc1 (const cpp_int  & l__val);
        cpp_int host_or_rc1() const;
    
        typedef pu_cpp_int< 1 > host_or_rc2_cpp_int_t;
        host_or_rc2_cpp_int_t int_var__host_or_rc2;
        void host_or_rc2 (const cpp_int  & l__val);
        cpp_int host_or_rc2() const;
    
        typedef pu_cpp_int< 1 > host_or_rc3_cpp_int_t;
        host_or_rc3_cpp_int_t int_var__host_or_rc3;
        void host_or_rc3 (const cpp_int  & l__val);
        cpp_int host_or_rc3() const;
    
        typedef pu_cpp_int< 1 > host_or_rc4_cpp_int_t;
        host_or_rc4_cpp_int_t int_var__host_or_rc4;
        void host_or_rc4 (const cpp_int  & l__val);
        cpp_int host_or_rc4() const;
    
        typedef pu_cpp_int< 1 > host_or_rc5_cpp_int_t;
        host_or_rc5_cpp_int_t int_var__host_or_rc5;
        void host_or_rc5 (const cpp_int  & l__val);
        cpp_int host_or_rc5() const;
    
        typedef pu_cpp_int< 1 > host_or_rc6_cpp_int_t;
        host_or_rc6_cpp_int_t int_var__host_or_rc6;
        void host_or_rc6 (const cpp_int  & l__val);
        cpp_int host_or_rc6() const;
    
        typedef pu_cpp_int< 1 > host_or_rc7_cpp_int_t;
        host_or_rc7_cpp_int_t int_var__host_or_rc7;
        void host_or_rc7 (const cpp_int  & l__val);
        cpp_int host_or_rc7() const;
    
}; // cap_pxb_csr_cfg_pc_port_type_t
    
class cap_pxb_csr_cfg_tgt_rx_credit_bfr_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_cfg_tgt_rx_credit_bfr_t(string name = "cap_pxb_csr_cfg_tgt_rx_credit_bfr_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_cfg_tgt_rx_credit_bfr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > adr_base0_cpp_int_t;
        adr_base0_cpp_int_t int_var__adr_base0;
        void adr_base0 (const cpp_int  & l__val);
        cpp_int adr_base0() const;
    
        typedef pu_cpp_int< 10 > adr_limit0_cpp_int_t;
        adr_limit0_cpp_int_t int_var__adr_limit0;
        void adr_limit0 (const cpp_int  & l__val);
        cpp_int adr_limit0() const;
    
        typedef pu_cpp_int< 1 > update0_cpp_int_t;
        update0_cpp_int_t int_var__update0;
        void update0 (const cpp_int  & l__val);
        cpp_int update0() const;
    
        typedef pu_cpp_int< 10 > adr_base1_cpp_int_t;
        adr_base1_cpp_int_t int_var__adr_base1;
        void adr_base1 (const cpp_int  & l__val);
        cpp_int adr_base1() const;
    
        typedef pu_cpp_int< 10 > adr_limit1_cpp_int_t;
        adr_limit1_cpp_int_t int_var__adr_limit1;
        void adr_limit1 (const cpp_int  & l__val);
        cpp_int adr_limit1() const;
    
        typedef pu_cpp_int< 1 > update1_cpp_int_t;
        update1_cpp_int_t int_var__update1;
        void update1 (const cpp_int  & l__val);
        cpp_int update1() const;
    
        typedef pu_cpp_int< 10 > adr_base2_cpp_int_t;
        adr_base2_cpp_int_t int_var__adr_base2;
        void adr_base2 (const cpp_int  & l__val);
        cpp_int adr_base2() const;
    
        typedef pu_cpp_int< 10 > adr_limit2_cpp_int_t;
        adr_limit2_cpp_int_t int_var__adr_limit2;
        void adr_limit2 (const cpp_int  & l__val);
        cpp_int adr_limit2() const;
    
        typedef pu_cpp_int< 1 > update2_cpp_int_t;
        update2_cpp_int_t int_var__update2;
        void update2 (const cpp_int  & l__val);
        cpp_int update2() const;
    
        typedef pu_cpp_int< 10 > adr_base3_cpp_int_t;
        adr_base3_cpp_int_t int_var__adr_base3;
        void adr_base3 (const cpp_int  & l__val);
        cpp_int adr_base3() const;
    
        typedef pu_cpp_int< 10 > adr_limit3_cpp_int_t;
        adr_limit3_cpp_int_t int_var__adr_limit3;
        void adr_limit3 (const cpp_int  & l__val);
        cpp_int adr_limit3() const;
    
        typedef pu_cpp_int< 1 > update3_cpp_int_t;
        update3_cpp_int_t int_var__update3;
        void update3 (const cpp_int  & l__val);
        cpp_int update3() const;
    
        typedef pu_cpp_int< 10 > adr_base4_cpp_int_t;
        adr_base4_cpp_int_t int_var__adr_base4;
        void adr_base4 (const cpp_int  & l__val);
        cpp_int adr_base4() const;
    
        typedef pu_cpp_int< 10 > adr_limit4_cpp_int_t;
        adr_limit4_cpp_int_t int_var__adr_limit4;
        void adr_limit4 (const cpp_int  & l__val);
        cpp_int adr_limit4() const;
    
        typedef pu_cpp_int< 1 > update4_cpp_int_t;
        update4_cpp_int_t int_var__update4;
        void update4 (const cpp_int  & l__val);
        cpp_int update4() const;
    
        typedef pu_cpp_int< 10 > adr_base5_cpp_int_t;
        adr_base5_cpp_int_t int_var__adr_base5;
        void adr_base5 (const cpp_int  & l__val);
        cpp_int adr_base5() const;
    
        typedef pu_cpp_int< 10 > adr_limit5_cpp_int_t;
        adr_limit5_cpp_int_t int_var__adr_limit5;
        void adr_limit5 (const cpp_int  & l__val);
        cpp_int adr_limit5() const;
    
        typedef pu_cpp_int< 1 > update5_cpp_int_t;
        update5_cpp_int_t int_var__update5;
        void update5 (const cpp_int  & l__val);
        cpp_int update5() const;
    
        typedef pu_cpp_int< 10 > adr_base6_cpp_int_t;
        adr_base6_cpp_int_t int_var__adr_base6;
        void adr_base6 (const cpp_int  & l__val);
        cpp_int adr_base6() const;
    
        typedef pu_cpp_int< 10 > adr_limit6_cpp_int_t;
        adr_limit6_cpp_int_t int_var__adr_limit6;
        void adr_limit6 (const cpp_int  & l__val);
        cpp_int adr_limit6() const;
    
        typedef pu_cpp_int< 1 > update6_cpp_int_t;
        update6_cpp_int_t int_var__update6;
        void update6 (const cpp_int  & l__val);
        cpp_int update6() const;
    
        typedef pu_cpp_int< 10 > adr_base7_cpp_int_t;
        adr_base7_cpp_int_t int_var__adr_base7;
        void adr_base7 (const cpp_int  & l__val);
        cpp_int adr_base7() const;
    
        typedef pu_cpp_int< 10 > adr_limit7_cpp_int_t;
        adr_limit7_cpp_int_t int_var__adr_limit7;
        void adr_limit7 (const cpp_int  & l__val);
        cpp_int adr_limit7() const;
    
        typedef pu_cpp_int< 1 > update7_cpp_int_t;
        update7_cpp_int_t int_var__update7;
        void update7 (const cpp_int  & l__val);
        cpp_int update7() const;
    
}; // cap_pxb_csr_cfg_tgt_rx_credit_bfr_t
    
class cap_pxb_csr_sta_tgt_axi_pending_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_tgt_axi_pending_t(string name = "cap_pxb_csr_sta_tgt_axi_pending_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_tgt_axi_pending_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > ids_p0_cpp_int_t;
        ids_p0_cpp_int_t int_var__ids_p0;
        void ids_p0 (const cpp_int  & l__val);
        cpp_int ids_p0() const;
    
        typedef pu_cpp_int< 8 > ids_p1_cpp_int_t;
        ids_p1_cpp_int_t int_var__ids_p1;
        void ids_p1 (const cpp_int  & l__val);
        cpp_int ids_p1() const;
    
        typedef pu_cpp_int< 8 > ids_p2_cpp_int_t;
        ids_p2_cpp_int_t int_var__ids_p2;
        void ids_p2 (const cpp_int  & l__val);
        cpp_int ids_p2() const;
    
        typedef pu_cpp_int< 8 > ids_p3_cpp_int_t;
        ids_p3_cpp_int_t int_var__ids_p3;
        void ids_p3 (const cpp_int  & l__val);
        cpp_int ids_p3() const;
    
        typedef pu_cpp_int< 8 > ids_p4_cpp_int_t;
        ids_p4_cpp_int_t int_var__ids_p4;
        void ids_p4 (const cpp_int  & l__val);
        cpp_int ids_p4() const;
    
        typedef pu_cpp_int< 8 > ids_p5_cpp_int_t;
        ids_p5_cpp_int_t int_var__ids_p5;
        void ids_p5 (const cpp_int  & l__val);
        cpp_int ids_p5() const;
    
        typedef pu_cpp_int< 8 > ids_p6_cpp_int_t;
        ids_p6_cpp_int_t int_var__ids_p6;
        void ids_p6 (const cpp_int  & l__val);
        cpp_int ids_p6() const;
    
        typedef pu_cpp_int< 8 > ids_p7_cpp_int_t;
        ids_p7_cpp_int_t int_var__ids_p7;
        void ids_p7 (const cpp_int  & l__val);
        cpp_int ids_p7() const;
    
}; // cap_pxb_csr_sta_tgt_axi_pending_t
    
class cap_pxb_csr_cfg_tgt_port_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_cfg_tgt_port_t(string name = "cap_pxb_csr_cfg_tgt_port_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_cfg_tgt_port_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > id_limit_cpp_int_t;
        id_limit_cpp_int_t int_var__id_limit;
        void id_limit (const cpp_int  & l__val);
        cpp_int id_limit() const;
    
        typedef pu_cpp_int< 1 > bus_adjust_cpp_int_t;
        bus_adjust_cpp_int_t int_var__bus_adjust;
        void bus_adjust (const cpp_int  & l__val);
        cpp_int bus_adjust() const;
    
        typedef pu_cpp_int< 1 > halt_cpp_int_t;
        halt_cpp_int_t int_var__halt;
        void halt (const cpp_int  & l__val);
        cpp_int halt() const;
    
        typedef pu_cpp_int< 1 > single_pnd_cpp_int_t;
        single_pnd_cpp_int_t int_var__single_pnd;
        void single_pnd (const cpp_int  & l__val);
        cpp_int single_pnd() const;
    
        typedef pu_cpp_int< 1 > td_miss_cpp_int_t;
        td_miss_cpp_int_t int_var__td_miss;
        void td_miss (const cpp_int  & l__val);
        cpp_int td_miss() const;
    
        typedef pu_cpp_int< 4 > depth_thres_cpp_int_t;
        depth_thres_cpp_int_t int_var__depth_thres;
        void depth_thres (const cpp_int  & l__val);
        cpp_int depth_thres() const;
    
}; // cap_pxb_csr_cfg_tgt_port_t
    
class cap_pxb_csr_cfg_itr_port_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_cfg_itr_port_t(string name = "cap_pxb_csr_cfg_itr_port_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_cfg_itr_port_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 8 > rdreq_limit_cpp_int_t;
        rdreq_limit_cpp_int_t int_var__rdreq_limit;
        void rdreq_limit (const cpp_int  & l__val);
        cpp_int rdreq_limit() const;
    
        typedef pu_cpp_int< 16 > timer_tick_cpp_int_t;
        timer_tick_cpp_int_t int_var__timer_tick;
        void timer_tick (const cpp_int  & l__val);
        cpp_int timer_tick() const;
    
        typedef pu_cpp_int< 10 > timer_limit_cpp_int_t;
        timer_limit_cpp_int_t int_var__timer_limit;
        void timer_limit (const cpp_int  & l__val);
        cpp_int timer_limit() const;
    
        typedef pu_cpp_int< 1 > bus_adjust_cpp_int_t;
        bus_adjust_cpp_int_t int_var__bus_adjust;
        void bus_adjust (const cpp_int  & l__val);
        cpp_int bus_adjust() const;
    
        typedef pu_cpp_int< 5 > arb_cr_hdr_cpp_int_t;
        arb_cr_hdr_cpp_int_t int_var__arb_cr_hdr;
        void arb_cr_hdr (const cpp_int  & l__val);
        cpp_int arb_cr_hdr() const;
    
        typedef pu_cpp_int< 6 > arb_cr_data_cpp_int_t;
        arb_cr_data_cpp_int_t int_var__arb_cr_data;
        void arb_cr_data (const cpp_int  & l__val);
        cpp_int arb_cr_data() const;
    
        typedef pu_cpp_int< 1 > arb_cr_dis_wr_cpp_int_t;
        arb_cr_dis_wr_cpp_int_t int_var__arb_cr_dis_wr;
        void arb_cr_dis_wr (const cpp_int  & l__val);
        cpp_int arb_cr_dis_wr() const;
    
        typedef pu_cpp_int< 1 > arb_cr_dis_rd_cpp_int_t;
        arb_cr_dis_rd_cpp_int_t int_var__arb_cr_dis_rd;
        void arb_cr_dis_rd (const cpp_int  & l__val);
        cpp_int arb_cr_dis_rd() const;
    
        typedef pu_cpp_int< 5 > arb_txfifocnt_cpp_int_t;
        arb_txfifocnt_cpp_int_t int_var__arb_txfifocnt;
        void arb_txfifocnt (const cpp_int  & l__val);
        cpp_int arb_txfifocnt() const;
    
        typedef pu_cpp_int< 1 > adr_extn_en_cpp_int_t;
        adr_extn_en_cpp_int_t int_var__adr_extn_en;
        void adr_extn_en (const cpp_int  & l__val);
        cpp_int adr_extn_en() const;
    
}; // cap_pxb_csr_cfg_itr_port_t
    
class cap_pxb_csr_cfg_tgt_req_indint_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_cfg_tgt_req_indint_t(string name = "cap_pxb_csr_cfg_tgt_req_indint_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_cfg_tgt_req_indint_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 34 > base_addr_35_2_cpp_int_t;
        base_addr_35_2_cpp_int_t int_var__base_addr_35_2;
        void base_addr_35_2 (const cpp_int  & l__val);
        cpp_int base_addr_35_2() const;
    
}; // cap_pxb_csr_cfg_tgt_req_indint_t
    
class cap_pxb_csr_cfg_tgt_req_notifyint_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_cfg_tgt_req_notifyint_t(string name = "cap_pxb_csr_cfg_tgt_req_notifyint_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_cfg_tgt_req_notifyint_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 34 > base_addr_35_2_cpp_int_t;
        base_addr_35_2_cpp_int_t int_var__base_addr_35_2;
        void base_addr_35_2 (const cpp_int  & l__val);
        cpp_int base_addr_35_2() const;
    
}; // cap_pxb_csr_cfg_tgt_req_notifyint_t
    
class cap_pxb_csr_cfg_tgt_req_notify_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_cfg_tgt_req_notify_t(string name = "cap_pxb_csr_cfg_tgt_req_notify_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_cfg_tgt_req_notify_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 20 > base_addr_35_16_cpp_int_t;
        base_addr_35_16_cpp_int_t int_var__base_addr_35_16;
        void base_addr_35_16 (const cpp_int  & l__val);
        cpp_int base_addr_35_16() const;
    
}; // cap_pxb_csr_cfg_tgt_req_notify_t
    
class cap_pxb_csr_sta_tgt_ind_info_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_sta_tgt_ind_info_t(string name = "cap_pxb_csr_sta_tgt_ind_info_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_sta_tgt_ind_info_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > pending_cpp_int_t;
        pending_cpp_int_t int_var__pending;
        void pending (const cpp_int  & l__val);
        cpp_int pending() const;
    
        typedef pu_cpp_int< 7 > ramaddr_cpp_int_t;
        ramaddr_cpp_int_t int_var__ramaddr;
        void ramaddr (const cpp_int  & l__val);
        cpp_int ramaddr() const;
    
}; // cap_pxb_csr_sta_tgt_ind_info_t
    
class cap_pxb_csr_base_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_base_t(string name = "cap_pxb_csr_base_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_base_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > scratch_reg_cpp_int_t;
        scratch_reg_cpp_int_t int_var__scratch_reg;
        void scratch_reg (const cpp_int  & l__val);
        cpp_int scratch_reg() const;
    
}; // cap_pxb_csr_base_t
    
class cap_pxb_csr_t : public cap_block_base {
    public:
        cpp_int_helper hlp;
    
        cap_pxb_csr_t(string name = "cap_pxb_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_pxb_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pxb_csr_base_t base;
    
        cap_pxb_csr_sta_tgt_ind_info_t sta_tgt_ind_info[8];
        int get_depth_sta_tgt_ind_info() { return 8; }
    
        cap_pxb_csr_cfg_tgt_req_notify_t cfg_tgt_req_notify[8];
        int get_depth_cfg_tgt_req_notify() { return 8; }
    
        cap_pxb_csr_cfg_tgt_req_notifyint_t cfg_tgt_req_notifyint[8];
        int get_depth_cfg_tgt_req_notifyint() { return 8; }
    
        cap_pxb_csr_cfg_tgt_req_indint_t cfg_tgt_req_indint[8];
        int get_depth_cfg_tgt_req_indint() { return 8; }
    
        cap_pxb_csr_cfg_itr_port_t cfg_itr_port[8];
        int get_depth_cfg_itr_port() { return 8; }
    
        cap_pxb_csr_cfg_tgt_port_t cfg_tgt_port[8];
        int get_depth_cfg_tgt_port() { return 8; }
    
        cap_pxb_csr_sta_tgt_axi_pending_t sta_tgt_axi_pending;
    
        cap_pxb_csr_cfg_tgt_rx_credit_bfr_t cfg_tgt_rx_credit_bfr;
    
        cap_pxb_csr_cfg_pc_port_type_t cfg_pc_port_type;
    
        cap_pxb_csr_cfg_tgt_ind_en_t cfg_tgt_ind_en;
    
        cap_pxb_csr_cfg_tgt_notify_en_t cfg_tgt_notify_en;
    
        cap_pxb_csr_cfg_pmt_tbl_id_t cfg_pmt_tbl_id;
    
        cap_pxb_csr_cfg_tgt_rc_key_t cfg_tgt_rc_key;
    
        cap_pxb_csr_cfg_pcie_local_memaddr_decode_t cfg_pcie_local_memaddr_decode;
    
        cap_pxb_csr_cfg_tgt_pmt_grst_t cfg_tgt_pmt_grst;
    
        cap_pxb_csr_cfg_tgt_pmt_ind_t cfg_tgt_pmt_ind;
    
        cap_pxb_csr_sta_tgt_pmt_ind_t sta_tgt_pmt_ind;
    
        cap_pxb_csr_cfg_itr_ecc_disable_t cfg_itr_ecc_disable;
    
        cap_pxb_csr_cfg_tgt_ecc_disable_t cfg_tgt_ecc_disable;
    
        cap_pxb_csr_sta_itr_ecc_wrbfr_0_t sta_itr_ecc_wrbfr_0;
    
        cap_pxb_csr_sta_itr_ecc_wrbfr_1_t sta_itr_ecc_wrbfr_1;
    
        cap_pxb_csr_sta_itr_ecc_wrbfr_2_t sta_itr_ecc_wrbfr_2;
    
        cap_pxb_csr_sta_itr_ecc_wrbfr_3_t sta_itr_ecc_wrbfr_3;
    
        cap_pxb_csr_sta_itr_ecc_pcihdrt_t sta_itr_ecc_pcihdrt;
    
        cap_pxb_csr_sta_itr_ecc_portmap_t sta_itr_ecc_portmap;
    
        cap_pxb_csr_sta_itr_ecc_wrhdr_t sta_itr_ecc_wrhdr;
    
        cap_pxb_csr_sta_itr_ecc_rdhdr_t sta_itr_ecc_rdhdr;
    
        cap_pxb_csr_sta_itr_ecc_cplbfr_t sta_itr_ecc_cplbfr;
    
        cap_pxb_csr_sta_tgt_ecc_rxcrbfr_0_t sta_tgt_ecc_rxcrbfr_0;
    
        cap_pxb_csr_sta_tgt_ecc_rxcrbfr_1_t sta_tgt_ecc_rxcrbfr_1;
    
        cap_pxb_csr_sta_tgt_ecc_rxcrbfr_2_t sta_tgt_ecc_rxcrbfr_2;
    
        cap_pxb_csr_sta_tgt_ecc_rxcrbfr_3_t sta_tgt_ecc_rxcrbfr_3;
    
        cap_pxb_csr_sta_tgt_ecc_aximst_0_t sta_tgt_ecc_aximst_0;
    
        cap_pxb_csr_sta_tgt_ecc_aximst_1_t sta_tgt_ecc_aximst_1;
    
        cap_pxb_csr_sta_tgt_ecc_aximst_2_t sta_tgt_ecc_aximst_2;
    
        cap_pxb_csr_sta_tgt_ecc_aximst_3_t sta_tgt_ecc_aximst_3;
    
        cap_pxb_csr_sta_tgt_ecc_aximst_4_t sta_tgt_ecc_aximst_4;
    
        cap_pxb_csr_sta_tgt_ecc_rxinfo_t sta_tgt_ecc_rxinfo;
    
        cap_pxb_csr_sta_tgt_ecc_cplst_t sta_tgt_ecc_cplst;
    
        cap_pxb_csr_sta_tgt_ecc_pmr_t sta_tgt_ecc_pmr;
    
        cap_pxb_csr_sta_tgt_ecc_prt_t sta_tgt_ecc_prt;
    
        cap_pxb_csr_sta_tgt_ecc_romask_t sta_tgt_ecc_romask;
    
        cap_pxb_csr_sat_tgt_rx_drop_t sat_tgt_rx_drop;
    
        cap_pxb_csr_cfg_tgt_vf_cfg_stride_t cfg_tgt_vf_cfg_stride[16];
        int get_depth_cfg_tgt_vf_cfg_stride() { return 16; }
    
        cap_pxb_csr_cnt_itr_tot_axi_wr_t cnt_itr_tot_axi_wr;
    
        cap_pxb_csr_cnt_itr_tot_axi_rd_t cnt_itr_tot_axi_rd;
    
        cap_pxb_csr_sta_itr_axi_wr_num_ids_t sta_itr_axi_wr_num_ids;
    
        cap_pxb_csr_sta_itr_axi_rd_num_ids_t sta_itr_axi_rd_num_ids;
    
        cap_pxb_csr_cnt_itr_tot_msg_t cnt_itr_tot_msg;
    
        cap_pxb_csr_cnt_itr_intx_assert_msg_t cnt_itr_intx_assert_msg;
    
        cap_pxb_csr_cnt_itr_tot_atomic_req_t cnt_itr_tot_atomic_req;
    
        cap_pxb_csr_cfg_tgt_doorbell_base_t cfg_tgt_doorbell_base;
    
        cap_pxb_csr_sta_tgt_marker_rx_t sta_tgt_marker_rx;
    
        cap_pxb_csr_cfg_itr_raw_tlp_t cfg_itr_raw_tlp;
    
        cap_pxb_csr_sta_itr_raw_tlp_t sta_itr_raw_tlp;
    
        cap_pxb_csr_sta_itr_tags_pending_t sta_itr_tags_pending;
    
        cap_pxb_csr_sta_itr_portfifo_depth_t sta_itr_portfifo_depth;
    
        cap_pxb_csr_sta_itr_atomic_seq_cnt_t sta_itr_atomic_seq_cnt;
    
        cap_pxb_csr_cfg_itr_atomic_seq_cnt_t cfg_itr_atomic_seq_cnt;
    
        cap_pxb_csr_sat_itr_req_err_t sat_itr_req_err;
    
        cap_pxb_csr_cfg_tgt_axi_attr_t cfg_tgt_axi_attr;
    
        cap_pxb_csr_cfg_debug_port_t cfg_debug_port;
    
        cap_pxb_csr_dhs_itr_pcihdrt_t dhs_itr_pcihdrt;
    
        cap_pxb_csr_dhs_itr_portmap_t dhs_itr_portmap;
    
        cap_pxb_csr_dhs_tgt_pmt_t dhs_tgt_pmt;
    
        cap_pxb_csr_dhs_tgt_pmr_t dhs_tgt_pmr;
    
        cap_pxb_csr_dhs_tgt_prt_t dhs_tgt_prt;
    
        cap_pxb_csr_dhs_tgt_rxcrbfr0_t dhs_tgt_rxcrbfr0;
    
        cap_pxb_csr_dhs_tgt_rxcrbfr1_t dhs_tgt_rxcrbfr1;
    
        cap_pxb_csr_dhs_tgt_rxcrbfr2_t dhs_tgt_rxcrbfr2;
    
        cap_pxb_csr_dhs_tgt_rxcrbfr3_t dhs_tgt_rxcrbfr3;
    
        cap_pxb_csr_dhs_tgt_rxinfo_t dhs_tgt_rxinfo;
    
        cap_pxb_csr_dhs_tgt_cplst_t dhs_tgt_cplst;
    
        cap_pxb_csr_dhs_tgt_romask_t dhs_tgt_romask;
    
        cap_pxb_csr_dhs_tgt_aximst0_t dhs_tgt_aximst0;
    
        cap_pxb_csr_dhs_tgt_aximst1_t dhs_tgt_aximst1;
    
        cap_pxb_csr_dhs_tgt_aximst2_t dhs_tgt_aximst2;
    
        cap_pxb_csr_dhs_tgt_aximst3_t dhs_tgt_aximst3;
    
        cap_pxb_csr_dhs_tgt_aximst4_t dhs_tgt_aximst4;
    
        cap_pxb_csr_dhs_itr_wrbfr0_t dhs_itr_wrbfr0;
    
        cap_pxb_csr_dhs_itr_wrbfr1_t dhs_itr_wrbfr1;
    
        cap_pxb_csr_dhs_itr_wrbfr2_t dhs_itr_wrbfr2;
    
        cap_pxb_csr_dhs_itr_wrbfr3_t dhs_itr_wrbfr3;
    
        cap_pxb_csr_dhs_itr_wrhdr_t dhs_itr_wrhdr;
    
        cap_pxb_csr_dhs_itr_rdhdr_t dhs_itr_rdhdr;
    
        cap_pxb_csr_dhs_itr_rdcontext_t dhs_itr_rdcontext;
    
        cap_pxb_csr_dhs_tgt_rdcontext_t dhs_tgt_rdcontext;
    
        cap_pxb_csr_dhs_tgt_rc_bdfmap_t dhs_tgt_rc_bdfmap;
    
        cap_pxb_csr_dhs_itr_cplbfr0_t dhs_itr_cplbfr0;
    
        cap_pxb_csr_dhs_itr_cplbfr1_t dhs_itr_cplbfr1;
    
        cap_pxb_csr_dhs_itr_cplbfr2_t dhs_itr_cplbfr2;
    
        cap_pxb_csr_dhs_itr_cplbfr3_t dhs_itr_cplbfr3;
    
        cap_pxb_csr_dhs_itr_cplbfr4_t dhs_itr_cplbfr4;
    
        cap_pxb_csr_dhs_itr_cplbfr5_t dhs_itr_cplbfr5;
    
        cap_pxb_csr_dhs_itr_cplbfr6_t dhs_itr_cplbfr6;
    
        cap_pxb_csr_dhs_itr_cplbfr7_t dhs_itr_cplbfr7;
    
        cap_pxb_csr_dhs_itr_cplbfr8_t dhs_itr_cplbfr8;
    
        cap_pxb_csr_dhs_itr_cplbfr9_t dhs_itr_cplbfr9;
    
        cap_pxb_csr_dhs_itr_cplbfr10_t dhs_itr_cplbfr10;
    
        cap_pxb_csr_dhs_itr_cplbfr11_t dhs_itr_cplbfr11;
    
        cap_pxb_csr_dhs_itr_cplbfr12_t dhs_itr_cplbfr12;
    
        cap_pxb_csr_dhs_itr_cplbfr13_t dhs_itr_cplbfr13;
    
        cap_pxb_csr_dhs_itr_cplbfr14_t dhs_itr_cplbfr14;
    
        cap_pxb_csr_dhs_itr_cplbfr15_t dhs_itr_cplbfr15;
    
        cap_pxb_csr_dhs_tgt_notify_t dhs_tgt_notify;
    
        cap_pxb_csr_dhs_tgt_ind_rsp_t dhs_tgt_ind_rsp;
    
        cap_pxb_csr_dhs_tgt_pmt_ind_t dhs_tgt_pmt_ind;
    
}; // cap_pxb_csr_t
    
#endif // CAP_PXB_CSR_H
        