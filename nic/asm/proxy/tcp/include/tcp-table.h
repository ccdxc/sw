// NOTE : ** These offsets need to match the offsets in 
// p4pd_tcp_proxy_api.h **
#define TCP_TCB_RX2TX_SHARED_OFFSET     0

#define TCP_TCB_RX2TX_TX_CI_OFFSET \
                    (TCP_TCB_RX2TX_SHARED_OFFSET + 40)
#define TCP_TCB_RX2TX_RETX_CI_OFFSET \
                    (TCP_TCB_RX2TX_SHARED_OFFSET + 42)
#define TCP_TCB_RX2TX_RETX_PENDING_OFFSET \
                    (TCP_TCB_RX2TX_SHARED_OFFSET + 46)
#define TCP_TCB_RX2TX_ATO_OFFSET \
                    (TCP_TCB_RX2TX_SHARED_OFFSET + 48)
#define TCP_TCB_RX2TX_RTO_OFFSET \
                    (TCP_TCB_RX2TX_SHARED_OFFSET + 50)
#define TCP_TCB_RX2TX_IDLE_TO_OFFSET \
                    (TCP_TCB_RX2TX_SHARED_OFFSET + 52)

#define TCP_TCB_TX2RX_SHARED_OFFSET     64
#define TCP_TCB_TX2RX_SHARED_WRITE_OFFSET \
                    (TCP_TCB_TX2RX_SHARED_OFFSET + 32)  // skip intrinsic part etc

#define TCP_TCB_RX2TX_SHARED_EXTRA_OFFSET     128
#define TCP_TCB_RX2TX_EXTRA_SND_CWND_OFFSET \
                    (TCP_TCB_RX2TX_SHARED_EXTRA_OFFSET + 0)

#define TCP_TCB_RX_OFFSET                     192
#define TCP_TCB_RX_OOQ_NOT_EMPTY \
                    (TCP_TCB_RX_OFFSET + 0)
#define TCP_TCB_RTT_OFFSET                    256

#define TCP_TCB_CC_OFFSET                     320
#define TCP_TCB_CC_SND_CWND_OFFSET \
                    (TCP_TCB_CC_OFFSET + 0)

#define TCP_TCB_RETX_OFFSET                   384
#define TCP_TCB_FC_OFFSET                     448
#define TCP_TCB_RX_DMA_OFFSET                 512
#define TCP_TCB_XMIT_OFFSET                   576
#define TCP_TCB_TSO_OFFSET                    640
#define TCP_TCB_HEADER_TEMPLATE_OFFSET        704
#define TCP_TCB_UNUSED1                       768
#define TCP_TCB_UNUSED2                       832
#define TCP_TCB_OOO_BOOK_KEEPING_OFFSET0      896

#define TCP_TCB_OOO_QADDR_OFFSET              960
// needs to match offset in p4pd_tcp_proxy_api.h
#define TCP_TCB_OOO_QADDR_CI_OFFSET           (TCP_TCB_OOO_QADDR_OFFSET + 0)
