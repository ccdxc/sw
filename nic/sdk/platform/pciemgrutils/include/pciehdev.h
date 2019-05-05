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

pciehdev_t *pciehdev_new(void);
void pciehdev_delete(pciehdev_t *pdev);

void pciehdev_set_cfg(pciehdev_t *pdev, pciehcfg_t *pcfg);
pciehcfg_t *pciehdev_get_cfg(pciehdev_t *pdev);

void pciehdev_set_bars(pciehdev_t *pdev, pciehbars_t *pbars);
pciehbars_t *pciehdev_get_bars(pciehdev_t *pdev);

int pciehdev_linkvf(pciehdev_t *pfdev, pciehdev_t *vfdev,
                    const u_int16_t totalvfs);
pciehbars_t *pciehdev_get_vfbars(pciehdev_t *pfdev);
u_int16_t pciehdev_get_totalvfs(pciehdev_t *pdev);
void pciehdev_set_totalvfs(pciehdev_t *pdev, const u_int16_t totalvfs);
int pciehdev_addchild(pciehdev_t *pdev, pciehdev_t *pchild);
int pciehdev_addvf(pciehdev_t *pfdev, pciehdev_t *vfdev);
int pciehdev_make_fn0(pciehdev_t *pdev);
int pciehdev_make_fnn(pciehdev_t *pdev, const int fnc);

int pciehdev_is_pf(pciehdev_t *pdev);
int pciehdev_is_vf(pciehdev_t *pdev);
void pciehdev_set_pf(pciehdev_t *pdev, const int pf);
void pciehdev_set_vf(pciehdev_t *pdev, const int vf);
int pciehdev_get_vfidx(pciehdev_t *pdev);
void pciehdev_set_vfidx(pciehdev_t *pdev, const int vfidx);

pciehdev_t *pciehdev_get_by_bdf(const u_int8_t port, const u_int16_t bdf);
pciehdev_t *pciehdev_get_by_name(const char *name);

u_int8_t pciehdev_get_port(pciehdev_t *pdev);
void pciehdev_set_port(pciehdev_t *pdev, const u_int8_t port);
u_int32_t pciehdev_get_lifb(pciehdev_t *pdev);
void pciehdev_set_lifb(pciehdev_t *pdev, const u_int32_t libf);
u_int32_t pciehdev_get_lifc(pciehdev_t *pdev);
void pciehdev_set_lifc(pciehdev_t *pdev, const u_int32_t lifc);
u_int32_t pciehdev_get_intrb(pciehdev_t *pdev);
void pciehdev_set_intrb(pciehdev_t *pdev, const u_int32_t intrb);
u_int32_t pciehdev_get_intrc(pciehdev_t *pdev);
void pciehdev_set_intrc(pciehdev_t *pdev, const u_int32_t intrc);
int pciehdev_get_intrm(pciehdev_t *pdev);
void pciehdev_set_intrm(pciehdev_t *pdev, const int intrm);
char *pciehdev_get_name(pciehdev_t *pdev);
void pciehdev_set_name(pciehdev_t *pdev, const char *name);

#ifdef __cplusplus
}
#endif

#endif /* __PCIEHDEV_H__ */
