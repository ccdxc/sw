#ifndef _TLS_TABLE_H_
#define _TLS_TABLE_H_

#define TLS_TCB_TABLE_BASE              0xbeef0000

#define TLS_TCB_TABLE_ENTRY_SIZE        1024 /* 1024B */
#define TLS_TCB_TABLE_ENTRY_SIZE_SHFT   10  /* 1024B */

#define TLS_TCB_OFFSET                   0
#define TLS_TCB_CRYPT_OFFSET             64
#define TLS_TCB_PRE_CRYPTO_STATS_OFFSET  128
#define TLS_TCB_POST_CRYPTO_STATS_OFFSET 320
#define TLS_TCB_CONFIG                   512
#define TLS_TCB_OPER_DATA_OFFSET         576

#define tls_alloc_rnmdr_process        0x00020400
#define tls_read_header_process        0x00040800
#define tls_alloc_rnmdr_split_process  0x00050000
#define tls_update_page_ctl_process    0x00050400


#endif /* #ifndef _TLS_TABLE_H_ */
