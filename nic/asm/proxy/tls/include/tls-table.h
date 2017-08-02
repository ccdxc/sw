#ifndef _TLS_TABLE_H_
#define _TLS_TABLE_H_

#define TLS_TCB_TABLE_BASE              0xbeef0000

#define TLS_TCB_TABLE_ENTRY_SIZE        512 /* 512B */
#define TLS_TCB_TABLE_ENTRY_SIZE_SHFT   9  /* 512B */
#define TLS_TCB_OFFSET                  0

#define TNMDR_TABLE_BASE                0xbeef1000
#define TNMDR_TABLE_ENTRY_SIZE          8 /* 8B */
#define TNMDR_TABLE_ENTRY_SIZE_SHFT     3 /* 8B */
#define TNMDR_TABLE_SIZE                255
#define TNMDR_TABLE_SIZE_SHFT           8

#define RNMDR_TABLE_BASE                0xbeef2000 
#define RNMDR_TABLE_ENTRY_SIZE          8 /* 8B */
#define RNMDR_TABLE_ENTRY_SIZE_SHFT     3 /* 8B */
#define RNMDR_TABLE_SIZE                255
#define RNMDR_TABLE_SIZE_SHFT           8

#define RNMPR_TABLE_BASE                0xbeef3000 
#define RNMPR_TABLE_ENTRY_SIZE          8 /* 8B */
#define RNMPR_TABLE_ENTRY_SIZE_SHFT     3 /* 8B */
#define RNMPR_TABLE_SIZE                255
#define RNMPR_TABLE_SIZE_SHFT           8

/* Global registers */
#define SERQ_BASE                      0xbeef4000
#define SERQ_PRODUCER_IDX              0xbababa00
#define SERQ_CONSUMER_IDX              0xbababa08
#define SERQ_TABLE_SIZE                255
#define SERQ_TABLE_SIZE_SHFT           8

#define SESQ_BASE                      0xbeef5000
#define SESQ_PRODUCER_IDX              0xbababa10
#define SESQ_CONSUMER_IDX              0xbababa18
#define SESQ_TABLE_SIZE                255
#define SESQ_TABLE_SIZE_SHFT           8

#define RNMPR_ALLOC_IDX                0xbababa20
#define RNMPR_FREE_IDX                 0xbababa28
#define RNMDR_ALLOC_IDX                0xbababa30
#define RNMDR_FREE_IDX                 0xbababa38

#define TNMPR_ALLOC_IDX                0xbababa40
#define TNMPR_FREE_IDX                 0xbababa48
#define TNMDR_ALLOC_IDX                0xbababa50
#define TNMDR_FREE_IDX                 0xbababa58

#define ARQ_PRODUCER_IDX               0xbababa60
#define ARQ_CONSUMER_IDX               0xbababa68

#define ASQ_PRODUCER_IDX               0xbababa70
#define ASQ_CONSUMER_IDX               0xbababa78

#define BRQ_BASE                       0xbeef6000
#define BRQ_PRODUCER_IDX               0xbababa80
#define BRQ_CONSUMER_IDX               0xbababa88
#define BRQ_TABLE_SIZE                 255
#define BRQ_TABLE_SIZE_SHFT            8

#define TABLE_TYPE_RAW                 0

#define tls_alloc_brq_pi_process       0x00020000
#define tls_alloc_rnmdr_process        0x00020400
#define tls_alloc_rnmpr_process        0x00020800
#define tls_alloc_sesq_pi_process      0x00020c00
#define tls_brq_completion_process     0x00030000
#define tls_rx_brq_process             0x00030400
#define tls_queue_brq_process          0x00030800
#define tls_queue_sesq_process         0x00030c00
#define tls_read_desc_process          0x00040000
#define tls_rx_serq_process            0x00040400
#define tls_read_header_process        0x00040800
#define tls_serq_read_process          0x00040c00
#define tls_alloc_rnmdr_split_process  0x00050000
#define tls_update_page_ctl_process    0x00050400
#define tls_serq_consume_process       0x00050800



#endif /* #ifndef _TLS_TABLE_H_ */
