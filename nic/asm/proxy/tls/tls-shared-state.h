/* 
 * This contains all the fields shared across TX and RX pipeline stages for TLS.
 * Each field has a unique writing stage
 */

#define NTLS_AES_GCM_128_IV_WIDTH       64
#define NTLS_AES_GCM_128_KEY_WIDTH      128
#define NTLS_AES_GCM_128_SALT_WIDTH     32
#define NTLS_CIPHER_AES_GCM_128         51
#define NTLS_TLS_1_2_MAJOR              0x03
#define NTLS_TLS_1_2_MINOR              0x03

#define NTLS_IV_WIDTH                   NTLS_AES_GCM_128_IV_WIDTH
#define NTLS_KEY_WIDTH                  NTLS_AES_GCM_128_KEY_WIDTH
#define NTLS_SALT_WIDTH                 NTLS_AES_GCM_128_SALT_WIDTH

#define SEQ_NUMBER_WIDTH                7
#define TS_WIDTH                        8
#define RING_INDEX_WIDTH                8
#define ADDRESS_WIDTH                   12
#define OFFSET_WIDTH                    8
#define LEN_WIDTH                       8
#define COUNTER32                       4
#define COUNTER16                       16
#define COUNTER8                        8
#define WINDOW_WIDTH                    8
#define MTU_WIDTH                       8

#define TLS_SHARED_STATE \
        cipher_type                     : 8                     ;\
        ver_major			: 4	                ;\
	ver_minor			: 4	                ;\
	key_addr		        : ADDRESS_WIDTH         ;\
	iv_addr		                : ADDRESS_WIDTH         ;\
	qhead			        : ADDRESS_WIDTH         ;\
	qtail			        : ADDRESS_WIDTH         ;\
	una_desc			: ADDRESS_WIDTH         ;\
	una_desc_idx			: 2                     ;\
	una_data_offset			: 16                    ;\
	una_data_len			: 16                    ;\
	nxt_desc			: ADDRESS_WIDTH         ;\
	nxt_desc_idx			: 2                     ;\
	nxt_data_offset			: 16                    ;\
	nxt_data_len			: 16                    ;\
       	next_tls_hdr_offset		: 16                    ;\
       	cur_tls_data_len		: 16                    ;\
        ofid		                : 16                    ;

