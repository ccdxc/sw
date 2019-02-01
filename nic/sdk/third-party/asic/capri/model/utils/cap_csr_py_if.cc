#include "cap_csr_py_if.h"
#include "cap_csr_base.h"
#include "cap_blk_reg_model.h"
#include "LogMsg.h"

// helper functions
uint32_t cur_chip_id =0;
uint32_t cur_inst_id =0;

class inst_info {
    public:
        uint32_t chip_id, inst_id;
        string prefix;
        inst_info(string path, uint32_t c_id, uint32_t i_id) {
            chip_id = c_id;
            prefix = path;
            inst_id = i_id;
        }
        virtual ~inst_info() {}
        inst_info() { }
};

vector< std::shared_ptr<inst_info> > inst_info_array;

uint32_t csr_get_cur_chip_id() { return cur_chip_id; }
uint32_t csr_get_cur_inst_id() { return cur_inst_id; }
void register_chip_inst(string path, uint32_t chip_id, uint32_t inst_id) {
    std::shared_ptr<inst_info> ptr (new inst_info(path, chip_id, inst_id));
    inst_info_array.push_back(ptr);
}


/////////////////////////////////////////////////////
string csr_convert_2_sbrackets(string path_name) {
  string out_path_name = path_name;
  uint32_t len = out_path_name.size();
  for(uint32_t ii=0; ii<len; ii++) {
    if((out_path_name[ii]=='}') || (out_path_name[ii]==')')) out_path_name[ii] = ']';
    else if((out_path_name[ii]=='{') || (out_path_name[ii]=='(')) out_path_name[ii] = '[';
  }
  return(out_path_name);
}

/////////////////////////////////////////////////////
void delete_chip_inst(string path_, uint32_t chip_id, uint32_t inst_id) {
    //string path = path_;
    string path = csr_convert_2_sbrackets(path_);
    for(vector<std::shared_ptr<inst_info> >::iterator it = inst_info_array.begin(); it != inst_info_array.end(); it++) {
        std::shared_ptr<inst_info> itr = *it;
        if( (itr->chip_id == chip_id) && (itr->inst_id == inst_id) &&
                (itr->prefix.find(path) != string::npos)) {
            inst_info_array.erase(it);
            return; 
        }
    }

    PLOG_ERR("can not find path" << path << endl);
}

///////////////////////////////////////////////////
uint32_t  plog_get_err_count() {
 return ( PLOG_GET_ERR_CNT() );
}

///////////////////////////////////////////////////
void  plog_clr_err_count() {
  PLOG_SET_ERR_CNT(0);
}

///////////////////////////////////////////////////
void plog_set_max_err_count(uint32_t max_err_cnt) {
  PLOG_SET_MAX_ERR_CNT(max_err_cnt);
}

///////////////////////////////////////////////////
void plog_msg(string msg) {
  PLOG_MSG(""  << msg << endl);
}

///////////////////////////////////////////////////
void plog_err(string msg) {
  PLOG_ERR(""  << msg << endl);
}

///////////////////////////////////////////////////
void plog_start(string fName, uint32_t size) {
    LogMsg::Instance()->startFlog(fName, size);
}

///////////////////////////////////////////////////
void plog_stop() {
    LogMsg::Instance()->stopFlog();
}

pen_csr_base * get_csr_base_from_path(string path_) {
    //string path = path_;
    string path = csr_convert_2_sbrackets(path_);
    pen_csr_base * objP=0;
    for(auto itr: inst_info_array) {

        // second, check for registered blocks
        for(auto blk : CAP_BLK_REG_MODEL_GET_ALL_CSRS(itr->chip_id)) {
            if(path.compare(blk->get_hier_path()) == 0) {
                objP = blk;
                break;
            }

            if( (path.compare(blk->get_hier_path()) == 0) ||
                (path.find(blk->get_hier_path() + ".") != string::npos)) {
                //string search_str=path;
                //auto pos = search_str.find(blk->get_hier_path() + '.');
                //if (pos == 0) search_str.erase(pos,  string(blk->get_hier_path()+'.').length() );
                //else { 
                //    auto pos = search_str.find(blk->get_hier_path());
                //    if (pos == 0) search_str.erase(pos , blk->get_hier_path().length());
                //}
                objP = blk->search_csr_by_name(path);
                break;
            }
        }
        if(objP != 0) break;
    }

    if(objP ==0) {
        PLOG_MSG("path : " << path << " not found!" << endl);
    }
    return objP;

}
void apply_action(pen_csr_base * objP, string action) {
    if(!objP) return;
    for(auto itr : cap_util_split_by_str(action, ':')) {
        if(!itr.compare("read")) objP->read();
        else if(!itr.compare("show")) objP->show();
    }

    
}
void csr_set_cur_chip_id(uint32_t id) { cur_chip_id = id; }
void csr_set_cur_inst_id(uint32_t id) { cur_inst_id = id; }

void csr_write_addr(uint64_t addr, uint32_t data, uint32_t flags) {
    cpu::access()->write(csr_get_cur_chip_id(), addr,data,0, flags);
}
uint32_t csr_read_addr(uint64_t addr, uint32_t flags) {
    return cpu::access()->read(csr_get_cur_chip_id(), addr, 0, flags);
}

void csr_block_write_addr(uint64_t addr, int size, const std::vector<unsigned int>& data, uint32_t secure) {
    cpu::access()->block_write(csr_get_cur_chip_id(), addr, size, data,secure);
}
std::vector<unsigned int> csr_block_read_addr(uint64_t addr, int size, uint32_t secure) {
    return cpu::access()->block_read(csr_get_cur_chip_id(), addr, size, secure);
}

void csr_write(string path, string data) {
    pen_csr_base * objP = get_csr_base_from_path(path);
    if(objP == 0) return;
    objP->all(cpp_int(data));
    objP->write();
}

void csr_block_write(string path, string data) {
    pen_csr_base * objP = get_csr_base_from_path(path);
    if(objP == 0) return;
    objP->all(cpp_int(data));
    objP->block_write();
}

string csr_read(string path) {
    pen_csr_base * objP = get_csr_base_from_path(path);
    if(objP == 0) return "0xdeadbeef";

    objP->read();
    cpp_int data = objP->all();
    stringstream ss;
    ss << hex << "0x" << data;
    return ss.str();
}

string csr_block_read(string path) {
    pen_csr_base * objP = get_csr_base_from_path(path);
    if(objP == 0) return "0xdeadbeef";

    objP->block_read();
    cpp_int data = objP->all();
    stringstream ss;
    ss << hex << "0x" << data;
    return ss.str();
}

void csr_write_field(string path, string field, string data, int shadow_update_only) {
    pen_csr_base * objP = get_csr_base_from_path(path);
    if(objP == 0) return;
    cpp_int val (data);
    bool success = objP->set_field_val(field, val);
    if(success) {
        if(shadow_update_only==0) { objP->write(); }
    } else {
        PLOG_MSG("csr_write_field fail for " << path << " field: " << field << endl);
    }

}
string csr_read_field(string path, string field, int shadow_update_only) {
    pen_csr_base * objP = get_csr_base_from_path(path);
    if(objP == 0) return "0xdeadbeef";
    if(shadow_update_only ==0) { objP->read(); }
    cpp_int data;
    bool success = objP->get_field_val(field, data);
    if(success) {
        stringstream ss;
        ss << hex << "0x"  << data;
        return ss.str();
    } else {
        PLOG_MSG("csr_read_field fail for " << path << " field: " << field << endl);
    }
    return "0xdeadbeef";
}

void csr_set_field_val(string path, string field, string data) {
    csr_write_field(path, field, data, 1);
}
string csr_get_field_val(string path, string field) {
    return csr_read_field(path, field, 1);
}

void csr_find(string path, string pattern, int level, string action) {
    pen_csr_base * objP = get_csr_base_from_path(path);
    if(objP == 0) return;
    for(auto itr : objP->get_children_string(pattern, level)) {
        PLOG_MSG(itr->get_hier_path() << endl);
        apply_action(itr, action);
    }
    
}


void csr_prefix(string path, string pattern, int level, string action) {
    pen_csr_base * objP = get_csr_base_from_path(path);
    if(objP == 0) return;
    for(auto itr : objP->get_children_prefix(pattern, level)) {
        PLOG_MSG(itr->get_hier_path() << endl);
        apply_action(itr, action);
    }
    
}


/*
void csr_search(string rgx) {
    
    //regx is delimited with //
    while(rgx.find("./") != string::npos) {
        
    }
    pen_csr_base * objP = get_csr_base_from_path(path);
    if(objP == 0) return;
    for(auto itr : objP->get_children(level)) {
        PLOG_MSG(itr->get_hier_path() << endl);
        apply_action(itr, action);
    }
    
}
*/

void csr_list(string path, int level, string action) {
    pen_csr_base * objP = get_csr_base_from_path(path);
    if(objP == 0) return;
    for(auto itr : objP->get_children(level)) {
        PLOG_MSG(itr->get_hier_path() << endl);
        apply_action(itr, action);
    }
    
}
void csr_show(string path, int level) {
    pen_csr_base * objP = get_csr_base_from_path(path);
    if(objP == 0) return;
    objP->show();
}


uint64_t csr_get_offset(string path) {
    pen_csr_base * objP = get_csr_base_from_path(path);
    if(objP == 0) return 0xdeadbeef;
    return objP->get_offset();
}

//string csr_get_type_name(string path) {
//    pen_csr_base * objP = get_csr_base_from_path(path);
//    if(objP == 0) return "unknown";
//    return objP->get_type_name();
//}


void plog_add_scope(string scope) {
    LogMsg::Instance()->enableScope(scope);
}

void plog_remove_scope(string scope) {
    LogMsg::Instance()->disableScope(scope);
}
void plog_set_verbose(uint32_t level) {
    LogMsg::Instance()->setVerbose(LogMsg::msgLevelEnum(level));
}

void cpu_set_cur_if_name(char * _name) {
    cpu::access()->set_cur_if_name((string) _name);
}
char * cpu_get_cur_if_name() {
    return (char *) cpu::access()->get_cur_if_name().c_str();
}

string csr_read_burst(uint64_t addr, uint32_t len, uint32_t flags) {

    stringstream ss;
    if(len == 0) {
        ss << hex << 0 ;
    } else {
        unsigned char * data_tmp = new unsigned char[len];
        cpu::access()->burst_read(csr_get_cur_chip_id(), addr, len , data_tmp, 0, flags); 
        ss << hex << "0x" << data_tmp;
        delete [] data_tmp;
    }
    return ss.str();
}


void csr_write_burst(uint64_t addr, uint32_t len, string data, uint32_t flags) {
    cpp_int val (data);

    if(len == 0) return;
    unsigned char * data_tmp = new unsigned char[len];
    for(unsigned ii=0; ii < len; ii++) {
        data_tmp[ii] = (val.convert_to<unsigned char>() & 0xff) ;
        val = val>>8;
    }
    cpu::access()->burst_write(csr_get_cur_chip_id(), addr, len , data_tmp, 0, flags); 
    delete [] data_tmp;
}

void cpu_force_global_flags(unsigned _val) {
    cpu::access()->set_global_flags(_val);
}

