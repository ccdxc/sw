//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_INFRA_CFG_CLI_H__
#define __VPP_INFRA_CFG_CLI_H__

#ifdef __cplusplus
extern "C" {
#endif

#define DISPLAY_BUF_SIZE 1024
#define PDS_VPP_MAX_DHCP_SUBNET 5

#define PRINT_HEADER_LINE(size)        \
{                                      \
    char buf[size+1];                  \
    clib_memset(buf, '-', size);       \
    buf[size] = '\0';                  \
    vlib_cli_output(vm,"%s", buf);     \
}                                      \

int vpc_cfg_dump(vlib_main_t *vm);
int subnet_cfg_dump(vlib_main_t *vm);
int vnic_cfg_dump(vlib_main_t *vm);
int device_cfg_dump(vlib_main_t *vm);
int dhcp_cfg_dump(vlib_main_t *vm);
int nat_cfg_dump(vlib_main_t *vm);
int security_profile_cfg_dump(vlib_main_t *vm);
int impl_db_dump(char *obj);

typedef struct buf_entry_s {
    char buf[DISPLAY_BUF_SIZE];
    vlib_main_t *vm;
} buf_entry_t;


#ifdef __cplusplus
}
#endif

#endif    // __VPP_INFRA_CFG_CLI_H__

