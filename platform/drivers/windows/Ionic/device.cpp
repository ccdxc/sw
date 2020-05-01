
#include "common.h"

NDIS_STATUS
ionic_dev_setup(struct ionic *ionic)
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    struct ionic_dev_bar *bar = ionic->bars;
    unsigned int num_bars = ionic->num_bars;
    struct ionic_dev *idev = &ionic->idev;
    int size;
    u32 sig;
    NDIS_TIMER_CHARACTERISTICS stTimer;

    /* BAR0: dev_cmd and interrupts */
    if (num_bars < 1) {
        DbgTrace((TRACE_COMPONENT_DEVICE, TRACE_LEVEL_ERROR,
                  "%s No bars found, aborting\n", __FUNCTION__));
        return NDIS_STATUS_INVALID_PARAMETER;
    }

    if (bar->len < BAR0_SIZE) {
        DbgTrace((TRACE_COMPONENT_DEVICE, TRACE_LEVEL_ERROR,
                  "%s Resource bar size %d too small, aborting\n", __FUNCTION__,
                  bar->len));
        return NDIS_STATUS_INVALID_PARAMETER;
    }

	idev->ionic = ionic;

    idev->dev_info_regs =
        (dev_info_regs *)((char *)bar->vaddr + BAR0_DEV_INFO_REGS_OFFSET);
    idev->dev_cmd_regs =
        (dev_cmd_regs *)((char *)bar->vaddr + BAR0_DEV_CMD_REGS_OFFSET);
    idev->intr_status = (u64 *)((char *)bar->vaddr + BAR0_INTR_STATUS_OFFSET);
    idev->intr_ctrl =
        (ionic_intr *)((char *)bar->vaddr + BAR0_INTR_CTRL_OFFSET);

    sig = ioread32(&idev->dev_info_regs->signature);
    if (sig != IONIC_DEV_INFO_SIGNATURE) {
        DbgTrace((TRACE_COMPONENT_DEVICE, TRACE_LEVEL_ERROR,
                  "%s Incompatible firmware signature 0x%08lX", __FUNCTION__,
                  sig));
        return NDIS_STATUS_INVALID_PARAMETER;
    }

    ionic_init_devinfo(ionic);

    /* BAR1: doorbells */
    bar++;
    if (num_bars < 2) {
        DbgTrace((TRACE_COMPONENT_DEVICE, TRACE_LEVEL_ERROR,
                  "%s Doorbell bar missing, aborting\n", __FUNCTION__));
        return NDIS_STATUS_INVALID_PARAMETER;
    }

    idev->db_pages = (u64 *)bar->vaddr;
    idev->phy_db_pages = (dma_addr_t)bar->bus_addr;

    stTimer.Header.Revision = NDIS_TIMER_CHARACTERISTICS_REVISION_1;
    stTimer.Header.Size = NDIS_SIZEOF_TIMER_CHARACTERISTICS_REVISION_1;
    stTimer.Header.Type = NDIS_OBJECT_TYPE_TIMER_CHARACTERISTICS;

    stTimer.AllocationTag = IONIC_TIMER_TAG;
    stTimer.TimerFunction = ionic_watchdog_cb;
    stTimer.FunctionContext = (void *)ionic;

    status = NdisAllocateTimerObject(ionic->adapterhandle, &stTimer,
                                     &ionic->WatchDogTimer);

    if (status != NDIS_STATUS_SUCCESS) {

        return status;
    }

    idev->cmb_inuse = NULL;
    idev->phy_cmb_pages = 0;
    idev->cmb_npages = 0;

	/* Mask all interrupts at this point */
	mask_all_interrupts( ionic);

    /* BAR2: optional controller memory mapping */
    bar++;
    if (num_bars < 3) {
        return 0;
    }

    NdisAllocateSpinLock(&idev->cmb_inuse_lock);

    idev->phy_cmb_pages = bar->bus_addr;
    idev->cmb_npages = bar->len / PAGE_SIZE;
    size = BITS_TO_LONGS(idev->cmb_npages) * sizeof(long);

    idev->cmb_inuse = (unsigned long *)NdisAllocateMemoryWithTagPriority_internal(
        ionic->adapterhandle, size, IONIC_GENERIC_TAG, NormalPoolPriority);
    if (!idev->cmb_inuse) {
        idev->phy_cmb_pages = 0;
        idev->cmb_npages = 0;
    } else {
        NdisZeroMemory(idev->cmb_inuse, size);

        DbgTrace((TRACE_COMPONENT_MEMORY, TRACE_LEVEL_VERBOSE,
                  "%s Allocated 0x%p len %08lX\n",
                  __FUNCTION__,
                  idev->cmb_inuse, size));
    }

    return 0;
}

void
ionic_init_devinfo(struct ionic *Adapter)
{

    Adapter->idev.dev_info.asic_type =
        ioread8(&Adapter->idev.dev_info_regs->asic_type);
    Adapter->idev.dev_info.asic_rev =
        ioread8(&Adapter->idev.dev_info_regs->asic_rev);

    memcpy_fromio(Adapter->idev.dev_info.fw_version,
                  Adapter->idev.dev_info_regs->fw_version,
                  IONIC_DEVINFO_FWVERS_BUFLEN);

    memcpy_fromio(Adapter->idev.dev_info.serial_num,
                  Adapter->idev.dev_info_regs->serial_num,
                  IONIC_DEVINFO_SERIAL_BUFLEN);

    Adapter->idev.dev_info.fw_version[IONIC_DEVINFO_FWVERS_BUFLEN] = 0;
    Adapter->idev.dev_info.serial_num[IONIC_DEVINFO_SERIAL_BUFLEN] = 0;

    return;
}

void
ionic_dev_teardown(struct ionic *ionic)
{
    struct ionic_dev *idev = &ionic->idev;

    if (ionic->WatchDogTimer != NULL) {
        NdisFreeTimerObject(ionic->WatchDogTimer);
        ionic->WatchDogTimer = NULL;
    }

    if (idev->cmb_inuse != NULL) {
        NdisFreeMemoryWithTagPriority_internal(ionic->adapterhandle, idev->cmb_inuse,
                                      IONIC_GENERIC_TAG);

        idev->cmb_inuse = NULL;
        idev->phy_cmb_pages = 0;
        idev->cmb_npages = 0;

        NdisFreeSpinLock(&idev->cmb_inuse_lock);
    }

    return;
}

static bool
ionic_q_is_posted(struct queue *q, unsigned int pos)
{
    unsigned int mask, tail, head;

    mask = q->num_descs - 1;
    tail = q->tail->index;
    head = q->head->index;

    return ((pos - tail) & mask) < ((head - tail) & mask);
}

void
ionic_q_service(struct queue *q,
                struct cq_info *cq_info,
                unsigned int stop_index)
{
    struct desc_info *desc_info;
    void *cb_arg;
    desc_cb cb;

    /* check for empty queue */
    if (q->tail->index == q->head->index)
        return;

    /* stop index must be for a descriptor that is not yet completed */
    if (unlikely(!ionic_q_is_posted(q, stop_index))) {
        DbgTrace((TRACE_COMPONENT_DEVICE, TRACE_LEVEL_ERROR,
                  "%s ionic stop is not posted %s stop %d tail %d head %d\n",
                  __FUNCTION__, q->name, stop_index, q->tail->index,
                  q->head->index));
    }

    do {
        desc_info = q->tail;
        q->tail = desc_info->next;

        cb = desc_info->cb;
        cb_arg = desc_info->cb_arg;

        desc_info->cb = NULL;
        desc_info->cb_arg = NULL;

        if (cb)
            cb(q, desc_info, cq_info, cb_arg, NULL, NULL);
    } while (desc_info->index != stop_index);
}

bool
ionic_adminq_service(struct cq *cq, struct cq_info *cq_info, void *cb_arg)
{
    struct admin_comp *comp = (struct admin_comp *)cq_info->cq_desc;

    UNREFERENCED_PARAMETER(cb_arg);

    if (!color_match(comp->color, cq->done_color))
        return false;

    ionic_q_service(cq->bound_q, cq_info, le16_to_cpu(comp->comp_index));

    return true;
}

NDIS_STATUS
ionic_reset(struct ionic *ionic)
{
    struct ionic_dev *idev = &ionic->idev;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;

    NdisAcquireSpinLock(&ionic->dev_cmd_lock);
    ionic_dev_cmd_reset(idev);
    status = ionic_dev_cmd_wait(ionic, devcmd_timeout);
    NdisReleaseSpinLock(&ionic->dev_cmd_lock);

    return status;
}

NDIS_STATUS
ionic_identify(struct ionic *ionic)
{
    struct identity *ident = &ionic->ident;
    struct ionic_dev *idev = &ionic->idev;
    ULONG sz;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    RTL_OSVERSIONINFOW rtlVersion;

    memset(ident, 0, sizeof(*ident));

    rtlVersion.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);
    RtlGetVersion(&rtlVersion);

    ident->drv.os_type = cpu_to_le32(IONIC_OS_TYPE_WIN);
    ident->drv.os_dist = 0;

    strncpy_s(ident->drv.os_dist_str, IONIC_WINDOWS_DISTRO,
              IONIC_WINDOWS_DISTRO_LEN);
    ident->drv.kernel_ver = cpu_to_le32(
        (ULONG)(rtlVersion.dwMajorVersion << 0x10) | rtlVersion.dwMinorVersion);
    // strncpy(ident->drv.kernel_ver_str, utsname()->version,
    //	sizeof(ident->drv.kernel_ver_str) - 1);

    NdisAcquireSpinLock(&ionic->dev_cmd_lock);

    sz = min(sizeof(ident->drv), sizeof(idev->dev_cmd_regs->data));
    memcpy_toio(&idev->dev_cmd_regs->data, &ident->drv, sz);

    ionic_dev_cmd_identify(idev, IONIC_IDENTITY_VERSION_1);
    status = ionic_dev_cmd_wait(ionic, devcmd_timeout);
    if (status == NDIS_STATUS_SUCCESS) {
        sz = min(sizeof(ident->dev), sizeof(idev->dev_cmd_regs->data));
        memcpy_fromio(&ident->dev, &idev->dev_cmd_regs->data, sz);
    }

    NdisReleaseSpinLock(&ionic->dev_cmd_lock);

    return status;
}

NDIS_STATUS
ionic_init(struct ionic *ionic)
{
    struct ionic_dev *idev = &ionic->idev;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;

    NDIS_SHARED_MEMORY_PARAMETERS stParams;
    ULONG ulSGListNumElements = 0;
    PSCATTER_GATHER_LIST pSGListBuffer = NULL;
    ULONG ulSGListSize = 0;
    ULONG buffer_len = 0;
    ULONG frag_buffer_len = 0;
    u64 curr_phys_addr = 0;
    u32 curr_phys_len = 0;
    u32 curr_sg_ind = 0;
    char *curr_va = NULL;
    struct tx_frag_pool_elem *curr_elem = NULL;
    struct tx_frag_pool_elem *next_elem = NULL;
    ULONG elem_cnt = 0;

    // Alloc our tx fragmention pool

    NdisAllocateSpinLock(&ionic->tx_frag_pool_lock);

    if (ionic->tx_frag_pool_count != 0) {

        buffer_len =
            (IONIC_TX_FRAG_PAGES * PAGE_SIZE) *
            ionic->tx_frag_pool_count; // So we can handle ionic->tx_frag_pool_count
                                      // mtu sized tx buffers

        NdisZeroMemory(&stParams, sizeof(NDIS_SHARED_MEMORY_PARAMETERS));

        stParams.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
        stParams.Header.Revision = NDIS_SHARED_MEMORY_PARAMETERS_REVISION_2;
        stParams.Header.Size = NDIS_SIZEOF_SHARED_MEMORY_PARAMETERS_REVISION_2;

        stParams.Flags = 0; // NDIS_SHARED_MEM_PARAMETERS_CONTIGOUS;

        stParams.Usage = NdisSharedMemoryUsageXmit;

        stParams.PreferredNode = ionic->numa_node; //MM_ANY_NODE_OK;

        stParams.Length = buffer_len;

        ulSGListNumElements = BYTES_TO_PAGES(stParams.Length);

        ulSGListSize = sizeof(SCATTER_GATHER_LIST) +
                       (sizeof(SCATTER_GATHER_ELEMENT) * ulSGListNumElements);
        pSGListBuffer = (PSCATTER_GATHER_LIST)NdisAllocateMemoryWithTagPriority_internal(
            ionic->adapterhandle, ulSGListSize, IONIC_SG_LIST_TAG,
            NormalPoolPriority);

        if (pSGListBuffer == NULL) {
            status = NDIS_STATUS_RESOURCES;
            goto exit;
        }

        NdisZeroMemory(pSGListBuffer, ulSGListSize);

        DbgTrace((TRACE_COMPONENT_MEMORY, TRACE_LEVEL_VERBOSE,
                      "%s Allocated 0x%p len %08lX (pSGListBuffer)\n",
                      __FUNCTION__,
                      pSGListBuffer, ulSGListSize));

        pSGListBuffer->NumberOfElements = ulSGListNumElements;
        stParams.SGListBufferLength = ulSGListSize;
        stParams.SGListBuffer = pSGListBuffer;

        status = NdisAllocateSharedMemory(ionic->adapterhandle, &stParams,
                                          &ionic->tx_frag_pool_handle);

        if (status != NDIS_STATUS_SUCCESS) {
            //ASSERT(FALSE);
            //status = NDIS_STATUS_RESOURCES;

            DbgTrace((TRACE_COMPONENT_MEMORY, TRACE_LEVEL_VERBOSE,
                      "%s Allocate tx_frag_pool_handle on preferred node %u failed %#lx\n",
                      __FUNCTION__, stParams.PreferredNode, status));

            stParams.PreferredNode = MM_ANY_NODE_OK;
            status = NdisAllocateSharedMemory(ionic->adapterhandle, &stParams,
                                              &ionic->tx_frag_pool_handle);
            if (status != NDIS_STATUS_SUCCESS) {
                DbgTrace((TRACE_COMPONENT_MEMORY, TRACE_LEVEL_VERBOSE,
                          "%s Allocate tx_frag_pool_handle on any node failed %#lx\n",
                          __FUNCTION__, status));
                goto exit;
            }

            DbgTrace((TRACE_COMPONENT_MEMORY, TRACE_LEVEL_VERBOSE,
                      "%s Allocate tx_frag_pool_handle on any node succeeded\n",
                      __FUNCTION__));
        }

        DbgTrace((TRACE_COMPONENT_MEMORY, TRACE_LEVEL_VERBOSE,
                      "%s Allocated tx_frag_pool_handle\n", __FUNCTION__));

        frag_buffer_len =
            (sizeof(struct tx_frag_pool_elem) + sizeof(SCATTER_GATHER_LIST) +
             (sizeof(SCATTER_GATHER_ELEMENT) * IONIC_TX_FRAG_PAGES)) *
            ionic->tx_frag_pool_count;

        ionic->tx_frag_pool =
            (struct tx_frag_pool_elem *)NdisAllocateMemoryWithTagPriority_internal(
                ionic->adapterhandle, frag_buffer_len, IONIC_FRAG_POOL_TAG,
                NormalPoolPriority);

        if (ionic->tx_frag_pool == NULL) {
            status = NDIS_STATUS_RESOURCES;
            goto exit;
        }

        NdisZeroMemory(ionic->tx_frag_pool, frag_buffer_len);

        DbgTrace((TRACE_COMPONENT_MEMORY, TRACE_LEVEL_VERBOSE,
                      "%s Allocated 0x%p len %08lX (tx_frag_pool)\n",
                      __FUNCTION__,
                      ionic->tx_frag_pool, frag_buffer_len));

            ASSERT( pSGListBuffer->NumberOfElements == 1);

        curr_phys_addr = pSGListBuffer->Elements[curr_sg_ind].Address.QuadPart;
        curr_phys_len = pSGListBuffer->Elements[curr_sg_ind].Length;
        curr_va = (char *)stParams.VirtualAddress;
        curr_elem = ionic->tx_frag_pool;

        // Init the pool
        while (elem_cnt < ionic->tx_frag_pool_count) {

            curr_elem->buffer = (void *)curr_va;
            curr_elem->length = IONIC_TX_FRAG_PAGES * PAGE_SIZE;
            curr_elem->tx_frag_list = (PSCATTER_GATHER_LIST)(
                (char *)curr_elem + sizeof(struct tx_frag_pool_elem));

            curr_elem->tx_frag_list->NumberOfElements = IONIC_TX_FRAG_PAGES;

            for (u32 i = 0; i < IONIC_TX_FRAG_PAGES; i++) {
                curr_elem->tx_frag_list->Elements[i].Address.QuadPart =
                    curr_phys_addr;
                curr_elem->tx_frag_list->Elements[i].Length = PAGE_SIZE;

                curr_phys_len -= PAGE_SIZE;

                if (curr_phys_len == 0) {
                    curr_sg_ind++;
                    curr_phys_addr =
                        pSGListBuffer->Elements[curr_sg_ind].Address.QuadPart;
                    curr_phys_len = pSGListBuffer->Elements[curr_sg_ind].Length;
                } else {
                    curr_phys_addr += PAGE_SIZE;
                }
            }

            curr_va += (IONIC_TX_FRAG_PAGES * PAGE_SIZE);
            elem_cnt++;
            if (elem_cnt == ionic->tx_frag_pool_count) {
                break;
            }

            next_elem =
                (struct tx_frag_pool_elem *)((char *)curr_elem +
                                             (sizeof(struct tx_frag_pool_elem) +
                                              sizeof(SCATTER_GATHER_LIST) +
                                              (sizeof(SCATTER_GATHER_ELEMENT) *
                                               IONIC_TX_FRAG_PAGES)));

            curr_elem->next = next_elem;
            curr_elem = next_elem;
        }

        ionic->tx_frag_pool_head = ionic->tx_frag_pool;
        ionic->tx_frag_pool_sg_list = pSGListBuffer;
    }

    // Init the idev

    NdisAcquireSpinLock(&ionic->dev_cmd_lock);
    ionic_dev_cmd_init(idev);
    status = ionic_dev_cmd_wait(ionic, devcmd_timeout);
    NdisReleaseSpinLock(&ionic->dev_cmd_lock);

    if (status != NDIS_STATUS_SUCCESS) {
        goto exit;
    }

	/* Reset the default lif. This is during init time so only the default lif exists */
    NdisAcquireSpinLock(&ionic->dev_cmd_lock);
    ionic_dev_cmd_lif_reset(idev, 0);
    ionic_dev_cmd_wait(ionic, devcmd_timeout);
    NdisReleaseSpinLock(&ionic->dev_cmd_lock);

	/* initialize the dma interface */
	status = init_dma(ionic);
    if (status != NDIS_STATUS_SUCCESS) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_ERROR,
                  "%s Failed to init dma Error %08lX\n", __FUNCTION__, status));
        goto exit;
    }
exit:

    if (status != NDIS_STATUS_SUCCESS) {

        if (ionic->tx_frag_pool_handle != NULL) {
            NdisFreeSharedMemory(ionic->adapterhandle,
                                 ionic->tx_frag_pool_handle);
            ionic->tx_frag_pool_handle = NULL;
        }

        if (pSGListBuffer != NULL) {
            NdisFreeMemoryWithTagPriority_internal(ionic->adapterhandle, pSGListBuffer,
                                          IONIC_SG_LIST_TAG);
			ionic->tx_frag_pool_sg_list = NULL;
        }

        if (ionic->tx_frag_pool != NULL) {
            NdisFreeMemoryWithTagPriority_internal(
                ionic->adapterhandle, ionic->tx_frag_pool, IONIC_FRAG_POOL_TAG);
            ionic->tx_frag_pool = NULL;
        }
    }

    return status;
}

void
ionic_deinit(struct ionic *ionic)
{

    if (ionic->intr_msg_tbl != NULL) {
        NdisFreeMemoryWithTagPriority_internal(
            ionic->adapterhandle, ionic->intr_msg_tbl, IONIC_INT_TABLE_TAG);
        ionic->intr_msg_tbl = NULL;
    }

    if (ionic->intr_tbl != NULL) {
        NdisFreeMemoryWithTagPriority_internal(
            ionic->adapterhandle, ionic->intr_tbl, IONIC_INT_TABLE_TAG);
        ionic->intr_tbl = NULL;
    }

    if (ionic->computer_name.Buffer != NULL) {
        NdisFreeMemoryWithTagPriority_internal(ionic->adapterhandle,
                                      ionic->computer_name.Buffer,
                                      IONIC_GENERIC_TAG);
        ionic->computer_name.Buffer = NULL;
    }

    if (ionic->tx_frag_pool_handle != NULL) {
        NdisFreeSharedMemory(ionic->adapterhandle, ionic->tx_frag_pool_handle);
        ionic->tx_frag_pool_handle = NULL;
    }

    if (ionic->tx_frag_pool_sg_list != NULL) {
        NdisFreeMemoryWithTagPriority_internal(ionic->adapterhandle,
                                      ionic->tx_frag_pool_sg_list,
                                      IONIC_SG_LIST_TAG);
        ionic->tx_frag_pool_sg_list = NULL;
    }

    if (ionic->tx_frag_pool != NULL) {
        NdisFreeMemoryWithTagPriority_internal(ionic->adapterhandle, ionic->tx_frag_pool,
                                      IONIC_FRAG_POOL_TAG);
        ionic->tx_frag_pool = NULL;
    }

    if (ionic->name.Buffer != NULL) {
        NdisFreeMemory( ionic->name.Buffer,
                        ionic->name.MaximumLength,
                        0);
        ionic->name.Buffer = NULL;
    }

    return;
}

struct tx_frag_pool_elem *
get_tx_frag(struct ionic *ionic)
{

    struct tx_frag_pool_elem *elem = NULL;

    NdisAcquireSpinLock(&ionic->tx_frag_pool_lock);

    elem = ionic->tx_frag_pool_head;
    if (elem != NULL) {
        ionic->tx_frag_pool_head = elem->next;
        elem->next = NULL;
    }

    NdisReleaseSpinLock(&ionic->tx_frag_pool_lock);

    return elem;
}

void
return_tx_frag(struct ionic *ionic, struct tx_frag_pool_elem *elem)
{
	struct tx_frag_pool_elem *next_elem = NULL;

	while( elem != NULL) {

		next_elem = elem->tso_next_elem;

		elem->tso_next_elem = NULL;

		elem->tx_frag_list->NumberOfElements = IONIC_TX_FRAG_PAGES;
		for (int i = 0; i < IONIC_TX_FRAG_PAGES; ++i) {
			elem->tx_frag_list->Elements[i].Length = PAGE_SIZE;
		}

		NdisAcquireSpinLock(&ionic->tx_frag_pool_lock);

		elem->next = ionic->tx_frag_pool_head;
		ionic->tx_frag_pool_head = elem;

		NdisReleaseSpinLock(&ionic->tx_frag_pool_lock);

		elem = next_elem;
	}

    return;
}

void
ReadPCIeConfigSpace(struct ionic *ionic)
{

    PCI_COMMON_CONFIG *pPciConfig = NULL;
    ULONG ulLength = 0;
    PCI_EXPRESS_SRIOV_CAPABILITY pciSriovCaps;
    USHORT usCapabilityOffset = 0;

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s Entry for adapter %p\n", __FUNCTION__, ionic));

    pPciConfig = (PCI_COMMON_CONFIG *)NdisAllocateMemoryWithTagPriority_internal(
        ionic->adapterhandle, sizeof(PCI_COMMON_CONFIG), IONIC_GENERIC_TAG,
        NormalPoolPriority);

    if (pPciConfig == NULL) {
        goto exit;
    }

    DbgTrace((TRACE_COMPONENT_MEMORY, TRACE_LEVEL_VERBOSE,
                  "%s Allocated 0x%p len %08lX\n",
                  __FUNCTION__,
                  pPciConfig, sizeof( PCI_COMMON_CONFIG)));
    
    ulLength = NdisMGetBusData(ionic->adapterhandle, PCI_WHICHSPACE_CONFIG, 0,
                               pPciConfig, sizeof(PCI_COMMON_CONFIG));

    if (ulLength == 0) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                  "%s Failed to read data\n", __FUNCTION__));
        goto exit;
    }

    if ((pPciConfig->Status & PCI_STATUS_CAPABILITIES_LIST) == 0) {
        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                  "%s No capability list\n", __FUNCTION__));
        goto exit;
    }

    usCapabilityOffset = IONIC_ENH_CAPS_OFFSET;

    while (usCapabilityOffset != 0) {

        ulLength = NdisMGetBusData(
            ionic->adapterhandle, PCI_WHICHSPACE_CONFIG, usCapabilityOffset,
            &pciSriovCaps, sizeof(PCI_EXPRESS_ENHANCED_CAPABILITY_HEADER));

        if (ulLength == 0) {
            goto exit;
        }

        DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                  "%s Id %08lX Next %d\n", __FUNCTION__,
                  pciSriovCaps.Header.CapabilityID, pciSriovCaps.Header.Next));

        if (pciSriovCaps.Header.CapabilityID ==
            PCI_EXPRESS_SINGLE_ROOT_IO_VIRTUALIZATION_CAP_ID) {

            ulLength = NdisMGetBusData(
                ionic->adapterhandle, PCI_WHICHSPACE_CONFIG, usCapabilityOffset,
                &ionic->sriov_caps, sizeof(PCI_EXPRESS_SRIOV_CAPABILITY));

            if (ulLength == 0) {
                DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                          "%s Failed to read sriov caps data\n", __FUNCTION__));
                goto exit;
            }

            DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
                      "\tSriov caps NumVfs %d\n", ionic->sriov_caps.NumVFs));

            if (ionic->sriov_caps.NumVFs != 0) {
                SetFlag(ionic->ConfigStatus, IONIC_SRIOV_CAPABLE);
            }

            break;
        } else {
            usCapabilityOffset = pciSriovCaps.Header.Next;
        }
    }

exit:

    if (pPciConfig != NULL) {
        NdisFreeMemoryWithTagPriority_internal(ionic->adapterhandle, pPciConfig,
                                      IONIC_GENERIC_TAG);
    }

    return;
}

NDIS_STATUS
oid_set_power_state(struct ionic *ionic,
                    void *buffer,
                    ULONG buffer_len,
                    ULONG *bytes_read)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_DEVICE_POWER_STATE *state = (NDIS_DEVICE_POWER_STATE *)buffer;

    if (buffer_len < sizeof(NDIS_DEVICE_POWER_STATE)) {
        ntStatus = NDIS_STATUS_INVALID_LENGTH;
        goto cleanup;
    }

    DbgTrace((TRACE_COMPONENT_INIT, TRACE_LEVEL_VERBOSE,
              "%s Entry for adapter %p state %d\n", __FUNCTION__, ionic,
              *state));

    *bytes_read = sizeof(NDIS_DEVICE_POWER_STATE);

    if (*state < NdisDeviceStateD0 || *state > NdisDeviceStateD3) {
        ntStatus = NDIS_STATUS_INVALID_DATA;
        goto cleanup;
    }

cleanup:

    return ntStatus;
}