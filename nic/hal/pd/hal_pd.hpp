#ifndef __HAL_PD_HPP__
#define __HAL_PD_HPP__

#include <hal.hpp>
#include <hal_pd_error.hpp>

namespace hal {
namespace pd {

extern hal_ret_t hal_pd_init(hal::hal_cfg_t *hal_cfg);
extern hal_ret_t hal_pd_mem_init(void);
extern hal_ret_t hal_pd_pgm_def_entries (void);
extern hal_ret_t hal_pd_pgm_def_p4plus_entries (void);
extern hal_ret_t p4pd_table_defaults_init(void);

}    // namespace pd
}    // namespace hal

#endif    // __HAL_PD_HPP__

