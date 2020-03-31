#ifndef __CAPRI_BARCO_RES_HPP__
#define __CAPRI_BARCO_RES_HPP__

#include "lib/indexer/indexer.hpp"
#include "lib/list/list.hpp"
#include "lib/slab/slab.hpp"

using sdk::lib::indexer;
using sdk::lib::dllist_ctxt_t;
using sdk::lib::slab;

namespace sdk {
namespace platform {
namespace capri {

extern slab *g_hal_capri_barco_pend_req_pd_slab;
extern thread_local dllist_ctxt_t g_pend_req_list;

/* Pending requests to Barco */
typedef struct crypto_pend_req_s {
    uint32_t                     hw_id;     // hw-id for the req e.g. PI/CI tag
    uint32_t                     sw_id;     // s/w id: match resp with the req e.g. qid
    dllist_ctxt_t                list_ctxt; // list context
} crypto_pend_req_t;

/* Indexer based allocator */

typedef struct capri_barco_resources_s {
    char                        allocator_name[32];
    char                        hbm_region_name[32];
    uint32_t                    obj_count;
    uint32_t                    obj_size;
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
    CRYPTO_BARCO_RES_HBM_MEM_512KB,
    CRYPTO_BARCO_RES_MAX
} capri_barco_res_type_t;

#define ASIC_HBM_REG_CRYPTO_ASYM_DMA_DESCR "crypto-asym-dma-descr"
#define ASIC_HBM_REG_CRYPTO_HBM_MEM     "crypto-hbm-mem"
#define ASIC_HBM_REG_ASYM_KEY_DESCR     "crypto-asym-key-desc-array"
#define ASIC_HBM_REG_CRYPTO_SYM_MSG_DESCR "crypto-sym-msg-descr"
#define ASIC_HBM_REG_CRYPTO_HBM_MEM_BIG  "crypto-hbm-mem-big"

#define CRYPTO_ASYM_DMA_DESCR_COUNT_MAX 1024
#define CRYPTO_SYM_MSG_DESCR_COUNT_MAX  1024
#define CRYPTO_HBM_MEM_COUNT_MAX        1024
#define CRYPTO_HBM_MEM_BIG_COUNT_MAX    10
#define CRYPTO_ASYM_KEY_DESCR_COUNT_MAX (64 * 1024)
#define CRYPTO_BARCO_RES_HBM_MEM_512B_SIZE 512

sdk_ret_t capri_barco_res_alloc(capri_barco_res_type_t res_type,
                                int32_t *res_id, uint64_t *res);
sdk_ret_t capri_barco_res_free(capri_barco_res_type_t res_type, uint64_t res);
sdk_ret_t capri_barco_res_free_by_id(capri_barco_res_type_t res_type,
                                     int32_t res_id);
sdk_ret_t capri_barco_res_get_by_id(capri_barco_res_type_t region_type,
                                    int32_t res_id, uint64_t *res);
sdk_ret_t capri_barco_res_region_get(capri_barco_res_type_t region_type,
                                     uint64_t *region);
sdk_ret_t capri_barco_res_region_size_get(capri_barco_res_type_t region_type,
                                          uint16_t *region_size);
sdk_ret_t capri_barco_res_obj_count_get(capri_barco_res_type_t region_type,
                                        uint32_t *obj_count);

sdk_ret_t capri_barco_add_pend_req_to_db(uint32_t hw_id, uint32_t sw_id);
sdk_ret_t capri_barco_del_pend_req_from_db(crypto_pend_req_t *req);

}    // namespace capri
}    // namespace platform
}    // namespace sdk

#endif  /* __CAPRI_BARCO_RES_HPP__ */
