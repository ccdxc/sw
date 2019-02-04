
#ifndef CAP_PHV_INTR_DECODERS_H
#define CAP_PHV_INTR_DECODERS_H

#include "cap_csr_base.h" 

using namespace std;
class cap_phv_intr_global_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_phv_intr_global_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_phv_intr_global_t(string name = "cap_phv_intr_global_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_phv_intr_global_t();
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
    
        typedef pu_cpp_int< 4 > tm_instance_type_cpp_int_t;
        cpp_int int_var__tm_instance_type;
        void tm_instance_type (const cpp_int  & l__val);
        cpp_int tm_instance_type() const;
    
        typedef pu_cpp_int< 6 > error_bits_cpp_int_t;
        cpp_int int_var__error_bits;
        void error_bits (const cpp_int  & l__val);
        cpp_int error_bits() const;
    
        typedef pu_cpp_int< 5 > csum_err_cpp_int_t;
        cpp_int int_var__csum_err;
        void csum_err (const cpp_int  & l__val);
        cpp_int csum_err() const;
    
        typedef pu_cpp_int< 1 > debug_trace_cpp_int_t;
        cpp_int int_var__debug_trace;
        void debug_trace (const cpp_int  & l__val);
        cpp_int debug_trace() const;
    
        typedef pu_cpp_int< 5 > tm_oq_cpp_int_t;
        cpp_int int_var__tm_oq;
        void tm_oq (const cpp_int  & l__val);
        cpp_int tm_oq() const;
    
        typedef pu_cpp_int< 1 > hw_error_cpp_int_t;
        cpp_int int_var__hw_error;
        void hw_error (const cpp_int  & l__val);
        cpp_int hw_error() const;
    
        typedef pu_cpp_int< 1 > bypass_cpp_int_t;
        cpp_int int_var__bypass;
        void bypass (const cpp_int  & l__val);
        cpp_int bypass() const;
    
        typedef pu_cpp_int< 1 > drop_cpp_int_t;
        cpp_int int_var__drop;
        void drop (const cpp_int  & l__val);
        cpp_int drop() const;
    
        typedef pu_cpp_int< 14 > tm_q_depth_cpp_int_t;
        cpp_int int_var__tm_q_depth;
        void tm_q_depth (const cpp_int  & l__val);
        cpp_int tm_q_depth() const;
    
        typedef pu_cpp_int< 1 > tm_cpu_cpp_int_t;
        cpp_int int_var__tm_cpu;
        void tm_cpu (const cpp_int  & l__val);
        cpp_int tm_cpu() const;
    
        typedef pu_cpp_int< 1 > tm_replicate_en_cpp_int_t;
        cpp_int int_var__tm_replicate_en;
        void tm_replicate_en (const cpp_int  & l__val);
        cpp_int tm_replicate_en() const;
    
        typedef pu_cpp_int< 16 > tm_replicate_ptr_cpp_int_t;
        cpp_int int_var__tm_replicate_ptr;
        void tm_replicate_ptr (const cpp_int  & l__val);
        cpp_int tm_replicate_ptr() const;
    
        typedef pu_cpp_int< 8 > tm_span_session_cpp_int_t;
        cpp_int int_var__tm_span_session;
        void tm_span_session (const cpp_int  & l__val);
        cpp_int tm_span_session() const;
    
        typedef pu_cpp_int< 48 > timestamp_cpp_int_t;
        cpp_int int_var__timestamp;
        void timestamp (const cpp_int  & l__val);
        cpp_int timestamp() const;
    
        typedef pu_cpp_int< 11 > lif_cpp_int_t;
        cpp_int int_var__lif;
        void lif (const cpp_int  & l__val);
        cpp_int lif() const;
    
        typedef pu_cpp_int< 5 > tm_iq_cpp_int_t;
        cpp_int int_var__tm_iq;
        void tm_iq (const cpp_int  & l__val);
        cpp_int tm_iq() const;
    
        typedef pu_cpp_int< 4 > tm_oport_cpp_int_t;
        cpp_int int_var__tm_oport;
        void tm_oport (const cpp_int  & l__val);
        cpp_int tm_oport() const;
    
        typedef pu_cpp_int< 4 > tm_iport_cpp_int_t;
        cpp_int int_var__tm_iport;
        void tm_iport (const cpp_int  & l__val);
        cpp_int tm_iport() const;
    
}; // cap_phv_intr_global_t
    
class cap_phv_intr_p4_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_phv_intr_p4_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_phv_intr_p4_t(string name = "cap_phv_intr_p4_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_phv_intr_p4_t();
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
    
        typedef pu_cpp_int< 128 > hdr_vld_cpp_int_t;
        cpp_int int_var__hdr_vld;
        void hdr_vld (const cpp_int  & l__val);
        cpp_int hdr_vld() const;
    
        typedef pu_cpp_int< 208 > padding_cpp_int_t;
        cpp_int int_var__padding;
        void padding (const cpp_int  & l__val);
        cpp_int padding() const;
    
        typedef pu_cpp_int< 14 > packet_len_cpp_int_t;
        cpp_int int_var__packet_len;
        void packet_len (const cpp_int  & l__val);
        cpp_int packet_len() const;
    
        typedef pu_cpp_int< 1 > recirc_cpp_int_t;
        cpp_int int_var__recirc;
        void recirc (const cpp_int  & l__val);
        cpp_int recirc() const;
    
        typedef pu_cpp_int< 1 > no_data_cpp_int_t;
        cpp_int int_var__no_data;
        void no_data (const cpp_int  & l__val);
        cpp_int no_data() const;
    
        typedef pu_cpp_int< 14 > frame_size_cpp_int_t;
        cpp_int int_var__frame_size;
        void frame_size (const cpp_int  & l__val);
        cpp_int frame_size() const;
    
        typedef pu_cpp_int< 1 > crypto_hdr_cpp_int_t;
        cpp_int int_var__crypto_hdr;
        void crypto_hdr (const cpp_int  & l__val);
        cpp_int crypto_hdr() const;
    
        typedef pu_cpp_int< 1 > parser_err_cpp_int_t;
        cpp_int int_var__parser_err;
        void parser_err (const cpp_int  & l__val);
        cpp_int parser_err() const;
    
        typedef pu_cpp_int< 3 > recirc_count_cpp_int_t;
        cpp_int int_var__recirc_count;
        void recirc_count (const cpp_int  & l__val);
        cpp_int recirc_count() const;
    
        typedef pu_cpp_int< 4 > len_err_cpp_int_t;
        cpp_int int_var__len_err;
        void len_err (const cpp_int  & l__val);
        cpp_int len_err() const;
    
        typedef pu_cpp_int< 1 > crc_err_cpp_int_t;
        cpp_int int_var__crc_err;
        void crc_err (const cpp_int  & l__val);
        cpp_int crc_err() const;
    
        cap_phv_intr_global_t phv_global;
    
}; // cap_phv_intr_p4_t
    
class cap_phv_intr_txdma_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_phv_intr_txdma_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_phv_intr_txdma_t(string name = "cap_phv_intr_txdma_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_phv_intr_txdma_t();
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
    
        typedef pu_cpp_int< 264 > padding_cpp_int_t;
        cpp_int int_var__padding;
        void padding (const cpp_int  & l__val);
        cpp_int padding() const;
    
        typedef pu_cpp_int< 5 > txdma_rsv_cpp_int_t;
        cpp_int int_var__txdma_rsv;
        void txdma_rsv (const cpp_int  & l__val);
        cpp_int txdma_rsv() const;
    
        typedef pu_cpp_int< 3 > qtype_cpp_int_t;
        cpp_int int_var__qtype;
        void qtype (const cpp_int  & l__val);
        cpp_int qtype() const;
    
        typedef pu_cpp_int< 34 > qstate_addr_cpp_int_t;
        cpp_int int_var__qstate_addr;
        void qstate_addr (const cpp_int  & l__val);
        cpp_int qstate_addr() const;
    
        typedef pu_cpp_int< 6 > dma_cmd_ptr_cpp_int_t;
        cpp_int int_var__dma_cmd_ptr;
        void dma_cmd_ptr (const cpp_int  & l__val);
        cpp_int dma_cmd_ptr() const;
    
        typedef pu_cpp_int< 24 > qid_cpp_int_t;
        cpp_int int_var__qid;
        void qid (const cpp_int  & l__val);
        cpp_int qid() const;
    
        typedef pu_cpp_int< 14 > packet_len_cpp_int_t;
        cpp_int int_var__packet_len;
        void packet_len (const cpp_int  & l__val);
        cpp_int packet_len() const;
    
        typedef pu_cpp_int< 1 > recirc_cpp_int_t;
        cpp_int int_var__recirc;
        void recirc (const cpp_int  & l__val);
        cpp_int recirc() const;
    
        typedef pu_cpp_int< 1 > no_data_cpp_int_t;
        cpp_int int_var__no_data;
        void no_data (const cpp_int  & l__val);
        cpp_int no_data() const;
    
        typedef pu_cpp_int< 14 > frame_size_cpp_int_t;
        cpp_int int_var__frame_size;
        void frame_size (const cpp_int  & l__val);
        cpp_int frame_size() const;
    
        typedef pu_cpp_int< 1 > crypto_hdr_cpp_int_t;
        cpp_int int_var__crypto_hdr;
        void crypto_hdr (const cpp_int  & l__val);
        cpp_int crypto_hdr() const;
    
        typedef pu_cpp_int< 1 > parser_err_cpp_int_t;
        cpp_int int_var__parser_err;
        void parser_err (const cpp_int  & l__val);
        cpp_int parser_err() const;
    
        typedef pu_cpp_int< 3 > recirc_count_cpp_int_t;
        cpp_int int_var__recirc_count;
        void recirc_count (const cpp_int  & l__val);
        cpp_int recirc_count() const;
    
        typedef pu_cpp_int< 4 > len_err_cpp_int_t;
        cpp_int int_var__len_err;
        void len_err (const cpp_int  & l__val);
        cpp_int len_err() const;
    
        typedef pu_cpp_int< 1 > crc_err_cpp_int_t;
        cpp_int int_var__crc_err;
        void crc_err (const cpp_int  & l__val);
        cpp_int crc_err() const;
    
        cap_phv_intr_global_t phv_global;
    
}; // cap_phv_intr_txdma_t
    
class cap_phv_intr_rxdma_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_phv_intr_rxdma_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_phv_intr_rxdma_t(string name = "cap_phv_intr_rxdma_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_phv_intr_rxdma_t();
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
    
        typedef pu_cpp_int< 256 > padding_cpp_int_t;
        cpp_int int_var__padding;
        void padding (const cpp_int  & l__val);
        cpp_int padding() const;
    
        typedef pu_cpp_int< 3 > rxdma_rsv_cpp_int_t;
        cpp_int int_var__rxdma_rsv;
        void rxdma_rsv (const cpp_int  & l__val);
        cpp_int rxdma_rsv() const;
    
        typedef pu_cpp_int< 10 > rx_splitter_offset_cpp_int_t;
        cpp_int int_var__rx_splitter_offset;
        void rx_splitter_offset (const cpp_int  & l__val);
        cpp_int rx_splitter_offset() const;
    
        typedef pu_cpp_int< 3 > qtype_cpp_int_t;
        cpp_int int_var__qtype;
        void qtype (const cpp_int  & l__val);
        cpp_int qtype() const;
    
        typedef pu_cpp_int< 34 > qstate_addr_cpp_int_t;
        cpp_int int_var__qstate_addr;
        void qstate_addr (const cpp_int  & l__val);
        cpp_int qstate_addr() const;
    
        typedef pu_cpp_int< 6 > dma_cmd_ptr_cpp_int_t;
        cpp_int int_var__dma_cmd_ptr;
        void dma_cmd_ptr (const cpp_int  & l__val);
        cpp_int dma_cmd_ptr() const;
    
        typedef pu_cpp_int< 24 > qid_cpp_int_t;
        cpp_int int_var__qid;
        void qid (const cpp_int  & l__val);
        cpp_int qid() const;
    
        typedef pu_cpp_int< 14 > packet_len_cpp_int_t;
        cpp_int int_var__packet_len;
        void packet_len (const cpp_int  & l__val);
        cpp_int packet_len() const;
    
        typedef pu_cpp_int< 1 > recirc_cpp_int_t;
        cpp_int int_var__recirc;
        void recirc (const cpp_int  & l__val);
        cpp_int recirc() const;
    
        typedef pu_cpp_int< 1 > no_data_cpp_int_t;
        cpp_int int_var__no_data;
        void no_data (const cpp_int  & l__val);
        cpp_int no_data() const;
    
        typedef pu_cpp_int< 14 > frame_size_cpp_int_t;
        cpp_int int_var__frame_size;
        void frame_size (const cpp_int  & l__val);
        cpp_int frame_size() const;
    
        typedef pu_cpp_int< 1 > crypto_hdr_cpp_int_t;
        cpp_int int_var__crypto_hdr;
        void crypto_hdr (const cpp_int  & l__val);
        cpp_int crypto_hdr() const;
    
        typedef pu_cpp_int< 1 > parser_err_cpp_int_t;
        cpp_int int_var__parser_err;
        void parser_err (const cpp_int  & l__val);
        cpp_int parser_err() const;
    
        typedef pu_cpp_int< 3 > recirc_count_cpp_int_t;
        cpp_int int_var__recirc_count;
        void recirc_count (const cpp_int  & l__val);
        cpp_int recirc_count() const;
    
        typedef pu_cpp_int< 4 > len_err_cpp_int_t;
        cpp_int int_var__len_err;
        void len_err (const cpp_int  & l__val);
        cpp_int len_err() const;
    
        typedef pu_cpp_int< 1 > crc_err_cpp_int_t;
        cpp_int int_var__crc_err;
        void crc_err (const cpp_int  & l__val);
        cpp_int crc_err() const;
    
        cap_phv_intr_global_t phv_global;
    
}; // cap_phv_intr_rxdma_t
    
class cap_phv_intr_txicrc_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_phv_intr_txicrc_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_phv_intr_txicrc_t(string name = "cap_phv_intr_txicrc_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_phv_intr_txicrc_t();
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
    
        typedef pu_cpp_int< 296 > padding_cpp_int_t;
        cpp_int int_var__padding;
        void padding (const cpp_int  & l__val);
        cpp_int padding() const;
    
        typedef pu_cpp_int< 4 > txicrc_rsv_cpp_int_t;
        cpp_int int_var__txicrc_rsv;
        void txicrc_rsv (const cpp_int  & l__val);
        cpp_int txicrc_rsv() const;
    
        typedef pu_cpp_int< 1 > udp_opt_csum_vld_cpp_int_t;
        cpp_int int_var__udp_opt_csum_vld;
        void udp_opt_csum_vld (const cpp_int  & l__val);
        cpp_int udp_opt_csum_vld() const;
    
        typedef pu_cpp_int< 14 > rdma_udp_len_cpp_int_t;
        cpp_int int_var__rdma_udp_len;
        void rdma_udp_len (const cpp_int  & l__val);
        cpp_int rdma_udp_len() const;
    
        typedef pu_cpp_int< 14 > rdma_ip_pld_len_cpp_int_t;
        cpp_int int_var__rdma_ip_pld_len;
        void rdma_ip_pld_len (const cpp_int  & l__val);
        cpp_int rdma_ip_pld_len() const;
    
        typedef pu_cpp_int< 8 > rdma_ip_offset_cpp_int_t;
        cpp_int int_var__rdma_ip_offset;
        void rdma_ip_offset (const cpp_int  & l__val);
        cpp_int rdma_ip_offset() const;
    
        typedef pu_cpp_int< 1 > rdma_ip_type_cpp_int_t;
        cpp_int int_var__rdma_ip_type;
        void rdma_ip_type (const cpp_int  & l__val);
        cpp_int rdma_ip_type() const;
    
        typedef pu_cpp_int< 14 > outer_udp_len_cpp_int_t;
        cpp_int int_var__outer_udp_len;
        void outer_udp_len (const cpp_int  & l__val);
        cpp_int outer_udp_len() const;
    
        typedef pu_cpp_int< 14 > outer_ip_pld_len_cpp_int_t;
        cpp_int int_var__outer_ip_pld_len;
        void outer_ip_pld_len (const cpp_int  & l__val);
        cpp_int outer_ip_pld_len() const;
    
        typedef pu_cpp_int< 8 > outer_ip_offset_cpp_int_t;
        cpp_int int_var__outer_ip_offset;
        void outer_ip_offset (const cpp_int  & l__val);
        cpp_int outer_ip_offset() const;
    
        typedef pu_cpp_int< 1 > outer_ip_type_cpp_int_t;
        cpp_int int_var__outer_ip_type;
        void outer_ip_type (const cpp_int  & l__val);
        cpp_int outer_ip_type() const;
    
        typedef pu_cpp_int< 1 > outer_layer_vld_cpp_int_t;
        cpp_int int_var__outer_layer_vld;
        void outer_layer_vld (const cpp_int  & l__val);
        cpp_int outer_layer_vld() const;
    
        cap_phv_intr_global_t phv_global;
    
}; // cap_phv_intr_txicrc_t
    
class cap_phv_intr_global_pad_t : public cap_decoder_base {
    public:
    
        #ifdef CSR_NO_CTOR_DEF_NAME
        cap_phv_intr_global_pad_t(string name = "", cap_csr_base *parent = nullptr);
        #else
        cap_phv_intr_global_pad_t(string name = "cap_phv_intr_global_pad_t", cap_csr_base *parent = nullptr);
        #endif
        virtual ~cap_phv_intr_global_pad_t();
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
    
        typedef pu_cpp_int< 376 > padding_cpp_int_t;
        cpp_int int_var__padding;
        void padding (const cpp_int  & l__val);
        cpp_int padding() const;
    
        cap_phv_intr_global_t phv_global;
    
}; // cap_phv_intr_global_pad_t
    
#endif // CAP_PHV_INTR_DECODERS_H
        