
#ifndef CAP_TE_PIC_TCAMIF_DECODERS_H
#define CAP_TE_PIC_TCAMIF_DECODERS_H

#include "cap_csr_base.h" 

using namespace std;
class cap_te_pic_tcamif_req_data_bundle_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_pic_tcamif_req_data_bundle_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_pic_tcamif_req_data_bundle_t(string name = "cap_te_pic_tcamif_req_data_bundle_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_pic_tcamif_req_data_bundle_t();
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
    
        typedef pu_cpp_int< 512 > key_cpp_int_t;
        cpp_int int_var__key;
        void key (const cpp_int  & l__val);
        cpp_int key() const;
    
        typedef pu_cpp_int< 4 > tableid_cpp_int_t;
        cpp_int int_var__tableid;
        void tableid (const cpp_int  & l__val);
        cpp_int tableid() const;
    
}; // cap_te_pic_tcamif_req_data_bundle_t
    
class cap_te_pic_tcamif_req_bundle_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_pic_tcamif_req_bundle_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_pic_tcamif_req_bundle_t(string name = "cap_te_pic_tcamif_req_bundle_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_pic_tcamif_req_bundle_t();
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
    
        typedef pu_cpp_int< 1 > srdy_cpp_int_t;
        cpp_int int_var__srdy;
        void srdy (const cpp_int  & l__val);
        cpp_int srdy() const;
    
        typedef pu_cpp_int< 1 > drdy_cpp_int_t;
        cpp_int int_var__drdy;
        void drdy (const cpp_int  & l__val);
        cpp_int drdy() const;
    
        cap_te_pic_tcamif_req_data_bundle_t data;
    
}; // cap_te_pic_tcamif_req_bundle_t
    
class cap_te_pic_tcamif_rsp_data_bundle_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_pic_tcamif_rsp_data_bundle_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_pic_tcamif_rsp_data_bundle_t(string name = "cap_te_pic_tcamif_rsp_data_bundle_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_pic_tcamif_rsp_data_bundle_t();
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
    
        typedef pu_cpp_int< 13 > hitaddr_cpp_int_t;
        cpp_int int_var__hitaddr;
        void hitaddr (const cpp_int  & l__val);
        cpp_int hitaddr() const;
    
        typedef pu_cpp_int< 1 > hit_cpp_int_t;
        cpp_int int_var__hit;
        void hit (const cpp_int  & l__val);
        cpp_int hit() const;
    
}; // cap_te_pic_tcamif_rsp_data_bundle_t
    
class cap_te_pic_tcamif_rsp_bundle_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_pic_tcamif_rsp_bundle_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_pic_tcamif_rsp_bundle_t(string name = "cap_te_pic_tcamif_rsp_bundle_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_pic_tcamif_rsp_bundle_t();
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
    
        typedef pu_cpp_int< 1 > vld_cpp_int_t;
        cpp_int int_var__vld;
        void vld (const cpp_int  & l__val);
        cpp_int vld() const;
    
        cap_te_pic_tcamif_rsp_data_bundle_t data;
    
}; // cap_te_pic_tcamif_rsp_bundle_t
    
class cap_te_pic_tcamif_bundle_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_te_pic_tcamif_bundle_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_te_pic_tcamif_bundle_t(string name = "cap_te_pic_tcamif_bundle_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_te_pic_tcamif_bundle_t();
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
    
        cap_te_pic_tcamif_req_bundle_t req;
    
        cap_te_pic_tcamif_rsp_bundle_t rsp;
    
}; // cap_te_pic_tcamif_bundle_t
    
#endif // CAP_TE_PIC_TCAMIF_DECODERS_H
        