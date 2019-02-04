
#ifndef CAP_QSTATE_DECODERS_H
#define CAP_QSTATE_DECODERS_H

#include "cap_csr_base.h" 

using namespace std;
class cap_qstate_index_pair_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_qstate_index_pair_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_qstate_index_pair_t(string name = "cap_qstate_index_pair_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_qstate_index_pair_t();
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
    
        typedef pu_cpp_int< 16 > pindex_cpp_int_t;
        cpp_int int_var__pindex;
        void pindex (const cpp_int  & l__val);
        cpp_int pindex() const;
    
        typedef pu_cpp_int< 16 > cindex_cpp_int_t;
        cpp_int int_var__cindex;
        void cindex (const cpp_int  & l__val);
        cpp_int cindex() const;
    
}; // cap_qstate_index_pair_t
    
class cap_qstate_decoder_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_qstate_decoder_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_qstate_decoder_t(string name = "cap_qstate_decoder_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_qstate_decoder_t();
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
    
        typedef pu_cpp_int< 8 > pc_cpp_int_t;
        cpp_int int_var__pc;
        void pc (const cpp_int  & l__val);
        cpp_int pc() const;
    
        typedef pu_cpp_int< 8 > rsvd_cpp_int_t;
        cpp_int int_var__rsvd;
        void rsvd (const cpp_int  & l__val);
        cpp_int rsvd() const;
    
        typedef pu_cpp_int< 4 > cosa_cpp_int_t;
        cpp_int int_var__cosa;
        void cosa (const cpp_int  & l__val);
        cpp_int cosa() const;
    
        typedef pu_cpp_int< 4 > cosb_cpp_int_t;
        cpp_int int_var__cosb;
        void cosb (const cpp_int  & l__val);
        cpp_int cosb() const;
    
        typedef pu_cpp_int< 8 > cos_sel_cpp_int_t;
        cpp_int int_var__cos_sel;
        void cos_sel (const cpp_int  & l__val);
        cpp_int cos_sel() const;
    
        typedef pu_cpp_int< 8 > eval_last_cpp_int_t;
        cpp_int int_var__eval_last;
        void eval_last (const cpp_int  & l__val);
        cpp_int eval_last() const;
    
        typedef pu_cpp_int< 4 > host_cpp_int_t;
        cpp_int int_var__host;
        void host (const cpp_int  & l__val);
        cpp_int host() const;
    
        typedef pu_cpp_int< 4 > total_cpp_int_t;
        cpp_int int_var__total;
        void total (const cpp_int  & l__val);
        cpp_int total() const;
    
        typedef pu_cpp_int< 16 > pid_cpp_int_t;
        cpp_int int_var__pid;
        void pid (const cpp_int  & l__val);
        cpp_int pid() const;
    
        #if 8 > CAP_CSR_LARGE_ARRAY_THRESHOLD
        cap_csr_large_array_wrapper<cap_qstate_index_pair_t, 8> index;
        #else 
        cap_qstate_index_pair_t index[8];
        #endif
        int get_depth_index() { return 8; }
    
}; // cap_qstate_decoder_t
    
#endif // CAP_QSTATE_DECODERS_H
        