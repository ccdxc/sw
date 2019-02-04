/////mem_access can access host or on-chip memory in the ways of:
////    1.  C model pen_mem 
//         a. direct access
////       b. by axi xn
////    2.  uvm model uvm_mem (TBD)
////       a. direct access by DPI
////       b. axi xn by DPI
////    3.  RTL model (TBD)
////       a. backdoor
////       b. frontdoor
////    4.  Hardware memory by OS (for bring up) (TBD)
//// 
/////           changqi.yang@pensando.io
//
#ifndef __CAP_MEM_ACCESS_HH__
#define __CAP_MEM_ACCESS_HH__

#include <cstdint>
#include <boost/multiprecision/gmp.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <vector>
#include <list>
#include "cap_axi4_slave.h"
#include "pen_mem_base.h"
#include "axi_xn_db.h"


using namespace std;
using namespace boost::multiprecision;
using boost::multiprecision::cpp_int;

class cap_mem_access;
class cap_memacc_handler_base 
{
public:
    typedef enum {
        rw_prefer_cpp_int = 0
    ,   rw_prefer_char_star
    } rw_prefer_t;

    virtual ~cap_memacc_handler_base() {}
protected:
    virtual bool write(uint64_t addr, const cpp_int& data, int len, bool secure, bool reverse_byte_order) = 0;
    virtual bool read(uint64_t addr, cpp_int& data, int len, bool secure, bool reverse_byte_order) = 0;
    virtual bool write(uint64_t addr, const unsigned char* data, int len, bool secure, bool reverse_byte_order) = 0;
    virtual bool read(uint64_t addr, unsigned char* data, int len, bool secure, bool reverse_byte_order) = 0;
    cap_memacc_handler_base(cap_mem_access* memacc, const string& name, int t, rw_prefer_t rd_prefer, rw_prefer_t wr_prefer);
    void display_data(const string& tag, const string& se, int last, const cpp_int& strb, const cpp_int& data, uint64_t addr, int len, int row_sz);
    void display_data(const string& tag, const string& se, int last, const cpp_int& strb, const unsigned char* data, uint64_t addr, int len, int row_sz);

protected:
    cap_mem_access*                                 memacc;
    string                                          name;
    const rw_prefer_t                               read_prefer; //0: cpp_int; 1: unsigned char*
    const rw_prefer_t                               write_prefer; //0: cpp_int; 1: unsigned char*
private:
private:
    bool                                            read_activated;
    bool                                            write_activated;
    const int                                       type; ///bit mask

    friend class cap_mem_access;
};


class cap_mem_access : public pen_mem_base
{
public:
    static cap_mem_access* instancep(int chip_id = 0);
    static cap_mem_access& instance(int chip_id = 0) { return *(instancep(chip_id)); }
    static void reverse_byte_order(const unsigned char* data, int len, unsigned char* odata);
    static void reverse_byte_order(const cpp_int& data, int len, cpp_int& odata);

    cap_mem_access(const string& name, int chip_id = 0);
    bool write(uint64_t addr, const cpp_int& data, int len, bool secure = false, bool reverse_byte_order = false);
    bool write(uint64_t addr, const unsigned char* data, int len, bool secure = false, bool reverse_byte_order = false);
    bool read(uint64_t addr, cpp_int& data, int len, bool secure = false, bool reverse_byte_order = false);
    bool read(uint64_t addr, unsigned char* data, int len, bool secure = false, bool reverse_byte_order = false);
    bool burst_read(uint64_t addr, unsigned char * data, unsigned int len, bool secure = false, bool reverse_byte_order = false);
    bool burst_write(uint64_t addr, const unsigned char * data, unsigned int len, bool secure = false, bool reverse_byte_order = false);
    void update();
    ~cap_mem_access();

public:
    int                                             chip_id;
    string                                          name;
    int                                             primary_read;
    int                                             active_read;
    int                                             active_write;
    bool                                            compare_read;
    int                                             arm_agt_idx;
    int                                             use_axidb;
    axi_xn_db                                       *axi_if;

protected:
protected:
private:
    void construct();
private:
    vector<cap_memacc_handler_base*>                handlers;
};


class cap_memacc_handler_c_direct : public cap_memacc_handler_base
{
public:
    cap_memacc_handler_c_direct(cap_mem_access* memacc);
    virtual ~cap_memacc_handler_c_direct();
protected:
    virtual bool write(uint64_t addr, const cpp_int& data, int len, bool secure = false, bool reverse_byte_order = false) { assert(0); }
    virtual bool write(uint64_t addr, const unsigned char* data, int len, bool secure = false, bool reverse_byte_order = false);
    virtual bool read(uint64_t addr, cpp_int& data, int len, bool secure = false, bool reverse_byte_order = false) { assert(0); }
    virtual bool read(uint64_t addr, unsigned char* data, int len, bool secure = false, bool reverse_byte_order = false);
private:
};


class cap_memacc_handler_c_axi : public cap_memacc_handler_base
{
public:
    cap_memacc_handler_c_axi(cap_mem_access* memacc);
    virtual ~cap_memacc_handler_c_axi();

    ////original axi xn, added error probe, still good if axi address protocol is not violated.
    static cap_axi_aw_bundle_t* make_xn(uint64_t addr, const cpp_int& data, int len, vector<cap_axi_w_bundle_t*>& xn_vec, bool secure = false, bool reverse_byte_order = false);
    static cap_axi_aw_bundle_t* make_xn(uint64_t addr, const unsigned char* data, int len, vector<cap_axi_w_bundle_t*>& xn_vec, bool secure = false, bool reverse_byte_order = false);
    static void make_data(uint64_t addr, cpp_int& data, int len, const vector<cap_axi_r_bundle_t*>& xn_vec, vector<cpp_int>& strb_vec, bool reverse_byte_order = false);
    static void make_data(uint64_t addr, unsigned char* data, int len, const vector<cap_axi_r_bundle_t*>& xn_vec, vector<cpp_int>& strb_vec, bool reverse_byte_order = false);
    static cap_axi_aw_bundle_t* make_xn_aw(uint64_t addr, int len, bool secure = false);
    static cap_axi_ar_bundle_t* make_xn_ar(uint64_t addr, int len, bool secure = false);


    //////enhanced api to split transactions
    static void make_xn(uint64_t addr, const cpp_int& data, int len, list<cap_axi_aw_bundle_t*>& aw_vec, list< list<cap_axi_w_bundle_t*> >& w_vec, bool secure = false, bool reverse_byte_order = false);
    static void make_xn(uint64_t addr, const unsigned char* data, int len, list<cap_axi_aw_bundle_t*>& aw_vec, list< list<cap_axi_w_bundle_t*> >& w_vec, bool secure = false, bool reverse_byte_order = false);
    static void make_xn(uint64_t addr, const cpp_int& data, int len, vector<cap_axi_aw_bundle_t*>& aw_vec, vector< vector<cap_axi_w_bundle_t*> >& w_vec, bool secure = false, bool reverse_byte_order = false);
    static void make_xn(uint64_t addr, const unsigned char* data, int len, vector<cap_axi_aw_bundle_t*>& aw_vec, vector< vector<cap_axi_w_bundle_t*> >& w_vec, bool secure = false, bool reverse_byte_order = false);
    static void make_xn(uint64_t addr, int len, list<cap_axi_ar_bundle_t*>& ar, bool secure = false);
    static void make_xn(uint64_t addr, int len, vector<cap_axi_ar_bundle_t*>& ar, bool secure = false);

    /////for he axi which need 32 bits for length, may generate invalid axi4 xn
    template<typename A> static A* make_xn(uint64_t addr, int len, bool secure = false);
    template<typename AW, typename W> static AW* make_xn(uint64_t addr, const cpp_int& data, int len, vector<W*>& xn_vec, bool secure = false, bool reverse_byte_order = false);
    template<typename AW, typename W> static AW* make_xn(uint64_t addr, const unsigned char* data, int len, vector<W*>& xn_vec, bool secure = false, bool reverse_byte_order = false);
    static void make_data(uint64_t addr, cpp_int& data, int len, const vector<cap_he_axi_r_bundle_t*>& xn_vec, vector<cpp_int>& strb_vec, bool reverse_byte_order = false);
    static void make_data(uint64_t addr, unsigned char* data, int len, const vector<cap_he_axi_r_bundle_t*>& xn_vec, vector<cpp_int>& strb_vec, bool reverse_byte_order = false);

    /////split and merge
    static bool splitting(uint64_t axi_addr, int axi_len, int xn_len);

    static void split(list<cap_axi_aw_bundle_t*>& aw, list< list<cap_axi_w_bundle_t*> >& w, const cap_he_axi_aw_bundle_t* he_aw, const vector<cap_he_axi_w_bundle_t*>& he_w);
    static void split(list<cap_axi_ar_bundle_t*>& ar, const cap_he_axi_ar_bundle_t* he_ar);
    static void merge(vector<cap_he_axi_r_bundle_t*>& he_r, const vector<cap_axi_r_bundle_t*>& r);

    static void split(list<cap_axi_aw_bundle_t*>& aw, list< list<cap_axi_w_bundle_t*> >& w, const cap_axi_aw_bundle_t* oaw, const vector<cap_axi_w_bundle_t*>& ow);
    static void split(list<cap_axi_ar_bundle_t*>& ar, const cap_axi_ar_bundle_t* oar);
    static void merge(vector<cap_axi_r_bundle_t*>& r, const vector<cap_axi_r_bundle_t*>& orr);
    
    ///// helper
    static bool validate_addr(uint64_t addr, int len);
    static cap_axi_aw_bundle_t* clone(cap_he_axi_aw_bundle_t* b, bool del = true);
    static cap_axi_ar_bundle_t* clone(cap_he_axi_ar_bundle_t* b, bool del = true);
    static cap_axi_w_bundle_t* clone(cap_he_axi_w_bundle_t* b, bool del = true);
    static cap_he_axi_r_bundle_t* clone(cap_axi_r_bundle_t* b, bool del = true);
    static cap_axi_aw_bundle_t* clone(const cap_he_axi_aw_bundle_t* b);
    static cap_axi_ar_bundle_t* clone(const cap_he_axi_ar_bundle_t* b);
    static cap_axi_w_bundle_t* clone(const cap_he_axi_w_bundle_t* b);
    static cap_he_axi_r_bundle_t* clone(const cap_axi_r_bundle_t* b);

    static cap_he_axi_aw_bundle_t* clone(cap_axi_aw_bundle_t* b, bool del = true);
    static cap_he_axi_ar_bundle_t* clone(cap_axi_ar_bundle_t* b, bool del = true);
    static cap_he_axi_w_bundle_t* clone(cap_axi_w_bundle_t* b, bool del = true);
    static cap_axi_r_bundle_t* clone(cap_he_axi_r_bundle_t* b, bool del = true);
    template<typename T, typename F> static T* clone(F* f, bool del = true);

    static cap_he_axi_aw_bundle_t* clone(const cap_axi_aw_bundle_t* b);
    static cap_he_axi_ar_bundle_t* clone(const cap_axi_ar_bundle_t* b);
    static cap_he_axi_w_bundle_t* clone(const cap_axi_w_bundle_t* b);
    static cap_axi_r_bundle_t* clone(const cap_he_axi_r_bundle_t* b);
    template<typename T, typename F> static T* clone(const F* f);


protected:
    virtual bool write(uint64_t addr, const cpp_int& data, int len, bool secure = false, bool reverse_byte_order = false);
    virtual bool write(uint64_t addr, const unsigned char* data, int len, bool secure = false, bool reverse_byte_order = false) { assert(0); }
    virtual bool read(uint64_t addr, cpp_int& data, int len, bool secure = false, bool reverse_byte_order = false);
    virtual bool read(uint64_t addr, unsigned char* data, int len, bool secure = false, bool reverse_byte_order = false) { assert(0); }

private:
    cap_axi4_slave_t              slv_hbm;
    cap_axi4_slave_t              slv_hostm;
    static const int                bz;
};


class cap_memacc_handler_host: public cap_memacc_handler_base
{
public:
    cap_memacc_handler_host(cap_mem_access* memacc);
    virtual ~cap_memacc_handler_host();
protected:
    virtual bool write(uint64_t addr, const cpp_int& data, int len, bool secure = false, bool reverse_byte_order = false) { assert(0); }
    virtual bool write(uint64_t addr, const unsigned char* data, int len, bool secure = false, bool reverse_byte_order = false);
    virtual bool read(uint64_t addr, cpp_int& data, int len, bool secure = false, bool reverse_byte_order = false) { assert(0); }
    virtual bool read(uint64_t addr, unsigned char* data, int len, bool secure = false, bool reverse_byte_order = false);
private:
};


#ifdef _CSV_INCLUDED_

class cap_memacc_handler_uvm_direct : public cap_memacc_handler_base
{
public:
    cap_memacc_handler_uvm_direct(cap_mem_access* memacc);
    virtual ~cap_memacc_handler_uvm_direct();
protected:
    virtual bool write(uint64_t addr, const cpp_int& data, int len, bool secure = false, bool reverse_byte_order = false) { assert(0); }
    virtual bool write(uint64_t addr, const unsigned char* data, int len, bool secure = false, bool reverse_byte_order = false);
    virtual bool read(uint64_t addr, cpp_int& data, int len, bool secure = false, bool reverse_byte_order = false) { assert(0); }
    virtual bool read(uint64_t addr, unsigned char* data, int len, bool secure = false, bool reverse_byte_order = false);
private:
};

class cap_memacc_handler_avery_direct : public cap_memacc_handler_base
{
public:
    cap_memacc_handler_avery_direct(cap_mem_access* memacc);
    virtual ~cap_memacc_handler_avery_direct();
protected:
    virtual bool write(uint64_t addr, const cpp_int& data, int len, bool secure = false, bool reverse_byte_order = false) { assert(0); }
    virtual bool write(uint64_t addr, const unsigned char* data, int len, bool secure = false, bool reverse_byte_order = false);
    virtual bool read(uint64_t addr, cpp_int& data, int len, bool secure = false, bool reverse_byte_order = false) { assert(0); }
    virtual bool read(uint64_t addr, unsigned char* data, int len, bool secure = false, bool reverse_byte_order = false);
private:
};

class cap_memacc_handler_rtl_fd: public cap_memacc_handler_base
{
public:
    cap_memacc_handler_rtl_fd(cap_mem_access* memacc);
    virtual ~cap_memacc_handler_rtl_fd();
protected:
    virtual bool write(uint64_t addr, const cpp_int& data, int len, bool secure = false, bool reverse_byte_order = false) { assert(0); }
    virtual bool write(uint64_t addr, const unsigned char* data, int len, bool secure = false, bool reverse_byte_order = false);
    virtual bool read(uint64_t addr, cpp_int& data, int len, bool secure = false, bool reverse_byte_order = false) { assert(0); }
    virtual bool read(uint64_t addr, unsigned char* data, int len, bool secure = false, bool reverse_byte_order = false);
private:
};
#endif //_CSV_INCLUDED_


#define CAP_MEMACC_C_DIRECT     (1 << 0)
#define CAP_MEMACC_C_AXI        (1 << 1)
#define CAP_MEMACC_UVM_DIRECT   (1 << 2)
#define CAP_MEMACC_UVM_AXI      (1 << 3)
#define CAP_MEMACC_RTL_BD       (1 << 4)
#define CAP_MEMACC_RTL_FD       (1 << 5)
#define CAP_MEMACC_HOST         (1 << 6)
#define CAP_MEMACC_AVERY_DIRECT (1 << 7)
#endif //__CAP_MEM_ACCESS_HH__
