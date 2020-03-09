/*
 * Copyright 2017-2019 Pensando Systems, Inc.  All rights reserved.
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
 * Device
 *
 ******************************************************************************
 */

/**
 * Init hardware
 *
 * @v ionic				ionic device
 * @ret rc				Return status code
 */
static int ionic_init(struct ionic *ionic)
{

	struct ionic_dev *idev = &ionic->idev;

	return ionic_dev_cmd_init(idev, devcmd_timeout);
}

/**
 * Reset hardware
 *
 * @v ionic				ionic device
 * @ret rc				Return status code
 */
static int ionic_reset(struct ionic *ionic)
{

	struct ionic_dev *idev = &ionic->idev;

	return ionic_dev_cmd_reset(idev, devcmd_timeout);
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
static int ionic_check_link ( struct net_device *netdev ) {
	struct ionic *ionic = netdev->priv;
	u16 link_up;

	link_up = ionic->lif->info->status.link_status;
	if (link_up != ionic->link_status) {
		ionic->link_status = link_up;
		if (link_up == IONIC_PORT_OPER_STATUS_UP) {
			netdev_link_up ( netdev );
		} else {
			netdev_link_down ( netdev );
		}
	}
	return 0;
}
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
	int mtu;
	int err;

	err = ionic_qcq_enable(ionic->lif->txqcqs);
	if (err)
		return err;

	// fill rx buffers
	mtu = ETH_HLEN + netdev->mtu + 4;
	ionic_rx_fill(netdev, mtu);

	err = ionic_lif_rx_mode(ionic->lif, 0x1F);
	if (err)
		return err;

	err = ionic_qcq_enable(ionic->lif->rxqcqs);
	if (err)
		return err;

	//Update Link Status
	ionic_check_link(netdev);

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

	ionic_qcq_disable(ionic->lif->rxqcqs);

	ionic_qcq_disable(ionic->lif->txqcqs);

	ionic_lif_quiesce(ionic->lif);

	ionic_tx_flush(netdev, ionic->lif);

	ionic_rx_flush(ionic->lif);
}

/**
 * Transmit packet
 *
 * @v netdev			Network device
 * @v iobuf				I/O buffer
 * @ret rc				Return status code
 */
static int ionic_transmit(struct net_device *netdev,
						  struct io_buffer *iobuf)
{
	struct ionic *ionic = netdev->priv;
	struct queue *txq = &ionic->lif->txqcqs->q;
	struct ionic_txq_desc *desc = txq->head->desc;

	if (!ionic_q_has_space(txq, 1)) {
		DBGC(ionic, "%s no more desc available the txq is full\n", __FUNCTION__);
		return -ENOBUFS;
	}

	// fill the descriptor
	desc->cmd = encode_txq_desc_cmd(IONIC_TXQ_DESC_OPCODE_CSUM_NONE,
					0, 0, virt_to_bus(iobuf->data));
	desc->len = iob_len(iobuf);
	desc->hword0 = 0;
	desc->hword1 = 0;
	desc->hword2 = 0;

	// store the iobuf in the txq
	txq->lif->tx_iobuf[txq->head->index] = iobuf;

	// increment the head for the q.
	txq->head = txq->head->next;

	// ring the doorbell
	struct ionic_doorbell db = {
		.qid_lo = txq->hw_index,
		.qid_hi = txq->hw_index >> 8,
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
	int mtu;

	// Poll for transmit completions
	ionic_poll_tx(netdev);

	// Poll for receive completions
	ionic_poll_rx(netdev);

	// Refill receive ring
	mtu = ETH_HLEN + netdev->mtu + 4;
	ionic_rx_fill(netdev, mtu);

	//Update Link Status
	ionic_check_link(netdev);

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
	for (i = 0, j = 0; i < IONIC_IPXE_BARS_MAX; i++) {
		bars[j].len = pci_bar_size(pci, PCI_BASE_ADDRESS(i * 2));
		bars[j].bus_addr = pci_bar_start(pci, PCI_BASE_ADDRESS(i * 2));
		bars[j].vaddr = ioremap(bars[j].bus_addr, bars[j].len);
		if (!bars[j].vaddr) {
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
		if (bars[i].vaddr)
			iounmap(bars[i].vaddr);
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

	ionic->pdev = pci;
	// Fix up PCI device
	adjust_pci_device(pci);

	// Map registers
	errorcode = ionic_map_bars(ionic, pci);
	if (errorcode) {
		DBG2("%s :: Failed to map bars\n", __FUNCTION__);
		goto err_ionicunmap;
	}

	errorcode = ionic_setup(ionic);
	if (errorcode) {
		DBG2("%s :: Cannot setup device, aborting\n", __FUNCTION__);
		goto err_ionicunmap;
	}

	// Init the NIC
	if ((errorcode = ionic_init(ionic)) != 0)
		goto err_ionicunmap;

	// Identify the Ionic
	errorcode = ionic_identify(ionic);
	if (errorcode) {
		DBG2("%s :: Cannot identify device, aborting\n", __FUNCTION__);
		goto err_reset;
	}

	errorcode = ionic_lif_alloc(ionic, 0);
	if (errorcode) {
		DBG2("%s :: Cannot allocate LIFs, aborting\n", __FUNCTION__);
		goto err_reset;
	}

	errorcode = ionic_lif_init(netdev);
	if (errorcode) {
		DBG2("%s :: Cannot initiate LIFs, aborting\n", __FUNCTION__);
		goto err_free_alloc;
	}

	// Register network device
	if ((errorcode = register_netdev(netdev)) != 0)
		goto err_register_netdev;


	//Check the link status
	ionic_check_link(netdev);

	return 0;

	unregister_netdev(netdev);
err_register_netdev:
	ionic_lif_reset(ionic);
err_free_alloc:
	ionic_qcq_dealloc(ionic->lif->adminqcq);
	ionic_qcq_dealloc(ionic->lif->notifyqcqs);
	ionic_qcq_dealloc(ionic->lif->txqcqs);
	ionic_qcq_dealloc(ionic->lif->rxqcqs);
	free_dma(ionic->lif->info, sizeof(ionic->lif->info_sz));
	free(ionic->lif);
err_reset:
	ionic_reset(ionic);
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

	// Reset lif
	ionic_lif_reset(ionic);

	// Reset card
	ionic_reset(ionic);

	// Free network device
	ionic_qcq_dealloc(ionic->lif->adminqcq);
	ionic_qcq_dealloc(ionic->lif->notifyqcqs);
	ionic_qcq_dealloc(ionic->lif->txqcqs);
	ionic_qcq_dealloc(ionic->lif->rxqcqs);
	free_dma(ionic->lif->info, ionic->lif->info_sz);
	free(ionic->lif);
	ionic_unmap_bars(ionic);
	free_dma(ionic->idev.port_info, ionic->idev.port_info_sz);
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
