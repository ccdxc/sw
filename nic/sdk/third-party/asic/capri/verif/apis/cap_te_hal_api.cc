#include "cap_te_hal_api.h"
#include "cpu.h"
#include "cap_base_api.h"
#include "dpi_registry.h"
#include "cap_blk_reg_model.h"
#include "cap_te_csr.h"
#include "cap_csr_util_api.h"
void cap_te_soft_reset(int chip_id, int inst_id) {

  //cap_te_csr_t & te_csr = CAP_BLK_REG_MODEL_ACCESS(cap_te_csr_t, chip_id, inst_id);

  PLOG_API_MSG("TE[" << inst_id << "]", "inside softreset\n");
  // te_csr.cfg_global.show();
  // te_csr.cfg_global.sw_rst(0);
  // te_csr.cfg_global.write();
  // te_csr.cfg_global.show();
  PLOG_API_MSG("TE[" << inst_id << "]", "done with softreset\n");

}

void cap_te_init_start(int chip_id, int inst_id) {

  cap_te_csr_t & te_csr = CAP_BLK_REG_MODEL_ACCESS(cap_te_csr_t, chip_id, inst_id);

  PLOG_API_MSG("TE[" << inst_id << "]", "Enabling all interrupts\n");
  enable_all_interrupts(&te_csr);
  PLOG_API_MSG("TE[" << inst_id << "]", "Clearing all interrupts\n");
  //te_csr.int_err.intreg.read();
  te_csr.int_err.intreg.all(0xfffff);
  te_csr.int_err.intreg.write();
  PLOG_API_MSG("TE[" << inst_id << "]", "Disabling timeout" << endl);
  te_csr.cfg_timeout.te2mpu(0xffffff);
  te_csr.cfg_timeout.write();

}

void cap_te_init_done(int chip_id, int inst_id) {
}

