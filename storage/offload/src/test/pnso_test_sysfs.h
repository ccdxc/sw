/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#ifndef __PNSO_TEST_SYSFS_H__
#define __PNSO_TEST_SYSFS_H__

#include "pnso_api.h"
#include "osal_logger.h"

#ifdef __cplusplus
extern "C"
{
#endif


enum {
	CTL_STATE_READ,
	CTL_STATE_START,
	CTL_STATE_REPEAT,
	CTL_STATE_STOP,
	CTL_STATE_MAX
};
int pnso_test_sysfs_init(void);
void pnso_test_sysfs_finit(void);
int pnso_test_sysfs_read_ctl(void);
char *pnso_test_sysfs_alloc_and_get_cfg(void);
void pnso_test_sysfs_write_status_data(const char *src, uint32_t len, void *opaque);

bool pnso_test_sysfs_is_fd(const char *devname, int *fdnum);
uint32_t pnso_test_sysfs_write_fd(int fdnum, uint32_t offset,
			const uint8_t *src, uint32_t size, uint32_t *gen_id);
uint32_t pnso_test_sysfs_read_fd(int fdnum, uint32_t offset,
			uint8_t *dst, uint32_t size, uint32_t *gen_id);
int pnso_test_cmp_fd_data(int fdnum, uint32_t offset,
			const uint8_t *src, uint32_t size);
int pnso_test_sysfs_get_fd_data(int fdnum, struct pnso_flat_buffer *flat_buf, uint32_t *gen_id);
uint32_t pnso_test_sysfs_get_fd_len(int fdnum, uint32_t *gen_id);


#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* __PNSO_TEST_SYSFS_H__ */
