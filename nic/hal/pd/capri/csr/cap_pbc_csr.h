
#ifndef CAP_PBC_CSR_H
#define CAP_PBC_CSR_H

#include "cap_csr_base.h" 

using namespace std;
class cap_pbc_csr_dhs_sched_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_sched_entry_t(string name = "cap_pbc_csr_dhs_sched_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_sched_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > current_credit_cpp_int_t;
        current_credit_cpp_int_t int_var__current_credit;
        void current_credit (const cpp_int  & l__val);
        cpp_int current_credit() const;
    
        typedef pu_cpp_int< 32 > quanta_cpp_int_t;
        quanta_cpp_int_t int_var__quanta;
        void quanta (const cpp_int  & l__val);
        cpp_int quanta() const;
    
}; // cap_pbc_csr_dhs_sched_entry_t
    
class cap_pbc_csr_dhs_sched_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_sched_t(string name = "cap_pbc_csr_dhs_sched_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_sched_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pbc_csr_dhs_sched_entry_t entry;
    
}; // cap_pbc_csr_dhs_sched_t
    
class cap_pbc_csr_dhs_rc_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_rc_entry_t(string name = "cap_pbc_csr_dhs_rc_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_rc_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 3 > count0_cpp_int_t;
        count0_cpp_int_t int_var__count0;
        void count0 (const cpp_int  & l__val);
        cpp_int count0() const;
    
        typedef pu_cpp_int< 3 > count1_cpp_int_t;
        count1_cpp_int_t int_var__count1;
        void count1 (const cpp_int  & l__val);
        cpp_int count1() const;
    
        typedef pu_cpp_int< 3 > count2_cpp_int_t;
        count2_cpp_int_t int_var__count2;
        void count2 (const cpp_int  & l__val);
        cpp_int count2() const;
    
        typedef pu_cpp_int< 3 > count3_cpp_int_t;
        count3_cpp_int_t int_var__count3;
        void count3 (const cpp_int  & l__val);
        cpp_int count3() const;
    
        typedef pu_cpp_int< 3 > count4_cpp_int_t;
        count4_cpp_int_t int_var__count4;
        void count4 (const cpp_int  & l__val);
        cpp_int count4() const;
    
        typedef pu_cpp_int< 3 > count5_cpp_int_t;
        count5_cpp_int_t int_var__count5;
        void count5 (const cpp_int  & l__val);
        cpp_int count5() const;
    
        typedef pu_cpp_int< 3 > count6_cpp_int_t;
        count6_cpp_int_t int_var__count6;
        void count6 (const cpp_int  & l__val);
        cpp_int count6() const;
    
        typedef pu_cpp_int< 3 > count7_cpp_int_t;
        count7_cpp_int_t int_var__count7;
        void count7 (const cpp_int  & l__val);
        cpp_int count7() const;
    
}; // cap_pbc_csr_dhs_rc_entry_t
    
class cap_pbc_csr_dhs_rc_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_rc_t(string name = "cap_pbc_csr_dhs_rc_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_rc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pbc_csr_dhs_rc_entry_t entry;
    
}; // cap_pbc_csr_dhs_rc_t
    
class cap_pbc_csr_dhs_gc_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_gc_entry_t(string name = "cap_pbc_csr_dhs_gc_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_gc_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 14 > next_cell_cpp_int_t;
        next_cell_cpp_int_t int_var__next_cell;
        void next_cell (const cpp_int  & l__val);
        cpp_int next_cell() const;
    
}; // cap_pbc_csr_dhs_gc_entry_t
    
class cap_pbc_csr_dhs_gc_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_gc_t(string name = "cap_pbc_csr_dhs_gc_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_gc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pbc_csr_dhs_gc_entry_t entry;
    
}; // cap_pbc_csr_dhs_gc_t
    
class cap_pbc_csr_dhs_desc_1_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_desc_1_entry_t(string name = "cap_pbc_csr_dhs_desc_1_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_desc_1_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 4 > src_port_cpp_int_t;
        src_port_cpp_int_t int_var__src_port;
        void src_port (const cpp_int  & l__val);
        cpp_int src_port() const;
    
        typedef pu_cpp_int< 3 > refcount_cpp_int_t;
        refcount_cpp_int_t int_var__refcount;
        void refcount (const cpp_int  & l__val);
        cpp_int refcount() const;
    
        typedef pu_cpp_int< 3 > input_queue_cpp_int_t;
        input_queue_cpp_int_t int_var__input_queue;
        void input_queue (const cpp_int  & l__val);
        cpp_int input_queue() const;
    
        typedef pu_cpp_int< 5 > eop_pack_cpp_int_t;
        eop_pack_cpp_int_t int_var__eop_pack;
        void eop_pack (const cpp_int  & l__val);
        cpp_int eop_pack() const;
    
        typedef pu_cpp_int< 5 > err_pack_cpp_int_t;
        err_pack_cpp_int_t int_var__err_pack;
        void err_pack (const cpp_int  & l__val);
        cpp_int err_pack() const;
    
}; // cap_pbc_csr_dhs_desc_1_entry_t
    
class cap_pbc_csr_dhs_desc_1_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_desc_1_t(string name = "cap_pbc_csr_dhs_desc_1_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_desc_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pbc_csr_dhs_desc_1_entry_t entry;
    
}; // cap_pbc_csr_dhs_desc_1_t
    
class cap_pbc_csr_dhs_ll_1_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_ll_1_entry_t(string name = "cap_pbc_csr_dhs_ll_1_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_ll_1_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 14 > next_cell_cpp_int_t;
        next_cell_cpp_int_t int_var__next_cell;
        void next_cell (const cpp_int  & l__val);
        cpp_int next_cell() const;
    
        typedef pu_cpp_int< 1 > eop_cpp_int_t;
        eop_cpp_int_t int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
        typedef pu_cpp_int< 1 > full_cpp_int_t;
        full_cpp_int_t int_var__full;
        void full (const cpp_int  & l__val);
        cpp_int full() const;
    
        typedef pu_cpp_int< 1 > ge256B_cpp_int_t;
        ge256B_cpp_int_t int_var__ge256B;
        void ge256B (const cpp_int  & l__val);
        cpp_int ge256B() const;
    
        typedef pu_cpp_int< 1 > err_cpp_int_t;
        err_cpp_int_t int_var__err;
        void err (const cpp_int  & l__val);
        cpp_int err() const;
    
}; // cap_pbc_csr_dhs_ll_1_entry_t
    
class cap_pbc_csr_dhs_ll_1_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_ll_1_t(string name = "cap_pbc_csr_dhs_ll_1_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_ll_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pbc_csr_dhs_ll_1_entry_t entry;
    
}; // cap_pbc_csr_dhs_ll_1_t
    
class cap_pbc_csr_dhs_fc_1_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_fc_1_entry_t(string name = "cap_pbc_csr_dhs_fc_1_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_fc_1_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 14 > cell0_cpp_int_t;
        cell0_cpp_int_t int_var__cell0;
        void cell0 (const cpp_int  & l__val);
        cpp_int cell0() const;
    
        typedef pu_cpp_int< 14 > cell1_cpp_int_t;
        cell1_cpp_int_t int_var__cell1;
        void cell1 (const cpp_int  & l__val);
        cpp_int cell1() const;
    
}; // cap_pbc_csr_dhs_fc_1_entry_t
    
class cap_pbc_csr_dhs_fc_1_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_fc_1_t(string name = "cap_pbc_csr_dhs_fc_1_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_fc_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pbc_csr_dhs_fc_1_entry_t entry;
    
}; // cap_pbc_csr_dhs_fc_1_t
    
class cap_pbc_csr_dhs_desc_0_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_desc_0_entry_t(string name = "cap_pbc_csr_dhs_desc_0_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_desc_0_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 4 > src_port_cpp_int_t;
        src_port_cpp_int_t int_var__src_port;
        void src_port (const cpp_int  & l__val);
        cpp_int src_port() const;
    
        typedef pu_cpp_int< 3 > refcount_cpp_int_t;
        refcount_cpp_int_t int_var__refcount;
        void refcount (const cpp_int  & l__val);
        cpp_int refcount() const;
    
        typedef pu_cpp_int< 3 > input_queue_cpp_int_t;
        input_queue_cpp_int_t int_var__input_queue;
        void input_queue (const cpp_int  & l__val);
        cpp_int input_queue() const;
    
        typedef pu_cpp_int< 5 > eop_pack_cpp_int_t;
        eop_pack_cpp_int_t int_var__eop_pack;
        void eop_pack (const cpp_int  & l__val);
        cpp_int eop_pack() const;
    
        typedef pu_cpp_int< 5 > err_pack_cpp_int_t;
        err_pack_cpp_int_t int_var__err_pack;
        void err_pack (const cpp_int  & l__val);
        cpp_int err_pack() const;
    
}; // cap_pbc_csr_dhs_desc_0_entry_t
    
class cap_pbc_csr_dhs_desc_0_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_desc_0_t(string name = "cap_pbc_csr_dhs_desc_0_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_desc_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pbc_csr_dhs_desc_0_entry_t entry;
    
}; // cap_pbc_csr_dhs_desc_0_t
    
class cap_pbc_csr_dhs_ll_0_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_ll_0_entry_t(string name = "cap_pbc_csr_dhs_ll_0_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_ll_0_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 14 > next_cell_cpp_int_t;
        next_cell_cpp_int_t int_var__next_cell;
        void next_cell (const cpp_int  & l__val);
        cpp_int next_cell() const;
    
        typedef pu_cpp_int< 1 > eop_cpp_int_t;
        eop_cpp_int_t int_var__eop;
        void eop (const cpp_int  & l__val);
        cpp_int eop() const;
    
        typedef pu_cpp_int< 1 > full_cpp_int_t;
        full_cpp_int_t int_var__full;
        void full (const cpp_int  & l__val);
        cpp_int full() const;
    
        typedef pu_cpp_int< 1 > ge256B_cpp_int_t;
        ge256B_cpp_int_t int_var__ge256B;
        void ge256B (const cpp_int  & l__val);
        cpp_int ge256B() const;
    
        typedef pu_cpp_int< 1 > err_cpp_int_t;
        err_cpp_int_t int_var__err;
        void err (const cpp_int  & l__val);
        cpp_int err() const;
    
}; // cap_pbc_csr_dhs_ll_0_entry_t
    
class cap_pbc_csr_dhs_ll_0_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_ll_0_t(string name = "cap_pbc_csr_dhs_ll_0_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_ll_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pbc_csr_dhs_ll_0_entry_t entry;
    
}; // cap_pbc_csr_dhs_ll_0_t
    
class cap_pbc_csr_dhs_fc_0_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_fc_0_entry_t(string name = "cap_pbc_csr_dhs_fc_0_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_fc_0_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 14 > cell0_cpp_int_t;
        cell0_cpp_int_t int_var__cell0;
        void cell0 (const cpp_int  & l__val);
        cpp_int cell0() const;
    
        typedef pu_cpp_int< 14 > cell1_cpp_int_t;
        cell1_cpp_int_t int_var__cell1;
        void cell1 (const cpp_int  & l__val);
        cpp_int cell1() const;
    
}; // cap_pbc_csr_dhs_fc_0_entry_t
    
class cap_pbc_csr_dhs_fc_0_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_fc_0_t(string name = "cap_pbc_csr_dhs_fc_0_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_fc_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pbc_csr_dhs_fc_0_entry_t entry;
    
}; // cap_pbc_csr_dhs_fc_0_t
    
class cap_pbc_csr_dhs_oq_11_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_oq_11_entry_t(string name = "cap_pbc_csr_dhs_oq_11_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_oq_11_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 13 > next_cell_cpp_int_t;
        next_cell_cpp_int_t int_var__next_cell;
        void next_cell (const cpp_int  & l__val);
        cpp_int next_cell() const;
    
        typedef pu_cpp_int< 5 > pack_info_cpp_int_t;
        pack_info_cpp_int_t int_var__pack_info;
        void pack_info (const cpp_int  & l__val);
        cpp_int pack_info() const;
    
}; // cap_pbc_csr_dhs_oq_11_entry_t
    
class cap_pbc_csr_dhs_oq_11_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_oq_11_t(string name = "cap_pbc_csr_dhs_oq_11_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_oq_11_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pbc_csr_dhs_oq_11_entry_t entry;
    
}; // cap_pbc_csr_dhs_oq_11_t
    
class cap_pbc_csr_dhs_oq_10_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_oq_10_entry_t(string name = "cap_pbc_csr_dhs_oq_10_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_oq_10_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 14 > next_cell_cpp_int_t;
        next_cell_cpp_int_t int_var__next_cell;
        void next_cell (const cpp_int  & l__val);
        cpp_int next_cell() const;
    
        typedef pu_cpp_int< 16 > rpl_ptr_cpp_int_t;
        rpl_ptr_cpp_int_t int_var__rpl_ptr;
        void rpl_ptr (const cpp_int  & l__val);
        cpp_int rpl_ptr() const;
    
        typedef pu_cpp_int< 1 > rpl_en_cpp_int_t;
        rpl_en_cpp_int_t int_var__rpl_en;
        void rpl_en (const cpp_int  & l__val);
        cpp_int rpl_en() const;
    
}; // cap_pbc_csr_dhs_oq_10_entry_t
    
class cap_pbc_csr_dhs_oq_10_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_oq_10_t(string name = "cap_pbc_csr_dhs_oq_10_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_oq_10_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pbc_csr_dhs_oq_10_entry_t entry;
    
}; // cap_pbc_csr_dhs_oq_10_t
    
class cap_pbc_csr_dhs_oq_9_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_oq_9_entry_t(string name = "cap_pbc_csr_dhs_oq_9_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_oq_9_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 14 > next_cell_cpp_int_t;
        next_cell_cpp_int_t int_var__next_cell;
        void next_cell (const cpp_int  & l__val);
        cpp_int next_cell() const;
    
}; // cap_pbc_csr_dhs_oq_9_entry_t
    
class cap_pbc_csr_dhs_oq_9_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_oq_9_t(string name = "cap_pbc_csr_dhs_oq_9_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_oq_9_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pbc_csr_dhs_oq_9_entry_t entry;
    
}; // cap_pbc_csr_dhs_oq_9_t
    
class cap_pbc_csr_dhs_oq_8_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_oq_8_entry_t(string name = "cap_pbc_csr_dhs_oq_8_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_oq_8_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 14 > next_cell_cpp_int_t;
        next_cell_cpp_int_t int_var__next_cell;
        void next_cell (const cpp_int  & l__val);
        cpp_int next_cell() const;
    
}; // cap_pbc_csr_dhs_oq_8_entry_t
    
class cap_pbc_csr_dhs_oq_8_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_oq_8_t(string name = "cap_pbc_csr_dhs_oq_8_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_oq_8_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pbc_csr_dhs_oq_8_entry_t entry;
    
}; // cap_pbc_csr_dhs_oq_8_t
    
class cap_pbc_csr_dhs_oq_7_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_oq_7_entry_t(string name = "cap_pbc_csr_dhs_oq_7_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_oq_7_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 14 > next_cell_cpp_int_t;
        next_cell_cpp_int_t int_var__next_cell;
        void next_cell (const cpp_int  & l__val);
        cpp_int next_cell() const;
    
}; // cap_pbc_csr_dhs_oq_7_entry_t
    
class cap_pbc_csr_dhs_oq_7_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_oq_7_t(string name = "cap_pbc_csr_dhs_oq_7_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_oq_7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pbc_csr_dhs_oq_7_entry_t entry;
    
}; // cap_pbc_csr_dhs_oq_7_t
    
class cap_pbc_csr_dhs_oq_6_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_oq_6_entry_t(string name = "cap_pbc_csr_dhs_oq_6_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_oq_6_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 14 > next_cell_cpp_int_t;
        next_cell_cpp_int_t int_var__next_cell;
        void next_cell (const cpp_int  & l__val);
        cpp_int next_cell() const;
    
}; // cap_pbc_csr_dhs_oq_6_entry_t
    
class cap_pbc_csr_dhs_oq_6_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_oq_6_t(string name = "cap_pbc_csr_dhs_oq_6_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_oq_6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pbc_csr_dhs_oq_6_entry_t entry;
    
}; // cap_pbc_csr_dhs_oq_6_t
    
class cap_pbc_csr_dhs_oq_5_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_oq_5_entry_t(string name = "cap_pbc_csr_dhs_oq_5_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_oq_5_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 14 > next_cell_cpp_int_t;
        next_cell_cpp_int_t int_var__next_cell;
        void next_cell (const cpp_int  & l__val);
        cpp_int next_cell() const;
    
}; // cap_pbc_csr_dhs_oq_5_entry_t
    
class cap_pbc_csr_dhs_oq_5_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_oq_5_t(string name = "cap_pbc_csr_dhs_oq_5_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_oq_5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pbc_csr_dhs_oq_5_entry_t entry;
    
}; // cap_pbc_csr_dhs_oq_5_t
    
class cap_pbc_csr_dhs_oq_4_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_oq_4_entry_t(string name = "cap_pbc_csr_dhs_oq_4_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_oq_4_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 14 > next_cell_cpp_int_t;
        next_cell_cpp_int_t int_var__next_cell;
        void next_cell (const cpp_int  & l__val);
        cpp_int next_cell() const;
    
}; // cap_pbc_csr_dhs_oq_4_entry_t
    
class cap_pbc_csr_dhs_oq_4_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_oq_4_t(string name = "cap_pbc_csr_dhs_oq_4_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_oq_4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pbc_csr_dhs_oq_4_entry_t entry;
    
}; // cap_pbc_csr_dhs_oq_4_t
    
class cap_pbc_csr_dhs_oq_3_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_oq_3_entry_t(string name = "cap_pbc_csr_dhs_oq_3_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_oq_3_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 14 > next_cell_cpp_int_t;
        next_cell_cpp_int_t int_var__next_cell;
        void next_cell (const cpp_int  & l__val);
        cpp_int next_cell() const;
    
}; // cap_pbc_csr_dhs_oq_3_entry_t
    
class cap_pbc_csr_dhs_oq_3_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_oq_3_t(string name = "cap_pbc_csr_dhs_oq_3_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_oq_3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pbc_csr_dhs_oq_3_entry_t entry;
    
}; // cap_pbc_csr_dhs_oq_3_t
    
class cap_pbc_csr_dhs_oq_2_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_oq_2_entry_t(string name = "cap_pbc_csr_dhs_oq_2_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_oq_2_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 14 > next_cell_cpp_int_t;
        next_cell_cpp_int_t int_var__next_cell;
        void next_cell (const cpp_int  & l__val);
        cpp_int next_cell() const;
    
}; // cap_pbc_csr_dhs_oq_2_entry_t
    
class cap_pbc_csr_dhs_oq_2_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_oq_2_t(string name = "cap_pbc_csr_dhs_oq_2_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_oq_2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pbc_csr_dhs_oq_2_entry_t entry;
    
}; // cap_pbc_csr_dhs_oq_2_t
    
class cap_pbc_csr_dhs_oq_1_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_oq_1_entry_t(string name = "cap_pbc_csr_dhs_oq_1_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_oq_1_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 14 > next_cell_cpp_int_t;
        next_cell_cpp_int_t int_var__next_cell;
        void next_cell (const cpp_int  & l__val);
        cpp_int next_cell() const;
    
}; // cap_pbc_csr_dhs_oq_1_entry_t
    
class cap_pbc_csr_dhs_oq_1_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_oq_1_t(string name = "cap_pbc_csr_dhs_oq_1_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_oq_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pbc_csr_dhs_oq_1_entry_t entry;
    
}; // cap_pbc_csr_dhs_oq_1_t
    
class cap_pbc_csr_dhs_oq_0_entry_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_oq_0_entry_t(string name = "cap_pbc_csr_dhs_oq_0_entry_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_oq_0_entry_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 14 > next_cell_cpp_int_t;
        next_cell_cpp_int_t int_var__next_cell;
        void next_cell (const cpp_int  & l__val);
        cpp_int next_cell() const;
    
}; // cap_pbc_csr_dhs_oq_0_entry_t
    
class cap_pbc_csr_dhs_oq_0_t : public cap_memory_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_dhs_oq_0_t(string name = "cap_pbc_csr_dhs_oq_0_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_dhs_oq_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pbc_csr_dhs_oq_0_entry_t entry;
    
}; // cap_pbc_csr_dhs_oq_0_t
    
class cap_pbc_csr_cfg_src_port_to_lif_map_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_src_port_to_lif_map_t(string name = "cap_pbc_csr_cfg_src_port_to_lif_map_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_src_port_to_lif_map_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 11 > entry_0_cpp_int_t;
        entry_0_cpp_int_t int_var__entry_0;
        void entry_0 (const cpp_int  & l__val);
        cpp_int entry_0() const;
    
        typedef pu_cpp_int< 11 > entry_1_cpp_int_t;
        entry_1_cpp_int_t int_var__entry_1;
        void entry_1 (const cpp_int  & l__val);
        cpp_int entry_1() const;
    
        typedef pu_cpp_int< 11 > entry_2_cpp_int_t;
        entry_2_cpp_int_t int_var__entry_2;
        void entry_2 (const cpp_int  & l__val);
        cpp_int entry_2() const;
    
        typedef pu_cpp_int< 11 > entry_3_cpp_int_t;
        entry_3_cpp_int_t int_var__entry_3;
        void entry_3 (const cpp_int  & l__val);
        cpp_int entry_3() const;
    
        typedef pu_cpp_int< 11 > entry_4_cpp_int_t;
        entry_4_cpp_int_t int_var__entry_4;
        void entry_4 (const cpp_int  & l__val);
        cpp_int entry_4() const;
    
        typedef pu_cpp_int< 11 > entry_5_cpp_int_t;
        entry_5_cpp_int_t int_var__entry_5;
        void entry_5 (const cpp_int  & l__val);
        cpp_int entry_5() const;
    
        typedef pu_cpp_int< 11 > entry_6_cpp_int_t;
        entry_6_cpp_int_t int_var__entry_6;
        void entry_6 (const cpp_int  & l__val);
        cpp_int entry_6() const;
    
        typedef pu_cpp_int< 11 > entry_7_cpp_int_t;
        entry_7_cpp_int_t int_var__entry_7;
        void entry_7 (const cpp_int  & l__val);
        cpp_int entry_7() const;
    
        typedef pu_cpp_int< 11 > entry_8_cpp_int_t;
        entry_8_cpp_int_t int_var__entry_8;
        void entry_8 (const cpp_int  & l__val);
        cpp_int entry_8() const;
    
}; // cap_pbc_csr_cfg_src_port_to_lif_map_t
    
class cap_pbc_csr_cfg_tail_drop_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_tail_drop_t(string name = "cap_pbc_csr_cfg_tail_drop_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_tail_drop_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > cpu_threshold_cpp_int_t;
        cpu_threshold_cpp_int_t int_var__cpu_threshold;
        void cpu_threshold (const cpp_int  & l__val);
        cpp_int cpu_threshold() const;
    
        typedef pu_cpp_int< 16 > span_threshold_cpp_int_t;
        span_threshold_cpp_int_t int_var__span_threshold;
        void span_threshold (const cpp_int  & l__val);
        cpp_int span_threshold() const;
    
}; // cap_pbc_csr_cfg_tail_drop_t
    
class cap_pbc_csr_sta_rpl_err_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_sta_rpl_err_t(string name = "cap_pbc_csr_sta_rpl_err_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_sta_rpl_err_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 5 > rid_cpp_int_t;
        rid_cpp_int_t int_var__rid;
        void rid (const cpp_int  & l__val);
        cpp_int rid() const;
    
        typedef pu_cpp_int< 2 > rresp_cpp_int_t;
        rresp_cpp_int_t int_var__rresp;
        void rresp (const cpp_int  & l__val);
        cpp_int rresp() const;
    
}; // cap_pbc_csr_sta_rpl_err_t
    
class cap_pbc_csr_sta_ecc_sideband_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_sta_ecc_sideband_t(string name = "cap_pbc_csr_sta_ecc_sideband_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_sta_ecc_sideband_t();
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
    
        typedef pu_cpp_int< 14 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pbc_csr_sta_ecc_sideband_t
    
class cap_pbc_csr_cfg_spare_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_spare_t(string name = "cap_pbc_csr_cfg_spare_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_spare_t();
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
    
}; // cap_pbc_csr_cfg_spare_t
    
class cap_pbc_csr_cfg_debug_port_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_debug_port_t(string name = "cap_pbc_csr_cfg_debug_port_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_debug_port_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        enable_cpp_int_t int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 1 > select_cpp_int_t;
        select_cpp_int_t int_var__select;
        void select (const cpp_int  & l__val);
        cpp_int select() const;
    
}; // cap_pbc_csr_cfg_debug_port_t
    
class cap_pbc_csr_cfg_dhs_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_dhs_t(string name = "cap_pbc_csr_cfg_dhs_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_dhs_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 14 > address_cpp_int_t;
        address_cpp_int_t int_var__address;
        void address (const cpp_int  & l__val);
        cpp_int address() const;
    
}; // cap_pbc_csr_cfg_dhs_t
    
class cap_pbc_csr_sta_oq_12_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_sta_oq_12_t(string name = "cap_pbc_csr_sta_oq_12_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_sta_oq_12_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > depth_value_cpp_int_t;
        depth_value_cpp_int_t int_var__depth_value;
        void depth_value (const cpp_int  & l__val);
        cpp_int depth_value() const;
    
}; // cap_pbc_csr_sta_oq_12_t
    
class cap_pbc_csr_sta_oq_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_sta_oq_t(string name = "cap_pbc_csr_sta_oq_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_sta_oq_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 512 > depth_value_cpp_int_t;
        depth_value_cpp_int_t int_var__depth_value;
        void depth_value (const cpp_int  & l__val);
        cpp_int depth_value() const;
    
}; // cap_pbc_csr_sta_oq_t
    
class cap_pbc_csr_cnt_write_error_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cnt_write_error_t(string name = "cap_pbc_csr_cnt_write_error_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cnt_write_error_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > discarded_cpp_int_t;
        discarded_cpp_int_t int_var__discarded;
        void discarded (const cpp_int  & l__val);
        cpp_int discarded() const;
    
        typedef pu_cpp_int< 16 > admitted_cpp_int_t;
        admitted_cpp_int_t int_var__admitted;
        void admitted (const cpp_int  & l__val);
        cpp_int admitted() const;
    
        typedef pu_cpp_int< 16 > intrinsic_drop_cpp_int_t;
        intrinsic_drop_cpp_int_t int_var__intrinsic_drop;
        void intrinsic_drop (const cpp_int  & l__val);
        cpp_int intrinsic_drop() const;
    
        typedef pu_cpp_int< 8 > out_of_cells_cpp_int_t;
        out_of_cells_cpp_int_t int_var__out_of_cells;
        void out_of_cells (const cpp_int  & l__val);
        cpp_int out_of_cells() const;
    
        typedef pu_cpp_int< 8 > out_of_credit_cpp_int_t;
        out_of_credit_cpp_int_t int_var__out_of_credit;
        void out_of_credit (const cpp_int  & l__val);
        cpp_int out_of_credit() const;
    
        typedef pu_cpp_int< 8 > truncation_cpp_int_t;
        truncation_cpp_int_t int_var__truncation;
        void truncation (const cpp_int  & l__val);
        cpp_int truncation() const;
    
        typedef pu_cpp_int< 8 > port_disabled_cpp_int_t;
        port_disabled_cpp_int_t int_var__port_disabled;
        void port_disabled (const cpp_int  & l__val);
        cpp_int port_disabled() const;
    
        typedef pu_cpp_int< 16 > out_of_cells1_cpp_int_t;
        out_of_cells1_cpp_int_t int_var__out_of_cells1;
        void out_of_cells1 (const cpp_int  & l__val);
        cpp_int out_of_cells1() const;
    
        typedef pu_cpp_int< 16 > tail_drop_cpu_cpp_int_t;
        tail_drop_cpu_cpp_int_t int_var__tail_drop_cpu;
        void tail_drop_cpu (const cpp_int  & l__val);
        cpp_int tail_drop_cpu() const;
    
        typedef pu_cpp_int< 16 > tail_drop_span_cpp_int_t;
        tail_drop_span_cpp_int_t int_var__tail_drop_span;
        void tail_drop_span (const cpp_int  & l__val);
        cpp_int tail_drop_span() const;
    
        typedef pu_cpp_int< 8 > enqueue_cpp_int_t;
        enqueue_cpp_int_t int_var__enqueue;
        void enqueue (const cpp_int  & l__val);
        cpp_int enqueue() const;
    
}; // cap_pbc_csr_cnt_write_error_t
    
class cap_pbc_csr_cnt_flits_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cnt_flits_t(string name = "cap_pbc_csr_cnt_flits_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cnt_flits_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > sop_in_cpp_int_t;
        sop_in_cpp_int_t int_var__sop_in;
        void sop_in (const cpp_int  & l__val);
        cpp_int sop_in() const;
    
        typedef pu_cpp_int< 16 > eop_in_cpp_int_t;
        eop_in_cpp_int_t int_var__eop_in;
        void eop_in (const cpp_int  & l__val);
        cpp_int eop_in() const;
    
        typedef pu_cpp_int< 16 > sop_out_cpp_int_t;
        sop_out_cpp_int_t int_var__sop_out;
        void sop_out (const cpp_int  & l__val);
        cpp_int sop_out() const;
    
        typedef pu_cpp_int< 16 > eop_out_cpp_int_t;
        eop_out_cpp_int_t int_var__eop_out;
        void eop_out (const cpp_int  & l__val);
        cpp_int eop_out() const;
    
}; // cap_pbc_csr_cnt_flits_t
    
class cap_pbc_csr_cfg_parser8_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_parser8_t(string name = "cap_pbc_csr_cfg_parser8_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_parser8_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > dot1q_type_cpp_int_t;
        dot1q_type_cpp_int_t int_var__dot1q_type;
        void dot1q_type (const cpp_int  & l__val);
        cpp_int dot1q_type() const;
    
        typedef pu_cpp_int< 16 > ipv4_type_cpp_int_t;
        ipv4_type_cpp_int_t int_var__ipv4_type;
        void ipv4_type (const cpp_int  & l__val);
        cpp_int ipv4_type() const;
    
        typedef pu_cpp_int< 16 > ipv6_type_cpp_int_t;
        ipv6_type_cpp_int_t int_var__ipv6_type;
        void ipv6_type (const cpp_int  & l__val);
        cpp_int ipv6_type() const;
    
        typedef pu_cpp_int< 1 > use_ip_cpp_int_t;
        use_ip_cpp_int_t int_var__use_ip;
        void use_ip (const cpp_int  & l__val);
        cpp_int use_ip() const;
    
        typedef pu_cpp_int< 1 > use_dot1q_cpp_int_t;
        use_dot1q_cpp_int_t int_var__use_dot1q;
        void use_dot1q (const cpp_int  & l__val);
        cpp_int use_dot1q() const;
    
        typedef pu_cpp_int< 3 > default_cos_cpp_int_t;
        default_cos_cpp_int_t int_var__default_cos;
        void default_cos (const cpp_int  & l__val);
        cpp_int default_cos() const;
    
        typedef pu_cpp_int< 4 > default_port_cpp_int_t;
        default_port_cpp_int_t int_var__default_port;
        void default_port (const cpp_int  & l__val);
        cpp_int default_port() const;
    
        typedef pu_cpp_int< 40 > oq_map_cpp_int_t;
        oq_map_cpp_int_t int_var__oq_map;
        void oq_map (const cpp_int  & l__val);
        cpp_int oq_map() const;
    
        typedef pu_cpp_int< 192 > dscp_map_cpp_int_t;
        dscp_map_cpp_int_t int_var__dscp_map;
        void dscp_map (const cpp_int  & l__val);
        cpp_int dscp_map() const;
    
}; // cap_pbc_csr_cfg_parser8_t
    
class cap_pbc_csr_cfg_parser7_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_parser7_t(string name = "cap_pbc_csr_cfg_parser7_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_parser7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > dot1q_type_cpp_int_t;
        dot1q_type_cpp_int_t int_var__dot1q_type;
        void dot1q_type (const cpp_int  & l__val);
        cpp_int dot1q_type() const;
    
        typedef pu_cpp_int< 16 > ipv4_type_cpp_int_t;
        ipv4_type_cpp_int_t int_var__ipv4_type;
        void ipv4_type (const cpp_int  & l__val);
        cpp_int ipv4_type() const;
    
        typedef pu_cpp_int< 16 > ipv6_type_cpp_int_t;
        ipv6_type_cpp_int_t int_var__ipv6_type;
        void ipv6_type (const cpp_int  & l__val);
        cpp_int ipv6_type() const;
    
        typedef pu_cpp_int< 1 > use_ip_cpp_int_t;
        use_ip_cpp_int_t int_var__use_ip;
        void use_ip (const cpp_int  & l__val);
        cpp_int use_ip() const;
    
        typedef pu_cpp_int< 1 > use_dot1q_cpp_int_t;
        use_dot1q_cpp_int_t int_var__use_dot1q;
        void use_dot1q (const cpp_int  & l__val);
        cpp_int use_dot1q() const;
    
        typedef pu_cpp_int< 3 > default_cos_cpp_int_t;
        default_cos_cpp_int_t int_var__default_cos;
        void default_cos (const cpp_int  & l__val);
        cpp_int default_cos() const;
    
        typedef pu_cpp_int< 4 > default_port_cpp_int_t;
        default_port_cpp_int_t int_var__default_port;
        void default_port (const cpp_int  & l__val);
        cpp_int default_port() const;
    
        typedef pu_cpp_int< 40 > oq_map_cpp_int_t;
        oq_map_cpp_int_t int_var__oq_map;
        void oq_map (const cpp_int  & l__val);
        cpp_int oq_map() const;
    
        typedef pu_cpp_int< 192 > dscp_map_cpp_int_t;
        dscp_map_cpp_int_t int_var__dscp_map;
        void dscp_map (const cpp_int  & l__val);
        cpp_int dscp_map() const;
    
}; // cap_pbc_csr_cfg_parser7_t
    
class cap_pbc_csr_cfg_parser6_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_parser6_t(string name = "cap_pbc_csr_cfg_parser6_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_parser6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > dot1q_type_cpp_int_t;
        dot1q_type_cpp_int_t int_var__dot1q_type;
        void dot1q_type (const cpp_int  & l__val);
        cpp_int dot1q_type() const;
    
        typedef pu_cpp_int< 16 > ipv4_type_cpp_int_t;
        ipv4_type_cpp_int_t int_var__ipv4_type;
        void ipv4_type (const cpp_int  & l__val);
        cpp_int ipv4_type() const;
    
        typedef pu_cpp_int< 16 > ipv6_type_cpp_int_t;
        ipv6_type_cpp_int_t int_var__ipv6_type;
        void ipv6_type (const cpp_int  & l__val);
        cpp_int ipv6_type() const;
    
        typedef pu_cpp_int< 1 > use_ip_cpp_int_t;
        use_ip_cpp_int_t int_var__use_ip;
        void use_ip (const cpp_int  & l__val);
        cpp_int use_ip() const;
    
        typedef pu_cpp_int< 1 > use_dot1q_cpp_int_t;
        use_dot1q_cpp_int_t int_var__use_dot1q;
        void use_dot1q (const cpp_int  & l__val);
        cpp_int use_dot1q() const;
    
        typedef pu_cpp_int< 3 > default_cos_cpp_int_t;
        default_cos_cpp_int_t int_var__default_cos;
        void default_cos (const cpp_int  & l__val);
        cpp_int default_cos() const;
    
        typedef pu_cpp_int< 4 > default_port_cpp_int_t;
        default_port_cpp_int_t int_var__default_port;
        void default_port (const cpp_int  & l__val);
        cpp_int default_port() const;
    
        typedef pu_cpp_int< 40 > oq_map_cpp_int_t;
        oq_map_cpp_int_t int_var__oq_map;
        void oq_map (const cpp_int  & l__val);
        cpp_int oq_map() const;
    
        typedef pu_cpp_int< 192 > dscp_map_cpp_int_t;
        dscp_map_cpp_int_t int_var__dscp_map;
        void dscp_map (const cpp_int  & l__val);
        cpp_int dscp_map() const;
    
}; // cap_pbc_csr_cfg_parser6_t
    
class cap_pbc_csr_cfg_parser5_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_parser5_t(string name = "cap_pbc_csr_cfg_parser5_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_parser5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > dot1q_type_cpp_int_t;
        dot1q_type_cpp_int_t int_var__dot1q_type;
        void dot1q_type (const cpp_int  & l__val);
        cpp_int dot1q_type() const;
    
        typedef pu_cpp_int< 16 > ipv4_type_cpp_int_t;
        ipv4_type_cpp_int_t int_var__ipv4_type;
        void ipv4_type (const cpp_int  & l__val);
        cpp_int ipv4_type() const;
    
        typedef pu_cpp_int< 16 > ipv6_type_cpp_int_t;
        ipv6_type_cpp_int_t int_var__ipv6_type;
        void ipv6_type (const cpp_int  & l__val);
        cpp_int ipv6_type() const;
    
        typedef pu_cpp_int< 1 > use_ip_cpp_int_t;
        use_ip_cpp_int_t int_var__use_ip;
        void use_ip (const cpp_int  & l__val);
        cpp_int use_ip() const;
    
        typedef pu_cpp_int< 1 > use_dot1q_cpp_int_t;
        use_dot1q_cpp_int_t int_var__use_dot1q;
        void use_dot1q (const cpp_int  & l__val);
        cpp_int use_dot1q() const;
    
        typedef pu_cpp_int< 3 > default_cos_cpp_int_t;
        default_cos_cpp_int_t int_var__default_cos;
        void default_cos (const cpp_int  & l__val);
        cpp_int default_cos() const;
    
        typedef pu_cpp_int< 4 > default_port_cpp_int_t;
        default_port_cpp_int_t int_var__default_port;
        void default_port (const cpp_int  & l__val);
        cpp_int default_port() const;
    
        typedef pu_cpp_int< 40 > oq_map_cpp_int_t;
        oq_map_cpp_int_t int_var__oq_map;
        void oq_map (const cpp_int  & l__val);
        cpp_int oq_map() const;
    
        typedef pu_cpp_int< 192 > dscp_map_cpp_int_t;
        dscp_map_cpp_int_t int_var__dscp_map;
        void dscp_map (const cpp_int  & l__val);
        cpp_int dscp_map() const;
    
}; // cap_pbc_csr_cfg_parser5_t
    
class cap_pbc_csr_cfg_parser4_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_parser4_t(string name = "cap_pbc_csr_cfg_parser4_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_parser4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > dot1q_type_cpp_int_t;
        dot1q_type_cpp_int_t int_var__dot1q_type;
        void dot1q_type (const cpp_int  & l__val);
        cpp_int dot1q_type() const;
    
        typedef pu_cpp_int< 16 > ipv4_type_cpp_int_t;
        ipv4_type_cpp_int_t int_var__ipv4_type;
        void ipv4_type (const cpp_int  & l__val);
        cpp_int ipv4_type() const;
    
        typedef pu_cpp_int< 16 > ipv6_type_cpp_int_t;
        ipv6_type_cpp_int_t int_var__ipv6_type;
        void ipv6_type (const cpp_int  & l__val);
        cpp_int ipv6_type() const;
    
        typedef pu_cpp_int< 1 > use_ip_cpp_int_t;
        use_ip_cpp_int_t int_var__use_ip;
        void use_ip (const cpp_int  & l__val);
        cpp_int use_ip() const;
    
        typedef pu_cpp_int< 1 > use_dot1q_cpp_int_t;
        use_dot1q_cpp_int_t int_var__use_dot1q;
        void use_dot1q (const cpp_int  & l__val);
        cpp_int use_dot1q() const;
    
        typedef pu_cpp_int< 3 > default_cos_cpp_int_t;
        default_cos_cpp_int_t int_var__default_cos;
        void default_cos (const cpp_int  & l__val);
        cpp_int default_cos() const;
    
        typedef pu_cpp_int< 4 > default_port_cpp_int_t;
        default_port_cpp_int_t int_var__default_port;
        void default_port (const cpp_int  & l__val);
        cpp_int default_port() const;
    
        typedef pu_cpp_int< 40 > oq_map_cpp_int_t;
        oq_map_cpp_int_t int_var__oq_map;
        void oq_map (const cpp_int  & l__val);
        cpp_int oq_map() const;
    
        typedef pu_cpp_int< 192 > dscp_map_cpp_int_t;
        dscp_map_cpp_int_t int_var__dscp_map;
        void dscp_map (const cpp_int  & l__val);
        cpp_int dscp_map() const;
    
}; // cap_pbc_csr_cfg_parser4_t
    
class cap_pbc_csr_cfg_parser3_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_parser3_t(string name = "cap_pbc_csr_cfg_parser3_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_parser3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > dot1q_type_cpp_int_t;
        dot1q_type_cpp_int_t int_var__dot1q_type;
        void dot1q_type (const cpp_int  & l__val);
        cpp_int dot1q_type() const;
    
        typedef pu_cpp_int< 16 > ipv4_type_cpp_int_t;
        ipv4_type_cpp_int_t int_var__ipv4_type;
        void ipv4_type (const cpp_int  & l__val);
        cpp_int ipv4_type() const;
    
        typedef pu_cpp_int< 16 > ipv6_type_cpp_int_t;
        ipv6_type_cpp_int_t int_var__ipv6_type;
        void ipv6_type (const cpp_int  & l__val);
        cpp_int ipv6_type() const;
    
        typedef pu_cpp_int< 1 > use_ip_cpp_int_t;
        use_ip_cpp_int_t int_var__use_ip;
        void use_ip (const cpp_int  & l__val);
        cpp_int use_ip() const;
    
        typedef pu_cpp_int< 1 > use_dot1q_cpp_int_t;
        use_dot1q_cpp_int_t int_var__use_dot1q;
        void use_dot1q (const cpp_int  & l__val);
        cpp_int use_dot1q() const;
    
        typedef pu_cpp_int< 3 > default_cos_cpp_int_t;
        default_cos_cpp_int_t int_var__default_cos;
        void default_cos (const cpp_int  & l__val);
        cpp_int default_cos() const;
    
        typedef pu_cpp_int< 4 > default_port_cpp_int_t;
        default_port_cpp_int_t int_var__default_port;
        void default_port (const cpp_int  & l__val);
        cpp_int default_port() const;
    
        typedef pu_cpp_int< 40 > oq_map_cpp_int_t;
        oq_map_cpp_int_t int_var__oq_map;
        void oq_map (const cpp_int  & l__val);
        cpp_int oq_map() const;
    
        typedef pu_cpp_int< 192 > dscp_map_cpp_int_t;
        dscp_map_cpp_int_t int_var__dscp_map;
        void dscp_map (const cpp_int  & l__val);
        cpp_int dscp_map() const;
    
}; // cap_pbc_csr_cfg_parser3_t
    
class cap_pbc_csr_cfg_parser2_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_parser2_t(string name = "cap_pbc_csr_cfg_parser2_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_parser2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > dot1q_type_cpp_int_t;
        dot1q_type_cpp_int_t int_var__dot1q_type;
        void dot1q_type (const cpp_int  & l__val);
        cpp_int dot1q_type() const;
    
        typedef pu_cpp_int< 16 > ipv4_type_cpp_int_t;
        ipv4_type_cpp_int_t int_var__ipv4_type;
        void ipv4_type (const cpp_int  & l__val);
        cpp_int ipv4_type() const;
    
        typedef pu_cpp_int< 16 > ipv6_type_cpp_int_t;
        ipv6_type_cpp_int_t int_var__ipv6_type;
        void ipv6_type (const cpp_int  & l__val);
        cpp_int ipv6_type() const;
    
        typedef pu_cpp_int< 1 > use_ip_cpp_int_t;
        use_ip_cpp_int_t int_var__use_ip;
        void use_ip (const cpp_int  & l__val);
        cpp_int use_ip() const;
    
        typedef pu_cpp_int< 1 > use_dot1q_cpp_int_t;
        use_dot1q_cpp_int_t int_var__use_dot1q;
        void use_dot1q (const cpp_int  & l__val);
        cpp_int use_dot1q() const;
    
        typedef pu_cpp_int< 3 > default_cos_cpp_int_t;
        default_cos_cpp_int_t int_var__default_cos;
        void default_cos (const cpp_int  & l__val);
        cpp_int default_cos() const;
    
        typedef pu_cpp_int< 4 > default_port_cpp_int_t;
        default_port_cpp_int_t int_var__default_port;
        void default_port (const cpp_int  & l__val);
        cpp_int default_port() const;
    
        typedef pu_cpp_int< 40 > oq_map_cpp_int_t;
        oq_map_cpp_int_t int_var__oq_map;
        void oq_map (const cpp_int  & l__val);
        cpp_int oq_map() const;
    
        typedef pu_cpp_int< 192 > dscp_map_cpp_int_t;
        dscp_map_cpp_int_t int_var__dscp_map;
        void dscp_map (const cpp_int  & l__val);
        cpp_int dscp_map() const;
    
}; // cap_pbc_csr_cfg_parser2_t
    
class cap_pbc_csr_cfg_parser1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_parser1_t(string name = "cap_pbc_csr_cfg_parser1_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_parser1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > dot1q_type_cpp_int_t;
        dot1q_type_cpp_int_t int_var__dot1q_type;
        void dot1q_type (const cpp_int  & l__val);
        cpp_int dot1q_type() const;
    
        typedef pu_cpp_int< 16 > ipv4_type_cpp_int_t;
        ipv4_type_cpp_int_t int_var__ipv4_type;
        void ipv4_type (const cpp_int  & l__val);
        cpp_int ipv4_type() const;
    
        typedef pu_cpp_int< 16 > ipv6_type_cpp_int_t;
        ipv6_type_cpp_int_t int_var__ipv6_type;
        void ipv6_type (const cpp_int  & l__val);
        cpp_int ipv6_type() const;
    
        typedef pu_cpp_int< 1 > use_ip_cpp_int_t;
        use_ip_cpp_int_t int_var__use_ip;
        void use_ip (const cpp_int  & l__val);
        cpp_int use_ip() const;
    
        typedef pu_cpp_int< 1 > use_dot1q_cpp_int_t;
        use_dot1q_cpp_int_t int_var__use_dot1q;
        void use_dot1q (const cpp_int  & l__val);
        cpp_int use_dot1q() const;
    
        typedef pu_cpp_int< 3 > default_cos_cpp_int_t;
        default_cos_cpp_int_t int_var__default_cos;
        void default_cos (const cpp_int  & l__val);
        cpp_int default_cos() const;
    
        typedef pu_cpp_int< 4 > default_port_cpp_int_t;
        default_port_cpp_int_t int_var__default_port;
        void default_port (const cpp_int  & l__val);
        cpp_int default_port() const;
    
        typedef pu_cpp_int< 40 > oq_map_cpp_int_t;
        oq_map_cpp_int_t int_var__oq_map;
        void oq_map (const cpp_int  & l__val);
        cpp_int oq_map() const;
    
        typedef pu_cpp_int< 192 > dscp_map_cpp_int_t;
        dscp_map_cpp_int_t int_var__dscp_map;
        void dscp_map (const cpp_int  & l__val);
        cpp_int dscp_map() const;
    
}; // cap_pbc_csr_cfg_parser1_t
    
class cap_pbc_csr_cfg_parser0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_parser0_t(string name = "cap_pbc_csr_cfg_parser0_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_parser0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > dot1q_type_cpp_int_t;
        dot1q_type_cpp_int_t int_var__dot1q_type;
        void dot1q_type (const cpp_int  & l__val);
        cpp_int dot1q_type() const;
    
        typedef pu_cpp_int< 16 > ipv4_type_cpp_int_t;
        ipv4_type_cpp_int_t int_var__ipv4_type;
        void ipv4_type (const cpp_int  & l__val);
        cpp_int ipv4_type() const;
    
        typedef pu_cpp_int< 16 > ipv6_type_cpp_int_t;
        ipv6_type_cpp_int_t int_var__ipv6_type;
        void ipv6_type (const cpp_int  & l__val);
        cpp_int ipv6_type() const;
    
        typedef pu_cpp_int< 1 > use_ip_cpp_int_t;
        use_ip_cpp_int_t int_var__use_ip;
        void use_ip (const cpp_int  & l__val);
        cpp_int use_ip() const;
    
        typedef pu_cpp_int< 1 > use_dot1q_cpp_int_t;
        use_dot1q_cpp_int_t int_var__use_dot1q;
        void use_dot1q (const cpp_int  & l__val);
        cpp_int use_dot1q() const;
    
        typedef pu_cpp_int< 3 > default_cos_cpp_int_t;
        default_cos_cpp_int_t int_var__default_cos;
        void default_cos (const cpp_int  & l__val);
        cpp_int default_cos() const;
    
        typedef pu_cpp_int< 4 > default_port_cpp_int_t;
        default_port_cpp_int_t int_var__default_port;
        void default_port (const cpp_int  & l__val);
        cpp_int default_port() const;
    
        typedef pu_cpp_int< 40 > oq_map_cpp_int_t;
        oq_map_cpp_int_t int_var__oq_map;
        void oq_map (const cpp_int  & l__val);
        cpp_int oq_map() const;
    
        typedef pu_cpp_int< 192 > dscp_map_cpp_int_t;
        dscp_map_cpp_int_t int_var__dscp_map;
        void dscp_map (const cpp_int  & l__val);
        cpp_int dscp_map() const;
    
}; // cap_pbc_csr_cfg_parser0_t
    
class cap_pbc_csr_cfg_rpl_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_rpl_t(string name = "cap_pbc_csr_cfg_rpl_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_rpl_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 27 > base_cpp_int_t;
        base_cpp_int_t int_var__base;
        void base (const cpp_int  & l__val);
        cpp_int base() const;
    
        typedef pu_cpp_int< 2 > token_size_cpp_int_t;
        token_size_cpp_int_t int_var__token_size;
        void token_size (const cpp_int  & l__val);
        cpp_int token_size() const;
    
}; // cap_pbc_csr_cfg_rpl_t
    
class cap_pbc_csr_cfg_island_control_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_island_control_t(string name = "cap_pbc_csr_cfg_island_control_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_island_control_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > map_cpp_int_t;
        map_cpp_int_t int_var__map;
        void map (const cpp_int  & l__val);
        cpp_int map() const;
    
}; // cap_pbc_csr_cfg_island_control_t
    
class cap_pbc_csr_sta_sched_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_sta_sched_t(string name = "cap_pbc_csr_sta_sched_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_sta_sched_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > ecc_uncorrectable_cpp_int_t;
        ecc_uncorrectable_cpp_int_t int_var__ecc_uncorrectable;
        void ecc_uncorrectable (const cpp_int  & l__val);
        cpp_int ecc_uncorrectable() const;
    
        typedef pu_cpp_int< 1 > ecc_correctable_cpp_int_t;
        ecc_correctable_cpp_int_t int_var__ecc_correctable;
        void ecc_correctable (const cpp_int  & l__val);
        cpp_int ecc_correctable() const;
    
        typedef pu_cpp_int< 8 > ecc_syndrome_cpp_int_t;
        ecc_syndrome_cpp_int_t int_var__ecc_syndrome;
        void ecc_syndrome (const cpp_int  & l__val);
        cpp_int ecc_syndrome() const;
    
        typedef pu_cpp_int< 9 > ecc_addr_cpp_int_t;
        ecc_addr_cpp_int_t int_var__ecc_addr;
        void ecc_addr (const cpp_int  & l__val);
        cpp_int ecc_addr() const;
    
}; // cap_pbc_csr_sta_sched_t
    
class cap_pbc_csr_cfg_sched_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_sched_t(string name = "cap_pbc_csr_cfg_sched_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_sched_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > enable_wrr_cpp_int_t;
        enable_wrr_cpp_int_t int_var__enable_wrr;
        void enable_wrr (const cpp_int  & l__val);
        cpp_int enable_wrr() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        ecc_disable_det_cpp_int_t int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        ecc_disable_cor_cpp_int_t int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        dhs_eccbypass_cpp_int_t int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 2 > dhs_selection_cpp_int_t;
        dhs_selection_cpp_int_t int_var__dhs_selection;
        void dhs_selection (const cpp_int  & l__val);
        cpp_int dhs_selection() const;
    
        typedef pu_cpp_int< 32 > timer_cpp_int_t;
        timer_cpp_int_t int_var__timer;
        void timer (const cpp_int  & l__val);
        cpp_int timer() const;
    
}; // cap_pbc_csr_cfg_sched_t
    
class cap_pbc_csr_sta_rc_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_sta_rc_t(string name = "cap_pbc_csr_sta_rc_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_sta_rc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > init_done_cpp_int_t;
        init_done_cpp_int_t int_var__init_done;
        void init_done (const cpp_int  & l__val);
        cpp_int init_done() const;
    
        typedef pu_cpp_int< 1 > ecc_uncorrectable_cpp_int_t;
        ecc_uncorrectable_cpp_int_t int_var__ecc_uncorrectable;
        void ecc_uncorrectable (const cpp_int  & l__val);
        cpp_int ecc_uncorrectable() const;
    
        typedef pu_cpp_int< 1 > ecc_correctable_cpp_int_t;
        ecc_correctable_cpp_int_t int_var__ecc_correctable;
        void ecc_correctable (const cpp_int  & l__val);
        cpp_int ecc_correctable() const;
    
        typedef pu_cpp_int< 6 > ecc_syndrome_cpp_int_t;
        ecc_syndrome_cpp_int_t int_var__ecc_syndrome;
        void ecc_syndrome (const cpp_int  & l__val);
        cpp_int ecc_syndrome() const;
    
        typedef pu_cpp_int< 11 > ecc_addr_cpp_int_t;
        ecc_addr_cpp_int_t int_var__ecc_addr;
        void ecc_addr (const cpp_int  & l__val);
        cpp_int ecc_addr() const;
    
}; // cap_pbc_csr_sta_rc_t
    
class cap_pbc_csr_cfg_rc_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_rc_t(string name = "cap_pbc_csr_cfg_rc_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_rc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > init_start_cpp_int_t;
        init_start_cpp_int_t int_var__init_start;
        void init_start (const cpp_int  & l__val);
        cpp_int init_start() const;
    
        typedef pu_cpp_int< 1 > init_reset_cpp_int_t;
        init_reset_cpp_int_t int_var__init_reset;
        void init_reset (const cpp_int  & l__val);
        cpp_int init_reset() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        ecc_disable_det_cpp_int_t int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        ecc_disable_cor_cpp_int_t int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        dhs_eccbypass_cpp_int_t int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
}; // cap_pbc_csr_cfg_rc_t
    
class cap_pbc_csr_cfg_gc_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_gc_t(string name = "cap_pbc_csr_cfg_gc_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_gc_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        ecc_disable_det_cpp_int_t int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        ecc_disable_cor_cpp_int_t int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        dhs_eccbypass_cpp_int_t int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
}; // cap_pbc_csr_cfg_gc_t
    
class cap_pbc_csr_sta_ecc_desc_1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_sta_ecc_desc_1_t(string name = "cap_pbc_csr_sta_ecc_desc_1_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_sta_ecc_desc_1_t();
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
    
        typedef pu_cpp_int< 14 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pbc_csr_sta_ecc_desc_1_t
    
class cap_pbc_csr_cfg_desc_1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_desc_1_t(string name = "cap_pbc_csr_cfg_desc_1_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_desc_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        ecc_disable_det_cpp_int_t int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        ecc_disable_cor_cpp_int_t int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        dhs_eccbypass_cpp_int_t int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
}; // cap_pbc_csr_cfg_desc_1_t
    
class cap_pbc_csr_sta_ecc_ll_1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_sta_ecc_ll_1_t(string name = "cap_pbc_csr_sta_ecc_ll_1_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_sta_ecc_ll_1_t();
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
    
        typedef pu_cpp_int< 14 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pbc_csr_sta_ecc_ll_1_t
    
class cap_pbc_csr_cfg_ll_1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_ll_1_t(string name = "cap_pbc_csr_cfg_ll_1_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_ll_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        ecc_disable_det_cpp_int_t int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        ecc_disable_cor_cpp_int_t int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        dhs_eccbypass_cpp_int_t int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
}; // cap_pbc_csr_cfg_ll_1_t
    
class cap_pbc_csr_sta_ecc_fc_1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_sta_ecc_fc_1_t(string name = "cap_pbc_csr_sta_ecc_fc_1_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_sta_ecc_fc_1_t();
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
    
        typedef pu_cpp_int< 12 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pbc_csr_sta_ecc_fc_1_t
    
class cap_pbc_csr_cfg_fc_mgr_1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_fc_mgr_1_t(string name = "cap_pbc_csr_cfg_fc_mgr_1_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_fc_mgr_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > init_start_cpp_int_t;
        init_start_cpp_int_t int_var__init_start;
        void init_start (const cpp_int  & l__val);
        cpp_int init_start() const;
    
        typedef pu_cpp_int< 1 > init_reset_cpp_int_t;
        init_reset_cpp_int_t int_var__init_reset;
        void init_reset (const cpp_int  & l__val);
        cpp_int init_reset() const;
    
        typedef pu_cpp_int< 12 > max_row_cpp_int_t;
        max_row_cpp_int_t int_var__max_row;
        void max_row (const cpp_int  & l__val);
        cpp_int max_row() const;
    
        typedef pu_cpp_int< 13 > min_cell_cpp_int_t;
        min_cell_cpp_int_t int_var__min_cell;
        void min_cell (const cpp_int  & l__val);
        cpp_int min_cell() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        ecc_disable_det_cpp_int_t int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        ecc_disable_cor_cpp_int_t int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        dhs_eccbypass_cpp_int_t int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
}; // cap_pbc_csr_cfg_fc_mgr_1_t
    
class cap_pbc_csr_sta_fc_mgr_1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_sta_fc_mgr_1_t(string name = "cap_pbc_csr_sta_fc_mgr_1_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_sta_fc_mgr_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 13 > cells_available_cpp_int_t;
        cells_available_cpp_int_t int_var__cells_available;
        void cells_available (const cpp_int  & l__val);
        cpp_int cells_available() const;
    
        typedef pu_cpp_int< 1 > init_done_cpp_int_t;
        init_done_cpp_int_t int_var__init_done;
        void init_done (const cpp_int  & l__val);
        cpp_int init_done() const;
    
}; // cap_pbc_csr_sta_fc_mgr_1_t
    
class cap_pbc_csr_sta_ecc_desc_0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_sta_ecc_desc_0_t(string name = "cap_pbc_csr_sta_ecc_desc_0_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_sta_ecc_desc_0_t();
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
    
        typedef pu_cpp_int< 14 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pbc_csr_sta_ecc_desc_0_t
    
class cap_pbc_csr_cfg_desc_0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_desc_0_t(string name = "cap_pbc_csr_cfg_desc_0_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_desc_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        ecc_disable_det_cpp_int_t int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        ecc_disable_cor_cpp_int_t int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        dhs_eccbypass_cpp_int_t int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
}; // cap_pbc_csr_cfg_desc_0_t
    
class cap_pbc_csr_sta_ecc_ll_0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_sta_ecc_ll_0_t(string name = "cap_pbc_csr_sta_ecc_ll_0_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_sta_ecc_ll_0_t();
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
    
        typedef pu_cpp_int< 14 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pbc_csr_sta_ecc_ll_0_t
    
class cap_pbc_csr_cfg_ll_0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_ll_0_t(string name = "cap_pbc_csr_cfg_ll_0_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_ll_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        ecc_disable_det_cpp_int_t int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        ecc_disable_cor_cpp_int_t int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        dhs_eccbypass_cpp_int_t int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
}; // cap_pbc_csr_cfg_ll_0_t
    
class cap_pbc_csr_sta_ecc_fc_0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_sta_ecc_fc_0_t(string name = "cap_pbc_csr_sta_ecc_fc_0_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_sta_ecc_fc_0_t();
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
    
        typedef pu_cpp_int< 12 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pbc_csr_sta_ecc_fc_0_t
    
class cap_pbc_csr_cfg_fc_mgr_0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_fc_mgr_0_t(string name = "cap_pbc_csr_cfg_fc_mgr_0_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_fc_mgr_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > init_start_cpp_int_t;
        init_start_cpp_int_t int_var__init_start;
        void init_start (const cpp_int  & l__val);
        cpp_int init_start() const;
    
        typedef pu_cpp_int< 1 > init_reset_cpp_int_t;
        init_reset_cpp_int_t int_var__init_reset;
        void init_reset (const cpp_int  & l__val);
        cpp_int init_reset() const;
    
        typedef pu_cpp_int< 12 > max_row_cpp_int_t;
        max_row_cpp_int_t int_var__max_row;
        void max_row (const cpp_int  & l__val);
        cpp_int max_row() const;
    
        typedef pu_cpp_int< 13 > min_cell_cpp_int_t;
        min_cell_cpp_int_t int_var__min_cell;
        void min_cell (const cpp_int  & l__val);
        cpp_int min_cell() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        ecc_disable_det_cpp_int_t int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        ecc_disable_cor_cpp_int_t int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        dhs_eccbypass_cpp_int_t int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
}; // cap_pbc_csr_cfg_fc_mgr_0_t
    
class cap_pbc_csr_sta_fc_mgr_0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_sta_fc_mgr_0_t(string name = "cap_pbc_csr_sta_fc_mgr_0_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_sta_fc_mgr_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 13 > cells_available_cpp_int_t;
        cells_available_cpp_int_t int_var__cells_available;
        void cells_available (const cpp_int  & l__val);
        cpp_int cells_available() const;
    
        typedef pu_cpp_int< 1 > init_done_cpp_int_t;
        init_done_cpp_int_t int_var__init_done;
        void init_done (const cpp_int  & l__val);
        cpp_int init_done() const;
    
}; // cap_pbc_csr_sta_fc_mgr_0_t
    
class cap_pbc_csr_sta_ecc_rwr_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_sta_ecc_rwr_t(string name = "cap_pbc_csr_sta_ecc_rwr_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_sta_ecc_rwr_t();
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
    
        typedef pu_cpp_int< 18 > syndrome_cpp_int_t;
        syndrome_cpp_int_t int_var__syndrome;
        void syndrome (const cpp_int  & l__val);
        cpp_int syndrome() const;
    
        typedef pu_cpp_int< 15 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pbc_csr_sta_ecc_rwr_t
    
class cap_pbc_csr_sta_ecc_pack_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_sta_ecc_pack_t(string name = "cap_pbc_csr_sta_ecc_pack_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_sta_ecc_pack_t();
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
    
        typedef pu_cpp_int< 17 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pbc_csr_sta_ecc_pack_t
    
class cap_pbc_csr_sta_ecc_oq_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_sta_ecc_oq_t(string name = "cap_pbc_csr_sta_ecc_oq_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_sta_ecc_oq_t();
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
    
        typedef pu_cpp_int< 18 > addr_cpp_int_t;
        addr_cpp_int_t int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
}; // cap_pbc_csr_sta_ecc_oq_t
    
class cap_pbc_csr_cfg_oq_11_arb_l2_strict_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_11_arb_l2_strict_t(string name = "cap_pbc_csr_cfg_oq_11_arb_l2_strict_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_11_arb_l2_strict_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > priority_cpp_int_t;
        priority_cpp_int_t int_var__priority;
        void priority (const cpp_int  & l__val);
        cpp_int priority() const;
    
}; // cap_pbc_csr_cfg_oq_11_arb_l2_strict_t
    
class cap_pbc_csr_cfg_oq_11_arb_l2_selection_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_11_arb_l2_selection_t(string name = "cap_pbc_csr_cfg_oq_11_arb_l2_selection_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_11_arb_l2_selection_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > node_0_cpp_int_t;
        node_0_cpp_int_t int_var__node_0;
        void node_0 (const cpp_int  & l__val);
        cpp_int node_0() const;
    
        typedef pu_cpp_int< 16 > node_1_cpp_int_t;
        node_1_cpp_int_t int_var__node_1;
        void node_1 (const cpp_int  & l__val);
        cpp_int node_1() const;
    
        typedef pu_cpp_int< 16 > node_2_cpp_int_t;
        node_2_cpp_int_t int_var__node_2;
        void node_2 (const cpp_int  & l__val);
        cpp_int node_2() const;
    
        typedef pu_cpp_int< 16 > node_3_cpp_int_t;
        node_3_cpp_int_t int_var__node_3;
        void node_3 (const cpp_int  & l__val);
        cpp_int node_3() const;
    
}; // cap_pbc_csr_cfg_oq_11_arb_l2_selection_t
    
class cap_pbc_csr_cfg_oq_11_arb_l1_strict_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_11_arb_l1_strict_t(string name = "cap_pbc_csr_cfg_oq_11_arb_l1_strict_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_11_arb_l1_strict_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > priority_cpp_int_t;
        priority_cpp_int_t int_var__priority;
        void priority (const cpp_int  & l__val);
        cpp_int priority() const;
    
}; // cap_pbc_csr_cfg_oq_11_arb_l1_strict_t
    
class cap_pbc_csr_cfg_oq_11_arb_l1_selection_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_11_arb_l1_selection_t(string name = "cap_pbc_csr_cfg_oq_11_arb_l1_selection_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_11_arb_l1_selection_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > node_0_cpp_int_t;
        node_0_cpp_int_t int_var__node_0;
        void node_0 (const cpp_int  & l__val);
        cpp_int node_0() const;
    
        typedef pu_cpp_int< 32 > node_1_cpp_int_t;
        node_1_cpp_int_t int_var__node_1;
        void node_1 (const cpp_int  & l__val);
        cpp_int node_1() const;
    
        typedef pu_cpp_int< 32 > node_2_cpp_int_t;
        node_2_cpp_int_t int_var__node_2;
        void node_2 (const cpp_int  & l__val);
        cpp_int node_2() const;
    
        typedef pu_cpp_int< 32 > node_3_cpp_int_t;
        node_3_cpp_int_t int_var__node_3;
        void node_3 (const cpp_int  & l__val);
        cpp_int node_3() const;
    
        typedef pu_cpp_int< 32 > node_4_cpp_int_t;
        node_4_cpp_int_t int_var__node_4;
        void node_4 (const cpp_int  & l__val);
        cpp_int node_4() const;
    
        typedef pu_cpp_int< 32 > node_5_cpp_int_t;
        node_5_cpp_int_t int_var__node_5;
        void node_5 (const cpp_int  & l__val);
        cpp_int node_5() const;
    
        typedef pu_cpp_int< 32 > node_6_cpp_int_t;
        node_6_cpp_int_t int_var__node_6;
        void node_6 (const cpp_int  & l__val);
        cpp_int node_6() const;
    
        typedef pu_cpp_int< 32 > node_7_cpp_int_t;
        node_7_cpp_int_t int_var__node_7;
        void node_7 (const cpp_int  & l__val);
        cpp_int node_7() const;
    
        typedef pu_cpp_int< 32 > node_8_cpp_int_t;
        node_8_cpp_int_t int_var__node_8;
        void node_8 (const cpp_int  & l__val);
        cpp_int node_8() const;
    
        typedef pu_cpp_int< 32 > node_9_cpp_int_t;
        node_9_cpp_int_t int_var__node_9;
        void node_9 (const cpp_int  & l__val);
        cpp_int node_9() const;
    
        typedef pu_cpp_int< 32 > node_10_cpp_int_t;
        node_10_cpp_int_t int_var__node_10;
        void node_10 (const cpp_int  & l__val);
        cpp_int node_10() const;
    
        typedef pu_cpp_int< 32 > node_11_cpp_int_t;
        node_11_cpp_int_t int_var__node_11;
        void node_11 (const cpp_int  & l__val);
        cpp_int node_11() const;
    
        typedef pu_cpp_int< 32 > node_12_cpp_int_t;
        node_12_cpp_int_t int_var__node_12;
        void node_12 (const cpp_int  & l__val);
        cpp_int node_12() const;
    
        typedef pu_cpp_int< 32 > node_13_cpp_int_t;
        node_13_cpp_int_t int_var__node_13;
        void node_13 (const cpp_int  & l__val);
        cpp_int node_13() const;
    
        typedef pu_cpp_int< 32 > node_14_cpp_int_t;
        node_14_cpp_int_t int_var__node_14;
        void node_14 (const cpp_int  & l__val);
        cpp_int node_14() const;
    
        typedef pu_cpp_int< 32 > node_15_cpp_int_t;
        node_15_cpp_int_t int_var__node_15;
        void node_15 (const cpp_int  & l__val);
        cpp_int node_15() const;
    
}; // cap_pbc_csr_cfg_oq_11_arb_l1_selection_t
    
class cap_pbc_csr_cfg_oq_11_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_11_t(string name = "cap_pbc_csr_cfg_oq_11_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_11_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        enable_cpp_int_t int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        ecc_disable_det_cpp_int_t int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        ecc_disable_cor_cpp_int_t int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        dhs_eccbypass_cpp_int_t int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 1 > packing_msb_cpp_int_t;
        packing_msb_cpp_int_t int_var__packing_msb;
        void packing_msb (const cpp_int  & l__val);
        cpp_int packing_msb() const;
    
        typedef pu_cpp_int< 4 > dhs_selection_cpp_int_t;
        dhs_selection_cpp_int_t int_var__dhs_selection;
        void dhs_selection (const cpp_int  & l__val);
        cpp_int dhs_selection() const;
    
        typedef pu_cpp_int< 1 > rewrite_enable_cpp_int_t;
        rewrite_enable_cpp_int_t int_var__rewrite_enable;
        void rewrite_enable (const cpp_int  & l__val);
        cpp_int rewrite_enable() const;
    
        typedef pu_cpp_int< 5 > ipg_bytes_cpp_int_t;
        ipg_bytes_cpp_int_t int_var__ipg_bytes;
        void ipg_bytes (const cpp_int  & l__val);
        cpp_int ipg_bytes() const;
    
}; // cap_pbc_csr_cfg_oq_11_t
    
class cap_pbc_csr_cfg_account_11_pg_31_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_31_t(string name = "cap_pbc_csr_cfg_account_11_pg_31_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_31_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_31_t
    
class cap_pbc_csr_cfg_account_11_pg_30_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_30_t(string name = "cap_pbc_csr_cfg_account_11_pg_30_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_30_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_30_t
    
class cap_pbc_csr_cfg_account_11_pg_29_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_29_t(string name = "cap_pbc_csr_cfg_account_11_pg_29_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_29_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_29_t
    
class cap_pbc_csr_cfg_account_11_pg_28_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_28_t(string name = "cap_pbc_csr_cfg_account_11_pg_28_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_28_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_28_t
    
class cap_pbc_csr_cfg_account_11_pg_27_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_27_t(string name = "cap_pbc_csr_cfg_account_11_pg_27_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_27_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_27_t
    
class cap_pbc_csr_cfg_account_11_pg_26_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_26_t(string name = "cap_pbc_csr_cfg_account_11_pg_26_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_26_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_26_t
    
class cap_pbc_csr_cfg_account_11_pg_25_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_25_t(string name = "cap_pbc_csr_cfg_account_11_pg_25_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_25_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_25_t
    
class cap_pbc_csr_cfg_account_11_pg_24_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_24_t(string name = "cap_pbc_csr_cfg_account_11_pg_24_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_24_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_24_t
    
class cap_pbc_csr_cfg_account_11_pg_23_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_23_t(string name = "cap_pbc_csr_cfg_account_11_pg_23_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_23_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_23_t
    
class cap_pbc_csr_cfg_account_11_pg_22_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_22_t(string name = "cap_pbc_csr_cfg_account_11_pg_22_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_22_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_22_t
    
class cap_pbc_csr_cfg_account_11_pg_21_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_21_t(string name = "cap_pbc_csr_cfg_account_11_pg_21_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_21_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_21_t
    
class cap_pbc_csr_cfg_account_11_pg_20_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_20_t(string name = "cap_pbc_csr_cfg_account_11_pg_20_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_20_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_20_t
    
class cap_pbc_csr_cfg_account_11_pg_19_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_19_t(string name = "cap_pbc_csr_cfg_account_11_pg_19_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_19_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_19_t
    
class cap_pbc_csr_cfg_account_11_pg_18_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_18_t(string name = "cap_pbc_csr_cfg_account_11_pg_18_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_18_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_18_t
    
class cap_pbc_csr_cfg_account_11_pg_17_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_17_t(string name = "cap_pbc_csr_cfg_account_11_pg_17_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_17_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_17_t
    
class cap_pbc_csr_cfg_account_11_pg_16_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_16_t(string name = "cap_pbc_csr_cfg_account_11_pg_16_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_16_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_16_t
    
class cap_pbc_csr_cfg_account_11_pg_15_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_15_t(string name = "cap_pbc_csr_cfg_account_11_pg_15_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_15_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_15_t
    
class cap_pbc_csr_cfg_account_11_pg_14_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_14_t(string name = "cap_pbc_csr_cfg_account_11_pg_14_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_14_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_14_t
    
class cap_pbc_csr_cfg_account_11_pg_13_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_13_t(string name = "cap_pbc_csr_cfg_account_11_pg_13_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_13_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_13_t
    
class cap_pbc_csr_cfg_account_11_pg_12_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_12_t(string name = "cap_pbc_csr_cfg_account_11_pg_12_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_12_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_12_t
    
class cap_pbc_csr_cfg_account_11_pg_11_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_11_t(string name = "cap_pbc_csr_cfg_account_11_pg_11_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_11_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_11_t
    
class cap_pbc_csr_cfg_account_11_pg_10_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_10_t(string name = "cap_pbc_csr_cfg_account_11_pg_10_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_10_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_10_t
    
class cap_pbc_csr_cfg_account_11_pg_9_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_9_t(string name = "cap_pbc_csr_cfg_account_11_pg_9_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_9_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_9_t
    
class cap_pbc_csr_cfg_account_11_pg_8_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_8_t(string name = "cap_pbc_csr_cfg_account_11_pg_8_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_8_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_8_t
    
class cap_pbc_csr_cfg_account_11_pg_7_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_7_t(string name = "cap_pbc_csr_cfg_account_11_pg_7_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_7_t
    
class cap_pbc_csr_cfg_account_11_pg_6_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_6_t(string name = "cap_pbc_csr_cfg_account_11_pg_6_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_6_t
    
class cap_pbc_csr_cfg_account_11_pg_5_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_5_t(string name = "cap_pbc_csr_cfg_account_11_pg_5_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_5_t
    
class cap_pbc_csr_cfg_account_11_pg_4_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_4_t(string name = "cap_pbc_csr_cfg_account_11_pg_4_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_4_t
    
class cap_pbc_csr_cfg_account_11_pg_3_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_3_t(string name = "cap_pbc_csr_cfg_account_11_pg_3_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_3_t
    
class cap_pbc_csr_cfg_account_11_pg_2_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_2_t(string name = "cap_pbc_csr_cfg_account_11_pg_2_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_2_t
    
class cap_pbc_csr_cfg_account_11_pg_1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_1_t(string name = "cap_pbc_csr_cfg_account_11_pg_1_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_1_t
    
class cap_pbc_csr_cfg_account_11_pg_0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pg_0_t(string name = "cap_pbc_csr_cfg_account_11_pg_0_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pg_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_11_pg_0_t
    
class cap_pbc_csr_cfg_account_11_mtu_table_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_mtu_table_t(string name = "cap_pbc_csr_cfg_account_11_mtu_table_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_mtu_table_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 5 > pg0_cpp_int_t;
        pg0_cpp_int_t int_var__pg0;
        void pg0 (const cpp_int  & l__val);
        cpp_int pg0() const;
    
        typedef pu_cpp_int< 5 > pg1_cpp_int_t;
        pg1_cpp_int_t int_var__pg1;
        void pg1 (const cpp_int  & l__val);
        cpp_int pg1() const;
    
        typedef pu_cpp_int< 5 > pg2_cpp_int_t;
        pg2_cpp_int_t int_var__pg2;
        void pg2 (const cpp_int  & l__val);
        cpp_int pg2() const;
    
        typedef pu_cpp_int< 5 > pg3_cpp_int_t;
        pg3_cpp_int_t int_var__pg3;
        void pg3 (const cpp_int  & l__val);
        cpp_int pg3() const;
    
        typedef pu_cpp_int< 5 > pg4_cpp_int_t;
        pg4_cpp_int_t int_var__pg4;
        void pg4 (const cpp_int  & l__val);
        cpp_int pg4() const;
    
        typedef pu_cpp_int< 5 > pg5_cpp_int_t;
        pg5_cpp_int_t int_var__pg5;
        void pg5 (const cpp_int  & l__val);
        cpp_int pg5() const;
    
        typedef pu_cpp_int< 5 > pg6_cpp_int_t;
        pg6_cpp_int_t int_var__pg6;
        void pg6 (const cpp_int  & l__val);
        cpp_int pg6() const;
    
        typedef pu_cpp_int< 5 > pg7_cpp_int_t;
        pg7_cpp_int_t int_var__pg7;
        void pg7 (const cpp_int  & l__val);
        cpp_int pg7() const;
    
        typedef pu_cpp_int< 5 > pg8_cpp_int_t;
        pg8_cpp_int_t int_var__pg8;
        void pg8 (const cpp_int  & l__val);
        cpp_int pg8() const;
    
        typedef pu_cpp_int< 5 > pg9_cpp_int_t;
        pg9_cpp_int_t int_var__pg9;
        void pg9 (const cpp_int  & l__val);
        cpp_int pg9() const;
    
        typedef pu_cpp_int< 5 > pg10_cpp_int_t;
        pg10_cpp_int_t int_var__pg10;
        void pg10 (const cpp_int  & l__val);
        cpp_int pg10() const;
    
        typedef pu_cpp_int< 5 > pg11_cpp_int_t;
        pg11_cpp_int_t int_var__pg11;
        void pg11 (const cpp_int  & l__val);
        cpp_int pg11() const;
    
        typedef pu_cpp_int< 5 > pg12_cpp_int_t;
        pg12_cpp_int_t int_var__pg12;
        void pg12 (const cpp_int  & l__val);
        cpp_int pg12() const;
    
        typedef pu_cpp_int< 5 > pg13_cpp_int_t;
        pg13_cpp_int_t int_var__pg13;
        void pg13 (const cpp_int  & l__val);
        cpp_int pg13() const;
    
        typedef pu_cpp_int< 5 > pg14_cpp_int_t;
        pg14_cpp_int_t int_var__pg14;
        void pg14 (const cpp_int  & l__val);
        cpp_int pg14() const;
    
        typedef pu_cpp_int< 5 > pg15_cpp_int_t;
        pg15_cpp_int_t int_var__pg15;
        void pg15 (const cpp_int  & l__val);
        cpp_int pg15() const;
    
        typedef pu_cpp_int< 5 > pg16_cpp_int_t;
        pg16_cpp_int_t int_var__pg16;
        void pg16 (const cpp_int  & l__val);
        cpp_int pg16() const;
    
        typedef pu_cpp_int< 5 > pg17_cpp_int_t;
        pg17_cpp_int_t int_var__pg17;
        void pg17 (const cpp_int  & l__val);
        cpp_int pg17() const;
    
        typedef pu_cpp_int< 5 > pg18_cpp_int_t;
        pg18_cpp_int_t int_var__pg18;
        void pg18 (const cpp_int  & l__val);
        cpp_int pg18() const;
    
        typedef pu_cpp_int< 5 > pg19_cpp_int_t;
        pg19_cpp_int_t int_var__pg19;
        void pg19 (const cpp_int  & l__val);
        cpp_int pg19() const;
    
        typedef pu_cpp_int< 5 > pg20_cpp_int_t;
        pg20_cpp_int_t int_var__pg20;
        void pg20 (const cpp_int  & l__val);
        cpp_int pg20() const;
    
        typedef pu_cpp_int< 5 > pg21_cpp_int_t;
        pg21_cpp_int_t int_var__pg21;
        void pg21 (const cpp_int  & l__val);
        cpp_int pg21() const;
    
        typedef pu_cpp_int< 5 > pg22_cpp_int_t;
        pg22_cpp_int_t int_var__pg22;
        void pg22 (const cpp_int  & l__val);
        cpp_int pg22() const;
    
        typedef pu_cpp_int< 5 > pg23_cpp_int_t;
        pg23_cpp_int_t int_var__pg23;
        void pg23 (const cpp_int  & l__val);
        cpp_int pg23() const;
    
        typedef pu_cpp_int< 5 > pg24_cpp_int_t;
        pg24_cpp_int_t int_var__pg24;
        void pg24 (const cpp_int  & l__val);
        cpp_int pg24() const;
    
        typedef pu_cpp_int< 5 > pg25_cpp_int_t;
        pg25_cpp_int_t int_var__pg25;
        void pg25 (const cpp_int  & l__val);
        cpp_int pg25() const;
    
        typedef pu_cpp_int< 5 > pg26_cpp_int_t;
        pg26_cpp_int_t int_var__pg26;
        void pg26 (const cpp_int  & l__val);
        cpp_int pg26() const;
    
        typedef pu_cpp_int< 5 > pg27_cpp_int_t;
        pg27_cpp_int_t int_var__pg27;
        void pg27 (const cpp_int  & l__val);
        cpp_int pg27() const;
    
        typedef pu_cpp_int< 5 > pg28_cpp_int_t;
        pg28_cpp_int_t int_var__pg28;
        void pg28 (const cpp_int  & l__val);
        cpp_int pg28() const;
    
        typedef pu_cpp_int< 5 > pg29_cpp_int_t;
        pg29_cpp_int_t int_var__pg29;
        void pg29 (const cpp_int  & l__val);
        cpp_int pg29() const;
    
        typedef pu_cpp_int< 5 > pg30_cpp_int_t;
        pg30_cpp_int_t int_var__pg30;
        void pg30 (const cpp_int  & l__val);
        cpp_int pg30() const;
    
        typedef pu_cpp_int< 5 > pg31_cpp_int_t;
        pg31_cpp_int_t int_var__pg31;
        void pg31 (const cpp_int  & l__val);
        cpp_int pg31() const;
    
}; // cap_pbc_csr_cfg_account_11_mtu_table_t
    
class cap_pbc_csr_cfg_account_11_pause_timer_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_pause_timer_t(string name = "cap_pbc_csr_cfg_account_11_pause_timer_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_pause_timer_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > xoff_timeout_cpp_int_t;
        xoff_timeout_cpp_int_t int_var__xoff_timeout;
        void xoff_timeout (const cpp_int  & l__val);
        cpp_int xoff_timeout() const;
    
}; // cap_pbc_csr_cfg_account_11_pause_timer_t
    
class cap_pbc_csr_sta_account_11_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_sta_account_11_t(string name = "cap_pbc_csr_sta_account_11_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_sta_account_11_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 14 > occupancy_0_cpp_int_t;
        occupancy_0_cpp_int_t int_var__occupancy_0;
        void occupancy_0 (const cpp_int  & l__val);
        cpp_int occupancy_0() const;
    
        typedef pu_cpp_int< 14 > occupancy_1_cpp_int_t;
        occupancy_1_cpp_int_t int_var__occupancy_1;
        void occupancy_1 (const cpp_int  & l__val);
        cpp_int occupancy_1() const;
    
        typedef pu_cpp_int< 14 > occupancy_2_cpp_int_t;
        occupancy_2_cpp_int_t int_var__occupancy_2;
        void occupancy_2 (const cpp_int  & l__val);
        cpp_int occupancy_2() const;
    
        typedef pu_cpp_int< 14 > occupancy_3_cpp_int_t;
        occupancy_3_cpp_int_t int_var__occupancy_3;
        void occupancy_3 (const cpp_int  & l__val);
        cpp_int occupancy_3() const;
    
        typedef pu_cpp_int< 14 > occupancy_4_cpp_int_t;
        occupancy_4_cpp_int_t int_var__occupancy_4;
        void occupancy_4 (const cpp_int  & l__val);
        cpp_int occupancy_4() const;
    
        typedef pu_cpp_int< 14 > occupancy_5_cpp_int_t;
        occupancy_5_cpp_int_t int_var__occupancy_5;
        void occupancy_5 (const cpp_int  & l__val);
        cpp_int occupancy_5() const;
    
        typedef pu_cpp_int< 14 > occupancy_6_cpp_int_t;
        occupancy_6_cpp_int_t int_var__occupancy_6;
        void occupancy_6 (const cpp_int  & l__val);
        cpp_int occupancy_6() const;
    
        typedef pu_cpp_int< 14 > occupancy_7_cpp_int_t;
        occupancy_7_cpp_int_t int_var__occupancy_7;
        void occupancy_7 (const cpp_int  & l__val);
        cpp_int occupancy_7() const;
    
        typedef pu_cpp_int< 14 > occupancy_8_cpp_int_t;
        occupancy_8_cpp_int_t int_var__occupancy_8;
        void occupancy_8 (const cpp_int  & l__val);
        cpp_int occupancy_8() const;
    
        typedef pu_cpp_int< 14 > occupancy_9_cpp_int_t;
        occupancy_9_cpp_int_t int_var__occupancy_9;
        void occupancy_9 (const cpp_int  & l__val);
        cpp_int occupancy_9() const;
    
        typedef pu_cpp_int< 14 > occupancy_10_cpp_int_t;
        occupancy_10_cpp_int_t int_var__occupancy_10;
        void occupancy_10 (const cpp_int  & l__val);
        cpp_int occupancy_10() const;
    
        typedef pu_cpp_int< 14 > occupancy_11_cpp_int_t;
        occupancy_11_cpp_int_t int_var__occupancy_11;
        void occupancy_11 (const cpp_int  & l__val);
        cpp_int occupancy_11() const;
    
        typedef pu_cpp_int< 14 > occupancy_12_cpp_int_t;
        occupancy_12_cpp_int_t int_var__occupancy_12;
        void occupancy_12 (const cpp_int  & l__val);
        cpp_int occupancy_12() const;
    
        typedef pu_cpp_int< 14 > occupancy_13_cpp_int_t;
        occupancy_13_cpp_int_t int_var__occupancy_13;
        void occupancy_13 (const cpp_int  & l__val);
        cpp_int occupancy_13() const;
    
        typedef pu_cpp_int< 14 > occupancy_14_cpp_int_t;
        occupancy_14_cpp_int_t int_var__occupancy_14;
        void occupancy_14 (const cpp_int  & l__val);
        cpp_int occupancy_14() const;
    
        typedef pu_cpp_int< 14 > occupancy_15_cpp_int_t;
        occupancy_15_cpp_int_t int_var__occupancy_15;
        void occupancy_15 (const cpp_int  & l__val);
        cpp_int occupancy_15() const;
    
        typedef pu_cpp_int< 14 > occupancy_16_cpp_int_t;
        occupancy_16_cpp_int_t int_var__occupancy_16;
        void occupancy_16 (const cpp_int  & l__val);
        cpp_int occupancy_16() const;
    
        typedef pu_cpp_int< 14 > occupancy_17_cpp_int_t;
        occupancy_17_cpp_int_t int_var__occupancy_17;
        void occupancy_17 (const cpp_int  & l__val);
        cpp_int occupancy_17() const;
    
        typedef pu_cpp_int< 14 > occupancy_18_cpp_int_t;
        occupancy_18_cpp_int_t int_var__occupancy_18;
        void occupancy_18 (const cpp_int  & l__val);
        cpp_int occupancy_18() const;
    
        typedef pu_cpp_int< 14 > occupancy_19_cpp_int_t;
        occupancy_19_cpp_int_t int_var__occupancy_19;
        void occupancy_19 (const cpp_int  & l__val);
        cpp_int occupancy_19() const;
    
        typedef pu_cpp_int< 14 > occupancy_20_cpp_int_t;
        occupancy_20_cpp_int_t int_var__occupancy_20;
        void occupancy_20 (const cpp_int  & l__val);
        cpp_int occupancy_20() const;
    
        typedef pu_cpp_int< 14 > occupancy_21_cpp_int_t;
        occupancy_21_cpp_int_t int_var__occupancy_21;
        void occupancy_21 (const cpp_int  & l__val);
        cpp_int occupancy_21() const;
    
        typedef pu_cpp_int< 14 > occupancy_22_cpp_int_t;
        occupancy_22_cpp_int_t int_var__occupancy_22;
        void occupancy_22 (const cpp_int  & l__val);
        cpp_int occupancy_22() const;
    
        typedef pu_cpp_int< 14 > occupancy_23_cpp_int_t;
        occupancy_23_cpp_int_t int_var__occupancy_23;
        void occupancy_23 (const cpp_int  & l__val);
        cpp_int occupancy_23() const;
    
        typedef pu_cpp_int< 14 > occupancy_24_cpp_int_t;
        occupancy_24_cpp_int_t int_var__occupancy_24;
        void occupancy_24 (const cpp_int  & l__val);
        cpp_int occupancy_24() const;
    
        typedef pu_cpp_int< 14 > occupancy_25_cpp_int_t;
        occupancy_25_cpp_int_t int_var__occupancy_25;
        void occupancy_25 (const cpp_int  & l__val);
        cpp_int occupancy_25() const;
    
        typedef pu_cpp_int< 14 > occupancy_26_cpp_int_t;
        occupancy_26_cpp_int_t int_var__occupancy_26;
        void occupancy_26 (const cpp_int  & l__val);
        cpp_int occupancy_26() const;
    
        typedef pu_cpp_int< 14 > occupancy_27_cpp_int_t;
        occupancy_27_cpp_int_t int_var__occupancy_27;
        void occupancy_27 (const cpp_int  & l__val);
        cpp_int occupancy_27() const;
    
        typedef pu_cpp_int< 14 > occupancy_28_cpp_int_t;
        occupancy_28_cpp_int_t int_var__occupancy_28;
        void occupancy_28 (const cpp_int  & l__val);
        cpp_int occupancy_28() const;
    
        typedef pu_cpp_int< 14 > occupancy_29_cpp_int_t;
        occupancy_29_cpp_int_t int_var__occupancy_29;
        void occupancy_29 (const cpp_int  & l__val);
        cpp_int occupancy_29() const;
    
        typedef pu_cpp_int< 14 > occupancy_30_cpp_int_t;
        occupancy_30_cpp_int_t int_var__occupancy_30;
        void occupancy_30 (const cpp_int  & l__val);
        cpp_int occupancy_30() const;
    
        typedef pu_cpp_int< 14 > occupancy_31_cpp_int_t;
        occupancy_31_cpp_int_t int_var__occupancy_31;
        void occupancy_31 (const cpp_int  & l__val);
        cpp_int occupancy_31() const;
    
}; // cap_pbc_csr_sta_account_11_t
    
class cap_pbc_csr_cfg_account_11_tc_to_pg_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_11_tc_to_pg_t(string name = "cap_pbc_csr_cfg_account_11_tc_to_pg_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_11_tc_to_pg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 160 > table_cpp_int_t;
        table_cpp_int_t int_var__table;
        void table (const cpp_int  & l__val);
        cpp_int table() const;
    
}; // cap_pbc_csr_cfg_account_11_tc_to_pg_t
    
class cap_pbc_csr_cfg_write_control_11_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_write_control_11_t(string name = "cap_pbc_csr_cfg_write_control_11_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_write_control_11_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        enable_cpp_int_t int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 5 > cut_thru_cpp_int_t;
        cut_thru_cpp_int_t int_var__cut_thru;
        void cut_thru (const cpp_int  & l__val);
        cpp_int cut_thru() const;
    
        typedef pu_cpp_int< 1 > rewrite_cpp_int_t;
        rewrite_cpp_int_t int_var__rewrite;
        void rewrite (const cpp_int  & l__val);
        cpp_int rewrite() const;
    
        typedef pu_cpp_int< 3 > rate_limiter_cpp_int_t;
        rate_limiter_cpp_int_t int_var__rate_limiter;
        void rate_limiter (const cpp_int  & l__val);
        cpp_int rate_limiter() const;
    
        typedef pu_cpp_int< 1 > drop_on_error_cpp_int_t;
        drop_on_error_cpp_int_t int_var__drop_on_error;
        void drop_on_error (const cpp_int  & l__val);
        cpp_int drop_on_error() const;
    
}; // cap_pbc_csr_cfg_write_control_11_t
    
class cap_pbc_csr_cfg_oq_10_arb_l2_strict_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_10_arb_l2_strict_t(string name = "cap_pbc_csr_cfg_oq_10_arb_l2_strict_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_10_arb_l2_strict_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > priority_cpp_int_t;
        priority_cpp_int_t int_var__priority;
        void priority (const cpp_int  & l__val);
        cpp_int priority() const;
    
}; // cap_pbc_csr_cfg_oq_10_arb_l2_strict_t
    
class cap_pbc_csr_cfg_oq_10_arb_l2_selection_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_10_arb_l2_selection_t(string name = "cap_pbc_csr_cfg_oq_10_arb_l2_selection_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_10_arb_l2_selection_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > node_0_cpp_int_t;
        node_0_cpp_int_t int_var__node_0;
        void node_0 (const cpp_int  & l__val);
        cpp_int node_0() const;
    
        typedef pu_cpp_int< 16 > node_1_cpp_int_t;
        node_1_cpp_int_t int_var__node_1;
        void node_1 (const cpp_int  & l__val);
        cpp_int node_1() const;
    
        typedef pu_cpp_int< 16 > node_2_cpp_int_t;
        node_2_cpp_int_t int_var__node_2;
        void node_2 (const cpp_int  & l__val);
        cpp_int node_2() const;
    
        typedef pu_cpp_int< 16 > node_3_cpp_int_t;
        node_3_cpp_int_t int_var__node_3;
        void node_3 (const cpp_int  & l__val);
        cpp_int node_3() const;
    
}; // cap_pbc_csr_cfg_oq_10_arb_l2_selection_t
    
class cap_pbc_csr_cfg_oq_10_arb_l1_strict_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_10_arb_l1_strict_t(string name = "cap_pbc_csr_cfg_oq_10_arb_l1_strict_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_10_arb_l1_strict_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > priority_cpp_int_t;
        priority_cpp_int_t int_var__priority;
        void priority (const cpp_int  & l__val);
        cpp_int priority() const;
    
}; // cap_pbc_csr_cfg_oq_10_arb_l1_strict_t
    
class cap_pbc_csr_cfg_oq_10_arb_l1_selection_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_10_arb_l1_selection_t(string name = "cap_pbc_csr_cfg_oq_10_arb_l1_selection_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_10_arb_l1_selection_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > node_0_cpp_int_t;
        node_0_cpp_int_t int_var__node_0;
        void node_0 (const cpp_int  & l__val);
        cpp_int node_0() const;
    
        typedef pu_cpp_int< 32 > node_1_cpp_int_t;
        node_1_cpp_int_t int_var__node_1;
        void node_1 (const cpp_int  & l__val);
        cpp_int node_1() const;
    
        typedef pu_cpp_int< 32 > node_2_cpp_int_t;
        node_2_cpp_int_t int_var__node_2;
        void node_2 (const cpp_int  & l__val);
        cpp_int node_2() const;
    
        typedef pu_cpp_int< 32 > node_3_cpp_int_t;
        node_3_cpp_int_t int_var__node_3;
        void node_3 (const cpp_int  & l__val);
        cpp_int node_3() const;
    
        typedef pu_cpp_int< 32 > node_4_cpp_int_t;
        node_4_cpp_int_t int_var__node_4;
        void node_4 (const cpp_int  & l__val);
        cpp_int node_4() const;
    
        typedef pu_cpp_int< 32 > node_5_cpp_int_t;
        node_5_cpp_int_t int_var__node_5;
        void node_5 (const cpp_int  & l__val);
        cpp_int node_5() const;
    
        typedef pu_cpp_int< 32 > node_6_cpp_int_t;
        node_6_cpp_int_t int_var__node_6;
        void node_6 (const cpp_int  & l__val);
        cpp_int node_6() const;
    
        typedef pu_cpp_int< 32 > node_7_cpp_int_t;
        node_7_cpp_int_t int_var__node_7;
        void node_7 (const cpp_int  & l__val);
        cpp_int node_7() const;
    
        typedef pu_cpp_int< 32 > node_8_cpp_int_t;
        node_8_cpp_int_t int_var__node_8;
        void node_8 (const cpp_int  & l__val);
        cpp_int node_8() const;
    
        typedef pu_cpp_int< 32 > node_9_cpp_int_t;
        node_9_cpp_int_t int_var__node_9;
        void node_9 (const cpp_int  & l__val);
        cpp_int node_9() const;
    
        typedef pu_cpp_int< 32 > node_10_cpp_int_t;
        node_10_cpp_int_t int_var__node_10;
        void node_10 (const cpp_int  & l__val);
        cpp_int node_10() const;
    
        typedef pu_cpp_int< 32 > node_11_cpp_int_t;
        node_11_cpp_int_t int_var__node_11;
        void node_11 (const cpp_int  & l__val);
        cpp_int node_11() const;
    
        typedef pu_cpp_int< 32 > node_12_cpp_int_t;
        node_12_cpp_int_t int_var__node_12;
        void node_12 (const cpp_int  & l__val);
        cpp_int node_12() const;
    
        typedef pu_cpp_int< 32 > node_13_cpp_int_t;
        node_13_cpp_int_t int_var__node_13;
        void node_13 (const cpp_int  & l__val);
        cpp_int node_13() const;
    
        typedef pu_cpp_int< 32 > node_14_cpp_int_t;
        node_14_cpp_int_t int_var__node_14;
        void node_14 (const cpp_int  & l__val);
        cpp_int node_14() const;
    
        typedef pu_cpp_int< 32 > node_15_cpp_int_t;
        node_15_cpp_int_t int_var__node_15;
        void node_15 (const cpp_int  & l__val);
        cpp_int node_15() const;
    
}; // cap_pbc_csr_cfg_oq_10_arb_l1_selection_t
    
class cap_pbc_csr_cfg_oq_10_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_10_t(string name = "cap_pbc_csr_cfg_oq_10_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_10_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        enable_cpp_int_t int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        ecc_disable_det_cpp_int_t int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        ecc_disable_cor_cpp_int_t int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        dhs_eccbypass_cpp_int_t int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 3 > dhs_selection_cpp_int_t;
        dhs_selection_cpp_int_t int_var__dhs_selection;
        void dhs_selection (const cpp_int  & l__val);
        cpp_int dhs_selection() const;
    
        typedef pu_cpp_int< 1 > rewrite_enable_cpp_int_t;
        rewrite_enable_cpp_int_t int_var__rewrite_enable;
        void rewrite_enable (const cpp_int  & l__val);
        cpp_int rewrite_enable() const;
    
        typedef pu_cpp_int< 5 > ipg_bytes_cpp_int_t;
        ipg_bytes_cpp_int_t int_var__ipg_bytes;
        void ipg_bytes (const cpp_int  & l__val);
        cpp_int ipg_bytes() const;
    
}; // cap_pbc_csr_cfg_oq_10_t
    
class cap_pbc_csr_cfg_account_10_pg_31_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_31_t(string name = "cap_pbc_csr_cfg_account_10_pg_31_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_31_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_31_t
    
class cap_pbc_csr_cfg_account_10_pg_30_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_30_t(string name = "cap_pbc_csr_cfg_account_10_pg_30_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_30_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_30_t
    
class cap_pbc_csr_cfg_account_10_pg_29_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_29_t(string name = "cap_pbc_csr_cfg_account_10_pg_29_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_29_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_29_t
    
class cap_pbc_csr_cfg_account_10_pg_28_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_28_t(string name = "cap_pbc_csr_cfg_account_10_pg_28_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_28_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_28_t
    
class cap_pbc_csr_cfg_account_10_pg_27_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_27_t(string name = "cap_pbc_csr_cfg_account_10_pg_27_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_27_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_27_t
    
class cap_pbc_csr_cfg_account_10_pg_26_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_26_t(string name = "cap_pbc_csr_cfg_account_10_pg_26_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_26_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_26_t
    
class cap_pbc_csr_cfg_account_10_pg_25_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_25_t(string name = "cap_pbc_csr_cfg_account_10_pg_25_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_25_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_25_t
    
class cap_pbc_csr_cfg_account_10_pg_24_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_24_t(string name = "cap_pbc_csr_cfg_account_10_pg_24_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_24_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_24_t
    
class cap_pbc_csr_cfg_account_10_pg_23_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_23_t(string name = "cap_pbc_csr_cfg_account_10_pg_23_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_23_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_23_t
    
class cap_pbc_csr_cfg_account_10_pg_22_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_22_t(string name = "cap_pbc_csr_cfg_account_10_pg_22_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_22_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_22_t
    
class cap_pbc_csr_cfg_account_10_pg_21_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_21_t(string name = "cap_pbc_csr_cfg_account_10_pg_21_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_21_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_21_t
    
class cap_pbc_csr_cfg_account_10_pg_20_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_20_t(string name = "cap_pbc_csr_cfg_account_10_pg_20_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_20_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_20_t
    
class cap_pbc_csr_cfg_account_10_pg_19_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_19_t(string name = "cap_pbc_csr_cfg_account_10_pg_19_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_19_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_19_t
    
class cap_pbc_csr_cfg_account_10_pg_18_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_18_t(string name = "cap_pbc_csr_cfg_account_10_pg_18_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_18_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_18_t
    
class cap_pbc_csr_cfg_account_10_pg_17_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_17_t(string name = "cap_pbc_csr_cfg_account_10_pg_17_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_17_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_17_t
    
class cap_pbc_csr_cfg_account_10_pg_16_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_16_t(string name = "cap_pbc_csr_cfg_account_10_pg_16_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_16_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_16_t
    
class cap_pbc_csr_cfg_account_10_pg_15_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_15_t(string name = "cap_pbc_csr_cfg_account_10_pg_15_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_15_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_15_t
    
class cap_pbc_csr_cfg_account_10_pg_14_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_14_t(string name = "cap_pbc_csr_cfg_account_10_pg_14_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_14_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_14_t
    
class cap_pbc_csr_cfg_account_10_pg_13_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_13_t(string name = "cap_pbc_csr_cfg_account_10_pg_13_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_13_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_13_t
    
class cap_pbc_csr_cfg_account_10_pg_12_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_12_t(string name = "cap_pbc_csr_cfg_account_10_pg_12_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_12_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_12_t
    
class cap_pbc_csr_cfg_account_10_pg_11_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_11_t(string name = "cap_pbc_csr_cfg_account_10_pg_11_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_11_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_11_t
    
class cap_pbc_csr_cfg_account_10_pg_10_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_10_t(string name = "cap_pbc_csr_cfg_account_10_pg_10_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_10_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_10_t
    
class cap_pbc_csr_cfg_account_10_pg_9_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_9_t(string name = "cap_pbc_csr_cfg_account_10_pg_9_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_9_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_9_t
    
class cap_pbc_csr_cfg_account_10_pg_8_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_8_t(string name = "cap_pbc_csr_cfg_account_10_pg_8_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_8_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_8_t
    
class cap_pbc_csr_cfg_account_10_pg_7_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_7_t(string name = "cap_pbc_csr_cfg_account_10_pg_7_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_7_t
    
class cap_pbc_csr_cfg_account_10_pg_6_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_6_t(string name = "cap_pbc_csr_cfg_account_10_pg_6_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_6_t
    
class cap_pbc_csr_cfg_account_10_pg_5_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_5_t(string name = "cap_pbc_csr_cfg_account_10_pg_5_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_5_t
    
class cap_pbc_csr_cfg_account_10_pg_4_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_4_t(string name = "cap_pbc_csr_cfg_account_10_pg_4_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_4_t
    
class cap_pbc_csr_cfg_account_10_pg_3_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_3_t(string name = "cap_pbc_csr_cfg_account_10_pg_3_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_3_t
    
class cap_pbc_csr_cfg_account_10_pg_2_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_2_t(string name = "cap_pbc_csr_cfg_account_10_pg_2_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_2_t
    
class cap_pbc_csr_cfg_account_10_pg_1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_1_t(string name = "cap_pbc_csr_cfg_account_10_pg_1_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_1_t
    
class cap_pbc_csr_cfg_account_10_pg_0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pg_0_t(string name = "cap_pbc_csr_cfg_account_10_pg_0_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pg_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_10_pg_0_t
    
class cap_pbc_csr_cfg_account_10_mtu_table_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_mtu_table_t(string name = "cap_pbc_csr_cfg_account_10_mtu_table_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_mtu_table_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 5 > pg0_cpp_int_t;
        pg0_cpp_int_t int_var__pg0;
        void pg0 (const cpp_int  & l__val);
        cpp_int pg0() const;
    
        typedef pu_cpp_int< 5 > pg1_cpp_int_t;
        pg1_cpp_int_t int_var__pg1;
        void pg1 (const cpp_int  & l__val);
        cpp_int pg1() const;
    
        typedef pu_cpp_int< 5 > pg2_cpp_int_t;
        pg2_cpp_int_t int_var__pg2;
        void pg2 (const cpp_int  & l__val);
        cpp_int pg2() const;
    
        typedef pu_cpp_int< 5 > pg3_cpp_int_t;
        pg3_cpp_int_t int_var__pg3;
        void pg3 (const cpp_int  & l__val);
        cpp_int pg3() const;
    
        typedef pu_cpp_int< 5 > pg4_cpp_int_t;
        pg4_cpp_int_t int_var__pg4;
        void pg4 (const cpp_int  & l__val);
        cpp_int pg4() const;
    
        typedef pu_cpp_int< 5 > pg5_cpp_int_t;
        pg5_cpp_int_t int_var__pg5;
        void pg5 (const cpp_int  & l__val);
        cpp_int pg5() const;
    
        typedef pu_cpp_int< 5 > pg6_cpp_int_t;
        pg6_cpp_int_t int_var__pg6;
        void pg6 (const cpp_int  & l__val);
        cpp_int pg6() const;
    
        typedef pu_cpp_int< 5 > pg7_cpp_int_t;
        pg7_cpp_int_t int_var__pg7;
        void pg7 (const cpp_int  & l__val);
        cpp_int pg7() const;
    
        typedef pu_cpp_int< 5 > pg8_cpp_int_t;
        pg8_cpp_int_t int_var__pg8;
        void pg8 (const cpp_int  & l__val);
        cpp_int pg8() const;
    
        typedef pu_cpp_int< 5 > pg9_cpp_int_t;
        pg9_cpp_int_t int_var__pg9;
        void pg9 (const cpp_int  & l__val);
        cpp_int pg9() const;
    
        typedef pu_cpp_int< 5 > pg10_cpp_int_t;
        pg10_cpp_int_t int_var__pg10;
        void pg10 (const cpp_int  & l__val);
        cpp_int pg10() const;
    
        typedef pu_cpp_int< 5 > pg11_cpp_int_t;
        pg11_cpp_int_t int_var__pg11;
        void pg11 (const cpp_int  & l__val);
        cpp_int pg11() const;
    
        typedef pu_cpp_int< 5 > pg12_cpp_int_t;
        pg12_cpp_int_t int_var__pg12;
        void pg12 (const cpp_int  & l__val);
        cpp_int pg12() const;
    
        typedef pu_cpp_int< 5 > pg13_cpp_int_t;
        pg13_cpp_int_t int_var__pg13;
        void pg13 (const cpp_int  & l__val);
        cpp_int pg13() const;
    
        typedef pu_cpp_int< 5 > pg14_cpp_int_t;
        pg14_cpp_int_t int_var__pg14;
        void pg14 (const cpp_int  & l__val);
        cpp_int pg14() const;
    
        typedef pu_cpp_int< 5 > pg15_cpp_int_t;
        pg15_cpp_int_t int_var__pg15;
        void pg15 (const cpp_int  & l__val);
        cpp_int pg15() const;
    
        typedef pu_cpp_int< 5 > pg16_cpp_int_t;
        pg16_cpp_int_t int_var__pg16;
        void pg16 (const cpp_int  & l__val);
        cpp_int pg16() const;
    
        typedef pu_cpp_int< 5 > pg17_cpp_int_t;
        pg17_cpp_int_t int_var__pg17;
        void pg17 (const cpp_int  & l__val);
        cpp_int pg17() const;
    
        typedef pu_cpp_int< 5 > pg18_cpp_int_t;
        pg18_cpp_int_t int_var__pg18;
        void pg18 (const cpp_int  & l__val);
        cpp_int pg18() const;
    
        typedef pu_cpp_int< 5 > pg19_cpp_int_t;
        pg19_cpp_int_t int_var__pg19;
        void pg19 (const cpp_int  & l__val);
        cpp_int pg19() const;
    
        typedef pu_cpp_int< 5 > pg20_cpp_int_t;
        pg20_cpp_int_t int_var__pg20;
        void pg20 (const cpp_int  & l__val);
        cpp_int pg20() const;
    
        typedef pu_cpp_int< 5 > pg21_cpp_int_t;
        pg21_cpp_int_t int_var__pg21;
        void pg21 (const cpp_int  & l__val);
        cpp_int pg21() const;
    
        typedef pu_cpp_int< 5 > pg22_cpp_int_t;
        pg22_cpp_int_t int_var__pg22;
        void pg22 (const cpp_int  & l__val);
        cpp_int pg22() const;
    
        typedef pu_cpp_int< 5 > pg23_cpp_int_t;
        pg23_cpp_int_t int_var__pg23;
        void pg23 (const cpp_int  & l__val);
        cpp_int pg23() const;
    
        typedef pu_cpp_int< 5 > pg24_cpp_int_t;
        pg24_cpp_int_t int_var__pg24;
        void pg24 (const cpp_int  & l__val);
        cpp_int pg24() const;
    
        typedef pu_cpp_int< 5 > pg25_cpp_int_t;
        pg25_cpp_int_t int_var__pg25;
        void pg25 (const cpp_int  & l__val);
        cpp_int pg25() const;
    
        typedef pu_cpp_int< 5 > pg26_cpp_int_t;
        pg26_cpp_int_t int_var__pg26;
        void pg26 (const cpp_int  & l__val);
        cpp_int pg26() const;
    
        typedef pu_cpp_int< 5 > pg27_cpp_int_t;
        pg27_cpp_int_t int_var__pg27;
        void pg27 (const cpp_int  & l__val);
        cpp_int pg27() const;
    
        typedef pu_cpp_int< 5 > pg28_cpp_int_t;
        pg28_cpp_int_t int_var__pg28;
        void pg28 (const cpp_int  & l__val);
        cpp_int pg28() const;
    
        typedef pu_cpp_int< 5 > pg29_cpp_int_t;
        pg29_cpp_int_t int_var__pg29;
        void pg29 (const cpp_int  & l__val);
        cpp_int pg29() const;
    
        typedef pu_cpp_int< 5 > pg30_cpp_int_t;
        pg30_cpp_int_t int_var__pg30;
        void pg30 (const cpp_int  & l__val);
        cpp_int pg30() const;
    
        typedef pu_cpp_int< 5 > pg31_cpp_int_t;
        pg31_cpp_int_t int_var__pg31;
        void pg31 (const cpp_int  & l__val);
        cpp_int pg31() const;
    
}; // cap_pbc_csr_cfg_account_10_mtu_table_t
    
class cap_pbc_csr_cfg_account_10_pause_timer_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_pause_timer_t(string name = "cap_pbc_csr_cfg_account_10_pause_timer_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_pause_timer_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > xoff_timeout_cpp_int_t;
        xoff_timeout_cpp_int_t int_var__xoff_timeout;
        void xoff_timeout (const cpp_int  & l__val);
        cpp_int xoff_timeout() const;
    
}; // cap_pbc_csr_cfg_account_10_pause_timer_t
    
class cap_pbc_csr_sta_account_10_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_sta_account_10_t(string name = "cap_pbc_csr_sta_account_10_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_sta_account_10_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 14 > occupancy_0_cpp_int_t;
        occupancy_0_cpp_int_t int_var__occupancy_0;
        void occupancy_0 (const cpp_int  & l__val);
        cpp_int occupancy_0() const;
    
        typedef pu_cpp_int< 14 > occupancy_1_cpp_int_t;
        occupancy_1_cpp_int_t int_var__occupancy_1;
        void occupancy_1 (const cpp_int  & l__val);
        cpp_int occupancy_1() const;
    
        typedef pu_cpp_int< 14 > occupancy_2_cpp_int_t;
        occupancy_2_cpp_int_t int_var__occupancy_2;
        void occupancy_2 (const cpp_int  & l__val);
        cpp_int occupancy_2() const;
    
        typedef pu_cpp_int< 14 > occupancy_3_cpp_int_t;
        occupancy_3_cpp_int_t int_var__occupancy_3;
        void occupancy_3 (const cpp_int  & l__val);
        cpp_int occupancy_3() const;
    
        typedef pu_cpp_int< 14 > occupancy_4_cpp_int_t;
        occupancy_4_cpp_int_t int_var__occupancy_4;
        void occupancy_4 (const cpp_int  & l__val);
        cpp_int occupancy_4() const;
    
        typedef pu_cpp_int< 14 > occupancy_5_cpp_int_t;
        occupancy_5_cpp_int_t int_var__occupancy_5;
        void occupancy_5 (const cpp_int  & l__val);
        cpp_int occupancy_5() const;
    
        typedef pu_cpp_int< 14 > occupancy_6_cpp_int_t;
        occupancy_6_cpp_int_t int_var__occupancy_6;
        void occupancy_6 (const cpp_int  & l__val);
        cpp_int occupancy_6() const;
    
        typedef pu_cpp_int< 14 > occupancy_7_cpp_int_t;
        occupancy_7_cpp_int_t int_var__occupancy_7;
        void occupancy_7 (const cpp_int  & l__val);
        cpp_int occupancy_7() const;
    
        typedef pu_cpp_int< 14 > occupancy_8_cpp_int_t;
        occupancy_8_cpp_int_t int_var__occupancy_8;
        void occupancy_8 (const cpp_int  & l__val);
        cpp_int occupancy_8() const;
    
        typedef pu_cpp_int< 14 > occupancy_9_cpp_int_t;
        occupancy_9_cpp_int_t int_var__occupancy_9;
        void occupancy_9 (const cpp_int  & l__val);
        cpp_int occupancy_9() const;
    
        typedef pu_cpp_int< 14 > occupancy_10_cpp_int_t;
        occupancy_10_cpp_int_t int_var__occupancy_10;
        void occupancy_10 (const cpp_int  & l__val);
        cpp_int occupancy_10() const;
    
        typedef pu_cpp_int< 14 > occupancy_11_cpp_int_t;
        occupancy_11_cpp_int_t int_var__occupancy_11;
        void occupancy_11 (const cpp_int  & l__val);
        cpp_int occupancy_11() const;
    
        typedef pu_cpp_int< 14 > occupancy_12_cpp_int_t;
        occupancy_12_cpp_int_t int_var__occupancy_12;
        void occupancy_12 (const cpp_int  & l__val);
        cpp_int occupancy_12() const;
    
        typedef pu_cpp_int< 14 > occupancy_13_cpp_int_t;
        occupancy_13_cpp_int_t int_var__occupancy_13;
        void occupancy_13 (const cpp_int  & l__val);
        cpp_int occupancy_13() const;
    
        typedef pu_cpp_int< 14 > occupancy_14_cpp_int_t;
        occupancy_14_cpp_int_t int_var__occupancy_14;
        void occupancy_14 (const cpp_int  & l__val);
        cpp_int occupancy_14() const;
    
        typedef pu_cpp_int< 14 > occupancy_15_cpp_int_t;
        occupancy_15_cpp_int_t int_var__occupancy_15;
        void occupancy_15 (const cpp_int  & l__val);
        cpp_int occupancy_15() const;
    
        typedef pu_cpp_int< 14 > occupancy_16_cpp_int_t;
        occupancy_16_cpp_int_t int_var__occupancy_16;
        void occupancy_16 (const cpp_int  & l__val);
        cpp_int occupancy_16() const;
    
        typedef pu_cpp_int< 14 > occupancy_17_cpp_int_t;
        occupancy_17_cpp_int_t int_var__occupancy_17;
        void occupancy_17 (const cpp_int  & l__val);
        cpp_int occupancy_17() const;
    
        typedef pu_cpp_int< 14 > occupancy_18_cpp_int_t;
        occupancy_18_cpp_int_t int_var__occupancy_18;
        void occupancy_18 (const cpp_int  & l__val);
        cpp_int occupancy_18() const;
    
        typedef pu_cpp_int< 14 > occupancy_19_cpp_int_t;
        occupancy_19_cpp_int_t int_var__occupancy_19;
        void occupancy_19 (const cpp_int  & l__val);
        cpp_int occupancy_19() const;
    
        typedef pu_cpp_int< 14 > occupancy_20_cpp_int_t;
        occupancy_20_cpp_int_t int_var__occupancy_20;
        void occupancy_20 (const cpp_int  & l__val);
        cpp_int occupancy_20() const;
    
        typedef pu_cpp_int< 14 > occupancy_21_cpp_int_t;
        occupancy_21_cpp_int_t int_var__occupancy_21;
        void occupancy_21 (const cpp_int  & l__val);
        cpp_int occupancy_21() const;
    
        typedef pu_cpp_int< 14 > occupancy_22_cpp_int_t;
        occupancy_22_cpp_int_t int_var__occupancy_22;
        void occupancy_22 (const cpp_int  & l__val);
        cpp_int occupancy_22() const;
    
        typedef pu_cpp_int< 14 > occupancy_23_cpp_int_t;
        occupancy_23_cpp_int_t int_var__occupancy_23;
        void occupancy_23 (const cpp_int  & l__val);
        cpp_int occupancy_23() const;
    
        typedef pu_cpp_int< 14 > occupancy_24_cpp_int_t;
        occupancy_24_cpp_int_t int_var__occupancy_24;
        void occupancy_24 (const cpp_int  & l__val);
        cpp_int occupancy_24() const;
    
        typedef pu_cpp_int< 14 > occupancy_25_cpp_int_t;
        occupancy_25_cpp_int_t int_var__occupancy_25;
        void occupancy_25 (const cpp_int  & l__val);
        cpp_int occupancy_25() const;
    
        typedef pu_cpp_int< 14 > occupancy_26_cpp_int_t;
        occupancy_26_cpp_int_t int_var__occupancy_26;
        void occupancy_26 (const cpp_int  & l__val);
        cpp_int occupancy_26() const;
    
        typedef pu_cpp_int< 14 > occupancy_27_cpp_int_t;
        occupancy_27_cpp_int_t int_var__occupancy_27;
        void occupancy_27 (const cpp_int  & l__val);
        cpp_int occupancy_27() const;
    
        typedef pu_cpp_int< 14 > occupancy_28_cpp_int_t;
        occupancy_28_cpp_int_t int_var__occupancy_28;
        void occupancy_28 (const cpp_int  & l__val);
        cpp_int occupancy_28() const;
    
        typedef pu_cpp_int< 14 > occupancy_29_cpp_int_t;
        occupancy_29_cpp_int_t int_var__occupancy_29;
        void occupancy_29 (const cpp_int  & l__val);
        cpp_int occupancy_29() const;
    
        typedef pu_cpp_int< 14 > occupancy_30_cpp_int_t;
        occupancy_30_cpp_int_t int_var__occupancy_30;
        void occupancy_30 (const cpp_int  & l__val);
        cpp_int occupancy_30() const;
    
        typedef pu_cpp_int< 14 > occupancy_31_cpp_int_t;
        occupancy_31_cpp_int_t int_var__occupancy_31;
        void occupancy_31 (const cpp_int  & l__val);
        cpp_int occupancy_31() const;
    
}; // cap_pbc_csr_sta_account_10_t
    
class cap_pbc_csr_cfg_account_10_tc_to_pg_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_10_tc_to_pg_t(string name = "cap_pbc_csr_cfg_account_10_tc_to_pg_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_10_tc_to_pg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 160 > table_cpp_int_t;
        table_cpp_int_t int_var__table;
        void table (const cpp_int  & l__val);
        cpp_int table() const;
    
}; // cap_pbc_csr_cfg_account_10_tc_to_pg_t
    
class cap_pbc_csr_cfg_write_control_10_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_write_control_10_t(string name = "cap_pbc_csr_cfg_write_control_10_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_write_control_10_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        enable_cpp_int_t int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 5 > cut_thru_cpp_int_t;
        cut_thru_cpp_int_t int_var__cut_thru;
        void cut_thru (const cpp_int  & l__val);
        cpp_int cut_thru() const;
    
        typedef pu_cpp_int< 1 > rewrite_cpp_int_t;
        rewrite_cpp_int_t int_var__rewrite;
        void rewrite (const cpp_int  & l__val);
        cpp_int rewrite() const;
    
        typedef pu_cpp_int< 3 > rate_limiter_cpp_int_t;
        rate_limiter_cpp_int_t int_var__rate_limiter;
        void rate_limiter (const cpp_int  & l__val);
        cpp_int rate_limiter() const;
    
        typedef pu_cpp_int< 1 > drop_on_error_cpp_int_t;
        drop_on_error_cpp_int_t int_var__drop_on_error;
        void drop_on_error (const cpp_int  & l__val);
        cpp_int drop_on_error() const;
    
}; // cap_pbc_csr_cfg_write_control_10_t
    
class cap_pbc_csr_cfg_oq_9_arb_l2_strict_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_9_arb_l2_strict_t(string name = "cap_pbc_csr_cfg_oq_9_arb_l2_strict_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_9_arb_l2_strict_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > priority_cpp_int_t;
        priority_cpp_int_t int_var__priority;
        void priority (const cpp_int  & l__val);
        cpp_int priority() const;
    
}; // cap_pbc_csr_cfg_oq_9_arb_l2_strict_t
    
class cap_pbc_csr_cfg_oq_9_arb_l2_selection_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_9_arb_l2_selection_t(string name = "cap_pbc_csr_cfg_oq_9_arb_l2_selection_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_9_arb_l2_selection_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > node_0_cpp_int_t;
        node_0_cpp_int_t int_var__node_0;
        void node_0 (const cpp_int  & l__val);
        cpp_int node_0() const;
    
        typedef pu_cpp_int< 16 > node_1_cpp_int_t;
        node_1_cpp_int_t int_var__node_1;
        void node_1 (const cpp_int  & l__val);
        cpp_int node_1() const;
    
        typedef pu_cpp_int< 16 > node_2_cpp_int_t;
        node_2_cpp_int_t int_var__node_2;
        void node_2 (const cpp_int  & l__val);
        cpp_int node_2() const;
    
        typedef pu_cpp_int< 16 > node_3_cpp_int_t;
        node_3_cpp_int_t int_var__node_3;
        void node_3 (const cpp_int  & l__val);
        cpp_int node_3() const;
    
}; // cap_pbc_csr_cfg_oq_9_arb_l2_selection_t
    
class cap_pbc_csr_cfg_oq_9_arb_l1_strict_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_9_arb_l1_strict_t(string name = "cap_pbc_csr_cfg_oq_9_arb_l1_strict_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_9_arb_l1_strict_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > priority_cpp_int_t;
        priority_cpp_int_t int_var__priority;
        void priority (const cpp_int  & l__val);
        cpp_int priority() const;
    
}; // cap_pbc_csr_cfg_oq_9_arb_l1_strict_t
    
class cap_pbc_csr_cfg_oq_9_arb_l1_selection_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_9_arb_l1_selection_t(string name = "cap_pbc_csr_cfg_oq_9_arb_l1_selection_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_9_arb_l1_selection_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > node_0_cpp_int_t;
        node_0_cpp_int_t int_var__node_0;
        void node_0 (const cpp_int  & l__val);
        cpp_int node_0() const;
    
        typedef pu_cpp_int< 32 > node_1_cpp_int_t;
        node_1_cpp_int_t int_var__node_1;
        void node_1 (const cpp_int  & l__val);
        cpp_int node_1() const;
    
        typedef pu_cpp_int< 32 > node_2_cpp_int_t;
        node_2_cpp_int_t int_var__node_2;
        void node_2 (const cpp_int  & l__val);
        cpp_int node_2() const;
    
        typedef pu_cpp_int< 32 > node_3_cpp_int_t;
        node_3_cpp_int_t int_var__node_3;
        void node_3 (const cpp_int  & l__val);
        cpp_int node_3() const;
    
        typedef pu_cpp_int< 32 > node_4_cpp_int_t;
        node_4_cpp_int_t int_var__node_4;
        void node_4 (const cpp_int  & l__val);
        cpp_int node_4() const;
    
        typedef pu_cpp_int< 32 > node_5_cpp_int_t;
        node_5_cpp_int_t int_var__node_5;
        void node_5 (const cpp_int  & l__val);
        cpp_int node_5() const;
    
        typedef pu_cpp_int< 32 > node_6_cpp_int_t;
        node_6_cpp_int_t int_var__node_6;
        void node_6 (const cpp_int  & l__val);
        cpp_int node_6() const;
    
        typedef pu_cpp_int< 32 > node_7_cpp_int_t;
        node_7_cpp_int_t int_var__node_7;
        void node_7 (const cpp_int  & l__val);
        cpp_int node_7() const;
    
        typedef pu_cpp_int< 32 > node_8_cpp_int_t;
        node_8_cpp_int_t int_var__node_8;
        void node_8 (const cpp_int  & l__val);
        cpp_int node_8() const;
    
        typedef pu_cpp_int< 32 > node_9_cpp_int_t;
        node_9_cpp_int_t int_var__node_9;
        void node_9 (const cpp_int  & l__val);
        cpp_int node_9() const;
    
        typedef pu_cpp_int< 32 > node_10_cpp_int_t;
        node_10_cpp_int_t int_var__node_10;
        void node_10 (const cpp_int  & l__val);
        cpp_int node_10() const;
    
        typedef pu_cpp_int< 32 > node_11_cpp_int_t;
        node_11_cpp_int_t int_var__node_11;
        void node_11 (const cpp_int  & l__val);
        cpp_int node_11() const;
    
        typedef pu_cpp_int< 32 > node_12_cpp_int_t;
        node_12_cpp_int_t int_var__node_12;
        void node_12 (const cpp_int  & l__val);
        cpp_int node_12() const;
    
        typedef pu_cpp_int< 32 > node_13_cpp_int_t;
        node_13_cpp_int_t int_var__node_13;
        void node_13 (const cpp_int  & l__val);
        cpp_int node_13() const;
    
        typedef pu_cpp_int< 32 > node_14_cpp_int_t;
        node_14_cpp_int_t int_var__node_14;
        void node_14 (const cpp_int  & l__val);
        cpp_int node_14() const;
    
        typedef pu_cpp_int< 32 > node_15_cpp_int_t;
        node_15_cpp_int_t int_var__node_15;
        void node_15 (const cpp_int  & l__val);
        cpp_int node_15() const;
    
}; // cap_pbc_csr_cfg_oq_9_arb_l1_selection_t
    
class cap_pbc_csr_cfg_oq_9_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_9_t(string name = "cap_pbc_csr_cfg_oq_9_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_9_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        enable_cpp_int_t int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        ecc_disable_det_cpp_int_t int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        ecc_disable_cor_cpp_int_t int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        dhs_eccbypass_cpp_int_t int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 1 > rewrite_enable_cpp_int_t;
        rewrite_enable_cpp_int_t int_var__rewrite_enable;
        void rewrite_enable (const cpp_int  & l__val);
        cpp_int rewrite_enable() const;
    
        typedef pu_cpp_int< 5 > ipg_bytes_cpp_int_t;
        ipg_bytes_cpp_int_t int_var__ipg_bytes;
        void ipg_bytes (const cpp_int  & l__val);
        cpp_int ipg_bytes() const;
    
}; // cap_pbc_csr_cfg_oq_9_t
    
class cap_pbc_csr_cfg_account_9_pg_15_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_9_pg_15_t(string name = "cap_pbc_csr_cfg_account_9_pg_15_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_9_pg_15_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_9_pg_15_t
    
class cap_pbc_csr_cfg_account_9_pg_14_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_9_pg_14_t(string name = "cap_pbc_csr_cfg_account_9_pg_14_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_9_pg_14_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_9_pg_14_t
    
class cap_pbc_csr_cfg_account_9_pg_13_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_9_pg_13_t(string name = "cap_pbc_csr_cfg_account_9_pg_13_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_9_pg_13_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_9_pg_13_t
    
class cap_pbc_csr_cfg_account_9_pg_12_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_9_pg_12_t(string name = "cap_pbc_csr_cfg_account_9_pg_12_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_9_pg_12_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_9_pg_12_t
    
class cap_pbc_csr_cfg_account_9_pg_11_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_9_pg_11_t(string name = "cap_pbc_csr_cfg_account_9_pg_11_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_9_pg_11_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_9_pg_11_t
    
class cap_pbc_csr_cfg_account_9_pg_10_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_9_pg_10_t(string name = "cap_pbc_csr_cfg_account_9_pg_10_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_9_pg_10_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_9_pg_10_t
    
class cap_pbc_csr_cfg_account_9_pg_9_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_9_pg_9_t(string name = "cap_pbc_csr_cfg_account_9_pg_9_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_9_pg_9_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_9_pg_9_t
    
class cap_pbc_csr_cfg_account_9_pg_8_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_9_pg_8_t(string name = "cap_pbc_csr_cfg_account_9_pg_8_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_9_pg_8_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_9_pg_8_t
    
class cap_pbc_csr_cfg_account_9_pg_7_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_9_pg_7_t(string name = "cap_pbc_csr_cfg_account_9_pg_7_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_9_pg_7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_9_pg_7_t
    
class cap_pbc_csr_cfg_account_9_pg_6_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_9_pg_6_t(string name = "cap_pbc_csr_cfg_account_9_pg_6_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_9_pg_6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_9_pg_6_t
    
class cap_pbc_csr_cfg_account_9_pg_5_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_9_pg_5_t(string name = "cap_pbc_csr_cfg_account_9_pg_5_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_9_pg_5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_9_pg_5_t
    
class cap_pbc_csr_cfg_account_9_pg_4_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_9_pg_4_t(string name = "cap_pbc_csr_cfg_account_9_pg_4_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_9_pg_4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_9_pg_4_t
    
class cap_pbc_csr_cfg_account_9_pg_3_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_9_pg_3_t(string name = "cap_pbc_csr_cfg_account_9_pg_3_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_9_pg_3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_9_pg_3_t
    
class cap_pbc_csr_cfg_account_9_pg_2_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_9_pg_2_t(string name = "cap_pbc_csr_cfg_account_9_pg_2_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_9_pg_2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_9_pg_2_t
    
class cap_pbc_csr_cfg_account_9_pg_1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_9_pg_1_t(string name = "cap_pbc_csr_cfg_account_9_pg_1_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_9_pg_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_9_pg_1_t
    
class cap_pbc_csr_cfg_account_9_pg_0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_9_pg_0_t(string name = "cap_pbc_csr_cfg_account_9_pg_0_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_9_pg_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_9_pg_0_t
    
class cap_pbc_csr_cfg_account_9_mtu_table_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_9_mtu_table_t(string name = "cap_pbc_csr_cfg_account_9_mtu_table_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_9_mtu_table_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 5 > pg0_cpp_int_t;
        pg0_cpp_int_t int_var__pg0;
        void pg0 (const cpp_int  & l__val);
        cpp_int pg0() const;
    
        typedef pu_cpp_int< 5 > pg1_cpp_int_t;
        pg1_cpp_int_t int_var__pg1;
        void pg1 (const cpp_int  & l__val);
        cpp_int pg1() const;
    
        typedef pu_cpp_int< 5 > pg2_cpp_int_t;
        pg2_cpp_int_t int_var__pg2;
        void pg2 (const cpp_int  & l__val);
        cpp_int pg2() const;
    
        typedef pu_cpp_int< 5 > pg3_cpp_int_t;
        pg3_cpp_int_t int_var__pg3;
        void pg3 (const cpp_int  & l__val);
        cpp_int pg3() const;
    
        typedef pu_cpp_int< 5 > pg4_cpp_int_t;
        pg4_cpp_int_t int_var__pg4;
        void pg4 (const cpp_int  & l__val);
        cpp_int pg4() const;
    
        typedef pu_cpp_int< 5 > pg5_cpp_int_t;
        pg5_cpp_int_t int_var__pg5;
        void pg5 (const cpp_int  & l__val);
        cpp_int pg5() const;
    
        typedef pu_cpp_int< 5 > pg6_cpp_int_t;
        pg6_cpp_int_t int_var__pg6;
        void pg6 (const cpp_int  & l__val);
        cpp_int pg6() const;
    
        typedef pu_cpp_int< 5 > pg7_cpp_int_t;
        pg7_cpp_int_t int_var__pg7;
        void pg7 (const cpp_int  & l__val);
        cpp_int pg7() const;
    
        typedef pu_cpp_int< 5 > pg8_cpp_int_t;
        pg8_cpp_int_t int_var__pg8;
        void pg8 (const cpp_int  & l__val);
        cpp_int pg8() const;
    
        typedef pu_cpp_int< 5 > pg9_cpp_int_t;
        pg9_cpp_int_t int_var__pg9;
        void pg9 (const cpp_int  & l__val);
        cpp_int pg9() const;
    
        typedef pu_cpp_int< 5 > pg10_cpp_int_t;
        pg10_cpp_int_t int_var__pg10;
        void pg10 (const cpp_int  & l__val);
        cpp_int pg10() const;
    
        typedef pu_cpp_int< 5 > pg11_cpp_int_t;
        pg11_cpp_int_t int_var__pg11;
        void pg11 (const cpp_int  & l__val);
        cpp_int pg11() const;
    
        typedef pu_cpp_int< 5 > pg12_cpp_int_t;
        pg12_cpp_int_t int_var__pg12;
        void pg12 (const cpp_int  & l__val);
        cpp_int pg12() const;
    
        typedef pu_cpp_int< 5 > pg13_cpp_int_t;
        pg13_cpp_int_t int_var__pg13;
        void pg13 (const cpp_int  & l__val);
        cpp_int pg13() const;
    
        typedef pu_cpp_int< 5 > pg14_cpp_int_t;
        pg14_cpp_int_t int_var__pg14;
        void pg14 (const cpp_int  & l__val);
        cpp_int pg14() const;
    
        typedef pu_cpp_int< 5 > pg15_cpp_int_t;
        pg15_cpp_int_t int_var__pg15;
        void pg15 (const cpp_int  & l__val);
        cpp_int pg15() const;
    
}; // cap_pbc_csr_cfg_account_9_mtu_table_t
    
class cap_pbc_csr_cfg_account_9_pause_timer_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_9_pause_timer_t(string name = "cap_pbc_csr_cfg_account_9_pause_timer_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_9_pause_timer_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > xoff_timeout_cpp_int_t;
        xoff_timeout_cpp_int_t int_var__xoff_timeout;
        void xoff_timeout (const cpp_int  & l__val);
        cpp_int xoff_timeout() const;
    
}; // cap_pbc_csr_cfg_account_9_pause_timer_t
    
class cap_pbc_csr_sta_account_9_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_sta_account_9_t(string name = "cap_pbc_csr_sta_account_9_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_sta_account_9_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 14 > occupancy_0_cpp_int_t;
        occupancy_0_cpp_int_t int_var__occupancy_0;
        void occupancy_0 (const cpp_int  & l__val);
        cpp_int occupancy_0() const;
    
        typedef pu_cpp_int< 14 > occupancy_1_cpp_int_t;
        occupancy_1_cpp_int_t int_var__occupancy_1;
        void occupancy_1 (const cpp_int  & l__val);
        cpp_int occupancy_1() const;
    
        typedef pu_cpp_int< 14 > occupancy_2_cpp_int_t;
        occupancy_2_cpp_int_t int_var__occupancy_2;
        void occupancy_2 (const cpp_int  & l__val);
        cpp_int occupancy_2() const;
    
        typedef pu_cpp_int< 14 > occupancy_3_cpp_int_t;
        occupancy_3_cpp_int_t int_var__occupancy_3;
        void occupancy_3 (const cpp_int  & l__val);
        cpp_int occupancy_3() const;
    
        typedef pu_cpp_int< 14 > occupancy_4_cpp_int_t;
        occupancy_4_cpp_int_t int_var__occupancy_4;
        void occupancy_4 (const cpp_int  & l__val);
        cpp_int occupancy_4() const;
    
        typedef pu_cpp_int< 14 > occupancy_5_cpp_int_t;
        occupancy_5_cpp_int_t int_var__occupancy_5;
        void occupancy_5 (const cpp_int  & l__val);
        cpp_int occupancy_5() const;
    
        typedef pu_cpp_int< 14 > occupancy_6_cpp_int_t;
        occupancy_6_cpp_int_t int_var__occupancy_6;
        void occupancy_6 (const cpp_int  & l__val);
        cpp_int occupancy_6() const;
    
        typedef pu_cpp_int< 14 > occupancy_7_cpp_int_t;
        occupancy_7_cpp_int_t int_var__occupancy_7;
        void occupancy_7 (const cpp_int  & l__val);
        cpp_int occupancy_7() const;
    
        typedef pu_cpp_int< 14 > occupancy_8_cpp_int_t;
        occupancy_8_cpp_int_t int_var__occupancy_8;
        void occupancy_8 (const cpp_int  & l__val);
        cpp_int occupancy_8() const;
    
        typedef pu_cpp_int< 14 > occupancy_9_cpp_int_t;
        occupancy_9_cpp_int_t int_var__occupancy_9;
        void occupancy_9 (const cpp_int  & l__val);
        cpp_int occupancy_9() const;
    
        typedef pu_cpp_int< 14 > occupancy_10_cpp_int_t;
        occupancy_10_cpp_int_t int_var__occupancy_10;
        void occupancy_10 (const cpp_int  & l__val);
        cpp_int occupancy_10() const;
    
        typedef pu_cpp_int< 14 > occupancy_11_cpp_int_t;
        occupancy_11_cpp_int_t int_var__occupancy_11;
        void occupancy_11 (const cpp_int  & l__val);
        cpp_int occupancy_11() const;
    
        typedef pu_cpp_int< 14 > occupancy_12_cpp_int_t;
        occupancy_12_cpp_int_t int_var__occupancy_12;
        void occupancy_12 (const cpp_int  & l__val);
        cpp_int occupancy_12() const;
    
        typedef pu_cpp_int< 14 > occupancy_13_cpp_int_t;
        occupancy_13_cpp_int_t int_var__occupancy_13;
        void occupancy_13 (const cpp_int  & l__val);
        cpp_int occupancy_13() const;
    
        typedef pu_cpp_int< 14 > occupancy_14_cpp_int_t;
        occupancy_14_cpp_int_t int_var__occupancy_14;
        void occupancy_14 (const cpp_int  & l__val);
        cpp_int occupancy_14() const;
    
        typedef pu_cpp_int< 14 > occupancy_15_cpp_int_t;
        occupancy_15_cpp_int_t int_var__occupancy_15;
        void occupancy_15 (const cpp_int  & l__val);
        cpp_int occupancy_15() const;
    
}; // cap_pbc_csr_sta_account_9_t
    
class cap_pbc_csr_cfg_account_9_tc_to_pg_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_9_tc_to_pg_t(string name = "cap_pbc_csr_cfg_account_9_tc_to_pg_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_9_tc_to_pg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 64 > table_cpp_int_t;
        table_cpp_int_t int_var__table;
        void table (const cpp_int  & l__val);
        cpp_int table() const;
    
}; // cap_pbc_csr_cfg_account_9_tc_to_pg_t
    
class cap_pbc_csr_cfg_write_control_9_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_write_control_9_t(string name = "cap_pbc_csr_cfg_write_control_9_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_write_control_9_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        enable_cpp_int_t int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 5 > cut_thru_cpp_int_t;
        cut_thru_cpp_int_t int_var__cut_thru;
        void cut_thru (const cpp_int  & l__val);
        cpp_int cut_thru() const;
    
        typedef pu_cpp_int< 1 > rewrite_cpp_int_t;
        rewrite_cpp_int_t int_var__rewrite;
        void rewrite (const cpp_int  & l__val);
        cpp_int rewrite() const;
    
        typedef pu_cpp_int< 3 > rate_limiter_cpp_int_t;
        rate_limiter_cpp_int_t int_var__rate_limiter;
        void rate_limiter (const cpp_int  & l__val);
        cpp_int rate_limiter() const;
    
        typedef pu_cpp_int< 1 > drop_on_error_cpp_int_t;
        drop_on_error_cpp_int_t int_var__drop_on_error;
        void drop_on_error (const cpp_int  & l__val);
        cpp_int drop_on_error() const;
    
}; // cap_pbc_csr_cfg_write_control_9_t
    
class cap_pbc_csr_cfg_oq_8_arb_l2_strict_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_8_arb_l2_strict_t(string name = "cap_pbc_csr_cfg_oq_8_arb_l2_strict_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_8_arb_l2_strict_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > priority_cpp_int_t;
        priority_cpp_int_t int_var__priority;
        void priority (const cpp_int  & l__val);
        cpp_int priority() const;
    
}; // cap_pbc_csr_cfg_oq_8_arb_l2_strict_t
    
class cap_pbc_csr_cfg_oq_8_arb_l2_selection_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_8_arb_l2_selection_t(string name = "cap_pbc_csr_cfg_oq_8_arb_l2_selection_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_8_arb_l2_selection_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > node_0_cpp_int_t;
        node_0_cpp_int_t int_var__node_0;
        void node_0 (const cpp_int  & l__val);
        cpp_int node_0() const;
    
        typedef pu_cpp_int< 16 > node_1_cpp_int_t;
        node_1_cpp_int_t int_var__node_1;
        void node_1 (const cpp_int  & l__val);
        cpp_int node_1() const;
    
        typedef pu_cpp_int< 16 > node_2_cpp_int_t;
        node_2_cpp_int_t int_var__node_2;
        void node_2 (const cpp_int  & l__val);
        cpp_int node_2() const;
    
        typedef pu_cpp_int< 16 > node_3_cpp_int_t;
        node_3_cpp_int_t int_var__node_3;
        void node_3 (const cpp_int  & l__val);
        cpp_int node_3() const;
    
}; // cap_pbc_csr_cfg_oq_8_arb_l2_selection_t
    
class cap_pbc_csr_cfg_oq_8_arb_l1_strict_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_8_arb_l1_strict_t(string name = "cap_pbc_csr_cfg_oq_8_arb_l1_strict_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_8_arb_l1_strict_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > priority_cpp_int_t;
        priority_cpp_int_t int_var__priority;
        void priority (const cpp_int  & l__val);
        cpp_int priority() const;
    
}; // cap_pbc_csr_cfg_oq_8_arb_l1_strict_t
    
class cap_pbc_csr_cfg_oq_8_arb_l1_selection_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_8_arb_l1_selection_t(string name = "cap_pbc_csr_cfg_oq_8_arb_l1_selection_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_8_arb_l1_selection_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > node_0_cpp_int_t;
        node_0_cpp_int_t int_var__node_0;
        void node_0 (const cpp_int  & l__val);
        cpp_int node_0() const;
    
        typedef pu_cpp_int< 32 > node_1_cpp_int_t;
        node_1_cpp_int_t int_var__node_1;
        void node_1 (const cpp_int  & l__val);
        cpp_int node_1() const;
    
        typedef pu_cpp_int< 32 > node_2_cpp_int_t;
        node_2_cpp_int_t int_var__node_2;
        void node_2 (const cpp_int  & l__val);
        cpp_int node_2() const;
    
        typedef pu_cpp_int< 32 > node_3_cpp_int_t;
        node_3_cpp_int_t int_var__node_3;
        void node_3 (const cpp_int  & l__val);
        cpp_int node_3() const;
    
        typedef pu_cpp_int< 32 > node_4_cpp_int_t;
        node_4_cpp_int_t int_var__node_4;
        void node_4 (const cpp_int  & l__val);
        cpp_int node_4() const;
    
        typedef pu_cpp_int< 32 > node_5_cpp_int_t;
        node_5_cpp_int_t int_var__node_5;
        void node_5 (const cpp_int  & l__val);
        cpp_int node_5() const;
    
        typedef pu_cpp_int< 32 > node_6_cpp_int_t;
        node_6_cpp_int_t int_var__node_6;
        void node_6 (const cpp_int  & l__val);
        cpp_int node_6() const;
    
        typedef pu_cpp_int< 32 > node_7_cpp_int_t;
        node_7_cpp_int_t int_var__node_7;
        void node_7 (const cpp_int  & l__val);
        cpp_int node_7() const;
    
        typedef pu_cpp_int< 32 > node_8_cpp_int_t;
        node_8_cpp_int_t int_var__node_8;
        void node_8 (const cpp_int  & l__val);
        cpp_int node_8() const;
    
        typedef pu_cpp_int< 32 > node_9_cpp_int_t;
        node_9_cpp_int_t int_var__node_9;
        void node_9 (const cpp_int  & l__val);
        cpp_int node_9() const;
    
        typedef pu_cpp_int< 32 > node_10_cpp_int_t;
        node_10_cpp_int_t int_var__node_10;
        void node_10 (const cpp_int  & l__val);
        cpp_int node_10() const;
    
        typedef pu_cpp_int< 32 > node_11_cpp_int_t;
        node_11_cpp_int_t int_var__node_11;
        void node_11 (const cpp_int  & l__val);
        cpp_int node_11() const;
    
        typedef pu_cpp_int< 32 > node_12_cpp_int_t;
        node_12_cpp_int_t int_var__node_12;
        void node_12 (const cpp_int  & l__val);
        cpp_int node_12() const;
    
        typedef pu_cpp_int< 32 > node_13_cpp_int_t;
        node_13_cpp_int_t int_var__node_13;
        void node_13 (const cpp_int  & l__val);
        cpp_int node_13() const;
    
        typedef pu_cpp_int< 32 > node_14_cpp_int_t;
        node_14_cpp_int_t int_var__node_14;
        void node_14 (const cpp_int  & l__val);
        cpp_int node_14() const;
    
        typedef pu_cpp_int< 32 > node_15_cpp_int_t;
        node_15_cpp_int_t int_var__node_15;
        void node_15 (const cpp_int  & l__val);
        cpp_int node_15() const;
    
}; // cap_pbc_csr_cfg_oq_8_arb_l1_selection_t
    
class cap_pbc_csr_cfg_oq_8_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_8_t(string name = "cap_pbc_csr_cfg_oq_8_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_8_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        enable_cpp_int_t int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        ecc_disable_det_cpp_int_t int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        ecc_disable_cor_cpp_int_t int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        dhs_eccbypass_cpp_int_t int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 7 > num_hdr_bytes_cpp_int_t;
        num_hdr_bytes_cpp_int_t int_var__num_hdr_bytes;
        void num_hdr_bytes (const cpp_int  & l__val);
        cpp_int num_hdr_bytes() const;
    
        typedef pu_cpp_int< 1 > rewrite_enable_cpp_int_t;
        rewrite_enable_cpp_int_t int_var__rewrite_enable;
        void rewrite_enable (const cpp_int  & l__val);
        cpp_int rewrite_enable() const;
    
        typedef pu_cpp_int< 5 > ipg_bytes_cpp_int_t;
        ipg_bytes_cpp_int_t int_var__ipg_bytes;
        void ipg_bytes (const cpp_int  & l__val);
        cpp_int ipg_bytes() const;
    
}; // cap_pbc_csr_cfg_oq_8_t
    
class cap_pbc_csr_cfg_account_8_pg_7_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_8_pg_7_t(string name = "cap_pbc_csr_cfg_account_8_pg_7_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_8_pg_7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_8_pg_7_t
    
class cap_pbc_csr_cfg_account_8_pg_6_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_8_pg_6_t(string name = "cap_pbc_csr_cfg_account_8_pg_6_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_8_pg_6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_8_pg_6_t
    
class cap_pbc_csr_cfg_account_8_pg_5_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_8_pg_5_t(string name = "cap_pbc_csr_cfg_account_8_pg_5_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_8_pg_5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_8_pg_5_t
    
class cap_pbc_csr_cfg_account_8_pg_4_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_8_pg_4_t(string name = "cap_pbc_csr_cfg_account_8_pg_4_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_8_pg_4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_8_pg_4_t
    
class cap_pbc_csr_cfg_account_8_pg_3_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_8_pg_3_t(string name = "cap_pbc_csr_cfg_account_8_pg_3_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_8_pg_3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_8_pg_3_t
    
class cap_pbc_csr_cfg_account_8_pg_2_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_8_pg_2_t(string name = "cap_pbc_csr_cfg_account_8_pg_2_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_8_pg_2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_8_pg_2_t
    
class cap_pbc_csr_cfg_account_8_pg_1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_8_pg_1_t(string name = "cap_pbc_csr_cfg_account_8_pg_1_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_8_pg_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_8_pg_1_t
    
class cap_pbc_csr_cfg_account_8_pg_0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_8_pg_0_t(string name = "cap_pbc_csr_cfg_account_8_pg_0_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_8_pg_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_8_pg_0_t
    
class cap_pbc_csr_cfg_account_8_mtu_table_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_8_mtu_table_t(string name = "cap_pbc_csr_cfg_account_8_mtu_table_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_8_mtu_table_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 5 > pg0_cpp_int_t;
        pg0_cpp_int_t int_var__pg0;
        void pg0 (const cpp_int  & l__val);
        cpp_int pg0() const;
    
        typedef pu_cpp_int< 5 > pg1_cpp_int_t;
        pg1_cpp_int_t int_var__pg1;
        void pg1 (const cpp_int  & l__val);
        cpp_int pg1() const;
    
        typedef pu_cpp_int< 5 > pg2_cpp_int_t;
        pg2_cpp_int_t int_var__pg2;
        void pg2 (const cpp_int  & l__val);
        cpp_int pg2() const;
    
        typedef pu_cpp_int< 5 > pg3_cpp_int_t;
        pg3_cpp_int_t int_var__pg3;
        void pg3 (const cpp_int  & l__val);
        cpp_int pg3() const;
    
        typedef pu_cpp_int< 5 > pg4_cpp_int_t;
        pg4_cpp_int_t int_var__pg4;
        void pg4 (const cpp_int  & l__val);
        cpp_int pg4() const;
    
        typedef pu_cpp_int< 5 > pg5_cpp_int_t;
        pg5_cpp_int_t int_var__pg5;
        void pg5 (const cpp_int  & l__val);
        cpp_int pg5() const;
    
        typedef pu_cpp_int< 5 > pg6_cpp_int_t;
        pg6_cpp_int_t int_var__pg6;
        void pg6 (const cpp_int  & l__val);
        cpp_int pg6() const;
    
        typedef pu_cpp_int< 5 > pg7_cpp_int_t;
        pg7_cpp_int_t int_var__pg7;
        void pg7 (const cpp_int  & l__val);
        cpp_int pg7() const;
    
}; // cap_pbc_csr_cfg_account_8_mtu_table_t
    
class cap_pbc_csr_cfg_account_8_pause_timer_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_8_pause_timer_t(string name = "cap_pbc_csr_cfg_account_8_pause_timer_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_8_pause_timer_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > xoff_timeout_cpp_int_t;
        xoff_timeout_cpp_int_t int_var__xoff_timeout;
        void xoff_timeout (const cpp_int  & l__val);
        cpp_int xoff_timeout() const;
    
}; // cap_pbc_csr_cfg_account_8_pause_timer_t
    
class cap_pbc_csr_sta_account_8_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_sta_account_8_t(string name = "cap_pbc_csr_sta_account_8_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_sta_account_8_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 14 > occupancy_0_cpp_int_t;
        occupancy_0_cpp_int_t int_var__occupancy_0;
        void occupancy_0 (const cpp_int  & l__val);
        cpp_int occupancy_0() const;
    
        typedef pu_cpp_int< 14 > occupancy_1_cpp_int_t;
        occupancy_1_cpp_int_t int_var__occupancy_1;
        void occupancy_1 (const cpp_int  & l__val);
        cpp_int occupancy_1() const;
    
        typedef pu_cpp_int< 14 > occupancy_2_cpp_int_t;
        occupancy_2_cpp_int_t int_var__occupancy_2;
        void occupancy_2 (const cpp_int  & l__val);
        cpp_int occupancy_2() const;
    
        typedef pu_cpp_int< 14 > occupancy_3_cpp_int_t;
        occupancy_3_cpp_int_t int_var__occupancy_3;
        void occupancy_3 (const cpp_int  & l__val);
        cpp_int occupancy_3() const;
    
        typedef pu_cpp_int< 14 > occupancy_4_cpp_int_t;
        occupancy_4_cpp_int_t int_var__occupancy_4;
        void occupancy_4 (const cpp_int  & l__val);
        cpp_int occupancy_4() const;
    
        typedef pu_cpp_int< 14 > occupancy_5_cpp_int_t;
        occupancy_5_cpp_int_t int_var__occupancy_5;
        void occupancy_5 (const cpp_int  & l__val);
        cpp_int occupancy_5() const;
    
        typedef pu_cpp_int< 14 > occupancy_6_cpp_int_t;
        occupancy_6_cpp_int_t int_var__occupancy_6;
        void occupancy_6 (const cpp_int  & l__val);
        cpp_int occupancy_6() const;
    
        typedef pu_cpp_int< 14 > occupancy_7_cpp_int_t;
        occupancy_7_cpp_int_t int_var__occupancy_7;
        void occupancy_7 (const cpp_int  & l__val);
        cpp_int occupancy_7() const;
    
}; // cap_pbc_csr_sta_account_8_t
    
class cap_pbc_csr_cfg_account_8_tc_to_pg_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_8_tc_to_pg_t(string name = "cap_pbc_csr_cfg_account_8_tc_to_pg_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_8_tc_to_pg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 24 > table_cpp_int_t;
        table_cpp_int_t int_var__table;
        void table (const cpp_int  & l__val);
        cpp_int table() const;
    
}; // cap_pbc_csr_cfg_account_8_tc_to_pg_t
    
class cap_pbc_csr_cfg_write_control_8_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_write_control_8_t(string name = "cap_pbc_csr_cfg_write_control_8_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_write_control_8_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        enable_cpp_int_t int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 5 > cut_thru_cpp_int_t;
        cut_thru_cpp_int_t int_var__cut_thru;
        void cut_thru (const cpp_int  & l__val);
        cpp_int cut_thru() const;
    
        typedef pu_cpp_int< 1 > packing_cpp_int_t;
        packing_cpp_int_t int_var__packing;
        void packing (const cpp_int  & l__val);
        cpp_int packing() const;
    
        typedef pu_cpp_int< 1 > packing_same_pg_cpp_int_t;
        packing_same_pg_cpp_int_t int_var__packing_same_pg;
        void packing_same_pg (const cpp_int  & l__val);
        cpp_int packing_same_pg() const;
    
        typedef pu_cpp_int< 10 > packing_timeout_cycles_cpp_int_t;
        packing_timeout_cycles_cpp_int_t int_var__packing_timeout_cycles;
        void packing_timeout_cycles (const cpp_int  & l__val);
        cpp_int packing_timeout_cycles() const;
    
        typedef pu_cpp_int< 1 > rewrite_cpp_int_t;
        rewrite_cpp_int_t int_var__rewrite;
        void rewrite (const cpp_int  & l__val);
        cpp_int rewrite() const;
    
        typedef pu_cpp_int< 3 > rate_limiter_cpp_int_t;
        rate_limiter_cpp_int_t int_var__rate_limiter;
        void rate_limiter (const cpp_int  & l__val);
        cpp_int rate_limiter() const;
    
        typedef pu_cpp_int< 1 > drop_on_error_cpp_int_t;
        drop_on_error_cpp_int_t int_var__drop_on_error;
        void drop_on_error (const cpp_int  & l__val);
        cpp_int drop_on_error() const;
    
}; // cap_pbc_csr_cfg_write_control_8_t
    
class cap_pbc_csr_cfg_oq_7_arb_l2_strict_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_7_arb_l2_strict_t(string name = "cap_pbc_csr_cfg_oq_7_arb_l2_strict_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_7_arb_l2_strict_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > priority_cpp_int_t;
        priority_cpp_int_t int_var__priority;
        void priority (const cpp_int  & l__val);
        cpp_int priority() const;
    
}; // cap_pbc_csr_cfg_oq_7_arb_l2_strict_t
    
class cap_pbc_csr_cfg_oq_7_arb_l2_selection_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_7_arb_l2_selection_t(string name = "cap_pbc_csr_cfg_oq_7_arb_l2_selection_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_7_arb_l2_selection_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > node_0_cpp_int_t;
        node_0_cpp_int_t int_var__node_0;
        void node_0 (const cpp_int  & l__val);
        cpp_int node_0() const;
    
        typedef pu_cpp_int< 16 > node_1_cpp_int_t;
        node_1_cpp_int_t int_var__node_1;
        void node_1 (const cpp_int  & l__val);
        cpp_int node_1() const;
    
        typedef pu_cpp_int< 16 > node_2_cpp_int_t;
        node_2_cpp_int_t int_var__node_2;
        void node_2 (const cpp_int  & l__val);
        cpp_int node_2() const;
    
        typedef pu_cpp_int< 16 > node_3_cpp_int_t;
        node_3_cpp_int_t int_var__node_3;
        void node_3 (const cpp_int  & l__val);
        cpp_int node_3() const;
    
}; // cap_pbc_csr_cfg_oq_7_arb_l2_selection_t
    
class cap_pbc_csr_cfg_oq_7_arb_l1_strict_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_7_arb_l1_strict_t(string name = "cap_pbc_csr_cfg_oq_7_arb_l1_strict_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_7_arb_l1_strict_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > priority_cpp_int_t;
        priority_cpp_int_t int_var__priority;
        void priority (const cpp_int  & l__val);
        cpp_int priority() const;
    
}; // cap_pbc_csr_cfg_oq_7_arb_l1_strict_t
    
class cap_pbc_csr_cfg_oq_7_arb_l1_selection_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_7_arb_l1_selection_t(string name = "cap_pbc_csr_cfg_oq_7_arb_l1_selection_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_7_arb_l1_selection_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > node_0_cpp_int_t;
        node_0_cpp_int_t int_var__node_0;
        void node_0 (const cpp_int  & l__val);
        cpp_int node_0() const;
    
        typedef pu_cpp_int< 32 > node_1_cpp_int_t;
        node_1_cpp_int_t int_var__node_1;
        void node_1 (const cpp_int  & l__val);
        cpp_int node_1() const;
    
        typedef pu_cpp_int< 32 > node_2_cpp_int_t;
        node_2_cpp_int_t int_var__node_2;
        void node_2 (const cpp_int  & l__val);
        cpp_int node_2() const;
    
        typedef pu_cpp_int< 32 > node_3_cpp_int_t;
        node_3_cpp_int_t int_var__node_3;
        void node_3 (const cpp_int  & l__val);
        cpp_int node_3() const;
    
        typedef pu_cpp_int< 32 > node_4_cpp_int_t;
        node_4_cpp_int_t int_var__node_4;
        void node_4 (const cpp_int  & l__val);
        cpp_int node_4() const;
    
        typedef pu_cpp_int< 32 > node_5_cpp_int_t;
        node_5_cpp_int_t int_var__node_5;
        void node_5 (const cpp_int  & l__val);
        cpp_int node_5() const;
    
        typedef pu_cpp_int< 32 > node_6_cpp_int_t;
        node_6_cpp_int_t int_var__node_6;
        void node_6 (const cpp_int  & l__val);
        cpp_int node_6() const;
    
        typedef pu_cpp_int< 32 > node_7_cpp_int_t;
        node_7_cpp_int_t int_var__node_7;
        void node_7 (const cpp_int  & l__val);
        cpp_int node_7() const;
    
        typedef pu_cpp_int< 32 > node_8_cpp_int_t;
        node_8_cpp_int_t int_var__node_8;
        void node_8 (const cpp_int  & l__val);
        cpp_int node_8() const;
    
        typedef pu_cpp_int< 32 > node_9_cpp_int_t;
        node_9_cpp_int_t int_var__node_9;
        void node_9 (const cpp_int  & l__val);
        cpp_int node_9() const;
    
        typedef pu_cpp_int< 32 > node_10_cpp_int_t;
        node_10_cpp_int_t int_var__node_10;
        void node_10 (const cpp_int  & l__val);
        cpp_int node_10() const;
    
        typedef pu_cpp_int< 32 > node_11_cpp_int_t;
        node_11_cpp_int_t int_var__node_11;
        void node_11 (const cpp_int  & l__val);
        cpp_int node_11() const;
    
        typedef pu_cpp_int< 32 > node_12_cpp_int_t;
        node_12_cpp_int_t int_var__node_12;
        void node_12 (const cpp_int  & l__val);
        cpp_int node_12() const;
    
        typedef pu_cpp_int< 32 > node_13_cpp_int_t;
        node_13_cpp_int_t int_var__node_13;
        void node_13 (const cpp_int  & l__val);
        cpp_int node_13() const;
    
        typedef pu_cpp_int< 32 > node_14_cpp_int_t;
        node_14_cpp_int_t int_var__node_14;
        void node_14 (const cpp_int  & l__val);
        cpp_int node_14() const;
    
        typedef pu_cpp_int< 32 > node_15_cpp_int_t;
        node_15_cpp_int_t int_var__node_15;
        void node_15 (const cpp_int  & l__val);
        cpp_int node_15() const;
    
}; // cap_pbc_csr_cfg_oq_7_arb_l1_selection_t
    
class cap_pbc_csr_cfg_oq_7_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_7_t(string name = "cap_pbc_csr_cfg_oq_7_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        enable_cpp_int_t int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        ecc_disable_det_cpp_int_t int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        ecc_disable_cor_cpp_int_t int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        dhs_eccbypass_cpp_int_t int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 7 > num_hdr_bytes_cpp_int_t;
        num_hdr_bytes_cpp_int_t int_var__num_hdr_bytes;
        void num_hdr_bytes (const cpp_int  & l__val);
        cpp_int num_hdr_bytes() const;
    
        typedef pu_cpp_int< 1 > rewrite_enable_cpp_int_t;
        rewrite_enable_cpp_int_t int_var__rewrite_enable;
        void rewrite_enable (const cpp_int  & l__val);
        cpp_int rewrite_enable() const;
    
        typedef pu_cpp_int< 5 > ipg_bytes_cpp_int_t;
        ipg_bytes_cpp_int_t int_var__ipg_bytes;
        void ipg_bytes (const cpp_int  & l__val);
        cpp_int ipg_bytes() const;
    
}; // cap_pbc_csr_cfg_oq_7_t
    
class cap_pbc_csr_cfg_account_7_pg_7_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_7_pg_7_t(string name = "cap_pbc_csr_cfg_account_7_pg_7_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_7_pg_7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_7_pg_7_t
    
class cap_pbc_csr_cfg_account_7_pg_6_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_7_pg_6_t(string name = "cap_pbc_csr_cfg_account_7_pg_6_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_7_pg_6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_7_pg_6_t
    
class cap_pbc_csr_cfg_account_7_pg_5_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_7_pg_5_t(string name = "cap_pbc_csr_cfg_account_7_pg_5_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_7_pg_5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_7_pg_5_t
    
class cap_pbc_csr_cfg_account_7_pg_4_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_7_pg_4_t(string name = "cap_pbc_csr_cfg_account_7_pg_4_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_7_pg_4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_7_pg_4_t
    
class cap_pbc_csr_cfg_account_7_pg_3_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_7_pg_3_t(string name = "cap_pbc_csr_cfg_account_7_pg_3_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_7_pg_3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_7_pg_3_t
    
class cap_pbc_csr_cfg_account_7_pg_2_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_7_pg_2_t(string name = "cap_pbc_csr_cfg_account_7_pg_2_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_7_pg_2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_7_pg_2_t
    
class cap_pbc_csr_cfg_account_7_pg_1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_7_pg_1_t(string name = "cap_pbc_csr_cfg_account_7_pg_1_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_7_pg_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_7_pg_1_t
    
class cap_pbc_csr_cfg_account_7_pg_0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_7_pg_0_t(string name = "cap_pbc_csr_cfg_account_7_pg_0_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_7_pg_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_7_pg_0_t
    
class cap_pbc_csr_cfg_account_7_mtu_table_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_7_mtu_table_t(string name = "cap_pbc_csr_cfg_account_7_mtu_table_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_7_mtu_table_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 5 > pg0_cpp_int_t;
        pg0_cpp_int_t int_var__pg0;
        void pg0 (const cpp_int  & l__val);
        cpp_int pg0() const;
    
        typedef pu_cpp_int< 5 > pg1_cpp_int_t;
        pg1_cpp_int_t int_var__pg1;
        void pg1 (const cpp_int  & l__val);
        cpp_int pg1() const;
    
        typedef pu_cpp_int< 5 > pg2_cpp_int_t;
        pg2_cpp_int_t int_var__pg2;
        void pg2 (const cpp_int  & l__val);
        cpp_int pg2() const;
    
        typedef pu_cpp_int< 5 > pg3_cpp_int_t;
        pg3_cpp_int_t int_var__pg3;
        void pg3 (const cpp_int  & l__val);
        cpp_int pg3() const;
    
        typedef pu_cpp_int< 5 > pg4_cpp_int_t;
        pg4_cpp_int_t int_var__pg4;
        void pg4 (const cpp_int  & l__val);
        cpp_int pg4() const;
    
        typedef pu_cpp_int< 5 > pg5_cpp_int_t;
        pg5_cpp_int_t int_var__pg5;
        void pg5 (const cpp_int  & l__val);
        cpp_int pg5() const;
    
        typedef pu_cpp_int< 5 > pg6_cpp_int_t;
        pg6_cpp_int_t int_var__pg6;
        void pg6 (const cpp_int  & l__val);
        cpp_int pg6() const;
    
        typedef pu_cpp_int< 5 > pg7_cpp_int_t;
        pg7_cpp_int_t int_var__pg7;
        void pg7 (const cpp_int  & l__val);
        cpp_int pg7() const;
    
}; // cap_pbc_csr_cfg_account_7_mtu_table_t
    
class cap_pbc_csr_cfg_account_7_pause_timer_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_7_pause_timer_t(string name = "cap_pbc_csr_cfg_account_7_pause_timer_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_7_pause_timer_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > xoff_timeout_cpp_int_t;
        xoff_timeout_cpp_int_t int_var__xoff_timeout;
        void xoff_timeout (const cpp_int  & l__val);
        cpp_int xoff_timeout() const;
    
}; // cap_pbc_csr_cfg_account_7_pause_timer_t
    
class cap_pbc_csr_sta_account_7_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_sta_account_7_t(string name = "cap_pbc_csr_sta_account_7_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_sta_account_7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 14 > occupancy_0_cpp_int_t;
        occupancy_0_cpp_int_t int_var__occupancy_0;
        void occupancy_0 (const cpp_int  & l__val);
        cpp_int occupancy_0() const;
    
        typedef pu_cpp_int< 14 > occupancy_1_cpp_int_t;
        occupancy_1_cpp_int_t int_var__occupancy_1;
        void occupancy_1 (const cpp_int  & l__val);
        cpp_int occupancy_1() const;
    
        typedef pu_cpp_int< 14 > occupancy_2_cpp_int_t;
        occupancy_2_cpp_int_t int_var__occupancy_2;
        void occupancy_2 (const cpp_int  & l__val);
        cpp_int occupancy_2() const;
    
        typedef pu_cpp_int< 14 > occupancy_3_cpp_int_t;
        occupancy_3_cpp_int_t int_var__occupancy_3;
        void occupancy_3 (const cpp_int  & l__val);
        cpp_int occupancy_3() const;
    
        typedef pu_cpp_int< 14 > occupancy_4_cpp_int_t;
        occupancy_4_cpp_int_t int_var__occupancy_4;
        void occupancy_4 (const cpp_int  & l__val);
        cpp_int occupancy_4() const;
    
        typedef pu_cpp_int< 14 > occupancy_5_cpp_int_t;
        occupancy_5_cpp_int_t int_var__occupancy_5;
        void occupancy_5 (const cpp_int  & l__val);
        cpp_int occupancy_5() const;
    
        typedef pu_cpp_int< 14 > occupancy_6_cpp_int_t;
        occupancy_6_cpp_int_t int_var__occupancy_6;
        void occupancy_6 (const cpp_int  & l__val);
        cpp_int occupancy_6() const;
    
        typedef pu_cpp_int< 14 > occupancy_7_cpp_int_t;
        occupancy_7_cpp_int_t int_var__occupancy_7;
        void occupancy_7 (const cpp_int  & l__val);
        cpp_int occupancy_7() const;
    
}; // cap_pbc_csr_sta_account_7_t
    
class cap_pbc_csr_cfg_account_7_tc_to_pg_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_7_tc_to_pg_t(string name = "cap_pbc_csr_cfg_account_7_tc_to_pg_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_7_tc_to_pg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 24 > table_cpp_int_t;
        table_cpp_int_t int_var__table;
        void table (const cpp_int  & l__val);
        cpp_int table() const;
    
}; // cap_pbc_csr_cfg_account_7_tc_to_pg_t
    
class cap_pbc_csr_cfg_write_control_7_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_write_control_7_t(string name = "cap_pbc_csr_cfg_write_control_7_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_write_control_7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        enable_cpp_int_t int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 5 > cut_thru_cpp_int_t;
        cut_thru_cpp_int_t int_var__cut_thru;
        void cut_thru (const cpp_int  & l__val);
        cpp_int cut_thru() const;
    
        typedef pu_cpp_int< 1 > packing_cpp_int_t;
        packing_cpp_int_t int_var__packing;
        void packing (const cpp_int  & l__val);
        cpp_int packing() const;
    
        typedef pu_cpp_int< 1 > packing_same_pg_cpp_int_t;
        packing_same_pg_cpp_int_t int_var__packing_same_pg;
        void packing_same_pg (const cpp_int  & l__val);
        cpp_int packing_same_pg() const;
    
        typedef pu_cpp_int< 10 > packing_timeout_cycles_cpp_int_t;
        packing_timeout_cycles_cpp_int_t int_var__packing_timeout_cycles;
        void packing_timeout_cycles (const cpp_int  & l__val);
        cpp_int packing_timeout_cycles() const;
    
        typedef pu_cpp_int< 1 > rewrite_cpp_int_t;
        rewrite_cpp_int_t int_var__rewrite;
        void rewrite (const cpp_int  & l__val);
        cpp_int rewrite() const;
    
        typedef pu_cpp_int< 3 > rate_limiter_cpp_int_t;
        rate_limiter_cpp_int_t int_var__rate_limiter;
        void rate_limiter (const cpp_int  & l__val);
        cpp_int rate_limiter() const;
    
        typedef pu_cpp_int< 1 > drop_on_error_cpp_int_t;
        drop_on_error_cpp_int_t int_var__drop_on_error;
        void drop_on_error (const cpp_int  & l__val);
        cpp_int drop_on_error() const;
    
}; // cap_pbc_csr_cfg_write_control_7_t
    
class cap_pbc_csr_cfg_oq_6_arb_l2_strict_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_6_arb_l2_strict_t(string name = "cap_pbc_csr_cfg_oq_6_arb_l2_strict_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_6_arb_l2_strict_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > priority_cpp_int_t;
        priority_cpp_int_t int_var__priority;
        void priority (const cpp_int  & l__val);
        cpp_int priority() const;
    
}; // cap_pbc_csr_cfg_oq_6_arb_l2_strict_t
    
class cap_pbc_csr_cfg_oq_6_arb_l2_selection_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_6_arb_l2_selection_t(string name = "cap_pbc_csr_cfg_oq_6_arb_l2_selection_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_6_arb_l2_selection_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > node_0_cpp_int_t;
        node_0_cpp_int_t int_var__node_0;
        void node_0 (const cpp_int  & l__val);
        cpp_int node_0() const;
    
        typedef pu_cpp_int< 16 > node_1_cpp_int_t;
        node_1_cpp_int_t int_var__node_1;
        void node_1 (const cpp_int  & l__val);
        cpp_int node_1() const;
    
        typedef pu_cpp_int< 16 > node_2_cpp_int_t;
        node_2_cpp_int_t int_var__node_2;
        void node_2 (const cpp_int  & l__val);
        cpp_int node_2() const;
    
        typedef pu_cpp_int< 16 > node_3_cpp_int_t;
        node_3_cpp_int_t int_var__node_3;
        void node_3 (const cpp_int  & l__val);
        cpp_int node_3() const;
    
}; // cap_pbc_csr_cfg_oq_6_arb_l2_selection_t
    
class cap_pbc_csr_cfg_oq_6_arb_l1_strict_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_6_arb_l1_strict_t(string name = "cap_pbc_csr_cfg_oq_6_arb_l1_strict_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_6_arb_l1_strict_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > priority_cpp_int_t;
        priority_cpp_int_t int_var__priority;
        void priority (const cpp_int  & l__val);
        cpp_int priority() const;
    
}; // cap_pbc_csr_cfg_oq_6_arb_l1_strict_t
    
class cap_pbc_csr_cfg_oq_6_arb_l1_selection_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_6_arb_l1_selection_t(string name = "cap_pbc_csr_cfg_oq_6_arb_l1_selection_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_6_arb_l1_selection_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > node_0_cpp_int_t;
        node_0_cpp_int_t int_var__node_0;
        void node_0 (const cpp_int  & l__val);
        cpp_int node_0() const;
    
        typedef pu_cpp_int< 32 > node_1_cpp_int_t;
        node_1_cpp_int_t int_var__node_1;
        void node_1 (const cpp_int  & l__val);
        cpp_int node_1() const;
    
        typedef pu_cpp_int< 32 > node_2_cpp_int_t;
        node_2_cpp_int_t int_var__node_2;
        void node_2 (const cpp_int  & l__val);
        cpp_int node_2() const;
    
        typedef pu_cpp_int< 32 > node_3_cpp_int_t;
        node_3_cpp_int_t int_var__node_3;
        void node_3 (const cpp_int  & l__val);
        cpp_int node_3() const;
    
        typedef pu_cpp_int< 32 > node_4_cpp_int_t;
        node_4_cpp_int_t int_var__node_4;
        void node_4 (const cpp_int  & l__val);
        cpp_int node_4() const;
    
        typedef pu_cpp_int< 32 > node_5_cpp_int_t;
        node_5_cpp_int_t int_var__node_5;
        void node_5 (const cpp_int  & l__val);
        cpp_int node_5() const;
    
        typedef pu_cpp_int< 32 > node_6_cpp_int_t;
        node_6_cpp_int_t int_var__node_6;
        void node_6 (const cpp_int  & l__val);
        cpp_int node_6() const;
    
        typedef pu_cpp_int< 32 > node_7_cpp_int_t;
        node_7_cpp_int_t int_var__node_7;
        void node_7 (const cpp_int  & l__val);
        cpp_int node_7() const;
    
        typedef pu_cpp_int< 32 > node_8_cpp_int_t;
        node_8_cpp_int_t int_var__node_8;
        void node_8 (const cpp_int  & l__val);
        cpp_int node_8() const;
    
        typedef pu_cpp_int< 32 > node_9_cpp_int_t;
        node_9_cpp_int_t int_var__node_9;
        void node_9 (const cpp_int  & l__val);
        cpp_int node_9() const;
    
        typedef pu_cpp_int< 32 > node_10_cpp_int_t;
        node_10_cpp_int_t int_var__node_10;
        void node_10 (const cpp_int  & l__val);
        cpp_int node_10() const;
    
        typedef pu_cpp_int< 32 > node_11_cpp_int_t;
        node_11_cpp_int_t int_var__node_11;
        void node_11 (const cpp_int  & l__val);
        cpp_int node_11() const;
    
        typedef pu_cpp_int< 32 > node_12_cpp_int_t;
        node_12_cpp_int_t int_var__node_12;
        void node_12 (const cpp_int  & l__val);
        cpp_int node_12() const;
    
        typedef pu_cpp_int< 32 > node_13_cpp_int_t;
        node_13_cpp_int_t int_var__node_13;
        void node_13 (const cpp_int  & l__val);
        cpp_int node_13() const;
    
        typedef pu_cpp_int< 32 > node_14_cpp_int_t;
        node_14_cpp_int_t int_var__node_14;
        void node_14 (const cpp_int  & l__val);
        cpp_int node_14() const;
    
        typedef pu_cpp_int< 32 > node_15_cpp_int_t;
        node_15_cpp_int_t int_var__node_15;
        void node_15 (const cpp_int  & l__val);
        cpp_int node_15() const;
    
}; // cap_pbc_csr_cfg_oq_6_arb_l1_selection_t
    
class cap_pbc_csr_cfg_oq_6_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_6_t(string name = "cap_pbc_csr_cfg_oq_6_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        enable_cpp_int_t int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        ecc_disable_det_cpp_int_t int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        ecc_disable_cor_cpp_int_t int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        dhs_eccbypass_cpp_int_t int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 7 > num_hdr_bytes_cpp_int_t;
        num_hdr_bytes_cpp_int_t int_var__num_hdr_bytes;
        void num_hdr_bytes (const cpp_int  & l__val);
        cpp_int num_hdr_bytes() const;
    
        typedef pu_cpp_int< 1 > rewrite_enable_cpp_int_t;
        rewrite_enable_cpp_int_t int_var__rewrite_enable;
        void rewrite_enable (const cpp_int  & l__val);
        cpp_int rewrite_enable() const;
    
        typedef pu_cpp_int< 5 > ipg_bytes_cpp_int_t;
        ipg_bytes_cpp_int_t int_var__ipg_bytes;
        void ipg_bytes (const cpp_int  & l__val);
        cpp_int ipg_bytes() const;
    
}; // cap_pbc_csr_cfg_oq_6_t
    
class cap_pbc_csr_cfg_account_6_pg_7_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_6_pg_7_t(string name = "cap_pbc_csr_cfg_account_6_pg_7_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_6_pg_7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_6_pg_7_t
    
class cap_pbc_csr_cfg_account_6_pg_6_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_6_pg_6_t(string name = "cap_pbc_csr_cfg_account_6_pg_6_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_6_pg_6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_6_pg_6_t
    
class cap_pbc_csr_cfg_account_6_pg_5_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_6_pg_5_t(string name = "cap_pbc_csr_cfg_account_6_pg_5_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_6_pg_5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_6_pg_5_t
    
class cap_pbc_csr_cfg_account_6_pg_4_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_6_pg_4_t(string name = "cap_pbc_csr_cfg_account_6_pg_4_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_6_pg_4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_6_pg_4_t
    
class cap_pbc_csr_cfg_account_6_pg_3_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_6_pg_3_t(string name = "cap_pbc_csr_cfg_account_6_pg_3_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_6_pg_3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_6_pg_3_t
    
class cap_pbc_csr_cfg_account_6_pg_2_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_6_pg_2_t(string name = "cap_pbc_csr_cfg_account_6_pg_2_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_6_pg_2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_6_pg_2_t
    
class cap_pbc_csr_cfg_account_6_pg_1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_6_pg_1_t(string name = "cap_pbc_csr_cfg_account_6_pg_1_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_6_pg_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_6_pg_1_t
    
class cap_pbc_csr_cfg_account_6_pg_0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_6_pg_0_t(string name = "cap_pbc_csr_cfg_account_6_pg_0_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_6_pg_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_6_pg_0_t
    
class cap_pbc_csr_cfg_account_6_mtu_table_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_6_mtu_table_t(string name = "cap_pbc_csr_cfg_account_6_mtu_table_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_6_mtu_table_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 5 > pg0_cpp_int_t;
        pg0_cpp_int_t int_var__pg0;
        void pg0 (const cpp_int  & l__val);
        cpp_int pg0() const;
    
        typedef pu_cpp_int< 5 > pg1_cpp_int_t;
        pg1_cpp_int_t int_var__pg1;
        void pg1 (const cpp_int  & l__val);
        cpp_int pg1() const;
    
        typedef pu_cpp_int< 5 > pg2_cpp_int_t;
        pg2_cpp_int_t int_var__pg2;
        void pg2 (const cpp_int  & l__val);
        cpp_int pg2() const;
    
        typedef pu_cpp_int< 5 > pg3_cpp_int_t;
        pg3_cpp_int_t int_var__pg3;
        void pg3 (const cpp_int  & l__val);
        cpp_int pg3() const;
    
        typedef pu_cpp_int< 5 > pg4_cpp_int_t;
        pg4_cpp_int_t int_var__pg4;
        void pg4 (const cpp_int  & l__val);
        cpp_int pg4() const;
    
        typedef pu_cpp_int< 5 > pg5_cpp_int_t;
        pg5_cpp_int_t int_var__pg5;
        void pg5 (const cpp_int  & l__val);
        cpp_int pg5() const;
    
        typedef pu_cpp_int< 5 > pg6_cpp_int_t;
        pg6_cpp_int_t int_var__pg6;
        void pg6 (const cpp_int  & l__val);
        cpp_int pg6() const;
    
        typedef pu_cpp_int< 5 > pg7_cpp_int_t;
        pg7_cpp_int_t int_var__pg7;
        void pg7 (const cpp_int  & l__val);
        cpp_int pg7() const;
    
}; // cap_pbc_csr_cfg_account_6_mtu_table_t
    
class cap_pbc_csr_cfg_account_6_pause_timer_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_6_pause_timer_t(string name = "cap_pbc_csr_cfg_account_6_pause_timer_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_6_pause_timer_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > xoff_timeout_cpp_int_t;
        xoff_timeout_cpp_int_t int_var__xoff_timeout;
        void xoff_timeout (const cpp_int  & l__val);
        cpp_int xoff_timeout() const;
    
}; // cap_pbc_csr_cfg_account_6_pause_timer_t
    
class cap_pbc_csr_sta_account_6_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_sta_account_6_t(string name = "cap_pbc_csr_sta_account_6_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_sta_account_6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 14 > occupancy_0_cpp_int_t;
        occupancy_0_cpp_int_t int_var__occupancy_0;
        void occupancy_0 (const cpp_int  & l__val);
        cpp_int occupancy_0() const;
    
        typedef pu_cpp_int< 14 > occupancy_1_cpp_int_t;
        occupancy_1_cpp_int_t int_var__occupancy_1;
        void occupancy_1 (const cpp_int  & l__val);
        cpp_int occupancy_1() const;
    
        typedef pu_cpp_int< 14 > occupancy_2_cpp_int_t;
        occupancy_2_cpp_int_t int_var__occupancy_2;
        void occupancy_2 (const cpp_int  & l__val);
        cpp_int occupancy_2() const;
    
        typedef pu_cpp_int< 14 > occupancy_3_cpp_int_t;
        occupancy_3_cpp_int_t int_var__occupancy_3;
        void occupancy_3 (const cpp_int  & l__val);
        cpp_int occupancy_3() const;
    
        typedef pu_cpp_int< 14 > occupancy_4_cpp_int_t;
        occupancy_4_cpp_int_t int_var__occupancy_4;
        void occupancy_4 (const cpp_int  & l__val);
        cpp_int occupancy_4() const;
    
        typedef pu_cpp_int< 14 > occupancy_5_cpp_int_t;
        occupancy_5_cpp_int_t int_var__occupancy_5;
        void occupancy_5 (const cpp_int  & l__val);
        cpp_int occupancy_5() const;
    
        typedef pu_cpp_int< 14 > occupancy_6_cpp_int_t;
        occupancy_6_cpp_int_t int_var__occupancy_6;
        void occupancy_6 (const cpp_int  & l__val);
        cpp_int occupancy_6() const;
    
        typedef pu_cpp_int< 14 > occupancy_7_cpp_int_t;
        occupancy_7_cpp_int_t int_var__occupancy_7;
        void occupancy_7 (const cpp_int  & l__val);
        cpp_int occupancy_7() const;
    
}; // cap_pbc_csr_sta_account_6_t
    
class cap_pbc_csr_cfg_account_6_tc_to_pg_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_6_tc_to_pg_t(string name = "cap_pbc_csr_cfg_account_6_tc_to_pg_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_6_tc_to_pg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 24 > table_cpp_int_t;
        table_cpp_int_t int_var__table;
        void table (const cpp_int  & l__val);
        cpp_int table() const;
    
}; // cap_pbc_csr_cfg_account_6_tc_to_pg_t
    
class cap_pbc_csr_cfg_write_control_6_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_write_control_6_t(string name = "cap_pbc_csr_cfg_write_control_6_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_write_control_6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        enable_cpp_int_t int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 5 > cut_thru_cpp_int_t;
        cut_thru_cpp_int_t int_var__cut_thru;
        void cut_thru (const cpp_int  & l__val);
        cpp_int cut_thru() const;
    
        typedef pu_cpp_int< 1 > packing_cpp_int_t;
        packing_cpp_int_t int_var__packing;
        void packing (const cpp_int  & l__val);
        cpp_int packing() const;
    
        typedef pu_cpp_int< 1 > packing_same_pg_cpp_int_t;
        packing_same_pg_cpp_int_t int_var__packing_same_pg;
        void packing_same_pg (const cpp_int  & l__val);
        cpp_int packing_same_pg() const;
    
        typedef pu_cpp_int< 10 > packing_timeout_cycles_cpp_int_t;
        packing_timeout_cycles_cpp_int_t int_var__packing_timeout_cycles;
        void packing_timeout_cycles (const cpp_int  & l__val);
        cpp_int packing_timeout_cycles() const;
    
        typedef pu_cpp_int< 1 > rewrite_cpp_int_t;
        rewrite_cpp_int_t int_var__rewrite;
        void rewrite (const cpp_int  & l__val);
        cpp_int rewrite() const;
    
        typedef pu_cpp_int< 3 > rate_limiter_cpp_int_t;
        rate_limiter_cpp_int_t int_var__rate_limiter;
        void rate_limiter (const cpp_int  & l__val);
        cpp_int rate_limiter() const;
    
        typedef pu_cpp_int< 1 > drop_on_error_cpp_int_t;
        drop_on_error_cpp_int_t int_var__drop_on_error;
        void drop_on_error (const cpp_int  & l__val);
        cpp_int drop_on_error() const;
    
}; // cap_pbc_csr_cfg_write_control_6_t
    
class cap_pbc_csr_cfg_oq_5_arb_l2_strict_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_5_arb_l2_strict_t(string name = "cap_pbc_csr_cfg_oq_5_arb_l2_strict_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_5_arb_l2_strict_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > priority_cpp_int_t;
        priority_cpp_int_t int_var__priority;
        void priority (const cpp_int  & l__val);
        cpp_int priority() const;
    
}; // cap_pbc_csr_cfg_oq_5_arb_l2_strict_t
    
class cap_pbc_csr_cfg_oq_5_arb_l2_selection_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_5_arb_l2_selection_t(string name = "cap_pbc_csr_cfg_oq_5_arb_l2_selection_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_5_arb_l2_selection_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > node_0_cpp_int_t;
        node_0_cpp_int_t int_var__node_0;
        void node_0 (const cpp_int  & l__val);
        cpp_int node_0() const;
    
        typedef pu_cpp_int< 16 > node_1_cpp_int_t;
        node_1_cpp_int_t int_var__node_1;
        void node_1 (const cpp_int  & l__val);
        cpp_int node_1() const;
    
        typedef pu_cpp_int< 16 > node_2_cpp_int_t;
        node_2_cpp_int_t int_var__node_2;
        void node_2 (const cpp_int  & l__val);
        cpp_int node_2() const;
    
        typedef pu_cpp_int< 16 > node_3_cpp_int_t;
        node_3_cpp_int_t int_var__node_3;
        void node_3 (const cpp_int  & l__val);
        cpp_int node_3() const;
    
}; // cap_pbc_csr_cfg_oq_5_arb_l2_selection_t
    
class cap_pbc_csr_cfg_oq_5_arb_l1_strict_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_5_arb_l1_strict_t(string name = "cap_pbc_csr_cfg_oq_5_arb_l1_strict_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_5_arb_l1_strict_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > priority_cpp_int_t;
        priority_cpp_int_t int_var__priority;
        void priority (const cpp_int  & l__val);
        cpp_int priority() const;
    
}; // cap_pbc_csr_cfg_oq_5_arb_l1_strict_t
    
class cap_pbc_csr_cfg_oq_5_arb_l1_selection_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_5_arb_l1_selection_t(string name = "cap_pbc_csr_cfg_oq_5_arb_l1_selection_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_5_arb_l1_selection_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > node_0_cpp_int_t;
        node_0_cpp_int_t int_var__node_0;
        void node_0 (const cpp_int  & l__val);
        cpp_int node_0() const;
    
        typedef pu_cpp_int< 32 > node_1_cpp_int_t;
        node_1_cpp_int_t int_var__node_1;
        void node_1 (const cpp_int  & l__val);
        cpp_int node_1() const;
    
        typedef pu_cpp_int< 32 > node_2_cpp_int_t;
        node_2_cpp_int_t int_var__node_2;
        void node_2 (const cpp_int  & l__val);
        cpp_int node_2() const;
    
        typedef pu_cpp_int< 32 > node_3_cpp_int_t;
        node_3_cpp_int_t int_var__node_3;
        void node_3 (const cpp_int  & l__val);
        cpp_int node_3() const;
    
        typedef pu_cpp_int< 32 > node_4_cpp_int_t;
        node_4_cpp_int_t int_var__node_4;
        void node_4 (const cpp_int  & l__val);
        cpp_int node_4() const;
    
        typedef pu_cpp_int< 32 > node_5_cpp_int_t;
        node_5_cpp_int_t int_var__node_5;
        void node_5 (const cpp_int  & l__val);
        cpp_int node_5() const;
    
        typedef pu_cpp_int< 32 > node_6_cpp_int_t;
        node_6_cpp_int_t int_var__node_6;
        void node_6 (const cpp_int  & l__val);
        cpp_int node_6() const;
    
        typedef pu_cpp_int< 32 > node_7_cpp_int_t;
        node_7_cpp_int_t int_var__node_7;
        void node_7 (const cpp_int  & l__val);
        cpp_int node_7() const;
    
        typedef pu_cpp_int< 32 > node_8_cpp_int_t;
        node_8_cpp_int_t int_var__node_8;
        void node_8 (const cpp_int  & l__val);
        cpp_int node_8() const;
    
        typedef pu_cpp_int< 32 > node_9_cpp_int_t;
        node_9_cpp_int_t int_var__node_9;
        void node_9 (const cpp_int  & l__val);
        cpp_int node_9() const;
    
        typedef pu_cpp_int< 32 > node_10_cpp_int_t;
        node_10_cpp_int_t int_var__node_10;
        void node_10 (const cpp_int  & l__val);
        cpp_int node_10() const;
    
        typedef pu_cpp_int< 32 > node_11_cpp_int_t;
        node_11_cpp_int_t int_var__node_11;
        void node_11 (const cpp_int  & l__val);
        cpp_int node_11() const;
    
        typedef pu_cpp_int< 32 > node_12_cpp_int_t;
        node_12_cpp_int_t int_var__node_12;
        void node_12 (const cpp_int  & l__val);
        cpp_int node_12() const;
    
        typedef pu_cpp_int< 32 > node_13_cpp_int_t;
        node_13_cpp_int_t int_var__node_13;
        void node_13 (const cpp_int  & l__val);
        cpp_int node_13() const;
    
        typedef pu_cpp_int< 32 > node_14_cpp_int_t;
        node_14_cpp_int_t int_var__node_14;
        void node_14 (const cpp_int  & l__val);
        cpp_int node_14() const;
    
        typedef pu_cpp_int< 32 > node_15_cpp_int_t;
        node_15_cpp_int_t int_var__node_15;
        void node_15 (const cpp_int  & l__val);
        cpp_int node_15() const;
    
}; // cap_pbc_csr_cfg_oq_5_arb_l1_selection_t
    
class cap_pbc_csr_cfg_oq_5_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_5_t(string name = "cap_pbc_csr_cfg_oq_5_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        enable_cpp_int_t int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        ecc_disable_det_cpp_int_t int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        ecc_disable_cor_cpp_int_t int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        dhs_eccbypass_cpp_int_t int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 7 > num_hdr_bytes_cpp_int_t;
        num_hdr_bytes_cpp_int_t int_var__num_hdr_bytes;
        void num_hdr_bytes (const cpp_int  & l__val);
        cpp_int num_hdr_bytes() const;
    
        typedef pu_cpp_int< 1 > rewrite_enable_cpp_int_t;
        rewrite_enable_cpp_int_t int_var__rewrite_enable;
        void rewrite_enable (const cpp_int  & l__val);
        cpp_int rewrite_enable() const;
    
        typedef pu_cpp_int< 5 > ipg_bytes_cpp_int_t;
        ipg_bytes_cpp_int_t int_var__ipg_bytes;
        void ipg_bytes (const cpp_int  & l__val);
        cpp_int ipg_bytes() const;
    
}; // cap_pbc_csr_cfg_oq_5_t
    
class cap_pbc_csr_cfg_account_5_pg_7_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_5_pg_7_t(string name = "cap_pbc_csr_cfg_account_5_pg_7_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_5_pg_7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_5_pg_7_t
    
class cap_pbc_csr_cfg_account_5_pg_6_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_5_pg_6_t(string name = "cap_pbc_csr_cfg_account_5_pg_6_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_5_pg_6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_5_pg_6_t
    
class cap_pbc_csr_cfg_account_5_pg_5_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_5_pg_5_t(string name = "cap_pbc_csr_cfg_account_5_pg_5_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_5_pg_5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_5_pg_5_t
    
class cap_pbc_csr_cfg_account_5_pg_4_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_5_pg_4_t(string name = "cap_pbc_csr_cfg_account_5_pg_4_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_5_pg_4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_5_pg_4_t
    
class cap_pbc_csr_cfg_account_5_pg_3_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_5_pg_3_t(string name = "cap_pbc_csr_cfg_account_5_pg_3_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_5_pg_3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_5_pg_3_t
    
class cap_pbc_csr_cfg_account_5_pg_2_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_5_pg_2_t(string name = "cap_pbc_csr_cfg_account_5_pg_2_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_5_pg_2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_5_pg_2_t
    
class cap_pbc_csr_cfg_account_5_pg_1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_5_pg_1_t(string name = "cap_pbc_csr_cfg_account_5_pg_1_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_5_pg_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_5_pg_1_t
    
class cap_pbc_csr_cfg_account_5_pg_0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_5_pg_0_t(string name = "cap_pbc_csr_cfg_account_5_pg_0_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_5_pg_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_5_pg_0_t
    
class cap_pbc_csr_cfg_account_5_mtu_table_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_5_mtu_table_t(string name = "cap_pbc_csr_cfg_account_5_mtu_table_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_5_mtu_table_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 5 > pg0_cpp_int_t;
        pg0_cpp_int_t int_var__pg0;
        void pg0 (const cpp_int  & l__val);
        cpp_int pg0() const;
    
        typedef pu_cpp_int< 5 > pg1_cpp_int_t;
        pg1_cpp_int_t int_var__pg1;
        void pg1 (const cpp_int  & l__val);
        cpp_int pg1() const;
    
        typedef pu_cpp_int< 5 > pg2_cpp_int_t;
        pg2_cpp_int_t int_var__pg2;
        void pg2 (const cpp_int  & l__val);
        cpp_int pg2() const;
    
        typedef pu_cpp_int< 5 > pg3_cpp_int_t;
        pg3_cpp_int_t int_var__pg3;
        void pg3 (const cpp_int  & l__val);
        cpp_int pg3() const;
    
        typedef pu_cpp_int< 5 > pg4_cpp_int_t;
        pg4_cpp_int_t int_var__pg4;
        void pg4 (const cpp_int  & l__val);
        cpp_int pg4() const;
    
        typedef pu_cpp_int< 5 > pg5_cpp_int_t;
        pg5_cpp_int_t int_var__pg5;
        void pg5 (const cpp_int  & l__val);
        cpp_int pg5() const;
    
        typedef pu_cpp_int< 5 > pg6_cpp_int_t;
        pg6_cpp_int_t int_var__pg6;
        void pg6 (const cpp_int  & l__val);
        cpp_int pg6() const;
    
        typedef pu_cpp_int< 5 > pg7_cpp_int_t;
        pg7_cpp_int_t int_var__pg7;
        void pg7 (const cpp_int  & l__val);
        cpp_int pg7() const;
    
}; // cap_pbc_csr_cfg_account_5_mtu_table_t
    
class cap_pbc_csr_cfg_account_5_pause_timer_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_5_pause_timer_t(string name = "cap_pbc_csr_cfg_account_5_pause_timer_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_5_pause_timer_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > xoff_timeout_cpp_int_t;
        xoff_timeout_cpp_int_t int_var__xoff_timeout;
        void xoff_timeout (const cpp_int  & l__val);
        cpp_int xoff_timeout() const;
    
}; // cap_pbc_csr_cfg_account_5_pause_timer_t
    
class cap_pbc_csr_sta_account_5_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_sta_account_5_t(string name = "cap_pbc_csr_sta_account_5_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_sta_account_5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 14 > occupancy_0_cpp_int_t;
        occupancy_0_cpp_int_t int_var__occupancy_0;
        void occupancy_0 (const cpp_int  & l__val);
        cpp_int occupancy_0() const;
    
        typedef pu_cpp_int< 14 > occupancy_1_cpp_int_t;
        occupancy_1_cpp_int_t int_var__occupancy_1;
        void occupancy_1 (const cpp_int  & l__val);
        cpp_int occupancy_1() const;
    
        typedef pu_cpp_int< 14 > occupancy_2_cpp_int_t;
        occupancy_2_cpp_int_t int_var__occupancy_2;
        void occupancy_2 (const cpp_int  & l__val);
        cpp_int occupancy_2() const;
    
        typedef pu_cpp_int< 14 > occupancy_3_cpp_int_t;
        occupancy_3_cpp_int_t int_var__occupancy_3;
        void occupancy_3 (const cpp_int  & l__val);
        cpp_int occupancy_3() const;
    
        typedef pu_cpp_int< 14 > occupancy_4_cpp_int_t;
        occupancy_4_cpp_int_t int_var__occupancy_4;
        void occupancy_4 (const cpp_int  & l__val);
        cpp_int occupancy_4() const;
    
        typedef pu_cpp_int< 14 > occupancy_5_cpp_int_t;
        occupancy_5_cpp_int_t int_var__occupancy_5;
        void occupancy_5 (const cpp_int  & l__val);
        cpp_int occupancy_5() const;
    
        typedef pu_cpp_int< 14 > occupancy_6_cpp_int_t;
        occupancy_6_cpp_int_t int_var__occupancy_6;
        void occupancy_6 (const cpp_int  & l__val);
        cpp_int occupancy_6() const;
    
        typedef pu_cpp_int< 14 > occupancy_7_cpp_int_t;
        occupancy_7_cpp_int_t int_var__occupancy_7;
        void occupancy_7 (const cpp_int  & l__val);
        cpp_int occupancy_7() const;
    
}; // cap_pbc_csr_sta_account_5_t
    
class cap_pbc_csr_cfg_account_5_tc_to_pg_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_5_tc_to_pg_t(string name = "cap_pbc_csr_cfg_account_5_tc_to_pg_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_5_tc_to_pg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 24 > table_cpp_int_t;
        table_cpp_int_t int_var__table;
        void table (const cpp_int  & l__val);
        cpp_int table() const;
    
}; // cap_pbc_csr_cfg_account_5_tc_to_pg_t
    
class cap_pbc_csr_cfg_write_control_5_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_write_control_5_t(string name = "cap_pbc_csr_cfg_write_control_5_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_write_control_5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        enable_cpp_int_t int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 5 > cut_thru_cpp_int_t;
        cut_thru_cpp_int_t int_var__cut_thru;
        void cut_thru (const cpp_int  & l__val);
        cpp_int cut_thru() const;
    
        typedef pu_cpp_int< 1 > packing_cpp_int_t;
        packing_cpp_int_t int_var__packing;
        void packing (const cpp_int  & l__val);
        cpp_int packing() const;
    
        typedef pu_cpp_int< 1 > packing_same_pg_cpp_int_t;
        packing_same_pg_cpp_int_t int_var__packing_same_pg;
        void packing_same_pg (const cpp_int  & l__val);
        cpp_int packing_same_pg() const;
    
        typedef pu_cpp_int< 10 > packing_timeout_cycles_cpp_int_t;
        packing_timeout_cycles_cpp_int_t int_var__packing_timeout_cycles;
        void packing_timeout_cycles (const cpp_int  & l__val);
        cpp_int packing_timeout_cycles() const;
    
        typedef pu_cpp_int< 1 > rewrite_cpp_int_t;
        rewrite_cpp_int_t int_var__rewrite;
        void rewrite (const cpp_int  & l__val);
        cpp_int rewrite() const;
    
        typedef pu_cpp_int< 3 > rate_limiter_cpp_int_t;
        rate_limiter_cpp_int_t int_var__rate_limiter;
        void rate_limiter (const cpp_int  & l__val);
        cpp_int rate_limiter() const;
    
        typedef pu_cpp_int< 1 > drop_on_error_cpp_int_t;
        drop_on_error_cpp_int_t int_var__drop_on_error;
        void drop_on_error (const cpp_int  & l__val);
        cpp_int drop_on_error() const;
    
}; // cap_pbc_csr_cfg_write_control_5_t
    
class cap_pbc_csr_cfg_oq_4_arb_l2_strict_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_4_arb_l2_strict_t(string name = "cap_pbc_csr_cfg_oq_4_arb_l2_strict_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_4_arb_l2_strict_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > priority_cpp_int_t;
        priority_cpp_int_t int_var__priority;
        void priority (const cpp_int  & l__val);
        cpp_int priority() const;
    
}; // cap_pbc_csr_cfg_oq_4_arb_l2_strict_t
    
class cap_pbc_csr_cfg_oq_4_arb_l2_selection_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_4_arb_l2_selection_t(string name = "cap_pbc_csr_cfg_oq_4_arb_l2_selection_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_4_arb_l2_selection_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > node_0_cpp_int_t;
        node_0_cpp_int_t int_var__node_0;
        void node_0 (const cpp_int  & l__val);
        cpp_int node_0() const;
    
        typedef pu_cpp_int< 16 > node_1_cpp_int_t;
        node_1_cpp_int_t int_var__node_1;
        void node_1 (const cpp_int  & l__val);
        cpp_int node_1() const;
    
        typedef pu_cpp_int< 16 > node_2_cpp_int_t;
        node_2_cpp_int_t int_var__node_2;
        void node_2 (const cpp_int  & l__val);
        cpp_int node_2() const;
    
        typedef pu_cpp_int< 16 > node_3_cpp_int_t;
        node_3_cpp_int_t int_var__node_3;
        void node_3 (const cpp_int  & l__val);
        cpp_int node_3() const;
    
}; // cap_pbc_csr_cfg_oq_4_arb_l2_selection_t
    
class cap_pbc_csr_cfg_oq_4_arb_l1_strict_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_4_arb_l1_strict_t(string name = "cap_pbc_csr_cfg_oq_4_arb_l1_strict_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_4_arb_l1_strict_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > priority_cpp_int_t;
        priority_cpp_int_t int_var__priority;
        void priority (const cpp_int  & l__val);
        cpp_int priority() const;
    
}; // cap_pbc_csr_cfg_oq_4_arb_l1_strict_t
    
class cap_pbc_csr_cfg_oq_4_arb_l1_selection_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_4_arb_l1_selection_t(string name = "cap_pbc_csr_cfg_oq_4_arb_l1_selection_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_4_arb_l1_selection_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > node_0_cpp_int_t;
        node_0_cpp_int_t int_var__node_0;
        void node_0 (const cpp_int  & l__val);
        cpp_int node_0() const;
    
        typedef pu_cpp_int< 32 > node_1_cpp_int_t;
        node_1_cpp_int_t int_var__node_1;
        void node_1 (const cpp_int  & l__val);
        cpp_int node_1() const;
    
        typedef pu_cpp_int< 32 > node_2_cpp_int_t;
        node_2_cpp_int_t int_var__node_2;
        void node_2 (const cpp_int  & l__val);
        cpp_int node_2() const;
    
        typedef pu_cpp_int< 32 > node_3_cpp_int_t;
        node_3_cpp_int_t int_var__node_3;
        void node_3 (const cpp_int  & l__val);
        cpp_int node_3() const;
    
        typedef pu_cpp_int< 32 > node_4_cpp_int_t;
        node_4_cpp_int_t int_var__node_4;
        void node_4 (const cpp_int  & l__val);
        cpp_int node_4() const;
    
        typedef pu_cpp_int< 32 > node_5_cpp_int_t;
        node_5_cpp_int_t int_var__node_5;
        void node_5 (const cpp_int  & l__val);
        cpp_int node_5() const;
    
        typedef pu_cpp_int< 32 > node_6_cpp_int_t;
        node_6_cpp_int_t int_var__node_6;
        void node_6 (const cpp_int  & l__val);
        cpp_int node_6() const;
    
        typedef pu_cpp_int< 32 > node_7_cpp_int_t;
        node_7_cpp_int_t int_var__node_7;
        void node_7 (const cpp_int  & l__val);
        cpp_int node_7() const;
    
        typedef pu_cpp_int< 32 > node_8_cpp_int_t;
        node_8_cpp_int_t int_var__node_8;
        void node_8 (const cpp_int  & l__val);
        cpp_int node_8() const;
    
        typedef pu_cpp_int< 32 > node_9_cpp_int_t;
        node_9_cpp_int_t int_var__node_9;
        void node_9 (const cpp_int  & l__val);
        cpp_int node_9() const;
    
        typedef pu_cpp_int< 32 > node_10_cpp_int_t;
        node_10_cpp_int_t int_var__node_10;
        void node_10 (const cpp_int  & l__val);
        cpp_int node_10() const;
    
        typedef pu_cpp_int< 32 > node_11_cpp_int_t;
        node_11_cpp_int_t int_var__node_11;
        void node_11 (const cpp_int  & l__val);
        cpp_int node_11() const;
    
        typedef pu_cpp_int< 32 > node_12_cpp_int_t;
        node_12_cpp_int_t int_var__node_12;
        void node_12 (const cpp_int  & l__val);
        cpp_int node_12() const;
    
        typedef pu_cpp_int< 32 > node_13_cpp_int_t;
        node_13_cpp_int_t int_var__node_13;
        void node_13 (const cpp_int  & l__val);
        cpp_int node_13() const;
    
        typedef pu_cpp_int< 32 > node_14_cpp_int_t;
        node_14_cpp_int_t int_var__node_14;
        void node_14 (const cpp_int  & l__val);
        cpp_int node_14() const;
    
        typedef pu_cpp_int< 32 > node_15_cpp_int_t;
        node_15_cpp_int_t int_var__node_15;
        void node_15 (const cpp_int  & l__val);
        cpp_int node_15() const;
    
}; // cap_pbc_csr_cfg_oq_4_arb_l1_selection_t
    
class cap_pbc_csr_cfg_oq_4_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_4_t(string name = "cap_pbc_csr_cfg_oq_4_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        enable_cpp_int_t int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        ecc_disable_det_cpp_int_t int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        ecc_disable_cor_cpp_int_t int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        dhs_eccbypass_cpp_int_t int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 7 > num_hdr_bytes_cpp_int_t;
        num_hdr_bytes_cpp_int_t int_var__num_hdr_bytes;
        void num_hdr_bytes (const cpp_int  & l__val);
        cpp_int num_hdr_bytes() const;
    
        typedef pu_cpp_int< 1 > rewrite_enable_cpp_int_t;
        rewrite_enable_cpp_int_t int_var__rewrite_enable;
        void rewrite_enable (const cpp_int  & l__val);
        cpp_int rewrite_enable() const;
    
        typedef pu_cpp_int< 5 > ipg_bytes_cpp_int_t;
        ipg_bytes_cpp_int_t int_var__ipg_bytes;
        void ipg_bytes (const cpp_int  & l__val);
        cpp_int ipg_bytes() const;
    
}; // cap_pbc_csr_cfg_oq_4_t
    
class cap_pbc_csr_cfg_account_4_pg_7_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_4_pg_7_t(string name = "cap_pbc_csr_cfg_account_4_pg_7_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_4_pg_7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_4_pg_7_t
    
class cap_pbc_csr_cfg_account_4_pg_6_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_4_pg_6_t(string name = "cap_pbc_csr_cfg_account_4_pg_6_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_4_pg_6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_4_pg_6_t
    
class cap_pbc_csr_cfg_account_4_pg_5_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_4_pg_5_t(string name = "cap_pbc_csr_cfg_account_4_pg_5_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_4_pg_5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_4_pg_5_t
    
class cap_pbc_csr_cfg_account_4_pg_4_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_4_pg_4_t(string name = "cap_pbc_csr_cfg_account_4_pg_4_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_4_pg_4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_4_pg_4_t
    
class cap_pbc_csr_cfg_account_4_pg_3_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_4_pg_3_t(string name = "cap_pbc_csr_cfg_account_4_pg_3_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_4_pg_3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_4_pg_3_t
    
class cap_pbc_csr_cfg_account_4_pg_2_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_4_pg_2_t(string name = "cap_pbc_csr_cfg_account_4_pg_2_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_4_pg_2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_4_pg_2_t
    
class cap_pbc_csr_cfg_account_4_pg_1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_4_pg_1_t(string name = "cap_pbc_csr_cfg_account_4_pg_1_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_4_pg_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_4_pg_1_t
    
class cap_pbc_csr_cfg_account_4_pg_0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_4_pg_0_t(string name = "cap_pbc_csr_cfg_account_4_pg_0_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_4_pg_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_4_pg_0_t
    
class cap_pbc_csr_cfg_account_4_mtu_table_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_4_mtu_table_t(string name = "cap_pbc_csr_cfg_account_4_mtu_table_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_4_mtu_table_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 5 > pg0_cpp_int_t;
        pg0_cpp_int_t int_var__pg0;
        void pg0 (const cpp_int  & l__val);
        cpp_int pg0() const;
    
        typedef pu_cpp_int< 5 > pg1_cpp_int_t;
        pg1_cpp_int_t int_var__pg1;
        void pg1 (const cpp_int  & l__val);
        cpp_int pg1() const;
    
        typedef pu_cpp_int< 5 > pg2_cpp_int_t;
        pg2_cpp_int_t int_var__pg2;
        void pg2 (const cpp_int  & l__val);
        cpp_int pg2() const;
    
        typedef pu_cpp_int< 5 > pg3_cpp_int_t;
        pg3_cpp_int_t int_var__pg3;
        void pg3 (const cpp_int  & l__val);
        cpp_int pg3() const;
    
        typedef pu_cpp_int< 5 > pg4_cpp_int_t;
        pg4_cpp_int_t int_var__pg4;
        void pg4 (const cpp_int  & l__val);
        cpp_int pg4() const;
    
        typedef pu_cpp_int< 5 > pg5_cpp_int_t;
        pg5_cpp_int_t int_var__pg5;
        void pg5 (const cpp_int  & l__val);
        cpp_int pg5() const;
    
        typedef pu_cpp_int< 5 > pg6_cpp_int_t;
        pg6_cpp_int_t int_var__pg6;
        void pg6 (const cpp_int  & l__val);
        cpp_int pg6() const;
    
        typedef pu_cpp_int< 5 > pg7_cpp_int_t;
        pg7_cpp_int_t int_var__pg7;
        void pg7 (const cpp_int  & l__val);
        cpp_int pg7() const;
    
}; // cap_pbc_csr_cfg_account_4_mtu_table_t
    
class cap_pbc_csr_cfg_account_4_pause_timer_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_4_pause_timer_t(string name = "cap_pbc_csr_cfg_account_4_pause_timer_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_4_pause_timer_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > xoff_timeout_cpp_int_t;
        xoff_timeout_cpp_int_t int_var__xoff_timeout;
        void xoff_timeout (const cpp_int  & l__val);
        cpp_int xoff_timeout() const;
    
}; // cap_pbc_csr_cfg_account_4_pause_timer_t
    
class cap_pbc_csr_sta_account_4_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_sta_account_4_t(string name = "cap_pbc_csr_sta_account_4_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_sta_account_4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 14 > occupancy_0_cpp_int_t;
        occupancy_0_cpp_int_t int_var__occupancy_0;
        void occupancy_0 (const cpp_int  & l__val);
        cpp_int occupancy_0() const;
    
        typedef pu_cpp_int< 14 > occupancy_1_cpp_int_t;
        occupancy_1_cpp_int_t int_var__occupancy_1;
        void occupancy_1 (const cpp_int  & l__val);
        cpp_int occupancy_1() const;
    
        typedef pu_cpp_int< 14 > occupancy_2_cpp_int_t;
        occupancy_2_cpp_int_t int_var__occupancy_2;
        void occupancy_2 (const cpp_int  & l__val);
        cpp_int occupancy_2() const;
    
        typedef pu_cpp_int< 14 > occupancy_3_cpp_int_t;
        occupancy_3_cpp_int_t int_var__occupancy_3;
        void occupancy_3 (const cpp_int  & l__val);
        cpp_int occupancy_3() const;
    
        typedef pu_cpp_int< 14 > occupancy_4_cpp_int_t;
        occupancy_4_cpp_int_t int_var__occupancy_4;
        void occupancy_4 (const cpp_int  & l__val);
        cpp_int occupancy_4() const;
    
        typedef pu_cpp_int< 14 > occupancy_5_cpp_int_t;
        occupancy_5_cpp_int_t int_var__occupancy_5;
        void occupancy_5 (const cpp_int  & l__val);
        cpp_int occupancy_5() const;
    
        typedef pu_cpp_int< 14 > occupancy_6_cpp_int_t;
        occupancy_6_cpp_int_t int_var__occupancy_6;
        void occupancy_6 (const cpp_int  & l__val);
        cpp_int occupancy_6() const;
    
        typedef pu_cpp_int< 14 > occupancy_7_cpp_int_t;
        occupancy_7_cpp_int_t int_var__occupancy_7;
        void occupancy_7 (const cpp_int  & l__val);
        cpp_int occupancy_7() const;
    
}; // cap_pbc_csr_sta_account_4_t
    
class cap_pbc_csr_cfg_account_4_tc_to_pg_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_4_tc_to_pg_t(string name = "cap_pbc_csr_cfg_account_4_tc_to_pg_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_4_tc_to_pg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 24 > table_cpp_int_t;
        table_cpp_int_t int_var__table;
        void table (const cpp_int  & l__val);
        cpp_int table() const;
    
}; // cap_pbc_csr_cfg_account_4_tc_to_pg_t
    
class cap_pbc_csr_cfg_write_control_4_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_write_control_4_t(string name = "cap_pbc_csr_cfg_write_control_4_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_write_control_4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        enable_cpp_int_t int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 5 > cut_thru_cpp_int_t;
        cut_thru_cpp_int_t int_var__cut_thru;
        void cut_thru (const cpp_int  & l__val);
        cpp_int cut_thru() const;
    
        typedef pu_cpp_int< 1 > packing_cpp_int_t;
        packing_cpp_int_t int_var__packing;
        void packing (const cpp_int  & l__val);
        cpp_int packing() const;
    
        typedef pu_cpp_int< 1 > packing_same_pg_cpp_int_t;
        packing_same_pg_cpp_int_t int_var__packing_same_pg;
        void packing_same_pg (const cpp_int  & l__val);
        cpp_int packing_same_pg() const;
    
        typedef pu_cpp_int< 10 > packing_timeout_cycles_cpp_int_t;
        packing_timeout_cycles_cpp_int_t int_var__packing_timeout_cycles;
        void packing_timeout_cycles (const cpp_int  & l__val);
        cpp_int packing_timeout_cycles() const;
    
        typedef pu_cpp_int< 1 > rewrite_cpp_int_t;
        rewrite_cpp_int_t int_var__rewrite;
        void rewrite (const cpp_int  & l__val);
        cpp_int rewrite() const;
    
        typedef pu_cpp_int< 3 > rate_limiter_cpp_int_t;
        rate_limiter_cpp_int_t int_var__rate_limiter;
        void rate_limiter (const cpp_int  & l__val);
        cpp_int rate_limiter() const;
    
        typedef pu_cpp_int< 1 > drop_on_error_cpp_int_t;
        drop_on_error_cpp_int_t int_var__drop_on_error;
        void drop_on_error (const cpp_int  & l__val);
        cpp_int drop_on_error() const;
    
}; // cap_pbc_csr_cfg_write_control_4_t
    
class cap_pbc_csr_cfg_oq_3_arb_l2_strict_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_3_arb_l2_strict_t(string name = "cap_pbc_csr_cfg_oq_3_arb_l2_strict_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_3_arb_l2_strict_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > priority_cpp_int_t;
        priority_cpp_int_t int_var__priority;
        void priority (const cpp_int  & l__val);
        cpp_int priority() const;
    
}; // cap_pbc_csr_cfg_oq_3_arb_l2_strict_t
    
class cap_pbc_csr_cfg_oq_3_arb_l2_selection_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_3_arb_l2_selection_t(string name = "cap_pbc_csr_cfg_oq_3_arb_l2_selection_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_3_arb_l2_selection_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > node_0_cpp_int_t;
        node_0_cpp_int_t int_var__node_0;
        void node_0 (const cpp_int  & l__val);
        cpp_int node_0() const;
    
        typedef pu_cpp_int< 16 > node_1_cpp_int_t;
        node_1_cpp_int_t int_var__node_1;
        void node_1 (const cpp_int  & l__val);
        cpp_int node_1() const;
    
        typedef pu_cpp_int< 16 > node_2_cpp_int_t;
        node_2_cpp_int_t int_var__node_2;
        void node_2 (const cpp_int  & l__val);
        cpp_int node_2() const;
    
        typedef pu_cpp_int< 16 > node_3_cpp_int_t;
        node_3_cpp_int_t int_var__node_3;
        void node_3 (const cpp_int  & l__val);
        cpp_int node_3() const;
    
}; // cap_pbc_csr_cfg_oq_3_arb_l2_selection_t
    
class cap_pbc_csr_cfg_oq_3_arb_l1_strict_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_3_arb_l1_strict_t(string name = "cap_pbc_csr_cfg_oq_3_arb_l1_strict_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_3_arb_l1_strict_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > priority_cpp_int_t;
        priority_cpp_int_t int_var__priority;
        void priority (const cpp_int  & l__val);
        cpp_int priority() const;
    
}; // cap_pbc_csr_cfg_oq_3_arb_l1_strict_t
    
class cap_pbc_csr_cfg_oq_3_arb_l1_selection_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_3_arb_l1_selection_t(string name = "cap_pbc_csr_cfg_oq_3_arb_l1_selection_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_3_arb_l1_selection_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > node_0_cpp_int_t;
        node_0_cpp_int_t int_var__node_0;
        void node_0 (const cpp_int  & l__val);
        cpp_int node_0() const;
    
        typedef pu_cpp_int< 32 > node_1_cpp_int_t;
        node_1_cpp_int_t int_var__node_1;
        void node_1 (const cpp_int  & l__val);
        cpp_int node_1() const;
    
        typedef pu_cpp_int< 32 > node_2_cpp_int_t;
        node_2_cpp_int_t int_var__node_2;
        void node_2 (const cpp_int  & l__val);
        cpp_int node_2() const;
    
        typedef pu_cpp_int< 32 > node_3_cpp_int_t;
        node_3_cpp_int_t int_var__node_3;
        void node_3 (const cpp_int  & l__val);
        cpp_int node_3() const;
    
        typedef pu_cpp_int< 32 > node_4_cpp_int_t;
        node_4_cpp_int_t int_var__node_4;
        void node_4 (const cpp_int  & l__val);
        cpp_int node_4() const;
    
        typedef pu_cpp_int< 32 > node_5_cpp_int_t;
        node_5_cpp_int_t int_var__node_5;
        void node_5 (const cpp_int  & l__val);
        cpp_int node_5() const;
    
        typedef pu_cpp_int< 32 > node_6_cpp_int_t;
        node_6_cpp_int_t int_var__node_6;
        void node_6 (const cpp_int  & l__val);
        cpp_int node_6() const;
    
        typedef pu_cpp_int< 32 > node_7_cpp_int_t;
        node_7_cpp_int_t int_var__node_7;
        void node_7 (const cpp_int  & l__val);
        cpp_int node_7() const;
    
        typedef pu_cpp_int< 32 > node_8_cpp_int_t;
        node_8_cpp_int_t int_var__node_8;
        void node_8 (const cpp_int  & l__val);
        cpp_int node_8() const;
    
        typedef pu_cpp_int< 32 > node_9_cpp_int_t;
        node_9_cpp_int_t int_var__node_9;
        void node_9 (const cpp_int  & l__val);
        cpp_int node_9() const;
    
        typedef pu_cpp_int< 32 > node_10_cpp_int_t;
        node_10_cpp_int_t int_var__node_10;
        void node_10 (const cpp_int  & l__val);
        cpp_int node_10() const;
    
        typedef pu_cpp_int< 32 > node_11_cpp_int_t;
        node_11_cpp_int_t int_var__node_11;
        void node_11 (const cpp_int  & l__val);
        cpp_int node_11() const;
    
        typedef pu_cpp_int< 32 > node_12_cpp_int_t;
        node_12_cpp_int_t int_var__node_12;
        void node_12 (const cpp_int  & l__val);
        cpp_int node_12() const;
    
        typedef pu_cpp_int< 32 > node_13_cpp_int_t;
        node_13_cpp_int_t int_var__node_13;
        void node_13 (const cpp_int  & l__val);
        cpp_int node_13() const;
    
        typedef pu_cpp_int< 32 > node_14_cpp_int_t;
        node_14_cpp_int_t int_var__node_14;
        void node_14 (const cpp_int  & l__val);
        cpp_int node_14() const;
    
        typedef pu_cpp_int< 32 > node_15_cpp_int_t;
        node_15_cpp_int_t int_var__node_15;
        void node_15 (const cpp_int  & l__val);
        cpp_int node_15() const;
    
}; // cap_pbc_csr_cfg_oq_3_arb_l1_selection_t
    
class cap_pbc_csr_cfg_oq_3_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_3_t(string name = "cap_pbc_csr_cfg_oq_3_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        enable_cpp_int_t int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        ecc_disable_det_cpp_int_t int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        ecc_disable_cor_cpp_int_t int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        dhs_eccbypass_cpp_int_t int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 7 > num_hdr_bytes_cpp_int_t;
        num_hdr_bytes_cpp_int_t int_var__num_hdr_bytes;
        void num_hdr_bytes (const cpp_int  & l__val);
        cpp_int num_hdr_bytes() const;
    
        typedef pu_cpp_int< 1 > rewrite_enable_cpp_int_t;
        rewrite_enable_cpp_int_t int_var__rewrite_enable;
        void rewrite_enable (const cpp_int  & l__val);
        cpp_int rewrite_enable() const;
    
        typedef pu_cpp_int< 5 > ipg_bytes_cpp_int_t;
        ipg_bytes_cpp_int_t int_var__ipg_bytes;
        void ipg_bytes (const cpp_int  & l__val);
        cpp_int ipg_bytes() const;
    
}; // cap_pbc_csr_cfg_oq_3_t
    
class cap_pbc_csr_cfg_account_3_pg_7_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_3_pg_7_t(string name = "cap_pbc_csr_cfg_account_3_pg_7_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_3_pg_7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_3_pg_7_t
    
class cap_pbc_csr_cfg_account_3_pg_6_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_3_pg_6_t(string name = "cap_pbc_csr_cfg_account_3_pg_6_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_3_pg_6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_3_pg_6_t
    
class cap_pbc_csr_cfg_account_3_pg_5_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_3_pg_5_t(string name = "cap_pbc_csr_cfg_account_3_pg_5_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_3_pg_5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_3_pg_5_t
    
class cap_pbc_csr_cfg_account_3_pg_4_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_3_pg_4_t(string name = "cap_pbc_csr_cfg_account_3_pg_4_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_3_pg_4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_3_pg_4_t
    
class cap_pbc_csr_cfg_account_3_pg_3_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_3_pg_3_t(string name = "cap_pbc_csr_cfg_account_3_pg_3_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_3_pg_3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_3_pg_3_t
    
class cap_pbc_csr_cfg_account_3_pg_2_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_3_pg_2_t(string name = "cap_pbc_csr_cfg_account_3_pg_2_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_3_pg_2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_3_pg_2_t
    
class cap_pbc_csr_cfg_account_3_pg_1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_3_pg_1_t(string name = "cap_pbc_csr_cfg_account_3_pg_1_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_3_pg_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_3_pg_1_t
    
class cap_pbc_csr_cfg_account_3_pg_0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_3_pg_0_t(string name = "cap_pbc_csr_cfg_account_3_pg_0_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_3_pg_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_3_pg_0_t
    
class cap_pbc_csr_cfg_account_3_mtu_table_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_3_mtu_table_t(string name = "cap_pbc_csr_cfg_account_3_mtu_table_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_3_mtu_table_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 5 > pg0_cpp_int_t;
        pg0_cpp_int_t int_var__pg0;
        void pg0 (const cpp_int  & l__val);
        cpp_int pg0() const;
    
        typedef pu_cpp_int< 5 > pg1_cpp_int_t;
        pg1_cpp_int_t int_var__pg1;
        void pg1 (const cpp_int  & l__val);
        cpp_int pg1() const;
    
        typedef pu_cpp_int< 5 > pg2_cpp_int_t;
        pg2_cpp_int_t int_var__pg2;
        void pg2 (const cpp_int  & l__val);
        cpp_int pg2() const;
    
        typedef pu_cpp_int< 5 > pg3_cpp_int_t;
        pg3_cpp_int_t int_var__pg3;
        void pg3 (const cpp_int  & l__val);
        cpp_int pg3() const;
    
        typedef pu_cpp_int< 5 > pg4_cpp_int_t;
        pg4_cpp_int_t int_var__pg4;
        void pg4 (const cpp_int  & l__val);
        cpp_int pg4() const;
    
        typedef pu_cpp_int< 5 > pg5_cpp_int_t;
        pg5_cpp_int_t int_var__pg5;
        void pg5 (const cpp_int  & l__val);
        cpp_int pg5() const;
    
        typedef pu_cpp_int< 5 > pg6_cpp_int_t;
        pg6_cpp_int_t int_var__pg6;
        void pg6 (const cpp_int  & l__val);
        cpp_int pg6() const;
    
        typedef pu_cpp_int< 5 > pg7_cpp_int_t;
        pg7_cpp_int_t int_var__pg7;
        void pg7 (const cpp_int  & l__val);
        cpp_int pg7() const;
    
}; // cap_pbc_csr_cfg_account_3_mtu_table_t
    
class cap_pbc_csr_cfg_account_3_pause_timer_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_3_pause_timer_t(string name = "cap_pbc_csr_cfg_account_3_pause_timer_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_3_pause_timer_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > xoff_timeout_cpp_int_t;
        xoff_timeout_cpp_int_t int_var__xoff_timeout;
        void xoff_timeout (const cpp_int  & l__val);
        cpp_int xoff_timeout() const;
    
}; // cap_pbc_csr_cfg_account_3_pause_timer_t
    
class cap_pbc_csr_sta_account_3_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_sta_account_3_t(string name = "cap_pbc_csr_sta_account_3_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_sta_account_3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 14 > occupancy_0_cpp_int_t;
        occupancy_0_cpp_int_t int_var__occupancy_0;
        void occupancy_0 (const cpp_int  & l__val);
        cpp_int occupancy_0() const;
    
        typedef pu_cpp_int< 14 > occupancy_1_cpp_int_t;
        occupancy_1_cpp_int_t int_var__occupancy_1;
        void occupancy_1 (const cpp_int  & l__val);
        cpp_int occupancy_1() const;
    
        typedef pu_cpp_int< 14 > occupancy_2_cpp_int_t;
        occupancy_2_cpp_int_t int_var__occupancy_2;
        void occupancy_2 (const cpp_int  & l__val);
        cpp_int occupancy_2() const;
    
        typedef pu_cpp_int< 14 > occupancy_3_cpp_int_t;
        occupancy_3_cpp_int_t int_var__occupancy_3;
        void occupancy_3 (const cpp_int  & l__val);
        cpp_int occupancy_3() const;
    
        typedef pu_cpp_int< 14 > occupancy_4_cpp_int_t;
        occupancy_4_cpp_int_t int_var__occupancy_4;
        void occupancy_4 (const cpp_int  & l__val);
        cpp_int occupancy_4() const;
    
        typedef pu_cpp_int< 14 > occupancy_5_cpp_int_t;
        occupancy_5_cpp_int_t int_var__occupancy_5;
        void occupancy_5 (const cpp_int  & l__val);
        cpp_int occupancy_5() const;
    
        typedef pu_cpp_int< 14 > occupancy_6_cpp_int_t;
        occupancy_6_cpp_int_t int_var__occupancy_6;
        void occupancy_6 (const cpp_int  & l__val);
        cpp_int occupancy_6() const;
    
        typedef pu_cpp_int< 14 > occupancy_7_cpp_int_t;
        occupancy_7_cpp_int_t int_var__occupancy_7;
        void occupancy_7 (const cpp_int  & l__val);
        cpp_int occupancy_7() const;
    
}; // cap_pbc_csr_sta_account_3_t
    
class cap_pbc_csr_cfg_account_3_tc_to_pg_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_3_tc_to_pg_t(string name = "cap_pbc_csr_cfg_account_3_tc_to_pg_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_3_tc_to_pg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 24 > table_cpp_int_t;
        table_cpp_int_t int_var__table;
        void table (const cpp_int  & l__val);
        cpp_int table() const;
    
}; // cap_pbc_csr_cfg_account_3_tc_to_pg_t
    
class cap_pbc_csr_cfg_write_control_3_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_write_control_3_t(string name = "cap_pbc_csr_cfg_write_control_3_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_write_control_3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        enable_cpp_int_t int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 5 > cut_thru_cpp_int_t;
        cut_thru_cpp_int_t int_var__cut_thru;
        void cut_thru (const cpp_int  & l__val);
        cpp_int cut_thru() const;
    
        typedef pu_cpp_int< 1 > packing_cpp_int_t;
        packing_cpp_int_t int_var__packing;
        void packing (const cpp_int  & l__val);
        cpp_int packing() const;
    
        typedef pu_cpp_int< 1 > packing_same_pg_cpp_int_t;
        packing_same_pg_cpp_int_t int_var__packing_same_pg;
        void packing_same_pg (const cpp_int  & l__val);
        cpp_int packing_same_pg() const;
    
        typedef pu_cpp_int< 10 > packing_timeout_cycles_cpp_int_t;
        packing_timeout_cycles_cpp_int_t int_var__packing_timeout_cycles;
        void packing_timeout_cycles (const cpp_int  & l__val);
        cpp_int packing_timeout_cycles() const;
    
        typedef pu_cpp_int< 1 > rewrite_cpp_int_t;
        rewrite_cpp_int_t int_var__rewrite;
        void rewrite (const cpp_int  & l__val);
        cpp_int rewrite() const;
    
        typedef pu_cpp_int< 3 > rate_limiter_cpp_int_t;
        rate_limiter_cpp_int_t int_var__rate_limiter;
        void rate_limiter (const cpp_int  & l__val);
        cpp_int rate_limiter() const;
    
        typedef pu_cpp_int< 1 > drop_on_error_cpp_int_t;
        drop_on_error_cpp_int_t int_var__drop_on_error;
        void drop_on_error (const cpp_int  & l__val);
        cpp_int drop_on_error() const;
    
}; // cap_pbc_csr_cfg_write_control_3_t
    
class cap_pbc_csr_cfg_oq_2_arb_l2_strict_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_2_arb_l2_strict_t(string name = "cap_pbc_csr_cfg_oq_2_arb_l2_strict_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_2_arb_l2_strict_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > priority_cpp_int_t;
        priority_cpp_int_t int_var__priority;
        void priority (const cpp_int  & l__val);
        cpp_int priority() const;
    
}; // cap_pbc_csr_cfg_oq_2_arb_l2_strict_t
    
class cap_pbc_csr_cfg_oq_2_arb_l2_selection_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_2_arb_l2_selection_t(string name = "cap_pbc_csr_cfg_oq_2_arb_l2_selection_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_2_arb_l2_selection_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > node_0_cpp_int_t;
        node_0_cpp_int_t int_var__node_0;
        void node_0 (const cpp_int  & l__val);
        cpp_int node_0() const;
    
        typedef pu_cpp_int< 16 > node_1_cpp_int_t;
        node_1_cpp_int_t int_var__node_1;
        void node_1 (const cpp_int  & l__val);
        cpp_int node_1() const;
    
        typedef pu_cpp_int< 16 > node_2_cpp_int_t;
        node_2_cpp_int_t int_var__node_2;
        void node_2 (const cpp_int  & l__val);
        cpp_int node_2() const;
    
        typedef pu_cpp_int< 16 > node_3_cpp_int_t;
        node_3_cpp_int_t int_var__node_3;
        void node_3 (const cpp_int  & l__val);
        cpp_int node_3() const;
    
}; // cap_pbc_csr_cfg_oq_2_arb_l2_selection_t
    
class cap_pbc_csr_cfg_oq_2_arb_l1_strict_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_2_arb_l1_strict_t(string name = "cap_pbc_csr_cfg_oq_2_arb_l1_strict_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_2_arb_l1_strict_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > priority_cpp_int_t;
        priority_cpp_int_t int_var__priority;
        void priority (const cpp_int  & l__val);
        cpp_int priority() const;
    
}; // cap_pbc_csr_cfg_oq_2_arb_l1_strict_t
    
class cap_pbc_csr_cfg_oq_2_arb_l1_selection_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_2_arb_l1_selection_t(string name = "cap_pbc_csr_cfg_oq_2_arb_l1_selection_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_2_arb_l1_selection_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > node_0_cpp_int_t;
        node_0_cpp_int_t int_var__node_0;
        void node_0 (const cpp_int  & l__val);
        cpp_int node_0() const;
    
        typedef pu_cpp_int< 32 > node_1_cpp_int_t;
        node_1_cpp_int_t int_var__node_1;
        void node_1 (const cpp_int  & l__val);
        cpp_int node_1() const;
    
        typedef pu_cpp_int< 32 > node_2_cpp_int_t;
        node_2_cpp_int_t int_var__node_2;
        void node_2 (const cpp_int  & l__val);
        cpp_int node_2() const;
    
        typedef pu_cpp_int< 32 > node_3_cpp_int_t;
        node_3_cpp_int_t int_var__node_3;
        void node_3 (const cpp_int  & l__val);
        cpp_int node_3() const;
    
        typedef pu_cpp_int< 32 > node_4_cpp_int_t;
        node_4_cpp_int_t int_var__node_4;
        void node_4 (const cpp_int  & l__val);
        cpp_int node_4() const;
    
        typedef pu_cpp_int< 32 > node_5_cpp_int_t;
        node_5_cpp_int_t int_var__node_5;
        void node_5 (const cpp_int  & l__val);
        cpp_int node_5() const;
    
        typedef pu_cpp_int< 32 > node_6_cpp_int_t;
        node_6_cpp_int_t int_var__node_6;
        void node_6 (const cpp_int  & l__val);
        cpp_int node_6() const;
    
        typedef pu_cpp_int< 32 > node_7_cpp_int_t;
        node_7_cpp_int_t int_var__node_7;
        void node_7 (const cpp_int  & l__val);
        cpp_int node_7() const;
    
        typedef pu_cpp_int< 32 > node_8_cpp_int_t;
        node_8_cpp_int_t int_var__node_8;
        void node_8 (const cpp_int  & l__val);
        cpp_int node_8() const;
    
        typedef pu_cpp_int< 32 > node_9_cpp_int_t;
        node_9_cpp_int_t int_var__node_9;
        void node_9 (const cpp_int  & l__val);
        cpp_int node_9() const;
    
        typedef pu_cpp_int< 32 > node_10_cpp_int_t;
        node_10_cpp_int_t int_var__node_10;
        void node_10 (const cpp_int  & l__val);
        cpp_int node_10() const;
    
        typedef pu_cpp_int< 32 > node_11_cpp_int_t;
        node_11_cpp_int_t int_var__node_11;
        void node_11 (const cpp_int  & l__val);
        cpp_int node_11() const;
    
        typedef pu_cpp_int< 32 > node_12_cpp_int_t;
        node_12_cpp_int_t int_var__node_12;
        void node_12 (const cpp_int  & l__val);
        cpp_int node_12() const;
    
        typedef pu_cpp_int< 32 > node_13_cpp_int_t;
        node_13_cpp_int_t int_var__node_13;
        void node_13 (const cpp_int  & l__val);
        cpp_int node_13() const;
    
        typedef pu_cpp_int< 32 > node_14_cpp_int_t;
        node_14_cpp_int_t int_var__node_14;
        void node_14 (const cpp_int  & l__val);
        cpp_int node_14() const;
    
        typedef pu_cpp_int< 32 > node_15_cpp_int_t;
        node_15_cpp_int_t int_var__node_15;
        void node_15 (const cpp_int  & l__val);
        cpp_int node_15() const;
    
}; // cap_pbc_csr_cfg_oq_2_arb_l1_selection_t
    
class cap_pbc_csr_cfg_oq_2_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_2_t(string name = "cap_pbc_csr_cfg_oq_2_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        enable_cpp_int_t int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        ecc_disable_det_cpp_int_t int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        ecc_disable_cor_cpp_int_t int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        dhs_eccbypass_cpp_int_t int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 7 > num_hdr_bytes_cpp_int_t;
        num_hdr_bytes_cpp_int_t int_var__num_hdr_bytes;
        void num_hdr_bytes (const cpp_int  & l__val);
        cpp_int num_hdr_bytes() const;
    
        typedef pu_cpp_int< 1 > rewrite_enable_cpp_int_t;
        rewrite_enable_cpp_int_t int_var__rewrite_enable;
        void rewrite_enable (const cpp_int  & l__val);
        cpp_int rewrite_enable() const;
    
        typedef pu_cpp_int< 5 > ipg_bytes_cpp_int_t;
        ipg_bytes_cpp_int_t int_var__ipg_bytes;
        void ipg_bytes (const cpp_int  & l__val);
        cpp_int ipg_bytes() const;
    
}; // cap_pbc_csr_cfg_oq_2_t
    
class cap_pbc_csr_cfg_account_2_pg_7_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_2_pg_7_t(string name = "cap_pbc_csr_cfg_account_2_pg_7_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_2_pg_7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_2_pg_7_t
    
class cap_pbc_csr_cfg_account_2_pg_6_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_2_pg_6_t(string name = "cap_pbc_csr_cfg_account_2_pg_6_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_2_pg_6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_2_pg_6_t
    
class cap_pbc_csr_cfg_account_2_pg_5_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_2_pg_5_t(string name = "cap_pbc_csr_cfg_account_2_pg_5_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_2_pg_5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_2_pg_5_t
    
class cap_pbc_csr_cfg_account_2_pg_4_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_2_pg_4_t(string name = "cap_pbc_csr_cfg_account_2_pg_4_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_2_pg_4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_2_pg_4_t
    
class cap_pbc_csr_cfg_account_2_pg_3_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_2_pg_3_t(string name = "cap_pbc_csr_cfg_account_2_pg_3_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_2_pg_3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_2_pg_3_t
    
class cap_pbc_csr_cfg_account_2_pg_2_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_2_pg_2_t(string name = "cap_pbc_csr_cfg_account_2_pg_2_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_2_pg_2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_2_pg_2_t
    
class cap_pbc_csr_cfg_account_2_pg_1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_2_pg_1_t(string name = "cap_pbc_csr_cfg_account_2_pg_1_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_2_pg_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_2_pg_1_t
    
class cap_pbc_csr_cfg_account_2_pg_0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_2_pg_0_t(string name = "cap_pbc_csr_cfg_account_2_pg_0_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_2_pg_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_2_pg_0_t
    
class cap_pbc_csr_cfg_account_2_mtu_table_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_2_mtu_table_t(string name = "cap_pbc_csr_cfg_account_2_mtu_table_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_2_mtu_table_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 5 > pg0_cpp_int_t;
        pg0_cpp_int_t int_var__pg0;
        void pg0 (const cpp_int  & l__val);
        cpp_int pg0() const;
    
        typedef pu_cpp_int< 5 > pg1_cpp_int_t;
        pg1_cpp_int_t int_var__pg1;
        void pg1 (const cpp_int  & l__val);
        cpp_int pg1() const;
    
        typedef pu_cpp_int< 5 > pg2_cpp_int_t;
        pg2_cpp_int_t int_var__pg2;
        void pg2 (const cpp_int  & l__val);
        cpp_int pg2() const;
    
        typedef pu_cpp_int< 5 > pg3_cpp_int_t;
        pg3_cpp_int_t int_var__pg3;
        void pg3 (const cpp_int  & l__val);
        cpp_int pg3() const;
    
        typedef pu_cpp_int< 5 > pg4_cpp_int_t;
        pg4_cpp_int_t int_var__pg4;
        void pg4 (const cpp_int  & l__val);
        cpp_int pg4() const;
    
        typedef pu_cpp_int< 5 > pg5_cpp_int_t;
        pg5_cpp_int_t int_var__pg5;
        void pg5 (const cpp_int  & l__val);
        cpp_int pg5() const;
    
        typedef pu_cpp_int< 5 > pg6_cpp_int_t;
        pg6_cpp_int_t int_var__pg6;
        void pg6 (const cpp_int  & l__val);
        cpp_int pg6() const;
    
        typedef pu_cpp_int< 5 > pg7_cpp_int_t;
        pg7_cpp_int_t int_var__pg7;
        void pg7 (const cpp_int  & l__val);
        cpp_int pg7() const;
    
}; // cap_pbc_csr_cfg_account_2_mtu_table_t
    
class cap_pbc_csr_cfg_account_2_pause_timer_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_2_pause_timer_t(string name = "cap_pbc_csr_cfg_account_2_pause_timer_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_2_pause_timer_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > xoff_timeout_cpp_int_t;
        xoff_timeout_cpp_int_t int_var__xoff_timeout;
        void xoff_timeout (const cpp_int  & l__val);
        cpp_int xoff_timeout() const;
    
}; // cap_pbc_csr_cfg_account_2_pause_timer_t
    
class cap_pbc_csr_sta_account_2_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_sta_account_2_t(string name = "cap_pbc_csr_sta_account_2_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_sta_account_2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 14 > occupancy_0_cpp_int_t;
        occupancy_0_cpp_int_t int_var__occupancy_0;
        void occupancy_0 (const cpp_int  & l__val);
        cpp_int occupancy_0() const;
    
        typedef pu_cpp_int< 14 > occupancy_1_cpp_int_t;
        occupancy_1_cpp_int_t int_var__occupancy_1;
        void occupancy_1 (const cpp_int  & l__val);
        cpp_int occupancy_1() const;
    
        typedef pu_cpp_int< 14 > occupancy_2_cpp_int_t;
        occupancy_2_cpp_int_t int_var__occupancy_2;
        void occupancy_2 (const cpp_int  & l__val);
        cpp_int occupancy_2() const;
    
        typedef pu_cpp_int< 14 > occupancy_3_cpp_int_t;
        occupancy_3_cpp_int_t int_var__occupancy_3;
        void occupancy_3 (const cpp_int  & l__val);
        cpp_int occupancy_3() const;
    
        typedef pu_cpp_int< 14 > occupancy_4_cpp_int_t;
        occupancy_4_cpp_int_t int_var__occupancy_4;
        void occupancy_4 (const cpp_int  & l__val);
        cpp_int occupancy_4() const;
    
        typedef pu_cpp_int< 14 > occupancy_5_cpp_int_t;
        occupancy_5_cpp_int_t int_var__occupancy_5;
        void occupancy_5 (const cpp_int  & l__val);
        cpp_int occupancy_5() const;
    
        typedef pu_cpp_int< 14 > occupancy_6_cpp_int_t;
        occupancy_6_cpp_int_t int_var__occupancy_6;
        void occupancy_6 (const cpp_int  & l__val);
        cpp_int occupancy_6() const;
    
        typedef pu_cpp_int< 14 > occupancy_7_cpp_int_t;
        occupancy_7_cpp_int_t int_var__occupancy_7;
        void occupancy_7 (const cpp_int  & l__val);
        cpp_int occupancy_7() const;
    
}; // cap_pbc_csr_sta_account_2_t
    
class cap_pbc_csr_cfg_account_2_tc_to_pg_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_2_tc_to_pg_t(string name = "cap_pbc_csr_cfg_account_2_tc_to_pg_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_2_tc_to_pg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 24 > table_cpp_int_t;
        table_cpp_int_t int_var__table;
        void table (const cpp_int  & l__val);
        cpp_int table() const;
    
}; // cap_pbc_csr_cfg_account_2_tc_to_pg_t
    
class cap_pbc_csr_cfg_write_control_2_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_write_control_2_t(string name = "cap_pbc_csr_cfg_write_control_2_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_write_control_2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        enable_cpp_int_t int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 5 > cut_thru_cpp_int_t;
        cut_thru_cpp_int_t int_var__cut_thru;
        void cut_thru (const cpp_int  & l__val);
        cpp_int cut_thru() const;
    
        typedef pu_cpp_int< 1 > packing_cpp_int_t;
        packing_cpp_int_t int_var__packing;
        void packing (const cpp_int  & l__val);
        cpp_int packing() const;
    
        typedef pu_cpp_int< 1 > packing_same_pg_cpp_int_t;
        packing_same_pg_cpp_int_t int_var__packing_same_pg;
        void packing_same_pg (const cpp_int  & l__val);
        cpp_int packing_same_pg() const;
    
        typedef pu_cpp_int< 10 > packing_timeout_cycles_cpp_int_t;
        packing_timeout_cycles_cpp_int_t int_var__packing_timeout_cycles;
        void packing_timeout_cycles (const cpp_int  & l__val);
        cpp_int packing_timeout_cycles() const;
    
        typedef pu_cpp_int< 1 > rewrite_cpp_int_t;
        rewrite_cpp_int_t int_var__rewrite;
        void rewrite (const cpp_int  & l__val);
        cpp_int rewrite() const;
    
        typedef pu_cpp_int< 3 > rate_limiter_cpp_int_t;
        rate_limiter_cpp_int_t int_var__rate_limiter;
        void rate_limiter (const cpp_int  & l__val);
        cpp_int rate_limiter() const;
    
        typedef pu_cpp_int< 1 > drop_on_error_cpp_int_t;
        drop_on_error_cpp_int_t int_var__drop_on_error;
        void drop_on_error (const cpp_int  & l__val);
        cpp_int drop_on_error() const;
    
}; // cap_pbc_csr_cfg_write_control_2_t
    
class cap_pbc_csr_cfg_oq_1_arb_l2_strict_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_1_arb_l2_strict_t(string name = "cap_pbc_csr_cfg_oq_1_arb_l2_strict_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_1_arb_l2_strict_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > priority_cpp_int_t;
        priority_cpp_int_t int_var__priority;
        void priority (const cpp_int  & l__val);
        cpp_int priority() const;
    
}; // cap_pbc_csr_cfg_oq_1_arb_l2_strict_t
    
class cap_pbc_csr_cfg_oq_1_arb_l2_selection_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_1_arb_l2_selection_t(string name = "cap_pbc_csr_cfg_oq_1_arb_l2_selection_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_1_arb_l2_selection_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > node_0_cpp_int_t;
        node_0_cpp_int_t int_var__node_0;
        void node_0 (const cpp_int  & l__val);
        cpp_int node_0() const;
    
        typedef pu_cpp_int< 16 > node_1_cpp_int_t;
        node_1_cpp_int_t int_var__node_1;
        void node_1 (const cpp_int  & l__val);
        cpp_int node_1() const;
    
        typedef pu_cpp_int< 16 > node_2_cpp_int_t;
        node_2_cpp_int_t int_var__node_2;
        void node_2 (const cpp_int  & l__val);
        cpp_int node_2() const;
    
        typedef pu_cpp_int< 16 > node_3_cpp_int_t;
        node_3_cpp_int_t int_var__node_3;
        void node_3 (const cpp_int  & l__val);
        cpp_int node_3() const;
    
}; // cap_pbc_csr_cfg_oq_1_arb_l2_selection_t
    
class cap_pbc_csr_cfg_oq_1_arb_l1_strict_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_1_arb_l1_strict_t(string name = "cap_pbc_csr_cfg_oq_1_arb_l1_strict_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_1_arb_l1_strict_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > priority_cpp_int_t;
        priority_cpp_int_t int_var__priority;
        void priority (const cpp_int  & l__val);
        cpp_int priority() const;
    
}; // cap_pbc_csr_cfg_oq_1_arb_l1_strict_t
    
class cap_pbc_csr_cfg_oq_1_arb_l1_selection_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_1_arb_l1_selection_t(string name = "cap_pbc_csr_cfg_oq_1_arb_l1_selection_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_1_arb_l1_selection_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > node_0_cpp_int_t;
        node_0_cpp_int_t int_var__node_0;
        void node_0 (const cpp_int  & l__val);
        cpp_int node_0() const;
    
        typedef pu_cpp_int< 32 > node_1_cpp_int_t;
        node_1_cpp_int_t int_var__node_1;
        void node_1 (const cpp_int  & l__val);
        cpp_int node_1() const;
    
        typedef pu_cpp_int< 32 > node_2_cpp_int_t;
        node_2_cpp_int_t int_var__node_2;
        void node_2 (const cpp_int  & l__val);
        cpp_int node_2() const;
    
        typedef pu_cpp_int< 32 > node_3_cpp_int_t;
        node_3_cpp_int_t int_var__node_3;
        void node_3 (const cpp_int  & l__val);
        cpp_int node_3() const;
    
        typedef pu_cpp_int< 32 > node_4_cpp_int_t;
        node_4_cpp_int_t int_var__node_4;
        void node_4 (const cpp_int  & l__val);
        cpp_int node_4() const;
    
        typedef pu_cpp_int< 32 > node_5_cpp_int_t;
        node_5_cpp_int_t int_var__node_5;
        void node_5 (const cpp_int  & l__val);
        cpp_int node_5() const;
    
        typedef pu_cpp_int< 32 > node_6_cpp_int_t;
        node_6_cpp_int_t int_var__node_6;
        void node_6 (const cpp_int  & l__val);
        cpp_int node_6() const;
    
        typedef pu_cpp_int< 32 > node_7_cpp_int_t;
        node_7_cpp_int_t int_var__node_7;
        void node_7 (const cpp_int  & l__val);
        cpp_int node_7() const;
    
        typedef pu_cpp_int< 32 > node_8_cpp_int_t;
        node_8_cpp_int_t int_var__node_8;
        void node_8 (const cpp_int  & l__val);
        cpp_int node_8() const;
    
        typedef pu_cpp_int< 32 > node_9_cpp_int_t;
        node_9_cpp_int_t int_var__node_9;
        void node_9 (const cpp_int  & l__val);
        cpp_int node_9() const;
    
        typedef pu_cpp_int< 32 > node_10_cpp_int_t;
        node_10_cpp_int_t int_var__node_10;
        void node_10 (const cpp_int  & l__val);
        cpp_int node_10() const;
    
        typedef pu_cpp_int< 32 > node_11_cpp_int_t;
        node_11_cpp_int_t int_var__node_11;
        void node_11 (const cpp_int  & l__val);
        cpp_int node_11() const;
    
        typedef pu_cpp_int< 32 > node_12_cpp_int_t;
        node_12_cpp_int_t int_var__node_12;
        void node_12 (const cpp_int  & l__val);
        cpp_int node_12() const;
    
        typedef pu_cpp_int< 32 > node_13_cpp_int_t;
        node_13_cpp_int_t int_var__node_13;
        void node_13 (const cpp_int  & l__val);
        cpp_int node_13() const;
    
        typedef pu_cpp_int< 32 > node_14_cpp_int_t;
        node_14_cpp_int_t int_var__node_14;
        void node_14 (const cpp_int  & l__val);
        cpp_int node_14() const;
    
        typedef pu_cpp_int< 32 > node_15_cpp_int_t;
        node_15_cpp_int_t int_var__node_15;
        void node_15 (const cpp_int  & l__val);
        cpp_int node_15() const;
    
}; // cap_pbc_csr_cfg_oq_1_arb_l1_selection_t
    
class cap_pbc_csr_cfg_oq_1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_1_t(string name = "cap_pbc_csr_cfg_oq_1_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        enable_cpp_int_t int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        ecc_disable_det_cpp_int_t int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        ecc_disable_cor_cpp_int_t int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        dhs_eccbypass_cpp_int_t int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 7 > num_hdr_bytes_cpp_int_t;
        num_hdr_bytes_cpp_int_t int_var__num_hdr_bytes;
        void num_hdr_bytes (const cpp_int  & l__val);
        cpp_int num_hdr_bytes() const;
    
        typedef pu_cpp_int< 1 > rewrite_enable_cpp_int_t;
        rewrite_enable_cpp_int_t int_var__rewrite_enable;
        void rewrite_enable (const cpp_int  & l__val);
        cpp_int rewrite_enable() const;
    
        typedef pu_cpp_int< 5 > ipg_bytes_cpp_int_t;
        ipg_bytes_cpp_int_t int_var__ipg_bytes;
        void ipg_bytes (const cpp_int  & l__val);
        cpp_int ipg_bytes() const;
    
}; // cap_pbc_csr_cfg_oq_1_t
    
class cap_pbc_csr_cfg_account_1_pg_7_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_1_pg_7_t(string name = "cap_pbc_csr_cfg_account_1_pg_7_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_1_pg_7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_1_pg_7_t
    
class cap_pbc_csr_cfg_account_1_pg_6_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_1_pg_6_t(string name = "cap_pbc_csr_cfg_account_1_pg_6_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_1_pg_6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_1_pg_6_t
    
class cap_pbc_csr_cfg_account_1_pg_5_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_1_pg_5_t(string name = "cap_pbc_csr_cfg_account_1_pg_5_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_1_pg_5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_1_pg_5_t
    
class cap_pbc_csr_cfg_account_1_pg_4_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_1_pg_4_t(string name = "cap_pbc_csr_cfg_account_1_pg_4_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_1_pg_4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_1_pg_4_t
    
class cap_pbc_csr_cfg_account_1_pg_3_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_1_pg_3_t(string name = "cap_pbc_csr_cfg_account_1_pg_3_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_1_pg_3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_1_pg_3_t
    
class cap_pbc_csr_cfg_account_1_pg_2_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_1_pg_2_t(string name = "cap_pbc_csr_cfg_account_1_pg_2_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_1_pg_2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_1_pg_2_t
    
class cap_pbc_csr_cfg_account_1_pg_1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_1_pg_1_t(string name = "cap_pbc_csr_cfg_account_1_pg_1_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_1_pg_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_1_pg_1_t
    
class cap_pbc_csr_cfg_account_1_pg_0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_1_pg_0_t(string name = "cap_pbc_csr_cfg_account_1_pg_0_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_1_pg_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_1_pg_0_t
    
class cap_pbc_csr_cfg_account_1_mtu_table_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_1_mtu_table_t(string name = "cap_pbc_csr_cfg_account_1_mtu_table_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_1_mtu_table_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 5 > pg0_cpp_int_t;
        pg0_cpp_int_t int_var__pg0;
        void pg0 (const cpp_int  & l__val);
        cpp_int pg0() const;
    
        typedef pu_cpp_int< 5 > pg1_cpp_int_t;
        pg1_cpp_int_t int_var__pg1;
        void pg1 (const cpp_int  & l__val);
        cpp_int pg1() const;
    
        typedef pu_cpp_int< 5 > pg2_cpp_int_t;
        pg2_cpp_int_t int_var__pg2;
        void pg2 (const cpp_int  & l__val);
        cpp_int pg2() const;
    
        typedef pu_cpp_int< 5 > pg3_cpp_int_t;
        pg3_cpp_int_t int_var__pg3;
        void pg3 (const cpp_int  & l__val);
        cpp_int pg3() const;
    
        typedef pu_cpp_int< 5 > pg4_cpp_int_t;
        pg4_cpp_int_t int_var__pg4;
        void pg4 (const cpp_int  & l__val);
        cpp_int pg4() const;
    
        typedef pu_cpp_int< 5 > pg5_cpp_int_t;
        pg5_cpp_int_t int_var__pg5;
        void pg5 (const cpp_int  & l__val);
        cpp_int pg5() const;
    
        typedef pu_cpp_int< 5 > pg6_cpp_int_t;
        pg6_cpp_int_t int_var__pg6;
        void pg6 (const cpp_int  & l__val);
        cpp_int pg6() const;
    
        typedef pu_cpp_int< 5 > pg7_cpp_int_t;
        pg7_cpp_int_t int_var__pg7;
        void pg7 (const cpp_int  & l__val);
        cpp_int pg7() const;
    
}; // cap_pbc_csr_cfg_account_1_mtu_table_t
    
class cap_pbc_csr_cfg_account_1_pause_timer_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_1_pause_timer_t(string name = "cap_pbc_csr_cfg_account_1_pause_timer_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_1_pause_timer_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > xoff_timeout_cpp_int_t;
        xoff_timeout_cpp_int_t int_var__xoff_timeout;
        void xoff_timeout (const cpp_int  & l__val);
        cpp_int xoff_timeout() const;
    
}; // cap_pbc_csr_cfg_account_1_pause_timer_t
    
class cap_pbc_csr_sta_account_1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_sta_account_1_t(string name = "cap_pbc_csr_sta_account_1_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_sta_account_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 14 > occupancy_0_cpp_int_t;
        occupancy_0_cpp_int_t int_var__occupancy_0;
        void occupancy_0 (const cpp_int  & l__val);
        cpp_int occupancy_0() const;
    
        typedef pu_cpp_int< 14 > occupancy_1_cpp_int_t;
        occupancy_1_cpp_int_t int_var__occupancy_1;
        void occupancy_1 (const cpp_int  & l__val);
        cpp_int occupancy_1() const;
    
        typedef pu_cpp_int< 14 > occupancy_2_cpp_int_t;
        occupancy_2_cpp_int_t int_var__occupancy_2;
        void occupancy_2 (const cpp_int  & l__val);
        cpp_int occupancy_2() const;
    
        typedef pu_cpp_int< 14 > occupancy_3_cpp_int_t;
        occupancy_3_cpp_int_t int_var__occupancy_3;
        void occupancy_3 (const cpp_int  & l__val);
        cpp_int occupancy_3() const;
    
        typedef pu_cpp_int< 14 > occupancy_4_cpp_int_t;
        occupancy_4_cpp_int_t int_var__occupancy_4;
        void occupancy_4 (const cpp_int  & l__val);
        cpp_int occupancy_4() const;
    
        typedef pu_cpp_int< 14 > occupancy_5_cpp_int_t;
        occupancy_5_cpp_int_t int_var__occupancy_5;
        void occupancy_5 (const cpp_int  & l__val);
        cpp_int occupancy_5() const;
    
        typedef pu_cpp_int< 14 > occupancy_6_cpp_int_t;
        occupancy_6_cpp_int_t int_var__occupancy_6;
        void occupancy_6 (const cpp_int  & l__val);
        cpp_int occupancy_6() const;
    
        typedef pu_cpp_int< 14 > occupancy_7_cpp_int_t;
        occupancy_7_cpp_int_t int_var__occupancy_7;
        void occupancy_7 (const cpp_int  & l__val);
        cpp_int occupancy_7() const;
    
}; // cap_pbc_csr_sta_account_1_t
    
class cap_pbc_csr_cfg_account_1_tc_to_pg_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_1_tc_to_pg_t(string name = "cap_pbc_csr_cfg_account_1_tc_to_pg_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_1_tc_to_pg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 24 > table_cpp_int_t;
        table_cpp_int_t int_var__table;
        void table (const cpp_int  & l__val);
        cpp_int table() const;
    
}; // cap_pbc_csr_cfg_account_1_tc_to_pg_t
    
class cap_pbc_csr_cfg_write_control_1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_write_control_1_t(string name = "cap_pbc_csr_cfg_write_control_1_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_write_control_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        enable_cpp_int_t int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 5 > cut_thru_cpp_int_t;
        cut_thru_cpp_int_t int_var__cut_thru;
        void cut_thru (const cpp_int  & l__val);
        cpp_int cut_thru() const;
    
        typedef pu_cpp_int< 1 > packing_cpp_int_t;
        packing_cpp_int_t int_var__packing;
        void packing (const cpp_int  & l__val);
        cpp_int packing() const;
    
        typedef pu_cpp_int< 1 > packing_same_pg_cpp_int_t;
        packing_same_pg_cpp_int_t int_var__packing_same_pg;
        void packing_same_pg (const cpp_int  & l__val);
        cpp_int packing_same_pg() const;
    
        typedef pu_cpp_int< 10 > packing_timeout_cycles_cpp_int_t;
        packing_timeout_cycles_cpp_int_t int_var__packing_timeout_cycles;
        void packing_timeout_cycles (const cpp_int  & l__val);
        cpp_int packing_timeout_cycles() const;
    
        typedef pu_cpp_int< 1 > rewrite_cpp_int_t;
        rewrite_cpp_int_t int_var__rewrite;
        void rewrite (const cpp_int  & l__val);
        cpp_int rewrite() const;
    
        typedef pu_cpp_int< 3 > rate_limiter_cpp_int_t;
        rate_limiter_cpp_int_t int_var__rate_limiter;
        void rate_limiter (const cpp_int  & l__val);
        cpp_int rate_limiter() const;
    
        typedef pu_cpp_int< 1 > drop_on_error_cpp_int_t;
        drop_on_error_cpp_int_t int_var__drop_on_error;
        void drop_on_error (const cpp_int  & l__val);
        cpp_int drop_on_error() const;
    
}; // cap_pbc_csr_cfg_write_control_1_t
    
class cap_pbc_csr_cfg_oq_0_arb_l2_strict_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_0_arb_l2_strict_t(string name = "cap_pbc_csr_cfg_oq_0_arb_l2_strict_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_0_arb_l2_strict_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > priority_cpp_int_t;
        priority_cpp_int_t int_var__priority;
        void priority (const cpp_int  & l__val);
        cpp_int priority() const;
    
}; // cap_pbc_csr_cfg_oq_0_arb_l2_strict_t
    
class cap_pbc_csr_cfg_oq_0_arb_l2_selection_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_0_arb_l2_selection_t(string name = "cap_pbc_csr_cfg_oq_0_arb_l2_selection_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_0_arb_l2_selection_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 16 > node_0_cpp_int_t;
        node_0_cpp_int_t int_var__node_0;
        void node_0 (const cpp_int  & l__val);
        cpp_int node_0() const;
    
        typedef pu_cpp_int< 16 > node_1_cpp_int_t;
        node_1_cpp_int_t int_var__node_1;
        void node_1 (const cpp_int  & l__val);
        cpp_int node_1() const;
    
        typedef pu_cpp_int< 16 > node_2_cpp_int_t;
        node_2_cpp_int_t int_var__node_2;
        void node_2 (const cpp_int  & l__val);
        cpp_int node_2() const;
    
        typedef pu_cpp_int< 16 > node_3_cpp_int_t;
        node_3_cpp_int_t int_var__node_3;
        void node_3 (const cpp_int  & l__val);
        cpp_int node_3() const;
    
}; // cap_pbc_csr_cfg_oq_0_arb_l2_selection_t
    
class cap_pbc_csr_cfg_oq_0_arb_l1_strict_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_0_arb_l1_strict_t(string name = "cap_pbc_csr_cfg_oq_0_arb_l1_strict_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_0_arb_l1_strict_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > priority_cpp_int_t;
        priority_cpp_int_t int_var__priority;
        void priority (const cpp_int  & l__val);
        cpp_int priority() const;
    
}; // cap_pbc_csr_cfg_oq_0_arb_l1_strict_t
    
class cap_pbc_csr_cfg_oq_0_arb_l1_selection_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_0_arb_l1_selection_t(string name = "cap_pbc_csr_cfg_oq_0_arb_l1_selection_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_0_arb_l1_selection_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > node_0_cpp_int_t;
        node_0_cpp_int_t int_var__node_0;
        void node_0 (const cpp_int  & l__val);
        cpp_int node_0() const;
    
        typedef pu_cpp_int< 32 > node_1_cpp_int_t;
        node_1_cpp_int_t int_var__node_1;
        void node_1 (const cpp_int  & l__val);
        cpp_int node_1() const;
    
        typedef pu_cpp_int< 32 > node_2_cpp_int_t;
        node_2_cpp_int_t int_var__node_2;
        void node_2 (const cpp_int  & l__val);
        cpp_int node_2() const;
    
        typedef pu_cpp_int< 32 > node_3_cpp_int_t;
        node_3_cpp_int_t int_var__node_3;
        void node_3 (const cpp_int  & l__val);
        cpp_int node_3() const;
    
        typedef pu_cpp_int< 32 > node_4_cpp_int_t;
        node_4_cpp_int_t int_var__node_4;
        void node_4 (const cpp_int  & l__val);
        cpp_int node_4() const;
    
        typedef pu_cpp_int< 32 > node_5_cpp_int_t;
        node_5_cpp_int_t int_var__node_5;
        void node_5 (const cpp_int  & l__val);
        cpp_int node_5() const;
    
        typedef pu_cpp_int< 32 > node_6_cpp_int_t;
        node_6_cpp_int_t int_var__node_6;
        void node_6 (const cpp_int  & l__val);
        cpp_int node_6() const;
    
        typedef pu_cpp_int< 32 > node_7_cpp_int_t;
        node_7_cpp_int_t int_var__node_7;
        void node_7 (const cpp_int  & l__val);
        cpp_int node_7() const;
    
        typedef pu_cpp_int< 32 > node_8_cpp_int_t;
        node_8_cpp_int_t int_var__node_8;
        void node_8 (const cpp_int  & l__val);
        cpp_int node_8() const;
    
        typedef pu_cpp_int< 32 > node_9_cpp_int_t;
        node_9_cpp_int_t int_var__node_9;
        void node_9 (const cpp_int  & l__val);
        cpp_int node_9() const;
    
        typedef pu_cpp_int< 32 > node_10_cpp_int_t;
        node_10_cpp_int_t int_var__node_10;
        void node_10 (const cpp_int  & l__val);
        cpp_int node_10() const;
    
        typedef pu_cpp_int< 32 > node_11_cpp_int_t;
        node_11_cpp_int_t int_var__node_11;
        void node_11 (const cpp_int  & l__val);
        cpp_int node_11() const;
    
        typedef pu_cpp_int< 32 > node_12_cpp_int_t;
        node_12_cpp_int_t int_var__node_12;
        void node_12 (const cpp_int  & l__val);
        cpp_int node_12() const;
    
        typedef pu_cpp_int< 32 > node_13_cpp_int_t;
        node_13_cpp_int_t int_var__node_13;
        void node_13 (const cpp_int  & l__val);
        cpp_int node_13() const;
    
        typedef pu_cpp_int< 32 > node_14_cpp_int_t;
        node_14_cpp_int_t int_var__node_14;
        void node_14 (const cpp_int  & l__val);
        cpp_int node_14() const;
    
        typedef pu_cpp_int< 32 > node_15_cpp_int_t;
        node_15_cpp_int_t int_var__node_15;
        void node_15 (const cpp_int  & l__val);
        cpp_int node_15() const;
    
}; // cap_pbc_csr_cfg_oq_0_arb_l1_selection_t
    
class cap_pbc_csr_cfg_oq_0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_oq_0_t(string name = "cap_pbc_csr_cfg_oq_0_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_oq_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        enable_cpp_int_t int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_det_cpp_int_t;
        ecc_disable_det_cpp_int_t int_var__ecc_disable_det;
        void ecc_disable_det (const cpp_int  & l__val);
        cpp_int ecc_disable_det() const;
    
        typedef pu_cpp_int< 1 > ecc_disable_cor_cpp_int_t;
        ecc_disable_cor_cpp_int_t int_var__ecc_disable_cor;
        void ecc_disable_cor (const cpp_int  & l__val);
        cpp_int ecc_disable_cor() const;
    
        typedef pu_cpp_int< 1 > dhs_eccbypass_cpp_int_t;
        dhs_eccbypass_cpp_int_t int_var__dhs_eccbypass;
        void dhs_eccbypass (const cpp_int  & l__val);
        cpp_int dhs_eccbypass() const;
    
        typedef pu_cpp_int< 7 > num_hdr_bytes_cpp_int_t;
        num_hdr_bytes_cpp_int_t int_var__num_hdr_bytes;
        void num_hdr_bytes (const cpp_int  & l__val);
        cpp_int num_hdr_bytes() const;
    
        typedef pu_cpp_int< 1 > rewrite_enable_cpp_int_t;
        rewrite_enable_cpp_int_t int_var__rewrite_enable;
        void rewrite_enable (const cpp_int  & l__val);
        cpp_int rewrite_enable() const;
    
        typedef pu_cpp_int< 5 > ipg_bytes_cpp_int_t;
        ipg_bytes_cpp_int_t int_var__ipg_bytes;
        void ipg_bytes (const cpp_int  & l__val);
        cpp_int ipg_bytes() const;
    
}; // cap_pbc_csr_cfg_oq_0_t
    
class cap_pbc_csr_cfg_account_0_pg_7_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_0_pg_7_t(string name = "cap_pbc_csr_cfg_account_0_pg_7_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_0_pg_7_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_0_pg_7_t
    
class cap_pbc_csr_cfg_account_0_pg_6_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_0_pg_6_t(string name = "cap_pbc_csr_cfg_account_0_pg_6_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_0_pg_6_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_0_pg_6_t
    
class cap_pbc_csr_cfg_account_0_pg_5_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_0_pg_5_t(string name = "cap_pbc_csr_cfg_account_0_pg_5_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_0_pg_5_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_0_pg_5_t
    
class cap_pbc_csr_cfg_account_0_pg_4_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_0_pg_4_t(string name = "cap_pbc_csr_cfg_account_0_pg_4_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_0_pg_4_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_0_pg_4_t
    
class cap_pbc_csr_cfg_account_0_pg_3_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_0_pg_3_t(string name = "cap_pbc_csr_cfg_account_0_pg_3_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_0_pg_3_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_0_pg_3_t
    
class cap_pbc_csr_cfg_account_0_pg_2_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_0_pg_2_t(string name = "cap_pbc_csr_cfg_account_0_pg_2_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_0_pg_2_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_0_pg_2_t
    
class cap_pbc_csr_cfg_account_0_pg_1_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_0_pg_1_t(string name = "cap_pbc_csr_cfg_account_0_pg_1_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_0_pg_1_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_0_pg_1_t
    
class cap_pbc_csr_cfg_account_0_pg_0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_0_pg_0_t(string name = "cap_pbc_csr_cfg_account_0_pg_0_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_0_pg_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 10 > reserved_min_cpp_int_t;
        reserved_min_cpp_int_t int_var__reserved_min;
        void reserved_min (const cpp_int  & l__val);
        cpp_int reserved_min() const;
    
        typedef pu_cpp_int< 14 > xon_threshold_cpp_int_t;
        xon_threshold_cpp_int_t int_var__xon_threshold;
        void xon_threshold (const cpp_int  & l__val);
        cpp_int xon_threshold() const;
    
        typedef pu_cpp_int< 12 > headroom_cpp_int_t;
        headroom_cpp_int_t int_var__headroom;
        void headroom (const cpp_int  & l__val);
        cpp_int headroom() const;
    
        typedef pu_cpp_int< 14 > low_limit_cpp_int_t;
        low_limit_cpp_int_t int_var__low_limit;
        void low_limit (const cpp_int  & l__val);
        cpp_int low_limit() const;
    
        typedef pu_cpp_int< 4 > alpha_cpp_int_t;
        alpha_cpp_int_t int_var__alpha;
        void alpha (const cpp_int  & l__val);
        cpp_int alpha() const;
    
}; // cap_pbc_csr_cfg_account_0_pg_0_t
    
class cap_pbc_csr_cfg_account_0_mtu_table_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_0_mtu_table_t(string name = "cap_pbc_csr_cfg_account_0_mtu_table_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_0_mtu_table_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 5 > pg0_cpp_int_t;
        pg0_cpp_int_t int_var__pg0;
        void pg0 (const cpp_int  & l__val);
        cpp_int pg0() const;
    
        typedef pu_cpp_int< 5 > pg1_cpp_int_t;
        pg1_cpp_int_t int_var__pg1;
        void pg1 (const cpp_int  & l__val);
        cpp_int pg1() const;
    
        typedef pu_cpp_int< 5 > pg2_cpp_int_t;
        pg2_cpp_int_t int_var__pg2;
        void pg2 (const cpp_int  & l__val);
        cpp_int pg2() const;
    
        typedef pu_cpp_int< 5 > pg3_cpp_int_t;
        pg3_cpp_int_t int_var__pg3;
        void pg3 (const cpp_int  & l__val);
        cpp_int pg3() const;
    
        typedef pu_cpp_int< 5 > pg4_cpp_int_t;
        pg4_cpp_int_t int_var__pg4;
        void pg4 (const cpp_int  & l__val);
        cpp_int pg4() const;
    
        typedef pu_cpp_int< 5 > pg5_cpp_int_t;
        pg5_cpp_int_t int_var__pg5;
        void pg5 (const cpp_int  & l__val);
        cpp_int pg5() const;
    
        typedef pu_cpp_int< 5 > pg6_cpp_int_t;
        pg6_cpp_int_t int_var__pg6;
        void pg6 (const cpp_int  & l__val);
        cpp_int pg6() const;
    
        typedef pu_cpp_int< 5 > pg7_cpp_int_t;
        pg7_cpp_int_t int_var__pg7;
        void pg7 (const cpp_int  & l__val);
        cpp_int pg7() const;
    
}; // cap_pbc_csr_cfg_account_0_mtu_table_t
    
class cap_pbc_csr_cfg_account_0_pause_timer_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_0_pause_timer_t(string name = "cap_pbc_csr_cfg_account_0_pause_timer_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_0_pause_timer_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 32 > xoff_timeout_cpp_int_t;
        xoff_timeout_cpp_int_t int_var__xoff_timeout;
        void xoff_timeout (const cpp_int  & l__val);
        cpp_int xoff_timeout() const;
    
}; // cap_pbc_csr_cfg_account_0_pause_timer_t
    
class cap_pbc_csr_sta_account_0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_sta_account_0_t(string name = "cap_pbc_csr_sta_account_0_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_sta_account_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 14 > occupancy_0_cpp_int_t;
        occupancy_0_cpp_int_t int_var__occupancy_0;
        void occupancy_0 (const cpp_int  & l__val);
        cpp_int occupancy_0() const;
    
        typedef pu_cpp_int< 14 > occupancy_1_cpp_int_t;
        occupancy_1_cpp_int_t int_var__occupancy_1;
        void occupancy_1 (const cpp_int  & l__val);
        cpp_int occupancy_1() const;
    
        typedef pu_cpp_int< 14 > occupancy_2_cpp_int_t;
        occupancy_2_cpp_int_t int_var__occupancy_2;
        void occupancy_2 (const cpp_int  & l__val);
        cpp_int occupancy_2() const;
    
        typedef pu_cpp_int< 14 > occupancy_3_cpp_int_t;
        occupancy_3_cpp_int_t int_var__occupancy_3;
        void occupancy_3 (const cpp_int  & l__val);
        cpp_int occupancy_3() const;
    
        typedef pu_cpp_int< 14 > occupancy_4_cpp_int_t;
        occupancy_4_cpp_int_t int_var__occupancy_4;
        void occupancy_4 (const cpp_int  & l__val);
        cpp_int occupancy_4() const;
    
        typedef pu_cpp_int< 14 > occupancy_5_cpp_int_t;
        occupancy_5_cpp_int_t int_var__occupancy_5;
        void occupancy_5 (const cpp_int  & l__val);
        cpp_int occupancy_5() const;
    
        typedef pu_cpp_int< 14 > occupancy_6_cpp_int_t;
        occupancy_6_cpp_int_t int_var__occupancy_6;
        void occupancy_6 (const cpp_int  & l__val);
        cpp_int occupancy_6() const;
    
        typedef pu_cpp_int< 14 > occupancy_7_cpp_int_t;
        occupancy_7_cpp_int_t int_var__occupancy_7;
        void occupancy_7 (const cpp_int  & l__val);
        cpp_int occupancy_7() const;
    
}; // cap_pbc_csr_sta_account_0_t
    
class cap_pbc_csr_cfg_account_0_tc_to_pg_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_account_0_tc_to_pg_t(string name = "cap_pbc_csr_cfg_account_0_tc_to_pg_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_account_0_tc_to_pg_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 24 > table_cpp_int_t;
        table_cpp_int_t int_var__table;
        void table (const cpp_int  & l__val);
        cpp_int table() const;
    
}; // cap_pbc_csr_cfg_account_0_tc_to_pg_t
    
class cap_pbc_csr_cfg_write_control_0_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_write_control_0_t(string name = "cap_pbc_csr_cfg_write_control_0_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_write_control_0_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > enable_cpp_int_t;
        enable_cpp_int_t int_var__enable;
        void enable (const cpp_int  & l__val);
        cpp_int enable() const;
    
        typedef pu_cpp_int< 5 > cut_thru_cpp_int_t;
        cut_thru_cpp_int_t int_var__cut_thru;
        void cut_thru (const cpp_int  & l__val);
        cpp_int cut_thru() const;
    
        typedef pu_cpp_int< 1 > packing_cpp_int_t;
        packing_cpp_int_t int_var__packing;
        void packing (const cpp_int  & l__val);
        cpp_int packing() const;
    
        typedef pu_cpp_int< 1 > packing_same_pg_cpp_int_t;
        packing_same_pg_cpp_int_t int_var__packing_same_pg;
        void packing_same_pg (const cpp_int  & l__val);
        cpp_int packing_same_pg() const;
    
        typedef pu_cpp_int< 10 > packing_timeout_cycles_cpp_int_t;
        packing_timeout_cycles_cpp_int_t int_var__packing_timeout_cycles;
        void packing_timeout_cycles (const cpp_int  & l__val);
        cpp_int packing_timeout_cycles() const;
    
        typedef pu_cpp_int< 1 > rewrite_cpp_int_t;
        rewrite_cpp_int_t int_var__rewrite;
        void rewrite (const cpp_int  & l__val);
        cpp_int rewrite() const;
    
        typedef pu_cpp_int< 3 > rate_limiter_cpp_int_t;
        rate_limiter_cpp_int_t int_var__rate_limiter;
        void rate_limiter (const cpp_int  & l__val);
        cpp_int rate_limiter() const;
    
        typedef pu_cpp_int< 1 > drop_on_error_cpp_int_t;
        drop_on_error_cpp_int_t int_var__drop_on_error;
        void drop_on_error (const cpp_int  & l__val);
        cpp_int drop_on_error() const;
    
}; // cap_pbc_csr_cfg_write_control_0_t
    
class cap_pbc_csr_cfg_pbc_control_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_cfg_pbc_control_t(string name = "cap_pbc_csr_cfg_pbc_control_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_cfg_pbc_control_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        typedef pu_cpp_int< 1 > sw_reset_cpp_int_t;
        sw_reset_cpp_int_t int_var__sw_reset;
        void sw_reset (const cpp_int  & l__val);
        cpp_int sw_reset() const;
    
}; // cap_pbc_csr_cfg_pbc_control_t
    
class cap_pbc_csr_base_t : public cap_register_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_base_t(string name = "cap_pbc_csr_base_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_base_t();
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
    
}; // cap_pbc_csr_base_t
    
class cap_pbc_csr_t : public cap_block_base {
    public:
        cpp_int_helper hlp;
    
        cap_pbc_csr_t(string name = "cap_pbc_csr_t", cap_csr_base *parent = 0);
        virtual ~cap_pbc_csr_t();
        virtual void init();
        virtual int get_width() const;
        static int s_get_width() ;
        virtual void show();
        virtual void all (const cpp_int & l__val);
        virtual cpp_int all() const;
    
        cap_pbc_csr_base_t base;
    
        cap_pbc_csr_cfg_pbc_control_t cfg_pbc_control;
    
        cap_pbc_csr_cfg_write_control_0_t cfg_write_control_0;
    
        cap_pbc_csr_cfg_account_0_tc_to_pg_t cfg_account_0_tc_to_pg;
    
        cap_pbc_csr_sta_account_0_t sta_account_0;
    
        cap_pbc_csr_cfg_account_0_pause_timer_t cfg_account_0_pause_timer;
    
        cap_pbc_csr_cfg_account_0_mtu_table_t cfg_account_0_mtu_table;
    
        cap_pbc_csr_cfg_account_0_pg_0_t cfg_account_0_pg_0;
    
        cap_pbc_csr_cfg_account_0_pg_1_t cfg_account_0_pg_1;
    
        cap_pbc_csr_cfg_account_0_pg_2_t cfg_account_0_pg_2;
    
        cap_pbc_csr_cfg_account_0_pg_3_t cfg_account_0_pg_3;
    
        cap_pbc_csr_cfg_account_0_pg_4_t cfg_account_0_pg_4;
    
        cap_pbc_csr_cfg_account_0_pg_5_t cfg_account_0_pg_5;
    
        cap_pbc_csr_cfg_account_0_pg_6_t cfg_account_0_pg_6;
    
        cap_pbc_csr_cfg_account_0_pg_7_t cfg_account_0_pg_7;
    
        cap_pbc_csr_cfg_oq_0_t cfg_oq_0;
    
        cap_pbc_csr_cfg_oq_0_arb_l1_selection_t cfg_oq_0_arb_l1_selection;
    
        cap_pbc_csr_cfg_oq_0_arb_l1_strict_t cfg_oq_0_arb_l1_strict;
    
        cap_pbc_csr_cfg_oq_0_arb_l2_selection_t cfg_oq_0_arb_l2_selection;
    
        cap_pbc_csr_cfg_oq_0_arb_l2_strict_t cfg_oq_0_arb_l2_strict;
    
        cap_pbc_csr_cfg_write_control_1_t cfg_write_control_1;
    
        cap_pbc_csr_cfg_account_1_tc_to_pg_t cfg_account_1_tc_to_pg;
    
        cap_pbc_csr_sta_account_1_t sta_account_1;
    
        cap_pbc_csr_cfg_account_1_pause_timer_t cfg_account_1_pause_timer;
    
        cap_pbc_csr_cfg_account_1_mtu_table_t cfg_account_1_mtu_table;
    
        cap_pbc_csr_cfg_account_1_pg_0_t cfg_account_1_pg_0;
    
        cap_pbc_csr_cfg_account_1_pg_1_t cfg_account_1_pg_1;
    
        cap_pbc_csr_cfg_account_1_pg_2_t cfg_account_1_pg_2;
    
        cap_pbc_csr_cfg_account_1_pg_3_t cfg_account_1_pg_3;
    
        cap_pbc_csr_cfg_account_1_pg_4_t cfg_account_1_pg_4;
    
        cap_pbc_csr_cfg_account_1_pg_5_t cfg_account_1_pg_5;
    
        cap_pbc_csr_cfg_account_1_pg_6_t cfg_account_1_pg_6;
    
        cap_pbc_csr_cfg_account_1_pg_7_t cfg_account_1_pg_7;
    
        cap_pbc_csr_cfg_oq_1_t cfg_oq_1;
    
        cap_pbc_csr_cfg_oq_1_arb_l1_selection_t cfg_oq_1_arb_l1_selection;
    
        cap_pbc_csr_cfg_oq_1_arb_l1_strict_t cfg_oq_1_arb_l1_strict;
    
        cap_pbc_csr_cfg_oq_1_arb_l2_selection_t cfg_oq_1_arb_l2_selection;
    
        cap_pbc_csr_cfg_oq_1_arb_l2_strict_t cfg_oq_1_arb_l2_strict;
    
        cap_pbc_csr_cfg_write_control_2_t cfg_write_control_2;
    
        cap_pbc_csr_cfg_account_2_tc_to_pg_t cfg_account_2_tc_to_pg;
    
        cap_pbc_csr_sta_account_2_t sta_account_2;
    
        cap_pbc_csr_cfg_account_2_pause_timer_t cfg_account_2_pause_timer;
    
        cap_pbc_csr_cfg_account_2_mtu_table_t cfg_account_2_mtu_table;
    
        cap_pbc_csr_cfg_account_2_pg_0_t cfg_account_2_pg_0;
    
        cap_pbc_csr_cfg_account_2_pg_1_t cfg_account_2_pg_1;
    
        cap_pbc_csr_cfg_account_2_pg_2_t cfg_account_2_pg_2;
    
        cap_pbc_csr_cfg_account_2_pg_3_t cfg_account_2_pg_3;
    
        cap_pbc_csr_cfg_account_2_pg_4_t cfg_account_2_pg_4;
    
        cap_pbc_csr_cfg_account_2_pg_5_t cfg_account_2_pg_5;
    
        cap_pbc_csr_cfg_account_2_pg_6_t cfg_account_2_pg_6;
    
        cap_pbc_csr_cfg_account_2_pg_7_t cfg_account_2_pg_7;
    
        cap_pbc_csr_cfg_oq_2_t cfg_oq_2;
    
        cap_pbc_csr_cfg_oq_2_arb_l1_selection_t cfg_oq_2_arb_l1_selection;
    
        cap_pbc_csr_cfg_oq_2_arb_l1_strict_t cfg_oq_2_arb_l1_strict;
    
        cap_pbc_csr_cfg_oq_2_arb_l2_selection_t cfg_oq_2_arb_l2_selection;
    
        cap_pbc_csr_cfg_oq_2_arb_l2_strict_t cfg_oq_2_arb_l2_strict;
    
        cap_pbc_csr_cfg_write_control_3_t cfg_write_control_3;
    
        cap_pbc_csr_cfg_account_3_tc_to_pg_t cfg_account_3_tc_to_pg;
    
        cap_pbc_csr_sta_account_3_t sta_account_3;
    
        cap_pbc_csr_cfg_account_3_pause_timer_t cfg_account_3_pause_timer;
    
        cap_pbc_csr_cfg_account_3_mtu_table_t cfg_account_3_mtu_table;
    
        cap_pbc_csr_cfg_account_3_pg_0_t cfg_account_3_pg_0;
    
        cap_pbc_csr_cfg_account_3_pg_1_t cfg_account_3_pg_1;
    
        cap_pbc_csr_cfg_account_3_pg_2_t cfg_account_3_pg_2;
    
        cap_pbc_csr_cfg_account_3_pg_3_t cfg_account_3_pg_3;
    
        cap_pbc_csr_cfg_account_3_pg_4_t cfg_account_3_pg_4;
    
        cap_pbc_csr_cfg_account_3_pg_5_t cfg_account_3_pg_5;
    
        cap_pbc_csr_cfg_account_3_pg_6_t cfg_account_3_pg_6;
    
        cap_pbc_csr_cfg_account_3_pg_7_t cfg_account_3_pg_7;
    
        cap_pbc_csr_cfg_oq_3_t cfg_oq_3;
    
        cap_pbc_csr_cfg_oq_3_arb_l1_selection_t cfg_oq_3_arb_l1_selection;
    
        cap_pbc_csr_cfg_oq_3_arb_l1_strict_t cfg_oq_3_arb_l1_strict;
    
        cap_pbc_csr_cfg_oq_3_arb_l2_selection_t cfg_oq_3_arb_l2_selection;
    
        cap_pbc_csr_cfg_oq_3_arb_l2_strict_t cfg_oq_3_arb_l2_strict;
    
        cap_pbc_csr_cfg_write_control_4_t cfg_write_control_4;
    
        cap_pbc_csr_cfg_account_4_tc_to_pg_t cfg_account_4_tc_to_pg;
    
        cap_pbc_csr_sta_account_4_t sta_account_4;
    
        cap_pbc_csr_cfg_account_4_pause_timer_t cfg_account_4_pause_timer;
    
        cap_pbc_csr_cfg_account_4_mtu_table_t cfg_account_4_mtu_table;
    
        cap_pbc_csr_cfg_account_4_pg_0_t cfg_account_4_pg_0;
    
        cap_pbc_csr_cfg_account_4_pg_1_t cfg_account_4_pg_1;
    
        cap_pbc_csr_cfg_account_4_pg_2_t cfg_account_4_pg_2;
    
        cap_pbc_csr_cfg_account_4_pg_3_t cfg_account_4_pg_3;
    
        cap_pbc_csr_cfg_account_4_pg_4_t cfg_account_4_pg_4;
    
        cap_pbc_csr_cfg_account_4_pg_5_t cfg_account_4_pg_5;
    
        cap_pbc_csr_cfg_account_4_pg_6_t cfg_account_4_pg_6;
    
        cap_pbc_csr_cfg_account_4_pg_7_t cfg_account_4_pg_7;
    
        cap_pbc_csr_cfg_oq_4_t cfg_oq_4;
    
        cap_pbc_csr_cfg_oq_4_arb_l1_selection_t cfg_oq_4_arb_l1_selection;
    
        cap_pbc_csr_cfg_oq_4_arb_l1_strict_t cfg_oq_4_arb_l1_strict;
    
        cap_pbc_csr_cfg_oq_4_arb_l2_selection_t cfg_oq_4_arb_l2_selection;
    
        cap_pbc_csr_cfg_oq_4_arb_l2_strict_t cfg_oq_4_arb_l2_strict;
    
        cap_pbc_csr_cfg_write_control_5_t cfg_write_control_5;
    
        cap_pbc_csr_cfg_account_5_tc_to_pg_t cfg_account_5_tc_to_pg;
    
        cap_pbc_csr_sta_account_5_t sta_account_5;
    
        cap_pbc_csr_cfg_account_5_pause_timer_t cfg_account_5_pause_timer;
    
        cap_pbc_csr_cfg_account_5_mtu_table_t cfg_account_5_mtu_table;
    
        cap_pbc_csr_cfg_account_5_pg_0_t cfg_account_5_pg_0;
    
        cap_pbc_csr_cfg_account_5_pg_1_t cfg_account_5_pg_1;
    
        cap_pbc_csr_cfg_account_5_pg_2_t cfg_account_5_pg_2;
    
        cap_pbc_csr_cfg_account_5_pg_3_t cfg_account_5_pg_3;
    
        cap_pbc_csr_cfg_account_5_pg_4_t cfg_account_5_pg_4;
    
        cap_pbc_csr_cfg_account_5_pg_5_t cfg_account_5_pg_5;
    
        cap_pbc_csr_cfg_account_5_pg_6_t cfg_account_5_pg_6;
    
        cap_pbc_csr_cfg_account_5_pg_7_t cfg_account_5_pg_7;
    
        cap_pbc_csr_cfg_oq_5_t cfg_oq_5;
    
        cap_pbc_csr_cfg_oq_5_arb_l1_selection_t cfg_oq_5_arb_l1_selection;
    
        cap_pbc_csr_cfg_oq_5_arb_l1_strict_t cfg_oq_5_arb_l1_strict;
    
        cap_pbc_csr_cfg_oq_5_arb_l2_selection_t cfg_oq_5_arb_l2_selection;
    
        cap_pbc_csr_cfg_oq_5_arb_l2_strict_t cfg_oq_5_arb_l2_strict;
    
        cap_pbc_csr_cfg_write_control_6_t cfg_write_control_6;
    
        cap_pbc_csr_cfg_account_6_tc_to_pg_t cfg_account_6_tc_to_pg;
    
        cap_pbc_csr_sta_account_6_t sta_account_6;
    
        cap_pbc_csr_cfg_account_6_pause_timer_t cfg_account_6_pause_timer;
    
        cap_pbc_csr_cfg_account_6_mtu_table_t cfg_account_6_mtu_table;
    
        cap_pbc_csr_cfg_account_6_pg_0_t cfg_account_6_pg_0;
    
        cap_pbc_csr_cfg_account_6_pg_1_t cfg_account_6_pg_1;
    
        cap_pbc_csr_cfg_account_6_pg_2_t cfg_account_6_pg_2;
    
        cap_pbc_csr_cfg_account_6_pg_3_t cfg_account_6_pg_3;
    
        cap_pbc_csr_cfg_account_6_pg_4_t cfg_account_6_pg_4;
    
        cap_pbc_csr_cfg_account_6_pg_5_t cfg_account_6_pg_5;
    
        cap_pbc_csr_cfg_account_6_pg_6_t cfg_account_6_pg_6;
    
        cap_pbc_csr_cfg_account_6_pg_7_t cfg_account_6_pg_7;
    
        cap_pbc_csr_cfg_oq_6_t cfg_oq_6;
    
        cap_pbc_csr_cfg_oq_6_arb_l1_selection_t cfg_oq_6_arb_l1_selection;
    
        cap_pbc_csr_cfg_oq_6_arb_l1_strict_t cfg_oq_6_arb_l1_strict;
    
        cap_pbc_csr_cfg_oq_6_arb_l2_selection_t cfg_oq_6_arb_l2_selection;
    
        cap_pbc_csr_cfg_oq_6_arb_l2_strict_t cfg_oq_6_arb_l2_strict;
    
        cap_pbc_csr_cfg_write_control_7_t cfg_write_control_7;
    
        cap_pbc_csr_cfg_account_7_tc_to_pg_t cfg_account_7_tc_to_pg;
    
        cap_pbc_csr_sta_account_7_t sta_account_7;
    
        cap_pbc_csr_cfg_account_7_pause_timer_t cfg_account_7_pause_timer;
    
        cap_pbc_csr_cfg_account_7_mtu_table_t cfg_account_7_mtu_table;
    
        cap_pbc_csr_cfg_account_7_pg_0_t cfg_account_7_pg_0;
    
        cap_pbc_csr_cfg_account_7_pg_1_t cfg_account_7_pg_1;
    
        cap_pbc_csr_cfg_account_7_pg_2_t cfg_account_7_pg_2;
    
        cap_pbc_csr_cfg_account_7_pg_3_t cfg_account_7_pg_3;
    
        cap_pbc_csr_cfg_account_7_pg_4_t cfg_account_7_pg_4;
    
        cap_pbc_csr_cfg_account_7_pg_5_t cfg_account_7_pg_5;
    
        cap_pbc_csr_cfg_account_7_pg_6_t cfg_account_7_pg_6;
    
        cap_pbc_csr_cfg_account_7_pg_7_t cfg_account_7_pg_7;
    
        cap_pbc_csr_cfg_oq_7_t cfg_oq_7;
    
        cap_pbc_csr_cfg_oq_7_arb_l1_selection_t cfg_oq_7_arb_l1_selection;
    
        cap_pbc_csr_cfg_oq_7_arb_l1_strict_t cfg_oq_7_arb_l1_strict;
    
        cap_pbc_csr_cfg_oq_7_arb_l2_selection_t cfg_oq_7_arb_l2_selection;
    
        cap_pbc_csr_cfg_oq_7_arb_l2_strict_t cfg_oq_7_arb_l2_strict;
    
        cap_pbc_csr_cfg_write_control_8_t cfg_write_control_8;
    
        cap_pbc_csr_cfg_account_8_tc_to_pg_t cfg_account_8_tc_to_pg;
    
        cap_pbc_csr_sta_account_8_t sta_account_8;
    
        cap_pbc_csr_cfg_account_8_pause_timer_t cfg_account_8_pause_timer;
    
        cap_pbc_csr_cfg_account_8_mtu_table_t cfg_account_8_mtu_table;
    
        cap_pbc_csr_cfg_account_8_pg_0_t cfg_account_8_pg_0;
    
        cap_pbc_csr_cfg_account_8_pg_1_t cfg_account_8_pg_1;
    
        cap_pbc_csr_cfg_account_8_pg_2_t cfg_account_8_pg_2;
    
        cap_pbc_csr_cfg_account_8_pg_3_t cfg_account_8_pg_3;
    
        cap_pbc_csr_cfg_account_8_pg_4_t cfg_account_8_pg_4;
    
        cap_pbc_csr_cfg_account_8_pg_5_t cfg_account_8_pg_5;
    
        cap_pbc_csr_cfg_account_8_pg_6_t cfg_account_8_pg_6;
    
        cap_pbc_csr_cfg_account_8_pg_7_t cfg_account_8_pg_7;
    
        cap_pbc_csr_cfg_oq_8_t cfg_oq_8;
    
        cap_pbc_csr_cfg_oq_8_arb_l1_selection_t cfg_oq_8_arb_l1_selection;
    
        cap_pbc_csr_cfg_oq_8_arb_l1_strict_t cfg_oq_8_arb_l1_strict;
    
        cap_pbc_csr_cfg_oq_8_arb_l2_selection_t cfg_oq_8_arb_l2_selection;
    
        cap_pbc_csr_cfg_oq_8_arb_l2_strict_t cfg_oq_8_arb_l2_strict;
    
        cap_pbc_csr_cfg_write_control_9_t cfg_write_control_9;
    
        cap_pbc_csr_cfg_account_9_tc_to_pg_t cfg_account_9_tc_to_pg;
    
        cap_pbc_csr_sta_account_9_t sta_account_9;
    
        cap_pbc_csr_cfg_account_9_pause_timer_t cfg_account_9_pause_timer;
    
        cap_pbc_csr_cfg_account_9_mtu_table_t cfg_account_9_mtu_table;
    
        cap_pbc_csr_cfg_account_9_pg_0_t cfg_account_9_pg_0;
    
        cap_pbc_csr_cfg_account_9_pg_1_t cfg_account_9_pg_1;
    
        cap_pbc_csr_cfg_account_9_pg_2_t cfg_account_9_pg_2;
    
        cap_pbc_csr_cfg_account_9_pg_3_t cfg_account_9_pg_3;
    
        cap_pbc_csr_cfg_account_9_pg_4_t cfg_account_9_pg_4;
    
        cap_pbc_csr_cfg_account_9_pg_5_t cfg_account_9_pg_5;
    
        cap_pbc_csr_cfg_account_9_pg_6_t cfg_account_9_pg_6;
    
        cap_pbc_csr_cfg_account_9_pg_7_t cfg_account_9_pg_7;
    
        cap_pbc_csr_cfg_account_9_pg_8_t cfg_account_9_pg_8;
    
        cap_pbc_csr_cfg_account_9_pg_9_t cfg_account_9_pg_9;
    
        cap_pbc_csr_cfg_account_9_pg_10_t cfg_account_9_pg_10;
    
        cap_pbc_csr_cfg_account_9_pg_11_t cfg_account_9_pg_11;
    
        cap_pbc_csr_cfg_account_9_pg_12_t cfg_account_9_pg_12;
    
        cap_pbc_csr_cfg_account_9_pg_13_t cfg_account_9_pg_13;
    
        cap_pbc_csr_cfg_account_9_pg_14_t cfg_account_9_pg_14;
    
        cap_pbc_csr_cfg_account_9_pg_15_t cfg_account_9_pg_15;
    
        cap_pbc_csr_cfg_oq_9_t cfg_oq_9;
    
        cap_pbc_csr_cfg_oq_9_arb_l1_selection_t cfg_oq_9_arb_l1_selection;
    
        cap_pbc_csr_cfg_oq_9_arb_l1_strict_t cfg_oq_9_arb_l1_strict;
    
        cap_pbc_csr_cfg_oq_9_arb_l2_selection_t cfg_oq_9_arb_l2_selection;
    
        cap_pbc_csr_cfg_oq_9_arb_l2_strict_t cfg_oq_9_arb_l2_strict;
    
        cap_pbc_csr_cfg_write_control_10_t cfg_write_control_10;
    
        cap_pbc_csr_cfg_account_10_tc_to_pg_t cfg_account_10_tc_to_pg;
    
        cap_pbc_csr_sta_account_10_t sta_account_10;
    
        cap_pbc_csr_cfg_account_10_pause_timer_t cfg_account_10_pause_timer;
    
        cap_pbc_csr_cfg_account_10_mtu_table_t cfg_account_10_mtu_table;
    
        cap_pbc_csr_cfg_account_10_pg_0_t cfg_account_10_pg_0;
    
        cap_pbc_csr_cfg_account_10_pg_1_t cfg_account_10_pg_1;
    
        cap_pbc_csr_cfg_account_10_pg_2_t cfg_account_10_pg_2;
    
        cap_pbc_csr_cfg_account_10_pg_3_t cfg_account_10_pg_3;
    
        cap_pbc_csr_cfg_account_10_pg_4_t cfg_account_10_pg_4;
    
        cap_pbc_csr_cfg_account_10_pg_5_t cfg_account_10_pg_5;
    
        cap_pbc_csr_cfg_account_10_pg_6_t cfg_account_10_pg_6;
    
        cap_pbc_csr_cfg_account_10_pg_7_t cfg_account_10_pg_7;
    
        cap_pbc_csr_cfg_account_10_pg_8_t cfg_account_10_pg_8;
    
        cap_pbc_csr_cfg_account_10_pg_9_t cfg_account_10_pg_9;
    
        cap_pbc_csr_cfg_account_10_pg_10_t cfg_account_10_pg_10;
    
        cap_pbc_csr_cfg_account_10_pg_11_t cfg_account_10_pg_11;
    
        cap_pbc_csr_cfg_account_10_pg_12_t cfg_account_10_pg_12;
    
        cap_pbc_csr_cfg_account_10_pg_13_t cfg_account_10_pg_13;
    
        cap_pbc_csr_cfg_account_10_pg_14_t cfg_account_10_pg_14;
    
        cap_pbc_csr_cfg_account_10_pg_15_t cfg_account_10_pg_15;
    
        cap_pbc_csr_cfg_account_10_pg_16_t cfg_account_10_pg_16;
    
        cap_pbc_csr_cfg_account_10_pg_17_t cfg_account_10_pg_17;
    
        cap_pbc_csr_cfg_account_10_pg_18_t cfg_account_10_pg_18;
    
        cap_pbc_csr_cfg_account_10_pg_19_t cfg_account_10_pg_19;
    
        cap_pbc_csr_cfg_account_10_pg_20_t cfg_account_10_pg_20;
    
        cap_pbc_csr_cfg_account_10_pg_21_t cfg_account_10_pg_21;
    
        cap_pbc_csr_cfg_account_10_pg_22_t cfg_account_10_pg_22;
    
        cap_pbc_csr_cfg_account_10_pg_23_t cfg_account_10_pg_23;
    
        cap_pbc_csr_cfg_account_10_pg_24_t cfg_account_10_pg_24;
    
        cap_pbc_csr_cfg_account_10_pg_25_t cfg_account_10_pg_25;
    
        cap_pbc_csr_cfg_account_10_pg_26_t cfg_account_10_pg_26;
    
        cap_pbc_csr_cfg_account_10_pg_27_t cfg_account_10_pg_27;
    
        cap_pbc_csr_cfg_account_10_pg_28_t cfg_account_10_pg_28;
    
        cap_pbc_csr_cfg_account_10_pg_29_t cfg_account_10_pg_29;
    
        cap_pbc_csr_cfg_account_10_pg_30_t cfg_account_10_pg_30;
    
        cap_pbc_csr_cfg_account_10_pg_31_t cfg_account_10_pg_31;
    
        cap_pbc_csr_cfg_oq_10_t cfg_oq_10;
    
        cap_pbc_csr_cfg_oq_10_arb_l1_selection_t cfg_oq_10_arb_l1_selection;
    
        cap_pbc_csr_cfg_oq_10_arb_l1_strict_t cfg_oq_10_arb_l1_strict;
    
        cap_pbc_csr_cfg_oq_10_arb_l2_selection_t cfg_oq_10_arb_l2_selection;
    
        cap_pbc_csr_cfg_oq_10_arb_l2_strict_t cfg_oq_10_arb_l2_strict;
    
        cap_pbc_csr_cfg_write_control_11_t cfg_write_control_11;
    
        cap_pbc_csr_cfg_account_11_tc_to_pg_t cfg_account_11_tc_to_pg;
    
        cap_pbc_csr_sta_account_11_t sta_account_11;
    
        cap_pbc_csr_cfg_account_11_pause_timer_t cfg_account_11_pause_timer;
    
        cap_pbc_csr_cfg_account_11_mtu_table_t cfg_account_11_mtu_table;
    
        cap_pbc_csr_cfg_account_11_pg_0_t cfg_account_11_pg_0;
    
        cap_pbc_csr_cfg_account_11_pg_1_t cfg_account_11_pg_1;
    
        cap_pbc_csr_cfg_account_11_pg_2_t cfg_account_11_pg_2;
    
        cap_pbc_csr_cfg_account_11_pg_3_t cfg_account_11_pg_3;
    
        cap_pbc_csr_cfg_account_11_pg_4_t cfg_account_11_pg_4;
    
        cap_pbc_csr_cfg_account_11_pg_5_t cfg_account_11_pg_5;
    
        cap_pbc_csr_cfg_account_11_pg_6_t cfg_account_11_pg_6;
    
        cap_pbc_csr_cfg_account_11_pg_7_t cfg_account_11_pg_7;
    
        cap_pbc_csr_cfg_account_11_pg_8_t cfg_account_11_pg_8;
    
        cap_pbc_csr_cfg_account_11_pg_9_t cfg_account_11_pg_9;
    
        cap_pbc_csr_cfg_account_11_pg_10_t cfg_account_11_pg_10;
    
        cap_pbc_csr_cfg_account_11_pg_11_t cfg_account_11_pg_11;
    
        cap_pbc_csr_cfg_account_11_pg_12_t cfg_account_11_pg_12;
    
        cap_pbc_csr_cfg_account_11_pg_13_t cfg_account_11_pg_13;
    
        cap_pbc_csr_cfg_account_11_pg_14_t cfg_account_11_pg_14;
    
        cap_pbc_csr_cfg_account_11_pg_15_t cfg_account_11_pg_15;
    
        cap_pbc_csr_cfg_account_11_pg_16_t cfg_account_11_pg_16;
    
        cap_pbc_csr_cfg_account_11_pg_17_t cfg_account_11_pg_17;
    
        cap_pbc_csr_cfg_account_11_pg_18_t cfg_account_11_pg_18;
    
        cap_pbc_csr_cfg_account_11_pg_19_t cfg_account_11_pg_19;
    
        cap_pbc_csr_cfg_account_11_pg_20_t cfg_account_11_pg_20;
    
        cap_pbc_csr_cfg_account_11_pg_21_t cfg_account_11_pg_21;
    
        cap_pbc_csr_cfg_account_11_pg_22_t cfg_account_11_pg_22;
    
        cap_pbc_csr_cfg_account_11_pg_23_t cfg_account_11_pg_23;
    
        cap_pbc_csr_cfg_account_11_pg_24_t cfg_account_11_pg_24;
    
        cap_pbc_csr_cfg_account_11_pg_25_t cfg_account_11_pg_25;
    
        cap_pbc_csr_cfg_account_11_pg_26_t cfg_account_11_pg_26;
    
        cap_pbc_csr_cfg_account_11_pg_27_t cfg_account_11_pg_27;
    
        cap_pbc_csr_cfg_account_11_pg_28_t cfg_account_11_pg_28;
    
        cap_pbc_csr_cfg_account_11_pg_29_t cfg_account_11_pg_29;
    
        cap_pbc_csr_cfg_account_11_pg_30_t cfg_account_11_pg_30;
    
        cap_pbc_csr_cfg_account_11_pg_31_t cfg_account_11_pg_31;
    
        cap_pbc_csr_cfg_oq_11_t cfg_oq_11;
    
        cap_pbc_csr_cfg_oq_11_arb_l1_selection_t cfg_oq_11_arb_l1_selection;
    
        cap_pbc_csr_cfg_oq_11_arb_l1_strict_t cfg_oq_11_arb_l1_strict;
    
        cap_pbc_csr_cfg_oq_11_arb_l2_selection_t cfg_oq_11_arb_l2_selection;
    
        cap_pbc_csr_cfg_oq_11_arb_l2_strict_t cfg_oq_11_arb_l2_strict;
    
        cap_pbc_csr_sta_ecc_oq_t sta_ecc_oq;
    
        cap_pbc_csr_sta_ecc_pack_t sta_ecc_pack;
    
        cap_pbc_csr_sta_ecc_rwr_t sta_ecc_rwr;
    
        cap_pbc_csr_sta_fc_mgr_0_t sta_fc_mgr_0;
    
        cap_pbc_csr_cfg_fc_mgr_0_t cfg_fc_mgr_0;
    
        cap_pbc_csr_sta_ecc_fc_0_t sta_ecc_fc_0;
    
        cap_pbc_csr_cfg_ll_0_t cfg_ll_0;
    
        cap_pbc_csr_sta_ecc_ll_0_t sta_ecc_ll_0;
    
        cap_pbc_csr_cfg_desc_0_t cfg_desc_0;
    
        cap_pbc_csr_sta_ecc_desc_0_t sta_ecc_desc_0;
    
        cap_pbc_csr_sta_fc_mgr_1_t sta_fc_mgr_1;
    
        cap_pbc_csr_cfg_fc_mgr_1_t cfg_fc_mgr_1;
    
        cap_pbc_csr_sta_ecc_fc_1_t sta_ecc_fc_1;
    
        cap_pbc_csr_cfg_ll_1_t cfg_ll_1;
    
        cap_pbc_csr_sta_ecc_ll_1_t sta_ecc_ll_1;
    
        cap_pbc_csr_cfg_desc_1_t cfg_desc_1;
    
        cap_pbc_csr_sta_ecc_desc_1_t sta_ecc_desc_1;
    
        cap_pbc_csr_cfg_gc_t cfg_gc;
    
        cap_pbc_csr_cfg_rc_t cfg_rc;
    
        cap_pbc_csr_sta_rc_t sta_rc;
    
        cap_pbc_csr_cfg_sched_t cfg_sched;
    
        cap_pbc_csr_sta_sched_t sta_sched;
    
        cap_pbc_csr_cfg_island_control_t cfg_island_control;
    
        cap_pbc_csr_cfg_rpl_t cfg_rpl;
    
        cap_pbc_csr_cfg_parser0_t cfg_parser0;
    
        cap_pbc_csr_cfg_parser1_t cfg_parser1;
    
        cap_pbc_csr_cfg_parser2_t cfg_parser2;
    
        cap_pbc_csr_cfg_parser3_t cfg_parser3;
    
        cap_pbc_csr_cfg_parser4_t cfg_parser4;
    
        cap_pbc_csr_cfg_parser5_t cfg_parser5;
    
        cap_pbc_csr_cfg_parser6_t cfg_parser6;
    
        cap_pbc_csr_cfg_parser7_t cfg_parser7;
    
        cap_pbc_csr_cfg_parser8_t cfg_parser8;
    
        cap_pbc_csr_cnt_flits_t cnt_flits[12];
        int get_depth_cnt_flits() { return 12; }
    
        cap_pbc_csr_cnt_write_error_t cnt_write_error[12];
        int get_depth_cnt_write_error() { return 12; }
    
        cap_pbc_csr_sta_oq_t sta_oq[12];
        int get_depth_sta_oq() { return 12; }
    
        cap_pbc_csr_sta_oq_12_t sta_oq_12;
    
        cap_pbc_csr_cfg_dhs_t cfg_dhs;
    
        cap_pbc_csr_cfg_debug_port_t cfg_debug_port;
    
        cap_pbc_csr_cfg_spare_t cfg_spare;
    
        cap_pbc_csr_sta_ecc_sideband_t sta_ecc_sideband;
    
        cap_pbc_csr_sta_rpl_err_t sta_rpl_err;
    
        cap_pbc_csr_cfg_tail_drop_t cfg_tail_drop;
    
        cap_pbc_csr_cfg_src_port_to_lif_map_t cfg_src_port_to_lif_map;
    
        cap_pbc_csr_dhs_oq_0_t dhs_oq_0;
    
        cap_pbc_csr_dhs_oq_1_t dhs_oq_1;
    
        cap_pbc_csr_dhs_oq_2_t dhs_oq_2;
    
        cap_pbc_csr_dhs_oq_3_t dhs_oq_3;
    
        cap_pbc_csr_dhs_oq_4_t dhs_oq_4;
    
        cap_pbc_csr_dhs_oq_5_t dhs_oq_5;
    
        cap_pbc_csr_dhs_oq_6_t dhs_oq_6;
    
        cap_pbc_csr_dhs_oq_7_t dhs_oq_7;
    
        cap_pbc_csr_dhs_oq_8_t dhs_oq_8;
    
        cap_pbc_csr_dhs_oq_9_t dhs_oq_9;
    
        cap_pbc_csr_dhs_oq_10_t dhs_oq_10;
    
        cap_pbc_csr_dhs_oq_11_t dhs_oq_11;
    
        cap_pbc_csr_dhs_fc_0_t dhs_fc_0;
    
        cap_pbc_csr_dhs_ll_0_t dhs_ll_0;
    
        cap_pbc_csr_dhs_desc_0_t dhs_desc_0;
    
        cap_pbc_csr_dhs_fc_1_t dhs_fc_1;
    
        cap_pbc_csr_dhs_ll_1_t dhs_ll_1;
    
        cap_pbc_csr_dhs_desc_1_t dhs_desc_1;
    
        cap_pbc_csr_dhs_gc_t dhs_gc;
    
        cap_pbc_csr_dhs_rc_t dhs_rc;
    
        cap_pbc_csr_dhs_sched_t dhs_sched;
    
}; // cap_pbc_csr_t
    
#endif // CAP_PBC_CSR_H
        