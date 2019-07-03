#ifndef __VIRTIO_IF_HPP__
#define __VIRTIO_IF_HPP__

#define VIRTIO_DEV_PAGE_SIZE             4096
#define VIRTIO_DEV_PAGE_MASK             (VIRTIO_DEV_PAGE_SIZE - 1)

#define VIRTIO_DEV_ADDR_ALIGN(addr, sz)  \
    (((addr) + ((uint64_t)(sz) - 1)) & ~((uint64_t)(sz) - 1))

#define VIRTIO_DEV_PAGE_ALIGN(addr)      \
    VIRTIO_DEV_ADDR_ALIGN(addr, VIRTIO_DEV_PAGE_SIZE)

/* Supply these for virtio_dev_if.h */
#define dma_addr_t uint64_t

#include "nic/include/virtio_dev_if.h"

#pragma pack(push, 1)

/**
 * VIRTIO PF Devcmd Region
 */

/* 4.1.4.3 - common configuration structure layout */
typedef struct virtio_pci_common_cfg {
    struct {
    /* About the whole device, and queue select. (24 bytes) */
    uint32_t            device_feature_select;
    uint32_t            device_feature;
    uint32_t            driver_feature_select;
    uint32_t            driver_feature;
    uint16_t            config_msix_vector;
    uint16_t            num_queues;
    uint8_t             device_status;
    uint8_t             config_generation;
    uint16_t            queue_select;

    /* About the selected virtqueue. (32 bytes) */
    uint16_t            queue_size;
    uint16_t            queue_msix_vector;
    uint16_t            queue_enable;
    uint16_t            queue_notify_off;
    uint64_t            queue_desc;
    uint64_t            queue_driver;
    uint64_t            queue_device;
    } cfg[2]; /* XXX duplicate config instead of select registers */

    uint8_t             rsvd[VIRTIO_DEV_PAGE_SIZE - 2 * (24 + 32)];
} virtio_dev_cmd_regs_t;

/* 2.1 - device status field */
enum {
    VIRTIO_S_ACKNOWLEDGE                = (1u << 0),
    VIRTIO_S_DRIVER                     = (1u << 1),
    VIRTIO_S_DRIVER_OK                  = (1u << 2),
    VIRTIO_S_FEATURES_OK                = (1u << 3),
    VIRTIO_S_NEEDS_RESET                = (1u << 6),
    VIRTIO_S_FAILED                     = (1u << 7),
};

/* 5.1.3 - network device - feature bits */
enum {
    VIRTIO_NET_F_CSUM                   = (1ull << 0),
    VIRTIO_NET_F_GUEST_CSUM             = (1ull << 1),
    VIRTIO_NET_F_CTRL_GUEST_OFFLOADS    = (1ull << 2),
    VIRTIO_NET_F_MTU                    = (1ull << 3),
    VIRTIO_NET_F_MAC                    = (1ull << 5),
    VIRTIO_NET_F_GSO                    = (1ull << 6),
    VIRTIO_NET_F_GUEST_TSO4             = (1ull << 7),
    VIRTIO_NET_F_GUEST_TSO6             = (1ull << 8),
    VIRTIO_NET_F_GUEST_ECN              = (1ull << 9),
    VIRTIO_NET_F_GUEST_UFO              = (1ull << 10),
    VIRTIO_NET_F_HOST_TSO4              = (1ull << 11),
    VIRTIO_NET_F_HOST_TSO6              = (1ull << 12),
    VIRTIO_NET_F_HOST_ECN               = (1ull << 13),
    VIRTIO_NET_F_HOST_UFO               = (1ull << 14),
    VIRTIO_NET_F_MRG_RXBUF              = (1ull << 15),
    VIRTIO_NET_F_STATUS                 = (1ull << 16),
    VIRTIO_NET_F_CTRL_VQ                = (1ull << 17),
    VIRTIO_NET_F_CTRL_RX                = (1ull << 18),
    VIRTIO_NET_F_CTRL_VLAN              = (1ull << 19),
    VIRTIO_NET_F_CTRL_RX_EXTRA          = (1ull << 20),
    VIRTIO_NET_F_GUEST_ANNOUNCE         = (1ull << 21),
    VIRTIO_NET_F_MQ                     = (1ull << 22),
    VIRTIO_NET_F_CTRL_MAC_ADDR          = (1ull << 23),
    VIRTIO_NET_F_STANDBY                = (1ull << 62),
    VIRTIO_NET_F_SPEED_DUPLEX           = (1ull << 63),
};

/* 6 - reserved feature bits */
enum {
    VIRTIO_F_NOTIFY_ON_EMPTY            = (1ull << 24),
    VIRTIO_F_ANY_LAYOUT                 = (1ull << 27),
    VIRTIO_F_RING_INDIRECT_DESC         = (1ull << 28),
    VIRTIO_F_RING_EVENT_IDX             = (1ull << 29),
    VIRTIO_F_UNUSED                     = (1ull << 30),
    VIRTIO_F_VERSION_1                  = (1ull << 32),
    VIRTIO_F_ACCESS_PLATFORM            = (1ull << 33),
    VIRTIO_F_RING_PACKED                = (1ull << 34),
    VIRTIO_F_IN_ORDER                   = (1ull << 35),
    VIRTIO_F_ORDER_PLATFORM             = (1ull << 36),
    VIRTIO_F_SR_IOV                     = (1ull << 37),
    VIRTIO_F_NOTIFICATION_DATA          = (1ull << 38),
};

static_assert(sizeof(virtio_dev_cmd_regs_t) == VIRTIO_DEV_PAGE_SIZE);

/**
 * VIRTIO QState
 */

typedef struct virtio_qstate_common_s {
    uint8_t     pc_offset;
    uint8_t     rsvd0;
    uint8_t     cosA : 4;
    uint8_t     cosB : 4;
    uint8_t     cos_sel;
    uint8_t     eval_last;
    uint8_t     host : 4;
    uint8_t     total : 4;
    uint16_t    pid;
} virtio_qstate_common_t;

#define VIRTIO_RX_RINGS 1

typedef struct virtio_qstate_rx_s {
    virtio_qstate_common_t qs;
    struct {
        uint16_t    pi;
        uint16_t    ci;
    } ring[VIRTIO_RX_RINGS];

    uint64_t    features;
    uint64_t    rx_virtq_desc_addr;
    uint64_t    rx_virtq_avail_addr;
    uint64_t    rx_virtq_used_addr;
    uint32_t    rx_intr_assert_addr;
    uint16_t    rx_queue_size_mask;

    uint16_t    rx_virtq_avail_ci;
    uint16_t    rx_virtq_used_pi;
} __attribute__((packed)) virtio_qstate_rx_t;

#define VIRTIO_TX_RINGS 2

typedef struct virtio_qstate_tx_s {
    virtio_qstate_common_t qs;
    struct {
        uint16_t    pi;
        uint16_t    ci;
    } ring[VIRTIO_TX_RINGS];

    uint64_t    features;
    uint64_t    tx_virtq_desc_addr;
    uint64_t    tx_virtq_avail_addr;
    uint64_t    tx_virtq_used_addr;
    uint32_t    tx_intr_assert_addr;
    uint16_t    tx_queue_size_mask;

    uint16_t    tx_virtq_avail_ci;
    uint16_t    tx_virtq_used_pi;
} __attribute__((packed)) virtio_qstate_tx_t;

#pragma pack(pop)

#endif //__VIRTIO_IF_HPP__
