#ifndef __HAL_BUF_POOL_PD_HPP__
#define __HAL_BUF_POOL_PD_HPP__

#include "nic/include/base.h"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"


namespace hal {
namespace pd {

struct pd_buf_pool_s {
    uint32_t    hw_buf_pool_id;
    uint32_t    dummy;

    // pi ptr
    void        *pi_buf_pool;
} __PACK__;


pd_buf_pool_t *buf_pool_pd_alloc ();
pd_buf_pool_t *buf_pool_pd_init (pd_buf_pool_t *buf_pool);
pd_buf_pool_t *buf_pool_pd_alloc_init ();
hal_ret_t buf_pool_pd_free (pd_buf_pool_t *buf_pool);
hal_ret_t buf_pool_pd_alloc_res(pd_buf_pool_t *pd_buf_pool);
hal_ret_t buf_pool_pd_program_hw(pd_buf_pool_t *pd_buf_pool);
void link_pi_pd(pd_buf_pool_t *pd_buf_pool, buf_pool_t *pi_buf_pool);
void unlink_pi_pd(pd_buf_pool_t *pd_buf_pool, buf_pool_t *pi_buf_pool);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_BUF_POOL_PD_HPP__
