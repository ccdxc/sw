/*
 * Copyright (c) 2017-2019, Pensando Systems Inc.
 */

#ifndef __PCIEHW_DEV_H__
#define __PCIEHW_DEV_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

struct pciehdev_s;
typedef struct pciehdev_s pciehdev_t;
struct pciehcfg_s;
typedef struct pciehcfg_s pciehcfg_t;
struct pciehbars_s;
typedef struct pciehbars_s pciehbars_t;
struct pciemgr_params_s;
typedef struct pciemgr_params_s pciemgr_params_t;

int pciehdev_open(pciemgr_params_t *params);
void pciehdev_close(void);

pciehdev_t *pciehdev_bridgeup_new(void);
pciehdev_t *pciehdev_bridgedn_new(const int port, const int memtun_en);

int pciehdev_initialize(const int port);
int pciehdev_finalize(const int port);

pciehcfg_t *pciehdev_get_cfg(pciehdev_t *pdev);
pciehbars_t *pciehdev_get_bars(pciehdev_t *pdev);

int pciehdev_add(pciehdev_t *pdev);
int pciehdev_addfn(pciehdev_t *pdev, pciehdev_t *pfn, const int fnc);
int pciehdev_addchild(pciehdev_t *pdev, pciehdev_t *pchild);

pciehdev_t *pciehdev_get_root(const u_int8_t port);
pciehdev_t *pciehdev_get_parent(pciehdev_t *pdev);
pciehdev_t *pciehdev_get_peer(pciehdev_t *pdev);
pciehdev_t *pciehdev_get_child(pciehdev_t *pdev);

pciehdev_t *pciehdev_get_by_bdf(const u_int8_t port, const u_int16_t bdf);
pciehdev_t *pciehdev_get_by_name(const char *name);

void *pciehdev_get_hwdev(pciehdev_t *pdev);
void pciehdev_set_hwdev(pciehdev_t *pdev, void *phwdev);
u_int16_t pciehdev_get_bdf(pciehdev_t *pdev);

union pciehwdev_u;
typedef union pciehwdev_u pciehwdev_t;

u_int16_t pciehwdev_get_hostbdf(const pciehwdev_t *phwdev);
pciehwdev_t *pciehwdev_get_by_id(const u_int8_t port,
                                 const u_int16_t venid, const u_int16_t devid);

int pciehw_notify_poll_init(const int port);
int pciehw_notify_poll(const int port);
int pciehw_notify_intr_init(const int port,
                            u_int64_t msgaddr, u_int32_t msgdata);
int pciehw_notify_intr(const int port);
void pciehw_notify_disable(const int port);
void pciehw_notify_disable_all_ports(void);

int pciehw_indirect_poll_init(const int port);
int pciehw_indirect_poll(const int port);
int pciehw_indirect_intr_init(const int port,
                              u_int64_t msgaddr, u_int32_t msgdata);
int pciehw_indirect_intr(const int port);
void pciehw_indirect_disable(const int port);
void pciehw_indirect_disable_all_ports(void);

/* flags for stats_show() */
#define PMGRSF_NONE     0x0
#define PMGRSF_ALL      0x1
void pciehw_stats_show(const int port, const unsigned int flags);
void pciehw_stats_clear(const int port, const unsigned int flags);
union pciemgr_stats; typedef union pciemgr_stats pciemgr_stats_t;
pciemgr_stats_t *pciehw_stats_get(const int port);

void pciehw_dbg(int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

#endif /* __PCIEHW_DEV_H__ */
