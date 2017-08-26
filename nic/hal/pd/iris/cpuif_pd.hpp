#ifndef __HAL_PD_CPUIF_HPP__
#define __HAL_PD_CPUIF_HPP__

#include <base.h>
#include <pd.hpp>

namespace hal {
namespace pd {

struct pd_cpuif_s {
    uint32_t    cpu_lport_id;   // lport for cpu

    void        *pi_if;         // pi ptr
} __PACK__;

hal_ret_t pd_cpuif_create(pd_if_args_t *args);
pd_cpuif_t *pd_cpuif_alloc_init(void);
pd_cpuif_t *pd_cpuif_alloc (void);
pd_cpuif_t *pd_cpuif_init (pd_cpuif_t *cpuif);
hal_ret_t pd_cpuif_alloc_res(pd_cpuif_t *pd_cpuif);
hal_ret_t pd_cpuif_program_hw(pd_cpuif_t *pd_cpuif);
hal_ret_t pd_cpuif_free (pd_cpuif_t *cpuif);
void link_pi_pd(pd_cpuif_t *pd_upif, if_t *pi_if);
void unlink_pi_pd(pd_cpuif_t *pd_upif, if_t *pi_if);
hal_ret_t pd_cpuif_pd_pgm_output_mapping_tbl(pd_cpuif_t *pd_cpuif);

pd_lif_t *pd_cpuif_get_pd_lif(pd_cpuif_t *pd_cpuif);
}   // namespace pd
}   // namespace hal
#endif    // __HAL_PD_CPUIF_HPP__

