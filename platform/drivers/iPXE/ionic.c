/*
 * Copyright 2017-2018 Pensando Systems, Inc.  All rights reserved.
 *
 * This program is free software; you may redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

FILE_LICENCE(GPL2_OR_LATER_OR_UBDL);

#include "ionic.h"

/** @file
 *
 * Ionic network driver
 *
 */

/******************************************************************************
 *
 * Device reset
 *
 ******************************************************************************
 */

/**
 * Reset hardware
 *
 * @v ionic				ionic device
 * @ret rc				Return status code
 */
static int ionic_reset(struct ionic *ionic)
{

	struct ionic_dev *idev = &ionic->idev;

	ionic_dev_cmd_reset(idev);
	return ionic_dev_cmd_wait_check(idev, devcmd_timeout);
}

/******************************************************************************
 *
 * Link state
 *
 ******************************************************************************
 */

/**
 * Check link state
 *
 * @v netdev				Network device
 */
#if 0
static void ionic_check_link ( struct net_device *netdev ) {
		struct ionic *ionic = netdev->priv;

		DBGC ( ionic, "IONIC %p does not yet support link state\n", ionic );
		netdev_link_err ( netdev, -ENOTSUP );
}
#endif
/******************************************************************************
 *
 * Network device interface
 *
 ******************************************************************************
 */

/**
 * Open network device
 *
 * @v netdev				Network device
 * @ret rc				Return status code
 */
static int ionic_open(struct net_device *netdev)
{
	struct ionic *ionic = netdev->priv;
	int err;

	err = ionic_qcq_enable(ionic->ionic_lif->txqcqs);
	if (err)
		return err;

	ionic_rx_fill(netdev, netdev->max_pkt_len);

	err = ionic_lif_rx_mode(ionic->ionic_lif, 0x1F);
	if (err)
		return err;

	err = ionic_qcq_enable(ionic->ionic_lif->rxqcqs);
	if (err)
		return err;

	return 0;
}

/**
 * Close network device
 *
 * @v netdev				Network device
 */
static void ionic_close(struct net_device *netdev)
{
	struct ionic *ionic = netdev->priv;

	ionic_qcq_disable(ionic->ionic_lif->txqcqs);

	ionic_qcq_disable(ionic->ionic_lif->rxqcqs);

	ionic_rx_flush(ionic->ionic_lif);
}

/**
 * Transmit packet
 *
 * @v netdev				Network device
 * @v iobuf				I/O buffer
 * @ret rc				Return status code
 */
static int ionic_transmit(struct net_device *netdev,
						  struct io_buffer *iobuf)
{
	struct ionic *ionic = netdev->priv;
	struct queue *txq = &ionic->ionic_lif->txqcqs->q;
	struct txq_desc *desc = txq->head->desc;

	if (!ionic_q_has_space(txq, 1)) {
		DBGC(ionic, "%s no more desc available the txq is full\n", __FUNCTION__);
		return -ENOBUFS;
	}

	// fill the descriptor
	desc->opcode = TXQ_DESC_OPCODE_CALC_NO_CSUM;
	desc->addr = virt_to_bus(iobuf->data);
	desc->num_sg_elems = 0;
	desc->len = iob_len(iobuf);
	desc->vlan_tci = 0;
	desc->hdr_len = 0;
	desc->csum_offset = 0;
	desc->V = 0;
	desc->C = 1;
	desc->O = 0;

	// increment the head for the q.
	txq->head = txq->head->next;

	// ring the doorbell
	struct doorbell db = {
		.qid_lo = txq->qid,
		.qid_hi = txq->qid >> 8,
		.ring = 0,
		.p_index = txq->head->index,
	};
	writeq(*(u64 *)&db, txq->db);

	return 0;
}

/**
 * Poll for completed and received packets
 *
 * @v netdev				Network device
 */
static void ionic_poll(struct net_device *netdev)
{

	// Poll for transmit completions
	ionic_poll_tx(netdev);

	// Poll for receive completions
	ionic_poll_rx(netdev);

	// Refill receive ring
	ionic_rx_fill(netdev, netdev->max_pkt_len);
}

/**
 * Enable or disable interrupts
 *
 * @v netdev				Network device
 * @v enable				Interrupts should be enabled
 */
/*static void skeleton_irq ( struct net_device *netdev, int enable ) {
		struct ionic *skel = netdev->priv;

		DBGC ( skel, "SKELETON %p does not yet support interrupts\n", skel );
		( void ) enable;
}*/

/** Skeleton network device operations */
static struct net_device_operations ionic_operations = {
	.open = ionic_open,
	.close = ionic_close,
	.transmit = ionic_transmit,
	.poll = ionic_poll,
};

/******************************************************************************
 *
 * Ionic PCI interface
 *
 ******************************************************************************
 */

/**
 * Map the bar registers and addresses.
 **/
static int ionic_map_bars(struct ionic *ionic, struct pci_device *pci)
{
	struct ionic_device_bar *bars = ionic->bars;
	unsigned int i, j;

	ionic->num_bars = 0;
	for (i = 0, j = 0; i < IONIC_BARS_MAX; i++) {
		bars[j].len = pci_bar_size(pci, PCI_BASE_ADDRESS(i * 2));
		bars[j].bus_addr = pci_bar_start(pci, PCI_BASE_ADDRESS(i * 2));
		bars[j].virtaddr = ioremap(bars[j].bus_addr, bars[j].len);
		if (!bars[j].virtaddr) {
			DBG("Cannot memory-map BAR %d, aborting\n", j);
			return -ENODEV;
		}
		ionic->num_bars++;
		j++;
	}
	return 0;
}

/**
 * Unmap the bar registers and addresses.
 **/
static void ionic_unmap_bars(struct ionic *ionic)
{
	struct ionic_device_bar *bars = ionic->bars;
	unsigned int i;

	for (i = 0; i < IONIC_BARS_MAX; i++)
		if (bars[i].virtaddr)
			iounmap(bars[i].virtaddr);
}

/**
 * Probe PCI device and setup the ionic driver.
 *
 * @v pci				PCI device
 * @ret rc				Return status code
 */
static int ionic_probe(struct pci_device *pci)
{
	struct net_device *netdev; // network device information.
	struct ionic *ionic;	   // ionic device information.
	int errorcode;

	// Allocate and initialise net device
	netdev = alloc_etherdev(sizeof(*ionic));
	if (!netdev) {
		errorcode = -ENOMEM;
		goto err_alloc;
	}

	netdev_init(netdev, &ionic_operations);
	ionic = netdev->priv;
	pci_set_drvdata(pci, netdev);
	netdev->dev = &pci->dev;
	memset(ionic, 0, sizeof(*ionic));

	// Fix up PCI device
	adjust_pci_device(pci);

	// Map registers
	errorcode = ionic_map_bars(ionic, pci);
	if (errorcode) {
		DBG("%s :: the number of bars is %x mapped the bars.\n", __FUNCTION__, ionic->num_bars);
		goto err_ionicunmap;
	}

	errorcode = ionic_setup(ionic);
	if (errorcode) {
		DBG("%s :: Cannot setup device, aborting\n", __FUNCTION__);
		goto err_ionicunmap;
	}

	// Reset the NIC
	if ((errorcode = ionic_reset(ionic)) != 0)
		goto err_reset;

	// Identify the Ionic
	errorcode = ionic_identify(ionic);
	if (errorcode) {
		DBG("%s :: Cannot identify device, aborting\n", __FUNCTION__);
		goto err_reset;
	}

	errorcode = ionic_lif_alloc(ionic, 0);
	if (errorcode) {
		DBG("%s :: Cannot allocate LIFs, aborting\n", __FUNCTION__);
		goto err_free_identify;
	}

	errorcode = ionic_lif_init(netdev);
	if (errorcode) {
		DBG("%s :: Cannot initiate LIFs, aborting\n", __FUNCTION__);
		goto err_free_alloc;
	}
	// Register network device
	if ((errorcode = register_netdev(netdev)) != 0)
		goto err_register_netdev;

	/* Mark as link up, since we have no way to test link state on
		 * this hardware.
		 */
	netdev_link_up(netdev);

	return 0;

	unregister_netdev(netdev);
err_register_netdev:
	ionic_reset(ionic);
err_free_alloc:
	ionic_qcq_dealloc(ionic->ionic_lif->adminqcq);
	ionic_qcq_dealloc(ionic->ionic_lif->txqcqs);
	ionic_qcq_dealloc(ionic->ionic_lif->rxqcqs);
	free(ionic->ionic_lif);
err_free_identify:
	free_dma(ionic->ident, sizeof(union identity));
err_reset:
err_ionicunmap:
	ionic_unmap_bars(ionic);
	netdev_nullify(netdev);
	netdev_put(netdev);
err_alloc:
	return errorcode;
}

/**
 * Remove PCI device
 *
 * @v pci				PCI device
 */
static void ionic_remove(struct pci_device *pci)
{
	struct net_device *netdev = pci_get_drvdata(pci);
	struct ionic *ionic = netdev->priv;

	// Unregister network device
	unregister_netdev(netdev);

	// Reset card
	ionic_reset(ionic);

	// Free network device
	free_dma(ionic->ident, sizeof(union identity));
	free(ionic->ionic_lif->adminqcq);
	free(ionic->ionic_lif->txqcqs);
	free(ionic->ionic_lif->rxqcqs);
	free(ionic->ionic_lif);
	ionic_unmap_bars(ionic);
	netdev_nullify(netdev);
	netdev_put(netdev);
}

/** Ionic PCI device IDs */
static struct pci_device_id ionic_nics[] = {
	PCI_ROM(0x1DD8, 0x1002, "ionic", "Pensando Eth-NIC Driver", 0),
};

/** Ionic PCI driver */
struct pci_driver ionic_driver __pci_driver = {
	.ids = ionic_nics,
	.id_count = ARRAY_SIZE(ionic_nics),
	.probe = ionic_probe,
	.remove = ionic_remove,
};
