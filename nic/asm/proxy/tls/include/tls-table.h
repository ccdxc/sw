#ifndef _TLS_TABLE_H_
#define _TLS_TABLE_H_

#define TLS_TCB_TABLE_BASE              0xbeef0000

#define TLS_TCB_TABLE_ENTRY_SIZE        512 /* 512B */
#define TLS_TCB_TABLE_ENTRY_SIZE_SHFT   9  /* 512B */
#define TLS_TCB_OFFSET                  0
#define TLS_TCB_CRYPT_OFFSET            64


/* Global registers */


#define tls_alloc_brq_pi_process       0x00020000
#define tls_alloc_rnmdr_process        0x00020400
#define tls_alloc_rnmpr_process        0x00020800
#define tls_alloc_sesq_pi_process      0x00020c00
#define tls_brq_completion_process     0x00030000
#define tls_rx_brq_process             0x00030400
#define tls_queue_brq_process          0x00030800
#define tls_queue_sesq_process         0x00030c00

#define tls_rx_serq_process            0x00040400
#define tls_read_header_process        0x00040800
#define tls_serq_read_process          0x00040c00
#define tls_alloc_rnmdr_split_process  0x00050000
#define tls_update_page_ctl_process    0x00050400




#endif /* #ifndef _TLS_TABLE_H_ */
