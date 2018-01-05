#ifndef _VIRTIO_DEV_REGS_H_
#define _VIRTIO_DEV_REGS_H_

/*  Supporting only legacy mode for now */
#define VIRTIO_PCIE_BAR_MAX                             6

#define VIRTIO_LEGACY_REG_OFFSET_MIN                    0   /* device_features      */
#define VIRTIO_LEGACY_REG_OFFSET_DEVICE_FEATURES        VIRTIO_LEGACY_REG_OFFSET_MIN
#define VIRTIO_LEGACY_REG_SIZE_DEVICE_FEATURES          4

#define VIRTIO_LEGACY_REG_OFFSET_DRIVER_FEATURES        (VIRTIO_LEGACY_REG_OFFSET_DEVICE_FEATURES + \
                                                            VIRTIO_LEGACY_REG_SIZE_DEVICE_FEATURES)
#define VIRTIO_LEGACY_REG_SIZE_DRIVER_FEATURES          4

#define VIRTIO_LEGACY_REG_OFFSET_QUEUE_ADDRESS          (VIRTIO_LEGACY_REG_OFFSET_DRIVER_FEATURES + \
                                                            VIRTIO_LEGACY_REG_SIZE_DRIVER_FEATURES)
#define VIRTIO_LEGACY_REG_SIZE_QUEUE_ADDRESS            4

#define VIRTIO_LEGACY_REG_OFFSET_QUEUE_SIZE             (VIRTIO_LEGACY_REG_OFFSET_QUEUE_ADDRESS + \
                                                            VIRTIO_LEGACY_REG_SIZE_QUEUE_ADDRESS)
#define VIRTIO_LEGACY_REG_SIZE_QUEUE_SIZE               2


#define VIRTIO_LEGACY_REG_OFFSET_QUEUE_SELECT           (VIRTIO_LEGACY_REG_OFFSET_QUEUE_SIZE + \
                                                            VIRTIO_LEGACY_REG_SIZE_QUEUE_SIZE)
#define VIRTIO_LEGACY_REG_SIZE_QUEUE_SELECT             2


#define VIRTIO_LEGACY_REG_OFFSET_QUEUE_NOTIFY           (VIRTIO_LEGACY_REG_OFFSET_QUEUE_SELECT + \
                                                            VIRTIO_LEGACY_REG_SIZE_QUEUE_SELECT)
#define VIRTIO_LEGACY_REG_SIZE_QUEUE_NOTIFY             2


#define VIRTIO_LEGACY_REG_OFFSET_DEVICE_STATUS          (VIRTIO_LEGACY_REG_OFFSET_QUEUE_NOTIFY + \
                                                            VIRTIO_LEGACY_REG_SIZE_QUEUE_NOTIFY)
#define VIRTIO_LEGACY_REG_SIZE_DEVICE_STATUS            1


#define VIRTIO_LEGACY_REG_OFFSET_ISR_STATUS             (VIRTIO_LEGACY_REG_OFFSET_DEVICE_STATUS + \
                                                            VIRTIO_LEGACY_REG_SIZE_DEVICE_STATUS)
#define VIRTIO_LEGACY_REG_SIZE_ISR_STATUS               1


#define VIRTIO_LEGACY_REG_OFFSET_CONFIG_MSIX_VECTOR     (VIRTIO_LEGACY_REG_OFFSET_ISR_STATUS + \
                                                            VIRTIO_LEGACY_REG_SIZE_ISR_STATUS)
#define VIRTIO_LEGACY_REG_SIZE_CONFIG_MSIX_VECTOR       2


#define VIRTIO_LEGACY_REG_OFFSET_QUEUE_MSIX_VECTOR      (VIRTIO_LEGACY_REG_OFFSET_CONFIG_MSIX_VECTOR + \
                                                            VIRTIO_LEGACY_REG_SIZE_CONFIG_MSIX_VECTOR)
#define VIRTIO_LEGACY_REG_SIZE_QUEUE_MSIX_VECTOR        2

#define VIRTIO_LEGACY_REG_OFFSET_MAX                    22  /* queue_msix_vector    */

#if VIRTIO_LEGACY_REG_OFFSET_QUEUE_MSIX_VECTOR != VIRTIO_LEGACY_REG_OFFSET_MAX
#error "Invalid Offset for register: Queue MSIX Vector"
#endif


typedef struct virtq_desc_s {
    uint64_t            addr;
    uint32_t            len;
    uint16_t            flags;
    uint16_t            nextid;
} __attribute__((packed)) virtq_desc_t;

typedef struct virtq_avail_s {
    uint16_t            flags;
    uint16_t            idx;
    uint16_t            ring[0];
} __attribute__((packed)) virtq_avail_t;

#define VIRTIO_PCI_VRING_ALIGN                          4096
#define VIRTIO_ALIGN_DOWN(n, m) ((n) / (m) * (m))
#define VIRTIO_ALIGN_UP(n, m) VIRTIO_ALIGN_DOWN((n) + (m) - 1, (m))

#define VIRTIO_TX_TYPE_RING_HOST                        0
#define VIRTIO_TX_TYPE_RING_PENDING_WORK                1

#endif  /* _VIRTIO_DEV_REGS_H_ */
