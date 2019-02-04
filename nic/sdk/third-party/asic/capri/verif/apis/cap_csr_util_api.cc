
#include "cap_csr_util_api.h"
#include "pknobs.h"

void cap_csr_randomize(pen_csr_base * csr_ptr, bool do_write) {
    static pknobs::RRKnob rand_knob("rand_knob", 0 , 0xffffffff);

    if( (csr_ptr->get_csr_type() == pen_csr_base::CSR_TYPE_DECODER) ||
            (csr_ptr->get_csr_type() == pen_csr_base::CSR_TYPE_REGISTER)) {
        int width = csr_ptr->get_width();
        cpp_int data;
        for(int i = 0; i < (width+31)/32; i++) {
            data = (data << 32) | rand_knob.eval();
        }
        csr_ptr->all(data);
        if(do_write) {
           csr_ptr->write();
        }
    }

    if( (csr_ptr->get_csr_type() == pen_csr_base::CSR_TYPE_MEMORY) ||
            (csr_ptr->get_csr_type() == pen_csr_base::CSR_TYPE_BLOCK)) {
        auto children = csr_ptr->get_children();
        for(auto itr : children) {
            cap_csr_randomize(itr, do_write);
        }
    }

}




void enable_all_interrupts(pen_csr_base * block_base, int level) {

    for(auto int_objs : block_base->get_children_prefix("csr_intr", level)) {
        int_objs->set_access_no_zero_time(1);
        int_objs->all(0);
        int_objs->all( int_objs->all() - 1);
        int_objs->write();
    }

    for(auto int_objs : block_base->get_children_prefix("int_enable_set", level<0 ? level: level+1)) {
        int_objs->set_access_no_zero_time(1);
        int_objs->all(0);
        int_objs->all( int_objs->all() - 1);
        int_objs->write();
    }

    for(auto int_objs : block_base->get_children_prefix("int_enable_rw_reg", level<0 ? level : level+1)) {
        int_objs->set_access_no_zero_time(1);
        int_objs->all(0);
        int_objs->all( int_objs->all() - 1);
        int_objs->write();
    }


}

void clear_all_interrupts(pen_csr_base * block_base, int level) {
    for(auto int_objs : block_base->get_children_prefix("intreg", level)) {
        //if(int_objs->get_type_name().find("intgrp_status_t") == string::npos) {
            int_objs->all(0);
            int_objs->all( int_objs->all() - 1);
            int_objs->write();
        //}
    }
}



void check_interrupts(pen_csr_base * block, bool print_only, int level, bool root_enabled, string slv_status_name, int verbose) {


    // loop 2 times
    // 1. for the current block
    // 2. for the blocks instantiated by this block

    for(int itr_loop =0; itr_loop < 2; itr_loop++) {

        vector<pen_csr_base*> all_children;
        if(itr_loop == 0) {
            all_children.clear();
            all_children.push_back(block);
        } else {
            all_children.clear();
            all_children = block->get_children(level);
        }
        for(auto all_objs : all_children) {
            if(all_objs->get_csr_type() == pen_csr_base::CSR_TYPE_BLOCK) {
                auto int_root_ptr = all_objs->search_csr_by_name(slv_status_name, true);
                if(int_root_ptr) {
                    cpp_int ret_val;
                    if(int_root_ptr->get_parent() != block) {
                        ret_val = show_interrupt_tree(all_objs, root_enabled, print_only, level<0?-1: level-1);
                    } else {
                        ret_val = show_interrupt_tree(all_objs, root_enabled, print_only, level);
                    }
                    if(root_enabled && (print_only ==0) && (ret_val != 0)) {
                        PLOG_ERR("Unexpected int set: " << all_objs->get_hier_path() << endl);
                    }
                } else {
                    if(verbose > 0) PLOG_MSG("int check ignored as no " << slv_status_name << " found in " << all_objs->get_hier_path() << endl);
                }

            }
        }
    }

}


cpp_int show_interrupt_tree(pen_csr_base * base, bool enabled, bool print_only, int level , string slv_status_name,unsigned indent) {
    static vector<pen_csr_base *> int_objs;
    static pen_csr_base * prev_root = 0;
    cpp_int intr_val;
    if(base->get_csr_type() == pen_csr_base::CSR_TYPE_BLOCK) {
        if(level == 0) return 0;
    }

    if(base->get_csr_type() == pen_csr_base::CSR_TYPE_BLOCK) {
        int_objs = base->get_children_prefix("int", level);
        prev_root = base;
    }
    pen_csr_base * int_root_ptr;
    if(prev_root) int_root_ptr = prev_root->search_csr_by_name(slv_status_name);
    else int_root_ptr = 0;

    if(!int_root_ptr) {
        PLOG_ERR("could not find int_root: searching for, " << (base->get_hier_path() + "." + slv_status_name) << endl);
    } else {
        auto root_children = int_root_ptr->get_children(1);

        for(auto itr : root_children) {
            string pad_str="----";
            string final_pad_str = "   |";
            for(unsigned i=0; i < indent; i++) { final_pad_str = final_pad_str + "   " ; }
            //PLOG_MSG(final_pad_str  << "name: " << itr->get_hier_path() << endl);
            if(itr->get_name().compare("intreg") == 0) {
                pen_csr_base * int_enable_csr_ptr = 0;
                pen_csr_base * int_test_set_ptr = 0;
                for(auto search_itr : root_children) {
                    if(search_itr->get_name().compare("int_enable_set") == 0) int_enable_csr_ptr = search_itr;
                    if(!int_enable_csr_ptr && search_itr->get_name().compare("int_enable_rw_reg") == 0) int_enable_csr_ptr = search_itr;

                    if(search_itr->get_name().compare("int_test_set") == 0) int_test_set_ptr = search_itr;
                    if(!int_test_set_ptr && search_itr->get_name().compare("int_rw_reg") == 0) int_test_set_ptr =search_itr;

                    if(int_test_set_ptr && int_enable_csr_ptr) break;
                }
                if( (int_test_set_ptr == 0) || (int_enable_csr_ptr == 0)) {
                    PLOG_ERR(hex << "intreg found but one of int_enable_set" << (void *) int_enable_csr_ptr << " OR int_test_set_ptr " << (void *) int_test_set_ptr << " is null " << endl << dec);
                    return 0;
                }

                //int_test_set_ptr->read();
                int_enable_csr_ptr->read();
                itr->read();

                unsigned pos=0;
                for(auto fields : itr->get_fields()) {
                    if(fields.compare("all") ==0) continue;
                    cpp_int field_val;
                    cpp_int field_en;

                    string reg_name = fields.substr(0, fields.find("_interrupt"));

                    if(int_enable_csr_ptr->get_field_val(reg_name + "_enable", field_en) == 0) PLOG_ERR(" no field found! " << reg_name + "_enable" << endl);
                    pen_csr_base * int_ptr=0;
                    for(auto find_itr : int_objs) {
                        if(find_itr->get_name().compare(reg_name)==0) {
                            int_ptr = find_itr;
                            break;
                        }
                    }
                    if(int_ptr) {

                        if(int_ptr->get_parent() != prev_root) {
                            field_val = show_interrupt_tree(int_ptr, (field_en && enabled), print_only, level <0 ? -1 : level-1, reg_name, indent+1);
                        } else {
                            field_val = show_interrupt_tree(int_ptr, (field_en && enabled), print_only, level, reg_name, indent+1);
                        }
                    } else {
                        itr->get_field_val(fields, field_val);
                        //if(field_val == 1) {
                        //PLOG_MSG( final_pad_str << pad_str << fields << ": 0x"<< field_val << endl);
                        //}
                    }


                    cpp_int compare_val = 0;
                    itr->get_field_val(fields, compare_val);

                    if( enabled && (field_en != 0) && ( (compare_val != 0))) { 
                        //if(compare_val != 1) {
                        //    PLOG_ERR("expected int val 1 for " << fields << " path: " << itr->get_hier_path() << endl);
                        //}

                        if(!print_only) {
                            PLOG_ERR(itr->get_hier_path() << ": " << fields << " : " << compare_val  << " EN " << field_en << " hier_enabled " << enabled << endl) ; 
                        }
                        intr_val = intr_val + (1 << pos); 
                    } 
                    
                    if(print_only && compare_val) {
                        if(field_en) {
                            PLOG_MSG(itr->get_hier_path() << ": " << fields << " : " << compare_val  << "(enabled)" << endl);
                        } else {
                            PLOG_MSG(itr->get_hier_path() << ": " << fields << " : " << compare_val  << "(disabled)" << endl);
                        }
                    }

                    pos++;
                }


            }
        }
    }

    return (intr_val != 0);
}

std::vector<std::string> string_tokenize(std::string const &in, char sep) {
    std::string::size_type b = 0;
    std::vector<std::string> result;

    while ((b = in.find_first_not_of(sep, b)) != std::string::npos) {
        auto e = in.find_first_of(sep, b);
        result.push_back(in.substr(b, e-b));
        b = e;
    }
    return result;
}


vector<unsigned> split_num_random(unsigned max, unsigned samples, Knob * rand_knob, unsigned min) {
	vector<unsigned> array;

    unsigned sum = 0;
    unsigned orig_max = max;
    if((min*samples) > max) {
        samples = max/min;
        PLOG_MSG("updating samples to : "<< samples << endl);
    }

    max = max - (samples*min);
    if(max == 0) {
        for(unsigned i=0; i < samples; i++) { array.push_back(min); }
    } else {
        //if(max<= 1) { array.push_back(max); return array; }
        //if(samples > max) { samples = max; }

        for(unsigned i=0; i < samples-1; i++) {
            array.push_back((rand_knob->eval() % max));
        }
        // adjustment
        sum=0;
        for(auto & i : array) { sum += i; }
        if(sum != 0) { 
            for(auto & i : array) { i = (unsigned) (i*i/sum); } 
        }

        // again count sum
        sum = 0;
        for(auto & i : array) { 
            i += min; // add min here 
            sum += i; 
        }
        array.push_back(orig_max - sum); 	
    }

    PLOG_MSG("pcie_split_memwr: input samples " << samples << " max val : 0x" << hex << orig_max << " data: ");
    for(auto & i : array) { 
        PLOG_MSG(" 0x" << i);
    }
    PLOG_MSG(endl << dec);

    return array;
}

