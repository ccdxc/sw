/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __UTILS_IMPL_H__
#define __UTILS_IMPL_H__

u_int64_t intr_msixcfg_addr(const int intr);
u_int64_t intr_fwcfg_addr(const int intr);
u_int64_t intr_drvcfg_addr(const int intr);
u_int64_t intr_assert_addr(const int intr);
u_int64_t intr_pba_addr(const int lif);
u_int64_t intr_pba_cfg_addr(const int lif);
u_int64_t db_host_addr(const int lif);

#endif /* __UTILS_IMPL_H__ */
