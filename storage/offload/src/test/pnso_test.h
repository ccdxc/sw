/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#ifndef __PNSO_TEST_H__
#define __PNSO_TEST_H__

#include "pnso_api.h"
#include "osal_logger.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define TEST_OUTPUT_FLAG_APPEND 0x01
#define TEST_OUTPUT_FLAG_TINY   0x02
#define TEST_OUTPUT_FLAG_JUMBO  0x04

#define TEST_MAX_RANDOM_LEN 32768

enum {
	PENCAKE_STATE_INIT,
	PENCAKE_STATE_CFG,
	PENCAKE_STATE_RUNNING,
	PENCAKE_STATE_IDLE,
	PENCAKE_STATE_SHUTDOWN,

	PENCAKE_STATE_MAX
};

/* Full definition in pnso_test_parse.h */
struct test_desc;

/* Must match function prototype of pnso_submit_request */
typedef pnso_error_t (*pnso_submit_req_fn_t)(
		struct pnso_service_request *svc_req,
		struct pnso_service_result *svc_res,
		completion_cb_t cb,
		void *cb_ctx,
		pnso_poll_fn_t *poll_fn,
		void **poll_ctx);

typedef void (*pnso_output_fn_t)(const char *str, void *opaque);
typedef void *(*pnso_alloc_fn_t)(size_t sz);
typedef void (*pnso_dealloc_fn_t)(void *ptr);
typedef void *(*pnso_realloc_fn_t)(void *ptr, size_t sz);

void pnso_test_init_fns(pnso_submit_req_fn_t submit,
			pnso_output_fn_t status_output,
			pnso_alloc_fn_t alloc,
			pnso_dealloc_fn_t dealloc,
			pnso_realloc_fn_t realloc);
void pnso_test_shutdown(void);
void pnso_test_set_shutdown_complete(void);
bool pnso_test_is_shutdown(void);
struct test_desc *pnso_test_desc_alloc(void);
void pnso_test_desc_free(struct test_desc *desc);
pnso_error_t pnso_test_parse_file(const char *fname, struct test_desc *desc);
pnso_error_t pnso_test_parse_buf(const unsigned char *buf, size_t buf_len,
				 struct test_desc *desc);
pnso_error_t pnso_test_run_all(struct test_desc *desc, int ctl_core);
pnso_error_t pnso_run_unit_tests(struct test_desc *desc);

void test_dump_desc(struct test_desc *desc);
void test_dump_yaml_desc_tree(void);

/* OSAL functions */
uint32_t test_file_size(const char *fname);
uint8_t *test_alloc_and_read_file(const char *fname, uint32_t foffset, uint32_t *len);
pnso_error_t test_read_file(const char *fname,
			    struct pnso_buffer_list *buflist,
			    uint32_t foffset, uint32_t flen);
pnso_error_t test_write_file(const char *fname,
			     const struct pnso_buffer_list *buflist,
			     uint32_t flen, uint32_t flags);
pnso_error_t test_delete_file(const char *fname);
void test_fill_buflist(struct pnso_buffer_list *buflist,
		       const uint8_t *data, uint32_t data_len);
pnso_error_t test_fill_random(struct pnso_buffer_list *buflist, uint32_t seed, uint32_t random_len);
int test_compare_files(const char *path1, const char *path2, uint32_t offset, uint32_t len);
int test_compare_file_data(const char *path, uint32_t offset, uint32_t len,
			   const uint8_t *pattern, uint32_t pat_len);

/* Functions in pnso_test_util.c */
uint32_t roundup_block_count(uint32_t len, uint32_t block_size);
uint32_t roundup_len(uint32_t len, uint32_t block_size);
uint32_t roundup_pow2(uint32_t len);
uint32_t safe_itoa(char *dst, uint32_t dst_len, uint64_t val);
uint32_t safe_strcpy_tolower(char *dst, const char *src, uint32_t max_len);
uint32_t safe_strcpy(char *dst, const char *src, uint32_t max_len);
int safe_strcmp(const char *str1, const char *str2);
int safe_strncmp(const char *str1, const char *str2, uint32_t len);
int safe_memcmp(const uint8_t *blob1, const uint8_t *blob2, uint32_t *cmp_len);
unsigned long long safe_strtoll(const char *val);
uint32_t safe_bintohex(char *dst, uint32_t dst_len,
		       const uint8_t *src, uint32_t src_len);

void *pnso_test_alloc(size_t sz);
void *pnso_test_alloc_aligned(size_t alignment, size_t sz);
void pnso_test_free(void *ptr);
#define TEST_ALLOC pnso_test_alloc
#define TEST_ALLOC_ALIGNED pnso_test_alloc_aligned
#define TEST_FREE pnso_test_free

  
#define PNSO_LOG_ERROR OSAL_LOG_ERROR
#define PNSO_LOG_WARN  OSAL_LOG_WARN
#define PNSO_LOG_NOTICE OSAL_LOG_NOTICE
#define PNSO_LOG_INFO  OSAL_LOG_INFO
#define PNSO_LOG_DEBUG OSAL_LOG_DEBUG
#define PNSO_LOG_TRACE(...)
#define PNSO_LOG       OSAL_LOG

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* __PNSO_TEST_H__ */
