
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

    if (BooleanFlagOn(pParameters->Flags, NDIS_RSS_PARAM_FLAG_DISABLE_RSS) ||
		(!BooleanFlagOn(pParameters->Flags,
                       NDIS_RSS_PARAM_FLAG_HASH_INFO_UNCHANGED) && 
		 !(pParameters->HashInformation & NdisHashFunctionToeplitz))) {

        ntStatus = ionic_lif_rss_deinit(lif);
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

        // program the indirection table
        ntStatus = map_rss_cpu_ind_tbl(lif, proc_array, tbl_len);
        if (ntStatus != NDIS_STATUS_SUCCESS) {
            goto cleanup;
        }

        // program indirection table in hardware
        ntStatus = ionic_lif_rss_config(lif, lif->rss_types, lif->rss_hash_key,
            lif->rss_ind_tbl);
        if (ntStatus != NDIS_STATUS_SUCCESS) {
            goto cleanup;
        }
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
                                        lif->rss_ind_tbl);

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

    *bytes_read = sizeof(NDIS_RECEIVE_HASH_PARAMETERS);
    
	if (!BooleanFlagOn(pParameters->Flags,
                       NDIS_RECEIVE_HASH_FLAG_ENABLE_HASH)) {

		ntStatus = ionic_lif_rss_deinit(lif);
        goto cleanup;
    }


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
                                        lif->rss_ind_tbl);

        if (ntStatus != NDIS_STATUS_SUCCESS) {
            goto cleanup;
        }

        *bytes_read += key_len;
        lif->rss_hash_flags = NDIS_RECEIVE_HASH_FLAG_ENABLE_HASH;
    }

cleanup:

    return ntStatus;
}

NDIS_STATUS
ionic_lif_rss_init(struct lif *lif)
{
    struct ionic* ionic = lif->ionic;
    NDIS_STATUS status;
    static const u8 key[IONIC_RSS_HASH_KEY_SIZE] = {
        0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
        0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
        0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
        0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A,
        0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A, 0x6D, 0x5A
    };

    NdisMoveMemory(lif->rss_hash_key, key, IONIC_RSS_HASH_KEY_SIZE);

    ULONG rss_proc_cnt = min(lif->nrxqs, ionic->sys_proc_info->NumberOfProcessors);

    /* select one preferred processor per queue */
    PPROCESSOR_NUMBER proc_array = (PPROCESSOR_NUMBER)NdisAllocateMemoryWithTagPriority_internal(
        ionic->adapterhandle,
        rss_proc_cnt * sizeof(PROCESSOR_NUMBER),
        IONIC_RSS_INDIR_TBL_TAG,
        NormalPoolPriority);
    if (proc_array == NULL) {
        IoPrint("%s failed to allocate processor array\n", __FUNCTION__);
        goto err_out;
    }

	NdisZeroMemory(proc_array, rss_proc_cnt * sizeof(PROCESSOR_NUMBER));

    ULONG proc_idx = 0;
    for (ULONG i = 0; i < rss_proc_cnt; ++i) {

        /* select the next preferred processor */
        while (ionic->sys_proc[proc_idx].NodeDistance != 0) {
            ++proc_idx %= rss_proc_cnt;
        }
        proc_array[i].Group = ionic->sys_proc[proc_idx].ProcNum.Group;
        proc_array[i].Number = ionic->sys_proc[proc_idx].ProcNum.Number;
        ++proc_idx %= rss_proc_cnt;
        IoPrint("%s lif %d proc_array[%d] %p group = %d proc = %d\n",
            __FUNCTION__, lif->index,
            i, &proc_array[i],
            proc_array[i].Group, proc_array[i].Number);
    }

    // program the indirection table
    status = map_rss_cpu_ind_tbl(lif, proc_array, rss_proc_cnt);
    if (status != NDIS_STATUS_SUCCESS) {
        IoPrint("%s failed to map indirection table\n", __FUNCTION__);
        goto err_out_free_proc_array;
    }

err_out_free_proc_array:
    NdisFreeMemoryWithTagPriority_internal(ionic->adapterhandle, proc_array, IONIC_RSS_INDIR_TBL_TAG);

err_out:
    lif->rss_types = IONIC_RSS_TYPE_IPV4 | IONIC_RSS_TYPE_IPV4_TCP |
                     IONIC_RSS_TYPE_IPV4_UDP | IONIC_RSS_TYPE_IPV6 |
                     IONIC_RSS_TYPE_IPV6_TCP | IONIC_RSS_TYPE_IPV6_UDP;

    return ionic_lif_rss_config(lif, lif->rss_types, lif->rss_hash_key,
                                lif->rss_ind_tbl);
}

NDIS_STATUS
ionic_lif_rss_deinit(struct lif *lif)
{
    /* Disable RSS on the NIC */
	lif->rss_hash_flags = 0;
    NdisZeroMemory(lif->rss_hash_key, IONIC_RSS_HASH_KEY_SIZE);
    NdisZeroMemory(lif->rss_ind_tbl, lif->rss_ind_tbl_sz);
   
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

    pParams->NumberOfIndirectionTableEntries = ionic->ident.lif.eth.rss_ind_tbl_sz;

    *bytes_written = sizeof(NDIS_RECEIVE_SCALE_CAPABILITIES);

cleanup:

    return ntStatus;
}

static NDIS_STATUS
check_rss_cpu_ind_tbl(struct lif *lif, PPROCESSOR_NUMBER proc_array, ULONG tbl_len)
{
    ULONG n_uniq_procs = 0;
    PPROCESSOR_NUMBER proc;
    RTL_BITMAP proc_used;
    char buffer[BITS_TO_LONGS(INTR_CTRL_REGS_MAX)] = {0};

    RtlInitializeBitMap(&proc_used, (PULONG)buffer, INTR_CTRL_REGS_MAX);

    /* Make sure we have an msi-x message affinitized to each processor in the table */
    for (ULONG i = 0; i < tbl_len; ++i) {
    
        proc = &proc_array[i % tbl_len];

        ULONG proc_idx = KeGetProcessorIndexFromNumber(proc);
        if (proc_idx == INVALID_PROCESSOR_INDEX) {
            IoPrint("%s invalid group %d proc %d\n",
                __FUNCTION__, proc->Group, proc->Number);
            return NDIS_STATUS_INVALID_PARAMETER;
        }

        IoPrint("%s lif %d proc_array[%d] %p group %d proc %d proc_idx %d\n",
            __FUNCTION__, lif->index,
            i, proc, proc->Group, proc->Number, proc_idx);

        if (find_intr_msg(lif->ionic, proc_idx) == NULL) {
            IoPrint("%s did not find any interrupts assigned to processor %d\n",
                __FUNCTION__, proc_idx);
            return NDIS_STATUS_INVALID_PARAMETER;
        }

        if (!RtlCheckBit(&proc_used, proc_idx)) {
            RtlSetBit(&proc_used, proc_idx);
            ++n_uniq_procs;
        }
    }

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
map_rss_cpu_ind_tbl(struct lif *lif, PPROCESSOR_NUMBER proc_array, ULONG tbl_len)
{
    ULONG q_idx = 0;
    ULONG q_reuse_idx = 0;
    NDIS_STATUS status;
    struct intr_msg* intr_msg;
    PPROCESSOR_NUMBER proc;
    RTL_BITMAP proc_used;
    char buffer[BITS_TO_LONGS(INTR_CTRL_REGS_MAX)] = { 0 };
    u8 proc_to_q_map[INTR_CTRL_REGS_MAX] = { 0 };

    // start with all msgs unused
    unuse_intr_msgs_rss(lif->ionic, lif);

    // validate the processor indirection table
    status = check_rss_cpu_ind_tbl(lif, proc_array, tbl_len);
    if (status != NDIS_STATUS_SUCCESS) {
        return status;
    }

    NdisZeroMemory(lif->rss_ind_tbl, lif->rss_ind_tbl_sz);

    RtlInitializeBitMap(&proc_used, (PULONG)buffer, INTR_CTRL_REGS_MAX);

    // map processors to queues
    for (ULONG i = 0; i < lif->ionic->ident.lif.eth.rss_ind_tbl_sz; ++i) {
        proc = &proc_array[i % tbl_len];

        ULONG proc_idx = KeGetProcessorIndexFromNumber(proc);
        if (proc_idx == INVALID_PROCESSOR_INDEX) {
            IoPrint("%s Invalid processor - proc_array[%d] %p group %d proc %d\n",
                    __FUNCTION__, i, proc, proc->Group, proc->Number);
            return NDIS_STATUS_INVALID_PARAMETER;
        }
        IoPrint("%s proc_array[%d] %p group %d proc %d proc_idx %d\n",
                __FUNCTION__, i, &proc_array[i],
                proc->Group, proc->Number, proc_idx);

        if (RtlCheckBit(&proc_used, proc_idx)) {
            // this proc index is already assigned to a queue
            lif->rss_ind_tbl[i] = proc_to_q_map[proc_idx];
        } else if (q_idx == lif->nrxqs) {
            // no available queues, send flows to a different rss cpu
            lif->rss_ind_tbl[i] = (u8)q_reuse_idx;
            proc_to_q_map[proc_idx] = (u8)q_reuse_idx;
            q_reuse_idx++;
            RtlSetBit(&proc_used, proc_idx);
        } else {
            // this proc index not assigned, and there is a queue available
            lif->rss_ind_tbl[i] = (u8)q_idx;
            proc_to_q_map[proc_idx] = (u8)q_idx;

            // find a message with matching cpu affinity
            intr_msg = find_intr_msg(lif->ionic, proc_idx);
            if (intr_msg == NULL) {
                IoPrint("%s did not find interrupt message affinitized to proc %d\n",
                        __FUNCTION__, proc_idx);
                return NDIS_STATUS_INVALID_PARAMETER;
            }

            intr_msg->inuse = true;

            // update the table for entry to use the msg
            struct intr_sync_ctx ctx = { 0 };
            ctx.ionic = lif->ionic;
            ctx.id = intr_msg->id;
            ctx.index = lif->rxqcqs[q_idx].qcq->cq.bound_intr->index;

            NdisMSynchronizeWithInterruptEx(lif->ionic->intr_obj,
                                            intr_msg->id, sync_intr_msg, &ctx);

            q_idx++;
            RtlSetBit(&proc_used, proc_idx);
        }
    }

    // for remaining queues not in rss indir table, assocaite any unsed msg
    for (; q_idx < lif->nrxqs; ++q_idx) {
        // find any unused message, don't care about affinity
        intr_msg = find_intr_msg(lif->ionic, ANY_PROCESSOR_INDEX);
        if (intr_msg == NULL) {
            IoPrint("%s did not find any unused interrupt message\n",
                    __FUNCTION__);
            return NDIS_STATUS_INVALID_PARAMETER;
        }

        intr_msg->inuse = true;

        // update the table for this entry to use the msg
        struct intr_sync_ctx ctx = { 0 };
        ctx.ionic = lif->ionic;
        ctx.id = intr_msg->id;
        ctx.index = lif->rxqcqs[q_idx].qcq->cq.bound_intr->index;

        NdisMSynchronizeWithInterruptEx(lif->ionic->intr_obj,
                                        intr_msg->id, sync_intr_msg, &ctx);
    }

    // During update, interrupts may have been serviced for the wrong queue
    // as not all messages are updated at the same time.
    //
    // Invoke the service routine once again for each message now inuse.
    invoke_intr_msgs_rss(lif->ionic, lif);

    return NDIS_STATUS_SUCCESS;
}