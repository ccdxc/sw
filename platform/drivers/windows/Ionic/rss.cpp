
#include "common.h"

NDIS_STATUS
oid_set_rss_parameters(struct ionic *ionic,
                       void *data_buffer,
                       ULONG data_buffer_length,
                       ULONG *bytes_read,
                       ULONG *bytes_needed)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_RECEIVE_SCALE_PARAMETERS *pParameters =
        (NDIS_RECEIVE_SCALE_PARAMETERS *)data_buffer;
    struct lif *lif = ionic->master_lif;
    int tbl_len = 0;
    int key_len = 0;
    u8 proc_num = 0;
    int tbl_indx = 0;

    if (!BooleanFlagOn(ionic->ConfigStatus, IONIC_RSS_ENABLED)) {
        ntStatus = NDIS_STATUS_NOT_SUPPORTED;
        goto cleanup;
    }

#if (NDIS_SUPPORT_NDIS660)
    if (data_buffer_length < NDIS_SIZEOF_RECEIVE_SCALE_PARAMETERS_REVISION_3 +
                                 pParameters->IndirectionTableSize +
                                 pParameters->HashSecretKeySize) {
#else
    if (data_buffer_length < NDIS_SIZEOF_RECEIVE_SCALE_PARAMETERS_REVISION_2 +
                                 pParameters->IndirectionTableSize +
                                 pParameters->HashSecretKeySize) {
#endif
        ntStatus = NDIS_STATUS_BUFFER_TOO_SHORT;
        *bytes_needed = NDIS_SIZEOF_RECEIVE_SCALE_PARAMETERS_REVISION_2 +
                        pParameters->IndirectionTableSize +
                        pParameters->HashSecretKeySize;
        goto cleanup;
    }

    DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE,
              "%s For %p Flags %08lX\n", __FUNCTION__, ionic,
              pParameters->Flags));

    if (pParameters->HashInformation != 0) {
        if ((pParameters->HashInformation &
             (NDIS_HASH_IPV4 | NDIS_HASH_TCP_IPV4 | NDIS_HASH_IPV6 |
              NDIS_HASH_TCP_IPV6 | NdisHashFunctionToeplitz)) !=
            pParameters->HashInformation) {
            ntStatus = NDIS_STATUS_INVALID_PARAMETER;
            goto cleanup;
        }
        if (!(pParameters->HashInformation &
              (NDIS_HASH_IPV4 | NDIS_HASH_TCP_IPV4 | NDIS_HASH_IPV6 |
               NDIS_HASH_TCP_IPV6))) {
            ntStatus = NDIS_STATUS_INVALID_PARAMETER;
            goto cleanup;
        }
    }

#if (NDIS_SUPPORT_NDIS660)
    *bytes_read = NDIS_SIZEOF_RECEIVE_SCALE_PARAMETERS_REVISION_3 +
                  pParameters->IndirectionTableSize +
                  pParameters->HashSecretKeySize;
#else
    *bytes_read = NDIS_SIZEOF_RECEIVE_SCALE_PARAMETERS_REVISION_2 +
                  pParameters->IndirectionTableSize +
                  pParameters->HashSecretKeySize;
#endif

    *bytes_needed = 0;

    if (BooleanFlagOn(pParameters->Flags, NDIS_RSS_PARAM_FLAG_DISABLE_RSS)) {

        lif->rss_hash_flags = 0;

        NdisZeroMemory(lif->rss_hash_key, IONIC_RSS_HASH_KEY_SIZE);
        NdisZeroMemory(lif->rss_ind_tbl, lif->rss_ind_tbl_sz);
        NdisZeroMemory(lif->rss_ind_tbl_mapped, lif->rss_ind_tbl_sz);

        ntStatus = ionic_lif_rss_config(lif, 0, NULL, NULL);

        if (ntStatus != NDIS_STATUS_SUCCESS) {
            goto cleanup;
        }

        goto cleanup;
    }

    SetFlag(ionic->ConfigStatus, IONIC_RSS_ENABLED);

    if (!BooleanFlagOn(pParameters->Flags,
                       NDIS_RSS_PARAM_FLAG_BASE_CPU_UNCHANGED)) {
        lif->rss_base_cpu = (u8)pParameters->BaseCpuNumber;
    }
    if (!BooleanFlagOn(pParameters->Flags,
                       NDIS_RSS_PARAM_FLAG_HASH_INFO_UNCHANGED)) {
        lif->rss_types = ionic_convert_ndis_hash_type(
            NDIS_RSS_HASH_TYPE_FROM_HASH_INFO(pParameters->HashInformation));
    }
    if (!BooleanFlagOn(pParameters->Flags,
                       NDIS_RSS_PARAM_FLAG_ITABLE_UNCHANGED)) {

        tbl_len =
            pParameters->IndirectionTableSize / sizeof(PROCESSOR_NUMBER) >
                    lif->rss_ind_tbl_sz
                ? lif->rss_ind_tbl_sz
                : pParameters->IndirectionTableSize / sizeof(PROCESSOR_NUMBER);

        PPROCESSOR_NUMBER proc_array = (PPROCESSOR_NUMBER)(
            ((UCHAR *)pParameters) + pParameters->IndirectionTableOffset);

        for (tbl_indx = 0; tbl_indx < tbl_len; tbl_indx++) {

            proc_num =
                (UCHAR)KeGetProcessorIndexFromNumber(proc_array + tbl_indx);

            if (proc_num >= (u8)ionic->proc_count) {
                ntStatus = NDIS_STATUS_INVALID_PARAMETER;
                goto cleanup;
            }
        }

        NdisZeroMemory(lif->rss_ind_tbl, lif->rss_ind_tbl_sz);

        NdisZeroMemory(lif->rss_ind_tbl_mapped, lif->rss_ind_tbl_sz);

        for (tbl_indx = 0; tbl_indx < tbl_len; tbl_indx++) {
            lif->rss_ind_tbl[tbl_indx] =
                (UCHAR)KeGetProcessorIndexFromNumber(proc_array + tbl_indx);
        }

        if (tbl_len < (int)lif->rss_ind_tbl_sz) {

            for (; tbl_indx < (int)lif->rss_ind_tbl_sz; tbl_indx++) {
                lif->rss_ind_tbl[tbl_indx] =
                    lif->rss_ind_tbl[tbl_indx % tbl_len];
            }
        }

        // Map the indirection table
        ntStatus = map_rss_table(lif);
        if (ntStatus != NDIS_STATUS_SUCCESS) {
            goto cleanup;
        }

        ntStatus = ionic_lif_rss_config(lif, lif->rss_types, lif->rss_hash_key,
                                        lif->rss_ind_tbl_mapped);
        if (ntStatus != NDIS_STATUS_SUCCESS) {
            goto cleanup;
        }

        // May need to update the affinity for the default port rx queues
        remap_rss_rx_affinity(lif);
    }
    if (!BooleanFlagOn(pParameters->Flags,
                       NDIS_RSS_PARAM_FLAG_HASH_KEY_UNCHANGED)) {

        NdisZeroMemory(lif->rss_hash_key, IONIC_RSS_HASH_KEY_SIZE);

        key_len = pParameters->HashSecretKeySize > IONIC_RSS_HASH_KEY_SIZE
                      ? IONIC_RSS_HASH_KEY_SIZE
                      : pParameters->HashSecretKeySize;

        NdisMoveMemory(
            lif->rss_hash_key,
            ((UCHAR *)pParameters) + pParameters->HashSecretKeyOffset, key_len);

        ntStatus = ionic_lif_rss_config(lif, lif->rss_types, lif->rss_hash_key,
                                        lif->rss_ind_tbl_mapped);

        if (ntStatus != NDIS_STATUS_SUCCESS) {
            goto cleanup;
        }

        lif->rss_hash_flags = NDIS_RECEIVE_HASH_FLAG_ENABLE_HASH;
    }

cleanup:

    return ntStatus;
}

NDIS_STATUS
oid_set_rss_hash(struct ionic *ionic,
                 void *data_buffer,
                 ULONG data_buffer_length,
                 ULONG *bytes_read,
                 ULONG *bytes_needed)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_RECEIVE_HASH_PARAMETERS *pParameters =
        (NDIS_RECEIVE_HASH_PARAMETERS *)data_buffer;
    struct lif *lif = ionic->master_lif;
    int key_len = 0;

    if (!BooleanFlagOn(ionic->ConfigStatus, IONIC_RSS_ENABLED)) {
        ntStatus = NDIS_STATUS_NOT_SUPPORTED;
        goto cleanup;
    }

    if (data_buffer_length < sizeof(NDIS_RECEIVE_HASH_PARAMETERS) ||
        data_buffer_length <
            pParameters->HashSecretKeyOffset + pParameters->HashSecretKeySize) {
        ntStatus = NDIS_STATUS_BUFFER_TOO_SHORT;
        *bytes_needed = sizeof(NDIS_RECEIVE_HASH_PARAMETERS);
        goto cleanup;
    }

    if (pParameters->HashInformation != 0) {
        if ((pParameters->HashInformation &
             (NDIS_HASH_IPV4 | NDIS_HASH_TCP_IPV4 | NDIS_HASH_IPV6 |
              NDIS_HASH_TCP_IPV6 | NdisHashFunctionToeplitz)) !=
            pParameters->HashInformation) {
            ntStatus = NDIS_STATUS_INVALID_PARAMETER;
            goto cleanup;
        }

        if (!(pParameters->HashInformation &
              (NDIS_HASH_IPV4 | NDIS_HASH_TCP_IPV4 | NDIS_HASH_IPV6 |
               NDIS_HASH_TCP_IPV6))) {
            ntStatus = NDIS_STATUS_INVALID_PARAMETER;
            goto cleanup;
        }
    }

    DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE,
              "%s For %p Flags %08lX HashInfo %08lX\n", __FUNCTION__, ionic,
              pParameters->Flags, pParameters->HashInformation));

    if (!BooleanFlagOn(pParameters->Flags,
                       NDIS_RECEIVE_HASH_FLAG_ENABLE_HASH)) {

        NdisZeroMemory(lif->rss_hash_key, IONIC_RSS_HASH_KEY_SIZE);
        NdisZeroMemory(lif->rss_ind_tbl, lif->rss_ind_tbl_sz);
        NdisZeroMemory(lif->rss_ind_tbl_mapped, lif->rss_ind_tbl_sz);

        lif->rss_hash_flags = 0;

        ntStatus = ionic_lif_rss_config(lif, 0, NULL, NULL);

        if (ntStatus != NDIS_STATUS_SUCCESS) {
            goto cleanup;
        }

        *bytes_read = sizeof(NDIS_RECEIVE_HASH_PARAMETERS);

        goto cleanup;
    }

    *bytes_read = sizeof(NDIS_RECEIVE_HASH_PARAMETERS);

    if (!BooleanFlagOn(pParameters->Flags,
                       NDIS_RECEIVE_HASH_FLAG_HASH_INFO_UNCHANGED) ||
        BooleanFlagOn(pParameters->Flags, NDIS_RECEIVE_HASH_FLAG_ENABLE_HASH)) {
        lif->rss_types = ionic_convert_ndis_hash_type(
            NDIS_RSS_HASH_TYPE_FROM_HASH_INFO(pParameters->HashInformation));
        lif->rss_types_raw =
            NDIS_RSS_HASH_TYPE_FROM_HASH_INFO(pParameters->HashInformation);
    }

    if (!BooleanFlagOn(pParameters->Flags,
                       NDIS_RECEIVE_HASH_FLAG_HASH_KEY_UNCHANGED) ||
        BooleanFlagOn(pParameters->Flags, NDIS_RECEIVE_HASH_FLAG_ENABLE_HASH)) {

        NdisZeroMemory(lif->rss_hash_key, IONIC_RSS_HASH_KEY_SIZE);

        key_len = pParameters->HashSecretKeySize > IONIC_RSS_HASH_KEY_SIZE
                      ? IONIC_RSS_HASH_KEY_SIZE
                      : pParameters->HashSecretKeySize;

        NdisMoveMemory(
            lif->rss_hash_key,
            ((UCHAR *)pParameters) + pParameters->HashSecretKeyOffset, key_len);

        lif->rss_hash_key_len = (u16)key_len;

        ntStatus = ionic_lif_rss_config(lif, lif->rss_types, lif->rss_hash_key,
                                        lif->rss_ind_tbl_mapped);

        if (ntStatus != NDIS_STATUS_SUCCESS) {
            goto cleanup;
        }

        *bytes_read += key_len;
        lif->rss_hash_flags = NDIS_RECEIVE_HASH_FLAG_ENABLE_HASH;
    }

cleanup:

    return ntStatus;
}

void
netdev_rss_key_fill(void *buffer, size_t len)
{
    /* Set of random keys generated using kernel random number generator */
    static const u8 seed[NETDEV_RSS_KEY_LEN] = {
        0xE6, 0xFA, 0x35, 0x62, 0x95, 0x12, 0x3E, 0xA3, 0xFB, 0x46, 0xC1,
        0x5F, 0xB1, 0x43, 0x82, 0x5B, 0x6A, 0x49, 0x50, 0x95, 0xCD, 0xAB,
        0xD8, 0x11, 0x8F, 0xC5, 0xBD, 0xBC, 0x6A, 0x4A, 0xB2, 0xD4, 0x1F,
        0xFE, 0xBC, 0x41, 0xBF, 0xAC, 0xB2, 0x9A, 0x8F, 0x70, 0xE9, 0x2A,
        0xD7, 0xB2, 0x80, 0xB6, 0x5B, 0xAA, 0x9D, 0x20};

    NdisMoveMemory(buffer, seed, len);

    return;
}

u8
ethtool_rxfh_indir_default(u32 index, u32 n_rx_rings)
{
    return (u8)(index % n_rx_rings);
}

NDIS_STATUS
ionic_lif_rss_init(struct lif *lif)
{
    u8 rss_key[IONIC_RSS_HASH_KEY_SIZE];

    netdev_rss_key_fill(rss_key, IONIC_RSS_HASH_KEY_SIZE);

    lif->rss_types = IONIC_RSS_TYPE_IPV4 | IONIC_RSS_TYPE_IPV4_TCP |
                     IONIC_RSS_TYPE_IPV4_UDP | IONIC_RSS_TYPE_IPV6 |
                     IONIC_RSS_TYPE_IPV6_TCP | IONIC_RSS_TYPE_IPV6_UDP;

    return ionic_lif_rss_config(lif, lif->rss_types, rss_key,
                                lif->rss_ind_tbl_mapped);
}

NDIS_STATUS
ionic_lif_rss_deinit(struct lif *lif)
{
	int tbl_sz = 0;

    /* Disable RSS on the NIC */
	tbl_sz = le16_to_cpu(lif->ionic->ident.lif.eth.rss_ind_tbl_sz);
	NdisZeroMemory(lif->rss_ind_tbl, tbl_sz);
	NdisZeroMemory(lif->rss_hash_key, IONIC_RSS_HASH_KEY_SIZE);

    return ionic_lif_rss_config(lif, 0x0, NULL, NULL);
}

u16
ionic_convert_ndis_hash_type(ULONG ndis_hash_type)
{
    u16 hash_type = 0;

    hash_type = ((ndis_hash_type & NDIS_HASH_IPV4) ? IONIC_RSS_TYPE_IPV4 : 0) |
                ((ndis_hash_type & NDIS_HASH_TCP_IPV4)
                     ? IONIC_RSS_TYPE_IPV4_TCP | IONIC_RSS_TYPE_IPV4
                     : 0) |
                ((ndis_hash_type & NDIS_HASH_IPV6) ? IONIC_RSS_TYPE_IPV6 : 0) |
                ((ndis_hash_type & NDIS_HASH_TCP_IPV6)
                     ? IONIC_RSS_TYPE_IPV6_TCP | IONIC_RSS_TYPE_IPV6
                     : 0); // |
    //((ndis_hash_type & NDIS_HASH_IPV6_EX) ? IONIC_RSS_TYPE_IPV6_EX : 0) |
    //((ndis_hash_type & NDIS_HASH_TCP_IPV6_EX) ? IONIC_RSS_TYPE_IPV6_TCP_EX :
    // 0);

    return hash_type;
}

NDIS_STATUS
oid_query_rss_hash(struct ionic *ionic,
                   void *info_buffer,
                   ULONG info_buffer_length,
                   ULONG *bytes_needed,
                   ULONG *bytes_written)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_RECEIVE_HASH_PARAMETERS *pParams =
        (NDIS_RECEIVE_HASH_PARAMETERS *)info_buffer;
    struct lif *lif = ionic->master_lif;

    if (info_buffer_length <
        sizeof(NDIS_RECEIVE_HASH_PARAMETERS) + IONIC_RSS_HASH_KEY_SIZE) {
        *bytes_needed =
            sizeof(NDIS_RECEIVE_HASH_PARAMETERS) + IONIC_RSS_HASH_KEY_SIZE;
        ntStatus = NDIS_STATUS_BUFFER_TOO_SHORT;
        goto cleanup;
    }

    pParams->Header.Revision = NDIS_RECEIVE_HASH_PARAMETERS_REVISION_1;
    pParams->Header.Size = NDIS_SIZEOF_RECEIVE_HASH_PARAMETERS_REVISION_1;
    pParams->Header.Type = NDIS_OBJECT_TYPE_DEFAULT;

    pParams->Flags = 0;
    pParams->HashInformation = NDIS_RSS_HASH_INFO_FROM_TYPE_AND_FUNC(
        lif->rss_types_raw, NdisHashFunctionToeplitz);

    pParams->HashSecretKeySize = lif->rss_hash_key_len;

    pParams->HashSecretKeyOffset = sizeof(NDIS_RECEIVE_HASH_PARAMETERS);

    NdisMoveMemory((void *)((char *)pParams + pParams->HashSecretKeyOffset),
                   lif->rss_hash_key, lif->rss_hash_key_len);

    *bytes_written =
        sizeof(NDIS_RECEIVE_HASH_PARAMETERS) + lif->rss_hash_key_len;

cleanup:

    return ntStatus;
}

NDIS_STATUS
oid_query_rss_caps(struct ionic *ionic,
                   void *info_buffer,
                   ULONG info_buffer_length,
                   ULONG *bytes_needed,
                   ULONG *bytes_written)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    NDIS_RECEIVE_SCALE_CAPABILITIES *pParams =
        (NDIS_RECEIVE_SCALE_CAPABILITIES *)info_buffer;

    if (info_buffer_length < sizeof(NDIS_RECEIVE_SCALE_CAPABILITIES)) {
        *bytes_needed = sizeof(NDIS_RECEIVE_SCALE_CAPABILITIES);
        ntStatus = NDIS_STATUS_BUFFER_TOO_SHORT;
        goto cleanup;
    }

    pParams->Header.Revision = NDIS_RECEIVE_SCALE_CAPABILITIES_REVISION_3;
    pParams->Header.Size = NDIS_SIZEOF_RECEIVE_SCALE_CAPABILITIES_REVISION_3;
    pParams->Header.Type = NDIS_OBJECT_TYPE_RSS_CAPABILITIES;

    pParams->CapabilitiesFlags =
        NDIS_RSS_CAPS_MESSAGE_SIGNALED_INTERRUPTS |
        NDIS_RSS_CAPS_CLASSIFICATION_AT_ISR | NDIS_RSS_CAPS_USING_MSI_X |
        NDIS_RSS_CAPS_SUPPORTS_MSI_X | NDIS_RSS_CAPS_HASH_TYPE_TCP_IPV4 |
        NDIS_RSS_CAPS_HASH_TYPE_TCP_IPV6 | NdisHashFunctionToeplitz;
    pParams->NumberOfInterruptMessages = ionic->ident.dev.nintrs;

    if (ionic->num_rss_queues != 0) {
        pParams->NumberOfReceiveQueues = ionic->num_rss_queues;
    } else {
        pParams->NumberOfReceiveQueues = ionic->nrxqs_per_lif;
    }

    pParams->NumberOfIndirectionTableEntries = 128;

    *bytes_written = sizeof(NDIS_RECEIVE_SCALE_CAPABILITIES);

cleanup:

    return ntStatus;
}

NDIS_STATUS
map_rss_table(struct lif *lif)
{

    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    u8 *rss_tbl = NULL;
    u8 *mapped_rss_tbl = NULL;
    u32 queue_indx = 0;
    u32 proc_indx = 0;
    u32 rss_indx = 0;
    struct rss_map *current_map = NULL;

    NdisZeroMemory(lif->rss_ind_tbl_mapped, lif->rss_ind_tbl_sz);

    if (lif->rss_mapping == NULL) {
        lif->rss_mapping = (struct rss_map *)NdisAllocateMemoryWithTagPriority_internal(
            lif->ionic->adapterhandle,
            sizeof(struct rss_map) * lif->ionic->proc_count, IONIC_GENERIC_TAG,
            NormalPoolPriority);
        if (lif->rss_mapping == NULL) {
            status = NDIS_STATUS_RESOURCES;
            goto cleanup;
        }
    }

    NdisZeroMemory(lif->rss_mapping,
                   sizeof(struct rss_map) * lif->ionic->proc_count);

    DbgTrace((TRACE_COMPONENT_MEMORY, TRACE_LEVEL_VERBOSE,
                  "%s Allocated 0x%p len %08lX\n",
                  __FUNCTION__,
                  lif->rss_mapping,
                  sizeof(struct rss_map) * lif->ionic->proc_count));
    
    current_map = lif->rss_mapping;

    // Go through the provided indirection table, indicating which processors
    // are being requested to be used

    rss_tbl = lif->rss_ind_tbl;

    for (rss_indx = 0; rss_indx < lif->rss_ind_tbl_sz; rss_indx++, rss_tbl++) {
        current_map = lif->rss_mapping + *rss_tbl;
        current_map->ref = TRUE;
        current_map->queue_id = (ULONG)-1;
    }

    current_map = lif->rss_mapping;
    queue_indx = 0;

    // Get the next cpu id that was referenced in the IT
    for (proc_indx = 0; proc_indx < lif->ionic->proc_count;
         proc_indx++, current_map++) {

        if (current_map->ref) {

            if (current_map->queue_id == (ULONG)-1) {
                current_map->queue_id = queue_indx;

                queue_indx++;
                if (queue_indx == lif->nrxqs) {
                    queue_indx = 0;
                }
            }
        }
    }

    // Now go through the IT and update each element to be the mapped rx id
    rss_tbl = lif->rss_ind_tbl;
    mapped_rss_tbl = lif->rss_ind_tbl_mapped;

    for (rss_indx = 0; rss_indx < lif->rss_ind_tbl_sz;
         rss_indx++, rss_tbl++, mapped_rss_tbl++) {
        // Locate the processor entry
        current_map = lif->rss_mapping + *rss_tbl;
        ASSERT(current_map->ref);
        *mapped_rss_tbl = (u8)current_map->queue_id;
    }

cleanup:

    return status;
}

u32
get_rss_affinity(struct lif *lif, u32 queue_id)
{

    u32 proc_indx = (u32)-1;
    struct rss_map *current_map = NULL;

    if (lif->rss_mapping == NULL) {
        goto cleanup;
    }

    current_map = lif->rss_mapping;
    for (proc_indx = 0; proc_indx < lif->ionic->proc_count;
         proc_indx++, current_map++) {

        if (current_map->ref && current_map->queue_id == queue_id) {
            break;
        }
    }

    if (proc_indx == lif->ionic->proc_count) {
        proc_indx = (ULONG)-1;
    }

cleanup:

    return proc_indx;
}

void
remap_rss_rx_affinity(struct lif *lif)
{

    u32 procIndex = 0;
    struct interrupt_info *int_tbl = NULL;
    PROCESSOR_NUMBER procNumber;

    for (u32 i = 0; i < lif->nrxqs; i++) {

        procIndex = get_rss_affinity(lif, i);

        int_tbl = get_interrupt(lif->ionic, lif->rxqcqs[i].qcq->intr.index);

        DbgTrace((TRACE_COMPONENT_RSS_PROCESSING, TRACE_LEVEL_VERBOSE,                          
                "%s Rx %d on core %d %d\n", __FUNCTION__, i, procIndex,
                        int_tbl->original_proc));

        ASSERT(int_tbl != NULL);

        if ((procIndex != (ULONG)-1 && int_tbl->current_proc != procIndex) ||
            (procIndex == (ULONG)-1 &&
             int_tbl->current_proc != int_tbl->original_proc)) {

            if (procIndex == (ULONG)-1) {

                DbgTrace((TRACE_COMPONENT_RSS_PROCESSING, TRACE_LEVEL_VERBOSE,
                          "%s Rx Id %d Remapped %d to %d curr %d\n",
                          __FUNCTION__, i, lif->rxqcqs[i].qcq->intr.index,
                          int_tbl->original_proc, int_tbl->current_proc));

                int_tbl->current_proc = int_tbl->original_proc;
            } else {

                DbgTrace((TRACE_COMPONENT_RSS_PROCESSING, TRACE_LEVEL_VERBOSE,
                          "%s Rx Id %d Mapped %d to %d curr %d Orig %d\n",
                          __FUNCTION__, i, lif->rxqcqs[i].qcq->intr.index,
                          procIndex, int_tbl->current_proc,
                          int_tbl->original_proc));

                int_tbl->current_proc = procIndex;

                KeGetProcessorNumberFromIndex(procIndex, &procNumber);

                int_tbl->group = procNumber.Group;
                int_tbl->group_proc = procNumber.Number;

                SetFlag(int_tbl->Flags, IONIC_TARGET_PROC_CHANGED);
            }
        }
    }

    return;
}