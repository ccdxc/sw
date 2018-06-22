/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#ifndef __PNSO_TEST_H__
#define __PNSO_TEST_H__

#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include "pnso_api.h"

#define TEST_OUTPUT_FLAG_APPEND 0x01

/* Must match function prototype of pnso_submit_request */
typedef pnso_error_t (*pnso_submit_req_fn_t)(
		struct pnso_service_request *svc_req,
		struct pnso_service_result *svc_res,
		completion_cb_t cb,
		void *cb_ctx,
		pnso_poll_fn_t *poll_fn,
		void **poll_ctx);

typedef void *(*pnso_alloc_fn_t)(size_t sz);
typedef void (*pnso_dealloc_fn_t)(void *ptr);
typedef void *(*pnso_realloc_fn_t)(void *ptr, size_t sz);

void pnso_test_init_fns(pnso_submit_req_fn_t submit,
			pnso_alloc_fn_t alloc,
			pnso_dealloc_fn_t dealloc,
			pnso_realloc_fn_t realloc);
void *pnso_test_parse_file(const char *fname);
void *pnso_test_parse_buf(const unsigned char *buf, size_t buf_len);
void pnso_test_parse_free(void *cfg);
pnso_error_t pnso_test_run_all(void *cfg);

/* OSAL functions */
uint32_t test_file_size(const char *fname);
uint8_t *test_alloc_and_read_file(const char *fname, uint32_t foffset, uint32_t *len);
pnso_error_t test_read_file(const char *fname,
			    struct pnso_buffer_list *buflist,
			    uint32_t foffset, uint32_t flen);
pnso_error_t test_write_file(const char *fname,
			     const struct pnso_buffer_list *buflist,
			     uint32_t flen, uint32_t flags);
pnso_error_t test_fill_random(struct pnso_buffer_list *buflist, uint32_t seed);
int test_compare_files(const char *path1, const char *path2, uint32_t offset, uint32_t len);

void *pnso_test_alloc(size_t sz);
void pnso_test_free(void *ptr);
#define TEST_ALLOC pnso_test_alloc
#define TEST_FREE pnso_test_free



/* TODO */
#define PNSO_LOG_ERROR printf
#define PNSO_LOG_WARN  printf
#define PNSO_LOG_INFO  printf
#define PNSO_LOG_DEBUG printf


#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* __PNSO_TEST_H__ */
