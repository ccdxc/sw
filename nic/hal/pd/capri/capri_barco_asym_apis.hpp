#ifndef __CAPRI_BARCO_ASYM_APIS_H__
#define __CAPRI_BARCO_ASYM_APIS_H__

namespace hal {
namespace pd {


hal_ret_t capri_barco_asym_ecc_point_mul_p256(uint8_t *p, uint8_t *n,
        uint8_t *xg, uint8_t *yg, uint8_t *a, uint8_t *b, uint8_t *x1, uint8_t *y1,
        uint8_t *k, uint8_t *x3, uint8_t *y3);


}    // namespace pd
}    // namespace hal

#endif /* __CAPRI_BARCO_ASYM_APIS_H__ */
