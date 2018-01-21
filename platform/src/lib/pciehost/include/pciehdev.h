/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __PCIEHDEV_H__
#define __PCIEHDEV_H__

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
struct pciehdevice_resources_s;
typedef struct pciehdevice_resources_s pciehdevice_resources_t;

typedef struct pciehdev_params_s {
    u_int8_t cap_gen;           /* default GenX capability (1,2,3,4) */
    u_int8_t cap_width;         /* xX lane width (1,2,4,8,16) */
    u_int16_t vendorid;         /* default vendorid */
    u_int16_t subvendorid;      /* default subvendorid */
    u_int16_t subdeviceid;      /* default subdeviceid */
    u_int8_t first_bus;         /* first bus number for virtual devices bdf */
    u_int32_t inithw:1;         /* initialize hw */
    u_int32_t fake_bios_scan:1; /* scan finalized topology, assign bus #'s */
    u_int32_t force_notify_cfg:1;/* force notify on cfg PMT entries */
    u_int32_t force_notify_bar:1;/* force notify on bar PMT entries */
    u_int32_t noexttag:1;       /* no extended tags capable */
    u_int32_t noexttag_en:1;    /* no extended tags enabled */
    u_int32_t nomsicap:1;       /* no msi cap */
    u_int32_t nomsixcap:1;      /* no msix cap */
} pciehdev_params_t;

int pciehdev_open(pciehdev_params_t *devparams);
void pciehdev_close(void);
pciehdev_params_t *pciehdev_get_params(void);

pciehdev_t *pciehdev_new(const char *name,
                         const pciehdevice_resources_t *pres);
void pciehdev_delete(pciehdev_t *pdev);

pciehdev_t *pciehdev_bridgeup_new(void);
pciehdev_t *pciehdev_bridgedn_new(void);

int pciehdev_initialize(void);
int pciehdev_finalize(void);
void *pcidehdev_get_priv(pciehdev_t *pdev);

void pciehdev_set_cfg(pciehdev_t *pdev, pciehcfg_t *pcfg);
pciehcfg_t *pciehdev_get_cfg(pciehdev_t *pdev);

void pciehdev_set_bars(pciehdev_t *pdev, pciehbars_t *pbars);
pciehbars_t *pciehdev_get_bars(pciehdev_t *pdev);

int pciehdev_add(pciehdev_t *pdev);
int pciehdev_addfn(pciehdev_t *pdev, pciehdev_t *pfn, const int fnc);
int pciehdev_addvf(pciehdev_t *pdev, pciehdev_t *pvf);
int pciehdev_addchild(pciehdev_t *pdev, pciehdev_t *pchild);
int pciehdev_make_fn0(pciehdev_t *pdev);
int pciehdev_make_fnn(pciehdev_t *pdev, const int fnc);

pciehdev_t *pciehdev_get_root(void);
pciehdev_t *pciehdev_get_parent(pciehdev_t *pdev);
pciehdev_t *pciehdev_get_peer(pciehdev_t *pdev);
pciehdev_t *pciehdev_get_child(pciehdev_t *pdev);

pciehdev_t *pciehdev_get_by_bdf(const u_int16_t bdf);
pciehdev_t *pciehdev_get_by_name(const char *name);

void *pciehdev_get_hwdev(pciehdev_t *pdev);
void pciehdev_set_hwdev(pciehdev_t *pdev, void *phwdev);
u_int16_t pciehdev_get_bdf(pciehdev_t *pdev);
u_int8_t pciehdev_get_port(pciehdev_t *pdev);
int pciehdev_get_lif(pciehdev_t *pdev);
u_int32_t pciehdev_get_intrb(pciehdev_t *pdev);
u_int32_t pciehdev_get_intrc(pciehdev_t *pdev);
char *pciehdev_get_name(pciehdev_t *pdev);

#ifdef __cplusplus
}
#endif

#endif /* __PCIEHDEV_H__ */
