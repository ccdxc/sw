#ifndef AXI_XN_DB_H
#define AXI_XN_DB_H

#include "pen_csr_base.h"  
#include "pknobs_reader.h"
#include "msg_man.h"


class axi_attribute_profile_t {
    public :
        u_int64_t start_address;
        u_int64_t end_address;
        u_int8_t lock;  // lock field, -1 if not constraint
        u_int8_t prot; // protection field, -1 if not constraint
        u_int8_t cache; // cache field, -1 if not constraint
        u_int8_t qos; // qos field, -1 if not constraint
        u_int16_t slave_port;
        axi_attribute_profile_t();
        virtual ~axi_attribute_profile_t() {}
};


class axi_xn_t {
    protected:
        static uint32_t cur_context_id;
        uint32_t get_next_context_id() { 
            uint32_t ret = cur_context_id; 
            cur_context_id++;
            return ret;
        }
    public:
        enum axi_xn_type {
            AXI_WR=0,
            AXI_RD=1
        };

        cpp_int address; // address of the transaction
        u_int16_t req_data_size; // requested data size
        vector<u_int8_t> data;
        axi_xn_type xn_type;

        axi_xn_t();
        virtual ~axi_xn_t() {}



        cpp_int write_strobe; // write strobes, -1 if not constraint
        int     id;   // id field, -1 if not constraint
        u_int8_t lock;  // lock field, -1 if not constraint
        u_int8_t prot; // protection field, -1 if not constraint
        u_int8_t cache; // cache field, -1 if not constraint
        u_int8_t qos; // qos field, -1 if not constraint
        u_int8_t burst_len; // no of transfers in a burst, -1 if not constraint
        u_int8_t burst_size; // burst size, 1B,2B..64B, -1 if not constraint
        u_int8_t burst_type; // -1 if not constraint
        u_int16_t slave_port; // -1 if not constraint


        u_int8_t status;
        uint32_t context_id;

        void set_xn_directed_options(const int _id, const u_int8_t _lock, const u_int8_t _prot, const u_int8_t _cache, 
                const u_int8_t _qos, const u_int8_t _b_len, const u_int8_t _b_size, const u_int8_t _b_type);
        void set_xn_profile_options(std::shared_ptr<axi_attribute_profile_t> _profile);

        void set_xn_options(const u_int64_t _addr, const u_int16_t _size, axi_xn_type _type) ;

        void set_data(const u_int16_t _size, u_int8_t * _valp);
        void get_data(const u_int16_t _size, u_int8_t * _valp);

};

class axi_xn_db {

    static map<int, atomic<axi_xn_db*> > _axi_xn_db_inst;
    static mutex rsp_mtx; 
    static mutex req_mtx;

    axi_xn_db(int _master_idx);
    
    public : 
        static axi_xn_db* access(int) ;
        virtual ~axi_xn_db() { };
        cpp_int_helper hlp;

        vector < std::shared_ptr<axi_attribute_profile_t> > profile_array;
        void set_profile(std::shared_ptr<axi_attribute_profile_t> _profile);
        std::shared_ptr<axi_attribute_profile_t> find_profile(const u_int64_t addr);

        vector< std::shared_ptr<axi_xn_t> > request_db_array;
        vector< std::shared_ptr<axi_xn_t> > rsp_ready_array;
        vector< std::shared_ptr<axi_xn_t> > pending_db_array;



        uint32_t nb_write(const u_int64_t addr, const u_int16_t size, u_int8_t *valp, const u_int32_t flags);
        int nb_write_rsp(const uint32_t context_id, u_int8_t & status);

        uint32_t nb_read(const u_int64_t addr, const u_int16_t size, const u_int32_t flags);
        int nb_read_rsp(const uint32_t context_id, u_int16_t & size, u_int8_t *valp, u_int8_t & status);
        
        bool axi_burst_write(const u_int64_t addr, const u_int16_t size, u_int8_t *valp, const u_int32_t flags, const int boundary=64);
        bool axi_burst_read(const u_int64_t addr, const u_int16_t size, u_int8_t *valp,  const u_int32_t flags, const int boundary=64);

        void flush_all_pending();
        void wait_for_response();

        std::shared_ptr<axi_xn_t> get_next_req(); 
        int is_req_available();
        int get_response(const uint32_t context_id, u_int8_t status, u_int8_t *valp);
        std::shared_ptr<axi_xn_t> pop_response();
        int find_rsp_size(const uint32_t context_id);

        int get_bit_num(int boundary);

};




#endif // AXI_XN_DB_H
