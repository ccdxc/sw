
#ifndef CAP_PTD_DECODERS_H
#define CAP_PTD_DECODERS_H

#include "cap_csr_base.h" 

using namespace std;
class cap_ptd_decoders_cmd_mem2pkt_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_decoders_cmd_mem2pkt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_decoders_cmd_mem2pkt_t(string name = "cap_ptd_decoders_cmd_mem2pkt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_decoders_cmd_mem2pkt_t();
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
    
        typedef pu_cpp_int< 3 > cmdtype_cpp_int_t;
        cpp_int int_var__cmdtype;
        void cmdtype (const cpp_int  & l__val);
        cpp_int cmdtype() const;
    
        typedef pu_cpp_int< 1 > cmdeop_cpp_int_t;
        cpp_int int_var__cmdeop;
        void cmdeop (const cpp_int  & l__val);
        cpp_int cmdeop() const;
    
        typedef pu_cpp_int< 1 > pkteop_cpp_int_t;
        cpp_int int_var__pkteop;
        void pkteop (const cpp_int  & l__val);
        cpp_int pkteop() const;
    
        typedef pu_cpp_int< 1 > host_addr_cpp_int_t;
        cpp_int int_var__host_addr;
        void host_addr (const cpp_int  & l__val);
        cpp_int host_addr() const;
    
        typedef pu_cpp_int< 1 > cache_cpp_int_t;
        cpp_int int_var__cache;
        void cache (const cpp_int  & l__val);
        cpp_int cache() const;
    
        typedef pu_cpp_int< 1 > use_override_lif_cpp_int_t;
        cpp_int int_var__use_override_lif;
        void use_override_lif (const cpp_int  & l__val);
        cpp_int use_override_lif() const;
    
        typedef pu_cpp_int< 52 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 11 > override_lif_cpp_int_t;
        cpp_int int_var__override_lif;
        void override_lif (const cpp_int  & l__val);
        cpp_int override_lif() const;
    
        typedef pu_cpp_int< 1 > rsvd1_cpp_int_t;
        cpp_int int_var__rsvd1;
        void rsvd1 (const cpp_int  & l__val);
        cpp_int rsvd1() const;
    
        typedef pu_cpp_int< 14 > psize_cpp_int_t;
        cpp_int int_var__psize;
        void psize (const cpp_int  & l__val);
        cpp_int psize() const;
    
        typedef pu_cpp_int< 42 > rsvd_cpp_int_t;
        cpp_int int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
}; // cap_ptd_decoders_cmd_mem2pkt_t
    
class cap_ptd_decoders_cmd_phv2pkt_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_decoders_cmd_phv2pkt_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_decoders_cmd_phv2pkt_t(string name = "cap_ptd_decoders_cmd_phv2pkt_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_decoders_cmd_phv2pkt_t();
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
    
        typedef pu_cpp_int< 3 > cmdtype_cpp_int_t;
        cpp_int int_var__cmdtype;
        void cmdtype (const cpp_int  & l__val);
        cpp_int cmdtype() const;
    
        typedef pu_cpp_int< 1 > cmdeop_cpp_int_t;
        cpp_int int_var__cmdeop;
        void cmdeop (const cpp_int  & l__val);
        cpp_int cmdeop() const;
    
        typedef pu_cpp_int< 1 > pkteop_cpp_int_t;
        cpp_int int_var__pkteop;
        void pkteop (const cpp_int  & l__val);
        cpp_int pkteop() const;
    
        typedef pu_cpp_int< 2 > cmdsize_cpp_int_t;
        cpp_int int_var__cmdsize;
        void cmdsize (const cpp_int  & l__val);
        cpp_int cmdsize() const;
    
        typedef pu_cpp_int< 10 > phv_start_cpp_int_t;
        cpp_int int_var__phv_start;
        void phv_start (const cpp_int  & l__val);
        cpp_int phv_start() const;
    
        typedef pu_cpp_int< 10 > phv_end_cpp_int_t;
        cpp_int int_var__phv_end;
        void phv_end (const cpp_int  & l__val);
        cpp_int phv_end() const;
    
        typedef pu_cpp_int< 10 > phv_start1_cpp_int_t;
        cpp_int int_var__phv_start1;
        void phv_start1 (const cpp_int  & l__val);
        cpp_int phv_start1() const;
    
        typedef pu_cpp_int< 10 > phv_end1_cpp_int_t;
        cpp_int int_var__phv_end1;
        void phv_end1 (const cpp_int  & l__val);
        cpp_int phv_end1() const;
    
        typedef pu_cpp_int< 10 > phv_start2_cpp_int_t;
        cpp_int int_var__phv_start2;
        void phv_start2 (const cpp_int  & l__val);
        cpp_int phv_start2() const;
    
        typedef pu_cpp_int< 10 > phv_end2_cpp_int_t;
        cpp_int int_var__phv_end2;
        void phv_end2 (const cpp_int  & l__val);
        cpp_int phv_end2() const;
    
        typedef pu_cpp_int< 10 > phv_start3_cpp_int_t;
        cpp_int int_var__phv_start3;
        void phv_start3 (const cpp_int  & l__val);
        cpp_int phv_start3() const;
    
        typedef pu_cpp_int< 10 > phv_end3_cpp_int_t;
        cpp_int int_var__phv_end3;
        void phv_end3 (const cpp_int  & l__val);
        cpp_int phv_end3() const;
    
        typedef pu_cpp_int< 41 > rsvd_cpp_int_t;
        cpp_int int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
}; // cap_ptd_decoders_cmd_phv2pkt_t
    
class cap_ptd_decoders_cmd_phv2mem_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_decoders_cmd_phv2mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_decoders_cmd_phv2mem_t(string name = "cap_ptd_decoders_cmd_phv2mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_decoders_cmd_phv2mem_t();
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
    
        typedef pu_cpp_int< 3 > cmdtype_cpp_int_t;
        cpp_int int_var__cmdtype;
        void cmdtype (const cpp_int  & l__val);
        cpp_int cmdtype() const;
    
        typedef pu_cpp_int< 1 > cmdeop_cpp_int_t;
        cpp_int int_var__cmdeop;
        void cmdeop (const cpp_int  & l__val);
        cpp_int cmdeop() const;
    
        typedef pu_cpp_int< 1 > host_addr_cpp_int_t;
        cpp_int int_var__host_addr;
        void host_addr (const cpp_int  & l__val);
        cpp_int host_addr() const;
    
        typedef pu_cpp_int< 1 > cache_cpp_int_t;
        cpp_int int_var__cache;
        void cache (const cpp_int  & l__val);
        cpp_int cache() const;
    
        typedef pu_cpp_int< 1 > wr_data_fence_cpp_int_t;
        cpp_int int_var__wr_data_fence;
        void wr_data_fence (const cpp_int  & l__val);
        cpp_int wr_data_fence() const;
    
        typedef pu_cpp_int< 1 > wr_fence_fence_cpp_int_t;
        cpp_int int_var__wr_fence_fence;
        void wr_fence_fence (const cpp_int  & l__val);
        cpp_int wr_fence_fence() const;
    
        typedef pu_cpp_int< 10 > phv_start_cpp_int_t;
        cpp_int int_var__phv_start;
        void phv_start (const cpp_int  & l__val);
        cpp_int phv_start() const;
    
        typedef pu_cpp_int< 10 > phv_end_cpp_int_t;
        cpp_int int_var__phv_end;
        void phv_end (const cpp_int  & l__val);
        cpp_int phv_end() const;
    
        typedef pu_cpp_int< 1 > use_override_lif_cpp_int_t;
        cpp_int int_var__use_override_lif;
        void use_override_lif (const cpp_int  & l__val);
        cpp_int use_override_lif() const;
    
        typedef pu_cpp_int< 1 > pcie_msg_cpp_int_t;
        cpp_int int_var__pcie_msg;
        void pcie_msg (const cpp_int  & l__val);
        cpp_int pcie_msg() const;
    
        typedef pu_cpp_int< 1 > round_cpp_int_t;
        cpp_int int_var__round;
        void round (const cpp_int  & l__val);
        cpp_int round() const;
    
        typedef pu_cpp_int< 1 > barrier_cpp_int_t;
        cpp_int int_var__barrier;
        void barrier (const cpp_int  & l__val);
        cpp_int barrier() const;
    
        typedef pu_cpp_int< 52 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 11 > override_lif_cpp_int_t;
        cpp_int int_var__override_lif;
        void override_lif (const cpp_int  & l__val);
        cpp_int override_lif() const;
    
        typedef pu_cpp_int< 33 > rsvd_cpp_int_t;
        cpp_int int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
}; // cap_ptd_decoders_cmd_phv2mem_t
    
class cap_ptd_decoders_cmd_pkt2mem_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_decoders_cmd_pkt2mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_decoders_cmd_pkt2mem_t(string name = "cap_ptd_decoders_cmd_pkt2mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_decoders_cmd_pkt2mem_t();
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
    
        typedef pu_cpp_int< 3 > cmdtype_cpp_int_t;
        cpp_int int_var__cmdtype;
        void cmdtype (const cpp_int  & l__val);
        cpp_int cmdtype() const;
    
        typedef pu_cpp_int< 1 > cmdeop_cpp_int_t;
        cpp_int int_var__cmdeop;
        void cmdeop (const cpp_int  & l__val);
        cpp_int cmdeop() const;
    
        typedef pu_cpp_int< 1 > round_cpp_int_t;
        cpp_int int_var__round;
        void round (const cpp_int  & l__val);
        cpp_int round() const;
    
        typedef pu_cpp_int< 1 > host_addr_cpp_int_t;
        cpp_int int_var__host_addr;
        void host_addr (const cpp_int  & l__val);
        cpp_int host_addr() const;
    
        typedef pu_cpp_int< 1 > cache_cpp_int_t;
        cpp_int int_var__cache;
        void cache (const cpp_int  & l__val);
        cpp_int cache() const;
    
        typedef pu_cpp_int< 1 > use_override_lif_cpp_int_t;
        cpp_int int_var__use_override_lif;
        void use_override_lif (const cpp_int  & l__val);
        cpp_int use_override_lif() const;
    
        typedef pu_cpp_int< 52 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 11 > override_lif_cpp_int_t;
        cpp_int int_var__override_lif;
        void override_lif (const cpp_int  & l__val);
        cpp_int override_lif() const;
    
        typedef pu_cpp_int< 1 > rsvd1_cpp_int_t;
        cpp_int int_var__rsvd1;
        void rsvd1 (const cpp_int  & l__val);
        cpp_int rsvd1() const;
    
        typedef pu_cpp_int< 14 > psize_cpp_int_t;
        cpp_int int_var__psize;
        void psize (const cpp_int  & l__val);
        cpp_int psize() const;
    
        typedef pu_cpp_int< 42 > rsvd_cpp_int_t;
        cpp_int int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
}; // cap_ptd_decoders_cmd_pkt2mem_t
    
class cap_ptd_decoders_cmd_skip_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_decoders_cmd_skip_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_decoders_cmd_skip_t(string name = "cap_ptd_decoders_cmd_skip_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_decoders_cmd_skip_t();
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
    
        typedef pu_cpp_int< 3 > cmdtype_cpp_int_t;
        cpp_int int_var__cmdtype;
        void cmdtype (const cpp_int  & l__val);
        cpp_int cmdtype() const;
    
        typedef pu_cpp_int< 1 > cmdeop_cpp_int_t;
        cpp_int int_var__cmdeop;
        void cmdeop (const cpp_int  & l__val);
        cpp_int cmdeop() const;
    
        typedef pu_cpp_int< 14 > psize_cpp_int_t;
        cpp_int int_var__psize;
        void psize (const cpp_int  & l__val);
        cpp_int psize() const;
    
        typedef pu_cpp_int< 1 > skip_to_eop_cpp_int_t;
        cpp_int int_var__skip_to_eop;
        void skip_to_eop (const cpp_int  & l__val);
        cpp_int skip_to_eop() const;
    
        typedef pu_cpp_int< 109 > rsvd_cpp_int_t;
        cpp_int int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
}; // cap_ptd_decoders_cmd_skip_t
    
class cap_ptd_decoders_cmd_mem2mem_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_ptd_decoders_cmd_mem2mem_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_ptd_decoders_cmd_mem2mem_t(string name = "cap_ptd_decoders_cmd_mem2mem_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_ptd_decoders_cmd_mem2mem_t();
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
    
        typedef pu_cpp_int< 3 > cmdtype_cpp_int_t;
        cpp_int int_var__cmdtype;
        void cmdtype (const cpp_int  & l__val);
        cpp_int cmdtype() const;
    
        typedef pu_cpp_int< 1 > cmdeop_cpp_int_t;
        cpp_int int_var__cmdeop;
        void cmdeop (const cpp_int  & l__val);
        cpp_int cmdeop() const;
    
        typedef pu_cpp_int< 2 > mem2mem_type_cpp_int_t;
        cpp_int int_var__mem2mem_type;
        void mem2mem_type (const cpp_int  & l__val);
        cpp_int mem2mem_type() const;
    
        typedef pu_cpp_int< 1 > host_addr_cpp_int_t;
        cpp_int int_var__host_addr;
        void host_addr (const cpp_int  & l__val);
        cpp_int host_addr() const;
    
        typedef pu_cpp_int< 1 > cache_cpp_int_t;
        cpp_int int_var__cache;
        void cache (const cpp_int  & l__val);
        cpp_int cache() const;
    
        typedef pu_cpp_int< 1 > wr_data_fence_cpp_int_t;
        cpp_int int_var__wr_data_fence;
        void wr_data_fence (const cpp_int  & l__val);
        cpp_int wr_data_fence() const;
    
        typedef pu_cpp_int< 1 > wr_fence_fence_cpp_int_t;
        cpp_int int_var__wr_fence_fence;
        void wr_fence_fence (const cpp_int  & l__val);
        cpp_int wr_fence_fence() const;
    
        typedef pu_cpp_int< 10 > phv_start_cpp_int_t;
        cpp_int int_var__phv_start;
        void phv_start (const cpp_int  & l__val);
        cpp_int phv_start() const;
    
        typedef pu_cpp_int< 10 > phv_end_cpp_int_t;
        cpp_int int_var__phv_end;
        void phv_end (const cpp_int  & l__val);
        cpp_int phv_end() const;
    
        typedef pu_cpp_int< 1 > use_override_lif_cpp_int_t;
        cpp_int int_var__use_override_lif;
        void use_override_lif (const cpp_int  & l__val);
        cpp_int use_override_lif() const;
    
        typedef pu_cpp_int< 1 > pcie_msg_cpp_int_t;
        cpp_int int_var__pcie_msg;
        void pcie_msg (const cpp_int  & l__val);
        cpp_int pcie_msg() const;
    
        typedef pu_cpp_int< 1 > round_cpp_int_t;
        cpp_int int_var__round;
        void round (const cpp_int  & l__val);
        cpp_int round() const;
    
        typedef pu_cpp_int< 1 > barrier_cpp_int_t;
        cpp_int int_var__barrier;
        void barrier (const cpp_int  & l__val);
        cpp_int barrier() const;
    
        typedef pu_cpp_int< 52 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 11 > override_lif_cpp_int_t;
        cpp_int int_var__override_lif;
        void override_lif (const cpp_int  & l__val);
        cpp_int override_lif() const;
    
        typedef pu_cpp_int< 1 > rsvd1_cpp_int_t;
        cpp_int int_var__rsvd1;
        void rsvd1 (const cpp_int  & l__val);
        cpp_int rsvd1() const;
    
        typedef pu_cpp_int< 14 > psize_cpp_int_t;
        cpp_int int_var__psize;
        void psize (const cpp_int  & l__val);
        cpp_int psize() const;
    
        typedef pu_cpp_int< 16 > rsvd_cpp_int_t;
        cpp_int int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
}; // cap_ptd_decoders_cmd_mem2mem_t
    
#endif // CAP_PTD_DECODERS_H
        