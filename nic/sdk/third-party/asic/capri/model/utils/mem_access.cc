#include "mem_access.h"
#include "msg_man.h"
#include "HBM.h"
#include "HOST_MEM.h"
#include "cpp_int_helper.h"
#include "common_dpi.h"
#ifdef _CSV_INCLUDED_
#include "cpu_bus_if.h"
#endif //_CSV_INCLUDED_

#define MY_INFO     MSG_INFO_SCOPE("/db/memacc/"+name)
#define MY_ERROR    MSG_ERROR_SCOPE("/db/memacc/"+name)
#define MY_DEBUG    MSG_DEBUG_SCOPE("/db/memacc/"+name)

#ifdef _CSV_INCLUDED_

#define BUILT_HANDLERS ( \
    CAP_MEMACC_C_DIRECT     | \
    CAP_MEMACC_C_AXI        | \
    CAP_MEMACC_RTL_FD       | \
    CAP_MEMACC_UVM_DIRECT   | \
    CAP_MEMACC_AVERY_DIRECT)

#define ENABLED_WRITE_HANDLERS ( \
    CAP_MEMACC_C_DIRECT | \
    CAP_MEMACC_UVM_DIRECT)

#define ENABLED_READ_HANDLERS ( \
    CAP_MEMACC_C_DIRECT | \
    CAP_MEMACC_UVM_DIRECT)

#define PRIMARY_READ_HANDLER (CAP_MEMACC_UVM_DIRECT)

#else ///// not defined _CSV_INCLUDED_

#ifdef _COSIM_

#define BUILT_HANDLERS ( \
    CAP_MEMACC_C_DIRECT | \
    CAP_MEMACC_C_AXI    | \
    CAP_MEMACC_HOST )

#ifdef _HETEST_
#define ENABLED_WRITE_HANDLERS ( \
    CAP_MEMACC_C_DIRECT | \
    CAP_MEMACC_HOST )
#else
#define ENABLED_WRITE_HANDLERS ( \
    CAP_MEMACC_HOST )
#endif

#ifdef _HETEST_
#define ENABLED_READ_HANDLERS ( \
    CAP_MEMACC_C_DIRECT | \
    CAP_MEMACC_HOST )
#else
#define ENABLED_READ_HANDLERS ( \
    CAP_MEMACC_HOST )
#endif

#define PRIMARY_READ_HANDLER (CAP_MEMACC_HOST)

#else /// not _COSIM_

#define BUILT_HANDLERS ( \
    CAP_MEMACC_C_DIRECT | \
    CAP_MEMACC_C_AXI    | \
    CAP_MEMACC_HOST )

#define ENABLED_WRITE_HANDLERS ( \
    CAP_MEMACC_C_DIRECT)

#define ENABLED_READ_HANDLERS ( \
    CAP_MEMACC_C_DIRECT)

#define PRIMARY_READ_HANDLER (CAP_MEMACC_C_DIRECT)

#endif // not _COSIM_
#endif ///not _CSV_INCLUDED_

const int cap_memacc_handler_c_axi::bz = 6;
    
    
cap_memacc_handler_base::cap_memacc_handler_base(cap_mem_access* memacc, const string& name, int t, rw_prefer_t rd_prefer, rw_prefer_t wr_prefer)
    : read_prefer(rd_prefer), write_prefer(wr_prefer), type(t)
{
    read_activated = false;
    write_activated = false;
    this->name = name;
    this->memacc = memacc;
}


cap_mem_access* cap_mem_access::instancep(int chip_id)
{
    static vector<unique_ptr<cap_mem_access> > g_insts;

    if (chip_id >= (int)g_insts.size()) {
        for (int i = g_insts.size(); i <= chip_id; i++) {
            cap_mem_access* mem = new cap_mem_access("config", i);
            mem->active_write = ENABLED_WRITE_HANDLERS;
            mem->active_read = ENABLED_READ_HANDLERS;
            mem->primary_read = PRIMARY_READ_HANDLER;
            mem->compare_read = true;
            int turn_on_fd_read = SKNOBS_GETD("mem_access/frontdoor_read", 0);
            int turn_on_avery_read = SKNOBS_GETD("mem_access/avery_read", 0);
            if (turn_on_fd_read) {
                mem->active_read |= CAP_MEMACC_RTL_FD;
                mem->active_read &= ~CAP_MEMACC_UVM_DIRECT;
                mem->primary_read = CAP_MEMACC_RTL_FD;
            }
	    if (turn_on_avery_read) {
                mem->active_read |= CAP_MEMACC_AVERY_DIRECT;
                mem->active_read &= ~CAP_MEMACC_UVM_DIRECT;
                mem->primary_read = CAP_MEMACC_AVERY_DIRECT;
	    }
            int turn_on_fd_write = SKNOBS_GETD("mem_access/frontdoor_write", 0);
            int turn_on_avery_write = SKNOBS_GETD("mem_access/avery_write", 0);
            if (turn_on_fd_write) {
                mem->active_write |= CAP_MEMACC_RTL_FD;
            }
	    if (turn_on_avery_write) {
                mem->active_write |= CAP_MEMACC_AVERY_DIRECT;
                mem->active_write &= ~CAP_MEMACC_UVM_DIRECT;
            }
            mem->arm_agt_idx = SKNOBS_GETD("mem_access/arm_agt_idx", 1);
            mem->use_axidb = SKNOBS_GETD("axi/use_axidb", 0);
            mem->update();

            if (mem->use_axidb == 1) {
                mem->axi_if = axi_xn_db::access(mem->arm_agt_idx);
            }
            g_insts.push_back(move(unique_ptr<cap_mem_access>(mem)));
        }
    }


    return g_insts[chip_id].get();
}


void cap_mem_access::construct()
{
    int t = BUILT_HANDLERS;

    if (t & CAP_MEMACC_C_DIRECT) {
        cap_memacc_handler_c_direct* h = new cap_memacc_handler_c_direct(this);
        handlers.push_back(h);
    }

    if (t & CAP_MEMACC_C_AXI) {
        cap_memacc_handler_c_axi* h = new cap_memacc_handler_c_axi(this);
        handlers.push_back(h);
    }

    if (t & CAP_MEMACC_HOST) {
        cap_memacc_handler_host* h = new cap_memacc_handler_host(this);
        handlers.push_back(h);
    }

#ifdef _CSV_INCLUDED_
    if (t & CAP_MEMACC_UVM_DIRECT) {
        cap_memacc_handler_uvm_direct* h = new cap_memacc_handler_uvm_direct(this);
        handlers.push_back(h);
    }
    if (t & CAP_MEMACC_AVERY_DIRECT) {
        cap_memacc_handler_avery_direct* h = new cap_memacc_handler_avery_direct(this);
        handlers.push_back(h);
    }
    if (t & CAP_MEMACC_RTL_FD) {
        cap_memacc_handler_rtl_fd* h = new cap_memacc_handler_rtl_fd(this);
        handlers.push_back(h);
    }
#endif //_CSV_INCLUDED_
}


cap_mem_access::cap_mem_access(const string& name, int chip_id)
{
    this->name = name;
    this->chip_id = chip_id;
    primary_read = 0;
    active_read = 0;
    active_write = 0;
    compare_read = false;
    construct();
}


cap_mem_access::~cap_mem_access()
{
    for (vector<cap_memacc_handler_base*>::iterator it = handlers.begin(); it != handlers.end(); it++) {
        delete *it;
    }
}


void cap_mem_access::update()
{
    if (primary_read == 0 && active_read != 0) {
        int b = active_read;
        primary_read = active_read;
        while ((b >>= 1) != 0) {
            if (active_read & b) { ///multiple 1's
                primary_read = 0; ///set back to 0
                break; 
            }
        }
    }

    active_read |= primary_read;

    for (vector<cap_memacc_handler_base*>::iterator it = handlers.begin(); it != handlers.end(); it++) {
        cap_memacc_handler_base* h = *it;
        if (h->type & active_read) h->read_activated = true;
        else h->read_activated = false;
        if (h->type & active_write) h->write_activated = true;
        else h->write_activated = false;
    }
}


bool cap_mem_access::write(uint64_t addr, const cpp_int& data, int len, bool secure, bool reverse_byte_order)
{
    bool ret = true;
    bool run = false;
    for (vector<cap_memacc_handler_base*>::iterator it = handlers.begin(); it != handlers.end(); it++) {
        cap_memacc_handler_base* h = *it;
        if (!h->write_activated) continue;
        run = true;
        if (h->write_prefer == cap_memacc_handler_base::rw_prefer_cpp_int) {
            ret &= h->write(addr, data, len, secure, reverse_byte_order);
        }
        if (h->write_prefer == cap_memacc_handler_base::rw_prefer_char_star) {
            unsigned char* tdata = new unsigned char [len];
            cpp_int_helper::s_array_from_cpp_int(data, 0, len - 1, tdata);
            ret &= h->write(addr, tdata, len, secure, reverse_byte_order);
            delete [] tdata;
        }
    }

    return ret && run;
}



bool cap_mem_access::write(uint64_t addr, const unsigned char* data, int len, bool secure, bool reverse_byte_order)
{
    bool ret = true;
    bool run = false;
    for (vector<cap_memacc_handler_base*>::iterator it = handlers.begin(); it != handlers.end(); it++) {
        cap_memacc_handler_base* h = *it;
        if (!h->write_activated) continue;
        run = true;
        if (h->write_prefer == cap_memacc_handler_base::rw_prefer_cpp_int) {
            cpp_int tdata(0);
            cpp_int_helper::s_cpp_int_from_array(tdata, 0, len - 1, data);
            ret &= h->write(addr, tdata, len, secure, reverse_byte_order);
        }
        if (h->write_prefer == cap_memacc_handler_base::rw_prefer_char_star) {
            ret &= h->write(addr, data, len, secure, reverse_byte_order);
        }
    }

    return ret && run;
}



bool cap_mem_access::read(uint64_t addr, cpp_int& data, int len, bool secure, bool reverse_byte_order)
{
    bool ret = true;
    bool run = false;
    cap_memacc_handler_base* p = 0;
    if (primary_read == 0) {
        MY_ERROR << "cap_mem_access::read: primary_read is not set" << endl;
        return false;
    }

    for (vector<cap_memacc_handler_base*>::iterator it = handlers.begin(); it != handlers.end(); it++) {
        cap_memacc_handler_base* h = *it;
        if (h->type != primary_read) continue;
        run = true;
        p = h;

        if (h->read_prefer == cap_memacc_handler_base::rw_prefer_cpp_int) {
            ret &= h->read(addr, data, len, secure, reverse_byte_order);
        }
        if (h->read_prefer == cap_memacc_handler_base::rw_prefer_char_star) {
            unsigned char* tdata = new unsigned char [len];
            ret &= h->read(addr, tdata, len, secure, reverse_byte_order);
            data.assign(0);
            cpp_int_helper::s_cpp_int_from_array(data, 0, len - 1, tdata);
            delete [] tdata;
        }
    }

    if (!run) {
        MY_ERROR << "cap_mem_access::read: primary_read is set to un-supported handler" << endl;
        return false;
    }
    if (!ret) {
        MY_ERROR << "cap_mem_access::read: primary read from '" << p->name << "' FAILED" << endl;
        return false;
    }


    if (!compare_read) return ret;

    cpp_int second;
    int osrc = 0;
    for (vector<cap_memacc_handler_base*>::iterator it = handlers.begin(); it != handlers.end(); it++) {
        cap_memacc_handler_base* h = *it;
        if (!h->read_activated) continue;
        if (h->type == primary_read) continue;
        bool rr;
        second.assign(0);
        if (h->read_prefer == cap_memacc_handler_base::rw_prefer_cpp_int) {
            rr = h->read(addr, second, len, secure, reverse_byte_order);
        }
        if (h->read_prefer == cap_memacc_handler_base::rw_prefer_char_star) {
            unsigned char* tdata = new unsigned char [len];
            rr = h->read(addr, tdata, len, secure, reverse_byte_order);
            cpp_int_helper::s_cpp_int_from_array(second, 0, len - 1, tdata);
            delete [] tdata;
        }
       
        ret &= rr;
        if (rr) {
            if (second != data) {
                ret &= false;
                MY_ERROR << "cap_mem_access::read: second read data addr 0x" << hex << addr << dec << " NOT matched:" << endl;
                MY_ERROR << " -- 0x" << hex << second << " ('" << h->name << "')" << dec << endl;
                MY_ERROR << " ** 0x" << hex << data << " ('" << p->name << "')" << dec << endl;
            }
            else {
                MY_DEBUG << "cap_mem_access::read: compare reading from alternative source " << osrc << " successfully" << endl;
            }
        }
        else {
            MY_ERROR << "cap_mem_access::read: second read from '" << h->name << "' FAILED" << endl;
        }
        osrc++;
    }

    return ret;
}


bool cap_mem_access::read(uint64_t addr, unsigned char* data, int len, bool secure, bool reverse_byte_order)
{
    bool ret = true;
    bool run = false;
    cap_memacc_handler_base* p = 0;
    if (primary_read == 0) {
        MY_ERROR << "cap_mem_access::read: primary_read is not set" << endl;
        return false;
    }

    cpp_int first;
    for (vector<cap_memacc_handler_base*>::iterator it = handlers.begin(); it != handlers.end(); it++) {
        cap_memacc_handler_base* h = *it;
        if (h->type != primary_read) continue;
        run = true;
        p = h;
        first.assign(0);

        if (h->read_prefer == cap_memacc_handler_base::rw_prefer_cpp_int) {
            cpp_int tdata(0);
            ret &= h->read(addr, tdata, len, secure, reverse_byte_order);
            cpp_int_helper::s_array_from_cpp_int(tdata, 0, len - 1, data);
            if (compare_read) first.assign(tdata);
        }
        if (h->read_prefer == cap_memacc_handler_base::rw_prefer_char_star) {
            ret &= h->read(addr, data, len, secure, reverse_byte_order);
            if (compare_read) cpp_int_helper::s_cpp_int_from_array(first, 0, len - 1, data);
        }
    }

    if (!run) {
        MY_ERROR << "cap_mem_access::read: primary_read is set to un-supported handler" << endl;
        return false;
    }
    if (!ret) {
        MY_ERROR << "cap_mem_access::read: primary read from '" << p->name << "' FAILED" << endl;
        return false;
    }


    if (!compare_read) return ret;

    cpp_int second;
    int osrc = 0;

    for (vector<cap_memacc_handler_base*>::iterator it = handlers.begin(); it != handlers.end(); it++) {
        cap_memacc_handler_base* h = *it;
        if (!h->read_activated) continue;
        if (h->type == primary_read) continue;
        bool rr;
        second.assign(0);
        if (h->read_prefer == cap_memacc_handler_base::rw_prefer_cpp_int) {
            rr = h->read(addr, second, len, secure, reverse_byte_order);
        }
        if (h->read_prefer == cap_memacc_handler_base::rw_prefer_char_star) {
            unsigned char* tdata = new unsigned char [len];
            rr = h->read(addr, tdata, len, secure, reverse_byte_order);
            cpp_int_helper::s_cpp_int_from_array(second, 0, len - 1, tdata);
            delete [] tdata;
        }
       
        ret &= rr;
        if (rr) {
            if (second != first) {
                ret &= false;
                MY_ERROR << "cap_mem_access::read: second read data addr 0x" << hex << addr << dec << " NOT matched:" << endl;
                MY_ERROR << " -- 0x" << hex << second << " ('" << h->name << "')" << dec << endl;
                MY_ERROR << " ** 0x" << hex << first << " ('" << p->name << "')" << dec << endl;
            }
            else {
                MY_DEBUG << "cap_mem_access::read: compare reading from alternative source " << osrc << " successfully" << endl;
            }
        }
        else {
            MY_ERROR << "cap_mem_access::read: second read from '" << h->name << "' FAILED" << endl;
        }
        osrc++;
    }

    return ret;
}



cap_memacc_handler_c_direct::cap_memacc_handler_c_direct(cap_mem_access* memacc)
    : cap_memacc_handler_base(memacc, memacc->name + "/intf_c_direct/" + itos(memacc->chip_id), CAP_MEMACC_C_DIRECT, 
                              cap_memacc_handler_base::rw_prefer_char_star, cap_memacc_handler_base::rw_prefer_char_star)
{
}


cap_memacc_handler_c_direct::~cap_memacc_handler_c_direct()
{
}


static pen_mem_base* get_HBM(uint64_t ain, uint64_t& aout, bool is_read)
{
    aout = ain;
    return HBM::access();
}


static pen_mem_base* get_host_memory(uint64_t ain, uint64_t& aout, bool is_read)
{
    aout = ain;
    return HOST_MEM::access();
}



cap_memacc_handler_c_axi::cap_memacc_handler_c_axi(cap_mem_access* memacc)
    : cap_memacc_handler_base(memacc, memacc->name + "/intf_c_axi/" + itos(memacc->chip_id), CAP_MEMACC_C_AXI,
                              cap_memacc_handler_base::rw_prefer_cpp_int, cap_memacc_handler_base::rw_prefer_cpp_int),
      slv_hbm("slv_hbm", get_HBM, 0x0, 0x100000),
      slv_hostm("slv_hostm", get_host_memory, 0x0, 0x100000)
{
}


cap_memacc_handler_c_axi::~cap_memacc_handler_c_axi()
{
}


bool cap_memacc_handler_c_direct::write(uint64_t addr, const unsigned char* data, int len, bool secure, bool reverse)
{
    pen_mem_base* m = 0;

    if (addr >> 63) { ////host memory, TBD
        m = HOST_MEM::access();
    }
    else {
        m = HBM::access();
    }
    bool ret;
    cpp_int strb(1);
    strb <<= (len);
    strb -= 1;

    if (!reverse) {
        ret = m->burst_write(addr, data, len);
        MY_INFO << "[mem_access::intf_c_direct] ------" << endl;
        display_data("W", (secure? "S": "N"), 0, strb, data, addr, len, 16);
    } 
    else {
        unsigned char* tdata = new unsigned char [len];
        cap_mem_access::reverse_byte_order(data, len, tdata);
        ret = m->burst_write(addr, tdata, len);
        MY_INFO << "[mem_access::intf_c_direct] ------" << endl;
        display_data("W", (secure? "S": "N"), 0, strb, tdata, addr, len, 16);
        delete [] tdata;
    }
    return ret;
}


bool cap_memacc_handler_c_direct::read(uint64_t addr, unsigned char* data, int len, bool secure, bool reverse)
{
    pen_mem_base* m = 0;
    if (addr >> 63) { ////host memory, TBD
        m = HOST_MEM::access();
    }
    else {
        m = HBM::access();
    }
    bool ret;
    cpp_int strb(1);
    strb <<= (len);
    strb -= 1;

    if (!reverse) {
        ret = m->burst_read(addr, data, len);
        MY_INFO << "[mem_access::intf_c_direct] ------" << endl;
        display_data("R", (secure? "S": "N"), 0, strb, data, addr, len, 16);
    }
    else {
        unsigned char* tdata = new unsigned char [len];
        ret = m->burst_read(addr, tdata, len);
        cap_mem_access::reverse_byte_order(tdata, len, data);
        MY_INFO << "[mem_access::intf_c_direct] ------" << endl;
        display_data("R", (secure? "S": "N"), 0, strb, tdata, addr, len, 16);
    }

    return ret;
}


bool cap_memacc_handler_c_axi::write(uint64_t addr, const cpp_int& data, int len, bool secure, bool reverse)
{
    cap_axi4_slave_t* pslv = 0;
    if (addr >> 63) { ////host memory, TBD
        pslv = &slv_hostm;
    }
    else {
        pslv = &slv_hbm;
    }

    vector<cap_axi_aw_bundle_t*> aw;
    vector< vector<cap_axi_w_bundle_t*> > xn_vec;
    make_xn(addr, data, len, aw, xn_vec, secure, reverse);
    
    for (unsigned int i = 0; i < aw.size(); i++) {
        int xn = xn_vec[i].size();
        assert(xn == aw[i]->len() + 1);
        pslv->write(aw[i]);
        uint64_t xn_addr = aw[i]->addr().template convert_to<uint64_t>();

        for (int j = 0; j < xn; j++) {
            MY_INFO << "[mem_access::intf_c_axi] ------" << endl;
            display_data("W", (secure? "S": "N"), (j == xn - 1), xn_vec[i][j]->strb(), xn_vec[i][j]->data(), xn_addr + j * (1 << bz), 1 << bz, 1 << bz);
            pslv->write_data(xn_vec[i][j]);
        }

        pen_csr_base* resp = 0;
        pslv->write_resp(resp);
        assert(resp);
        delete resp;    
    }
    return true;
}


bool cap_memacc_handler_c_axi::read(uint64_t addr, cpp_int& data, int len, bool secure, bool reverse)
{
    cap_axi4_slave_t* pslv = 0;
    if (addr >> 63) { ////host memory, TBD
        pslv = &slv_hostm;
    }
    else {
        pslv = &slv_hbm;
    }


    vector<cap_axi_ar_bundle_t*> ar;
    make_xn(addr, len, ar, secure);
    vector<cap_he_axi_r_bundle_t*> he_r;

    for (unsigned int i = 0; i < ar.size(); i++) {
        vector<pen_csr_base*> xn_vec;
        vector<cap_axi_r_bundle_t*> cast_xn_vec;
        int xn = ar[i]->len().convert_to<int>() + 1;

        pslv->read(ar[i]);
        pslv->read_resp(xn_vec);
        assert((int)xn_vec.size() == xn);

        for (int j = 0; j < xn; j++) {
            cap_axi_r_bundle_t* r = dynamic_cast<cap_axi_r_bundle_t*>(xn_vec[j]);
            assert(r);
            cast_xn_vec.push_back(r);
        }

        merge(he_r, cast_xn_vec);
        for (int j = 0; j < xn; j++) delete xn_vec[j];
    }


    uint64_t xn_addr = addr >> bz << bz;
    vector<cpp_int> strb_vec;
    make_data(addr, data, len, he_r, strb_vec, reverse);
    assert(he_r.size() == strb_vec.size()); 
    for (unsigned int i = 0; i < he_r.size(); i++) {
        MY_INFO << "[mem_access::intf_c_axi] ------" << endl;
        display_data("R", (secure? "S": "N"), (i == he_r.size() - 1), strb_vec[i], he_r[i]->data(), xn_addr + i * (1 << bz), 1 << bz, 1 << bz);
        delete he_r[i];
    }

    return true;
}


void cap_memacc_handler_base::display_data(const string& tag, const string& se, int last, const cpp_int& strb, const cpp_int& data, uint64_t addr, int len, int row_sz)
{
    unsigned char* tdata = new unsigned char[len];
    cpp_int_helper::s_array_from_cpp_int(data, 0, len-1, tdata);
    display_data(tag, se, last, strb, tdata, addr, len, row_sz);
    delete [] tdata;
}


void cap_memacc_handler_base::display_data(const string& tag, const string& se, int last, const cpp_int& strb, const unsigned char* data, uint64_t addr, int len, int row_sz)
{
    MY_INFO << msg_man::ctl_save_format << hex << setfill('0');
    for (uint64_t i = addr; i < addr + len; i += row_sz) {
        int start_byte = (int)(i - addr);
        int byte_num = min((int)(addr + len - i), row_sz);
        cpp_int strb_slc = cpp_int_helper::s_get_slc(strb, byte_num, start_byte);
        MY_INFO << tag << " " << se << " " 
                << "0x" << setw(16) << i << "-0x" << setw(16) << (i + row_sz - 1) << " "
                << setw(1) << "(" << last << ":"
                << "0x" << setw(row_sz / 4) << strb_slc << ") "
                << "--"; 
       for (int j = start_byte; j < start_byte + row_sz; j++)  {
           if (j >= len) MSGX << "00";
           else MSGX << setw(2) << (int)data[j];
       }
       MSGX << endl;
    }
    MSG_MAN << msg_man::ctl_restore_format << dec;
}


bool cap_memacc_handler_c_axi::validate_addr(uint64_t addr, int len)
{
    bool ret = true;
    uint64_t start = addr;
    uint64_t end = addr + len - 1;
    int xn = (end >> bz) - (start >> bz) + 1;
    
    if ((start & 0xfffffffffffff000) != (end & 0xfffffffffffff000)) {
        MSG_ERROR_SCOPE("/") << "cap_memacc_handler_c_axi::make_xn_aw(): AXI violation: addr=0x" << hex << addr << ", len=0x" << len << dec << ", has crossed 4KB address boundary" << endl;
        ret = false;
    }
    if (xn > 4) {
        MSG_ERROR_SCOPE("/") << "cap_memacc_handler_c_axi::make_xn_aw(): AXI violation: addr=0x" << hex << addr << ", len=0x" << len << dec << ", has exceeded 256B limitation" << endl;
        ret = false;
    }
    
    return ret;
}


template<>
cap_he_axi_aw_bundle_t* cap_memacc_handler_c_axi::make_xn(uint64_t addr, int len, bool secure)
{
    uint64_t start = addr;
    uint64_t end = addr + len - 1;
    int xn = (end >> bz) - (start >> bz) + 1;
    uint64_t xn_addr = start >> bz << bz;
    
    cap_he_axi_aw_bundle_t* aw = new cap_he_axi_aw_bundle_t;
    aw->valid(1);
    aw->id(0);
    aw->addr(xn_addr);
    aw->burst(1);
    aw->sz(bz);
    aw->len(xn - 1);
    return aw;
}


cap_axi_aw_bundle_t* cap_memacc_handler_c_axi::make_xn_aw(uint64_t addr, int len, bool secure)
{
    validate_addr(addr, len);
    cap_he_axi_aw_bundle_t* a = make_xn<cap_he_axi_aw_bundle_t>(addr, len, secure);
    return clone(a);
}

template<>
cap_he_axi_ar_bundle_t* cap_memacc_handler_c_axi::make_xn(uint64_t addr, int len, bool secure)
{
    uint64_t start = addr;
    uint64_t end = addr + len - 1;
    int xn = (end >> bz) - (start >> bz) + 1;
    uint64_t xn_addr = start >> bz << bz;

    cap_he_axi_ar_bundle_t* ar = new cap_he_axi_ar_bundle_t;
    ar->valid(1);
    ar->id(0);
    ar->addr(xn_addr);
    ar->burst(1);
    ar->sz(bz);
    ar->len(xn - 1);
    return ar;
}


cap_axi_ar_bundle_t* cap_memacc_handler_c_axi::make_xn_ar(uint64_t addr, int len, bool secure)
{
    validate_addr(addr, len);
    cap_he_axi_ar_bundle_t* a = make_xn<cap_he_axi_ar_bundle_t>(addr, len, secure);
    return clone(a);
}


template<>
cap_he_axi_aw_bundle_t* cap_memacc_handler_c_axi::make_xn(uint64_t addr, const cpp_int& data, int len, vector<cap_he_axi_w_bundle_t*>& xn_vec, bool secure, bool reverse)
{
    uint64_t start = addr;
    uint64_t end = start + len - 1;
    cap_he_axi_aw_bundle_t* aw = make_xn<cap_he_axi_aw_bundle_t>(addr, len, secure);
    int xn = aw->len().template convert_to<int>() + 1;
    uint64_t xn_addr = aw->addr().template convert_to<uint64_t>();

    cpp_int tdata;
    if (!reverse) tdata.assign(data);
    else cap_mem_access::reverse_byte_order(data, len, tdata);

    cpp_int_helper hlp;
    for (int i = 0; i < xn; i++) {
        uint64_t xn_start = xn_addr + i * (1 << bz);
        uint64_t xn_end = xn_start + (1 << bz) - 1;
        cpp_int xn_data;
        int byte = 0;

        cpp_int strb(1);
        strb <<= (1 << bz);
        strb -= 1;

        cpp_int strb_mask = strb;

        if (xn_start < start) {
            for (uint64_t j = 0; j < start - xn_start; j++) {
                hlp.set_slc(xn_data, 0x5a, byte * 8, byte * 8 + 7);
                byte++;
            }
            strb <<= (start - xn_start);
            strb &= strb_mask;
        }

        int more = min(((1 << bz) - byte), (int)(end - max(start, xn_start) + 1));
        if (more > 0) {
            hlp.set_slc(xn_data, 
                        hlp.get_slc(tdata, (max(start, xn_start) - start) * 8, (max(start, xn_start) - start + more) * 8 - 1), 
                        byte * 8,
                        (byte + more) * 8 - 1);
            byte += more;
        }

        if (byte < (1 << bz)) {
            assert(end < xn_end);
            for (uint64_t j = 0; j < xn_end - end; j++) {
                hlp.set_slc(xn_data, 0xa5, byte * 8, byte * 8 + 7);
                byte++;
            }
            strb <<= (xn_end - end);
            strb &= strb_mask;
            strb >>= (xn_end - end);
        }
        assert(byte == (1 << bz));
        
        cap_he_axi_w_bundle_t* w = new cap_he_axi_w_bundle_t;
        w->valid(1);
        w->data(xn_data);
        w->strb(strb);
        w->last(i == xn - 1);
        xn_vec.push_back(w);
    }
    return aw;
}


cap_axi_aw_bundle_t* cap_memacc_handler_c_axi::make_xn(uint64_t addr, const cpp_int& data, int len, vector<cap_axi_w_bundle_t*>& xn_vec, bool secure, bool reverse)
{
    validate_addr(addr, len);
    vector<cap_he_axi_w_bundle_t*> v;
    cap_he_axi_aw_bundle_t * aw = make_xn<cap_he_axi_aw_bundle_t>(addr, data, len, v, secure, reverse);
    for (vector<cap_he_axi_w_bundle_t*>::iterator it = v.begin(); it != v.end(); it++) {
        xn_vec.push_back(clone(*it));
    }
    return clone(aw);
}


template<>
cap_he_axi_aw_bundle_t* cap_memacc_handler_c_axi::make_xn(uint64_t addr, const unsigned char* data, int len, vector<cap_he_axi_w_bundle_t*>& xn_vec, bool secure, bool reverse)
{
    uint64_t start = addr;
    uint64_t end = start + len - 1;
    cap_he_axi_aw_bundle_t* aw = make_xn<cap_he_axi_aw_bundle_t>(addr, len, secure);
    int xn = aw->len().template convert_to<int>() + 1;
    uint64_t xn_addr = aw->addr().template convert_to<uint64_t>();

    const unsigned char* tdata = 0;
    unsigned char* ndata = 0;
    if (!reverse) {
        tdata = data;
    }
    else {
        unsigned char* ndata = new unsigned char [len];
        tdata = ndata;
    }

    cpp_int_helper hlp;
    for (int i = 0; i < xn; i++) {
        uint64_t xn_start = xn_addr + i * (1 << bz);
        uint64_t xn_end = xn_start + (1 << bz) - 1;
        cpp_int xn_data;
        int byte = 0;

        cpp_int strb(1);
        strb <<= (1 << bz);
        strb -= 1;

        cpp_int strb_mask = strb;

        if (xn_start < start) {
            for (uint64_t j = 0; j < start - xn_start; j++) {
                hlp.set_slc(xn_data, 0x5a, byte * 8, byte * 8 + 7);
                byte++;
            }
            strb <<= (start - xn_start);
            strb &= strb_mask;
        }

        int more = min(((1 << bz) - byte), (int)(end - max(start, xn_start) + 1));
        if (more > 0) {
            cpp_int slice(0);
            cpp_int_helper::s_cpp_int_from_array(slice, 0, more - 1, tdata + (max(start, xn_start) - start));
            hlp.set_slc(xn_data, slice, byte * 8, (byte + more) * 8 - 1);
            byte += more;
        }

        if (byte < (1 << bz)) {
            assert(end < xn_end);
            for (uint64_t j = 0; j < xn_end - end; j++) {
                hlp.set_slc(xn_data, 0xa5, byte * 8, byte * 8 + 7);
                byte++;
            }
            strb <<= (xn_end - end);
            strb &= strb_mask;
            strb >>= (xn_end - end);
        }
        assert(byte == (1 << bz));
        
        cap_he_axi_w_bundle_t* w = new cap_he_axi_w_bundle_t;
        w->valid(1);
        w->data(xn_data);
        w->strb(strb);
        w->last(i == xn - 1);
        xn_vec.push_back(w);
    }

    if (ndata) delete [] ndata;

    return aw;
}



cap_axi_aw_bundle_t* cap_memacc_handler_c_axi::make_xn(uint64_t addr, const unsigned char* data, int len, vector<cap_axi_w_bundle_t*>& xn_vec, bool secure, bool reverse)
{
    validate_addr(addr, len);
    vector<cap_he_axi_w_bundle_t*> v;
    cap_he_axi_aw_bundle_t* aw = make_xn<cap_he_axi_aw_bundle_t>(addr, data, len, v, secure, reverse);
    for (vector<cap_he_axi_w_bundle_t*>::iterator it = v.begin(); it != v.end(); it++) {
        xn_vec.push_back(clone(*it));
    }
    return clone(aw);
}


void cap_memacc_handler_c_axi::make_data(uint64_t addr, cpp_int& data, int len, const vector<cap_he_axi_r_bundle_t*>& xn_vec, vector<cpp_int>& strb_vec, bool reverse)
{
    data.assign(0);

    cpp_int tdata(0);

    uint64_t start = addr;
    uint64_t end = start + len - 1;
    int xn = (end >> bz) - (start >> bz) + 1;
    uint64_t xn_addr = start >> bz << bz;

    cpp_int_helper hlp;
    for (int i = 0; i < xn; i++) {
        uint64_t xn_start = xn_addr + i * (1 << bz);
        uint64_t xn_end = xn_start + (1 << bz) - 1;
        int byte = 0;

        cpp_int strb(1);
        strb <<= (1 << bz);
        strb -= 1;

        cpp_int strb_mask = strb;

        if (xn_start < start) {
            byte += (start - xn_start);
            strb <<= (start - xn_start);
            strb &= strb_mask;
        }

        int more = min(((1 << bz) - byte), (int)(end - max(start, xn_start) + 1));
        if (more > 0) {
            hlp.set_slc(tdata, 
                        hlp.get_slc(xn_vec[i]->data(), byte * 8, (byte + more) * 8 - 1), 
                        (max(start, xn_start) - start) * 8,
                        (max(start, xn_start) - start + more) * 8 - 1);
            byte += more;
        }

        if (byte < (1 << bz)) {
            assert(end < xn_end);
            byte += (xn_end - end);
            strb <<= (xn_end - end);
            strb &= strb_mask;
            strb >>= (xn_end - end);
        }
        assert(byte == (1 << bz));
        
        assert(xn_vec[i]->last() == (i == xn - 1));
        strb_vec.emplace_back(strb);
    }

    if (!reverse) data.assign(tdata);
    else cap_mem_access::reverse_byte_order(tdata, len, data);
}



void cap_memacc_handler_c_axi::make_data(uint64_t addr, cpp_int& data, int len, const vector<cap_axi_r_bundle_t*>& xn_vec, vector<cpp_int>& strb_vec, bool reverse)
{
    vector<cap_he_axi_r_bundle_t*> v;
    for (vector<cap_axi_r_bundle_t*>::const_iterator it = xn_vec.cbegin(); it != xn_vec.cend(); it++) {
        v.push_back(clone(*it));
    }
    make_data(addr, data, len, v, strb_vec, reverse);
    for (vector<cap_he_axi_r_bundle_t*>::iterator it = v.begin(); it != v.end(); it++) {
        delete *it;
    }
}



void cap_memacc_handler_c_axi::make_data(uint64_t addr, unsigned char* data, int len, const vector<cap_he_axi_r_bundle_t*>& xn_vec, vector<cpp_int>& strb_vec, bool reverse)
{
    uint64_t start = addr;
    uint64_t end = start + len - 1;
    int xn = (end >> bz) - (start >> bz) + 1;
    uint64_t xn_addr = start >> bz << bz;

    unsigned char* tdata = 0;
    if (!reverse) {
        tdata = data;
    }
    else {
        tdata = new unsigned char [len];
    }

    cpp_int_helper hlp;
    for (int i = 0; i < xn; i++) {
        uint64_t xn_start = xn_addr + i * (1 << bz);
        uint64_t xn_end = xn_start + (1 << bz) - 1;
        int byte = 0;

        cpp_int strb(1);
        strb <<= (1 << bz);
        strb -= 1;

        cpp_int strb_mask = strb;

        if (xn_start < start) {
            byte += (start - xn_start);
            strb <<= (start - xn_start);
            strb &= strb_mask;
        }

        int more = min(((1 << bz) - byte), (int)(end - max(start, xn_start) + 1));
        if (more > 0) {
            cpp_int_helper::s_array_from_cpp_int(xn_vec[i]->data(), byte, (byte + more - 1), tdata + (max(start, xn_start) - start));
            byte += more;
        }

        if (byte < (1 << bz)) {
            assert(end < xn_end);
            byte += (xn_end - end);
            strb <<= (xn_end - end);
            strb &= strb_mask;
            strb >>= (xn_end - end);
        }
        assert(byte == (1 << bz));
        
        assert(xn_vec[i]->last() == (i == xn - 1));
        strb_vec.emplace_back(strb);
    }

    if (reverse) {
        cap_mem_access::reverse_byte_order(tdata, len, data);
        delete [] tdata;
    }
}


void cap_memacc_handler_c_axi::make_data(uint64_t addr, unsigned char* data, int len, const vector<cap_axi_r_bundle_t*>& xn_vec, vector<cpp_int>& strb_vec, bool reverse)
{
    vector<cap_he_axi_r_bundle_t*> v;
    for (vector<cap_axi_r_bundle_t*>::const_iterator it = xn_vec.cbegin(); it != xn_vec.cend(); it++) {
        v.push_back(clone(*it));
    }
    make_data(addr, data, len, v, strb_vec, reverse);
    for (vector<cap_he_axi_r_bundle_t*>::iterator it = v.begin(); it != v.end(); it++) {
        delete *it;
    }
}


///////
//
bool cap_mem_access::burst_read(uint64_t addr, unsigned char * data, unsigned int len, bool secure, bool reverse)
{
    return read(addr, data, len, secure, reverse);
}


bool cap_mem_access::burst_write(uint64_t addr, const unsigned char * data, unsigned int len, bool secure, bool reverse)
{
    return write(addr, data, len, secure, reverse);
}


void cap_mem_access::reverse_byte_order(const unsigned char* data, int len, unsigned char* odata)
{
    for (int i = 0; i < len; i++) {
        odata[i] = data[len - i - 1];
    }
}


void cap_mem_access::reverse_byte_order(const cpp_int& data, int len, cpp_int& odata)
{
    cpp_int tdata(data);
    odata.assign(0);
    for (int i = 0; i < len; i++) {
        odata <<= 8;
        odata |= (tdata & 0xff);
        tdata >>= 8;
    }
}



#ifdef _CSV_INCLUDED_
cap_memacc_handler_uvm_direct::cap_memacc_handler_uvm_direct(cap_mem_access* memacc)
    : cap_memacc_handler_base(memacc, memacc->name + "/intf_uvm_direct/" + itos(memacc->chip_id), CAP_MEMACC_UVM_DIRECT, 
                              cap_memacc_handler_base::rw_prefer_char_star, cap_memacc_handler_base::rw_prefer_char_star)
{
}


cap_memacc_handler_uvm_direct::~cap_memacc_handler_uvm_direct()
{
}


bool cap_memacc_handler_uvm_direct::write(uint64_t addr, const unsigned char* data, int len, bool secure, bool reverse)
{
  if (0 && (addr >> 63)) { ////host memory, blame Alex if something is broken
        assert(0);
        return false;
    }
    else {
        cpp_int strb(1);
        strb <<= (len);
        strb -= 1;
        if (!reverse) {
            MY_INFO << "[mem_access::intf_uvm_direct] ------" << endl;
            display_data("W", (secure? "S": "N"), 0, strb, data, addr, len, 16);
            dpi_sv_puvm_mem_write(addr, data, len);
        }
        else {
            unsigned char* tdata = new unsigned char [len];
            cap_mem_access::reverse_byte_order(data, len, tdata);
            MY_INFO << "[mem_access::intf_uvm_direct] ------" << endl;
            display_data("W", (secure? "S": "N"), 0, strb, tdata, addr, len, 16);
            dpi_sv_puvm_mem_write(addr, tdata, len);
            delete [] tdata;
        }
        return true;
    }
}


bool cap_memacc_handler_uvm_direct::read(uint64_t addr, unsigned char* data, int len, bool secure, bool reverse)
{
  if (0 && (addr >> 63)) { ////host memory, blame Alex if something is broken
        assert(0);
        return false;
    }
    else {
        cpp_int strb(1);
        strb <<= (len);
        strb -= 1;
        if (!reverse) {
            dpi_sv_puvm_mem_read(addr, data, len);
            MY_INFO << "[mem_access::intf_uvm_direct] ------" << endl;
            display_data("R", (secure? "S": "N"), 0, strb, data, addr, len, 16);
        }
        else {
            unsigned char* tdata = new unsigned char [len];
            dpi_sv_puvm_mem_read(addr, tdata, len);
            cap_mem_access::reverse_byte_order(tdata, len, data);
            MY_INFO << "[mem_access::intf_uvm_direct] ------" << endl;
            display_data("R", (secure? "S": "N"), 0, strb, tdata, addr, len, 16);
            delete [] tdata;
        }

        return true;
    }
}


cap_memacc_handler_avery_direct::cap_memacc_handler_avery_direct(cap_mem_access* memacc)
    : cap_memacc_handler_base(memacc, memacc->name + "/intf_avery_direct/" + itos(memacc->chip_id), CAP_MEMACC_AVERY_DIRECT, 
                              cap_memacc_handler_base::rw_prefer_char_star, cap_memacc_handler_base::rw_prefer_char_star)
{
}


cap_memacc_handler_avery_direct::~cap_memacc_handler_avery_direct()
{
}

bool cap_memacc_handler_avery_direct::write(uint64_t addr, const unsigned char* data, int len, bool secure, bool reverse)
{
  if (0 && (addr >> 63)) { ////host memory, blame Alex if something is broken
        assert(0);
        return false;
    }
    else {
        cpp_int strb(1);
        strb <<= (len);
        strb -= 1;
        if (!reverse) {
            MY_INFO << "[mem_access::intf_avery_direct] ------" << endl;
            display_data("W", (secure? "S": "N"), 0, strb, data, addr, len, 16);
            dpi_sv_puvm_avery_mem_write(addr, data, len);
        }
        else {
            unsigned char* tdata = new unsigned char [len];
            cap_mem_access::reverse_byte_order(data, len, tdata);
            MY_INFO << "[mem_access::intf_avery_direct] ------" << endl;
            display_data("W", (secure? "S": "N"), 0, strb, tdata, addr, len, 16);
            dpi_sv_puvm_avery_mem_write(addr, tdata, len);
            delete [] tdata;
        }
        return true;
    }
}


bool cap_memacc_handler_avery_direct::read(uint64_t addr, unsigned char* data, int len, bool secure, bool reverse)
{
  if (0 && (addr >> 63)) { ////host memory, blame Alex if something is broken
        assert(0);
        return false;
    }
    else {
        cpp_int strb(1);
        strb <<= (len);
        strb -= 1;
        if (!reverse) {
            dpi_sv_puvm_avery_mem_read(addr, data, len);
            MY_INFO << "[mem_access::intf_avery_direct] ------" << endl;
            display_data("R", (secure? "S": "N"), 0, strb, data, addr, len, 16);
        }
        else {
            unsigned char* tdata = new unsigned char [len];
            dpi_sv_puvm_avery_mem_read(addr, tdata, len);
            cap_mem_access::reverse_byte_order(tdata, len, data);
            MY_INFO << "[mem_access::intf_avery_direct] ------" << endl;
            display_data("R", (secure? "S": "N"), 0, strb, tdata, addr, len, 16);
            delete [] tdata;
        }

        return true;
    }
}


///==========
cap_memacc_handler_rtl_fd::cap_memacc_handler_rtl_fd(cap_mem_access* memacc)
    : cap_memacc_handler_base(memacc, memacc->name + "/intf_rtl_frontdoor/" + itos(memacc->chip_id), CAP_MEMACC_RTL_FD, 
                              cap_memacc_handler_base::rw_prefer_char_star, cap_memacc_handler_base::rw_prefer_char_star)
{
}


cap_memacc_handler_rtl_fd::~cap_memacc_handler_rtl_fd()
{
}


bool cap_memacc_handler_rtl_fd::write(uint64_t addr, const unsigned char* data, int len, bool secure, bool reverse)
{
    uint64_t orig_addr = addr;
    if (0 && (addr >> 63)) { ////host memory, blame Alex if something is broken
        assert(0);
        return false;
    }
    else {
        cpp_int strb(1);
        strb <<= (len);
        strb -= 1;

        unsigned char* tdata = new unsigned char [len + 256];

        if (!reverse) {
            memcpy(tdata, data, len);
        }
        else {
            cap_mem_access::reverse_byte_order(data, len, tdata);
        }
        MY_INFO << "[mem_access::intf_rtl_fd] ------" << endl;
        display_data("W", (secure? "S": "N"), 0, strb, tdata, orig_addr, len, 16);
        if (memacc->use_axidb == 0) {
            svScope new_scope;
            svScope old_scope = svGetScope();
            if (sknobs_exists(const_cast<char *>("stub_build"))) {
                new_scope = svGetScopeFromName("\\LIBVERIF.puvm_cpp_reg_shim_pkg ");
            } else {
                new_scope = svGetScopeFromName("puvm_cpp_reg_shim_pkg");
            }
            svSetScope(new_scope);
            const unsigned char* from = tdata;
            while (len > 0) {
                int clen = min(len, 256);
                if ((addr & (-1ul << 8)) != ((addr + clen - 1) & (-1ul << 8))) {
                    clen = (((addr >> 8) + 1) << 8) - addr;
                } 
                axi_xn_write(memacc->arm_agt_idx, addr, (uint32_t*)from, clen, secure);
                addr += clen;
                from += clen;
                len -= clen;
            }
            svSetScope(old_scope);

            delete [] tdata;
        }
        else {
            u_int8_t  *valp = new u_int8_t [len];
            for (int i = 0; i < len; i++) {
                valp[i] = data[i];
            }
            memacc->axi_if->axi_burst_write(addr, len, valp, 256);

            delete [] valp;
        }
        
        return true;
    }
}


bool cap_memacc_handler_rtl_fd::read(uint64_t addr, unsigned char* data, int len, bool secure, bool reverse)
{
    uint64_t orig_addr = addr;
    if (0 && (addr >> 63)) { ////host memory, blame Alex if something is broken
        assert(0);
        return false;
    }
    else {
        cpp_int strb(1);
        strb <<= (len);
        strb -= 1;

        unsigned char tdata[256];
        int rlen = len;
        unsigned char* odata;

        if (reverse) {
            odata = new unsigned char[len];
        }
        else {
            odata = data;
        }
        
        if (memacc->use_axidb == 0) {
            svScope new_scope;
            svScope old_scope = svGetScope();
            if (sknobs_exists(const_cast<char *>("stub_build"))) {
                new_scope = svGetScopeFromName("\\LIBVERIF.puvm_cpp_reg_shim_pkg ");
            } else {
                new_scope = svGetScopeFromName("puvm_cpp_reg_shim_pkg");
            }

            svSetScope(new_scope);
            unsigned char* to = odata;
            while (rlen > 0) {
                int clen = min(rlen, 256);
                if ((addr & (-1ul << 8)) != ((addr + clen - 1) & (-1ul << 8))) {
                    clen = (((addr >> 8) + 1) << 8) - addr;
                }
                axi_xn_read(memacc->arm_agt_idx, addr, (uint32_t*)tdata, clen, secure);
                memcpy(to, tdata, clen);
                addr += clen;
                to += clen;
                rlen -= clen;
            }
            svSetScope(old_scope);
        }
        else {
            u_int8_t *valp = new u_int8_t [len];

            memacc->axi_if->axi_burst_read(addr, len, valp, 256);
            for (int i = 0; i < len; i++) {
                data[i] = valp[i];
            }
            delete [] valp;
        }

        if (reverse) {
            cap_mem_access::reverse_byte_order(odata, len, data);
            delete [] odata;
        }
        
        MY_INFO << "[mem_access::intf_rtl_fd] ------ " << endl; 
        display_data("R", (secure? "S": "N"), 0, strb, data, orig_addr, len, 16);

        return true;
    }
}




#endif //_CSV_INCLUDED_




cap_axi_aw_bundle_t* cap_memacc_handler_c_axi::clone(cap_he_axi_aw_bundle_t* b, bool del)
{
    cap_axi_aw_bundle_t* aw = new cap_axi_aw_bundle_t;
    aw->all(0);
    aw->valid(b->valid());
    aw->id(b->id());
    aw->addr(b->addr());
    aw->burst(b->burst());
    aw->sz(b->sz());
    aw->len(b->len());
    if (del) delete b;
    return aw;
}


cap_axi_ar_bundle_t* cap_memacc_handler_c_axi::clone(cap_he_axi_ar_bundle_t* b, bool del)
{
    cap_axi_ar_bundle_t* ar = new cap_axi_ar_bundle_t;
    ar->all(0);
    ar->valid(b->valid());
    ar->id(b->id());
    ar->addr(b->addr());
    ar->burst(b->burst());
    ar->sz(b->sz());
    ar->len(b->len());
    if (del) delete b;
    return ar;
}


cap_axi_w_bundle_t* cap_memacc_handler_c_axi::clone(cap_he_axi_w_bundle_t* b, bool del)
{
    cap_axi_w_bundle_t* w = new cap_axi_w_bundle_t;
    w->all(0);
    w->valid(b->valid());
    w->data(b->data());
    w->strb(b->strb());
    w->last(b->last());
    if (del) delete b;
    return w;
}


cap_axi_aw_bundle_t* cap_memacc_handler_c_axi::clone(const cap_he_axi_aw_bundle_t* b)
{
    return clone(const_cast<cap_he_axi_aw_bundle_t*>(b), false);
}


cap_axi_ar_bundle_t* cap_memacc_handler_c_axi::clone(const cap_he_axi_ar_bundle_t* b)
{
    return clone(const_cast<cap_he_axi_ar_bundle_t*>(b), false);
}


cap_axi_w_bundle_t* cap_memacc_handler_c_axi::clone(const cap_he_axi_w_bundle_t* b)
{
    return clone(const_cast<cap_he_axi_w_bundle_t*>(b), false);
}


cap_he_axi_r_bundle_t* cap_memacc_handler_c_axi::clone(cap_axi_r_bundle_t* b, bool del)
{
    cap_he_axi_r_bundle_t* r = new cap_he_axi_r_bundle_t;
    r->all(0);
    r->valid(b->valid());
    r->id(b->id());
    r->resp(b->resp());
    r->ready(b->ready());
    r->last(b->last());
    r->data(b->data());
    if (del) delete b;
    return r;
}


template<>
cap_axi_r_bundle_t* cap_memacc_handler_c_axi::clone(cap_axi_r_bundle_t* b, bool del)
{
    cap_axi_r_bundle_t* r = new cap_axi_r_bundle_t;
    r->all(0);
    r->valid(b->valid());
    r->id(b->id());
    r->resp(b->resp());
    r->ready(b->ready());
    r->last(b->last());
    r->data(b->data());
    if (del) delete b;
    return r;
}



cap_he_axi_r_bundle_t* cap_memacc_handler_c_axi::clone(const cap_axi_r_bundle_t* b)
{
    return clone(const_cast<cap_axi_r_bundle_t*>(b), false);
}


template<>
cap_axi_r_bundle_t* cap_memacc_handler_c_axi::clone(const cap_axi_r_bundle_t* b)
{
    return clone<cap_axi_r_bundle_t, cap_axi_r_bundle_t>(const_cast<cap_axi_r_bundle_t*>(b), false);
}


cap_he_axi_aw_bundle_t* cap_memacc_handler_c_axi::clone(cap_axi_aw_bundle_t* b, bool del)
{
    cap_he_axi_aw_bundle_t* aw = new cap_he_axi_aw_bundle_t;
    aw->all(0);
    aw->valid(b->valid());
    aw->id(b->id());
    aw->addr(b->addr());
    aw->burst(b->burst());
    aw->sz(b->sz());
    aw->len(b->len());
    if (del) delete b;
    return aw;
}


cap_he_axi_ar_bundle_t* cap_memacc_handler_c_axi::clone(cap_axi_ar_bundle_t* b, bool del)
{
    cap_he_axi_ar_bundle_t* ar = new cap_he_axi_ar_bundle_t;
    ar->all(0);
    ar->valid(b->valid());
    ar->id(b->id());
    ar->addr(b->addr());
    ar->burst(b->burst());
    ar->sz(b->sz());
    ar->len(b->len());
    if (del) delete b;
    return ar;
}


cap_he_axi_w_bundle_t* cap_memacc_handler_c_axi::clone(cap_axi_w_bundle_t* b, bool del)
{
    cap_he_axi_w_bundle_t* w = new cap_he_axi_w_bundle_t;
    w->all(0);
    w->valid(b->valid());
    w->data(b->data());
    w->strb(b->strb());
    w->last(b->last());
    if (del) delete b;
    return w;
}


cap_axi_r_bundle_t* cap_memacc_handler_c_axi::clone(cap_he_axi_r_bundle_t* b, bool del)
{
    cap_axi_r_bundle_t* r = new cap_axi_r_bundle_t;
    r->all(0);
    r->valid(b->valid());
    r->id(b->id());
    r->resp(b->resp());
    r->ready(b->ready());
    r->last(b->last());
    r->data(b->data());
    if (del) delete b;
    return r;
}


cap_he_axi_aw_bundle_t* cap_memacc_handler_c_axi::clone(const cap_axi_aw_bundle_t* b)
{
    return clone(const_cast<cap_axi_aw_bundle_t*>(b), false);
}


cap_he_axi_ar_bundle_t* cap_memacc_handler_c_axi::clone(const cap_axi_ar_bundle_t* b)
{
    return clone(const_cast<cap_axi_ar_bundle_t*>(b), false);
}


cap_he_axi_w_bundle_t* cap_memacc_handler_c_axi::clone(const cap_axi_w_bundle_t* b)
{
    return clone(const_cast<cap_axi_w_bundle_t*>(b), false);
}


cap_axi_r_bundle_t* cap_memacc_handler_c_axi::clone(const cap_he_axi_r_bundle_t* b)
{
    return clone(const_cast<cap_he_axi_r_bundle_t*>(b), false);
}



///////////

bool cap_memacc_handler_c_axi::splitting(uint64_t axi_addr, int axi_len, int xn_len)
{
    uint64_t start_addr = axi_addr + ((axi_len) << bz);
    uint64_t end_addr = axi_addr + ((axi_len + 1) << bz);
    if ((start_addr & 0xfffffffffffff000) != (end_addr & 0xfffffffffffff000)) {
        return true;
    }

    if ((xn_len & 0x3) == 3) {
        return true;
    }

    return false;
}


void cap_memacc_handler_c_axi::split(list<cap_axi_aw_bundle_t*>& aw, list< list<cap_axi_w_bundle_t*> >& w, const cap_he_axi_aw_bundle_t* he_aw, const vector<cap_he_axi_w_bundle_t*>& he_w)
{
    uint64_t addr = he_aw->addr().convert_to<uint64_t>();
    int len = he_aw->len().convert_to<int>() + 1;

    bool new_xn = true;
    int  xn_len = 0;
    cap_axi_aw_bundle_t* new_aw = NULL;
    list<cap_axi_w_bundle_t*> v;
    cap_axi_w_bundle_t* new_w = NULL;

    assert((unsigned)len == he_w.size());

    for (int i = 0; i < len; i++, xn_len++) {
        if (new_xn) {
            new_aw = clone(he_aw);
            aw.push_back(new_aw);
            new_aw->addr(addr + (i << bz));
            xn_len = 0;
            v.clear();
        }
        const cap_he_axi_w_bundle_t* he_w_i = he_w.at(i);
        new_w = clone(he_w_i);
        v.push_back(new_w);
        if (splitting(addr, i, xn_len)) {
            new_w->last(1);
            new_aw->len(xn_len);
            w.push_back(v);
            new_xn = true;
        }
        else {
            new_xn = false;
        }
    }
    ///last one
    if (!new_xn) {
        assert(xn_len <= 4);
        assert(new_w->last().convert_to<int>() == 1);
        new_aw->len(xn_len - 1);
        w.push_back(v);
    }
}


void cap_memacc_handler_c_axi::split(list<cap_axi_ar_bundle_t*>& ar, const cap_he_axi_ar_bundle_t* he_ar)
{
    uint64_t addr = he_ar->addr().convert_to<uint64_t>();
    int len = he_ar->len().convert_to<int>() + 1;

    bool new_xn = true;
    int xn_len = 0;
    cap_axi_ar_bundle_t* new_ar = NULL;

    for (int i = 0; i < len; i++, xn_len++) {
        if (new_xn) {
            new_ar = clone(he_ar);
            ar.push_back(new_ar);
            new_ar->addr(addr + (i << bz));
            xn_len = 0;
        }
        if (splitting(addr, i, xn_len)) {
            new_ar->len(xn_len);
            new_xn = true;
        }
        else {
            new_xn = false;
        }
    }
    ///last one
    if (!new_xn) {
        assert(xn_len <= 4);
        new_ar->len(xn_len - 1);
    }
}


void cap_memacc_handler_c_axi::merge(vector<cap_he_axi_r_bundle_t*>& he_r, const vector<cap_axi_r_bundle_t*>& r)
{
    if (he_r.size() > 0) {
        vector<cap_he_axi_r_bundle_t*>::reverse_iterator it = he_r.rbegin();
        (*it)->last(0);
    }
    for (vector<cap_axi_r_bundle_t*>::const_iterator it = r.cbegin(); it != r.cend(); it++) {
        const cap_axi_r_bundle_t* r1 = *it;
        he_r.push_back(clone(r1));   
    }
    assert((*he_r.rbegin())->last().convert_to<int>() == 1);
}


////////
void cap_memacc_handler_c_axi::split(list<cap_axi_aw_bundle_t*>& aw, list< list<cap_axi_w_bundle_t*> >& w, const cap_axi_aw_bundle_t* oaw, const vector<cap_axi_w_bundle_t*>& ow)
{
    cap_he_axi_aw_bundle_t* caw = clone(oaw);
    vector<cap_he_axi_w_bundle_t*> cw;
    for (vector<cap_axi_w_bundle_t*>::const_iterator it = ow.cbegin(); it != ow.cend(); it++) {
        cw.push_back(clone(*it));
    }
    split(aw, w, caw, cw);
    delete caw;
    for (vector<cap_he_axi_w_bundle_t*>::iterator it = cw.begin(); it != cw.end(); it++) delete *it;
}


void cap_memacc_handler_c_axi::split(list<cap_axi_ar_bundle_t*>& ar, const cap_axi_ar_bundle_t* oar)
{
    cap_he_axi_ar_bundle_t* car = clone(oar);
    split(ar, car);
    delete car;
}


void cap_memacc_handler_c_axi::merge(vector<cap_axi_r_bundle_t*>& r, const vector<cap_axi_r_bundle_t*>& oor)
{
    if (r.size() > 0) {
        vector<cap_axi_r_bundle_t*>::reverse_iterator it = r.rbegin();
        (*it)->last(0);
    }
    for (vector<cap_axi_r_bundle_t*>::const_iterator it = oor.cbegin(); it != oor.cend(); it++) {
        r.push_back(clone<cap_axi_r_bundle_t>(*it));   
    }
    assert((*r.rbegin())->last().convert_to<int>() == 1);
}


/////
void cap_memacc_handler_c_axi::make_xn(uint64_t addr, const cpp_int& data, int len, list<cap_axi_aw_bundle_t*>& aw_vec, list< list<cap_axi_w_bundle_t*> >& w_vec, bool secure, bool reverse_byte_order)
{
    vector<cap_he_axi_w_bundle_t*> he_w;
    cap_he_axi_aw_bundle_t* he_aw = make_xn<cap_he_axi_aw_bundle_t>(addr, data, len, he_w, secure, reverse_byte_order);
    split(aw_vec, w_vec, he_aw, he_w);
    delete he_aw;
    for (vector<cap_he_axi_w_bundle_t*>::iterator it = he_w.begin(); it != he_w.end(); it++) delete *it;
}


void cap_memacc_handler_c_axi::make_xn(uint64_t addr, const unsigned char* data, int len, list<cap_axi_aw_bundle_t*>& aw_vec, list< list<cap_axi_w_bundle_t*> >& w_vec, bool secure, bool reverse_byte_order)
{
    vector<cap_he_axi_w_bundle_t*> he_w;
    cap_he_axi_aw_bundle_t* he_aw = make_xn<cap_he_axi_aw_bundle_t>(addr, data, len, he_w, secure, reverse_byte_order);
    split(aw_vec, w_vec, he_aw, he_w);
    delete he_aw;
    for (vector<cap_he_axi_w_bundle_t*>::iterator it = he_w.begin(); it != he_w.end(); it++) delete *it;
}


void cap_memacc_handler_c_axi::make_xn(uint64_t addr, const cpp_int& data, int len, vector<cap_axi_aw_bundle_t*>& aw_vec, vector< vector<cap_axi_w_bundle_t*> >& w_vec, bool secure, bool reverse_byte_order)
{
    list<cap_axi_aw_bundle_t*> aw_lst;
    list< list<cap_axi_w_bundle_t*> > w_lst;

    make_xn(addr, data, len, aw_lst, w_lst, secure, reverse_byte_order);

    while (aw_lst.size() > 0) {
        aw_vec.push_back(aw_lst.front());
        aw_lst.pop_front();
    }

    vector<cap_axi_w_bundle_t*> v;
    while (w_lst.size() > 0) {
        list<cap_axi_w_bundle_t*>& l = w_lst.front();
        v.clear();
        while (l.size() > 0) {
            v.push_back(l.front());
            l.pop_front();
        }
        w_vec.push_back(v);
        w_lst.pop_front();
    }
}


void cap_memacc_handler_c_axi::make_xn(uint64_t addr, const unsigned char* data, int len, vector<cap_axi_aw_bundle_t*>& aw_vec, vector< vector<cap_axi_w_bundle_t*> >& w_vec, bool secure, bool reverse_byte_order)
{
    list<cap_axi_aw_bundle_t*> aw_lst;
    list< list<cap_axi_w_bundle_t*> > w_lst;

    make_xn(addr, data, len, aw_lst, w_lst, secure, reverse_byte_order);

    while (aw_lst.size() > 0) {
        aw_vec.push_back(aw_lst.front());
        aw_lst.pop_front();
    }

    vector<cap_axi_w_bundle_t*> v;
    while (w_lst.size() > 0) {
        list<cap_axi_w_bundle_t*>& l = w_lst.front();
        v.clear();
        while (l.size() > 0) {
            v.push_back(l.front());
            l.pop_front();
        }
        w_vec.push_back(v);
        w_lst.pop_front();
    }
}

void cap_memacc_handler_c_axi::make_xn(uint64_t addr, int len, list<cap_axi_ar_bundle_t*>& ar, bool secure)
{
    cap_he_axi_ar_bundle_t* he_ar = make_xn<cap_he_axi_ar_bundle_t>(addr, len, secure);
    split(ar, he_ar);
    delete he_ar;
}


void cap_memacc_handler_c_axi::make_xn(uint64_t addr, int len, vector<cap_axi_ar_bundle_t*>& ar, bool secure)
{
    list<cap_axi_ar_bundle_t*> ar_lst;
    make_xn(addr, len, ar_lst, secure);
    while (ar_lst.size() > 0) {
        ar.push_back(ar_lst.front());
        ar_lst.pop_front();
    }
}


///==========
cap_memacc_handler_host::cap_memacc_handler_host(cap_mem_access* memacc)
    : cap_memacc_handler_base(memacc, memacc->name + "/intf_host/" + itos(memacc->chip_id), CAP_MEMACC_HOST, 
                              cap_memacc_handler_base::rw_prefer_char_star, cap_memacc_handler_base::rw_prefer_char_star)
{
}


cap_memacc_handler_host::~cap_memacc_handler_host()
{
}


bool cap_memacc_handler_host::write(uint64_t addr, const unsigned char* data, int len, bool secure, bool reverse)
{
  if (0 && (addr >> 63)) { ////host memory, blame Alex if something is broken
        assert(0);
        return false;
    }
    else {
        cpp_int strb(1);
        strb <<= (len);
        strb -= 1;

        unsigned char* tdata = new unsigned char [len + 256];

        if (!reverse) {
            memcpy(tdata, data, len);
        }
        else {
            cap_mem_access::reverse_byte_order(data, len, tdata);
        }
        MY_INFO << "[mem_access::intf_host] ------" << endl;
        display_data("W", (secure? "S": "N"), 0, strb, tdata, addr, len, 16);
        
        vector<uint32_t> dv;
        for (int i = 0; i < (len + 3) / 4; i++) {
            uint32_t d = 0;
            for (int j = 0; j < 4 && i * 4 + j < len; j++) {
                d |= (((uint32_t)data[i * 4 + j]) << (j * 8));
            }
            dv.push_back(d);
        }
        cpu::access()->block_write(0, addr, dv.size(), dv, true, (secure)? secure_byte_acc_e: none_acc_e);
        delete [] tdata;
        return true;
    }
}


bool cap_memacc_handler_host::read(uint64_t addr, unsigned char* data, int len, bool secure, bool reverse)
{
  if (0 && (addr >> 63)) { ////host memory, blame Alex if something is broken
        assert(0);
        return false;
    }
    else {
        cpp_int strb(1);
        strb <<= (len);
        strb -= 1;

        unsigned char* odata;
        if (reverse) {
            odata = new unsigned char[len];
        }
        else {
            odata = data;
        }

        vector<uint32_t> dv;
        int  word_len = (len + 3)/4;
        dv = cpu::access()->block_read(0, addr, word_len, true, (secure)? secure_byte_acc_e: none_acc_e);
        for (int i = 0; i < (len + 3) / 4; i++) {
            for (int j = 0; j < 4 && i * 4 + j < len; j++) {
                odata[i * 4 + j] = (unsigned char)(dv[i] >> (j * 8));
            }
        }

        if (reverse) {
            cap_mem_access::reverse_byte_order(odata, len, data);
            delete [] odata;
        }
        
        MY_INFO << "[mem_access::intf_host] ------ " << endl; 
        display_data("R", (secure? "S": "N"), 0, strb, data, addr, len, 16);

        return true;
    }
}



