#ifndef __CAPRI_QUIESCE_H__
#define __CAPRI_QUIESCE_H__


namespace hal {
namespace pd {

hal_ret_t   capri_quiesce_start(void);
hal_ret_t   capri_quiesce_stop(void);
hal_ret_t   capri_quiesce_init(void);

}    // namespace pd
}    // namespace hal

#endif  /* __CAPRI_QUIESCE_H__ */
