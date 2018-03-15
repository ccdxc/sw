#include "../../p4/iris/include/defines.h"

#define ASSERT_PHVWR(d, d_start, d_end, s, s_start, s_end) \
    .assert((offsetof(d, d_start) - offsetof(d, d_end)) == \
            (offsetof(s, s_start) - offsetof(s, s_end)))

/*****************************************************************************/
/* Defines for writing into a DBG_WR trace for assembly. This is disabled by */
/* default, to enable write to DBG_HBM_EN                                    */
/*****************************************************************************/
#define DBG_HBM_EN      0 
#define DBG_HBM_BASE    0x0C0000000
#define DBG_HBM_COUNT   0x200

#if DBG_HBM_EN
#define DBG_WR(index, val) \
    .assert((index) < DBG_HBM_COUNT); \
    addui    r7, r0, (DBG_HBM_BASE + ((index) << 3)) >> 32; \
    addi     r7, r7, (DBG_HBM_BASE + ((index) << 3)) & 0xfffffff8; \
    add      r6, r0, val; \
    memwr.dx r7, r6; \

#else
#define DBG_WR(index, val) ;
#endif

#define K_DBG_WR(index) \
  DBG_WR(index + 0, k[511:448]); \
  DBG_WR(index + 1, k[447:384]); \
  DBG_WR(index + 2, k[383:320]); \
  DBG_WR(index + 3, k[319:256]); \
  DBG_WR(index + 4, k[255:192]); \
  DBG_WR(index + 5, k[191:128]); \
  DBG_WR(index + 6, k[127:64]); \
  DBG_WR(index + 7, k[63:0]); \


