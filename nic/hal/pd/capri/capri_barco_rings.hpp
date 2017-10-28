#ifndef __CAPRI_BARCO_RINGS_H__
#define __CAPRI_BARCO_RINGS_H__

namespace hal {
namespace pd {
/* Asymmetric engine related definitions */

typedef struct barco_asym_descriptor_s {
    uint64_t                input_list_addr;
    uint64_t                output_list_addr;
    uint8_t                 key_descr_idx_lo;
    uint16_t                key_descr_idx_hi;
    uint64_t                status_addr;
    uint32_t                opaque_tag_value;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    uint8_t                 opage_tag_wr_en:1;
    uint8_t                 flag_a:1;
    uint8_t                 flag_b:1;
    uint8_t                 rsvd:5;
#else
    uint8_t                 rsvd:5;
    uint8_t                 flag_b:1;
    uint8_t                 flag_a:1;
    uint8_t                 opage_tag_wr_en:1;
#endif
} __attribute__((__packed__)) barco_asym_descriptor_t;



typedef struct barco_asym_dma_descriptor_s {
    uint64_t                address;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    uint64_t                stop:1;
    uint64_t                rsvd0:1;
    uint64_t                next:62;

    uint32_t                int_en:1;
    uint32_t                discard:1;
    uint32_t                realign:1;
    uint32_t                cst_addr:1;
    uint32_t                length:28;
#else
    uint32_t                length:28;
    uint32_t                cst_addr:1;
    uint32_t                realign:1;
    uint32_t                discard:1;
    uint32_t                int_en:1;

    uint64_t                next:62;
    uint64_t                rsvd0:1;
    uint64_t                stop:1;
#endif
    uint32_t                tag;
} __attribute__((__packed__)) barco_asym_dma_descriptor_t;


/* Barco Asym DMA Descriptor allocator */

hal_ret_t pd_crypto_asym_dma_descr_alloc(uint64_t *asym_dma_descr);
hal_ret_t pd_crypto_asym_dma_descr_free(uint64_t asym_dma_descr);
hal_ret_t pd_crypto_asym_dma_descr_init(void);


typedef hal_ret_t (*barco_ring_init_t) (struct capri_barco_ring_s *);
typedef bool (*barco_ring_poller_t) (struct capri_barco_ring_s *);
typedef hal_ret_t (*barco_ring_queue_request) (struct capri_barco_ring_s *, void *);

typedef struct capri_barco_ring_s {
    char                ring_name[32];      /*  Friendly name for logging       */
    char                hbm_region[32];     /*  HBM region name                 */
    uint64_t            ring_base;          /*  Address to the ring memory      */
    uint64_t            ring_alignment;     /*  Alignment of the ring elements  */
    uint16_t            ring_size;          /*  number of entries               */
    uint16_t            descriptor_size;    /*  in bytes                        */
    uint16_t            producer_idx;       /*  S/W Write Ptr                   */
    uint16_t            consumer_idx;       /*  S/W Read ptr                    */
    /* TBD lock/spinlock for ring access */
    barco_ring_init_t   init;               /* Ring initialization fn           */
    barco_ring_poller_t poller;             /* Ring poller fn                   */
    barco_ring_queue_request    queue_request;  /* Request queue support        */
} capri_barco_ring_t;

//#define BARCO_RING_XTS_STR(id)  "Barco XTS" ## #id
#define BARCO_RING_GCM_STR(id)  "Barco GCM" ## #id
#define BARCO_RING_MPP_STR(id)  "Barco MPP" ## #id
#define BARCO_RING_ASYM_STR     "Barco Asym"
#define BARCO_RING_XTS_STR      "Barco XTS"


hal_ret_t capri_barco_ring_queue_request(types::BarcoRings barco_ring_type, void *req);
bool capri_barco_ring_poll(types::BarcoRings barco_ring_type);


}    // namespace pd
}    // namespace hal

#endif /* __CAPRI_BARCO_RINGS_H__ */
