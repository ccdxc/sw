#ifndef _PEN_BLK_REG_MODEL_H_
#define _PEN_BLK_REG_MODEL_H_

#include "LogMsg.h" 
#include "pen_csr_base.h"
#include <iostream>
#include <vector>
#include <memory>

using namespace std;

template <class T>
class pen_blk_reg_model{

    private:
        pen_blk_reg_model() { };

        typedef map<int, T* > inst_to_class_map_t;
        typedef map<int, inst_to_class_map_t> chip_to_inst_to_class_map_t;
        map< string, chip_to_inst_to_class_map_t  > class_map_db;

    public:
        static std::shared_ptr<pen_blk_reg_model> access(){
            static std::shared_ptr<pen_blk_reg_model> d(new pen_blk_reg_model);
            return d;
        }
        

        
        template <class A>
        A&  Get(int chip_id, string classname, int inst) {
            A * a_ptr = nullptr;

            if(class_map_db.find(classname) != class_map_db.end()) {
                auto & class_map = class_map_db[classname];
                if(class_map.find(chip_id) != class_map.end()) {
                    string str_idx = classname + "[" + to_string(inst) + "]";
                    if(class_map[chip_id].find( inst ) == class_map[chip_id].end()) {
                        PLOG_ERR("chip_id : " << chip_id << " has to be registered for " << str_idx  << endl);
                        PU_ASSERT(0);
                        return *a_ptr;
                    } else {
                        A * a_ptr = dynamic_cast<A *> (class_map[chip_id][inst]);
                        if(a_ptr == nullptr) {
                            PLOG_ERR("can not cast from pen_csr_base to " << classname << endl);
                            PU_ASSERT(0);
                        }
                        return *a_ptr;
                    }
                }
            }
            PLOG_ERR("chip_id : " << chip_id << " inst " << inst << " is not registerd yet! for class " << classname << endl);
            PU_ASSERT(0);

            return *a_ptr;
        }

        void Register(int chip_id, string classname, int inst, T * blk) {


            if(class_map_db.find(classname) != class_map_db.end()) {
                auto & class_map = class_map_db[classname];

                if(class_map.find(chip_id) != class_map.end()) {
                    //string str_idx = classname + "[" + to_string(inst) + "]";
                    if(class_map[chip_id].find( inst ) != class_map[chip_id].end()) {
                        PLOG_ERR("chip_id : " << chip_id << " has " << inst << " already registered" << endl);
                    }

                    ///class_map.insert(make_pair(chip_id, inst_to_class_map_t()) ); I believe it is a typos which causes trouble for multi instance, comment out. by Changqi 
                    class_map[chip_id].insert(make_pair(inst, blk));
                    PLOG_MSG("added reg model:" << classname << " with chip_id " << chip_id << ", " << inst << endl);
                } else {
                    class_map.insert(make_pair(chip_id, inst_to_class_map_t()) );
                    class_map[chip_id].insert(make_pair(inst, blk));
                    PLOG_MSG("created reg model:" << classname << " with chip_id " << chip_id << ", " << inst << endl);
                }
            } else {
                class_map_db[classname][chip_id][inst] = blk;
                PLOG_MSG("first time created reg model:" << classname << " with chip_id " << chip_id << ", " << inst << endl);
            }



        }

        vector<T*> GetAllCSR(int chip_id) {
            vector<T *> all_csrs;

            for(auto t_itr : class_map_db) {
                for(auto chip_itr : t_itr.second) {
                    if(chip_itr.first != chip_id) continue;

                    for(auto inst_itr : chip_itr.second) {
                        all_csrs.push_back(inst_itr.second);
                    }
                }
            }
            return all_csrs;


        }


        virtual ~pen_blk_reg_model() {
            //TODO: delete? 
        }
};

#define PEN_BLK_REG_MODEL_ACCESS(CLASSNAME, CHIP_ID, INST_ID) pen_blk_reg_model<pen_csr_base>::access()->Get<CLASSNAME>(CHIP_ID, #CLASSNAME, INST_ID)
#define PEN_BLK_REG_MODEL_REGISTER(CLASSNAME, CHIP_ID, INST_ID, BLK) pen_blk_reg_model<pen_csr_base>::access()->Register(CHIP_ID, #CLASSNAME, INST_ID, BLK)
#define PEN_BLK_REG_MODEL_ACCESS_STR(CLASSNAME_STR, CHIP_ID, INST_ID) pen_blk_reg_model<pen_csr_base>::access()->Get<pen_csr_base>(CHIP_ID, CLASSNAME_STR, INST_ID)
#define PEN_BLK_REG_MODEL_GET_ALL_CSRS(CHIP_ID) pen_blk_reg_model<pen_csr_base>::access()->GetAllCSR(CHIP_ID)

#endif // _PEN_BLK_REG_MODEL_H_
