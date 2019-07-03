/*
 * Copyright (c) 2018-2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>

#include "platform/pciehdevices/include/pci_ids.h"
#include "platform/intrutils/include/intrutils.h"
#include "platform/pciemgrutils/include/pciemgrutils.h"
#include "pciehdevices.h"
#include "pciehdevices_impl.h"

static int
virtio_bars(pciehdev_t *pdev, const pciehdev_res_t *res)
{
    u_int32_t msixtbloff, msixpbaoff;
    pciehbars_t *pbars;
    pciehbar_t pbar;
    pciehbarreg_t preg;
    prt_t prt;

    pbars = pciehbars_new();

    /*****************
     * virtio resource io bar
     */
    memset(&pbar, 0, sizeof(pbar));
    memset(&preg, 0, sizeof(preg));
    memset(&prt, 0, sizeof(prt));

    pbar.type = PCIEHBARTYPE_IO;
    pbar.size = 0x1000; /* XXX */
    pbar.cfgidx = 0;

    pmt_bar_enc(&preg.pmt,
                res->port,
                PMT_TYPE_IO,
                pbar.size, /* barsize */
                pbar.size, /* prtsize */
                PMTF_RW);

    prt_res_enc(&prt,
                res->virtio.virtioregspa,
                0x1000,
                PRT_RESF_NONE);

    pciehbarreg_add_prt(&preg, &prt);
    pciehbar_add_reg(&pbar, &preg);
    pciehbars_add_bar(pbars, &pbar);

    /*****************
     * msix bar - mem64
     */
    memset(&pbar, 0, sizeof(pbar));
    pbar.type = PCIEHBARTYPE_MEM64;
    pbar.size = 2 * 0x1000;
    pbar.cfgidx = 2;

    assert(res->intrc <= 256); /* 256 intrs per page (XXX grow bar) */
    msixtbloff = 0;
    msixpbaoff = 0x1000;
    add_msix_region(pbars, &pbar, res, msixtbloff, msixpbaoff);
    pciehbars_add_bar(pbars, &pbar);

    /* XXX TODO:
     *
     * TODO Common Config:
     *
     * 4.1.4.3 Common configuration structure layout
     * https://docs.oasis-open.org/virtio/virtio/v1.1/cs01/virtio-v1.1-cs01.html#x1-1090004
     *
     * 2.1 Device Status Field
     * 3.1 Device Initialization
     *
     * Need special behavior select registers and the device status
     * register.
     *
     * TODO Notify/Doorbells:
     *
     * 4.1.5.2 Available Buffer Notifications
     * https://docs.oasis-open.org/virtio/virtio/v1.1/cs01/virtio-v1.1-cs01.html#x1-1370002
     *
     * Notify registers should go direcctly to real doorbells.
     * We can provide two notify registers, for RX and TX queue
     * types.
     *
     * The value written to the notify register is the queue id,
     * however it is the queue id in terms of virtio.  To transform
     * the virtio queue id into the TX or RX queue id in the
     * device, shift right the doorbell value by one bit.
     *
     * TODO Control Queue Notify/Doorbell (lowprio/future):
     *
     * If we later add support for a control queue, that can be
     * another queue type.  There will be only one control queue,
     * but its virtio queue id will be one greater than the highest
     * virtio TX queue id.  We would need to tell the doorbell
     * logic to ignore the queue id for that one queue type, or
     * subtract to get the queue id, or size the number of queues
     * so that the control queue id is zero mod that many bits.
     *
     * TODO ISR Status Region (lowprio):
     *
     * Needs read-to-clear, and should indicate the current
     * intrrupt control status.  Low priority, since this is only
     * needed for INTX legacy pci interrupt emulation.
     */

    pciehdev_set_bars(pdev, pbars);
    return 0;
}

static int
virtio_cfg(pciehdev_t *pdev, const pciehdev_res_t *res)
{
    pciehcfg_t *pcfg = pciehcfg_new();
    pciehbars_t *pbars = pciehdev_get_bars(pdev);

    /* XXX TODO:
     *
     * 4.1.4 Virtio Structure PCI Capabilities
     * https://docs.oasis-open.org/virtio/virtio/v1.1/cs01/virtio-v1.1-cs01.html#x1-1090004
     *
     * To be compatible with unmodified virtio driver, we will need
     * to add capability structs to describe the following regions
     * in io/mmio space:
     *
     * -> 4.1.4.3 Notification struture layout
     * -> 4.1.4.4 Notification struture layout
     * -> 4.1.4.5 IRS status capability
     * -> 4.1.4.6 Device-specific configuration
     *
     * Specified as MUST, but is unsed by the linux virtio driver.
     * -> 4.1.4.7 PCI configuration access capability
     *
     *
     * The current workaround in modified virtio driver replaces
     * virtio_pci_modern_probe with xxx_virtio_pci_modern_probe,
     * and hardcodes the configuration that would be described by
     * the above capabilities 4.1.4.3-6.
     */

    pciehcfg_setconf_vendorid(pcfg, 0x1af4);    /* Vendor ID Redhat */
    pciehcfg_setconf_deviceid(pcfg, 0x1000);    /* transitional virtio-net */
    /* XXX */
    //pciehcfg_setconf_deviceid(pcfg, 0x2000);    /* XXXtest: keep drv away */
    /* XXX */
    pciehcfg_setconf_subvendorid(pcfg, 0x1af4); /* Subvendor ID Redhat */
    pciehcfg_setconf_subdeviceid(pcfg, 0x0001); /* Subdevice ID virtio-net */
    pciehcfg_setconf_classcode(pcfg, 0x020000); /*PCI_CLASS_NETWORK_ETHERNET*/
    pciehcfg_setconf_nintrs(pcfg, res->intrc);
    pciehcfg_setconf_msix_tblbir(pcfg, pciehbars_get_msix_tblbir(pbars));
    pciehcfg_setconf_msix_tbloff(pcfg, pciehbars_get_msix_tbloff(pbars));
    pciehcfg_setconf_msix_pbabir(pcfg, pciehbars_get_msix_pbabir(pbars));
    pciehcfg_setconf_msix_pbaoff(pcfg, pciehbars_get_msix_pbaoff(pbars));
    pciehcfg_setconf_dsn(pcfg, res->dsn);

    pciehcfg_sethdr_type0(pcfg, pbars);
    pciehcfg_add_standard_caps(pcfg);

    pciehdev_set_cfg(pdev, pcfg);
    return 0;
}

static int
virtio_initpf(pciehdev_t *pfdev, const pciehdev_res_t *pfres)
{
    if (virtio_bars(pfdev, pfres) < 0) {
        return -1;
    }
    if (virtio_cfg(pfdev, pfres) < 0) {
        return -1;
    }
    return 0;
}

static pciehdevice_t virtio_device = {
    .type = PCIEHDEVICE_VIRTIO,
    .initpf = virtio_initpf,
};
PCIEHDEVICE_REGISTER(virtio_device);
