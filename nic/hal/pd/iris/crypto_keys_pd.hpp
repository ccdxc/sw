#ifndef __CRYPTO_KEYS_PD_H__
#define __CRYPTO_KEYS_PD_H__

namespace hal {
namespace pd {
#define CRYPTO_KEY_COUNT_MAX        (64 * 1024) /* 32K sessions x 2 */

hal_ret_t crypto_pd_init(void);

}    // namespace pd
}    // namespace hal
#endif  /* __CRYPTO_KEYS_PD_H__ */
