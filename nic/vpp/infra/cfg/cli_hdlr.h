//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_INFRA_CFG_CLI_HDLR_H__
#define __VPP_INFRA_CFG_CLI_HDLR_H__

#ifdef __cplusplus
extern "C" {
#endif

int vpc_cfg_db_dump_hdlr(void *buf);
int subnet_cfg_db_dump_hdlr(void *buf);
int vnic_cfg_db_dump_hdlr(void *buf);
int device_cfg_db_dump_hdlr(void *buf);
int nat_cfg_db_dump_hdlr(void *buf);
int dhcp_cfg_db_dump_hdlr(void *buf, uint8_t proxy);
int security_profile_cfg_db_dump_hdlr(void *buf);
void buf_entry_dump(void *buf_entry);

#ifdef __cplusplus
}
#endif

#endif    // __VPP_INFRA_CFG_CLI_HDLR_H__

