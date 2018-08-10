/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_GLOBAL_UT_H__
#define __PNSO_GLOBAL_UT_H__

/*
 * NOTE:
 * 	Some or most of the UTs may appear to be repeatitive with minor changes
 * 	and thereby the functions may be long, and this is deliberate for the
 * 	following reasons:
 * 		(a) to keep the relevant pieces of a UT within the vicinity
 * 		(b) to keep up the readability
 * 		(c) to ease the troubleshooting
 *
 * 	TODO-chksum_ut:
 * 	TODO-hash_ut:
 * 		- make PNSO_BLOCK_SIZE visible via config-get, when pnso_chain.c
 * 		comes into play
 * 		- while code optimization is not the concern for UT, reduce the
 * 		possible duplicates
 */
#define PNSO_BLOCK_SIZE		4096
#define PNSO_BUFFER_LEN		(32 * 1024)

#endif /* __PNSO_GLOBAL_UT_H__ */
