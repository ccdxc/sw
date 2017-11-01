#ifndef __CAPRI_BARCO_RES_HPP__
#define __CAPRI_BARCO_RES_HPP__

namespace hal {
namespace pd {

/* Indexer based allocator */

typedef struct capri_barco_resources_s {
    char                        allocator_name[32];
    char                        hbm_region_name[32];
    uint32_t                    obj_count;
    uint16_t                    obj_size;
    uint16_t                    obj_alignment;
    uint64_t                    hbm_region;
    uint32_t                    hbm_region_size;
    indexer                     *idxer;
} capri_barco_resources_t;

typedef enum capri_barco_res_type_e {
    CRYPTO_BARCO_RES_MIN = 0,
    CRYPTO_BARCO_RES_ASYM_DMA_DESCR = CRYPTO_BARCO_RES_MIN,
    CRYPTO_BARCO_RES_HBM_MEM_512B,
    CRYPTO_BARCO_RES_ASYM_KEY_DESCR,
    CRYPTO_BARCO_RES_SYM_MSG_DESCR,
    CRYPTO_BARCO_RES_MAX
} capri_barco_res_type_t;


#define CRYPTO_ASYM_DMA_DESCR_COUNT_MAX 1024
#define CRYPTO_SYM_MSG_DESCR_COUNT_MAX  1024
#define CRYPTO_HBM_MEM_COUNT_MAX        1024
#define CRYPTO_ASYM_KEY_DESCR_COUNT_MAX (64 * 1024)

hal_ret_t capri_barco_res_alloc(capri_barco_res_type_t res_type,
        int32_t *res_id, uint64_t *res);
hal_ret_t capri_barco_res_free(capri_barco_res_type_t res_type, uint64_t res);
hal_ret_t capri_barco_res_free_by_id(capri_barco_res_type_t res_type,
        int32_t res_id);
hal_ret_t capri_barco_res_get_by_id(capri_barco_res_type_t region_type,
        int32_t res_id, uint64_t *res);
hal_ret_t capri_barco_res_region_get(capri_barco_res_type_t region_type,
        uint64_t *region);
hal_ret_t capri_barco_res_region_size_get(capri_barco_res_type_t region_type,
        uint16_t *region_size);
hal_ret_t capri_barco_res_obj_count_get(capri_barco_res_type_t region_type,
        uint32_t *obj_count);

}    // namespace pd
}    // namespace hal

#endif  /* __CAPRI_BARCO_RES_HPP__ */
