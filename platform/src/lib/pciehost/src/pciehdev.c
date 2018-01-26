/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>

#include "misc.h"
#include "bdf.h"
#include "pci_ids.h"
#include "pciehdevices.h"
#include "pciehdev.h"
#include "pciehdev_impl.h"
#include "pciehcfg.h"
#include "pciehbar.h"
#include "pciehsys.h"
#include "pciehw.h"

typedef struct pciehdev_data_s {
    pciehdev_params_t devparams;
    u_int32_t finalized:1;
    u_int32_t memtun_br:1;
    pciehdev_t *root;
} pciehdev_data_t;

static pciehdev_data_t pciehdev_data;

static pciehdev_data_t *
pciehdev_get_data(void)
{
    return &pciehdev_data;
}

/******************************************************************/

int
pciehdev_open(pciehdev_params_t *devparams)
{
    pciehdev_data_t *pdevdata = pciehdev_get_data();
    pciehw_params_t hwparams;
    int r;

    if (devparams) {
        pdevdata->devparams = *devparams;
    }

    if (pdevdata->devparams.vendorid == 0) {
        /* vendorid defaults to Pensando. */
        pdevdata->devparams.vendorid = PCI_VENDOR_ID_PENSANDO;
    }
    if (pdevdata->devparams.subvendorid == 0) {
        /* subvendorid defaults to vendorid. */
        pdevdata->devparams.subvendorid = pdevdata->devparams.vendorid;
    }
    if (pdevdata->devparams.cap_gen == 0) {
        pdevdata->devparams.cap_gen = 4;
    }
    if (pdevdata->devparams.cap_width == 0) {
        pdevdata->devparams.cap_width = 16;
    }

    memset(&hwparams, 0, sizeof(hwparams));
    hwparams.inithw = pdevdata->devparams.inithw;
    hwparams.fake_bios_scan = pdevdata->devparams.fake_bios_scan;
    hwparams.force_notify_cfg = pdevdata->devparams.force_notify_cfg;
    hwparams.force_notify_bar = pdevdata->devparams.force_notify_bar;
    hwparams.first_bus = pdevdata->devparams.first_bus;
    if ((r = pciehw_open(&hwparams)) < 0) {
        return r;
    }
    return 0;
}

void
pciehdev_close(void)
{
    pciehw_close();
}

/******************************************************************/

static void
pciehdev_setconf_defaults(pciehdev_t *pdev)
{
}

pciehdev_t *
pciehdev_new(const char *name, const pciehdevice_resources_t *pres)
{
    pciehdev_t *pdev = pciehsys_zalloc(sizeof(pciehdev_t));
    assert(pdev != NULL);

    pciehdev_setconf_defaults(pdev);
    if (pres) {
        if (pres->lif_valid) {
            pdev->lif = pres->lif;
            pdev->lif_valid = pres->lif_valid;
            pdev->port = pres->port;
        }
        pdev->intrb = pres->intrb;
        pdev->intrc = pres->intrc;
    }
    strncpy0(pdev->name, name, sizeof(pdev->name));
    return pdev;
}

void
pciehdev_delete(pciehdev_t *pdev)
{
    if (pdev == NULL) return;
    pciehbars_delete(pdev->pbars);
    pciehcfg_delete(pdev->pcfg);
    pciehsys_free(pdev);
}

static void
pciehdev_delete_topology(pciehdev_t *pdev)
{
    if (pdev == NULL) return;
    pciehdev_delete_topology(pdev->child);
    pciehdev_delete_topology(pdev->peer);
    pciehdev_delete(pdev);
}

int
pciehdev_initialize(void)
{
    pciehdev_data_t *pdevdata = pciehdev_get_data();

    if (pdevdata->finalized) {
        pdevdata->finalized = 0;
    }
    pciehw_initialize_topology();
    pciehdev_delete_topology(pdevdata->root);
    pdevdata->root = NULL;
    pdevdata->memtun_br = 0;
    return 0;
}

static int
pciehdev_finalize_dev(pciehdev_t *pdev, u_int16_t bdf, u_int8_t *nextbus)
{
    if (pdev) {
        pdev->bdf = bdf;

#if 0
        printf("finalize_dev: %-8s bdf %s par %s peer %s child %s\n",
               pciehdev_get_name(pdev), bdf_to_str(pdev->bdf),
               pdev->parent ? pciehdev_get_name(pdev->parent) : "(none)",
               pdev->peer ? pciehdev_get_name(pdev->peer) : "(none)",
               pdev->child ? pciehdev_get_name(pdev->child) : "(none)");
#endif

        /*
         * Process child link first, then peer, so we match
         * the path of the BIOS scan.  Our assigned bdf's
         * will match the actual bdf's when the BIOS assigns
         * bus numbers to bridges.
         */
        if (pdev->child) {
            int childbdf;
            if (pdev->fn0) {
                childbdf = bdf_make(bdf_to_bus(bdf),
                                    bdf_to_dev(bdf),
                                    1);
            } else {
                childbdf = bdf_make((*nextbus)++, 0, 0);
            }
            pciehdev_finalize_dev(pdev->child, childbdf, nextbus);
        }

        if (pdev->peer) {
            int peerbdf;
            if (pdev->peer->fnn) {
                peerbdf = bdf_make(bdf_to_bus(bdf),
                                   bdf_to_dev(bdf),
                                   bdf_to_fnc(bdf) + 1);
            } else {
                peerbdf = bdf_make(bdf_to_bus(bdf), bdf_to_dev(bdf) + 1, 0);
            }
            pciehdev_finalize_dev(pdev->peer, peerbdf, nextbus);
        }
    }
    return 0;
}

int
pciehdev_finalize(void)
{
    pciehdev_data_t *pdevdata = pciehdev_get_data();
    u_int8_t nextbus = pdevdata->devparams.first_bus;

    if (pdevdata->finalized) {
        return -EALREADY;
    }
    if (pdevdata->root == NULL) {
        pciehdev_t *pbrdn;
        const int memtun_en = pdevdata->memtun_br == 0;
        pdevdata->root = pciehdev_bridgeup_new();
        pbrdn = pciehdev_bridgedn_new(memtun_en);
        pciehdev_addchild(pdevdata->root, pbrdn);
        pdevdata->memtun_br = 1;
    }
    pciehdev_finalize_dev(pdevdata->root, bdf_make(0, 0, 0), &nextbus);
    pciehw_finalize_topology(pdevdata->root);
    pdevdata->finalized = 1;
    return 0;
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
pciehdev_addchild(pciehdev_t *pdev, pciehdev_t *pchild)
{
    pciehdev_data_t *pdevdata = pciehdev_get_data();
    pciehdev_t **ppdev;

    if (pdevdata->finalized) {
        return -EBUSY;
    }
    ppdev = pdev ? &pdev->child : &pdevdata->root;
    for (; *ppdev; ppdev = &(*ppdev)->peer) {
        continue;
    }
    *ppdev = pchild;
    if (pchild) {
        pchild->parent = pdev;
    }
    return 0;
}

int
pciehdev_add(pciehdev_t *pdev)
{
    pciehdev_data_t *pdevdata = pciehdev_get_data();
    pciehdev_t *pbrdn;
    int memtun_en = 0;

    if (pdevdata->finalized) {
        return -EBUSY;
    }
    if (pdevdata->root == NULL) {
        pdevdata->root = pciehdev_bridgeup_new();
    }
    memtun_en = pdevdata->memtun_br == 0;
    pbrdn = pciehdev_bridgedn_new(memtun_en);
    pciehdev_addchild(pdevdata->root, pbrdn);
    pdevdata->memtun_br = 1;
    return pciehdev_addchild(pbrdn, pdev);
}

int
pciehdev_addfn(pciehdev_t *pdev, pciehdev_t *pfn, const int fnc)
{
    pciehdev_data_t *pdevdata = pciehdev_get_data();

    if (pdevdata->finalized) {
        return -EBUSY;
    }
    pciehdev_make_fn0(pdev);
    pciehdev_make_fnn(pfn, fnc);
    return pciehdev_addchild(pdev, pfn);
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

pciehdev_t *
pciehdev_get_root(void)
{
    pciehdev_data_t *pdevdata = pciehdev_get_data();
    return pdevdata->root;
}

pciehdev_t *
pciehdev_get_parent(pciehdev_t *pdev)
{
    return pdev->parent;
}

pciehdev_t *
pciehdev_get_child(pciehdev_t *pdev)
{
    return pdev->child;
}

pciehdev_t *
pciehdev_get_peer(pciehdev_t *pdev)
{
    return pdev->peer;
}

static pciehdev_t *
pciehdev_find_by_name(pciehdev_t *pdev, const char *name)
{
    pciehdev_t *pdev_found = NULL;

    if (pdev == NULL) {
        return NULL;
    }
    if (strcmp(pdev->name, name) == 0) {
        pdev_found = pdev;
    }
    if (pdev_found == NULL) {
        pdev_found = pciehdev_find_by_name(pdev->peer, name);
    }
    if (pdev_found == NULL) {
        pdev_found = pciehdev_find_by_name(pdev->child, name);
    }
    return pdev_found;
}

pciehdev_t *
pciehdev_get_by_name(const char *name)
{
    pciehdev_data_t *pdevdata = pciehdev_get_data();
    return pciehdev_find_by_name(pdevdata->root, name);
}

static pciehdev_t *
pciehdev_find_by_bdf(pciehdev_t *pdev, const u_int16_t bdf)
{
    pciehdev_t *pdev_found = NULL;

    if (pdev == NULL) {
        return NULL;
    }
    if (pdev->bdf == bdf) {
        pdev_found = pdev;
    }
    if (pdev_found == NULL) {
        pdev_found = pciehdev_find_by_bdf(pdev->peer, bdf);
    }
    if (pdev_found == NULL) {
        pdev_found = pciehdev_find_by_bdf(pdev->child, bdf);
    }
    return pdev_found;
}

pciehdev_t *
pciehdev_get_by_bdf(const u_int16_t bdf)
{
    pciehdev_data_t *pdevdata = pciehdev_get_data();
    return pciehdev_find_by_bdf(pdevdata->root, bdf);
}

void *
pciehdev_get_hwdev(pciehdev_t *pdev)
{
    return pdev->phwdev;
}

void
pciehdev_set_hwdev(pciehdev_t *pdev, void *phwdev)
{
    pdev->phwdev = phwdev;
}

u_int16_t
pciehdev_get_bdf(pciehdev_t *pdev)
{
    return pdev->bdf;
}

u_int8_t
pciehdev_get_port(pciehdev_t *pdev)
{
    return pdev->port;
}

int
pciehdev_get_lif(pciehdev_t *pdev)
{
    return pdev->lif_valid ? pdev->lif : -1;
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

pciehdev_params_t *
pciehdev_get_params(void)
{
    pciehdev_data_t *pdevdata = pciehdev_get_data();
    return &pdevdata->devparams;
}
