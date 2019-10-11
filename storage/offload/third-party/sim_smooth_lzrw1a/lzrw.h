
//
// LZR core C model
// The quirky interface matches the RW reference code, see lzrw.h for details
//
// (C) 2017 IP Cores, Inc.
//
// Rev. 1.00 - initial release
//      1.01 - fixed boolean constants
//
#ifndef LZR_MODEL
#define LZR_MODEL

#ifndef __KERNEL__
#include <stdint.h>
#endif

// Set the below to 1 (enable) or 0 (disable) to control adler32 checksum
// calculation during lzrw1a
#define LZRW1A_WITH_ADLER32_ENABLE  1

//typedef unsigned char uint8_t;
typedef unsigned char byte;
//typedef unsigned int  uint32_t;

#define COMPRESS_ACTION_COMPRESS   1
#define COMPRESS_ACTION_DECOMPRESS 2

#define TRUE	1
#define FALSE	0

#define addrBits	12

int  lzrw1a_compress(        /* Single function interface to compression algorithm. */
	uint32_t   action,       /* Action to be performed (see above)                  */
	uint8_t   *wrk_mem,      /* Scratch memory (16KB)                               */
	uint8_t   *src_adr,      /* Address of input  data.                             */
	uint32_t   src_len,      /* Length  of input  data.                             */
	uint8_t   *dst_adr,      /* Address of output data.                             */
	uint32_t  *p_dst_len,    /* Pointer to a word with length of output data        */
	uint32_t   thr,			 /* Threshold                                           */
	uint32_t  *p_adler32_chksum     /* Pointer to returned adler32 checksum        */
	);

int  orig_lzrw1a_compress(        /* Single function interface to compression algorithm. */
	uint32_t   action,       /* Action to be performed (see above)                  */
	uint8_t   *wrk_mem,      /* Scratch memory (16KB)                               */
	uint8_t   *src_adr,      /* Address of input  data.                             */
	uint32_t   src_len,      /* Length  of input  data.                             */
	uint8_t   *dst_adr,      /* Address of output data.                             */
	uint32_t  *p_dst_len,    /* Pointer to a word with length of output data        */
	uint32_t   thr			 /* Threshold                                           */
	);

int  orig_lzrw1a_compress_adler32(        /* Single function interface to compression algorithm. */
	uint32_t   action,       /* Action to be performed (see above)                  */
	uint8_t   *wrk_mem,      /* Scratch memory (16KB)                               */
	uint8_t   *src_adr,      /* Address of input  data.                             */
	uint32_t   src_len,      /* Length  of input  data.                             */
	uint8_t   *dst_adr,      /* Address of output data.                             */
	uint32_t  *p_dst_len,    /* Pointer to a word with length of output data        */
	uint32_t   thr,			 /* Threshold                                           */
	uint32_t  *p_adler32_chksum     /* Pointer to returned adler32 checksum        */
	);
#endif

