/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __MNET_H__
#define __MNET_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/ioctl.h>

#define MNET_DEVICE_FILE		"/dev/mnet"

#define MNIC_NAME_LEN       (32)

struct mnet_dev_create_req_t
{
	uint64_t devcmd_pa;
	uint64_t devcmd_db_pa;
	uint64_t doorbell_pa;
	uint64_t drvcfg_pa;
	uint64_t msixcfg_pa;
	char iface_name[MNIC_NAME_LEN];
};

#define MNET_CREATE_DEV 		_IOWR('Q', 11, struct mnet_dev_create_req_t)
#define MNET_DESTROY_DEV 		_IOW('Q', 12, const char*)

int create_mnet(struct mnet_dev_create_req_t *mneq_req);
int remove_mnet(const char *if_name);


#ifdef __cplusplus
}
#endif

#endif /* __MNET_H__ */
