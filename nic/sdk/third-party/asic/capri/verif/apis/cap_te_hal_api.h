#ifndef CAP_TE_HAL_API_H
#define CAP_TE_HAL_API_H

void cap_te_soft_reset(int chip_id, int inst_id);
void cap_te_init_start(int chip_id, int inst_id);
void cap_te_init_done(int chip_id, int inst_id);

#endif
