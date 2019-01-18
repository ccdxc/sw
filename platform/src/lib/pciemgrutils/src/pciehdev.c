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

#include "platform/src/lib/misc/include/misc.h"
#include "platform/src/lib/misc/include/bdf.h"
#include "platform/src/lib/pciehdevices/include/pciehdevices.h"
#include "pciemgrutils.h"
#include "pciehdev_impl.h"

#define NPORTS 8

/******************************************************************/

static void
pciehdev_setconf_defaults(pciehdev_t *pdev)
{
}

pciehdev_t *
pciehdev_new(const char *name, const pciehdevice_resources_t *pres)
{
    pciehdev_t *pdev = pciesys_zalloc(sizeof(pciehdev_t));
    assert(pdev != NULL);

    pciehdev_setconf_defaults(pdev);
    if (pres) {
        pdev->lifb = pres->lifb;
        pdev->lifc = pres->lifc;
        pdev->intrb = pres->intrb;
        pdev->intrc = pres->intrc;
        pdev->port = pres->port;
    }
    strncpy0(pdev->name, name, sizeof(pdev->name));
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
pciehdev_set_priv(pciehdev_t *pdev, void *priv)
{
    pdev->priv = priv;
}

void *
pciehdev_get_priv(pciehdev_t *pdev)
{
    return pdev->priv;
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

u_int32_t
pciehdev_get_lifb(pciehdev_t *pdev)
{
    return pdev->lifb;
}

u_int32_t
pciehdev_get_lifc(pciehdev_t *pdev)
{
    return pdev->lifc;
}

u_int32_t
pciehdev_get_intrb(pciehdev_t *pdev)
{
    return pdev->intrb;
}

u_int32_t
pciehdev_get_intrc(pciehdev_t *pdev)
{
    return pdev->intrc;
}

char *
pciehdev_get_name(pciehdev_t *pdev)
{
    return pdev->name;
}
