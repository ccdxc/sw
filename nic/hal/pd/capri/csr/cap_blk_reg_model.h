#ifndef _CAP_BLK_REG_MODEL_H_
#define _CAP_BLK_REG_MODEL_H_

#include "LogMsg.h" 
#include "cap_csr_base.h"
#include <iostream>
#include <vector>
#include <memory>
#include "string.h"

using namespace std;

template <class T>
class cap_blk_reg_model{

    private:
        cap_blk_reg_model() { };

        typedef map<int, T* > inst_to_class_map_t;
        map<int, inst_to_class_map_t > class_map;

    public:
        static std::shared_ptr<cap_blk_reg_model> access(){
            static std::shared_ptr<cap_blk_reg_model> d(new cap_blk_reg_model);
            return d;
        }
        

        
        T&  Get(int chip_id, string classname, int inst) {
            if(class_map.find(chip_id) != class_map.end()) {
                string str_idx = classname + "[" + to_string(inst) + "]";
                if(class_map[chip_id].find( inst ) == class_map[chip_id].end()) {
                    PLOG_ERR("chip_id : " << chip_id << " has to be registered for " << str_idx  << endl);
                    PU_ASSERT(0);
                    return *class_map[chip_id][inst];
                } else {
                    return *class_map[chip_id][inst];
                }
            }
            PLOG_ERR("chip_id : " << chip_id << " is not registerd yet! for class " << classname << endl)
            PU_ASSERT(0);
            return *class_map[chip_id][inst];
        }

        void Register(int chip_id, string classname, int inst, T * blk) {
            if(class_map.find(chip_id) != class_map.end()) {
                //string str_idx = classname + "[" + to_string(inst) + "]";
                if(class_map[chip_id].find( inst ) != class_map[chip_id].end()) {
                    PLOG_ERR("chip_id : " << chip_id << " has " << inst << " already registered" << endl);
                }
                
                ///class_map.insert(make_pair(chip_id, inst_to_class_map_t()) ); I believe it is a typos which causes trouble for multi instance, comment out. by Changqi 
                class_map[chip_id].insert(make_pair(inst, blk));
                PLOG_MSG("added reg model:" << classname << " with chip_id " << chip_id << ", " << inst << endl)
            } else {
                class_map.insert(make_pair(chip_id, inst_to_class_map_t()) );
                class_map[chip_id].insert(make_pair(inst, blk));
                PLOG_MSG("created reg model:" << classname << " with chip_id " << chip_id << ", " << inst << endl)
            }
        }


        virtual ~cap_blk_reg_model() {
            //TODO: delete? 
        }
};

#define CAP_BLK_REG_MODEL_ACCESS(CLASSNAME, CHIP_ID, INST_ID) cap_blk_reg_model<CLASSNAME>::access()->Get(CHIP_ID, #CLASSNAME, INST_ID)
#define CAP_BLK_REG_MODEL_REGISTER(CLASSNAME, CHIP_ID, INST_ID, BLK) cap_blk_reg_model<CLASSNAME>::access()->Register(CHIP_ID, #CLASSNAME, INST_ID, BLK)

#endif // _CAP_BLK_REG_MODEL_H_
