#include "sknobs.h"
#include "cap_pb_api.h"
#include "cap_pb_port_def.gh"
#include "cap_pbc_decoders.h"
#include "msg_man.h"
#include <bitset>
#include <queue>
// THIS IS A GENERATED FILE. DO NOT EDIT /////

// //typedef enum {SoftReset, InitStart, InitDone, Config, EOSInt, EOSCnt, EOS} api_step_e;
// void cap_pb_api::setup() {
//    DPIRegistry::access()->add_fn(SoftReset, std::bind(&cap_pb_api::softreset, this));
//    DPIRegistry::access()->add_fn(InitStart, std::bind(&cap_pb_api::init, this));
//    DPIRegistry::access()->add_fn(EOS, std::bind(&cap_pb_api::eos, this));
// }

void cap_pb_soft_reset(int chip_id, int inst_id) {

PLOG_MSG ("Dumping Sknob vars" << std::endl);
//sknobs_dump();

PLOG_MSG("inside softreset\n");
 //cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
 //cap_pbm_csr_t & pbm_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbm_csr_t, chip_id, inst_id);

}

void cap_pb_set_soft_reset(int chip_id, int inst_id, int value) { }


void cap_pb_init_start(int chip_id, int inst_id) {

PLOG_MSG("inside cap_pb_init_start \n");
 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
 //cap_pbm_csr_t & pbm_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbm_csr_t, chip_id, inst_id);

 unsigned int pb_skip_init = SKNOBS_GET("tb/pb_skip_init", 0);

 unsigned int pb_gatesim = SKNOBS_GET("tb/pb_gatesim", 0);
 if (pb_gatesim > 0) {
    PLOG_MSG("setting front door for pb regs \n");
    for(auto itr : pbc_csr.get_children()) {
        itr->set_access_no_zero_time(1);
    }
 }

 // turn off zero time for these
       for (int ii=0; ii<pbc_csr.port_10.dhs_oq_flow_control.get_depth_entry(); ii++) {
          pbc_csr.port_10.dhs_oq_flow_control.entry[ii].set_access_no_zero_time(1);
       }
       for (int ii=0; ii<pbc_csr.port_11.dhs_oq_flow_control.get_depth_entry(); ii++) {
          pbc_csr.port_11.dhs_oq_flow_control.entry[ii].set_access_no_zero_time(1);
       }
  pbc_csr.hbm.dhs_hbm_mem.entry.set_access_no_zero_time(1);

 if (pb_skip_init == 0) {


  cpu_access_type_e cur_cpu_access_type = cpu::access()->get_access_type();
  cpu::access()->set_access_type(front_door_e);
   pbc_csr.hbm.cfg_hbm_eth_ctrl_init.head_start(1);
   pbc_csr.hbm.cfg_hbm_eth_ctrl_init.tail_start(1);
   pbc_csr.hbm.cfg_hbm_eth_ctrl_init.write();
   //if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.cfg_hbm_eth_ctrl_init.show();}
   pbc_csr.hbm.cfg_hbm_tx_ctrl_init.head_start(1);
   pbc_csr.hbm.cfg_hbm_tx_ctrl_init.tail_start(1);
   pbc_csr.hbm.cfg_hbm_tx_ctrl_init.write();
   //if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.cfg_hbm_tx_ctrl_init.show();}
   pbc_csr.hbm.cfg_hbm_eth_ctrl_init.head_start(0);
   pbc_csr.hbm.cfg_hbm_eth_ctrl_init.tail_start(0);
   pbc_csr.hbm.cfg_hbm_eth_ctrl_init.write();
   //if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.cfg_hbm_eth_ctrl_init.show();}
   pbc_csr.hbm.cfg_hbm_tx_ctrl_init.head_start(0);
   pbc_csr.hbm.cfg_hbm_tx_ctrl_init.tail_start(0);
   pbc_csr.hbm.cfg_hbm_tx_ctrl_init.write();
   //if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.cfg_hbm_tx_ctrl_init.show();}


  // do a frontdoor read to flush all frontdoor writes
  // to avoid a race condition when we switch to a different
  // method
  pbc_csr.hbm.cfg_hbm_tx_ctrl_init.read();

  cpu::access()->set_access_type(cur_cpu_access_type);

   //give 400 cells to each input queue in each port
    pbc_csr.port_0.cfg_account_pg_0.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_0.cfg_account_pg_0.load_from_cfg(0); 
    if (pbc_csr.port_0.cfg_account_pg_0.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_0.cfg_account_pg_0.xon_threshold(0);
    } else if (pbc_csr.port_0.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_0.cfg_account_pg_0.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_0.cfg_account_pg_0.xon_threshold((pbc_csr.port_0.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_0.cfg_account_pg_0.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_0.cfg_account_pg_0.show();}
    //
    pbc_csr.port_0.cfg_account_pg_1.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_0.cfg_account_pg_1.load_from_cfg(0); 
    if (pbc_csr.port_0.cfg_account_pg_1.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_0.cfg_account_pg_1.xon_threshold(0);
    } else if (pbc_csr.port_0.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_0.cfg_account_pg_1.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_0.cfg_account_pg_1.xon_threshold((pbc_csr.port_0.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_0.cfg_account_pg_1.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_0.cfg_account_pg_1.show();}
    //
    pbc_csr.port_0.cfg_account_pg_2.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_0.cfg_account_pg_2.load_from_cfg(0); 
    if (pbc_csr.port_0.cfg_account_pg_2.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_0.cfg_account_pg_2.xon_threshold(0);
    } else if (pbc_csr.port_0.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_0.cfg_account_pg_2.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_0.cfg_account_pg_2.xon_threshold((pbc_csr.port_0.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_0.cfg_account_pg_2.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_0.cfg_account_pg_2.show();}
    //
    pbc_csr.port_0.cfg_account_pg_3.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_0.cfg_account_pg_3.load_from_cfg(0); 
    if (pbc_csr.port_0.cfg_account_pg_3.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_0.cfg_account_pg_3.xon_threshold(0);
    } else if (pbc_csr.port_0.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_0.cfg_account_pg_3.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_0.cfg_account_pg_3.xon_threshold((pbc_csr.port_0.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_0.cfg_account_pg_3.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_0.cfg_account_pg_3.show();}
    //
    pbc_csr.port_0.cfg_account_pg_4.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_0.cfg_account_pg_4.load_from_cfg(0); 
    if (pbc_csr.port_0.cfg_account_pg_4.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_0.cfg_account_pg_4.xon_threshold(0);
    } else if (pbc_csr.port_0.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_0.cfg_account_pg_4.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_0.cfg_account_pg_4.xon_threshold((pbc_csr.port_0.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_0.cfg_account_pg_4.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_0.cfg_account_pg_4.show();}
    //
    pbc_csr.port_0.cfg_account_pg_5.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_0.cfg_account_pg_5.load_from_cfg(0); 
    if (pbc_csr.port_0.cfg_account_pg_5.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_0.cfg_account_pg_5.xon_threshold(0);
    } else if (pbc_csr.port_0.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_0.cfg_account_pg_5.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_0.cfg_account_pg_5.xon_threshold((pbc_csr.port_0.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_0.cfg_account_pg_5.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_0.cfg_account_pg_5.show();}
    //
    pbc_csr.port_0.cfg_account_pg_6.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_0.cfg_account_pg_6.load_from_cfg(0); 
    if (pbc_csr.port_0.cfg_account_pg_6.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_0.cfg_account_pg_6.xon_threshold(0);
    } else if (pbc_csr.port_0.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_0.cfg_account_pg_6.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_0.cfg_account_pg_6.xon_threshold((pbc_csr.port_0.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_0.cfg_account_pg_6.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_0.cfg_account_pg_6.show();}
    //
    pbc_csr.port_0.cfg_account_pg_7.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_0.cfg_account_pg_7.load_from_cfg(0); 
    if (pbc_csr.port_0.cfg_account_pg_7.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_0.cfg_account_pg_7.xon_threshold(0);
    } else if (pbc_csr.port_0.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_0.cfg_account_pg_7.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_0.cfg_account_pg_7.xon_threshold((pbc_csr.port_0.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_0.cfg_account_pg_7.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_0.cfg_account_pg_7.show();}
    //
    pbc_csr.port_1.cfg_account_pg_0.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_1.cfg_account_pg_0.load_from_cfg(0); 
    if (pbc_csr.port_1.cfg_account_pg_0.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_1.cfg_account_pg_0.xon_threshold(0);
    } else if (pbc_csr.port_1.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_1.cfg_account_pg_0.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_1.cfg_account_pg_0.xon_threshold((pbc_csr.port_1.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_1.cfg_account_pg_0.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_1.cfg_account_pg_0.show();}
    //
    pbc_csr.port_1.cfg_account_pg_1.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_1.cfg_account_pg_1.load_from_cfg(0); 
    if (pbc_csr.port_1.cfg_account_pg_1.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_1.cfg_account_pg_1.xon_threshold(0);
    } else if (pbc_csr.port_1.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_1.cfg_account_pg_1.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_1.cfg_account_pg_1.xon_threshold((pbc_csr.port_1.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_1.cfg_account_pg_1.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_1.cfg_account_pg_1.show();}
    //
    pbc_csr.port_1.cfg_account_pg_2.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_1.cfg_account_pg_2.load_from_cfg(0); 
    if (pbc_csr.port_1.cfg_account_pg_2.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_1.cfg_account_pg_2.xon_threshold(0);
    } else if (pbc_csr.port_1.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_1.cfg_account_pg_2.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_1.cfg_account_pg_2.xon_threshold((pbc_csr.port_1.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_1.cfg_account_pg_2.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_1.cfg_account_pg_2.show();}
    //
    pbc_csr.port_1.cfg_account_pg_3.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_1.cfg_account_pg_3.load_from_cfg(0); 
    if (pbc_csr.port_1.cfg_account_pg_3.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_1.cfg_account_pg_3.xon_threshold(0);
    } else if (pbc_csr.port_1.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_1.cfg_account_pg_3.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_1.cfg_account_pg_3.xon_threshold((pbc_csr.port_1.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_1.cfg_account_pg_3.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_1.cfg_account_pg_3.show();}
    //
    pbc_csr.port_1.cfg_account_pg_4.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_1.cfg_account_pg_4.load_from_cfg(0); 
    if (pbc_csr.port_1.cfg_account_pg_4.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_1.cfg_account_pg_4.xon_threshold(0);
    } else if (pbc_csr.port_1.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_1.cfg_account_pg_4.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_1.cfg_account_pg_4.xon_threshold((pbc_csr.port_1.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_1.cfg_account_pg_4.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_1.cfg_account_pg_4.show();}
    //
    pbc_csr.port_1.cfg_account_pg_5.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_1.cfg_account_pg_5.load_from_cfg(0); 
    if (pbc_csr.port_1.cfg_account_pg_5.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_1.cfg_account_pg_5.xon_threshold(0);
    } else if (pbc_csr.port_1.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_1.cfg_account_pg_5.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_1.cfg_account_pg_5.xon_threshold((pbc_csr.port_1.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_1.cfg_account_pg_5.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_1.cfg_account_pg_5.show();}
    //
    pbc_csr.port_1.cfg_account_pg_6.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_1.cfg_account_pg_6.load_from_cfg(0); 
    if (pbc_csr.port_1.cfg_account_pg_6.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_1.cfg_account_pg_6.xon_threshold(0);
    } else if (pbc_csr.port_1.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_1.cfg_account_pg_6.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_1.cfg_account_pg_6.xon_threshold((pbc_csr.port_1.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_1.cfg_account_pg_6.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_1.cfg_account_pg_6.show();}
    //
    pbc_csr.port_1.cfg_account_pg_7.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_1.cfg_account_pg_7.load_from_cfg(0); 
    if (pbc_csr.port_1.cfg_account_pg_7.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_1.cfg_account_pg_7.xon_threshold(0);
    } else if (pbc_csr.port_1.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_1.cfg_account_pg_7.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_1.cfg_account_pg_7.xon_threshold((pbc_csr.port_1.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_1.cfg_account_pg_7.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_1.cfg_account_pg_7.show();}
    //
    pbc_csr.port_2.cfg_account_pg_0.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_2.cfg_account_pg_0.load_from_cfg(0); 
    if (pbc_csr.port_2.cfg_account_pg_0.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_2.cfg_account_pg_0.xon_threshold(0);
    } else if (pbc_csr.port_2.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_2.cfg_account_pg_0.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_2.cfg_account_pg_0.xon_threshold((pbc_csr.port_2.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_2.cfg_account_pg_0.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_2.cfg_account_pg_0.show();}
    //
    pbc_csr.port_2.cfg_account_pg_1.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_2.cfg_account_pg_1.load_from_cfg(0); 
    if (pbc_csr.port_2.cfg_account_pg_1.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_2.cfg_account_pg_1.xon_threshold(0);
    } else if (pbc_csr.port_2.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_2.cfg_account_pg_1.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_2.cfg_account_pg_1.xon_threshold((pbc_csr.port_2.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_2.cfg_account_pg_1.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_2.cfg_account_pg_1.show();}
    //
    pbc_csr.port_2.cfg_account_pg_2.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_2.cfg_account_pg_2.load_from_cfg(0); 
    if (pbc_csr.port_2.cfg_account_pg_2.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_2.cfg_account_pg_2.xon_threshold(0);
    } else if (pbc_csr.port_2.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_2.cfg_account_pg_2.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_2.cfg_account_pg_2.xon_threshold((pbc_csr.port_2.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_2.cfg_account_pg_2.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_2.cfg_account_pg_2.show();}
    //
    pbc_csr.port_2.cfg_account_pg_3.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_2.cfg_account_pg_3.load_from_cfg(0); 
    if (pbc_csr.port_2.cfg_account_pg_3.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_2.cfg_account_pg_3.xon_threshold(0);
    } else if (pbc_csr.port_2.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_2.cfg_account_pg_3.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_2.cfg_account_pg_3.xon_threshold((pbc_csr.port_2.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_2.cfg_account_pg_3.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_2.cfg_account_pg_3.show();}
    //
    pbc_csr.port_2.cfg_account_pg_4.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_2.cfg_account_pg_4.load_from_cfg(0); 
    if (pbc_csr.port_2.cfg_account_pg_4.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_2.cfg_account_pg_4.xon_threshold(0);
    } else if (pbc_csr.port_2.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_2.cfg_account_pg_4.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_2.cfg_account_pg_4.xon_threshold((pbc_csr.port_2.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_2.cfg_account_pg_4.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_2.cfg_account_pg_4.show();}
    //
    pbc_csr.port_2.cfg_account_pg_5.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_2.cfg_account_pg_5.load_from_cfg(0); 
    if (pbc_csr.port_2.cfg_account_pg_5.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_2.cfg_account_pg_5.xon_threshold(0);
    } else if (pbc_csr.port_2.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_2.cfg_account_pg_5.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_2.cfg_account_pg_5.xon_threshold((pbc_csr.port_2.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_2.cfg_account_pg_5.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_2.cfg_account_pg_5.show();}
    //
    pbc_csr.port_2.cfg_account_pg_6.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_2.cfg_account_pg_6.load_from_cfg(0); 
    if (pbc_csr.port_2.cfg_account_pg_6.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_2.cfg_account_pg_6.xon_threshold(0);
    } else if (pbc_csr.port_2.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_2.cfg_account_pg_6.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_2.cfg_account_pg_6.xon_threshold((pbc_csr.port_2.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_2.cfg_account_pg_6.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_2.cfg_account_pg_6.show();}
    //
    pbc_csr.port_2.cfg_account_pg_7.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_2.cfg_account_pg_7.load_from_cfg(0); 
    if (pbc_csr.port_2.cfg_account_pg_7.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_2.cfg_account_pg_7.xon_threshold(0);
    } else if (pbc_csr.port_2.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_2.cfg_account_pg_7.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_2.cfg_account_pg_7.xon_threshold((pbc_csr.port_2.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_2.cfg_account_pg_7.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_2.cfg_account_pg_7.show();}
    //
    pbc_csr.port_3.cfg_account_pg_0.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_3.cfg_account_pg_0.load_from_cfg(0); 
    if (pbc_csr.port_3.cfg_account_pg_0.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_3.cfg_account_pg_0.xon_threshold(0);
    } else if (pbc_csr.port_3.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_3.cfg_account_pg_0.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_3.cfg_account_pg_0.xon_threshold((pbc_csr.port_3.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_3.cfg_account_pg_0.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_3.cfg_account_pg_0.show();}
    //
    pbc_csr.port_3.cfg_account_pg_1.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_3.cfg_account_pg_1.load_from_cfg(0); 
    if (pbc_csr.port_3.cfg_account_pg_1.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_3.cfg_account_pg_1.xon_threshold(0);
    } else if (pbc_csr.port_3.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_3.cfg_account_pg_1.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_3.cfg_account_pg_1.xon_threshold((pbc_csr.port_3.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_3.cfg_account_pg_1.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_3.cfg_account_pg_1.show();}
    //
    pbc_csr.port_3.cfg_account_pg_2.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_3.cfg_account_pg_2.load_from_cfg(0); 
    if (pbc_csr.port_3.cfg_account_pg_2.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_3.cfg_account_pg_2.xon_threshold(0);
    } else if (pbc_csr.port_3.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_3.cfg_account_pg_2.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_3.cfg_account_pg_2.xon_threshold((pbc_csr.port_3.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_3.cfg_account_pg_2.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_3.cfg_account_pg_2.show();}
    //
    pbc_csr.port_3.cfg_account_pg_3.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_3.cfg_account_pg_3.load_from_cfg(0); 
    if (pbc_csr.port_3.cfg_account_pg_3.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_3.cfg_account_pg_3.xon_threshold(0);
    } else if (pbc_csr.port_3.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_3.cfg_account_pg_3.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_3.cfg_account_pg_3.xon_threshold((pbc_csr.port_3.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_3.cfg_account_pg_3.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_3.cfg_account_pg_3.show();}
    //
    pbc_csr.port_3.cfg_account_pg_4.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_3.cfg_account_pg_4.load_from_cfg(0); 
    if (pbc_csr.port_3.cfg_account_pg_4.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_3.cfg_account_pg_4.xon_threshold(0);
    } else if (pbc_csr.port_3.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_3.cfg_account_pg_4.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_3.cfg_account_pg_4.xon_threshold((pbc_csr.port_3.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_3.cfg_account_pg_4.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_3.cfg_account_pg_4.show();}
    //
    pbc_csr.port_3.cfg_account_pg_5.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_3.cfg_account_pg_5.load_from_cfg(0); 
    if (pbc_csr.port_3.cfg_account_pg_5.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_3.cfg_account_pg_5.xon_threshold(0);
    } else if (pbc_csr.port_3.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_3.cfg_account_pg_5.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_3.cfg_account_pg_5.xon_threshold((pbc_csr.port_3.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_3.cfg_account_pg_5.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_3.cfg_account_pg_5.show();}
    //
    pbc_csr.port_3.cfg_account_pg_6.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_3.cfg_account_pg_6.load_from_cfg(0); 
    if (pbc_csr.port_3.cfg_account_pg_6.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_3.cfg_account_pg_6.xon_threshold(0);
    } else if (pbc_csr.port_3.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_3.cfg_account_pg_6.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_3.cfg_account_pg_6.xon_threshold((pbc_csr.port_3.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_3.cfg_account_pg_6.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_3.cfg_account_pg_6.show();}
    //
    pbc_csr.port_3.cfg_account_pg_7.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_3.cfg_account_pg_7.load_from_cfg(0); 
    if (pbc_csr.port_3.cfg_account_pg_7.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_3.cfg_account_pg_7.xon_threshold(0);
    } else if (pbc_csr.port_3.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_3.cfg_account_pg_7.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_3.cfg_account_pg_7.xon_threshold((pbc_csr.port_3.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_3.cfg_account_pg_7.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_3.cfg_account_pg_7.show();}
    //
    pbc_csr.port_4.cfg_account_pg_0.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_4.cfg_account_pg_0.load_from_cfg(0); 
    if (pbc_csr.port_4.cfg_account_pg_0.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_4.cfg_account_pg_0.xon_threshold(0);
    } else if (pbc_csr.port_4.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_4.cfg_account_pg_0.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_4.cfg_account_pg_0.xon_threshold((pbc_csr.port_4.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_4.cfg_account_pg_0.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_4.cfg_account_pg_0.show();}
    //
    pbc_csr.port_4.cfg_account_pg_1.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_4.cfg_account_pg_1.load_from_cfg(0); 
    if (pbc_csr.port_4.cfg_account_pg_1.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_4.cfg_account_pg_1.xon_threshold(0);
    } else if (pbc_csr.port_4.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_4.cfg_account_pg_1.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_4.cfg_account_pg_1.xon_threshold((pbc_csr.port_4.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_4.cfg_account_pg_1.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_4.cfg_account_pg_1.show();}
    //
    pbc_csr.port_4.cfg_account_pg_2.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_4.cfg_account_pg_2.load_from_cfg(0); 
    if (pbc_csr.port_4.cfg_account_pg_2.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_4.cfg_account_pg_2.xon_threshold(0);
    } else if (pbc_csr.port_4.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_4.cfg_account_pg_2.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_4.cfg_account_pg_2.xon_threshold((pbc_csr.port_4.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_4.cfg_account_pg_2.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_4.cfg_account_pg_2.show();}
    //
    pbc_csr.port_4.cfg_account_pg_3.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_4.cfg_account_pg_3.load_from_cfg(0); 
    if (pbc_csr.port_4.cfg_account_pg_3.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_4.cfg_account_pg_3.xon_threshold(0);
    } else if (pbc_csr.port_4.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_4.cfg_account_pg_3.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_4.cfg_account_pg_3.xon_threshold((pbc_csr.port_4.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_4.cfg_account_pg_3.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_4.cfg_account_pg_3.show();}
    //
    pbc_csr.port_4.cfg_account_pg_4.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_4.cfg_account_pg_4.load_from_cfg(0); 
    if (pbc_csr.port_4.cfg_account_pg_4.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_4.cfg_account_pg_4.xon_threshold(0);
    } else if (pbc_csr.port_4.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_4.cfg_account_pg_4.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_4.cfg_account_pg_4.xon_threshold((pbc_csr.port_4.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_4.cfg_account_pg_4.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_4.cfg_account_pg_4.show();}
    //
    pbc_csr.port_4.cfg_account_pg_5.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_4.cfg_account_pg_5.load_from_cfg(0); 
    if (pbc_csr.port_4.cfg_account_pg_5.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_4.cfg_account_pg_5.xon_threshold(0);
    } else if (pbc_csr.port_4.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_4.cfg_account_pg_5.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_4.cfg_account_pg_5.xon_threshold((pbc_csr.port_4.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_4.cfg_account_pg_5.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_4.cfg_account_pg_5.show();}
    //
    pbc_csr.port_4.cfg_account_pg_6.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_4.cfg_account_pg_6.load_from_cfg(0); 
    if (pbc_csr.port_4.cfg_account_pg_6.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_4.cfg_account_pg_6.xon_threshold(0);
    } else if (pbc_csr.port_4.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_4.cfg_account_pg_6.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_4.cfg_account_pg_6.xon_threshold((pbc_csr.port_4.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_4.cfg_account_pg_6.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_4.cfg_account_pg_6.show();}
    //
    pbc_csr.port_4.cfg_account_pg_7.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_4.cfg_account_pg_7.load_from_cfg(0); 
    if (pbc_csr.port_4.cfg_account_pg_7.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_4.cfg_account_pg_7.xon_threshold(0);
    } else if (pbc_csr.port_4.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_4.cfg_account_pg_7.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_4.cfg_account_pg_7.xon_threshold((pbc_csr.port_4.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_4.cfg_account_pg_7.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_4.cfg_account_pg_7.show();}
    //
    pbc_csr.port_5.cfg_account_pg_0.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_5.cfg_account_pg_0.load_from_cfg(0); 
    if (pbc_csr.port_5.cfg_account_pg_0.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_5.cfg_account_pg_0.xon_threshold(0);
    } else if (pbc_csr.port_5.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_5.cfg_account_pg_0.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_5.cfg_account_pg_0.xon_threshold((pbc_csr.port_5.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_5.cfg_account_pg_0.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_5.cfg_account_pg_0.show();}
    //
    pbc_csr.port_5.cfg_account_pg_1.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_5.cfg_account_pg_1.load_from_cfg(0); 
    if (pbc_csr.port_5.cfg_account_pg_1.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_5.cfg_account_pg_1.xon_threshold(0);
    } else if (pbc_csr.port_5.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_5.cfg_account_pg_1.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_5.cfg_account_pg_1.xon_threshold((pbc_csr.port_5.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_5.cfg_account_pg_1.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_5.cfg_account_pg_1.show();}
    //
    pbc_csr.port_5.cfg_account_pg_2.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_5.cfg_account_pg_2.load_from_cfg(0); 
    if (pbc_csr.port_5.cfg_account_pg_2.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_5.cfg_account_pg_2.xon_threshold(0);
    } else if (pbc_csr.port_5.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_5.cfg_account_pg_2.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_5.cfg_account_pg_2.xon_threshold((pbc_csr.port_5.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_5.cfg_account_pg_2.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_5.cfg_account_pg_2.show();}
    //
    pbc_csr.port_5.cfg_account_pg_3.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_5.cfg_account_pg_3.load_from_cfg(0); 
    if (pbc_csr.port_5.cfg_account_pg_3.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_5.cfg_account_pg_3.xon_threshold(0);
    } else if (pbc_csr.port_5.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_5.cfg_account_pg_3.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_5.cfg_account_pg_3.xon_threshold((pbc_csr.port_5.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_5.cfg_account_pg_3.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_5.cfg_account_pg_3.show();}
    //
    pbc_csr.port_5.cfg_account_pg_4.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_5.cfg_account_pg_4.load_from_cfg(0); 
    if (pbc_csr.port_5.cfg_account_pg_4.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_5.cfg_account_pg_4.xon_threshold(0);
    } else if (pbc_csr.port_5.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_5.cfg_account_pg_4.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_5.cfg_account_pg_4.xon_threshold((pbc_csr.port_5.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_5.cfg_account_pg_4.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_5.cfg_account_pg_4.show();}
    //
    pbc_csr.port_5.cfg_account_pg_5.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_5.cfg_account_pg_5.load_from_cfg(0); 
    if (pbc_csr.port_5.cfg_account_pg_5.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_5.cfg_account_pg_5.xon_threshold(0);
    } else if (pbc_csr.port_5.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_5.cfg_account_pg_5.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_5.cfg_account_pg_5.xon_threshold((pbc_csr.port_5.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_5.cfg_account_pg_5.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_5.cfg_account_pg_5.show();}
    //
    pbc_csr.port_5.cfg_account_pg_6.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_5.cfg_account_pg_6.load_from_cfg(0); 
    if (pbc_csr.port_5.cfg_account_pg_6.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_5.cfg_account_pg_6.xon_threshold(0);
    } else if (pbc_csr.port_5.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_5.cfg_account_pg_6.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_5.cfg_account_pg_6.xon_threshold((pbc_csr.port_5.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_5.cfg_account_pg_6.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_5.cfg_account_pg_6.show();}
    //
    pbc_csr.port_5.cfg_account_pg_7.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_5.cfg_account_pg_7.load_from_cfg(0); 
    if (pbc_csr.port_5.cfg_account_pg_7.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_5.cfg_account_pg_7.xon_threshold(0);
    } else if (pbc_csr.port_5.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_5.cfg_account_pg_7.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_5.cfg_account_pg_7.xon_threshold((pbc_csr.port_5.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_5.cfg_account_pg_7.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_5.cfg_account_pg_7.show();}
    //
    pbc_csr.port_6.cfg_account_pg_0.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_6.cfg_account_pg_0.load_from_cfg(0); 
    if (pbc_csr.port_6.cfg_account_pg_0.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_6.cfg_account_pg_0.xon_threshold(0);
    } else if (pbc_csr.port_6.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_6.cfg_account_pg_0.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_6.cfg_account_pg_0.xon_threshold((pbc_csr.port_6.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_6.cfg_account_pg_0.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_6.cfg_account_pg_0.show();}
    //
    pbc_csr.port_6.cfg_account_pg_1.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_6.cfg_account_pg_1.load_from_cfg(0); 
    if (pbc_csr.port_6.cfg_account_pg_1.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_6.cfg_account_pg_1.xon_threshold(0);
    } else if (pbc_csr.port_6.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_6.cfg_account_pg_1.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_6.cfg_account_pg_1.xon_threshold((pbc_csr.port_6.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_6.cfg_account_pg_1.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_6.cfg_account_pg_1.show();}
    //
    pbc_csr.port_6.cfg_account_pg_2.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_6.cfg_account_pg_2.load_from_cfg(0); 
    if (pbc_csr.port_6.cfg_account_pg_2.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_6.cfg_account_pg_2.xon_threshold(0);
    } else if (pbc_csr.port_6.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_6.cfg_account_pg_2.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_6.cfg_account_pg_2.xon_threshold((pbc_csr.port_6.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_6.cfg_account_pg_2.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_6.cfg_account_pg_2.show();}
    //
    pbc_csr.port_6.cfg_account_pg_3.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_6.cfg_account_pg_3.load_from_cfg(0); 
    if (pbc_csr.port_6.cfg_account_pg_3.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_6.cfg_account_pg_3.xon_threshold(0);
    } else if (pbc_csr.port_6.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_6.cfg_account_pg_3.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_6.cfg_account_pg_3.xon_threshold((pbc_csr.port_6.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_6.cfg_account_pg_3.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_6.cfg_account_pg_3.show();}
    //
    pbc_csr.port_6.cfg_account_pg_4.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_6.cfg_account_pg_4.load_from_cfg(0); 
    if (pbc_csr.port_6.cfg_account_pg_4.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_6.cfg_account_pg_4.xon_threshold(0);
    } else if (pbc_csr.port_6.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_6.cfg_account_pg_4.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_6.cfg_account_pg_4.xon_threshold((pbc_csr.port_6.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_6.cfg_account_pg_4.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_6.cfg_account_pg_4.show();}
    //
    pbc_csr.port_6.cfg_account_pg_5.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_6.cfg_account_pg_5.load_from_cfg(0); 
    if (pbc_csr.port_6.cfg_account_pg_5.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_6.cfg_account_pg_5.xon_threshold(0);
    } else if (pbc_csr.port_6.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_6.cfg_account_pg_5.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_6.cfg_account_pg_5.xon_threshold((pbc_csr.port_6.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_6.cfg_account_pg_5.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_6.cfg_account_pg_5.show();}
    //
    pbc_csr.port_6.cfg_account_pg_6.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_6.cfg_account_pg_6.load_from_cfg(0); 
    if (pbc_csr.port_6.cfg_account_pg_6.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_6.cfg_account_pg_6.xon_threshold(0);
    } else if (pbc_csr.port_6.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_6.cfg_account_pg_6.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_6.cfg_account_pg_6.xon_threshold((pbc_csr.port_6.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_6.cfg_account_pg_6.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_6.cfg_account_pg_6.show();}
    //
    pbc_csr.port_6.cfg_account_pg_7.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_6.cfg_account_pg_7.load_from_cfg(0); 
    if (pbc_csr.port_6.cfg_account_pg_7.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_6.cfg_account_pg_7.xon_threshold(0);
    } else if (pbc_csr.port_6.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_6.cfg_account_pg_7.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_6.cfg_account_pg_7.xon_threshold((pbc_csr.port_6.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_6.cfg_account_pg_7.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_6.cfg_account_pg_7.show();}
    //
    pbc_csr.port_7.cfg_account_pg_0.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_7.cfg_account_pg_0.load_from_cfg(0); 
    if (pbc_csr.port_7.cfg_account_pg_0.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_7.cfg_account_pg_0.xon_threshold(0);
    } else if (pbc_csr.port_7.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_7.cfg_account_pg_0.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_7.cfg_account_pg_0.xon_threshold((pbc_csr.port_7.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_7.cfg_account_pg_0.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_7.cfg_account_pg_0.show();}
    //
    pbc_csr.port_7.cfg_account_pg_1.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_7.cfg_account_pg_1.load_from_cfg(0); 
    if (pbc_csr.port_7.cfg_account_pg_1.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_7.cfg_account_pg_1.xon_threshold(0);
    } else if (pbc_csr.port_7.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_7.cfg_account_pg_1.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_7.cfg_account_pg_1.xon_threshold((pbc_csr.port_7.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_7.cfg_account_pg_1.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_7.cfg_account_pg_1.show();}
    //
    pbc_csr.port_7.cfg_account_pg_2.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_7.cfg_account_pg_2.load_from_cfg(0); 
    if (pbc_csr.port_7.cfg_account_pg_2.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_7.cfg_account_pg_2.xon_threshold(0);
    } else if (pbc_csr.port_7.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_7.cfg_account_pg_2.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_7.cfg_account_pg_2.xon_threshold((pbc_csr.port_7.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_7.cfg_account_pg_2.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_7.cfg_account_pg_2.show();}
    //
    pbc_csr.port_7.cfg_account_pg_3.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_7.cfg_account_pg_3.load_from_cfg(0); 
    if (pbc_csr.port_7.cfg_account_pg_3.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_7.cfg_account_pg_3.xon_threshold(0);
    } else if (pbc_csr.port_7.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_7.cfg_account_pg_3.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_7.cfg_account_pg_3.xon_threshold((pbc_csr.port_7.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_7.cfg_account_pg_3.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_7.cfg_account_pg_3.show();}
    //
    pbc_csr.port_7.cfg_account_pg_4.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_7.cfg_account_pg_4.load_from_cfg(0); 
    if (pbc_csr.port_7.cfg_account_pg_4.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_7.cfg_account_pg_4.xon_threshold(0);
    } else if (pbc_csr.port_7.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_7.cfg_account_pg_4.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_7.cfg_account_pg_4.xon_threshold((pbc_csr.port_7.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_7.cfg_account_pg_4.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_7.cfg_account_pg_4.show();}
    //
    pbc_csr.port_7.cfg_account_pg_5.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_7.cfg_account_pg_5.load_from_cfg(0); 
    if (pbc_csr.port_7.cfg_account_pg_5.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_7.cfg_account_pg_5.xon_threshold(0);
    } else if (pbc_csr.port_7.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_7.cfg_account_pg_5.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_7.cfg_account_pg_5.xon_threshold((pbc_csr.port_7.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_7.cfg_account_pg_5.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_7.cfg_account_pg_5.show();}
    //
    pbc_csr.port_7.cfg_account_pg_6.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_7.cfg_account_pg_6.load_from_cfg(0); 
    if (pbc_csr.port_7.cfg_account_pg_6.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_7.cfg_account_pg_6.xon_threshold(0);
    } else if (pbc_csr.port_7.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_7.cfg_account_pg_6.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_7.cfg_account_pg_6.xon_threshold((pbc_csr.port_7.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_7.cfg_account_pg_6.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_7.cfg_account_pg_6.show();}
    //
    pbc_csr.port_7.cfg_account_pg_7.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_7.cfg_account_pg_7.load_from_cfg(0); 
    if (pbc_csr.port_7.cfg_account_pg_7.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_7.cfg_account_pg_7.xon_threshold(0);
    } else if (pbc_csr.port_7.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_7.cfg_account_pg_7.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_7.cfg_account_pg_7.xon_threshold((pbc_csr.port_7.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_7.cfg_account_pg_7.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_7.cfg_account_pg_7.show();}
    //
    pbc_csr.port_8.cfg_account_pg_0.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_8.cfg_account_pg_0.load_from_cfg(0); 
    if (pbc_csr.port_8.cfg_account_pg_0.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_8.cfg_account_pg_0.xon_threshold(0);
    } else if (pbc_csr.port_8.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_8.cfg_account_pg_0.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_8.cfg_account_pg_0.xon_threshold((pbc_csr.port_8.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_8.cfg_account_pg_0.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_8.cfg_account_pg_0.show();}
    //
    pbc_csr.port_8.cfg_account_pg_1.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_8.cfg_account_pg_1.load_from_cfg(0); 
    if (pbc_csr.port_8.cfg_account_pg_1.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_8.cfg_account_pg_1.xon_threshold(0);
    } else if (pbc_csr.port_8.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_8.cfg_account_pg_1.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_8.cfg_account_pg_1.xon_threshold((pbc_csr.port_8.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_8.cfg_account_pg_1.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_8.cfg_account_pg_1.show();}
    //
    pbc_csr.port_8.cfg_account_pg_2.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_8.cfg_account_pg_2.load_from_cfg(0); 
    if (pbc_csr.port_8.cfg_account_pg_2.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_8.cfg_account_pg_2.xon_threshold(0);
    } else if (pbc_csr.port_8.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_8.cfg_account_pg_2.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_8.cfg_account_pg_2.xon_threshold((pbc_csr.port_8.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_8.cfg_account_pg_2.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_8.cfg_account_pg_2.show();}
    //
    pbc_csr.port_8.cfg_account_pg_3.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_8.cfg_account_pg_3.load_from_cfg(0); 
    if (pbc_csr.port_8.cfg_account_pg_3.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_8.cfg_account_pg_3.xon_threshold(0);
    } else if (pbc_csr.port_8.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_8.cfg_account_pg_3.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_8.cfg_account_pg_3.xon_threshold((pbc_csr.port_8.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_8.cfg_account_pg_3.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_8.cfg_account_pg_3.show();}
    //
    pbc_csr.port_8.cfg_account_pg_4.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_8.cfg_account_pg_4.load_from_cfg(0); 
    if (pbc_csr.port_8.cfg_account_pg_4.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_8.cfg_account_pg_4.xon_threshold(0);
    } else if (pbc_csr.port_8.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_8.cfg_account_pg_4.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_8.cfg_account_pg_4.xon_threshold((pbc_csr.port_8.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_8.cfg_account_pg_4.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_8.cfg_account_pg_4.show();}
    //
    pbc_csr.port_8.cfg_account_pg_5.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_8.cfg_account_pg_5.load_from_cfg(0); 
    if (pbc_csr.port_8.cfg_account_pg_5.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_8.cfg_account_pg_5.xon_threshold(0);
    } else if (pbc_csr.port_8.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_8.cfg_account_pg_5.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_8.cfg_account_pg_5.xon_threshold((pbc_csr.port_8.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_8.cfg_account_pg_5.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_8.cfg_account_pg_5.show();}
    //
    pbc_csr.port_8.cfg_account_pg_6.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_8.cfg_account_pg_6.load_from_cfg(0); 
    if (pbc_csr.port_8.cfg_account_pg_6.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_8.cfg_account_pg_6.xon_threshold(0);
    } else if (pbc_csr.port_8.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_8.cfg_account_pg_6.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_8.cfg_account_pg_6.xon_threshold((pbc_csr.port_8.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_8.cfg_account_pg_6.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_8.cfg_account_pg_6.show();}
    //
    pbc_csr.port_8.cfg_account_pg_7.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_8.cfg_account_pg_7.load_from_cfg(0); 
    if (pbc_csr.port_8.cfg_account_pg_7.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_8.cfg_account_pg_7.xon_threshold(0);
    } else if (pbc_csr.port_8.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_8.cfg_account_pg_7.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_8.cfg_account_pg_7.xon_threshold((pbc_csr.port_8.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_8.cfg_account_pg_7.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_8.cfg_account_pg_7.show();}
    //
    pbc_csr.port_9.cfg_account_pg_0.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_9.cfg_account_pg_0.load_from_cfg(0); 
    if (pbc_csr.port_9.cfg_account_pg_0.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_9.cfg_account_pg_0.xon_threshold(0);
    } else if (pbc_csr.port_9.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_9.cfg_account_pg_0.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_9.cfg_account_pg_0.xon_threshold((pbc_csr.port_9.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_9.cfg_account_pg_0.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_0.show();}
    //
    pbc_csr.port_9.cfg_account_pg_1.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_9.cfg_account_pg_1.load_from_cfg(0); 
    if (pbc_csr.port_9.cfg_account_pg_1.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_9.cfg_account_pg_1.xon_threshold(0);
    } else if (pbc_csr.port_9.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_9.cfg_account_pg_1.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_9.cfg_account_pg_1.xon_threshold((pbc_csr.port_9.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_9.cfg_account_pg_1.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_1.show();}
    //
    pbc_csr.port_9.cfg_account_pg_2.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_9.cfg_account_pg_2.load_from_cfg(0); 
    if (pbc_csr.port_9.cfg_account_pg_2.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_9.cfg_account_pg_2.xon_threshold(0);
    } else if (pbc_csr.port_9.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_9.cfg_account_pg_2.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_9.cfg_account_pg_2.xon_threshold((pbc_csr.port_9.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_9.cfg_account_pg_2.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_2.show();}
    //
    pbc_csr.port_9.cfg_account_pg_3.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_9.cfg_account_pg_3.load_from_cfg(0); 
    if (pbc_csr.port_9.cfg_account_pg_3.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_9.cfg_account_pg_3.xon_threshold(0);
    } else if (pbc_csr.port_9.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_9.cfg_account_pg_3.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_9.cfg_account_pg_3.xon_threshold((pbc_csr.port_9.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_9.cfg_account_pg_3.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_3.show();}
    //
    pbc_csr.port_9.cfg_account_pg_4.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_9.cfg_account_pg_4.load_from_cfg(0); 
    if (pbc_csr.port_9.cfg_account_pg_4.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_9.cfg_account_pg_4.xon_threshold(0);
    } else if (pbc_csr.port_9.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_9.cfg_account_pg_4.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_9.cfg_account_pg_4.xon_threshold((pbc_csr.port_9.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_9.cfg_account_pg_4.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_4.show();}
    //
    pbc_csr.port_9.cfg_account_pg_5.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_9.cfg_account_pg_5.load_from_cfg(0); 
    if (pbc_csr.port_9.cfg_account_pg_5.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_9.cfg_account_pg_5.xon_threshold(0);
    } else if (pbc_csr.port_9.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_9.cfg_account_pg_5.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_9.cfg_account_pg_5.xon_threshold((pbc_csr.port_9.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_9.cfg_account_pg_5.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_5.show();}
    //
    pbc_csr.port_9.cfg_account_pg_6.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_9.cfg_account_pg_6.load_from_cfg(0); 
    if (pbc_csr.port_9.cfg_account_pg_6.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_9.cfg_account_pg_6.xon_threshold(0);
    } else if (pbc_csr.port_9.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_9.cfg_account_pg_6.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_9.cfg_account_pg_6.xon_threshold((pbc_csr.port_9.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_9.cfg_account_pg_6.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_6.show();}
    //
    pbc_csr.port_9.cfg_account_pg_7.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_9.cfg_account_pg_7.load_from_cfg(0); 
    if (pbc_csr.port_9.cfg_account_pg_7.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_9.cfg_account_pg_7.xon_threshold(0);
    } else if (pbc_csr.port_9.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_9.cfg_account_pg_7.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_9.cfg_account_pg_7.xon_threshold((pbc_csr.port_9.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_9.cfg_account_pg_7.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_7.show();}
    //
    pbc_csr.port_9.cfg_account_pg_8.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_9.cfg_account_pg_8.load_from_cfg(0); 
    if (pbc_csr.port_9.cfg_account_pg_8.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_9.cfg_account_pg_8.xon_threshold(0);
    } else if (pbc_csr.port_9.cfg_account_pg_8.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_9.cfg_account_pg_8.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_9.cfg_account_pg_8.xon_threshold((pbc_csr.port_9.cfg_account_pg_8.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_9.cfg_account_pg_8.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_8.show();}
    //
    pbc_csr.port_9.cfg_account_pg_9.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_9.cfg_account_pg_9.load_from_cfg(0); 
    if (pbc_csr.port_9.cfg_account_pg_9.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_9.cfg_account_pg_9.xon_threshold(0);
    } else if (pbc_csr.port_9.cfg_account_pg_9.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_9.cfg_account_pg_9.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_9.cfg_account_pg_9.xon_threshold((pbc_csr.port_9.cfg_account_pg_9.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_9.cfg_account_pg_9.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_9.show();}
    //
    pbc_csr.port_9.cfg_account_pg_10.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_9.cfg_account_pg_10.load_from_cfg(0); 
    if (pbc_csr.port_9.cfg_account_pg_10.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_9.cfg_account_pg_10.xon_threshold(0);
    } else if (pbc_csr.port_9.cfg_account_pg_10.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_9.cfg_account_pg_10.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_9.cfg_account_pg_10.xon_threshold((pbc_csr.port_9.cfg_account_pg_10.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_9.cfg_account_pg_10.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_10.show();}
    //
    pbc_csr.port_9.cfg_account_pg_11.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_9.cfg_account_pg_11.load_from_cfg(0); 
    if (pbc_csr.port_9.cfg_account_pg_11.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_9.cfg_account_pg_11.xon_threshold(0);
    } else if (pbc_csr.port_9.cfg_account_pg_11.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_9.cfg_account_pg_11.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_9.cfg_account_pg_11.xon_threshold((pbc_csr.port_9.cfg_account_pg_11.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_9.cfg_account_pg_11.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_11.show();}
    //
    pbc_csr.port_9.cfg_account_pg_12.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_9.cfg_account_pg_12.load_from_cfg(0); 
    if (pbc_csr.port_9.cfg_account_pg_12.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_9.cfg_account_pg_12.xon_threshold(0);
    } else if (pbc_csr.port_9.cfg_account_pg_12.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_9.cfg_account_pg_12.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_9.cfg_account_pg_12.xon_threshold((pbc_csr.port_9.cfg_account_pg_12.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_9.cfg_account_pg_12.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_12.show();}
    //
    pbc_csr.port_9.cfg_account_pg_13.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_9.cfg_account_pg_13.load_from_cfg(0); 
    if (pbc_csr.port_9.cfg_account_pg_13.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_9.cfg_account_pg_13.xon_threshold(0);
    } else if (pbc_csr.port_9.cfg_account_pg_13.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_9.cfg_account_pg_13.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_9.cfg_account_pg_13.xon_threshold((pbc_csr.port_9.cfg_account_pg_13.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_9.cfg_account_pg_13.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_13.show();}
    //
    pbc_csr.port_9.cfg_account_pg_14.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_9.cfg_account_pg_14.load_from_cfg(0); 
    if (pbc_csr.port_9.cfg_account_pg_14.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_9.cfg_account_pg_14.xon_threshold(0);
    } else if (pbc_csr.port_9.cfg_account_pg_14.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_9.cfg_account_pg_14.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_9.cfg_account_pg_14.xon_threshold((pbc_csr.port_9.cfg_account_pg_14.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_9.cfg_account_pg_14.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_14.show();}
    //
    pbc_csr.port_9.cfg_account_pg_15.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_9.cfg_account_pg_15.load_from_cfg(0); 
    if (pbc_csr.port_9.cfg_account_pg_15.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_9.cfg_account_pg_15.xon_threshold(0);
    } else if (pbc_csr.port_9.cfg_account_pg_15.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_9.cfg_account_pg_15.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_9.cfg_account_pg_15.xon_threshold((pbc_csr.port_9.cfg_account_pg_15.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_9.cfg_account_pg_15.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_15.show();}
    //
    pbc_csr.port_10.cfg_account_pg_0.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_0.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_0.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_0.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_0.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_0.xon_threshold((pbc_csr.port_10.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_0.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_0.show();}
    //
    pbc_csr.port_10.cfg_account_pg_1.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_1.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_1.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_1.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_1.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_1.xon_threshold((pbc_csr.port_10.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_1.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_1.show();}
    //
    pbc_csr.port_10.cfg_account_pg_2.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_2.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_2.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_2.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_2.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_2.xon_threshold((pbc_csr.port_10.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_2.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_2.show();}
    //
    pbc_csr.port_10.cfg_account_pg_3.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_3.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_3.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_3.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_3.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_3.xon_threshold((pbc_csr.port_10.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_3.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_3.show();}
    //
    pbc_csr.port_10.cfg_account_pg_4.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_4.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_4.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_4.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_4.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_4.xon_threshold((pbc_csr.port_10.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_4.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_4.show();}
    //
    pbc_csr.port_10.cfg_account_pg_5.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_5.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_5.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_5.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_5.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_5.xon_threshold((pbc_csr.port_10.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_5.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_5.show();}
    //
    pbc_csr.port_10.cfg_account_pg_6.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_6.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_6.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_6.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_6.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_6.xon_threshold((pbc_csr.port_10.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_6.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_6.show();}
    //
    pbc_csr.port_10.cfg_account_pg_7.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_7.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_7.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_7.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_7.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_7.xon_threshold((pbc_csr.port_10.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_7.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_7.show();}
    //
    pbc_csr.port_10.cfg_account_pg_8.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_8.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_8.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_8.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_8.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_8.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_8.xon_threshold((pbc_csr.port_10.cfg_account_pg_8.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_8.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_8.show();}
    //
    pbc_csr.port_10.cfg_account_pg_9.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_9.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_9.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_9.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_9.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_9.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_9.xon_threshold((pbc_csr.port_10.cfg_account_pg_9.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_9.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_9.show();}
    //
    pbc_csr.port_10.cfg_account_pg_10.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_10.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_10.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_10.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_10.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_10.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_10.xon_threshold((pbc_csr.port_10.cfg_account_pg_10.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_10.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_10.show();}
    //
    pbc_csr.port_10.cfg_account_pg_11.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_11.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_11.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_11.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_11.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_11.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_11.xon_threshold((pbc_csr.port_10.cfg_account_pg_11.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_11.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_11.show();}
    //
    pbc_csr.port_10.cfg_account_pg_12.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_12.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_12.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_12.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_12.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_12.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_12.xon_threshold((pbc_csr.port_10.cfg_account_pg_12.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_12.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_12.show();}
    //
    pbc_csr.port_10.cfg_account_pg_13.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_13.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_13.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_13.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_13.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_13.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_13.xon_threshold((pbc_csr.port_10.cfg_account_pg_13.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_13.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_13.show();}
    //
    pbc_csr.port_10.cfg_account_pg_14.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_14.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_14.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_14.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_14.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_14.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_14.xon_threshold((pbc_csr.port_10.cfg_account_pg_14.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_14.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_14.show();}
    //
    pbc_csr.port_10.cfg_account_pg_15.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_15.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_15.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_15.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_15.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_15.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_15.xon_threshold((pbc_csr.port_10.cfg_account_pg_15.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_15.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_15.show();}
    //
    pbc_csr.port_10.cfg_account_pg_16.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_16.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_16.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_16.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_16.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_16.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_16.xon_threshold((pbc_csr.port_10.cfg_account_pg_16.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_16.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_16.show();}
    //
    pbc_csr.port_10.cfg_account_pg_17.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_17.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_17.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_17.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_17.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_17.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_17.xon_threshold((pbc_csr.port_10.cfg_account_pg_17.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_17.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_17.show();}
    //
    pbc_csr.port_10.cfg_account_pg_18.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_18.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_18.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_18.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_18.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_18.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_18.xon_threshold((pbc_csr.port_10.cfg_account_pg_18.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_18.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_18.show();}
    //
    pbc_csr.port_10.cfg_account_pg_19.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_19.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_19.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_19.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_19.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_19.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_19.xon_threshold((pbc_csr.port_10.cfg_account_pg_19.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_19.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_19.show();}
    //
    pbc_csr.port_10.cfg_account_pg_20.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_20.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_20.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_20.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_20.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_20.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_20.xon_threshold((pbc_csr.port_10.cfg_account_pg_20.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_20.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_20.show();}
    //
    pbc_csr.port_10.cfg_account_pg_21.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_21.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_21.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_21.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_21.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_21.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_21.xon_threshold((pbc_csr.port_10.cfg_account_pg_21.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_21.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_21.show();}
    //
    pbc_csr.port_10.cfg_account_pg_22.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_22.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_22.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_22.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_22.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_22.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_22.xon_threshold((pbc_csr.port_10.cfg_account_pg_22.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_22.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_22.show();}
    //
    pbc_csr.port_10.cfg_account_pg_23.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_23.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_23.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_23.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_23.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_23.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_23.xon_threshold((pbc_csr.port_10.cfg_account_pg_23.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_23.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_23.show();}
    //
    pbc_csr.port_10.cfg_account_pg_24.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_24.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_24.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_24.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_24.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_24.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_24.xon_threshold((pbc_csr.port_10.cfg_account_pg_24.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_24.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_24.show();}
    //
    pbc_csr.port_10.cfg_account_pg_25.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_25.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_25.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_25.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_25.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_25.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_25.xon_threshold((pbc_csr.port_10.cfg_account_pg_25.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_25.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_25.show();}
    //
    pbc_csr.port_10.cfg_account_pg_26.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_26.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_26.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_26.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_26.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_26.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_26.xon_threshold((pbc_csr.port_10.cfg_account_pg_26.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_26.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_26.show();}
    //
    pbc_csr.port_10.cfg_account_pg_27.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_27.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_27.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_27.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_27.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_27.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_27.xon_threshold((pbc_csr.port_10.cfg_account_pg_27.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_27.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_27.show();}
    //
    pbc_csr.port_10.cfg_account_pg_28.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_28.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_28.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_28.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_28.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_28.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_28.xon_threshold((pbc_csr.port_10.cfg_account_pg_28.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_28.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_28.show();}
    //
    pbc_csr.port_10.cfg_account_pg_29.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_29.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_29.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_29.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_29.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_29.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_29.xon_threshold((pbc_csr.port_10.cfg_account_pg_29.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_29.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_29.show();}
    //
    pbc_csr.port_10.cfg_account_pg_30.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_30.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_30.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_30.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_30.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_30.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_30.xon_threshold((pbc_csr.port_10.cfg_account_pg_30.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_30.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_30.show();}
    //
    pbc_csr.port_10.cfg_account_pg_31.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_10.cfg_account_pg_31.load_from_cfg(0); 
    if (pbc_csr.port_10.cfg_account_pg_31.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_10.cfg_account_pg_31.xon_threshold(0);
    } else if (pbc_csr.port_10.cfg_account_pg_31.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_10.cfg_account_pg_31.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_10.cfg_account_pg_31.xon_threshold((pbc_csr.port_10.cfg_account_pg_31.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_10.cfg_account_pg_31.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_31.show();}
    //
    pbc_csr.port_11.cfg_account_pg_0.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_0.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_0.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_0.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_0.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_0.xon_threshold((pbc_csr.port_11.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_0.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_0.show();}
    //
    pbc_csr.port_11.cfg_account_pg_1.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_1.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_1.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_1.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_1.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_1.xon_threshold((pbc_csr.port_11.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_1.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_1.show();}
    //
    pbc_csr.port_11.cfg_account_pg_2.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_2.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_2.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_2.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_2.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_2.xon_threshold((pbc_csr.port_11.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_2.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_2.show();}
    //
    pbc_csr.port_11.cfg_account_pg_3.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_3.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_3.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_3.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_3.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_3.xon_threshold((pbc_csr.port_11.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_3.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_3.show();}
    //
    pbc_csr.port_11.cfg_account_pg_4.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_4.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_4.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_4.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_4.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_4.xon_threshold((pbc_csr.port_11.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_4.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_4.show();}
    //
    pbc_csr.port_11.cfg_account_pg_5.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_5.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_5.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_5.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_5.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_5.xon_threshold((pbc_csr.port_11.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_5.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_5.show();}
    //
    pbc_csr.port_11.cfg_account_pg_6.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_6.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_6.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_6.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_6.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_6.xon_threshold((pbc_csr.port_11.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_6.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_6.show();}
    //
    pbc_csr.port_11.cfg_account_pg_7.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_7.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_7.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_7.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_7.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_7.xon_threshold((pbc_csr.port_11.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_7.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_7.show();}
    //
    pbc_csr.port_11.cfg_account_pg_8.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_8.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_8.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_8.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_8.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_8.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_8.xon_threshold((pbc_csr.port_11.cfg_account_pg_8.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_8.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_8.show();}
    //
    pbc_csr.port_11.cfg_account_pg_9.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_9.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_9.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_9.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_9.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_9.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_9.xon_threshold((pbc_csr.port_11.cfg_account_pg_9.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_9.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_9.show();}
    //
    pbc_csr.port_11.cfg_account_pg_10.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_10.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_10.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_10.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_10.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_10.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_10.xon_threshold((pbc_csr.port_11.cfg_account_pg_10.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_10.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_10.show();}
    //
    pbc_csr.port_11.cfg_account_pg_11.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_11.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_11.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_11.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_11.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_11.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_11.xon_threshold((pbc_csr.port_11.cfg_account_pg_11.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_11.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_11.show();}
    //
    pbc_csr.port_11.cfg_account_pg_12.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_12.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_12.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_12.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_12.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_12.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_12.xon_threshold((pbc_csr.port_11.cfg_account_pg_12.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_12.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_12.show();}
    //
    pbc_csr.port_11.cfg_account_pg_13.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_13.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_13.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_13.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_13.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_13.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_13.xon_threshold((pbc_csr.port_11.cfg_account_pg_13.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_13.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_13.show();}
    //
    pbc_csr.port_11.cfg_account_pg_14.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_14.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_14.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_14.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_14.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_14.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_14.xon_threshold((pbc_csr.port_11.cfg_account_pg_14.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_14.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_14.show();}
    //
    pbc_csr.port_11.cfg_account_pg_15.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_15.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_15.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_15.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_15.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_15.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_15.xon_threshold((pbc_csr.port_11.cfg_account_pg_15.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_15.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_15.show();}
    //
    pbc_csr.port_11.cfg_account_pg_16.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_16.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_16.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_16.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_16.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_16.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_16.xon_threshold((pbc_csr.port_11.cfg_account_pg_16.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_16.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_16.show();}
    //
    pbc_csr.port_11.cfg_account_pg_17.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_17.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_17.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_17.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_17.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_17.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_17.xon_threshold((pbc_csr.port_11.cfg_account_pg_17.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_17.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_17.show();}
    //
    pbc_csr.port_11.cfg_account_pg_18.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_18.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_18.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_18.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_18.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_18.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_18.xon_threshold((pbc_csr.port_11.cfg_account_pg_18.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_18.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_18.show();}
    //
    pbc_csr.port_11.cfg_account_pg_19.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_19.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_19.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_19.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_19.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_19.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_19.xon_threshold((pbc_csr.port_11.cfg_account_pg_19.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_19.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_19.show();}
    //
    pbc_csr.port_11.cfg_account_pg_20.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_20.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_20.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_20.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_20.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_20.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_20.xon_threshold((pbc_csr.port_11.cfg_account_pg_20.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_20.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_20.show();}
    //
    pbc_csr.port_11.cfg_account_pg_21.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_21.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_21.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_21.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_21.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_21.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_21.xon_threshold((pbc_csr.port_11.cfg_account_pg_21.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_21.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_21.show();}
    //
    pbc_csr.port_11.cfg_account_pg_22.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_22.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_22.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_22.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_22.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_22.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_22.xon_threshold((pbc_csr.port_11.cfg_account_pg_22.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_22.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_22.show();}
    //
    pbc_csr.port_11.cfg_account_pg_23.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_23.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_23.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_23.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_23.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_23.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_23.xon_threshold((pbc_csr.port_11.cfg_account_pg_23.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_23.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_23.show();}
    //
    pbc_csr.port_11.cfg_account_pg_24.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_24.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_24.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_24.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_24.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_24.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_24.xon_threshold((pbc_csr.port_11.cfg_account_pg_24.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_24.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_24.show();}
    //
    pbc_csr.port_11.cfg_account_pg_25.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_25.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_25.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_25.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_25.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_25.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_25.xon_threshold((pbc_csr.port_11.cfg_account_pg_25.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_25.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_25.show();}
    //
    pbc_csr.port_11.cfg_account_pg_26.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_26.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_26.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_26.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_26.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_26.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_26.xon_threshold((pbc_csr.port_11.cfg_account_pg_26.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_26.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_26.show();}
    //
    pbc_csr.port_11.cfg_account_pg_27.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_27.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_27.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_27.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_27.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_27.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_27.xon_threshold((pbc_csr.port_11.cfg_account_pg_27.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_27.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_27.show();}
    //
    pbc_csr.port_11.cfg_account_pg_28.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_28.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_28.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_28.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_28.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_28.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_28.xon_threshold((pbc_csr.port_11.cfg_account_pg_28.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_28.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_28.show();}
    //
    pbc_csr.port_11.cfg_account_pg_29.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_29.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_29.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_29.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_29.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_29.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_29.xon_threshold((pbc_csr.port_11.cfg_account_pg_29.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_29.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_29.show();}
    //
    pbc_csr.port_11.cfg_account_pg_30.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_30.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_30.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_30.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_30.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_30.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_30.xon_threshold((pbc_csr.port_11.cfg_account_pg_30.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_30.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_30.show();}
    //
    pbc_csr.port_11.cfg_account_pg_31.reserved_min(1000); // make it very high for now (is 1000*4)
    pbc_csr.port_11.cfg_account_pg_31.load_from_cfg(0); 
    if (pbc_csr.port_11.cfg_account_pg_31.reserved_min().convert_to<uint32_t>() == 0) {
       pbc_csr.port_11.cfg_account_pg_31.xon_threshold(0);
    } else if (pbc_csr.port_11.cfg_account_pg_31.reserved_min().convert_to<uint32_t>()*4 < pbc_csr.port_11.cfg_account_pg_31.xon_threshold().convert_to<uint32_t>()) {
       pbc_csr.port_11.cfg_account_pg_31.xon_threshold((pbc_csr.port_11.cfg_account_pg_31.reserved_min().convert_to<uint32_t>()*4)-1);
    }
    pbc_csr.port_11.cfg_account_pg_31.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_31.show();}
    //
   bool ret_val = false;
  string apath = pbc_csr.get_hier_path()+".adjust_island_max";
  std::replace( apath.begin(), apath.end(), '.', '/');
  unsigned int adjust_island_max = sknobs_get_value((char *)apath.c_str(), 0);
   if (adjust_island_max) { ret_val = cap_pb_adjust_island_max(chip_id,inst_id); }
   if (!ret_val) {
      //start fc initialization for both islands. give 2000 cells to each island, islands 1 has cells 8192 and above.
     pbc_csr.cfg_fc_mgr_0.init_start(1);
     pbc_csr.cfg_fc_mgr_0.max_row(1000);
     pbc_csr.cfg_fc_mgr_0.min_cell(0);
     // get from cfg
     pbc_csr.cfg_fc_mgr_0.load_from_cfg(0);
     pbc_csr.cfg_fc_mgr_0.write();
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.cfg_fc_mgr_0.show();}

     pbc_csr.cfg_fc_mgr_1.init_start(1);
     pbc_csr.cfg_fc_mgr_1.max_row(1000);
     pbc_csr.cfg_fc_mgr_1.min_cell(4096);
     // get from cfg
     pbc_csr.cfg_fc_mgr_1.load_from_cfg(0);
     pbc_csr.cfg_fc_mgr_1.write();
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.cfg_fc_mgr_1.show();}
      
     pbc_csr.cfg_island_control.load_from_cfg();
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.cfg_island_control.show();}
   }

   //initialize the reference counter to zero
  pbc_csr.cfg_rc.init_start(1);
  pbc_csr.cfg_rc.write();
  if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.cfg_rc.show();}

 }
}

void cap_pb_init_done(int chip_id, int inst_id) {

PLOG_MSG("inside init done\n");
 unsigned int pb_skip_init = SKNOBS_GET("tb/pb_skip_init", 0);

 if (pb_skip_init == 0) {

 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
 //cap_pbm_csr_t & pbm_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbm_csr_t, chip_id, inst_id);

   //poll for init routine done
  //bool done;
  int timeout=200;
  do {    
      pbc_csr.sta_fc_mgr_0.read();
      PLOG_MSG("timeout:"<< timeout << " ; sta_fc_mgr_0 returned \n")
      if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.sta_fc_mgr_0.show();}
      SLEEP(100);
      timeout--;
  } while (pbc_csr.sta_fc_mgr_0.init_done().convert_to<uint32_t>()==0 && timeout>0);
  if (timeout <1) {
      PLOG_ERR("sta_fc_mgr_0 init done timed out \n");
  }

  timeout=100;
  do {    
      pbc_csr.sta_fc_mgr_1.read();
      PLOG_MSG("timeout:"<< timeout << " ; sta_fc_mgr_1 returned \n")
      if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.sta_fc_mgr_1.show();}
      SLEEP(100);
      timeout--;
  } while (pbc_csr.sta_fc_mgr_1.init_done().convert_to<uint32_t>()==0 && timeout>0);
  if (timeout <1) {
      PLOG_ERR("sta_fc_mgr_1 init done timed out \n");
  }

  timeout=100;
  do {    
      pbc_csr.sta_rc.read();
      PLOG_MSG("timeout:"<< timeout << " ; ssta_rc returned \n")
      if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.sta_rc.show();}
      SLEEP(100);
      timeout--;
  } while (pbc_csr.sta_rc.init_done().convert_to<uint32_t>()==0 && timeout>0);
  if (timeout <1) {
      PLOG_ERR("sta_rc init done timed out \n");
  }
 // PLOG_MSG ("Enabling all interrupts" << std::endl);
 // enable_all_interrupts(&pbc_csr);
 // enable_all_interrupts(&pbm_csr);
 }
}

void cap_pb_load_from_cfg(int chip_id, int inst_id) {
PLOG_MSG("inside cap_pb_load_from_cfg \n");
 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
 //cap_pbm_csr_t & pbm_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbm_csr_t, chip_id, inst_id);

 unsigned int pb_skip_init = SKNOBS_GET("tb/pb_skip_init", 0);

 if (pb_skip_init == 0) {


#ifdef MODULE_SIM
  unsigned int pre_parser_test = SKNOBS_GET("tb/pre_parser_test", 0);
  if(pre_parser_test) {
    RRKnob   rand_knob48("rand_knob", 0, 0xffffffffffff);
    RRKnob   rand_knob("rand_knob", 0, 0xffffffff);
    cpp_int_helper hlp;
    cpp_int dscp_val;
    cpp_int type_val;

     pbc_csr.hbm.hbm_port_0.cfg_hbm_parser.dot1q_type(rand_knob.eval());
     pbc_csr.hbm.hbm_port_0.cfg_hbm_parser.ipv4_type(rand_knob.eval());
     pbc_csr.hbm.hbm_port_0.cfg_hbm_parser.ipv6_type(rand_knob.eval());
     while(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser.ipv4_type() == pbc_csr.hbm.hbm_port_0.cfg_hbm_parser.dot1q_type()) {
        pbc_csr.hbm.hbm_port_0.cfg_hbm_parser.ipv4_type(rand_knob.eval());
     }
     while( (pbc_csr.hbm.hbm_port_0.cfg_hbm_parser.ipv6_type() == pbc_csr.hbm.hbm_port_0.cfg_hbm_parser.dot1q_type())
         || (pbc_csr.hbm.hbm_port_0.cfg_hbm_parser.ipv6_type() == pbc_csr.hbm.hbm_port_0.cfg_hbm_parser.ipv4_type()) ) {
        pbc_csr.hbm.hbm_port_0.cfg_hbm_parser.ipv6_type(rand_knob.eval());
     }
     pbc_csr.hbm.hbm_port_0.cfg_hbm_parser.use_ip(rand_knob.eval() % 2);
     pbc_csr.hbm.hbm_port_0.cfg_hbm_parser.use_dot1q(rand_knob.eval() % 2);
     pbc_csr.hbm.hbm_port_0.cfg_hbm_parser.default_cos(rand_knob.eval());

     for(int idx=0; idx<192 /*pbc_csr.hbm.hbm_port_0.cfg_hbm_parser.dscp_map().size()*/; idx=idx+32) {
         hlp.set_slc(dscp_val, rand_knob.eval(), idx , idx+31);
     }
     pbc_csr.hbm.hbm_port_0.cfg_hbm_parser.dscp_map(dscp_val);
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_0.cfg_hbm_parser.show();}
     pbc_csr.hbm.hbm_port_0.cfg_hbm_parser.write();

     pbc_csr.hbm.hbm_port_1.cfg_hbm_parser.dot1q_type(rand_knob.eval());
     pbc_csr.hbm.hbm_port_1.cfg_hbm_parser.ipv4_type(rand_knob.eval());
     pbc_csr.hbm.hbm_port_1.cfg_hbm_parser.ipv6_type(rand_knob.eval());
     while(pbc_csr.hbm.hbm_port_1.cfg_hbm_parser.ipv4_type() == pbc_csr.hbm.hbm_port_1.cfg_hbm_parser.dot1q_type()) {
        pbc_csr.hbm.hbm_port_1.cfg_hbm_parser.ipv4_type(rand_knob.eval());
     }
     while( (pbc_csr.hbm.hbm_port_1.cfg_hbm_parser.ipv6_type() == pbc_csr.hbm.hbm_port_1.cfg_hbm_parser.dot1q_type())
         || (pbc_csr.hbm.hbm_port_1.cfg_hbm_parser.ipv6_type() == pbc_csr.hbm.hbm_port_1.cfg_hbm_parser.ipv4_type()) ) {
        pbc_csr.hbm.hbm_port_1.cfg_hbm_parser.ipv6_type(rand_knob.eval());
     }
     pbc_csr.hbm.hbm_port_1.cfg_hbm_parser.use_ip(rand_knob.eval() % 2);
     pbc_csr.hbm.hbm_port_1.cfg_hbm_parser.use_dot1q(rand_knob.eval() % 2);
     pbc_csr.hbm.hbm_port_1.cfg_hbm_parser.default_cos(rand_knob.eval());

     for(int idx=0; idx<192 /*pbc_csr.hbm.hbm_port_1.cfg_hbm_parser.dscp_map().size()*/; idx=idx+32) {
         hlp.set_slc(dscp_val, rand_knob.eval(), idx , idx+31);
     }
     pbc_csr.hbm.hbm_port_1.cfg_hbm_parser.dscp_map(dscp_val);
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_1.cfg_hbm_parser.show();}
     pbc_csr.hbm.hbm_port_1.cfg_hbm_parser.write();

     pbc_csr.hbm.hbm_port_2.cfg_hbm_parser.dot1q_type(rand_knob.eval());
     pbc_csr.hbm.hbm_port_2.cfg_hbm_parser.ipv4_type(rand_knob.eval());
     pbc_csr.hbm.hbm_port_2.cfg_hbm_parser.ipv6_type(rand_knob.eval());
     while(pbc_csr.hbm.hbm_port_2.cfg_hbm_parser.ipv4_type() == pbc_csr.hbm.hbm_port_2.cfg_hbm_parser.dot1q_type()) {
        pbc_csr.hbm.hbm_port_2.cfg_hbm_parser.ipv4_type(rand_knob.eval());
     }
     while( (pbc_csr.hbm.hbm_port_2.cfg_hbm_parser.ipv6_type() == pbc_csr.hbm.hbm_port_2.cfg_hbm_parser.dot1q_type())
         || (pbc_csr.hbm.hbm_port_2.cfg_hbm_parser.ipv6_type() == pbc_csr.hbm.hbm_port_2.cfg_hbm_parser.ipv4_type()) ) {
        pbc_csr.hbm.hbm_port_2.cfg_hbm_parser.ipv6_type(rand_knob.eval());
     }
     pbc_csr.hbm.hbm_port_2.cfg_hbm_parser.use_ip(rand_knob.eval() % 2);
     pbc_csr.hbm.hbm_port_2.cfg_hbm_parser.use_dot1q(rand_knob.eval() % 2);
     pbc_csr.hbm.hbm_port_2.cfg_hbm_parser.default_cos(rand_knob.eval());

     for(int idx=0; idx<192 /*pbc_csr.hbm.hbm_port_2.cfg_hbm_parser.dscp_map().size()*/; idx=idx+32) {
         hlp.set_slc(dscp_val, rand_knob.eval(), idx , idx+31);
     }
     pbc_csr.hbm.hbm_port_2.cfg_hbm_parser.dscp_map(dscp_val);
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_2.cfg_hbm_parser.show();}
     pbc_csr.hbm.hbm_port_2.cfg_hbm_parser.write();

     pbc_csr.hbm.hbm_port_3.cfg_hbm_parser.dot1q_type(rand_knob.eval());
     pbc_csr.hbm.hbm_port_3.cfg_hbm_parser.ipv4_type(rand_knob.eval());
     pbc_csr.hbm.hbm_port_3.cfg_hbm_parser.ipv6_type(rand_knob.eval());
     while(pbc_csr.hbm.hbm_port_3.cfg_hbm_parser.ipv4_type() == pbc_csr.hbm.hbm_port_3.cfg_hbm_parser.dot1q_type()) {
        pbc_csr.hbm.hbm_port_3.cfg_hbm_parser.ipv4_type(rand_knob.eval());
     }
     while( (pbc_csr.hbm.hbm_port_3.cfg_hbm_parser.ipv6_type() == pbc_csr.hbm.hbm_port_3.cfg_hbm_parser.dot1q_type())
         || (pbc_csr.hbm.hbm_port_3.cfg_hbm_parser.ipv6_type() == pbc_csr.hbm.hbm_port_3.cfg_hbm_parser.ipv4_type()) ) {
        pbc_csr.hbm.hbm_port_3.cfg_hbm_parser.ipv6_type(rand_knob.eval());
     }
     pbc_csr.hbm.hbm_port_3.cfg_hbm_parser.use_ip(rand_knob.eval() % 2);
     pbc_csr.hbm.hbm_port_3.cfg_hbm_parser.use_dot1q(rand_knob.eval() % 2);
     pbc_csr.hbm.hbm_port_3.cfg_hbm_parser.default_cos(rand_knob.eval());

     for(int idx=0; idx<192 /*pbc_csr.hbm.hbm_port_3.cfg_hbm_parser.dscp_map().size()*/; idx=idx+32) {
         hlp.set_slc(dscp_val, rand_knob.eval(), idx , idx+31);
     }
     pbc_csr.hbm.hbm_port_3.cfg_hbm_parser.dscp_map(dscp_val);
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_3.cfg_hbm_parser.show();}
     pbc_csr.hbm.hbm_port_3.cfg_hbm_parser.write();

     pbc_csr.hbm.hbm_port_4.cfg_hbm_parser.dot1q_type(rand_knob.eval());
     pbc_csr.hbm.hbm_port_4.cfg_hbm_parser.ipv4_type(rand_knob.eval());
     pbc_csr.hbm.hbm_port_4.cfg_hbm_parser.ipv6_type(rand_knob.eval());
     while(pbc_csr.hbm.hbm_port_4.cfg_hbm_parser.ipv4_type() == pbc_csr.hbm.hbm_port_4.cfg_hbm_parser.dot1q_type()) {
        pbc_csr.hbm.hbm_port_4.cfg_hbm_parser.ipv4_type(rand_knob.eval());
     }
     while( (pbc_csr.hbm.hbm_port_4.cfg_hbm_parser.ipv6_type() == pbc_csr.hbm.hbm_port_4.cfg_hbm_parser.dot1q_type())
         || (pbc_csr.hbm.hbm_port_4.cfg_hbm_parser.ipv6_type() == pbc_csr.hbm.hbm_port_4.cfg_hbm_parser.ipv4_type()) ) {
        pbc_csr.hbm.hbm_port_4.cfg_hbm_parser.ipv6_type(rand_knob.eval());
     }
     pbc_csr.hbm.hbm_port_4.cfg_hbm_parser.use_ip(rand_knob.eval() % 2);
     pbc_csr.hbm.hbm_port_4.cfg_hbm_parser.use_dot1q(rand_knob.eval() % 2);
     pbc_csr.hbm.hbm_port_4.cfg_hbm_parser.default_cos(rand_knob.eval());

     for(int idx=0; idx<192 /*pbc_csr.hbm.hbm_port_4.cfg_hbm_parser.dscp_map().size()*/; idx=idx+32) {
         hlp.set_slc(dscp_val, rand_knob.eval(), idx , idx+31);
     }
     pbc_csr.hbm.hbm_port_4.cfg_hbm_parser.dscp_map(dscp_val);
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_4.cfg_hbm_parser.show();}
     pbc_csr.hbm.hbm_port_4.cfg_hbm_parser.write();

     pbc_csr.hbm.hbm_port_5.cfg_hbm_parser.dot1q_type(rand_knob.eval());
     pbc_csr.hbm.hbm_port_5.cfg_hbm_parser.ipv4_type(rand_knob.eval());
     pbc_csr.hbm.hbm_port_5.cfg_hbm_parser.ipv6_type(rand_knob.eval());
     while(pbc_csr.hbm.hbm_port_5.cfg_hbm_parser.ipv4_type() == pbc_csr.hbm.hbm_port_5.cfg_hbm_parser.dot1q_type()) {
        pbc_csr.hbm.hbm_port_5.cfg_hbm_parser.ipv4_type(rand_knob.eval());
     }
     while( (pbc_csr.hbm.hbm_port_5.cfg_hbm_parser.ipv6_type() == pbc_csr.hbm.hbm_port_5.cfg_hbm_parser.dot1q_type())
         || (pbc_csr.hbm.hbm_port_5.cfg_hbm_parser.ipv6_type() == pbc_csr.hbm.hbm_port_5.cfg_hbm_parser.ipv4_type()) ) {
        pbc_csr.hbm.hbm_port_5.cfg_hbm_parser.ipv6_type(rand_knob.eval());
     }
     pbc_csr.hbm.hbm_port_5.cfg_hbm_parser.use_ip(rand_knob.eval() % 2);
     pbc_csr.hbm.hbm_port_5.cfg_hbm_parser.use_dot1q(rand_knob.eval() % 2);
     pbc_csr.hbm.hbm_port_5.cfg_hbm_parser.default_cos(rand_knob.eval());

     for(int idx=0; idx<192 /*pbc_csr.hbm.hbm_port_5.cfg_hbm_parser.dscp_map().size()*/; idx=idx+32) {
         hlp.set_slc(dscp_val, rand_knob.eval(), idx , idx+31);
     }
     pbc_csr.hbm.hbm_port_5.cfg_hbm_parser.dscp_map(dscp_val);
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_5.cfg_hbm_parser.show();}
     pbc_csr.hbm.hbm_port_5.cfg_hbm_parser.write();

     pbc_csr.hbm.hbm_port_6.cfg_hbm_parser.dot1q_type(rand_knob.eval());
     pbc_csr.hbm.hbm_port_6.cfg_hbm_parser.ipv4_type(rand_knob.eval());
     pbc_csr.hbm.hbm_port_6.cfg_hbm_parser.ipv6_type(rand_knob.eval());
     while(pbc_csr.hbm.hbm_port_6.cfg_hbm_parser.ipv4_type() == pbc_csr.hbm.hbm_port_6.cfg_hbm_parser.dot1q_type()) {
        pbc_csr.hbm.hbm_port_6.cfg_hbm_parser.ipv4_type(rand_knob.eval());
     }
     while( (pbc_csr.hbm.hbm_port_6.cfg_hbm_parser.ipv6_type() == pbc_csr.hbm.hbm_port_6.cfg_hbm_parser.dot1q_type())
         || (pbc_csr.hbm.hbm_port_6.cfg_hbm_parser.ipv6_type() == pbc_csr.hbm.hbm_port_6.cfg_hbm_parser.ipv4_type()) ) {
        pbc_csr.hbm.hbm_port_6.cfg_hbm_parser.ipv6_type(rand_knob.eval());
     }
     pbc_csr.hbm.hbm_port_6.cfg_hbm_parser.use_ip(rand_knob.eval() % 2);
     pbc_csr.hbm.hbm_port_6.cfg_hbm_parser.use_dot1q(rand_knob.eval() % 2);
     pbc_csr.hbm.hbm_port_6.cfg_hbm_parser.default_cos(rand_knob.eval());

     for(int idx=0; idx<192 /*pbc_csr.hbm.hbm_port_6.cfg_hbm_parser.dscp_map().size()*/; idx=idx+32) {
         hlp.set_slc(dscp_val, rand_knob.eval(), idx , idx+31);
     }
     pbc_csr.hbm.hbm_port_6.cfg_hbm_parser.dscp_map(dscp_val);
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_6.cfg_hbm_parser.show();}
     pbc_csr.hbm.hbm_port_6.cfg_hbm_parser.write();

     pbc_csr.hbm.hbm_port_7.cfg_hbm_parser.dot1q_type(rand_knob.eval());
     pbc_csr.hbm.hbm_port_7.cfg_hbm_parser.ipv4_type(rand_knob.eval());
     pbc_csr.hbm.hbm_port_7.cfg_hbm_parser.ipv6_type(rand_knob.eval());
     while(pbc_csr.hbm.hbm_port_7.cfg_hbm_parser.ipv4_type() == pbc_csr.hbm.hbm_port_7.cfg_hbm_parser.dot1q_type()) {
        pbc_csr.hbm.hbm_port_7.cfg_hbm_parser.ipv4_type(rand_knob.eval());
     }
     while( (pbc_csr.hbm.hbm_port_7.cfg_hbm_parser.ipv6_type() == pbc_csr.hbm.hbm_port_7.cfg_hbm_parser.dot1q_type())
         || (pbc_csr.hbm.hbm_port_7.cfg_hbm_parser.ipv6_type() == pbc_csr.hbm.hbm_port_7.cfg_hbm_parser.ipv4_type()) ) {
        pbc_csr.hbm.hbm_port_7.cfg_hbm_parser.ipv6_type(rand_knob.eval());
     }
     pbc_csr.hbm.hbm_port_7.cfg_hbm_parser.use_ip(rand_knob.eval() % 2);
     pbc_csr.hbm.hbm_port_7.cfg_hbm_parser.use_dot1q(rand_knob.eval() % 2);
     pbc_csr.hbm.hbm_port_7.cfg_hbm_parser.default_cos(rand_knob.eval());

     for(int idx=0; idx<192 /*pbc_csr.hbm.hbm_port_7.cfg_hbm_parser.dscp_map().size()*/; idx=idx+32) {
         hlp.set_slc(dscp_val, rand_knob.eval(), idx , idx+31);
     }
     pbc_csr.hbm.hbm_port_7.cfg_hbm_parser.dscp_map(dscp_val);
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_7.cfg_hbm_parser.show();}
     pbc_csr.hbm.hbm_port_7.cfg_hbm_parser.write();

     pbc_csr.hbm.hbm_port_8.cfg_hbm_parser.dot1q_type(rand_knob.eval());
     pbc_csr.hbm.hbm_port_8.cfg_hbm_parser.ipv4_type(rand_knob.eval());
     pbc_csr.hbm.hbm_port_8.cfg_hbm_parser.ipv6_type(rand_knob.eval());
     while(pbc_csr.hbm.hbm_port_8.cfg_hbm_parser.ipv4_type() == pbc_csr.hbm.hbm_port_8.cfg_hbm_parser.dot1q_type()) {
        pbc_csr.hbm.hbm_port_8.cfg_hbm_parser.ipv4_type(rand_knob.eval());
     }
     while( (pbc_csr.hbm.hbm_port_8.cfg_hbm_parser.ipv6_type() == pbc_csr.hbm.hbm_port_8.cfg_hbm_parser.dot1q_type())
         || (pbc_csr.hbm.hbm_port_8.cfg_hbm_parser.ipv6_type() == pbc_csr.hbm.hbm_port_8.cfg_hbm_parser.ipv4_type()) ) {
        pbc_csr.hbm.hbm_port_8.cfg_hbm_parser.ipv6_type(rand_knob.eval());
     }
     pbc_csr.hbm.hbm_port_8.cfg_hbm_parser.use_ip(rand_knob.eval() % 2);
     pbc_csr.hbm.hbm_port_8.cfg_hbm_parser.use_dot1q(rand_knob.eval() % 2);
     pbc_csr.hbm.hbm_port_8.cfg_hbm_parser.default_cos(rand_knob.eval());

     for(int idx=0; idx<192 /*pbc_csr.hbm.hbm_port_8.cfg_hbm_parser.dscp_map().size()*/; idx=idx+32) {
         hlp.set_slc(dscp_val, rand_knob.eval(), idx , idx+31);
     }
     pbc_csr.hbm.hbm_port_8.cfg_hbm_parser.dscp_map(dscp_val);
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_8.cfg_hbm_parser.show();}
     pbc_csr.hbm.hbm_port_8.cfg_hbm_parser.write();

    // program them the same for all ports
     pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_da.entry_0(rand_knob48.eval());
     type_val = rand_knob.eval() % (1<<16); if (type_val <= 1500) { type_val = 1501 ;} 
     pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_type.entry_0(type_val);
     pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_enable.entry_0(rand_knob.eval() % (1<<2));
     pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_cos.entry_0(rand_knob.eval() % (1<<3));
     pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_da.entry_1(rand_knob48.eval());
     type_val = rand_knob.eval() % (1<<16); if (type_val <= 1500) { type_val = 1501 ;} 
     pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_type.entry_1(type_val);
     pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_enable.entry_1(rand_knob.eval() % (1<<2));
     pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_cos.entry_1(rand_knob.eval() % (1<<3));
     pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_da.entry_2(rand_knob48.eval());
     type_val = rand_knob.eval() % (1<<16); if (type_val <= 1500) { type_val = 1501 ;} 
     pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_type.entry_2(type_val);
     pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_enable.entry_2(rand_knob.eval() % (1<<2));
     pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_cos.entry_2(rand_knob.eval() % (1<<3));
     pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_da.entry_3(rand_knob48.eval());
     type_val = rand_knob.eval() % (1<<16); if (type_val <= 1500) { type_val = 1501 ;} 
     pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_type.entry_3(type_val);
     pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_enable.entry_3(rand_knob.eval() % (1<<2));
     pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_cos.entry_3(rand_knob.eval() % (1<<3));
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_da.show();}
     pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_da.write();
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_type.show();}
     pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_type.write();
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_enable.show();}
     pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_enable.write();
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_cos.show();}
     pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_cos.write();
     pbc_csr.hbm.hbm_port_1.cfg_hbm_parser_cam_da.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_da.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_1.cfg_hbm_parser_cam_da.show();}
     pbc_csr.hbm.hbm_port_1.cfg_hbm_parser_cam_da.write();
     pbc_csr.hbm.hbm_port_1.cfg_hbm_parser_cam_type.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_type.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_1.cfg_hbm_parser_cam_type.show();}
     pbc_csr.hbm.hbm_port_1.cfg_hbm_parser_cam_type.write();
     pbc_csr.hbm.hbm_port_1.cfg_hbm_parser_cam_enable.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_enable.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_1.cfg_hbm_parser_cam_enable.show();}
     pbc_csr.hbm.hbm_port_1.cfg_hbm_parser_cam_enable.write();
     pbc_csr.hbm.hbm_port_1.cfg_hbm_parser_cam_cos.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_cos.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_1.cfg_hbm_parser_cam_cos.show();}
     pbc_csr.hbm.hbm_port_1.cfg_hbm_parser_cam_cos.write();
     pbc_csr.hbm.hbm_port_2.cfg_hbm_parser_cam_da.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_da.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_2.cfg_hbm_parser_cam_da.show();}
     pbc_csr.hbm.hbm_port_2.cfg_hbm_parser_cam_da.write();
     pbc_csr.hbm.hbm_port_2.cfg_hbm_parser_cam_type.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_type.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_2.cfg_hbm_parser_cam_type.show();}
     pbc_csr.hbm.hbm_port_2.cfg_hbm_parser_cam_type.write();
     pbc_csr.hbm.hbm_port_2.cfg_hbm_parser_cam_enable.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_enable.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_2.cfg_hbm_parser_cam_enable.show();}
     pbc_csr.hbm.hbm_port_2.cfg_hbm_parser_cam_enable.write();
     pbc_csr.hbm.hbm_port_2.cfg_hbm_parser_cam_cos.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_cos.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_2.cfg_hbm_parser_cam_cos.show();}
     pbc_csr.hbm.hbm_port_2.cfg_hbm_parser_cam_cos.write();
     pbc_csr.hbm.hbm_port_3.cfg_hbm_parser_cam_da.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_da.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_3.cfg_hbm_parser_cam_da.show();}
     pbc_csr.hbm.hbm_port_3.cfg_hbm_parser_cam_da.write();
     pbc_csr.hbm.hbm_port_3.cfg_hbm_parser_cam_type.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_type.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_3.cfg_hbm_parser_cam_type.show();}
     pbc_csr.hbm.hbm_port_3.cfg_hbm_parser_cam_type.write();
     pbc_csr.hbm.hbm_port_3.cfg_hbm_parser_cam_enable.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_enable.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_3.cfg_hbm_parser_cam_enable.show();}
     pbc_csr.hbm.hbm_port_3.cfg_hbm_parser_cam_enable.write();
     pbc_csr.hbm.hbm_port_3.cfg_hbm_parser_cam_cos.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_cos.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_3.cfg_hbm_parser_cam_cos.show();}
     pbc_csr.hbm.hbm_port_3.cfg_hbm_parser_cam_cos.write();
     pbc_csr.hbm.hbm_port_4.cfg_hbm_parser_cam_da.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_da.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_4.cfg_hbm_parser_cam_da.show();}
     pbc_csr.hbm.hbm_port_4.cfg_hbm_parser_cam_da.write();
     pbc_csr.hbm.hbm_port_4.cfg_hbm_parser_cam_type.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_type.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_4.cfg_hbm_parser_cam_type.show();}
     pbc_csr.hbm.hbm_port_4.cfg_hbm_parser_cam_type.write();
     pbc_csr.hbm.hbm_port_4.cfg_hbm_parser_cam_enable.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_enable.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_4.cfg_hbm_parser_cam_enable.show();}
     pbc_csr.hbm.hbm_port_4.cfg_hbm_parser_cam_enable.write();
     pbc_csr.hbm.hbm_port_4.cfg_hbm_parser_cam_cos.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_cos.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_4.cfg_hbm_parser_cam_cos.show();}
     pbc_csr.hbm.hbm_port_4.cfg_hbm_parser_cam_cos.write();
     pbc_csr.hbm.hbm_port_5.cfg_hbm_parser_cam_da.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_da.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_5.cfg_hbm_parser_cam_da.show();}
     pbc_csr.hbm.hbm_port_5.cfg_hbm_parser_cam_da.write();
     pbc_csr.hbm.hbm_port_5.cfg_hbm_parser_cam_type.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_type.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_5.cfg_hbm_parser_cam_type.show();}
     pbc_csr.hbm.hbm_port_5.cfg_hbm_parser_cam_type.write();
     pbc_csr.hbm.hbm_port_5.cfg_hbm_parser_cam_enable.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_enable.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_5.cfg_hbm_parser_cam_enable.show();}
     pbc_csr.hbm.hbm_port_5.cfg_hbm_parser_cam_enable.write();
     pbc_csr.hbm.hbm_port_5.cfg_hbm_parser_cam_cos.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_cos.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_5.cfg_hbm_parser_cam_cos.show();}
     pbc_csr.hbm.hbm_port_5.cfg_hbm_parser_cam_cos.write();
     pbc_csr.hbm.hbm_port_6.cfg_hbm_parser_cam_da.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_da.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_6.cfg_hbm_parser_cam_da.show();}
     pbc_csr.hbm.hbm_port_6.cfg_hbm_parser_cam_da.write();
     pbc_csr.hbm.hbm_port_6.cfg_hbm_parser_cam_type.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_type.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_6.cfg_hbm_parser_cam_type.show();}
     pbc_csr.hbm.hbm_port_6.cfg_hbm_parser_cam_type.write();
     pbc_csr.hbm.hbm_port_6.cfg_hbm_parser_cam_enable.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_enable.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_6.cfg_hbm_parser_cam_enable.show();}
     pbc_csr.hbm.hbm_port_6.cfg_hbm_parser_cam_enable.write();
     pbc_csr.hbm.hbm_port_6.cfg_hbm_parser_cam_cos.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_cos.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_6.cfg_hbm_parser_cam_cos.show();}
     pbc_csr.hbm.hbm_port_6.cfg_hbm_parser_cam_cos.write();
     pbc_csr.hbm.hbm_port_7.cfg_hbm_parser_cam_da.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_da.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_7.cfg_hbm_parser_cam_da.show();}
     pbc_csr.hbm.hbm_port_7.cfg_hbm_parser_cam_da.write();
     pbc_csr.hbm.hbm_port_7.cfg_hbm_parser_cam_type.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_type.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_7.cfg_hbm_parser_cam_type.show();}
     pbc_csr.hbm.hbm_port_7.cfg_hbm_parser_cam_type.write();
     pbc_csr.hbm.hbm_port_7.cfg_hbm_parser_cam_enable.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_enable.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_7.cfg_hbm_parser_cam_enable.show();}
     pbc_csr.hbm.hbm_port_7.cfg_hbm_parser_cam_enable.write();
     pbc_csr.hbm.hbm_port_7.cfg_hbm_parser_cam_cos.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_cos.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_7.cfg_hbm_parser_cam_cos.show();}
     pbc_csr.hbm.hbm_port_7.cfg_hbm_parser_cam_cos.write();
     pbc_csr.hbm.hbm_port_8.cfg_hbm_parser_cam_da.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_da.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_8.cfg_hbm_parser_cam_da.show();}
     pbc_csr.hbm.hbm_port_8.cfg_hbm_parser_cam_da.write();
     pbc_csr.hbm.hbm_port_8.cfg_hbm_parser_cam_type.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_type.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_8.cfg_hbm_parser_cam_type.show();}
     pbc_csr.hbm.hbm_port_8.cfg_hbm_parser_cam_type.write();
     pbc_csr.hbm.hbm_port_8.cfg_hbm_parser_cam_enable.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_enable.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_8.cfg_hbm_parser_cam_enable.show();}
     pbc_csr.hbm.hbm_port_8.cfg_hbm_parser_cam_enable.write();
     pbc_csr.hbm.hbm_port_8.cfg_hbm_parser_cam_cos.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_parser_cam_cos.all());
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_8.cfg_hbm_parser_cam_cos.show();}
     pbc_csr.hbm.hbm_port_8.cfg_hbm_parser_cam_cos.write();
  }
#endif
    string spath;

    // decoder
    cap_pbc_oq_map_t oq_map;
    cap_pbc_pg8_map_t pg8_map;
    cap_pbc_pg16_map_t pg16_map;
    cap_pbc_pg32_map_t pg32_map;
    cap_pbc_eth_oq_xoff_map_t eth_oq_xoff_map; 
    cap_pbc_p4_oq_xoff_map_t p4_oq_xoff_map; 
    oq_map.init();
    pg8_map.init();
    pg16_map.init();
    pg32_map.init();
    eth_oq_xoff_map.init();
    p4_oq_xoff_map.init();
    uint32_t cfg_found_cnt;
    uint32_t recirc_oq[12];

    // check if recirc queue set
    pbc_csr.port_10.cfg_write_control.load_from_cfg(0);
    recirc_oq[10] = pbc_csr.port_10.cfg_write_control.recirc_oq().convert_to<uint32_t>();
    pbc_csr.port_11.cfg_write_control.load_from_cfg(0);
    recirc_oq[11] = pbc_csr.port_11.cfg_write_control.recirc_oq().convert_to<uint32_t>();
    // pbc_csr.port_10.cfg_oq.set_access_no_zero_time(1);
    // pbc_csr.port_11.cfg_oq.set_access_no_zero_time(1);
    // parse cfg
    cfg_found_cnt = pbc_csr.cfg_parser0.load_from_cfg_rtn_status(0);
    oq_map.all(pbc_csr.cfg_parser0.oq_map()); // init
    oq_map.set_name(pbc_csr.cfg_parser0.get_hier_path()+".decoder");
    cfg_found_cnt +=oq_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       if (pbc_csr.cfg_parser0.default_port()  == E_CAP_PB_PORT_P4_RWR || pbc_csr.cfg_parser0.default_port()  == E_CAP_PB_PORT_P4_PCK) {
          spath = oq_map.get_hier_path()+".fix_oqueue";
          std::replace( spath.begin(), spath.end(), '.', '/');
          //if (sknobs_exists((char *)spath.c_str())) {
          unsigned int fix_oqueue = sknobs_get_value((char *)spath.c_str(), 0);
          //}
          for (int ii=0; ii<8; ii++) {
             if ((oq_map.oqueue(ii) > 29) && pbc_csr.cfg_parser0.default_port()  == E_CAP_PB_PORT_P4_RWR) {
               if (fix_oqueue == 1) {
                  oq_map.oqueue(oq_map.oqueue(ii)-2,ii);
               } else {
                  PLOG_ERR("cap_pb_load_from_cfg : Programming error: cfg_parser0.default_port == E_CAP_PB_PORT_P4_RWR and oq_map["<<ii<<"] is > 29 : " << oq_map.oqueue(ii)<<endl);
               }
             }
             if (pbc_csr.port_11.cfg_write_control.recirc_enable().convert_to<uint32_t>() ==1) {
                if ((oq_map.oqueue(ii) == pbc_csr.port_11.cfg_write_control.recirc_oq().convert_to<uint32_t>()) && pbc_csr.cfg_parser0.default_port()  == 11) {
                  //PLOG_MSG("Came here for 11 "<<ii<< " oqueue="<<oq_map.oqueue(ii)<<" recirc_oq=" <<pbc_csr.port_11.cfg_write_control.recirc_oq().convert_to<uint32_t>() <<endl);
                  if (fix_oqueue == 1) {
                     if (oq_map.oqueue(ii) == 0) {
                        oq_map.oqueue(oq_map.oqueue(ii)+1,ii);
                     } else {
                        oq_map.oqueue(oq_map.oqueue(ii)-1,ii);
                     }
                  } else {
                     PLOG_ERR("cap_pb_load_from_cfg : Programming error: cfg_parser0.default_port == 11 and oq_map["<<ii<<"] == port_11.cfg_write_control.recirc_oq : " << oq_map.oqueue(ii)<<endl);
                  }
                }
             }
             if (pbc_csr.port_10.cfg_write_control.recirc_enable().convert_to<uint32_t>() ==1) {
                if ((oq_map.oqueue(ii) == pbc_csr.port_10.cfg_write_control.recirc_oq().convert_to<uint32_t>()) && pbc_csr.cfg_parser0.default_port()  == 10) {
                  //PLOG_MSG("Came here for 10 "<<ii<< " oqueue="<<oq_map.oqueue(ii)<<" recirc_oq=" <<pbc_csr.port_10.cfg_write_control.recirc_oq().convert_to<uint32_t>() <<endl);
                  if (fix_oqueue == 1) {
                     if (oq_map.oqueue(ii) == 0) {
                        oq_map.oqueue(oq_map.oqueue(ii)+1,ii);
                     } else {
                        oq_map.oqueue(oq_map.oqueue(ii)-1,ii);
                     }
                  } else {
                     PLOG_ERR("cap_pb_load_from_cfg : Programming error: cfg_parser0.default_port == 10 and oq_map["<<ii<<"] == port_10.cfg_write_control.recirc_oq : " << oq_map.oqueue(ii)<<endl);
                  }
                }
             }
          }
       }
       pbc_csr.cfg_parser0.oq_map(oq_map.all());
       pbc_csr.cfg_parser0.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.cfg_parser0.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {oq_map.show();}

    cfg_found_cnt = pbc_csr.port_0.cfg_account_tc_to_pg.load_from_cfg_rtn_status(0);
    pg8_map.all(pbc_csr.port_0.cfg_account_tc_to_pg.table()); // init
    pg8_map.set_name(pbc_csr.port_0.cfg_account_tc_to_pg.get_hier_path()+".decoder");
    cfg_found_cnt += pg8_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.port_0.cfg_account_tc_to_pg.table(pg8_map.all());
       pbc_csr.port_0.cfg_account_tc_to_pg.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_0.cfg_account_tc_to_pg.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pg8_map.show();}
    cfg_found_cnt = pbc_csr.cfg_parser1.load_from_cfg_rtn_status(0);
    oq_map.all(pbc_csr.cfg_parser1.oq_map()); // init
    oq_map.set_name(pbc_csr.cfg_parser1.get_hier_path()+".decoder");
    cfg_found_cnt +=oq_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       if (pbc_csr.cfg_parser1.default_port()  == E_CAP_PB_PORT_P4_RWR || pbc_csr.cfg_parser1.default_port()  == E_CAP_PB_PORT_P4_PCK) {
          spath = oq_map.get_hier_path()+".fix_oqueue";
          std::replace( spath.begin(), spath.end(), '.', '/');
          //if (sknobs_exists((char *)spath.c_str())) {
          unsigned int fix_oqueue = sknobs_get_value((char *)spath.c_str(), 0);
          //}
          for (int ii=0; ii<8; ii++) {
             if ((oq_map.oqueue(ii) > 29) && pbc_csr.cfg_parser1.default_port()  == E_CAP_PB_PORT_P4_RWR) {
               if (fix_oqueue == 1) {
                  oq_map.oqueue(oq_map.oqueue(ii)-2,ii);
               } else {
                  PLOG_ERR("cap_pb_load_from_cfg : Programming error: cfg_parser1.default_port == E_CAP_PB_PORT_P4_RWR and oq_map["<<ii<<"] is > 29 : " << oq_map.oqueue(ii)<<endl);
               }
             }
             if (pbc_csr.port_11.cfg_write_control.recirc_enable().convert_to<uint32_t>() ==1) {
                if ((oq_map.oqueue(ii) == pbc_csr.port_11.cfg_write_control.recirc_oq().convert_to<uint32_t>()) && pbc_csr.cfg_parser1.default_port()  == 11) {
                  //PLOG_MSG("Came here for 11 "<<ii<< " oqueue="<<oq_map.oqueue(ii)<<" recirc_oq=" <<pbc_csr.port_11.cfg_write_control.recirc_oq().convert_to<uint32_t>() <<endl);
                  if (fix_oqueue == 1) {
                     if (oq_map.oqueue(ii) == 0) {
                        oq_map.oqueue(oq_map.oqueue(ii)+1,ii);
                     } else {
                        oq_map.oqueue(oq_map.oqueue(ii)-1,ii);
                     }
                  } else {
                     PLOG_ERR("cap_pb_load_from_cfg : Programming error: cfg_parser1.default_port == 11 and oq_map["<<ii<<"] == port_11.cfg_write_control.recirc_oq : " << oq_map.oqueue(ii)<<endl);
                  }
                }
             }
             if (pbc_csr.port_10.cfg_write_control.recirc_enable().convert_to<uint32_t>() ==1) {
                if ((oq_map.oqueue(ii) == pbc_csr.port_10.cfg_write_control.recirc_oq().convert_to<uint32_t>()) && pbc_csr.cfg_parser1.default_port()  == 10) {
                  //PLOG_MSG("Came here for 10 "<<ii<< " oqueue="<<oq_map.oqueue(ii)<<" recirc_oq=" <<pbc_csr.port_10.cfg_write_control.recirc_oq().convert_to<uint32_t>() <<endl);
                  if (fix_oqueue == 1) {
                     if (oq_map.oqueue(ii) == 0) {
                        oq_map.oqueue(oq_map.oqueue(ii)+1,ii);
                     } else {
                        oq_map.oqueue(oq_map.oqueue(ii)-1,ii);
                     }
                  } else {
                     PLOG_ERR("cap_pb_load_from_cfg : Programming error: cfg_parser1.default_port == 10 and oq_map["<<ii<<"] == port_10.cfg_write_control.recirc_oq : " << oq_map.oqueue(ii)<<endl);
                  }
                }
             }
          }
       }
       pbc_csr.cfg_parser1.oq_map(oq_map.all());
       pbc_csr.cfg_parser1.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.cfg_parser1.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {oq_map.show();}

    cfg_found_cnt = pbc_csr.port_1.cfg_account_tc_to_pg.load_from_cfg_rtn_status(0);
    pg8_map.all(pbc_csr.port_1.cfg_account_tc_to_pg.table()); // init
    pg8_map.set_name(pbc_csr.port_1.cfg_account_tc_to_pg.get_hier_path()+".decoder");
    cfg_found_cnt += pg8_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.port_1.cfg_account_tc_to_pg.table(pg8_map.all());
       pbc_csr.port_1.cfg_account_tc_to_pg.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_1.cfg_account_tc_to_pg.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pg8_map.show();}
    cfg_found_cnt = pbc_csr.cfg_parser2.load_from_cfg_rtn_status(0);
    oq_map.all(pbc_csr.cfg_parser2.oq_map()); // init
    oq_map.set_name(pbc_csr.cfg_parser2.get_hier_path()+".decoder");
    cfg_found_cnt +=oq_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       if (pbc_csr.cfg_parser2.default_port()  == E_CAP_PB_PORT_P4_RWR || pbc_csr.cfg_parser2.default_port()  == E_CAP_PB_PORT_P4_PCK) {
          spath = oq_map.get_hier_path()+".fix_oqueue";
          std::replace( spath.begin(), spath.end(), '.', '/');
          //if (sknobs_exists((char *)spath.c_str())) {
          unsigned int fix_oqueue = sknobs_get_value((char *)spath.c_str(), 0);
          //}
          for (int ii=0; ii<8; ii++) {
             if ((oq_map.oqueue(ii) > 29) && pbc_csr.cfg_parser2.default_port()  == E_CAP_PB_PORT_P4_RWR) {
               if (fix_oqueue == 1) {
                  oq_map.oqueue(oq_map.oqueue(ii)-2,ii);
               } else {
                  PLOG_ERR("cap_pb_load_from_cfg : Programming error: cfg_parser2.default_port == E_CAP_PB_PORT_P4_RWR and oq_map["<<ii<<"] is > 29 : " << oq_map.oqueue(ii)<<endl);
               }
             }
             if (pbc_csr.port_11.cfg_write_control.recirc_enable().convert_to<uint32_t>() ==1) {
                if ((oq_map.oqueue(ii) == pbc_csr.port_11.cfg_write_control.recirc_oq().convert_to<uint32_t>()) && pbc_csr.cfg_parser2.default_port()  == 11) {
                  //PLOG_MSG("Came here for 11 "<<ii<< " oqueue="<<oq_map.oqueue(ii)<<" recirc_oq=" <<pbc_csr.port_11.cfg_write_control.recirc_oq().convert_to<uint32_t>() <<endl);
                  if (fix_oqueue == 1) {
                     if (oq_map.oqueue(ii) == 0) {
                        oq_map.oqueue(oq_map.oqueue(ii)+1,ii);
                     } else {
                        oq_map.oqueue(oq_map.oqueue(ii)-1,ii);
                     }
                  } else {
                     PLOG_ERR("cap_pb_load_from_cfg : Programming error: cfg_parser2.default_port == 11 and oq_map["<<ii<<"] == port_11.cfg_write_control.recirc_oq : " << oq_map.oqueue(ii)<<endl);
                  }
                }
             }
             if (pbc_csr.port_10.cfg_write_control.recirc_enable().convert_to<uint32_t>() ==1) {
                if ((oq_map.oqueue(ii) == pbc_csr.port_10.cfg_write_control.recirc_oq().convert_to<uint32_t>()) && pbc_csr.cfg_parser2.default_port()  == 10) {
                  //PLOG_MSG("Came here for 10 "<<ii<< " oqueue="<<oq_map.oqueue(ii)<<" recirc_oq=" <<pbc_csr.port_10.cfg_write_control.recirc_oq().convert_to<uint32_t>() <<endl);
                  if (fix_oqueue == 1) {
                     if (oq_map.oqueue(ii) == 0) {
                        oq_map.oqueue(oq_map.oqueue(ii)+1,ii);
                     } else {
                        oq_map.oqueue(oq_map.oqueue(ii)-1,ii);
                     }
                  } else {
                     PLOG_ERR("cap_pb_load_from_cfg : Programming error: cfg_parser2.default_port == 10 and oq_map["<<ii<<"] == port_10.cfg_write_control.recirc_oq : " << oq_map.oqueue(ii)<<endl);
                  }
                }
             }
          }
       }
       pbc_csr.cfg_parser2.oq_map(oq_map.all());
       pbc_csr.cfg_parser2.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.cfg_parser2.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {oq_map.show();}

    cfg_found_cnt = pbc_csr.port_2.cfg_account_tc_to_pg.load_from_cfg_rtn_status(0);
    pg8_map.all(pbc_csr.port_2.cfg_account_tc_to_pg.table()); // init
    pg8_map.set_name(pbc_csr.port_2.cfg_account_tc_to_pg.get_hier_path()+".decoder");
    cfg_found_cnt += pg8_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.port_2.cfg_account_tc_to_pg.table(pg8_map.all());
       pbc_csr.port_2.cfg_account_tc_to_pg.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_2.cfg_account_tc_to_pg.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pg8_map.show();}
    cfg_found_cnt = pbc_csr.cfg_parser3.load_from_cfg_rtn_status(0);
    oq_map.all(pbc_csr.cfg_parser3.oq_map()); // init
    oq_map.set_name(pbc_csr.cfg_parser3.get_hier_path()+".decoder");
    cfg_found_cnt +=oq_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       if (pbc_csr.cfg_parser3.default_port()  == E_CAP_PB_PORT_P4_RWR || pbc_csr.cfg_parser3.default_port()  == E_CAP_PB_PORT_P4_PCK) {
          spath = oq_map.get_hier_path()+".fix_oqueue";
          std::replace( spath.begin(), spath.end(), '.', '/');
          //if (sknobs_exists((char *)spath.c_str())) {
          unsigned int fix_oqueue = sknobs_get_value((char *)spath.c_str(), 0);
          //}
          for (int ii=0; ii<8; ii++) {
             if ((oq_map.oqueue(ii) > 29) && pbc_csr.cfg_parser3.default_port()  == E_CAP_PB_PORT_P4_RWR) {
               if (fix_oqueue == 1) {
                  oq_map.oqueue(oq_map.oqueue(ii)-2,ii);
               } else {
                  PLOG_ERR("cap_pb_load_from_cfg : Programming error: cfg_parser3.default_port == E_CAP_PB_PORT_P4_RWR and oq_map["<<ii<<"] is > 29 : " << oq_map.oqueue(ii)<<endl);
               }
             }
             if (pbc_csr.port_11.cfg_write_control.recirc_enable().convert_to<uint32_t>() ==1) {
                if ((oq_map.oqueue(ii) == pbc_csr.port_11.cfg_write_control.recirc_oq().convert_to<uint32_t>()) && pbc_csr.cfg_parser3.default_port()  == 11) {
                  //PLOG_MSG("Came here for 11 "<<ii<< " oqueue="<<oq_map.oqueue(ii)<<" recirc_oq=" <<pbc_csr.port_11.cfg_write_control.recirc_oq().convert_to<uint32_t>() <<endl);
                  if (fix_oqueue == 1) {
                     if (oq_map.oqueue(ii) == 0) {
                        oq_map.oqueue(oq_map.oqueue(ii)+1,ii);
                     } else {
                        oq_map.oqueue(oq_map.oqueue(ii)-1,ii);
                     }
                  } else {
                     PLOG_ERR("cap_pb_load_from_cfg : Programming error: cfg_parser3.default_port == 11 and oq_map["<<ii<<"] == port_11.cfg_write_control.recirc_oq : " << oq_map.oqueue(ii)<<endl);
                  }
                }
             }
             if (pbc_csr.port_10.cfg_write_control.recirc_enable().convert_to<uint32_t>() ==1) {
                if ((oq_map.oqueue(ii) == pbc_csr.port_10.cfg_write_control.recirc_oq().convert_to<uint32_t>()) && pbc_csr.cfg_parser3.default_port()  == 10) {
                  //PLOG_MSG("Came here for 10 "<<ii<< " oqueue="<<oq_map.oqueue(ii)<<" recirc_oq=" <<pbc_csr.port_10.cfg_write_control.recirc_oq().convert_to<uint32_t>() <<endl);
                  if (fix_oqueue == 1) {
                     if (oq_map.oqueue(ii) == 0) {
                        oq_map.oqueue(oq_map.oqueue(ii)+1,ii);
                     } else {
                        oq_map.oqueue(oq_map.oqueue(ii)-1,ii);
                     }
                  } else {
                     PLOG_ERR("cap_pb_load_from_cfg : Programming error: cfg_parser3.default_port == 10 and oq_map["<<ii<<"] == port_10.cfg_write_control.recirc_oq : " << oq_map.oqueue(ii)<<endl);
                  }
                }
             }
          }
       }
       pbc_csr.cfg_parser3.oq_map(oq_map.all());
       pbc_csr.cfg_parser3.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.cfg_parser3.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {oq_map.show();}

    cfg_found_cnt = pbc_csr.port_3.cfg_account_tc_to_pg.load_from_cfg_rtn_status(0);
    pg8_map.all(pbc_csr.port_3.cfg_account_tc_to_pg.table()); // init
    pg8_map.set_name(pbc_csr.port_3.cfg_account_tc_to_pg.get_hier_path()+".decoder");
    cfg_found_cnt += pg8_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.port_3.cfg_account_tc_to_pg.table(pg8_map.all());
       pbc_csr.port_3.cfg_account_tc_to_pg.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_3.cfg_account_tc_to_pg.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pg8_map.show();}
    cfg_found_cnt = pbc_csr.cfg_parser4.load_from_cfg_rtn_status(0);
    oq_map.all(pbc_csr.cfg_parser4.oq_map()); // init
    oq_map.set_name(pbc_csr.cfg_parser4.get_hier_path()+".decoder");
    cfg_found_cnt +=oq_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       if (pbc_csr.cfg_parser4.default_port()  == E_CAP_PB_PORT_P4_RWR || pbc_csr.cfg_parser4.default_port()  == E_CAP_PB_PORT_P4_PCK) {
          spath = oq_map.get_hier_path()+".fix_oqueue";
          std::replace( spath.begin(), spath.end(), '.', '/');
          //if (sknobs_exists((char *)spath.c_str())) {
          unsigned int fix_oqueue = sknobs_get_value((char *)spath.c_str(), 0);
          //}
          for (int ii=0; ii<8; ii++) {
             if ((oq_map.oqueue(ii) > 29) && pbc_csr.cfg_parser4.default_port()  == E_CAP_PB_PORT_P4_RWR) {
               if (fix_oqueue == 1) {
                  oq_map.oqueue(oq_map.oqueue(ii)-2,ii);
               } else {
                  PLOG_ERR("cap_pb_load_from_cfg : Programming error: cfg_parser4.default_port == E_CAP_PB_PORT_P4_RWR and oq_map["<<ii<<"] is > 29 : " << oq_map.oqueue(ii)<<endl);
               }
             }
             if (pbc_csr.port_11.cfg_write_control.recirc_enable().convert_to<uint32_t>() ==1) {
                if ((oq_map.oqueue(ii) == pbc_csr.port_11.cfg_write_control.recirc_oq().convert_to<uint32_t>()) && pbc_csr.cfg_parser4.default_port()  == 11) {
                  //PLOG_MSG("Came here for 11 "<<ii<< " oqueue="<<oq_map.oqueue(ii)<<" recirc_oq=" <<pbc_csr.port_11.cfg_write_control.recirc_oq().convert_to<uint32_t>() <<endl);
                  if (fix_oqueue == 1) {
                     if (oq_map.oqueue(ii) == 0) {
                        oq_map.oqueue(oq_map.oqueue(ii)+1,ii);
                     } else {
                        oq_map.oqueue(oq_map.oqueue(ii)-1,ii);
                     }
                  } else {
                     PLOG_ERR("cap_pb_load_from_cfg : Programming error: cfg_parser4.default_port == 11 and oq_map["<<ii<<"] == port_11.cfg_write_control.recirc_oq : " << oq_map.oqueue(ii)<<endl);
                  }
                }
             }
             if (pbc_csr.port_10.cfg_write_control.recirc_enable().convert_to<uint32_t>() ==1) {
                if ((oq_map.oqueue(ii) == pbc_csr.port_10.cfg_write_control.recirc_oq().convert_to<uint32_t>()) && pbc_csr.cfg_parser4.default_port()  == 10) {
                  //PLOG_MSG("Came here for 10 "<<ii<< " oqueue="<<oq_map.oqueue(ii)<<" recirc_oq=" <<pbc_csr.port_10.cfg_write_control.recirc_oq().convert_to<uint32_t>() <<endl);
                  if (fix_oqueue == 1) {
                     if (oq_map.oqueue(ii) == 0) {
                        oq_map.oqueue(oq_map.oqueue(ii)+1,ii);
                     } else {
                        oq_map.oqueue(oq_map.oqueue(ii)-1,ii);
                     }
                  } else {
                     PLOG_ERR("cap_pb_load_from_cfg : Programming error: cfg_parser4.default_port == 10 and oq_map["<<ii<<"] == port_10.cfg_write_control.recirc_oq : " << oq_map.oqueue(ii)<<endl);
                  }
                }
             }
          }
       }
       pbc_csr.cfg_parser4.oq_map(oq_map.all());
       pbc_csr.cfg_parser4.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.cfg_parser4.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {oq_map.show();}

    cfg_found_cnt = pbc_csr.port_4.cfg_account_tc_to_pg.load_from_cfg_rtn_status(0);
    pg8_map.all(pbc_csr.port_4.cfg_account_tc_to_pg.table()); // init
    pg8_map.set_name(pbc_csr.port_4.cfg_account_tc_to_pg.get_hier_path()+".decoder");
    cfg_found_cnt += pg8_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.port_4.cfg_account_tc_to_pg.table(pg8_map.all());
       pbc_csr.port_4.cfg_account_tc_to_pg.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_4.cfg_account_tc_to_pg.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pg8_map.show();}
    cfg_found_cnt = pbc_csr.cfg_parser5.load_from_cfg_rtn_status(0);
    oq_map.all(pbc_csr.cfg_parser5.oq_map()); // init
    oq_map.set_name(pbc_csr.cfg_parser5.get_hier_path()+".decoder");
    cfg_found_cnt +=oq_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       if (pbc_csr.cfg_parser5.default_port()  == E_CAP_PB_PORT_P4_RWR || pbc_csr.cfg_parser5.default_port()  == E_CAP_PB_PORT_P4_PCK) {
          spath = oq_map.get_hier_path()+".fix_oqueue";
          std::replace( spath.begin(), spath.end(), '.', '/');
          //if (sknobs_exists((char *)spath.c_str())) {
          unsigned int fix_oqueue = sknobs_get_value((char *)spath.c_str(), 0);
          //}
          for (int ii=0; ii<8; ii++) {
             if ((oq_map.oqueue(ii) > 29) && pbc_csr.cfg_parser5.default_port()  == E_CAP_PB_PORT_P4_RWR) {
               if (fix_oqueue == 1) {
                  oq_map.oqueue(oq_map.oqueue(ii)-2,ii);
               } else {
                  PLOG_ERR("cap_pb_load_from_cfg : Programming error: cfg_parser5.default_port == E_CAP_PB_PORT_P4_RWR and oq_map["<<ii<<"] is > 29 : " << oq_map.oqueue(ii)<<endl);
               }
             }
             if (pbc_csr.port_11.cfg_write_control.recirc_enable().convert_to<uint32_t>() ==1) {
                if ((oq_map.oqueue(ii) == pbc_csr.port_11.cfg_write_control.recirc_oq().convert_to<uint32_t>()) && pbc_csr.cfg_parser5.default_port()  == 11) {
                  //PLOG_MSG("Came here for 11 "<<ii<< " oqueue="<<oq_map.oqueue(ii)<<" recirc_oq=" <<pbc_csr.port_11.cfg_write_control.recirc_oq().convert_to<uint32_t>() <<endl);
                  if (fix_oqueue == 1) {
                     if (oq_map.oqueue(ii) == 0) {
                        oq_map.oqueue(oq_map.oqueue(ii)+1,ii);
                     } else {
                        oq_map.oqueue(oq_map.oqueue(ii)-1,ii);
                     }
                  } else {
                     PLOG_ERR("cap_pb_load_from_cfg : Programming error: cfg_parser5.default_port == 11 and oq_map["<<ii<<"] == port_11.cfg_write_control.recirc_oq : " << oq_map.oqueue(ii)<<endl);
                  }
                }
             }
             if (pbc_csr.port_10.cfg_write_control.recirc_enable().convert_to<uint32_t>() ==1) {
                if ((oq_map.oqueue(ii) == pbc_csr.port_10.cfg_write_control.recirc_oq().convert_to<uint32_t>()) && pbc_csr.cfg_parser5.default_port()  == 10) {
                  //PLOG_MSG("Came here for 10 "<<ii<< " oqueue="<<oq_map.oqueue(ii)<<" recirc_oq=" <<pbc_csr.port_10.cfg_write_control.recirc_oq().convert_to<uint32_t>() <<endl);
                  if (fix_oqueue == 1) {
                     if (oq_map.oqueue(ii) == 0) {
                        oq_map.oqueue(oq_map.oqueue(ii)+1,ii);
                     } else {
                        oq_map.oqueue(oq_map.oqueue(ii)-1,ii);
                     }
                  } else {
                     PLOG_ERR("cap_pb_load_from_cfg : Programming error: cfg_parser5.default_port == 10 and oq_map["<<ii<<"] == port_10.cfg_write_control.recirc_oq : " << oq_map.oqueue(ii)<<endl);
                  }
                }
             }
          }
       }
       pbc_csr.cfg_parser5.oq_map(oq_map.all());
       pbc_csr.cfg_parser5.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.cfg_parser5.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {oq_map.show();}

    cfg_found_cnt = pbc_csr.port_5.cfg_account_tc_to_pg.load_from_cfg_rtn_status(0);
    pg8_map.all(pbc_csr.port_5.cfg_account_tc_to_pg.table()); // init
    pg8_map.set_name(pbc_csr.port_5.cfg_account_tc_to_pg.get_hier_path()+".decoder");
    cfg_found_cnt += pg8_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.port_5.cfg_account_tc_to_pg.table(pg8_map.all());
       pbc_csr.port_5.cfg_account_tc_to_pg.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_5.cfg_account_tc_to_pg.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pg8_map.show();}
    cfg_found_cnt = pbc_csr.cfg_parser6.load_from_cfg_rtn_status(0);
    oq_map.all(pbc_csr.cfg_parser6.oq_map()); // init
    oq_map.set_name(pbc_csr.cfg_parser6.get_hier_path()+".decoder");
    cfg_found_cnt +=oq_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       if (pbc_csr.cfg_parser6.default_port()  == E_CAP_PB_PORT_P4_RWR || pbc_csr.cfg_parser6.default_port()  == E_CAP_PB_PORT_P4_PCK) {
          spath = oq_map.get_hier_path()+".fix_oqueue";
          std::replace( spath.begin(), spath.end(), '.', '/');
          //if (sknobs_exists((char *)spath.c_str())) {
          unsigned int fix_oqueue = sknobs_get_value((char *)spath.c_str(), 0);
          //}
          for (int ii=0; ii<8; ii++) {
             if ((oq_map.oqueue(ii) > 29) && pbc_csr.cfg_parser6.default_port()  == E_CAP_PB_PORT_P4_RWR) {
               if (fix_oqueue == 1) {
                  oq_map.oqueue(oq_map.oqueue(ii)-2,ii);
               } else {
                  PLOG_ERR("cap_pb_load_from_cfg : Programming error: cfg_parser6.default_port == E_CAP_PB_PORT_P4_RWR and oq_map["<<ii<<"] is > 29 : " << oq_map.oqueue(ii)<<endl);
               }
             }
             if (pbc_csr.port_11.cfg_write_control.recirc_enable().convert_to<uint32_t>() ==1) {
                if ((oq_map.oqueue(ii) == pbc_csr.port_11.cfg_write_control.recirc_oq().convert_to<uint32_t>()) && pbc_csr.cfg_parser6.default_port()  == 11) {
                  //PLOG_MSG("Came here for 11 "<<ii<< " oqueue="<<oq_map.oqueue(ii)<<" recirc_oq=" <<pbc_csr.port_11.cfg_write_control.recirc_oq().convert_to<uint32_t>() <<endl);
                  if (fix_oqueue == 1) {
                     if (oq_map.oqueue(ii) == 0) {
                        oq_map.oqueue(oq_map.oqueue(ii)+1,ii);
                     } else {
                        oq_map.oqueue(oq_map.oqueue(ii)-1,ii);
                     }
                  } else {
                     PLOG_ERR("cap_pb_load_from_cfg : Programming error: cfg_parser6.default_port == 11 and oq_map["<<ii<<"] == port_11.cfg_write_control.recirc_oq : " << oq_map.oqueue(ii)<<endl);
                  }
                }
             }
             if (pbc_csr.port_10.cfg_write_control.recirc_enable().convert_to<uint32_t>() ==1) {
                if ((oq_map.oqueue(ii) == pbc_csr.port_10.cfg_write_control.recirc_oq().convert_to<uint32_t>()) && pbc_csr.cfg_parser6.default_port()  == 10) {
                  //PLOG_MSG("Came here for 10 "<<ii<< " oqueue="<<oq_map.oqueue(ii)<<" recirc_oq=" <<pbc_csr.port_10.cfg_write_control.recirc_oq().convert_to<uint32_t>() <<endl);
                  if (fix_oqueue == 1) {
                     if (oq_map.oqueue(ii) == 0) {
                        oq_map.oqueue(oq_map.oqueue(ii)+1,ii);
                     } else {
                        oq_map.oqueue(oq_map.oqueue(ii)-1,ii);
                     }
                  } else {
                     PLOG_ERR("cap_pb_load_from_cfg : Programming error: cfg_parser6.default_port == 10 and oq_map["<<ii<<"] == port_10.cfg_write_control.recirc_oq : " << oq_map.oqueue(ii)<<endl);
                  }
                }
             }
          }
       }
       pbc_csr.cfg_parser6.oq_map(oq_map.all());
       pbc_csr.cfg_parser6.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.cfg_parser6.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {oq_map.show();}

    cfg_found_cnt = pbc_csr.port_6.cfg_account_tc_to_pg.load_from_cfg_rtn_status(0);
    pg8_map.all(pbc_csr.port_6.cfg_account_tc_to_pg.table()); // init
    pg8_map.set_name(pbc_csr.port_6.cfg_account_tc_to_pg.get_hier_path()+".decoder");
    cfg_found_cnt += pg8_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.port_6.cfg_account_tc_to_pg.table(pg8_map.all());
       pbc_csr.port_6.cfg_account_tc_to_pg.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_6.cfg_account_tc_to_pg.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pg8_map.show();}
    cfg_found_cnt = pbc_csr.cfg_parser7.load_from_cfg_rtn_status(0);
    oq_map.all(pbc_csr.cfg_parser7.oq_map()); // init
    oq_map.set_name(pbc_csr.cfg_parser7.get_hier_path()+".decoder");
    cfg_found_cnt +=oq_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       if (pbc_csr.cfg_parser7.default_port()  == E_CAP_PB_PORT_P4_RWR || pbc_csr.cfg_parser7.default_port()  == E_CAP_PB_PORT_P4_PCK) {
          spath = oq_map.get_hier_path()+".fix_oqueue";
          std::replace( spath.begin(), spath.end(), '.', '/');
          //if (sknobs_exists((char *)spath.c_str())) {
          unsigned int fix_oqueue = sknobs_get_value((char *)spath.c_str(), 0);
          //}
          for (int ii=0; ii<8; ii++) {
             if ((oq_map.oqueue(ii) > 29) && pbc_csr.cfg_parser7.default_port()  == E_CAP_PB_PORT_P4_RWR) {
               if (fix_oqueue == 1) {
                  oq_map.oqueue(oq_map.oqueue(ii)-2,ii);
               } else {
                  PLOG_ERR("cap_pb_load_from_cfg : Programming error: cfg_parser7.default_port == E_CAP_PB_PORT_P4_RWR and oq_map["<<ii<<"] is > 29 : " << oq_map.oqueue(ii)<<endl);
               }
             }
             if (pbc_csr.port_11.cfg_write_control.recirc_enable().convert_to<uint32_t>() ==1) {
                if ((oq_map.oqueue(ii) == pbc_csr.port_11.cfg_write_control.recirc_oq().convert_to<uint32_t>()) && pbc_csr.cfg_parser7.default_port()  == 11) {
                  //PLOG_MSG("Came here for 11 "<<ii<< " oqueue="<<oq_map.oqueue(ii)<<" recirc_oq=" <<pbc_csr.port_11.cfg_write_control.recirc_oq().convert_to<uint32_t>() <<endl);
                  if (fix_oqueue == 1) {
                     if (oq_map.oqueue(ii) == 0) {
                        oq_map.oqueue(oq_map.oqueue(ii)+1,ii);
                     } else {
                        oq_map.oqueue(oq_map.oqueue(ii)-1,ii);
                     }
                  } else {
                     PLOG_ERR("cap_pb_load_from_cfg : Programming error: cfg_parser7.default_port == 11 and oq_map["<<ii<<"] == port_11.cfg_write_control.recirc_oq : " << oq_map.oqueue(ii)<<endl);
                  }
                }
             }
             if (pbc_csr.port_10.cfg_write_control.recirc_enable().convert_to<uint32_t>() ==1) {
                if ((oq_map.oqueue(ii) == pbc_csr.port_10.cfg_write_control.recirc_oq().convert_to<uint32_t>()) && pbc_csr.cfg_parser7.default_port()  == 10) {
                  //PLOG_MSG("Came here for 10 "<<ii<< " oqueue="<<oq_map.oqueue(ii)<<" recirc_oq=" <<pbc_csr.port_10.cfg_write_control.recirc_oq().convert_to<uint32_t>() <<endl);
                  if (fix_oqueue == 1) {
                     if (oq_map.oqueue(ii) == 0) {
                        oq_map.oqueue(oq_map.oqueue(ii)+1,ii);
                     } else {
                        oq_map.oqueue(oq_map.oqueue(ii)-1,ii);
                     }
                  } else {
                     PLOG_ERR("cap_pb_load_from_cfg : Programming error: cfg_parser7.default_port == 10 and oq_map["<<ii<<"] == port_10.cfg_write_control.recirc_oq : " << oq_map.oqueue(ii)<<endl);
                  }
                }
             }
          }
       }
       pbc_csr.cfg_parser7.oq_map(oq_map.all());
       pbc_csr.cfg_parser7.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.cfg_parser7.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {oq_map.show();}

    cfg_found_cnt = pbc_csr.port_7.cfg_account_tc_to_pg.load_from_cfg_rtn_status(0);
    pg8_map.all(pbc_csr.port_7.cfg_account_tc_to_pg.table()); // init
    pg8_map.set_name(pbc_csr.port_7.cfg_account_tc_to_pg.get_hier_path()+".decoder");
    cfg_found_cnt += pg8_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.port_7.cfg_account_tc_to_pg.table(pg8_map.all());
       pbc_csr.port_7.cfg_account_tc_to_pg.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_7.cfg_account_tc_to_pg.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pg8_map.show();}
    cfg_found_cnt = pbc_csr.cfg_parser8.load_from_cfg_rtn_status(0);
    oq_map.all(pbc_csr.cfg_parser8.oq_map()); // init
    oq_map.set_name(pbc_csr.cfg_parser8.get_hier_path()+".decoder");
    cfg_found_cnt +=oq_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       if (pbc_csr.cfg_parser8.default_port()  == E_CAP_PB_PORT_P4_RWR || pbc_csr.cfg_parser8.default_port()  == E_CAP_PB_PORT_P4_PCK) {
          spath = oq_map.get_hier_path()+".fix_oqueue";
          std::replace( spath.begin(), spath.end(), '.', '/');
          //if (sknobs_exists((char *)spath.c_str())) {
          unsigned int fix_oqueue = sknobs_get_value((char *)spath.c_str(), 0);
          //}
          for (int ii=0; ii<8; ii++) {
             if ((oq_map.oqueue(ii) > 29) && pbc_csr.cfg_parser8.default_port()  == E_CAP_PB_PORT_P4_RWR) {
               if (fix_oqueue == 1) {
                  oq_map.oqueue(oq_map.oqueue(ii)-2,ii);
               } else {
                  PLOG_ERR("cap_pb_load_from_cfg : Programming error: cfg_parser8.default_port == E_CAP_PB_PORT_P4_RWR and oq_map["<<ii<<"] is > 29 : " << oq_map.oqueue(ii)<<endl);
               }
             }
             if (pbc_csr.port_11.cfg_write_control.recirc_enable().convert_to<uint32_t>() ==1) {
                if ((oq_map.oqueue(ii) == pbc_csr.port_11.cfg_write_control.recirc_oq().convert_to<uint32_t>()) && pbc_csr.cfg_parser8.default_port()  == 11) {
                  //PLOG_MSG("Came here for 11 "<<ii<< " oqueue="<<oq_map.oqueue(ii)<<" recirc_oq=" <<pbc_csr.port_11.cfg_write_control.recirc_oq().convert_to<uint32_t>() <<endl);
                  if (fix_oqueue == 1) {
                     if (oq_map.oqueue(ii) == 0) {
                        oq_map.oqueue(oq_map.oqueue(ii)+1,ii);
                     } else {
                        oq_map.oqueue(oq_map.oqueue(ii)-1,ii);
                     }
                  } else {
                     PLOG_ERR("cap_pb_load_from_cfg : Programming error: cfg_parser8.default_port == 11 and oq_map["<<ii<<"] == port_11.cfg_write_control.recirc_oq : " << oq_map.oqueue(ii)<<endl);
                  }
                }
             }
             if (pbc_csr.port_10.cfg_write_control.recirc_enable().convert_to<uint32_t>() ==1) {
                if ((oq_map.oqueue(ii) == pbc_csr.port_10.cfg_write_control.recirc_oq().convert_to<uint32_t>()) && pbc_csr.cfg_parser8.default_port()  == 10) {
                  //PLOG_MSG("Came here for 10 "<<ii<< " oqueue="<<oq_map.oqueue(ii)<<" recirc_oq=" <<pbc_csr.port_10.cfg_write_control.recirc_oq().convert_to<uint32_t>() <<endl);
                  if (fix_oqueue == 1) {
                     if (oq_map.oqueue(ii) == 0) {
                        oq_map.oqueue(oq_map.oqueue(ii)+1,ii);
                     } else {
                        oq_map.oqueue(oq_map.oqueue(ii)-1,ii);
                     }
                  } else {
                     PLOG_ERR("cap_pb_load_from_cfg : Programming error: cfg_parser8.default_port == 10 and oq_map["<<ii<<"] == port_10.cfg_write_control.recirc_oq : " << oq_map.oqueue(ii)<<endl);
                  }
                }
             }
          }
       }
       pbc_csr.cfg_parser8.oq_map(oq_map.all());
       pbc_csr.cfg_parser8.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.cfg_parser8.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {oq_map.show();}

    cfg_found_cnt = pbc_csr.port_8.cfg_account_tc_to_pg.load_from_cfg_rtn_status(0);
    pg8_map.all(pbc_csr.port_8.cfg_account_tc_to_pg.table()); // init
    pg8_map.set_name(pbc_csr.port_8.cfg_account_tc_to_pg.get_hier_path()+".decoder");
    cfg_found_cnt += pg8_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.port_8.cfg_account_tc_to_pg.table(pg8_map.all());
       pbc_csr.port_8.cfg_account_tc_to_pg.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_8.cfg_account_tc_to_pg.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pg8_map.show();}
    // p4 / p4+
    cfg_found_cnt = pbc_csr.port_9.cfg_account_tc_to_pg.load_from_cfg_rtn_status(0);
    pg16_map.all(pbc_csr.port_9.cfg_account_tc_to_pg.table()); // init
    pg16_map.set_name(pbc_csr.port_9.cfg_account_tc_to_pg.get_hier_path()+".decoder");
    cfg_found_cnt += pg16_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.port_9.cfg_account_tc_to_pg.table(pg16_map.all());
       pbc_csr.port_9.cfg_account_tc_to_pg.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_tc_to_pg.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pg16_map.show();}

    cfg_found_cnt = pbc_csr.port_10.cfg_account_tc_to_pg.load_from_cfg_rtn_status(0);
    pg32_map.all(pbc_csr.port_10.cfg_account_tc_to_pg.table()); // init
    pg32_map.set_name(pbc_csr.port_10.cfg_account_tc_to_pg.get_hier_path()+".decoder");
    cfg_found_cnt += pg32_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.port_10.cfg_account_tc_to_pg.table(pg32_map.all());
       pbc_csr.port_10.cfg_account_tc_to_pg.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_tc_to_pg.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pg32_map.show();}

    cfg_found_cnt = pbc_csr.port_11.cfg_account_tc_to_pg.load_from_cfg_rtn_status(0);
    pg32_map.all(pbc_csr.port_11.cfg_account_tc_to_pg.table()); // init
    pg32_map.set_name(pbc_csr.port_11.cfg_account_tc_to_pg.get_hier_path()+".decoder");
    cfg_found_cnt += pg32_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.port_11.cfg_account_tc_to_pg.table(pg32_map.all());
       pbc_csr.port_11.cfg_account_tc_to_pg.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_tc_to_pg.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pg32_map.show();}

   //enable write ports
   //set cut thru threshold to 8'h1f (basically store and forward)
   //set packing to 1 (only valid for eth ports)
   //set packing timeout cycles to 10
    // default
    if (pbc_csr.cfg_parser0.default_port() == E_CAP_PB_PORT_P4_PCK) {
       pbc_csr.port_0.cfg_write_control.packing(0);
       pbc_csr.port_0.cfg_write_control.packing_timeout_cycles(10);
    }
    pbc_csr.port_0.cfg_write_control.enable(1);
    pbc_csr.port_0.cfg_write_control.cut_thru(-1);
    pbc_csr.port_0.cfg_write_control.load_from_cfg(0);
    // overwride 
    if (pbc_csr.cfg_parser0.default_port() != E_CAP_PB_PORT_P4_PCK) {
       pbc_csr.port_0.cfg_write_control.packing(0);
    }
    if (pbc_csr.cfg_parser0.default_port() <= PB_PORT_ETH_LAST ) {
       pbc_csr.port_0.cfg_write_control.rewrite(0); // 
    } else {
       pbc_csr.port_0.cfg_write_control.rewrite(1); // enable this on eth ports when going to P4
    }
    pbc_csr.port_0.cfg_write_control.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_0.cfg_write_control.show();}
    //
    // default
    if (pbc_csr.cfg_parser1.default_port() == E_CAP_PB_PORT_P4_PCK) {
       pbc_csr.port_1.cfg_write_control.packing(0);
       pbc_csr.port_1.cfg_write_control.packing_timeout_cycles(10);
    }
    pbc_csr.port_1.cfg_write_control.enable(1);
    pbc_csr.port_1.cfg_write_control.cut_thru(-1);
    pbc_csr.port_1.cfg_write_control.load_from_cfg(0);
    // overwride 
    if (pbc_csr.cfg_parser1.default_port() != E_CAP_PB_PORT_P4_PCK) {
       pbc_csr.port_1.cfg_write_control.packing(0);
    }
    if (pbc_csr.cfg_parser1.default_port() <= PB_PORT_ETH_LAST ) {
       pbc_csr.port_1.cfg_write_control.rewrite(0); // 
    } else {
       pbc_csr.port_1.cfg_write_control.rewrite(1); // enable this on eth ports when going to P4
    }
    pbc_csr.port_1.cfg_write_control.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_1.cfg_write_control.show();}
    //
    // default
    if (pbc_csr.cfg_parser2.default_port() == E_CAP_PB_PORT_P4_PCK) {
       pbc_csr.port_2.cfg_write_control.packing(0);
       pbc_csr.port_2.cfg_write_control.packing_timeout_cycles(10);
    }
    pbc_csr.port_2.cfg_write_control.enable(1);
    pbc_csr.port_2.cfg_write_control.cut_thru(-1);
    pbc_csr.port_2.cfg_write_control.load_from_cfg(0);
    // overwride 
    if (pbc_csr.cfg_parser2.default_port() != E_CAP_PB_PORT_P4_PCK) {
       pbc_csr.port_2.cfg_write_control.packing(0);
    }
    if (pbc_csr.cfg_parser2.default_port() <= PB_PORT_ETH_LAST ) {
       pbc_csr.port_2.cfg_write_control.rewrite(0); // 
    } else {
       pbc_csr.port_2.cfg_write_control.rewrite(1); // enable this on eth ports when going to P4
    }
    pbc_csr.port_2.cfg_write_control.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_2.cfg_write_control.show();}
    //
    // default
    if (pbc_csr.cfg_parser3.default_port() == E_CAP_PB_PORT_P4_PCK) {
       pbc_csr.port_3.cfg_write_control.packing(0);
       pbc_csr.port_3.cfg_write_control.packing_timeout_cycles(10);
    }
    pbc_csr.port_3.cfg_write_control.enable(1);
    pbc_csr.port_3.cfg_write_control.cut_thru(-1);
    pbc_csr.port_3.cfg_write_control.load_from_cfg(0);
    // overwride 
    if (pbc_csr.cfg_parser3.default_port() != E_CAP_PB_PORT_P4_PCK) {
       pbc_csr.port_3.cfg_write_control.packing(0);
    }
    if (pbc_csr.cfg_parser3.default_port() <= PB_PORT_ETH_LAST ) {
       pbc_csr.port_3.cfg_write_control.rewrite(0); // 
    } else {
       pbc_csr.port_3.cfg_write_control.rewrite(1); // enable this on eth ports when going to P4
    }
    pbc_csr.port_3.cfg_write_control.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_3.cfg_write_control.show();}
    //
    // default
    if (pbc_csr.cfg_parser4.default_port() == E_CAP_PB_PORT_P4_PCK) {
       pbc_csr.port_4.cfg_write_control.packing(0);
       pbc_csr.port_4.cfg_write_control.packing_timeout_cycles(10);
    }
    pbc_csr.port_4.cfg_write_control.enable(1);
    pbc_csr.port_4.cfg_write_control.cut_thru(-1);
    pbc_csr.port_4.cfg_write_control.load_from_cfg(0);
    // overwride 
    if (pbc_csr.cfg_parser4.default_port() != E_CAP_PB_PORT_P4_PCK) {
       pbc_csr.port_4.cfg_write_control.packing(0);
    }
    if (pbc_csr.cfg_parser4.default_port() <= PB_PORT_ETH_LAST ) {
       pbc_csr.port_4.cfg_write_control.rewrite(0); // 
    } else {
       pbc_csr.port_4.cfg_write_control.rewrite(1); // enable this on eth ports when going to P4
    }
    pbc_csr.port_4.cfg_write_control.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_4.cfg_write_control.show();}
    //
    // default
    if (pbc_csr.cfg_parser5.default_port() == E_CAP_PB_PORT_P4_PCK) {
       pbc_csr.port_5.cfg_write_control.packing(0);
       pbc_csr.port_5.cfg_write_control.packing_timeout_cycles(10);
    }
    pbc_csr.port_5.cfg_write_control.enable(1);
    pbc_csr.port_5.cfg_write_control.cut_thru(-1);
    pbc_csr.port_5.cfg_write_control.load_from_cfg(0);
    // overwride 
    if (pbc_csr.cfg_parser5.default_port() != E_CAP_PB_PORT_P4_PCK) {
       pbc_csr.port_5.cfg_write_control.packing(0);
    }
    if (pbc_csr.cfg_parser5.default_port() <= PB_PORT_ETH_LAST ) {
       pbc_csr.port_5.cfg_write_control.rewrite(0); // 
    } else {
       pbc_csr.port_5.cfg_write_control.rewrite(1); // enable this on eth ports when going to P4
    }
    pbc_csr.port_5.cfg_write_control.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_5.cfg_write_control.show();}
    //
    // default
    if (pbc_csr.cfg_parser6.default_port() == E_CAP_PB_PORT_P4_PCK) {
       pbc_csr.port_6.cfg_write_control.packing(0);
       pbc_csr.port_6.cfg_write_control.packing_timeout_cycles(10);
    }
    pbc_csr.port_6.cfg_write_control.enable(1);
    pbc_csr.port_6.cfg_write_control.cut_thru(-1);
    pbc_csr.port_6.cfg_write_control.load_from_cfg(0);
    // overwride 
    if (pbc_csr.cfg_parser6.default_port() != E_CAP_PB_PORT_P4_PCK) {
       pbc_csr.port_6.cfg_write_control.packing(0);
    }
    if (pbc_csr.cfg_parser6.default_port() <= PB_PORT_ETH_LAST ) {
       pbc_csr.port_6.cfg_write_control.rewrite(0); // 
    } else {
       pbc_csr.port_6.cfg_write_control.rewrite(1); // enable this on eth ports when going to P4
    }
    pbc_csr.port_6.cfg_write_control.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_6.cfg_write_control.show();}
    //
    // default
    if (pbc_csr.cfg_parser7.default_port() == E_CAP_PB_PORT_P4_PCK) {
       pbc_csr.port_7.cfg_write_control.packing(0);
       pbc_csr.port_7.cfg_write_control.packing_timeout_cycles(10);
    }
    pbc_csr.port_7.cfg_write_control.enable(1);
    pbc_csr.port_7.cfg_write_control.cut_thru(-1);
    pbc_csr.port_7.cfg_write_control.load_from_cfg(0);
    // overwride 
    if (pbc_csr.cfg_parser7.default_port() != E_CAP_PB_PORT_P4_PCK) {
       pbc_csr.port_7.cfg_write_control.packing(0);
    }
    if (pbc_csr.cfg_parser7.default_port() <= PB_PORT_ETH_LAST ) {
       pbc_csr.port_7.cfg_write_control.rewrite(0); // 
    } else {
       pbc_csr.port_7.cfg_write_control.rewrite(1); // enable this on eth ports when going to P4
    }
    pbc_csr.port_7.cfg_write_control.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_7.cfg_write_control.show();}
    //
    // default
    if (pbc_csr.cfg_parser8.default_port() == E_CAP_PB_PORT_P4_PCK) {
       pbc_csr.port_8.cfg_write_control.packing(0);
       pbc_csr.port_8.cfg_write_control.packing_timeout_cycles(10);
    }
    pbc_csr.port_8.cfg_write_control.enable(1);
    pbc_csr.port_8.cfg_write_control.cut_thru(-1);
    pbc_csr.port_8.cfg_write_control.load_from_cfg(0);
    // overwride 
    if (pbc_csr.cfg_parser8.default_port() != E_CAP_PB_PORT_P4_PCK) {
       pbc_csr.port_8.cfg_write_control.packing(0);
    }
    if (pbc_csr.cfg_parser8.default_port() <= PB_PORT_ETH_LAST ) {
       pbc_csr.port_8.cfg_write_control.rewrite(0); // 
    } else {
       pbc_csr.port_8.cfg_write_control.rewrite(1); // enable this on eth ports when going to P4
    }
    pbc_csr.port_8.cfg_write_control.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_8.cfg_write_control.show();}
    //
    pbc_csr.port_9.cfg_write_control.enable(1);
    pbc_csr.port_9.cfg_write_control.cut_thru(-1);
    pbc_csr.port_9.cfg_write_control.load_from_cfg(0);
       pbc_csr.port_9.cfg_write_control.rewrite(1);
    pbc_csr.port_9.cfg_write_control.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_write_control.show();}
    //
    pbc_csr.port_10.cfg_write_control.enable(1);
    pbc_csr.port_10.cfg_write_control.cut_thru(-1);
    pbc_csr.port_10.cfg_write_control.load_from_cfg(0);
       pbc_csr.port_10.cfg_write_control.rewrite(1);
       pbc_csr.port_10.cfg_write_control.recirc_oq(recirc_oq[10]); // since this is after the 2nd load_from_cfg call, a rand knob would eval twice for ex
    pbc_csr.port_10.cfg_write_control.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_write_control.show();}
    //
    pbc_csr.port_11.cfg_write_control.enable(1);
    pbc_csr.port_11.cfg_write_control.cut_thru(-1);
    pbc_csr.port_11.cfg_write_control.load_from_cfg(0);
       pbc_csr.port_11.cfg_write_control.rewrite(1);
       pbc_csr.port_11.cfg_write_control.recirc_oq(recirc_oq[11]); // since this is after the 2nd load_from_cfg call, a rand knob would eval twice for ex
    pbc_csr.port_11.cfg_write_control.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_write_control.show();}
    //
   //enable output queues ()
    pbc_csr.port_0.cfg_oq.load_from_cfg(0);
    pbc_csr.port_0.cfg_oq.enable(1);
    pbc_csr.port_0.cfg_oq.rewrite_enable(1);
    pbc_csr.port_0.cfg_oq.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_0.cfg_oq.show();}
    //
    pbc_csr.port_1.cfg_oq.load_from_cfg(0);
    pbc_csr.port_1.cfg_oq.enable(1);
    pbc_csr.port_1.cfg_oq.rewrite_enable(1);
    pbc_csr.port_1.cfg_oq.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_1.cfg_oq.show();}
    //
    pbc_csr.port_2.cfg_oq.load_from_cfg(0);
    pbc_csr.port_2.cfg_oq.enable(1);
    pbc_csr.port_2.cfg_oq.rewrite_enable(1);
    pbc_csr.port_2.cfg_oq.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_2.cfg_oq.show();}
    //
    pbc_csr.port_3.cfg_oq.load_from_cfg(0);
    pbc_csr.port_3.cfg_oq.enable(1);
    pbc_csr.port_3.cfg_oq.rewrite_enable(1);
    pbc_csr.port_3.cfg_oq.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_3.cfg_oq.show();}
    //
    pbc_csr.port_4.cfg_oq.load_from_cfg(0);
    pbc_csr.port_4.cfg_oq.enable(1);
    pbc_csr.port_4.cfg_oq.rewrite_enable(1);
    pbc_csr.port_4.cfg_oq.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_4.cfg_oq.show();}
    //
    pbc_csr.port_5.cfg_oq.load_from_cfg(0);
    pbc_csr.port_5.cfg_oq.enable(1);
    pbc_csr.port_5.cfg_oq.rewrite_enable(1);
    pbc_csr.port_5.cfg_oq.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_5.cfg_oq.show();}
    //
    pbc_csr.port_6.cfg_oq.load_from_cfg(0);
    pbc_csr.port_6.cfg_oq.enable(1);
    pbc_csr.port_6.cfg_oq.rewrite_enable(1);
    pbc_csr.port_6.cfg_oq.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_6.cfg_oq.show();}
    //
    pbc_csr.port_7.cfg_oq.load_from_cfg(0);
    pbc_csr.port_7.cfg_oq.enable(1);
    pbc_csr.port_7.cfg_oq.rewrite_enable(1);
    pbc_csr.port_7.cfg_oq.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_7.cfg_oq.show();}
    //
    pbc_csr.port_8.cfg_oq.load_from_cfg(0);
    pbc_csr.port_8.cfg_oq.enable(1);
    pbc_csr.port_8.cfg_oq.rewrite_enable(1);
    pbc_csr.port_8.cfg_oq.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_8.cfg_oq.show();}
    //
    pbc_csr.port_9.cfg_oq.load_from_cfg(0);
    pbc_csr.port_9.cfg_oq.enable(1);
    pbc_csr.port_9.cfg_oq.rewrite_enable(1);
    pbc_csr.port_9.cfg_oq.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_oq.show();}
    //
    pbc_csr.port_10.cfg_oq.load_from_cfg(0);
    pbc_csr.port_10.cfg_oq.enable(1);
    pbc_csr.port_10.cfg_oq.rewrite_enable(1);
    pbc_csr.port_10.cfg_oq.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_oq.show();}
    //
    pbc_csr.port_11.cfg_oq.packing_msb( pbc_csr.cfg_island_control.map().convert_to<uint32_t>() ==1 ? 0: 1); // if island 1 (ethernet ports) use the upper 5K cells
    pbc_csr.port_11.cfg_oq.load_from_cfg(0);
    pbc_csr.port_11.cfg_oq.enable(1);
    pbc_csr.port_11.cfg_oq.rewrite_enable(1);
    pbc_csr.port_11.cfg_oq.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_oq.show();}
    //
    uint32_t mtu_max=0;   
    pbc_csr.port_0.cfg_account_mtu_table.load_from_cfg();
        if (mtu_max < (pbc_csr.port_0.cfg_account_mtu_table.pg0().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_0.cfg_account_mtu_table.pg0().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_0.cfg_account_mtu_table.pg1().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_0.cfg_account_mtu_table.pg1().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_0.cfg_account_mtu_table.pg2().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_0.cfg_account_mtu_table.pg2().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_0.cfg_account_mtu_table.pg3().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_0.cfg_account_mtu_table.pg3().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_0.cfg_account_mtu_table.pg4().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_0.cfg_account_mtu_table.pg4().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_0.cfg_account_mtu_table.pg5().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_0.cfg_account_mtu_table.pg5().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_0.cfg_account_mtu_table.pg6().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_0.cfg_account_mtu_table.pg6().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_0.cfg_account_mtu_table.pg7().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_0.cfg_account_mtu_table.pg7().convert_to<uint32_t>())+1;
    pbc_csr.port_1.cfg_account_mtu_table.load_from_cfg();
        if (mtu_max < (pbc_csr.port_1.cfg_account_mtu_table.pg0().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_1.cfg_account_mtu_table.pg0().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_1.cfg_account_mtu_table.pg1().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_1.cfg_account_mtu_table.pg1().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_1.cfg_account_mtu_table.pg2().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_1.cfg_account_mtu_table.pg2().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_1.cfg_account_mtu_table.pg3().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_1.cfg_account_mtu_table.pg3().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_1.cfg_account_mtu_table.pg4().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_1.cfg_account_mtu_table.pg4().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_1.cfg_account_mtu_table.pg5().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_1.cfg_account_mtu_table.pg5().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_1.cfg_account_mtu_table.pg6().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_1.cfg_account_mtu_table.pg6().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_1.cfg_account_mtu_table.pg7().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_1.cfg_account_mtu_table.pg7().convert_to<uint32_t>())+1;
    pbc_csr.port_2.cfg_account_mtu_table.load_from_cfg();
        if (mtu_max < (pbc_csr.port_2.cfg_account_mtu_table.pg0().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_2.cfg_account_mtu_table.pg0().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_2.cfg_account_mtu_table.pg1().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_2.cfg_account_mtu_table.pg1().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_2.cfg_account_mtu_table.pg2().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_2.cfg_account_mtu_table.pg2().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_2.cfg_account_mtu_table.pg3().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_2.cfg_account_mtu_table.pg3().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_2.cfg_account_mtu_table.pg4().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_2.cfg_account_mtu_table.pg4().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_2.cfg_account_mtu_table.pg5().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_2.cfg_account_mtu_table.pg5().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_2.cfg_account_mtu_table.pg6().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_2.cfg_account_mtu_table.pg6().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_2.cfg_account_mtu_table.pg7().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_2.cfg_account_mtu_table.pg7().convert_to<uint32_t>())+1;
    pbc_csr.port_3.cfg_account_mtu_table.load_from_cfg();
        if (mtu_max < (pbc_csr.port_3.cfg_account_mtu_table.pg0().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_3.cfg_account_mtu_table.pg0().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_3.cfg_account_mtu_table.pg1().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_3.cfg_account_mtu_table.pg1().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_3.cfg_account_mtu_table.pg2().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_3.cfg_account_mtu_table.pg2().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_3.cfg_account_mtu_table.pg3().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_3.cfg_account_mtu_table.pg3().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_3.cfg_account_mtu_table.pg4().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_3.cfg_account_mtu_table.pg4().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_3.cfg_account_mtu_table.pg5().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_3.cfg_account_mtu_table.pg5().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_3.cfg_account_mtu_table.pg6().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_3.cfg_account_mtu_table.pg6().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_3.cfg_account_mtu_table.pg7().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_3.cfg_account_mtu_table.pg7().convert_to<uint32_t>())+1;
    pbc_csr.port_4.cfg_account_mtu_table.load_from_cfg();
        if (mtu_max < (pbc_csr.port_4.cfg_account_mtu_table.pg0().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_4.cfg_account_mtu_table.pg0().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_4.cfg_account_mtu_table.pg1().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_4.cfg_account_mtu_table.pg1().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_4.cfg_account_mtu_table.pg2().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_4.cfg_account_mtu_table.pg2().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_4.cfg_account_mtu_table.pg3().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_4.cfg_account_mtu_table.pg3().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_4.cfg_account_mtu_table.pg4().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_4.cfg_account_mtu_table.pg4().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_4.cfg_account_mtu_table.pg5().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_4.cfg_account_mtu_table.pg5().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_4.cfg_account_mtu_table.pg6().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_4.cfg_account_mtu_table.pg6().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_4.cfg_account_mtu_table.pg7().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_4.cfg_account_mtu_table.pg7().convert_to<uint32_t>())+1;
    pbc_csr.port_5.cfg_account_mtu_table.load_from_cfg();
        if (mtu_max < (pbc_csr.port_5.cfg_account_mtu_table.pg0().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_5.cfg_account_mtu_table.pg0().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_5.cfg_account_mtu_table.pg1().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_5.cfg_account_mtu_table.pg1().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_5.cfg_account_mtu_table.pg2().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_5.cfg_account_mtu_table.pg2().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_5.cfg_account_mtu_table.pg3().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_5.cfg_account_mtu_table.pg3().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_5.cfg_account_mtu_table.pg4().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_5.cfg_account_mtu_table.pg4().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_5.cfg_account_mtu_table.pg5().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_5.cfg_account_mtu_table.pg5().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_5.cfg_account_mtu_table.pg6().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_5.cfg_account_mtu_table.pg6().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_5.cfg_account_mtu_table.pg7().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_5.cfg_account_mtu_table.pg7().convert_to<uint32_t>())+1;
    pbc_csr.port_6.cfg_account_mtu_table.load_from_cfg();
        if (mtu_max < (pbc_csr.port_6.cfg_account_mtu_table.pg0().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_6.cfg_account_mtu_table.pg0().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_6.cfg_account_mtu_table.pg1().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_6.cfg_account_mtu_table.pg1().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_6.cfg_account_mtu_table.pg2().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_6.cfg_account_mtu_table.pg2().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_6.cfg_account_mtu_table.pg3().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_6.cfg_account_mtu_table.pg3().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_6.cfg_account_mtu_table.pg4().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_6.cfg_account_mtu_table.pg4().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_6.cfg_account_mtu_table.pg5().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_6.cfg_account_mtu_table.pg5().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_6.cfg_account_mtu_table.pg6().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_6.cfg_account_mtu_table.pg6().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_6.cfg_account_mtu_table.pg7().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_6.cfg_account_mtu_table.pg7().convert_to<uint32_t>())+1;
    pbc_csr.port_7.cfg_account_mtu_table.load_from_cfg();
        if (mtu_max < (pbc_csr.port_7.cfg_account_mtu_table.pg0().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_7.cfg_account_mtu_table.pg0().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_7.cfg_account_mtu_table.pg1().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_7.cfg_account_mtu_table.pg1().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_7.cfg_account_mtu_table.pg2().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_7.cfg_account_mtu_table.pg2().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_7.cfg_account_mtu_table.pg3().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_7.cfg_account_mtu_table.pg3().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_7.cfg_account_mtu_table.pg4().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_7.cfg_account_mtu_table.pg4().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_7.cfg_account_mtu_table.pg5().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_7.cfg_account_mtu_table.pg5().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_7.cfg_account_mtu_table.pg6().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_7.cfg_account_mtu_table.pg6().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_7.cfg_account_mtu_table.pg7().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_7.cfg_account_mtu_table.pg7().convert_to<uint32_t>())+1;
    pbc_csr.port_8.cfg_account_mtu_table.load_from_cfg();
        if (mtu_max < (pbc_csr.port_8.cfg_account_mtu_table.pg0().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_8.cfg_account_mtu_table.pg0().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_8.cfg_account_mtu_table.pg1().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_8.cfg_account_mtu_table.pg1().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_8.cfg_account_mtu_table.pg2().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_8.cfg_account_mtu_table.pg2().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_8.cfg_account_mtu_table.pg3().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_8.cfg_account_mtu_table.pg3().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_8.cfg_account_mtu_table.pg4().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_8.cfg_account_mtu_table.pg4().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_8.cfg_account_mtu_table.pg5().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_8.cfg_account_mtu_table.pg5().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_8.cfg_account_mtu_table.pg6().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_8.cfg_account_mtu_table.pg6().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_8.cfg_account_mtu_table.pg7().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_8.cfg_account_mtu_table.pg7().convert_to<uint32_t>())+1;
    pbc_csr.port_9.cfg_account_mtu_table.load_from_cfg();
        if (mtu_max < (pbc_csr.port_9.cfg_account_mtu_table.pg0().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_9.cfg_account_mtu_table.pg0().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_9.cfg_account_mtu_table.pg1().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_9.cfg_account_mtu_table.pg1().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_9.cfg_account_mtu_table.pg2().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_9.cfg_account_mtu_table.pg2().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_9.cfg_account_mtu_table.pg3().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_9.cfg_account_mtu_table.pg3().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_9.cfg_account_mtu_table.pg4().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_9.cfg_account_mtu_table.pg4().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_9.cfg_account_mtu_table.pg5().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_9.cfg_account_mtu_table.pg5().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_9.cfg_account_mtu_table.pg6().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_9.cfg_account_mtu_table.pg6().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_9.cfg_account_mtu_table.pg7().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_9.cfg_account_mtu_table.pg7().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_9.cfg_account_mtu_table.pg8().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_9.cfg_account_mtu_table.pg8().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_9.cfg_account_mtu_table.pg9().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_9.cfg_account_mtu_table.pg9().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_9.cfg_account_mtu_table.pg10().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_9.cfg_account_mtu_table.pg10().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_9.cfg_account_mtu_table.pg11().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_9.cfg_account_mtu_table.pg11().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_9.cfg_account_mtu_table.pg12().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_9.cfg_account_mtu_table.pg12().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_9.cfg_account_mtu_table.pg13().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_9.cfg_account_mtu_table.pg13().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_9.cfg_account_mtu_table.pg14().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_9.cfg_account_mtu_table.pg14().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_9.cfg_account_mtu_table.pg15().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_9.cfg_account_mtu_table.pg15().convert_to<uint32_t>())+1;
    pbc_csr.port_10.cfg_account_mtu_table.load_from_cfg();
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg0().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg0().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg1().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg1().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg2().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg2().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg3().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg3().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg4().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg4().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg5().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg5().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg6().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg6().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg7().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg7().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg8().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg8().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg9().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg9().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg10().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg10().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg11().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg11().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg12().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg12().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg13().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg13().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg14().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg14().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg15().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg15().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg16().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg16().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg17().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg17().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg18().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg18().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg19().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg19().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg20().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg20().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg21().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg21().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg22().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg22().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg23().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg23().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg24().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg24().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg25().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg25().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg26().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg26().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg27().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg27().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg28().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg28().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg29().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg29().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg30().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg30().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_10.cfg_account_mtu_table.pg31().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_10.cfg_account_mtu_table.pg31().convert_to<uint32_t>())+1;
    pbc_csr.port_11.cfg_account_mtu_table.load_from_cfg();
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg0().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg0().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg1().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg1().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg2().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg2().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg3().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg3().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg4().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg4().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg5().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg5().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg6().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg6().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg7().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg7().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg8().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg8().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg9().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg9().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg10().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg10().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg11().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg11().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg12().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg12().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg13().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg13().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg14().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg14().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg15().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg15().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg16().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg16().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg17().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg17().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg18().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg18().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg19().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg19().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg20().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg20().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg21().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg21().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg22().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg22().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg23().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg23().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg24().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg24().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg25().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg25().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg26().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg26().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg27().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg27().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg28().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg28().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg29().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg29().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg30().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg30().convert_to<uint32_t>())+1;
        if (mtu_max < (pbc_csr.port_11.cfg_account_mtu_table.pg31().convert_to<uint32_t>())+1) mtu_max = (pbc_csr.port_11.cfg_account_mtu_table.pg31().convert_to<uint32_t>())+1;
        mtu_max+=1; // repl or eth->p4 can grow a cell_size pkt to cell_sz+1. So give an extra credit

    pbc_csr.cfg_tail_drop.load_from_cfg();
    pbc_csr.cfg_rpl.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.cfg_rpl.show();}

    pbc_csr.port_0.cfg_oq_queue.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_0.cfg_oq_queue.show();}
    pbc_csr.port_1.cfg_oq_queue.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_1.cfg_oq_queue.show();}
    pbc_csr.port_2.cfg_oq_queue.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_2.cfg_oq_queue.show();}
    pbc_csr.port_3.cfg_oq_queue.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_3.cfg_oq_queue.show();}
    pbc_csr.port_4.cfg_oq_queue.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_4.cfg_oq_queue.show();}
    pbc_csr.port_5.cfg_oq_queue.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_5.cfg_oq_queue.show();}
    pbc_csr.port_6.cfg_oq_queue.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_6.cfg_oq_queue.show();}
    pbc_csr.port_7.cfg_oq_queue.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_7.cfg_oq_queue.show();}
    pbc_csr.port_8.cfg_oq_queue.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_8.cfg_oq_queue.show();}
    pbc_csr.port_9.cfg_oq_queue.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_oq_queue.show();}
    pbc_csr.port_10.cfg_oq_queue.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_oq_queue.show();}
    pbc_csr.port_11.cfg_oq_queue.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_oq_queue.show();}
    //credits
    unsigned int pg_thr_consistent_with_credits;
    unsigned int credits,thr[32],mtu[32];
    cap_pbc_max_growth_map_t max_growth_map;
    uint32_t credit_en,flip;
    if (pbc_csr.port_10.cfg_oq.flow_control_enable_credits().convert_to<uint32_t>() ==1) {
       uint32_t thr_update=0;

       pbc_csr.port_10.cfg_account_credit_return.load_from_cfg(0);
       credit_en = pbc_csr.port_10.cfg_account_credit_return.enable().convert_to<uint32_t>();
       if (pbc_csr.port_10.cfg_write_control.recirc_enable().convert_to<uint32_t>()) {
          flip = 1 << pbc_csr.port_10.cfg_write_control.recirc_oq().convert_to<uint32_t>() ; 
          credit_en = credit_en & ~flip;
          pbc_csr.port_10.cfg_account_credit_return.enable(credit_en);
          pbc_csr.port_10.cfg_oq_queue.recirc(flip); // written later below
       }
       pbc_csr.port_10.cfg_account_credit_return.write();
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_credit_return.show();}

       max_growth_map.init();
       cfg_found_cnt = pbc_csr.cfg_credits_max_growth_10.load_from_cfg_rtn_status(0);
       max_growth_map.all(pbc_csr.cfg_credits_max_growth_10.cells()); // init
       max_growth_map.set_name(pbc_csr.cfg_credits_max_growth_10.get_hier_path()+".decoder");
       cfg_found_cnt += max_growth_map.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.cfg_credits_max_growth_10.cells(max_growth_map.all());
          pbc_csr.cfg_credits_max_growth_10.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.cfg_credits_max_growth_10.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {max_growth_map.show(); }

       spath = pbc_csr.port_10.dhs_oq_flow_control.get_hier_path()+".pg_thr_consistent_with_credits";
       std::replace( spath.begin(), spath.end(), '.', '/');
       pg_thr_consistent_with_credits = sknobs_get_value((char *)spath.c_str(), 0);

        thr[0] = pbc_csr.port_10.cfg_account_pg_0.reserved_min().convert_to<uint32_t>();
        mtu[0] = (pbc_csr.port_10.cfg_account_mtu_table.pg0().convert_to<int>())+1;
        thr[1] = pbc_csr.port_10.cfg_account_pg_1.reserved_min().convert_to<uint32_t>();
        mtu[1] = (pbc_csr.port_10.cfg_account_mtu_table.pg1().convert_to<int>())+1;
        thr[2] = pbc_csr.port_10.cfg_account_pg_2.reserved_min().convert_to<uint32_t>();
        mtu[2] = (pbc_csr.port_10.cfg_account_mtu_table.pg2().convert_to<int>())+1;
        thr[3] = pbc_csr.port_10.cfg_account_pg_3.reserved_min().convert_to<uint32_t>();
        mtu[3] = (pbc_csr.port_10.cfg_account_mtu_table.pg3().convert_to<int>())+1;
        thr[4] = pbc_csr.port_10.cfg_account_pg_4.reserved_min().convert_to<uint32_t>();
        mtu[4] = (pbc_csr.port_10.cfg_account_mtu_table.pg4().convert_to<int>())+1;
        thr[5] = pbc_csr.port_10.cfg_account_pg_5.reserved_min().convert_to<uint32_t>();
        mtu[5] = (pbc_csr.port_10.cfg_account_mtu_table.pg5().convert_to<int>())+1;
        thr[6] = pbc_csr.port_10.cfg_account_pg_6.reserved_min().convert_to<uint32_t>();
        mtu[6] = (pbc_csr.port_10.cfg_account_mtu_table.pg6().convert_to<int>())+1;
        thr[7] = pbc_csr.port_10.cfg_account_pg_7.reserved_min().convert_to<uint32_t>();
        mtu[7] = (pbc_csr.port_10.cfg_account_mtu_table.pg7().convert_to<int>())+1;
        thr[8] = pbc_csr.port_10.cfg_account_pg_8.reserved_min().convert_to<uint32_t>();
        mtu[8] = (pbc_csr.port_10.cfg_account_mtu_table.pg8().convert_to<int>())+1;
        thr[9] = pbc_csr.port_10.cfg_account_pg_9.reserved_min().convert_to<uint32_t>();
        mtu[9] = (pbc_csr.port_10.cfg_account_mtu_table.pg9().convert_to<int>())+1;
        thr[10] = pbc_csr.port_10.cfg_account_pg_10.reserved_min().convert_to<uint32_t>();
        mtu[10] = (pbc_csr.port_10.cfg_account_mtu_table.pg10().convert_to<int>())+1;
        thr[11] = pbc_csr.port_10.cfg_account_pg_11.reserved_min().convert_to<uint32_t>();
        mtu[11] = (pbc_csr.port_10.cfg_account_mtu_table.pg11().convert_to<int>())+1;
        thr[12] = pbc_csr.port_10.cfg_account_pg_12.reserved_min().convert_to<uint32_t>();
        mtu[12] = (pbc_csr.port_10.cfg_account_mtu_table.pg12().convert_to<int>())+1;
        thr[13] = pbc_csr.port_10.cfg_account_pg_13.reserved_min().convert_to<uint32_t>();
        mtu[13] = (pbc_csr.port_10.cfg_account_mtu_table.pg13().convert_to<int>())+1;
        thr[14] = pbc_csr.port_10.cfg_account_pg_14.reserved_min().convert_to<uint32_t>();
        mtu[14] = (pbc_csr.port_10.cfg_account_mtu_table.pg14().convert_to<int>())+1;
        thr[15] = pbc_csr.port_10.cfg_account_pg_15.reserved_min().convert_to<uint32_t>();
        mtu[15] = (pbc_csr.port_10.cfg_account_mtu_table.pg15().convert_to<int>())+1;
        thr[16] = pbc_csr.port_10.cfg_account_pg_16.reserved_min().convert_to<uint32_t>();
        mtu[16] = (pbc_csr.port_10.cfg_account_mtu_table.pg16().convert_to<int>())+1;
        thr[17] = pbc_csr.port_10.cfg_account_pg_17.reserved_min().convert_to<uint32_t>();
        mtu[17] = (pbc_csr.port_10.cfg_account_mtu_table.pg17().convert_to<int>())+1;
        thr[18] = pbc_csr.port_10.cfg_account_pg_18.reserved_min().convert_to<uint32_t>();
        mtu[18] = (pbc_csr.port_10.cfg_account_mtu_table.pg18().convert_to<int>())+1;
        thr[19] = pbc_csr.port_10.cfg_account_pg_19.reserved_min().convert_to<uint32_t>();
        mtu[19] = (pbc_csr.port_10.cfg_account_mtu_table.pg19().convert_to<int>())+1;
        thr[20] = pbc_csr.port_10.cfg_account_pg_20.reserved_min().convert_to<uint32_t>();
        mtu[20] = (pbc_csr.port_10.cfg_account_mtu_table.pg20().convert_to<int>())+1;
        thr[21] = pbc_csr.port_10.cfg_account_pg_21.reserved_min().convert_to<uint32_t>();
        mtu[21] = (pbc_csr.port_10.cfg_account_mtu_table.pg21().convert_to<int>())+1;
        thr[22] = pbc_csr.port_10.cfg_account_pg_22.reserved_min().convert_to<uint32_t>();
        mtu[22] = (pbc_csr.port_10.cfg_account_mtu_table.pg22().convert_to<int>())+1;
        thr[23] = pbc_csr.port_10.cfg_account_pg_23.reserved_min().convert_to<uint32_t>();
        mtu[23] = (pbc_csr.port_10.cfg_account_mtu_table.pg23().convert_to<int>())+1;
        thr[24] = pbc_csr.port_10.cfg_account_pg_24.reserved_min().convert_to<uint32_t>();
        mtu[24] = (pbc_csr.port_10.cfg_account_mtu_table.pg24().convert_to<int>())+1;
        thr[25] = pbc_csr.port_10.cfg_account_pg_25.reserved_min().convert_to<uint32_t>();
        mtu[25] = (pbc_csr.port_10.cfg_account_mtu_table.pg25().convert_to<int>())+1;
        thr[26] = pbc_csr.port_10.cfg_account_pg_26.reserved_min().convert_to<uint32_t>();
        mtu[26] = (pbc_csr.port_10.cfg_account_mtu_table.pg26().convert_to<int>())+1;
        thr[27] = pbc_csr.port_10.cfg_account_pg_27.reserved_min().convert_to<uint32_t>();
        mtu[27] = (pbc_csr.port_10.cfg_account_mtu_table.pg27().convert_to<int>())+1;
        thr[28] = pbc_csr.port_10.cfg_account_pg_28.reserved_min().convert_to<uint32_t>();
        mtu[28] = (pbc_csr.port_10.cfg_account_mtu_table.pg28().convert_to<int>())+1;
        thr[29] = pbc_csr.port_10.cfg_account_pg_29.reserved_min().convert_to<uint32_t>();
        mtu[29] = (pbc_csr.port_10.cfg_account_mtu_table.pg29().convert_to<int>())+1;
        thr[30] = pbc_csr.port_10.cfg_account_pg_30.reserved_min().convert_to<uint32_t>();
        mtu[30] = (pbc_csr.port_10.cfg_account_mtu_table.pg30().convert_to<int>())+1;
        thr[31] = pbc_csr.port_10.cfg_account_pg_31.reserved_min().convert_to<uint32_t>();
        mtu[31] = (pbc_csr.port_10.cfg_account_mtu_table.pg31().convert_to<int>())+1;
       pg32_map.all(pbc_csr.port_10.cfg_account_tc_to_pg.table()); 

       // program credits
       cap_pbcport10_csr_dhs_oq_flow_control_entry_t tmp10;
       for (int ii=0; ii<pbc_csr.port_10.dhs_oq_flow_control.get_depth_entry(); ii++) {
          if ( ((credit_en >> ii)& 1) ==0) {
             pbc_csr.port_10.dhs_oq_flow_control.entry[ii].entry(0);
          } else {
             tmp10.set_name(pbc_csr.port_10.dhs_oq_flow_control.entry[ii].get_hier_path()+".credits_over_min");
             tmp10.init();
             tmp10.load_from_cfg(0);
             //tmp10.show();
             int credits_over_min = tmp10.all().convert_to<int>();
             credits = mtu_max + max_growth_map.max_growth(ii).convert_to<int>() + credits_over_min;
             PLOG_MSG(     "iq: "<<ii<<
                           " credits="<<credits<<
                           " credits_over_min="<<credits_over_min<< 
                           " mtu_max="<<mtu_max<< 
                           " max_growth=" <<max_growth_map.max_growth(ii)<<
                           endl);
             pbc_csr.port_10.dhs_oq_flow_control.entry[ii].entry(credits);
             // set rsv min appropriately
             if (pg_thr_consistent_with_credits) { // ii is the iq
                    int pg = pg32_map.pg(ii).convert_to<int>();
                    thr_update = (thr_update | (1 << pg));
                    thr[pg] = std::ceil((credits + mtu[pg])/4.0);
                    PLOG_MSG(     "iq: "<<ii<<
                                  " pg="<<pg<<
                                  " thr="<<thr[pg]<< 
                                  " credits="<<credits<<
                                  " mtu="<<mtu[pg]<< 
                                  endl);
             }
          }
          pbc_csr.port_10.dhs_oq_flow_control.entry[ii].set_access_no_zero_time(1);
          pbc_csr.port_10.dhs_oq_flow_control.entry[ii].write();

       }
       for (int ii=0; ii<pbc_csr.port_10.dhs_oq_flow_control.get_depth_entry(); ii++) {
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.dhs_oq_flow_control.entry[ii].show();}
       }
       if (pg_thr_consistent_with_credits) {
          if ((thr_update >> 0 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_0.reserved_min(thr[0]);
             pbc_csr.port_10.cfg_account_pg_0.headroom(0);
             pbc_csr.port_10.cfg_account_pg_0.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_0.show();}
          }
          if ((thr_update >> 1 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_1.reserved_min(thr[1]);
             pbc_csr.port_10.cfg_account_pg_1.headroom(0);
             pbc_csr.port_10.cfg_account_pg_1.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_1.show();}
          }
          if ((thr_update >> 2 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_2.reserved_min(thr[2]);
             pbc_csr.port_10.cfg_account_pg_2.headroom(0);
             pbc_csr.port_10.cfg_account_pg_2.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_2.show();}
          }
          if ((thr_update >> 3 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_3.reserved_min(thr[3]);
             pbc_csr.port_10.cfg_account_pg_3.headroom(0);
             pbc_csr.port_10.cfg_account_pg_3.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_3.show();}
          }
          if ((thr_update >> 4 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_4.reserved_min(thr[4]);
             pbc_csr.port_10.cfg_account_pg_4.headroom(0);
             pbc_csr.port_10.cfg_account_pg_4.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_4.show();}
          }
          if ((thr_update >> 5 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_5.reserved_min(thr[5]);
             pbc_csr.port_10.cfg_account_pg_5.headroom(0);
             pbc_csr.port_10.cfg_account_pg_5.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_5.show();}
          }
          if ((thr_update >> 6 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_6.reserved_min(thr[6]);
             pbc_csr.port_10.cfg_account_pg_6.headroom(0);
             pbc_csr.port_10.cfg_account_pg_6.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_6.show();}
          }
          if ((thr_update >> 7 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_7.reserved_min(thr[7]);
             pbc_csr.port_10.cfg_account_pg_7.headroom(0);
             pbc_csr.port_10.cfg_account_pg_7.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_7.show();}
          }
          if ((thr_update >> 8 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_8.reserved_min(thr[8]);
             pbc_csr.port_10.cfg_account_pg_8.headroom(0);
             pbc_csr.port_10.cfg_account_pg_8.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_8.show();}
          }
          if ((thr_update >> 9 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_9.reserved_min(thr[9]);
             pbc_csr.port_10.cfg_account_pg_9.headroom(0);
             pbc_csr.port_10.cfg_account_pg_9.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_9.show();}
          }
          if ((thr_update >> 10 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_10.reserved_min(thr[10]);
             pbc_csr.port_10.cfg_account_pg_10.headroom(0);
             pbc_csr.port_10.cfg_account_pg_10.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_10.show();}
          }
          if ((thr_update >> 11 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_11.reserved_min(thr[11]);
             pbc_csr.port_10.cfg_account_pg_11.headroom(0);
             pbc_csr.port_10.cfg_account_pg_11.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_11.show();}
          }
          if ((thr_update >> 12 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_12.reserved_min(thr[12]);
             pbc_csr.port_10.cfg_account_pg_12.headroom(0);
             pbc_csr.port_10.cfg_account_pg_12.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_12.show();}
          }
          if ((thr_update >> 13 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_13.reserved_min(thr[13]);
             pbc_csr.port_10.cfg_account_pg_13.headroom(0);
             pbc_csr.port_10.cfg_account_pg_13.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_13.show();}
          }
          if ((thr_update >> 14 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_14.reserved_min(thr[14]);
             pbc_csr.port_10.cfg_account_pg_14.headroom(0);
             pbc_csr.port_10.cfg_account_pg_14.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_14.show();}
          }
          if ((thr_update >> 15 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_15.reserved_min(thr[15]);
             pbc_csr.port_10.cfg_account_pg_15.headroom(0);
             pbc_csr.port_10.cfg_account_pg_15.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_15.show();}
          }
          if ((thr_update >> 16 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_16.reserved_min(thr[16]);
             pbc_csr.port_10.cfg_account_pg_16.headroom(0);
             pbc_csr.port_10.cfg_account_pg_16.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_16.show();}
          }
          if ((thr_update >> 17 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_17.reserved_min(thr[17]);
             pbc_csr.port_10.cfg_account_pg_17.headroom(0);
             pbc_csr.port_10.cfg_account_pg_17.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_17.show();}
          }
          if ((thr_update >> 18 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_18.reserved_min(thr[18]);
             pbc_csr.port_10.cfg_account_pg_18.headroom(0);
             pbc_csr.port_10.cfg_account_pg_18.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_18.show();}
          }
          if ((thr_update >> 19 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_19.reserved_min(thr[19]);
             pbc_csr.port_10.cfg_account_pg_19.headroom(0);
             pbc_csr.port_10.cfg_account_pg_19.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_19.show();}
          }
          if ((thr_update >> 20 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_20.reserved_min(thr[20]);
             pbc_csr.port_10.cfg_account_pg_20.headroom(0);
             pbc_csr.port_10.cfg_account_pg_20.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_20.show();}
          }
          if ((thr_update >> 21 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_21.reserved_min(thr[21]);
             pbc_csr.port_10.cfg_account_pg_21.headroom(0);
             pbc_csr.port_10.cfg_account_pg_21.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_21.show();}
          }
          if ((thr_update >> 22 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_22.reserved_min(thr[22]);
             pbc_csr.port_10.cfg_account_pg_22.headroom(0);
             pbc_csr.port_10.cfg_account_pg_22.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_22.show();}
          }
          if ((thr_update >> 23 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_23.reserved_min(thr[23]);
             pbc_csr.port_10.cfg_account_pg_23.headroom(0);
             pbc_csr.port_10.cfg_account_pg_23.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_23.show();}
          }
          if ((thr_update >> 24 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_24.reserved_min(thr[24]);
             pbc_csr.port_10.cfg_account_pg_24.headroom(0);
             pbc_csr.port_10.cfg_account_pg_24.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_24.show();}
          }
          if ((thr_update >> 25 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_25.reserved_min(thr[25]);
             pbc_csr.port_10.cfg_account_pg_25.headroom(0);
             pbc_csr.port_10.cfg_account_pg_25.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_25.show();}
          }
          if ((thr_update >> 26 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_26.reserved_min(thr[26]);
             pbc_csr.port_10.cfg_account_pg_26.headroom(0);
             pbc_csr.port_10.cfg_account_pg_26.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_26.show();}
          }
          if ((thr_update >> 27 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_27.reserved_min(thr[27]);
             pbc_csr.port_10.cfg_account_pg_27.headroom(0);
             pbc_csr.port_10.cfg_account_pg_27.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_27.show();}
          }
          if ((thr_update >> 28 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_28.reserved_min(thr[28]);
             pbc_csr.port_10.cfg_account_pg_28.headroom(0);
             pbc_csr.port_10.cfg_account_pg_28.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_28.show();}
          }
          if ((thr_update >> 29 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_29.reserved_min(thr[29]);
             pbc_csr.port_10.cfg_account_pg_29.headroom(0);
             pbc_csr.port_10.cfg_account_pg_29.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_29.show();}
          }
          if ((thr_update >> 30 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_30.reserved_min(thr[30]);
             pbc_csr.port_10.cfg_account_pg_30.headroom(0);
             pbc_csr.port_10.cfg_account_pg_30.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_30.show();}
          }
          if ((thr_update >> 31 ) & 1) {
             pbc_csr.port_10.cfg_account_pg_31.reserved_min(thr[31]);
             pbc_csr.port_10.cfg_account_pg_31.headroom(0);
             pbc_csr.port_10.cfg_account_pg_31.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_31.show();}
          }
       }
       // program flush
          pbc_csr.port_10.cfg_oq_queue.flush(pbc_csr.port_10.cfg_oq_queue.flush().convert_to<uint32_t>() | ~(pbc_csr.port_10.cfg_account_credit_return.enable().convert_to<uint32_t>() | pbc_csr.port_10.cfg_oq_queue.recirc().convert_to<uint32_t>()));
          pbc_csr.port_10.cfg_oq_queue.write();
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_oq_queue.show();}

    }
    if (pbc_csr.port_11.cfg_oq.flow_control_enable_credits().convert_to<uint32_t>() ==1) {
       uint32_t thr_update=0;

       pbc_csr.port_11.cfg_account_credit_return.load_from_cfg(0);
       credit_en = pbc_csr.port_11.cfg_account_credit_return.enable().convert_to<uint32_t>();
       if (pbc_csr.port_11.cfg_write_control.recirc_enable().convert_to<uint32_t>()) {
          flip = 1 << pbc_csr.port_11.cfg_write_control.recirc_oq().convert_to<uint32_t>() ; 
          credit_en = credit_en & ~flip;
          pbc_csr.port_11.cfg_account_credit_return.enable(credit_en);
          pbc_csr.port_11.cfg_oq_queue.recirc(flip); // written later below
       }
       pbc_csr.port_11.cfg_account_credit_return.write();
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_credit_return.show();}

       max_growth_map.init();
       cfg_found_cnt = pbc_csr.cfg_credits_max_growth_11.load_from_cfg_rtn_status(0);
       max_growth_map.all(pbc_csr.cfg_credits_max_growth_11.cells()); // init
       max_growth_map.set_name(pbc_csr.cfg_credits_max_growth_11.get_hier_path()+".decoder");
       cfg_found_cnt += max_growth_map.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.cfg_credits_max_growth_11.cells(max_growth_map.all());
          pbc_csr.cfg_credits_max_growth_11.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.cfg_credits_max_growth_11.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {max_growth_map.show(); }

       spath = pbc_csr.port_11.dhs_oq_flow_control.get_hier_path()+".pg_thr_consistent_with_credits";
       std::replace( spath.begin(), spath.end(), '.', '/');
       pg_thr_consistent_with_credits = sknobs_get_value((char *)spath.c_str(), 0);

        thr[0] = pbc_csr.port_11.cfg_account_pg_0.reserved_min().convert_to<uint32_t>();
        mtu[0] = (pbc_csr.port_11.cfg_account_mtu_table.pg0().convert_to<int>())+1;
        thr[1] = pbc_csr.port_11.cfg_account_pg_1.reserved_min().convert_to<uint32_t>();
        mtu[1] = (pbc_csr.port_11.cfg_account_mtu_table.pg1().convert_to<int>())+1;
        thr[2] = pbc_csr.port_11.cfg_account_pg_2.reserved_min().convert_to<uint32_t>();
        mtu[2] = (pbc_csr.port_11.cfg_account_mtu_table.pg2().convert_to<int>())+1;
        thr[3] = pbc_csr.port_11.cfg_account_pg_3.reserved_min().convert_to<uint32_t>();
        mtu[3] = (pbc_csr.port_11.cfg_account_mtu_table.pg3().convert_to<int>())+1;
        thr[4] = pbc_csr.port_11.cfg_account_pg_4.reserved_min().convert_to<uint32_t>();
        mtu[4] = (pbc_csr.port_11.cfg_account_mtu_table.pg4().convert_to<int>())+1;
        thr[5] = pbc_csr.port_11.cfg_account_pg_5.reserved_min().convert_to<uint32_t>();
        mtu[5] = (pbc_csr.port_11.cfg_account_mtu_table.pg5().convert_to<int>())+1;
        thr[6] = pbc_csr.port_11.cfg_account_pg_6.reserved_min().convert_to<uint32_t>();
        mtu[6] = (pbc_csr.port_11.cfg_account_mtu_table.pg6().convert_to<int>())+1;
        thr[7] = pbc_csr.port_11.cfg_account_pg_7.reserved_min().convert_to<uint32_t>();
        mtu[7] = (pbc_csr.port_11.cfg_account_mtu_table.pg7().convert_to<int>())+1;
        thr[8] = pbc_csr.port_11.cfg_account_pg_8.reserved_min().convert_to<uint32_t>();
        mtu[8] = (pbc_csr.port_11.cfg_account_mtu_table.pg8().convert_to<int>())+1;
        thr[9] = pbc_csr.port_11.cfg_account_pg_9.reserved_min().convert_to<uint32_t>();
        mtu[9] = (pbc_csr.port_11.cfg_account_mtu_table.pg9().convert_to<int>())+1;
        thr[10] = pbc_csr.port_11.cfg_account_pg_10.reserved_min().convert_to<uint32_t>();
        mtu[10] = (pbc_csr.port_11.cfg_account_mtu_table.pg10().convert_to<int>())+1;
        thr[11] = pbc_csr.port_11.cfg_account_pg_11.reserved_min().convert_to<uint32_t>();
        mtu[11] = (pbc_csr.port_11.cfg_account_mtu_table.pg11().convert_to<int>())+1;
        thr[12] = pbc_csr.port_11.cfg_account_pg_12.reserved_min().convert_to<uint32_t>();
        mtu[12] = (pbc_csr.port_11.cfg_account_mtu_table.pg12().convert_to<int>())+1;
        thr[13] = pbc_csr.port_11.cfg_account_pg_13.reserved_min().convert_to<uint32_t>();
        mtu[13] = (pbc_csr.port_11.cfg_account_mtu_table.pg13().convert_to<int>())+1;
        thr[14] = pbc_csr.port_11.cfg_account_pg_14.reserved_min().convert_to<uint32_t>();
        mtu[14] = (pbc_csr.port_11.cfg_account_mtu_table.pg14().convert_to<int>())+1;
        thr[15] = pbc_csr.port_11.cfg_account_pg_15.reserved_min().convert_to<uint32_t>();
        mtu[15] = (pbc_csr.port_11.cfg_account_mtu_table.pg15().convert_to<int>())+1;
        thr[16] = pbc_csr.port_11.cfg_account_pg_16.reserved_min().convert_to<uint32_t>();
        mtu[16] = (pbc_csr.port_11.cfg_account_mtu_table.pg16().convert_to<int>())+1;
        thr[17] = pbc_csr.port_11.cfg_account_pg_17.reserved_min().convert_to<uint32_t>();
        mtu[17] = (pbc_csr.port_11.cfg_account_mtu_table.pg17().convert_to<int>())+1;
        thr[18] = pbc_csr.port_11.cfg_account_pg_18.reserved_min().convert_to<uint32_t>();
        mtu[18] = (pbc_csr.port_11.cfg_account_mtu_table.pg18().convert_to<int>())+1;
        thr[19] = pbc_csr.port_11.cfg_account_pg_19.reserved_min().convert_to<uint32_t>();
        mtu[19] = (pbc_csr.port_11.cfg_account_mtu_table.pg19().convert_to<int>())+1;
        thr[20] = pbc_csr.port_11.cfg_account_pg_20.reserved_min().convert_to<uint32_t>();
        mtu[20] = (pbc_csr.port_11.cfg_account_mtu_table.pg20().convert_to<int>())+1;
        thr[21] = pbc_csr.port_11.cfg_account_pg_21.reserved_min().convert_to<uint32_t>();
        mtu[21] = (pbc_csr.port_11.cfg_account_mtu_table.pg21().convert_to<int>())+1;
        thr[22] = pbc_csr.port_11.cfg_account_pg_22.reserved_min().convert_to<uint32_t>();
        mtu[22] = (pbc_csr.port_11.cfg_account_mtu_table.pg22().convert_to<int>())+1;
        thr[23] = pbc_csr.port_11.cfg_account_pg_23.reserved_min().convert_to<uint32_t>();
        mtu[23] = (pbc_csr.port_11.cfg_account_mtu_table.pg23().convert_to<int>())+1;
        thr[24] = pbc_csr.port_11.cfg_account_pg_24.reserved_min().convert_to<uint32_t>();
        mtu[24] = (pbc_csr.port_11.cfg_account_mtu_table.pg24().convert_to<int>())+1;
        thr[25] = pbc_csr.port_11.cfg_account_pg_25.reserved_min().convert_to<uint32_t>();
        mtu[25] = (pbc_csr.port_11.cfg_account_mtu_table.pg25().convert_to<int>())+1;
        thr[26] = pbc_csr.port_11.cfg_account_pg_26.reserved_min().convert_to<uint32_t>();
        mtu[26] = (pbc_csr.port_11.cfg_account_mtu_table.pg26().convert_to<int>())+1;
        thr[27] = pbc_csr.port_11.cfg_account_pg_27.reserved_min().convert_to<uint32_t>();
        mtu[27] = (pbc_csr.port_11.cfg_account_mtu_table.pg27().convert_to<int>())+1;
        thr[28] = pbc_csr.port_11.cfg_account_pg_28.reserved_min().convert_to<uint32_t>();
        mtu[28] = (pbc_csr.port_11.cfg_account_mtu_table.pg28().convert_to<int>())+1;
        thr[29] = pbc_csr.port_11.cfg_account_pg_29.reserved_min().convert_to<uint32_t>();
        mtu[29] = (pbc_csr.port_11.cfg_account_mtu_table.pg29().convert_to<int>())+1;
        thr[30] = pbc_csr.port_11.cfg_account_pg_30.reserved_min().convert_to<uint32_t>();
        mtu[30] = (pbc_csr.port_11.cfg_account_mtu_table.pg30().convert_to<int>())+1;
        thr[31] = pbc_csr.port_11.cfg_account_pg_31.reserved_min().convert_to<uint32_t>();
        mtu[31] = (pbc_csr.port_11.cfg_account_mtu_table.pg31().convert_to<int>())+1;
       pg32_map.all(pbc_csr.port_11.cfg_account_tc_to_pg.table()); 

       // program credits
       cap_pbcport11_csr_dhs_oq_flow_control_entry_t tmp11;
       for (int ii=0; ii<pbc_csr.port_11.dhs_oq_flow_control.get_depth_entry(); ii++) {
          if ( ((credit_en >> ii)& 1) ==0) {
             pbc_csr.port_11.dhs_oq_flow_control.entry[ii].entry(0);
          } else {
             tmp11.set_name(pbc_csr.port_11.dhs_oq_flow_control.entry[ii].get_hier_path()+".credits_over_min");
             tmp11.init();
             tmp11.load_from_cfg(0);
             //tmp11.show();
             int credits_over_min = tmp11.all().convert_to<int>();
             credits = mtu_max + max_growth_map.max_growth(ii).convert_to<int>() + credits_over_min;
             PLOG_MSG(     "iq: "<<ii<<
                           " credits="<<credits<<
                           " credits_over_min="<<credits_over_min<< 
                           " mtu_max="<<mtu_max<< 
                           " max_growth=" <<max_growth_map.max_growth(ii)<<
                           endl);
             pbc_csr.port_11.dhs_oq_flow_control.entry[ii].entry(credits);
             // set rsv min appropriately
             if (pg_thr_consistent_with_credits) { // ii is the iq
                    int pg = pg32_map.pg(ii).convert_to<int>();
                    thr_update = (thr_update | (1 << pg));
                    thr[pg] = std::ceil((credits + mtu[pg])/4.0);
                    PLOG_MSG(     "iq: "<<ii<<
                                  " pg="<<pg<<
                                  " thr="<<thr[pg]<< 
                                  " credits="<<credits<<
                                  " mtu="<<mtu[pg]<< 
                                  endl);
             }
          }
          pbc_csr.port_11.dhs_oq_flow_control.entry[ii].set_access_no_zero_time(1);
          pbc_csr.port_11.dhs_oq_flow_control.entry[ii].write();

       }
       for (int ii=0; ii<pbc_csr.port_11.dhs_oq_flow_control.get_depth_entry(); ii++) {
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.dhs_oq_flow_control.entry[ii].show();}
       }
       if (pg_thr_consistent_with_credits) {
          if ((thr_update >> 0 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_0.reserved_min(thr[0]);
             pbc_csr.port_11.cfg_account_pg_0.headroom(0);
             pbc_csr.port_11.cfg_account_pg_0.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_0.show();}
          }
          if ((thr_update >> 1 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_1.reserved_min(thr[1]);
             pbc_csr.port_11.cfg_account_pg_1.headroom(0);
             pbc_csr.port_11.cfg_account_pg_1.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_1.show();}
          }
          if ((thr_update >> 2 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_2.reserved_min(thr[2]);
             pbc_csr.port_11.cfg_account_pg_2.headroom(0);
             pbc_csr.port_11.cfg_account_pg_2.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_2.show();}
          }
          if ((thr_update >> 3 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_3.reserved_min(thr[3]);
             pbc_csr.port_11.cfg_account_pg_3.headroom(0);
             pbc_csr.port_11.cfg_account_pg_3.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_3.show();}
          }
          if ((thr_update >> 4 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_4.reserved_min(thr[4]);
             pbc_csr.port_11.cfg_account_pg_4.headroom(0);
             pbc_csr.port_11.cfg_account_pg_4.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_4.show();}
          }
          if ((thr_update >> 5 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_5.reserved_min(thr[5]);
             pbc_csr.port_11.cfg_account_pg_5.headroom(0);
             pbc_csr.port_11.cfg_account_pg_5.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_5.show();}
          }
          if ((thr_update >> 6 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_6.reserved_min(thr[6]);
             pbc_csr.port_11.cfg_account_pg_6.headroom(0);
             pbc_csr.port_11.cfg_account_pg_6.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_6.show();}
          }
          if ((thr_update >> 7 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_7.reserved_min(thr[7]);
             pbc_csr.port_11.cfg_account_pg_7.headroom(0);
             pbc_csr.port_11.cfg_account_pg_7.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_7.show();}
          }
          if ((thr_update >> 8 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_8.reserved_min(thr[8]);
             pbc_csr.port_11.cfg_account_pg_8.headroom(0);
             pbc_csr.port_11.cfg_account_pg_8.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_8.show();}
          }
          if ((thr_update >> 9 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_9.reserved_min(thr[9]);
             pbc_csr.port_11.cfg_account_pg_9.headroom(0);
             pbc_csr.port_11.cfg_account_pg_9.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_9.show();}
          }
          if ((thr_update >> 10 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_10.reserved_min(thr[10]);
             pbc_csr.port_11.cfg_account_pg_10.headroom(0);
             pbc_csr.port_11.cfg_account_pg_10.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_10.show();}
          }
          if ((thr_update >> 11 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_11.reserved_min(thr[11]);
             pbc_csr.port_11.cfg_account_pg_11.headroom(0);
             pbc_csr.port_11.cfg_account_pg_11.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_11.show();}
          }
          if ((thr_update >> 12 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_12.reserved_min(thr[12]);
             pbc_csr.port_11.cfg_account_pg_12.headroom(0);
             pbc_csr.port_11.cfg_account_pg_12.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_12.show();}
          }
          if ((thr_update >> 13 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_13.reserved_min(thr[13]);
             pbc_csr.port_11.cfg_account_pg_13.headroom(0);
             pbc_csr.port_11.cfg_account_pg_13.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_13.show();}
          }
          if ((thr_update >> 14 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_14.reserved_min(thr[14]);
             pbc_csr.port_11.cfg_account_pg_14.headroom(0);
             pbc_csr.port_11.cfg_account_pg_14.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_14.show();}
          }
          if ((thr_update >> 15 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_15.reserved_min(thr[15]);
             pbc_csr.port_11.cfg_account_pg_15.headroom(0);
             pbc_csr.port_11.cfg_account_pg_15.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_15.show();}
          }
          if ((thr_update >> 16 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_16.reserved_min(thr[16]);
             pbc_csr.port_11.cfg_account_pg_16.headroom(0);
             pbc_csr.port_11.cfg_account_pg_16.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_16.show();}
          }
          if ((thr_update >> 17 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_17.reserved_min(thr[17]);
             pbc_csr.port_11.cfg_account_pg_17.headroom(0);
             pbc_csr.port_11.cfg_account_pg_17.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_17.show();}
          }
          if ((thr_update >> 18 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_18.reserved_min(thr[18]);
             pbc_csr.port_11.cfg_account_pg_18.headroom(0);
             pbc_csr.port_11.cfg_account_pg_18.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_18.show();}
          }
          if ((thr_update >> 19 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_19.reserved_min(thr[19]);
             pbc_csr.port_11.cfg_account_pg_19.headroom(0);
             pbc_csr.port_11.cfg_account_pg_19.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_19.show();}
          }
          if ((thr_update >> 20 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_20.reserved_min(thr[20]);
             pbc_csr.port_11.cfg_account_pg_20.headroom(0);
             pbc_csr.port_11.cfg_account_pg_20.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_20.show();}
          }
          if ((thr_update >> 21 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_21.reserved_min(thr[21]);
             pbc_csr.port_11.cfg_account_pg_21.headroom(0);
             pbc_csr.port_11.cfg_account_pg_21.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_21.show();}
          }
          if ((thr_update >> 22 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_22.reserved_min(thr[22]);
             pbc_csr.port_11.cfg_account_pg_22.headroom(0);
             pbc_csr.port_11.cfg_account_pg_22.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_22.show();}
          }
          if ((thr_update >> 23 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_23.reserved_min(thr[23]);
             pbc_csr.port_11.cfg_account_pg_23.headroom(0);
             pbc_csr.port_11.cfg_account_pg_23.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_23.show();}
          }
          if ((thr_update >> 24 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_24.reserved_min(thr[24]);
             pbc_csr.port_11.cfg_account_pg_24.headroom(0);
             pbc_csr.port_11.cfg_account_pg_24.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_24.show();}
          }
          if ((thr_update >> 25 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_25.reserved_min(thr[25]);
             pbc_csr.port_11.cfg_account_pg_25.headroom(0);
             pbc_csr.port_11.cfg_account_pg_25.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_25.show();}
          }
          if ((thr_update >> 26 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_26.reserved_min(thr[26]);
             pbc_csr.port_11.cfg_account_pg_26.headroom(0);
             pbc_csr.port_11.cfg_account_pg_26.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_26.show();}
          }
          if ((thr_update >> 27 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_27.reserved_min(thr[27]);
             pbc_csr.port_11.cfg_account_pg_27.headroom(0);
             pbc_csr.port_11.cfg_account_pg_27.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_27.show();}
          }
          if ((thr_update >> 28 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_28.reserved_min(thr[28]);
             pbc_csr.port_11.cfg_account_pg_28.headroom(0);
             pbc_csr.port_11.cfg_account_pg_28.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_28.show();}
          }
          if ((thr_update >> 29 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_29.reserved_min(thr[29]);
             pbc_csr.port_11.cfg_account_pg_29.headroom(0);
             pbc_csr.port_11.cfg_account_pg_29.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_29.show();}
          }
          if ((thr_update >> 30 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_30.reserved_min(thr[30]);
             pbc_csr.port_11.cfg_account_pg_30.headroom(0);
             pbc_csr.port_11.cfg_account_pg_30.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_30.show();}
          }
          if ((thr_update >> 31 ) & 1) {
             pbc_csr.port_11.cfg_account_pg_31.reserved_min(thr[31]);
             pbc_csr.port_11.cfg_account_pg_31.headroom(0);
             pbc_csr.port_11.cfg_account_pg_31.write();
             if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_31.show();}
          }
       }
       // program flush
          pbc_csr.port_11.cfg_oq_queue.flush(pbc_csr.port_11.cfg_oq_queue.flush().convert_to<uint32_t>() | ~(pbc_csr.port_11.cfg_account_credit_return.enable().convert_to<uint32_t>() | pbc_csr.port_11.cfg_oq_queue.recirc().convert_to<uint32_t>()));
          pbc_csr.port_11.cfg_oq_queue.write();
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_oq_queue.show();}

    }
    // pbc_csr.cfg_credits.load_from_cfg();
    // if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.cfg_credits.show();}

  uint32_t max_growth_max=0,m;
       max_growth_map.all(pbc_csr.cfg_credits_max_growth_10.cells());
       for (int i=0 ; i<32; i++) {
          m=max_growth_map.max_growth(i).convert_to<uint32_t>();
          if (m > max_growth_max) { max_growth_max = m;}
       }
       max_growth_map.all(pbc_csr.cfg_credits_max_growth_11.cells());
       for (int i=0 ; i<32; i++) {
          m=max_growth_map.max_growth(i).convert_to<uint32_t>();
          if (m > max_growth_max) { max_growth_max = m;}
       }
    pbc_csr.port_10.cfg_oq.span_cpu_num_cells(max_growth_max+mtu_max);
    pbc_csr.port_10.cfg_oq.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_oq.show();}

    cfg_found_cnt = pbc_csr.hbm.hbm_port_0.cfg_hbm_tc_to_q.load_from_cfg_rtn_status(0);
    pg8_map.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_tc_to_q.table()); // init
    pg8_map.set_name(pbc_csr.hbm.hbm_port_0.cfg_hbm_tc_to_q.get_hier_path()+".decoder");
    cfg_found_cnt += pg8_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.hbm.hbm_port_0.cfg_hbm_tc_to_q.table(pg8_map.all());
       pbc_csr.hbm.hbm_port_0.cfg_hbm_tc_to_q.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_0.cfg_hbm_tc_to_q.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pg8_map.show();}
    cfg_found_cnt = pbc_csr.hbm.hbm_port_1.cfg_hbm_tc_to_q.load_from_cfg_rtn_status(0);
    pg8_map.all(pbc_csr.hbm.hbm_port_1.cfg_hbm_tc_to_q.table()); // init
    pg8_map.set_name(pbc_csr.hbm.hbm_port_1.cfg_hbm_tc_to_q.get_hier_path()+".decoder");
    cfg_found_cnt += pg8_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.hbm.hbm_port_1.cfg_hbm_tc_to_q.table(pg8_map.all());
       pbc_csr.hbm.hbm_port_1.cfg_hbm_tc_to_q.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_1.cfg_hbm_tc_to_q.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pg8_map.show();}
    cfg_found_cnt = pbc_csr.hbm.hbm_port_2.cfg_hbm_tc_to_q.load_from_cfg_rtn_status(0);
    pg8_map.all(pbc_csr.hbm.hbm_port_2.cfg_hbm_tc_to_q.table()); // init
    pg8_map.set_name(pbc_csr.hbm.hbm_port_2.cfg_hbm_tc_to_q.get_hier_path()+".decoder");
    cfg_found_cnt += pg8_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.hbm.hbm_port_2.cfg_hbm_tc_to_q.table(pg8_map.all());
       pbc_csr.hbm.hbm_port_2.cfg_hbm_tc_to_q.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_2.cfg_hbm_tc_to_q.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pg8_map.show();}
    cfg_found_cnt = pbc_csr.hbm.hbm_port_3.cfg_hbm_tc_to_q.load_from_cfg_rtn_status(0);
    pg8_map.all(pbc_csr.hbm.hbm_port_3.cfg_hbm_tc_to_q.table()); // init
    pg8_map.set_name(pbc_csr.hbm.hbm_port_3.cfg_hbm_tc_to_q.get_hier_path()+".decoder");
    cfg_found_cnt += pg8_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.hbm.hbm_port_3.cfg_hbm_tc_to_q.table(pg8_map.all());
       pbc_csr.hbm.hbm_port_3.cfg_hbm_tc_to_q.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_3.cfg_hbm_tc_to_q.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pg8_map.show();}
    cfg_found_cnt = pbc_csr.hbm.hbm_port_4.cfg_hbm_tc_to_q.load_from_cfg_rtn_status(0);
    pg8_map.all(pbc_csr.hbm.hbm_port_4.cfg_hbm_tc_to_q.table()); // init
    pg8_map.set_name(pbc_csr.hbm.hbm_port_4.cfg_hbm_tc_to_q.get_hier_path()+".decoder");
    cfg_found_cnt += pg8_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.hbm.hbm_port_4.cfg_hbm_tc_to_q.table(pg8_map.all());
       pbc_csr.hbm.hbm_port_4.cfg_hbm_tc_to_q.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_4.cfg_hbm_tc_to_q.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pg8_map.show();}
    cfg_found_cnt = pbc_csr.hbm.hbm_port_5.cfg_hbm_tc_to_q.load_from_cfg_rtn_status(0);
    pg8_map.all(pbc_csr.hbm.hbm_port_5.cfg_hbm_tc_to_q.table()); // init
    pg8_map.set_name(pbc_csr.hbm.hbm_port_5.cfg_hbm_tc_to_q.get_hier_path()+".decoder");
    cfg_found_cnt += pg8_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.hbm.hbm_port_5.cfg_hbm_tc_to_q.table(pg8_map.all());
       pbc_csr.hbm.hbm_port_5.cfg_hbm_tc_to_q.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_5.cfg_hbm_tc_to_q.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pg8_map.show();}
    cfg_found_cnt = pbc_csr.hbm.hbm_port_6.cfg_hbm_tc_to_q.load_from_cfg_rtn_status(0);
    pg8_map.all(pbc_csr.hbm.hbm_port_6.cfg_hbm_tc_to_q.table()); // init
    pg8_map.set_name(pbc_csr.hbm.hbm_port_6.cfg_hbm_tc_to_q.get_hier_path()+".decoder");
    cfg_found_cnt += pg8_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.hbm.hbm_port_6.cfg_hbm_tc_to_q.table(pg8_map.all());
       pbc_csr.hbm.hbm_port_6.cfg_hbm_tc_to_q.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_6.cfg_hbm_tc_to_q.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pg8_map.show();}
    cfg_found_cnt = pbc_csr.hbm.hbm_port_7.cfg_hbm_tc_to_q.load_from_cfg_rtn_status(0);
    pg8_map.all(pbc_csr.hbm.hbm_port_7.cfg_hbm_tc_to_q.table()); // init
    pg8_map.set_name(pbc_csr.hbm.hbm_port_7.cfg_hbm_tc_to_q.get_hier_path()+".decoder");
    cfg_found_cnt += pg8_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.hbm.hbm_port_7.cfg_hbm_tc_to_q.table(pg8_map.all());
       pbc_csr.hbm.hbm_port_7.cfg_hbm_tc_to_q.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_7.cfg_hbm_tc_to_q.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pg8_map.show();}
    cfg_found_cnt = pbc_csr.hbm.hbm_port_8.cfg_hbm_tc_to_q.load_from_cfg_rtn_status(0);
    pg8_map.all(pbc_csr.hbm.hbm_port_8.cfg_hbm_tc_to_q.table()); // init
    pg8_map.set_name(pbc_csr.hbm.hbm_port_8.cfg_hbm_tc_to_q.get_hier_path()+".decoder");
    cfg_found_cnt += pg8_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.hbm.hbm_port_8.cfg_hbm_tc_to_q.table(pg8_map.all());
       pbc_csr.hbm.hbm_port_8.cfg_hbm_tc_to_q.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_8.cfg_hbm_tc_to_q.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pg8_map.show();}
    cfg_found_cnt = pbc_csr.hbm.hbm_port_9.cfg_hbm_tc_to_q.load_from_cfg_rtn_status(0);
    pg16_map.all(pbc_csr.hbm.hbm_port_9.cfg_hbm_tc_to_q.table()); // init
    pg16_map.set_name(pbc_csr.hbm.hbm_port_9.cfg_hbm_tc_to_q.get_hier_path()+".decoder");
    cfg_found_cnt += pg16_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.hbm.hbm_port_9.cfg_hbm_tc_to_q.table(pg16_map.all());
       pbc_csr.hbm.hbm_port_9.cfg_hbm_tc_to_q.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_9.cfg_hbm_tc_to_q.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pg16_map.show();}
     pbc_csr.hbm.hbm_port_0.cfg_hbm.load_from_cfg();
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_0.cfg_hbm.show();}
     pbc_csr.hbm.hbm_port_1.cfg_hbm.load_from_cfg();
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_1.cfg_hbm.show();}
     pbc_csr.hbm.hbm_port_2.cfg_hbm.load_from_cfg();
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_2.cfg_hbm.show();}
     pbc_csr.hbm.hbm_port_3.cfg_hbm.load_from_cfg();
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_3.cfg_hbm.show();}
     pbc_csr.hbm.hbm_port_4.cfg_hbm.load_from_cfg();
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_4.cfg_hbm.show();}
     pbc_csr.hbm.hbm_port_5.cfg_hbm.load_from_cfg();
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_5.cfg_hbm.show();}
     pbc_csr.hbm.hbm_port_6.cfg_hbm.load_from_cfg();
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_6.cfg_hbm.show();}
     pbc_csr.hbm.hbm_port_7.cfg_hbm.load_from_cfg();
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_7.cfg_hbm.show();}
     pbc_csr.hbm.hbm_port_8.cfg_hbm.load_from_cfg();
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_8.cfg_hbm.show();}
     pbc_csr.hbm.hbm_port_9.cfg_hbm.load_from_cfg();
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_9.cfg_hbm.show();}
     pbc_csr.hbm.hbm_port_0.cfg_hbm_parser.load_from_cfg();
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_0.cfg_hbm_parser.show();}
     pbc_csr.hbm.hbm_port_1.cfg_hbm_parser.load_from_cfg();
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_1.cfg_hbm_parser.show();}
     pbc_csr.hbm.hbm_port_2.cfg_hbm_parser.load_from_cfg();
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_2.cfg_hbm_parser.show();}
     pbc_csr.hbm.hbm_port_3.cfg_hbm_parser.load_from_cfg();
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_3.cfg_hbm_parser.show();}
     pbc_csr.hbm.hbm_port_4.cfg_hbm_parser.load_from_cfg();
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_4.cfg_hbm_parser.show();}
     pbc_csr.hbm.hbm_port_5.cfg_hbm_parser.load_from_cfg();
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_5.cfg_hbm_parser.show();}
     pbc_csr.hbm.hbm_port_6.cfg_hbm_parser.load_from_cfg();
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_6.cfg_hbm_parser.show();}
     pbc_csr.hbm.hbm_port_7.cfg_hbm_parser.load_from_cfg();
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_7.cfg_hbm_parser.show();}
     pbc_csr.hbm.hbm_port_8.cfg_hbm_parser.load_from_cfg();
     if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_8.cfg_hbm_parser.show();}
    spath = pbc_csr.get_hier_path()+".hbm.hbm_thr_from_api";
    std::replace( spath.begin(), spath.end(), '.', '/');
    unsigned int  hbm_thr_from_api = sknobs_get_value((char *)spath.c_str(), 0);
    if (hbm_thr_from_api) {
       cap_pb_hbm_thr_from_api(chip_id,inst_id);
    } else {
      cap_pbc_hbm_tx_ctl_t rxd_hbm_ctl;
      cap_pbc_hbm_eth_ctl_t eth_hbm_ctl;
      cap_pbc_hbm_eth_port_ctl_t eth_hbm_port_ctl;
       rxd_hbm_ctl.init();
       cfg_found_cnt = pbc_csr.hbm.cfg_hbm_tx_ctrl.load_from_cfg_rtn_status(0);
       rxd_hbm_ctl.all(pbc_csr.hbm.cfg_hbm_tx_ctrl.all()); // init
       rxd_hbm_ctl.set_name(pbc_csr.hbm.cfg_hbm_tx_ctrl.get_hier_path()+".decoder");
       cfg_found_cnt += rxd_hbm_ctl.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.hbm.cfg_hbm_tx_ctrl.all(rxd_hbm_ctl.all());
          pbc_csr.hbm.cfg_hbm_tx_ctrl.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.cfg_hbm_tx_ctrl.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {rxd_hbm_ctl.show();}
       rxd_hbm_ctl.init();
       cfg_found_cnt = pbc_csr.hbm.cfg_hbm_tx_payload.load_from_cfg_rtn_status(0);
       rxd_hbm_ctl.all(pbc_csr.hbm.cfg_hbm_tx_payload.all()); // init
       rxd_hbm_ctl.set_name(pbc_csr.hbm.cfg_hbm_tx_payload.get_hier_path()+".decoder");
       cfg_found_cnt += rxd_hbm_ctl.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.hbm.cfg_hbm_tx_payload.all(rxd_hbm_ctl.all());
          pbc_csr.hbm.cfg_hbm_tx_payload.write();
          pbc_csr.hbm.hbm_port_9.cfg_hbm_tx_payload.all(rxd_hbm_ctl.all());
          pbc_csr.hbm.hbm_port_9.cfg_hbm_tx_payload.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.cfg_hbm_tx_payload.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {rxd_hbm_ctl.show();}
        pbc_csr.hbm.hbm_port_9.cfg_hbm_context.load_from_cfg();
        if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_9.cfg_hbm_context.show();}
       eth_hbm_ctl.init();
       cfg_found_cnt = pbc_csr.hbm.cfg_hbm_eth_ctrl.load_from_cfg_rtn_status(0);
       eth_hbm_ctl.all(pbc_csr.hbm.cfg_hbm_eth_ctrl.all()); // init
       eth_hbm_ctl.set_name(pbc_csr.hbm.cfg_hbm_eth_ctrl.get_hier_path()+".decoder");
       cfg_found_cnt += eth_hbm_ctl.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.hbm.cfg_hbm_eth_ctrl.all(eth_hbm_ctl.all());
          pbc_csr.hbm.cfg_hbm_eth_ctrl.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.cfg_hbm_eth_ctrl.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_ctl.show();}
       eth_hbm_ctl.init();
       cfg_found_cnt = pbc_csr.hbm.cfg_hbm_eth_payload.load_from_cfg_rtn_status(0);
       eth_hbm_ctl.all(pbc_csr.hbm.cfg_hbm_eth_payload.all()); // init
       eth_hbm_ctl.set_name(pbc_csr.hbm.cfg_hbm_eth_payload.get_hier_path()+".decoder");
       cfg_found_cnt += eth_hbm_ctl.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.hbm.cfg_hbm_eth_payload.all(eth_hbm_ctl.all());
          pbc_csr.hbm.cfg_hbm_eth_payload.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.cfg_hbm_eth_payload.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_ctl.show();}
       eth_hbm_port_ctl.init();
       cfg_found_cnt = pbc_csr.hbm.hbm_port_0.cfg_hbm_eth_ctrl.load_from_cfg_rtn_status(0);
       eth_hbm_port_ctl.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_eth_ctrl.all()); // init
       eth_hbm_port_ctl.set_name(pbc_csr.hbm.hbm_port_0.cfg_hbm_eth_ctrl.get_hier_path()+".decoder");
       cfg_found_cnt += eth_hbm_port_ctl.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.hbm.hbm_port_0.cfg_hbm_eth_ctrl.all(eth_hbm_port_ctl.all());
          pbc_csr.hbm.hbm_port_0.cfg_hbm_eth_ctrl.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_0.cfg_hbm_eth_ctrl.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_ctl.show();}
       eth_hbm_port_ctl.init();
       cfg_found_cnt = pbc_csr.hbm.hbm_port_0.cfg_hbm_eth_payload.load_from_cfg_rtn_status(0);
       eth_hbm_port_ctl.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_eth_payload.all()); // init
       eth_hbm_port_ctl.set_name(pbc_csr.hbm.hbm_port_0.cfg_hbm_eth_payload.get_hier_path()+".decoder");
       cfg_found_cnt += eth_hbm_port_ctl.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.hbm.hbm_port_0.cfg_hbm_eth_payload.all(eth_hbm_port_ctl.all());
          pbc_csr.hbm.hbm_port_0.cfg_hbm_eth_payload.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_0.cfg_hbm_eth_payload.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_ctl.show();}
       eth_hbm_port_ctl.init();
       cfg_found_cnt = pbc_csr.hbm.hbm_port_1.cfg_hbm_eth_ctrl.load_from_cfg_rtn_status(0);
       eth_hbm_port_ctl.all(pbc_csr.hbm.hbm_port_1.cfg_hbm_eth_ctrl.all()); // init
       eth_hbm_port_ctl.set_name(pbc_csr.hbm.hbm_port_1.cfg_hbm_eth_ctrl.get_hier_path()+".decoder");
       cfg_found_cnt += eth_hbm_port_ctl.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.hbm.hbm_port_1.cfg_hbm_eth_ctrl.all(eth_hbm_port_ctl.all());
          pbc_csr.hbm.hbm_port_1.cfg_hbm_eth_ctrl.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_1.cfg_hbm_eth_ctrl.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_ctl.show();}
       eth_hbm_port_ctl.init();
       cfg_found_cnt = pbc_csr.hbm.hbm_port_1.cfg_hbm_eth_payload.load_from_cfg_rtn_status(0);
       eth_hbm_port_ctl.all(pbc_csr.hbm.hbm_port_1.cfg_hbm_eth_payload.all()); // init
       eth_hbm_port_ctl.set_name(pbc_csr.hbm.hbm_port_1.cfg_hbm_eth_payload.get_hier_path()+".decoder");
       cfg_found_cnt += eth_hbm_port_ctl.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.hbm.hbm_port_1.cfg_hbm_eth_payload.all(eth_hbm_port_ctl.all());
          pbc_csr.hbm.hbm_port_1.cfg_hbm_eth_payload.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_1.cfg_hbm_eth_payload.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_ctl.show();}
       eth_hbm_port_ctl.init();
       cfg_found_cnt = pbc_csr.hbm.hbm_port_2.cfg_hbm_eth_ctrl.load_from_cfg_rtn_status(0);
       eth_hbm_port_ctl.all(pbc_csr.hbm.hbm_port_2.cfg_hbm_eth_ctrl.all()); // init
       eth_hbm_port_ctl.set_name(pbc_csr.hbm.hbm_port_2.cfg_hbm_eth_ctrl.get_hier_path()+".decoder");
       cfg_found_cnt += eth_hbm_port_ctl.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.hbm.hbm_port_2.cfg_hbm_eth_ctrl.all(eth_hbm_port_ctl.all());
          pbc_csr.hbm.hbm_port_2.cfg_hbm_eth_ctrl.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_2.cfg_hbm_eth_ctrl.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_ctl.show();}
       eth_hbm_port_ctl.init();
       cfg_found_cnt = pbc_csr.hbm.hbm_port_2.cfg_hbm_eth_payload.load_from_cfg_rtn_status(0);
       eth_hbm_port_ctl.all(pbc_csr.hbm.hbm_port_2.cfg_hbm_eth_payload.all()); // init
       eth_hbm_port_ctl.set_name(pbc_csr.hbm.hbm_port_2.cfg_hbm_eth_payload.get_hier_path()+".decoder");
       cfg_found_cnt += eth_hbm_port_ctl.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.hbm.hbm_port_2.cfg_hbm_eth_payload.all(eth_hbm_port_ctl.all());
          pbc_csr.hbm.hbm_port_2.cfg_hbm_eth_payload.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_2.cfg_hbm_eth_payload.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_ctl.show();}
       eth_hbm_port_ctl.init();
       cfg_found_cnt = pbc_csr.hbm.hbm_port_3.cfg_hbm_eth_ctrl.load_from_cfg_rtn_status(0);
       eth_hbm_port_ctl.all(pbc_csr.hbm.hbm_port_3.cfg_hbm_eth_ctrl.all()); // init
       eth_hbm_port_ctl.set_name(pbc_csr.hbm.hbm_port_3.cfg_hbm_eth_ctrl.get_hier_path()+".decoder");
       cfg_found_cnt += eth_hbm_port_ctl.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.hbm.hbm_port_3.cfg_hbm_eth_ctrl.all(eth_hbm_port_ctl.all());
          pbc_csr.hbm.hbm_port_3.cfg_hbm_eth_ctrl.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_3.cfg_hbm_eth_ctrl.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_ctl.show();}
       eth_hbm_port_ctl.init();
       cfg_found_cnt = pbc_csr.hbm.hbm_port_3.cfg_hbm_eth_payload.load_from_cfg_rtn_status(0);
       eth_hbm_port_ctl.all(pbc_csr.hbm.hbm_port_3.cfg_hbm_eth_payload.all()); // init
       eth_hbm_port_ctl.set_name(pbc_csr.hbm.hbm_port_3.cfg_hbm_eth_payload.get_hier_path()+".decoder");
       cfg_found_cnt += eth_hbm_port_ctl.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.hbm.hbm_port_3.cfg_hbm_eth_payload.all(eth_hbm_port_ctl.all());
          pbc_csr.hbm.hbm_port_3.cfg_hbm_eth_payload.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_3.cfg_hbm_eth_payload.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_ctl.show();}
       eth_hbm_port_ctl.init();
       cfg_found_cnt = pbc_csr.hbm.hbm_port_4.cfg_hbm_eth_ctrl.load_from_cfg_rtn_status(0);
       eth_hbm_port_ctl.all(pbc_csr.hbm.hbm_port_4.cfg_hbm_eth_ctrl.all()); // init
       eth_hbm_port_ctl.set_name(pbc_csr.hbm.hbm_port_4.cfg_hbm_eth_ctrl.get_hier_path()+".decoder");
       cfg_found_cnt += eth_hbm_port_ctl.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.hbm.hbm_port_4.cfg_hbm_eth_ctrl.all(eth_hbm_port_ctl.all());
          pbc_csr.hbm.hbm_port_4.cfg_hbm_eth_ctrl.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_4.cfg_hbm_eth_ctrl.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_ctl.show();}
       eth_hbm_port_ctl.init();
       cfg_found_cnt = pbc_csr.hbm.hbm_port_4.cfg_hbm_eth_payload.load_from_cfg_rtn_status(0);
       eth_hbm_port_ctl.all(pbc_csr.hbm.hbm_port_4.cfg_hbm_eth_payload.all()); // init
       eth_hbm_port_ctl.set_name(pbc_csr.hbm.hbm_port_4.cfg_hbm_eth_payload.get_hier_path()+".decoder");
       cfg_found_cnt += eth_hbm_port_ctl.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.hbm.hbm_port_4.cfg_hbm_eth_payload.all(eth_hbm_port_ctl.all());
          pbc_csr.hbm.hbm_port_4.cfg_hbm_eth_payload.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_4.cfg_hbm_eth_payload.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_ctl.show();}
       eth_hbm_port_ctl.init();
       cfg_found_cnt = pbc_csr.hbm.hbm_port_5.cfg_hbm_eth_ctrl.load_from_cfg_rtn_status(0);
       eth_hbm_port_ctl.all(pbc_csr.hbm.hbm_port_5.cfg_hbm_eth_ctrl.all()); // init
       eth_hbm_port_ctl.set_name(pbc_csr.hbm.hbm_port_5.cfg_hbm_eth_ctrl.get_hier_path()+".decoder");
       cfg_found_cnt += eth_hbm_port_ctl.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.hbm.hbm_port_5.cfg_hbm_eth_ctrl.all(eth_hbm_port_ctl.all());
          pbc_csr.hbm.hbm_port_5.cfg_hbm_eth_ctrl.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_5.cfg_hbm_eth_ctrl.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_ctl.show();}
       eth_hbm_port_ctl.init();
       cfg_found_cnt = pbc_csr.hbm.hbm_port_5.cfg_hbm_eth_payload.load_from_cfg_rtn_status(0);
       eth_hbm_port_ctl.all(pbc_csr.hbm.hbm_port_5.cfg_hbm_eth_payload.all()); // init
       eth_hbm_port_ctl.set_name(pbc_csr.hbm.hbm_port_5.cfg_hbm_eth_payload.get_hier_path()+".decoder");
       cfg_found_cnt += eth_hbm_port_ctl.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.hbm.hbm_port_5.cfg_hbm_eth_payload.all(eth_hbm_port_ctl.all());
          pbc_csr.hbm.hbm_port_5.cfg_hbm_eth_payload.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_5.cfg_hbm_eth_payload.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_ctl.show();}
       eth_hbm_port_ctl.init();
       cfg_found_cnt = pbc_csr.hbm.hbm_port_6.cfg_hbm_eth_ctrl.load_from_cfg_rtn_status(0);
       eth_hbm_port_ctl.all(pbc_csr.hbm.hbm_port_6.cfg_hbm_eth_ctrl.all()); // init
       eth_hbm_port_ctl.set_name(pbc_csr.hbm.hbm_port_6.cfg_hbm_eth_ctrl.get_hier_path()+".decoder");
       cfg_found_cnt += eth_hbm_port_ctl.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.hbm.hbm_port_6.cfg_hbm_eth_ctrl.all(eth_hbm_port_ctl.all());
          pbc_csr.hbm.hbm_port_6.cfg_hbm_eth_ctrl.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_6.cfg_hbm_eth_ctrl.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_ctl.show();}
       eth_hbm_port_ctl.init();
       cfg_found_cnt = pbc_csr.hbm.hbm_port_6.cfg_hbm_eth_payload.load_from_cfg_rtn_status(0);
       eth_hbm_port_ctl.all(pbc_csr.hbm.hbm_port_6.cfg_hbm_eth_payload.all()); // init
       eth_hbm_port_ctl.set_name(pbc_csr.hbm.hbm_port_6.cfg_hbm_eth_payload.get_hier_path()+".decoder");
       cfg_found_cnt += eth_hbm_port_ctl.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.hbm.hbm_port_6.cfg_hbm_eth_payload.all(eth_hbm_port_ctl.all());
          pbc_csr.hbm.hbm_port_6.cfg_hbm_eth_payload.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_6.cfg_hbm_eth_payload.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_ctl.show();}
       eth_hbm_port_ctl.init();
       cfg_found_cnt = pbc_csr.hbm.hbm_port_7.cfg_hbm_eth_ctrl.load_from_cfg_rtn_status(0);
       eth_hbm_port_ctl.all(pbc_csr.hbm.hbm_port_7.cfg_hbm_eth_ctrl.all()); // init
       eth_hbm_port_ctl.set_name(pbc_csr.hbm.hbm_port_7.cfg_hbm_eth_ctrl.get_hier_path()+".decoder");
       cfg_found_cnt += eth_hbm_port_ctl.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.hbm.hbm_port_7.cfg_hbm_eth_ctrl.all(eth_hbm_port_ctl.all());
          pbc_csr.hbm.hbm_port_7.cfg_hbm_eth_ctrl.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_7.cfg_hbm_eth_ctrl.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_ctl.show();}
       eth_hbm_port_ctl.init();
       cfg_found_cnt = pbc_csr.hbm.hbm_port_7.cfg_hbm_eth_payload.load_from_cfg_rtn_status(0);
       eth_hbm_port_ctl.all(pbc_csr.hbm.hbm_port_7.cfg_hbm_eth_payload.all()); // init
       eth_hbm_port_ctl.set_name(pbc_csr.hbm.hbm_port_7.cfg_hbm_eth_payload.get_hier_path()+".decoder");
       cfg_found_cnt += eth_hbm_port_ctl.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.hbm.hbm_port_7.cfg_hbm_eth_payload.all(eth_hbm_port_ctl.all());
          pbc_csr.hbm.hbm_port_7.cfg_hbm_eth_payload.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_7.cfg_hbm_eth_payload.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_ctl.show();}
       cap_pbc_hbm_eth_occ_thr_t eth_occ_thr;
       eth_occ_thr.init();
       cfg_found_cnt = pbc_csr.hbm.hbm_port_0.cfg_hbm_eth_payload_occupancy.load_from_cfg_rtn_status(0);
       eth_occ_thr.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_eth_payload_occupancy.all()); // init
       eth_occ_thr.set_name(pbc_csr.hbm.hbm_port_0.cfg_hbm_eth_payload_occupancy.get_hier_path()+".decoder");
       cfg_found_cnt += eth_occ_thr.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.hbm.hbm_port_0.cfg_hbm_eth_payload_occupancy.all(eth_occ_thr.all());
          pbc_csr.hbm.hbm_port_0.cfg_hbm_eth_payload_occupancy.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_0.cfg_hbm_eth_payload_occupancy.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_occ_thr.show();}
       cfg_found_cnt = pbc_csr.hbm.hbm_port_1.cfg_hbm_eth_payload_occupancy.load_from_cfg_rtn_status(0);
       eth_occ_thr.all(pbc_csr.hbm.hbm_port_1.cfg_hbm_eth_payload_occupancy.all()); // init
       eth_occ_thr.set_name(pbc_csr.hbm.hbm_port_1.cfg_hbm_eth_payload_occupancy.get_hier_path()+".decoder");
       cfg_found_cnt += eth_occ_thr.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.hbm.hbm_port_1.cfg_hbm_eth_payload_occupancy.all(eth_occ_thr.all());
          pbc_csr.hbm.hbm_port_1.cfg_hbm_eth_payload_occupancy.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_1.cfg_hbm_eth_payload_occupancy.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_occ_thr.show();}
       cfg_found_cnt = pbc_csr.hbm.hbm_port_2.cfg_hbm_eth_payload_occupancy.load_from_cfg_rtn_status(0);
       eth_occ_thr.all(pbc_csr.hbm.hbm_port_2.cfg_hbm_eth_payload_occupancy.all()); // init
       eth_occ_thr.set_name(pbc_csr.hbm.hbm_port_2.cfg_hbm_eth_payload_occupancy.get_hier_path()+".decoder");
       cfg_found_cnt += eth_occ_thr.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.hbm.hbm_port_2.cfg_hbm_eth_payload_occupancy.all(eth_occ_thr.all());
          pbc_csr.hbm.hbm_port_2.cfg_hbm_eth_payload_occupancy.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_2.cfg_hbm_eth_payload_occupancy.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_occ_thr.show();}
       cfg_found_cnt = pbc_csr.hbm.hbm_port_3.cfg_hbm_eth_payload_occupancy.load_from_cfg_rtn_status(0);
       eth_occ_thr.all(pbc_csr.hbm.hbm_port_3.cfg_hbm_eth_payload_occupancy.all()); // init
       eth_occ_thr.set_name(pbc_csr.hbm.hbm_port_3.cfg_hbm_eth_payload_occupancy.get_hier_path()+".decoder");
       cfg_found_cnt += eth_occ_thr.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.hbm.hbm_port_3.cfg_hbm_eth_payload_occupancy.all(eth_occ_thr.all());
          pbc_csr.hbm.hbm_port_3.cfg_hbm_eth_payload_occupancy.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_3.cfg_hbm_eth_payload_occupancy.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_occ_thr.show();}
       cfg_found_cnt = pbc_csr.hbm.hbm_port_4.cfg_hbm_eth_payload_occupancy.load_from_cfg_rtn_status(0);
       eth_occ_thr.all(pbc_csr.hbm.hbm_port_4.cfg_hbm_eth_payload_occupancy.all()); // init
       eth_occ_thr.set_name(pbc_csr.hbm.hbm_port_4.cfg_hbm_eth_payload_occupancy.get_hier_path()+".decoder");
       cfg_found_cnt += eth_occ_thr.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.hbm.hbm_port_4.cfg_hbm_eth_payload_occupancy.all(eth_occ_thr.all());
          pbc_csr.hbm.hbm_port_4.cfg_hbm_eth_payload_occupancy.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_4.cfg_hbm_eth_payload_occupancy.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_occ_thr.show();}
       cfg_found_cnt = pbc_csr.hbm.hbm_port_5.cfg_hbm_eth_payload_occupancy.load_from_cfg_rtn_status(0);
       eth_occ_thr.all(pbc_csr.hbm.hbm_port_5.cfg_hbm_eth_payload_occupancy.all()); // init
       eth_occ_thr.set_name(pbc_csr.hbm.hbm_port_5.cfg_hbm_eth_payload_occupancy.get_hier_path()+".decoder");
       cfg_found_cnt += eth_occ_thr.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.hbm.hbm_port_5.cfg_hbm_eth_payload_occupancy.all(eth_occ_thr.all());
          pbc_csr.hbm.hbm_port_5.cfg_hbm_eth_payload_occupancy.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_5.cfg_hbm_eth_payload_occupancy.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_occ_thr.show();}
       cfg_found_cnt = pbc_csr.hbm.hbm_port_6.cfg_hbm_eth_payload_occupancy.load_from_cfg_rtn_status(0);
       eth_occ_thr.all(pbc_csr.hbm.hbm_port_6.cfg_hbm_eth_payload_occupancy.all()); // init
       eth_occ_thr.set_name(pbc_csr.hbm.hbm_port_6.cfg_hbm_eth_payload_occupancy.get_hier_path()+".decoder");
       cfg_found_cnt += eth_occ_thr.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.hbm.hbm_port_6.cfg_hbm_eth_payload_occupancy.all(eth_occ_thr.all());
          pbc_csr.hbm.hbm_port_6.cfg_hbm_eth_payload_occupancy.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_6.cfg_hbm_eth_payload_occupancy.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_occ_thr.show();}
       cfg_found_cnt = pbc_csr.hbm.hbm_port_7.cfg_hbm_eth_payload_occupancy.load_from_cfg_rtn_status(0);
       eth_occ_thr.all(pbc_csr.hbm.hbm_port_7.cfg_hbm_eth_payload_occupancy.all()); // init
       eth_occ_thr.set_name(pbc_csr.hbm.hbm_port_7.cfg_hbm_eth_payload_occupancy.get_hier_path()+".decoder");
       cfg_found_cnt += eth_occ_thr.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.hbm.hbm_port_7.cfg_hbm_eth_payload_occupancy.all(eth_occ_thr.all());
          pbc_csr.hbm.hbm_port_7.cfg_hbm_eth_payload_occupancy.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_7.cfg_hbm_eth_payload_occupancy.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_occ_thr.show();}
       cap_pbc_hbm_tx_occ_thr_t tx_occ_thr;
       tx_occ_thr.init();
       cfg_found_cnt = pbc_csr.hbm.hbm_port_9.cfg_hbm_tx_payload_occupancy.load_from_cfg_rtn_status(0);
       tx_occ_thr.all(pbc_csr.hbm.hbm_port_9.cfg_hbm_tx_payload_occupancy.all()); // init
       tx_occ_thr.set_name(pbc_csr.hbm.hbm_port_9.cfg_hbm_tx_payload_occupancy.get_hier_path()+".decoder");
       cfg_found_cnt += tx_occ_thr.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.hbm.hbm_port_9.cfg_hbm_tx_payload_occupancy.all(tx_occ_thr.all());
          pbc_csr.hbm.hbm_port_9.cfg_hbm_tx_payload_occupancy.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_9.cfg_hbm_tx_payload_occupancy.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {tx_occ_thr.show();}

       cap_pbc_hbm_eth_xoff_thr_t eth_xoff_thr;
       eth_xoff_thr.init();
       cfg_found_cnt = pbc_csr.hbm.cfg_hbm_threshold.load_from_cfg_rtn_status(0);
       eth_xoff_thr.all(pbc_csr.hbm.cfg_hbm_threshold.all()); // init
       eth_xoff_thr.set_name(pbc_csr.hbm.cfg_hbm_threshold.get_hier_path()+".decoder");
       cfg_found_cnt += eth_xoff_thr.load_from_cfg_rtn_status(0);
       if (cfg_found_cnt > 0) {
          pbc_csr.hbm.cfg_hbm_threshold.all(eth_xoff_thr.all());
          pbc_csr.hbm.cfg_hbm_threshold.write();
       }
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.cfg_hbm_threshold.show();}
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_xoff_thr.show();}

       // eth context
       unsigned int context_based_on_def_cos;
       spath = pbc_csr.hbm.hbm_port_0.cfg_hbm_context.get_hier_path()+".set_based_on_default_cos";
       std::replace( spath.begin(), spath.end(), '.', '/');
       context_based_on_def_cos = sknobs_get_value((char *)spath.c_str(), 0);
       PLOG_MSG("context_based_on_def_cos for "<< spath << " returned " << context_based_on_def_cos << endl);
       if (context_based_on_def_cos) {
          pbc_csr.hbm.hbm_port_0.cfg_hbm_context.enable(1); // need to be packed
          pbc_csr.hbm.hbm_port_0.cfg_hbm_context.base(0*8);
          pbc_csr.hbm.hbm_port_0.cfg_hbm_context.no_drop(pbc_csr.hbm.hbm_port_0.cfg_hbm_context.enable().convert_to<uint32_t>());
          pbc_csr.hbm.hbm_port_0.cfg_hbm_context.write();
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_0.cfg_hbm_context.show();}
          pg8_map.set_name(pbc_csr.hbm.hbm_port_0.cfg_hbm_tc_to_q.get_hier_path()+".decoder");
          pg8_map.all(pbc_csr.hbm.hbm_port_0.cfg_hbm_tc_to_q.table()); // init
          pg8_map.pg(0,pbc_csr.hbm.hbm_port_0.cfg_hbm_parser.default_cos().convert_to<uint32_t>());
          pbc_csr.hbm.hbm_port_0.cfg_hbm_tc_to_q.table(pg8_map.all());
          pbc_csr.hbm.hbm_port_0.cfg_hbm_tc_to_q.write();
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_0.cfg_hbm_tc_to_q.show();}
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pg8_map.show();}
       } else {
          pbc_csr.hbm.hbm_port_0.cfg_hbm_context.load_from_cfg(0);
          pbc_csr.hbm.hbm_port_0.cfg_hbm_context.base(0*8);
          pbc_csr.hbm.hbm_port_0.cfg_hbm_context.write();
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_0.cfg_hbm_context.show();}
       }
       spath = pbc_csr.hbm.hbm_port_1.cfg_hbm_context.get_hier_path()+".set_based_on_default_cos";
       std::replace( spath.begin(), spath.end(), '.', '/');
       context_based_on_def_cos = sknobs_get_value((char *)spath.c_str(), 0);
       PLOG_MSG("context_based_on_def_cos for "<< spath << " returned " << context_based_on_def_cos << endl);
       if (context_based_on_def_cos) {
          pbc_csr.hbm.hbm_port_1.cfg_hbm_context.enable(1); // need to be packed
          pbc_csr.hbm.hbm_port_1.cfg_hbm_context.base(1*8);
          pbc_csr.hbm.hbm_port_1.cfg_hbm_context.no_drop(pbc_csr.hbm.hbm_port_1.cfg_hbm_context.enable().convert_to<uint32_t>());
          pbc_csr.hbm.hbm_port_1.cfg_hbm_context.write();
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_1.cfg_hbm_context.show();}
          pg8_map.set_name(pbc_csr.hbm.hbm_port_1.cfg_hbm_tc_to_q.get_hier_path()+".decoder");
          pg8_map.all(pbc_csr.hbm.hbm_port_1.cfg_hbm_tc_to_q.table()); // init
          pg8_map.pg(0,pbc_csr.hbm.hbm_port_1.cfg_hbm_parser.default_cos().convert_to<uint32_t>());
          pbc_csr.hbm.hbm_port_1.cfg_hbm_tc_to_q.table(pg8_map.all());
          pbc_csr.hbm.hbm_port_1.cfg_hbm_tc_to_q.write();
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_1.cfg_hbm_tc_to_q.show();}
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pg8_map.show();}
       } else {
          pbc_csr.hbm.hbm_port_1.cfg_hbm_context.load_from_cfg(0);
          pbc_csr.hbm.hbm_port_1.cfg_hbm_context.base(1*8);
          pbc_csr.hbm.hbm_port_1.cfg_hbm_context.write();
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_1.cfg_hbm_context.show();}
       }
       spath = pbc_csr.hbm.hbm_port_2.cfg_hbm_context.get_hier_path()+".set_based_on_default_cos";
       std::replace( spath.begin(), spath.end(), '.', '/');
       context_based_on_def_cos = sknobs_get_value((char *)spath.c_str(), 0);
       PLOG_MSG("context_based_on_def_cos for "<< spath << " returned " << context_based_on_def_cos << endl);
       if (context_based_on_def_cos) {
          pbc_csr.hbm.hbm_port_2.cfg_hbm_context.enable(1); // need to be packed
          pbc_csr.hbm.hbm_port_2.cfg_hbm_context.base(2*8);
          pbc_csr.hbm.hbm_port_2.cfg_hbm_context.no_drop(pbc_csr.hbm.hbm_port_2.cfg_hbm_context.enable().convert_to<uint32_t>());
          pbc_csr.hbm.hbm_port_2.cfg_hbm_context.write();
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_2.cfg_hbm_context.show();}
          pg8_map.set_name(pbc_csr.hbm.hbm_port_2.cfg_hbm_tc_to_q.get_hier_path()+".decoder");
          pg8_map.all(pbc_csr.hbm.hbm_port_2.cfg_hbm_tc_to_q.table()); // init
          pg8_map.pg(0,pbc_csr.hbm.hbm_port_2.cfg_hbm_parser.default_cos().convert_to<uint32_t>());
          pbc_csr.hbm.hbm_port_2.cfg_hbm_tc_to_q.table(pg8_map.all());
          pbc_csr.hbm.hbm_port_2.cfg_hbm_tc_to_q.write();
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_2.cfg_hbm_tc_to_q.show();}
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pg8_map.show();}
       } else {
          pbc_csr.hbm.hbm_port_2.cfg_hbm_context.load_from_cfg(0);
          pbc_csr.hbm.hbm_port_2.cfg_hbm_context.base(2*8);
          pbc_csr.hbm.hbm_port_2.cfg_hbm_context.write();
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_2.cfg_hbm_context.show();}
       }
       spath = pbc_csr.hbm.hbm_port_3.cfg_hbm_context.get_hier_path()+".set_based_on_default_cos";
       std::replace( spath.begin(), spath.end(), '.', '/');
       context_based_on_def_cos = sknobs_get_value((char *)spath.c_str(), 0);
       PLOG_MSG("context_based_on_def_cos for "<< spath << " returned " << context_based_on_def_cos << endl);
       if (context_based_on_def_cos) {
          pbc_csr.hbm.hbm_port_3.cfg_hbm_context.enable(1); // need to be packed
          pbc_csr.hbm.hbm_port_3.cfg_hbm_context.base(3*8);
          pbc_csr.hbm.hbm_port_3.cfg_hbm_context.no_drop(pbc_csr.hbm.hbm_port_3.cfg_hbm_context.enable().convert_to<uint32_t>());
          pbc_csr.hbm.hbm_port_3.cfg_hbm_context.write();
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_3.cfg_hbm_context.show();}
          pg8_map.set_name(pbc_csr.hbm.hbm_port_3.cfg_hbm_tc_to_q.get_hier_path()+".decoder");
          pg8_map.all(pbc_csr.hbm.hbm_port_3.cfg_hbm_tc_to_q.table()); // init
          pg8_map.pg(0,pbc_csr.hbm.hbm_port_3.cfg_hbm_parser.default_cos().convert_to<uint32_t>());
          pbc_csr.hbm.hbm_port_3.cfg_hbm_tc_to_q.table(pg8_map.all());
          pbc_csr.hbm.hbm_port_3.cfg_hbm_tc_to_q.write();
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_3.cfg_hbm_tc_to_q.show();}
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pg8_map.show();}
       } else {
          pbc_csr.hbm.hbm_port_3.cfg_hbm_context.load_from_cfg(0);
          pbc_csr.hbm.hbm_port_3.cfg_hbm_context.base(3*8);
          pbc_csr.hbm.hbm_port_3.cfg_hbm_context.write();
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_3.cfg_hbm_context.show();}
       }
       spath = pbc_csr.hbm.hbm_port_4.cfg_hbm_context.get_hier_path()+".set_based_on_default_cos";
       std::replace( spath.begin(), spath.end(), '.', '/');
       context_based_on_def_cos = sknobs_get_value((char *)spath.c_str(), 0);
       PLOG_MSG("context_based_on_def_cos for "<< spath << " returned " << context_based_on_def_cos << endl);
       if (context_based_on_def_cos) {
          pbc_csr.hbm.hbm_port_4.cfg_hbm_context.enable(0); // Not enabled on BMC port and ports > 3
          pbc_csr.hbm.hbm_port_4.cfg_hbm_context.base(((4-4)*8)+4);
          pbc_csr.hbm.hbm_port_4.cfg_hbm_context.no_drop(pbc_csr.hbm.hbm_port_4.cfg_hbm_context.enable().convert_to<uint32_t>());
          pbc_csr.hbm.hbm_port_4.cfg_hbm_context.write();
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_4.cfg_hbm_context.show();}
          pg8_map.set_name(pbc_csr.hbm.hbm_port_4.cfg_hbm_tc_to_q.get_hier_path()+".decoder");
          pg8_map.all(pbc_csr.hbm.hbm_port_4.cfg_hbm_tc_to_q.table()); // init
          pg8_map.pg(0,pbc_csr.hbm.hbm_port_4.cfg_hbm_parser.default_cos().convert_to<uint32_t>());
          pbc_csr.hbm.hbm_port_4.cfg_hbm_tc_to_q.table(pg8_map.all());
          pbc_csr.hbm.hbm_port_4.cfg_hbm_tc_to_q.write();
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_4.cfg_hbm_tc_to_q.show();}
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pg8_map.show();}
       } else {
          pbc_csr.hbm.hbm_port_4.cfg_hbm_context.load_from_cfg(0);
          pbc_csr.hbm.hbm_port_4.cfg_hbm_context.base(((4-4)*8)+4);
          pbc_csr.hbm.hbm_port_4.cfg_hbm_context.write();
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_4.cfg_hbm_context.show();}
       }
       spath = pbc_csr.hbm.hbm_port_5.cfg_hbm_context.get_hier_path()+".set_based_on_default_cos";
       std::replace( spath.begin(), spath.end(), '.', '/');
       context_based_on_def_cos = sknobs_get_value((char *)spath.c_str(), 0);
       PLOG_MSG("context_based_on_def_cos for "<< spath << " returned " << context_based_on_def_cos << endl);
       if (context_based_on_def_cos) {
          pbc_csr.hbm.hbm_port_5.cfg_hbm_context.enable(0); // Not enabled on BMC port and ports > 3
          pbc_csr.hbm.hbm_port_5.cfg_hbm_context.base(((5-4)*8)+4);
          pbc_csr.hbm.hbm_port_5.cfg_hbm_context.no_drop(pbc_csr.hbm.hbm_port_5.cfg_hbm_context.enable().convert_to<uint32_t>());
          pbc_csr.hbm.hbm_port_5.cfg_hbm_context.write();
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_5.cfg_hbm_context.show();}
          pg8_map.set_name(pbc_csr.hbm.hbm_port_5.cfg_hbm_tc_to_q.get_hier_path()+".decoder");
          pg8_map.all(pbc_csr.hbm.hbm_port_5.cfg_hbm_tc_to_q.table()); // init
          pg8_map.pg(0,pbc_csr.hbm.hbm_port_5.cfg_hbm_parser.default_cos().convert_to<uint32_t>());
          pbc_csr.hbm.hbm_port_5.cfg_hbm_tc_to_q.table(pg8_map.all());
          pbc_csr.hbm.hbm_port_5.cfg_hbm_tc_to_q.write();
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_5.cfg_hbm_tc_to_q.show();}
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pg8_map.show();}
       } else {
          pbc_csr.hbm.hbm_port_5.cfg_hbm_context.load_from_cfg(0);
          pbc_csr.hbm.hbm_port_5.cfg_hbm_context.base(((5-4)*8)+4);
          pbc_csr.hbm.hbm_port_5.cfg_hbm_context.write();
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_5.cfg_hbm_context.show();}
       }
       spath = pbc_csr.hbm.hbm_port_6.cfg_hbm_context.get_hier_path()+".set_based_on_default_cos";
       std::replace( spath.begin(), spath.end(), '.', '/');
       context_based_on_def_cos = sknobs_get_value((char *)spath.c_str(), 0);
       PLOG_MSG("context_based_on_def_cos for "<< spath << " returned " << context_based_on_def_cos << endl);
       if (context_based_on_def_cos) {
          pbc_csr.hbm.hbm_port_6.cfg_hbm_context.enable(0); // Not enabled on BMC port and ports > 3
          pbc_csr.hbm.hbm_port_6.cfg_hbm_context.base(((6-4)*8)+4);
          pbc_csr.hbm.hbm_port_6.cfg_hbm_context.no_drop(pbc_csr.hbm.hbm_port_6.cfg_hbm_context.enable().convert_to<uint32_t>());
          pbc_csr.hbm.hbm_port_6.cfg_hbm_context.write();
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_6.cfg_hbm_context.show();}
          pg8_map.set_name(pbc_csr.hbm.hbm_port_6.cfg_hbm_tc_to_q.get_hier_path()+".decoder");
          pg8_map.all(pbc_csr.hbm.hbm_port_6.cfg_hbm_tc_to_q.table()); // init
          pg8_map.pg(0,pbc_csr.hbm.hbm_port_6.cfg_hbm_parser.default_cos().convert_to<uint32_t>());
          pbc_csr.hbm.hbm_port_6.cfg_hbm_tc_to_q.table(pg8_map.all());
          pbc_csr.hbm.hbm_port_6.cfg_hbm_tc_to_q.write();
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_6.cfg_hbm_tc_to_q.show();}
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pg8_map.show();}
       } else {
          pbc_csr.hbm.hbm_port_6.cfg_hbm_context.load_from_cfg(0);
          pbc_csr.hbm.hbm_port_6.cfg_hbm_context.base(((6-4)*8)+4);
          pbc_csr.hbm.hbm_port_6.cfg_hbm_context.write();
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_6.cfg_hbm_context.show();}
       }
       spath = pbc_csr.hbm.hbm_port_7.cfg_hbm_context.get_hier_path()+".set_based_on_default_cos";
       std::replace( spath.begin(), spath.end(), '.', '/');
       context_based_on_def_cos = sknobs_get_value((char *)spath.c_str(), 0);
       PLOG_MSG("context_based_on_def_cos for "<< spath << " returned " << context_based_on_def_cos << endl);
       if (context_based_on_def_cos) {
          pbc_csr.hbm.hbm_port_7.cfg_hbm_context.enable(0); // Not enabled on BMC port and ports > 3
          pbc_csr.hbm.hbm_port_7.cfg_hbm_context.base(((7-4)*8)+4);
          pbc_csr.hbm.hbm_port_7.cfg_hbm_context.no_drop(pbc_csr.hbm.hbm_port_7.cfg_hbm_context.enable().convert_to<uint32_t>());
          pbc_csr.hbm.hbm_port_7.cfg_hbm_context.write();
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_7.cfg_hbm_context.show();}
          pg8_map.set_name(pbc_csr.hbm.hbm_port_7.cfg_hbm_tc_to_q.get_hier_path()+".decoder");
          pg8_map.all(pbc_csr.hbm.hbm_port_7.cfg_hbm_tc_to_q.table()); // init
          pg8_map.pg(0,pbc_csr.hbm.hbm_port_7.cfg_hbm_parser.default_cos().convert_to<uint32_t>());
          pbc_csr.hbm.hbm_port_7.cfg_hbm_tc_to_q.table(pg8_map.all());
          pbc_csr.hbm.hbm_port_7.cfg_hbm_tc_to_q.write();
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_7.cfg_hbm_tc_to_q.show();}
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pg8_map.show();}
       } else {
          pbc_csr.hbm.hbm_port_7.cfg_hbm_context.load_from_cfg(0);
          pbc_csr.hbm.hbm_port_7.cfg_hbm_context.base(((7-4)*8)+4);
          pbc_csr.hbm.hbm_port_7.cfg_hbm_context.write();
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_7.cfg_hbm_context.show();}
       }
       spath = pbc_csr.hbm.hbm_port_8.cfg_hbm_context.get_hier_path()+".set_based_on_default_cos";
       std::replace( spath.begin(), spath.end(), '.', '/');
       context_based_on_def_cos = sknobs_get_value((char *)spath.c_str(), 0);
       PLOG_MSG("context_based_on_def_cos for "<< spath << " returned " << context_based_on_def_cos << endl);
       if (context_based_on_def_cos) {
          pbc_csr.hbm.hbm_port_8.cfg_hbm_context.enable(0); // Not enabled on BMC port and ports > 3
          pbc_csr.hbm.hbm_port_8.cfg_hbm_context.base(((8-4)*8)+4);
          pbc_csr.hbm.hbm_port_8.cfg_hbm_context.no_drop(pbc_csr.hbm.hbm_port_8.cfg_hbm_context.enable().convert_to<uint32_t>());
          pbc_csr.hbm.hbm_port_8.cfg_hbm_context.write();
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_8.cfg_hbm_context.show();}
          pg8_map.set_name(pbc_csr.hbm.hbm_port_8.cfg_hbm_tc_to_q.get_hier_path()+".decoder");
          pg8_map.all(pbc_csr.hbm.hbm_port_8.cfg_hbm_tc_to_q.table()); // init
          pg8_map.pg(0,pbc_csr.hbm.hbm_port_8.cfg_hbm_parser.default_cos().convert_to<uint32_t>());
          pbc_csr.hbm.hbm_port_8.cfg_hbm_tc_to_q.table(pg8_map.all());
          pbc_csr.hbm.hbm_port_8.cfg_hbm_tc_to_q.write();
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_8.cfg_hbm_tc_to_q.show();}
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pg8_map.show();}
       } else {
          pbc_csr.hbm.hbm_port_8.cfg_hbm_context.load_from_cfg(0);
          pbc_csr.hbm.hbm_port_8.cfg_hbm_context.base(((8-4)*8)+4);
          pbc_csr.hbm.hbm_port_8.cfg_hbm_context.write();
          if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_8.cfg_hbm_context.show();}
       }
    }

   pbc_csr.hbm.cfg_hbm_wb.load_from_cfg();
   if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.cfg_hbm_wb.show();}

   // egress timestamp
   cap_pbc_egr_tstamp_t egr_tstamp;
   egr_tstamp.init();
   cfg_found_cnt = pbc_csr.cfg_eg_ts_ctrl.load_from_cfg_rtn_status(0);
   egr_tstamp.all(pbc_csr.cfg_eg_ts_ctrl.all()); // init
   egr_tstamp.set_name(pbc_csr.cfg_eg_ts_ctrl.get_hier_path()+".decoder");
   cfg_found_cnt += egr_tstamp.load_from_cfg_rtn_status(0);
   if (cfg_found_cnt > 0) {
      pbc_csr.cfg_eg_ts_ctrl.base(egr_tstamp.all());
      pbc_csr.cfg_eg_ts_ctrl.write();
   }
   if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.cfg_eg_ts_ctrl.show();}
   if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {egr_tstamp.show();}

    cfg_found_cnt = pbc_csr.port_0.cfg_oq_xoff2oq.load_from_cfg_rtn_status(0);
    eth_oq_xoff_map.all(pbc_csr.port_0.cfg_oq_xoff2oq.map()); // init
    eth_oq_xoff_map.set_name(pbc_csr.port_0.cfg_oq_xoff2oq.get_hier_path()+".decoder");
    cfg_found_cnt += eth_oq_xoff_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.port_0.cfg_oq_xoff2oq.map(eth_oq_xoff_map.all());
       pbc_csr.port_0.cfg_oq_xoff2oq.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_0.cfg_oq_xoff2oq.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_oq_xoff_map.show();}
    cfg_found_cnt = pbc_csr.port_1.cfg_oq_xoff2oq.load_from_cfg_rtn_status(0);
    eth_oq_xoff_map.all(pbc_csr.port_1.cfg_oq_xoff2oq.map()); // init
    eth_oq_xoff_map.set_name(pbc_csr.port_1.cfg_oq_xoff2oq.get_hier_path()+".decoder");
    cfg_found_cnt += eth_oq_xoff_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.port_1.cfg_oq_xoff2oq.map(eth_oq_xoff_map.all());
       pbc_csr.port_1.cfg_oq_xoff2oq.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_1.cfg_oq_xoff2oq.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_oq_xoff_map.show();}
    cfg_found_cnt = pbc_csr.port_2.cfg_oq_xoff2oq.load_from_cfg_rtn_status(0);
    eth_oq_xoff_map.all(pbc_csr.port_2.cfg_oq_xoff2oq.map()); // init
    eth_oq_xoff_map.set_name(pbc_csr.port_2.cfg_oq_xoff2oq.get_hier_path()+".decoder");
    cfg_found_cnt += eth_oq_xoff_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.port_2.cfg_oq_xoff2oq.map(eth_oq_xoff_map.all());
       pbc_csr.port_2.cfg_oq_xoff2oq.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_2.cfg_oq_xoff2oq.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_oq_xoff_map.show();}
    cfg_found_cnt = pbc_csr.port_3.cfg_oq_xoff2oq.load_from_cfg_rtn_status(0);
    eth_oq_xoff_map.all(pbc_csr.port_3.cfg_oq_xoff2oq.map()); // init
    eth_oq_xoff_map.set_name(pbc_csr.port_3.cfg_oq_xoff2oq.get_hier_path()+".decoder");
    cfg_found_cnt += eth_oq_xoff_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.port_3.cfg_oq_xoff2oq.map(eth_oq_xoff_map.all());
       pbc_csr.port_3.cfg_oq_xoff2oq.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_3.cfg_oq_xoff2oq.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_oq_xoff_map.show();}
    cfg_found_cnt = pbc_csr.port_4.cfg_oq_xoff2oq.load_from_cfg_rtn_status(0);
    eth_oq_xoff_map.all(pbc_csr.port_4.cfg_oq_xoff2oq.map()); // init
    eth_oq_xoff_map.set_name(pbc_csr.port_4.cfg_oq_xoff2oq.get_hier_path()+".decoder");
    cfg_found_cnt += eth_oq_xoff_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.port_4.cfg_oq_xoff2oq.map(eth_oq_xoff_map.all());
       pbc_csr.port_4.cfg_oq_xoff2oq.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_4.cfg_oq_xoff2oq.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_oq_xoff_map.show();}
    cfg_found_cnt = pbc_csr.port_5.cfg_oq_xoff2oq.load_from_cfg_rtn_status(0);
    eth_oq_xoff_map.all(pbc_csr.port_5.cfg_oq_xoff2oq.map()); // init
    eth_oq_xoff_map.set_name(pbc_csr.port_5.cfg_oq_xoff2oq.get_hier_path()+".decoder");
    cfg_found_cnt += eth_oq_xoff_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.port_5.cfg_oq_xoff2oq.map(eth_oq_xoff_map.all());
       pbc_csr.port_5.cfg_oq_xoff2oq.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_5.cfg_oq_xoff2oq.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_oq_xoff_map.show();}
    cfg_found_cnt = pbc_csr.port_6.cfg_oq_xoff2oq.load_from_cfg_rtn_status(0);
    eth_oq_xoff_map.all(pbc_csr.port_6.cfg_oq_xoff2oq.map()); // init
    eth_oq_xoff_map.set_name(pbc_csr.port_6.cfg_oq_xoff2oq.get_hier_path()+".decoder");
    cfg_found_cnt += eth_oq_xoff_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.port_6.cfg_oq_xoff2oq.map(eth_oq_xoff_map.all());
       pbc_csr.port_6.cfg_oq_xoff2oq.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_6.cfg_oq_xoff2oq.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_oq_xoff_map.show();}
    cfg_found_cnt = pbc_csr.port_7.cfg_oq_xoff2oq.load_from_cfg_rtn_status(0);
    eth_oq_xoff_map.all(pbc_csr.port_7.cfg_oq_xoff2oq.map()); // init
    eth_oq_xoff_map.set_name(pbc_csr.port_7.cfg_oq_xoff2oq.get_hier_path()+".decoder");
    cfg_found_cnt += eth_oq_xoff_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.port_7.cfg_oq_xoff2oq.map(eth_oq_xoff_map.all());
       pbc_csr.port_7.cfg_oq_xoff2oq.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_7.cfg_oq_xoff2oq.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_oq_xoff_map.show();}
    cfg_found_cnt = pbc_csr.port_8.cfg_oq_xoff2oq.load_from_cfg_rtn_status(0);
    eth_oq_xoff_map.all(pbc_csr.port_8.cfg_oq_xoff2oq.map()); // init
    eth_oq_xoff_map.set_name(pbc_csr.port_8.cfg_oq_xoff2oq.get_hier_path()+".decoder");
    cfg_found_cnt += eth_oq_xoff_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.port_8.cfg_oq_xoff2oq.map(eth_oq_xoff_map.all());
       pbc_csr.port_8.cfg_oq_xoff2oq.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_8.cfg_oq_xoff2oq.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_oq_xoff_map.show();}
    cfg_found_cnt = pbc_csr.port_9.cfg_oq_xoff2oq.load_from_cfg_rtn_status(0);
    p4_oq_xoff_map.all(pbc_csr.port_9.cfg_oq_xoff2oq.map()); // init
    p4_oq_xoff_map.set_name(pbc_csr.port_9.cfg_oq_xoff2oq.get_hier_path()+".decoder");
    cfg_found_cnt += p4_oq_xoff_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.port_9.cfg_oq_xoff2oq.map(p4_oq_xoff_map.all());
       pbc_csr.port_9.cfg_oq_xoff2oq.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_oq_xoff2oq.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {p4_oq_xoff_map.show();}
    cfg_found_cnt = pbc_csr.port_10.cfg_oq_xoff2oq.load_from_cfg_rtn_status(0);
    p4_oq_xoff_map.all(pbc_csr.port_10.cfg_oq_xoff2oq.map()); // init
    p4_oq_xoff_map.set_name(pbc_csr.port_10.cfg_oq_xoff2oq.get_hier_path()+".decoder");
    cfg_found_cnt += p4_oq_xoff_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.port_10.cfg_oq_xoff2oq.map(p4_oq_xoff_map.all());
       pbc_csr.port_10.cfg_oq_xoff2oq.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_oq_xoff2oq.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {p4_oq_xoff_map.show();}
    cfg_found_cnt = pbc_csr.port_11.cfg_oq_xoff2oq.load_from_cfg_rtn_status(0);
    p4_oq_xoff_map.all(pbc_csr.port_11.cfg_oq_xoff2oq.map()); // init
    p4_oq_xoff_map.set_name(pbc_csr.port_11.cfg_oq_xoff2oq.get_hier_path()+".decoder");
    cfg_found_cnt += p4_oq_xoff_map.load_from_cfg_rtn_status(0);
    if (cfg_found_cnt > 0) {
       pbc_csr.port_11.cfg_oq_xoff2oq.map(p4_oq_xoff_map.all());
       pbc_csr.port_11.cfg_oq_xoff2oq.write();
    }
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_oq_xoff2oq.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {p4_oq_xoff_map.show();}
   pbc_csr.hbm.cfg_hbm_axi_base.load_from_cfg();
   if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.cfg_hbm_axi_base.show();}
   pbc_csr.cfg_axi.load_from_cfg();
   if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.cfg_axi.show();}
   pbc_csr.hbm.cfg_hbm_wb.load_from_cfg();
   if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.cfg_hbm_wb.show();}

   //pause timer

    pbc_csr.port_0.cfg_account_pause_timer.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_0.cfg_account_pause_timer.show();}
    pbc_csr.port_1.cfg_account_pause_timer.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_1.cfg_account_pause_timer.show();}
    pbc_csr.port_2.cfg_account_pause_timer.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_2.cfg_account_pause_timer.show();}
    pbc_csr.port_3.cfg_account_pause_timer.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_3.cfg_account_pause_timer.show();}
    pbc_csr.port_4.cfg_account_pause_timer.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_4.cfg_account_pause_timer.show();}
    pbc_csr.port_5.cfg_account_pause_timer.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_5.cfg_account_pause_timer.show();}
    pbc_csr.port_6.cfg_account_pause_timer.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_6.cfg_account_pause_timer.show();}
    pbc_csr.port_7.cfg_account_pause_timer.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_7.cfg_account_pause_timer.show();}
    pbc_csr.port_8.cfg_account_pause_timer.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_8.cfg_account_pause_timer.show();}
    pbc_csr.port_9.cfg_account_pause_timer.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pause_timer.show();}
    pbc_csr.port_10.cfg_account_pause_timer.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pause_timer.show();}
    pbc_csr.port_11.cfg_account_pause_timer.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pause_timer.show();}
    pbc_csr.hbm.hbm_port_0.cfg_hbm_read_fifo.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_0.cfg_hbm_read_fifo.show();}
    pbc_csr.hbm.hbm_port_1.cfg_hbm_read_fifo.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_1.cfg_hbm_read_fifo.show();}
    pbc_csr.hbm.hbm_port_2.cfg_hbm_read_fifo.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_2.cfg_hbm_read_fifo.show();}
    pbc_csr.hbm.hbm_port_3.cfg_hbm_read_fifo.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_3.cfg_hbm_read_fifo.show();}
    pbc_csr.hbm.hbm_port_4.cfg_hbm_read_fifo.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_4.cfg_hbm_read_fifo.show();}
    pbc_csr.hbm.hbm_port_5.cfg_hbm_read_fifo.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_5.cfg_hbm_read_fifo.show();}
    pbc_csr.hbm.hbm_port_6.cfg_hbm_read_fifo.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_6.cfg_hbm_read_fifo.show();}
    pbc_csr.hbm.hbm_port_7.cfg_hbm_read_fifo.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_7.cfg_hbm_read_fifo.show();}
    pbc_csr.hbm.hbm_port_8.cfg_hbm_read_fifo.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_8.cfg_hbm_read_fifo.show();}
    pbc_csr.hbm.hbm_port_9.cfg_hbm_read_fifo.load_from_cfg();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_9.cfg_hbm_read_fifo.show();}
    // read to flush writes
   pbc_csr.cfg_island_control.read();

   unsigned int sched_test = SKNOBS_GET("tb/sched_test", 0);
   spath = pbc_csr.get_hier_path()+".sched_prog_from_api";
   std::replace( spath.begin(), spath.end(), '.', '/');
   unsigned int  sched_prog_from_api = sknobs_get_value((char *)spath.c_str(), 0);

   if(sched_test || sched_prog_from_api) {
     cap_pb_cfg_sched(chip_id, inst_id, 0);
   }


   //bool ret_val = false;
   string apath = pbc_csr.get_hier_path()+".check_island_split";
   std::replace( apath.begin(), apath.end(), '.', '/');
   unsigned int check_island_split = sknobs_get_value((char *)apath.c_str(), 0);
   if (check_island_split) { cap_pb_check_island_split(chip_id,inst_id); }

   pbc_csr.hbm.cfg_hbm_cut_thru.load_from_cfg();
   if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.cfg_hbm_cut_thru.show();}

   pbc_csr.cfg_port_mon_in.load_from_cfg();  
   pbc_csr.cfg_port_mon_out.load_from_cfg();  

   pbc_csr.port_0.cfg_account_control.load_from_cfg(0);
   pbc_csr.port_0.cfg_account_control.use_sp_as_wm(1);
   pbc_csr.port_0.cfg_account_control.write();
   pbc_csr.port_1.cfg_account_control.load_from_cfg(0);
   pbc_csr.port_1.cfg_account_control.use_sp_as_wm(1);
   pbc_csr.port_1.cfg_account_control.write();
   pbc_csr.port_2.cfg_account_control.load_from_cfg(0);
   pbc_csr.port_2.cfg_account_control.use_sp_as_wm(1);
   pbc_csr.port_2.cfg_account_control.write();
   pbc_csr.port_3.cfg_account_control.load_from_cfg(0);
   pbc_csr.port_3.cfg_account_control.use_sp_as_wm(1);
   pbc_csr.port_3.cfg_account_control.write();
   pbc_csr.port_4.cfg_account_control.load_from_cfg(0);
   pbc_csr.port_4.cfg_account_control.use_sp_as_wm(1);
   pbc_csr.port_4.cfg_account_control.write();
   pbc_csr.port_5.cfg_account_control.load_from_cfg(0);
   pbc_csr.port_5.cfg_account_control.use_sp_as_wm(1);
   pbc_csr.port_5.cfg_account_control.write();
   pbc_csr.port_6.cfg_account_control.load_from_cfg(0);
   pbc_csr.port_6.cfg_account_control.use_sp_as_wm(1);
   pbc_csr.port_6.cfg_account_control.write();
   pbc_csr.port_7.cfg_account_control.load_from_cfg(0);
   pbc_csr.port_7.cfg_account_control.use_sp_as_wm(1);
   pbc_csr.port_7.cfg_account_control.write();
   pbc_csr.port_8.cfg_account_control.load_from_cfg(0);
   pbc_csr.port_8.cfg_account_control.use_sp_as_wm(1);
   pbc_csr.port_8.cfg_account_control.write();
   pbc_csr.port_9.cfg_account_control.load_from_cfg(0);
   pbc_csr.port_9.cfg_account_control.use_sp_as_wm(1);
   pbc_csr.port_9.cfg_account_control.write();
   pbc_csr.port_10.cfg_account_control.load_from_cfg(0);
   pbc_csr.port_10.cfg_account_control.use_sp_as_wm(1);
   pbc_csr.port_10.cfg_account_control.write();
   pbc_csr.port_11.cfg_account_control.load_from_cfg(0);
   pbc_csr.port_11.cfg_account_control.use_sp_as_wm(1);
   pbc_csr.port_11.cfg_account_control.write();
   pbc_csr.hbm.cfg_hbm_eth_xoff_timeout.load_from_cfg();
   if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.cfg_hbm_eth_xoff_timeout.show();}

   pbc_csr.hbm.cfg_hbm_eth_xoff_force.load_from_cfg();
   if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.cfg_hbm_eth_xoff_force.show();}

   pbc_csr.hbm.cfg_hbm_xoff.load_from_cfg();
   if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.cfg_hbm_xoff.show();}

   pbc_csr.hbm.cfg_hbm_rb.load_from_cfg(0);
   pbc_csr.hbm.cfg_hbm_rb.enable_wrr(0);
   pbc_csr.hbm.cfg_hbm_rb.write();
   if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.cfg_hbm_rb.show();}

    // read to flush writes
   pbc_csr.cfg_island_control.read();

 } //if(pb_skip_init == 0)

PLOG_MSG("done cap_pb_load_from_cfg \n");
}

void cap_pb_hbm_thr_from_api(int chip_id, int inst_id) {
PLOG_MSG("inside cap_pb_hbm_thr_from_api \n");

 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
 //cap_pbm_csr_t & pbm_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbm_csr_t, chip_id, inst_id);

 cap_pbc_hbm_tx_ctl_t tx_hbm_ctl,tx_hbm_pld;
 cap_pbc_hbm_eth_ctl_t eth_hbm_ctl,eth_hbm_pld;
 cap_pbc_hbm_eth_port_ctl_t eth_hbm_port_ctl[8],eth_hbm_port_pld[8];
 cap_pbc_hbm_eth_occ_thr_t eth_occ_thr[8];
 cap_pbc_hbm_tx_occ_thr_t tx_occ_thr;
 cap_pbc_hbm_eth_xoff_thr_t eth_xoff_thr;

 for (int jj=0; jj<8; jj++) {
    eth_occ_thr[jj].init();
    eth_hbm_port_ctl[jj].init();
    eth_hbm_port_pld[jj].init();
 }
 uint32_t context_base,enable,no_drop;
 uint32_t context,timeout;
 string mpath,path,ppath;
#ifdef MODULE_SIM
 RRKnob   rand_knob("rand_knob", 0, 1);
#endif
 uint32_t hbm_start=0,hbm_end=0;
 uint32_t base_addr,size,ctx_size,headroom,base_addr_ctrl,nodrop,xoff,xon,hbm_en,hbm_enabled_for_port;
 string orig_path = pbc_csr.get_hier_path()+".hbm.port";
 std::replace( orig_path.begin(), orig_path.end(), '.', '/');
    // port 0
    ppath =orig_path+ "/"+to_string(0);
    mpath=ppath+"/xoff_timeout";   timeout = sknobs_get_value((char *)mpath.c_str(), 0);
    // pre-parse 
    hbm_enabled_for_port = 0;
    for (uint32_t q=0; q<8; q++) {
       path = ppath+"/queue/"+to_string(q);
       mpath=path+"/hbm_en"; hbm_en = sknobs_get_value((char *)mpath.c_str(), 0);
       if (hbm_en) { hbm_enabled_for_port = 1;}
    }
    enable=0; no_drop=0;
    for (uint32_t q=0; q<8; q++) {
       path = ppath+"/queue/"+to_string(q);
       mpath=path+"/hbm_en"; if (hbm_enabled_for_port) { CHK_SKNOB_EXISTS(mpath); } hbm_en = sknobs_get_value((char *)mpath.c_str(), 0);
       if (hbm_en) {
            // per port, but for simplicity of impl, ready for every queue
          // mpath=ppath+"/context_base"; CHK_SKNOB_EXISTS(mpath); context_base = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/base_addr";     CHK_SKNOB_EXISTS(mpath); base_addr = sknobs_get_value((char *)mpath.c_str(), 0); 
          mpath=path+"/size";          CHK_SKNOB_EXISTS(mpath); size = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/headroom";      CHK_SKNOB_EXISTS(mpath); headroom = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/base_addr_ctrl";CHK_SKNOB_EXISTS(mpath); base_addr_ctrl = sknobs_get_value((char *)mpath.c_str(), 0);
          if (hbm_start > base_addr) { hbm_start = base_addr; }
          if (hbm_start > base_addr_ctrl) { hbm_start = base_addr_ctrl; }
          if (hbm_end < base_addr) { hbm_end = base_addr+size; }
          if (hbm_end < base_addr_ctrl) { hbm_end = base_addr_ctrl+((size+49)/50); }
#ifdef MODULE_SIM
          uint32_t nodrop_rand;
          mpath=path+"/nodrop_rand";   nodrop_rand = sknobs_get_value((char *)mpath.c_str(), 0);
          if (nodrop_rand ==1) {
             nodrop = rand_knob.eval();
          } else {
#endif
             mpath=path+"/nodrop";         nodrop = sknobs_get_value((char *)mpath.c_str(), 0);
#ifdef MODULE_SIM
          }
#endif
          mpath=path+"/xoff";          CHK_SKNOB_EXISTS(mpath); xoff = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/xon";           CHK_SKNOB_EXISTS(mpath); xon = sknobs_get_value((char *)mpath.c_str(), 0);
          // program the last index  
          ctx_size  = (size+49)/50;
          ctx_size = (ctx_size > 0) ? ctx_size-1 : ctx_size;
          size = (size > 0) ? size-1 : size;
          context_base = 0*8;
          context = context_base+q;
          enable |= 1 << q;
          no_drop |= nodrop << q;
          eth_hbm_pld.base(base_addr,context);
          eth_hbm_pld.mem_sz(size,context);
          eth_hbm_ctl.base(base_addr_ctrl,context);
          eth_hbm_ctl.mem_sz(ctx_size,context);
          eth_hbm_port_pld[0].base(base_addr,q);
          eth_hbm_port_pld[0].mem_sz(size,q);
          eth_hbm_port_ctl[0].base(base_addr_ctrl,q);
          eth_hbm_port_ctl[0].mem_sz(ctx_size,q);
          eth_occ_thr[0].thr((size - headroom) / 16,q); //granularity of 1k
          eth_xoff_thr.xoff(xoff / 8 ,context); //granularity of 512   
          eth_xoff_thr.xon(xon / 8 ,context); //granularity of 512   
       }
    }
    pbc_csr.hbm.hbm_port_0.cfg_hbm_context.base(context_base);
    pbc_csr.hbm.hbm_port_0.cfg_hbm_context.xoff_timeout(timeout);
    pbc_csr.hbm.hbm_port_0.cfg_hbm_context.enable(enable);
    pbc_csr.hbm.hbm_port_0.cfg_hbm_context.no_drop(no_drop);
    pbc_csr.hbm.hbm_port_0.cfg_hbm_eth_payload_occupancy.all(eth_occ_thr[0].all());
    pbc_csr.hbm.hbm_port_0.cfg_hbm_eth_payload_occupancy.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_0.cfg_hbm_eth_payload_occupancy.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_occ_thr[0].show();}
    pbc_csr.hbm.hbm_port_0.cfg_hbm_eth_ctrl.all(eth_hbm_port_ctl[0].all());
    pbc_csr.hbm.hbm_port_0.cfg_hbm_eth_ctrl.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_0.cfg_hbm_eth_ctrl.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_ctl[0].show();}
    pbc_csr.hbm.hbm_port_0.cfg_hbm_eth_payload.all(eth_hbm_port_pld[0].all());
    pbc_csr.hbm.hbm_port_0.cfg_hbm_eth_payload.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_0.cfg_hbm_eth_payload.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_pld[0].show();}
    pbc_csr.hbm.hbm_port_0.cfg_hbm_context.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_0.cfg_hbm_context.show();}
    // port 1
    ppath =orig_path+ "/"+to_string(1);
    mpath=ppath+"/xoff_timeout";   timeout = sknobs_get_value((char *)mpath.c_str(), 0);
    // pre-parse 
    hbm_enabled_for_port = 0;
    for (uint32_t q=0; q<8; q++) {
       path = ppath+"/queue/"+to_string(q);
       mpath=path+"/hbm_en"; hbm_en = sknobs_get_value((char *)mpath.c_str(), 0);
       if (hbm_en) { hbm_enabled_for_port = 1;}
    }
    enable=0; no_drop=0;
    for (uint32_t q=0; q<8; q++) {
       path = ppath+"/queue/"+to_string(q);
       mpath=path+"/hbm_en"; if (hbm_enabled_for_port) { CHK_SKNOB_EXISTS(mpath); } hbm_en = sknobs_get_value((char *)mpath.c_str(), 0);
       if (hbm_en) {
            // per port, but for simplicity of impl, ready for every queue
          // mpath=ppath+"/context_base"; CHK_SKNOB_EXISTS(mpath); context_base = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/base_addr";     CHK_SKNOB_EXISTS(mpath); base_addr = sknobs_get_value((char *)mpath.c_str(), 0); 
          mpath=path+"/size";          CHK_SKNOB_EXISTS(mpath); size = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/headroom";      CHK_SKNOB_EXISTS(mpath); headroom = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/base_addr_ctrl";CHK_SKNOB_EXISTS(mpath); base_addr_ctrl = sknobs_get_value((char *)mpath.c_str(), 0);
          if (hbm_start > base_addr) { hbm_start = base_addr; }
          if (hbm_start > base_addr_ctrl) { hbm_start = base_addr_ctrl; }
          if (hbm_end < base_addr) { hbm_end = base_addr+size; }
          if (hbm_end < base_addr_ctrl) { hbm_end = base_addr_ctrl+((size+49)/50); }
#ifdef MODULE_SIM
          mpath=path+"/nodrop_rand";   nodrop_rand = sknobs_get_value((char *)mpath.c_str(), 0);
          if (nodrop_rand ==1) {
             nodrop = rand_knob.eval();
          } else {
#endif
             mpath=path+"/nodrop";         nodrop = sknobs_get_value((char *)mpath.c_str(), 0);
#ifdef MODULE_SIM
          }
#endif
          mpath=path+"/xoff";          CHK_SKNOB_EXISTS(mpath); xoff = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/xon";           CHK_SKNOB_EXISTS(mpath); xon = sknobs_get_value((char *)mpath.c_str(), 0);
          // program the last index  
          ctx_size  = (size+49)/50;
          ctx_size = (ctx_size > 0) ? ctx_size-1 : ctx_size;
          size = (size > 0) ? size-1 : size;
          context_base = 1*8;
          context = context_base+q;
          enable |= 1 << q;
          no_drop |= nodrop << q;
          eth_hbm_pld.base(base_addr,context);
          eth_hbm_pld.mem_sz(size,context);
          eth_hbm_ctl.base(base_addr_ctrl,context);
          eth_hbm_ctl.mem_sz(ctx_size,context);
          eth_hbm_port_pld[1].base(base_addr,q);
          eth_hbm_port_pld[1].mem_sz(size,q);
          eth_hbm_port_ctl[1].base(base_addr_ctrl,q);
          eth_hbm_port_ctl[1].mem_sz(ctx_size,q);
          eth_occ_thr[1].thr((size - headroom) / 16,q); //granularity of 1k
          eth_xoff_thr.xoff(xoff / 8 ,context); //granularity of 512   
          eth_xoff_thr.xon(xon / 8 ,context); //granularity of 512   
       }
    }
    pbc_csr.hbm.hbm_port_1.cfg_hbm_context.base(context_base);
    pbc_csr.hbm.hbm_port_1.cfg_hbm_context.xoff_timeout(timeout);
    pbc_csr.hbm.hbm_port_1.cfg_hbm_context.enable(enable);
    pbc_csr.hbm.hbm_port_1.cfg_hbm_context.no_drop(no_drop);
    pbc_csr.hbm.hbm_port_1.cfg_hbm_eth_payload_occupancy.all(eth_occ_thr[1].all());
    pbc_csr.hbm.hbm_port_1.cfg_hbm_eth_payload_occupancy.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_1.cfg_hbm_eth_payload_occupancy.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_occ_thr[1].show();}
    pbc_csr.hbm.hbm_port_1.cfg_hbm_eth_ctrl.all(eth_hbm_port_ctl[1].all());
    pbc_csr.hbm.hbm_port_1.cfg_hbm_eth_ctrl.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_1.cfg_hbm_eth_ctrl.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_ctl[1].show();}
    pbc_csr.hbm.hbm_port_1.cfg_hbm_eth_payload.all(eth_hbm_port_pld[1].all());
    pbc_csr.hbm.hbm_port_1.cfg_hbm_eth_payload.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_1.cfg_hbm_eth_payload.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_pld[1].show();}
    pbc_csr.hbm.hbm_port_1.cfg_hbm_context.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_1.cfg_hbm_context.show();}
    // port 2
    ppath =orig_path+ "/"+to_string(2);
    mpath=ppath+"/xoff_timeout";   timeout = sknobs_get_value((char *)mpath.c_str(), 0);
    // pre-parse 
    hbm_enabled_for_port = 0;
    for (uint32_t q=0; q<8; q++) {
       path = ppath+"/queue/"+to_string(q);
       mpath=path+"/hbm_en"; hbm_en = sknobs_get_value((char *)mpath.c_str(), 0);
       if (hbm_en) { hbm_enabled_for_port = 1;}
    }
    enable=0; no_drop=0;
    for (uint32_t q=0; q<8; q++) {
       path = ppath+"/queue/"+to_string(q);
       mpath=path+"/hbm_en"; if (hbm_enabled_for_port) { CHK_SKNOB_EXISTS(mpath); } hbm_en = sknobs_get_value((char *)mpath.c_str(), 0);
       if (hbm_en) {
            // per port, but for simplicity of impl, ready for every queue
          // mpath=ppath+"/context_base"; CHK_SKNOB_EXISTS(mpath); context_base = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/base_addr";     CHK_SKNOB_EXISTS(mpath); base_addr = sknobs_get_value((char *)mpath.c_str(), 0); 
          mpath=path+"/size";          CHK_SKNOB_EXISTS(mpath); size = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/headroom";      CHK_SKNOB_EXISTS(mpath); headroom = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/base_addr_ctrl";CHK_SKNOB_EXISTS(mpath); base_addr_ctrl = sknobs_get_value((char *)mpath.c_str(), 0);
          if (hbm_start > base_addr) { hbm_start = base_addr; }
          if (hbm_start > base_addr_ctrl) { hbm_start = base_addr_ctrl; }
          if (hbm_end < base_addr) { hbm_end = base_addr+size; }
          if (hbm_end < base_addr_ctrl) { hbm_end = base_addr_ctrl+((size+49)/50); }
#ifdef MODULE_SIM
          mpath=path+"/nodrop_rand";   nodrop_rand = sknobs_get_value((char *)mpath.c_str(), 0);
          if (nodrop_rand ==1) {
             nodrop = rand_knob.eval();
          } else {
#endif
             mpath=path+"/nodrop";         nodrop = sknobs_get_value((char *)mpath.c_str(), 0);
#ifdef MODULE_SIM
          }
#endif
          mpath=path+"/xoff";          CHK_SKNOB_EXISTS(mpath); xoff = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/xon";           CHK_SKNOB_EXISTS(mpath); xon = sknobs_get_value((char *)mpath.c_str(), 0);
          // program the last index  
          ctx_size  = (size+49)/50;
          ctx_size = (ctx_size > 0) ? ctx_size-1 : ctx_size;
          size = (size > 0) ? size-1 : size;
          context_base = 2*8;
          context = context_base+q;
          enable |= 1 << q;
          no_drop |= nodrop << q;
          eth_hbm_pld.base(base_addr,context);
          eth_hbm_pld.mem_sz(size,context);
          eth_hbm_ctl.base(base_addr_ctrl,context);
          eth_hbm_ctl.mem_sz(ctx_size,context);
          eth_hbm_port_pld[2].base(base_addr,q);
          eth_hbm_port_pld[2].mem_sz(size,q);
          eth_hbm_port_ctl[2].base(base_addr_ctrl,q);
          eth_hbm_port_ctl[2].mem_sz(ctx_size,q);
          eth_occ_thr[2].thr((size - headroom) / 16,q); //granularity of 1k
          eth_xoff_thr.xoff(xoff / 8 ,context); //granularity of 512   
          eth_xoff_thr.xon(xon / 8 ,context); //granularity of 512   
       }
    }
    pbc_csr.hbm.hbm_port_2.cfg_hbm_context.base(context_base);
    pbc_csr.hbm.hbm_port_2.cfg_hbm_context.xoff_timeout(timeout);
    pbc_csr.hbm.hbm_port_2.cfg_hbm_context.enable(enable);
    pbc_csr.hbm.hbm_port_2.cfg_hbm_context.no_drop(no_drop);
    pbc_csr.hbm.hbm_port_2.cfg_hbm_eth_payload_occupancy.all(eth_occ_thr[2].all());
    pbc_csr.hbm.hbm_port_2.cfg_hbm_eth_payload_occupancy.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_2.cfg_hbm_eth_payload_occupancy.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_occ_thr[2].show();}
    pbc_csr.hbm.hbm_port_2.cfg_hbm_eth_ctrl.all(eth_hbm_port_ctl[2].all());
    pbc_csr.hbm.hbm_port_2.cfg_hbm_eth_ctrl.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_2.cfg_hbm_eth_ctrl.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_ctl[2].show();}
    pbc_csr.hbm.hbm_port_2.cfg_hbm_eth_payload.all(eth_hbm_port_pld[2].all());
    pbc_csr.hbm.hbm_port_2.cfg_hbm_eth_payload.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_2.cfg_hbm_eth_payload.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_pld[2].show();}
    pbc_csr.hbm.hbm_port_2.cfg_hbm_context.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_2.cfg_hbm_context.show();}
    // port 3
    ppath =orig_path+ "/"+to_string(3);
    mpath=ppath+"/xoff_timeout";   timeout = sknobs_get_value((char *)mpath.c_str(), 0);
    // pre-parse 
    hbm_enabled_for_port = 0;
    for (uint32_t q=0; q<8; q++) {
       path = ppath+"/queue/"+to_string(q);
       mpath=path+"/hbm_en"; hbm_en = sknobs_get_value((char *)mpath.c_str(), 0);
       if (hbm_en) { hbm_enabled_for_port = 1;}
    }
    enable=0; no_drop=0;
    for (uint32_t q=0; q<8; q++) {
       path = ppath+"/queue/"+to_string(q);
       mpath=path+"/hbm_en"; if (hbm_enabled_for_port) { CHK_SKNOB_EXISTS(mpath); } hbm_en = sknobs_get_value((char *)mpath.c_str(), 0);
       if (hbm_en) {
            // per port, but for simplicity of impl, ready for every queue
          // mpath=ppath+"/context_base"; CHK_SKNOB_EXISTS(mpath); context_base = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/base_addr";     CHK_SKNOB_EXISTS(mpath); base_addr = sknobs_get_value((char *)mpath.c_str(), 0); 
          mpath=path+"/size";          CHK_SKNOB_EXISTS(mpath); size = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/headroom";      CHK_SKNOB_EXISTS(mpath); headroom = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/base_addr_ctrl";CHK_SKNOB_EXISTS(mpath); base_addr_ctrl = sknobs_get_value((char *)mpath.c_str(), 0);
          if (hbm_start > base_addr) { hbm_start = base_addr; }
          if (hbm_start > base_addr_ctrl) { hbm_start = base_addr_ctrl; }
          if (hbm_end < base_addr) { hbm_end = base_addr+size; }
          if (hbm_end < base_addr_ctrl) { hbm_end = base_addr_ctrl+((size+49)/50); }
#ifdef MODULE_SIM
          mpath=path+"/nodrop_rand";   nodrop_rand = sknobs_get_value((char *)mpath.c_str(), 0);
          if (nodrop_rand ==1) {
             nodrop = rand_knob.eval();
          } else {
#endif
             mpath=path+"/nodrop";         nodrop = sknobs_get_value((char *)mpath.c_str(), 0);
#ifdef MODULE_SIM
          }
#endif
          mpath=path+"/xoff";          CHK_SKNOB_EXISTS(mpath); xoff = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/xon";           CHK_SKNOB_EXISTS(mpath); xon = sknobs_get_value((char *)mpath.c_str(), 0);
          // program the last index  
          ctx_size  = (size+49)/50;
          ctx_size = (ctx_size > 0) ? ctx_size-1 : ctx_size;
          size = (size > 0) ? size-1 : size;
          context_base = 3*8;
          context = context_base+q;
          enable |= 1 << q;
          no_drop |= nodrop << q;
          eth_hbm_pld.base(base_addr,context);
          eth_hbm_pld.mem_sz(size,context);
          eth_hbm_ctl.base(base_addr_ctrl,context);
          eth_hbm_ctl.mem_sz(ctx_size,context);
          eth_hbm_port_pld[3].base(base_addr,q);
          eth_hbm_port_pld[3].mem_sz(size,q);
          eth_hbm_port_ctl[3].base(base_addr_ctrl,q);
          eth_hbm_port_ctl[3].mem_sz(ctx_size,q);
          eth_occ_thr[3].thr((size - headroom) / 16,q); //granularity of 1k
          eth_xoff_thr.xoff(xoff / 8 ,context); //granularity of 512   
          eth_xoff_thr.xon(xon / 8 ,context); //granularity of 512   
       }
    }
    pbc_csr.hbm.hbm_port_3.cfg_hbm_context.base(context_base);
    pbc_csr.hbm.hbm_port_3.cfg_hbm_context.xoff_timeout(timeout);
    pbc_csr.hbm.hbm_port_3.cfg_hbm_context.enable(enable);
    pbc_csr.hbm.hbm_port_3.cfg_hbm_context.no_drop(no_drop);
    pbc_csr.hbm.hbm_port_3.cfg_hbm_eth_payload_occupancy.all(eth_occ_thr[3].all());
    pbc_csr.hbm.hbm_port_3.cfg_hbm_eth_payload_occupancy.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_3.cfg_hbm_eth_payload_occupancy.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_occ_thr[3].show();}
    pbc_csr.hbm.hbm_port_3.cfg_hbm_eth_ctrl.all(eth_hbm_port_ctl[3].all());
    pbc_csr.hbm.hbm_port_3.cfg_hbm_eth_ctrl.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_3.cfg_hbm_eth_ctrl.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_ctl[3].show();}
    pbc_csr.hbm.hbm_port_3.cfg_hbm_eth_payload.all(eth_hbm_port_pld[3].all());
    pbc_csr.hbm.hbm_port_3.cfg_hbm_eth_payload.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_3.cfg_hbm_eth_payload.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_pld[3].show();}
    pbc_csr.hbm.hbm_port_3.cfg_hbm_context.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_3.cfg_hbm_context.show();}
    // port 4
    ppath =orig_path+ "/"+to_string(4);
    mpath=ppath+"/xoff_timeout";   timeout = sknobs_get_value((char *)mpath.c_str(), 0);
    // pre-parse 
    hbm_enabled_for_port = 0;
    for (uint32_t q=0; q<8; q++) {
       path = ppath+"/queue/"+to_string(q);
       mpath=path+"/hbm_en"; hbm_en = sknobs_get_value((char *)mpath.c_str(), 0);
       if (hbm_en) { hbm_enabled_for_port = 1;}
    }
    enable=0; no_drop=0;
    for (uint32_t q=0; q<8; q++) {
       path = ppath+"/queue/"+to_string(q);
       mpath=path+"/hbm_en"; if (hbm_enabled_for_port) { CHK_SKNOB_EXISTS(mpath); } hbm_en = sknobs_get_value((char *)mpath.c_str(), 0);
       if (hbm_en) {
            // per port, but for simplicity of impl, ready for every queue
          // mpath=ppath+"/context_base"; CHK_SKNOB_EXISTS(mpath); context_base = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/base_addr";     CHK_SKNOB_EXISTS(mpath); base_addr = sknobs_get_value((char *)mpath.c_str(), 0); 
          mpath=path+"/size";          CHK_SKNOB_EXISTS(mpath); size = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/headroom";      CHK_SKNOB_EXISTS(mpath); headroom = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/base_addr_ctrl";CHK_SKNOB_EXISTS(mpath); base_addr_ctrl = sknobs_get_value((char *)mpath.c_str(), 0);
          if (hbm_start > base_addr) { hbm_start = base_addr; }
          if (hbm_start > base_addr_ctrl) { hbm_start = base_addr_ctrl; }
          if (hbm_end < base_addr) { hbm_end = base_addr+size; }
          if (hbm_end < base_addr_ctrl) { hbm_end = base_addr_ctrl+((size+49)/50); }
#ifdef MODULE_SIM
          mpath=path+"/nodrop_rand";   nodrop_rand = sknobs_get_value((char *)mpath.c_str(), 0);
          if (nodrop_rand ==1) {
             nodrop = rand_knob.eval();
          } else {
#endif
             mpath=path+"/nodrop";         nodrop = sknobs_get_value((char *)mpath.c_str(), 0);
#ifdef MODULE_SIM
          }
#endif
          mpath=path+"/xoff";          CHK_SKNOB_EXISTS(mpath); xoff = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/xon";           CHK_SKNOB_EXISTS(mpath); xon = sknobs_get_value((char *)mpath.c_str(), 0);
          // program the last index  
          ctx_size  = (size+49)/50;
          ctx_size = (ctx_size > 0) ? ctx_size-1 : ctx_size;
          size = (size > 0) ? size-1 : size;
          context_base = ((4-4)*8)+4;
          context = context_base+q;
          enable |= 1 << q;
          no_drop |= nodrop << q;
          eth_hbm_pld.base(base_addr,context);
          eth_hbm_pld.mem_sz(size,context);
          eth_hbm_ctl.base(base_addr_ctrl,context);
          eth_hbm_ctl.mem_sz(ctx_size,context);
          eth_hbm_port_pld[4].base(base_addr,q);
          eth_hbm_port_pld[4].mem_sz(size,q);
          eth_hbm_port_ctl[4].base(base_addr_ctrl,q);
          eth_hbm_port_ctl[4].mem_sz(ctx_size,q);
          eth_occ_thr[4].thr((size - headroom) / 16,q); //granularity of 1k
          eth_xoff_thr.xoff(xoff / 8 ,context); //granularity of 512   
          eth_xoff_thr.xon(xon / 8 ,context); //granularity of 512   
       }
    }
    pbc_csr.hbm.hbm_port_4.cfg_hbm_context.base(context_base);
    pbc_csr.hbm.hbm_port_4.cfg_hbm_context.xoff_timeout(timeout);
    pbc_csr.hbm.hbm_port_4.cfg_hbm_context.enable(enable);
    pbc_csr.hbm.hbm_port_4.cfg_hbm_context.no_drop(no_drop);
    pbc_csr.hbm.hbm_port_4.cfg_hbm_eth_payload_occupancy.all(eth_occ_thr[4].all());
    pbc_csr.hbm.hbm_port_4.cfg_hbm_eth_payload_occupancy.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_4.cfg_hbm_eth_payload_occupancy.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_occ_thr[4].show();}
    pbc_csr.hbm.hbm_port_4.cfg_hbm_eth_ctrl.all(eth_hbm_port_ctl[4].all());
    pbc_csr.hbm.hbm_port_4.cfg_hbm_eth_ctrl.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_4.cfg_hbm_eth_ctrl.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_ctl[4].show();}
    pbc_csr.hbm.hbm_port_4.cfg_hbm_eth_payload.all(eth_hbm_port_pld[4].all());
    pbc_csr.hbm.hbm_port_4.cfg_hbm_eth_payload.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_4.cfg_hbm_eth_payload.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_pld[4].show();}
    pbc_csr.hbm.hbm_port_4.cfg_hbm_context.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_4.cfg_hbm_context.show();}
    // port 5
    ppath =orig_path+ "/"+to_string(5);
    mpath=ppath+"/xoff_timeout";   timeout = sknobs_get_value((char *)mpath.c_str(), 0);
    // pre-parse 
    hbm_enabled_for_port = 0;
    for (uint32_t q=0; q<8; q++) {
       path = ppath+"/queue/"+to_string(q);
       mpath=path+"/hbm_en"; hbm_en = sknobs_get_value((char *)mpath.c_str(), 0);
       if (hbm_en) { hbm_enabled_for_port = 1;}
    }
    enable=0; no_drop=0;
    for (uint32_t q=0; q<8; q++) {
       path = ppath+"/queue/"+to_string(q);
       mpath=path+"/hbm_en"; if (hbm_enabled_for_port) { CHK_SKNOB_EXISTS(mpath); } hbm_en = sknobs_get_value((char *)mpath.c_str(), 0);
       if (hbm_en) {
            // per port, but for simplicity of impl, ready for every queue
          // mpath=ppath+"/context_base"; CHK_SKNOB_EXISTS(mpath); context_base = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/base_addr";     CHK_SKNOB_EXISTS(mpath); base_addr = sknobs_get_value((char *)mpath.c_str(), 0); 
          mpath=path+"/size";          CHK_SKNOB_EXISTS(mpath); size = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/headroom";      CHK_SKNOB_EXISTS(mpath); headroom = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/base_addr_ctrl";CHK_SKNOB_EXISTS(mpath); base_addr_ctrl = sknobs_get_value((char *)mpath.c_str(), 0);
          if (hbm_start > base_addr) { hbm_start = base_addr; }
          if (hbm_start > base_addr_ctrl) { hbm_start = base_addr_ctrl; }
          if (hbm_end < base_addr) { hbm_end = base_addr+size; }
          if (hbm_end < base_addr_ctrl) { hbm_end = base_addr_ctrl+((size+49)/50); }
#ifdef MODULE_SIM
          mpath=path+"/nodrop_rand";   nodrop_rand = sknobs_get_value((char *)mpath.c_str(), 0);
          if (nodrop_rand ==1) {
             nodrop = rand_knob.eval();
          } else {
#endif
             mpath=path+"/nodrop";         nodrop = sknobs_get_value((char *)mpath.c_str(), 0);
#ifdef MODULE_SIM
          }
#endif
          mpath=path+"/xoff";          CHK_SKNOB_EXISTS(mpath); xoff = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/xon";           CHK_SKNOB_EXISTS(mpath); xon = sknobs_get_value((char *)mpath.c_str(), 0);
          // program the last index  
          ctx_size  = (size+49)/50;
          ctx_size = (ctx_size > 0) ? ctx_size-1 : ctx_size;
          size = (size > 0) ? size-1 : size;
          context_base = ((5-4)*8)+4;
          context = context_base+q;
          enable |= 1 << q;
          no_drop |= nodrop << q;
          eth_hbm_pld.base(base_addr,context);
          eth_hbm_pld.mem_sz(size,context);
          eth_hbm_ctl.base(base_addr_ctrl,context);
          eth_hbm_ctl.mem_sz(ctx_size,context);
          eth_hbm_port_pld[5].base(base_addr,q);
          eth_hbm_port_pld[5].mem_sz(size,q);
          eth_hbm_port_ctl[5].base(base_addr_ctrl,q);
          eth_hbm_port_ctl[5].mem_sz(ctx_size,q);
          eth_occ_thr[5].thr((size - headroom) / 16,q); //granularity of 1k
          eth_xoff_thr.xoff(xoff / 8 ,context); //granularity of 512   
          eth_xoff_thr.xon(xon / 8 ,context); //granularity of 512   
       }
    }
    pbc_csr.hbm.hbm_port_5.cfg_hbm_context.base(context_base);
    pbc_csr.hbm.hbm_port_5.cfg_hbm_context.xoff_timeout(timeout);
    pbc_csr.hbm.hbm_port_5.cfg_hbm_context.enable(enable);
    pbc_csr.hbm.hbm_port_5.cfg_hbm_context.no_drop(no_drop);
    pbc_csr.hbm.hbm_port_5.cfg_hbm_eth_payload_occupancy.all(eth_occ_thr[5].all());
    pbc_csr.hbm.hbm_port_5.cfg_hbm_eth_payload_occupancy.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_5.cfg_hbm_eth_payload_occupancy.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_occ_thr[5].show();}
    pbc_csr.hbm.hbm_port_5.cfg_hbm_eth_ctrl.all(eth_hbm_port_ctl[5].all());
    pbc_csr.hbm.hbm_port_5.cfg_hbm_eth_ctrl.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_5.cfg_hbm_eth_ctrl.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_ctl[5].show();}
    pbc_csr.hbm.hbm_port_5.cfg_hbm_eth_payload.all(eth_hbm_port_pld[5].all());
    pbc_csr.hbm.hbm_port_5.cfg_hbm_eth_payload.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_5.cfg_hbm_eth_payload.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_pld[5].show();}
    pbc_csr.hbm.hbm_port_5.cfg_hbm_context.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_5.cfg_hbm_context.show();}
    // port 6
    ppath =orig_path+ "/"+to_string(6);
    mpath=ppath+"/xoff_timeout";   timeout = sknobs_get_value((char *)mpath.c_str(), 0);
    // pre-parse 
    hbm_enabled_for_port = 0;
    for (uint32_t q=0; q<8; q++) {
       path = ppath+"/queue/"+to_string(q);
       mpath=path+"/hbm_en"; hbm_en = sknobs_get_value((char *)mpath.c_str(), 0);
       if (hbm_en) { hbm_enabled_for_port = 1;}
    }
    enable=0; no_drop=0;
    for (uint32_t q=0; q<8; q++) {
       path = ppath+"/queue/"+to_string(q);
       mpath=path+"/hbm_en"; if (hbm_enabled_for_port) { CHK_SKNOB_EXISTS(mpath); } hbm_en = sknobs_get_value((char *)mpath.c_str(), 0);
       if (hbm_en) {
            // per port, but for simplicity of impl, ready for every queue
          // mpath=ppath+"/context_base"; CHK_SKNOB_EXISTS(mpath); context_base = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/base_addr";     CHK_SKNOB_EXISTS(mpath); base_addr = sknobs_get_value((char *)mpath.c_str(), 0); 
          mpath=path+"/size";          CHK_SKNOB_EXISTS(mpath); size = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/headroom";      CHK_SKNOB_EXISTS(mpath); headroom = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/base_addr_ctrl";CHK_SKNOB_EXISTS(mpath); base_addr_ctrl = sknobs_get_value((char *)mpath.c_str(), 0);
          if (hbm_start > base_addr) { hbm_start = base_addr; }
          if (hbm_start > base_addr_ctrl) { hbm_start = base_addr_ctrl; }
          if (hbm_end < base_addr) { hbm_end = base_addr+size; }
          if (hbm_end < base_addr_ctrl) { hbm_end = base_addr_ctrl+((size+49)/50); }
#ifdef MODULE_SIM
          mpath=path+"/nodrop_rand";   nodrop_rand = sknobs_get_value((char *)mpath.c_str(), 0);
          if (nodrop_rand ==1) {
             nodrop = rand_knob.eval();
          } else {
#endif
             mpath=path+"/nodrop";         nodrop = sknobs_get_value((char *)mpath.c_str(), 0);
#ifdef MODULE_SIM
          }
#endif
          mpath=path+"/xoff";          CHK_SKNOB_EXISTS(mpath); xoff = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/xon";           CHK_SKNOB_EXISTS(mpath); xon = sknobs_get_value((char *)mpath.c_str(), 0);
          // program the last index  
          ctx_size  = (size+49)/50;
          ctx_size = (ctx_size > 0) ? ctx_size-1 : ctx_size;
          size = (size > 0) ? size-1 : size;
          context_base = ((6-4)*8)+4;
          context = context_base+q;
          enable |= 1 << q;
          no_drop |= nodrop << q;
          eth_hbm_pld.base(base_addr,context);
          eth_hbm_pld.mem_sz(size,context);
          eth_hbm_ctl.base(base_addr_ctrl,context);
          eth_hbm_ctl.mem_sz(ctx_size,context);
          eth_hbm_port_pld[6].base(base_addr,q);
          eth_hbm_port_pld[6].mem_sz(size,q);
          eth_hbm_port_ctl[6].base(base_addr_ctrl,q);
          eth_hbm_port_ctl[6].mem_sz(ctx_size,q);
          eth_occ_thr[6].thr((size - headroom) / 16,q); //granularity of 1k
          eth_xoff_thr.xoff(xoff / 8 ,context); //granularity of 512   
          eth_xoff_thr.xon(xon / 8 ,context); //granularity of 512   
       }
    }
    pbc_csr.hbm.hbm_port_6.cfg_hbm_context.base(context_base);
    pbc_csr.hbm.hbm_port_6.cfg_hbm_context.xoff_timeout(timeout);
    pbc_csr.hbm.hbm_port_6.cfg_hbm_context.enable(enable);
    pbc_csr.hbm.hbm_port_6.cfg_hbm_context.no_drop(no_drop);
    pbc_csr.hbm.hbm_port_6.cfg_hbm_eth_payload_occupancy.all(eth_occ_thr[6].all());
    pbc_csr.hbm.hbm_port_6.cfg_hbm_eth_payload_occupancy.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_6.cfg_hbm_eth_payload_occupancy.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_occ_thr[6].show();}
    pbc_csr.hbm.hbm_port_6.cfg_hbm_eth_ctrl.all(eth_hbm_port_ctl[6].all());
    pbc_csr.hbm.hbm_port_6.cfg_hbm_eth_ctrl.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_6.cfg_hbm_eth_ctrl.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_ctl[6].show();}
    pbc_csr.hbm.hbm_port_6.cfg_hbm_eth_payload.all(eth_hbm_port_pld[6].all());
    pbc_csr.hbm.hbm_port_6.cfg_hbm_eth_payload.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_6.cfg_hbm_eth_payload.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_pld[6].show();}
    pbc_csr.hbm.hbm_port_6.cfg_hbm_context.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_6.cfg_hbm_context.show();}
    // port 7
    ppath =orig_path+ "/"+to_string(7);
    mpath=ppath+"/xoff_timeout";   timeout = sknobs_get_value((char *)mpath.c_str(), 0);
    // pre-parse 
    hbm_enabled_for_port = 0;
    for (uint32_t q=0; q<8; q++) {
       path = ppath+"/queue/"+to_string(q);
       mpath=path+"/hbm_en"; hbm_en = sknobs_get_value((char *)mpath.c_str(), 0);
       if (hbm_en) { hbm_enabled_for_port = 1;}
    }
    enable=0; no_drop=0;
    for (uint32_t q=0; q<8; q++) {
       path = ppath+"/queue/"+to_string(q);
       mpath=path+"/hbm_en"; if (hbm_enabled_for_port) { CHK_SKNOB_EXISTS(mpath); } hbm_en = sknobs_get_value((char *)mpath.c_str(), 0);
       if (hbm_en) {
            // per port, but for simplicity of impl, ready for every queue
          // mpath=ppath+"/context_base"; CHK_SKNOB_EXISTS(mpath); context_base = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/base_addr";     CHK_SKNOB_EXISTS(mpath); base_addr = sknobs_get_value((char *)mpath.c_str(), 0); 
          mpath=path+"/size";          CHK_SKNOB_EXISTS(mpath); size = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/headroom";      CHK_SKNOB_EXISTS(mpath); headroom = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/base_addr_ctrl";CHK_SKNOB_EXISTS(mpath); base_addr_ctrl = sknobs_get_value((char *)mpath.c_str(), 0);
          if (hbm_start > base_addr) { hbm_start = base_addr; }
          if (hbm_start > base_addr_ctrl) { hbm_start = base_addr_ctrl; }
          if (hbm_end < base_addr) { hbm_end = base_addr+size; }
          if (hbm_end < base_addr_ctrl) { hbm_end = base_addr_ctrl+((size+49)/50); }
#ifdef MODULE_SIM
          mpath=path+"/nodrop_rand";   nodrop_rand = sknobs_get_value((char *)mpath.c_str(), 0);
          if (nodrop_rand ==1) {
             nodrop = rand_knob.eval();
          } else {
#endif
             mpath=path+"/nodrop";         nodrop = sknobs_get_value((char *)mpath.c_str(), 0);
#ifdef MODULE_SIM
          }
#endif
          mpath=path+"/xoff";          CHK_SKNOB_EXISTS(mpath); xoff = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/xon";           CHK_SKNOB_EXISTS(mpath); xon = sknobs_get_value((char *)mpath.c_str(), 0);
          // program the last index  
          ctx_size  = (size+49)/50;
          ctx_size = (ctx_size > 0) ? ctx_size-1 : ctx_size;
          size = (size > 0) ? size-1 : size;
          context_base = ((7-4)*8)+4;
          context = context_base+q;
          enable |= 1 << q;
          no_drop |= nodrop << q;
          eth_hbm_pld.base(base_addr,context);
          eth_hbm_pld.mem_sz(size,context);
          eth_hbm_ctl.base(base_addr_ctrl,context);
          eth_hbm_ctl.mem_sz(ctx_size,context);
          eth_hbm_port_pld[7].base(base_addr,q);
          eth_hbm_port_pld[7].mem_sz(size,q);
          eth_hbm_port_ctl[7].base(base_addr_ctrl,q);
          eth_hbm_port_ctl[7].mem_sz(ctx_size,q);
          eth_occ_thr[7].thr((size - headroom) / 16,q); //granularity of 1k
          eth_xoff_thr.xoff(xoff / 8 ,context); //granularity of 512   
          eth_xoff_thr.xon(xon / 8 ,context); //granularity of 512   
       }
    }
    pbc_csr.hbm.hbm_port_7.cfg_hbm_context.base(context_base);
    pbc_csr.hbm.hbm_port_7.cfg_hbm_context.xoff_timeout(timeout);
    pbc_csr.hbm.hbm_port_7.cfg_hbm_context.enable(enable);
    pbc_csr.hbm.hbm_port_7.cfg_hbm_context.no_drop(no_drop);
    pbc_csr.hbm.hbm_port_7.cfg_hbm_eth_payload_occupancy.all(eth_occ_thr[7].all());
    pbc_csr.hbm.hbm_port_7.cfg_hbm_eth_payload_occupancy.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_7.cfg_hbm_eth_payload_occupancy.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_occ_thr[7].show();}
    pbc_csr.hbm.hbm_port_7.cfg_hbm_eth_ctrl.all(eth_hbm_port_ctl[7].all());
    pbc_csr.hbm.hbm_port_7.cfg_hbm_eth_ctrl.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_7.cfg_hbm_eth_ctrl.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_ctl[7].show();}
    pbc_csr.hbm.hbm_port_7.cfg_hbm_eth_payload.all(eth_hbm_port_pld[7].all());
    pbc_csr.hbm.hbm_port_7.cfg_hbm_eth_payload.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_7.cfg_hbm_eth_payload.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_port_pld[7].show();}
    pbc_csr.hbm.hbm_port_7.cfg_hbm_context.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_7.cfg_hbm_context.show();}
    // port 9
    ppath =orig_path+ "/"+to_string(9);
    mpath=ppath+"/xoff_timeout";   timeout = sknobs_get_value((char *)mpath.c_str(), 0);
    // pre-parse 
    hbm_enabled_for_port = 0;
    for (uint32_t q=0; q<16; q++) {
       path = ppath+"/queue/"+to_string(q);
       mpath=path+"/hbm_en"; hbm_en = sknobs_get_value((char *)mpath.c_str(), 0);
       if (hbm_en) { hbm_enabled_for_port = 1;}
    }
    enable=0; no_drop=0;
    for (uint32_t q=0; q<16; q++) {
       path = ppath+"/queue/"+to_string(q);
       mpath=path+"/hbm_en"; if (hbm_enabled_for_port) { CHK_SKNOB_EXISTS(mpath); } hbm_en = sknobs_get_value((char *)mpath.c_str(), 0);
       if (hbm_en) {
          mpath=path+"/base_addr";     CHK_SKNOB_EXISTS(mpath); base_addr = sknobs_get_value((char *)mpath.c_str(), 0); 
          mpath=path+"/size";          CHK_SKNOB_EXISTS(mpath); size = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/headroom";      CHK_SKNOB_EXISTS(mpath); headroom = sknobs_get_value((char *)mpath.c_str(), 0);
          mpath=path+"/base_addr_ctrl";CHK_SKNOB_EXISTS(mpath); base_addr_ctrl = sknobs_get_value((char *)mpath.c_str(), 0);
          if (hbm_start > base_addr) { hbm_start = base_addr; }
          if (hbm_start > base_addr_ctrl) { hbm_start = base_addr_ctrl; }
          if (hbm_end < base_addr) { hbm_end = base_addr+size; }
          if (hbm_end < base_addr_ctrl) { hbm_end = base_addr_ctrl+((size+49)/50); }
          // program the last index  
          ctx_size  = (size+49)/50;
          ctx_size = (ctx_size > 0) ? ctx_size-1 : ctx_size;
          size = (size > 0) ? size-1 : size;
          context = q;
          enable |= 1 << q;
          tx_hbm_pld.base(base_addr,context);
          tx_hbm_pld.mem_sz(size,context);
          tx_hbm_ctl.base(base_addr_ctrl,context);
          tx_hbm_ctl.mem_sz(ctx_size,context);
          tx_occ_thr.thr((size - headroom) / 16,context); //granularity of 1k
       }
    }
    pbc_csr.hbm.hbm_port_9.cfg_hbm_context.enable(enable);
    pbc_csr.hbm.hbm_port_9.cfg_hbm_tx_payload_occupancy.all(tx_occ_thr.all());
    pbc_csr.hbm.hbm_port_9.cfg_hbm_tx_payload_occupancy.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_9.cfg_hbm_tx_payload_occupancy.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {tx_occ_thr.show();}
    pbc_csr.hbm.hbm_port_9.cfg_hbm_tx_payload.all(tx_hbm_pld.all());
    pbc_csr.hbm.hbm_port_9.cfg_hbm_tx_payload.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_9.cfg_hbm_tx_payload.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {tx_hbm_pld.show();}
    pbc_csr.hbm.hbm_port_9.cfg_hbm_context.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.hbm_port_9.cfg_hbm_context.show();}
    pbc_csr.hbm.cfg_hbm_eth_ctrl.all(eth_hbm_ctl.all());
    pbc_csr.hbm.cfg_hbm_eth_ctrl.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.cfg_hbm_eth_ctrl.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_ctl.show();}
    pbc_csr.hbm.cfg_hbm_eth_payload.all(eth_hbm_pld.all());
    pbc_csr.hbm.cfg_hbm_eth_payload.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.cfg_hbm_eth_payload.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_hbm_pld.show();}
    pbc_csr.hbm.cfg_hbm_threshold.all(eth_xoff_thr.all());
    pbc_csr.hbm.cfg_hbm_threshold.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.cfg_hbm_threshold.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {eth_xoff_thr.show();}

    pbc_csr.hbm.cfg_hbm_tx_ctrl.all(tx_hbm_ctl.all());
    pbc_csr.hbm.cfg_hbm_tx_ctrl.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.cfg_hbm_tx_ctrl.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {tx_hbm_ctl.show();}
    pbc_csr.hbm.cfg_hbm_tx_payload.all(tx_hbm_pld.all());
    pbc_csr.hbm.cfg_hbm_tx_payload.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.hbm.cfg_hbm_tx_payload.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {tx_hbm_pld.show();}
 
    PLOG_MSG ("cap_pb_hbm_thr_from_api : PB HBM overflow : start address = 0x" << hex << hbm_start << " end_address = 0x"<< hbm_end <<dec << "total_mem = " << (hbm_end-hbm_start+1)*64 << endl);
    mpath = pbc_csr.get_hier_path()+".total_pb_hbm_buffer";
    std::replace( mpath.begin(), mpath.end(), '.', '/');
    sknobs_set_value((char*)mpath.c_str(), (hbm_end-hbm_start+1)*64);
    // uint32_t tmppp = sknobs_get_value((char *)mpath.c_str(), 0);
    // PLOG_MSG("path is " << mpath<< " value is "<< tmppp << endl);
}


bool cap_pb_adjust_island_max(int chip_id, int inst_id) {
PLOG_MSG("inside cap_pb_adjust_island_max \n");

 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
 //cap_pbm_csr_t & pbm_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbm_csr_t, chip_id, inst_id);

 int island[2];
 int padding[2];
 int lo,hi,hi_idx;
 island[0] = 0; island[1]=0;
 padding[0] = 2*2; padding[1]=10*2;

    island[1]+=(pbc_csr.port_0.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_0.cfg_account_pg_0.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_0.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_0.cfg_account_pg_1.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_0.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_0.cfg_account_pg_2.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_0.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_0.cfg_account_pg_3.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_0.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_0.cfg_account_pg_4.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_0.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_0.cfg_account_pg_5.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_0.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_0.cfg_account_pg_6.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_0.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_0.cfg_account_pg_7.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_1.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_1.cfg_account_pg_0.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_1.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_1.cfg_account_pg_1.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_1.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_1.cfg_account_pg_2.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_1.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_1.cfg_account_pg_3.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_1.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_1.cfg_account_pg_4.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_1.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_1.cfg_account_pg_5.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_1.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_1.cfg_account_pg_6.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_1.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_1.cfg_account_pg_7.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_2.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_2.cfg_account_pg_0.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_2.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_2.cfg_account_pg_1.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_2.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_2.cfg_account_pg_2.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_2.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_2.cfg_account_pg_3.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_2.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_2.cfg_account_pg_4.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_2.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_2.cfg_account_pg_5.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_2.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_2.cfg_account_pg_6.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_2.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_2.cfg_account_pg_7.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_3.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_3.cfg_account_pg_0.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_3.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_3.cfg_account_pg_1.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_3.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_3.cfg_account_pg_2.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_3.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_3.cfg_account_pg_3.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_3.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_3.cfg_account_pg_4.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_3.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_3.cfg_account_pg_5.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_3.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_3.cfg_account_pg_6.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_3.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_3.cfg_account_pg_7.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_4.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_4.cfg_account_pg_0.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_4.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_4.cfg_account_pg_1.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_4.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_4.cfg_account_pg_2.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_4.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_4.cfg_account_pg_3.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_4.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_4.cfg_account_pg_4.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_4.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_4.cfg_account_pg_5.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_4.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_4.cfg_account_pg_6.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_4.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_4.cfg_account_pg_7.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_5.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_5.cfg_account_pg_0.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_5.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_5.cfg_account_pg_1.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_5.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_5.cfg_account_pg_2.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_5.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_5.cfg_account_pg_3.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_5.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_5.cfg_account_pg_4.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_5.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_5.cfg_account_pg_5.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_5.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_5.cfg_account_pg_6.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_5.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_5.cfg_account_pg_7.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_6.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_6.cfg_account_pg_0.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_6.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_6.cfg_account_pg_1.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_6.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_6.cfg_account_pg_2.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_6.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_6.cfg_account_pg_3.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_6.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_6.cfg_account_pg_4.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_6.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_6.cfg_account_pg_5.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_6.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_6.cfg_account_pg_6.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_6.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_6.cfg_account_pg_7.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_7.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_7.cfg_account_pg_0.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_7.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_7.cfg_account_pg_1.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_7.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_7.cfg_account_pg_2.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_7.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_7.cfg_account_pg_3.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_7.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_7.cfg_account_pg_4.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_7.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_7.cfg_account_pg_5.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_7.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_7.cfg_account_pg_6.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_7.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_7.cfg_account_pg_7.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_8.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_8.cfg_account_pg_0.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_8.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_8.cfg_account_pg_1.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_8.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_8.cfg_account_pg_2.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_8.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_8.cfg_account_pg_3.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_8.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_8.cfg_account_pg_4.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_8.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_8.cfg_account_pg_5.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_8.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_8.cfg_account_pg_6.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_8.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_8.cfg_account_pg_7.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_9.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_0.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_9.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_1.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_9.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_2.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_9.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_3.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_9.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_4.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_9.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_5.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_9.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_6.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_9.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_7.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_9.cfg_account_pg_8.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_8.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_9.cfg_account_pg_9.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_9.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_9.cfg_account_pg_10.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_10.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_9.cfg_account_pg_11.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_11.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_9.cfg_account_pg_12.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_12.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_9.cfg_account_pg_13.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_13.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_9.cfg_account_pg_14.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_14.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_9.cfg_account_pg_15.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_15.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_0.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_1.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_2.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_3.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_4.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_5.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_6.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_7.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_8.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_8.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_9.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_9.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_10.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_10.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_11.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_11.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_12.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_12.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_13.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_13.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_14.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_14.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_15.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_15.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_16.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_16.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_17.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_17.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_18.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_18.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_19.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_19.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_20.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_20.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_21.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_21.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_22.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_22.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_23.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_23.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_24.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_24.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_25.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_25.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_26.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_26.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_27.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_27.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_28.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_28.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_29.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_29.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_30.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_30.headroom().convert_to<uint32_t>()*4);
    island[1]+=(pbc_csr.port_10.cfg_account_pg_31.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_31.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_0.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_1.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_2.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_3.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_4.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_5.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_6.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_7.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_8.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_8.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_9.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_9.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_10.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_10.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_11.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_11.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_12.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_12.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_13.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_13.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_14.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_14.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_15.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_15.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_16.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_16.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_17.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_17.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_18.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_18.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_19.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_19.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_20.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_20.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_21.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_21.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_22.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_22.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_23.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_23.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_24.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_24.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_25.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_25.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_26.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_26.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_27.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_27.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_28.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_28.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_29.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_29.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_30.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_30.headroom().convert_to<uint32_t>()*4);
    island[0]+=(pbc_csr.port_11.cfg_account_pg_31.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_31.headroom().convert_to<uint32_t>()*4);
 if (island[0] > island[1]) {
    hi = island[0] ; lo = island[1]; hi_idx=0;
 } else {
    hi = island[1] ; lo = island[0]; hi_idx=1;
 }
 if (hi > (8*1024) || lo > (5*1024)) {
    PLOG_ERR ("cap_pb_adjust_island_max : invalid configuration hi = " << hi << " lo = "<< lo << endl);
    return false;
 } else {
    PLOG_MSG ("cap_pb_adjust_island_max : w/o padding hi = " << hi << " lo = "<< lo << endl);
    hi = (hi+padding[hi_idx]) > 8*1024 ? 8*1024 : hi+padding[hi_idx];
    lo = (lo+padding[(hi_idx+1)%2]) > 8*1024 ? 5*1024 : lo+padding[(hi_idx+1)%2];
    PLOG_MSG ("cap_pb_adjust_island_max : with padding hi = " << hi << " lo = "<< lo << endl);
    if (hi_idx == 0) {
       PLOG_MSG ("cap_pb_adjust_island_max : programming island 0 max = " << hi << " island 1 max = "<< lo << endl);
       pbc_csr.cfg_fc_mgr_0.min_cell(0);
       pbc_csr.cfg_fc_mgr_0.max_row((hi%2 ==0 ) ? hi/2 : (hi/2)+1);
       pbc_csr.cfg_fc_mgr_1.min_cell(4096);
       pbc_csr.cfg_fc_mgr_1.max_row((lo%2 ==0 ) ? lo/2 : (lo/2)+1);
       pbc_csr.cfg_island_control.map(0);
    } else {
       PLOG_MSG ("cap_pb_adjust_island_max : programming island 0 max = " << lo << " island 1 max = "<< hi << endl);
       pbc_csr.cfg_fc_mgr_1.min_cell(0);
       pbc_csr.cfg_fc_mgr_1.max_row((hi%2 ==0 ) ? hi/2 : (hi/2)+1);
       pbc_csr.cfg_fc_mgr_0.min_cell(4096);
       pbc_csr.cfg_fc_mgr_0.max_row((lo%2 ==0 ) ? lo/2 : (lo/2)+1);
       pbc_csr.cfg_island_control.map(1);
    }
    pbc_csr.cfg_fc_mgr_0.init_start(1);
    pbc_csr.cfg_fc_mgr_0.write();
    pbc_csr.cfg_fc_mgr_1.init_start(1);
    pbc_csr.cfg_fc_mgr_1.write();
    pbc_csr.cfg_island_control.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.cfg_fc_mgr_0.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.cfg_fc_mgr_1.show();}
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.cfg_island_control.show();}

    // poll for done
    // cap_pb_init_done(chip_id,inst_id);
    return true;
 }
}

// API assumes P4 interfaces have credits programmed
void cap_pb_check_island_split(int chip_id, int inst_id) {
PLOG_MSG("inside cap_pb_check_island_split \n");

 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
 //cap_pbm_csr_t & pbm_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbm_csr_t, chip_id, inst_id);

 int cnt[2];
 int island[2];
 int padding[2];
 int eth_total=0,rxd_total=0,rwr_total=0,pck_total=0;
 uint32_t recirc_oq[12];

 island[0] = pbc_csr.cfg_fc_mgr_0.max_row().convert_to<uint32_t>()*2;
 island[1] = pbc_csr.cfg_fc_mgr_1.max_row().convert_to<uint32_t>()*2;
 padding[0] = 2*2; padding[1]=10*2;

    recirc_oq[10] = pbc_csr.port_10.cfg_write_control.recirc_oq().convert_to<uint32_t>();
    recirc_oq[11] = pbc_csr.port_11.cfg_write_control.recirc_oq().convert_to<uint32_t>();
    eth_total+=(pbc_csr.port_0.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_0.cfg_account_pg_0.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_0.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_0.cfg_account_pg_1.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_0.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_0.cfg_account_pg_2.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_0.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_0.cfg_account_pg_3.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_0.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_0.cfg_account_pg_4.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_0.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_0.cfg_account_pg_5.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_0.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_0.cfg_account_pg_6.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_0.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_0.cfg_account_pg_7.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_1.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_1.cfg_account_pg_0.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_1.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_1.cfg_account_pg_1.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_1.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_1.cfg_account_pg_2.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_1.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_1.cfg_account_pg_3.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_1.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_1.cfg_account_pg_4.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_1.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_1.cfg_account_pg_5.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_1.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_1.cfg_account_pg_6.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_1.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_1.cfg_account_pg_7.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_2.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_2.cfg_account_pg_0.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_2.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_2.cfg_account_pg_1.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_2.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_2.cfg_account_pg_2.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_2.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_2.cfg_account_pg_3.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_2.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_2.cfg_account_pg_4.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_2.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_2.cfg_account_pg_5.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_2.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_2.cfg_account_pg_6.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_2.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_2.cfg_account_pg_7.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_3.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_3.cfg_account_pg_0.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_3.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_3.cfg_account_pg_1.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_3.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_3.cfg_account_pg_2.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_3.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_3.cfg_account_pg_3.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_3.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_3.cfg_account_pg_4.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_3.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_3.cfg_account_pg_5.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_3.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_3.cfg_account_pg_6.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_3.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_3.cfg_account_pg_7.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_4.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_4.cfg_account_pg_0.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_4.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_4.cfg_account_pg_1.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_4.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_4.cfg_account_pg_2.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_4.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_4.cfg_account_pg_3.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_4.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_4.cfg_account_pg_4.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_4.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_4.cfg_account_pg_5.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_4.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_4.cfg_account_pg_6.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_4.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_4.cfg_account_pg_7.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_5.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_5.cfg_account_pg_0.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_5.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_5.cfg_account_pg_1.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_5.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_5.cfg_account_pg_2.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_5.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_5.cfg_account_pg_3.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_5.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_5.cfg_account_pg_4.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_5.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_5.cfg_account_pg_5.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_5.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_5.cfg_account_pg_6.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_5.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_5.cfg_account_pg_7.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_6.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_6.cfg_account_pg_0.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_6.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_6.cfg_account_pg_1.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_6.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_6.cfg_account_pg_2.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_6.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_6.cfg_account_pg_3.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_6.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_6.cfg_account_pg_4.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_6.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_6.cfg_account_pg_5.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_6.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_6.cfg_account_pg_6.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_6.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_6.cfg_account_pg_7.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_7.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_7.cfg_account_pg_0.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_7.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_7.cfg_account_pg_1.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_7.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_7.cfg_account_pg_2.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_7.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_7.cfg_account_pg_3.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_7.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_7.cfg_account_pg_4.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_7.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_7.cfg_account_pg_5.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_7.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_7.cfg_account_pg_6.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_7.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_7.cfg_account_pg_7.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_8.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_8.cfg_account_pg_0.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_8.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_8.cfg_account_pg_1.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_8.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_8.cfg_account_pg_2.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_8.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_8.cfg_account_pg_3.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_8.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_8.cfg_account_pg_4.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_8.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_8.cfg_account_pg_5.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_8.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_8.cfg_account_pg_6.headroom().convert_to<uint32_t>()*4);
    eth_total+=(pbc_csr.port_8.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_8.cfg_account_pg_7.headroom().convert_to<uint32_t>()*4);
    rxd_total+=(pbc_csr.port_9.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_0.headroom().convert_to<uint32_t>()*4);
    rxd_total+=(pbc_csr.port_9.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_1.headroom().convert_to<uint32_t>()*4);
    rxd_total+=(pbc_csr.port_9.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_2.headroom().convert_to<uint32_t>()*4);
    rxd_total+=(pbc_csr.port_9.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_3.headroom().convert_to<uint32_t>()*4);
    rxd_total+=(pbc_csr.port_9.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_4.headroom().convert_to<uint32_t>()*4);
    rxd_total+=(pbc_csr.port_9.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_5.headroom().convert_to<uint32_t>()*4);
    rxd_total+=(pbc_csr.port_9.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_6.headroom().convert_to<uint32_t>()*4);
    rxd_total+=(pbc_csr.port_9.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_7.headroom().convert_to<uint32_t>()*4);
    rxd_total+=(pbc_csr.port_9.cfg_account_pg_8.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_8.headroom().convert_to<uint32_t>()*4);
    rxd_total+=(pbc_csr.port_9.cfg_account_pg_9.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_9.headroom().convert_to<uint32_t>()*4);
    rxd_total+=(pbc_csr.port_9.cfg_account_pg_10.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_10.headroom().convert_to<uint32_t>()*4);
    rxd_total+=(pbc_csr.port_9.cfg_account_pg_11.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_11.headroom().convert_to<uint32_t>()*4);
    rxd_total+=(pbc_csr.port_9.cfg_account_pg_12.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_12.headroom().convert_to<uint32_t>()*4);
    rxd_total+=(pbc_csr.port_9.cfg_account_pg_13.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_13.headroom().convert_to<uint32_t>()*4);
    rxd_total+=(pbc_csr.port_9.cfg_account_pg_14.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_14.headroom().convert_to<uint32_t>()*4);
    rxd_total+=(pbc_csr.port_9.cfg_account_pg_15.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_9.cfg_account_pg_15.headroom().convert_to<uint32_t>()*4);
    if (0 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_0.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[0].entry().convert_to<uint32_t>();
    }
    if (1 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_1.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[1].entry().convert_to<uint32_t>();
    }
    if (2 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_2.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[2].entry().convert_to<uint32_t>();
    }
    if (3 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_3.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[3].entry().convert_to<uint32_t>();
    }
    if (4 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_4.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[4].entry().convert_to<uint32_t>();
    }
    if (5 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_5.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[5].entry().convert_to<uint32_t>();
    }
    if (6 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_6.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[6].entry().convert_to<uint32_t>();
    }
    if (7 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_7.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[7].entry().convert_to<uint32_t>();
    }
    if (8 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_8.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_8.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[8].entry().convert_to<uint32_t>();
    }
    if (9 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_9.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_9.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[9].entry().convert_to<uint32_t>();
    }
    if (10 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_10.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_10.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[10].entry().convert_to<uint32_t>();
    }
    if (11 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_11.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_11.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[11].entry().convert_to<uint32_t>();
    }
    if (12 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_12.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_12.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[12].entry().convert_to<uint32_t>();
    }
    if (13 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_13.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_13.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[13].entry().convert_to<uint32_t>();
    }
    if (14 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_14.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_14.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[14].entry().convert_to<uint32_t>();
    }
    if (15 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_15.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_15.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[15].entry().convert_to<uint32_t>();
    }
    if (16 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_16.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_16.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[16].entry().convert_to<uint32_t>();
    }
    if (17 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_17.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_17.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[17].entry().convert_to<uint32_t>();
    }
    if (18 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_18.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_18.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[18].entry().convert_to<uint32_t>();
    }
    if (19 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_19.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_19.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[19].entry().convert_to<uint32_t>();
    }
    if (20 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_20.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_20.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[20].entry().convert_to<uint32_t>();
    }
    if (21 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_21.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_21.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[21].entry().convert_to<uint32_t>();
    }
    if (22 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_22.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_22.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[22].entry().convert_to<uint32_t>();
    }
    if (23 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_23.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_23.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[23].entry().convert_to<uint32_t>();
    }
    if (24 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_24.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_24.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[24].entry().convert_to<uint32_t>();
    }
    if (25 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_25.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_25.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[25].entry().convert_to<uint32_t>();
    }
    if (26 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_26.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_26.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[26].entry().convert_to<uint32_t>();
    }
    if (27 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_27.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_27.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[27].entry().convert_to<uint32_t>();
    }
    if (28 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_28.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_28.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[28].entry().convert_to<uint32_t>();
    }
    if (29 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_29.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_29.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[29].entry().convert_to<uint32_t>();
    }
    if (30 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_30.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_30.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[30].entry().convert_to<uint32_t>();
    }
    if (31 == recirc_oq[10]) {
       rwr_total+=(pbc_csr.port_10.cfg_account_pg_31.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_10.cfg_account_pg_31.headroom().convert_to<uint32_t>()*4);
    } else {
       rwr_total+= pbc_csr.port_10.dhs_oq_flow_control.entry[31].entry().convert_to<uint32_t>();
    }
    if (0 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_0.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_0.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[0].entry().convert_to<uint32_t>();
    }
    if (1 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_1.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_1.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[1].entry().convert_to<uint32_t>();
    }
    if (2 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_2.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_2.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[2].entry().convert_to<uint32_t>();
    }
    if (3 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_3.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_3.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[3].entry().convert_to<uint32_t>();
    }
    if (4 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_4.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_4.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[4].entry().convert_to<uint32_t>();
    }
    if (5 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_5.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_5.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[5].entry().convert_to<uint32_t>();
    }
    if (6 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_6.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_6.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[6].entry().convert_to<uint32_t>();
    }
    if (7 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_7.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_7.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[7].entry().convert_to<uint32_t>();
    }
    if (8 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_8.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_8.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[8].entry().convert_to<uint32_t>();
    }
    if (9 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_9.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_9.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[9].entry().convert_to<uint32_t>();
    }
    if (10 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_10.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_10.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[10].entry().convert_to<uint32_t>();
    }
    if (11 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_11.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_11.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[11].entry().convert_to<uint32_t>();
    }
    if (12 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_12.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_12.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[12].entry().convert_to<uint32_t>();
    }
    if (13 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_13.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_13.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[13].entry().convert_to<uint32_t>();
    }
    if (14 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_14.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_14.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[14].entry().convert_to<uint32_t>();
    }
    if (15 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_15.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_15.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[15].entry().convert_to<uint32_t>();
    }
    if (16 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_16.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_16.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[16].entry().convert_to<uint32_t>();
    }
    if (17 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_17.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_17.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[17].entry().convert_to<uint32_t>();
    }
    if (18 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_18.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_18.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[18].entry().convert_to<uint32_t>();
    }
    if (19 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_19.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_19.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[19].entry().convert_to<uint32_t>();
    }
    if (20 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_20.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_20.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[20].entry().convert_to<uint32_t>();
    }
    if (21 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_21.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_21.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[21].entry().convert_to<uint32_t>();
    }
    if (22 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_22.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_22.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[22].entry().convert_to<uint32_t>();
    }
    if (23 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_23.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_23.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[23].entry().convert_to<uint32_t>();
    }
    if (24 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_24.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_24.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[24].entry().convert_to<uint32_t>();
    }
    if (25 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_25.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_25.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[25].entry().convert_to<uint32_t>();
    }
    if (26 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_26.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_26.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[26].entry().convert_to<uint32_t>();
    }
    if (27 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_27.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_27.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[27].entry().convert_to<uint32_t>();
    }
    if (28 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_28.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_28.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[28].entry().convert_to<uint32_t>();
    }
    if (29 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_29.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_29.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[29].entry().convert_to<uint32_t>();
    }
    if (30 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_30.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_30.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[30].entry().convert_to<uint32_t>();
    }
    if (31 == recirc_oq[11]) {
       pck_total+=(pbc_csr.port_11.cfg_account_pg_31.reserved_min().convert_to<uint32_t>()*4) + (pbc_csr.port_11.cfg_account_pg_31.headroom().convert_to<uint32_t>()*4);
    } else {
       pck_total+= pbc_csr.port_11.dhs_oq_flow_control.entry[31].entry().convert_to<uint32_t>();
    }
 cnt[0] = rxd_total + pck_total;
 cnt[1] = eth_total + rwr_total;

    PLOG_MSG ("cap_pb_check_island_split : island0 : rxd_total = " << rxd_total << " pck_total = "<< pck_total << endl);
    PLOG_MSG ("cap_pb_check_island_split : island1 : eth_total = " << eth_total << " rwr_total = "<< rwr_total << endl);

 if (cnt[0]+padding[0] > island[0]) {
    PLOG_ERR ("cap_pb_check_island_split : invalid configuration island0_max = " << island[0] << " cnt = "<< cnt[0] << " padding = "<< padding[0] << endl);
 } else {
    PLOG_MSG ("cap_pb_check_island_split : valid configuration island0_max = " << island[0] << " cnt = "<< cnt[0] << " padding = "<< padding[0] << endl);
 }
 if (cnt[1]+padding[1] > island[1]) {
    PLOG_ERR ("cap_pb_check_island_split : invalid configuration island1_max = " << island[1] << " cnt = "<< cnt[1] << " padding = "<< padding[1] << endl);
 } else {
    PLOG_MSG ("cap_pb_check_island_split : valid configuration island1_max = " << island[1] << " cnt = "<< cnt[1] << " padding = "<< padding[1] << endl);
 }
}


void cap_pb_oq_ctl(int chip_id, int inst_id, uint32_t enable) {
PLOG_MSG("called cap_pb_oq_ctl with enable =" << enable << endl);

 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
 //cap_pbm_csr_t & pbm_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbm_csr_t, chip_id, inst_id);

    pbc_csr.port_0.cfg_oq.enable(enable);
    pbc_csr.port_0.cfg_oq.rewrite_enable(1);
    pbc_csr.port_0.cfg_oq.write();
    //if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_0.cfg_oq.show();}
    pbc_csr.port_1.cfg_oq.enable(enable);
    pbc_csr.port_1.cfg_oq.rewrite_enable(1);
    pbc_csr.port_1.cfg_oq.write();
    //if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_1.cfg_oq.show();}
    pbc_csr.port_2.cfg_oq.enable(enable);
    pbc_csr.port_2.cfg_oq.rewrite_enable(1);
    pbc_csr.port_2.cfg_oq.write();
    //if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_2.cfg_oq.show();}
    pbc_csr.port_3.cfg_oq.enable(enable);
    pbc_csr.port_3.cfg_oq.rewrite_enable(1);
    pbc_csr.port_3.cfg_oq.write();
    //if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_3.cfg_oq.show();}
    pbc_csr.port_4.cfg_oq.enable(enable);
    pbc_csr.port_4.cfg_oq.rewrite_enable(1);
    pbc_csr.port_4.cfg_oq.write();
    //if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_4.cfg_oq.show();}
    pbc_csr.port_5.cfg_oq.enable(enable);
    pbc_csr.port_5.cfg_oq.rewrite_enable(1);
    pbc_csr.port_5.cfg_oq.write();
    //if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_5.cfg_oq.show();}
    pbc_csr.port_6.cfg_oq.enable(enable);
    pbc_csr.port_6.cfg_oq.rewrite_enable(1);
    pbc_csr.port_6.cfg_oq.write();
    //if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_6.cfg_oq.show();}
    pbc_csr.port_7.cfg_oq.enable(enable);
    pbc_csr.port_7.cfg_oq.rewrite_enable(1);
    pbc_csr.port_7.cfg_oq.write();
    //if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_7.cfg_oq.show();}
    pbc_csr.port_8.cfg_oq.enable(enable);
    pbc_csr.port_8.cfg_oq.rewrite_enable(1);
    pbc_csr.port_8.cfg_oq.write();
    //if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_8.cfg_oq.show();}
    pbc_csr.port_9.cfg_oq.enable(enable);
    pbc_csr.port_9.cfg_oq.rewrite_enable(1);
    pbc_csr.port_9.cfg_oq.write();
    //if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_oq.show();}
    pbc_csr.port_10.cfg_oq.enable(enable);
    pbc_csr.port_10.cfg_oq.rewrite_enable(1);
    pbc_csr.port_10.cfg_oq.write();
    //if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_oq.show();}
    pbc_csr.port_11.cfg_oq.enable(enable);
    pbc_csr.port_11.cfg_oq.rewrite_enable(1);
    pbc_csr.port_11.cfg_oq.write();
    //if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_oq.show();}
}


void cap_pb_hbm_force_xon(int chip_id, int inst_id, int pri, uint32_t enable) {
PLOG_MSG("called cap_pb_hbm_force_xon with enable =" << enable << endl);
											 
 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);

    uint32_t mod_xon;
											 
    mod_xon = pbc_csr.hbm.cfg_hbm_eth_xoff_force.xon().convert_to<uint32_t>();

    if (enable) {
       mod_xon |= (1<<pri);
    }
    else {
       mod_xon &= ~(1<<pri);
    }										     
 											 
    pbc_csr.hbm.cfg_hbm_eth_xoff_force.xon(mod_xon);
    pbc_csr.hbm.cfg_hbm_eth_xoff_force.write();

}

  
void cap_pb_bump_up_min_resv(int chip_id, int inst_id) {
PLOG_MSG("called cap_pb_bump_up_min_resv "  << endl);

 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
 //cap_pbm_csr_t & pbm_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbm_csr_t, chip_id, inst_id);

    pbc_csr.port_0.cfg_account_pg_0.reserved_min(100);
    pbc_csr.port_0.cfg_account_pg_0.headroom(0);
    pbc_csr.port_0.cfg_account_pg_0.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_0.cfg_account_pg_0.show();}
    pbc_csr.port_0.cfg_account_pg_1.reserved_min(100);
    pbc_csr.port_0.cfg_account_pg_1.headroom(0);
    pbc_csr.port_0.cfg_account_pg_1.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_0.cfg_account_pg_1.show();}
    pbc_csr.port_0.cfg_account_pg_2.reserved_min(100);
    pbc_csr.port_0.cfg_account_pg_2.headroom(0);
    pbc_csr.port_0.cfg_account_pg_2.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_0.cfg_account_pg_2.show();}
    pbc_csr.port_0.cfg_account_pg_3.reserved_min(100);
    pbc_csr.port_0.cfg_account_pg_3.headroom(0);
    pbc_csr.port_0.cfg_account_pg_3.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_0.cfg_account_pg_3.show();}
    pbc_csr.port_0.cfg_account_pg_4.reserved_min(100);
    pbc_csr.port_0.cfg_account_pg_4.headroom(0);
    pbc_csr.port_0.cfg_account_pg_4.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_0.cfg_account_pg_4.show();}
    pbc_csr.port_0.cfg_account_pg_5.reserved_min(100);
    pbc_csr.port_0.cfg_account_pg_5.headroom(0);
    pbc_csr.port_0.cfg_account_pg_5.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_0.cfg_account_pg_5.show();}
    pbc_csr.port_0.cfg_account_pg_6.reserved_min(100);
    pbc_csr.port_0.cfg_account_pg_6.headroom(0);
    pbc_csr.port_0.cfg_account_pg_6.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_0.cfg_account_pg_6.show();}
    pbc_csr.port_0.cfg_account_pg_7.reserved_min(100);
    pbc_csr.port_0.cfg_account_pg_7.headroom(0);
    pbc_csr.port_0.cfg_account_pg_7.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_0.cfg_account_pg_7.show();}
    pbc_csr.port_1.cfg_account_pg_0.reserved_min(100);
    pbc_csr.port_1.cfg_account_pg_0.headroom(0);
    pbc_csr.port_1.cfg_account_pg_0.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_1.cfg_account_pg_0.show();}
    pbc_csr.port_1.cfg_account_pg_1.reserved_min(100);
    pbc_csr.port_1.cfg_account_pg_1.headroom(0);
    pbc_csr.port_1.cfg_account_pg_1.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_1.cfg_account_pg_1.show();}
    pbc_csr.port_1.cfg_account_pg_2.reserved_min(100);
    pbc_csr.port_1.cfg_account_pg_2.headroom(0);
    pbc_csr.port_1.cfg_account_pg_2.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_1.cfg_account_pg_2.show();}
    pbc_csr.port_1.cfg_account_pg_3.reserved_min(100);
    pbc_csr.port_1.cfg_account_pg_3.headroom(0);
    pbc_csr.port_1.cfg_account_pg_3.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_1.cfg_account_pg_3.show();}
    pbc_csr.port_1.cfg_account_pg_4.reserved_min(100);
    pbc_csr.port_1.cfg_account_pg_4.headroom(0);
    pbc_csr.port_1.cfg_account_pg_4.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_1.cfg_account_pg_4.show();}
    pbc_csr.port_1.cfg_account_pg_5.reserved_min(100);
    pbc_csr.port_1.cfg_account_pg_5.headroom(0);
    pbc_csr.port_1.cfg_account_pg_5.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_1.cfg_account_pg_5.show();}
    pbc_csr.port_1.cfg_account_pg_6.reserved_min(100);
    pbc_csr.port_1.cfg_account_pg_6.headroom(0);
    pbc_csr.port_1.cfg_account_pg_6.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_1.cfg_account_pg_6.show();}
    pbc_csr.port_1.cfg_account_pg_7.reserved_min(100);
    pbc_csr.port_1.cfg_account_pg_7.headroom(0);
    pbc_csr.port_1.cfg_account_pg_7.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_1.cfg_account_pg_7.show();}
    pbc_csr.port_2.cfg_account_pg_0.reserved_min(100);
    pbc_csr.port_2.cfg_account_pg_0.headroom(0);
    pbc_csr.port_2.cfg_account_pg_0.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_2.cfg_account_pg_0.show();}
    pbc_csr.port_2.cfg_account_pg_1.reserved_min(100);
    pbc_csr.port_2.cfg_account_pg_1.headroom(0);
    pbc_csr.port_2.cfg_account_pg_1.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_2.cfg_account_pg_1.show();}
    pbc_csr.port_2.cfg_account_pg_2.reserved_min(100);
    pbc_csr.port_2.cfg_account_pg_2.headroom(0);
    pbc_csr.port_2.cfg_account_pg_2.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_2.cfg_account_pg_2.show();}
    pbc_csr.port_2.cfg_account_pg_3.reserved_min(100);
    pbc_csr.port_2.cfg_account_pg_3.headroom(0);
    pbc_csr.port_2.cfg_account_pg_3.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_2.cfg_account_pg_3.show();}
    pbc_csr.port_2.cfg_account_pg_4.reserved_min(100);
    pbc_csr.port_2.cfg_account_pg_4.headroom(0);
    pbc_csr.port_2.cfg_account_pg_4.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_2.cfg_account_pg_4.show();}
    pbc_csr.port_2.cfg_account_pg_5.reserved_min(100);
    pbc_csr.port_2.cfg_account_pg_5.headroom(0);
    pbc_csr.port_2.cfg_account_pg_5.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_2.cfg_account_pg_5.show();}
    pbc_csr.port_2.cfg_account_pg_6.reserved_min(100);
    pbc_csr.port_2.cfg_account_pg_6.headroom(0);
    pbc_csr.port_2.cfg_account_pg_6.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_2.cfg_account_pg_6.show();}
    pbc_csr.port_2.cfg_account_pg_7.reserved_min(100);
    pbc_csr.port_2.cfg_account_pg_7.headroom(0);
    pbc_csr.port_2.cfg_account_pg_7.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_2.cfg_account_pg_7.show();}
    pbc_csr.port_3.cfg_account_pg_0.reserved_min(100);
    pbc_csr.port_3.cfg_account_pg_0.headroom(0);
    pbc_csr.port_3.cfg_account_pg_0.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_3.cfg_account_pg_0.show();}
    pbc_csr.port_3.cfg_account_pg_1.reserved_min(100);
    pbc_csr.port_3.cfg_account_pg_1.headroom(0);
    pbc_csr.port_3.cfg_account_pg_1.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_3.cfg_account_pg_1.show();}
    pbc_csr.port_3.cfg_account_pg_2.reserved_min(100);
    pbc_csr.port_3.cfg_account_pg_2.headroom(0);
    pbc_csr.port_3.cfg_account_pg_2.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_3.cfg_account_pg_2.show();}
    pbc_csr.port_3.cfg_account_pg_3.reserved_min(100);
    pbc_csr.port_3.cfg_account_pg_3.headroom(0);
    pbc_csr.port_3.cfg_account_pg_3.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_3.cfg_account_pg_3.show();}
    pbc_csr.port_3.cfg_account_pg_4.reserved_min(100);
    pbc_csr.port_3.cfg_account_pg_4.headroom(0);
    pbc_csr.port_3.cfg_account_pg_4.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_3.cfg_account_pg_4.show();}
    pbc_csr.port_3.cfg_account_pg_5.reserved_min(100);
    pbc_csr.port_3.cfg_account_pg_5.headroom(0);
    pbc_csr.port_3.cfg_account_pg_5.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_3.cfg_account_pg_5.show();}
    pbc_csr.port_3.cfg_account_pg_6.reserved_min(100);
    pbc_csr.port_3.cfg_account_pg_6.headroom(0);
    pbc_csr.port_3.cfg_account_pg_6.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_3.cfg_account_pg_6.show();}
    pbc_csr.port_3.cfg_account_pg_7.reserved_min(100);
    pbc_csr.port_3.cfg_account_pg_7.headroom(0);
    pbc_csr.port_3.cfg_account_pg_7.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_3.cfg_account_pg_7.show();}
    pbc_csr.port_4.cfg_account_pg_0.reserved_min(100);
    pbc_csr.port_4.cfg_account_pg_0.headroom(0);
    pbc_csr.port_4.cfg_account_pg_0.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_4.cfg_account_pg_0.show();}
    pbc_csr.port_4.cfg_account_pg_1.reserved_min(100);
    pbc_csr.port_4.cfg_account_pg_1.headroom(0);
    pbc_csr.port_4.cfg_account_pg_1.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_4.cfg_account_pg_1.show();}
    pbc_csr.port_4.cfg_account_pg_2.reserved_min(100);
    pbc_csr.port_4.cfg_account_pg_2.headroom(0);
    pbc_csr.port_4.cfg_account_pg_2.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_4.cfg_account_pg_2.show();}
    pbc_csr.port_4.cfg_account_pg_3.reserved_min(100);
    pbc_csr.port_4.cfg_account_pg_3.headroom(0);
    pbc_csr.port_4.cfg_account_pg_3.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_4.cfg_account_pg_3.show();}
    pbc_csr.port_4.cfg_account_pg_4.reserved_min(100);
    pbc_csr.port_4.cfg_account_pg_4.headroom(0);
    pbc_csr.port_4.cfg_account_pg_4.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_4.cfg_account_pg_4.show();}
    pbc_csr.port_4.cfg_account_pg_5.reserved_min(100);
    pbc_csr.port_4.cfg_account_pg_5.headroom(0);
    pbc_csr.port_4.cfg_account_pg_5.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_4.cfg_account_pg_5.show();}
    pbc_csr.port_4.cfg_account_pg_6.reserved_min(100);
    pbc_csr.port_4.cfg_account_pg_6.headroom(0);
    pbc_csr.port_4.cfg_account_pg_6.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_4.cfg_account_pg_6.show();}
    pbc_csr.port_4.cfg_account_pg_7.reserved_min(100);
    pbc_csr.port_4.cfg_account_pg_7.headroom(0);
    pbc_csr.port_4.cfg_account_pg_7.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_4.cfg_account_pg_7.show();}
    pbc_csr.port_5.cfg_account_pg_0.reserved_min(100);
    pbc_csr.port_5.cfg_account_pg_0.headroom(0);
    pbc_csr.port_5.cfg_account_pg_0.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_5.cfg_account_pg_0.show();}
    pbc_csr.port_5.cfg_account_pg_1.reserved_min(100);
    pbc_csr.port_5.cfg_account_pg_1.headroom(0);
    pbc_csr.port_5.cfg_account_pg_1.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_5.cfg_account_pg_1.show();}
    pbc_csr.port_5.cfg_account_pg_2.reserved_min(100);
    pbc_csr.port_5.cfg_account_pg_2.headroom(0);
    pbc_csr.port_5.cfg_account_pg_2.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_5.cfg_account_pg_2.show();}
    pbc_csr.port_5.cfg_account_pg_3.reserved_min(100);
    pbc_csr.port_5.cfg_account_pg_3.headroom(0);
    pbc_csr.port_5.cfg_account_pg_3.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_5.cfg_account_pg_3.show();}
    pbc_csr.port_5.cfg_account_pg_4.reserved_min(100);
    pbc_csr.port_5.cfg_account_pg_4.headroom(0);
    pbc_csr.port_5.cfg_account_pg_4.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_5.cfg_account_pg_4.show();}
    pbc_csr.port_5.cfg_account_pg_5.reserved_min(100);
    pbc_csr.port_5.cfg_account_pg_5.headroom(0);
    pbc_csr.port_5.cfg_account_pg_5.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_5.cfg_account_pg_5.show();}
    pbc_csr.port_5.cfg_account_pg_6.reserved_min(100);
    pbc_csr.port_5.cfg_account_pg_6.headroom(0);
    pbc_csr.port_5.cfg_account_pg_6.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_5.cfg_account_pg_6.show();}
    pbc_csr.port_5.cfg_account_pg_7.reserved_min(100);
    pbc_csr.port_5.cfg_account_pg_7.headroom(0);
    pbc_csr.port_5.cfg_account_pg_7.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_5.cfg_account_pg_7.show();}
    pbc_csr.port_6.cfg_account_pg_0.reserved_min(100);
    pbc_csr.port_6.cfg_account_pg_0.headroom(0);
    pbc_csr.port_6.cfg_account_pg_0.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_6.cfg_account_pg_0.show();}
    pbc_csr.port_6.cfg_account_pg_1.reserved_min(100);
    pbc_csr.port_6.cfg_account_pg_1.headroom(0);
    pbc_csr.port_6.cfg_account_pg_1.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_6.cfg_account_pg_1.show();}
    pbc_csr.port_6.cfg_account_pg_2.reserved_min(100);
    pbc_csr.port_6.cfg_account_pg_2.headroom(0);
    pbc_csr.port_6.cfg_account_pg_2.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_6.cfg_account_pg_2.show();}
    pbc_csr.port_6.cfg_account_pg_3.reserved_min(100);
    pbc_csr.port_6.cfg_account_pg_3.headroom(0);
    pbc_csr.port_6.cfg_account_pg_3.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_6.cfg_account_pg_3.show();}
    pbc_csr.port_6.cfg_account_pg_4.reserved_min(100);
    pbc_csr.port_6.cfg_account_pg_4.headroom(0);
    pbc_csr.port_6.cfg_account_pg_4.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_6.cfg_account_pg_4.show();}
    pbc_csr.port_6.cfg_account_pg_5.reserved_min(100);
    pbc_csr.port_6.cfg_account_pg_5.headroom(0);
    pbc_csr.port_6.cfg_account_pg_5.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_6.cfg_account_pg_5.show();}
    pbc_csr.port_6.cfg_account_pg_6.reserved_min(100);
    pbc_csr.port_6.cfg_account_pg_6.headroom(0);
    pbc_csr.port_6.cfg_account_pg_6.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_6.cfg_account_pg_6.show();}
    pbc_csr.port_6.cfg_account_pg_7.reserved_min(100);
    pbc_csr.port_6.cfg_account_pg_7.headroom(0);
    pbc_csr.port_6.cfg_account_pg_7.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_6.cfg_account_pg_7.show();}
    pbc_csr.port_7.cfg_account_pg_0.reserved_min(100);
    pbc_csr.port_7.cfg_account_pg_0.headroom(0);
    pbc_csr.port_7.cfg_account_pg_0.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_7.cfg_account_pg_0.show();}
    pbc_csr.port_7.cfg_account_pg_1.reserved_min(100);
    pbc_csr.port_7.cfg_account_pg_1.headroom(0);
    pbc_csr.port_7.cfg_account_pg_1.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_7.cfg_account_pg_1.show();}
    pbc_csr.port_7.cfg_account_pg_2.reserved_min(100);
    pbc_csr.port_7.cfg_account_pg_2.headroom(0);
    pbc_csr.port_7.cfg_account_pg_2.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_7.cfg_account_pg_2.show();}
    pbc_csr.port_7.cfg_account_pg_3.reserved_min(100);
    pbc_csr.port_7.cfg_account_pg_3.headroom(0);
    pbc_csr.port_7.cfg_account_pg_3.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_7.cfg_account_pg_3.show();}
    pbc_csr.port_7.cfg_account_pg_4.reserved_min(100);
    pbc_csr.port_7.cfg_account_pg_4.headroom(0);
    pbc_csr.port_7.cfg_account_pg_4.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_7.cfg_account_pg_4.show();}
    pbc_csr.port_7.cfg_account_pg_5.reserved_min(100);
    pbc_csr.port_7.cfg_account_pg_5.headroom(0);
    pbc_csr.port_7.cfg_account_pg_5.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_7.cfg_account_pg_5.show();}
    pbc_csr.port_7.cfg_account_pg_6.reserved_min(100);
    pbc_csr.port_7.cfg_account_pg_6.headroom(0);
    pbc_csr.port_7.cfg_account_pg_6.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_7.cfg_account_pg_6.show();}
    pbc_csr.port_7.cfg_account_pg_7.reserved_min(100);
    pbc_csr.port_7.cfg_account_pg_7.headroom(0);
    pbc_csr.port_7.cfg_account_pg_7.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_7.cfg_account_pg_7.show();}
    pbc_csr.port_8.cfg_account_pg_0.reserved_min(100);
    pbc_csr.port_8.cfg_account_pg_0.headroom(0);
    pbc_csr.port_8.cfg_account_pg_0.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_8.cfg_account_pg_0.show();}
    pbc_csr.port_8.cfg_account_pg_1.reserved_min(100);
    pbc_csr.port_8.cfg_account_pg_1.headroom(0);
    pbc_csr.port_8.cfg_account_pg_1.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_8.cfg_account_pg_1.show();}
    pbc_csr.port_8.cfg_account_pg_2.reserved_min(100);
    pbc_csr.port_8.cfg_account_pg_2.headroom(0);
    pbc_csr.port_8.cfg_account_pg_2.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_8.cfg_account_pg_2.show();}
    pbc_csr.port_8.cfg_account_pg_3.reserved_min(100);
    pbc_csr.port_8.cfg_account_pg_3.headroom(0);
    pbc_csr.port_8.cfg_account_pg_3.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_8.cfg_account_pg_3.show();}
    pbc_csr.port_8.cfg_account_pg_4.reserved_min(100);
    pbc_csr.port_8.cfg_account_pg_4.headroom(0);
    pbc_csr.port_8.cfg_account_pg_4.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_8.cfg_account_pg_4.show();}
    pbc_csr.port_8.cfg_account_pg_5.reserved_min(100);
    pbc_csr.port_8.cfg_account_pg_5.headroom(0);
    pbc_csr.port_8.cfg_account_pg_5.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_8.cfg_account_pg_5.show();}
    pbc_csr.port_8.cfg_account_pg_6.reserved_min(100);
    pbc_csr.port_8.cfg_account_pg_6.headroom(0);
    pbc_csr.port_8.cfg_account_pg_6.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_8.cfg_account_pg_6.show();}
    pbc_csr.port_8.cfg_account_pg_7.reserved_min(100);
    pbc_csr.port_8.cfg_account_pg_7.headroom(0);
    pbc_csr.port_8.cfg_account_pg_7.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_8.cfg_account_pg_7.show();}
    pbc_csr.port_9.cfg_account_pg_0.reserved_min(100);
    pbc_csr.port_9.cfg_account_pg_0.headroom(0);
    pbc_csr.port_9.cfg_account_pg_0.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_0.show();}
    pbc_csr.port_9.cfg_account_pg_1.reserved_min(100);
    pbc_csr.port_9.cfg_account_pg_1.headroom(0);
    pbc_csr.port_9.cfg_account_pg_1.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_1.show();}
    pbc_csr.port_9.cfg_account_pg_2.reserved_min(100);
    pbc_csr.port_9.cfg_account_pg_2.headroom(0);
    pbc_csr.port_9.cfg_account_pg_2.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_2.show();}
    pbc_csr.port_9.cfg_account_pg_3.reserved_min(100);
    pbc_csr.port_9.cfg_account_pg_3.headroom(0);
    pbc_csr.port_9.cfg_account_pg_3.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_3.show();}
    pbc_csr.port_9.cfg_account_pg_4.reserved_min(100);
    pbc_csr.port_9.cfg_account_pg_4.headroom(0);
    pbc_csr.port_9.cfg_account_pg_4.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_4.show();}
    pbc_csr.port_9.cfg_account_pg_5.reserved_min(100);
    pbc_csr.port_9.cfg_account_pg_5.headroom(0);
    pbc_csr.port_9.cfg_account_pg_5.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_5.show();}
    pbc_csr.port_9.cfg_account_pg_6.reserved_min(100);
    pbc_csr.port_9.cfg_account_pg_6.headroom(0);
    pbc_csr.port_9.cfg_account_pg_6.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_6.show();}
    pbc_csr.port_9.cfg_account_pg_7.reserved_min(100);
    pbc_csr.port_9.cfg_account_pg_7.headroom(0);
    pbc_csr.port_9.cfg_account_pg_7.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_7.show();}
    pbc_csr.port_9.cfg_account_pg_8.reserved_min(100);
    pbc_csr.port_9.cfg_account_pg_8.headroom(0);
    pbc_csr.port_9.cfg_account_pg_8.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_8.show();}
    pbc_csr.port_9.cfg_account_pg_9.reserved_min(100);
    pbc_csr.port_9.cfg_account_pg_9.headroom(0);
    pbc_csr.port_9.cfg_account_pg_9.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_9.show();}
    pbc_csr.port_9.cfg_account_pg_10.reserved_min(100);
    pbc_csr.port_9.cfg_account_pg_10.headroom(0);
    pbc_csr.port_9.cfg_account_pg_10.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_10.show();}
    pbc_csr.port_9.cfg_account_pg_11.reserved_min(100);
    pbc_csr.port_9.cfg_account_pg_11.headroom(0);
    pbc_csr.port_9.cfg_account_pg_11.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_11.show();}
    pbc_csr.port_9.cfg_account_pg_12.reserved_min(100);
    pbc_csr.port_9.cfg_account_pg_12.headroom(0);
    pbc_csr.port_9.cfg_account_pg_12.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_12.show();}
    pbc_csr.port_9.cfg_account_pg_13.reserved_min(100);
    pbc_csr.port_9.cfg_account_pg_13.headroom(0);
    pbc_csr.port_9.cfg_account_pg_13.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_13.show();}
    pbc_csr.port_9.cfg_account_pg_14.reserved_min(100);
    pbc_csr.port_9.cfg_account_pg_14.headroom(0);
    pbc_csr.port_9.cfg_account_pg_14.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_14.show();}
    pbc_csr.port_9.cfg_account_pg_15.reserved_min(100);
    pbc_csr.port_9.cfg_account_pg_15.headroom(0);
    pbc_csr.port_9.cfg_account_pg_15.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_account_pg_15.show();}
    pbc_csr.port_10.cfg_account_pg_0.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_0.headroom(0);
    pbc_csr.port_10.cfg_account_pg_0.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_0.show();}
    pbc_csr.port_10.cfg_account_pg_1.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_1.headroom(0);
    pbc_csr.port_10.cfg_account_pg_1.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_1.show();}
    pbc_csr.port_10.cfg_account_pg_2.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_2.headroom(0);
    pbc_csr.port_10.cfg_account_pg_2.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_2.show();}
    pbc_csr.port_10.cfg_account_pg_3.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_3.headroom(0);
    pbc_csr.port_10.cfg_account_pg_3.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_3.show();}
    pbc_csr.port_10.cfg_account_pg_4.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_4.headroom(0);
    pbc_csr.port_10.cfg_account_pg_4.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_4.show();}
    pbc_csr.port_10.cfg_account_pg_5.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_5.headroom(0);
    pbc_csr.port_10.cfg_account_pg_5.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_5.show();}
    pbc_csr.port_10.cfg_account_pg_6.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_6.headroom(0);
    pbc_csr.port_10.cfg_account_pg_6.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_6.show();}
    pbc_csr.port_10.cfg_account_pg_7.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_7.headroom(0);
    pbc_csr.port_10.cfg_account_pg_7.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_7.show();}
    pbc_csr.port_10.cfg_account_pg_8.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_8.headroom(0);
    pbc_csr.port_10.cfg_account_pg_8.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_8.show();}
    pbc_csr.port_10.cfg_account_pg_9.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_9.headroom(0);
    pbc_csr.port_10.cfg_account_pg_9.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_9.show();}
    pbc_csr.port_10.cfg_account_pg_10.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_10.headroom(0);
    pbc_csr.port_10.cfg_account_pg_10.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_10.show();}
    pbc_csr.port_10.cfg_account_pg_11.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_11.headroom(0);
    pbc_csr.port_10.cfg_account_pg_11.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_11.show();}
    pbc_csr.port_10.cfg_account_pg_12.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_12.headroom(0);
    pbc_csr.port_10.cfg_account_pg_12.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_12.show();}
    pbc_csr.port_10.cfg_account_pg_13.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_13.headroom(0);
    pbc_csr.port_10.cfg_account_pg_13.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_13.show();}
    pbc_csr.port_10.cfg_account_pg_14.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_14.headroom(0);
    pbc_csr.port_10.cfg_account_pg_14.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_14.show();}
    pbc_csr.port_10.cfg_account_pg_15.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_15.headroom(0);
    pbc_csr.port_10.cfg_account_pg_15.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_15.show();}
    pbc_csr.port_10.cfg_account_pg_16.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_16.headroom(0);
    pbc_csr.port_10.cfg_account_pg_16.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_16.show();}
    pbc_csr.port_10.cfg_account_pg_17.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_17.headroom(0);
    pbc_csr.port_10.cfg_account_pg_17.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_17.show();}
    pbc_csr.port_10.cfg_account_pg_18.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_18.headroom(0);
    pbc_csr.port_10.cfg_account_pg_18.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_18.show();}
    pbc_csr.port_10.cfg_account_pg_19.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_19.headroom(0);
    pbc_csr.port_10.cfg_account_pg_19.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_19.show();}
    pbc_csr.port_10.cfg_account_pg_20.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_20.headroom(0);
    pbc_csr.port_10.cfg_account_pg_20.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_20.show();}
    pbc_csr.port_10.cfg_account_pg_21.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_21.headroom(0);
    pbc_csr.port_10.cfg_account_pg_21.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_21.show();}
    pbc_csr.port_10.cfg_account_pg_22.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_22.headroom(0);
    pbc_csr.port_10.cfg_account_pg_22.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_22.show();}
    pbc_csr.port_10.cfg_account_pg_23.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_23.headroom(0);
    pbc_csr.port_10.cfg_account_pg_23.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_23.show();}
    pbc_csr.port_10.cfg_account_pg_24.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_24.headroom(0);
    pbc_csr.port_10.cfg_account_pg_24.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_24.show();}
    pbc_csr.port_10.cfg_account_pg_25.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_25.headroom(0);
    pbc_csr.port_10.cfg_account_pg_25.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_25.show();}
    pbc_csr.port_10.cfg_account_pg_26.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_26.headroom(0);
    pbc_csr.port_10.cfg_account_pg_26.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_26.show();}
    pbc_csr.port_10.cfg_account_pg_27.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_27.headroom(0);
    pbc_csr.port_10.cfg_account_pg_27.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_27.show();}
    pbc_csr.port_10.cfg_account_pg_28.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_28.headroom(0);
    pbc_csr.port_10.cfg_account_pg_28.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_28.show();}
    pbc_csr.port_10.cfg_account_pg_29.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_29.headroom(0);
    pbc_csr.port_10.cfg_account_pg_29.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_29.show();}
    pbc_csr.port_10.cfg_account_pg_30.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_30.headroom(0);
    pbc_csr.port_10.cfg_account_pg_30.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_30.show();}
    pbc_csr.port_10.cfg_account_pg_31.reserved_min(100);
    pbc_csr.port_10.cfg_account_pg_31.headroom(0);
    pbc_csr.port_10.cfg_account_pg_31.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_account_pg_31.show();}
    pbc_csr.port_11.cfg_account_pg_0.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_0.headroom(0);
    pbc_csr.port_11.cfg_account_pg_0.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_0.show();}
    pbc_csr.port_11.cfg_account_pg_1.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_1.headroom(0);
    pbc_csr.port_11.cfg_account_pg_1.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_1.show();}
    pbc_csr.port_11.cfg_account_pg_2.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_2.headroom(0);
    pbc_csr.port_11.cfg_account_pg_2.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_2.show();}
    pbc_csr.port_11.cfg_account_pg_3.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_3.headroom(0);
    pbc_csr.port_11.cfg_account_pg_3.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_3.show();}
    pbc_csr.port_11.cfg_account_pg_4.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_4.headroom(0);
    pbc_csr.port_11.cfg_account_pg_4.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_4.show();}
    pbc_csr.port_11.cfg_account_pg_5.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_5.headroom(0);
    pbc_csr.port_11.cfg_account_pg_5.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_5.show();}
    pbc_csr.port_11.cfg_account_pg_6.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_6.headroom(0);
    pbc_csr.port_11.cfg_account_pg_6.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_6.show();}
    pbc_csr.port_11.cfg_account_pg_7.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_7.headroom(0);
    pbc_csr.port_11.cfg_account_pg_7.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_7.show();}
    pbc_csr.port_11.cfg_account_pg_8.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_8.headroom(0);
    pbc_csr.port_11.cfg_account_pg_8.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_8.show();}
    pbc_csr.port_11.cfg_account_pg_9.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_9.headroom(0);
    pbc_csr.port_11.cfg_account_pg_9.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_9.show();}
    pbc_csr.port_11.cfg_account_pg_10.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_10.headroom(0);
    pbc_csr.port_11.cfg_account_pg_10.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_10.show();}
    pbc_csr.port_11.cfg_account_pg_11.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_11.headroom(0);
    pbc_csr.port_11.cfg_account_pg_11.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_11.show();}
    pbc_csr.port_11.cfg_account_pg_12.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_12.headroom(0);
    pbc_csr.port_11.cfg_account_pg_12.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_12.show();}
    pbc_csr.port_11.cfg_account_pg_13.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_13.headroom(0);
    pbc_csr.port_11.cfg_account_pg_13.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_13.show();}
    pbc_csr.port_11.cfg_account_pg_14.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_14.headroom(0);
    pbc_csr.port_11.cfg_account_pg_14.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_14.show();}
    pbc_csr.port_11.cfg_account_pg_15.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_15.headroom(0);
    pbc_csr.port_11.cfg_account_pg_15.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_15.show();}
    pbc_csr.port_11.cfg_account_pg_16.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_16.headroom(0);
    pbc_csr.port_11.cfg_account_pg_16.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_16.show();}
    pbc_csr.port_11.cfg_account_pg_17.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_17.headroom(0);
    pbc_csr.port_11.cfg_account_pg_17.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_17.show();}
    pbc_csr.port_11.cfg_account_pg_18.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_18.headroom(0);
    pbc_csr.port_11.cfg_account_pg_18.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_18.show();}
    pbc_csr.port_11.cfg_account_pg_19.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_19.headroom(0);
    pbc_csr.port_11.cfg_account_pg_19.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_19.show();}
    pbc_csr.port_11.cfg_account_pg_20.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_20.headroom(0);
    pbc_csr.port_11.cfg_account_pg_20.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_20.show();}
    pbc_csr.port_11.cfg_account_pg_21.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_21.headroom(0);
    pbc_csr.port_11.cfg_account_pg_21.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_21.show();}
    pbc_csr.port_11.cfg_account_pg_22.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_22.headroom(0);
    pbc_csr.port_11.cfg_account_pg_22.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_22.show();}
    pbc_csr.port_11.cfg_account_pg_23.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_23.headroom(0);
    pbc_csr.port_11.cfg_account_pg_23.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_23.show();}
    pbc_csr.port_11.cfg_account_pg_24.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_24.headroom(0);
    pbc_csr.port_11.cfg_account_pg_24.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_24.show();}
    pbc_csr.port_11.cfg_account_pg_25.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_25.headroom(0);
    pbc_csr.port_11.cfg_account_pg_25.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_25.show();}
    pbc_csr.port_11.cfg_account_pg_26.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_26.headroom(0);
    pbc_csr.port_11.cfg_account_pg_26.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_26.show();}
    pbc_csr.port_11.cfg_account_pg_27.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_27.headroom(0);
    pbc_csr.port_11.cfg_account_pg_27.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_27.show();}
    pbc_csr.port_11.cfg_account_pg_28.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_28.headroom(0);
    pbc_csr.port_11.cfg_account_pg_28.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_28.show();}
    pbc_csr.port_11.cfg_account_pg_29.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_29.headroom(0);
    pbc_csr.port_11.cfg_account_pg_29.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_29.show();}
    pbc_csr.port_11.cfg_account_pg_30.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_30.headroom(0);
    pbc_csr.port_11.cfg_account_pg_30.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_30.show();}
    pbc_csr.port_11.cfg_account_pg_31.reserved_min(100);
    pbc_csr.port_11.cfg_account_pg_31.headroom(0);
    pbc_csr.port_11.cfg_account_pg_31.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_account_pg_31.show();}
  // read to flush writes
  pbc_csr.cfg_island_control.read();
}

void cap_pb_enable_write_ports(int chip_id, int inst_id) {
   cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
  // enable write ports back
   pbc_csr.port_0.cfg_write_control.enable(1);
   pbc_csr.port_0.cfg_write_control.write();
   pbc_csr.port_1.cfg_write_control.enable(1);
   pbc_csr.port_1.cfg_write_control.write();
   pbc_csr.port_2.cfg_write_control.enable(1);
   pbc_csr.port_2.cfg_write_control.write();
   pbc_csr.port_3.cfg_write_control.enable(1);
   pbc_csr.port_3.cfg_write_control.write();
   pbc_csr.port_4.cfg_write_control.enable(1);
   pbc_csr.port_4.cfg_write_control.write();
   pbc_csr.port_5.cfg_write_control.enable(1);
   pbc_csr.port_5.cfg_write_control.write();
   pbc_csr.port_6.cfg_write_control.enable(1);
   pbc_csr.port_6.cfg_write_control.write();
   pbc_csr.port_7.cfg_write_control.enable(1);
   pbc_csr.port_7.cfg_write_control.write();
   pbc_csr.port_8.cfg_write_control.enable(1);
   pbc_csr.port_8.cfg_write_control.write();
   pbc_csr.port_9.cfg_write_control.enable(1);
   pbc_csr.port_9.cfg_write_control.write();
   pbc_csr.port_10.cfg_write_control.enable(1);
   pbc_csr.port_10.cfg_write_control.write();
   pbc_csr.port_11.cfg_write_control.enable(1);
   pbc_csr.port_11.cfg_write_control.write();
}

void cap_pb_eos(int chip_id, int inst_id) {
   const uint32_t pb_skip_eos_fc_check = SKNOBS_GET("tb/pb_skip_eos_fc_check", 1);
   const uint32_t pb_skip_eos_dhs_hbm_check = SKNOBS_GET("tb/pb_skip_eos_dhs_hbm_check", 1);
   const uint32_t pb_skip_eos_bist = SKNOBS_GET("tb/pb_skip_eos_bist", 1);

   cap_pb_eos_cnt(chip_id,inst_id);
   cap_pb_eos_int(chip_id,inst_id);
   cap_pb_eos_sta(chip_id,inst_id);

   if (!pb_skip_eos_fc_check) {
	   cap_pb_eos_fc_check(chip_id, inst_id);
   }

   if (!pb_skip_eos_dhs_hbm_check ) {
	   cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);

	   const int nodes[] = {0,1,2,3,6,9,10,11,12,13,14,15};
	   int const * iptr = &nodes[0];

	   do { const uint32_t write_val = 0xdeadbee0 + *iptr;
		pbc_csr.hbm.cfg_hbm.read();
		pbc_csr.hbm.cfg_hbm.dhs_mem_address(0);
		pbc_csr.hbm.cfg_hbm.mtu_dhs_selection(*iptr);
		pbc_csr.hbm.cfg_hbm.write();

		pbc_csr.hbm.dhs_hbm_mem.entry.rsvd(write_val);
		pbc_csr.hbm.dhs_hbm_mem.entry.write();

		pbc_csr.hbm.dhs_hbm_mem.entry.read();
		cpp_int rval = pbc_csr.hbm.dhs_hbm_mem.entry.all();

		if (rval.convert_to<uint32_t>() != write_val) {
			PLOG_ERR("expected " << hex << write_val << " got " << rval.convert_to<uint32_t>() << endl);
		}

	   } while (*iptr++ != 15);
   }
   if (!pb_skip_eos_bist) {
	cap_pb_bist(0,0);
   }
}

void cap_pb_eos_cnt(int chip_id, int inst_id) {
 unsigned int pb_skip_eos_cnt_chk = SKNOBS_GET("tb/pb_skip_eos_cnt_chk", 0);
 unsigned int pb_skip_eos_cnt_hbm_ctx_chk = SKNOBS_GET("tb/pb_skip_eos_cnt_hbm_ctx_chk", 0);

 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);

 uint64_t sop_in_total = 0;
 uint64_t eop_in_total = 0;
 uint64_t sop_out_total = 0;
 uint64_t eop_out_total = 0;
 uint64_t drop_total = 0;
 uint64_t good_count_total = 0;
 uint64_t flush_total = 0;
 int cnt_saturated = 0;
 int error = 0;
 cpp_int rval;
 cpp_int good_count;
 cpp_int error_count;

 for (int p=0; p<12; p=p+1) {
   cap_pb_read_flits_cnt(chip_id, inst_id, 0, 0, p, rval); 
   sop_in_total += rval.convert_to<uint64_t>();
   cap_pb_read_flits_cnt(chip_id, inst_id, 0, 1, p, rval); 
   eop_in_total += rval.convert_to<uint64_t>();
   cap_pb_read_flits_cnt(chip_id, inst_id, 0, 2, p, rval); 
   sop_out_total += rval.convert_to<uint64_t>();
   cap_pb_read_flits_cnt(chip_id, inst_id, 0, 3, p, rval); 
   eop_out_total += rval.convert_to<uint64_t>();
   pbc_csr.sat_write_error[p].read();
   rval = pbc_csr.sat_write_error[p].discarded();
   if (rval.convert_to<int>() == 0xffff) {
     cnt_saturated = 1;
   }
   drop_total += rval.convert_to<uint64_t>();
   rval = pbc_csr.sat_write_error[p].intrinsic_drop();
   if (rval.convert_to<int>() == 0xffff) {
     cnt_saturated = 1;
   }
   drop_total += rval.convert_to<uint64_t>();
   rval = pbc_csr.sat_write_error[p].out_of_credit();
   if (rval.convert_to<int>() == 0xff) {
     cnt_saturated = 1;
   }
   drop_total += rval.convert_to<uint64_t>();
   rval = pbc_csr.sat_write_error[p].port_disabled();
   if (rval.convert_to<int>() == 0xff) {
     cnt_saturated = 1;
   }
   drop_total += rval.convert_to<uint64_t>();
   rval = pbc_csr.sat_write_error[p].out_of_cells1();
   if (rval.convert_to<int>() == 0xffff) {
     cnt_saturated = 1;
   }
   drop_total += rval.convert_to<uint64_t>();
   rval = pbc_csr.sat_write_error[p].tail_drop_cpu();
   if (rval.convert_to<int>() == 0xffff) {
     cnt_saturated = 1;
   }
   drop_total += rval.convert_to<uint64_t>();
   rval = pbc_csr.sat_write_error[p].tail_drop_span();
   if (rval.convert_to<int>() == 0xffff) {
     cnt_saturated = 1;
   }
   drop_total += rval.convert_to<uint64_t>();
   rval = pbc_csr.sat_write_error[p].min_size_viol();
   if (rval.convert_to<int>() == 0xff) {
     cnt_saturated = 1;
   }
   drop_total += rval.convert_to<uint64_t>();
   rval = pbc_csr.sat_write_error[p].enqueue();
   if (rval.convert_to<int>() == 0xff) {
     cnt_saturated = 1;
   }
   drop_total += rval.convert_to<uint64_t>();
   rval = pbc_csr.sat_write_error[p].port_range();
   if (rval.convert_to<int>() == 0xff) {
     cnt_saturated = 1;
   }
   drop_total += rval.convert_to<uint64_t>();
   rval = pbc_csr.sat_write_error[p].oq_range();
   if (rval.convert_to<int>() == 0xff) {
     cnt_saturated = 1;
   }
   drop_total += rval.convert_to<uint64_t>();

   pbc_csr.cnt_flush[p].read();
   rval = pbc_csr.cnt_flush[p].packets();
   flush_total += rval.convert_to<uint64_t>();
 }
 drop_total += flush_total;

 if ((cnt_saturated == 0) && (pb_skip_eos_cnt_chk == 0)) {
    // use 16 bit mask since each of the non-saturating counters are 16 bit
   if ((sop_in_total & 0xFFFF) != ((sop_out_total + drop_total)& 0xFFFF)) {
     PLOG_ERR("[eos_cnt]: in_cnt=" << hex << sop_in_total << ", out_cnt_p_drop_cnt=" << (sop_out_total + drop_total) << ", out_cnt=" << sop_out_total << ", drop_cnt=" << drop_total << ", flush_total=" << flush_total << dec << endl);
     error = 1;
   }
 } else {
   PLOG_MSG("[eos_cnt]: skip port count check due to drop count saturation or pb_skip_eos_cnt_chk knob is set\n");
 }
 if (sop_in_total != eop_in_total) {
   PLOG_ERR("[eos_cnt]: in_sop_cnt=" << hex << sop_in_total << ", in_eop_cnt=" << eop_in_total << dec << endl);
   error = 1;
 }
 if (sop_out_total != eop_out_total) {
   PLOG_ERR("[eos_cnt]: out_sop_cnt=" << hex << sop_out_total << ", out_eop_cnt=" << eop_out_total << dec << endl);
   error = 1;
 }
 if (error == 0) {
   PLOG_MSG("[eos_cnt]: port count check passed \n");
 }

 uint32_t drop_total_across_ports=0;

 for (int p=0; p<9; p=p+1) {
   sop_in_total = 0;
   eop_in_total = 0;
   sop_out_total = 0;
   eop_out_total = 0;
   drop_total = 0;
   error = 0;

   cap_pb_read_flits_cnt(chip_id, inst_id, 1, 0, p, rval); 
   sop_in_total = rval.convert_to<uint64_t>();

   cap_pb_read_flits_cnt(chip_id, inst_id, 1, 1, p, rval); 
   eop_in_total = rval.convert_to<uint64_t>();

   cap_pb_read_flits_cnt(chip_id, inst_id, 1, 2, p, rval); 
   sop_out_total = rval.convert_to<uint64_t>();

   cap_pb_read_flits_cnt(chip_id, inst_id, 1, 3, p, rval); 
   eop_out_total = rval.convert_to<uint64_t>();

   cap_pb_read_flits_cnt(chip_id, inst_id, 1, 4, p, rval); 
   uint64_t occup_drops = rval.convert_to<uint64_t>();
   drop_total = occup_drops;

   cap_pb_read_emergency_stop_drop_count(chip_id, inst_id, p, rval); 
   uint64_t emergency_stop = rval.convert_to<uint64_t>();
   drop_total += emergency_stop;

   cap_pb_read_write_ack_filling_up_drop_count(chip_id, inst_id, p, rval); 
   uint64_t write_ack_filling_up = rval.convert_to<uint64_t>();
   drop_total += write_ack_filling_up;

   cap_pb_read_write_ack_full_drop_count(chip_id, inst_id, p, rval); 
   uint64_t write_ack_full = rval.convert_to<uint64_t>();
   drop_total += write_ack_full;

   cap_pb_read_truncate_drop_count(chip_id, inst_id, p, rval); 
   uint64_t truncate = rval.convert_to<uint64_t>();
   drop_total += truncate;

   cap_pb_read_ctrl_full_drop_count(chip_id, inst_id, p, rval); 
   uint64_t ctrl_full = rval.convert_to<uint64_t>();
   drop_total += ctrl_full;

   PLOG_MSG (" port " << p << " drops = " << drop_total << endl);
   drop_total_across_ports += drop_total;

   if (pb_skip_eos_cnt_chk == 0) {
	 if (sop_in_total != (sop_out_total + drop_total)) {
	   PLOG_ERR("[eos_cnt]: port: " << p << " hbm_in_cnt=" << hex << sop_in_total << ", hbm_out_cnt_p_drop_cnt=" << (sop_out_total + drop_total) << ", hbm_out_cnt=" << sop_out_total << ", hbm_drop_cnt=" << drop_total << dec << endl);
	   PLOG_ERR("[eos_cnt]: port: " << p << " emergency_stop =" << dec << emergency_stop << endl);
	   PLOG_ERR("[eos_cnt]: port: " << p << " write_ack_filling_up =" << dec << write_ack_filling_up << endl);
	   PLOG_ERR("[eos_cnt]: port: " << p << " write_ack_full =" << dec << write_ack_full << endl);
	   PLOG_ERR("[eos_cnt]: port: " << p << " truncate =" << dec << truncate << endl);
	   PLOG_ERR("[eos_cnt]: port: " << p << " ctrl_full =" << dec << ctrl_full << endl);
	   error = 1;
	 }
	 if (sop_in_total != eop_in_total) {
	   PLOG_ERR("[eos_cnt]: hbm in_sop_cnt=" << hex << sop_in_total << ", hbm in_eop_cnt=" << eop_in_total << dec << endl);
	   error = 1;
	 }
	 if (sop_out_total != eop_out_total) {
	   PLOG_ERR("[eos_cnt]: hbm out_sop_cnt=" << hex << sop_out_total << ", hbm out_eop_cnt=" << eop_out_total << dec << endl);
	   error = 1;
	 }
	 if (error == 0) {
	   PLOG_MSG("[eos_cnt]: hbm port count check passed \n");
	 }
    }
  }
 if (pb_skip_eos_cnt_chk == 0 && pb_skip_eos_cnt_hbm_ctx_chk==0) {
 // port 12: eth_head, 13: eth_tail, 14: tx_head, 15: tx_tail
 // check input packet counts
 uint64_t error_count_total = 0;
 for (int p=13; p<14; p=p+2) {
   good_count = 0;
   error_count = 0;
   for (int q=0; q<32; q=q+1) {
     cap_pb_read_hbm_ctx_stat(chip_id, inst_id, p, q, good_count, error_count);
     //PLOG_MSG("[eos_cnt]: port " << p << ", queue " << q << ", good_cnt " << good_count << ", error_cnt " << error_count << "\n");
     good_count_total += good_count.convert_to<uint64_t>();
     error_count_total += error_count.convert_to<uint64_t>();
   }
 }
   
   if (drop_total_across_ports != error_count_total) {
     PLOG_ERR("[eos_cnt]: hbm ctx stats error_count_total=" << hex << error_count_total << ", hbm drop_total_cnt=" << drop_total_across_ports << dec << endl);
   } else {
     PLOG_MSG("[eos_cnt]: hbm ctx stats error_count check passed \n");
   }
 }
 // port 12: eth_head, 13: eth_tail, 14: tx_head, 15: tx_tail
 // check per ctxt packet counts
 if (pb_skip_eos_cnt_hbm_ctx_chk == 0) {
    cpp_int e_good_count;
    error = 0;
    for (int p=12; p<15; p=p+2) {
      for (int q=0; q<32; q=q+1) {
        cap_pb_read_hbm_ctx_stat(chip_id, inst_id, p, q, good_count, error_count);
        cap_pb_read_hbm_ctx_stat(chip_id, inst_id, p+1, q, e_good_count, error_count);
        if (good_count != e_good_count) {
          if (p == 12) {
            PLOG_ERR("[eos_cnt]: hbm eth ctx " << q << " stats output good_count=" << hex << good_count << ", hbm input good_count=" << e_good_count << dec << endl);
       error = 1;
          } else {
            PLOG_ERR("[eos_cnt]: hbm tdma ctx " << q << " stats output good_count=" << hex << good_count << ", hbm input good_count=" << e_good_count << dec << endl);
       error = 1;
          }
        }
      }
    }
    if (error == 0) {
      PLOG_MSG("[eos_cnt]: hbm ctxt count check passed \n");
    }
 }
}

void cap_pb_eos_int(int chip_id, int inst_id) {
   // PLOG_MSG("inside cap_pb_eos_int \n");

   // cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
   // cap_pbm_csr_t & pbm_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbm_csr_t, chip_id, inst_id);
   // check_interrupts(&pbc_csr,0);
   // check_interrupts(&pbm_csr,0);
   // pbc_csr.int_write_11.read();
   // if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.int_write_11.show();}
   // PLOG_MSG("int csr name is "<<pbc_csr.int_write_11.get_name() << endl );
}

void cap_pb_eos_sta(int chip_id, int inst_id) {
PLOG_MSG("inside cap_pb_eos_sta \n");

 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
 //cap_pbm_csr_t & pbm_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbm_csr_t, chip_id, inst_id);

 for(int p=0; p<12; p++) {
    pbc_csr.sat_write_error[p].read();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.sat_write_error[p].show();}
 }

 int sched_expd_quota, quota, current_credit;
 unsigned int sched_test = SKNOBS_GET("tb/sched_test", 0);
 string csr_prefix = pbc_csr.get_hier_path();
 std::replace(csr_prefix.begin(), csr_prefix.end(), '.', '/');
 if(sched_test) {

   for(int p_idx=0; p_idx<12; p_idx++) {
       for(int q_idx=0; q_idx<32; q_idx++) {
           sched_expd_quota = SKNOBS_GET((csr_prefix+"/oport/" + to_string(p_idx) + "/l1_sched/" + to_string(q_idx) + "/cfg_quota").c_str(), 0);
   
           if(sched_expd_quota != 0) {
               cap_pb_read_sched_node(chip_id, inst_id, 1, p_idx, q_idx, quota, current_credit);
               if(sched_expd_quota != quota) {
                   PLOG_ERR("L1 oport : " << p_idx << ", oqueue : " << q_idx << ", configured quota : " << sched_expd_quota << " mismatched RTL value : " << quota << endl);
               } else {
                   PLOG_MSG("L1 oport : " << p_idx << ", oqueue : " << q_idx << ", configured quota : " << sched_expd_quota << " matched RTL value : " << quota << endl);
               }
           }
       }
   }
   
   for(int p_idx=0; p_idx<12; p_idx++) {
       for(int node_idx=0; node_idx<16; node_idx++) {
           sched_expd_quota = SKNOBS_GET((csr_prefix+"/oport/" + to_string(p_idx) + "/l2_sched/" + to_string(node_idx) + "/cfg_quota").c_str(), 0);
   
           if(sched_expd_quota != 0) {
               cap_pb_read_sched_node(chip_id, inst_id, 2, p_idx, node_idx, quota, current_credit);
               if(sched_expd_quota != quota) {
                   PLOG_ERR("L2 oport : " << p_idx << ", node : " << node_idx << ", configured quota : " << sched_expd_quota << " mismatched RTL value : " << quota << endl);
               } else {
                   PLOG_MSG("L2 oport : " << p_idx << ", node : " << node_idx << ", configured quota : " << sched_expd_quota << " matched RTL value : " << quota << endl);
               }
           }
       }
   }

   for(int p_idx=9; p_idx<12; p_idx++) {
       for(int node_idx=0; node_idx<4; node_idx++) {
           sched_expd_quota = SKNOBS_GET((csr_prefix+"/oport/" + to_string(p_idx) + "/l3_sched/" + to_string(node_idx) + "/cfg_quota").c_str(), 0);
   
           if(sched_expd_quota != 0) {
               cap_pb_read_sched_node(chip_id, inst_id, 3, p_idx, node_idx, quota, current_credit);
               if(sched_expd_quota != quota) {
                   PLOG_ERR("L3 oport : " << p_idx << ", node : " << node_idx << ", configured quota : " << sched_expd_quota << " mismatched RTL value : " << quota << endl);
               } else {
                   PLOG_MSG("L3 oport : " << p_idx << ", node : " << node_idx << ", configured quota : " << sched_expd_quota << " matched RTL value : " << quota << endl);
               }
           }
       }
   }

 } //if sched_test 

  // disable write ports to flush pre-fetch cache
   pbc_csr.port_0.cfg_write_control.enable(0);
   pbc_csr.port_0.cfg_write_control.write();
   pbc_csr.port_1.cfg_write_control.enable(0);
   pbc_csr.port_1.cfg_write_control.write();
   pbc_csr.port_2.cfg_write_control.enable(0);
   pbc_csr.port_2.cfg_write_control.write();
   pbc_csr.port_3.cfg_write_control.enable(0);
   pbc_csr.port_3.cfg_write_control.write();
   pbc_csr.port_4.cfg_write_control.enable(0);
   pbc_csr.port_4.cfg_write_control.write();
   pbc_csr.port_5.cfg_write_control.enable(0);
   pbc_csr.port_5.cfg_write_control.write();
   pbc_csr.port_6.cfg_write_control.enable(0);
   pbc_csr.port_6.cfg_write_control.write();
   pbc_csr.port_7.cfg_write_control.enable(0);
   pbc_csr.port_7.cfg_write_control.write();
   pbc_csr.port_8.cfg_write_control.enable(0);
   pbc_csr.port_8.cfg_write_control.write();
   pbc_csr.port_9.cfg_write_control.enable(0);
   pbc_csr.port_9.cfg_write_control.write();
   pbc_csr.port_10.cfg_write_control.enable(0);
   pbc_csr.port_10.cfg_write_control.write();
   pbc_csr.port_11.cfg_write_control.enable(0);
   pbc_csr.port_11.cfg_write_control.write();
   // check occ cntrs
    pbc_csr.port_0.sta_account.read();
    if (pbc_csr.port_0.sta_account.occupancy_0() != 0) PLOG_ERR("port_0.sta_account pg 0 is not zero as expected at EOS: " << pbc_csr.port_0.sta_account.occupancy_0() << endl);
    if (pbc_csr.port_0.sta_account.occupancy_1() != 0) PLOG_ERR("port_0.sta_account pg 1 is not zero as expected at EOS: " << pbc_csr.port_0.sta_account.occupancy_1() << endl);
    if (pbc_csr.port_0.sta_account.occupancy_2() != 0) PLOG_ERR("port_0.sta_account pg 2 is not zero as expected at EOS: " << pbc_csr.port_0.sta_account.occupancy_2() << endl);
    if (pbc_csr.port_0.sta_account.occupancy_3() != 0) PLOG_ERR("port_0.sta_account pg 3 is not zero as expected at EOS: " << pbc_csr.port_0.sta_account.occupancy_3() << endl);
    if (pbc_csr.port_0.sta_account.occupancy_4() != 0) PLOG_ERR("port_0.sta_account pg 4 is not zero as expected at EOS: " << pbc_csr.port_0.sta_account.occupancy_4() << endl);
    if (pbc_csr.port_0.sta_account.occupancy_5() != 0) PLOG_ERR("port_0.sta_account pg 5 is not zero as expected at EOS: " << pbc_csr.port_0.sta_account.occupancy_5() << endl);
    if (pbc_csr.port_0.sta_account.occupancy_6() != 0) PLOG_ERR("port_0.sta_account pg 6 is not zero as expected at EOS: " << pbc_csr.port_0.sta_account.occupancy_6() << endl);
    if (pbc_csr.port_0.sta_account.occupancy_7() != 0) PLOG_ERR("port_0.sta_account pg 7 is not zero as expected at EOS: " << pbc_csr.port_0.sta_account.occupancy_7() << endl);
    pbc_csr.port_1.sta_account.read();
    if (pbc_csr.port_1.sta_account.occupancy_0() != 0) PLOG_ERR("port_1.sta_account pg 0 is not zero as expected at EOS: " << pbc_csr.port_1.sta_account.occupancy_0() << endl);
    if (pbc_csr.port_1.sta_account.occupancy_1() != 0) PLOG_ERR("port_1.sta_account pg 1 is not zero as expected at EOS: " << pbc_csr.port_1.sta_account.occupancy_1() << endl);
    if (pbc_csr.port_1.sta_account.occupancy_2() != 0) PLOG_ERR("port_1.sta_account pg 2 is not zero as expected at EOS: " << pbc_csr.port_1.sta_account.occupancy_2() << endl);
    if (pbc_csr.port_1.sta_account.occupancy_3() != 0) PLOG_ERR("port_1.sta_account pg 3 is not zero as expected at EOS: " << pbc_csr.port_1.sta_account.occupancy_3() << endl);
    if (pbc_csr.port_1.sta_account.occupancy_4() != 0) PLOG_ERR("port_1.sta_account pg 4 is not zero as expected at EOS: " << pbc_csr.port_1.sta_account.occupancy_4() << endl);
    if (pbc_csr.port_1.sta_account.occupancy_5() != 0) PLOG_ERR("port_1.sta_account pg 5 is not zero as expected at EOS: " << pbc_csr.port_1.sta_account.occupancy_5() << endl);
    if (pbc_csr.port_1.sta_account.occupancy_6() != 0) PLOG_ERR("port_1.sta_account pg 6 is not zero as expected at EOS: " << pbc_csr.port_1.sta_account.occupancy_6() << endl);
    if (pbc_csr.port_1.sta_account.occupancy_7() != 0) PLOG_ERR("port_1.sta_account pg 7 is not zero as expected at EOS: " << pbc_csr.port_1.sta_account.occupancy_7() << endl);
    pbc_csr.port_2.sta_account.read();
    if (pbc_csr.port_2.sta_account.occupancy_0() != 0) PLOG_ERR("port_2.sta_account pg 0 is not zero as expected at EOS: " << pbc_csr.port_2.sta_account.occupancy_0() << endl);
    if (pbc_csr.port_2.sta_account.occupancy_1() != 0) PLOG_ERR("port_2.sta_account pg 1 is not zero as expected at EOS: " << pbc_csr.port_2.sta_account.occupancy_1() << endl);
    if (pbc_csr.port_2.sta_account.occupancy_2() != 0) PLOG_ERR("port_2.sta_account pg 2 is not zero as expected at EOS: " << pbc_csr.port_2.sta_account.occupancy_2() << endl);
    if (pbc_csr.port_2.sta_account.occupancy_3() != 0) PLOG_ERR("port_2.sta_account pg 3 is not zero as expected at EOS: " << pbc_csr.port_2.sta_account.occupancy_3() << endl);
    if (pbc_csr.port_2.sta_account.occupancy_4() != 0) PLOG_ERR("port_2.sta_account pg 4 is not zero as expected at EOS: " << pbc_csr.port_2.sta_account.occupancy_4() << endl);
    if (pbc_csr.port_2.sta_account.occupancy_5() != 0) PLOG_ERR("port_2.sta_account pg 5 is not zero as expected at EOS: " << pbc_csr.port_2.sta_account.occupancy_5() << endl);
    if (pbc_csr.port_2.sta_account.occupancy_6() != 0) PLOG_ERR("port_2.sta_account pg 6 is not zero as expected at EOS: " << pbc_csr.port_2.sta_account.occupancy_6() << endl);
    if (pbc_csr.port_2.sta_account.occupancy_7() != 0) PLOG_ERR("port_2.sta_account pg 7 is not zero as expected at EOS: " << pbc_csr.port_2.sta_account.occupancy_7() << endl);
    pbc_csr.port_3.sta_account.read();
    if (pbc_csr.port_3.sta_account.occupancy_0() != 0) PLOG_ERR("port_3.sta_account pg 0 is not zero as expected at EOS: " << pbc_csr.port_3.sta_account.occupancy_0() << endl);
    if (pbc_csr.port_3.sta_account.occupancy_1() != 0) PLOG_ERR("port_3.sta_account pg 1 is not zero as expected at EOS: " << pbc_csr.port_3.sta_account.occupancy_1() << endl);
    if (pbc_csr.port_3.sta_account.occupancy_2() != 0) PLOG_ERR("port_3.sta_account pg 2 is not zero as expected at EOS: " << pbc_csr.port_3.sta_account.occupancy_2() << endl);
    if (pbc_csr.port_3.sta_account.occupancy_3() != 0) PLOG_ERR("port_3.sta_account pg 3 is not zero as expected at EOS: " << pbc_csr.port_3.sta_account.occupancy_3() << endl);
    if (pbc_csr.port_3.sta_account.occupancy_4() != 0) PLOG_ERR("port_3.sta_account pg 4 is not zero as expected at EOS: " << pbc_csr.port_3.sta_account.occupancy_4() << endl);
    if (pbc_csr.port_3.sta_account.occupancy_5() != 0) PLOG_ERR("port_3.sta_account pg 5 is not zero as expected at EOS: " << pbc_csr.port_3.sta_account.occupancy_5() << endl);
    if (pbc_csr.port_3.sta_account.occupancy_6() != 0) PLOG_ERR("port_3.sta_account pg 6 is not zero as expected at EOS: " << pbc_csr.port_3.sta_account.occupancy_6() << endl);
    if (pbc_csr.port_3.sta_account.occupancy_7() != 0) PLOG_ERR("port_3.sta_account pg 7 is not zero as expected at EOS: " << pbc_csr.port_3.sta_account.occupancy_7() << endl);
    pbc_csr.port_4.sta_account.read();
    if (pbc_csr.port_4.sta_account.occupancy_0() != 0) PLOG_ERR("port_4.sta_account pg 0 is not zero as expected at EOS: " << pbc_csr.port_4.sta_account.occupancy_0() << endl);
    if (pbc_csr.port_4.sta_account.occupancy_1() != 0) PLOG_ERR("port_4.sta_account pg 1 is not zero as expected at EOS: " << pbc_csr.port_4.sta_account.occupancy_1() << endl);
    if (pbc_csr.port_4.sta_account.occupancy_2() != 0) PLOG_ERR("port_4.sta_account pg 2 is not zero as expected at EOS: " << pbc_csr.port_4.sta_account.occupancy_2() << endl);
    if (pbc_csr.port_4.sta_account.occupancy_3() != 0) PLOG_ERR("port_4.sta_account pg 3 is not zero as expected at EOS: " << pbc_csr.port_4.sta_account.occupancy_3() << endl);
    if (pbc_csr.port_4.sta_account.occupancy_4() != 0) PLOG_ERR("port_4.sta_account pg 4 is not zero as expected at EOS: " << pbc_csr.port_4.sta_account.occupancy_4() << endl);
    if (pbc_csr.port_4.sta_account.occupancy_5() != 0) PLOG_ERR("port_4.sta_account pg 5 is not zero as expected at EOS: " << pbc_csr.port_4.sta_account.occupancy_5() << endl);
    if (pbc_csr.port_4.sta_account.occupancy_6() != 0) PLOG_ERR("port_4.sta_account pg 6 is not zero as expected at EOS: " << pbc_csr.port_4.sta_account.occupancy_6() << endl);
    if (pbc_csr.port_4.sta_account.occupancy_7() != 0) PLOG_ERR("port_4.sta_account pg 7 is not zero as expected at EOS: " << pbc_csr.port_4.sta_account.occupancy_7() << endl);
    pbc_csr.port_5.sta_account.read();
    if (pbc_csr.port_5.sta_account.occupancy_0() != 0) PLOG_ERR("port_5.sta_account pg 0 is not zero as expected at EOS: " << pbc_csr.port_5.sta_account.occupancy_0() << endl);
    if (pbc_csr.port_5.sta_account.occupancy_1() != 0) PLOG_ERR("port_5.sta_account pg 1 is not zero as expected at EOS: " << pbc_csr.port_5.sta_account.occupancy_1() << endl);
    if (pbc_csr.port_5.sta_account.occupancy_2() != 0) PLOG_ERR("port_5.sta_account pg 2 is not zero as expected at EOS: " << pbc_csr.port_5.sta_account.occupancy_2() << endl);
    if (pbc_csr.port_5.sta_account.occupancy_3() != 0) PLOG_ERR("port_5.sta_account pg 3 is not zero as expected at EOS: " << pbc_csr.port_5.sta_account.occupancy_3() << endl);
    if (pbc_csr.port_5.sta_account.occupancy_4() != 0) PLOG_ERR("port_5.sta_account pg 4 is not zero as expected at EOS: " << pbc_csr.port_5.sta_account.occupancy_4() << endl);
    if (pbc_csr.port_5.sta_account.occupancy_5() != 0) PLOG_ERR("port_5.sta_account pg 5 is not zero as expected at EOS: " << pbc_csr.port_5.sta_account.occupancy_5() << endl);
    if (pbc_csr.port_5.sta_account.occupancy_6() != 0) PLOG_ERR("port_5.sta_account pg 6 is not zero as expected at EOS: " << pbc_csr.port_5.sta_account.occupancy_6() << endl);
    if (pbc_csr.port_5.sta_account.occupancy_7() != 0) PLOG_ERR("port_5.sta_account pg 7 is not zero as expected at EOS: " << pbc_csr.port_5.sta_account.occupancy_7() << endl);
    pbc_csr.port_6.sta_account.read();
    if (pbc_csr.port_6.sta_account.occupancy_0() != 0) PLOG_ERR("port_6.sta_account pg 0 is not zero as expected at EOS: " << pbc_csr.port_6.sta_account.occupancy_0() << endl);
    if (pbc_csr.port_6.sta_account.occupancy_1() != 0) PLOG_ERR("port_6.sta_account pg 1 is not zero as expected at EOS: " << pbc_csr.port_6.sta_account.occupancy_1() << endl);
    if (pbc_csr.port_6.sta_account.occupancy_2() != 0) PLOG_ERR("port_6.sta_account pg 2 is not zero as expected at EOS: " << pbc_csr.port_6.sta_account.occupancy_2() << endl);
    if (pbc_csr.port_6.sta_account.occupancy_3() != 0) PLOG_ERR("port_6.sta_account pg 3 is not zero as expected at EOS: " << pbc_csr.port_6.sta_account.occupancy_3() << endl);
    if (pbc_csr.port_6.sta_account.occupancy_4() != 0) PLOG_ERR("port_6.sta_account pg 4 is not zero as expected at EOS: " << pbc_csr.port_6.sta_account.occupancy_4() << endl);
    if (pbc_csr.port_6.sta_account.occupancy_5() != 0) PLOG_ERR("port_6.sta_account pg 5 is not zero as expected at EOS: " << pbc_csr.port_6.sta_account.occupancy_5() << endl);
    if (pbc_csr.port_6.sta_account.occupancy_6() != 0) PLOG_ERR("port_6.sta_account pg 6 is not zero as expected at EOS: " << pbc_csr.port_6.sta_account.occupancy_6() << endl);
    if (pbc_csr.port_6.sta_account.occupancy_7() != 0) PLOG_ERR("port_6.sta_account pg 7 is not zero as expected at EOS: " << pbc_csr.port_6.sta_account.occupancy_7() << endl);
    pbc_csr.port_7.sta_account.read();
    if (pbc_csr.port_7.sta_account.occupancy_0() != 0) PLOG_ERR("port_7.sta_account pg 0 is not zero as expected at EOS: " << pbc_csr.port_7.sta_account.occupancy_0() << endl);
    if (pbc_csr.port_7.sta_account.occupancy_1() != 0) PLOG_ERR("port_7.sta_account pg 1 is not zero as expected at EOS: " << pbc_csr.port_7.sta_account.occupancy_1() << endl);
    if (pbc_csr.port_7.sta_account.occupancy_2() != 0) PLOG_ERR("port_7.sta_account pg 2 is not zero as expected at EOS: " << pbc_csr.port_7.sta_account.occupancy_2() << endl);
    if (pbc_csr.port_7.sta_account.occupancy_3() != 0) PLOG_ERR("port_7.sta_account pg 3 is not zero as expected at EOS: " << pbc_csr.port_7.sta_account.occupancy_3() << endl);
    if (pbc_csr.port_7.sta_account.occupancy_4() != 0) PLOG_ERR("port_7.sta_account pg 4 is not zero as expected at EOS: " << pbc_csr.port_7.sta_account.occupancy_4() << endl);
    if (pbc_csr.port_7.sta_account.occupancy_5() != 0) PLOG_ERR("port_7.sta_account pg 5 is not zero as expected at EOS: " << pbc_csr.port_7.sta_account.occupancy_5() << endl);
    if (pbc_csr.port_7.sta_account.occupancy_6() != 0) PLOG_ERR("port_7.sta_account pg 6 is not zero as expected at EOS: " << pbc_csr.port_7.sta_account.occupancy_6() << endl);
    if (pbc_csr.port_7.sta_account.occupancy_7() != 0) PLOG_ERR("port_7.sta_account pg 7 is not zero as expected at EOS: " << pbc_csr.port_7.sta_account.occupancy_7() << endl);
    pbc_csr.port_8.sta_account.read();
    if (pbc_csr.port_8.sta_account.occupancy_0() != 0) PLOG_ERR("port_8.sta_account pg 0 is not zero as expected at EOS: " << pbc_csr.port_8.sta_account.occupancy_0() << endl);
    if (pbc_csr.port_8.sta_account.occupancy_1() != 0) PLOG_ERR("port_8.sta_account pg 1 is not zero as expected at EOS: " << pbc_csr.port_8.sta_account.occupancy_1() << endl);
    if (pbc_csr.port_8.sta_account.occupancy_2() != 0) PLOG_ERR("port_8.sta_account pg 2 is not zero as expected at EOS: " << pbc_csr.port_8.sta_account.occupancy_2() << endl);
    if (pbc_csr.port_8.sta_account.occupancy_3() != 0) PLOG_ERR("port_8.sta_account pg 3 is not zero as expected at EOS: " << pbc_csr.port_8.sta_account.occupancy_3() << endl);
    if (pbc_csr.port_8.sta_account.occupancy_4() != 0) PLOG_ERR("port_8.sta_account pg 4 is not zero as expected at EOS: " << pbc_csr.port_8.sta_account.occupancy_4() << endl);
    if (pbc_csr.port_8.sta_account.occupancy_5() != 0) PLOG_ERR("port_8.sta_account pg 5 is not zero as expected at EOS: " << pbc_csr.port_8.sta_account.occupancy_5() << endl);
    if (pbc_csr.port_8.sta_account.occupancy_6() != 0) PLOG_ERR("port_8.sta_account pg 6 is not zero as expected at EOS: " << pbc_csr.port_8.sta_account.occupancy_6() << endl);
    if (pbc_csr.port_8.sta_account.occupancy_7() != 0) PLOG_ERR("port_8.sta_account pg 7 is not zero as expected at EOS: " << pbc_csr.port_8.sta_account.occupancy_7() << endl);
    pbc_csr.port_9.sta_account.read();
    if (pbc_csr.port_9.sta_account.occupancy_0() != 0) PLOG_ERR("port_9.sta_account pg 0 is not zero as expected at EOS: " << pbc_csr.port_9.sta_account.occupancy_0() << endl);
    if (pbc_csr.port_9.sta_account.occupancy_1() != 0) PLOG_ERR("port_9.sta_account pg 1 is not zero as expected at EOS: " << pbc_csr.port_9.sta_account.occupancy_1() << endl);
    if (pbc_csr.port_9.sta_account.occupancy_2() != 0) PLOG_ERR("port_9.sta_account pg 2 is not zero as expected at EOS: " << pbc_csr.port_9.sta_account.occupancy_2() << endl);
    if (pbc_csr.port_9.sta_account.occupancy_3() != 0) PLOG_ERR("port_9.sta_account pg 3 is not zero as expected at EOS: " << pbc_csr.port_9.sta_account.occupancy_3() << endl);
    if (pbc_csr.port_9.sta_account.occupancy_4() != 0) PLOG_ERR("port_9.sta_account pg 4 is not zero as expected at EOS: " << pbc_csr.port_9.sta_account.occupancy_4() << endl);
    if (pbc_csr.port_9.sta_account.occupancy_5() != 0) PLOG_ERR("port_9.sta_account pg 5 is not zero as expected at EOS: " << pbc_csr.port_9.sta_account.occupancy_5() << endl);
    if (pbc_csr.port_9.sta_account.occupancy_6() != 0) PLOG_ERR("port_9.sta_account pg 6 is not zero as expected at EOS: " << pbc_csr.port_9.sta_account.occupancy_6() << endl);
    if (pbc_csr.port_9.sta_account.occupancy_7() != 0) PLOG_ERR("port_9.sta_account pg 7 is not zero as expected at EOS: " << pbc_csr.port_9.sta_account.occupancy_7() << endl);
    if (pbc_csr.port_9.sta_account.occupancy_8() != 0) PLOG_ERR("port_9.sta_account pg 8 is not zero as expected at EOS: " << pbc_csr.port_9.sta_account.occupancy_8() << endl);
    if (pbc_csr.port_9.sta_account.occupancy_9() != 0) PLOG_ERR("port_9.sta_account pg 9 is not zero as expected at EOS: " << pbc_csr.port_9.sta_account.occupancy_9() << endl);
    if (pbc_csr.port_9.sta_account.occupancy_10() != 0) PLOG_ERR("port_9.sta_account pg 10 is not zero as expected at EOS: " << pbc_csr.port_9.sta_account.occupancy_10() << endl);
    if (pbc_csr.port_9.sta_account.occupancy_11() != 0) PLOG_ERR("port_9.sta_account pg 11 is not zero as expected at EOS: " << pbc_csr.port_9.sta_account.occupancy_11() << endl);
    if (pbc_csr.port_9.sta_account.occupancy_12() != 0) PLOG_ERR("port_9.sta_account pg 12 is not zero as expected at EOS: " << pbc_csr.port_9.sta_account.occupancy_12() << endl);
    if (pbc_csr.port_9.sta_account.occupancy_13() != 0) PLOG_ERR("port_9.sta_account pg 13 is not zero as expected at EOS: " << pbc_csr.port_9.sta_account.occupancy_13() << endl);
    if (pbc_csr.port_9.sta_account.occupancy_14() != 0) PLOG_ERR("port_9.sta_account pg 14 is not zero as expected at EOS: " << pbc_csr.port_9.sta_account.occupancy_14() << endl);
    if (pbc_csr.port_9.sta_account.occupancy_15() != 0) PLOG_ERR("port_9.sta_account pg 15 is not zero as expected at EOS: " << pbc_csr.port_9.sta_account.occupancy_15() << endl);
    pbc_csr.port_10.sta_account.read();
    if (pbc_csr.port_10.sta_account.occupancy_0() != 0) PLOG_ERR("port_10.sta_account pg 0 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_0() << endl);
    if (pbc_csr.port_10.sta_account.occupancy_1() != 0) PLOG_ERR("port_10.sta_account pg 1 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_1() << endl);
    if (pbc_csr.port_10.sta_account.occupancy_2() != 0) PLOG_ERR("port_10.sta_account pg 2 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_2() << endl);
    if (pbc_csr.port_10.sta_account.occupancy_3() != 0) PLOG_ERR("port_10.sta_account pg 3 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_3() << endl);
    if (pbc_csr.port_10.sta_account.occupancy_4() != 0) PLOG_ERR("port_10.sta_account pg 4 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_4() << endl);
    if (pbc_csr.port_10.sta_account.occupancy_5() != 0) PLOG_ERR("port_10.sta_account pg 5 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_5() << endl);
    if (pbc_csr.port_10.sta_account.occupancy_6() != 0) PLOG_ERR("port_10.sta_account pg 6 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_6() << endl);
    if (pbc_csr.port_10.sta_account.occupancy_7() != 0) PLOG_ERR("port_10.sta_account pg 7 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_7() << endl);
    if (pbc_csr.port_10.sta_account.occupancy_8() != 0) PLOG_ERR("port_10.sta_account pg 8 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_8() << endl);
    if (pbc_csr.port_10.sta_account.occupancy_9() != 0) PLOG_ERR("port_10.sta_account pg 9 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_9() << endl);
    if (pbc_csr.port_10.sta_account.occupancy_10() != 0) PLOG_ERR("port_10.sta_account pg 10 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_10() << endl);
    if (pbc_csr.port_10.sta_account.occupancy_11() != 0) PLOG_ERR("port_10.sta_account pg 11 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_11() << endl);
    if (pbc_csr.port_10.sta_account.occupancy_12() != 0) PLOG_ERR("port_10.sta_account pg 12 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_12() << endl);
    if (pbc_csr.port_10.sta_account.occupancy_13() != 0) PLOG_ERR("port_10.sta_account pg 13 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_13() << endl);
    if (pbc_csr.port_10.sta_account.occupancy_14() != 0) PLOG_ERR("port_10.sta_account pg 14 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_14() << endl);
    if (pbc_csr.port_10.sta_account.occupancy_15() != 0) PLOG_ERR("port_10.sta_account pg 15 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_15() << endl);
    if (pbc_csr.port_10.sta_account.occupancy_16() != 0) PLOG_ERR("port_10.sta_account pg 16 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_16() << endl);
    if (pbc_csr.port_10.sta_account.occupancy_17() != 0) PLOG_ERR("port_10.sta_account pg 17 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_17() << endl);
    if (pbc_csr.port_10.sta_account.occupancy_18() != 0) PLOG_ERR("port_10.sta_account pg 18 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_18() << endl);
    if (pbc_csr.port_10.sta_account.occupancy_19() != 0) PLOG_ERR("port_10.sta_account pg 19 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_19() << endl);
    if (pbc_csr.port_10.sta_account.occupancy_20() != 0) PLOG_ERR("port_10.sta_account pg 20 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_20() << endl);
    if (pbc_csr.port_10.sta_account.occupancy_21() != 0) PLOG_ERR("port_10.sta_account pg 21 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_21() << endl);
    if (pbc_csr.port_10.sta_account.occupancy_22() != 0) PLOG_ERR("port_10.sta_account pg 22 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_22() << endl);
    if (pbc_csr.port_10.sta_account.occupancy_23() != 0) PLOG_ERR("port_10.sta_account pg 23 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_23() << endl);
    if (pbc_csr.port_10.sta_account.occupancy_24() != 0) PLOG_ERR("port_10.sta_account pg 24 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_24() << endl);
    if (pbc_csr.port_10.sta_account.occupancy_25() != 0) PLOG_ERR("port_10.sta_account pg 25 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_25() << endl);
    if (pbc_csr.port_10.sta_account.occupancy_26() != 0) PLOG_ERR("port_10.sta_account pg 26 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_26() << endl);
    if (pbc_csr.port_10.sta_account.occupancy_27() != 0) PLOG_ERR("port_10.sta_account pg 27 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_27() << endl);
    if (pbc_csr.port_10.sta_account.occupancy_28() != 0) PLOG_ERR("port_10.sta_account pg 28 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_28() << endl);
    if (pbc_csr.port_10.sta_account.occupancy_29() != 0) PLOG_ERR("port_10.sta_account pg 29 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_29() << endl);
    if (pbc_csr.port_10.sta_account.occupancy_30() != 0) PLOG_ERR("port_10.sta_account pg 30 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_30() << endl);
    if (pbc_csr.port_10.sta_account.occupancy_31() != 0) PLOG_ERR("port_10.sta_account pg 31 is not zero as expected at EOS: " << pbc_csr.port_10.sta_account.occupancy_31() << endl);
    pbc_csr.port_11.sta_account.read();
    if (pbc_csr.port_11.sta_account.occupancy_0() != 0) PLOG_ERR("port_11.sta_account pg 0 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_0() << endl);
    if (pbc_csr.port_11.sta_account.occupancy_1() != 0) PLOG_ERR("port_11.sta_account pg 1 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_1() << endl);
    if (pbc_csr.port_11.sta_account.occupancy_2() != 0) PLOG_ERR("port_11.sta_account pg 2 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_2() << endl);
    if (pbc_csr.port_11.sta_account.occupancy_3() != 0) PLOG_ERR("port_11.sta_account pg 3 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_3() << endl);
    if (pbc_csr.port_11.sta_account.occupancy_4() != 0) PLOG_ERR("port_11.sta_account pg 4 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_4() << endl);
    if (pbc_csr.port_11.sta_account.occupancy_5() != 0) PLOG_ERR("port_11.sta_account pg 5 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_5() << endl);
    if (pbc_csr.port_11.sta_account.occupancy_6() != 0) PLOG_ERR("port_11.sta_account pg 6 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_6() << endl);
    if (pbc_csr.port_11.sta_account.occupancy_7() != 0) PLOG_ERR("port_11.sta_account pg 7 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_7() << endl);
    if (pbc_csr.port_11.sta_account.occupancy_8() != 0) PLOG_ERR("port_11.sta_account pg 8 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_8() << endl);
    if (pbc_csr.port_11.sta_account.occupancy_9() != 0) PLOG_ERR("port_11.sta_account pg 9 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_9() << endl);
    if (pbc_csr.port_11.sta_account.occupancy_10() != 0) PLOG_ERR("port_11.sta_account pg 10 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_10() << endl);
    if (pbc_csr.port_11.sta_account.occupancy_11() != 0) PLOG_ERR("port_11.sta_account pg 11 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_11() << endl);
    if (pbc_csr.port_11.sta_account.occupancy_12() != 0) PLOG_ERR("port_11.sta_account pg 12 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_12() << endl);
    if (pbc_csr.port_11.sta_account.occupancy_13() != 0) PLOG_ERR("port_11.sta_account pg 13 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_13() << endl);
    if (pbc_csr.port_11.sta_account.occupancy_14() != 0) PLOG_ERR("port_11.sta_account pg 14 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_14() << endl);
    if (pbc_csr.port_11.sta_account.occupancy_15() != 0) PLOG_ERR("port_11.sta_account pg 15 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_15() << endl);
    if (pbc_csr.port_11.sta_account.occupancy_16() != 0) PLOG_ERR("port_11.sta_account pg 16 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_16() << endl);
    if (pbc_csr.port_11.sta_account.occupancy_17() != 0) PLOG_ERR("port_11.sta_account pg 17 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_17() << endl);
    if (pbc_csr.port_11.sta_account.occupancy_18() != 0) PLOG_ERR("port_11.sta_account pg 18 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_18() << endl);
    if (pbc_csr.port_11.sta_account.occupancy_19() != 0) PLOG_ERR("port_11.sta_account pg 19 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_19() << endl);
    if (pbc_csr.port_11.sta_account.occupancy_20() != 0) PLOG_ERR("port_11.sta_account pg 20 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_20() << endl);
    if (pbc_csr.port_11.sta_account.occupancy_21() != 0) PLOG_ERR("port_11.sta_account pg 21 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_21() << endl);
    if (pbc_csr.port_11.sta_account.occupancy_22() != 0) PLOG_ERR("port_11.sta_account pg 22 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_22() << endl);
    if (pbc_csr.port_11.sta_account.occupancy_23() != 0) PLOG_ERR("port_11.sta_account pg 23 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_23() << endl);
    if (pbc_csr.port_11.sta_account.occupancy_24() != 0) PLOG_ERR("port_11.sta_account pg 24 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_24() << endl);
    if (pbc_csr.port_11.sta_account.occupancy_25() != 0) PLOG_ERR("port_11.sta_account pg 25 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_25() << endl);
    if (pbc_csr.port_11.sta_account.occupancy_26() != 0) PLOG_ERR("port_11.sta_account pg 26 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_26() << endl);
    if (pbc_csr.port_11.sta_account.occupancy_27() != 0) PLOG_ERR("port_11.sta_account pg 27 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_27() << endl);
    if (pbc_csr.port_11.sta_account.occupancy_28() != 0) PLOG_ERR("port_11.sta_account pg 28 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_28() << endl);
    if (pbc_csr.port_11.sta_account.occupancy_29() != 0) PLOG_ERR("port_11.sta_account pg 29 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_29() << endl);
    if (pbc_csr.port_11.sta_account.occupancy_30() != 0) PLOG_ERR("port_11.sta_account pg 30 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_30() << endl);
    if (pbc_csr.port_11.sta_account.occupancy_31() != 0) PLOG_ERR("port_11.sta_account pg 31 is not zero as expected at EOS: " << pbc_csr.port_11.sta_account.occupancy_31() << endl);
    for (int i=0; i<12 ; i++) {
       pbc_csr.sta_oq[i].read();
       if (pbc_csr.sta_oq[i].all() != 0) {
         PLOG_ERR("sta_oq[i] is not zero as expected at EOS \n");
         if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.sta_oq[i].show();}
       }
    }
    //make sure all credits are returned if enabled
    for (int ii=0; ii<pbc_csr.port_10.dhs_oq_flow_control.get_depth_entry(); ii++) {
       pbc_csr.port_10.dhs_oq_flow_control.entry[ii].read_compare();
    }
    for (int ii=0; ii<pbc_csr.port_11.dhs_oq_flow_control.get_depth_entry(); ii++) {
       pbc_csr.port_11.dhs_oq_flow_control.entry[ii].read_compare();
    }
}

void cap_pb_eos_fc_check(int chip_id, int inst_id) {
PLOG_MSG("inside cap_pb_eos_fc_check\n");

 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
 std::bitset<13312> uniq;
 uint32_t last_size=0;
 uint32_t tries=0;
 std::queue<uint32_t> cell_list;

 // make sure we turn off the conservation of mass check in simulation
   pbc_csr.cfg_spare.data(0xffffffff);
   pbc_csr.cfg_spare.write();
   pbc_csr.cfg_spare.read(); // flush

 while (tries < 13314) {

    pbc_csr.dhs_fc_backdoor_0.entry.read();

    const uint32_t valid= pbc_csr.dhs_fc_backdoor_0.entry.valid().convert_to<uint32_t>();

    if (!valid) break;
    
    const uint32_t cell = pbc_csr.dhs_fc_backdoor_0.entry.cell().convert_to<uint32_t>();

    if (uniq[cell]) { 
         PLOG_ERR("island 0: cell " << cell << " repeated " << endl);
    } else {
	 uniq[cell] = 1;
	 cell_list.push(cell);
    }

    tries++;
    
    if ((tries % 100) ==0) {
	    PLOG_MSG("cap_pb_eos_fc_check: read " << tries << " cells\n");
    }
 }

 pbc_csr.cfg_fc_mgr_0.read();

 { 	const uint32_t exp_cells = (pbc_csr.cfg_fc_mgr_0.max_row().convert_to<uint32_t>() + 1) * 2;
 	const uint32_t diff = uniq.count() - last_size;

	 if (diff != exp_cells) {
		 PLOG_ERR("island 0: expected " << exp_cells << " found " << diff  << endl);
	 }
 }
  last_size = uniq.count();
  

 while (!cell_list.empty()) {
    pbc_csr.dhs_fc_backdoor_0.entry.cell(cell_list.front());
    pbc_csr.dhs_fc_backdoor_0.entry.write();

    if ((cell_list.size() % 100) ==0) {
	    PLOG_MSG("cap_pb_eos_fc_check: writes pending: " << cell_list.size() << endl);
    }

    cell_list.pop();
 }

 while (tries < 13314) {

    pbc_csr.dhs_fc_backdoor_1.entry.read();

    const uint32_t valid= pbc_csr.dhs_fc_backdoor_1.entry.valid().convert_to<uint32_t>();

    if (!valid) break;
    
    const uint32_t cell = pbc_csr.dhs_fc_backdoor_1.entry.cell().convert_to<uint32_t>();

    if (uniq[cell]) { 
         PLOG_ERR("island 1: cell " << cell << " repeated " << endl);
    } else {
	 uniq[cell] = 1;
	 cell_list.push(cell);
    }

    tries++;
    
    if ((tries % 100) ==0) {
	    PLOG_MSG("cap_pb_eos_fc_check: read " << tries << " cells\n");
    }
 }

 pbc_csr.cfg_fc_mgr_1.read();

 { 	const uint32_t exp_cells = (pbc_csr.cfg_fc_mgr_1.max_row().convert_to<uint32_t>() + 1) * 2;
 	const uint32_t diff = uniq.count() - last_size;

	 if (diff != exp_cells) {
		 PLOG_ERR("island 1: expected " << exp_cells << " found " << diff  << endl);
	 }
 }
  last_size = uniq.count();
  

 while (!cell_list.empty()) {
    pbc_csr.dhs_fc_backdoor_1.entry.cell(cell_list.front());
    pbc_csr.dhs_fc_backdoor_1.entry.write();

    if ((cell_list.size() % 100) ==0) {
	    PLOG_MSG("cap_pb_eos_fc_check: writes pending: " << cell_list.size() << endl);
    }

    cell_list.pop();
 }

}

// hbm read and write fifo memories
void cap_pb_bist_hbm_rd_wr_launch(int chip_id, int inst_id) {
	 const char* const fname = "cap_pb_bist_hbm_rd_wr_launch";
  	 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);

         PLOG_MSG("\t" << fname << ": starts\n");

	 pbc_csr.hbm.hbm_port_0.cfg_hbm_mem.bist_run(3);
	 pbc_csr.hbm.hbm_port_0.cfg_hbm_mem.write();
	 pbc_csr.hbm.hbm_port_1.cfg_hbm_mem.bist_run(3);
	 pbc_csr.hbm.hbm_port_1.cfg_hbm_mem.write();
	 pbc_csr.hbm.hbm_port_2.cfg_hbm_mem.bist_run(3);
	 pbc_csr.hbm.hbm_port_2.cfg_hbm_mem.write();
	 pbc_csr.hbm.hbm_port_3.cfg_hbm_mem.bist_run(3);
	 pbc_csr.hbm.hbm_port_3.cfg_hbm_mem.write();
	 pbc_csr.hbm.hbm_port_9.cfg_hbm_mem.bist_run(3);
	 pbc_csr.hbm.hbm_port_9.cfg_hbm_mem.write();
	 // port 6 bist logic is connected to the reorder buffer 2 extra rams
	 pbc_csr.hbm.hbm_port_6.cfg_hbm_mem.bist_run(3);
	 pbc_csr.hbm.hbm_port_6.cfg_hbm_mem.write();

         PLOG_MSG("\t" << fname << ": ends\n");
}

void cap_pb_bist_hbm_rd_wr_poll(int chip_id, int inst_id) {
  	 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
	 const char* const fname = "cap_pb_bist_hbm_rd_wr_poll";
	 bool done=false;
	 int tries=1000;
	 uint32_t results_mask=1<<6; // expect port 6 to be set, the rams are used as reorder buffer

         PLOG_MSG("\t" << fname << ": starts\n");

	 results_mask |= 1 << 0;
	 results_mask |= 1 << 1;
	 results_mask |= 1 << 2;
	 results_mask |= 1 << 3;
	 results_mask |= 1 << 9;
	 do {	pbc_csr.hbm.sta_bist.read();
		const uint32_t write_result	= pbc_csr.hbm.sta_bist.hbm_write_fifo_done_fail().convert_to<uint32_t>()
						| pbc_csr.hbm.sta_bist.hbm_write_fifo_done_pass().convert_to<uint32_t>()
						;
		const uint32_t read_result	= pbc_csr.hbm.sta_bist.hbm_read_fifo_done_fail().convert_to<uint32_t>()
						| pbc_csr.hbm.sta_bist.hbm_read_fifo_done_pass().convert_to<uint32_t>()
						;
		done	= ( (write_result & results_mask) == results_mask)
			| ( (read_result & results_mask) == results_mask)
			;

	 } while (!done && tries--);

	 if (tries<=0) {
	      PLOG_ERR("\t" << fname << ": timed out waiting for bist status\n");
	 }

	 if (pbc_csr.hbm.sta_bist.hbm_write_fifo_done_fail().convert_to<uint32_t>()) {
	      PLOG_ERR("\t" << fname << ": write bist failed: "<<pbc_csr.hbm.sta_bist.hbm_write_fifo_done_fail().convert_to<uint32_t>()<<endl);
	 }
	 if (pbc_csr.hbm.sta_bist.hbm_read_fifo_done_fail().convert_to<uint32_t>()) {
	      PLOG_ERR("\t" << fname << ": read bist failed: "<<pbc_csr.hbm.sta_bist.hbm_read_fifo_done_fail().convert_to<uint32_t>()<<endl);
	 }

         PLOG_MSG("\t" << fname << ": ends: read pass=" << hex << pbc_csr.hbm.sta_bist.hbm_read_fifo_done_pass().convert_to<uint32_t>() 
	 		<< " write pass = " << pbc_csr.hbm.sta_bist.hbm_write_fifo_done_pass().convert_to<uint32_t>() 
	 		<< endl);
}

void cap_pb_bist_hbm_rd_wr_clear(int chip_id, int inst_id) {
	 const char* const fname = "cap_pb_bist_hbm_rd_wr_clear";
  	 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);

         PLOG_MSG("\t" << fname << ": starts\n");

	 pbc_csr.hbm.hbm_port_0.cfg_hbm_mem.bist_run(0);
	 pbc_csr.hbm.hbm_port_0.cfg_hbm_mem.write();
	 pbc_csr.hbm.hbm_port_1.cfg_hbm_mem.bist_run(0);
	 pbc_csr.hbm.hbm_port_1.cfg_hbm_mem.write();
	 pbc_csr.hbm.hbm_port_2.cfg_hbm_mem.bist_run(0);
	 pbc_csr.hbm.hbm_port_2.cfg_hbm_mem.write();
	 pbc_csr.hbm.hbm_port_3.cfg_hbm_mem.bist_run(0);
	 pbc_csr.hbm.hbm_port_3.cfg_hbm_mem.write();
	 pbc_csr.hbm.hbm_port_9.cfg_hbm_mem.bist_run(0);
	 pbc_csr.hbm.hbm_port_9.cfg_hbm_mem.write();
	 // port 6 bist logic is connected to the reorder buffer 2 extra rams
	 pbc_csr.hbm.hbm_port_6.cfg_hbm_mem.bist_run(0);
	 pbc_csr.hbm.hbm_port_6.cfg_hbm_mem.write();

         PLOG_MSG("\t" << fname << ": ends\n");
}

// hbm reorder buffer
void cap_pb_bist_hbm_rb_launch(int chip_id, int inst_id) {
	 const char* const fname = "cap_pb_bist_hbm_rb_launch";
  	 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
         PLOG_MSG("\t" << fname << ": starts\n");

	 pbc_csr.hbm.cfg_hbm_rb.bist_run(3);
	 pbc_csr.hbm.cfg_hbm_rb.write();

         PLOG_MSG("\t" << fname << ": ends\n");
}

void cap_pb_bist_hbm_rb_poll(int chip_id, int inst_id) {
  	 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
	 const char* const fname = "cap_pb_bist_hbm_rb_poll";
	 bool done=false;
	 int tries=1000;
	 const uint32_t results_mask = 3;
	 uint32_t fail,pass;

         PLOG_MSG("\t" << fname << ": starts\n");

	 do {	pbc_csr.hbm.sta_hbm_rb.read();

	 	fail = pbc_csr.hbm.sta_hbm_rb.bist_done_fail().convert_to<uint32_t>();
		pass = pbc_csr.hbm.sta_hbm_rb.bist_done_pass().convert_to<uint32_t>();

		const uint32_t result	= fail | pass ;

		done = (result & results_mask) == results_mask;

	 } while (!done && tries--);

	 if (tries<=0)   { PLOG_ERR("\t" << fname << ": timed out waiting for bist status\n");
	 } else if (fail){ PLOG_ERR("\t" << fname << ": bist failed: " << hex << fail << endl);
	 }

         PLOG_MSG("\t" << fname << ": ends: pass=" << hex << pass << endl);
}

void cap_pb_bist_hbm_rb_clear(int chip_id, int inst_id) {
	 const char* const fname = "cap_pb_bist_hbm_rb_clear";
  	 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
         PLOG_MSG("\t" << fname << ": starts\n");

	 pbc_csr.hbm.cfg_hbm_rb.bist_run(0);
	 pbc_csr.hbm.cfg_hbm_rb.write();

         PLOG_MSG("\t" << fname << ": ends\n");
}

// hbm write back buffer
void cap_pb_bist_hbm_wb_launch(int chip_id, int inst_id) {
	 const char* const fname = "cap_pb_bist_hbm_wb_launch";
  	 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
         PLOG_MSG("\t" << fname << ": starts\n");

	 pbc_csr.hbm.cfg_hbm_wb.bist_run(1);
	 pbc_csr.hbm.cfg_hbm_wb.write();

         PLOG_MSG("\t" << fname << ": ends\n");
}

void cap_pb_bist_hbm_wb_poll(int chip_id, int inst_id) {
	 const char* const fname = "cap_pb_bist_hbm_wb_poll";
  	 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
         PLOG_MSG("\t" << fname << ": starts\n");
	 bool done=false;
	 int tries=5000;
	 const uint32_t results_mask = 1;
	 uint32_t fail,pass;
	 do {	pbc_csr.hbm.sta_hbm_wb.read();

	 	fail = pbc_csr.hbm.sta_hbm_wb.bist_done_fail().convert_to<uint32_t>();
		pass = pbc_csr.hbm.sta_hbm_wb.bist_done_pass().convert_to<uint32_t>();

		const uint32_t result	= fail | pass ;

		done	= (result & results_mask) == results_mask;

	 } while (!done && tries--);

	 if (tries<=0)   { PLOG_ERR("\t" << fname << ": timed out waiting for bist status\n");
	 } else if (fail){ PLOG_ERR("\t" << fname << ": bist failed: " << hex << fail << endl);
	 }

         PLOG_MSG("\t" << fname << ": ends: pass=" << hex << pass << endl);
}

void cap_pb_bist_hbm_wb_clear(int chip_id, int inst_id) {
	 const char* const fname = "cap_pb_bist_hbm_wb_clear";
  	 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
         PLOG_MSG("\t" << fname << ": starts\n");

	 pbc_csr.hbm.cfg_hbm_wb.bist_run(0);
	 pbc_csr.hbm.cfg_hbm_wb.write();

         PLOG_MSG("\t" << fname << ": ends\n");
}

// hbm context rams
void cap_pb_bist_hbm_ctx_launch(int chip_id, int inst_id) {
	 const char* const fname = "cap_pb_bist_hbm_ctx_launch";
  	 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
         PLOG_MSG("\t" << fname << ": starts\n");

	 pbc_csr.hbm.cfg_hbm.bist_run_tx_head(1);
	 pbc_csr.hbm.cfg_hbm.bist_run_tx_tail(1);
	 pbc_csr.hbm.cfg_hbm.bist_run_eth_head(1);
	 pbc_csr.hbm.cfg_hbm.bist_run_eth_tail(1);
	 pbc_csr.hbm.cfg_hbm.write();
         PLOG_MSG("\t" << fname << ": ends\n");
}

void cap_pb_bist_hbm_ctx_poll(int chip_id, int inst_id) {
	 const char* const fname = "cap_pb_bist_hbm_ctx_poll";
  	 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
         PLOG_MSG("\t" << fname << ": starts\n");
	 bool done=false;
	 int tries=5000;
	 const uint32_t results_mask = 0xf;
	 uint32_t fail,pass;
	 do {	pbc_csr.hbm.sta_bist.read();

	 	fail = (pbc_csr.hbm.sta_bist.ht_eth_tail_done_fail().convert_to<uint32_t>() ? 1 : 0)
		     | (pbc_csr.hbm.sta_bist.ht_eth_head_done_fail().convert_to<uint32_t>() ? 2 : 0)
		     | (pbc_csr.hbm.sta_bist.ht_tx_tail_done_fail().convert_to<uint32_t>()  ? 4 : 0)
		     | ( pbc_csr.hbm.sta_bist.ht_tx_head_done_fail().convert_to<uint32_t>() ? 8 : 0)
		     ;

	 	pass = (pbc_csr.hbm.sta_bist.ht_eth_tail_done_pass().convert_to<uint32_t>() ? 1 : 0)
		     | (pbc_csr.hbm.sta_bist.ht_eth_head_done_pass().convert_to<uint32_t>() ? 2 : 0)
		     | (pbc_csr.hbm.sta_bist.ht_tx_tail_done_pass().convert_to<uint32_t>()  ? 4 : 0)
		     | ( pbc_csr.hbm.sta_bist.ht_tx_head_done_pass().convert_to<uint32_t>() ? 8 : 0)
		     ;

		const uint32_t result = fail | pass ;

		done	= (result & results_mask) == results_mask ;

	 } while (!done && tries--);

	 if (tries<=0)   { PLOG_ERR("\t" << fname << ": timed out waiting for bist status\n");
	 } else if (fail){ PLOG_ERR("\t" << fname << ": bist failed: " << hex << fail << endl);
	 }

         PLOG_MSG("\t" << fname << ": ends: pass=" << hex << pass << endl);
}

void cap_pb_bist_hbm_ctx_clear(int chip_id, int inst_id) {
	 const char* const fname = "cap_pb_bist_hbm_ctx_clear";
  	 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
         PLOG_MSG("\t" << fname << ": starts\n");

	 pbc_csr.hbm.cfg_hbm.bist_run_tx_head(0);
	 pbc_csr.hbm.cfg_hbm.bist_run_tx_tail(0);
	 pbc_csr.hbm.cfg_hbm.bist_run_eth_head(0);
	 pbc_csr.hbm.cfg_hbm.bist_run_eth_tail(0);
	 pbc_csr.hbm.cfg_hbm.write();
         PLOG_MSG("\t" << fname << ": ends\n");
}

// output queues
void cap_pb_bist_launch(int chip_id, int inst_id) {
	 const char* const fname = "cap_pb_bist_launch";
  	 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
         PLOG_MSG("\t" << fname << ": starts\n");

	 pbc_csr.port_10.cfg_oq.bist_run(0xf);
	 pbc_csr.port_10.cfg_oq.write();

	 pbc_csr.cfg_rc.bist_run(1);
	 pbc_csr.cfg_rc.write();

	 pbc_csr.cfg_sched.bist_run(3);
	 pbc_csr.cfg_sched.write();

	 pbc_csr.cfg_nc.bist_run(1);
	 pbc_csr.cfg_nc.write();

	 pbc_csr.cfg_uc.bist_run(1);
	 pbc_csr.cfg_uc.write();

	 pbc_csr.cfg_port_mon_in.bist_run(1);
	 pbc_csr.cfg_port_mon_in.write();

	 pbc_csr.cfg_port_mon_out.bist_run(1);
	 pbc_csr.cfg_port_mon_out.write();

	 pbc_csr.port_11.cfg_oq.bist_run(0xff);
	 pbc_csr.port_11.cfg_oq.write();

	 pbc_csr.cfg_fc_mgr_0.bist_run(1);
	 pbc_csr.cfg_fc_mgr_0.write();

	 pbc_csr.cfg_ll_0.bist_run(1);
	 pbc_csr.cfg_ll_0.write();

	 pbc_csr.cfg_desc_0.bist_run(1);
	 pbc_csr.cfg_desc_0.write();
	 pbc_csr.cfg_fc_mgr_1.bist_run(1);
	 pbc_csr.cfg_fc_mgr_1.write();

	 pbc_csr.cfg_ll_1.bist_run(1);
	 pbc_csr.cfg_ll_1.write();

	 pbc_csr.cfg_desc_1.bist_run(1);
	 pbc_csr.cfg_desc_1.write();
         PLOG_MSG("\t" << fname << ": ends\n");
}

void cap_pb_bist_clear(int chip_id, int inst_id) {
	 const char* const fname = "cap_pb_bist_clear";
  	 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
         PLOG_MSG("\t" << fname << ": starts\n");

	 pbc_csr.port_10.cfg_oq.bist_run(0);
	 pbc_csr.port_10.cfg_oq.write();

	 pbc_csr.cfg_rc.bist_run(0);
	 pbc_csr.cfg_rc.write();

	 pbc_csr.cfg_sched.bist_run(0);
	 pbc_csr.cfg_sched.write();

	 pbc_csr.cfg_nc.bist_run(0);
	 pbc_csr.cfg_nc.write();

	 pbc_csr.cfg_uc.bist_run(0);
	 pbc_csr.cfg_uc.write();

	 pbc_csr.cfg_port_mon_in.bist_run(0);
	 pbc_csr.cfg_port_mon_in.write();

	 pbc_csr.cfg_port_mon_out.bist_run(0);
	 pbc_csr.cfg_port_mon_out.write();

	 pbc_csr.port_11.cfg_oq.bist_run(0);
	 pbc_csr.port_11.cfg_oq.write();

	 pbc_csr.cfg_fc_mgr_0.bist_run(0);
	 pbc_csr.cfg_fc_mgr_0.write();

	 pbc_csr.cfg_ll_0.bist_run(0);
	 pbc_csr.cfg_ll_0.write();

	 pbc_csr.cfg_desc_0.bist_run(0);
	 pbc_csr.cfg_desc_0.write();
	 pbc_csr.cfg_fc_mgr_1.bist_run(0);
	 pbc_csr.cfg_fc_mgr_1.write();

	 pbc_csr.cfg_ll_1.bist_run(0);
	 pbc_csr.cfg_ll_1.write();

	 pbc_csr.cfg_desc_1.bist_run(0);
	 pbc_csr.cfg_desc_1.write();
         PLOG_MSG("\t" << fname << ": ends\n");
}

void cap_pm_bist_launch(int chip_id, int inst_id) {
	 const char* const fname = "cap_pm_bist_launch";
  	 cap_pbm_csr_t & pbm_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbm_csr_t, chip_id, inst_id);
         PLOG_MSG("\t" << fname << ": starts\n");

	 pbm_csr.cfg_bist_row_0.run( (1ULL<<40ULL)-1ULL );
	 pbm_csr.cfg_bist_row_0.write();
	 pbm_csr.cfg_bist_row_1.run( (1ULL<<40ULL)-1ULL );
	 pbm_csr.cfg_bist_row_1.write();

         PLOG_MSG("\t" << fname << ": ends\n");
}

void cap_pm_bist_clear(int chip_id, int inst_id) {
	 const char* const fname = "cap_pm_bist_clear";
  	 cap_pbm_csr_t & pbm_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbm_csr_t, chip_id, inst_id);
         PLOG_MSG("\t" << fname << ": starts\n");

	 pbm_csr.cfg_bist_row_0.run(0);
	 pbm_csr.cfg_bist_row_0.write();
	 pbm_csr.cfg_bist_row_1.run(0);
	 pbm_csr.cfg_bist_row_1.write();

         PLOG_MSG("\t" << fname << ": ends\n");
}

#define AND_SHIFT(v,w) \
	pass |= (( pbc_csr.sta_bist.v##_done_pass().convert_to<uint32_t>() & ((1<<w)-1) ) << offset); \
	fail |= (( pbc_csr.sta_bist.v##_done_fail().convert_to<uint32_t>() & ((1<<w)-1) ) << offset); \
	offset += w;

void cap_pb_bist_poll(int chip_id, int inst_id) {
	 const char* const fname = "cap_pb_bist_poll";
  	 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
         PLOG_MSG("\t" << fname << ": starts\n");
	 bool done=false;
	 const double total_tries=80000;
	 float tries=total_tries; 
	 uint32_t last_pcnt=0, last_result=0;
	 uint32_t fail=0, pass=0;
	 do {	pbc_csr.sta_bist.read();

		uint32_t offset=0;
	 	AND_SHIFT(linked_list,2);
	 	AND_SHIFT(free_cell,2);
	 	AND_SHIFT(refcount,1);
	 	AND_SHIFT(num_cells,1);
	 	AND_SHIFT(oq_pck,8);
	 	AND_SHIFT(unicast_mgr,1);
	 	AND_SHIFT(oq_rwr,4);
	 	AND_SHIFT(desc,2);
	 	AND_SHIFT(sched,2);

		const uint32_t result = fail | pass ;

		const uint32_t results_mask = (1<<offset)-1;

		const uint32_t pcnt = uint32_t(((total_tries - tries) / total_tries)*100);

		if ( (last_result != result) || (pcnt != last_pcnt) ) {
         		PLOG_MSG("\t" << fname << ": pass=" << hex << pass << " fail= " << fail << dec << " (" << pcnt << "%)" << " mask=" << hex << results_mask << endl);
			last_pcnt= pcnt;
			last_result=result;
		}

		done = (result & results_mask) == results_mask;

	 } while (!done && tries--);

	 if (tries<=0)   { PLOG_ERR("\t" << fname << ": timed out waiting for bist status\n");
	 } else if (fail){ PLOG_ERR("\t" << fname << ": bist failed: " << hex << fail << endl);
	 }

         PLOG_MSG("\t" << fname << ": ends: pass=" << hex << pass << " fail=" << fail << endl);
}

void cap_pm_bist_poll(int chip_id, int inst_id) {
	 const char* const fname = "cap_pm_bist_poll";
  	 cap_pbm_csr_t & pbm_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbm_csr_t, chip_id, inst_id);
         PLOG_MSG("\t" << fname << ": starts\n");
	 bool done=true;
	 const int total_tries=80000;
	 int tries=total_tries; 
	 uint64_t fail[2]={0,0}, pass[2]={0,0};
	 do {	pbm_csr.sta_bist_row_0.read();

		pass[0] = pbm_csr.sta_bist_row_0.done_pass().convert_to<uint64_t>();
		fail[0] = pbm_csr.sta_bist_row_0.done_fail().convert_to<uint64_t>();

		done 	= ((pass[0] | fail[0]) == ((1ULL<<40ULL)-1ULL));

		if (done) {
			pbm_csr.sta_bist_row_1.read();

			pass[1] = pbm_csr.sta_bist_row_1.done_pass().convert_to<uint64_t>();
			fail[1] = pbm_csr.sta_bist_row_1.done_fail().convert_to<uint64_t>();

			done 	= ((pass[1] | fail[1]) == ((1ULL<<40ULL)-1ULL));
		}
		if ((tries % 100)==0) {
			 PLOG_MSG("\t" << fname << ": try " << tries << " : row 0 pass=" << hex << pass[0] << " fail=" << fail[0] << endl);
			 PLOG_MSG("\t" << fname << ": try " << tries << " : row 1 pass=" << hex << pass[1] << " fail=" << fail[1] << endl);
		}


	 } while (!done && tries--);

	 if (tries<=0)   { PLOG_ERR("\t" << fname << ": timed out waiting for bist status\n");
	 } else if (fail[0]){ PLOG_ERR("\t" << fname << ": bist row 0: fail " << hex << fail[0] << endl);
	 } else if (fail[1]){ PLOG_ERR("\t" << fname << ": bist row 1: fail " << hex << fail[1] << endl);
	 }

         PLOG_MSG("\t" << fname << ": ends: row 0 pass=" << hex << pass[0] << " fail=" << fail[0] << endl);
         PLOG_MSG("\t" << fname << ": ends: row 1 pass=" << hex << pass[1] << " fail=" << fail[1] << endl);
}

void cap_pb_bist(int chip_id, int inst_id) {
PLOG_MSG("cap_pb_bist starts\n");

 cpu_access_type_e cur_cpu_access_type = cpu::access()->get_access_type();
 cpu::access()->set_access_type(front_door_e);

 if (true) cap_pb_bist_hbm_rd_wr_launch(chip_id, inst_id);
 if (true) cap_pb_bist_hbm_rb_launch(chip_id, inst_id);
 if (true) cap_pb_bist_hbm_wb_launch(chip_id, inst_id);
 if (true) cap_pb_bist_hbm_ctx_launch(chip_id, inst_id);
 if (true)  cap_pb_bist_launch(chip_id, inst_id);
 if (true) cap_pm_bist_launch(chip_id, inst_id);

 if (true) cap_pb_bist_hbm_rd_wr_poll(chip_id, inst_id);
 if (true) cap_pb_bist_hbm_rb_poll(chip_id, inst_id);
 if (true) cap_pb_bist_hbm_wb_poll(chip_id, inst_id);
 if (true) cap_pb_bist_hbm_ctx_poll(chip_id, inst_id);
 if (true)  cap_pb_bist_poll(chip_id, inst_id);
 if (true) cap_pm_bist_poll(chip_id, inst_id);
		
// restore original access type
 cpu::access()->set_access_type(cur_cpu_access_type);

PLOG_MSG("cap_pb_bist ends\n");

}

bool cap_pb_is_empty(int chip_id, int inst_id) {
PLOG_MSG("inside cap_pb_is_empty \n");

 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
 //cap_pbm_csr_t & pbm_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbm_csr_t, chip_id, inst_id);

   // check occ cntrs
    pbc_csr.port_0.sta_account.read();
    if (pbc_csr.port_0.sta_account.all() != 0) {
       return false;
    }
    pbc_csr.port_1.sta_account.read();
    if (pbc_csr.port_1.sta_account.all() != 0) {
       return false;
    }
    pbc_csr.port_2.sta_account.read();
    if (pbc_csr.port_2.sta_account.all() != 0) {
       return false;
    }
    pbc_csr.port_3.sta_account.read();
    if (pbc_csr.port_3.sta_account.all() != 0) {
       return false;
    }
    pbc_csr.port_4.sta_account.read();
    if (pbc_csr.port_4.sta_account.all() != 0) {
       return false;
    }
    pbc_csr.port_5.sta_account.read();
    if (pbc_csr.port_5.sta_account.all() != 0) {
       return false;
    }
    pbc_csr.port_6.sta_account.read();
    if (pbc_csr.port_6.sta_account.all() != 0) {
       return false;
    }
    pbc_csr.port_7.sta_account.read();
    if (pbc_csr.port_7.sta_account.all() != 0) {
       return false;
    }
    pbc_csr.port_8.sta_account.read();
    if (pbc_csr.port_8.sta_account.all() != 0) {
       return false;
    }
    pbc_csr.port_9.sta_account.read();
    if (pbc_csr.port_9.sta_account.all() != 0) {
       return false;
    }
    pbc_csr.port_10.sta_account.read();
    if (pbc_csr.port_10.sta_account.all() != 0) {
       return false;
    }
    pbc_csr.port_11.sta_account.read();
    if (pbc_csr.port_11.sta_account.all() != 0) {
       return false;
    }
   return true;
}

void cap_pbc_csr_set_hdl_path(int chip_id, int inst_id, string path) {
  cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);

  pbc_csr.set_csr_inst_path(0, (path + ".cap_pbc_csr"));
  pbc_csr.port_10.dhs_oq_flow_control.set_csr_inst_path(1, (path + ".cap_pbc_oq_rwr_top_10.flow_control.credits"));
  pbc_csr.port_11.dhs_oq_flow_control.set_csr_inst_path(1, (path + ".cap_pbc_oq_pck_top_11.flow_control.credits"));
  pbc_csr.port_0.set_csr_inst_path(10, (path + ".pbcport0_csr"));
  pbc_csr.port_1.set_csr_inst_path(11, (path + ".pbcport1_csr"));
  pbc_csr.port_2.set_csr_inst_path(12, (path + ".pbcport2_csr"));
  pbc_csr.port_3.set_csr_inst_path(13, (path + ".pbcport3_csr"));
  pbc_csr.port_4.set_csr_inst_path(14, (path + ".pbcport4_csr"));
  pbc_csr.port_5.set_csr_inst_path(15, (path + ".pbcport5_csr"));
  pbc_csr.port_6.set_csr_inst_path(16, (path + ".pbcport6_csr"));
  pbc_csr.port_7.set_csr_inst_path(17, (path + ".pbcport7_csr"));
  pbc_csr.port_8.set_csr_inst_path(18, (path + ".pbcport8_csr"));
  pbc_csr.port_9.set_csr_inst_path(19, (path + ".pbcport9_csr"));
  pbc_csr.port_10.set_csr_inst_path(20, (path + ".pbcport10_csr"));
  pbc_csr.port_11.set_csr_inst_path(21, (path + ".pbcport11_csr"));
}

void cap_pbm_csr_set_hdl_path(int chip_id, int inst_id, string path) {
  cap_pbm_csr_t & pbm_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbm_csr_t, chip_id, inst_id);
  pbm_csr.set_csr_inst_path(0, (path + ".cap_pbm_csr"));
}

void cap_pf_csr_set_hdl_path(int chip_id, int inst_id, string path) {
  cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
  pbc_csr.hbm.set_csr_inst_path(2, (path + ".cap_pbchbm_csr"));
  pbc_csr.hbm.hbm_port_0.set_csr_inst_path(30, (path + ".pbchbmeth_csr0"));
  pbc_csr.hbm.hbm_port_1.set_csr_inst_path(31, (path + ".pbchbmeth_csr1"));
  pbc_csr.hbm.hbm_port_2.set_csr_inst_path(32, (path + ".pbchbmeth_csr2"));
  pbc_csr.hbm.hbm_port_3.set_csr_inst_path(33, (path + ".pbchbmeth_csr3"));
  pbc_csr.hbm.hbm_port_4.set_csr_inst_path(34, (path + ".pbchbmeth_csr4"));
  pbc_csr.hbm.hbm_port_5.set_csr_inst_path(35, (path + ".pbchbmeth_csr5"));
  pbc_csr.hbm.hbm_port_6.set_csr_inst_path(36, (path + ".pbchbmeth_csr6"));
  pbc_csr.hbm.hbm_port_7.set_csr_inst_path(37, (path + ".pbchbmeth_csr7"));
  pbc_csr.hbm.hbm_port_8.set_csr_inst_path(38, (path + ".pbchbmeth_csr8"));
  pbc_csr.hbm.hbm_port_9.set_csr_inst_path(39, (path + ".pbchbmtx_csr9"));
}

void cap_pb_cfg_sched(int chip_id, int inst_id, int is_random) {

  cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);

     string csr_prefix = pbc_csr.get_hier_path();
     std::replace(csr_prefix.begin(), csr_prefix.end(), '.', '/');
     //Configure slave in fdoor mode & write credits to oq0, oq1 in port0 with 2:1 ratio.
     cpu_access_type_e cur_cpu_access_type = cpu::access()->get_access_type();
     cpu::access()->set_access_type(front_door_e);

     PLOG_MSG("start configuring scheduler \n");

     unsigned int clk_period = 833; //TBD : TODO : get it from define or config???
     unsigned int bus_width = 512; //TBD : TODO : get it from define or config???

     unsigned int l1_sched_dwrr_unit;
     unsigned int l1_node[12][32];
     unsigned int l1_is_strict[12][32];
     unsigned int l1_priority_bypass_timer[12][32];
     unsigned int l1_wt_rate[12][32];
            float l1_dwrr_ratio[12][32];
     unsigned int l1_sp_rate_mbps[12][32];

     unsigned int l1_per_node_tot_wt[12][16];
     unsigned int l1_per_node_tot_dwrr_inputs[12][16];
     unsigned int l2_node[12][16];
     unsigned int l2_is_strict[12][16];
     unsigned int l2_priority_bypass_timer[12][16];
     unsigned int l2_wt_rate[12][16];
            float l2_dwrr_ratio[12][16];
     unsigned int l2_sp_rate_mbps[12][16];

     unsigned int l2_per_node_tot_wt[12][4];
     unsigned int l2_per_node_tot_dwrr_inputs[12][4];

     unsigned int l3_node[12][4];
     unsigned int l3_is_strict[12][4];
     unsigned int l3_priority_bypass_timer[12][4];
     unsigned int l3_wt_rate[12][4];
            float l3_dwrr_ratio[12][4];
     unsigned int l3_sp_rate_mbps[12][4];

     unsigned int l3_per_node_tot_wt[12][1];
     unsigned int l3_per_node_tot_dwrr_inputs[12][1];
     int cfg_quota;

     l1_sched_dwrr_unit  = is_random ? PKnobsReader::evalKnob(csr_prefix+"/sched_rand_32") : PKnobsReader::evalKnob(csr_prefix+"/l1_sched_dwrr_unit");

     unsigned long long sched_timer  =  PKnobsReader::evalKnob(csr_prefix+"/sched_timer");
     pbc_csr.cfg_sched.timer(sched_timer);
     pbc_csr.cfg_sched.write();
  
     //Configuration for L1 scheduler
     for(int p_idx=0; p_idx<12; p_idx++) {
       for(int node_idx=0; node_idx<16; node_idx++) {
          l1_per_node_tot_wt[p_idx][node_idx] = 0;
          l1_per_node_tot_dwrr_inputs[p_idx][node_idx] = 0;
       }
       for(int q_idx=0; q_idx<32; q_idx++) {
         if(is_random) {
            l1_node[p_idx][q_idx]                    = PKnobsReader::evalKnob(csr_prefix+"/sched_rand_32") % 16;
            l1_is_strict[p_idx][q_idx]               = PKnobsReader::evalKnob(csr_prefix+"/sched_rand_32") % 2;
            l1_priority_bypass_timer[p_idx][q_idx]   = PKnobsReader::evalKnob(csr_prefix+"/sched_rand_32") % 2;
            l1_wt_rate[p_idx][q_idx]                 = PKnobsReader::evalKnob(csr_prefix+"/sched_rand_32") % 100;
            l1_sp_rate_mbps[p_idx][q_idx]            = PKnobsReader::evalKnob(csr_prefix+"/sched_rand_32") % 100000;
         } else {
            l1_node[p_idx][q_idx]          = PKnobsReader::evalKnob(csr_prefix+"/oport/" + to_string(p_idx) + "/l1_sched/" + to_string(q_idx) + "/node");
            l1_is_strict[p_idx][q_idx]     = PKnobsReader::evalKnob(csr_prefix+"/oport/" + to_string(p_idx) + "/l1_sched/" + to_string(q_idx) + "/is_strict");
            l1_priority_bypass_timer[p_idx][q_idx]   = PKnobsReader::evalKnob(csr_prefix+"/oport/" + to_string(p_idx) + "/l1_sched/" + to_string(q_idx) + "/priority_bypass_timer");
            l1_wt_rate[p_idx][q_idx]       = PKnobsReader::evalKnob(csr_prefix+"/oport/" + to_string(p_idx) + "/l1_sched/" + to_string(q_idx) + "/weight_rate");
            l1_sp_rate_mbps[p_idx][q_idx]  = PKnobsReader::evalKnob(csr_prefix+"/oport/" + to_string(p_idx) + "/l1_sched/" + to_string(q_idx) + "/sp_rate_mbps");
         }
         if(l1_is_strict[p_idx][q_idx] == 0) {
           l1_per_node_tot_wt[p_idx][l1_node[p_idx][q_idx]] += l1_wt_rate[p_idx][q_idx];
           l1_per_node_tot_dwrr_inputs[p_idx][l1_node[p_idx][q_idx]] += (l1_wt_rate[p_idx][q_idx] == 0) ? 0 : 1;
         }
       }

       for(int q_idx=0; q_idx<32; q_idx++) {
         if(l1_is_strict[p_idx][q_idx] == 0 && l1_wt_rate[p_idx][q_idx] != 0) {
           l1_dwrr_ratio[p_idx][q_idx] = (l1_wt_rate[p_idx][q_idx] * 100) / (l1_per_node_tot_wt[p_idx][l1_node[p_idx][q_idx]]);
         } else {
           l1_dwrr_ratio[p_idx][q_idx] = 0;
         }

         if(l1_is_strict[p_idx][q_idx] == 0) {
           l1_sp_rate_mbps[p_idx][q_idx] = 0;
         }

         if(is_random == 0) {
            SKNOBS_SET((char *) (csr_prefix+"/oport/" + to_string(p_idx) + "/l1_sched/" + to_string(q_idx) + "/cfg_node").c_str(), l1_node[p_idx][q_idx]);
            SKNOBS_SET((char *) (csr_prefix+"/oport/" + to_string(p_idx) + "/l1_sched/" + to_string(q_idx) + "/cfg_is_strict").c_str(), l1_is_strict[p_idx][q_idx]);
            SKNOBS_SET((char *) (csr_prefix+"/oport/" + to_string(p_idx) + "/l1_sched/" + to_string(q_idx) + "/cfg_priority_bypass_timer").c_str(), l1_priority_bypass_timer[p_idx][q_idx]);
            SKNOBS_SET((char *) (csr_prefix+"/oport/" + to_string(p_idx) + "/l1_sched/" + to_string(q_idx) + "/cfg_dwrr_ratio").c_str(), l1_dwrr_ratio[p_idx][q_idx]);
            SKNOBS_SET((char *) (csr_prefix+"/oport/" + to_string(p_idx) + "/l1_sched/" + to_string(q_idx) + "/cfg_sp_rate").c_str(), l1_sp_rate_mbps[p_idx][q_idx]);
         }

         if(l1_dwrr_ratio[p_idx][q_idx] != 0) {
           cfg_quota = (l1_dwrr_ratio[p_idx][q_idx] * l1_sched_dwrr_unit * l1_per_node_tot_dwrr_inputs[p_idx][l1_node[p_idx][q_idx]]);
           cap_pb_cfg_sched_node(chip_id, inst_id, 1, p_idx, q_idx, cfg_quota);
           if(is_random == 0) {  SKNOBS_SET((char *) (csr_prefix+"/oport/" + to_string(p_idx) + "/l1_sched/" + to_string(q_idx) + "/cfg_quota").c_str(), cfg_quota);  }
         } else if(l1_is_strict[p_idx][q_idx] != 0) {
           if(l1_priority_bypass_timer[p_idx][q_idx] != 0) {
             cfg_quota = 10 + ( PKnobsReader::evalKnob(csr_prefix+"/sched_timer") % 1000 );
           } else {
             cfg_quota = ((l1_sp_rate_mbps[p_idx][q_idx] * (unsigned long long) 64 * sched_timer) / (clk_period*bus_width));
           }

           cap_pb_cfg_sched_node(chip_id, inst_id, 1, p_idx, q_idx, cfg_quota );
           if(is_random == 0) { SKNOBS_SET((char *) (csr_prefix+"/oport/" + to_string(p_idx) + "/l1_sched/" + to_string(q_idx) + "/cfg_quota").c_str(), cfg_quota); }
         }
       }
     } //for port

     //Configuration for L2 scheduler
     for(int p_idx=0; p_idx<12; p_idx++) {
       for(int node_idx=0; node_idx<4; node_idx++) {
          l2_per_node_tot_wt[p_idx][node_idx] = 0;
          l2_per_node_tot_dwrr_inputs[p_idx][node_idx] = 0;
       }
       for(int node_idx=0; node_idx<16; node_idx++) {
         if(is_random) {
            l2_node[p_idx][node_idx]                    = PKnobsReader::evalKnob(csr_prefix+"/sched_rand_32") % 4;
            l2_is_strict[p_idx][node_idx]               = PKnobsReader::evalKnob(csr_prefix+"/sched_rand_32") % 2;
            l2_priority_bypass_timer[p_idx][node_idx]   = PKnobsReader::evalKnob(csr_prefix+"/sched_rand_32") % 2;
            l2_wt_rate[p_idx][node_idx]                 = PKnobsReader::evalKnob(csr_prefix+"/sched_rand_32") % 100;
            l2_sp_rate_mbps[p_idx][node_idx]            = PKnobsReader::evalKnob(csr_prefix+"/sched_rand_32") % 100000;
         } else {
            l2_node[p_idx][node_idx]         = PKnobsReader::evalKnob(csr_prefix+"/oport/" + to_string(p_idx) + "/l2_sched/" + to_string(node_idx) + "/node");
            l2_is_strict[p_idx][node_idx]    = PKnobsReader::evalKnob(csr_prefix+"/oport/" + to_string(p_idx) + "/l2_sched/" + to_string(node_idx) + "/is_strict");
            l2_priority_bypass_timer[p_idx][node_idx]   = PKnobsReader::evalKnob(csr_prefix+"/oport/" + to_string(p_idx) + "/l2_sched/" + to_string(node_idx) + "/priority_bypass_timer");
            l2_wt_rate[p_idx][node_idx]      = PKnobsReader::evalKnob(csr_prefix+"/oport/" + to_string(p_idx) + "/l2_sched/" + to_string(node_idx) + "/weight_rate");
            l2_sp_rate_mbps[p_idx][node_idx] = PKnobsReader::evalKnob(csr_prefix+"/oport/" + to_string(p_idx) + "/l2_sched/" + to_string(node_idx) + "/sp_rate_mbps");
         }
         if(l2_is_strict[p_idx][node_idx] == 0) {
           l2_per_node_tot_wt[p_idx][l2_node[p_idx][node_idx]] += l2_wt_rate[p_idx][node_idx];
           l2_per_node_tot_dwrr_inputs[p_idx][l2_node[p_idx][node_idx]] += (l2_wt_rate[p_idx][node_idx] == 0) ? 0 : l1_per_node_tot_dwrr_inputs[p_idx][node_idx];
         }
       }

       for(int node_idx=0; node_idx<16; node_idx++) {
         if(l2_is_strict[p_idx][node_idx] == 0 && l2_wt_rate[p_idx][node_idx] != 0) {
           l2_dwrr_ratio[p_idx][node_idx] = (l2_wt_rate[p_idx][node_idx] * 100) / (l2_per_node_tot_wt[p_idx][l2_node[p_idx][node_idx]]);
         } else {
           l2_dwrr_ratio[p_idx][node_idx] = 0;
         }

         if(l2_is_strict[p_idx][node_idx] == 0) {
           l2_sp_rate_mbps[p_idx][node_idx] = 0;
         }

         if(is_random == 0) {
            SKNOBS_SET((char *) (csr_prefix+"/oport/" + to_string(p_idx) + "/l2_sched/" + to_string(node_idx) + "/cfg_node").c_str(), l2_node[p_idx][node_idx]);
            SKNOBS_SET((char *) (csr_prefix+"/oport/" + to_string(p_idx) + "/l2_sched/" + to_string(node_idx) + "/cfg_is_strict").c_str(), l2_is_strict[p_idx][node_idx]);
            SKNOBS_SET((char *) (csr_prefix+"/oport/" + to_string(p_idx) + "/l2_sched/" + to_string(node_idx) + "/cfg_priority_bypass_timer").c_str(), l2_priority_bypass_timer[p_idx][node_idx]);
            SKNOBS_SET((char *) (csr_prefix+"/oport/" + to_string(p_idx) + "/l2_sched/" + to_string(node_idx) + "/cfg_dwrr_ratio").c_str(), l2_dwrr_ratio[p_idx][node_idx]);
            SKNOBS_SET((char *) (csr_prefix+"/oport/" + to_string(p_idx) + "/l2_sched/" + to_string(node_idx) + "/cfg_sp_rate").c_str(), l2_sp_rate_mbps[p_idx][node_idx]);
         }

         if(l2_dwrr_ratio[p_idx][node_idx] != 0) {
           cfg_quota  = (l2_dwrr_ratio[p_idx][node_idx] * l1_sched_dwrr_unit * l2_per_node_tot_dwrr_inputs[p_idx][l2_node[p_idx][node_idx]]);
           cap_pb_cfg_sched_node(chip_id, inst_id, 2, p_idx, node_idx, cfg_quota);
           if(is_random == 0) {  SKNOBS_SET((char *) (csr_prefix+"/oport/" + to_string(p_idx) + "/l2_sched/" + to_string(node_idx) + "/cfg_quota").c_str(), cfg_quota); }
         } else if(l2_is_strict[p_idx][node_idx] != 0) {
           if(l2_priority_bypass_timer[p_idx][node_idx] != 0) {
             cfg_quota = 10 + ( PKnobsReader::evalKnob(csr_prefix+"/sched_timer") % 1000 );
           } else {
             cfg_quota = ((l2_sp_rate_mbps[p_idx][node_idx] * (unsigned long long) 64 * sched_timer) / (clk_period*bus_width));
           }
           cap_pb_cfg_sched_node(chip_id, inst_id, 2, p_idx, node_idx, cfg_quota);
           if(is_random == 0) { SKNOBS_SET((char *) (csr_prefix+"/oport/" + to_string(p_idx) + "/l2_sched/" + to_string(node_idx) + "/cfg_quota").c_str(), cfg_quota); }
         }
       }
     } //for port

     //Configuration for L3 scheduler
     for(int p_idx=9; p_idx<12; p_idx++) {
       for(int node_idx=0; node_idx<1; node_idx++) {
          l3_per_node_tot_wt[p_idx][node_idx] = 0;
          l3_per_node_tot_dwrr_inputs[p_idx][node_idx] = 0;
       }
       for(int node_idx=0; node_idx<4; node_idx++) {
         if(is_random) {
            l3_node[p_idx][node_idx]                    = 0;
            l3_is_strict[p_idx][node_idx]               = PKnobsReader::evalKnob(csr_prefix+"/sched_rand_32") % 2;
            l3_priority_bypass_timer[p_idx][node_idx]   = PKnobsReader::evalKnob(csr_prefix+"/sched_rand_32") % 2;
            l3_wt_rate[p_idx][node_idx]                 = PKnobsReader::evalKnob(csr_prefix+"/sched_rand_32") % 100;
            l3_sp_rate_mbps[p_idx][node_idx]            = PKnobsReader::evalKnob(csr_prefix+"/sched_rand_32") % 100000;
         } else {
            l3_node[p_idx][node_idx]          = PKnobsReader::evalKnob(csr_prefix+"/oport/" + to_string(p_idx) + "/l3_sched/" + to_string(node_idx) + "/node");
            l3_is_strict[p_idx][node_idx]     = PKnobsReader::evalKnob(csr_prefix+"/oport/" + to_string(p_idx) + "/l3_sched/" + to_string(node_idx) + "/is_strict");
            l3_priority_bypass_timer[p_idx][node_idx]   = PKnobsReader::evalKnob(csr_prefix+"/oport/" + to_string(p_idx) + "/l3_sched/" + to_string(node_idx) + "/priority_bypass_timer");
            l3_wt_rate[p_idx][node_idx]       = PKnobsReader::evalKnob(csr_prefix+"/oport/" + to_string(p_idx) + "/l3_sched/" + to_string(node_idx) + "/weight_rate");
            l3_sp_rate_mbps[p_idx][node_idx]  = PKnobsReader::evalKnob(csr_prefix+"/oport/" + to_string(p_idx) + "/l3_sched/" + to_string(node_idx) + "/sp_rate_mbps");
         }

         if(l3_is_strict[p_idx][node_idx] == 0) {
           l3_per_node_tot_wt[p_idx][l3_node[p_idx][node_idx]] += l3_wt_rate[p_idx][node_idx];
           l3_per_node_tot_dwrr_inputs[p_idx][l3_node[p_idx][node_idx]] += (l3_wt_rate[p_idx][node_idx] == 0) ? 0 : l2_per_node_tot_dwrr_inputs[p_idx][node_idx];
         }
       }

       for(int node_idx=0; node_idx<4; node_idx++) {
         if(l3_is_strict[p_idx][node_idx] == 0 && l3_wt_rate[p_idx][node_idx] != 0) {
           l3_dwrr_ratio[p_idx][node_idx] = (l3_wt_rate[p_idx][node_idx] * 100) / (l3_per_node_tot_wt[p_idx][l3_node[p_idx][node_idx]]);
         } else {
           l3_dwrr_ratio[p_idx][node_idx] = 0;
         }

         if(l3_is_strict[p_idx][node_idx] == 0) {
           l3_sp_rate_mbps[p_idx][node_idx] = 0;
         }

         if(is_random == 0) {
            SKNOBS_SET((char *) (csr_prefix+"/oport/" + to_string(p_idx) + "/l3_sched/" + to_string(node_idx) + "/cfg_node").c_str(), l3_node[p_idx][node_idx]);
            SKNOBS_SET((char *) (csr_prefix+"/oport/" + to_string(p_idx) + "/l3_sched/" + to_string(node_idx) + "/cfg_is_strict").c_str(), l3_is_strict[p_idx][node_idx]);
            SKNOBS_SET((char *) (csr_prefix+"/oport/" + to_string(p_idx) + "/l3_sched/" + to_string(node_idx) + "/cfg_priority_bypass_timer").c_str(), l3_priority_bypass_timer[p_idx][node_idx]);
            SKNOBS_SET((char *) (csr_prefix+"/oport/" + to_string(p_idx) + "/l3_sched/" + to_string(node_idx) + "/cfg_dwrr_ratio").c_str(), l3_dwrr_ratio[p_idx][node_idx]);
            SKNOBS_SET((char *) (csr_prefix+"/oport/" + to_string(p_idx) + "/l3_sched/" + to_string(node_idx) + "/cfg_sp_rate").c_str(), l3_sp_rate_mbps[p_idx][node_idx]);
         }

         if(l3_dwrr_ratio[p_idx][node_idx] != 0) {
           cfg_quota  = (l3_dwrr_ratio[p_idx][node_idx] * l1_sched_dwrr_unit * l3_per_node_tot_dwrr_inputs[p_idx][l3_node[p_idx][node_idx]]);
           cap_pb_cfg_sched_node(chip_id, inst_id, 3, p_idx, node_idx, cfg_quota);
           if(is_random == 0) { SKNOBS_SET((char *) (csr_prefix+"/oport/" + to_string(p_idx) + "/l3_sched/" + to_string(node_idx) + "/cfg_quota").c_str(), cfg_quota); }
         } else if(l3_is_strict[p_idx][node_idx] != 0) {
           if(l3_priority_bypass_timer[p_idx][node_idx] != 0) {
             cfg_quota = 10 + ( PKnobsReader::evalKnob(csr_prefix+"/sched_timer") % 1000 );
           } else {
             cfg_quota = ((l3_sp_rate_mbps[p_idx][node_idx] * 64 * sched_timer) / (clk_period*bus_width));
           }
           cap_pb_cfg_sched_node(chip_id, inst_id, 3, p_idx, node_idx, cfg_quota);
           if(is_random == 0) { SKNOBS_SET((char *) (csr_prefix+"/oport/" + to_string(p_idx) + "/l3_sched/" + to_string(node_idx) + "/cfg_quota").c_str(), cfg_quota); }
         }
       }
     } //for port


    pbc_csr.port_0.cfg_oq_arb_l1_selection.all(0);

    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[0][q_idx] == 0) {
        pbc_csr.port_0.cfg_oq_arb_l1_selection.node_0(pbc_csr.port_0.cfg_oq_arb_l1_selection.node_0().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[0][q_idx] == 1) {
        pbc_csr.port_0.cfg_oq_arb_l1_selection.node_1(pbc_csr.port_0.cfg_oq_arb_l1_selection.node_1().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[0][q_idx] == 2) {
        pbc_csr.port_0.cfg_oq_arb_l1_selection.node_2(pbc_csr.port_0.cfg_oq_arb_l1_selection.node_2().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[0][q_idx] == 3) {
        pbc_csr.port_0.cfg_oq_arb_l1_selection.node_3(pbc_csr.port_0.cfg_oq_arb_l1_selection.node_3().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[0][q_idx] == 4) {
        pbc_csr.port_0.cfg_oq_arb_l1_selection.node_4(pbc_csr.port_0.cfg_oq_arb_l1_selection.node_4().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[0][q_idx] == 5) {
        pbc_csr.port_0.cfg_oq_arb_l1_selection.node_5(pbc_csr.port_0.cfg_oq_arb_l1_selection.node_5().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[0][q_idx] == 6) {
        pbc_csr.port_0.cfg_oq_arb_l1_selection.node_6(pbc_csr.port_0.cfg_oq_arb_l1_selection.node_6().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[0][q_idx] == 7) {
        pbc_csr.port_0.cfg_oq_arb_l1_selection.node_7(pbc_csr.port_0.cfg_oq_arb_l1_selection.node_7().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[0][q_idx] == 8) {
        pbc_csr.port_0.cfg_oq_arb_l1_selection.node_8(pbc_csr.port_0.cfg_oq_arb_l1_selection.node_8().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[0][q_idx] == 9) {
        pbc_csr.port_0.cfg_oq_arb_l1_selection.node_9(pbc_csr.port_0.cfg_oq_arb_l1_selection.node_9().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[0][q_idx] == 10) {
        pbc_csr.port_0.cfg_oq_arb_l1_selection.node_10(pbc_csr.port_0.cfg_oq_arb_l1_selection.node_10().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[0][q_idx] == 11) {
        pbc_csr.port_0.cfg_oq_arb_l1_selection.node_11(pbc_csr.port_0.cfg_oq_arb_l1_selection.node_11().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[0][q_idx] == 12) {
        pbc_csr.port_0.cfg_oq_arb_l1_selection.node_12(pbc_csr.port_0.cfg_oq_arb_l1_selection.node_12().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[0][q_idx] == 13) {
        pbc_csr.port_0.cfg_oq_arb_l1_selection.node_13(pbc_csr.port_0.cfg_oq_arb_l1_selection.node_13().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[0][q_idx] == 14) {
        pbc_csr.port_0.cfg_oq_arb_l1_selection.node_14(pbc_csr.port_0.cfg_oq_arb_l1_selection.node_14().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[0][q_idx] == 15) {
        pbc_csr.port_0.cfg_oq_arb_l1_selection.node_15(pbc_csr.port_0.cfg_oq_arb_l1_selection.node_15().convert_to<int>() | 1 << q_idx);
      }
    }
    pbc_csr.port_0.cfg_oq_arb_l1_strict.all(0);
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_is_strict[0][q_idx] == 1) {
        pbc_csr.port_0.cfg_oq_arb_l1_strict.priority(pbc_csr.port_0.cfg_oq_arb_l1_strict.priority().convert_to<int>() | 1 << q_idx);
      }
    }

    pbc_csr.port_0.cfg_oq_arb_l1_selection.write();
    pbc_csr.port_0.cfg_oq_arb_l1_strict.write();

    pbc_csr.port_1.cfg_oq_arb_l1_selection.all(0);

    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[1][q_idx] == 0) {
        pbc_csr.port_1.cfg_oq_arb_l1_selection.node_0(pbc_csr.port_1.cfg_oq_arb_l1_selection.node_0().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[1][q_idx] == 1) {
        pbc_csr.port_1.cfg_oq_arb_l1_selection.node_1(pbc_csr.port_1.cfg_oq_arb_l1_selection.node_1().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[1][q_idx] == 2) {
        pbc_csr.port_1.cfg_oq_arb_l1_selection.node_2(pbc_csr.port_1.cfg_oq_arb_l1_selection.node_2().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[1][q_idx] == 3) {
        pbc_csr.port_1.cfg_oq_arb_l1_selection.node_3(pbc_csr.port_1.cfg_oq_arb_l1_selection.node_3().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[1][q_idx] == 4) {
        pbc_csr.port_1.cfg_oq_arb_l1_selection.node_4(pbc_csr.port_1.cfg_oq_arb_l1_selection.node_4().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[1][q_idx] == 5) {
        pbc_csr.port_1.cfg_oq_arb_l1_selection.node_5(pbc_csr.port_1.cfg_oq_arb_l1_selection.node_5().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[1][q_idx] == 6) {
        pbc_csr.port_1.cfg_oq_arb_l1_selection.node_6(pbc_csr.port_1.cfg_oq_arb_l1_selection.node_6().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[1][q_idx] == 7) {
        pbc_csr.port_1.cfg_oq_arb_l1_selection.node_7(pbc_csr.port_1.cfg_oq_arb_l1_selection.node_7().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[1][q_idx] == 8) {
        pbc_csr.port_1.cfg_oq_arb_l1_selection.node_8(pbc_csr.port_1.cfg_oq_arb_l1_selection.node_8().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[1][q_idx] == 9) {
        pbc_csr.port_1.cfg_oq_arb_l1_selection.node_9(pbc_csr.port_1.cfg_oq_arb_l1_selection.node_9().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[1][q_idx] == 10) {
        pbc_csr.port_1.cfg_oq_arb_l1_selection.node_10(pbc_csr.port_1.cfg_oq_arb_l1_selection.node_10().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[1][q_idx] == 11) {
        pbc_csr.port_1.cfg_oq_arb_l1_selection.node_11(pbc_csr.port_1.cfg_oq_arb_l1_selection.node_11().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[1][q_idx] == 12) {
        pbc_csr.port_1.cfg_oq_arb_l1_selection.node_12(pbc_csr.port_1.cfg_oq_arb_l1_selection.node_12().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[1][q_idx] == 13) {
        pbc_csr.port_1.cfg_oq_arb_l1_selection.node_13(pbc_csr.port_1.cfg_oq_arb_l1_selection.node_13().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[1][q_idx] == 14) {
        pbc_csr.port_1.cfg_oq_arb_l1_selection.node_14(pbc_csr.port_1.cfg_oq_arb_l1_selection.node_14().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[1][q_idx] == 15) {
        pbc_csr.port_1.cfg_oq_arb_l1_selection.node_15(pbc_csr.port_1.cfg_oq_arb_l1_selection.node_15().convert_to<int>() | 1 << q_idx);
      }
    }
    pbc_csr.port_1.cfg_oq_arb_l1_strict.all(0);
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_is_strict[1][q_idx] == 1) {
        pbc_csr.port_1.cfg_oq_arb_l1_strict.priority(pbc_csr.port_1.cfg_oq_arb_l1_strict.priority().convert_to<int>() | 1 << q_idx);
      }
    }

    pbc_csr.port_1.cfg_oq_arb_l1_selection.write();
    pbc_csr.port_1.cfg_oq_arb_l1_strict.write();

    pbc_csr.port_2.cfg_oq_arb_l1_selection.all(0);

    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[2][q_idx] == 0) {
        pbc_csr.port_2.cfg_oq_arb_l1_selection.node_0(pbc_csr.port_2.cfg_oq_arb_l1_selection.node_0().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[2][q_idx] == 1) {
        pbc_csr.port_2.cfg_oq_arb_l1_selection.node_1(pbc_csr.port_2.cfg_oq_arb_l1_selection.node_1().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[2][q_idx] == 2) {
        pbc_csr.port_2.cfg_oq_arb_l1_selection.node_2(pbc_csr.port_2.cfg_oq_arb_l1_selection.node_2().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[2][q_idx] == 3) {
        pbc_csr.port_2.cfg_oq_arb_l1_selection.node_3(pbc_csr.port_2.cfg_oq_arb_l1_selection.node_3().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[2][q_idx] == 4) {
        pbc_csr.port_2.cfg_oq_arb_l1_selection.node_4(pbc_csr.port_2.cfg_oq_arb_l1_selection.node_4().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[2][q_idx] == 5) {
        pbc_csr.port_2.cfg_oq_arb_l1_selection.node_5(pbc_csr.port_2.cfg_oq_arb_l1_selection.node_5().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[2][q_idx] == 6) {
        pbc_csr.port_2.cfg_oq_arb_l1_selection.node_6(pbc_csr.port_2.cfg_oq_arb_l1_selection.node_6().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[2][q_idx] == 7) {
        pbc_csr.port_2.cfg_oq_arb_l1_selection.node_7(pbc_csr.port_2.cfg_oq_arb_l1_selection.node_7().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[2][q_idx] == 8) {
        pbc_csr.port_2.cfg_oq_arb_l1_selection.node_8(pbc_csr.port_2.cfg_oq_arb_l1_selection.node_8().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[2][q_idx] == 9) {
        pbc_csr.port_2.cfg_oq_arb_l1_selection.node_9(pbc_csr.port_2.cfg_oq_arb_l1_selection.node_9().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[2][q_idx] == 10) {
        pbc_csr.port_2.cfg_oq_arb_l1_selection.node_10(pbc_csr.port_2.cfg_oq_arb_l1_selection.node_10().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[2][q_idx] == 11) {
        pbc_csr.port_2.cfg_oq_arb_l1_selection.node_11(pbc_csr.port_2.cfg_oq_arb_l1_selection.node_11().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[2][q_idx] == 12) {
        pbc_csr.port_2.cfg_oq_arb_l1_selection.node_12(pbc_csr.port_2.cfg_oq_arb_l1_selection.node_12().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[2][q_idx] == 13) {
        pbc_csr.port_2.cfg_oq_arb_l1_selection.node_13(pbc_csr.port_2.cfg_oq_arb_l1_selection.node_13().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[2][q_idx] == 14) {
        pbc_csr.port_2.cfg_oq_arb_l1_selection.node_14(pbc_csr.port_2.cfg_oq_arb_l1_selection.node_14().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[2][q_idx] == 15) {
        pbc_csr.port_2.cfg_oq_arb_l1_selection.node_15(pbc_csr.port_2.cfg_oq_arb_l1_selection.node_15().convert_to<int>() | 1 << q_idx);
      }
    }
    pbc_csr.port_2.cfg_oq_arb_l1_strict.all(0);
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_is_strict[2][q_idx] == 1) {
        pbc_csr.port_2.cfg_oq_arb_l1_strict.priority(pbc_csr.port_2.cfg_oq_arb_l1_strict.priority().convert_to<int>() | 1 << q_idx);
      }
    }

    pbc_csr.port_2.cfg_oq_arb_l1_selection.write();
    pbc_csr.port_2.cfg_oq_arb_l1_strict.write();

    pbc_csr.port_3.cfg_oq_arb_l1_selection.all(0);

    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[3][q_idx] == 0) {
        pbc_csr.port_3.cfg_oq_arb_l1_selection.node_0(pbc_csr.port_3.cfg_oq_arb_l1_selection.node_0().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[3][q_idx] == 1) {
        pbc_csr.port_3.cfg_oq_arb_l1_selection.node_1(pbc_csr.port_3.cfg_oq_arb_l1_selection.node_1().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[3][q_idx] == 2) {
        pbc_csr.port_3.cfg_oq_arb_l1_selection.node_2(pbc_csr.port_3.cfg_oq_arb_l1_selection.node_2().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[3][q_idx] == 3) {
        pbc_csr.port_3.cfg_oq_arb_l1_selection.node_3(pbc_csr.port_3.cfg_oq_arb_l1_selection.node_3().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[3][q_idx] == 4) {
        pbc_csr.port_3.cfg_oq_arb_l1_selection.node_4(pbc_csr.port_3.cfg_oq_arb_l1_selection.node_4().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[3][q_idx] == 5) {
        pbc_csr.port_3.cfg_oq_arb_l1_selection.node_5(pbc_csr.port_3.cfg_oq_arb_l1_selection.node_5().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[3][q_idx] == 6) {
        pbc_csr.port_3.cfg_oq_arb_l1_selection.node_6(pbc_csr.port_3.cfg_oq_arb_l1_selection.node_6().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[3][q_idx] == 7) {
        pbc_csr.port_3.cfg_oq_arb_l1_selection.node_7(pbc_csr.port_3.cfg_oq_arb_l1_selection.node_7().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[3][q_idx] == 8) {
        pbc_csr.port_3.cfg_oq_arb_l1_selection.node_8(pbc_csr.port_3.cfg_oq_arb_l1_selection.node_8().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[3][q_idx] == 9) {
        pbc_csr.port_3.cfg_oq_arb_l1_selection.node_9(pbc_csr.port_3.cfg_oq_arb_l1_selection.node_9().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[3][q_idx] == 10) {
        pbc_csr.port_3.cfg_oq_arb_l1_selection.node_10(pbc_csr.port_3.cfg_oq_arb_l1_selection.node_10().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[3][q_idx] == 11) {
        pbc_csr.port_3.cfg_oq_arb_l1_selection.node_11(pbc_csr.port_3.cfg_oq_arb_l1_selection.node_11().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[3][q_idx] == 12) {
        pbc_csr.port_3.cfg_oq_arb_l1_selection.node_12(pbc_csr.port_3.cfg_oq_arb_l1_selection.node_12().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[3][q_idx] == 13) {
        pbc_csr.port_3.cfg_oq_arb_l1_selection.node_13(pbc_csr.port_3.cfg_oq_arb_l1_selection.node_13().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[3][q_idx] == 14) {
        pbc_csr.port_3.cfg_oq_arb_l1_selection.node_14(pbc_csr.port_3.cfg_oq_arb_l1_selection.node_14().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[3][q_idx] == 15) {
        pbc_csr.port_3.cfg_oq_arb_l1_selection.node_15(pbc_csr.port_3.cfg_oq_arb_l1_selection.node_15().convert_to<int>() | 1 << q_idx);
      }
    }
    pbc_csr.port_3.cfg_oq_arb_l1_strict.all(0);
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_is_strict[3][q_idx] == 1) {
        pbc_csr.port_3.cfg_oq_arb_l1_strict.priority(pbc_csr.port_3.cfg_oq_arb_l1_strict.priority().convert_to<int>() | 1 << q_idx);
      }
    }

    pbc_csr.port_3.cfg_oq_arb_l1_selection.write();
    pbc_csr.port_3.cfg_oq_arb_l1_strict.write();

    pbc_csr.port_4.cfg_oq_arb_l1_selection.all(0);

    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[4][q_idx] == 0) {
        pbc_csr.port_4.cfg_oq_arb_l1_selection.node_0(pbc_csr.port_4.cfg_oq_arb_l1_selection.node_0().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[4][q_idx] == 1) {
        pbc_csr.port_4.cfg_oq_arb_l1_selection.node_1(pbc_csr.port_4.cfg_oq_arb_l1_selection.node_1().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[4][q_idx] == 2) {
        pbc_csr.port_4.cfg_oq_arb_l1_selection.node_2(pbc_csr.port_4.cfg_oq_arb_l1_selection.node_2().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[4][q_idx] == 3) {
        pbc_csr.port_4.cfg_oq_arb_l1_selection.node_3(pbc_csr.port_4.cfg_oq_arb_l1_selection.node_3().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[4][q_idx] == 4) {
        pbc_csr.port_4.cfg_oq_arb_l1_selection.node_4(pbc_csr.port_4.cfg_oq_arb_l1_selection.node_4().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[4][q_idx] == 5) {
        pbc_csr.port_4.cfg_oq_arb_l1_selection.node_5(pbc_csr.port_4.cfg_oq_arb_l1_selection.node_5().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[4][q_idx] == 6) {
        pbc_csr.port_4.cfg_oq_arb_l1_selection.node_6(pbc_csr.port_4.cfg_oq_arb_l1_selection.node_6().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[4][q_idx] == 7) {
        pbc_csr.port_4.cfg_oq_arb_l1_selection.node_7(pbc_csr.port_4.cfg_oq_arb_l1_selection.node_7().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[4][q_idx] == 8) {
        pbc_csr.port_4.cfg_oq_arb_l1_selection.node_8(pbc_csr.port_4.cfg_oq_arb_l1_selection.node_8().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[4][q_idx] == 9) {
        pbc_csr.port_4.cfg_oq_arb_l1_selection.node_9(pbc_csr.port_4.cfg_oq_arb_l1_selection.node_9().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[4][q_idx] == 10) {
        pbc_csr.port_4.cfg_oq_arb_l1_selection.node_10(pbc_csr.port_4.cfg_oq_arb_l1_selection.node_10().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[4][q_idx] == 11) {
        pbc_csr.port_4.cfg_oq_arb_l1_selection.node_11(pbc_csr.port_4.cfg_oq_arb_l1_selection.node_11().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[4][q_idx] == 12) {
        pbc_csr.port_4.cfg_oq_arb_l1_selection.node_12(pbc_csr.port_4.cfg_oq_arb_l1_selection.node_12().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[4][q_idx] == 13) {
        pbc_csr.port_4.cfg_oq_arb_l1_selection.node_13(pbc_csr.port_4.cfg_oq_arb_l1_selection.node_13().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[4][q_idx] == 14) {
        pbc_csr.port_4.cfg_oq_arb_l1_selection.node_14(pbc_csr.port_4.cfg_oq_arb_l1_selection.node_14().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[4][q_idx] == 15) {
        pbc_csr.port_4.cfg_oq_arb_l1_selection.node_15(pbc_csr.port_4.cfg_oq_arb_l1_selection.node_15().convert_to<int>() | 1 << q_idx);
      }
    }
    pbc_csr.port_4.cfg_oq_arb_l1_strict.all(0);
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_is_strict[4][q_idx] == 1) {
        pbc_csr.port_4.cfg_oq_arb_l1_strict.priority(pbc_csr.port_4.cfg_oq_arb_l1_strict.priority().convert_to<int>() | 1 << q_idx);
      }
    }

    pbc_csr.port_4.cfg_oq_arb_l1_selection.write();
    pbc_csr.port_4.cfg_oq_arb_l1_strict.write();

    pbc_csr.port_5.cfg_oq_arb_l1_selection.all(0);

    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[5][q_idx] == 0) {
        pbc_csr.port_5.cfg_oq_arb_l1_selection.node_0(pbc_csr.port_5.cfg_oq_arb_l1_selection.node_0().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[5][q_idx] == 1) {
        pbc_csr.port_5.cfg_oq_arb_l1_selection.node_1(pbc_csr.port_5.cfg_oq_arb_l1_selection.node_1().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[5][q_idx] == 2) {
        pbc_csr.port_5.cfg_oq_arb_l1_selection.node_2(pbc_csr.port_5.cfg_oq_arb_l1_selection.node_2().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[5][q_idx] == 3) {
        pbc_csr.port_5.cfg_oq_arb_l1_selection.node_3(pbc_csr.port_5.cfg_oq_arb_l1_selection.node_3().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[5][q_idx] == 4) {
        pbc_csr.port_5.cfg_oq_arb_l1_selection.node_4(pbc_csr.port_5.cfg_oq_arb_l1_selection.node_4().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[5][q_idx] == 5) {
        pbc_csr.port_5.cfg_oq_arb_l1_selection.node_5(pbc_csr.port_5.cfg_oq_arb_l1_selection.node_5().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[5][q_idx] == 6) {
        pbc_csr.port_5.cfg_oq_arb_l1_selection.node_6(pbc_csr.port_5.cfg_oq_arb_l1_selection.node_6().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[5][q_idx] == 7) {
        pbc_csr.port_5.cfg_oq_arb_l1_selection.node_7(pbc_csr.port_5.cfg_oq_arb_l1_selection.node_7().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[5][q_idx] == 8) {
        pbc_csr.port_5.cfg_oq_arb_l1_selection.node_8(pbc_csr.port_5.cfg_oq_arb_l1_selection.node_8().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[5][q_idx] == 9) {
        pbc_csr.port_5.cfg_oq_arb_l1_selection.node_9(pbc_csr.port_5.cfg_oq_arb_l1_selection.node_9().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[5][q_idx] == 10) {
        pbc_csr.port_5.cfg_oq_arb_l1_selection.node_10(pbc_csr.port_5.cfg_oq_arb_l1_selection.node_10().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[5][q_idx] == 11) {
        pbc_csr.port_5.cfg_oq_arb_l1_selection.node_11(pbc_csr.port_5.cfg_oq_arb_l1_selection.node_11().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[5][q_idx] == 12) {
        pbc_csr.port_5.cfg_oq_arb_l1_selection.node_12(pbc_csr.port_5.cfg_oq_arb_l1_selection.node_12().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[5][q_idx] == 13) {
        pbc_csr.port_5.cfg_oq_arb_l1_selection.node_13(pbc_csr.port_5.cfg_oq_arb_l1_selection.node_13().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[5][q_idx] == 14) {
        pbc_csr.port_5.cfg_oq_arb_l1_selection.node_14(pbc_csr.port_5.cfg_oq_arb_l1_selection.node_14().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[5][q_idx] == 15) {
        pbc_csr.port_5.cfg_oq_arb_l1_selection.node_15(pbc_csr.port_5.cfg_oq_arb_l1_selection.node_15().convert_to<int>() | 1 << q_idx);
      }
    }
    pbc_csr.port_5.cfg_oq_arb_l1_strict.all(0);
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_is_strict[5][q_idx] == 1) {
        pbc_csr.port_5.cfg_oq_arb_l1_strict.priority(pbc_csr.port_5.cfg_oq_arb_l1_strict.priority().convert_to<int>() | 1 << q_idx);
      }
    }

    pbc_csr.port_5.cfg_oq_arb_l1_selection.write();
    pbc_csr.port_5.cfg_oq_arb_l1_strict.write();

    pbc_csr.port_6.cfg_oq_arb_l1_selection.all(0);

    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[6][q_idx] == 0) {
        pbc_csr.port_6.cfg_oq_arb_l1_selection.node_0(pbc_csr.port_6.cfg_oq_arb_l1_selection.node_0().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[6][q_idx] == 1) {
        pbc_csr.port_6.cfg_oq_arb_l1_selection.node_1(pbc_csr.port_6.cfg_oq_arb_l1_selection.node_1().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[6][q_idx] == 2) {
        pbc_csr.port_6.cfg_oq_arb_l1_selection.node_2(pbc_csr.port_6.cfg_oq_arb_l1_selection.node_2().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[6][q_idx] == 3) {
        pbc_csr.port_6.cfg_oq_arb_l1_selection.node_3(pbc_csr.port_6.cfg_oq_arb_l1_selection.node_3().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[6][q_idx] == 4) {
        pbc_csr.port_6.cfg_oq_arb_l1_selection.node_4(pbc_csr.port_6.cfg_oq_arb_l1_selection.node_4().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[6][q_idx] == 5) {
        pbc_csr.port_6.cfg_oq_arb_l1_selection.node_5(pbc_csr.port_6.cfg_oq_arb_l1_selection.node_5().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[6][q_idx] == 6) {
        pbc_csr.port_6.cfg_oq_arb_l1_selection.node_6(pbc_csr.port_6.cfg_oq_arb_l1_selection.node_6().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[6][q_idx] == 7) {
        pbc_csr.port_6.cfg_oq_arb_l1_selection.node_7(pbc_csr.port_6.cfg_oq_arb_l1_selection.node_7().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[6][q_idx] == 8) {
        pbc_csr.port_6.cfg_oq_arb_l1_selection.node_8(pbc_csr.port_6.cfg_oq_arb_l1_selection.node_8().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[6][q_idx] == 9) {
        pbc_csr.port_6.cfg_oq_arb_l1_selection.node_9(pbc_csr.port_6.cfg_oq_arb_l1_selection.node_9().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[6][q_idx] == 10) {
        pbc_csr.port_6.cfg_oq_arb_l1_selection.node_10(pbc_csr.port_6.cfg_oq_arb_l1_selection.node_10().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[6][q_idx] == 11) {
        pbc_csr.port_6.cfg_oq_arb_l1_selection.node_11(pbc_csr.port_6.cfg_oq_arb_l1_selection.node_11().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[6][q_idx] == 12) {
        pbc_csr.port_6.cfg_oq_arb_l1_selection.node_12(pbc_csr.port_6.cfg_oq_arb_l1_selection.node_12().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[6][q_idx] == 13) {
        pbc_csr.port_6.cfg_oq_arb_l1_selection.node_13(pbc_csr.port_6.cfg_oq_arb_l1_selection.node_13().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[6][q_idx] == 14) {
        pbc_csr.port_6.cfg_oq_arb_l1_selection.node_14(pbc_csr.port_6.cfg_oq_arb_l1_selection.node_14().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[6][q_idx] == 15) {
        pbc_csr.port_6.cfg_oq_arb_l1_selection.node_15(pbc_csr.port_6.cfg_oq_arb_l1_selection.node_15().convert_to<int>() | 1 << q_idx);
      }
    }
    pbc_csr.port_6.cfg_oq_arb_l1_strict.all(0);
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_is_strict[6][q_idx] == 1) {
        pbc_csr.port_6.cfg_oq_arb_l1_strict.priority(pbc_csr.port_6.cfg_oq_arb_l1_strict.priority().convert_to<int>() | 1 << q_idx);
      }
    }

    pbc_csr.port_6.cfg_oq_arb_l1_selection.write();
    pbc_csr.port_6.cfg_oq_arb_l1_strict.write();

    pbc_csr.port_7.cfg_oq_arb_l1_selection.all(0);

    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[7][q_idx] == 0) {
        pbc_csr.port_7.cfg_oq_arb_l1_selection.node_0(pbc_csr.port_7.cfg_oq_arb_l1_selection.node_0().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[7][q_idx] == 1) {
        pbc_csr.port_7.cfg_oq_arb_l1_selection.node_1(pbc_csr.port_7.cfg_oq_arb_l1_selection.node_1().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[7][q_idx] == 2) {
        pbc_csr.port_7.cfg_oq_arb_l1_selection.node_2(pbc_csr.port_7.cfg_oq_arb_l1_selection.node_2().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[7][q_idx] == 3) {
        pbc_csr.port_7.cfg_oq_arb_l1_selection.node_3(pbc_csr.port_7.cfg_oq_arb_l1_selection.node_3().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[7][q_idx] == 4) {
        pbc_csr.port_7.cfg_oq_arb_l1_selection.node_4(pbc_csr.port_7.cfg_oq_arb_l1_selection.node_4().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[7][q_idx] == 5) {
        pbc_csr.port_7.cfg_oq_arb_l1_selection.node_5(pbc_csr.port_7.cfg_oq_arb_l1_selection.node_5().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[7][q_idx] == 6) {
        pbc_csr.port_7.cfg_oq_arb_l1_selection.node_6(pbc_csr.port_7.cfg_oq_arb_l1_selection.node_6().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[7][q_idx] == 7) {
        pbc_csr.port_7.cfg_oq_arb_l1_selection.node_7(pbc_csr.port_7.cfg_oq_arb_l1_selection.node_7().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[7][q_idx] == 8) {
        pbc_csr.port_7.cfg_oq_arb_l1_selection.node_8(pbc_csr.port_7.cfg_oq_arb_l1_selection.node_8().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[7][q_idx] == 9) {
        pbc_csr.port_7.cfg_oq_arb_l1_selection.node_9(pbc_csr.port_7.cfg_oq_arb_l1_selection.node_9().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[7][q_idx] == 10) {
        pbc_csr.port_7.cfg_oq_arb_l1_selection.node_10(pbc_csr.port_7.cfg_oq_arb_l1_selection.node_10().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[7][q_idx] == 11) {
        pbc_csr.port_7.cfg_oq_arb_l1_selection.node_11(pbc_csr.port_7.cfg_oq_arb_l1_selection.node_11().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[7][q_idx] == 12) {
        pbc_csr.port_7.cfg_oq_arb_l1_selection.node_12(pbc_csr.port_7.cfg_oq_arb_l1_selection.node_12().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[7][q_idx] == 13) {
        pbc_csr.port_7.cfg_oq_arb_l1_selection.node_13(pbc_csr.port_7.cfg_oq_arb_l1_selection.node_13().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[7][q_idx] == 14) {
        pbc_csr.port_7.cfg_oq_arb_l1_selection.node_14(pbc_csr.port_7.cfg_oq_arb_l1_selection.node_14().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[7][q_idx] == 15) {
        pbc_csr.port_7.cfg_oq_arb_l1_selection.node_15(pbc_csr.port_7.cfg_oq_arb_l1_selection.node_15().convert_to<int>() | 1 << q_idx);
      }
    }
    pbc_csr.port_7.cfg_oq_arb_l1_strict.all(0);
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_is_strict[7][q_idx] == 1) {
        pbc_csr.port_7.cfg_oq_arb_l1_strict.priority(pbc_csr.port_7.cfg_oq_arb_l1_strict.priority().convert_to<int>() | 1 << q_idx);
      }
    }

    pbc_csr.port_7.cfg_oq_arb_l1_selection.write();
    pbc_csr.port_7.cfg_oq_arb_l1_strict.write();

    pbc_csr.port_8.cfg_oq_arb_l1_selection.all(0);

    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[8][q_idx] == 0) {
        pbc_csr.port_8.cfg_oq_arb_l1_selection.node_0(pbc_csr.port_8.cfg_oq_arb_l1_selection.node_0().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[8][q_idx] == 1) {
        pbc_csr.port_8.cfg_oq_arb_l1_selection.node_1(pbc_csr.port_8.cfg_oq_arb_l1_selection.node_1().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[8][q_idx] == 2) {
        pbc_csr.port_8.cfg_oq_arb_l1_selection.node_2(pbc_csr.port_8.cfg_oq_arb_l1_selection.node_2().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[8][q_idx] == 3) {
        pbc_csr.port_8.cfg_oq_arb_l1_selection.node_3(pbc_csr.port_8.cfg_oq_arb_l1_selection.node_3().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[8][q_idx] == 4) {
        pbc_csr.port_8.cfg_oq_arb_l1_selection.node_4(pbc_csr.port_8.cfg_oq_arb_l1_selection.node_4().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[8][q_idx] == 5) {
        pbc_csr.port_8.cfg_oq_arb_l1_selection.node_5(pbc_csr.port_8.cfg_oq_arb_l1_selection.node_5().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[8][q_idx] == 6) {
        pbc_csr.port_8.cfg_oq_arb_l1_selection.node_6(pbc_csr.port_8.cfg_oq_arb_l1_selection.node_6().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[8][q_idx] == 7) {
        pbc_csr.port_8.cfg_oq_arb_l1_selection.node_7(pbc_csr.port_8.cfg_oq_arb_l1_selection.node_7().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[8][q_idx] == 8) {
        pbc_csr.port_8.cfg_oq_arb_l1_selection.node_8(pbc_csr.port_8.cfg_oq_arb_l1_selection.node_8().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[8][q_idx] == 9) {
        pbc_csr.port_8.cfg_oq_arb_l1_selection.node_9(pbc_csr.port_8.cfg_oq_arb_l1_selection.node_9().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[8][q_idx] == 10) {
        pbc_csr.port_8.cfg_oq_arb_l1_selection.node_10(pbc_csr.port_8.cfg_oq_arb_l1_selection.node_10().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[8][q_idx] == 11) {
        pbc_csr.port_8.cfg_oq_arb_l1_selection.node_11(pbc_csr.port_8.cfg_oq_arb_l1_selection.node_11().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[8][q_idx] == 12) {
        pbc_csr.port_8.cfg_oq_arb_l1_selection.node_12(pbc_csr.port_8.cfg_oq_arb_l1_selection.node_12().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[8][q_idx] == 13) {
        pbc_csr.port_8.cfg_oq_arb_l1_selection.node_13(pbc_csr.port_8.cfg_oq_arb_l1_selection.node_13().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[8][q_idx] == 14) {
        pbc_csr.port_8.cfg_oq_arb_l1_selection.node_14(pbc_csr.port_8.cfg_oq_arb_l1_selection.node_14().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[8][q_idx] == 15) {
        pbc_csr.port_8.cfg_oq_arb_l1_selection.node_15(pbc_csr.port_8.cfg_oq_arb_l1_selection.node_15().convert_to<int>() | 1 << q_idx);
      }
    }
    pbc_csr.port_8.cfg_oq_arb_l1_strict.all(0);
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_is_strict[8][q_idx] == 1) {
        pbc_csr.port_8.cfg_oq_arb_l1_strict.priority(pbc_csr.port_8.cfg_oq_arb_l1_strict.priority().convert_to<int>() | 1 << q_idx);
      }
    }

    pbc_csr.port_8.cfg_oq_arb_l1_selection.write();
    pbc_csr.port_8.cfg_oq_arb_l1_strict.write();

    pbc_csr.port_9.cfg_oq_arb_l1_selection.all(0);

    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[9][q_idx] == 0) {
        pbc_csr.port_9.cfg_oq_arb_l1_selection.node_0(pbc_csr.port_9.cfg_oq_arb_l1_selection.node_0().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[9][q_idx] == 1) {
        pbc_csr.port_9.cfg_oq_arb_l1_selection.node_1(pbc_csr.port_9.cfg_oq_arb_l1_selection.node_1().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[9][q_idx] == 2) {
        pbc_csr.port_9.cfg_oq_arb_l1_selection.node_2(pbc_csr.port_9.cfg_oq_arb_l1_selection.node_2().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[9][q_idx] == 3) {
        pbc_csr.port_9.cfg_oq_arb_l1_selection.node_3(pbc_csr.port_9.cfg_oq_arb_l1_selection.node_3().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[9][q_idx] == 4) {
        pbc_csr.port_9.cfg_oq_arb_l1_selection.node_4(pbc_csr.port_9.cfg_oq_arb_l1_selection.node_4().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[9][q_idx] == 5) {
        pbc_csr.port_9.cfg_oq_arb_l1_selection.node_5(pbc_csr.port_9.cfg_oq_arb_l1_selection.node_5().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[9][q_idx] == 6) {
        pbc_csr.port_9.cfg_oq_arb_l1_selection.node_6(pbc_csr.port_9.cfg_oq_arb_l1_selection.node_6().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[9][q_idx] == 7) {
        pbc_csr.port_9.cfg_oq_arb_l1_selection.node_7(pbc_csr.port_9.cfg_oq_arb_l1_selection.node_7().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[9][q_idx] == 8) {
        pbc_csr.port_9.cfg_oq_arb_l1_selection.node_8(pbc_csr.port_9.cfg_oq_arb_l1_selection.node_8().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[9][q_idx] == 9) {
        pbc_csr.port_9.cfg_oq_arb_l1_selection.node_9(pbc_csr.port_9.cfg_oq_arb_l1_selection.node_9().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[9][q_idx] == 10) {
        pbc_csr.port_9.cfg_oq_arb_l1_selection.node_10(pbc_csr.port_9.cfg_oq_arb_l1_selection.node_10().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[9][q_idx] == 11) {
        pbc_csr.port_9.cfg_oq_arb_l1_selection.node_11(pbc_csr.port_9.cfg_oq_arb_l1_selection.node_11().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[9][q_idx] == 12) {
        pbc_csr.port_9.cfg_oq_arb_l1_selection.node_12(pbc_csr.port_9.cfg_oq_arb_l1_selection.node_12().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[9][q_idx] == 13) {
        pbc_csr.port_9.cfg_oq_arb_l1_selection.node_13(pbc_csr.port_9.cfg_oq_arb_l1_selection.node_13().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[9][q_idx] == 14) {
        pbc_csr.port_9.cfg_oq_arb_l1_selection.node_14(pbc_csr.port_9.cfg_oq_arb_l1_selection.node_14().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[9][q_idx] == 15) {
        pbc_csr.port_9.cfg_oq_arb_l1_selection.node_15(pbc_csr.port_9.cfg_oq_arb_l1_selection.node_15().convert_to<int>() | 1 << q_idx);
      }
    }
    pbc_csr.port_9.cfg_oq_arb_l1_strict.all(0);
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_is_strict[9][q_idx] == 1) {
        pbc_csr.port_9.cfg_oq_arb_l1_strict.priority(pbc_csr.port_9.cfg_oq_arb_l1_strict.priority().convert_to<int>() | 1 << q_idx);
      }
    }

    pbc_csr.port_9.cfg_oq_arb_l1_selection.write();
    pbc_csr.port_9.cfg_oq_arb_l1_strict.write();

    pbc_csr.port_10.cfg_oq_arb_l1_selection.all(0);

    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[10][q_idx] == 0) {
        pbc_csr.port_10.cfg_oq_arb_l1_selection.node_0(pbc_csr.port_10.cfg_oq_arb_l1_selection.node_0().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[10][q_idx] == 1) {
        pbc_csr.port_10.cfg_oq_arb_l1_selection.node_1(pbc_csr.port_10.cfg_oq_arb_l1_selection.node_1().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[10][q_idx] == 2) {
        pbc_csr.port_10.cfg_oq_arb_l1_selection.node_2(pbc_csr.port_10.cfg_oq_arb_l1_selection.node_2().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[10][q_idx] == 3) {
        pbc_csr.port_10.cfg_oq_arb_l1_selection.node_3(pbc_csr.port_10.cfg_oq_arb_l1_selection.node_3().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[10][q_idx] == 4) {
        pbc_csr.port_10.cfg_oq_arb_l1_selection.node_4(pbc_csr.port_10.cfg_oq_arb_l1_selection.node_4().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[10][q_idx] == 5) {
        pbc_csr.port_10.cfg_oq_arb_l1_selection.node_5(pbc_csr.port_10.cfg_oq_arb_l1_selection.node_5().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[10][q_idx] == 6) {
        pbc_csr.port_10.cfg_oq_arb_l1_selection.node_6(pbc_csr.port_10.cfg_oq_arb_l1_selection.node_6().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[10][q_idx] == 7) {
        pbc_csr.port_10.cfg_oq_arb_l1_selection.node_7(pbc_csr.port_10.cfg_oq_arb_l1_selection.node_7().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[10][q_idx] == 8) {
        pbc_csr.port_10.cfg_oq_arb_l1_selection.node_8(pbc_csr.port_10.cfg_oq_arb_l1_selection.node_8().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[10][q_idx] == 9) {
        pbc_csr.port_10.cfg_oq_arb_l1_selection.node_9(pbc_csr.port_10.cfg_oq_arb_l1_selection.node_9().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[10][q_idx] == 10) {
        pbc_csr.port_10.cfg_oq_arb_l1_selection.node_10(pbc_csr.port_10.cfg_oq_arb_l1_selection.node_10().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[10][q_idx] == 11) {
        pbc_csr.port_10.cfg_oq_arb_l1_selection.node_11(pbc_csr.port_10.cfg_oq_arb_l1_selection.node_11().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[10][q_idx] == 12) {
        pbc_csr.port_10.cfg_oq_arb_l1_selection.node_12(pbc_csr.port_10.cfg_oq_arb_l1_selection.node_12().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[10][q_idx] == 13) {
        pbc_csr.port_10.cfg_oq_arb_l1_selection.node_13(pbc_csr.port_10.cfg_oq_arb_l1_selection.node_13().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[10][q_idx] == 14) {
        pbc_csr.port_10.cfg_oq_arb_l1_selection.node_14(pbc_csr.port_10.cfg_oq_arb_l1_selection.node_14().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[10][q_idx] == 15) {
        pbc_csr.port_10.cfg_oq_arb_l1_selection.node_15(pbc_csr.port_10.cfg_oq_arb_l1_selection.node_15().convert_to<int>() | 1 << q_idx);
      }
    }
    pbc_csr.port_10.cfg_oq_arb_l1_strict.all(0);
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_is_strict[10][q_idx] == 1) {
        pbc_csr.port_10.cfg_oq_arb_l1_strict.priority(pbc_csr.port_10.cfg_oq_arb_l1_strict.priority().convert_to<int>() | 1 << q_idx);
      }
      if(l1_priority_bypass_timer [10][q_idx] == 1) {
        pbc_csr.port_10.cfg_oq_arb_l1_strict.priority_bypass_timer(pbc_csr.port_10.cfg_oq_arb_l1_strict.priority_bypass_timer().convert_to<int>() | 1 << q_idx);
      }
    }

    pbc_csr.port_10.cfg_oq_arb_l1_selection.write();
    pbc_csr.port_10.cfg_oq_arb_l1_strict.write();

    pbc_csr.port_11.cfg_oq_arb_l1_selection.all(0);

    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[11][q_idx] == 0) {
        pbc_csr.port_11.cfg_oq_arb_l1_selection.node_0(pbc_csr.port_11.cfg_oq_arb_l1_selection.node_0().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[11][q_idx] == 1) {
        pbc_csr.port_11.cfg_oq_arb_l1_selection.node_1(pbc_csr.port_11.cfg_oq_arb_l1_selection.node_1().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[11][q_idx] == 2) {
        pbc_csr.port_11.cfg_oq_arb_l1_selection.node_2(pbc_csr.port_11.cfg_oq_arb_l1_selection.node_2().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[11][q_idx] == 3) {
        pbc_csr.port_11.cfg_oq_arb_l1_selection.node_3(pbc_csr.port_11.cfg_oq_arb_l1_selection.node_3().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[11][q_idx] == 4) {
        pbc_csr.port_11.cfg_oq_arb_l1_selection.node_4(pbc_csr.port_11.cfg_oq_arb_l1_selection.node_4().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[11][q_idx] == 5) {
        pbc_csr.port_11.cfg_oq_arb_l1_selection.node_5(pbc_csr.port_11.cfg_oq_arb_l1_selection.node_5().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[11][q_idx] == 6) {
        pbc_csr.port_11.cfg_oq_arb_l1_selection.node_6(pbc_csr.port_11.cfg_oq_arb_l1_selection.node_6().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[11][q_idx] == 7) {
        pbc_csr.port_11.cfg_oq_arb_l1_selection.node_7(pbc_csr.port_11.cfg_oq_arb_l1_selection.node_7().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[11][q_idx] == 8) {
        pbc_csr.port_11.cfg_oq_arb_l1_selection.node_8(pbc_csr.port_11.cfg_oq_arb_l1_selection.node_8().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[11][q_idx] == 9) {
        pbc_csr.port_11.cfg_oq_arb_l1_selection.node_9(pbc_csr.port_11.cfg_oq_arb_l1_selection.node_9().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[11][q_idx] == 10) {
        pbc_csr.port_11.cfg_oq_arb_l1_selection.node_10(pbc_csr.port_11.cfg_oq_arb_l1_selection.node_10().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[11][q_idx] == 11) {
        pbc_csr.port_11.cfg_oq_arb_l1_selection.node_11(pbc_csr.port_11.cfg_oq_arb_l1_selection.node_11().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[11][q_idx] == 12) {
        pbc_csr.port_11.cfg_oq_arb_l1_selection.node_12(pbc_csr.port_11.cfg_oq_arb_l1_selection.node_12().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[11][q_idx] == 13) {
        pbc_csr.port_11.cfg_oq_arb_l1_selection.node_13(pbc_csr.port_11.cfg_oq_arb_l1_selection.node_13().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[11][q_idx] == 14) {
        pbc_csr.port_11.cfg_oq_arb_l1_selection.node_14(pbc_csr.port_11.cfg_oq_arb_l1_selection.node_14().convert_to<int>() | 1 << q_idx);
      }
    }
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_node[11][q_idx] == 15) {
        pbc_csr.port_11.cfg_oq_arb_l1_selection.node_15(pbc_csr.port_11.cfg_oq_arb_l1_selection.node_15().convert_to<int>() | 1 << q_idx);
      }
    }
    pbc_csr.port_11.cfg_oq_arb_l1_strict.all(0);
    for(int q_idx=0; q_idx<32; q_idx++) {
      if(l1_is_strict[11][q_idx] == 1) {
        pbc_csr.port_11.cfg_oq_arb_l1_strict.priority(pbc_csr.port_11.cfg_oq_arb_l1_strict.priority().convert_to<int>() | 1 << q_idx);
      }
      if(l1_priority_bypass_timer [11][q_idx] == 1) {
        pbc_csr.port_11.cfg_oq_arb_l1_strict.priority_bypass_timer(pbc_csr.port_11.cfg_oq_arb_l1_strict.priority_bypass_timer().convert_to<int>() | 1 << q_idx);
      }
    }

    pbc_csr.port_11.cfg_oq_arb_l1_selection.write();
    pbc_csr.port_11.cfg_oq_arb_l1_strict.write();

    pbc_csr.port_0.cfg_oq_arb_l2_selection.all(0);
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[0][l1_node_idx] == 0) {
        pbc_csr.port_0.cfg_oq_arb_l2_selection.node_0(pbc_csr.port_0.cfg_oq_arb_l2_selection.node_0().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[0][l1_node_idx] == 1) {
        pbc_csr.port_0.cfg_oq_arb_l2_selection.node_1(pbc_csr.port_0.cfg_oq_arb_l2_selection.node_1().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[0][l1_node_idx] == 2) {
        pbc_csr.port_0.cfg_oq_arb_l2_selection.node_2(pbc_csr.port_0.cfg_oq_arb_l2_selection.node_2().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[0][l1_node_idx] == 3) {
        pbc_csr.port_0.cfg_oq_arb_l2_selection.node_3(pbc_csr.port_0.cfg_oq_arb_l2_selection.node_3().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    pbc_csr.port_0.cfg_oq_arb_l2_strict.all(0);
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_is_strict[0][l1_node_idx] == 1) {
        pbc_csr.port_0.cfg_oq_arb_l2_strict.priority(pbc_csr.port_0.cfg_oq_arb_l2_strict.priority().convert_to<int>() | 1 << l1_node_idx);
      }
    }

    pbc_csr.port_0.cfg_oq_arb_l2_selection.write();
    pbc_csr.port_0.cfg_oq_arb_l2_strict.write();

    pbc_csr.port_1.cfg_oq_arb_l2_selection.all(0);
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[1][l1_node_idx] == 0) {
        pbc_csr.port_1.cfg_oq_arb_l2_selection.node_0(pbc_csr.port_1.cfg_oq_arb_l2_selection.node_0().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[1][l1_node_idx] == 1) {
        pbc_csr.port_1.cfg_oq_arb_l2_selection.node_1(pbc_csr.port_1.cfg_oq_arb_l2_selection.node_1().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[1][l1_node_idx] == 2) {
        pbc_csr.port_1.cfg_oq_arb_l2_selection.node_2(pbc_csr.port_1.cfg_oq_arb_l2_selection.node_2().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[1][l1_node_idx] == 3) {
        pbc_csr.port_1.cfg_oq_arb_l2_selection.node_3(pbc_csr.port_1.cfg_oq_arb_l2_selection.node_3().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    pbc_csr.port_1.cfg_oq_arb_l2_strict.all(0);
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_is_strict[1][l1_node_idx] == 1) {
        pbc_csr.port_1.cfg_oq_arb_l2_strict.priority(pbc_csr.port_1.cfg_oq_arb_l2_strict.priority().convert_to<int>() | 1 << l1_node_idx);
      }
    }

    pbc_csr.port_1.cfg_oq_arb_l2_selection.write();
    pbc_csr.port_1.cfg_oq_arb_l2_strict.write();

    pbc_csr.port_2.cfg_oq_arb_l2_selection.all(0);
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[2][l1_node_idx] == 0) {
        pbc_csr.port_2.cfg_oq_arb_l2_selection.node_0(pbc_csr.port_2.cfg_oq_arb_l2_selection.node_0().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[2][l1_node_idx] == 1) {
        pbc_csr.port_2.cfg_oq_arb_l2_selection.node_1(pbc_csr.port_2.cfg_oq_arb_l2_selection.node_1().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[2][l1_node_idx] == 2) {
        pbc_csr.port_2.cfg_oq_arb_l2_selection.node_2(pbc_csr.port_2.cfg_oq_arb_l2_selection.node_2().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[2][l1_node_idx] == 3) {
        pbc_csr.port_2.cfg_oq_arb_l2_selection.node_3(pbc_csr.port_2.cfg_oq_arb_l2_selection.node_3().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    pbc_csr.port_2.cfg_oq_arb_l2_strict.all(0);
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_is_strict[2][l1_node_idx] == 1) {
        pbc_csr.port_2.cfg_oq_arb_l2_strict.priority(pbc_csr.port_2.cfg_oq_arb_l2_strict.priority().convert_to<int>() | 1 << l1_node_idx);
      }
    }

    pbc_csr.port_2.cfg_oq_arb_l2_selection.write();
    pbc_csr.port_2.cfg_oq_arb_l2_strict.write();

    pbc_csr.port_3.cfg_oq_arb_l2_selection.all(0);
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[3][l1_node_idx] == 0) {
        pbc_csr.port_3.cfg_oq_arb_l2_selection.node_0(pbc_csr.port_3.cfg_oq_arb_l2_selection.node_0().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[3][l1_node_idx] == 1) {
        pbc_csr.port_3.cfg_oq_arb_l2_selection.node_1(pbc_csr.port_3.cfg_oq_arb_l2_selection.node_1().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[3][l1_node_idx] == 2) {
        pbc_csr.port_3.cfg_oq_arb_l2_selection.node_2(pbc_csr.port_3.cfg_oq_arb_l2_selection.node_2().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[3][l1_node_idx] == 3) {
        pbc_csr.port_3.cfg_oq_arb_l2_selection.node_3(pbc_csr.port_3.cfg_oq_arb_l2_selection.node_3().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    pbc_csr.port_3.cfg_oq_arb_l2_strict.all(0);
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_is_strict[3][l1_node_idx] == 1) {
        pbc_csr.port_3.cfg_oq_arb_l2_strict.priority(pbc_csr.port_3.cfg_oq_arb_l2_strict.priority().convert_to<int>() | 1 << l1_node_idx);
      }
    }

    pbc_csr.port_3.cfg_oq_arb_l2_selection.write();
    pbc_csr.port_3.cfg_oq_arb_l2_strict.write();

    pbc_csr.port_4.cfg_oq_arb_l2_selection.all(0);
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[4][l1_node_idx] == 0) {
        pbc_csr.port_4.cfg_oq_arb_l2_selection.node_0(pbc_csr.port_4.cfg_oq_arb_l2_selection.node_0().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[4][l1_node_idx] == 1) {
        pbc_csr.port_4.cfg_oq_arb_l2_selection.node_1(pbc_csr.port_4.cfg_oq_arb_l2_selection.node_1().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[4][l1_node_idx] == 2) {
        pbc_csr.port_4.cfg_oq_arb_l2_selection.node_2(pbc_csr.port_4.cfg_oq_arb_l2_selection.node_2().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[4][l1_node_idx] == 3) {
        pbc_csr.port_4.cfg_oq_arb_l2_selection.node_3(pbc_csr.port_4.cfg_oq_arb_l2_selection.node_3().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    pbc_csr.port_4.cfg_oq_arb_l2_strict.all(0);
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_is_strict[4][l1_node_idx] == 1) {
        pbc_csr.port_4.cfg_oq_arb_l2_strict.priority(pbc_csr.port_4.cfg_oq_arb_l2_strict.priority().convert_to<int>() | 1 << l1_node_idx);
      }
    }

    pbc_csr.port_4.cfg_oq_arb_l2_selection.write();
    pbc_csr.port_4.cfg_oq_arb_l2_strict.write();

    pbc_csr.port_5.cfg_oq_arb_l2_selection.all(0);
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[5][l1_node_idx] == 0) {
        pbc_csr.port_5.cfg_oq_arb_l2_selection.node_0(pbc_csr.port_5.cfg_oq_arb_l2_selection.node_0().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[5][l1_node_idx] == 1) {
        pbc_csr.port_5.cfg_oq_arb_l2_selection.node_1(pbc_csr.port_5.cfg_oq_arb_l2_selection.node_1().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[5][l1_node_idx] == 2) {
        pbc_csr.port_5.cfg_oq_arb_l2_selection.node_2(pbc_csr.port_5.cfg_oq_arb_l2_selection.node_2().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[5][l1_node_idx] == 3) {
        pbc_csr.port_5.cfg_oq_arb_l2_selection.node_3(pbc_csr.port_5.cfg_oq_arb_l2_selection.node_3().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    pbc_csr.port_5.cfg_oq_arb_l2_strict.all(0);
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_is_strict[5][l1_node_idx] == 1) {
        pbc_csr.port_5.cfg_oq_arb_l2_strict.priority(pbc_csr.port_5.cfg_oq_arb_l2_strict.priority().convert_to<int>() | 1 << l1_node_idx);
      }
    }

    pbc_csr.port_5.cfg_oq_arb_l2_selection.write();
    pbc_csr.port_5.cfg_oq_arb_l2_strict.write();

    pbc_csr.port_6.cfg_oq_arb_l2_selection.all(0);
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[6][l1_node_idx] == 0) {
        pbc_csr.port_6.cfg_oq_arb_l2_selection.node_0(pbc_csr.port_6.cfg_oq_arb_l2_selection.node_0().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[6][l1_node_idx] == 1) {
        pbc_csr.port_6.cfg_oq_arb_l2_selection.node_1(pbc_csr.port_6.cfg_oq_arb_l2_selection.node_1().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[6][l1_node_idx] == 2) {
        pbc_csr.port_6.cfg_oq_arb_l2_selection.node_2(pbc_csr.port_6.cfg_oq_arb_l2_selection.node_2().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[6][l1_node_idx] == 3) {
        pbc_csr.port_6.cfg_oq_arb_l2_selection.node_3(pbc_csr.port_6.cfg_oq_arb_l2_selection.node_3().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    pbc_csr.port_6.cfg_oq_arb_l2_strict.all(0);
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_is_strict[6][l1_node_idx] == 1) {
        pbc_csr.port_6.cfg_oq_arb_l2_strict.priority(pbc_csr.port_6.cfg_oq_arb_l2_strict.priority().convert_to<int>() | 1 << l1_node_idx);
      }
    }

    pbc_csr.port_6.cfg_oq_arb_l2_selection.write();
    pbc_csr.port_6.cfg_oq_arb_l2_strict.write();

    pbc_csr.port_7.cfg_oq_arb_l2_selection.all(0);
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[7][l1_node_idx] == 0) {
        pbc_csr.port_7.cfg_oq_arb_l2_selection.node_0(pbc_csr.port_7.cfg_oq_arb_l2_selection.node_0().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[7][l1_node_idx] == 1) {
        pbc_csr.port_7.cfg_oq_arb_l2_selection.node_1(pbc_csr.port_7.cfg_oq_arb_l2_selection.node_1().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[7][l1_node_idx] == 2) {
        pbc_csr.port_7.cfg_oq_arb_l2_selection.node_2(pbc_csr.port_7.cfg_oq_arb_l2_selection.node_2().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[7][l1_node_idx] == 3) {
        pbc_csr.port_7.cfg_oq_arb_l2_selection.node_3(pbc_csr.port_7.cfg_oq_arb_l2_selection.node_3().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    pbc_csr.port_7.cfg_oq_arb_l2_strict.all(0);
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_is_strict[7][l1_node_idx] == 1) {
        pbc_csr.port_7.cfg_oq_arb_l2_strict.priority(pbc_csr.port_7.cfg_oq_arb_l2_strict.priority().convert_to<int>() | 1 << l1_node_idx);
      }
    }

    pbc_csr.port_7.cfg_oq_arb_l2_selection.write();
    pbc_csr.port_7.cfg_oq_arb_l2_strict.write();

    pbc_csr.port_8.cfg_oq_arb_l2_selection.all(0);
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[8][l1_node_idx] == 0) {
        pbc_csr.port_8.cfg_oq_arb_l2_selection.node_0(pbc_csr.port_8.cfg_oq_arb_l2_selection.node_0().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[8][l1_node_idx] == 1) {
        pbc_csr.port_8.cfg_oq_arb_l2_selection.node_1(pbc_csr.port_8.cfg_oq_arb_l2_selection.node_1().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[8][l1_node_idx] == 2) {
        pbc_csr.port_8.cfg_oq_arb_l2_selection.node_2(pbc_csr.port_8.cfg_oq_arb_l2_selection.node_2().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[8][l1_node_idx] == 3) {
        pbc_csr.port_8.cfg_oq_arb_l2_selection.node_3(pbc_csr.port_8.cfg_oq_arb_l2_selection.node_3().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    pbc_csr.port_8.cfg_oq_arb_l2_strict.all(0);
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_is_strict[8][l1_node_idx] == 1) {
        pbc_csr.port_8.cfg_oq_arb_l2_strict.priority(pbc_csr.port_8.cfg_oq_arb_l2_strict.priority().convert_to<int>() | 1 << l1_node_idx);
      }
    }

    pbc_csr.port_8.cfg_oq_arb_l2_selection.write();
    pbc_csr.port_8.cfg_oq_arb_l2_strict.write();

    pbc_csr.port_9.cfg_oq_arb_l2_selection.all(0);
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[9][l1_node_idx] == 0) {
        pbc_csr.port_9.cfg_oq_arb_l2_selection.node_0(pbc_csr.port_9.cfg_oq_arb_l2_selection.node_0().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[9][l1_node_idx] == 1) {
        pbc_csr.port_9.cfg_oq_arb_l2_selection.node_1(pbc_csr.port_9.cfg_oq_arb_l2_selection.node_1().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[9][l1_node_idx] == 2) {
        pbc_csr.port_9.cfg_oq_arb_l2_selection.node_2(pbc_csr.port_9.cfg_oq_arb_l2_selection.node_2().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[9][l1_node_idx] == 3) {
        pbc_csr.port_9.cfg_oq_arb_l2_selection.node_3(pbc_csr.port_9.cfg_oq_arb_l2_selection.node_3().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    pbc_csr.port_9.cfg_oq_arb_l2_strict.all(0);
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_is_strict[9][l1_node_idx] == 1) {
        pbc_csr.port_9.cfg_oq_arb_l2_strict.priority(pbc_csr.port_9.cfg_oq_arb_l2_strict.priority().convert_to<int>() | 1 << l1_node_idx);
      }
    }

    pbc_csr.port_9.cfg_oq_arb_l2_selection.write();
    pbc_csr.port_9.cfg_oq_arb_l2_strict.write();

    pbc_csr.port_10.cfg_oq_arb_l2_selection.all(0);
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[10][l1_node_idx] == 0) {
        pbc_csr.port_10.cfg_oq_arb_l2_selection.node_0(pbc_csr.port_10.cfg_oq_arb_l2_selection.node_0().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[10][l1_node_idx] == 1) {
        pbc_csr.port_10.cfg_oq_arb_l2_selection.node_1(pbc_csr.port_10.cfg_oq_arb_l2_selection.node_1().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[10][l1_node_idx] == 2) {
        pbc_csr.port_10.cfg_oq_arb_l2_selection.node_2(pbc_csr.port_10.cfg_oq_arb_l2_selection.node_2().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[10][l1_node_idx] == 3) {
        pbc_csr.port_10.cfg_oq_arb_l2_selection.node_3(pbc_csr.port_10.cfg_oq_arb_l2_selection.node_3().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    pbc_csr.port_10.cfg_oq_arb_l2_strict.all(0);
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_is_strict[10][l1_node_idx] == 1) {
        pbc_csr.port_10.cfg_oq_arb_l2_strict.priority(pbc_csr.port_10.cfg_oq_arb_l2_strict.priority().convert_to<int>() | 1 << l1_node_idx);
      }
      if(l2_priority_bypass_timer [10][l1_node_idx] == 1) {
        pbc_csr.port_10.cfg_oq_arb_l2_strict.priority_bypass_timer(pbc_csr.port_10.cfg_oq_arb_l2_strict.priority_bypass_timer().convert_to<int>() | 1 << l1_node_idx);
      }
    }

    pbc_csr.port_10.cfg_oq_arb_l2_selection.write();
    pbc_csr.port_10.cfg_oq_arb_l2_strict.write();

    pbc_csr.port_11.cfg_oq_arb_l2_selection.all(0);
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[11][l1_node_idx] == 0) {
        pbc_csr.port_11.cfg_oq_arb_l2_selection.node_0(pbc_csr.port_11.cfg_oq_arb_l2_selection.node_0().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[11][l1_node_idx] == 1) {
        pbc_csr.port_11.cfg_oq_arb_l2_selection.node_1(pbc_csr.port_11.cfg_oq_arb_l2_selection.node_1().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[11][l1_node_idx] == 2) {
        pbc_csr.port_11.cfg_oq_arb_l2_selection.node_2(pbc_csr.port_11.cfg_oq_arb_l2_selection.node_2().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_node[11][l1_node_idx] == 3) {
        pbc_csr.port_11.cfg_oq_arb_l2_selection.node_3(pbc_csr.port_11.cfg_oq_arb_l2_selection.node_3().convert_to<int>() | 1 << l1_node_idx);
      }
    }
    pbc_csr.port_11.cfg_oq_arb_l2_strict.all(0);
    for(int l1_node_idx=0; l1_node_idx<16; l1_node_idx++) {
      if(l2_is_strict[11][l1_node_idx] == 1) {
        pbc_csr.port_11.cfg_oq_arb_l2_strict.priority(pbc_csr.port_11.cfg_oq_arb_l2_strict.priority().convert_to<int>() | 1 << l1_node_idx);
      }
      if(l2_priority_bypass_timer [11][l1_node_idx] == 1) {
        pbc_csr.port_11.cfg_oq_arb_l2_strict.priority_bypass_timer(pbc_csr.port_11.cfg_oq_arb_l2_strict.priority_bypass_timer().convert_to<int>() | 1 << l1_node_idx);
      }
    }

    pbc_csr.port_11.cfg_oq_arb_l2_selection.write();
    pbc_csr.port_11.cfg_oq_arb_l2_strict.write();

 //   pbc_csr.port_9.cfg_oq_arb_l3_selection.all(0);
 // for(int l2_node_idx=0; l2_node_idx<4; l2_node_idx++) {
 //   if(l3_node[9][l2_node_idx] == 0) {
 //     pbc_csr.port_9.cfg_oq_arb_l3_selection.node_0(pbc_csr.port_9.cfg_oq_arb_l3_selection.node_0().convert_to<int>() | 1 << l2_node_idx);
 //   }
 // }
 // pbc_csr.port_9.cfg_oq_arb_l3_selection.write();
     
    pbc_csr.port_9.cfg_oq_arb_l3_strict.all(0);
    for(int l2_node_idx=0; l2_node_idx<4; l2_node_idx++) {
      if(l3_is_strict[9][l2_node_idx] == 1) {
        pbc_csr.port_9.cfg_oq_arb_l3_strict.priority(pbc_csr.port_9.cfg_oq_arb_l3_strict.priority().convert_to<int>() | 1 << l2_node_idx);
      }
    }

    pbc_csr.port_9.cfg_oq_arb_l3_strict.write();

 //   pbc_csr.port_10.cfg_oq_arb_l3_selection.all(0);
 // for(int l2_node_idx=0; l2_node_idx<4; l2_node_idx++) {
 //   if(l3_node[10][l2_node_idx] == 0) {
 //     pbc_csr.port_10.cfg_oq_arb_l3_selection.node_0(pbc_csr.port_10.cfg_oq_arb_l3_selection.node_0().convert_to<int>() | 1 << l2_node_idx);
 //   }
 // }
 // pbc_csr.port_10.cfg_oq_arb_l3_selection.write();
     
    pbc_csr.port_10.cfg_oq_arb_l3_strict.all(0);
    for(int l2_node_idx=0; l2_node_idx<4; l2_node_idx++) {
      if(l3_is_strict[10][l2_node_idx] == 1) {
        pbc_csr.port_10.cfg_oq_arb_l3_strict.priority(pbc_csr.port_10.cfg_oq_arb_l3_strict.priority().convert_to<int>() | 1 << l2_node_idx);
      }
      if(l3_priority_bypass_timer [10][l2_node_idx] == 1) {
        pbc_csr.port_10.cfg_oq_arb_l3_strict.priority_bypass_timer(pbc_csr.port_10.cfg_oq_arb_l3_strict.priority_bypass_timer().convert_to<int>() | 1 << l2_node_idx);
      }
    }

    pbc_csr.port_10.cfg_oq_arb_l3_strict.write();

 //   pbc_csr.port_11.cfg_oq_arb_l3_selection.all(0);
 // for(int l2_node_idx=0; l2_node_idx<4; l2_node_idx++) {
 //   if(l3_node[11][l2_node_idx] == 0) {
 //     pbc_csr.port_11.cfg_oq_arb_l3_selection.node_0(pbc_csr.port_11.cfg_oq_arb_l3_selection.node_0().convert_to<int>() | 1 << l2_node_idx);
 //   }
 // }
 // pbc_csr.port_11.cfg_oq_arb_l3_selection.write();
     
    pbc_csr.port_11.cfg_oq_arb_l3_strict.all(0);
    for(int l2_node_idx=0; l2_node_idx<4; l2_node_idx++) {
      if(l3_is_strict[11][l2_node_idx] == 1) {
        pbc_csr.port_11.cfg_oq_arb_l3_strict.priority(pbc_csr.port_11.cfg_oq_arb_l3_strict.priority().convert_to<int>() | 1 << l2_node_idx);
      }
      if(l3_priority_bypass_timer [11][l2_node_idx] == 1) {
        pbc_csr.port_11.cfg_oq_arb_l3_strict.priority_bypass_timer(pbc_csr.port_11.cfg_oq_arb_l3_strict.priority_bypass_timer().convert_to<int>() | 1 << l2_node_idx);
      }
    }

    pbc_csr.port_11.cfg_oq_arb_l3_strict.write();

     // read to flush writes
     pbc_csr.cfg_island_control.read();
     PLOG_MSG("done configuring scheduler \n");

     cpu::access()->set_access_type(cur_cpu_access_type);

}

void cap_pb_cfg_sched_node(int chip_id, int inst_id, int level, int oport, int queue_node, int quota) {

  cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);

  if(level < 1 || level > 3) {
    PLOG_ERR("cap_pb_cfg_sched_node :: Invalid level : " << level << endl)
    return;
  }

  int max_queues = 32;
  if(level == 1) {
    max_queues = 32;
  } else if(level == 2) {
    max_queues = 16;
  } else {
    max_queues = 4;
  }

  PLOG("pb_api", "cap_pb_cfg_sched_node :: level : " << level << " oport : " << oport << " queue_node : " << queue_node << " quota : " << quota << endl)
  pbc_csr.cfg_sched.enable_wrr(1);
  pbc_csr.cfg_sched.dhs_selection( (level-1)*2 );
  pbc_csr.cfg_sched.write();
  
  pbc_csr.cfg_dhs_mem.address(oport*max_queues + queue_node);
  pbc_csr.cfg_dhs_mem.write();
  
  pbc_csr.dhs_sched.entry[0].command(1);   //1: overwrite quota and credits
  pbc_csr.dhs_sched.entry[0].current_credit(quota);
  pbc_csr.dhs_sched.entry[0].quota(quota);
  pbc_csr.dhs_sched.entry[0].write();
  
}

void cap_pb_read_sched_node(int chip_id, int inst_id, int level, int oport, int queue_node, int & quota, int & current_credit) {

  cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);

 if(level < 1 || level > 3) {
    PLOG_ERR("cap_pb_read_sched_node :: Invalid level : " << level << endl)
    return;
  }

  int max_queues = 32;
  if(level == 1) {
    max_queues = 32;
  } else if(level == 2) {
    max_queues = 16;
  } else {
    max_queues = 4;
  }

  pbc_csr.cfg_sched.dhs_selection( (level-1)*2 );
  pbc_csr.cfg_sched.write();
  
  pbc_csr.cfg_dhs_mem.address(oport*max_queues + queue_node);
  pbc_csr.cfg_dhs_mem.write();
  
  pbc_csr.dhs_sched.entry[0].read();
  quota         = pbc_csr.dhs_sched.entry[0].quota().convert_to<int>();
  current_credit = pbc_csr.dhs_sched.entry[0].current_credit().convert_to<int>();

}

void cap_pb_read_port_mon_in(int chip_id, int inst_id, int addr, cpp_int & rval) {

  //PLOG_MSG("cap_pb_read_port_mon_in: addr: " << addr << endl);

  cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);

  // pbc_csr.cfg_port_mon_in.read();
  // pbc_csr.cfg_port_mon_in.dhs_selection(0x1);
  // pbc_csr.cfg_port_mon_in.write();
  pbc_csr.cfg_dhs_mem.address(addr);
  pbc_csr.cfg_dhs_mem.write();

  pbc_csr.dhs_port_mon_in.entry.read();
  rval = pbc_csr.dhs_port_mon_in.entry.all();
}

void cap_pb_read_port_mon_out(int chip_id, int inst_id, int addr, cpp_int & rval) {

  //PLOG_MSG("cap_pb_read_port_mon_out: addr: " << addr << endl);

  cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);

  // pbc_csr.cfg_port_mon_out.read();
  // pbc_csr.cfg_port_mon_out.dhs_selection(0x1);
  // pbc_csr.cfg_port_mon_out.write();
  pbc_csr.cfg_dhs_mem.address(addr);
  pbc_csr.cfg_dhs_mem.write();

  pbc_csr.dhs_port_mon_out.entry.read();
  rval = pbc_csr.dhs_port_mon_out.entry.all();
}

void cap_pb_write_port_mon_in(int chip_id, int inst_id, int addr, cpp_int wval) {

  PLOG_MSG("cap_pb_write_port_mon_in: addr: " << addr << ", wval: " << wval << endl);

  cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);

  pbc_csr.cfg_port_mon_in.read();
  pbc_csr.cfg_port_mon_in.dhs_selection(0x1);
  pbc_csr.cfg_port_mon_in.write();
  pbc_csr.cfg_dhs_mem.address(addr);
  pbc_csr.cfg_dhs_mem.write();

  pbc_csr.dhs_port_mon_in.entry.all(wval);
  pbc_csr.dhs_port_mon_in.entry.write();
}

void cap_pb_write_port_mon_out(int chip_id, int inst_id, int addr, cpp_int wval) {

  PLOG_MSG("cap_pb_write_port_mon_out: addr: " << addr << ", wval: " << wval << endl);

  cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);

  pbc_csr.cfg_port_mon_out.read();
  pbc_csr.cfg_port_mon_out.dhs_selection(0x1);
  pbc_csr.cfg_port_mon_out.write();
  pbc_csr.cfg_dhs_mem.address(addr);
  pbc_csr.cfg_dhs_mem.write();

  pbc_csr.dhs_port_mon_out.entry.all(wval);
  pbc_csr.dhs_port_mon_out.entry.write();
}

void cap_pb_read_flits_cnt(int chip_id, int inst_id, int hbm, int sel, int port, cpp_int & rval) {

  cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);

  if (hbm == 0) {
    pbc_csr.cnt_flits[port].read();
    switch (sel) {
      case 0: rval = pbc_csr.cnt_flits[port].sop_in(); break;
      case 1: rval = pbc_csr.cnt_flits[port].eop_in(); break;
      case 2: rval = pbc_csr.cnt_flits[port].sop_out(); break;
      case 3: rval = pbc_csr.cnt_flits[port].eop_out(); break;
      default: rval = 0; break;
    }
  } else {
    switch (port) {
      case 0: pbc_csr.hbm.hbm_port_0.cnt_hbm.read(); 
    	      switch (sel) {
                case 0: rval = pbc_csr.hbm.hbm_port_0.cnt_hbm.flits_sop_in(); break;
                case 1: rval = pbc_csr.hbm.hbm_port_0.cnt_hbm.flits_eop_in(); break;
                case 2: rval = pbc_csr.hbm.hbm_port_0.cnt_hbm.flits_sop_out(); break;
                case 3: rval = pbc_csr.hbm.hbm_port_0.cnt_hbm.flits_eop_out(); break;
                case 4: rval = pbc_csr.hbm.hbm_port_0.cnt_hbm.occupancy_drop(); break;
                default: rval = 0; break;
              }
	      break;
      case 1: pbc_csr.hbm.hbm_port_1.cnt_hbm.read(); 
    	      switch (sel) {
                case 0: rval = pbc_csr.hbm.hbm_port_1.cnt_hbm.flits_sop_in(); break;
                case 1: rval = pbc_csr.hbm.hbm_port_1.cnt_hbm.flits_eop_in(); break;
                case 2: rval = pbc_csr.hbm.hbm_port_1.cnt_hbm.flits_sop_out(); break;
                case 3: rval = pbc_csr.hbm.hbm_port_1.cnt_hbm.flits_eop_out(); break;
                case 4: rval = pbc_csr.hbm.hbm_port_1.cnt_hbm.occupancy_drop(); break;
                default: rval = 0; break;
              }
	      break;
      case 2: pbc_csr.hbm.hbm_port_2.cnt_hbm.read(); 
    	      switch (sel) {
                case 0: rval = pbc_csr.hbm.hbm_port_2.cnt_hbm.flits_sop_in(); break;
                case 1: rval = pbc_csr.hbm.hbm_port_2.cnt_hbm.flits_eop_in(); break;
                case 2: rval = pbc_csr.hbm.hbm_port_2.cnt_hbm.flits_sop_out(); break;
                case 3: rval = pbc_csr.hbm.hbm_port_2.cnt_hbm.flits_eop_out(); break;
                case 4: rval = pbc_csr.hbm.hbm_port_2.cnt_hbm.occupancy_drop(); break;
                default: rval = 0; break;
              }
	      break;
      case 3: pbc_csr.hbm.hbm_port_3.cnt_hbm.read(); 
    	      switch (sel) {
                case 0: rval = pbc_csr.hbm.hbm_port_3.cnt_hbm.flits_sop_in(); break;
                case 1: rval = pbc_csr.hbm.hbm_port_3.cnt_hbm.flits_eop_in(); break;
                case 2: rval = pbc_csr.hbm.hbm_port_3.cnt_hbm.flits_sop_out(); break;
                case 3: rval = pbc_csr.hbm.hbm_port_3.cnt_hbm.flits_eop_out(); break;
                case 4: rval = pbc_csr.hbm.hbm_port_3.cnt_hbm.occupancy_drop(); break;
                default: rval = 0; break;
              }
	      break;
      case 4: pbc_csr.hbm.hbm_port_4.cnt_hbm.read(); 
    	      switch (sel) {
                case 0: rval = pbc_csr.hbm.hbm_port_4.cnt_hbm.flits_sop_in(); break;
                case 1: rval = pbc_csr.hbm.hbm_port_4.cnt_hbm.flits_eop_in(); break;
                case 2: rval = pbc_csr.hbm.hbm_port_4.cnt_hbm.flits_sop_out(); break;
                case 3: rval = pbc_csr.hbm.hbm_port_4.cnt_hbm.flits_eop_out(); break;
                case 4: rval = pbc_csr.hbm.hbm_port_4.cnt_hbm.occupancy_drop(); break;
                default: rval = 0; break;
              }
	      break;
      case 5: pbc_csr.hbm.hbm_port_5.cnt_hbm.read(); 
    	      switch (sel) {
                case 0: rval = pbc_csr.hbm.hbm_port_5.cnt_hbm.flits_sop_in(); break;
                case 1: rval = pbc_csr.hbm.hbm_port_5.cnt_hbm.flits_eop_in(); break;
                case 2: rval = pbc_csr.hbm.hbm_port_5.cnt_hbm.flits_sop_out(); break;
                case 3: rval = pbc_csr.hbm.hbm_port_5.cnt_hbm.flits_eop_out(); break;
                case 4: rval = pbc_csr.hbm.hbm_port_5.cnt_hbm.occupancy_drop(); break;
                default: rval = 0; break;
              }
	      break;
      case 6: pbc_csr.hbm.hbm_port_6.cnt_hbm.read(); 
    	      switch (sel) {
                case 0: rval = pbc_csr.hbm.hbm_port_6.cnt_hbm.flits_sop_in(); break;
                case 1: rval = pbc_csr.hbm.hbm_port_6.cnt_hbm.flits_eop_in(); break;
                case 2: rval = pbc_csr.hbm.hbm_port_6.cnt_hbm.flits_sop_out(); break;
                case 3: rval = pbc_csr.hbm.hbm_port_6.cnt_hbm.flits_eop_out(); break;
                case 4: rval = pbc_csr.hbm.hbm_port_6.cnt_hbm.occupancy_drop(); break;
                default: rval = 0; break;
              }
	      break;
      case 7: pbc_csr.hbm.hbm_port_7.cnt_hbm.read(); 
    	      switch (sel) {
                case 0: rval = pbc_csr.hbm.hbm_port_7.cnt_hbm.flits_sop_in(); break;
                case 1: rval = pbc_csr.hbm.hbm_port_7.cnt_hbm.flits_eop_in(); break;
                case 2: rval = pbc_csr.hbm.hbm_port_7.cnt_hbm.flits_sop_out(); break;
                case 3: rval = pbc_csr.hbm.hbm_port_7.cnt_hbm.flits_eop_out(); break;
                case 4: rval = pbc_csr.hbm.hbm_port_7.cnt_hbm.occupancy_drop(); break;
                default: rval = 0; break;
              }
	      break;
      case 8: pbc_csr.hbm.hbm_port_8.cnt_hbm.read(); 
    	      switch (sel) {
                case 0: rval = pbc_csr.hbm.hbm_port_8.cnt_hbm.flits_sop_in(); break;
                case 1: rval = pbc_csr.hbm.hbm_port_8.cnt_hbm.flits_eop_in(); break;
                case 2: rval = pbc_csr.hbm.hbm_port_8.cnt_hbm.flits_sop_out(); break;
                case 3: rval = pbc_csr.hbm.hbm_port_8.cnt_hbm.flits_eop_out(); break;
                case 4: rval = pbc_csr.hbm.hbm_port_8.cnt_hbm.occupancy_drop(); break;
                default: rval = 0; break;
              }
	      break;
      default: rval = 0; break;
    }
  }
}

void cap_pb_read_emergency_stop_drop_count(int chip_id, int inst_id, int hbm, cpp_int & rval) {

  cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);

  rval = 0;
  switch(hbm) {
      case 1: pbc_csr.hbm.cnt_hbm1_emergency_stop.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm1_emergency_stop.drop(); 
      	         break;
      case 2: pbc_csr.hbm.cnt_hbm2_emergency_stop.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm2_emergency_stop.drop(); 
      	         break;
      case 3: pbc_csr.hbm.cnt_hbm3_emergency_stop.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm3_emergency_stop.drop(); 
      	         break;
      case 4: pbc_csr.hbm.cnt_hbm4_emergency_stop.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm4_emergency_stop.drop(); 
      	         break;
      case 5: pbc_csr.hbm.cnt_hbm5_emergency_stop.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm5_emergency_stop.drop(); 
      	         break;
      case 6: pbc_csr.hbm.cnt_hbm6_emergency_stop.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm6_emergency_stop.drop(); 
      	         break;
      case 7: pbc_csr.hbm.cnt_hbm7_emergency_stop.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm7_emergency_stop.drop(); 
      	         break;
      case 8: pbc_csr.hbm.cnt_hbm8_emergency_stop.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm8_emergency_stop.drop(); 
      	         break;
      default:   pbc_csr.hbm.cnt_hbm0_emergency_stop.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm0_emergency_stop.drop(); 
      	         break;
    }
}

void cap_pb_read_write_ack_filling_up_drop_count(int chip_id, int inst_id, int hbm, cpp_int & rval) {

  cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);

  rval = 0;
  switch(hbm) {
      case 1: pbc_csr.hbm.cnt_hbm1_write_ack_filling_up.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm1_write_ack_filling_up.drop(); 
      	         break;
      case 2: pbc_csr.hbm.cnt_hbm2_write_ack_filling_up.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm2_write_ack_filling_up.drop(); 
      	         break;
      case 3: pbc_csr.hbm.cnt_hbm3_write_ack_filling_up.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm3_write_ack_filling_up.drop(); 
      	         break;
      case 4: pbc_csr.hbm.cnt_hbm4_write_ack_filling_up.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm4_write_ack_filling_up.drop(); 
      	         break;
      case 5: pbc_csr.hbm.cnt_hbm5_write_ack_filling_up.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm5_write_ack_filling_up.drop(); 
      	         break;
      case 6: pbc_csr.hbm.cnt_hbm6_write_ack_filling_up.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm6_write_ack_filling_up.drop(); 
      	         break;
      case 7: pbc_csr.hbm.cnt_hbm7_write_ack_filling_up.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm7_write_ack_filling_up.drop(); 
      	         break;
      case 8: pbc_csr.hbm.cnt_hbm8_write_ack_filling_up.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm8_write_ack_filling_up.drop(); 
      	         break;
      default:   pbc_csr.hbm.cnt_hbm0_write_ack_filling_up.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm0_write_ack_filling_up.drop(); 
      	         break;
    }
}

void cap_pb_read_write_ack_full_drop_count(int chip_id, int inst_id, int hbm, cpp_int & rval) {

  cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);

  rval = 0;
  switch(hbm) {
      case 1: pbc_csr.hbm.cnt_hbm1_write_ack_full.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm1_write_ack_full.drop(); 
      	         break;
      case 2: pbc_csr.hbm.cnt_hbm2_write_ack_full.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm2_write_ack_full.drop(); 
      	         break;
      case 3: pbc_csr.hbm.cnt_hbm3_write_ack_full.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm3_write_ack_full.drop(); 
      	         break;
      case 4: pbc_csr.hbm.cnt_hbm4_write_ack_full.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm4_write_ack_full.drop(); 
      	         break;
      case 5: pbc_csr.hbm.cnt_hbm5_write_ack_full.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm5_write_ack_full.drop(); 
      	         break;
      case 6: pbc_csr.hbm.cnt_hbm6_write_ack_full.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm6_write_ack_full.drop(); 
      	         break;
      case 7: pbc_csr.hbm.cnt_hbm7_write_ack_full.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm7_write_ack_full.drop(); 
      	         break;
      case 8: pbc_csr.hbm.cnt_hbm8_write_ack_full.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm8_write_ack_full.drop(); 
      	         break;
      default:   pbc_csr.hbm.cnt_hbm0_write_ack_full.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm0_write_ack_full.drop(); 
      	         break;
    }
}

void cap_pb_read_truncate_drop_count(int chip_id, int inst_id, int hbm, cpp_int & rval) {

  cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);

  rval = 0;
  switch(hbm) {
      case 1: pbc_csr.hbm.cnt_hbm1_truncate.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm1_truncate.drop(); 
      	         break;
      case 2: pbc_csr.hbm.cnt_hbm2_truncate.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm2_truncate.drop(); 
      	         break;
      case 3: pbc_csr.hbm.cnt_hbm3_truncate.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm3_truncate.drop(); 
      	         break;
      case 4: pbc_csr.hbm.cnt_hbm4_truncate.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm4_truncate.drop(); 
      	         break;
      case 5: pbc_csr.hbm.cnt_hbm5_truncate.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm5_truncate.drop(); 
      	         break;
      case 6: pbc_csr.hbm.cnt_hbm6_truncate.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm6_truncate.drop(); 
      	         break;
      case 7: pbc_csr.hbm.cnt_hbm7_truncate.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm7_truncate.drop(); 
      	         break;
      case 8: pbc_csr.hbm.cnt_hbm8_truncate.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm8_truncate.drop(); 
      	         break;
      default:   pbc_csr.hbm.cnt_hbm0_truncate.read(); 
      		 rval = pbc_csr.hbm.cnt_hbm0_truncate.drop(); 
      	         break;
    }
}

void cap_pb_read_ctrl_full_drop_count(int chip_id, int inst_id, int hbm, cpp_int & rval) {

  cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);

  rval = 0;
  switch(hbm) {
      case 1: pbc_csr.hbm.sat_hbm1_ctrl_full.read(); 
      		 rval = pbc_csr.hbm.sat_hbm1_ctrl_full.drop(); 
      	         break;
      case 2: pbc_csr.hbm.sat_hbm2_ctrl_full.read(); 
      		 rval = pbc_csr.hbm.sat_hbm2_ctrl_full.drop(); 
      	         break;
      case 3: pbc_csr.hbm.sat_hbm3_ctrl_full.read(); 
      		 rval = pbc_csr.hbm.sat_hbm3_ctrl_full.drop(); 
      	         break;
      case 4: pbc_csr.hbm.sat_hbm4_ctrl_full.read(); 
      		 rval = pbc_csr.hbm.sat_hbm4_ctrl_full.drop(); 
      	         break;
      case 5: pbc_csr.hbm.sat_hbm5_ctrl_full.read(); 
      		 rval = pbc_csr.hbm.sat_hbm5_ctrl_full.drop(); 
      	         break;
      case 6: pbc_csr.hbm.sat_hbm6_ctrl_full.read(); 
      		 rval = pbc_csr.hbm.sat_hbm6_ctrl_full.drop(); 
      	         break;
      case 7: pbc_csr.hbm.sat_hbm7_ctrl_full.read(); 
      		 rval = pbc_csr.hbm.sat_hbm7_ctrl_full.drop(); 
      	         break;
      case 8: pbc_csr.hbm.sat_hbm8_ctrl_full.read(); 
      		 rval = pbc_csr.hbm.sat_hbm8_ctrl_full.drop(); 
      	         break;
      default:   pbc_csr.hbm.sat_hbm0_ctrl_full.read(); 
      		 rval = pbc_csr.hbm.sat_hbm0_ctrl_full.drop(); 
      	         break;
    }
}

void cap_pb_write_flits_cnt(int chip_id, int inst_id, int hbm, int sel, int port, cpp_int wval) {

  cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);

  if (hbm == 0) {
    pbc_csr.cnt_flits[port].read();
    switch (sel) {
      case 0: pbc_csr.cnt_flits[port].sop_in(wval); break;
      case 1: pbc_csr.cnt_flits[port].eop_in(wval); break;
      case 2: pbc_csr.cnt_flits[port].sop_out(wval); break;
      case 3: pbc_csr.cnt_flits[port].eop_out(wval); break;
      default: break;
    }
    pbc_csr.cnt_flits[port].write();
  } else {
    switch (port) {
      case 0: pbc_csr.hbm.hbm_port_0.cnt_hbm.read(); 
    	      switch (sel) {
                case 0: pbc_csr.hbm.hbm_port_0.cnt_hbm.flits_sop_in(wval); break;
                case 1: pbc_csr.hbm.hbm_port_0.cnt_hbm.flits_eop_in(wval); break;
                case 2: pbc_csr.hbm.hbm_port_0.cnt_hbm.flits_sop_out(wval); break;
                case 3: pbc_csr.hbm.hbm_port_0.cnt_hbm.flits_eop_out(wval); break;
                case 4: pbc_csr.hbm.hbm_port_0.cnt_hbm.occupancy_drop(wval); break;
                default: break;
              }
	      pbc_csr.hbm.hbm_port_0.cnt_hbm.write();
	      break;
      case 1: pbc_csr.hbm.hbm_port_1.cnt_hbm.read(); 
    	      switch (sel) {
                case 0: pbc_csr.hbm.hbm_port_1.cnt_hbm.flits_sop_in(wval); break;
                case 1: pbc_csr.hbm.hbm_port_1.cnt_hbm.flits_eop_in(wval); break;
                case 2: pbc_csr.hbm.hbm_port_1.cnt_hbm.flits_sop_out(wval); break;
                case 3: pbc_csr.hbm.hbm_port_1.cnt_hbm.flits_eop_out(wval); break;
                case 4: pbc_csr.hbm.hbm_port_1.cnt_hbm.occupancy_drop(wval); break;
                default: break;
              }
	      pbc_csr.hbm.hbm_port_1.cnt_hbm.write();
	      break;
      case 2: pbc_csr.hbm.hbm_port_2.cnt_hbm.read(); 
    	      switch (sel) {
                case 0: pbc_csr.hbm.hbm_port_2.cnt_hbm.flits_sop_in(wval); break;
                case 1: pbc_csr.hbm.hbm_port_2.cnt_hbm.flits_eop_in(wval); break;
                case 2: pbc_csr.hbm.hbm_port_2.cnt_hbm.flits_sop_out(wval); break;
                case 3: pbc_csr.hbm.hbm_port_2.cnt_hbm.flits_eop_out(wval); break;
                case 4: pbc_csr.hbm.hbm_port_2.cnt_hbm.occupancy_drop(wval); break;
                default: break;
              }
	      pbc_csr.hbm.hbm_port_2.cnt_hbm.write();
	      break;
      case 3: pbc_csr.hbm.hbm_port_3.cnt_hbm.read(); 
    	      switch (sel) {
                case 0: pbc_csr.hbm.hbm_port_3.cnt_hbm.flits_sop_in(wval); break;
                case 1: pbc_csr.hbm.hbm_port_3.cnt_hbm.flits_eop_in(wval); break;
                case 2: pbc_csr.hbm.hbm_port_3.cnt_hbm.flits_sop_out(wval); break;
                case 3: pbc_csr.hbm.hbm_port_3.cnt_hbm.flits_eop_out(wval); break;
                case 4: pbc_csr.hbm.hbm_port_3.cnt_hbm.occupancy_drop(wval); break;
                default: break;
              }
	      pbc_csr.hbm.hbm_port_3.cnt_hbm.write();
	      break;
      case 4: pbc_csr.hbm.hbm_port_4.cnt_hbm.read(); 
    	      switch (sel) {
                case 0: pbc_csr.hbm.hbm_port_4.cnt_hbm.flits_sop_in(wval); break;
                case 1: pbc_csr.hbm.hbm_port_4.cnt_hbm.flits_eop_in(wval); break;
                case 2: pbc_csr.hbm.hbm_port_4.cnt_hbm.flits_sop_out(wval); break;
                case 3: pbc_csr.hbm.hbm_port_4.cnt_hbm.flits_eop_out(wval); break;
                case 4: pbc_csr.hbm.hbm_port_4.cnt_hbm.occupancy_drop(wval); break;
                default: break;
              }
	      pbc_csr.hbm.hbm_port_4.cnt_hbm.write();
	      break;
      case 5: pbc_csr.hbm.hbm_port_5.cnt_hbm.read(); 
    	      switch (sel) {
                case 0: pbc_csr.hbm.hbm_port_5.cnt_hbm.flits_sop_in(wval); break;
                case 1: pbc_csr.hbm.hbm_port_5.cnt_hbm.flits_eop_in(wval); break;
                case 2: pbc_csr.hbm.hbm_port_5.cnt_hbm.flits_sop_out(wval); break;
                case 3: pbc_csr.hbm.hbm_port_5.cnt_hbm.flits_eop_out(wval); break;
                case 4: pbc_csr.hbm.hbm_port_5.cnt_hbm.occupancy_drop(wval); break;
                default: break;
              }
	      pbc_csr.hbm.hbm_port_5.cnt_hbm.write();
	      break;
      case 6: pbc_csr.hbm.hbm_port_6.cnt_hbm.read(); 
    	      switch (sel) {
                case 0: pbc_csr.hbm.hbm_port_6.cnt_hbm.flits_sop_in(wval); break;
                case 1: pbc_csr.hbm.hbm_port_6.cnt_hbm.flits_eop_in(wval); break;
                case 2: pbc_csr.hbm.hbm_port_6.cnt_hbm.flits_sop_out(wval); break;
                case 3: pbc_csr.hbm.hbm_port_6.cnt_hbm.flits_eop_out(wval); break;
                case 4: pbc_csr.hbm.hbm_port_6.cnt_hbm.occupancy_drop(wval); break;
                default: break;
              }
	      pbc_csr.hbm.hbm_port_6.cnt_hbm.write();
	      break;
      case 7: pbc_csr.hbm.hbm_port_7.cnt_hbm.read(); 
    	      switch (sel) {
                case 0: pbc_csr.hbm.hbm_port_7.cnt_hbm.flits_sop_in(wval); break;
                case 1: pbc_csr.hbm.hbm_port_7.cnt_hbm.flits_eop_in(wval); break;
                case 2: pbc_csr.hbm.hbm_port_7.cnt_hbm.flits_sop_out(wval); break;
                case 3: pbc_csr.hbm.hbm_port_7.cnt_hbm.flits_eop_out(wval); break;
                case 4: pbc_csr.hbm.hbm_port_7.cnt_hbm.occupancy_drop(wval); break;
                default: break;
              }
	      pbc_csr.hbm.hbm_port_7.cnt_hbm.write();
	      break;
      case 8: pbc_csr.hbm.hbm_port_8.cnt_hbm.read(); 
    	      switch (sel) {
                case 0: pbc_csr.hbm.hbm_port_8.cnt_hbm.flits_sop_in(wval); break;
                case 1: pbc_csr.hbm.hbm_port_8.cnt_hbm.flits_eop_in(wval); break;
                case 2: pbc_csr.hbm.hbm_port_8.cnt_hbm.flits_sop_out(wval); break;
                case 3: pbc_csr.hbm.hbm_port_8.cnt_hbm.flits_eop_out(wval); break;
                case 4: pbc_csr.hbm.hbm_port_8.cnt_hbm.occupancy_drop(wval); break;
                default: break;
              }
	      pbc_csr.hbm.hbm_port_8.cnt_hbm.write();
	      break;
      default: break;
    }
  }
}

void cap_pb_wait_port_mon_init_done(int chip_id, int inst_id) {

  cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);

  do {
    pbc_csr.sta_port_mon_in.read();
  } while (pbc_csr.sta_port_mon_in.init_done() == 0);

  do {
    pbc_csr.sta_port_mon_out.read();
  } while (pbc_csr.sta_port_mon_out.init_done() == 0);
}

void cap_pb_get_write_error_cnt(int chip_id, int inst_id, int port, int sel, cpp_int & rval) {

  cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
  
  pbc_csr.sat_write_error[port].read();
  switch (sel) {
    case 0: rval = pbc_csr.sat_write_error[port].discarded();
	    break;
    case 1: rval = pbc_csr.sat_write_error[port].admitted();
	    break;
    case 2: rval = pbc_csr.sat_write_error[port].intrinsic_drop();
	    break;
    case 3: rval = pbc_csr.sat_write_error[port].out_of_cells();
	    break;
    case 4: rval = pbc_csr.sat_write_error[port].out_of_credit();
	    break;
    case 5: rval = pbc_csr.sat_write_error[port].truncation();
	    break;
    case 6: rval = pbc_csr.sat_write_error[port].port_disabled();
	    break;
    case 7: rval = pbc_csr.sat_write_error[port].out_of_cells1();
	    break;
    case 8: rval = pbc_csr.sat_write_error[port].tail_drop_cpu();
	    break;
    case 9: rval = pbc_csr.sat_write_error[port].tail_drop_span();
	    break;
    case 10: rval = pbc_csr.sat_write_error[port].min_size_viol();
	     break;
    case 11: rval = pbc_csr.sat_write_error[port].enqueue();
	     break;
    case 12: rval = pbc_csr.sat_write_error[port].port_range();
	     break;
    case 13: rval = pbc_csr.sat_write_error[port].oq_range();
	     break;
    default: break;
  }
}

void cap_pb_port_enable(int chip_id, int inst_id, int port, int value) {
   cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
   int enable = value & 0x1;

   PLOG_MSG("cap_pb_port_enable: port_num : " << port << " enable : " << enable << endl);

  if(port == 0) {
    pbc_csr.port_0.cfg_oq.enable(enable);
    pbc_csr.port_0.cfg_oq.write();
  }
  if(port == 1) {
    pbc_csr.port_1.cfg_oq.enable(enable);
    pbc_csr.port_1.cfg_oq.write();
  }
  if(port == 2) {
    pbc_csr.port_2.cfg_oq.enable(enable);
    pbc_csr.port_2.cfg_oq.write();
  }
  if(port == 3) {
    pbc_csr.port_3.cfg_oq.enable(enable);
    pbc_csr.port_3.cfg_oq.write();
  }
  if(port == 4) {
    pbc_csr.port_4.cfg_oq.enable(enable);
    pbc_csr.port_4.cfg_oq.write();
  }
  if(port == 5) {
    pbc_csr.port_5.cfg_oq.enable(enable);
    pbc_csr.port_5.cfg_oq.write();
  }
  if(port == 6) {
    pbc_csr.port_6.cfg_oq.enable(enable);
    pbc_csr.port_6.cfg_oq.write();
  }
  if(port == 7) {
    pbc_csr.port_7.cfg_oq.enable(enable);
    pbc_csr.port_7.cfg_oq.write();
  }
  if(port == 8) {
    pbc_csr.port_8.cfg_oq.enable(enable);
    pbc_csr.port_8.cfg_oq.write();
  }
  if(port == 9) {
    pbc_csr.port_9.cfg_oq.enable(enable);
    pbc_csr.port_9.cfg_oq.write();
  }
  if(port == 10) {
    pbc_csr.port_10.cfg_oq.enable(enable);
    pbc_csr.port_10.cfg_oq.write();
  }
  if(port == 11) {
    pbc_csr.port_11.cfg_oq.enable(enable);
    pbc_csr.port_11.cfg_oq.write();
  }
}

void cap_pb_port_enable_toggle(int chip_id, int inst_id, int port, int down_time_ns, int up_time_ns) {
   cap_pb_port_enable(chip_id, inst_id, port, 0);
   SLEEP(down_time_ns);
   cap_pb_port_enable(chip_id, inst_id, port, 1);
   SLEEP(up_time_ns);
}

void cap_pb_port_flush(int chip_id, int inst_id, int port, int value) {
   cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
   int flush = value & 0x1;
   if (port == 0) {
      pbc_csr.port_0.cfg_oq.read();
      pbc_csr.port_0.cfg_oq.flush(flush);
      pbc_csr.port_0.cfg_oq.write();
      if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_0.cfg_oq.show();}
   } else if (port == 1) {
      pbc_csr.port_1.cfg_oq.read();
      pbc_csr.port_1.cfg_oq.flush(flush);
      pbc_csr.port_1.cfg_oq.write();
      if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_1.cfg_oq.show();}
   } else if (port == 2) {
      pbc_csr.port_2.cfg_oq.read();
      pbc_csr.port_2.cfg_oq.flush(flush);
      pbc_csr.port_2.cfg_oq.write();
      if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_2.cfg_oq.show();}
   } else if (port == 3) {
      pbc_csr.port_3.cfg_oq.read();
      pbc_csr.port_3.cfg_oq.flush(flush);
      pbc_csr.port_3.cfg_oq.write();
      if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_3.cfg_oq.show();}
   } else if (port == 4) {
      pbc_csr.port_4.cfg_oq.read();
      pbc_csr.port_4.cfg_oq.flush(flush);
      pbc_csr.port_4.cfg_oq.write();
      if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_4.cfg_oq.show();}
   } else if (port == 5) {
      pbc_csr.port_5.cfg_oq.read();
      pbc_csr.port_5.cfg_oq.flush(flush);
      pbc_csr.port_5.cfg_oq.write();
      if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_5.cfg_oq.show();}
   } else if (port == 6) {
      pbc_csr.port_6.cfg_oq.read();
      pbc_csr.port_6.cfg_oq.flush(flush);
      pbc_csr.port_6.cfg_oq.write();
      if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_6.cfg_oq.show();}
   } else if (port == 7) {
      pbc_csr.port_7.cfg_oq.read();
      pbc_csr.port_7.cfg_oq.flush(flush);
      pbc_csr.port_7.cfg_oq.write();
      if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_7.cfg_oq.show();}
   } else if (port == 8) {
      pbc_csr.port_8.cfg_oq.read();
      pbc_csr.port_8.cfg_oq.flush(flush);
      pbc_csr.port_8.cfg_oq.write();
      if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_8.cfg_oq.show();}
   }
}
void cap_pb_pcos_flush(int chip_id, int inst_id, int port, int cos, int value) {
   cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
   int xoff2oq_val;
   int oq_flush;

   if (port == 0) {
       pbc_csr.port_0.cfg_oq_queue.read();
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_0.cfg_oq_queue.show();}
       oq_flush = pbc_csr.port_0.cfg_oq_queue.flush().convert_to<int>();

       pbc_csr.port_0.cfg_oq_xoff2oq.read();
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_0.cfg_oq_xoff2oq.show();}
       xoff2oq_val = pbc_csr.port_0.cfg_oq_xoff2oq.map().convert_to<int>();

       for (int i = 0; i < 32; i++) {
           if ((xoff2oq_val & 0x7) == cos) {
               if (value == 1) {
                   oq_flush = oq_flush | (1 << i);
               }
               else if (value == 0) {
                   oq_flush = oq_flush & (~(1 << i));
               }
           }
           xoff2oq_val = xoff2oq_val >> 3;
       }
       pbc_csr.port_0.cfg_oq_queue.flush(oq_flush);
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_0.cfg_oq_queue.show();}
       pbc_csr.port_0.cfg_oq_queue.write();
   }
   if (port == 1) {
       pbc_csr.port_1.cfg_oq_queue.read();
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_1.cfg_oq_queue.show();}
       oq_flush = pbc_csr.port_1.cfg_oq_queue.flush().convert_to<int>();

       pbc_csr.port_1.cfg_oq_xoff2oq.read();
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_1.cfg_oq_xoff2oq.show();}
       xoff2oq_val = pbc_csr.port_1.cfg_oq_xoff2oq.map().convert_to<int>();

       for (int i = 0; i < 32; i++) {
           if ((xoff2oq_val & 0x7) == cos) {
               if (value == 1) {
                   oq_flush = oq_flush | (1 << i);
               }
               else if (value == 0) {
                   oq_flush = oq_flush & (~(1 << i));
               }
           }
           xoff2oq_val = xoff2oq_val >> 3;
       }
       pbc_csr.port_1.cfg_oq_queue.flush(oq_flush);
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_1.cfg_oq_queue.show();}
       pbc_csr.port_1.cfg_oq_queue.write();
   }
   if (port == 2) {
       pbc_csr.port_2.cfg_oq_queue.read();
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_2.cfg_oq_queue.show();}
       oq_flush = pbc_csr.port_2.cfg_oq_queue.flush().convert_to<int>();

       pbc_csr.port_2.cfg_oq_xoff2oq.read();
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_2.cfg_oq_xoff2oq.show();}
       xoff2oq_val = pbc_csr.port_2.cfg_oq_xoff2oq.map().convert_to<int>();

       for (int i = 0; i < 32; i++) {
           if ((xoff2oq_val & 0x7) == cos) {
               if (value == 1) {
                   oq_flush = oq_flush | (1 << i);
               }
               else if (value == 0) {
                   oq_flush = oq_flush & (~(1 << i));
               }
           }
           xoff2oq_val = xoff2oq_val >> 3;
       }
       pbc_csr.port_2.cfg_oq_queue.flush(oq_flush);
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_2.cfg_oq_queue.show();}
       pbc_csr.port_2.cfg_oq_queue.write();
   }
   if (port == 3) {
       pbc_csr.port_3.cfg_oq_queue.read();
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_3.cfg_oq_queue.show();}
       oq_flush = pbc_csr.port_3.cfg_oq_queue.flush().convert_to<int>();

       pbc_csr.port_3.cfg_oq_xoff2oq.read();
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_3.cfg_oq_xoff2oq.show();}
       xoff2oq_val = pbc_csr.port_3.cfg_oq_xoff2oq.map().convert_to<int>();

       for (int i = 0; i < 32; i++) {
           if ((xoff2oq_val & 0x7) == cos) {
               if (value == 1) {
                   oq_flush = oq_flush | (1 << i);
               }
               else if (value == 0) {
                   oq_flush = oq_flush & (~(1 << i));
               }
           }
           xoff2oq_val = xoff2oq_val >> 3;
       }
       pbc_csr.port_3.cfg_oq_queue.flush(oq_flush);
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_3.cfg_oq_queue.show();}
       pbc_csr.port_3.cfg_oq_queue.write();
   }
   if (port == 4) {
       pbc_csr.port_4.cfg_oq_queue.read();
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_4.cfg_oq_queue.show();}
       oq_flush = pbc_csr.port_4.cfg_oq_queue.flush().convert_to<int>();

       pbc_csr.port_4.cfg_oq_xoff2oq.read();
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_4.cfg_oq_xoff2oq.show();}
       xoff2oq_val = pbc_csr.port_4.cfg_oq_xoff2oq.map().convert_to<int>();

       for (int i = 0; i < 32; i++) {
           if ((xoff2oq_val & 0x7) == cos) {
               if (value == 1) {
                   oq_flush = oq_flush | (1 << i);
               }
               else if (value == 0) {
                   oq_flush = oq_flush & (~(1 << i));
               }
           }
           xoff2oq_val = xoff2oq_val >> 3;
       }
       pbc_csr.port_4.cfg_oq_queue.flush(oq_flush);
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_4.cfg_oq_queue.show();}
       pbc_csr.port_4.cfg_oq_queue.write();
   }
   if (port == 5) {
       pbc_csr.port_5.cfg_oq_queue.read();
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_5.cfg_oq_queue.show();}
       oq_flush = pbc_csr.port_5.cfg_oq_queue.flush().convert_to<int>();

       pbc_csr.port_5.cfg_oq_xoff2oq.read();
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_5.cfg_oq_xoff2oq.show();}
       xoff2oq_val = pbc_csr.port_5.cfg_oq_xoff2oq.map().convert_to<int>();

       for (int i = 0; i < 32; i++) {
           if ((xoff2oq_val & 0x7) == cos) {
               if (value == 1) {
                   oq_flush = oq_flush | (1 << i);
               }
               else if (value == 0) {
                   oq_flush = oq_flush & (~(1 << i));
               }
           }
           xoff2oq_val = xoff2oq_val >> 3;
       }
       pbc_csr.port_5.cfg_oq_queue.flush(oq_flush);
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_5.cfg_oq_queue.show();}
       pbc_csr.port_5.cfg_oq_queue.write();
   }
   if (port == 6) {
       pbc_csr.port_6.cfg_oq_queue.read();
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_6.cfg_oq_queue.show();}
       oq_flush = pbc_csr.port_6.cfg_oq_queue.flush().convert_to<int>();

       pbc_csr.port_6.cfg_oq_xoff2oq.read();
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_6.cfg_oq_xoff2oq.show();}
       xoff2oq_val = pbc_csr.port_6.cfg_oq_xoff2oq.map().convert_to<int>();

       for (int i = 0; i < 32; i++) {
           if ((xoff2oq_val & 0x7) == cos) {
               if (value == 1) {
                   oq_flush = oq_flush | (1 << i);
               }
               else if (value == 0) {
                   oq_flush = oq_flush & (~(1 << i));
               }
           }
           xoff2oq_val = xoff2oq_val >> 3;
       }
       pbc_csr.port_6.cfg_oq_queue.flush(oq_flush);
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_6.cfg_oq_queue.show();}
       pbc_csr.port_6.cfg_oq_queue.write();
   }
   if (port == 7) {
       pbc_csr.port_7.cfg_oq_queue.read();
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_7.cfg_oq_queue.show();}
       oq_flush = pbc_csr.port_7.cfg_oq_queue.flush().convert_to<int>();

       pbc_csr.port_7.cfg_oq_xoff2oq.read();
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_7.cfg_oq_xoff2oq.show();}
       xoff2oq_val = pbc_csr.port_7.cfg_oq_xoff2oq.map().convert_to<int>();

       for (int i = 0; i < 32; i++) {
           if ((xoff2oq_val & 0x7) == cos) {
               if (value == 1) {
                   oq_flush = oq_flush | (1 << i);
               }
               else if (value == 0) {
                   oq_flush = oq_flush & (~(1 << i));
               }
           }
           xoff2oq_val = xoff2oq_val >> 3;
       }
       pbc_csr.port_7.cfg_oq_queue.flush(oq_flush);
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_7.cfg_oq_queue.show();}
       pbc_csr.port_7.cfg_oq_queue.write();
   }
   if (port == 8) {
       pbc_csr.port_8.cfg_oq_queue.read();
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_8.cfg_oq_queue.show();}
       oq_flush = pbc_csr.port_8.cfg_oq_queue.flush().convert_to<int>();

       pbc_csr.port_8.cfg_oq_xoff2oq.read();
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_8.cfg_oq_xoff2oq.show();}
       xoff2oq_val = pbc_csr.port_8.cfg_oq_xoff2oq.map().convert_to<int>();

       for (int i = 0; i < 32; i++) {
           if ((xoff2oq_val & 0x7) == cos) {
               if (value == 1) {
                   oq_flush = oq_flush | (1 << i);
               }
               else if (value == 0) {
                   oq_flush = oq_flush & (~(1 << i));
               }
           }
           xoff2oq_val = xoff2oq_val >> 3;
       }
       pbc_csr.port_8.cfg_oq_queue.flush(oq_flush);
       if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_8.cfg_oq_queue.show();}
       pbc_csr.port_8.cfg_oq_queue.write();
   }
}

void cap_pb_read_hbm_ctx_mem(int chip_id, int inst_id, int port, int addr, cpp_int & rval) {

  //PLOG_MSG("cap_pb_read_hbm_ctx_mem: addr: " << addr << endl);

  cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);

  pbc_csr.hbm.cfg_hbm.read();
  pbc_csr.hbm.cfg_hbm.dhs_mem_address(addr);
  pbc_csr.hbm.cfg_hbm.mtu_dhs_selection(port);
  pbc_csr.hbm.cfg_hbm.write();

  pbc_csr.hbm.dhs_hbm_mem.entry.read();
  rval = pbc_csr.hbm.dhs_hbm_mem.entry.all();
}

void cap_pb_write_hbm_ctx_mem(int chip_id, int inst_id, int port, int addr, cpp_int wval) {

  //PLOG_MSG("cap_pb_write_hbm_ctx_mem: addr: " << addr << ", wval: " << wval << endl);

  cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);

  pbc_csr.hbm.cfg_hbm.read();
  pbc_csr.hbm.cfg_hbm.dhs_mem_address(addr);
  pbc_csr.hbm.cfg_hbm.mtu_dhs_selection(port);
  pbc_csr.hbm.cfg_hbm.write();

  pbc_csr.hbm.dhs_hbm_mem.entry.all(wval);
  pbc_csr.hbm.dhs_hbm_mem.entry.write();
}

void cap_pb_read_hbm_ctx_stat(int chip_id, int inst_id, int port, int addr, cpp_int & good_cnt, cpp_int & error_cnt) {

  //PLOG_MSG("cap_pb_read_hbm_ctx_stat: addr: " << addr << endl);

  cpp_int_helper hlp;
  cpp_int rval;

  cap_pb_read_hbm_ctx_mem(chip_id, inst_id, port, addr, rval);

  good_cnt = hlp.get_slc(rval, 640, 671).convert_to<uint32_t>();
  error_cnt = hlp.get_slc(rval, 672, 703).convert_to<uint32_t>();
}

void cap_pb_init_hbm_ctx_mem(int chip_id, int inst_id) {

  //PLOG_MSG("cap_pb_init_hbm_ctx_mem" << endl);

  for (int p=12; p<16; p++) {
    for (int i=0; i<32; i++) {
      cap_pb_write_hbm_ctx_mem(chip_id, inst_id, p, i, 0); 
    }
  }
}

void cap_pb_dump_cntrs(int chip_id, int inst_id, int print_ctxt, int print_qdepth, int is_hex) {

 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, 0);

 map <string, uint64_t> cntr_map0;
 map <string, uint64_t> cntr_map1;
 map <string, uint64_t> cntr_map2;
 map <string, uint64_t> cntr_map3;
 map <string, uint64_t> cntr_map4;
 map <string, uint64_t> cntr_map5;
 map <string, uint64_t> cntr_map6;
 map <string, uint64_t> cntr_map7;
 map <string, uint64_t> cntr_map8;
 map <string, uint64_t> cntr_map9;
 map <string, uint64_t> cntr_map10;
 map <string, uint64_t> cntr_map11;

 map <string, uint64_t> cntr_map_hbm0;
 map <string, uint64_t> cntr_map_hbm1;
 map <string, uint64_t> cntr_map_hbm2;
 map <string, uint64_t> cntr_map_hbm3;
 map <string, uint64_t> cntr_map_hbm4;
 map <string, uint64_t> cntr_map_hbm5;
 map <string, uint64_t> cntr_map_hbm6;
 map <string, uint64_t> cntr_map_hbm7;
 map <string, uint64_t> cntr_map_hbm8;
 map <string, uint64_t> cntr_map_hbm9;

 cpu_access_type_e cur_cpu_access_type = cpu::access()->get_access_type();
 cpu::access()->set_access_type(front_door_e);

 map <string, uint64_t> cntr_map_ctx0;
 map <string, uint64_t> cntr_map_ctx1;
 map <string, uint64_t> cntr_map_ctx2;
 map <string, uint64_t> cntr_map_ctx3;
 map <string, uint64_t> cntr_map_ctx4;
 map <string, uint64_t> cntr_map_ctx5;
 map <string, uint64_t> cntr_map_ctx6;
 map <string, uint64_t> cntr_map_ctx7;
 map <string, uint64_t> cntr_map_ctx8;
 map <string, uint64_t> cntr_map_ctx9;
 map <string, uint64_t> cntr_map_ctx10;
 map <string, uint64_t> cntr_map_ctx11;
 map <string, uint64_t> cntr_map_ctx12;
 map <string, uint64_t> cntr_map_ctx13;
 map <string, uint64_t> cntr_map_ctx14;
 map <string, uint64_t> cntr_map_ctx15;
 map <string, uint64_t> cntr_map_ctx16;
 map <string, uint64_t> cntr_map_ctx17;
 map <string, uint64_t> cntr_map_ctx18;
 map <string, uint64_t> cntr_map_ctx19;
 map <string, uint64_t> cntr_map_ctx20;
 map <string, uint64_t> cntr_map_ctx21;
 map <string, uint64_t> cntr_map_ctx22;
 map <string, uint64_t> cntr_map_ctx23;
 map <string, uint64_t> cntr_map_ctx24;
 map <string, uint64_t> cntr_map_ctx25;
 map <string, uint64_t> cntr_map_ctx26;
 map <string, uint64_t> cntr_map_ctx27;
 map <string, uint64_t> cntr_map_ctx28;
 map <string, uint64_t> cntr_map_ctx29;
 map <string, uint64_t> cntr_map_ctx30;
 map <string, uint64_t> cntr_map_ctx31;
 cpp_int rtmp;
 cpp_int good_count, error_count;
 stringstream sdata;

   pbc_csr.cnt_flits[0].read();
   cntr_map0["FROM_PORT_TO_PB_SOP"] = pbc_csr.cnt_flits[0].sop_in().convert_to<uint64_t>();
   cntr_map0["FROM_PORT_TO_PB_EOP"] = pbc_csr.cnt_flits[0].eop_in().convert_to<uint64_t>();
   cntr_map0["TO_PORT_FROM_PB_SOP"] = pbc_csr.cnt_flits[0].sop_out().convert_to<uint64_t>();
   cntr_map0["TO_PORT_FROM_PB_EOP"] = pbc_csr.cnt_flits[0].eop_out().convert_to<uint64_t>();

   cap_pb_get_write_error_cnt(chip_id, inst_id, 0, 0, rtmp);
   cntr_map0["WERR_DISCARDED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 0, 1, rtmp);
   cntr_map0["WERR_ADMITTED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 0, 2, rtmp);
   cntr_map0["WERR_INTRINSIC_DROP"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 0, 3, rtmp);
   cntr_map0["WERR_OUT_OF_CELLS"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 0, 4, rtmp);
   cntr_map0["WERR_OUT_OF_CREDIT"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 0, 5, rtmp);
   cntr_map0["WERR_TRUNCATION"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 0, 6, rtmp);
   cntr_map0["WERR_PORT_DISABLED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 0, 7, rtmp);
   cntr_map0["WERR_OUT_OF_CELLS1"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 0, 8, rtmp);
   cntr_map0["WERR_TAIL_DROP_CPU"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 0, 9, rtmp);
   cntr_map0["WERR_TAIL_DROP_SPAN"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 0, 10, rtmp);
   cntr_map0["WERR_MIN_SIZE_VIOL"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 0, 11, rtmp);
   cntr_map0["WERR_ENQUEUE_DROP"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 0, 12, rtmp);
   cntr_map0["WERR_PORT_RANGE"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 0, 13, rtmp);
   cntr_map0["WERR_OQ_RANGE"] = rtmp.convert_to<uint64_t>();
   pbc_csr.cnt_flits[1].read();
   cntr_map1["FROM_PORT_TO_PB_SOP"] = pbc_csr.cnt_flits[1].sop_in().convert_to<uint64_t>();
   cntr_map1["FROM_PORT_TO_PB_EOP"] = pbc_csr.cnt_flits[1].eop_in().convert_to<uint64_t>();
   cntr_map1["TO_PORT_FROM_PB_SOP"] = pbc_csr.cnt_flits[1].sop_out().convert_to<uint64_t>();
   cntr_map1["TO_PORT_FROM_PB_EOP"] = pbc_csr.cnt_flits[1].eop_out().convert_to<uint64_t>();

   cap_pb_get_write_error_cnt(chip_id, inst_id, 1, 0, rtmp);
   cntr_map1["WERR_DISCARDED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 1, 1, rtmp);
   cntr_map1["WERR_ADMITTED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 1, 2, rtmp);
   cntr_map1["WERR_INTRINSIC_DROP"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 1, 3, rtmp);
   cntr_map1["WERR_OUT_OF_CELLS"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 1, 4, rtmp);
   cntr_map1["WERR_OUT_OF_CREDIT"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 1, 5, rtmp);
   cntr_map1["WERR_TRUNCATION"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 1, 6, rtmp);
   cntr_map1["WERR_PORT_DISABLED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 1, 7, rtmp);
   cntr_map1["WERR_OUT_OF_CELLS1"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 1, 8, rtmp);
   cntr_map1["WERR_TAIL_DROP_CPU"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 1, 9, rtmp);
   cntr_map1["WERR_TAIL_DROP_SPAN"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 1, 10, rtmp);
   cntr_map1["WERR_MIN_SIZE_VIOL"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 1, 11, rtmp);
   cntr_map1["WERR_ENQUEUE_DROP"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 1, 12, rtmp);
   cntr_map1["WERR_PORT_RANGE"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 1, 13, rtmp);
   cntr_map1["WERR_OQ_RANGE"] = rtmp.convert_to<uint64_t>();
   pbc_csr.cnt_flits[2].read();
   cntr_map2["FROM_PORT_TO_PB_SOP"] = pbc_csr.cnt_flits[2].sop_in().convert_to<uint64_t>();
   cntr_map2["FROM_PORT_TO_PB_EOP"] = pbc_csr.cnt_flits[2].eop_in().convert_to<uint64_t>();
   cntr_map2["TO_PORT_FROM_PB_SOP"] = pbc_csr.cnt_flits[2].sop_out().convert_to<uint64_t>();
   cntr_map2["TO_PORT_FROM_PB_EOP"] = pbc_csr.cnt_flits[2].eop_out().convert_to<uint64_t>();

   cap_pb_get_write_error_cnt(chip_id, inst_id, 2, 0, rtmp);
   cntr_map2["WERR_DISCARDED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 2, 1, rtmp);
   cntr_map2["WERR_ADMITTED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 2, 2, rtmp);
   cntr_map2["WERR_INTRINSIC_DROP"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 2, 3, rtmp);
   cntr_map2["WERR_OUT_OF_CELLS"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 2, 4, rtmp);
   cntr_map2["WERR_OUT_OF_CREDIT"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 2, 5, rtmp);
   cntr_map2["WERR_TRUNCATION"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 2, 6, rtmp);
   cntr_map2["WERR_PORT_DISABLED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 2, 7, rtmp);
   cntr_map2["WERR_OUT_OF_CELLS1"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 2, 8, rtmp);
   cntr_map2["WERR_TAIL_DROP_CPU"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 2, 9, rtmp);
   cntr_map2["WERR_TAIL_DROP_SPAN"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 2, 10, rtmp);
   cntr_map2["WERR_MIN_SIZE_VIOL"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 2, 11, rtmp);
   cntr_map2["WERR_ENQUEUE_DROP"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 2, 12, rtmp);
   cntr_map2["WERR_PORT_RANGE"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 2, 13, rtmp);
   cntr_map2["WERR_OQ_RANGE"] = rtmp.convert_to<uint64_t>();
   pbc_csr.cnt_flits[3].read();
   cntr_map3["FROM_PORT_TO_PB_SOP"] = pbc_csr.cnt_flits[3].sop_in().convert_to<uint64_t>();
   cntr_map3["FROM_PORT_TO_PB_EOP"] = pbc_csr.cnt_flits[3].eop_in().convert_to<uint64_t>();
   cntr_map3["TO_PORT_FROM_PB_SOP"] = pbc_csr.cnt_flits[3].sop_out().convert_to<uint64_t>();
   cntr_map3["TO_PORT_FROM_PB_EOP"] = pbc_csr.cnt_flits[3].eop_out().convert_to<uint64_t>();

   cap_pb_get_write_error_cnt(chip_id, inst_id, 3, 0, rtmp);
   cntr_map3["WERR_DISCARDED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 3, 1, rtmp);
   cntr_map3["WERR_ADMITTED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 3, 2, rtmp);
   cntr_map3["WERR_INTRINSIC_DROP"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 3, 3, rtmp);
   cntr_map3["WERR_OUT_OF_CELLS"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 3, 4, rtmp);
   cntr_map3["WERR_OUT_OF_CREDIT"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 3, 5, rtmp);
   cntr_map3["WERR_TRUNCATION"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 3, 6, rtmp);
   cntr_map3["WERR_PORT_DISABLED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 3, 7, rtmp);
   cntr_map3["WERR_OUT_OF_CELLS1"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 3, 8, rtmp);
   cntr_map3["WERR_TAIL_DROP_CPU"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 3, 9, rtmp);
   cntr_map3["WERR_TAIL_DROP_SPAN"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 3, 10, rtmp);
   cntr_map3["WERR_MIN_SIZE_VIOL"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 3, 11, rtmp);
   cntr_map3["WERR_ENQUEUE_DROP"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 3, 12, rtmp);
   cntr_map3["WERR_PORT_RANGE"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 3, 13, rtmp);
   cntr_map3["WERR_OQ_RANGE"] = rtmp.convert_to<uint64_t>();
   pbc_csr.cnt_flits[4].read();
   cntr_map4["FROM_PORT_TO_PB_SOP"] = pbc_csr.cnt_flits[4].sop_in().convert_to<uint64_t>();
   cntr_map4["FROM_PORT_TO_PB_EOP"] = pbc_csr.cnt_flits[4].eop_in().convert_to<uint64_t>();
   cntr_map4["TO_PORT_FROM_PB_SOP"] = pbc_csr.cnt_flits[4].sop_out().convert_to<uint64_t>();
   cntr_map4["TO_PORT_FROM_PB_EOP"] = pbc_csr.cnt_flits[4].eop_out().convert_to<uint64_t>();

   cap_pb_get_write_error_cnt(chip_id, inst_id, 4, 0, rtmp);
   cntr_map4["WERR_DISCARDED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 4, 1, rtmp);
   cntr_map4["WERR_ADMITTED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 4, 2, rtmp);
   cntr_map4["WERR_INTRINSIC_DROP"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 4, 3, rtmp);
   cntr_map4["WERR_OUT_OF_CELLS"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 4, 4, rtmp);
   cntr_map4["WERR_OUT_OF_CREDIT"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 4, 5, rtmp);
   cntr_map4["WERR_TRUNCATION"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 4, 6, rtmp);
   cntr_map4["WERR_PORT_DISABLED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 4, 7, rtmp);
   cntr_map4["WERR_OUT_OF_CELLS1"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 4, 8, rtmp);
   cntr_map4["WERR_TAIL_DROP_CPU"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 4, 9, rtmp);
   cntr_map4["WERR_TAIL_DROP_SPAN"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 4, 10, rtmp);
   cntr_map4["WERR_MIN_SIZE_VIOL"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 4, 11, rtmp);
   cntr_map4["WERR_ENQUEUE_DROP"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 4, 12, rtmp);
   cntr_map4["WERR_PORT_RANGE"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 4, 13, rtmp);
   cntr_map4["WERR_OQ_RANGE"] = rtmp.convert_to<uint64_t>();
   pbc_csr.cnt_flits[5].read();
   cntr_map5["FROM_PORT_TO_PB_SOP"] = pbc_csr.cnt_flits[5].sop_in().convert_to<uint64_t>();
   cntr_map5["FROM_PORT_TO_PB_EOP"] = pbc_csr.cnt_flits[5].eop_in().convert_to<uint64_t>();
   cntr_map5["TO_PORT_FROM_PB_SOP"] = pbc_csr.cnt_flits[5].sop_out().convert_to<uint64_t>();
   cntr_map5["TO_PORT_FROM_PB_EOP"] = pbc_csr.cnt_flits[5].eop_out().convert_to<uint64_t>();

   cap_pb_get_write_error_cnt(chip_id, inst_id, 5, 0, rtmp);
   cntr_map5["WERR_DISCARDED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 5, 1, rtmp);
   cntr_map5["WERR_ADMITTED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 5, 2, rtmp);
   cntr_map5["WERR_INTRINSIC_DROP"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 5, 3, rtmp);
   cntr_map5["WERR_OUT_OF_CELLS"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 5, 4, rtmp);
   cntr_map5["WERR_OUT_OF_CREDIT"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 5, 5, rtmp);
   cntr_map5["WERR_TRUNCATION"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 5, 6, rtmp);
   cntr_map5["WERR_PORT_DISABLED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 5, 7, rtmp);
   cntr_map5["WERR_OUT_OF_CELLS1"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 5, 8, rtmp);
   cntr_map5["WERR_TAIL_DROP_CPU"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 5, 9, rtmp);
   cntr_map5["WERR_TAIL_DROP_SPAN"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 5, 10, rtmp);
   cntr_map5["WERR_MIN_SIZE_VIOL"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 5, 11, rtmp);
   cntr_map5["WERR_ENQUEUE_DROP"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 5, 12, rtmp);
   cntr_map5["WERR_PORT_RANGE"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 5, 13, rtmp);
   cntr_map5["WERR_OQ_RANGE"] = rtmp.convert_to<uint64_t>();
   pbc_csr.cnt_flits[6].read();
   cntr_map6["FROM_PORT_TO_PB_SOP"] = pbc_csr.cnt_flits[6].sop_in().convert_to<uint64_t>();
   cntr_map6["FROM_PORT_TO_PB_EOP"] = pbc_csr.cnt_flits[6].eop_in().convert_to<uint64_t>();
   cntr_map6["TO_PORT_FROM_PB_SOP"] = pbc_csr.cnt_flits[6].sop_out().convert_to<uint64_t>();
   cntr_map6["TO_PORT_FROM_PB_EOP"] = pbc_csr.cnt_flits[6].eop_out().convert_to<uint64_t>();

   cap_pb_get_write_error_cnt(chip_id, inst_id, 6, 0, rtmp);
   cntr_map6["WERR_DISCARDED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 6, 1, rtmp);
   cntr_map6["WERR_ADMITTED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 6, 2, rtmp);
   cntr_map6["WERR_INTRINSIC_DROP"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 6, 3, rtmp);
   cntr_map6["WERR_OUT_OF_CELLS"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 6, 4, rtmp);
   cntr_map6["WERR_OUT_OF_CREDIT"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 6, 5, rtmp);
   cntr_map6["WERR_TRUNCATION"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 6, 6, rtmp);
   cntr_map6["WERR_PORT_DISABLED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 6, 7, rtmp);
   cntr_map6["WERR_OUT_OF_CELLS1"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 6, 8, rtmp);
   cntr_map6["WERR_TAIL_DROP_CPU"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 6, 9, rtmp);
   cntr_map6["WERR_TAIL_DROP_SPAN"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 6, 10, rtmp);
   cntr_map6["WERR_MIN_SIZE_VIOL"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 6, 11, rtmp);
   cntr_map6["WERR_ENQUEUE_DROP"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 6, 12, rtmp);
   cntr_map6["WERR_PORT_RANGE"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 6, 13, rtmp);
   cntr_map6["WERR_OQ_RANGE"] = rtmp.convert_to<uint64_t>();
   pbc_csr.cnt_flits[7].read();
   cntr_map7["FROM_PORT_TO_PB_SOP"] = pbc_csr.cnt_flits[7].sop_in().convert_to<uint64_t>();
   cntr_map7["FROM_PORT_TO_PB_EOP"] = pbc_csr.cnt_flits[7].eop_in().convert_to<uint64_t>();
   cntr_map7["TO_PORT_FROM_PB_SOP"] = pbc_csr.cnt_flits[7].sop_out().convert_to<uint64_t>();
   cntr_map7["TO_PORT_FROM_PB_EOP"] = pbc_csr.cnt_flits[7].eop_out().convert_to<uint64_t>();

   cap_pb_get_write_error_cnt(chip_id, inst_id, 7, 0, rtmp);
   cntr_map7["WERR_DISCARDED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 7, 1, rtmp);
   cntr_map7["WERR_ADMITTED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 7, 2, rtmp);
   cntr_map7["WERR_INTRINSIC_DROP"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 7, 3, rtmp);
   cntr_map7["WERR_OUT_OF_CELLS"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 7, 4, rtmp);
   cntr_map7["WERR_OUT_OF_CREDIT"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 7, 5, rtmp);
   cntr_map7["WERR_TRUNCATION"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 7, 6, rtmp);
   cntr_map7["WERR_PORT_DISABLED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 7, 7, rtmp);
   cntr_map7["WERR_OUT_OF_CELLS1"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 7, 8, rtmp);
   cntr_map7["WERR_TAIL_DROP_CPU"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 7, 9, rtmp);
   cntr_map7["WERR_TAIL_DROP_SPAN"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 7, 10, rtmp);
   cntr_map7["WERR_MIN_SIZE_VIOL"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 7, 11, rtmp);
   cntr_map7["WERR_ENQUEUE_DROP"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 7, 12, rtmp);
   cntr_map7["WERR_PORT_RANGE"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 7, 13, rtmp);
   cntr_map7["WERR_OQ_RANGE"] = rtmp.convert_to<uint64_t>();
   pbc_csr.cnt_flits[8].read();
   cntr_map8["FROM_PORT_TO_PB_SOP"] = pbc_csr.cnt_flits[8].sop_in().convert_to<uint64_t>();
   cntr_map8["FROM_PORT_TO_PB_EOP"] = pbc_csr.cnt_flits[8].eop_in().convert_to<uint64_t>();
   cntr_map8["TO_PORT_FROM_PB_SOP"] = pbc_csr.cnt_flits[8].sop_out().convert_to<uint64_t>();
   cntr_map8["TO_PORT_FROM_PB_EOP"] = pbc_csr.cnt_flits[8].eop_out().convert_to<uint64_t>();

   cap_pb_get_write_error_cnt(chip_id, inst_id, 8, 0, rtmp);
   cntr_map8["WERR_DISCARDED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 8, 1, rtmp);
   cntr_map8["WERR_ADMITTED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 8, 2, rtmp);
   cntr_map8["WERR_INTRINSIC_DROP"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 8, 3, rtmp);
   cntr_map8["WERR_OUT_OF_CELLS"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 8, 4, rtmp);
   cntr_map8["WERR_OUT_OF_CREDIT"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 8, 5, rtmp);
   cntr_map8["WERR_TRUNCATION"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 8, 6, rtmp);
   cntr_map8["WERR_PORT_DISABLED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 8, 7, rtmp);
   cntr_map8["WERR_OUT_OF_CELLS1"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 8, 8, rtmp);
   cntr_map8["WERR_TAIL_DROP_CPU"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 8, 9, rtmp);
   cntr_map8["WERR_TAIL_DROP_SPAN"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 8, 10, rtmp);
   cntr_map8["WERR_MIN_SIZE_VIOL"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 8, 11, rtmp);
   cntr_map8["WERR_ENQUEUE_DROP"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 8, 12, rtmp);
   cntr_map8["WERR_PORT_RANGE"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 8, 13, rtmp);
   cntr_map8["WERR_OQ_RANGE"] = rtmp.convert_to<uint64_t>();
   pbc_csr.cnt_flits[9].read();
   cntr_map9["FROM_PORT_TO_PB_SOP"] = pbc_csr.cnt_flits[9].sop_in().convert_to<uint64_t>();
   cntr_map9["FROM_PORT_TO_PB_EOP"] = pbc_csr.cnt_flits[9].eop_in().convert_to<uint64_t>();
   cntr_map9["TO_PORT_FROM_PB_SOP"] = pbc_csr.cnt_flits[9].sop_out().convert_to<uint64_t>();
   cntr_map9["TO_PORT_FROM_PB_EOP"] = pbc_csr.cnt_flits[9].eop_out().convert_to<uint64_t>();

   cap_pb_get_write_error_cnt(chip_id, inst_id, 9, 0, rtmp);
   cntr_map9["WERR_DISCARDED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 9, 1, rtmp);
   cntr_map9["WERR_ADMITTED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 9, 2, rtmp);
   cntr_map9["WERR_INTRINSIC_DROP"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 9, 3, rtmp);
   cntr_map9["WERR_OUT_OF_CELLS"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 9, 4, rtmp);
   cntr_map9["WERR_OUT_OF_CREDIT"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 9, 5, rtmp);
   cntr_map9["WERR_TRUNCATION"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 9, 6, rtmp);
   cntr_map9["WERR_PORT_DISABLED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 9, 7, rtmp);
   cntr_map9["WERR_OUT_OF_CELLS1"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 9, 8, rtmp);
   cntr_map9["WERR_TAIL_DROP_CPU"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 9, 9, rtmp);
   cntr_map9["WERR_TAIL_DROP_SPAN"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 9, 10, rtmp);
   cntr_map9["WERR_MIN_SIZE_VIOL"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 9, 11, rtmp);
   cntr_map9["WERR_ENQUEUE_DROP"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 9, 12, rtmp);
   cntr_map9["WERR_PORT_RANGE"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 9, 13, rtmp);
   cntr_map9["WERR_OQ_RANGE"] = rtmp.convert_to<uint64_t>();
   pbc_csr.cnt_flits[10].read();
   cntr_map10["FROM_PORT_TO_PB_SOP"] = pbc_csr.cnt_flits[10].sop_in().convert_to<uint64_t>();
   cntr_map10["FROM_PORT_TO_PB_EOP"] = pbc_csr.cnt_flits[10].eop_in().convert_to<uint64_t>();
   cntr_map10["TO_PORT_FROM_PB_SOP"] = pbc_csr.cnt_flits[10].sop_out().convert_to<uint64_t>();
   cntr_map10["TO_PORT_FROM_PB_EOP"] = pbc_csr.cnt_flits[10].eop_out().convert_to<uint64_t>();

   cap_pb_get_write_error_cnt(chip_id, inst_id, 10, 0, rtmp);
   cntr_map10["WERR_DISCARDED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 10, 1, rtmp);
   cntr_map10["WERR_ADMITTED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 10, 2, rtmp);
   cntr_map10["WERR_INTRINSIC_DROP"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 10, 3, rtmp);
   cntr_map10["WERR_OUT_OF_CELLS"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 10, 4, rtmp);
   cntr_map10["WERR_OUT_OF_CREDIT"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 10, 5, rtmp);
   cntr_map10["WERR_TRUNCATION"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 10, 6, rtmp);
   cntr_map10["WERR_PORT_DISABLED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 10, 7, rtmp);
   cntr_map10["WERR_OUT_OF_CELLS1"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 10, 8, rtmp);
   cntr_map10["WERR_TAIL_DROP_CPU"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 10, 9, rtmp);
   cntr_map10["WERR_TAIL_DROP_SPAN"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 10, 10, rtmp);
   cntr_map10["WERR_MIN_SIZE_VIOL"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 10, 11, rtmp);
   cntr_map10["WERR_ENQUEUE_DROP"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 10, 12, rtmp);
   cntr_map10["WERR_PORT_RANGE"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 10, 13, rtmp);
   cntr_map10["WERR_OQ_RANGE"] = rtmp.convert_to<uint64_t>();
   pbc_csr.cnt_flits[11].read();
   cntr_map11["FROM_PORT_TO_PB_SOP"] = pbc_csr.cnt_flits[11].sop_in().convert_to<uint64_t>();
   cntr_map11["FROM_PORT_TO_PB_EOP"] = pbc_csr.cnt_flits[11].eop_in().convert_to<uint64_t>();
   cntr_map11["TO_PORT_FROM_PB_SOP"] = pbc_csr.cnt_flits[11].sop_out().convert_to<uint64_t>();
   cntr_map11["TO_PORT_FROM_PB_EOP"] = pbc_csr.cnt_flits[11].eop_out().convert_to<uint64_t>();

   cap_pb_get_write_error_cnt(chip_id, inst_id, 11, 0, rtmp);
   cntr_map11["WERR_DISCARDED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 11, 1, rtmp);
   cntr_map11["WERR_ADMITTED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 11, 2, rtmp);
   cntr_map11["WERR_INTRINSIC_DROP"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 11, 3, rtmp);
   cntr_map11["WERR_OUT_OF_CELLS"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 11, 4, rtmp);
   cntr_map11["WERR_OUT_OF_CREDIT"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 11, 5, rtmp);
   cntr_map11["WERR_TRUNCATION"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 11, 6, rtmp);
   cntr_map11["WERR_PORT_DISABLED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 11, 7, rtmp);
   cntr_map11["WERR_OUT_OF_CELLS1"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 11, 8, rtmp);
   cntr_map11["WERR_TAIL_DROP_CPU"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 11, 9, rtmp);
   cntr_map11["WERR_TAIL_DROP_SPAN"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 11, 10, rtmp);
   cntr_map11["WERR_MIN_SIZE_VIOL"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 11, 11, rtmp);
   cntr_map11["WERR_ENQUEUE_DROP"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 11, 12, rtmp);
   cntr_map11["WERR_PORT_RANGE"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 11, 13, rtmp);
   cntr_map11["WERR_OQ_RANGE"] = rtmp.convert_to<uint64_t>();
   pbc_csr.hbm.hbm_port_0.cnt_hbm.read();
   cntr_map_hbm0["IN_SOP_TO_PF"] = pbc_csr.hbm.hbm_port_0.cnt_hbm.flits_sop_in().convert_to<uint64_t>();
   cntr_map_hbm0["IN_EOP_TO_PF"] = pbc_csr.hbm.hbm_port_0.cnt_hbm.flits_eop_in().convert_to<uint64_t>();
   cntr_map_hbm0["OUT_SOP_FROM_PF"] = pbc_csr.hbm.hbm_port_0.cnt_hbm.flits_sop_out().convert_to<uint64_t>();
   cntr_map_hbm0["OUT_EOP_FROM_PF"] = pbc_csr.hbm.hbm_port_0.cnt_hbm.flits_eop_out().convert_to<uint64_t>();
   cntr_map_hbm0["HBM_OCCUPANCY_DROP"] = pbc_csr.hbm.hbm_port_0.cnt_hbm.occupancy_drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm0_emergency_stop.read();
   cntr_map_hbm0["HBM_WDROP_EMERGENCY_STOP"] = pbc_csr.hbm.cnt_hbm0_emergency_stop.drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm0_write_ack_filling_up.read();
   cntr_map_hbm0["HBM_WDROP_WR_ACK_FILL_UP"] = pbc_csr.hbm.cnt_hbm0_write_ack_filling_up.drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm0_write_ack_full.read();
   cntr_map_hbm0["HBM_WDROP_WR_ACK_FULL"] = pbc_csr.hbm.cnt_hbm0_write_ack_full.drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm0_truncate.read();
   cntr_map_hbm0["HBM_WDROP_TRUNCATE"] = pbc_csr.hbm.cnt_hbm0_truncate.drop().convert_to<uint64_t>();
   pbc_csr.hbm.sat_hbm0_ctrl_full.read();
   cntr_map_hbm0["HBM_WDROP_CTRL_FULL"] = pbc_csr.hbm.sat_hbm0_ctrl_full.drop().convert_to<uint64_t>();
   pbc_csr.hbm.hbm_port_1.cnt_hbm.read();
   cntr_map_hbm1["IN_SOP_TO_PF"] = pbc_csr.hbm.hbm_port_1.cnt_hbm.flits_sop_in().convert_to<uint64_t>();
   cntr_map_hbm1["IN_EOP_TO_PF"] = pbc_csr.hbm.hbm_port_1.cnt_hbm.flits_eop_in().convert_to<uint64_t>();
   cntr_map_hbm1["OUT_SOP_FROM_PF"] = pbc_csr.hbm.hbm_port_1.cnt_hbm.flits_sop_out().convert_to<uint64_t>();
   cntr_map_hbm1["OUT_EOP_FROM_PF"] = pbc_csr.hbm.hbm_port_1.cnt_hbm.flits_eop_out().convert_to<uint64_t>();
   cntr_map_hbm1["HBM_OCCUPANCY_DROP"] = pbc_csr.hbm.hbm_port_1.cnt_hbm.occupancy_drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm1_emergency_stop.read();
   cntr_map_hbm1["HBM_WDROP_EMERGENCY_STOP"] = pbc_csr.hbm.cnt_hbm1_emergency_stop.drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm1_write_ack_filling_up.read();
   cntr_map_hbm1["HBM_WDROP_WR_ACK_FILL_UP"] = pbc_csr.hbm.cnt_hbm1_write_ack_filling_up.drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm1_write_ack_full.read();
   cntr_map_hbm1["HBM_WDROP_WR_ACK_FULL"] = pbc_csr.hbm.cnt_hbm1_write_ack_full.drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm1_truncate.read();
   cntr_map_hbm1["HBM_WDROP_TRUNCATE"] = pbc_csr.hbm.cnt_hbm1_truncate.drop().convert_to<uint64_t>();
   pbc_csr.hbm.sat_hbm1_ctrl_full.read();
   cntr_map_hbm1["HBM_WDROP_CTRL_FULL"] = pbc_csr.hbm.sat_hbm1_ctrl_full.drop().convert_to<uint64_t>();
   pbc_csr.hbm.hbm_port_2.cnt_hbm.read();
   cntr_map_hbm2["IN_SOP_TO_PF"] = pbc_csr.hbm.hbm_port_2.cnt_hbm.flits_sop_in().convert_to<uint64_t>();
   cntr_map_hbm2["IN_EOP_TO_PF"] = pbc_csr.hbm.hbm_port_2.cnt_hbm.flits_eop_in().convert_to<uint64_t>();
   cntr_map_hbm2["OUT_SOP_FROM_PF"] = pbc_csr.hbm.hbm_port_2.cnt_hbm.flits_sop_out().convert_to<uint64_t>();
   cntr_map_hbm2["OUT_EOP_FROM_PF"] = pbc_csr.hbm.hbm_port_2.cnt_hbm.flits_eop_out().convert_to<uint64_t>();
   cntr_map_hbm2["HBM_OCCUPANCY_DROP"] = pbc_csr.hbm.hbm_port_2.cnt_hbm.occupancy_drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm2_emergency_stop.read();
   cntr_map_hbm2["HBM_WDROP_EMERGENCY_STOP"] = pbc_csr.hbm.cnt_hbm2_emergency_stop.drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm2_write_ack_filling_up.read();
   cntr_map_hbm2["HBM_WDROP_WR_ACK_FILL_UP"] = pbc_csr.hbm.cnt_hbm2_write_ack_filling_up.drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm2_write_ack_full.read();
   cntr_map_hbm2["HBM_WDROP_WR_ACK_FULL"] = pbc_csr.hbm.cnt_hbm2_write_ack_full.drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm2_truncate.read();
   cntr_map_hbm2["HBM_WDROP_TRUNCATE"] = pbc_csr.hbm.cnt_hbm2_truncate.drop().convert_to<uint64_t>();
   pbc_csr.hbm.sat_hbm2_ctrl_full.read();
   cntr_map_hbm2["HBM_WDROP_CTRL_FULL"] = pbc_csr.hbm.sat_hbm2_ctrl_full.drop().convert_to<uint64_t>();
   pbc_csr.hbm.hbm_port_3.cnt_hbm.read();
   cntr_map_hbm3["IN_SOP_TO_PF"] = pbc_csr.hbm.hbm_port_3.cnt_hbm.flits_sop_in().convert_to<uint64_t>();
   cntr_map_hbm3["IN_EOP_TO_PF"] = pbc_csr.hbm.hbm_port_3.cnt_hbm.flits_eop_in().convert_to<uint64_t>();
   cntr_map_hbm3["OUT_SOP_FROM_PF"] = pbc_csr.hbm.hbm_port_3.cnt_hbm.flits_sop_out().convert_to<uint64_t>();
   cntr_map_hbm3["OUT_EOP_FROM_PF"] = pbc_csr.hbm.hbm_port_3.cnt_hbm.flits_eop_out().convert_to<uint64_t>();
   cntr_map_hbm3["HBM_OCCUPANCY_DROP"] = pbc_csr.hbm.hbm_port_3.cnt_hbm.occupancy_drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm3_emergency_stop.read();
   cntr_map_hbm3["HBM_WDROP_EMERGENCY_STOP"] = pbc_csr.hbm.cnt_hbm3_emergency_stop.drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm3_write_ack_filling_up.read();
   cntr_map_hbm3["HBM_WDROP_WR_ACK_FILL_UP"] = pbc_csr.hbm.cnt_hbm3_write_ack_filling_up.drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm3_write_ack_full.read();
   cntr_map_hbm3["HBM_WDROP_WR_ACK_FULL"] = pbc_csr.hbm.cnt_hbm3_write_ack_full.drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm3_truncate.read();
   cntr_map_hbm3["HBM_WDROP_TRUNCATE"] = pbc_csr.hbm.cnt_hbm3_truncate.drop().convert_to<uint64_t>();
   pbc_csr.hbm.sat_hbm3_ctrl_full.read();
   cntr_map_hbm3["HBM_WDROP_CTRL_FULL"] = pbc_csr.hbm.sat_hbm3_ctrl_full.drop().convert_to<uint64_t>();
   pbc_csr.hbm.hbm_port_4.cnt_hbm.read();
   cntr_map_hbm4["IN_SOP_TO_PF"] = pbc_csr.hbm.hbm_port_4.cnt_hbm.flits_sop_in().convert_to<uint64_t>();
   cntr_map_hbm4["IN_EOP_TO_PF"] = pbc_csr.hbm.hbm_port_4.cnt_hbm.flits_eop_in().convert_to<uint64_t>();
   cntr_map_hbm4["OUT_SOP_FROM_PF"] = pbc_csr.hbm.hbm_port_4.cnt_hbm.flits_sop_out().convert_to<uint64_t>();
   cntr_map_hbm4["OUT_EOP_FROM_PF"] = pbc_csr.hbm.hbm_port_4.cnt_hbm.flits_eop_out().convert_to<uint64_t>();
   cntr_map_hbm4["HBM_OCCUPANCY_DROP"] = pbc_csr.hbm.hbm_port_4.cnt_hbm.occupancy_drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm4_emergency_stop.read();
   cntr_map_hbm4["HBM_WDROP_EMERGENCY_STOP"] = pbc_csr.hbm.cnt_hbm4_emergency_stop.drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm4_write_ack_filling_up.read();
   cntr_map_hbm4["HBM_WDROP_WR_ACK_FILL_UP"] = pbc_csr.hbm.cnt_hbm4_write_ack_filling_up.drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm4_write_ack_full.read();
   cntr_map_hbm4["HBM_WDROP_WR_ACK_FULL"] = pbc_csr.hbm.cnt_hbm4_write_ack_full.drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm4_truncate.read();
   cntr_map_hbm4["HBM_WDROP_TRUNCATE"] = pbc_csr.hbm.cnt_hbm4_truncate.drop().convert_to<uint64_t>();
   pbc_csr.hbm.sat_hbm4_ctrl_full.read();
   cntr_map_hbm4["HBM_WDROP_CTRL_FULL"] = pbc_csr.hbm.sat_hbm4_ctrl_full.drop().convert_to<uint64_t>();
   pbc_csr.hbm.hbm_port_5.cnt_hbm.read();
   cntr_map_hbm5["IN_SOP_TO_PF"] = pbc_csr.hbm.hbm_port_5.cnt_hbm.flits_sop_in().convert_to<uint64_t>();
   cntr_map_hbm5["IN_EOP_TO_PF"] = pbc_csr.hbm.hbm_port_5.cnt_hbm.flits_eop_in().convert_to<uint64_t>();
   cntr_map_hbm5["OUT_SOP_FROM_PF"] = pbc_csr.hbm.hbm_port_5.cnt_hbm.flits_sop_out().convert_to<uint64_t>();
   cntr_map_hbm5["OUT_EOP_FROM_PF"] = pbc_csr.hbm.hbm_port_5.cnt_hbm.flits_eop_out().convert_to<uint64_t>();
   cntr_map_hbm5["HBM_OCCUPANCY_DROP"] = pbc_csr.hbm.hbm_port_5.cnt_hbm.occupancy_drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm5_emergency_stop.read();
   cntr_map_hbm5["HBM_WDROP_EMERGENCY_STOP"] = pbc_csr.hbm.cnt_hbm5_emergency_stop.drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm5_write_ack_filling_up.read();
   cntr_map_hbm5["HBM_WDROP_WR_ACK_FILL_UP"] = pbc_csr.hbm.cnt_hbm5_write_ack_filling_up.drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm5_write_ack_full.read();
   cntr_map_hbm5["HBM_WDROP_WR_ACK_FULL"] = pbc_csr.hbm.cnt_hbm5_write_ack_full.drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm5_truncate.read();
   cntr_map_hbm5["HBM_WDROP_TRUNCATE"] = pbc_csr.hbm.cnt_hbm5_truncate.drop().convert_to<uint64_t>();
   pbc_csr.hbm.sat_hbm5_ctrl_full.read();
   cntr_map_hbm5["HBM_WDROP_CTRL_FULL"] = pbc_csr.hbm.sat_hbm5_ctrl_full.drop().convert_to<uint64_t>();
   pbc_csr.hbm.hbm_port_6.cnt_hbm.read();
   cntr_map_hbm6["IN_SOP_TO_PF"] = pbc_csr.hbm.hbm_port_6.cnt_hbm.flits_sop_in().convert_to<uint64_t>();
   cntr_map_hbm6["IN_EOP_TO_PF"] = pbc_csr.hbm.hbm_port_6.cnt_hbm.flits_eop_in().convert_to<uint64_t>();
   cntr_map_hbm6["OUT_SOP_FROM_PF"] = pbc_csr.hbm.hbm_port_6.cnt_hbm.flits_sop_out().convert_to<uint64_t>();
   cntr_map_hbm6["OUT_EOP_FROM_PF"] = pbc_csr.hbm.hbm_port_6.cnt_hbm.flits_eop_out().convert_to<uint64_t>();
   cntr_map_hbm6["HBM_OCCUPANCY_DROP"] = pbc_csr.hbm.hbm_port_6.cnt_hbm.occupancy_drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm6_emergency_stop.read();
   cntr_map_hbm6["HBM_WDROP_EMERGENCY_STOP"] = pbc_csr.hbm.cnt_hbm6_emergency_stop.drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm6_write_ack_filling_up.read();
   cntr_map_hbm6["HBM_WDROP_WR_ACK_FILL_UP"] = pbc_csr.hbm.cnt_hbm6_write_ack_filling_up.drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm6_write_ack_full.read();
   cntr_map_hbm6["HBM_WDROP_WR_ACK_FULL"] = pbc_csr.hbm.cnt_hbm6_write_ack_full.drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm6_truncate.read();
   cntr_map_hbm6["HBM_WDROP_TRUNCATE"] = pbc_csr.hbm.cnt_hbm6_truncate.drop().convert_to<uint64_t>();
   pbc_csr.hbm.sat_hbm6_ctrl_full.read();
   cntr_map_hbm6["HBM_WDROP_CTRL_FULL"] = pbc_csr.hbm.sat_hbm6_ctrl_full.drop().convert_to<uint64_t>();
   pbc_csr.hbm.hbm_port_7.cnt_hbm.read();
   cntr_map_hbm7["IN_SOP_TO_PF"] = pbc_csr.hbm.hbm_port_7.cnt_hbm.flits_sop_in().convert_to<uint64_t>();
   cntr_map_hbm7["IN_EOP_TO_PF"] = pbc_csr.hbm.hbm_port_7.cnt_hbm.flits_eop_in().convert_to<uint64_t>();
   cntr_map_hbm7["OUT_SOP_FROM_PF"] = pbc_csr.hbm.hbm_port_7.cnt_hbm.flits_sop_out().convert_to<uint64_t>();
   cntr_map_hbm7["OUT_EOP_FROM_PF"] = pbc_csr.hbm.hbm_port_7.cnt_hbm.flits_eop_out().convert_to<uint64_t>();
   cntr_map_hbm7["HBM_OCCUPANCY_DROP"] = pbc_csr.hbm.hbm_port_7.cnt_hbm.occupancy_drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm7_emergency_stop.read();
   cntr_map_hbm7["HBM_WDROP_EMERGENCY_STOP"] = pbc_csr.hbm.cnt_hbm7_emergency_stop.drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm7_write_ack_filling_up.read();
   cntr_map_hbm7["HBM_WDROP_WR_ACK_FILL_UP"] = pbc_csr.hbm.cnt_hbm7_write_ack_filling_up.drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm7_write_ack_full.read();
   cntr_map_hbm7["HBM_WDROP_WR_ACK_FULL"] = pbc_csr.hbm.cnt_hbm7_write_ack_full.drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm7_truncate.read();
   cntr_map_hbm7["HBM_WDROP_TRUNCATE"] = pbc_csr.hbm.cnt_hbm7_truncate.drop().convert_to<uint64_t>();
   pbc_csr.hbm.sat_hbm7_ctrl_full.read();
   cntr_map_hbm7["HBM_WDROP_CTRL_FULL"] = pbc_csr.hbm.sat_hbm7_ctrl_full.drop().convert_to<uint64_t>();
   pbc_csr.hbm.hbm_port_8.cnt_hbm.read();
   cntr_map_hbm8["IN_SOP_TO_PF"] = pbc_csr.hbm.hbm_port_8.cnt_hbm.flits_sop_in().convert_to<uint64_t>();
   cntr_map_hbm8["IN_EOP_TO_PF"] = pbc_csr.hbm.hbm_port_8.cnt_hbm.flits_eop_in().convert_to<uint64_t>();
   cntr_map_hbm8["OUT_SOP_FROM_PF"] = pbc_csr.hbm.hbm_port_8.cnt_hbm.flits_sop_out().convert_to<uint64_t>();
   cntr_map_hbm8["OUT_EOP_FROM_PF"] = pbc_csr.hbm.hbm_port_8.cnt_hbm.flits_eop_out().convert_to<uint64_t>();
   cntr_map_hbm8["HBM_OCCUPANCY_DROP"] = pbc_csr.hbm.hbm_port_8.cnt_hbm.occupancy_drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm8_emergency_stop.read();
   cntr_map_hbm8["HBM_WDROP_EMERGENCY_STOP"] = pbc_csr.hbm.cnt_hbm8_emergency_stop.drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm8_write_ack_filling_up.read();
   cntr_map_hbm8["HBM_WDROP_WR_ACK_FILL_UP"] = pbc_csr.hbm.cnt_hbm8_write_ack_filling_up.drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm8_write_ack_full.read();
   cntr_map_hbm8["HBM_WDROP_WR_ACK_FULL"] = pbc_csr.hbm.cnt_hbm8_write_ack_full.drop().convert_to<uint64_t>();
   pbc_csr.hbm.cnt_hbm8_truncate.read();
   cntr_map_hbm8["HBM_WDROP_TRUNCATE"] = pbc_csr.hbm.cnt_hbm8_truncate.drop().convert_to<uint64_t>();
   pbc_csr.hbm.sat_hbm8_ctrl_full.read();
   cntr_map_hbm8["HBM_WDROP_CTRL_FULL"] = pbc_csr.hbm.sat_hbm8_ctrl_full.drop().convert_to<uint64_t>();
   pbc_csr.hbm.hbm_port_9.cnt_hbm.read();
   cntr_map_hbm9["IN_SOP_TO_PF"] = pbc_csr.hbm.hbm_port_9.cnt_hbm.flits_sop_in().convert_to<uint64_t>();
   cntr_map_hbm9["IN_EOP_TO_PF"] = pbc_csr.hbm.hbm_port_9.cnt_hbm.flits_eop_in().convert_to<uint64_t>();
   cntr_map_hbm9["OUT_SOP_FROM_PF"] = pbc_csr.hbm.hbm_port_9.cnt_hbm.flits_sop_out().convert_to<uint64_t>();
   cntr_map_hbm9["OUT_EOP_FROM_PF"] = pbc_csr.hbm.hbm_port_9.cnt_hbm.flits_eop_out().convert_to<uint64_t>();
   cntr_map_hbm9["HBM_OCCUPANCY_STOP"] = pbc_csr.hbm.hbm_port_9.cnt_hbm.occupancy_stop().convert_to<uint64_t>();
 if (print_ctxt) {
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 0, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx0[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx0[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx0[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 1, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx1[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx1[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx1[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 2, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx2[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx2[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx2[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 3, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx3[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx3[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx3[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 4, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx4[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx4[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx4[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 5, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx5[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx5[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx5[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 6, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx6[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx6[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx6[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 7, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx7[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx7[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx7[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 8, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx8[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx8[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx8[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 9, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx9[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx9[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx9[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 10, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx10[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx10[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx10[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 11, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx11[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx11[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx11[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 12, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx12[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx12[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx12[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 13, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx13[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx13[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx13[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 14, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx14[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx14[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx14[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 15, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx15[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx15[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx15[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 16, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx16[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx16[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx16[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 17, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx17[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx17[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx17[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 18, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx18[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx18[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx18[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 19, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx19[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx19[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx19[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 20, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx20[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx20[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx20[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 21, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx21[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx21[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx21[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 22, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx22[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx22[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx22[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 23, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx23[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx23[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx23[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 24, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx24[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx24[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx24[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 25, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx25[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx25[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx25[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 26, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx26[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx26[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx26[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 27, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx27[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx27[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx27[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 28, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx28[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx28[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx28[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 29, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx29[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx29[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx29[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 30, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx30[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx30[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx30[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 13, 31, good_count, error_count);
   sdata.str("");
   sdata << "IN_ETH" << "_GOOD";
   cntr_map_ctx31[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_ERROR";
   cntr_map_ctx31[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_ETH" << "_WATERMARK";
   cntr_map_ctx31[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 0, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx0[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx0[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx0[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 1, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx1[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx1[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx1[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 2, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx2[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx2[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx2[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 3, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx3[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx3[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx3[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 4, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx4[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx4[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx4[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 5, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx5[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx5[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx5[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 6, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx6[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx6[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx6[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 7, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx7[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx7[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx7[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 8, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx8[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx8[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx8[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 9, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx9[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx9[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx9[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 10, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx10[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx10[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx10[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 11, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx11[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx11[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx11[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 12, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx12[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx12[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx12[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 13, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx13[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx13[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx13[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 14, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx14[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx14[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx14[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 15, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx15[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx15[sdata.str()] = error_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx15[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 16, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx16[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx16[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx16[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 17, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx17[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx17[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx17[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 18, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx18[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx18[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx18[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 19, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx19[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx19[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx19[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 20, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx20[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx20[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx20[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 21, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx21[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx21[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx21[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 22, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx22[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx22[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx22[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 23, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx23[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx23[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx23[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 24, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx24[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx24[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx24[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 25, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx25[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx25[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx25[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 26, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx26[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx26[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx26[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 27, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx27[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx27[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx27[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 28, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx28[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx28[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx28[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 29, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx29[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx29[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx29[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 30, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx30[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx30[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx30[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 15, 31, good_count, error_count);
   sdata.str("");
   sdata << "IN_TDMA" << "_GOOD";
   cntr_map_ctx31[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_ERROR";
   cntr_map_ctx31[sdata.str()] = 0;
   sdata.str("");
   sdata << "IN_TDMA" << "_WATERMARK";
   cntr_map_ctx31[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 0, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx0[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx0[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx0[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 1, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx1[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx1[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx1[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 2, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx2[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx2[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx2[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 3, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx3[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx3[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx3[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 4, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx4[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx4[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx4[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 5, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx5[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx5[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx5[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 6, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx6[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx6[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx6[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 7, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx7[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx7[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx7[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 8, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx8[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx8[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx8[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 9, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx9[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx9[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx9[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 10, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx10[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx10[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx10[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 11, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx11[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx11[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx11[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 12, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx12[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx12[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx12[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 13, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx13[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx13[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx13[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 14, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx14[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx14[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx14[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 15, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx15[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx15[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx15[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 16, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx16[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx16[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx16[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 17, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx17[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx17[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx17[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 18, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx18[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx18[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx18[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 19, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx19[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx19[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx19[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 20, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx20[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx20[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx20[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 21, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx21[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx21[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx21[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 22, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx22[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx22[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx22[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 23, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx23[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx23[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx23[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 24, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx24[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx24[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx24[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 25, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx25[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx25[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx25[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 26, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx26[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx26[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx26[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 27, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx27[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx27[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx27[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 28, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx28[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx28[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx28[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 29, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx29[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx29[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx29[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 30, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx30[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx30[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx30[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 12, 31, good_count, error_count);
   sdata.str("");
   sdata << "OUT_ETH" << "_GOOD";
   cntr_map_ctx31[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_ETH" << "_ERROR";
   cntr_map_ctx31[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_ETH" << "_WATERMARK";
   cntr_map_ctx31[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 0, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx0[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx0[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx0[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 1, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx1[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx1[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx1[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 2, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx2[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx2[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx2[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 3, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx3[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx3[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx3[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 4, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx4[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx4[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx4[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 5, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx5[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx5[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx5[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 6, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx6[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx6[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx6[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 7, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx7[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx7[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx7[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 8, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx8[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx8[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx8[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 9, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx9[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx9[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx9[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 10, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx10[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx10[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx10[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 11, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx11[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx11[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx11[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 12, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx12[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx12[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx12[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 13, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx13[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx13[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx13[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 14, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx14[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx14[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx14[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 15, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx15[sdata.str()] = good_count.convert_to<uint64_t>();
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx15[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx15[sdata.str()] = error_count.convert_to<uint64_t>();
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 16, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx16[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx16[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx16[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 17, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx17[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx17[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx17[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 18, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx18[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx18[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx18[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 19, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx19[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx19[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx19[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 20, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx20[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx20[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx20[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 21, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx21[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx21[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx21[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 22, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx22[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx22[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx22[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 23, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx23[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx23[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx23[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 24, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx24[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx24[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx24[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 25, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx25[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx25[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx25[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 26, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx26[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx26[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx26[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 27, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx27[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx27[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx27[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 28, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx28[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx28[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx28[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 29, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx29[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx29[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx29[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 30, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx30[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx30[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx30[sdata.str()] = 0;
   cap_pb_read_hbm_ctx_stat(chip_id, inst_id, 14, 31, good_count, error_count);
   sdata.str("");
   sdata << "OUT_TDMA" << "_GOOD";
   cntr_map_ctx31[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_ERROR";
   cntr_map_ctx31[sdata.str()] = 0;
   sdata.str("");
   sdata << "OUT_TDMA" << "_WATERMARK";
   cntr_map_ctx31[sdata.str()] = 0;
 }
 if (print_qdepth) {
   pbc_csr.hbm.sta_hbm_eth_context_0.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx0[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_0.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_1.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx1[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_1.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_2.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx2[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_2.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_3.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx3[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_3.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_4.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx4[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_4.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_5.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx5[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_5.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_6.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx6[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_6.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_7.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx7[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_7.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_8.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx8[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_8.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_9.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx9[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_9.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_10.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx10[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_10.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_11.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx11[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_11.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_12.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx12[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_12.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_13.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx13[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_13.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_14.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx14[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_14.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_15.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx15[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_15.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_16.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx16[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_16.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_17.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx17[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_17.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_18.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx18[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_18.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_19.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx19[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_19.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_20.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx20[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_20.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_21.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx21[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_21.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_22.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx22[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_22.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_23.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx23[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_23.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_24.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx24[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_24.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_25.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx25[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_25.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_26.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx26[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_26.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_27.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx27[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_27.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_28.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx28[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_28.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_29.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx29[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_29.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_30.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx30[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_30.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_31.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx31[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_31.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_tx_context_0.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx0[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_0.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_tx_context_1.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx1[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_1.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_tx_context_2.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx2[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_2.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_tx_context_3.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx3[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_3.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_tx_context_4.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx4[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_4.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_tx_context_5.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx5[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_5.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_tx_context_6.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx6[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_6.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_tx_context_7.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx7[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_7.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_tx_context_8.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx8[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_8.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_tx_context_9.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx9[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_9.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_tx_context_10.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx10[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_10.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_tx_context_11.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx11[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_11.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_tx_context_12.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx12[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_12.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_tx_context_13.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx13[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_13.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_tx_context_14.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx14[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_14.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_tx_context_15.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx15[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_15.depth().convert_to<uint64_t>();
   sdata.str("");
 }

   PB_PRN_MSG(setw(125) << "========================================================= PB CNTRS ==============================================================\n");
   PB_PRN_MSG(setw(39) << "port." << 0 << setw(14) << "port." << 1 << setw(14) << "port." << 2 << setw(14) << "port." << 3 << setw(14) << "port." << 4 << setw(14) << "port." << 5 << "\n\n");
   for(auto i : cntr_map0) {
     string counter_name = i.first;
     counter_name.append(25-counter_name.length(),' ');
     stringstream sdata;
     sdata.str("");
     stringstream rval0;
     if (i.second == 0) {
       rval0 << "-";
     } else {
       if (is_hex) {
          rval0 << "0x" << hex << i.second;
       } else {
          rval0 << dec << i.second;
       }
     }
     stringstream rval1;
     rval1.str("-");
     if (cntr_map1[i.first] != 0) {
       if (is_hex) {
          rval1 << "0x" << hex << cntr_map1[i.first];
       } else {
          rval1 << dec << cntr_map1[i.first];
       }
     }
     stringstream rval2;
     rval2.str("-");
     if (cntr_map2[i.first] != 0) {
       if (is_hex) {
          rval2 << "0x" << hex << cntr_map2[i.first];
       } else {
          rval2 << dec << cntr_map2[i.first];
       }
     }
     stringstream rval3;
     rval3.str("-");
     if (cntr_map3[i.first] != 0) {
       if (is_hex) {
          rval3 << "0x" << hex << cntr_map3[i.first];
       } else {
          rval3 << dec << cntr_map3[i.first];
       }
     }
     stringstream rval4;
     rval4.str("-");
     if (cntr_map4[i.first] != 0) {
       if (is_hex) {
          rval4 << "0x" << hex << cntr_map4[i.first];
       } else {
          rval4 << dec << cntr_map4[i.first];
       }
     }
     stringstream rval5;
     rval5.str("-");
     if (cntr_map5[i.first] != 0) {
       if (is_hex) {
          rval5 << "0x" << hex << cntr_map5[i.first];
       } else {
          rval5 << dec << cntr_map5[i.first];
       }
     }
     sdata << setw(15) << rval0.str() << setw(15) << rval1.str() << setw(15) << rval2.str() << setw(15) << rval3.str() << setw(15) << rval4.str() << setw(15) << rval5.str();
     PB_PRN_MSG(setw(25) << counter_name << sdata.str() << "\n");
   }
   PB_PRN_MSG(setw(125) << "------------------------------------------------------------------------------------------------------------------------------\n");
   for(auto i : cntr_map_hbm0) {
     string counter_name = i.first;
     counter_name.append(25-counter_name.length(),' ');
     stringstream sdata;
     sdata.str("");
     stringstream rval0;
     if (i.second == 0) {
       rval0 << "-";
     } else {
       if (is_hex) {
          rval0 << "0x" << hex << i.second;
       } else {
          rval0 << dec << i.second;
       }
     }
     stringstream rval1;
     rval1.str("-");
     if (cntr_map_hbm1[i.first] != 0) {
       if (is_hex) {
          rval1 << "0x" << hex << cntr_map_hbm1[i.first];
       } else {
          rval1 << dec << cntr_map_hbm1[i.first];
       }
     }
     stringstream rval2;
     rval2.str("-");
     if (cntr_map_hbm2[i.first] != 0) {
       if (is_hex) {
          rval2 << "0x" << hex << cntr_map_hbm2[i.first];
       } else {
          rval2 << dec << cntr_map_hbm2[i.first];
       }
     }
     stringstream rval3;
     rval3.str("-");
     if (cntr_map_hbm3[i.first] != 0) {
       if (is_hex) {
          rval3 << "0x" << hex << cntr_map_hbm3[i.first];
       } else {
          rval3 << dec << cntr_map_hbm3[i.first];
       }
     }
     stringstream rval4;
     rval4.str("-");
     if (cntr_map_hbm4[i.first] != 0) {
       if (is_hex) {
          rval4 << "0x" << hex << cntr_map_hbm4[i.first];
       } else {
          rval4 << dec << cntr_map_hbm4[i.first];
       }
     }
     stringstream rval5;
     rval5.str("-");
     if (cntr_map_hbm5[i.first] != 0) {
       if (is_hex) {
          rval5 << "0x" << hex << cntr_map_hbm5[i.first];
       } else {
          rval5 << dec << cntr_map_hbm5[i.first];
       }
     }
     sdata << setw(15) << rval0.str() << setw(15) << rval1.str() << setw(15) << rval2.str() << setw(15) << rval3.str() << setw(15) << rval4.str() << setw(15) << rval5.str();
     PB_PRN_MSG(setw(25) << counter_name << sdata.str() << "\n");
   }
   PB_PRN_MSG(setw(125) << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
   PB_PRN_MSG(setw(39) << "port." << 6 << setw(14) << "port." << 7 << setw(14) << "port." << 8 << setw(14) << "port." << 9 << setw(13) << "port." << 10 << setw(13) << "port." << 11 << "\n\n");
   for(auto i : cntr_map6) {
     string counter_name = i.first;
     counter_name.append(25-counter_name.length(),' ');
     stringstream sdata;
     sdata.str("");
     stringstream rval0;
     if (i.second == 0) {
       rval0 << "-";
     } else {
       if (is_hex) {
          rval0 << "0x" << hex << i.second;
       } else {
          rval0 << dec << i.second;
       }
     }
     stringstream rval1;
     rval1.str("-");
     if (cntr_map7[i.first] != 0) {
       if (is_hex) {
          rval1 << "0x" << hex << cntr_map7[i.first];
       } else {
          rval1 << dec << cntr_map7[i.first];
       }
     }
     stringstream rval2;
     rval2.str("-");
     if (cntr_map8[i.first] != 0) {
       if (is_hex) {
          rval2 << "0x" << hex << cntr_map8[i.first];
       } else {
          rval2 << dec << cntr_map8[i.first];
       }
     }
     stringstream rval3;
     rval3.str("-");
     if (cntr_map9[i.first] != 0) {
       if (is_hex) {
          rval3 << "0x" << hex << cntr_map9[i.first];
       } else {
          rval3 << dec << cntr_map9[i.first];
       }
     }
     stringstream rval4;
     rval4.str("-");
     if (cntr_map10[i.first] != 0) {
       if (is_hex) {
          rval4 << "0x" << hex << cntr_map10[i.first];
       } else {
          rval4 << dec << cntr_map10[i.first];
       }
     }
     stringstream rval5;
     rval5.str("-");
     if (cntr_map11[i.first] != 0) {
       if (is_hex) {
          rval5 << "0x" << hex << cntr_map11[i.first];
       } else {
          rval5 << dec << cntr_map11[i.first];
       }
     }
     sdata << setw(15) << rval0.str() << setw(15) << rval1.str() << setw(15) << rval2.str() << setw(15) << rval3.str() << setw(15) << rval4.str() << setw(15) << rval5.str();
     PB_PRN_MSG(setw(25) << counter_name << sdata.str() << "\n");
   }
   PB_PRN_MSG(setw(125) << "------------------------------------------------------------------------------------------------------------------------------\n");
   for(auto i : cntr_map_hbm6) {
     string counter_name = i.first;
     counter_name.append(25-counter_name.length(),' ');
     stringstream sdata;
     sdata.str("");
     stringstream rval0;
     if (i.second == 0) {
       rval0 << "-";
     } else {
       if (is_hex) {
          rval0 << "0x" << hex << i.second;
       } else {
          rval0 << dec << i.second;
       }
     }
     stringstream rval1;
     rval1.str("-");
     if (cntr_map_hbm7[i.first] != 0) {
       if (is_hex) {
          rval1 << "0x" << hex << cntr_map_hbm7[i.first];
       } else {
          rval1 << dec << cntr_map_hbm7[i.first];
       }
     }
     stringstream rval2;
     rval2.str("-");
     if (cntr_map_hbm8[i.first] != 0) {
       if (is_hex) {
          rval2 << "0x" << hex << cntr_map_hbm8[i.first];
       } else {
          rval2 << dec << cntr_map_hbm8[i.first];
       }
     }
     stringstream rval3;
     rval3.str("-");
     if (cntr_map_hbm9[i.first] != 0) {
       if (is_hex) {
          rval3 << "0x" << hex << cntr_map_hbm9[i.first];
       } else {
          rval3 << dec << cntr_map_hbm9[i.first];
       }
     }
     sdata << setw(15) << rval0.str() << setw(15) << rval1.str() << setw(15) << rval2.str() << setw(15) << rval3.str();
     PB_PRN_MSG(setw(25) << counter_name << sdata.str() << "\n");
   }
 if (print_ctxt) {
   PB_PRN_MSG(setw(125) << "################################################################################################################################\n");
   PB_PRN_MSG(setw(36) << "ctxt." << 0 << setw(11) << "ctxt." << 1 << setw(11) << "ctxt." << 2 << setw(11) << "ctxt." << 3 << setw(11) << "ctxt." << 4 << setw(11) << "ctxt." << 5 << setw(11) << "ctxt." << 6 << setw(11) << "ctxt." << 7 << "\n\n");
   for(auto i : cntr_map_ctx0) {
     string counter_name = i.first;
     counter_name.append(25-counter_name.length(),' ');
     stringstream sdata;
     sdata.str("");
     stringstream rval0;
     if (i.second == 0) {
       rval0 << "-";
     } else {
       if (is_hex) {
          rval0 << "0x" << hex << i.second;
       } else {
          rval0 << dec << i.second;
       }
     }
     stringstream rval1;
     rval1.str("-");
     if (cntr_map_ctx1[i.first] != 0) {
       if (is_hex) {
          rval1 << "0x" << hex << cntr_map_ctx1[i.first];
       } else {
          rval1 << dec << cntr_map_ctx1[i.first];
       }
     }
     stringstream rval2;
     rval2.str("-");
     if (cntr_map_ctx2[i.first] != 0) {
       if (is_hex) {
          rval2 << "0x" << hex << cntr_map_ctx2[i.first];
       } else {
          rval2 << dec << cntr_map_ctx2[i.first];
       }
     }
     stringstream rval3;
     rval3.str("-");
     if (cntr_map_ctx3[i.first] != 0) {
       if (is_hex) {
          rval3 << "0x" << hex << cntr_map_ctx3[i.first];
       } else {
          rval3 << dec << cntr_map_ctx3[i.first];
       }
     }
     stringstream rval4;
     rval4.str("-");
     if (cntr_map_ctx4[i.first] != 0) {
       if (is_hex) {
          rval4 << "0x" << hex << cntr_map_ctx4[i.first];
       } else {
          rval4 << dec << cntr_map_ctx4[i.first];
       }
     }
     stringstream rval5;
     rval5.str("-");
     if (cntr_map_ctx5[i.first] != 0) {
       if (is_hex) {
          rval5 << "0x" << hex << cntr_map_ctx5[i.first];
       } else {
          rval5 << dec << cntr_map_ctx5[i.first];
       }
     }
     stringstream rval6;
     rval6.str("-");
     if (cntr_map_ctx6[i.first] != 0) {
       if (is_hex) {
          rval6 << "0x" << hex << cntr_map_ctx6[i.first];
       } else {
          rval6 << dec << cntr_map_ctx6[i.first];
       }
     }
     stringstream rval7;
     rval7.str("-");
     if (cntr_map_ctx7[i.first] != 0) {
       if (is_hex) {
          rval7 << "0x" << hex << cntr_map_ctx7[i.first];
       } else {
          rval7 << dec << cntr_map_ctx7[i.first];
       }
     }
     sdata << setw(12) << rval0.str() << setw(12) << rval1.str() << setw(12) << rval2.str() << setw(12) << rval3.str() << setw(12) << rval4.str() << setw(12) << rval5.str() << setw(12) << rval6.str() << setw(12) << rval7.str();
     PB_PRN_MSG(setw(25) << counter_name << sdata.str() << "\n");
   }
   PB_PRN_MSG(setw(36) << "ctxt." << 8 << setw(11) << "ctxt." << 9 << setw(11) << "ctxt." << 10 << setw(10) << "ctxt." << 11 << setw(10) << "ctxt." << 12 << setw(10) << "ctxt." << 13 << setw(10) << "ctxt." << 14 << setw(10) << "ctxt." << 15 << "\n\n");
   for(auto i : cntr_map_ctx8) {
     string counter_name = i.first;
     counter_name.append(25-counter_name.length(),' ');
     stringstream sdata;
     sdata.str("");
     stringstream rval0;
     if (i.second == 0) {
       rval0 << "-";
     } else {
       if (is_hex) {
          rval0 << "0x" << hex << i.second;
       } else {
          rval0 << dec << i.second;
       }
     }
     stringstream rval1;
     rval1.str("-");
     if (cntr_map_ctx9[i.first] != 0) {
       if (is_hex) {
          rval1 << "0x" << hex << cntr_map_ctx9[i.first];
       } else {
          rval1 << dec << cntr_map_ctx9[i.first];
       }
     }
     stringstream rval2;
     rval2.str("-");
     if (cntr_map_ctx10[i.first] != 0) {
       if (is_hex) {
          rval2 << "0x" << hex << cntr_map_ctx10[i.first];
       } else {
          rval2 << dec << cntr_map_ctx10[i.first];
       }
     }
     stringstream rval3;
     rval3.str("-");
     if (cntr_map_ctx11[i.first] != 0) {
       if (is_hex) {
          rval3 << "0x" << hex << cntr_map_ctx11[i.first];
       } else {
          rval3 << dec << cntr_map_ctx11[i.first];
       }
     }
     stringstream rval4;
     rval4.str("-");
     if (cntr_map_ctx12[i.first] != 0) {
       if (is_hex) {
          rval4 << "0x" << hex << cntr_map_ctx12[i.first];
       } else {
          rval4 << dec << cntr_map_ctx12[i.first];
       }
     }
     stringstream rval5;
     rval5.str("-");
     if (cntr_map_ctx13[i.first] != 0) {
       if (is_hex) {
          rval5 << "0x" << hex << cntr_map_ctx13[i.first];
       } else {
          rval5 << dec << cntr_map_ctx13[i.first];
       }
     }
     stringstream rval6;
     rval6.str("-");
     if (cntr_map_ctx14[i.first] != 0) {
       if (is_hex) {
          rval6 << "0x" << hex << cntr_map_ctx14[i.first];
       } else {
          rval6 << dec << cntr_map_ctx14[i.first];
       }
     }
     stringstream rval7;
     rval7.str("-");
     if (cntr_map_ctx15[i.first] != 0) {
       if (is_hex) {
          rval7 << "0x" << hex << cntr_map_ctx15[i.first];
       } else {
          rval7 << dec << cntr_map_ctx15[i.first];
       }
     }
     sdata << setw(12) << rval0.str() << setw(12) << rval1.str() << setw(12) << rval2.str() << setw(12) << rval3.str() << setw(12) << rval4.str() << setw(12) << rval5.str() << setw(12) << rval6.str() << setw(12) << rval7.str();
     PB_PRN_MSG(setw(25) << counter_name << sdata.str() << "\n");
   }
   PB_PRN_MSG(setw(36) << "ctxt." << 16 << setw(10) << "ctxt." << 17 << setw(10) << "ctxt." << 18 << setw(10) << "ctxt." << 19 << setw(10) << "ctxt." << 20 << setw(10) << "ctxt." << 21 << setw(10) << "ctxt." << 22 << setw(10) << "ctxt." << 23 << "\n\n");
   for(auto i : cntr_map_ctx16) {
     string counter_name = i.first;
     counter_name.append(25-counter_name.length(),' ');
     stringstream sdata;
     sdata.str("");
     stringstream rval0;
     if (i.second == 0) {
       rval0 << "-";
     } else {
       if (is_hex) {
          rval0 << "0x" << hex << i.second;
       } else {
          rval0 << dec << i.second;
       }
     }
     stringstream rval1;
     rval1.str("-");
     if (cntr_map_ctx17[i.first] != 0) {
       if (is_hex) {
          rval1 << "0x" << hex << cntr_map_ctx17[i.first];
       } else {
          rval1 << dec << cntr_map_ctx17[i.first];
       }
     }
     stringstream rval2;
     rval2.str("-");
     if (cntr_map_ctx18[i.first] != 0) {
       if (is_hex) {
          rval2 << "0x" << hex << cntr_map_ctx18[i.first];
       } else {
          rval2 << dec << cntr_map_ctx18[i.first];
       }
     }
     stringstream rval3;
     rval3.str("-");
     if (cntr_map_ctx19[i.first] != 0) {
       if (is_hex) {
          rval3 << "0x" << hex << cntr_map_ctx19[i.first];
       } else {
          rval3 << dec << cntr_map_ctx19[i.first];
       }
     }
     stringstream rval4;
     rval4.str("-");
     if (cntr_map_ctx20[i.first] != 0) {
       if (is_hex) {
          rval4 << "0x" << hex << cntr_map_ctx20[i.first];
       } else {
          rval4 << dec << cntr_map_ctx20[i.first];
       }
     }
     stringstream rval5;
     rval5.str("-");
     if (cntr_map_ctx21[i.first] != 0) {
       if (is_hex) {
          rval5 << "0x" << hex << cntr_map_ctx21[i.first];
       } else {
          rval5 << dec << cntr_map_ctx21[i.first];
       }
     }
     stringstream rval6;
     rval6.str("-");
     if (cntr_map_ctx22[i.first] != 0) {
       if (is_hex) {
          rval6 << "0x" << hex << cntr_map_ctx22[i.first];
       } else {
          rval6 << dec << cntr_map_ctx22[i.first];
       }
     }
     stringstream rval7;
     rval7.str("-");
     if (cntr_map_ctx23[i.first] != 0) {
       if (is_hex) {
          rval7 << "0x" << hex << cntr_map_ctx23[i.first];
       } else {
          rval7 << dec << cntr_map_ctx23[i.first];
       }
     }
     sdata << setw(12) << rval0.str() << setw(12) << rval1.str() << setw(12) << rval2.str() << setw(12) << rval3.str() << setw(12) << rval4.str() << setw(12) << rval5.str() << setw(12) << rval6.str() << setw(12) << rval7.str();
     PB_PRN_MSG(setw(25) << counter_name << sdata.str() << "\n");
   }
   PB_PRN_MSG(setw(36) << "ctxt." << 24 << setw(10) << "ctxt." << 25 << setw(10) << "ctxt." << 26 << setw(10) << "ctxt." << 27 << setw(10) << "ctxt." << 28 << setw(10) << "ctxt." << 29 << setw(10) << "ctxt." << 30 << setw(10) << "ctxt." << 31 << "\n\n");
   for(auto i : cntr_map_ctx24) {
     string counter_name = i.first;
     counter_name.append(25-counter_name.length(),' ');
     stringstream sdata;
     sdata.str("");
     stringstream rval0;
     if (i.second == 0) {
       rval0 << "-";
     } else {
       if (is_hex) {
          rval0 << "0x" << hex << i.second;
       } else {
          rval0 << dec << i.second;
       }
     }
     stringstream rval1;
     rval1.str("-");
     if (cntr_map_ctx25[i.first] != 0) {
       if (is_hex) {
          rval1 << "0x" << hex << cntr_map_ctx25[i.first];
       } else {
          rval1 << dec << cntr_map_ctx25[i.first];
       }
     }
     stringstream rval2;
     rval2.str("-");
     if (cntr_map_ctx26[i.first] != 0) {
       if (is_hex) {
          rval2 << "0x" << hex << cntr_map_ctx26[i.first];
       } else {
          rval2 << dec << cntr_map_ctx26[i.first];
       }
     }
     stringstream rval3;
     rval3.str("-");
     if (cntr_map_ctx27[i.first] != 0) {
       if (is_hex) {
          rval3 << "0x" << hex << cntr_map_ctx27[i.first];
       } else {
          rval3 << dec << cntr_map_ctx27[i.first];
       }
     }
     stringstream rval4;
     rval4.str("-");
     if (cntr_map_ctx28[i.first] != 0) {
       if (is_hex) {
          rval4 << "0x" << hex << cntr_map_ctx28[i.first];
       } else {
          rval4 << dec << cntr_map_ctx28[i.first];
       }
     }
     stringstream rval5;
     rval5.str("-");
     if (cntr_map_ctx29[i.first] != 0) {
       if (is_hex) {
          rval5 << "0x" << hex << cntr_map_ctx29[i.first];
       } else {
          rval5 << dec << cntr_map_ctx29[i.first];
       }
     }
     stringstream rval6;
     rval6.str("-");
     if (cntr_map_ctx30[i.first] != 0) {
       if (is_hex) {
          rval6 << "0x" << hex << cntr_map_ctx30[i.first];
       } else {
          rval6 << dec << cntr_map_ctx30[i.first];
       }
     }
     stringstream rval7;
     rval7.str("-");
     if (cntr_map_ctx31[i.first] != 0) {
       if (is_hex) {
          rval7 << "0x" << hex << cntr_map_ctx31[i.first];
       } else {
          rval7 << dec << cntr_map_ctx31[i.first];
       }
     }
     sdata << setw(12) << rval0.str() << setw(12) << rval1.str() << setw(12) << rval2.str() << setw(12) << rval3.str() << setw(12) << rval4.str() << setw(12) << rval5.str() << setw(12) << rval6.str() << setw(12) << rval7.str();
     PB_PRN_MSG(setw(25) << counter_name << sdata.str() << "\n");
   }
 }
 PB_PRN_MSG(setw(125) << "================================================================================================================================\n");

 cpu::access()->set_access_type(cur_cpu_access_type);
}

void pb_print_msg(string msg) {
#ifdef _CSV_INCLUDED_
    vpi_printf((PLI_BYTE8*) "%s", msg.c_str());
#else 
    std::cout << msg;
#endif    
}

void cap_pb_clr_cntrs(int chip_id, int inst_id, int clr_ctxt) {

 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, 0);

   pbc_csr.cnt_flits[0].all(0);
   pbc_csr.cnt_flits[0].write();

   pbc_csr.sat_write_error[0].all(0);
   pbc_csr.sat_write_error[0].write();
   pbc_csr.cnt_flits[1].all(0);
   pbc_csr.cnt_flits[1].write();

   pbc_csr.sat_write_error[1].all(0);
   pbc_csr.sat_write_error[1].write();
   pbc_csr.cnt_flits[2].all(0);
   pbc_csr.cnt_flits[2].write();

   pbc_csr.sat_write_error[2].all(0);
   pbc_csr.sat_write_error[2].write();
   pbc_csr.cnt_flits[3].all(0);
   pbc_csr.cnt_flits[3].write();

   pbc_csr.sat_write_error[3].all(0);
   pbc_csr.sat_write_error[3].write();
   pbc_csr.cnt_flits[4].all(0);
   pbc_csr.cnt_flits[4].write();

   pbc_csr.sat_write_error[4].all(0);
   pbc_csr.sat_write_error[4].write();
   pbc_csr.cnt_flits[5].all(0);
   pbc_csr.cnt_flits[5].write();

   pbc_csr.sat_write_error[5].all(0);
   pbc_csr.sat_write_error[5].write();
   pbc_csr.cnt_flits[6].all(0);
   pbc_csr.cnt_flits[6].write();

   pbc_csr.sat_write_error[6].all(0);
   pbc_csr.sat_write_error[6].write();
   pbc_csr.cnt_flits[7].all(0);
   pbc_csr.cnt_flits[7].write();

   pbc_csr.sat_write_error[7].all(0);
   pbc_csr.sat_write_error[7].write();
   pbc_csr.cnt_flits[8].all(0);
   pbc_csr.cnt_flits[8].write();

   pbc_csr.sat_write_error[8].all(0);
   pbc_csr.sat_write_error[8].write();
   pbc_csr.cnt_flits[9].all(0);
   pbc_csr.cnt_flits[9].write();

   pbc_csr.sat_write_error[9].all(0);
   pbc_csr.sat_write_error[9].write();
   pbc_csr.cnt_flits[10].all(0);
   pbc_csr.cnt_flits[10].write();

   pbc_csr.sat_write_error[10].all(0);
   pbc_csr.sat_write_error[10].write();
   pbc_csr.cnt_flits[11].all(0);
   pbc_csr.cnt_flits[11].write();

   pbc_csr.sat_write_error[11].all(0);
   pbc_csr.sat_write_error[11].write();
   pbc_csr.hbm.hbm_port_0.cnt_hbm.all(0);
   pbc_csr.hbm.hbm_port_0.cnt_hbm.write();

   pbc_csr.hbm.cnt_hbm0_emergency_stop.all(0);
   pbc_csr.hbm.cnt_hbm0_emergency_stop.write();
   pbc_csr.hbm.cnt_hbm0_write_ack_filling_up.all(0);
   pbc_csr.hbm.cnt_hbm0_write_ack_filling_up.write();
   pbc_csr.hbm.cnt_hbm0_write_ack_full.all(0);
   pbc_csr.hbm.cnt_hbm0_write_ack_full.write();
   pbc_csr.hbm.cnt_hbm0_truncate.all(0);
   pbc_csr.hbm.cnt_hbm0_truncate.write();
   pbc_csr.hbm.sat_hbm0_ctrl_full.all(0);
   pbc_csr.hbm.sat_hbm0_ctrl_full.write();
   pbc_csr.hbm.hbm_port_1.cnt_hbm.all(0);
   pbc_csr.hbm.hbm_port_1.cnt_hbm.write();

   pbc_csr.hbm.cnt_hbm1_emergency_stop.all(0);
   pbc_csr.hbm.cnt_hbm1_emergency_stop.write();
   pbc_csr.hbm.cnt_hbm1_write_ack_filling_up.all(0);
   pbc_csr.hbm.cnt_hbm1_write_ack_filling_up.write();
   pbc_csr.hbm.cnt_hbm1_write_ack_full.all(0);
   pbc_csr.hbm.cnt_hbm1_write_ack_full.write();
   pbc_csr.hbm.cnt_hbm1_truncate.all(0);
   pbc_csr.hbm.cnt_hbm1_truncate.write();
   pbc_csr.hbm.sat_hbm1_ctrl_full.all(0);
   pbc_csr.hbm.sat_hbm1_ctrl_full.write();
   pbc_csr.hbm.hbm_port_2.cnt_hbm.all(0);
   pbc_csr.hbm.hbm_port_2.cnt_hbm.write();

   pbc_csr.hbm.cnt_hbm2_emergency_stop.all(0);
   pbc_csr.hbm.cnt_hbm2_emergency_stop.write();
   pbc_csr.hbm.cnt_hbm2_write_ack_filling_up.all(0);
   pbc_csr.hbm.cnt_hbm2_write_ack_filling_up.write();
   pbc_csr.hbm.cnt_hbm2_write_ack_full.all(0);
   pbc_csr.hbm.cnt_hbm2_write_ack_full.write();
   pbc_csr.hbm.cnt_hbm2_truncate.all(0);
   pbc_csr.hbm.cnt_hbm2_truncate.write();
   pbc_csr.hbm.sat_hbm2_ctrl_full.all(0);
   pbc_csr.hbm.sat_hbm2_ctrl_full.write();
   pbc_csr.hbm.hbm_port_3.cnt_hbm.all(0);
   pbc_csr.hbm.hbm_port_3.cnt_hbm.write();

   pbc_csr.hbm.cnt_hbm3_emergency_stop.all(0);
   pbc_csr.hbm.cnt_hbm3_emergency_stop.write();
   pbc_csr.hbm.cnt_hbm3_write_ack_filling_up.all(0);
   pbc_csr.hbm.cnt_hbm3_write_ack_filling_up.write();
   pbc_csr.hbm.cnt_hbm3_write_ack_full.all(0);
   pbc_csr.hbm.cnt_hbm3_write_ack_full.write();
   pbc_csr.hbm.cnt_hbm3_truncate.all(0);
   pbc_csr.hbm.cnt_hbm3_truncate.write();
   pbc_csr.hbm.sat_hbm3_ctrl_full.all(0);
   pbc_csr.hbm.sat_hbm3_ctrl_full.write();
   pbc_csr.hbm.hbm_port_4.cnt_hbm.all(0);
   pbc_csr.hbm.hbm_port_4.cnt_hbm.write();

   pbc_csr.hbm.cnt_hbm4_emergency_stop.all(0);
   pbc_csr.hbm.cnt_hbm4_emergency_stop.write();
   pbc_csr.hbm.cnt_hbm4_write_ack_filling_up.all(0);
   pbc_csr.hbm.cnt_hbm4_write_ack_filling_up.write();
   pbc_csr.hbm.cnt_hbm4_write_ack_full.all(0);
   pbc_csr.hbm.cnt_hbm4_write_ack_full.write();
   pbc_csr.hbm.cnt_hbm4_truncate.all(0);
   pbc_csr.hbm.cnt_hbm4_truncate.write();
   pbc_csr.hbm.sat_hbm4_ctrl_full.all(0);
   pbc_csr.hbm.sat_hbm4_ctrl_full.write();
   pbc_csr.hbm.hbm_port_5.cnt_hbm.all(0);
   pbc_csr.hbm.hbm_port_5.cnt_hbm.write();

   pbc_csr.hbm.cnt_hbm5_emergency_stop.all(0);
   pbc_csr.hbm.cnt_hbm5_emergency_stop.write();
   pbc_csr.hbm.cnt_hbm5_write_ack_filling_up.all(0);
   pbc_csr.hbm.cnt_hbm5_write_ack_filling_up.write();
   pbc_csr.hbm.cnt_hbm5_write_ack_full.all(0);
   pbc_csr.hbm.cnt_hbm5_write_ack_full.write();
   pbc_csr.hbm.cnt_hbm5_truncate.all(0);
   pbc_csr.hbm.cnt_hbm5_truncate.write();
   pbc_csr.hbm.sat_hbm5_ctrl_full.all(0);
   pbc_csr.hbm.sat_hbm5_ctrl_full.write();
   pbc_csr.hbm.hbm_port_6.cnt_hbm.all(0);
   pbc_csr.hbm.hbm_port_6.cnt_hbm.write();

   pbc_csr.hbm.cnt_hbm6_emergency_stop.all(0);
   pbc_csr.hbm.cnt_hbm6_emergency_stop.write();
   pbc_csr.hbm.cnt_hbm6_write_ack_filling_up.all(0);
   pbc_csr.hbm.cnt_hbm6_write_ack_filling_up.write();
   pbc_csr.hbm.cnt_hbm6_write_ack_full.all(0);
   pbc_csr.hbm.cnt_hbm6_write_ack_full.write();
   pbc_csr.hbm.cnt_hbm6_truncate.all(0);
   pbc_csr.hbm.cnt_hbm6_truncate.write();
   pbc_csr.hbm.sat_hbm6_ctrl_full.all(0);
   pbc_csr.hbm.sat_hbm6_ctrl_full.write();
   pbc_csr.hbm.hbm_port_7.cnt_hbm.all(0);
   pbc_csr.hbm.hbm_port_7.cnt_hbm.write();

   pbc_csr.hbm.cnt_hbm7_emergency_stop.all(0);
   pbc_csr.hbm.cnt_hbm7_emergency_stop.write();
   pbc_csr.hbm.cnt_hbm7_write_ack_filling_up.all(0);
   pbc_csr.hbm.cnt_hbm7_write_ack_filling_up.write();
   pbc_csr.hbm.cnt_hbm7_write_ack_full.all(0);
   pbc_csr.hbm.cnt_hbm7_write_ack_full.write();
   pbc_csr.hbm.cnt_hbm7_truncate.all(0);
   pbc_csr.hbm.cnt_hbm7_truncate.write();
   pbc_csr.hbm.sat_hbm7_ctrl_full.all(0);
   pbc_csr.hbm.sat_hbm7_ctrl_full.write();
   pbc_csr.hbm.hbm_port_8.cnt_hbm.all(0);
   pbc_csr.hbm.hbm_port_8.cnt_hbm.write();

   pbc_csr.hbm.cnt_hbm8_emergency_stop.all(0);
   pbc_csr.hbm.cnt_hbm8_emergency_stop.write();
   pbc_csr.hbm.cnt_hbm8_write_ack_filling_up.all(0);
   pbc_csr.hbm.cnt_hbm8_write_ack_filling_up.write();
   pbc_csr.hbm.cnt_hbm8_write_ack_full.all(0);
   pbc_csr.hbm.cnt_hbm8_write_ack_full.write();
   pbc_csr.hbm.cnt_hbm8_truncate.all(0);
   pbc_csr.hbm.cnt_hbm8_truncate.write();
   pbc_csr.hbm.sat_hbm8_ctrl_full.all(0);
   pbc_csr.hbm.sat_hbm8_ctrl_full.write();
   pbc_csr.hbm.hbm_port_9.cnt_hbm.all(0);
   pbc_csr.hbm.hbm_port_9.cnt_hbm.write();

 if (clr_ctxt) {

   pbc_csr.hbm.cfg_hbm_eth_ctrl_init.all(0);
   pbc_csr.hbm.cfg_hbm_eth_ctrl_init.write();
   pbc_csr.hbm.cfg_hbm_eth_ctrl_init.all(3);
   pbc_csr.hbm.cfg_hbm_eth_ctrl_init.write();
   pbc_csr.hbm.cfg_hbm_eth_ctrl_init.all(0);
   pbc_csr.hbm.cfg_hbm_eth_ctrl_init.write();

   SLEEP(5);
   pbc_csr.hbm.cfg_hbm_tx_ctrl_init.all(0);
   pbc_csr.hbm.cfg_hbm_tx_ctrl_init.write();
   pbc_csr.hbm.cfg_hbm_tx_ctrl_init.all(3);
   pbc_csr.hbm.cfg_hbm_tx_ctrl_init.write();
   pbc_csr.hbm.cfg_hbm_tx_ctrl_init.all(0);
   pbc_csr.hbm.cfg_hbm_tx_ctrl_init.write();
 }
}

void cap_pb_dump_qdepth(int chip_id, int inst_id, int is_hex) {

 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, 0);

 map <string, uint64_t> cntr_map_ctx0;
 map <string, uint64_t> cntr_map_ctx1;
 map <string, uint64_t> cntr_map_ctx2;
 map <string, uint64_t> cntr_map_ctx3;
 map <string, uint64_t> cntr_map_ctx4;
 map <string, uint64_t> cntr_map_ctx5;
 map <string, uint64_t> cntr_map_ctx6;
 map <string, uint64_t> cntr_map_ctx7;
 map <string, uint64_t> cntr_map_ctx8;
 map <string, uint64_t> cntr_map_ctx9;
 map <string, uint64_t> cntr_map_ctx10;
 map <string, uint64_t> cntr_map_ctx11;
 map <string, uint64_t> cntr_map_ctx12;
 map <string, uint64_t> cntr_map_ctx13;
 map <string, uint64_t> cntr_map_ctx14;
 map <string, uint64_t> cntr_map_ctx15;
 map <string, uint64_t> cntr_map_ctx16;
 map <string, uint64_t> cntr_map_ctx17;
 map <string, uint64_t> cntr_map_ctx18;
 map <string, uint64_t> cntr_map_ctx19;
 map <string, uint64_t> cntr_map_ctx20;
 map <string, uint64_t> cntr_map_ctx21;
 map <string, uint64_t> cntr_map_ctx22;
 map <string, uint64_t> cntr_map_ctx23;
 map <string, uint64_t> cntr_map_ctx24;
 map <string, uint64_t> cntr_map_ctx25;
 map <string, uint64_t> cntr_map_ctx26;
 map <string, uint64_t> cntr_map_ctx27;
 map <string, uint64_t> cntr_map_ctx28;
 map <string, uint64_t> cntr_map_ctx29;
 map <string, uint64_t> cntr_map_ctx30;
 map <string, uint64_t> cntr_map_ctx31;
 stringstream sdata;
   pbc_csr.hbm.sta_hbm_eth_context_0.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx0[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_0.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_1.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx1[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_1.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_2.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx2[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_2.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_3.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx3[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_3.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_4.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx4[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_4.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_5.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx5[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_5.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_6.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx6[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_6.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_7.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx7[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_7.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_8.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx8[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_8.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_9.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx9[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_9.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_10.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx10[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_10.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_11.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx11[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_11.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_12.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx12[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_12.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_13.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx13[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_13.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_14.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx14[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_14.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_15.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx15[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_15.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_16.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx16[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_16.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_17.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx17[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_17.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_18.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx18[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_18.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_19.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx19[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_19.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_20.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx20[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_20.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_21.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx21[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_21.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_22.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx22[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_22.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_23.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx23[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_23.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_24.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx24[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_24.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_25.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx25[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_25.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_26.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx26[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_26.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_27.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx27[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_27.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_28.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx28[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_28.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_29.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx29[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_29.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_30.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx30[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_30.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_31.read();
   sdata.str("");
   sdata << "QDEPTH_ETH";
   cntr_map_ctx31[sdata.str()] = pbc_csr.hbm.sta_hbm_eth_context_31.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_0.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx0[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_0.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_1.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx1[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_1.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_2.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx2[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_2.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_3.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx3[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_3.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_4.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx4[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_4.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_5.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx5[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_5.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_6.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx6[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_6.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_7.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx7[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_7.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_8.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx8[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_8.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_9.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx9[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_9.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_10.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx10[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_10.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_11.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx11[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_11.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_12.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx12[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_12.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_13.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx13[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_13.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_14.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx14[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_14.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_15.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx15[sdata.str()] = pbc_csr.hbm.sta_hbm_tx_context_15.depth().convert_to<uint64_t>();
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_16.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx16[sdata.str()] = 0;
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_17.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx17[sdata.str()] = 0;
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_18.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx18[sdata.str()] = 0;
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_19.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx19[sdata.str()] = 0;
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_20.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx20[sdata.str()] = 0;
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_21.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx21[sdata.str()] = 0;
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_22.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx22[sdata.str()] = 0;
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_23.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx23[sdata.str()] = 0;
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_24.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx24[sdata.str()] = 0;
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_25.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx25[sdata.str()] = 0;
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_26.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx26[sdata.str()] = 0;
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_27.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx27[sdata.str()] = 0;
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_28.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx28[sdata.str()] = 0;
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_29.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx29[sdata.str()] = 0;
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_30.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx30[sdata.str()] = 0;
   sdata.str("");
   pbc_csr.hbm.sta_hbm_eth_context_31.read();
   sdata.str("");
   sdata << "QDEPTH_TDMA";
   cntr_map_ctx31[sdata.str()] = 0;
   sdata.str("");
   PB_PRN_MSG(setw(125) << "===================================================== PB HBM Q_DEPTH ==========================================================\n");

   PB_PRN_MSG(setw(36) << "ctxt." << 0 << setw(11) << "ctxt." << 1 << setw(11) << "ctxt." << 2 << setw(11) << "ctxt." << 3 << setw(11) << "ctxt." << 4 << setw(11) << "ctxt." << 5 << setw(11) << "ctxt." << 6 << setw(11) << "ctxt." << 7 << "\n\n");
   for(auto i : cntr_map_ctx0) {
     string counter_name = i.first;
     counter_name.append(25-counter_name.length(),' ');
     stringstream sdata;
     sdata.str("");
     stringstream rval0;
     if (i.second == 0) {
       rval0 << "-";
     } else {
       if (is_hex) {
          rval0 << "0x" << hex << i.second;
       } else {
          rval0 << dec << i.second;
       }
     }
     stringstream rval1;
     rval1.str("-");
     if (cntr_map_ctx1[i.first] != 0) {
       if (is_hex) {
          rval1 << "0x" << hex << cntr_map_ctx1[i.first];
       } else {
          rval1 << dec << cntr_map_ctx1[i.first];
       }
     }
     stringstream rval2;
     rval2.str("-");
     if (cntr_map_ctx2[i.first] != 0) {
       if (is_hex) {
          rval2 << "0x" << hex << cntr_map_ctx2[i.first];
       } else {
          rval2 << dec << cntr_map_ctx2[i.first];
       }
     }
     stringstream rval3;
     rval3.str("-");
     if (cntr_map_ctx3[i.first] != 0) {
       if (is_hex) {
          rval3 << "0x" << hex << cntr_map_ctx3[i.first];
       } else {
          rval3 << dec << cntr_map_ctx3[i.first];
       }
     }
     stringstream rval4;
     rval4.str("-");
     if (cntr_map_ctx4[i.first] != 0) {
       if (is_hex) {
          rval4 << "0x" << hex << cntr_map_ctx4[i.first];
       } else {
          rval4 << dec << cntr_map_ctx4[i.first];
       }
     }
     stringstream rval5;
     rval5.str("-");
     if (cntr_map_ctx5[i.first] != 0) {
       if (is_hex) {
          rval5 << "0x" << hex << cntr_map_ctx5[i.first];
       } else {
          rval5 << dec << cntr_map_ctx5[i.first];
       }
     }
     stringstream rval6;
     rval6.str("-");
     if (cntr_map_ctx6[i.first] != 0) {
       if (is_hex) {
          rval6 << "0x" << hex << cntr_map_ctx6[i.first];
       } else {
          rval6 << dec << cntr_map_ctx6[i.first];
       }
     }
     stringstream rval7;
     rval7.str("-");
     if (cntr_map_ctx7[i.first] != 0) {
       if (is_hex) {
          rval7 << "0x" << hex << cntr_map_ctx7[i.first];
       } else {
          rval7 << dec << cntr_map_ctx7[i.first];
       }
     }
     sdata << setw(12) << rval0.str() << setw(12) << rval1.str() << setw(12) << rval2.str() << setw(12) << rval3.str() << setw(12) << rval4.str() << setw(12) << rval5.str() << setw(12) << rval6.str() << setw(12) << rval7.str();
     PB_PRN_MSG(setw(25) << counter_name << sdata.str() << "\n");
   }
   PB_PRN_MSG(setw(36) << "ctxt." << 8 << setw(11) << "ctxt." << 9 << setw(11) << "ctxt." << 10 << setw(10) << "ctxt." << 11 << setw(10) << "ctxt." << 12 << setw(10) << "ctxt." << 13 << setw(10) << "ctxt." << 14 << setw(10) << "ctxt." << 15 << "\n\n");
   for(auto i : cntr_map_ctx8) {
     string counter_name = i.first;
     counter_name.append(25-counter_name.length(),' ');
     stringstream sdata;
     sdata.str("");
     stringstream rval0;
     if (i.second == 0) {
       rval0 << "-";
     } else {
       if (is_hex) {
          rval0 << "0x" << hex << i.second;
       } else {
          rval0 << dec << i.second;
       }
     }
     stringstream rval1;
     rval1.str("-");
     if (cntr_map_ctx9[i.first] != 0) {
       if (is_hex) {
          rval1 << "0x" << hex << cntr_map_ctx9[i.first];
       } else {
          rval1 << dec << cntr_map_ctx9[i.first];
       }
     }
     stringstream rval2;
     rval2.str("-");
     if (cntr_map_ctx10[i.first] != 0) {
       if (is_hex) {
          rval2 << "0x" << hex << cntr_map_ctx10[i.first];
       } else {
          rval2 << dec << cntr_map_ctx10[i.first];
       }
     }
     stringstream rval3;
     rval3.str("-");
     if (cntr_map_ctx11[i.first] != 0) {
       if (is_hex) {
          rval3 << "0x" << hex << cntr_map_ctx11[i.first];
       } else {
          rval3 << dec << cntr_map_ctx11[i.first];
       }
     }
     stringstream rval4;
     rval4.str("-");
     if (cntr_map_ctx12[i.first] != 0) {
       if (is_hex) {
          rval4 << "0x" << hex << cntr_map_ctx12[i.first];
       } else {
          rval4 << dec << cntr_map_ctx12[i.first];
       }
     }
     stringstream rval5;
     rval5.str("-");
     if (cntr_map_ctx13[i.first] != 0) {
       if (is_hex) {
          rval5 << "0x" << hex << cntr_map_ctx13[i.first];
       } else {
          rval5 << dec << cntr_map_ctx13[i.first];
       }
     }
     stringstream rval6;
     rval6.str("-");
     if (cntr_map_ctx14[i.first] != 0) {
       if (is_hex) {
          rval6 << "0x" << hex << cntr_map_ctx14[i.first];
       } else {
          rval6 << dec << cntr_map_ctx14[i.first];
       }
     }
     stringstream rval7;
     rval7.str("-");
     if (cntr_map_ctx15[i.first] != 0) {
       if (is_hex) {
          rval7 << "0x" << hex << cntr_map_ctx15[i.first];
       } else {
          rval7 << dec << cntr_map_ctx15[i.first];
       }
     }
     sdata << setw(12) << rval0.str() << setw(12) << rval1.str() << setw(12) << rval2.str() << setw(12) << rval3.str() << setw(12) << rval4.str() << setw(12) << rval5.str() << setw(12) << rval6.str() << setw(12) << rval7.str();
     PB_PRN_MSG(setw(25) << counter_name << sdata.str() << "\n");
   }
   PB_PRN_MSG(setw(36) << "ctxt." << 16 << setw(10) << "ctxt." << 17 << setw(10) << "ctxt." << 18 << setw(10) << "ctxt." << 19 << setw(10) << "ctxt." << 20 << setw(10) << "ctxt." << 21 << setw(10) << "ctxt." << 22 << setw(10) << "ctxt." << 23 << "\n\n");
   for(auto i : cntr_map_ctx16) {
     string counter_name = i.first;
     counter_name.append(25-counter_name.length(),' ');
     stringstream sdata;
     sdata.str("");
     stringstream rval0;
     if (i.second == 0) {
       rval0 << "-";
     } else {
       if (is_hex) {
          rval0 << "0x" << hex << i.second;
       } else {
          rval0 << dec << i.second;
       }
     }
     stringstream rval1;
     rval1.str("-");
     if (cntr_map_ctx17[i.first] != 0) {
       if (is_hex) {
          rval1 << "0x" << hex << cntr_map_ctx17[i.first];
       } else {
          rval1 << dec << cntr_map_ctx17[i.first];
       }
     }
     stringstream rval2;
     rval2.str("-");
     if (cntr_map_ctx18[i.first] != 0) {
       if (is_hex) {
          rval2 << "0x" << hex << cntr_map_ctx18[i.first];
       } else {
          rval2 << dec << cntr_map_ctx18[i.first];
       }
     }
     stringstream rval3;
     rval3.str("-");
     if (cntr_map_ctx19[i.first] != 0) {
       if (is_hex) {
          rval3 << "0x" << hex << cntr_map_ctx19[i.first];
       } else {
          rval3 << dec << cntr_map_ctx19[i.first];
       }
     }
     stringstream rval4;
     rval4.str("-");
     if (cntr_map_ctx20[i.first] != 0) {
       if (is_hex) {
          rval4 << "0x" << hex << cntr_map_ctx20[i.first];
       } else {
          rval4 << dec << cntr_map_ctx20[i.first];
       }
     }
     stringstream rval5;
     rval5.str("-");
     if (cntr_map_ctx21[i.first] != 0) {
       if (is_hex) {
          rval5 << "0x" << hex << cntr_map_ctx21[i.first];
       } else {
          rval5 << dec << cntr_map_ctx21[i.first];
       }
     }
     stringstream rval6;
     rval6.str("-");
     if (cntr_map_ctx22[i.first] != 0) {
       if (is_hex) {
          rval6 << "0x" << hex << cntr_map_ctx22[i.first];
       } else {
          rval6 << dec << cntr_map_ctx22[i.first];
       }
     }
     stringstream rval7;
     rval7.str("-");
     if (cntr_map_ctx23[i.first] != 0) {
       if (is_hex) {
          rval7 << "0x" << hex << cntr_map_ctx23[i.first];
       } else {
          rval7 << dec << cntr_map_ctx23[i.first];
       }
     }
     sdata << setw(12) << rval0.str() << setw(12) << rval1.str() << setw(12) << rval2.str() << setw(12) << rval3.str() << setw(12) << rval4.str() << setw(12) << rval5.str() << setw(12) << rval6.str() << setw(12) << rval7.str();
     PB_PRN_MSG(setw(25) << counter_name << sdata.str() << "\n");
   }
   PB_PRN_MSG(setw(36) << "ctxt." << 24 << setw(10) << "ctxt." << 25 << setw(10) << "ctxt." << 26 << setw(10) << "ctxt." << 27 << setw(10) << "ctxt." << 28 << setw(10) << "ctxt." << 29 << setw(10) << "ctxt." << 30 << setw(10) << "ctxt." << 31 << "\n\n");
   for(auto i : cntr_map_ctx24) {
     string counter_name = i.first;
     counter_name.append(25-counter_name.length(),' ');
     stringstream sdata;
     sdata.str("");
     stringstream rval0;
     if (i.second == 0) {
       rval0 << "-";
     } else {
       if (is_hex) {
          rval0 << "0x" << hex << i.second;
       } else {
          rval0 << dec << i.second;
       }
     }
     stringstream rval1;
     rval1.str("-");
     if (cntr_map_ctx25[i.first] != 0) {
       if (is_hex) {
          rval1 << "0x" << hex << cntr_map_ctx25[i.first];
       } else {
          rval1 << dec << cntr_map_ctx25[i.first];
       }
     }
     stringstream rval2;
     rval2.str("-");
     if (cntr_map_ctx26[i.first] != 0) {
       if (is_hex) {
          rval2 << "0x" << hex << cntr_map_ctx26[i.first];
       } else {
          rval2 << dec << cntr_map_ctx26[i.first];
       }
     }
     stringstream rval3;
     rval3.str("-");
     if (cntr_map_ctx27[i.first] != 0) {
       if (is_hex) {
          rval3 << "0x" << hex << cntr_map_ctx27[i.first];
       } else {
          rval3 << dec << cntr_map_ctx27[i.first];
       }
     }
     stringstream rval4;
     rval4.str("-");
     if (cntr_map_ctx28[i.first] != 0) {
       if (is_hex) {
          rval4 << "0x" << hex << cntr_map_ctx28[i.first];
       } else {
          rval4 << dec << cntr_map_ctx28[i.first];
       }
     }
     stringstream rval5;
     rval5.str("-");
     if (cntr_map_ctx29[i.first] != 0) {
       if (is_hex) {
          rval5 << "0x" << hex << cntr_map_ctx29[i.first];
       } else {
          rval5 << dec << cntr_map_ctx29[i.first];
       }
     }
     stringstream rval6;
     rval6.str("-");
     if (cntr_map_ctx30[i.first] != 0) {
       if (is_hex) {
          rval6 << "0x" << hex << cntr_map_ctx30[i.first];
       } else {
          rval6 << dec << cntr_map_ctx30[i.first];
       }
     }
     stringstream rval7;
     rval7.str("-");
     if (cntr_map_ctx31[i.first] != 0) {
       if (is_hex) {
          rval7 << "0x" << hex << cntr_map_ctx31[i.first];
       } else {
          rval7 << dec << cntr_map_ctx31[i.first];
       }
     }
     sdata << setw(12) << rval0.str() << setw(12) << rval1.str() << setw(12) << rval2.str() << setw(12) << rval3.str() << setw(12) << rval4.str() << setw(12) << rval5.str() << setw(12) << rval6.str() << setw(12) << rval7.str();
     PB_PRN_MSG(setw(25) << counter_name << sdata.str() << "\n");
   }
 PB_PRN_MSG(setw(125) << "================================================================================================================================\n");
}

void cap_pb_rd_flush_cnt(int chip_id, int inst_id, int port, cpp_int & rval) {
 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);

 pbc_csr.cnt_flush[port].read();
 rval = pbc_csr.cnt_flush[port].packets();
}

void cap_pb_rand_qisable(int chip_id, int inst_id, int loop_cnt) {
#ifdef MODULE_SIM
 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, inst_id);
    RRKnob   rand_knob("rand_knob", 0, 0xffffffff);
 for (int ii=0; ii<loop_cnt; ii++) {
    PLOG_MSG ("cap_pb_rand_qisable : loop count "<<ii << std::endl);
    pbc_csr.port_0.cfg_oq_queue.enable(rand_knob.eval());
    pbc_csr.port_0.cfg_oq_queue.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_0.cfg_oq_queue.show();}
    pbc_csr.port_1.cfg_oq_queue.enable(rand_knob.eval());
    pbc_csr.port_1.cfg_oq_queue.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_1.cfg_oq_queue.show();}
    pbc_csr.port_2.cfg_oq_queue.enable(rand_knob.eval());
    pbc_csr.port_2.cfg_oq_queue.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_2.cfg_oq_queue.show();}
    pbc_csr.port_3.cfg_oq_queue.enable(rand_knob.eval());
    pbc_csr.port_3.cfg_oq_queue.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_3.cfg_oq_queue.show();}
    pbc_csr.port_4.cfg_oq_queue.enable(rand_knob.eval());
    pbc_csr.port_4.cfg_oq_queue.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_4.cfg_oq_queue.show();}
    pbc_csr.port_5.cfg_oq_queue.enable(rand_knob.eval());
    pbc_csr.port_5.cfg_oq_queue.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_5.cfg_oq_queue.show();}
    pbc_csr.port_6.cfg_oq_queue.enable(rand_knob.eval());
    pbc_csr.port_6.cfg_oq_queue.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_6.cfg_oq_queue.show();}
    pbc_csr.port_7.cfg_oq_queue.enable(rand_knob.eval());
    pbc_csr.port_7.cfg_oq_queue.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_7.cfg_oq_queue.show();}
    pbc_csr.port_8.cfg_oq_queue.enable(rand_knob.eval());
    pbc_csr.port_8.cfg_oq_queue.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_8.cfg_oq_queue.show();}
    pbc_csr.port_9.cfg_oq_queue.enable(rand_knob.eval());
    pbc_csr.port_9.cfg_oq_queue.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_oq_queue.show();}
    pbc_csr.port_10.cfg_oq_queue.enable(rand_knob.eval());
    pbc_csr.port_10.cfg_oq_queue.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_oq_queue.show();}
    pbc_csr.port_11.cfg_oq_queue.enable(rand_knob.eval());
    pbc_csr.port_11.cfg_oq_queue.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_oq_queue.show();}
   SLEEP(500);
 }
    PLOG_MSG ("cap_pb_rand_qisable : end of qdisable loop. restore original values " << std::endl);
    pbc_csr.port_0.cfg_oq_queue.enable(0xffffffff);
    pbc_csr.port_0.cfg_oq_queue.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_0.cfg_oq_queue.show();}
    pbc_csr.port_1.cfg_oq_queue.enable(0xffffffff);
    pbc_csr.port_1.cfg_oq_queue.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_1.cfg_oq_queue.show();}
    pbc_csr.port_2.cfg_oq_queue.enable(0xffffffff);
    pbc_csr.port_2.cfg_oq_queue.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_2.cfg_oq_queue.show();}
    pbc_csr.port_3.cfg_oq_queue.enable(0xffffffff);
    pbc_csr.port_3.cfg_oq_queue.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_3.cfg_oq_queue.show();}
    pbc_csr.port_4.cfg_oq_queue.enable(0xffffffff);
    pbc_csr.port_4.cfg_oq_queue.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_4.cfg_oq_queue.show();}
    pbc_csr.port_5.cfg_oq_queue.enable(0xffffffff);
    pbc_csr.port_5.cfg_oq_queue.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_5.cfg_oq_queue.show();}
    pbc_csr.port_6.cfg_oq_queue.enable(0xffffffff);
    pbc_csr.port_6.cfg_oq_queue.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_6.cfg_oq_queue.show();}
    pbc_csr.port_7.cfg_oq_queue.enable(0xffffffff);
    pbc_csr.port_7.cfg_oq_queue.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_7.cfg_oq_queue.show();}
    pbc_csr.port_8.cfg_oq_queue.enable(0xffffffff);
    pbc_csr.port_8.cfg_oq_queue.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_8.cfg_oq_queue.show();}
    pbc_csr.port_9.cfg_oq_queue.enable(0xffffffff);
    pbc_csr.port_9.cfg_oq_queue.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_9.cfg_oq_queue.show();}
    pbc_csr.port_10.cfg_oq_queue.enable(0xffffffff);
    pbc_csr.port_10.cfg_oq_queue.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_10.cfg_oq_queue.show();}
    pbc_csr.port_11.cfg_oq_queue.enable(0xffffffff);
    pbc_csr.port_11.cfg_oq_queue.write();
    if ( PLOG_CHECK_MSG_LEVEL("pb_api") ) {pbc_csr.port_11.cfg_oq_queue.show();}
#endif
}

void cap_pb_dump_iq_stats(int chip_id, int inst_id, int is_hex) {

 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, 0);

 map <string, uint64_t> cntr_map_iq0;
 map <string, uint64_t> cntr_map_iq1;
 map <string, uint64_t> cntr_map_iq2;
 map <string, uint64_t> cntr_map_iq3;
 map <string, uint64_t> cntr_map_iq4;
 map <string, uint64_t> cntr_map_iq5;
 map <string, uint64_t> cntr_map_iq6;
 map <string, uint64_t> cntr_map_iq7;
 map <string, uint64_t> cntr_map_iq8;
 map <string, uint64_t> cntr_map_iq9;
 map <string, uint64_t> cntr_map_iq10;
 map <string, uint64_t> cntr_map_iq11;
 map <string, uint64_t> cntr_map_iq12;
 map <string, uint64_t> cntr_map_iq13;
 map <string, uint64_t> cntr_map_iq14;
 map <string, uint64_t> cntr_map_iq15;
 map <string, uint64_t> cntr_map_iq16;
 map <string, uint64_t> cntr_map_iq17;
 map <string, uint64_t> cntr_map_iq18;
 map <string, uint64_t> cntr_map_iq19;
 map <string, uint64_t> cntr_map_iq20;
 map <string, uint64_t> cntr_map_iq21;
 map <string, uint64_t> cntr_map_iq22;
 map <string, uint64_t> cntr_map_iq23;
 map <string, uint64_t> cntr_map_iq24;
 map <string, uint64_t> cntr_map_iq25;
 map <string, uint64_t> cntr_map_iq26;
 map <string, uint64_t> cntr_map_iq27;
 map <string, uint64_t> cntr_map_iq28;
 map <string, uint64_t> cntr_map_iq29;
 map <string, uint64_t> cntr_map_iq30;
 map <string, uint64_t> cntr_map_iq31;
 // int p4ig_offset = 0;  
 // int p4eg_offset =   32;
 // int p4p_offset =   32 + 32;
 // int eth_offset =   32 + 32 + 16;

 pbc_csr.cfg_port_mon_in.read();  
 int start = pbc_csr.cfg_port_mon_in.eth().convert_to<uint32_t>();

 int cur_address=0;
 cpp_int count;
 stringstream sdata;
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq0[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq1[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq2[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq3[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq4[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq5[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq6[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq7[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq8[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq9[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq10[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq11[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq12[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq13[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq14[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq15[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq16[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq17[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq18[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq19[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq20[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq21[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq22[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq23[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq24[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq25[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq26[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq27[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq28[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq29[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq30[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 11;
      cntr_map_iq31[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq0[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq1[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq2[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq3[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq4[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq5[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq6[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq7[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq8[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq9[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq10[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq11[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq12[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq13[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq14[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq15[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq16[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq17[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq18[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq19[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq20[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq21[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq22[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq23[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq24[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq25[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq26[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq27[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq28[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq29[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq30[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 10;
      cntr_map_iq31[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 9;
      cntr_map_iq0[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 9;
      cntr_map_iq1[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 9;
      cntr_map_iq2[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 9;
      cntr_map_iq3[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 9;
      cntr_map_iq4[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 9;
      cntr_map_iq5[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 9;
      cntr_map_iq6[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 9;
      cntr_map_iq7[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 9;
      cntr_map_iq8[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 9;
      cntr_map_iq9[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 9;
      cntr_map_iq10[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 9;
      cntr_map_iq11[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 9;
      cntr_map_iq12[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 9;
      cntr_map_iq13[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 9;
      cntr_map_iq14[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_P4_";
      sdata << 9;
      cntr_map_iq15[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+0;
      cntr_map_iq0[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+0;
      cntr_map_iq1[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+0;
      cntr_map_iq2[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+0;
      cntr_map_iq3[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+0;
      cntr_map_iq4[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+0;
      cntr_map_iq5[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+0;
      cntr_map_iq6[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+0;
      cntr_map_iq7[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+1;
      cntr_map_iq0[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+1;
      cntr_map_iq1[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+1;
      cntr_map_iq2[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+1;
      cntr_map_iq3[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+1;
      cntr_map_iq4[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+1;
      cntr_map_iq5[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+1;
      cntr_map_iq6[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+1;
      cntr_map_iq7[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+2;
      cntr_map_iq0[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+2;
      cntr_map_iq1[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+2;
      cntr_map_iq2[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+2;
      cntr_map_iq3[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+2;
      cntr_map_iq4[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+2;
      cntr_map_iq5[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+2;
      cntr_map_iq6[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+2;
      cntr_map_iq7[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+3;
      cntr_map_iq0[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+3;
      cntr_map_iq1[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+3;
      cntr_map_iq2[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+3;
      cntr_map_iq3[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+3;
      cntr_map_iq4[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+3;
      cntr_map_iq5[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+3;
      cntr_map_iq6[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+3;
      cntr_map_iq7[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+4;
      cntr_map_iq0[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+4;
      cntr_map_iq1[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+4;
      cntr_map_iq2[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+4;
      cntr_map_iq3[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+4;
      cntr_map_iq4[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+4;
      cntr_map_iq5[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+4;
      cntr_map_iq6[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+4;
      cntr_map_iq7[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+5;
      cntr_map_iq0[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+5;
      cntr_map_iq1[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+5;
      cntr_map_iq2[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+5;
      cntr_map_iq3[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+5;
      cntr_map_iq4[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+5;
      cntr_map_iq5[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+5;
      cntr_map_iq6[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_in(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_ETH_";
      sdata << start+5;
      cntr_map_iq7[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
   PB_PRN_MSG(setw(125) << "===================================================== PB PER IQ STATS ==========================================================\n");

   PB_PRN_MSG(setw(36) << "iqueue." << 0 << setw(11) << "iqueue." << 1 << setw(11) << "iqueue." << 2 << setw(11) << "iqueue." << 3 << setw(11) << "iqueue." << 4 << setw(11) << "iqueue." << 5 << setw(11) << "iqueue." << 6 << setw(11) << "iqueue." << 7 << "\n\n");
   for(auto i : cntr_map_iq0) {
     string counter_name = i.first;
     counter_name.append(25-counter_name.length(),' ');
     stringstream sdata;
     sdata.str("");
     stringstream rval0;
     if (i.second == 0) {
       rval0 << "-";
     } else {
       if (is_hex) {
          rval0 << "0x" << hex << i.second;
       } else {
          rval0 << dec << i.second;
       }
     }
     stringstream rval1;
     rval1.str("-");
     if (cntr_map_iq1[i.first] != 0) {
       if (is_hex) {
          rval1 << "0x" << hex << cntr_map_iq1[i.first];
       } else {
          rval1 << dec << cntr_map_iq1[i.first];
       }
     }
     stringstream rval2;
     rval2.str("-");
     if (cntr_map_iq2[i.first] != 0) {
       if (is_hex) {
          rval2 << "0x" << hex << cntr_map_iq2[i.first];
       } else {
          rval2 << dec << cntr_map_iq2[i.first];
       }
     }
     stringstream rval3;
     rval3.str("-");
     if (cntr_map_iq3[i.first] != 0) {
       if (is_hex) {
          rval3 << "0x" << hex << cntr_map_iq3[i.first];
       } else {
          rval3 << dec << cntr_map_iq3[i.first];
       }
     }
     stringstream rval4;
     rval4.str("-");
     if (cntr_map_iq4[i.first] != 0) {
       if (is_hex) {
          rval4 << "0x" << hex << cntr_map_iq4[i.first];
       } else {
          rval4 << dec << cntr_map_iq4[i.first];
       }
     }
     stringstream rval5;
     rval5.str("-");
     if (cntr_map_iq5[i.first] != 0) {
       if (is_hex) {
          rval5 << "0x" << hex << cntr_map_iq5[i.first];
       } else {
          rval5 << dec << cntr_map_iq5[i.first];
       }
     }
     stringstream rval6;
     rval6.str("-");
     if (cntr_map_iq6[i.first] != 0) {
       if (is_hex) {
          rval6 << "0x" << hex << cntr_map_iq6[i.first];
       } else {
          rval6 << dec << cntr_map_iq6[i.first];
       }
     }
     stringstream rval7;
     rval7.str("-");
     if (cntr_map_iq7[i.first] != 0) {
       if (is_hex) {
          rval7 << "0x" << hex << cntr_map_iq7[i.first];
       } else {
          rval7 << dec << cntr_map_iq7[i.first];
       }
     }
     sdata << setw(12) << rval0.str() << setw(12) << rval1.str() << setw(12) << rval2.str() << setw(12) << rval3.str() << setw(12) << rval4.str() << setw(12) << rval5.str() << setw(12) << rval6.str() << setw(12) << rval7.str();
     PB_PRN_MSG(setw(25) << counter_name << sdata.str() << "\n");
   }
     //PB_PRN_MSG(setw(25) << "\n");
   PB_PRN_MSG(setw(36) << "iqueue." << 8 << setw(11) << "iqueue." << 9 << setw(11) << "iqueue." << 10 << setw(10) << "iqueue." << 11 << setw(10) << "iqueue." << 12 << setw(10) << "iqueue." << 13 << setw(10) << "iqueue." << 14 << setw(10) << "iqueue." << 15 << "\n\n");
   for(auto i : cntr_map_iq8) {
     string counter_name = i.first;
     counter_name.append(25-counter_name.length(),' ');
     stringstream sdata;
     sdata.str("");
     stringstream rval0;
     if (i.second == 0) {
       rval0 << "-";
     } else {
       if (is_hex) {
          rval0 << "0x" << hex << i.second;
       } else {
          rval0 << dec << i.second;
       }
     }
     stringstream rval1;
     rval1.str("-");
     if (cntr_map_iq9[i.first] != 0) {
       if (is_hex) {
          rval1 << "0x" << hex << cntr_map_iq9[i.first];
       } else {
          rval1 << dec << cntr_map_iq9[i.first];
       }
     }
     stringstream rval2;
     rval2.str("-");
     if (cntr_map_iq10[i.first] != 0) {
       if (is_hex) {
          rval2 << "0x" << hex << cntr_map_iq10[i.first];
       } else {
          rval2 << dec << cntr_map_iq10[i.first];
       }
     }
     stringstream rval3;
     rval3.str("-");
     if (cntr_map_iq11[i.first] != 0) {
       if (is_hex) {
          rval3 << "0x" << hex << cntr_map_iq11[i.first];
       } else {
          rval3 << dec << cntr_map_iq11[i.first];
       }
     }
     stringstream rval4;
     rval4.str("-");
     if (cntr_map_iq12[i.first] != 0) {
       if (is_hex) {
          rval4 << "0x" << hex << cntr_map_iq12[i.first];
       } else {
          rval4 << dec << cntr_map_iq12[i.first];
       }
     }
     stringstream rval5;
     rval5.str("-");
     if (cntr_map_iq13[i.first] != 0) {
       if (is_hex) {
          rval5 << "0x" << hex << cntr_map_iq13[i.first];
       } else {
          rval5 << dec << cntr_map_iq13[i.first];
       }
     }
     stringstream rval6;
     rval6.str("-");
     if (cntr_map_iq14[i.first] != 0) {
       if (is_hex) {
          rval6 << "0x" << hex << cntr_map_iq14[i.first];
       } else {
          rval6 << dec << cntr_map_iq14[i.first];
       }
     }
     stringstream rval7;
     rval7.str("-");
     if (cntr_map_iq15[i.first] != 0) {
       if (is_hex) {
          rval7 << "0x" << hex << cntr_map_iq15[i.first];
       } else {
          rval7 << dec << cntr_map_iq15[i.first];
       }
     }
     sdata << setw(12) << rval0.str() << setw(12) << rval1.str() << setw(12) << rval2.str() << setw(12) << rval3.str() << setw(12) << rval4.str() << setw(12) << rval5.str() << setw(12) << rval6.str() << setw(12) << rval7.str();
     PB_PRN_MSG(setw(25) << counter_name << sdata.str() << "\n");
   }
     //PB_PRN_MSG(setw(25) << "\n");
   PB_PRN_MSG(setw(36) << "iqueue." << 16 << setw(10) << "iqueue." << 17 << setw(10) << "iqueue." << 18 << setw(10) << "iqueue." << 19 << setw(10) << "iqueue." << 20 << setw(10) << "iqueue." << 21 << setw(10) << "iqueue." << 22 << setw(10) << "iqueue." << 23 << "\n\n");
   for(auto i : cntr_map_iq16) {
     string counter_name = i.first;
     counter_name.append(25-counter_name.length(),' ');
     stringstream sdata;
     sdata.str("");
     stringstream rval0;
     if (i.second == 0) {
       rval0 << "-";
     } else {
       if (is_hex) {
          rval0 << "0x" << hex << i.second;
       } else {
          rval0 << dec << i.second;
       }
     }
     stringstream rval1;
     rval1.str("-");
     if (cntr_map_iq17[i.first] != 0) {
       if (is_hex) {
          rval1 << "0x" << hex << cntr_map_iq17[i.first];
       } else {
          rval1 << dec << cntr_map_iq17[i.first];
       }
     }
     stringstream rval2;
     rval2.str("-");
     if (cntr_map_iq18[i.first] != 0) {
       if (is_hex) {
          rval2 << "0x" << hex << cntr_map_iq18[i.first];
       } else {
          rval2 << dec << cntr_map_iq18[i.first];
       }
     }
     stringstream rval3;
     rval3.str("-");
     if (cntr_map_iq19[i.first] != 0) {
       if (is_hex) {
          rval3 << "0x" << hex << cntr_map_iq19[i.first];
       } else {
          rval3 << dec << cntr_map_iq19[i.first];
       }
     }
     stringstream rval4;
     rval4.str("-");
     if (cntr_map_iq20[i.first] != 0) {
       if (is_hex) {
          rval4 << "0x" << hex << cntr_map_iq20[i.first];
       } else {
          rval4 << dec << cntr_map_iq20[i.first];
       }
     }
     stringstream rval5;
     rval5.str("-");
     if (cntr_map_iq21[i.first] != 0) {
       if (is_hex) {
          rval5 << "0x" << hex << cntr_map_iq21[i.first];
       } else {
          rval5 << dec << cntr_map_iq21[i.first];
       }
     }
     stringstream rval6;
     rval6.str("-");
     if (cntr_map_iq22[i.first] != 0) {
       if (is_hex) {
          rval6 << "0x" << hex << cntr_map_iq22[i.first];
       } else {
          rval6 << dec << cntr_map_iq22[i.first];
       }
     }
     stringstream rval7;
     rval7.str("-");
     if (cntr_map_iq23[i.first] != 0) {
       if (is_hex) {
          rval7 << "0x" << hex << cntr_map_iq23[i.first];
       } else {
          rval7 << dec << cntr_map_iq23[i.first];
       }
     }
     sdata << setw(12) << rval0.str() << setw(12) << rval1.str() << setw(12) << rval2.str() << setw(12) << rval3.str() << setw(12) << rval4.str() << setw(12) << rval5.str() << setw(12) << rval6.str() << setw(12) << rval7.str();
     PB_PRN_MSG(setw(25) << counter_name << sdata.str() << "\n");
   }
     //PB_PRN_MSG(setw(25) << "\n");
   PB_PRN_MSG(setw(36) << "iqueue." << 24 << setw(10) << "iqueue." << 25 << setw(10) << "iqueue." << 26 << setw(10) << "iqueue." << 27 << setw(10) << "iqueue." << 28 << setw(10) << "iqueue." << 29 << setw(10) << "iqueue." << 30 << setw(10) << "iqueue." << 31 << "\n\n");
   for(auto i : cntr_map_iq24) {
     string counter_name = i.first;
     counter_name.append(25-counter_name.length(),' ');
     stringstream sdata;
     sdata.str("");
     stringstream rval0;
     if (i.second == 0) {
       rval0 << "-";
     } else {
       if (is_hex) {
          rval0 << "0x" << hex << i.second;
       } else {
          rval0 << dec << i.second;
       }
     }
     stringstream rval1;
     rval1.str("-");
     if (cntr_map_iq25[i.first] != 0) {
       if (is_hex) {
          rval1 << "0x" << hex << cntr_map_iq25[i.first];
       } else {
          rval1 << dec << cntr_map_iq25[i.first];
       }
     }
     stringstream rval2;
     rval2.str("-");
     if (cntr_map_iq26[i.first] != 0) {
       if (is_hex) {
          rval2 << "0x" << hex << cntr_map_iq26[i.first];
       } else {
          rval2 << dec << cntr_map_iq26[i.first];
       }
     }
     stringstream rval3;
     rval3.str("-");
     if (cntr_map_iq27[i.first] != 0) {
       if (is_hex) {
          rval3 << "0x" << hex << cntr_map_iq27[i.first];
       } else {
          rval3 << dec << cntr_map_iq27[i.first];
       }
     }
     stringstream rval4;
     rval4.str("-");
     if (cntr_map_iq28[i.first] != 0) {
       if (is_hex) {
          rval4 << "0x" << hex << cntr_map_iq28[i.first];
       } else {
          rval4 << dec << cntr_map_iq28[i.first];
       }
     }
     stringstream rval5;
     rval5.str("-");
     if (cntr_map_iq29[i.first] != 0) {
       if (is_hex) {
          rval5 << "0x" << hex << cntr_map_iq29[i.first];
       } else {
          rval5 << dec << cntr_map_iq29[i.first];
       }
     }
     stringstream rval6;
     rval6.str("-");
     if (cntr_map_iq30[i.first] != 0) {
       if (is_hex) {
          rval6 << "0x" << hex << cntr_map_iq30[i.first];
       } else {
          rval6 << dec << cntr_map_iq30[i.first];
       }
     }
     stringstream rval7;
     rval7.str("-");
     if (cntr_map_iq31[i.first] != 0) {
       if (is_hex) {
          rval7 << "0x" << hex << cntr_map_iq31[i.first];
       } else {
          rval7 << dec << cntr_map_iq31[i.first];
       }
     }
     sdata << setw(12) << rval0.str() << setw(12) << rval1.str() << setw(12) << rval2.str() << setw(12) << rval3.str() << setw(12) << rval4.str() << setw(12) << rval5.str() << setw(12) << rval6.str() << setw(12) << rval7.str();
     PB_PRN_MSG(setw(25) << counter_name << sdata.str() << "\n");
   }
     //PB_PRN_MSG(setw(25) << "\n");
 PB_PRN_MSG(setw(125) << "================================================================================================================================\n");
}

void cap_pb_dump_oq_stats(int chip_id, int inst_id, int is_hex) {

 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, 0);

 map <string, uint64_t> cntr_map_oq0;
 map <string, uint64_t> cntr_map_oq1;
 map <string, uint64_t> cntr_map_oq2;
 map <string, uint64_t> cntr_map_oq3;
 map <string, uint64_t> cntr_map_oq4;
 map <string, uint64_t> cntr_map_oq5;
 map <string, uint64_t> cntr_map_oq6;
 map <string, uint64_t> cntr_map_oq7;
 map <string, uint64_t> cntr_map_oq8;
 map <string, uint64_t> cntr_map_oq9;
 map <string, uint64_t> cntr_map_oq10;
 map <string, uint64_t> cntr_map_oq11;
 map <string, uint64_t> cntr_map_oq12;
 map <string, uint64_t> cntr_map_oq13;
 map <string, uint64_t> cntr_map_oq14;
 map <string, uint64_t> cntr_map_oq15;
 map <string, uint64_t> cntr_map_oq16;
 map <string, uint64_t> cntr_map_oq17;
 map <string, uint64_t> cntr_map_oq18;
 map <string, uint64_t> cntr_map_oq19;
 map <string, uint64_t> cntr_map_oq20;
 map <string, uint64_t> cntr_map_oq21;
 map <string, uint64_t> cntr_map_oq22;
 map <string, uint64_t> cntr_map_oq23;
 map <string, uint64_t> cntr_map_oq24;
 map <string, uint64_t> cntr_map_oq25;
 map <string, uint64_t> cntr_map_oq26;
 map <string, uint64_t> cntr_map_oq27;
 map <string, uint64_t> cntr_map_oq28;
 map <string, uint64_t> cntr_map_oq29;
 map <string, uint64_t> cntr_map_oq30;
 map <string, uint64_t> cntr_map_oq31;
 // int p4ig_offset = 0;  
 // int p4eg_offset =   32;
 // int p4p_offset =   32 + 32;
 // int eth_offset =   32 + 32 + 16;

 pbc_csr.cfg_port_mon_out.read();  

 int cur_address=0;
 cpp_int count;
 stringstream sdata;
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq0[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq1[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq2[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq3[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq4[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq5[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq6[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq7[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq8[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq9[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq10[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq11[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq12[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq13[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq14[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq15[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq16[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq17[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq18[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq19[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq20[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq21[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq22[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq23[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq24[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq25[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq26[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq27[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq28[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq29[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq30[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_0().convert_to<int>();
      cntr_map_oq31[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq0[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq1[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq2[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq3[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq4[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq5[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq6[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq7[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq8[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq9[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq10[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq11[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq12[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq13[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq14[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq15[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq16[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq17[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq18[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq19[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq20[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq21[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq22[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq23[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq24[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq25[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq26[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq27[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq28[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq29[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq30[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_1().convert_to<int>();
      cntr_map_oq31[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq0[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq1[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq2[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq3[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq4[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq5[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq6[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq7[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq8[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq9[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq10[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq11[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq12[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq13[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq14[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq15[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq16[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq17[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq18[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq19[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq20[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq21[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq22[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq23[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq24[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq25[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq26[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq27[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq28[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq29[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq30[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_2().convert_to<int>();
      cntr_map_oq31[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq0[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq1[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq2[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq3[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq4[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq5[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq6[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq7[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq8[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq9[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq10[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq11[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq12[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq13[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq14[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq15[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq16[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq17[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq18[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq19[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq20[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq21[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq22[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq23[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq24[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq25[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq26[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq27[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq28[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq29[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq30[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
      cap_pb_read_port_mon_out(chip_id,inst_id,cur_address,count);
      cur_address ++;
      sdata.str("");
      sdata << "port_";
      sdata << pbc_csr.cfg_port_mon_out.port_3().convert_to<int>();
      cntr_map_oq31[sdata.str()] = count.convert_to<uint64_t>();
      sdata.str("");
   PB_PRN_MSG(setw(125) << "===================================================== PB PER OQ STATS ==========================================================\n");

   PB_PRN_MSG(setw(36) << "oqueue." << 0 << setw(11) << "oqueue." << 1 << setw(11) << "oqueue." << 2 << setw(11) << "oqueue." << 3 << setw(11) << "oqueue." << 4 << setw(11) << "oqueue." << 5 << setw(11) << "oqueue." << 6 << setw(11) << "oqueue." << 7 << "\n\n");
   for(auto i : cntr_map_oq0) {
     string counter_name = i.first;
     counter_name.append(25-counter_name.length(),' ');
     stringstream sdata;
     sdata.str("");
     stringstream rval0;
     if (i.second == 0) {
       rval0 << "-";
     } else {
       if (is_hex) {
          rval0 << "0x" << hex << i.second;
       } else {
          rval0 << dec << i.second;
       }
     }
     stringstream rval1;
     rval1.str("-");
     if (cntr_map_oq1[i.first] != 0) {
       if (is_hex) {
          rval1 << "0x" << hex << cntr_map_oq1[i.first];
       } else {
          rval1 << dec << cntr_map_oq1[i.first];
       }
     }
     stringstream rval2;
     rval2.str("-");
     if (cntr_map_oq2[i.first] != 0) {
       if (is_hex) {
          rval2 << "0x" << hex << cntr_map_oq2[i.first];
       } else {
          rval2 << dec << cntr_map_oq2[i.first];
       }
     }
     stringstream rval3;
     rval3.str("-");
     if (cntr_map_oq3[i.first] != 0) {
       if (is_hex) {
          rval3 << "0x" << hex << cntr_map_oq3[i.first];
       } else {
          rval3 << dec << cntr_map_oq3[i.first];
       }
     }
     stringstream rval4;
     rval4.str("-");
     if (cntr_map_oq4[i.first] != 0) {
       if (is_hex) {
          rval4 << "0x" << hex << cntr_map_oq4[i.first];
       } else {
          rval4 << dec << cntr_map_oq4[i.first];
       }
     }
     stringstream rval5;
     rval5.str("-");
     if (cntr_map_oq5[i.first] != 0) {
       if (is_hex) {
          rval5 << "0x" << hex << cntr_map_oq5[i.first];
       } else {
          rval5 << dec << cntr_map_oq5[i.first];
       }
     }
     stringstream rval6;
     rval6.str("-");
     if (cntr_map_oq6[i.first] != 0) {
       if (is_hex) {
          rval6 << "0x" << hex << cntr_map_oq6[i.first];
       } else {
          rval6 << dec << cntr_map_oq6[i.first];
       }
     }
     stringstream rval7;
     rval7.str("-");
     if (cntr_map_oq7[i.first] != 0) {
       if (is_hex) {
          rval7 << "0x" << hex << cntr_map_oq7[i.first];
       } else {
          rval7 << dec << cntr_map_oq7[i.first];
       }
     }
     sdata << setw(12) << rval0.str() << setw(12) << rval1.str() << setw(12) << rval2.str() << setw(12) << rval3.str() << setw(12) << rval4.str() << setw(12) << rval5.str() << setw(12) << rval6.str() << setw(12) << rval7.str();
     PB_PRN_MSG(setw(25) << counter_name << sdata.str() << "\n");
   }
     //PB_PRN_MSG(setw(25) << "\n");
   PB_PRN_MSG(setw(36) << "oqueue." << 8 << setw(11) << "oqueue." << 9 << setw(11) << "oqueue." << 10 << setw(10) << "oqueue." << 11 << setw(10) << "oqueue." << 12 << setw(10) << "oqueue." << 13 << setw(10) << "oqueue." << 14 << setw(10) << "oqueue." << 15 << "\n\n");
   for(auto i : cntr_map_oq8) {
     string counter_name = i.first;
     counter_name.append(25-counter_name.length(),' ');
     stringstream sdata;
     sdata.str("");
     stringstream rval0;
     if (i.second == 0) {
       rval0 << "-";
     } else {
       if (is_hex) {
          rval0 << "0x" << hex << i.second;
       } else {
          rval0 << dec << i.second;
       }
     }
     stringstream rval1;
     rval1.str("-");
     if (cntr_map_oq9[i.first] != 0) {
       if (is_hex) {
          rval1 << "0x" << hex << cntr_map_oq9[i.first];
       } else {
          rval1 << dec << cntr_map_oq9[i.first];
       }
     }
     stringstream rval2;
     rval2.str("-");
     if (cntr_map_oq10[i.first] != 0) {
       if (is_hex) {
          rval2 << "0x" << hex << cntr_map_oq10[i.first];
       } else {
          rval2 << dec << cntr_map_oq10[i.first];
       }
     }
     stringstream rval3;
     rval3.str("-");
     if (cntr_map_oq11[i.first] != 0) {
       if (is_hex) {
          rval3 << "0x" << hex << cntr_map_oq11[i.first];
       } else {
          rval3 << dec << cntr_map_oq11[i.first];
       }
     }
     stringstream rval4;
     rval4.str("-");
     if (cntr_map_oq12[i.first] != 0) {
       if (is_hex) {
          rval4 << "0x" << hex << cntr_map_oq12[i.first];
       } else {
          rval4 << dec << cntr_map_oq12[i.first];
       }
     }
     stringstream rval5;
     rval5.str("-");
     if (cntr_map_oq13[i.first] != 0) {
       if (is_hex) {
          rval5 << "0x" << hex << cntr_map_oq13[i.first];
       } else {
          rval5 << dec << cntr_map_oq13[i.first];
       }
     }
     stringstream rval6;
     rval6.str("-");
     if (cntr_map_oq14[i.first] != 0) {
       if (is_hex) {
          rval6 << "0x" << hex << cntr_map_oq14[i.first];
       } else {
          rval6 << dec << cntr_map_oq14[i.first];
       }
     }
     stringstream rval7;
     rval7.str("-");
     if (cntr_map_oq15[i.first] != 0) {
       if (is_hex) {
          rval7 << "0x" << hex << cntr_map_oq15[i.first];
       } else {
          rval7 << dec << cntr_map_oq15[i.first];
       }
     }
     sdata << setw(12) << rval0.str() << setw(12) << rval1.str() << setw(12) << rval2.str() << setw(12) << rval3.str() << setw(12) << rval4.str() << setw(12) << rval5.str() << setw(12) << rval6.str() << setw(12) << rval7.str();
     PB_PRN_MSG(setw(25) << counter_name << sdata.str() << "\n");
   }
     //PB_PRN_MSG(setw(25) << "\n");
   PB_PRN_MSG(setw(36) << "oqueue." << 16 << setw(10) << "oqueue." << 17 << setw(10) << "oqueue." << 18 << setw(10) << "oqueue." << 19 << setw(10) << "oqueue." << 20 << setw(10) << "oqueue." << 21 << setw(10) << "oqueue." << 22 << setw(10) << "oqueue." << 23 << "\n\n");
   for(auto i : cntr_map_oq16) {
     string counter_name = i.first;
     counter_name.append(25-counter_name.length(),' ');
     stringstream sdata;
     sdata.str("");
     stringstream rval0;
     if (i.second == 0) {
       rval0 << "-";
     } else {
       if (is_hex) {
          rval0 << "0x" << hex << i.second;
       } else {
          rval0 << dec << i.second;
       }
     }
     stringstream rval1;
     rval1.str("-");
     if (cntr_map_oq17[i.first] != 0) {
       if (is_hex) {
          rval1 << "0x" << hex << cntr_map_oq17[i.first];
       } else {
          rval1 << dec << cntr_map_oq17[i.first];
       }
     }
     stringstream rval2;
     rval2.str("-");
     if (cntr_map_oq18[i.first] != 0) {
       if (is_hex) {
          rval2 << "0x" << hex << cntr_map_oq18[i.first];
       } else {
          rval2 << dec << cntr_map_oq18[i.first];
       }
     }
     stringstream rval3;
     rval3.str("-");
     if (cntr_map_oq19[i.first] != 0) {
       if (is_hex) {
          rval3 << "0x" << hex << cntr_map_oq19[i.first];
       } else {
          rval3 << dec << cntr_map_oq19[i.first];
       }
     }
     stringstream rval4;
     rval4.str("-");
     if (cntr_map_oq20[i.first] != 0) {
       if (is_hex) {
          rval4 << "0x" << hex << cntr_map_oq20[i.first];
       } else {
          rval4 << dec << cntr_map_oq20[i.first];
       }
     }
     stringstream rval5;
     rval5.str("-");
     if (cntr_map_oq21[i.first] != 0) {
       if (is_hex) {
          rval5 << "0x" << hex << cntr_map_oq21[i.first];
       } else {
          rval5 << dec << cntr_map_oq21[i.first];
       }
     }
     stringstream rval6;
     rval6.str("-");
     if (cntr_map_oq22[i.first] != 0) {
       if (is_hex) {
          rval6 << "0x" << hex << cntr_map_oq22[i.first];
       } else {
          rval6 << dec << cntr_map_oq22[i.first];
       }
     }
     stringstream rval7;
     rval7.str("-");
     if (cntr_map_oq23[i.first] != 0) {
       if (is_hex) {
          rval7 << "0x" << hex << cntr_map_oq23[i.first];
       } else {
          rval7 << dec << cntr_map_oq23[i.first];
       }
     }
     sdata << setw(12) << rval0.str() << setw(12) << rval1.str() << setw(12) << rval2.str() << setw(12) << rval3.str() << setw(12) << rval4.str() << setw(12) << rval5.str() << setw(12) << rval6.str() << setw(12) << rval7.str();
     PB_PRN_MSG(setw(25) << counter_name << sdata.str() << "\n");
   }
     //PB_PRN_MSG(setw(25) << "\n");
   PB_PRN_MSG(setw(36) << "oqueue." << 24 << setw(10) << "oqueue." << 25 << setw(10) << "oqueue." << 26 << setw(10) << "oqueue." << 27 << setw(10) << "oqueue." << 28 << setw(10) << "oqueue." << 29 << setw(10) << "oqueue." << 30 << setw(10) << "oqueue." << 31 << "\n\n");
   for(auto i : cntr_map_oq24) {
     string counter_name = i.first;
     counter_name.append(25-counter_name.length(),' ');
     stringstream sdata;
     sdata.str("");
     stringstream rval0;
     if (i.second == 0) {
       rval0 << "-";
     } else {
       if (is_hex) {
          rval0 << "0x" << hex << i.second;
       } else {
          rval0 << dec << i.second;
       }
     }
     stringstream rval1;
     rval1.str("-");
     if (cntr_map_oq25[i.first] != 0) {
       if (is_hex) {
          rval1 << "0x" << hex << cntr_map_oq25[i.first];
       } else {
          rval1 << dec << cntr_map_oq25[i.first];
       }
     }
     stringstream rval2;
     rval2.str("-");
     if (cntr_map_oq26[i.first] != 0) {
       if (is_hex) {
          rval2 << "0x" << hex << cntr_map_oq26[i.first];
       } else {
          rval2 << dec << cntr_map_oq26[i.first];
       }
     }
     stringstream rval3;
     rval3.str("-");
     if (cntr_map_oq27[i.first] != 0) {
       if (is_hex) {
          rval3 << "0x" << hex << cntr_map_oq27[i.first];
       } else {
          rval3 << dec << cntr_map_oq27[i.first];
       }
     }
     stringstream rval4;
     rval4.str("-");
     if (cntr_map_oq28[i.first] != 0) {
       if (is_hex) {
          rval4 << "0x" << hex << cntr_map_oq28[i.first];
       } else {
          rval4 << dec << cntr_map_oq28[i.first];
       }
     }
     stringstream rval5;
     rval5.str("-");
     if (cntr_map_oq29[i.first] != 0) {
       if (is_hex) {
          rval5 << "0x" << hex << cntr_map_oq29[i.first];
       } else {
          rval5 << dec << cntr_map_oq29[i.first];
       }
     }
     stringstream rval6;
     rval6.str("-");
     if (cntr_map_oq30[i.first] != 0) {
       if (is_hex) {
          rval6 << "0x" << hex << cntr_map_oq30[i.first];
       } else {
          rval6 << dec << cntr_map_oq30[i.first];
       }
     }
     stringstream rval7;
     rval7.str("-");
     if (cntr_map_oq31[i.first] != 0) {
       if (is_hex) {
          rval7 << "0x" << hex << cntr_map_oq31[i.first];
       } else {
          rval7 << dec << cntr_map_oq31[i.first];
       }
     }
     sdata << setw(12) << rval0.str() << setw(12) << rval1.str() << setw(12) << rval2.str() << setw(12) << rval3.str() << setw(12) << rval4.str() << setw(12) << rval5.str() << setw(12) << rval6.str() << setw(12) << rval7.str();
     PB_PRN_MSG(setw(25) << counter_name << sdata.str() << "\n");
   }
     //PB_PRN_MSG(setw(25) << "\n");
 PB_PRN_MSG(setw(125) << "================================================================================================================================\n");
}


uint64_t cap_pb_dump_drop_and_err_cntrs(int chip_id, int inst_id, int no_print, int is_hex) {

 cap_pbc_csr_t & pbc_csr = CAP_BLK_REG_MODEL_ACCESS(cap_pbc_csr_t, chip_id, 0);

 uint64_t tot_drop_cnt = 0;

 map <string, uint64_t> cntr_map0;
 map <string, uint64_t> cntr_map1;
 map <string, uint64_t> cntr_map2;
 map <string, uint64_t> cntr_map3;
 map <string, uint64_t> cntr_map4;
 map <string, uint64_t> cntr_map5;
 map <string, uint64_t> cntr_map6;
 map <string, uint64_t> cntr_map7;
 map <string, uint64_t> cntr_map8;
 map <string, uint64_t> cntr_map9;
 map <string, uint64_t> cntr_map10;
 map <string, uint64_t> cntr_map11;

 map <string, uint64_t> cntr_map_hbm0;
 map <string, uint64_t> cntr_map_hbm1;
 map <string, uint64_t> cntr_map_hbm2;
 map <string, uint64_t> cntr_map_hbm3;
 map <string, uint64_t> cntr_map_hbm4;
 map <string, uint64_t> cntr_map_hbm5;
 map <string, uint64_t> cntr_map_hbm6;
 map <string, uint64_t> cntr_map_hbm7;
 map <string, uint64_t> cntr_map_hbm8;
 map <string, uint64_t> cntr_map_hbm9;

 cpu_access_type_e cur_cpu_access_type = cpu::access()->get_access_type();
 cpu::access()->set_access_type(front_door_e);

 map <string, uint64_t> cntr_map_ctx0;
 map <string, uint64_t> cntr_map_ctx1;
 map <string, uint64_t> cntr_map_ctx2;
 map <string, uint64_t> cntr_map_ctx3;
 map <string, uint64_t> cntr_map_ctx4;
 map <string, uint64_t> cntr_map_ctx5;
 map <string, uint64_t> cntr_map_ctx6;
 map <string, uint64_t> cntr_map_ctx7;
 map <string, uint64_t> cntr_map_ctx8;
 map <string, uint64_t> cntr_map_ctx9;
 map <string, uint64_t> cntr_map_ctx10;
 map <string, uint64_t> cntr_map_ctx11;
 map <string, uint64_t> cntr_map_ctx12;
 map <string, uint64_t> cntr_map_ctx13;
 map <string, uint64_t> cntr_map_ctx14;
 map <string, uint64_t> cntr_map_ctx15;
 map <string, uint64_t> cntr_map_ctx16;
 map <string, uint64_t> cntr_map_ctx17;
 map <string, uint64_t> cntr_map_ctx18;
 map <string, uint64_t> cntr_map_ctx19;
 map <string, uint64_t> cntr_map_ctx20;
 map <string, uint64_t> cntr_map_ctx21;
 map <string, uint64_t> cntr_map_ctx22;
 map <string, uint64_t> cntr_map_ctx23;
 map <string, uint64_t> cntr_map_ctx24;
 map <string, uint64_t> cntr_map_ctx25;
 map <string, uint64_t> cntr_map_ctx26;
 map <string, uint64_t> cntr_map_ctx27;
 map <string, uint64_t> cntr_map_ctx28;
 map <string, uint64_t> cntr_map_ctx29;
 map <string, uint64_t> cntr_map_ctx30;
 map <string, uint64_t> cntr_map_ctx31;
 cpp_int rtmp;
 cpp_int good_count, error_count;
 stringstream sdata;

   cap_pb_get_write_error_cnt(chip_id, inst_id, 0, 0, rtmp);
   cntr_map0["WERR_DISCARDED"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map0["WERR_DISCARDED"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 0, 1, rtmp);
   cntr_map0["WERR_ADMITTED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 0, 2, rtmp);
   cntr_map0["WERR_INTRINSIC_DROP"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map0["WERR_INTRINSIC_DROP"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 0, 3, rtmp);
   cntr_map0["WERR_OUT_OF_CELLS"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 0, 4, rtmp);
   cntr_map0["WERR_OUT_OF_CREDIT"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map0["WERR_OUT_OF_CREDIT"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 0, 5, rtmp);
   cntr_map0["WERR_TRUNCATION"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 0, 6, rtmp);
   cntr_map0["WERR_PORT_DISABLED"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map0["WERR_PORT_DISABLED"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 0, 7, rtmp);
   cntr_map0["WERR_OUT_OF_CELLS1"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map0["WERR_OUT_OF_CELLS1"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 0, 8, rtmp);
   cntr_map0["WERR_TAIL_DROP_CPU"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 0, 9, rtmp);
   cntr_map0["WERR_TAIL_DROP_SPAN"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 0, 10, rtmp);
   cntr_map0["WERR_MIN_SIZE_VIOL"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map0["WERR_MIN_SIZE_VIOL"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 0, 11, rtmp);
   cntr_map0["WERR_ENQUEUE_DROP"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map0["WERR_ENQUEUE_DROP"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 0, 12, rtmp);
   cntr_map0["WERR_PORT_RANGE"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map0["WERR_PORT_RANGE"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 0, 13, rtmp);
   cntr_map0["WERR_OQ_RANGE"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map0["WERR_OQ_RANGE"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 1, 0, rtmp);
   cntr_map1["WERR_DISCARDED"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map1["WERR_DISCARDED"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 1, 1, rtmp);
   cntr_map1["WERR_ADMITTED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 1, 2, rtmp);
   cntr_map1["WERR_INTRINSIC_DROP"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map1["WERR_INTRINSIC_DROP"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 1, 3, rtmp);
   cntr_map1["WERR_OUT_OF_CELLS"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 1, 4, rtmp);
   cntr_map1["WERR_OUT_OF_CREDIT"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map1["WERR_OUT_OF_CREDIT"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 1, 5, rtmp);
   cntr_map1["WERR_TRUNCATION"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 1, 6, rtmp);
   cntr_map1["WERR_PORT_DISABLED"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map1["WERR_PORT_DISABLED"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 1, 7, rtmp);
   cntr_map1["WERR_OUT_OF_CELLS1"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map1["WERR_OUT_OF_CELLS1"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 1, 8, rtmp);
   cntr_map1["WERR_TAIL_DROP_CPU"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 1, 9, rtmp);
   cntr_map1["WERR_TAIL_DROP_SPAN"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 1, 10, rtmp);
   cntr_map1["WERR_MIN_SIZE_VIOL"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map1["WERR_MIN_SIZE_VIOL"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 1, 11, rtmp);
   cntr_map1["WERR_ENQUEUE_DROP"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map1["WERR_ENQUEUE_DROP"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 1, 12, rtmp);
   cntr_map1["WERR_PORT_RANGE"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map1["WERR_PORT_RANGE"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 1, 13, rtmp);
   cntr_map1["WERR_OQ_RANGE"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map1["WERR_OQ_RANGE"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 2, 0, rtmp);
   cntr_map2["WERR_DISCARDED"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map2["WERR_DISCARDED"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 2, 1, rtmp);
   cntr_map2["WERR_ADMITTED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 2, 2, rtmp);
   cntr_map2["WERR_INTRINSIC_DROP"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map2["WERR_INTRINSIC_DROP"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 2, 3, rtmp);
   cntr_map2["WERR_OUT_OF_CELLS"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 2, 4, rtmp);
   cntr_map2["WERR_OUT_OF_CREDIT"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map2["WERR_OUT_OF_CREDIT"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 2, 5, rtmp);
   cntr_map2["WERR_TRUNCATION"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 2, 6, rtmp);
   cntr_map2["WERR_PORT_DISABLED"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map2["WERR_PORT_DISABLED"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 2, 7, rtmp);
   cntr_map2["WERR_OUT_OF_CELLS1"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map2["WERR_OUT_OF_CELLS1"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 2, 8, rtmp);
   cntr_map2["WERR_TAIL_DROP_CPU"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 2, 9, rtmp);
   cntr_map2["WERR_TAIL_DROP_SPAN"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 2, 10, rtmp);
   cntr_map2["WERR_MIN_SIZE_VIOL"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map2["WERR_MIN_SIZE_VIOL"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 2, 11, rtmp);
   cntr_map2["WERR_ENQUEUE_DROP"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map2["WERR_ENQUEUE_DROP"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 2, 12, rtmp);
   cntr_map2["WERR_PORT_RANGE"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map2["WERR_PORT_RANGE"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 2, 13, rtmp);
   cntr_map2["WERR_OQ_RANGE"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map2["WERR_OQ_RANGE"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 3, 0, rtmp);
   cntr_map3["WERR_DISCARDED"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map3["WERR_DISCARDED"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 3, 1, rtmp);
   cntr_map3["WERR_ADMITTED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 3, 2, rtmp);
   cntr_map3["WERR_INTRINSIC_DROP"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map3["WERR_INTRINSIC_DROP"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 3, 3, rtmp);
   cntr_map3["WERR_OUT_OF_CELLS"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 3, 4, rtmp);
   cntr_map3["WERR_OUT_OF_CREDIT"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map3["WERR_OUT_OF_CREDIT"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 3, 5, rtmp);
   cntr_map3["WERR_TRUNCATION"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 3, 6, rtmp);
   cntr_map3["WERR_PORT_DISABLED"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map3["WERR_PORT_DISABLED"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 3, 7, rtmp);
   cntr_map3["WERR_OUT_OF_CELLS1"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map3["WERR_OUT_OF_CELLS1"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 3, 8, rtmp);
   cntr_map3["WERR_TAIL_DROP_CPU"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 3, 9, rtmp);
   cntr_map3["WERR_TAIL_DROP_SPAN"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 3, 10, rtmp);
   cntr_map3["WERR_MIN_SIZE_VIOL"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map3["WERR_MIN_SIZE_VIOL"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 3, 11, rtmp);
   cntr_map3["WERR_ENQUEUE_DROP"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map3["WERR_ENQUEUE_DROP"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 3, 12, rtmp);
   cntr_map3["WERR_PORT_RANGE"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map3["WERR_PORT_RANGE"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 3, 13, rtmp);
   cntr_map3["WERR_OQ_RANGE"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map3["WERR_OQ_RANGE"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 4, 0, rtmp);
   cntr_map4["WERR_DISCARDED"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map4["WERR_DISCARDED"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 4, 1, rtmp);
   cntr_map4["WERR_ADMITTED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 4, 2, rtmp);
   cntr_map4["WERR_INTRINSIC_DROP"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map4["WERR_INTRINSIC_DROP"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 4, 3, rtmp);
   cntr_map4["WERR_OUT_OF_CELLS"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 4, 4, rtmp);
   cntr_map4["WERR_OUT_OF_CREDIT"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map4["WERR_OUT_OF_CREDIT"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 4, 5, rtmp);
   cntr_map4["WERR_TRUNCATION"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 4, 6, rtmp);
   cntr_map4["WERR_PORT_DISABLED"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map4["WERR_PORT_DISABLED"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 4, 7, rtmp);
   cntr_map4["WERR_OUT_OF_CELLS1"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map4["WERR_OUT_OF_CELLS1"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 4, 8, rtmp);
   cntr_map4["WERR_TAIL_DROP_CPU"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 4, 9, rtmp);
   cntr_map4["WERR_TAIL_DROP_SPAN"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 4, 10, rtmp);
   cntr_map4["WERR_MIN_SIZE_VIOL"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map4["WERR_MIN_SIZE_VIOL"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 4, 11, rtmp);
   cntr_map4["WERR_ENQUEUE_DROP"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map4["WERR_ENQUEUE_DROP"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 4, 12, rtmp);
   cntr_map4["WERR_PORT_RANGE"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map4["WERR_PORT_RANGE"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 4, 13, rtmp);
   cntr_map4["WERR_OQ_RANGE"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map4["WERR_OQ_RANGE"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 5, 0, rtmp);
   cntr_map5["WERR_DISCARDED"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map5["WERR_DISCARDED"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 5, 1, rtmp);
   cntr_map5["WERR_ADMITTED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 5, 2, rtmp);
   cntr_map5["WERR_INTRINSIC_DROP"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map5["WERR_INTRINSIC_DROP"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 5, 3, rtmp);
   cntr_map5["WERR_OUT_OF_CELLS"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 5, 4, rtmp);
   cntr_map5["WERR_OUT_OF_CREDIT"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map5["WERR_OUT_OF_CREDIT"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 5, 5, rtmp);
   cntr_map5["WERR_TRUNCATION"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 5, 6, rtmp);
   cntr_map5["WERR_PORT_DISABLED"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map5["WERR_PORT_DISABLED"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 5, 7, rtmp);
   cntr_map5["WERR_OUT_OF_CELLS1"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map5["WERR_OUT_OF_CELLS1"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 5, 8, rtmp);
   cntr_map5["WERR_TAIL_DROP_CPU"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 5, 9, rtmp);
   cntr_map5["WERR_TAIL_DROP_SPAN"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 5, 10, rtmp);
   cntr_map5["WERR_MIN_SIZE_VIOL"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map5["WERR_MIN_SIZE_VIOL"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 5, 11, rtmp);
   cntr_map5["WERR_ENQUEUE_DROP"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map5["WERR_ENQUEUE_DROP"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 5, 12, rtmp);
   cntr_map5["WERR_PORT_RANGE"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map5["WERR_PORT_RANGE"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 5, 13, rtmp);
   cntr_map5["WERR_OQ_RANGE"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map5["WERR_OQ_RANGE"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 6, 0, rtmp);
   cntr_map6["WERR_DISCARDED"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map6["WERR_DISCARDED"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 6, 1, rtmp);
   cntr_map6["WERR_ADMITTED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 6, 2, rtmp);
   cntr_map6["WERR_INTRINSIC_DROP"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map6["WERR_INTRINSIC_DROP"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 6, 3, rtmp);
   cntr_map6["WERR_OUT_OF_CELLS"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 6, 4, rtmp);
   cntr_map6["WERR_OUT_OF_CREDIT"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map6["WERR_OUT_OF_CREDIT"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 6, 5, rtmp);
   cntr_map6["WERR_TRUNCATION"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 6, 6, rtmp);
   cntr_map6["WERR_PORT_DISABLED"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map6["WERR_PORT_DISABLED"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 6, 7, rtmp);
   cntr_map6["WERR_OUT_OF_CELLS1"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map6["WERR_OUT_OF_CELLS1"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 6, 8, rtmp);
   cntr_map6["WERR_TAIL_DROP_CPU"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 6, 9, rtmp);
   cntr_map6["WERR_TAIL_DROP_SPAN"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 6, 10, rtmp);
   cntr_map6["WERR_MIN_SIZE_VIOL"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map6["WERR_MIN_SIZE_VIOL"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 6, 11, rtmp);
   cntr_map6["WERR_ENQUEUE_DROP"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map6["WERR_ENQUEUE_DROP"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 6, 12, rtmp);
   cntr_map6["WERR_PORT_RANGE"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map6["WERR_PORT_RANGE"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 6, 13, rtmp);
   cntr_map6["WERR_OQ_RANGE"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map6["WERR_OQ_RANGE"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 7, 0, rtmp);
   cntr_map7["WERR_DISCARDED"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map7["WERR_DISCARDED"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 7, 1, rtmp);
   cntr_map7["WERR_ADMITTED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 7, 2, rtmp);
   cntr_map7["WERR_INTRINSIC_DROP"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map7["WERR_INTRINSIC_DROP"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 7, 3, rtmp);
   cntr_map7["WERR_OUT_OF_CELLS"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 7, 4, rtmp);
   cntr_map7["WERR_OUT_OF_CREDIT"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map7["WERR_OUT_OF_CREDIT"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 7, 5, rtmp);
   cntr_map7["WERR_TRUNCATION"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 7, 6, rtmp);
   cntr_map7["WERR_PORT_DISABLED"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map7["WERR_PORT_DISABLED"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 7, 7, rtmp);
   cntr_map7["WERR_OUT_OF_CELLS1"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map7["WERR_OUT_OF_CELLS1"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 7, 8, rtmp);
   cntr_map7["WERR_TAIL_DROP_CPU"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 7, 9, rtmp);
   cntr_map7["WERR_TAIL_DROP_SPAN"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 7, 10, rtmp);
   cntr_map7["WERR_MIN_SIZE_VIOL"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map7["WERR_MIN_SIZE_VIOL"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 7, 11, rtmp);
   cntr_map7["WERR_ENQUEUE_DROP"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map7["WERR_ENQUEUE_DROP"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 7, 12, rtmp);
   cntr_map7["WERR_PORT_RANGE"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map7["WERR_PORT_RANGE"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 7, 13, rtmp);
   cntr_map7["WERR_OQ_RANGE"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map7["WERR_OQ_RANGE"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 8, 0, rtmp);
   cntr_map8["WERR_DISCARDED"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map8["WERR_DISCARDED"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 8, 1, rtmp);
   cntr_map8["WERR_ADMITTED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 8, 2, rtmp);
   cntr_map8["WERR_INTRINSIC_DROP"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map8["WERR_INTRINSIC_DROP"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 8, 3, rtmp);
   cntr_map8["WERR_OUT_OF_CELLS"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 8, 4, rtmp);
   cntr_map8["WERR_OUT_OF_CREDIT"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map8["WERR_OUT_OF_CREDIT"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 8, 5, rtmp);
   cntr_map8["WERR_TRUNCATION"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 8, 6, rtmp);
   cntr_map8["WERR_PORT_DISABLED"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map8["WERR_PORT_DISABLED"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 8, 7, rtmp);
   cntr_map8["WERR_OUT_OF_CELLS1"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map8["WERR_OUT_OF_CELLS1"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 8, 8, rtmp);
   cntr_map8["WERR_TAIL_DROP_CPU"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 8, 9, rtmp);
   cntr_map8["WERR_TAIL_DROP_SPAN"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 8, 10, rtmp);
   cntr_map8["WERR_MIN_SIZE_VIOL"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map8["WERR_MIN_SIZE_VIOL"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 8, 11, rtmp);
   cntr_map8["WERR_ENQUEUE_DROP"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map8["WERR_ENQUEUE_DROP"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 8, 12, rtmp);
   cntr_map8["WERR_PORT_RANGE"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map8["WERR_PORT_RANGE"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 8, 13, rtmp);
   cntr_map8["WERR_OQ_RANGE"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map8["WERR_OQ_RANGE"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 9, 0, rtmp);
   cntr_map9["WERR_DISCARDED"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map9["WERR_DISCARDED"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 9, 1, rtmp);
   cntr_map9["WERR_ADMITTED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 9, 2, rtmp);
   cntr_map9["WERR_INTRINSIC_DROP"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map9["WERR_INTRINSIC_DROP"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 9, 3, rtmp);
   cntr_map9["WERR_OUT_OF_CELLS"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 9, 4, rtmp);
   cntr_map9["WERR_OUT_OF_CREDIT"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map9["WERR_OUT_OF_CREDIT"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 9, 5, rtmp);
   cntr_map9["WERR_TRUNCATION"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 9, 6, rtmp);
   cntr_map9["WERR_PORT_DISABLED"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map9["WERR_PORT_DISABLED"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 9, 7, rtmp);
   cntr_map9["WERR_OUT_OF_CELLS1"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map9["WERR_OUT_OF_CELLS1"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 9, 8, rtmp);
   cntr_map9["WERR_TAIL_DROP_CPU"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 9, 9, rtmp);
   cntr_map9["WERR_TAIL_DROP_SPAN"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 9, 10, rtmp);
   cntr_map9["WERR_MIN_SIZE_VIOL"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map9["WERR_MIN_SIZE_VIOL"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 9, 11, rtmp);
   cntr_map9["WERR_ENQUEUE_DROP"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map9["WERR_ENQUEUE_DROP"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 9, 12, rtmp);
   cntr_map9["WERR_PORT_RANGE"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map9["WERR_PORT_RANGE"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 9, 13, rtmp);
   cntr_map9["WERR_OQ_RANGE"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map9["WERR_OQ_RANGE"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 10, 0, rtmp);
   cntr_map10["WERR_DISCARDED"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map10["WERR_DISCARDED"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 10, 1, rtmp);
   cntr_map10["WERR_ADMITTED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 10, 2, rtmp);
   cntr_map10["WERR_INTRINSIC_DROP"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map10["WERR_INTRINSIC_DROP"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 10, 3, rtmp);
   cntr_map10["WERR_OUT_OF_CELLS"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 10, 4, rtmp);
   cntr_map10["WERR_OUT_OF_CREDIT"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map10["WERR_OUT_OF_CREDIT"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 10, 5, rtmp);
   cntr_map10["WERR_TRUNCATION"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 10, 6, rtmp);
   cntr_map10["WERR_PORT_DISABLED"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map10["WERR_PORT_DISABLED"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 10, 7, rtmp);
   cntr_map10["WERR_OUT_OF_CELLS1"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map10["WERR_OUT_OF_CELLS1"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 10, 8, rtmp);
   cntr_map10["WERR_TAIL_DROP_CPU"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 10, 9, rtmp);
   cntr_map10["WERR_TAIL_DROP_SPAN"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 10, 10, rtmp);
   cntr_map10["WERR_MIN_SIZE_VIOL"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map10["WERR_MIN_SIZE_VIOL"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 10, 11, rtmp);
   cntr_map10["WERR_ENQUEUE_DROP"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map10["WERR_ENQUEUE_DROP"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 10, 12, rtmp);
   cntr_map10["WERR_PORT_RANGE"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map10["WERR_PORT_RANGE"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 10, 13, rtmp);
   cntr_map10["WERR_OQ_RANGE"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map10["WERR_OQ_RANGE"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 11, 0, rtmp);
   cntr_map11["WERR_DISCARDED"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map11["WERR_DISCARDED"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 11, 1, rtmp);
   cntr_map11["WERR_ADMITTED"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 11, 2, rtmp);
   cntr_map11["WERR_INTRINSIC_DROP"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map11["WERR_INTRINSIC_DROP"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 11, 3, rtmp);
   cntr_map11["WERR_OUT_OF_CELLS"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 11, 4, rtmp);
   cntr_map11["WERR_OUT_OF_CREDIT"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map11["WERR_OUT_OF_CREDIT"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 11, 5, rtmp);
   cntr_map11["WERR_TRUNCATION"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 11, 6, rtmp);
   cntr_map11["WERR_PORT_DISABLED"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map11["WERR_PORT_DISABLED"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 11, 7, rtmp);
   cntr_map11["WERR_OUT_OF_CELLS1"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map11["WERR_OUT_OF_CELLS1"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 11, 8, rtmp);
   cntr_map11["WERR_TAIL_DROP_CPU"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 11, 9, rtmp);
   cntr_map11["WERR_TAIL_DROP_SPAN"] = rtmp.convert_to<uint64_t>();
   cap_pb_get_write_error_cnt(chip_id, inst_id, 11, 10, rtmp);
   cntr_map11["WERR_MIN_SIZE_VIOL"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map11["WERR_MIN_SIZE_VIOL"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 11, 11, rtmp);
   cntr_map11["WERR_ENQUEUE_DROP"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map11["WERR_ENQUEUE_DROP"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 11, 12, rtmp);
   cntr_map11["WERR_PORT_RANGE"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map11["WERR_PORT_RANGE"];
   cap_pb_get_write_error_cnt(chip_id, inst_id, 11, 13, rtmp);
   cntr_map11["WERR_OQ_RANGE"] = rtmp.convert_to<uint64_t>();
   tot_drop_cnt += cntr_map11["WERR_OQ_RANGE"];
   pbc_csr.hbm.hbm_port_0.cnt_hbm.read();
   cntr_map_hbm0["HBM_OCCUPANCY_DROP"] = pbc_csr.hbm.hbm_port_0.cnt_hbm.occupancy_drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm0["HBM_OCCUPANCY_DROP"];
   pbc_csr.hbm.cnt_hbm0_emergency_stop.read();
   cntr_map_hbm0["HBM_WDROP_EMERGENCY_STOP"] = pbc_csr.hbm.cnt_hbm0_emergency_stop.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm0["HBM_WDROP_EMERGENCY_STOP"];
   pbc_csr.hbm.cnt_hbm0_write_ack_filling_up.read();
   cntr_map_hbm0["HBM_WDROP_WR_ACK_FILL_UP"] = pbc_csr.hbm.cnt_hbm0_write_ack_filling_up.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm0["HBM_WDROP_WR_ACK_FILL_UP"];
   pbc_csr.hbm.cnt_hbm0_write_ack_full.read();
   cntr_map_hbm0["HBM_WDROP_WR_ACK_FULL"] = pbc_csr.hbm.cnt_hbm0_write_ack_full.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm0["HBM_WDROP_WR_ACK_FULL"];
   pbc_csr.hbm.cnt_hbm0_truncate.read();
   cntr_map_hbm0["HBM_WDROP_TRUNCATE"] = pbc_csr.hbm.cnt_hbm0_truncate.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm0["HBM_WDROP_TRUNCATE"];
   pbc_csr.hbm.sat_hbm0_ctrl_full.read();
   cntr_map_hbm0["HBM_WDROP_CTRL_FULL"] = pbc_csr.hbm.sat_hbm0_ctrl_full.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm0["HBM_WDROP_CTRL_FULL"];
   pbc_csr.hbm.hbm_port_1.cnt_hbm.read();
   cntr_map_hbm1["HBM_OCCUPANCY_DROP"] = pbc_csr.hbm.hbm_port_1.cnt_hbm.occupancy_drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm1["HBM_OCCUPANCY_DROP"];
   pbc_csr.hbm.cnt_hbm1_emergency_stop.read();
   cntr_map_hbm1["HBM_WDROP_EMERGENCY_STOP"] = pbc_csr.hbm.cnt_hbm1_emergency_stop.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm1["HBM_WDROP_EMERGENCY_STOP"];
   pbc_csr.hbm.cnt_hbm1_write_ack_filling_up.read();
   cntr_map_hbm1["HBM_WDROP_WR_ACK_FILL_UP"] = pbc_csr.hbm.cnt_hbm1_write_ack_filling_up.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm1["HBM_WDROP_WR_ACK_FILL_UP"];
   pbc_csr.hbm.cnt_hbm1_write_ack_full.read();
   cntr_map_hbm1["HBM_WDROP_WR_ACK_FULL"] = pbc_csr.hbm.cnt_hbm1_write_ack_full.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm1["HBM_WDROP_WR_ACK_FULL"];
   pbc_csr.hbm.cnt_hbm1_truncate.read();
   cntr_map_hbm1["HBM_WDROP_TRUNCATE"] = pbc_csr.hbm.cnt_hbm1_truncate.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm1["HBM_WDROP_TRUNCATE"];
   pbc_csr.hbm.sat_hbm1_ctrl_full.read();
   cntr_map_hbm1["HBM_WDROP_CTRL_FULL"] = pbc_csr.hbm.sat_hbm1_ctrl_full.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm1["HBM_WDROP_CTRL_FULL"];
   pbc_csr.hbm.hbm_port_2.cnt_hbm.read();
   cntr_map_hbm2["HBM_OCCUPANCY_DROP"] = pbc_csr.hbm.hbm_port_2.cnt_hbm.occupancy_drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm2["HBM_OCCUPANCY_DROP"];
   pbc_csr.hbm.cnt_hbm2_emergency_stop.read();
   cntr_map_hbm2["HBM_WDROP_EMERGENCY_STOP"] = pbc_csr.hbm.cnt_hbm2_emergency_stop.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm2["HBM_WDROP_EMERGENCY_STOP"];
   pbc_csr.hbm.cnt_hbm2_write_ack_filling_up.read();
   cntr_map_hbm2["HBM_WDROP_WR_ACK_FILL_UP"] = pbc_csr.hbm.cnt_hbm2_write_ack_filling_up.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm2["HBM_WDROP_WR_ACK_FILL_UP"];
   pbc_csr.hbm.cnt_hbm2_write_ack_full.read();
   cntr_map_hbm2["HBM_WDROP_WR_ACK_FULL"] = pbc_csr.hbm.cnt_hbm2_write_ack_full.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm2["HBM_WDROP_WR_ACK_FULL"];
   pbc_csr.hbm.cnt_hbm2_truncate.read();
   cntr_map_hbm2["HBM_WDROP_TRUNCATE"] = pbc_csr.hbm.cnt_hbm2_truncate.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm2["HBM_WDROP_TRUNCATE"];
   pbc_csr.hbm.sat_hbm2_ctrl_full.read();
   cntr_map_hbm2["HBM_WDROP_CTRL_FULL"] = pbc_csr.hbm.sat_hbm2_ctrl_full.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm2["HBM_WDROP_CTRL_FULL"];
   pbc_csr.hbm.hbm_port_3.cnt_hbm.read();
   cntr_map_hbm3["HBM_OCCUPANCY_DROP"] = pbc_csr.hbm.hbm_port_3.cnt_hbm.occupancy_drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm3["HBM_OCCUPANCY_DROP"];
   pbc_csr.hbm.cnt_hbm3_emergency_stop.read();
   cntr_map_hbm3["HBM_WDROP_EMERGENCY_STOP"] = pbc_csr.hbm.cnt_hbm3_emergency_stop.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm3["HBM_WDROP_EMERGENCY_STOP"];
   pbc_csr.hbm.cnt_hbm3_write_ack_filling_up.read();
   cntr_map_hbm3["HBM_WDROP_WR_ACK_FILL_UP"] = pbc_csr.hbm.cnt_hbm3_write_ack_filling_up.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm3["HBM_WDROP_WR_ACK_FILL_UP"];
   pbc_csr.hbm.cnt_hbm3_write_ack_full.read();
   cntr_map_hbm3["HBM_WDROP_WR_ACK_FULL"] = pbc_csr.hbm.cnt_hbm3_write_ack_full.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm3["HBM_WDROP_WR_ACK_FULL"];
   pbc_csr.hbm.cnt_hbm3_truncate.read();
   cntr_map_hbm3["HBM_WDROP_TRUNCATE"] = pbc_csr.hbm.cnt_hbm3_truncate.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm3["HBM_WDROP_TRUNCATE"];
   pbc_csr.hbm.sat_hbm3_ctrl_full.read();
   cntr_map_hbm3["HBM_WDROP_CTRL_FULL"] = pbc_csr.hbm.sat_hbm3_ctrl_full.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm3["HBM_WDROP_CTRL_FULL"];
   pbc_csr.hbm.hbm_port_4.cnt_hbm.read();
   cntr_map_hbm4["HBM_OCCUPANCY_DROP"] = pbc_csr.hbm.hbm_port_4.cnt_hbm.occupancy_drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm4["HBM_OCCUPANCY_DROP"];
   pbc_csr.hbm.cnt_hbm4_emergency_stop.read();
   cntr_map_hbm4["HBM_WDROP_EMERGENCY_STOP"] = pbc_csr.hbm.cnt_hbm4_emergency_stop.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm4["HBM_WDROP_EMERGENCY_STOP"];
   pbc_csr.hbm.cnt_hbm4_write_ack_filling_up.read();
   cntr_map_hbm4["HBM_WDROP_WR_ACK_FILL_UP"] = pbc_csr.hbm.cnt_hbm4_write_ack_filling_up.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm4["HBM_WDROP_WR_ACK_FILL_UP"];
   pbc_csr.hbm.cnt_hbm4_write_ack_full.read();
   cntr_map_hbm4["HBM_WDROP_WR_ACK_FULL"] = pbc_csr.hbm.cnt_hbm4_write_ack_full.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm4["HBM_WDROP_WR_ACK_FULL"];
   pbc_csr.hbm.cnt_hbm4_truncate.read();
   cntr_map_hbm4["HBM_WDROP_TRUNCATE"] = pbc_csr.hbm.cnt_hbm4_truncate.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm4["HBM_WDROP_TRUNCATE"];
   pbc_csr.hbm.sat_hbm4_ctrl_full.read();
   cntr_map_hbm4["HBM_WDROP_CTRL_FULL"] = pbc_csr.hbm.sat_hbm4_ctrl_full.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm4["HBM_WDROP_CTRL_FULL"];
   pbc_csr.hbm.hbm_port_5.cnt_hbm.read();
   cntr_map_hbm5["HBM_OCCUPANCY_DROP"] = pbc_csr.hbm.hbm_port_5.cnt_hbm.occupancy_drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm5["HBM_OCCUPANCY_DROP"];
   pbc_csr.hbm.cnt_hbm5_emergency_stop.read();
   cntr_map_hbm5["HBM_WDROP_EMERGENCY_STOP"] = pbc_csr.hbm.cnt_hbm5_emergency_stop.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm5["HBM_WDROP_EMERGENCY_STOP"];
   pbc_csr.hbm.cnt_hbm5_write_ack_filling_up.read();
   cntr_map_hbm5["HBM_WDROP_WR_ACK_FILL_UP"] = pbc_csr.hbm.cnt_hbm5_write_ack_filling_up.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm5["HBM_WDROP_WR_ACK_FILL_UP"];
   pbc_csr.hbm.cnt_hbm5_write_ack_full.read();
   cntr_map_hbm5["HBM_WDROP_WR_ACK_FULL"] = pbc_csr.hbm.cnt_hbm5_write_ack_full.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm5["HBM_WDROP_WR_ACK_FULL"];
   pbc_csr.hbm.cnt_hbm5_truncate.read();
   cntr_map_hbm5["HBM_WDROP_TRUNCATE"] = pbc_csr.hbm.cnt_hbm5_truncate.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm5["HBM_WDROP_TRUNCATE"];
   pbc_csr.hbm.sat_hbm5_ctrl_full.read();
   cntr_map_hbm5["HBM_WDROP_CTRL_FULL"] = pbc_csr.hbm.sat_hbm5_ctrl_full.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm5["HBM_WDROP_CTRL_FULL"];
   pbc_csr.hbm.hbm_port_6.cnt_hbm.read();
   cntr_map_hbm6["HBM_OCCUPANCY_DROP"] = pbc_csr.hbm.hbm_port_6.cnt_hbm.occupancy_drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm6["HBM_OCCUPANCY_DROP"];
   pbc_csr.hbm.cnt_hbm6_emergency_stop.read();
   cntr_map_hbm6["HBM_WDROP_EMERGENCY_STOP"] = pbc_csr.hbm.cnt_hbm6_emergency_stop.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm6["HBM_WDROP_EMERGENCY_STOP"];
   pbc_csr.hbm.cnt_hbm6_write_ack_filling_up.read();
   cntr_map_hbm6["HBM_WDROP_WR_ACK_FILL_UP"] = pbc_csr.hbm.cnt_hbm6_write_ack_filling_up.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm6["HBM_WDROP_WR_ACK_FILL_UP"];
   pbc_csr.hbm.cnt_hbm6_write_ack_full.read();
   cntr_map_hbm6["HBM_WDROP_WR_ACK_FULL"] = pbc_csr.hbm.cnt_hbm6_write_ack_full.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm6["HBM_WDROP_WR_ACK_FULL"];
   pbc_csr.hbm.cnt_hbm6_truncate.read();
   cntr_map_hbm6["HBM_WDROP_TRUNCATE"] = pbc_csr.hbm.cnt_hbm6_truncate.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm6["HBM_WDROP_TRUNCATE"];
   pbc_csr.hbm.sat_hbm6_ctrl_full.read();
   cntr_map_hbm6["HBM_WDROP_CTRL_FULL"] = pbc_csr.hbm.sat_hbm6_ctrl_full.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm6["HBM_WDROP_CTRL_FULL"];
   pbc_csr.hbm.hbm_port_7.cnt_hbm.read();
   cntr_map_hbm7["HBM_OCCUPANCY_DROP"] = pbc_csr.hbm.hbm_port_7.cnt_hbm.occupancy_drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm7["HBM_OCCUPANCY_DROP"];
   pbc_csr.hbm.cnt_hbm7_emergency_stop.read();
   cntr_map_hbm7["HBM_WDROP_EMERGENCY_STOP"] = pbc_csr.hbm.cnt_hbm7_emergency_stop.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm7["HBM_WDROP_EMERGENCY_STOP"];
   pbc_csr.hbm.cnt_hbm7_write_ack_filling_up.read();
   cntr_map_hbm7["HBM_WDROP_WR_ACK_FILL_UP"] = pbc_csr.hbm.cnt_hbm7_write_ack_filling_up.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm7["HBM_WDROP_WR_ACK_FILL_UP"];
   pbc_csr.hbm.cnt_hbm7_write_ack_full.read();
   cntr_map_hbm7["HBM_WDROP_WR_ACK_FULL"] = pbc_csr.hbm.cnt_hbm7_write_ack_full.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm7["HBM_WDROP_WR_ACK_FULL"];
   pbc_csr.hbm.cnt_hbm7_truncate.read();
   cntr_map_hbm7["HBM_WDROP_TRUNCATE"] = pbc_csr.hbm.cnt_hbm7_truncate.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm7["HBM_WDROP_TRUNCATE"];
   pbc_csr.hbm.sat_hbm7_ctrl_full.read();
   cntr_map_hbm7["HBM_WDROP_CTRL_FULL"] = pbc_csr.hbm.sat_hbm7_ctrl_full.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm7["HBM_WDROP_CTRL_FULL"];
   pbc_csr.hbm.hbm_port_8.cnt_hbm.read();
   cntr_map_hbm8["HBM_OCCUPANCY_DROP"] = pbc_csr.hbm.hbm_port_8.cnt_hbm.occupancy_drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm8["HBM_OCCUPANCY_DROP"];
   pbc_csr.hbm.cnt_hbm8_emergency_stop.read();
   cntr_map_hbm8["HBM_WDROP_EMERGENCY_STOP"] = pbc_csr.hbm.cnt_hbm8_emergency_stop.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm8["HBM_WDROP_EMERGENCY_STOP"];
   pbc_csr.hbm.cnt_hbm8_write_ack_filling_up.read();
   cntr_map_hbm8["HBM_WDROP_WR_ACK_FILL_UP"] = pbc_csr.hbm.cnt_hbm8_write_ack_filling_up.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm8["HBM_WDROP_WR_ACK_FILL_UP"];
   pbc_csr.hbm.cnt_hbm8_write_ack_full.read();
   cntr_map_hbm8["HBM_WDROP_WR_ACK_FULL"] = pbc_csr.hbm.cnt_hbm8_write_ack_full.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm8["HBM_WDROP_WR_ACK_FULL"];
   pbc_csr.hbm.cnt_hbm8_truncate.read();
   cntr_map_hbm8["HBM_WDROP_TRUNCATE"] = pbc_csr.hbm.cnt_hbm8_truncate.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm8["HBM_WDROP_TRUNCATE"];
   pbc_csr.hbm.sat_hbm8_ctrl_full.read();
   cntr_map_hbm8["HBM_WDROP_CTRL_FULL"] = pbc_csr.hbm.sat_hbm8_ctrl_full.drop().convert_to<uint64_t>();
   tot_drop_cnt += cntr_map_hbm8["HBM_WDROP_CTRL_FULL"];
   pbc_csr.hbm.hbm_port_9.cnt_hbm.read();
   cntr_map_hbm9["HBM_OCCUPANCY_STOP"] = pbc_csr.hbm.hbm_port_9.cnt_hbm.occupancy_stop().convert_to<uint64_t>();
 if (no_print == 0) {
   PB_PRN_MSG(setw(125) << "======================================================= PB DROP CNTRS ===========================================================\n");
   PB_PRN_MSG(setw(39) << "port." << 0 << setw(14) << "port." << 1 << setw(14) << "port." << 2 << setw(14) << "port." << 3 << setw(14) << "port." << 4 << setw(14) << "port." << 5 << "\n\n");
   for(auto i : cntr_map0) {
     string counter_name = i.first;
     counter_name.append(25-counter_name.length(),' ');
     stringstream sdata;
     sdata.str("");
     stringstream rval0;
     if (i.second == 0) {
       rval0 << "-";
     } else {
       if (is_hex) {
          rval0 << "0x" << hex << i.second;
       } else {
          rval0 << dec << i.second;
       }
     }
     stringstream rval1;
     rval1.str("-");
     if (cntr_map1[i.first] != 0) {
       if (is_hex) {
          rval1 << "0x" << hex << cntr_map1[i.first];
       } else {
          rval1 << dec << cntr_map1[i.first];
       }
     }
     stringstream rval2;
     rval2.str("-");
     if (cntr_map2[i.first] != 0) {
       if (is_hex) {
          rval2 << "0x" << hex << cntr_map2[i.first];
       } else {
          rval2 << dec << cntr_map2[i.first];
       }
     }
     stringstream rval3;
     rval3.str("-");
     if (cntr_map3[i.first] != 0) {
       if (is_hex) {
          rval3 << "0x" << hex << cntr_map3[i.first];
       } else {
          rval3 << dec << cntr_map3[i.first];
       }
     }
     stringstream rval4;
     rval4.str("-");
     if (cntr_map4[i.first] != 0) {
       if (is_hex) {
          rval4 << "0x" << hex << cntr_map4[i.first];
       } else {
          rval4 << dec << cntr_map4[i.first];
       }
     }
     stringstream rval5;
     rval5.str("-");
     if (cntr_map5[i.first] != 0) {
       if (is_hex) {
          rval5 << "0x" << hex << cntr_map5[i.first];
       } else {
          rval5 << dec << cntr_map5[i.first];
       }
     }
     sdata << setw(15) << rval0.str() << setw(15) << rval1.str() << setw(15) << rval2.str() << setw(15) << rval3.str() << setw(15) << rval4.str() << setw(15) << rval5.str();
     PB_PRN_MSG(setw(25) << counter_name << sdata.str() << "\n");
   }
   PB_PRN_MSG(setw(125) << "------------------------------------------------------------------------------------------------------------------------------\n");
   for(auto i : cntr_map_hbm0) {
     string counter_name = i.first;
     counter_name.append(25-counter_name.length(),' ');
     stringstream sdata;
     sdata.str("");
     stringstream rval0;
     if (i.second == 0) {
       rval0 << "-";
     } else {
       if (is_hex) {
          rval0 << "0x" << hex << i.second;
       } else {
          rval0 << dec << i.second;
       }
     }
     stringstream rval1;
     rval1.str("-");
     if (cntr_map_hbm1[i.first] != 0) {
       if (is_hex) {
          rval1 << "0x" << hex << cntr_map_hbm1[i.first];
       } else {
          rval1 << dec << cntr_map_hbm1[i.first];
       }
     }
     stringstream rval2;
     rval2.str("-");
     if (cntr_map_hbm2[i.first] != 0) {
       if (is_hex) {
          rval2 << "0x" << hex << cntr_map_hbm2[i.first];
       } else {
          rval2 << dec << cntr_map_hbm2[i.first];
       }
     }
     stringstream rval3;
     rval3.str("-");
     if (cntr_map_hbm3[i.first] != 0) {
       if (is_hex) {
          rval3 << "0x" << hex << cntr_map_hbm3[i.first];
       } else {
          rval3 << dec << cntr_map_hbm3[i.first];
       }
     }
     stringstream rval4;
     rval4.str("-");
     if (cntr_map_hbm4[i.first] != 0) {
       if (is_hex) {
          rval4 << "0x" << hex << cntr_map_hbm4[i.first];
       } else {
          rval4 << dec << cntr_map_hbm4[i.first];
       }
     }
     stringstream rval5;
     rval5.str("-");
     if (cntr_map_hbm5[i.first] != 0) {
       if (is_hex) {
          rval5 << "0x" << hex << cntr_map_hbm5[i.first];
       } else {
          rval5 << dec << cntr_map_hbm5[i.first];
       }
     }
     sdata << setw(15) << rval0.str() << setw(15) << rval1.str() << setw(15) << rval2.str() << setw(15) << rval3.str() << setw(15) << rval4.str() << setw(15) << rval5.str();
     PB_PRN_MSG(setw(25) << counter_name << sdata.str() << "\n");
   }
   PB_PRN_MSG(setw(125) << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
   PB_PRN_MSG(setw(39) << "port." << 6 << setw(14) << "port." << 7 << setw(14) << "port." << 8 << setw(14) << "port." << 9 << setw(13) << "port." << 10 << setw(13) << "port." << 11 << "\n\n");
   for(auto i : cntr_map6) {
     string counter_name = i.first;
     counter_name.append(25-counter_name.length(),' ');
     stringstream sdata;
     sdata.str("");
     stringstream rval0;
     if (i.second == 0) {
       rval0 << "-";
     } else {
       if (is_hex) {
          rval0 << "0x" << hex << i.second;
       } else {
          rval0 << dec << i.second;
       }
     }
     stringstream rval1;
     rval1.str("-");
     if (cntr_map7[i.first] != 0) {
       if (is_hex) {
          rval1 << "0x" << hex << cntr_map7[i.first];
       } else {
          rval1 << dec << cntr_map7[i.first];
       }
     }
     stringstream rval2;
     rval2.str("-");
     if (cntr_map8[i.first] != 0) {
       if (is_hex) {
          rval2 << "0x" << hex << cntr_map8[i.first];
       } else {
          rval2 << dec << cntr_map8[i.first];
       }
     }
     stringstream rval3;
     rval3.str("-");
     if (cntr_map9[i.first] != 0) {
       if (is_hex) {
          rval3 << "0x" << hex << cntr_map9[i.first];
       } else {
          rval3 << dec << cntr_map9[i.first];
       }
     }
     stringstream rval4;
     rval4.str("-");
     if (cntr_map10[i.first] != 0) {
       if (is_hex) {
          rval4 << "0x" << hex << cntr_map10[i.first];
       } else {
          rval4 << dec << cntr_map10[i.first];
       }
     }
     stringstream rval5;
     rval5.str("-");
     if (cntr_map11[i.first] != 0) {
       if (is_hex) {
          rval5 << "0x" << hex << cntr_map11[i.first];
       } else {
          rval5 << dec << cntr_map11[i.first];
       }
     }
     sdata << setw(15) << rval0.str() << setw(15) << rval1.str() << setw(15) << rval2.str() << setw(15) << rval3.str() << setw(15) << rval4.str() << setw(15) << rval5.str();
     PB_PRN_MSG(setw(25) << counter_name << sdata.str() << "\n");
   }
   PB_PRN_MSG(setw(125) << "------------------------------------------------------------------------------------------------------------------------------\n");
   for(auto i : cntr_map_hbm6) {
     string counter_name = i.first;
     counter_name.append(25-counter_name.length(),' ');
     stringstream sdata;
     sdata.str("");
     stringstream rval0;
     if (i.second == 0) {
       rval0 << "-";
     } else {
       if (is_hex) {
          rval0 << "0x" << hex << i.second;
       } else {
          rval0 << dec << i.second;
       }
     }
     stringstream rval1;
     rval1.str("-");
     if (cntr_map_hbm7[i.first] != 0) {
       if (is_hex) {
          rval1 << "0x" << hex << cntr_map_hbm7[i.first];
       } else {
          rval1 << dec << cntr_map_hbm7[i.first];
       }
     }
     stringstream rval2;
     rval2.str("-");
     if (cntr_map_hbm8[i.first] != 0) {
       if (is_hex) {
          rval2 << "0x" << hex << cntr_map_hbm8[i.first];
       } else {
          rval2 << dec << cntr_map_hbm8[i.first];
       }
     }
     stringstream rval3;
     rval3.str("-");
     if (cntr_map_hbm9[i.first] != 0) {
       if (is_hex) {
          rval3 << "0x" << hex << cntr_map_hbm9[i.first];
       } else {
          rval3 << dec << cntr_map_hbm9[i.first];
       }
     }
     sdata << setw(15) << rval0.str() << setw(15) << rval1.str() << setw(15) << rval2.str() << setw(15) << rval3.str();
     PB_PRN_MSG(setw(25) << counter_name << sdata.str() << "\n");
   }
 PLOG_MSG("===> PB TOTAL DROP CNT = " << tot_drop_cnt  <<" <==="<< endl);
 PB_PRN_MSG(setw(125) << "================================================================================================================================\n");
 }
 cpu::access()->set_access_type(cur_cpu_access_type);
 return tot_drop_cnt;
}




