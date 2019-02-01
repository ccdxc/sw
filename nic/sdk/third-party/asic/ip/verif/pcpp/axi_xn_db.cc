#include "axi_xn_db.h"

map<int, atomic<axi_xn_db * > >  axi_xn_db::_axi_xn_db_inst;
uint32_t axi_xn_t::cur_context_id = 0;

std::mutex axi_xn_db::rsp_mtx;
std::mutex axi_xn_db::req_mtx;

axi_xn_db * axi_xn_db::access(int _master_idx) {

    if(_axi_xn_db_inst.find(_master_idx) == _axi_xn_db_inst.end()) {
        _axi_xn_db_inst[_master_idx] = new axi_xn_db(_master_idx);
        PLOG_MSG("new a axi_xn_db..." << endl)
     }
  return _axi_xn_db_inst[_master_idx];
}

axi_xn_db::axi_xn_db(int _master_idx) {
    _axi_xn_db_inst[_master_idx] = this;
}

void axi_xn_db::flush_all_pending() {
    while(request_db_array.size() || pending_db_array.size() ) { SLEEP(10); }
}
void axi_xn_db::wait_for_response() { 
    // first make sure we have size available
    while(rsp_ready_array.size() == 0) { SLEEP(10); }
}

axi_attribute_profile_t::axi_attribute_profile_t() {
    lock = -1;
    prot = -1;
    cache = -1;
    qos = -1;
    slave_port = -1;
}
axi_xn_t::axi_xn_t() {
    id = -1;
    lock = -1; 
    prot = -1; 
    cache = -1 ;
    qos = -1;
    burst_len = -1; 
    burst_size = -1; 
    burst_type = -1;
    write_strobe = -1;
    slave_port = -1;
    status = -1;

}
void axi_xn_t::set_xn_directed_options(const int _id, const u_int8_t _lock, const u_int8_t _prot, const u_int8_t _cache, 
                const u_int8_t _qos, const u_int8_t _b_len, const u_int8_t _b_size, const u_int8_t _b_type) {
    id = _id;
    lock = _lock;
    prot = _prot;
    cache = _cache;
    qos = _qos;
    burst_len = _b_len;
    burst_size = _b_size;
    burst_type = _b_type;
}

void axi_xn_t::set_xn_profile_options(std::shared_ptr<axi_attribute_profile_t> _profile) {
    lock = _profile->lock;
    prot = _profile->prot;
    cache = _profile->cache;
    qos = _profile->qos;
    slave_port = _profile->slave_port;

}

void axi_xn_t::set_xn_options(const u_int64_t _addr, const u_int16_t _size, axi_xn_type _type)  {
    address = _addr;
    req_data_size = _size;
    xn_type = _type;
    context_id = get_next_context_id();
}


void axi_xn_t::set_data(const u_int16_t _size, u_int8_t * _valp) {
    for(u_int16_t idx = 0; idx < _size; idx++) {
        data.push_back(_valp[idx]);
    }
}

void axi_xn_t::get_data(const u_int16_t _size, u_int8_t * _valp) {
    if(data.size() != _size) {
        PLOG_ERR("size is not matching with avail data, skip further process" << endl);
        return;
    }
    for(u_int16_t idx = 0; idx < _size; idx++) {
        _valp[idx] = data[idx];
        //PLOG_MSG("get_data: _valp[" << idx << "] = " << hex << int(_valp[idx]) << endl)
    }
}


std::shared_ptr<axi_attribute_profile_t> axi_xn_db::find_profile(const u_int64_t addr) {
    for(auto profile_ptr : profile_array) {
        if( (addr >= profile_ptr->start_address) &&
                (addr <= profile_ptr->end_address)) {
            return profile_ptr;
        }
    }
    return std::shared_ptr<axi_attribute_profile_t>(0);
    //return nullPtr;
}

uint32_t axi_xn_db::nb_write(const u_int64_t _addr, const u_int16_t _size, u_int8_t *valp, const uint32_t flags) {
    std::shared_ptr< axi_xn_t > req_ptr (new axi_xn_t());
    
    PLOG("axi_xn_db", "nb_write:addr = " << hex << _addr << ", _size = " << dec << _size << endl)
    req_ptr->set_xn_options(_addr, _size, axi_xn_t::AXI_WR);
    req_ptr->set_data(_size, valp);
    auto cur_profile = find_profile(_addr);
    if(cur_profile) {
        req_ptr->set_xn_profile_options(cur_profile);
    }
    else {
        req_ptr->prot = flags;
    }

    req_mtx.lock();
    request_db_array.push_back(req_ptr);
    req_mtx.unlock();

    PLOG("axi_xn_db", "nb_write req received for context_id : 0x" << hex << req_ptr->context_id << dec <<endl); 
    fflush(stdout); fflush(stderr);
    return req_ptr->context_id;
}


int axi_xn_db::nb_write_rsp(const uint32_t context_id, u_int8_t & status) {
    int ret = -1;
    unsigned idx = 0;

    rsp_mtx.lock();

    for(auto it : rsp_ready_array) {
        if( it->context_id == context_id) {
            ret = 0;
            status = it->status;
            break;
        }
        idx++;
    }

    if(ret != -1) { 
        PLOG("axi_xn_db", "nb_write_rsp done for context_id : 0x" << hex << context_id << dec <<endl); 
        rsp_ready_array.erase(rsp_ready_array.begin() + idx);  
    }

    rsp_mtx.unlock();

    return ret;
}


uint32_t axi_xn_db::nb_read(const u_int64_t _addr, const u_int16_t _size, const u_int32_t flags) {
    std::shared_ptr< axi_xn_t > req_ptr (new axi_xn_t());
    
    PLOG("axi_xn_db", "nb_read:addr = " << hex << _addr << ", _size = " << dec << _size << endl)
    req_ptr->set_xn_options(_addr, _size, axi_xn_t::AXI_RD);
    auto cur_profile = find_profile(_addr);
    if(cur_profile) {
        req_ptr->set_xn_profile_options(cur_profile);
    }
    else {
        req_ptr->prot = flags;
    }

    req_mtx.lock();
    request_db_array.push_back(req_ptr);
    req_mtx.unlock();

    PLOG("axi_xn_db", "nb_read req received for context_id : 0x" << hex << req_ptr->context_id << dec <<endl); 
    fflush(stdout); fflush(stderr);
    return req_ptr->context_id;
}


int axi_xn_db::nb_read_rsp(const uint32_t context_id, u_int16_t & size, u_int8_t *valp, u_int8_t & status) {
    int ret = -1;
    unsigned idx = 0;

    rsp_mtx.lock();

    for(auto it : rsp_ready_array) {
        if( it->context_id == context_id) {
            size = it->data.size();
            it->get_data(size, valp);
            ret = 0;
            status = it->status;
            break;
        }
        idx++;
    }

    if(ret != -1) { 
        PLOG("axi_xn_db", "nb_read_rsp done for context_id : 0x" << hex << context_id << dec <<endl); 
        rsp_ready_array.erase(rsp_ready_array.begin() + idx);  
    }

    rsp_mtx.unlock();

    return ret;
}


void axi_xn_db::set_profile(std::shared_ptr<axi_attribute_profile_t> _profile) {
    profile_array.push_back(_profile);
}

std::shared_ptr<axi_xn_t> axi_xn_db::get_next_req() {

    if(request_db_array.size() == 0) {
        PLOG_ERR("nothing in the request_db_array" << endl)
        return nullptr;
    }
    
    req_mtx.lock();

    std::shared_ptr<axi_xn_t>  req;
    req = request_db_array.front();
    request_db_array.erase(request_db_array.begin());
    req_mtx.unlock();
    
    rsp_mtx.lock();
    pending_db_array.push_back(req);
    rsp_mtx.unlock();
    PLOG("axi_xn_db", "get_next_req:addr = 0x" << hex << req->address << ", xact_type = " << int(req->xn_type) << ", context_id = 0x" << hex << req->context_id << endl);
    fflush(stdout); fflush(stderr);

    return req;
}

int axi_xn_db::is_req_available() {
    int   ok;

    if (request_db_array.size() > 0) {
        ok = 1;
    }
    else {
        ok = 0;
    }

    return ok;
}

int axi_xn_db::find_rsp_size(const uint32_t context_id) {
    int ret = -1;
    int req_size = 0;

    rsp_mtx.lock();
    for (auto it : pending_db_array) {
        if (it->context_id == context_id) {
            ret = 0;
            req_size = it->req_data_size;

            break;
        }
    }
    rsp_mtx.unlock();

    if (ret == -1) {
        PLOG_ERR("axi_xn_db::find_rsp_size, can not find context_id = 0x" << hex << context_id << endl)
    }
    return req_size;
}

int axi_xn_db::get_response(const uint32_t context_id, u_int8_t status, u_int8_t *valp) {
    int   ret = -1;
    unsigned idx = 0;
    
    //PLOG_MSG("start get_response: context_id = 0x" << hex << context_id << endl)
    rsp_mtx.lock();
    for(auto it : pending_db_array) {
        if(it->context_id == context_id) {

            it->status = status;
            if (it->xn_type == axi_xn_t::AXI_RD) {
                it->set_data(it->req_data_size, valp);
            }
            ret =0;
            rsp_ready_array.push_back(it);
            PLOG("axi_xn_db", "get_response: context_id = 0x" << hex << context_id << ", size = " << it->req_data_size << ", status = " << int(status) << endl)

            if (it->status > 1) {
                if (SKNOBS_GET("axi/axi_rpt_err", 1)) {
                    if (it->xn_type == axi_xn_t::AXI_RD) {
                        PLOG_ERR("AXI_XN_DB_RD: addr = 0x" << hex << it->address << ", req_data_size = " << dec << it->req_data_size << ", rresp = " << int(status) << endl)
                    }
                    else {
                        PLOG_ERR("AXI_XN_DB_WR: addr = 0x" << hex << it->address << ", req_data_size = " << dec << it->req_data_size << ", bresp = " << int(status) << endl)
                    }
                }
            }
            fflush(stdout); fflush(stderr);
            break;
        }
        idx++;
    }

    if(ret!=-1) {
        pending_db_array.erase(pending_db_array.begin() + idx);
    }
    else {
        PLOG_ERR("get_response::can not find context_id = 0x" << hex <<  context_id << " in pending_db_array" << endl)
    }
    rsp_mtx.unlock();

    return ret;
}

std::shared_ptr<axi_xn_t> axi_xn_db::pop_response() {
    std::shared_ptr<axi_xn_t>  rsp;
    
    rsp_mtx.lock();

    if (rsp_ready_array.size() > 0) {
        rsp = rsp_ready_array.front();
        rsp_ready_array.erase(rsp_ready_array.begin());
    }
    else  {
        rsp = nullptr;
    }
    rsp_mtx.unlock();

    return rsp;
}

bool axi_xn_db::axi_burst_write(const u_int64_t _addr, const u_int16_t _size, u_int8_t *valp, const u_int32_t flags, const int boundary) {
    int               left_len = _size;
    auto              addr_idx = _addr;
    unsigned          block_size;
    u_int8_t *        ptr_idx = valp;
    vector<uint32_t>  context_array;
    uint32_t          context_id;
    bool              ret = true;
    bool              ok;

    int bit_num = get_bit_num(boundary);
    
    PLOG("axi_xn_db", "axi_burst_write: _addr = " << hex << _addr << ", _size = " << dec << _size << endl)
    while(left_len) {
        block_size = std::min(left_len, boundary);
        if ((addr_idx & (-1ul << bit_num)) != ((addr_idx + block_size - 1) & (-1ul << bit_num))) {
            block_size = (((addr_idx >> bit_num) + 1)<< bit_num) - addr_idx;
        }
        context_id = nb_write(addr_idx, block_size, ptr_idx, flags);
        left_len = left_len - block_size;
        addr_idx = addr_idx + block_size;
        ptr_idx  = ptr_idx + block_size;
        context_array.push_back(context_id);
    }
    
    while(1) {
        rsp_mtx.lock();
        for (auto it = rsp_ready_array.begin(); it != rsp_ready_array.end();) {
            if (context_array.size() == 0) break;
            ok = false;

            for (unsigned int i = 0; i < context_array.size();) {
                if ((*it)->context_id == context_array[i]) {
                    context_array.erase(context_array.begin() + i);
                    
                    if (int((*it)->status) >= 2)
                        ret = false;
                    ok = true;
                    break;
                }
                i++;
            }
            if (ok == true)
                it = rsp_ready_array.erase(it);
            else
                it++;
        }
        rsp_mtx.unlock();
        
        if (context_array.size() > 0) {
            SLEEP(2);
        }
        else {
            break;
        }
    }
    
    return ret;
}

bool axi_xn_db::axi_burst_read(const u_int64_t _addr, const u_int16_t _size, u_int8_t *valp, const u_int32_t flags, const int boundary) {
    int                left_len = _size;
    auto               addr_idx = _addr;
    unsigned           block_size;
    vector<int>        context_len; 
    vector<uint32_t>   context_array;
    uint32_t           context_id;
    bool               ret = true;
    bool               ok;
    int                xact_num = 0;
    int                sta_ptr;
    
    PLOG("axi_xn_db", "axi_burst_read: _addr = " << hex << _addr << ", _size = " << dec << _size << endl)

    int bit_num = get_bit_num(boundary);
    context_len.reserve(200);

    while(left_len) {
        block_size = std::min(left_len, boundary);
        if ((addr_idx & (-1ul << bit_num)) != ((addr_idx + block_size - 1) & (-1ul << bit_num))) {
            block_size = (((addr_idx >> bit_num) + 1) << bit_num) - addr_idx;
        }
        context_id = nb_read(addr_idx, block_size, flags); 
        left_len   = left_len - block_size;
        addr_idx   = addr_idx + block_size;
        context_array.push_back(context_id);
        context_len.push_back(block_size);
        xact_num ++;
        if (xact_num > 200) {
            context_len.reserve(xact_num + 200);
        }
    }
    context_len.resize(xact_num);

    while(1) {
        rsp_mtx.lock();

        for (auto it = rsp_ready_array.begin(); it != rsp_ready_array.end();) {

            ok = false; sta_ptr = 0;
            for (unsigned int i = 0; i < context_array.size(); ) {
                if ((*it)->context_id == context_array[i]) {
                    xact_num --;
                    ok = true;

                    for (int k = 0; k < context_len[i]; k++) {
                        valp[sta_ptr +k] = (*it)->data[k];
                        //PLOG_MSG("valp[" << dec << sta_ptr+k << "] = " << hex << int(valp[sta_ptr+k]) << endl)
                    }
                    if (int((*it)->status) >= 2)
                        ret = false;
                    break;
                }
                sta_ptr = sta_ptr + context_len[i];
                i++;
            }
            if (ok == true)
                it = rsp_ready_array.erase(it);
            else
                it ++;
            
            if (xact_num == 0) break;
        }

        rsp_mtx.unlock();
        
        if (xact_num == 0) break;

        SLEEP(2);
    }
    context_array.clear();
    context_len.clear();

    return ret;
}

int axi_xn_db::get_bit_num(int boundary) {
     
    int  bit_num = 1; 
    int  bound_tmp = boundary;

    while(bound_tmp/2 > 1) {
        bit_num ++;
        bound_tmp = bound_tmp/2;
    }
    
    return bit_num;
}
