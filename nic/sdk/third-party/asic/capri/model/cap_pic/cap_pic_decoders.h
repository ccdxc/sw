
#ifndef CAP_PIC_DECODERS_H
#define CAP_PIC_DECODERS_H

#include "cap_csr_base.h" 

using namespace std;
class cap_pic_opcode_decoder_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pic_opcode_decoder_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pic_opcode_decoder_t(string name = "cap_pic_opcode_decoder_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pic_opcode_decoder_t();
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
    
        typedef pu_cpp_int< 4 > operation_cpp_int_t;
        cpp_int int_var__operation;
        void operation (const cpp_int  & l__val);
        cpp_int operation() const;
    
        typedef pu_cpp_int< 2 > oprd1_sel_cpp_int_t;
        cpp_int int_var__oprd1_sel;
        void oprd1_sel (const cpp_int  & l__val);
        cpp_int oprd1_sel() const;
    
        typedef pu_cpp_int< 2 > oprd2_sel_cpp_int_t;
        cpp_int int_var__oprd2_sel;
        void oprd2_sel (const cpp_int  & l__val);
        cpp_int oprd2_sel() const;
    
        typedef pu_cpp_int< 2 > saturate_cpp_int_t;
        cpp_int int_var__saturate;
        void saturate (const cpp_int  & l__val);
        cpp_int saturate() const;
    
        typedef pu_cpp_int< 1 > policer_cpp_int_t;
        cpp_int int_var__policer;
        void policer (const cpp_int  & l__val);
        cpp_int policer() const;
    
}; // cap_pic_opcode_decoder_t
    
class cap_pic_policer_decoder_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pic_policer_decoder_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pic_policer_decoder_t(string name = "cap_pic_policer_decoder_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pic_policer_decoder_t();
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
    
        typedef pu_cpp_int< 40 > tbkt_cpp_int_t;
        cpp_int int_var__tbkt;
        void tbkt (const cpp_int  & l__val);
        cpp_int tbkt() const;
    
        typedef pu_cpp_int< 40 > rate_cpp_int_t;
        cpp_int int_var__rate;
        void rate (const cpp_int  & l__val);
        cpp_int rate() const;
    
        typedef pu_cpp_int< 40 > burst_cpp_int_t;
        cpp_int int_var__burst;
        void burst (const cpp_int  & l__val);
        cpp_int burst() const;
    
        typedef pu_cpp_int< 1 > axi_wr_pend_cpp_int_t;
        cpp_int int_var__axi_wr_pend;
        void axi_wr_pend (const cpp_int  & l__val);
        cpp_int axi_wr_pend() const;
    
        typedef pu_cpp_int< 1 > rsvd_cpp_int_t;
        cpp_int int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
        typedef pu_cpp_int< 1 > color_aware_cpp_int_t;
        cpp_int int_var__color_aware;
        void color_aware (const cpp_int  & l__val);
        cpp_int color_aware() const;
    
        typedef pu_cpp_int< 2 > rlimit_prof_cpp_int_t;
        cpp_int int_var__rlimit_prof;
        void rlimit_prof (const cpp_int  & l__val);
        cpp_int rlimit_prof() const;
    
        typedef pu_cpp_int< 1 > rlimit_en_cpp_int_t;
        cpp_int int_var__rlimit_en;
        void rlimit_en (const cpp_int  & l__val);
        cpp_int rlimit_en() const;
    
        typedef pu_cpp_int< 1 > pkt_rate_cpp_int_t;
        cpp_int int_var__pkt_rate;
        void pkt_rate (const cpp_int  & l__val);
        cpp_int pkt_rate() const;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        cpp_int int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
}; // cap_pic_policer_decoder_t
    
class cap_pic_bg_sm_decoder_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pic_bg_sm_decoder_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pic_bg_sm_decoder_t(string name = "cap_pic_bg_sm_decoder_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pic_bg_sm_decoder_t();
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
    
        typedef pu_cpp_int< 16 > timer_cpp_int_t;
        cpp_int int_var__timer;
        void timer (const cpp_int  & l__val);
        cpp_int timer() const;
    
        typedef pu_cpp_int< 20 > addr_cpp_int_t;
        cpp_int int_var__addr;
        void addr (const cpp_int  & l__val);
        cpp_int addr() const;
    
        typedef pu_cpp_int< 8 > cycle_cpp_int_t;
        cpp_int int_var__cycle;
        void cycle (const cpp_int  & l__val);
        cpp_int cycle() const;
    
}; // cap_pic_bg_sm_decoder_t
    
class cap_pic_rl_decoder_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pic_rl_decoder_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pic_rl_decoder_t(string name = "cap_pic_rl_decoder_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pic_rl_decoder_t();
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
    
        typedef pu_cpp_int< 4 > spare_cpp_int_t;
        cpp_int int_var__spare;
        void spare (const cpp_int  & l__val);
        cpp_int spare() const;
    
        typedef pu_cpp_int< 1 > stop_cpp_int_t;
        cpp_int int_var__stop;
        void stop (const cpp_int  & l__val);
        cpp_int stop() const;
    
        typedef pu_cpp_int< 11 > rlid_cpp_int_t;
        cpp_int int_var__rlid;
        void rlid (const cpp_int  & l__val);
        cpp_int rlid() const;
    
}; // cap_pic_rl_decoder_t
    
class cap_pic_rl_vld_decoder_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_pic_rl_vld_decoder_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_pic_rl_vld_decoder_t(string name = "cap_pic_rl_vld_decoder_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_pic_rl_vld_decoder_t();
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
    
        cap_pic_rl_decoder_t data;
    
        typedef pu_cpp_int< 1 > valid_cpp_int_t;
        cpp_int int_var__valid;
        void valid (const cpp_int  & l__val);
        cpp_int valid() const;
    
}; // cap_pic_rl_vld_decoder_t
    
#endif // CAP_PIC_DECODERS_H
        