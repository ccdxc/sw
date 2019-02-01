#ifndef _CAP_BLK_REG_MODEL_H_
#define _CAP_BLK_REG_MODEL_H_

#include "LogMsg.h" 
#include "cap_csr_base.h"
#include "pen_blk_reg_model.h"

using namespace std;

template<class T>
using cap_blk_reg_model = pen_blk_reg_model<T>;


#define CAP_BLK_REG_MODEL_ACCESS(CLASSNAME, CHIP_ID, INST_ID) pen_blk_reg_model<pen_csr_base>::access()->Get<CLASSNAME>(CHIP_ID, #CLASSNAME, INST_ID)
#define CAP_BLK_REG_MODEL_REGISTER(CLASSNAME, CHIP_ID, INST_ID, BLK) pen_blk_reg_model<pen_csr_base>::access()->Register(CHIP_ID, #CLASSNAME, INST_ID, BLK)
#define CAP_BLK_REG_MODEL_ACCESS_STR(CLASSNAME_STR, CHIP_ID, INST_ID) pen_blk_reg_model<pen_csr_base>::access()->Get<pen_csr_base>(CHIP_ID, CLASSNAME_STR, INST_ID)
#define CAP_BLK_REG_MODEL_GET_ALL_CSRS(CHIP_ID) pen_blk_reg_model<pen_csr_base>::access()->GetAllCSR(CHIP_ID)

#endif // _CAP_BLK_REG_MODEL_H_
