#include <linux/types.h>
#include <netinet/in.h>

extern "C" {

typedef __u64 u64;
typedef __u32 u32;
typedef __u16 u16;
typedef __u8 u8;

#define cpu_to_be64 htonll
#define cpu_to_be32 htonl
#define cpu_to_be16 htons
#define be64_to_cpu ntohll
#define be32_to_cpu ntohl
#define be16_to_cpu ntohs

#define cpu_to_le64
#define cpu_to_le32
#define cpu_to_le16
#define le64_to_cpu
#define le32_to_cpu
#define le16_to_cpu

enum ib_wc_status {
	IB_WC_SUCCESS,
	IB_WC_LOC_LEN_ERR,
	IB_WC_LOC_QP_OP_ERR,
	IB_WC_LOC_EEC_OP_ERR,
	IB_WC_LOC_PROT_ERR,
	IB_WC_WR_FLUSH_ERR,
	IB_WC_MW_BIND_ERR,
	IB_WC_BAD_RESP_ERR,
	IB_WC_LOC_ACCESS_ERR,
	IB_WC_REM_INV_REQ_ERR,
	IB_WC_REM_ACCESS_ERR,
	IB_WC_REM_OP_ERR,
	IB_WC_RETRY_EXC_ERR,
	IB_WC_RNR_RETRY_EXC_ERR,
	IB_WC_LOC_RDD_VIOL_ERR,
	IB_WC_REM_INV_RD_REQ_ERR,
	IB_WC_REM_ABORT_ERR,
	IB_WC_INV_EECN_ERR,
	IB_WC_INV_EEC_STATE_ERR,
	IB_WC_FATAL_ERR,
	IB_WC_RESP_TIMEOUT_ERR,
	IB_WC_GENERAL_ERR
};

enum ib_qp_type {
	/*
	 * IB_QPT_SMI and IB_QPT_GSI have to be the first two entries
	 * here (and in that order) since the MAD layer uses them as
	 * indices into a 2-entry table.
	 */
	IB_QPT_SMI,
	IB_QPT_GSI,

	IB_QPT_RC,
	IB_QPT_UC,
	IB_QPT_UD,
	IB_QPT_RAW_IPV6,
	IB_QPT_RAW_ETHERTYPE,
	IB_QPT_RAW_PACKET = 8,
	IB_QPT_XRC_INI = 9,
	IB_QPT_XRC_TGT,
	IB_QPT_MAX,
	IB_QPT_DRIVER = 0xFF,
	/* Reserve a range for qp types internal to the low level driver.
	 * These qp types will not be visible at the IB core layer, so the
	 * IB_QPT_MAX usages should not be affected in the core layer
	 */
	IB_QPT_RESERVED1 = 0x1000,
	IB_QPT_RESERVED2,
	IB_QPT_RESERVED3,
	IB_QPT_RESERVED4,
	IB_QPT_RESERVED5,
	IB_QPT_RESERVED6,
	IB_QPT_RESERVED7,
	IB_QPT_RESERVED8,
	IB_QPT_RESERVED9,
	IB_QPT_RESERVED10,
};

enum ib_qp_attr_mask {
	IB_QP_STATE			= 1,
	IB_QP_CUR_STATE			= (1<<1),
	IB_QP_EN_SQD_ASYNC_NOTIFY	= (1<<2),
	IB_QP_ACCESS_FLAGS		= (1<<3),
	IB_QP_PKEY_INDEX		= (1<<4),
	IB_QP_PORT			= (1<<5),
	IB_QP_QKEY			= (1<<6),
	IB_QP_AV			= (1<<7),
	IB_QP_PATH_MTU			= (1<<8),
	IB_QP_TIMEOUT			= (1<<9),
	IB_QP_RETRY_CNT			= (1<<10),
	IB_QP_RNR_RETRY			= (1<<11),
	IB_QP_RQ_PSN			= (1<<12),
	IB_QP_MAX_QP_RD_ATOMIC		= (1<<13),
	IB_QP_ALT_PATH			= (1<<14),
	IB_QP_MIN_RNR_TIMER		= (1<<15),
	IB_QP_SQ_PSN			= (1<<16),
	IB_QP_MAX_DEST_RD_ATOMIC	= (1<<17),
	IB_QP_PATH_MIG_STATE		= (1<<18),
	IB_QP_CAP			= (1<<19),
	IB_QP_DEST_QPN			= (1<<20),
	IB_QP_RESERVED1			= (1<<21),
	IB_QP_RESERVED2			= (1<<22),
	IB_QP_RESERVED3			= (1<<23),
	IB_QP_RESERVED4			= (1<<24),
	IB_QP_RATE_LIMIT		= (1<<25),
};

enum ib_qp_state {
	IB_QPS_RESET,
	IB_QPS_INIT,
	IB_QPS_RTR,
	IB_QPS_RTS,
	IB_QPS_SQD,
	IB_QPS_SQE,
	IB_QPS_ERR
};

enum ib_uverbs_access_flags {
	IB_UVERBS_ACCESS_LOCAL_WRITE = 1 << 0,
	IB_UVERBS_ACCESS_REMOTE_WRITE = 1 << 1,
	IB_UVERBS_ACCESS_REMOTE_READ = 1 << 2,
	IB_UVERBS_ACCESS_REMOTE_ATOMIC = 1 << 3,
	IB_UVERBS_ACCESS_MW_BIND = 1 << 4,
	IB_UVERBS_ACCESS_ZERO_BASED = 1 << 5,
	IB_UVERBS_ACCESS_ON_DEMAND = 1 << 6,
	IB_UVERBS_ACCESS_HUGETLB = 1 << 7,
};

enum ib_access_flags {
	IB_ACCESS_LOCAL_WRITE = IB_UVERBS_ACCESS_LOCAL_WRITE,
	IB_ACCESS_REMOTE_WRITE = IB_UVERBS_ACCESS_REMOTE_WRITE,
	IB_ACCESS_REMOTE_READ = IB_UVERBS_ACCESS_REMOTE_READ,
	IB_ACCESS_REMOTE_ATOMIC = IB_UVERBS_ACCESS_REMOTE_ATOMIC,
	IB_ACCESS_MW_BIND = IB_UVERBS_ACCESS_MW_BIND,
	IB_ZERO_BASED = IB_UVERBS_ACCESS_ZERO_BASED,
	IB_ACCESS_ON_DEMAND = IB_UVERBS_ACCESS_ON_DEMAND,
	IB_ACCESS_HUGETLB = IB_UVERBS_ACCESS_HUGETLB,

	IB_ACCESS_SUPPORTED = ((IB_ACCESS_HUGETLB << 1) - 1)
};

}
