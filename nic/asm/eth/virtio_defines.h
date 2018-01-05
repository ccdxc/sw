
/* Virtio TX qstats rings */
#define VIRTIO_TX_HOST_RING     0
#define VIRTIO_TX_PEND_RING     1


/* Virtio specification definitions */
/* virtq_desc definitions */
#define VIRTIO_VIRTQ_DESC_RING_ELEM_SIZE    16
#define VIRTIO_VIRTQ_DESC_RING_ELEM_SHIFT   4

/* virtq_desc.flags */
#define VIRTQ_DESC_F_NEXT           1
#define VIRTQ_DESC_F_WRITE          2
#define VIRTQ_DESC_F_INDIRECT       4

/* virtq_avail definitions */
#define VIRTQ_AVAIL_F_NO_INTERRUPT          1

#define VIRTIO_VIRTQ_AVAIL_RING_OFFSET      4
#define VIRTIO_VIRTQ_AVAIL_RING_ELEM_SIZE   2
#define VIRTIO_VIRTQ_AVAIL_RING_ELEM_SHIFT  1

/* virtq_used definitions */
#define VIRTIO_VIRTQ_USED_IDX_OFFSET        2
#define VIRTIO_VIRTQ_USED_RING_OFFSET       4
#define VIRTIO_VIRTQ_USED_RING_ELEM_SIZE    8
#define VIRTIO_VIRTQ_USED_RING_ELEM_SHIFT   3


/* virtio_net_hdr definitions */
#define VIRTIO_NET_HDR_SIZE                     12
#define VIRTIO_NET_HDR_SIZE_NO_MRG_RXBUF        10
