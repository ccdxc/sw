/*
 * Copyright (c) 2017-2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>

#include "platform/misc/include/misc.h"
#include "platform/misc/include/bdf.h"
#include "platform/pciehdevices/include/pciehdevices.h"
#include "pciemgrutils.h"
#include "pciehdev_impl.h"

#define NPORTS 8

/******************************************************************/

pciehdev_t *
pciehdev_new(void)
{
    pciehdev_t *pdev = pciesys_zalloc(sizeof(pciehdev_t));
    assert(pdev != NULL);
    return pdev;
}

static void
pciehdev_delete_dev(pciehdev_t *pdev)
{
    if (pdev == NULL) return;
    pciehbars_delete(pdev->pbars);
    pciehcfg_delete(pdev->pcfg);
    pciesys_free(pdev);
}

void
pciehdev_delete(pciehdev_t *pdev)
{
    if (pdev == NULL) return;
    pciehdev_delete(pdev->child);
    pciehdev_delete(pdev->peer);
    pciehdev_delete_dev(pdev);
}

void
pciehdev_set_cfg(pciehdev_t *pdev, pciehcfg_t *pcfg)
{
    pdev->pcfg = pcfg;
}

pciehcfg_t *
pciehdev_get_cfg(pciehdev_t *pdev)
{
    return pdev->pcfg;
}

void
pciehdev_set_bars(pciehdev_t *pdev, pciehbars_t *pbars)
{
    pciehbars_finalize(pbars);
    pdev->pbars = pbars;
}

pciehbars_t *
pciehdev_get_bars(pciehdev_t *pdev)
{
    return pdev->pbars;
}

int
pciehdev_linkvf(pciehdev_t *pfdev, pciehdev_t *vfdev, const u_int16_t totalvfs)
{
    assert(pfdev->child == NULL);
    assert(vfdev->parent == NULL);

    pfdev->pf = 1;
    pfdev->totalvfs = totalvfs;
    pfdev->child = vfdev;
    vfdev->vf = 1;
    vfdev->parent = pfdev;
    return 0;
}

int
pciehdev_addvf(pciehdev_t *pfdev, pciehdev_t *vfdev)
{
    pciehdev_t **ppdev;

    ppdev = &pfdev->child;
    for (; *ppdev; ppdev = &(*ppdev)->peer) {
        continue;
    }
    *ppdev = vfdev;
    if (vfdev) {
        vfdev->parent = pfdev;
    }
    return 0;
}

pciehbars_t *
pciehdev_get_vfbars(pciehdev_t *pfdev)
{
    return pciehdev_is_pf(pfdev) ? pciehdev_get_bars(pfdev->child) : NULL;
}

u_int16_t
pciehdev_get_totalvfs(pciehdev_t *pdev)
{
    return pdev->totalvfs;
}

void
pciehdev_set_totalvfs(pciehdev_t *pdev, const u_int16_t totalvfs)
{
    pdev->totalvfs = totalvfs;
}

int
pciehdev_is_pf(pciehdev_t *pdev)
{
    return pdev->pf;
}

void
pciehdev_set_pf(pciehdev_t *pdev, const int pf)
{
    pdev->pf = pf;
}

int
pciehdev_is_vf(pciehdev_t *pdev)
{
    return pdev->vf;
}

void
pciehdev_set_vf(pciehdev_t *pdev, const int vf)
{
    pdev->vf = vf;
}

int
pciehdev_get_vfidx(pciehdev_t *pdev)
{
    return pdev->vfidx;
}

void
pciehdev_set_vfidx(pciehdev_t *pdev, const int vfidx)
{
    pdev->vfidx = vfidx;
}

int
pciehdev_make_fn0(pciehdev_t *pdev)
{
    if (!pdev->fn0) {
        pdev->fn0 = 1;
        pciehcfg_make_fn0(pdev->pcfg);
    }
    return 0;
}

int
pciehdev_make_fnn(pciehdev_t *pdev, const int fnn)
{
    pdev->fnn = fnn;
    pciehcfg_make_fnn(pdev->pcfg, fnn);
    return 0;
}

u_int8_t
pciehdev_get_port(pciehdev_t *pdev)
{
    return pdev->port;
}

void
pciehdev_set_port(pciehdev_t *pdev, const u_int8_t port)
{
    pdev->port = port;
}

u_int32_t
pciehdev_get_lifb(pciehdev_t *pdev)
{
    return pdev->lifb;
}

void
pciehdev_set_lifb(pciehdev_t *pdev, const u_int32_t lifb)
{
    pdev->lifb = lifb;
}

u_int32_t
pciehdev_get_lifc(pciehdev_t *pdev)
{
    return pdev->lifc;
}

void
pciehdev_set_lifc(pciehdev_t *pdev, const u_int32_t lifc)
{
    pdev->lifc = lifc;
}

u_int32_t
pciehdev_get_intrb(pciehdev_t *pdev)
{
    return pdev->intrb;
}

void
pciehdev_set_intrb(pciehdev_t *pdev, const u_int32_t intrb)
{
    pdev->intrb = intrb;
}

u_int32_t
pciehdev_get_intrc(pciehdev_t *pdev)
{
    return pdev->intrc;
}

void
pciehdev_set_intrc(pciehdev_t *pdev, const u_int32_t intrc)
{
    pdev->intrc = intrc;
}

int
pciehdev_get_intrm(pciehdev_t *pdev)
{
    return pdev->intrdmask;
}

void
pciehdev_set_intrm(pciehdev_t *pdev, const int intrm)
{
    pdev->intrdmask = intrm;
}

char *
pciehdev_get_name(pciehdev_t *pdev)
{
    return pdev->name;
}

void
pciehdev_set_name(pciehdev_t *pdev, const char *name)
{
    strncpy0(pdev->name, name, sizeof(pdev->name));
}
