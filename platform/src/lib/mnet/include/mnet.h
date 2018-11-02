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

typedef enum
{
	MNIC_TYPE_ETH,
	MNIC_TYPE_SPAN,
	MNIC_TYPE_MAX
} mnic_type_e;

struct mnet_dev_create_req_t
{
	uint64_t devcmd_pa;
	uint64_t devcmd_db_pa;
	uint64_t doorbell_pa;
	uint64_t drvcfg_pa;
	uint64_t msixcfg_pa;
	mnic_type_e iface_type;
	char iface_name[MNIC_NAME_LEN];
};

struct mnet_create_resp_t
{
	char eth_iface_name[MNIC_NAME_LEN];
};

struct mnet_req_resp_t
{
	struct mnet_dev_create_req_t req;
	struct mnet_create_resp_t resp;
};

#define MNET_CREATE_DEV 		_IOWR('Q', 11, struct mnet_req_resp_t )
#define MNET_DESTROY_DEV 		_IOW('Q', 12, const char*)

int create_mnet(struct mnet_req_resp_t *mnet_info);
int remove_mnet(const char *if_name);


#ifdef __cplusplus
}
#endif

#endif /* __MNET_H__ */
