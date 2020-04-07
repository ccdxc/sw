
#include "common.h"
#include "Ndischimney.h"

#if 0
// Ionic_Statistics - Ionic_Statistics
static const NDIS_GUID supportedGUIDs[] =
{
    { Ionic_LoggingGuid,    OID_IONIC_CUSTOM_1, Ionic_Logging_SIZE, fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ | fNDIS_GUID_ALLOW_WRITE },
    { Ionic_StatisticsGuid, OID_IONIC_CUSTOM_2, Ionic_Statistics_SIZE, fNDIS_GUID_TO_OID | fNDIS_GUID_ALLOW_READ | fNDIS_GUID_ALLOW_WRITE }
};
#endif

NDIS_OID ionic_oid_list[] = {
    // mandatory generic OIDs
    OID_GEN_SUPPORTED_LIST, OID_GEN_HARDWARE_STATUS, OID_GEN_MEDIA_SUPPORTED,
    OID_GEN_MEDIA_IN_USE, OID_GEN_PHYSICAL_MEDIUM, OID_GEN_MAXIMUM_LOOKAHEAD,
    OID_GEN_MAXIMUM_FRAME_SIZE, OID_GEN_LINK_SPEED,
    OID_GEN_TRANSMIT_BUFFER_SPACE, OID_GEN_RECEIVE_BUFFER_SPACE,
    OID_GEN_TRANSMIT_BLOCK_SIZE, OID_GEN_RECEIVE_BLOCK_SIZE, OID_GEN_VENDOR_ID,
    OID_GEN_VENDOR_DESCRIPTION, OID_GEN_VENDOR_DRIVER_VERSION,
    OID_GEN_CURRENT_PACKET_FILTER, OID_GEN_CURRENT_LOOKAHEAD,
    OID_GEN_DRIVER_VERSION, OID_GEN_MAXIMUM_TOTAL_SIZE, OID_GEN_MAC_OPTIONS,
    OID_GEN_VLAN_ID, OID_GEN_MEDIA_CONNECT_STATUS, OID_GEN_MAXIMUM_SEND_PACKETS,
    OID_GEN_LINK_PARAMETERS, OID_PNP_SET_POWER, OID_PNP_QUERY_POWER,

    // mandatory statistical OIDs
    OID_GEN_STATISTICS, OID_GEN_XMIT_OK, OID_GEN_RCV_OK, OID_GEN_XMIT_ERROR,
    OID_GEN_XMIT_DISCARDS, OID_GEN_RCV_ERROR, OID_GEN_RCV_NO_BUFFER,

    // optional statistical OIDs
    OID_GEN_RCV_CRC_ERROR, OID_GEN_DIRECTED_BYTES_XMIT,
    OID_GEN_DIRECTED_FRAMES_XMIT, OID_GEN_MULTICAST_BYTES_XMIT,
    OID_GEN_MULTICAST_FRAMES_XMIT, OID_GEN_BROADCAST_BYTES_XMIT,
    OID_GEN_BROADCAST_FRAMES_XMIT, OID_GEN_TRANSMIT_QUEUE_LENGTH,
    OID_GEN_DIRECTED_BYTES_RCV, OID_GEN_DIRECTED_FRAMES_RCV,
    OID_GEN_MULTICAST_BYTES_RCV, OID_GEN_MULTICAST_FRAMES_RCV,
    OID_GEN_BROADCAST_BYTES_RCV, OID_GEN_BROADCAST_FRAMES_RCV,
    OID_GEN_BYTES_RCV, OID_GEN_RCV_DISCARDS, OID_GEN_BYTES_XMIT,

    // mandatory ethernet OIDs
    OID_802_3_PERMANENT_ADDRESS, OID_802_3_CURRENT_ADDRESS,
    OID_802_3_MULTICAST_LIST, OID_802_3_MAXIMUM_LIST_SIZE,

    // mandatory ethernet statistics OIDs
    OID_802_3_RCV_ERROR_ALIGNMENT, OID_802_3_XMIT_ONE_COLLISION,
    OID_802_3_XMIT_MORE_COLLISIONS,

    // optional ethernet statistics OIDs
    OID_802_3_XMIT_DEFERRED, OID_802_3_XMIT_MAX_COLLISIONS,
    OID_802_3_RCV_OVERRUN,

    // NDIS 6.x offload OIDs
    OID_TCP_OFFLOAD_PARAMETERS, OID_TCP_OFFLOAD_HARDWARE_CAPABILITIES,
    OID_TCP_OFFLOAD_CURRENT_CONFIG, OID_OFFLOAD_ENCAPSULATION,

    // NDIS 6.x RSS OID
    OID_GEN_RECEIVE_SCALE_CAPABILITIES, OID_GEN_RECEIVE_SCALE_PARAMETERS,
    OID_GEN_RECEIVE_HASH,

    OID_GEN_INTERRUPT_MODERATION,

    OID_SWITCH_PROPERTY_ADD, OID_SWITCH_PROPERTY_UPDATE,
    OID_SWITCH_PROPERTY_DELETE, OID_SWITCH_PROPERTY_ENUM,
    OID_SWITCH_FEATURE_STATUS_QUERY,

    OID_SWITCH_NIC_REQUEST, OID_SWITCH_PORT_PROPERTY_ADD,
    OID_SWITCH_PORT_PROPERTY_UPDATE, OID_SWITCH_PORT_PROPERTY_DELETE,
    OID_SWITCH_PORT_PROPERTY_ENUM, OID_SWITCH_PARAMETERS, OID_SWITCH_PORT_ARRAY,
    OID_SWITCH_NIC_ARRAY, OID_SWITCH_PORT_CREATE, OID_SWITCH_PORT_DELETE,
    OID_SWITCH_NIC_CREATE, OID_SWITCH_NIC_CONNECT, OID_SWITCH_NIC_DISCONNECT,
    OID_SWITCH_NIC_DELETE, OID_SWITCH_PORT_FEATURE_STATUS_QUERY,
    OID_SWITCH_PORT_TEARDOWN, OID_SWITCH_NIC_SAVE, OID_SWITCH_NIC_SAVE_COMPLETE,
    OID_SWITCH_NIC_RESTORE, OID_SWITCH_NIC_RESTORE_COMPLETE,
    OID_SWITCH_NIC_UPDATED, OID_SWITCH_PORT_UPDATED,

#if (NDIS_SUPPORT_NDIS650)
    OID_SWITCH_NIC_DIRECT_REQUEST, OID_SWITCH_NIC_SUSPEND,
    OID_SWITCH_NIC_RESUME,
#endif

    OID_NIC_SWITCH_CREATE_SWITCH, OID_NIC_SWITCH_PARAMETERS,
    OID_NIC_SWITCH_DELETE_SWITCH, OID_NIC_SWITCH_ENUM_SWITCHES,
    OID_NIC_SWITCH_CREATE_VPORT, OID_NIC_SWITCH_VPORT_PARAMETERS,
    OID_NIC_SWITCH_ENUM_VPORTS, OID_NIC_SWITCH_DELETE_VPORT,
    OID_NIC_SWITCH_ALLOCATE_VF, OID_NIC_SWITCH_FREE_VF,
    OID_NIC_SWITCH_VF_PARAMETERS, OID_NIC_SWITCH_ENUM_VFS,
    OID_SRIOV_HARDWARE_CAPABILITIES, OID_SRIOV_CURRENT_CAPABILITIES,
    OID_SRIOV_READ_VF_CONFIG_SPACE, OID_SRIOV_WRITE_VF_CONFIG_SPACE,
    OID_SRIOV_READ_VF_CONFIG_BLOCK, OID_SRIOV_WRITE_VF_CONFIG_BLOCK,
    OID_SRIOV_RESET_VF, OID_SRIOV_SET_VF_POWER_STATE,
    OID_SRIOV_VF_VENDOR_DEVICE_ID, OID_SRIOV_PROBED_BARS,
    OID_SRIOV_BAR_RESOURCES, OID_SRIOV_PF_LUID,

    OID_SRIOV_CONFIG_STATE, OID_SRIOV_VF_SERIAL_NUMBER,

#if (NDIS_SUPPORT_NDIS670)
    OID_SRIOV_OVERLYING_ADAPTER_INFO,
#endif

    OID_SRIOV_VF_INVALIDATE_CONFIG_BLOCK,

#if (NDIS_SUPPORT_NDIS650)
    OID_SWITCH_NIC_DIRECT_REQUEST, OID_SWITCH_NIC_SUSPEND,
    OID_SWITCH_NIC_RESUME,
#endif

#if (NDIS_SUPPORT_NDIS680)
    OID_SWITCH_NIC_SUSPENDED_LM_SOURCE_STARTED,
    OID_SWITCH_NIC_SUSPENDED_LM_SOURCE_FINISHED,
#endif

    OID_RECEIVE_FILTER_HARDWARE_CAPABILITIES,
    OID_RECEIVE_FILTER_GLOBAL_PARAMETERS, OID_RECEIVE_FILTER_ALLOCATE_QUEUE,
    OID_RECEIVE_FILTER_FREE_QUEUE, OID_RECEIVE_FILTER_ENUM_QUEUES,
    OID_RECEIVE_FILTER_QUEUE_PARAMETERS, OID_RECEIVE_FILTER_SET_FILTER,
    OID_RECEIVE_FILTER_CLEAR_FILTER, OID_RECEIVE_FILTER_ENUM_FILTERS,
    OID_RECEIVE_FILTER_PARAMETERS, OID_RECEIVE_FILTER_QUEUE_ALLOCATION_COMPLETE,
    OID_RECEIVE_FILTER_CURRENT_CAPABILITIES,
    OID_NIC_SWITCH_HARDWARE_CAPABILITIES, OID_NIC_SWITCH_CURRENT_CAPABILITIES,
    OID_RECEIVE_FILTER_MOVE_FILTER,

    OID_TCP4_OFFLOAD_STATS, OID_TCP6_OFFLOAD_STATS, OID_IP4_OFFLOAD_STATS,
    OID_IP6_OFFLOAD_STATS,

    // To support Custom OID's
    // OID_GEN_SUPPORTED_GUIDS,
    // OID_IONIC_CUSTOM_1,
    // OID_IONIC_CUSTOM_2,
};

char *
GetSwitchTypeString(ULONG SwitchType)
{

    char *pName = "NdisNicSwitchTypeUnspecified";

    switch (SwitchType) {

    case NdisNicSwitchTypeExternal: {
        pName = "NdisNicSwitchTypeExternal";
        break;
    }
    }

    return pName;
}

char *
GetNicTypeString(ULONG NicType)
{
    char *pchName = "NdisSwitchNicTypeExternal";

    switch (NicType) {

    case NdisSwitchNicTypeSynthetic: {
        pchName = "NdisSwitchNicTypeSynthetic";
        break;
    }

    case NdisSwitchNicTypeEmulated: {
        pchName = "NdisSwitchNicTypeEmulated";
        break;
    }

    case NdisSwitchNicTypeInternal: {
        pchName = "NdisSwitchNicTypeInternal";
        break;
    }
    }

    return pchName;
}

char *
GetNicStateString(ULONG NicState)
{
    char *pchName = "NdisSwitchNicStateUnknown";

    switch (NicState) {

    case NdisSwitchNicStateCreated: {
        pchName = "NdisSwitchNicStateCreated";
        break;
    }

    case NdisSwitchNicStateConnected: {
        pchName = "NdisSwitchNicStateConnected";
        break;
    }

    case NdisSwitchNicStateDisconnected: {
        pchName = "NdisSwitchNicStateDisconnected";
        break;
    }

    case NdisSwitchNicStateDeleted: {
        pchName = "NdisSwitchNicStateDeleted";
        break;
    }
    }

    return pchName;
}

char *
GetPortStateString(ULONG PortState)
{
    char *pchName = "NdisSwitchPortStateUnknown";

    switch (PortState) {

    case NdisSwitchPortStateCreated: {
        pchName = "NdisSwitchPortStateCreated";
        break;
    }

    case NdisSwitchPortStateTeardown: {
        pchName = "NdisSwitchPortStateTeardown";
        break;
    }

    case NdisSwitchPortStateDeleted: {
        pchName = "NdisSwitchPortStateDeleted";
        break;
    }
    }

    return pchName;
}

char *
GetPortTypeString(ULONG PortType)
{
    char *pchName = "NdisSwitchPortTypeGeneric";

    switch (PortType) {

    case NdisSwitchPortTypeExternal: {
        pchName = "NdisSwitchPortTypeExternal";
        break;
    }

    case NdisSwitchPortTypeSynthetic: {
        pchName = "NdisSwitchPortTypeSynthetic";
        break;
    }

    case NdisSwitchPortTypeEmulated: {
        pchName = "NdisSwitchPortTypeEmulated";
        break;
    }

    case NdisSwitchPortTypeInternal: {
        pchName = "NdisSwitchPortTypeInternal";
        break;
    }
    }

    return pchName;
}
NDIS_STATUS
oid_multicast_list_set(struct ionic *ionic,
                       VOID *info_buffer,
                       ULONG info_buffer_length)
{
    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    ULONG mc_count;
    u8 *currentFilter = NULL;
    struct lif *lif = ionic->master_lif;
    LIST_ENTRY *head;
    LIST_ENTRY *cur;
    LIST_ENTRY *next;
    struct rx_filter *f;
    unsigned int i;

    //
    // Clear the current mc list in the adapter
    //

    NdisZeroMemory(ionic->mcast_filter_array,
                   sizeof(ionic->mcast_filter_array));

    for (i = 0; i < RX_FILTER_HLISTS; i++) {

        head = &lif->rx_filters.by_id[i];

        if (!IsListEmpty(head)) {

            cur = head->Flink;
            do {

                f = CONTAINING_RECORD(cur, struct rx_filter, by_id);

                next = cur->Flink;

                if (is_multicast_ether_addr(f->cmd.mac.addr)) {
                    ntStatus = ionic_lif_addr(lif, f->cmd.mac.addr, false);
                    if (ntStatus != NDIS_STATUS_SUCCESS) {
                        DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE,
                                  "%s Failed to clear rx filter Status %08lX\n",
                                  __FUNCTION__, ntStatus));
                        break;
                    }
                }

                cur = next;
            } while (cur != head);
        }

        if (ntStatus != NDIS_STATUS_SUCCESS) {
            break;
        }
    }

    if (ntStatus != NDIS_STATUS_SUCCESS) {
        goto cleanup;
    }

    mc_count = info_buffer_length / ETH_ALEN;
    currentFilter = (u8 *)info_buffer;

    for (i = 0; i < mc_count; i++) {
        ntStatus = ionic_lif_addr(lif, currentFilter, true);

        if (ntStatus != NDIS_STATUS_SUCCESS) {
            DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE,
                      "%s Failed to set rx filter Status %08lX\n", __FUNCTION__,
                      ntStatus));
            break;
        }

        currentFilter += ETH_ALEN;
    }

cleanup:

    return ntStatus;
}

NDIS_STATUS
oid_multicast_list_remove(struct ionic *ionic)
{
    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    struct lif *lif = ionic->master_lif;
    LIST_ENTRY *head;
    LIST_ENTRY *cur;
    LIST_ENTRY *next;
    struct rx_filter *f;
    unsigned int i;
    u8 *curr_fltr = NULL;
    BOOLEAN removed = FALSE;

    //
    // Clear the current mc list in the adapter
    //

    curr_fltr = ionic->mcast_filter_array;

    removed = !is_zero_ether_addr(curr_fltr);

    for (i = 0; i < RX_FILTER_HLISTS; i++) {

        head = &lif->rx_filters.by_id[i];

        if (!IsListEmpty(head)) {

            cur = head->Flink;
            do {

                f = CONTAINING_RECORD(cur, struct rx_filter, by_id);

                next = cur->Flink;

                if (is_multicast_ether_addr(f->cmd.mac.addr)) {

                    ASSERT(!removed);

                    NdisMoveMemory(curr_fltr, f->cmd.mac.addr, ETH_ALEN);
                    curr_fltr += ETH_ALEN;

                    ntStatus = ionic_lif_addr(lif, f->cmd.mac.addr, false);
                    if (ntStatus != NDIS_STATUS_SUCCESS) {
                        DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE,
                                  "%s Failed to clear rx filter Status %08lX\n",
                                  __FUNCTION__, ntStatus));
                    }
                }

                cur = next;
            } while (cur != head);
        }
    }

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
oid_multicast_list_restore(struct ionic *ionic)
{
    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    u8 *currentFilter = NULL;
    struct lif *lif = ionic->master_lif;

    currentFilter = ionic->mcast_filter_array;

    while (!is_zero_ether_addr(currentFilter)) {
        ntStatus = ionic_lif_addr(lif, currentFilter, true);

        if (ntStatus != NDIS_STATUS_SUCCESS) {
            DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE,
                      "%s Failed to set rx filter Status %08lX\n", __FUNCTION__,
                      ntStatus));
        }

        currentFilter += ETH_ALEN;
    }

    NdisZeroMemory(ionic->mcast_filter_array,
                   sizeof(ionic->mcast_filter_array));

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
oid_packet_filter_set(struct ionic *ionic,
                      VOID *info_buffer,
                      ULONG info_buffer_length,
                      ULONG *bytes_read,
                      ULONG *bytes_needed)
{
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;
    unsigned int rx_mode = 0;

    unsigned long supported_filters =
        NDIS_PACKET_TYPE_DIRECTED | NDIS_PACKET_TYPE_MULTICAST |
        NDIS_PACKET_TYPE_BROADCAST | NDIS_PACKET_TYPE_PROMISCUOUS |
        NDIS_PACKET_TYPE_ALL_MULTICAST;
    unsigned long packet_filter = *(unsigned long *)info_buffer;
    int directed_en, multicast_en, broadcast_en, promisc_en, allmulti_en;

    if (info_buffer_length < sizeof(unsigned long)) {
        *bytes_needed = sizeof(unsigned long);
        *bytes_read = 0;
        return NDIS_STATUS_INVALID_LENGTH;
    }

    *bytes_read = sizeof(unsigned long);
    *bytes_needed = 0;

    if (packet_filter & ~supported_filters) {
        return NDIS_STATUS_NOT_SUPPORTED;
    }

    if (!packet_filter) {
        // NULL Packet Filter
        // Make sure all Receive Queues are disabled
    } else {

        directed_en = packet_filter & NDIS_PACKET_TYPE_DIRECTED;
        multicast_en = packet_filter & NDIS_PACKET_TYPE_MULTICAST;
        broadcast_en = packet_filter & NDIS_PACKET_TYPE_BROADCAST;
        promisc_en = packet_filter & NDIS_PACKET_TYPE_PROMISCUOUS;
        allmulti_en = packet_filter & NDIS_PACKET_TYPE_ALL_MULTICAST;

        rx_mode |= (directed_en ? RX_MODE_F_UNICAST : 0);
        rx_mode |= (multicast_en ? RX_MODE_F_MULTICAST : 0);
        rx_mode |= (broadcast_en ? RX_MODE_F_BROADCAST : 0);
        rx_mode |= (allmulti_en ? RX_MODE_F_ALLMULTI : 0);
        rx_mode |= (promisc_en ? (RX_MODE_F_PROMISC | RX_MODE_F_ALLMULTI |
                                  RX_MODE_F_BROADCAST)
                               : 0);

        if (rx_mode == RX_MODE_F_UNICAST || rx_mode == RX_MODE_F_BROADCAST ||
            rx_mode == (RX_MODE_F_BROADCAST | RX_MODE_F_UNICAST)) {

            DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE,
                      "%s Removing any mcast filters for rx mode %08lX\n",
                      __FUNCTION__, rx_mode));

            oid_multicast_list_remove(ionic);
        } else {

            DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE,
                      "%s Removing any mcast filters for rx mode %08lX\n",
                      __FUNCTION__, rx_mode));

            oid_multicast_list_restore(ionic);
        }

        status = ionic_set_rx_mode(ionic->master_lif, rx_mode);

        if (status != NDIS_STATUS_SUCCESS) {
            return status;
        }
    }

    // set the proper packet filter using ADMIN DEVCMD

    if (packet_filter) {
        // Enable the Receive Queues if proper packet filters are set
    }

    DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE,
              "%s Adapter %p packetfilter %08lX mode %08lX\n", __FUNCTION__,
              ionic, packet_filter, rx_mode));

    return NDIS_STATUS_SUCCESS;
}

char *
GetMethodOidName(NDIS_OID Oid)
{

    char *pName = "Unknown Oid";

    switch (Oid) {

    case OID_NIC_SWITCH_CREATE_SWITCH: {
        pName = "OID_NIC_SWITCH_CREATE_SWITCH";
        break;
    }

    case OID_RECEIVE_FILTER_ALLOCATE_QUEUE: {
        pName = "OID_RECEIVE_FILTER_ALLOCATE_QUEUE";
        break;
    }

    case OID_RECEIVE_FILTER_SET_FILTER: {
        pName = "OID_RECEIVE_FILTER_SET_FILTER";
        break;
    }

    case OID_RECEIVE_FILTER_QUEUE_ALLOCATION_COMPLETE: {
        pName = "OID_RECEIVE_FILTER_QUEUE_ALLOCATION_COMPLETE";
        break;
    }

    case OID_NIC_SWITCH_CREATE_VPORT: {
        pName = "OID_NIC_SWITCH_CREATE_VPORT";
        break;
    }

    case OID_NIC_SWITCH_VPORT_PARAMETERS: {
        pName = "OID_NIC_SWITCH_VPORT_PARAMETERS";
        break;
    }

    case OID_NIC_SWITCH_ALLOCATE_VF: {
        pName = "OID_NIC_SWITCH_ALLOCATE_VF";
        break;
    }

    case OID_SRIOV_READ_VF_CONFIG_SPACE: {
        pName = "OID_SRIOV_READ_VF_CONFIG_SPACE";
        break;
    }

    case OID_SRIOV_VF_VENDOR_DEVICE_ID: {
        pName = "OID_SRIOV_VF_VENDOR_DEVICE_ID";
        break;
    }

    case OID_SRIOV_BAR_RESOURCES: {
        pName = "OID_SRIOV_BAR_RESOURCES";
        break;
    }

    case OID_RECEIVE_FILTER_QUEUE_PARAMETERS: {
        pName = "OID_RECEIVE_FILTER_QUEUE_PARAMETERS";
        break;
    }
    }

    return pName;
}

char *
GetQueryOidName(NDIS_OID Oid)
{

    char *name = "Unknown Oid";

    switch (Oid) {
    case OID_GEN_SUPPORTED_LIST: {
        name = "OID_GEN_SUPPORTED_LIST";
        break;
    }

    case OID_GEN_HARDWARE_STATUS: {
        name = "OID_GEN_HARDWARE_STATUS";
        break;
    }

    case OID_GEN_MEDIA_SUPPORTED: {
        name = "OID_GEN_MEDIA_SUPPORTED";
        break;
    }
    case OID_GEN_MEDIA_IN_USE: {
        name = "OID_GEN_MEDIA_IN_USE";
        break;
    }

    case OID_GEN_PHYSICAL_MEDIUM: {
        name = "OID_GEN_PHYSICAL_MEDIUM";
        break;
    }

    case OID_GEN_CURRENT_LOOKAHEAD: {
        name = "OID_GEN_CURRENT_LOOKAHEAD";
        break;
    }

    case OID_GEN_MAXIMUM_LOOKAHEAD: {
        name = "OID_GEN_MAXIMUM_LOOKAHEAD";
        break;
    }
    case OID_GEN_MAXIMUM_FRAME_SIZE: {
        name = "OID_GEN_MAXIMUM_FRAME_SIZE";
        break;
    }

    case OID_GEN_MAXIMUM_TOTAL_SIZE: {
        name = "OID_GEN_MAXIMUM_TOTAL_SIZE";
        break;
    }
    case OID_GEN_TRANSMIT_BLOCK_SIZE: {
        name = "OID_GEN_TRANSMIT_BLOCK_SIZE";
        break;
    }

    case OID_GEN_RECEIVE_BLOCK_SIZE: {
        name = "OID_GEN_RECEIVE_BLOCK_SIZE";
        break;
    }

    case OID_GEN_VENDOR_ID: {
        name = "OID_GEN_VENDOR_ID";
        break;
    }

    case OID_GEN_VENDOR_DESCRIPTION: {
        name = "OID_GEN_VENDOR_DESCRIPTION";
        break;
    }

    case OID_GEN_VENDOR_DRIVER_VERSION: {
        name = "OID_GEN_VENDOR_DRIVER_VERSION";
        break;
    }

    case OID_GEN_DRIVER_VERSION: {
        name = "OID_GEN_DRIVER_VERSION";
        break;
    }

    case OID_GEN_MAC_OPTIONS: {
        name = "OID_GEN_MAC_OPTIONS";
        break;
    }

    case OID_GEN_VLAN_ID: {
        name = "OID_GEN_VLAN_ID";
        break;
    }

    case OID_GEN_LINK_SPEED: {
        name = "OID_GEN_LINK_SPEED";
        break;
    }

    case OID_GEN_TRANSMIT_BUFFER_SPACE: {
        name = "OID_GEN_TRANSMIT_BUFFER_SPACE";
        break;
    }

    case OID_GEN_RECEIVE_BUFFER_SPACE: {
        name = "OID_GEN_RECEIVE_BUFFER_SPACE";
        break;
    }

    case OID_GEN_MEDIA_CONNECT_STATUS: {
        name = "OID_GEN_MEDIA_CONNECT_STATUS";
        break;
    }

    case OID_GEN_MAXIMUM_SEND_PACKETS: {
        name = "OID_GEN_MAXIMUM_SEND_PACKETS";
        break;
    }

    case OID_GEN_STATISTICS: {
        name = "OID_GEN_STATISTICS";
        break;
    }

    case OID_GEN_XMIT_OK: {
        name = "OID_GEN_XMIT_OK";
        break;
    }

    case OID_GEN_RCV_OK: {
        name = "OID_GEN_RCV_OK";
        break;
    }

    case OID_GEN_XMIT_ERROR: {
        name = "OID_GEN_XMIT_ERROR";
        break;
    }

    case OID_GEN_XMIT_DISCARDS: {
        name = "OID_GEN_XMIT_DISCARDS";
        break;
    }

    case OID_GEN_RCV_ERROR: {
        name = "OID_GEN_RCV_ERROR";
        break;
    }

    case OID_GEN_RCV_NO_BUFFER: {
        name = "OID_GEN_RCV_NO_BUFFER";
        break;
    }

    case OID_GEN_RCV_CRC_ERROR: {
        name = "OID_GEN_RCV_CRC_ERROR";
        break;
    }

    case OID_GEN_DIRECTED_BYTES_XMIT: {
        name = "OID_GEN_DIRECTED_BYTES_XMIT";
        break;
    }

    case OID_GEN_DIRECTED_FRAMES_XMIT: {
        name = "OID_GEN_DIRECTED_FRAMES_XMIT";
        break;
    }

    case OID_GEN_MULTICAST_BYTES_XMIT: {
        name = "OID_GEN_MULTICAST_BYTES_XMIT";
        break;
    }

    case OID_GEN_MULTICAST_FRAMES_XMIT: {
        name = "OID_GEN_MULTICAST_FRAMES_XMIT";
        break;
    }

    case OID_GEN_BROADCAST_BYTES_XMIT: {
        name = "OID_GEN_BROADCAST_BYTES_XMIT";
        break;
    }

    case OID_GEN_BROADCAST_FRAMES_XMIT: {
        name = "OID_GEN_BROADCAST_FRAMES_XMIT";
        break;
    }

    case OID_GEN_TRANSMIT_QUEUE_LENGTH: {
        name = "OID_GEN_TRANSMIT_QUEUE_LENGTH";
        break;
    }

    case OID_GEN_DIRECTED_BYTES_RCV: {
        name = "OID_GEN_DIRECTED_BYTES_RCV";
        break;
    }

    case OID_GEN_DIRECTED_FRAMES_RCV: {
        name = "OID_GEN_DIRECTED_FRAMES_RCV";
        break;
    }

    case OID_GEN_MULTICAST_BYTES_RCV: {
        name = "OID_GEN_MULTICAST_BYTES_RCV";
        break;
    }

    case OID_GEN_MULTICAST_FRAMES_RCV: {
        name = "OID_GEN_MULTICAST_FRAMES_RCV";
        break;
    }

    case OID_GEN_BROADCAST_BYTES_RCV: {
        name = "OID_GEN_BROADCAST_BYTES_RCV";
        break;
    }

    case OID_GEN_BROADCAST_FRAMES_RCV: {
        name = "OID_GEN_BROADCAST_FRAMES_RCV";
        break;
    }

    case OID_GEN_BYTES_RCV: {
        name = "OID_GEN_BYTES_RCV";
        break;
    }

    case OID_GEN_RCV_DISCARDS: {
        name = "OID_GEN_RCV_DISCARDS";
        break;
    }

    case OID_GEN_BYTES_XMIT: {
        name = "OID_GEN_BYTES_XMIT";
        break;
    }

    case OID_802_3_RCV_OVERRUN: {
        name = "OID_802_3_RCV_OVERRUN";
        break;
    }

    case OID_802_3_XMIT_DEFERRED: {
        name = "OID_802_3_XMIT_DEFERRED";
        break;
    }

    case OID_802_3_XMIT_MAX_COLLISIONS: {
        name = "OID_802_3_XMIT_MAX_COLLISIONS";
        break;
    }

    case OID_802_3_XMIT_ONE_COLLISION: {
        name = "OID_802_3_XMIT_ONE_COLLISION";
        break;
    }

    case OID_802_3_XMIT_MORE_COLLISIONS: {
        name = "OID_802_3_XMIT_MORE_COLLISIONS";
        break;
    }

    case OID_802_3_RCV_ERROR_ALIGNMENT: {
        name = "OID_802_3_RCV_ERROR_ALIGNMENT";
        break;
    }

    case OID_802_3_PERMANENT_ADDRESS: {
        name = "OID_802_3_PERMANENT_ADDRESS";
        break;
    }

    case OID_802_3_CURRENT_ADDRESS: {
        name = "OID_802_3_CURRENT_ADDRESS";
        break;
    }

    case OID_802_3_MAXIMUM_LIST_SIZE: {
        name = "OID_802_3_MAXIMUM_LIST_SIZE";
        break;
    }

    case OID_GEN_CURRENT_PACKET_FILTER: {
        name = "OID_GEN_CURRENT_PACKET_FILTER";
        break;
    }

    case OID_TCP_OFFLOAD_HARDWARE_CAPABILITIES: {
        name = "OID_TCP_OFFLOAD_HARDWARE_CAPABILITIES";
        break;
    }

    case OID_TCP_OFFLOAD_CURRENT_CONFIG: {
        name = "OID_TCP_OFFLOAD_CURRENT_CONFIG";
        break;
    }

    case OID_OFFLOAD_ENCAPSULATION: {
        name = "OID_OFFLOAD_ENCAPSULATION";
        break;
    }

    case OID_GEN_RECEIVE_SCALE_CAPABILITIES: {
        name = "OID_GEN_RECEIVE_SCALE_CAPABILITIES";
        break;
    }

    case OID_GEN_RECEIVE_SCALE_PARAMETERS: {
        name = "OID_GEN_RECEIVE_SCALE_PARAMETERS";
        break;
    }

    case OID_GEN_RECEIVE_HASH: {
        name = "OID_GEN_RECEIVE_HASH";
        break;
    }

    case OID_GEN_INTERRUPT_MODERATION: {
        name = "OID_GEN_INTERRUPT_MODERATION";
        break;
    }

    case OID_GEN_SUPPORTED_GUIDS: {
        name = "OID_GEN_SUPPORTED_GUIDS";
        break;
    }

#if 0
		case OID_IONIC_CUSTOM_1:
		{
			name = "OID_IONIC_CUSTOM_1";
			break;
		}

		case OID_IONIC_CUSTOM_2:
		{
			name = "OID_IONIC_CUSTOM_2";
			break;
		}
#endif

    case OID_GEN_ISOLATION_PARAMETERS: {
        name = "OID_GEN_ISOLATION_PARAMETERS";
        break;
    }

    case OID_SWITCH_PORT_ARRAY: {
        name = "OID_SWITCH_PORT_ARRAY";
        break;
    }

    case OID_SWITCH_NIC_ARRAY: {
        name = "OID_SWITCH_NIC_ARRAY";
        break;
    }

    case OID_SRIOV_PROBED_BARS: {
        name = "OID_SRIOV_PROBED_BARS";
        break;
    }

    case OID_PNP_QUERY_POWER: {
        name = "OID_PNP_QUERY_POWER";
        break;
    }

    case OID_TCP4_OFFLOAD_STATS: {
        name = "OID_TCP4_OFFLOAD_STATS";
        break;
    }

    case OID_TCP6_OFFLOAD_STATS: {
        name = "OID_TCP6_OFFLOAD_STATS";
        break;
    }

    case OID_IP4_OFFLOAD_STATS: {
        name = "OID_IP4_OFFLOAD_STATS";
        break;
    }

    case OID_IP6_OFFLOAD_STATS: {
        name = "OID_IP6_OFFLOAD_STATS";
        break;
    }
    default: {
        break;
    }
    }

    return name;
}

char *
GetSetOidName(NDIS_OID Oid)
{

    char *name = "Unknown Oid";

    switch (Oid) {
    case OID_GEN_CURRENT_PACKET_FILTER: {
        name = "OID_GEN_CURRENT_PACKET_FILTER";
        break;
    }

    case OID_802_3_MULTICAST_LIST: {
        name = "OID_802_3_MULTICAST_LIST";
        break;
    }

    case OID_GEN_CURRENT_LOOKAHEAD: {
        name = "OID_GEN_CURRENT_LOOKAHEAD";
        break;
    }

    case OID_GEN_VLAN_ID: {
        name = "OID_GEN_VLAN_ID";
        break;
    }

    case OID_TCP_OFFLOAD_PARAMETERS: {
        name = "OID_TCP_OFFLOAD_PARAMETERS";
        break;
    }

    case OID_OFFLOAD_ENCAPSULATION: {
        name = "OID_OFFLOAD_ENCAPSULATION";
        break;
    }

    case OID_GEN_RECEIVE_SCALE_PARAMETERS: {
        name = "OID_GEN_RECEIVE_SCALE_PARAMETERS";
        break;
    }

    case OID_GEN_RECEIVE_HASH: {
        name = "OID_GEN_RECEIVE_HASH";
        break;
    }

    case OID_GEN_MACHINE_NAME: {
        name = "OID_GEN_MACHINE_NAME";
        break;
    }

    case OID_SWITCH_PORT_ARRAY: {
        name = "OID_SWITCH_PORT_ARRAY";
        break;
    }

    case OID_SWITCH_NIC_ARRAY: {
        name = "OID_SWITCH_NIC_ARRAY";
        break;
    }

    case OID_SWITCH_NIC_CREATE: {
        name = "OID_SWITCH_NIC_CREATE";
        break;
    }

    case OID_SWITCH_NIC_CONNECT: {
        name = "OID_SWITCH_NIC_CONNECT";
        break;
    }

    case OID_NIC_SWITCH_DELETE_SWITCH: {
        name = "OID_NIC_SWITCH_DELETE_SWITCH";
        break;
    }

    case OID_GEN_NETWORK_LAYER_ADDRESSES: {
        name = "OID_GEN_NETWORK_LAYER_ADDRESSES";
        break;
    }

    case OID_RECEIVE_FILTER_CLEAR_FILTER: {
        name = "OID_RECEIVE_FILTER_CLEAR_FILTER";
        break;
    }

    case OID_SWITCH_NIC_DISCONNECT: {
        name = "OID_SWITCH_NIC_DISCONNECT";
        break;
    }

    case OID_SWITCH_PORT_CREATE: {
        name = "OID_SWITCH_PORT_CREATE";
        break;
    }

    case OID_SWITCH_PORT_TEARDOWN: {
        name = "OID_SWITCH_PORT_TEARDOWN";
        break;
    }

    case OID_SWITCH_PORT_DELETE: {
        name = "OID_SWITCH_PORT_DELETE";
        break;
    }

    case OID_RECEIVE_FILTER_FREE_QUEUE: {
        name = "OID_RECEIVE_FILTER_FREE_QUEUE";
        break;
    }

#if (NDIS_SUPPORT_NDIS650)
    case OID_SWITCH_NIC_SUSPEND: {
        name = "OID_SWITCH_NIC_SUSPEND";
        break;
    }
#endif

    case OID_SWITCH_NIC_DELETE: {
        name = "OID_SWITCH_NIC_DELETE";
        break;
    }

    case OID_RECEIVE_FILTER_MOVE_FILTER: {
        name = "OID_RECEIVE_FILTER_MOVE_FILTER";
        break;
    }

    case OID_NIC_SWITCH_DELETE_VPORT: {
        name = "OID_NIC_SWITCH_DELETE_VPORT";
        break;
    }

    case OID_NIC_SWITCH_VPORT_PARAMETERS: {
        name = "OID_NIC_SWITCH_VPORT_PARAMETERS";
        break;
    }

    case OID_NIC_SWITCH_FREE_VF: {
        name = "OID_NIC_SWITCH_FREE_VF";
        break;
    }

    case OID_SRIOV_RESET_VF: {
        name = "OID_SRIOV_RESET_VF";
        break;
    }

    case OID_SRIOV_SET_VF_POWER_STATE: {
        name = "OID_SRIOV_SET_VF_POWER_STATE";
        break;
    }

    case OID_SRIOV_WRITE_VF_CONFIG_SPACE: {
        name = "OID_SRIOV_WRITE_VF_CONFIG_SPACE";
        break;
    }

    case OID_SWITCH_NIC_UPDATED: {
        name = "OID_SWITCH_NIC_UPDATED";
        break;
    }

#if 0
		case OID_IONIC_CUSTOM_1:
		{
			name = "OID_IONIC_CUSTOM_1";
			break;
		}

		case OID_IONIC_CUSTOM_2:
		{
			name = "OID_IONIC_CUSTOM_2";
			break;
		}
#endif

    case OID_RECEIVE_FILTER_QUEUE_PARAMETERS: {
        name = "OID_RECEIVE_FILTER_QUEUE_PARAMETERS";
        break;
    }

    case OID_GEN_INTERRUPT_MODERATION: {
        name = "OID_GEN_INTERRUPT_MODERATION";
        break;
    }

    case OID_PNP_SET_POWER: {
        name = "OID_PNP_SET_POWER";
        break;
    }

    default: {
        break;
    }
    }

    return name;
}

void
ionic_get_supported_oids(PNDIS_OID *ppoids, ULONG *list_length)
{

    *ppoids = ionic_oid_list;
    *list_length = sizeof(ionic_oid_list);

    return;
}

NDIS_STATUS
oid_method_information(struct ionic *ionic,
                       NDIS_OID oid,
                       PNDIS_OID_REQUEST OidRequest)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;

    DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE, "%s Enter for oid %s\n",
              __FUNCTION__, GetMethodOidName(oid)));

    switch (oid) {

    case OID_RECEIVE_FILTER_SET_FILTER: {

        //
        // Set the filter based on the mdoe we are running in
        //

        if (BooleanFlagOn(ionic->ConfigStatus, IONIC_SRIOV_MODE)) {
            ntStatus = oid_filter_set_filter_vswitch(ionic, OidRequest);
        } else {
            ntStatus = oid_filter_set_filter(ionic, OidRequest);
        }

        break;
    }

    case OID_NIC_SWITCH_CREATE_SWITCH: {

        ntStatus = oid_nic_switch_create_switch(ionic, OidRequest);

        break;
    }

    case OID_RECEIVE_FILTER_ALLOCATE_QUEUE: {

        ntStatus = oid_filter_allocate_queue(ionic, OidRequest);

        break;
    }

    case OID_RECEIVE_FILTER_QUEUE_ALLOCATION_COMPLETE: {

        ntStatus = oid_filter_queue_alloc_complete(ionic, OidRequest);

        break;
    }

    case OID_NIC_SWITCH_CREATE_VPORT: {

        ntStatus = oid_switch_create_vport(ionic, OidRequest);

        break;
    }

    case OID_NIC_SWITCH_VPORT_PARAMETERS: {

        ntStatus = oid_switch_query_vport_params(ionic, OidRequest);

        break;
    }

    case OID_NIC_SWITCH_ALLOCATE_VF: {

        ASSERT(BooleanFlagOn(ionic->ConfigStatus, IONIC_SRIOV_MODE));

        ntStatus = oid_switch_create_vf(ionic, OidRequest);

        break;
    }

    case OID_SRIOV_READ_VF_CONFIG_SPACE: {

        ntStatus = oid_sriov_read_vf_pci_config(ionic, OidRequest);

        break;
    }

    case OID_SRIOV_VF_VENDOR_DEVICE_ID: {

        ntStatus = oid_switch_read_vf_ven_dev_id(ionic, OidRequest);

        break;
    }

    case OID_SRIOV_BAR_RESOURCES: {

        ntStatus = oid_sriov_bar_resources(ionic, OidRequest);
        break;
    }

    case OID_RECEIVE_FILTER_QUEUE_PARAMETERS: {

        ntStatus = oid_method_receive_filter_params(ionic, OidRequest);

        break;
    }

    default:
        ntStatus = NDIS_STATUS_NOT_SUPPORTED;
        DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_ERROR,
                  "%s Unhandled Oid 0x%08lX *************************\n",
                  __FUNCTION__, oid));
        break;
    }

    return ntStatus;
}

NDIS_STATUS
oid_set_information(struct ionic *ionic,
                    NDIS_OID_REQUEST *OidRequest,
                    VOID *info_buffer,
                    ULONG info_buffer_length,
                    ULONG *bytes_read,
                    ULONG *bytes_needed)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(info_buffer);
    UNREFERENCED_PARAMETER(ionic);

    DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE, "%s Enter for oid %s\n",
              __FUNCTION__,
              GetSetOidName(OidRequest->DATA.SET_INFORMATION.Oid)));

    switch (OidRequest->DATA.SET_INFORMATION.Oid) {

    case OID_GEN_MACHINE_NAME: {

        if (ionic->computer_name.Buffer != NULL) {
            NdisFreeMemoryWithTagPriority_internal(ionic->adapterhandle,
                                          ionic->computer_name.Buffer,
                                          IONIC_GENERIC_TAG);
        }

        ionic->computer_name.MaximumLength =
            (USHORT)(info_buffer_length + sizeof(WCHAR));
        ionic->computer_name.Buffer =
            (WCHAR *)NdisAllocateMemoryWithTagPriority_internal(
                ionic->adapterhandle, ionic->computer_name.MaximumLength,
                IONIC_GENERIC_TAG, NormalPoolPriority);

        if (ionic->computer_name.Buffer != NULL) {

            NdisZeroMemory(ionic->computer_name.Buffer,
                           ionic->computer_name.MaximumLength);

            DbgTrace((TRACE_COMPONENT_MEMORY, TRACE_LEVEL_VERBOSE,
                  "%s Allocated 0x%p len %08lX\n",
                  __FUNCTION__,
                  ionic->computer_name.Buffer,
                  ionic->computer_name.MaximumLength));

            NdisMoveMemory(ionic->computer_name.Buffer, info_buffer,
                           info_buffer_length);
            ionic->computer_name.Length = (USHORT)info_buffer_length;
        }

        break;
    }

    case OID_GEN_CURRENT_PACKET_FILTER: {
        ntStatus = oid_packet_filter_set(ionic, info_buffer, info_buffer_length,
                                         bytes_read, bytes_needed);
        break;
    }

    case OID_802_3_MULTICAST_LIST: {

        ntStatus =
            oid_multicast_list_set(ionic, info_buffer, info_buffer_length);

        break;
    }

    case OID_GEN_CURRENT_LOOKAHEAD: {
        if (info_buffer_length < sizeof(ULONG32)) {
            *bytes_needed = sizeof(ULONG32);
            *bytes_read = 0;
            ntStatus = NDIS_STATUS_BUFFER_TOO_SHORT;
        } else {
            *bytes_read = sizeof(ULONG32);
        }

        break;
    }

    case OID_GEN_VLAN_ID: {

        ntStatus = oid_set_vlan(ionic, info_buffer, info_buffer_length,
                                bytes_needed, bytes_read);

        break;
    }

    case OID_TCP_OFFLOAD_PARAMETERS: {

        ntStatus = oid_handle_offload_parameters(
            ionic, info_buffer, info_buffer_length, bytes_read, bytes_needed);
        break;
    }

    case OID_OFFLOAD_ENCAPSULATION: {

        ntStatus = oid_handle_offload_encapsulation(
            ionic, info_buffer, info_buffer_length, bytes_read, bytes_needed);

        break;
    }

    case OID_SWITCH_PORT_ARRAY: {
        NDIS_SWITCH_PORT_ARRAY *pArray = (NDIS_SWITCH_PORT_ARRAY *)info_buffer;
        NDIS_SWITCH_PORT_PARAMETERS *pPort = NULL;
        ULONG ulIndex = 0;

        if (pArray->NumElements == 0) {
            DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE,
                      "%s (OID_SWITCH_PORT_ARRAY) No elements for %p\n",
                      __FUNCTION__, ionic));
        } else {

            DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE,
                      "%s (OID_SWITCH_PORT_ARRAY) %d elements for %p\n",
                      __FUNCTION__, pArray->NumElements, ionic));

            while (ulIndex < pArray->NumElements) {
                pPort = NDIS_SWITCH_PORT_AT_ARRAY_INDEX(pArray, ulIndex);

                DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE,
                          "\tPortId %d State %s Type %s Validation %s\n",
                          pPort->PortId, GetPortStateString(pPort->PortState),
                          GetPortTypeString(pPort->PortType),
                          pPort->IsValidationPort ? "Yes" : "No"));

                ulIndex++;
            }
        }

        break;
    }

    case OID_SWITCH_NIC_ARRAY: {
        NDIS_SWITCH_NIC_ARRAY *pArray = (NDIS_SWITCH_NIC_ARRAY *)info_buffer;
        NDIS_SWITCH_NIC_PARAMETERS *pNic = NULL;
        ULONG ulIndex = 0;

        if (pArray->NumElements == 0) {
            DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE,
                      "%s (OID_SWITCH_NIC_ARRAY) No elements for %p\n",
                      __FUNCTION__, ionic));
        } else {

            DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE,
                      "%s (OID_SWITCH_NIC_ARRAY) %d elements for %p\n",
                      __FUNCTION__, pArray->NumElements, ionic));

            while (ulIndex < pArray->NumElements) {
                pNic = NDIS_SWITCH_NIC_AT_ARRAY_INDEX(pArray, ulIndex);

                DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE,
                          "\tFlags %08lX PortId %d NicIndex %d State %s Type "
                          "%s VFAssigned %s\n",
                          pNic->Flags, pNic->PortId, pNic->NicIndex,
                          GetNicStateString(pNic->NicState),
                          GetNicTypeString(pNic->NicType),
                          pNic->VFAssigned ? "Yes" : "No"));

                ulIndex++;
            }
        }

        break;
    }

    case OID_SWITCH_NIC_CREATE: {

        ntStatus = oid_nic_switch_create(ionic, info_buffer, info_buffer_length,
                                         bytes_needed);

        break;
    }

    case OID_SWITCH_NIC_DELETE: {

        ntStatus = oid_nic_switch_delete(ionic, info_buffer, info_buffer_length,
                                         bytes_needed);

        break;
    }

    case OID_SWITCH_NIC_CONNECT: {

        ntStatus = oid_nic_switch_connect(ionic, info_buffer,
                                          info_buffer_length, bytes_needed);

        break;
    }

    case OID_SWITCH_PORT_CREATE: {

        ntStatus = oid_port_create(ionic, info_buffer, info_buffer_length,
                                   bytes_needed);

        break;
    }

    case OID_SWITCH_PORT_TEARDOWN: {

        ntStatus = oid_port_teardown(ionic, info_buffer, info_buffer_length,
                                     bytes_needed);

        break;
    }

    case OID_SWITCH_PORT_DELETE: {

        ntStatus = oid_port_delete(ionic, info_buffer, info_buffer_length,
                                   bytes_needed);
        break;
    }

    case OID_SWITCH_NIC_UPDATED: {

        ntStatus = oid_nic_switch_updated(ionic, info_buffer,
                                          info_buffer_length, bytes_needed);

        break;
    }

    case OID_NIC_SWITCH_DELETE_SWITCH: {

        ntStatus = oid_nic_switch_delete_switch(
            ionic, info_buffer, info_buffer_length, bytes_needed);

        break;
    }

    case OID_GEN_RECEIVE_SCALE_PARAMETERS: {

        ntStatus = oid_set_rss_parameters(
            ionic, info_buffer, info_buffer_length, bytes_read, bytes_needed);

        break;
    }

    case OID_GEN_RECEIVE_HASH: {

        ntStatus = oid_set_rss_hash(ionic, info_buffer, info_buffer_length,
                                    bytes_read, bytes_needed);

        break;
    }

    case OID_GEN_NETWORK_LAYER_ADDRESSES: {

        NETWORK_ADDRESS_LIST *pNetAddr = (NETWORK_ADDRESS_LIST *)info_buffer;
        NETWORK_ADDRESS *pAddr = NULL;
        ULONG ulIndex = 0;

        DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE,
                  "%s (OID_GEN_NETWORK_LAYER_ADDRESSES) Address Count %d\n",
                  __FUNCTION__, pNetAddr->AddressCount));

        pAddr = pNetAddr->Address;

        while (ulIndex < (ULONG)pNetAddr->AddressCount) {

            DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE, "\tType %d\n",
                      pAddr->AddressType));

            pAddr =
                (NETWORK_ADDRESS *)((char *)pAddr + sizeof(NETWORK_ADDRESS) +
                                    pAddr->AddressLength - 1);

            ulIndex++;
        }

        break;
    }

    case OID_RECEIVE_FILTER_CLEAR_FILTER: {

        if (BooleanFlagOn(ionic->ConfigStatus, IONIC_SRIOV_MODE)) {
            ntStatus = oid_filter_clear_filter_vswitch(ionic, info_buffer);
        } else {
            ntStatus = oid_filter_clear_filter(ionic, info_buffer);
        }

        break;
    }

    case OID_SWITCH_NIC_DISCONNECT: {

        NDIS_SWITCH_NIC_PARAMETERS *pParams =
            (NDIS_SWITCH_NIC_PARAMETERS *)info_buffer;

        if (pParams == NULL) {
            break;
        }

        DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE,
                  "%s (OID_SWITCH_NIC_DISCONNECT) Flags %08lX NicIndex %d\n",
                  __FUNCTION__, pParams->Flags, pParams->NicIndex));

        break;
    }

    case OID_RECEIVE_FILTER_FREE_QUEUE: {

        ntStatus = oid_free_queue(ionic, OidRequest, info_buffer);
        break;
    }

#if (NDIS_SUPPORT_NDIS650)
    case OID_SWITCH_NIC_SUSPEND: {

        break;
    }
#endif

    case OID_RECEIVE_FILTER_MOVE_FILTER: {

        ASSERT(BooleanFlagOn(ionic->ConfigStatus, IONIC_SRIOV_MODE));

        ntStatus = oid_filter_move_filter_vswitch(ionic, info_buffer);

        break;
    }

    case OID_NIC_SWITCH_DELETE_VPORT: {

        ntStatus = oid_switch_delete_vport(ionic, info_buffer);

        break;
    }

    case OID_NIC_SWITCH_VPORT_PARAMETERS: {

        ntStatus = oid_switch_set_vport_params(ionic, info_buffer);
        break;
    }

    case OID_NIC_SWITCH_FREE_VF: {

        ntStatus = oid_switch_delete_vf(ionic, info_buffer);

        break;
    }

    case OID_SRIOV_RESET_VF: {

        ntStatus = oid_switch_reset_vf(ionic, info_buffer);

        break;
    }

    case OID_SRIOV_SET_VF_POWER_STATE: {

        ntStatus = oid_switch_set_vf_power(ionic, info_buffer,
                                           info_buffer_length, bytes_read);

        break;
    }

    case OID_SRIOV_WRITE_VF_CONFIG_SPACE: {

        ntStatus = oid_sriov_write_vf_pci_config(
            ionic, info_buffer, info_buffer_length, bytes_read);

        break;
    }

    case OID_RECEIVE_FILTER_QUEUE_PARAMETERS: {

        ntStatus = oid_set_receive_filter_params(
            ionic, info_buffer, info_buffer_length, bytes_read);
        break;
    }

    case OID_GEN_INTERRUPT_MODERATION: {

        ntStatus = oid_set_interrupt_moderation(ionic, info_buffer,
                                                info_buffer_length, bytes_read);

        break;
    }

    case OID_PNP_SET_POWER: {

        ntStatus = oid_set_power_state(ionic, info_buffer, info_buffer_length,
                                       bytes_read);

        break;
    }
#if 0
		case OID_IONIC_CUSTOM_1:
		case OID_IONIC_CUSTOM_2:
		{
			ntStatus = NDIS_STATUS_SUCCESS;
			*bytes_read = info_buffer_length;
			break;
		}
#endif
    default: {
        DbgTrace(
            (TRACE_COMPONENT_OID, TRACE_LEVEL_ERROR,
             "%s Unhandled Oid 0x%08lX *************************************\n",
             __FUNCTION__, OidRequest->DATA.SET_INFORMATION.Oid));
        ntStatus = NDIS_STATUS_NOT_SUPPORTED;
        break;
    }
    }

    DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE,
              "%s Complete for oid %s Status 0x%08lX\n", __FUNCTION__,
              GetSetOidName(OidRequest->DATA.SET_INFORMATION.Oid), ntStatus));

    return ntStatus;
}

static PVOID
ionic_query_gen_stat(struct ionic *ionic,
                     PULONG buf_len,
                     PNDIS_STATISTICS_INFO psi)
{
    struct lif *lif = ionic->master_lif;

    NdisZeroMemory(psi, sizeof(NDIS_STATISTICS_INFO));
    psi->Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
    psi->Header.Revision = NDIS_STATISTICS_INFO_REVISION_1;
    psi->Header.Size = NDIS_SIZEOF_STATISTICS_INFO_REVISION_1;

    if (lif->rxqcqs != NULL && lif->txqcqs != NULL) {
        // OID_GEN_DIRECTED_FRAMES_RCV
        psi->SupportedStatistics |=
            NDIS_STATISTICS_FLAGS_VALID_DIRECTED_FRAMES_RCV;
        psi->ifHCInUcastPkts = ionic->idev.port_info->stats.frames_rx_unicast;
        // OID_GEN_MULTICAST_FRAMES_RCV
        psi->SupportedStatistics |=
            NDIS_STATISTICS_FLAGS_VALID_MULTICAST_FRAMES_RCV;
        psi->ifHCInMulticastPkts =
            ionic->idev.port_info->stats.frames_rx_multicast;
        // OID_GEN_BROADCAST_FRAMES_RCV
        psi->SupportedStatistics |=
            NDIS_STATISTICS_FLAGS_VALID_BROADCAST_FRAMES_RCV;
        psi->ifHCInBroadcastPkts =
            ionic->idev.port_info->stats.frames_rx_broadcast;
        // OID_GEN_BYTES_RCV
        psi->SupportedStatistics |= NDIS_STATISTICS_FLAGS_VALID_BYTES_RCV;
        psi->ifHCInOctets =
            ionic->idev.port_info->stats.octets_rx_ok;
        // OID_GEN_RCV_ERROR
        psi->SupportedStatistics |= NDIS_STATISTICS_FLAGS_VALID_RCV_ERROR;
        psi->ifInErrors = ionic->idev.port_info->stats.frames_rx_dropped +
                            ionic->idev.port_info->stats.frames_rx_bad_all;
        // OID_GEN_RCV_DISCARDS
        psi->SupportedStatistics |= NDIS_STATISTICS_FLAGS_VALID_RCV_DISCARDS;
        psi->ifInDiscards = psi->ifInErrors +
                            ionic->master_lif->info->stats.rx_queue_empty;
        // OID_GEN_DIRECTED_FRAMES_XMIT
        psi->SupportedStatistics |=
            NDIS_STATISTICS_FLAGS_VALID_DIRECTED_FRAMES_XMIT;
        psi->ifHCOutUcastPkts =
            ionic->idev.port_info->stats
                .frames_tx_unicast;
        // OID_GEN_MULTICAST_FRAMES_XMIT
        psi->SupportedStatistics |=
            NDIS_STATISTICS_FLAGS_VALID_MULTICAST_FRAMES_XMIT;
        psi->ifHCOutMulticastPkts = lif->txqcqs[0].tx_stats->mcast_packets;
        // OID_GEN_BROADCAST_FRAMES_XMIT
        psi->SupportedStatistics |=
            NDIS_STATISTICS_FLAGS_VALID_BROADCAST_FRAMES_XMIT;
        psi->ifHCOutBroadcastPkts = lif->txqcqs[0].tx_stats->bcast_packets;
        // OID_GEN_BYTES_XMIT
        psi->SupportedStatistics |= NDIS_STATISTICS_FLAGS_VALID_BYTES_XMIT;
        psi->ifHCOutOctets =
            ionic->idev.port_info->stats
                .octets_tx_ok;
        // OID_GEN_XMIT_ERROR
        psi->SupportedStatistics |= NDIS_STATISTICS_FLAGS_VALID_XMIT_ERROR;
        psi->ifOutErrors = lif->txqcqs[0].tx_stats->no_descs;
        // OID_GEN_XMIT_DISCARDS
        psi->SupportedStatistics |= NDIS_STATISTICS_FLAGS_VALID_XMIT_DISCARDS;
        psi->ifOutDiscards = 0;
        // OID_GEN_DIRECTED_BYTES_RCV
        psi->SupportedStatistics |=
            NDIS_STATISTICS_FLAGS_VALID_DIRECTED_BYTES_RCV;
        psi->ifHCInUcastOctets = lif->rxqcqs[0].rx_stats->directed_bytes;
        // OID_GEN_MULTICAST_BYTES_RCV
        psi->SupportedStatistics |=
            NDIS_STATISTICS_FLAGS_VALID_MULTICAST_BYTES_RCV;
        psi->ifHCInMulticastOctets = lif->rxqcqs[0].rx_stats->mcast_bytes;
        // OID_GEN_BROADCAST_BYTES_RCV
        psi->SupportedStatistics |=
            NDIS_STATISTICS_FLAGS_VALID_BROADCAST_BYTES_RCV;
        psi->ifHCInBroadcastOctets = lif->rxqcqs[0].rx_stats->bcast_bytes;
        // OID_GEN_DIRECTED_BYTES_XMIT
        psi->SupportedStatistics |=
            NDIS_STATISTICS_FLAGS_VALID_DIRECTED_BYTES_XMIT;
        psi->ifHCOutUcastOctets = lif->txqcqs[0].tx_stats->directed_bytes;
        // OID_GEN_MULTICAST_BYTES_XMIT
        psi->SupportedStatistics |=
            NDIS_STATISTICS_FLAGS_VALID_MULTICAST_BYTES_XMIT;
        psi->ifHCOutMulticastOctets = lif->txqcqs[0].tx_stats->mcast_bytes;
        // OID_GEN_BROADCAST_BYTES_XMIT
        psi->SupportedStatistics |=
            NDIS_STATISTICS_FLAGS_VALID_BROADCAST_BYTES_XMIT;
        psi->ifHCOutBroadcastOctets = lif->txqcqs[0].tx_stats->bcast_bytes;
    }

    *buf_len = sizeof(NDIS_STATISTICS_INFO);
    return psi;
}

NDIS_STATUS
oid_query_information(struct ionic *ionic,
                      NDIS_OID oid,
                      PVOID info_buffer,
                      ULONG info_buffer_length,
                      PULONG bytes_written,
                      PULONG bytes_needed)
{
    union {
        NDIS_RECEIVE_SCALE_CAPABILITIES sc;
        NDIS_RECEIVE_SCALE_PARAMETERS sp;
        char sp_pad_1[NDIS_RSS_INDIRECTION_TABLE_MAX_SIZE_REVISION_1 +
                      NDIS_RSS_HASH_SECRET_KEY_MAX_SIZE_REVISION_1 +
                      sizeof(NDIS_RECEIVE_SCALE_PARAMETERS)];
        NDIS_RECEIVE_HASH_PARAMETERS hp;
        char sp_pad_2[NDIS_RSS_HASH_SECRET_KEY_MAX_SIZE_REVISION_1 +
                      sizeof(NDIS_RECEIVE_SCALE_PARAMETERS)];
        NDIS_STATISTICS_INFO si;
        NDIS_OFFLOAD ndol;
        NDIS_OFFLOAD_ENCAPSULATION oe;
        NDIS_INTERRUPT_MODERATION_PARAMETERS im;
    } var;

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    struct lif *lif = ionic->master_lif;
    UCHAR buf_copy = 1;
    UCHAR using_64bit_stat = 0;
    NDIS_MEDIUM medium = NdisMedium802_3;
    NDIS_PHYSICAL_MEDIUM phy_medium = NdisPhysicalMedium802_3;
    UCHAR drv_product[] = DRV_PRODUCT;
    TCP_OFFLOAD_STATS tcp_offload_stats;
    IP_OFFLOAD_STATS ip_offload_stats;

    PVOID buf_ptr = NULL;
    ULONG buf_len = 0;
    ULONG32 buf_u32;
    ULONG64 buf_u64;
    BOOLEAN bNoCopy = FALSE;

    // Initialize the result
    *bytes_written = 0;
    *bytes_needed = 0;

    if (oid != OID_GEN_STATISTICS) {
        DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE,
                  "%s Process Oid %s\n", __FUNCTION__, GetQueryOidName(oid)));
    }

    switch (oid) {
    case OID_GEN_SUPPORTED_LIST: {
        buf_ptr = ionic_oid_list;
        buf_len = sizeof(ionic_oid_list);
        break;
    }

    case OID_GEN_HARDWARE_STATUS: {
        buf_ptr = &ionic->hardware_status;
        buf_len = sizeof(NDIS_HARDWARE_STATUS);

        DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE_2,
                  "%s (OID_GEN_HARDWARE_STATUS) Adapter %p status 0x%08lX\n",
                  __FUNCTION__, ionic, ionic->hardware_status));

        break;
    }

    case OID_GEN_MEDIA_SUPPORTED:
    case OID_GEN_MEDIA_IN_USE: {
        buf_ptr = &medium;
        buf_len = sizeof(NDIS_MEDIUM);
        break;
    }

    case OID_GEN_PHYSICAL_MEDIUM: {
        buf_ptr = &phy_medium;
        buf_len = sizeof(NDIS_PHYSICAL_MEDIUM);
        break;
    }

    case OID_GEN_CURRENT_LOOKAHEAD:
    case OID_GEN_MAXIMUM_LOOKAHEAD:
    case OID_GEN_MAXIMUM_FRAME_SIZE: {

        buf_u32 = ionic->frame_size - ETH_COMPLETE_HDR;
        buf_ptr = &buf_u32;
        buf_len = sizeof(buf_u32);

        DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE_2,
                  "%s (OID_GEN_CURRENT_LOOKAHEAD) Mtu %d\n", __FUNCTION__,
                  buf_u32));

        break;
    }

    case OID_GEN_MAXIMUM_TOTAL_SIZE:
    case OID_GEN_TRANSMIT_BLOCK_SIZE:
    case OID_GEN_RECEIVE_BLOCK_SIZE: {

        buf_u32 = ionic->frame_size;
        buf_ptr = &buf_u32;
        buf_len = sizeof(buf_u32);

        DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE_2,
                  "%s (OID_GEN_MAXIMUM_TOTAL_SIZE) Mtu %d\n", __FUNCTION__,
                  buf_u32));

        break;
    }

    case OID_GEN_VENDOR_ID: {
        buf_u32 = PCI_VENDOR_ID_PENSANDO;
        buf_ptr = &buf_u32;
        buf_len = sizeof(buf_u32);
        break;
    }

    case OID_GEN_VENDOR_DESCRIPTION: {
        buf_ptr = drv_product;
        buf_len = sizeof(drv_product);
        break;
    }

    case OID_GEN_VENDOR_DRIVER_VERSION: {
        buf_u32 = IONIC_MP_VENDOR_DRIVER_VERSION;
        buf_ptr = &buf_u32;
        buf_len = sizeof(buf_u32);
        break;
    }

    case OID_GEN_DRIVER_VERSION: {
        // buf_u16 = (USHORT) DRV_NDIS_VERSION;
        // buf_ptr = &buf_u16;
        // buf_len = sizeof(buf_u16);
        break;
    }

    case OID_GEN_MAC_OPTIONS: {
        buf_u32 = NDIS_MAC_OPTION_TRANSFERS_NOT_PEND |
                  NDIS_MAC_OPTION_NO_LOOPBACK | NDIS_MAC_OPTION_FULL_DUPLEX |
                  NDIS_MAC_OPTION_SUPPORTS_MAC_ADDRESS_OVERWRITE;

        if (BooleanFlagOn(ionic->ConfigStatus, IONIC_PRIORITY_ENABLED)) {
            buf_u32 |= NDIS_MAC_OPTION_8021P_PRIORITY;
        }

        if (BooleanFlagOn(ionic->ConfigStatus, IONIC_VLAN_ENABLED)) {
            buf_u32 |= NDIS_MAC_OPTION_8021Q_VLAN;
        }

        buf_ptr = &buf_u32;
        buf_len = sizeof(buf_u32);
        break;
    }

    case OID_GEN_VLAN_ID: {

        IoPrint("%s (OID_GEN_VLAN_ID) Return Id %d\n", __FUNCTION__,
                ionic->master_lif->vlan_id);

        buf_u32 = ionic->master_lif->vlan_id;
        buf_ptr = &buf_u32;
        buf_len = sizeof(buf_u32);
        break;
    }

    case OID_GEN_LINK_SPEED: {
        ULONG link_spd = 0;
        /* Represented in units of 100bps */
        link_spd =  le32_to_cpu(ionic->master_lif->info->status.link_speed);
        link_spd *= (MEGABITS_PER_SECOND/100);
        buf_u32 = link_spd;
        buf_ptr = &buf_u32;
        buf_len = sizeof(buf_u32);
        break;
    }

    case OID_GEN_TRANSMIT_BUFFER_SPACE: {
        buf_u32 = ionic->ntx_buffers * ionic->frame_size;
        buf_ptr = &buf_u32;
        buf_len = sizeof(buf_u32);
        break;
    }

    case OID_GEN_RECEIVE_BUFFER_SPACE: {
        buf_u32 = ionic->nrx_buffers * ionic->frame_size;
        buf_ptr = &buf_u32;
        buf_len = sizeof(buf_u32);
        break;
    }

    case OID_GEN_MEDIA_CONNECT_STATUS: {

        if (lif->info->status.link_status == PORT_OPER_STATUS_UP) {
            DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE_2,
                      "%s (OID_GEN_MEDIA_CONNECT_STATUS) Connected\n",
                      __FUNCTION__));
            buf_u32 = NdisMediaStateConnected;
        } else {
            DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE_2,
                      "%s (OID_GEN_MEDIA_CONNECT_STATUS) Disconnected\n",
                      __FUNCTION__));
            buf_u32 = NdisMediaStateDisconnected;
        }

        buf_ptr = &buf_u32;
        buf_len = sizeof(buf_u32);
        break;
    }

    case OID_GEN_MAXIMUM_SEND_PACKETS: {
        /* de-serialized drivers can ignore this */
        buf_u32 = ionic->ntx_buffers;
        buf_ptr = &buf_u32;
        buf_len = sizeof(buf_u32);
        break;
    }

    /* general statistics OIDs */
    case OID_GEN_STATISTICS: {
        NdisZeroMemory(&var.si, sizeof(var.si));
        buf_ptr = ionic_query_gen_stat(ionic, &buf_len, &var.si);
        break;
    }

    case OID_GEN_XMIT_OK: {
        buf_u64 = ionic->idev.port_info->stats.octets_tx_ok;
        using_64bit_stat = 1;
        break;
    }

    case OID_GEN_RCV_OK: {
        buf_u64 = ionic->idev.port_info->stats.octets_rx_ok;
        using_64bit_stat = 1;
        break;
    }

    case OID_GEN_XMIT_ERROR: {
        buf_u64 = lif->txqcqs[0].tx_stats->dma_map_error;
        using_64bit_stat = 1;
        break;
    }

    case OID_GEN_XMIT_DISCARDS: {
        buf_u64 = 0;
        using_64bit_stat = 1;
        break;
    }

    case OID_GEN_RCV_ERROR: {
        buf_u64 = ionic->idev.port_info->stats.frames_rx_dropped +
                            ionic->idev.port_info->stats.frames_rx_bad_all;
        using_64bit_stat = 1;
        break;
    }

    case OID_GEN_RCV_NO_BUFFER: {
        buf_u64 = ionic->master_lif->info->stats.rx_queue_empty;
        using_64bit_stat = 1;
        break;
    }

    case OID_GEN_RCV_CRC_ERROR: {
        buf_u64 = ionic->idev.port_info->stats.frames_rx_bad_fcs;
        using_64bit_stat = 1;
        break;
    }

    case OID_GEN_DIRECTED_BYTES_XMIT: {
        buf_u64 = lif->txqcqs[0].tx_stats->directed_bytes;
        using_64bit_stat = 1;
        break;
    }

    case OID_GEN_DIRECTED_FRAMES_XMIT: {
        buf_u64 = ionic->idev.port_info->stats.frames_tx_unicast;
        using_64bit_stat = 1;
        break;
    }

    case OID_GEN_MULTICAST_BYTES_XMIT: {
        buf_u64 = lif->txqcqs[0].tx_stats->mcast_bytes;
        using_64bit_stat = 1;
        break;
    }

    case OID_GEN_MULTICAST_FRAMES_XMIT: {
        buf_u64 = ionic->idev.port_info->stats.frames_tx_multicast;
        using_64bit_stat = 1;
        break;
    }

    case OID_GEN_BROADCAST_BYTES_XMIT: {
        buf_u64 = lif->txqcqs[0].tx_stats->bcast_bytes;
        using_64bit_stat = 1;
        break;
    }

    case OID_GEN_BROADCAST_FRAMES_XMIT: {
        buf_u64 = ionic->idev.port_info->stats.frames_tx_broadcast;
        using_64bit_stat = 1;
        break;
    }

    case OID_GEN_TRANSMIT_QUEUE_LENGTH: {
        buf_u64 = 0;
        // return the Pending Queue Depth
        using_64bit_stat = 1;
        break;
    }

    case OID_GEN_DIRECTED_BYTES_RCV: {
        buf_u64 = lif->rxqcqs[0].rx_stats->directed_bytes;
        using_64bit_stat = 1;
        break;
    }

    case OID_GEN_DIRECTED_FRAMES_RCV: {
        buf_u64 = ionic->idev.port_info->stats.frames_rx_unicast;
        using_64bit_stat = 1;
        break;
    }

    case OID_GEN_MULTICAST_BYTES_RCV: {
        buf_u64 = lif->rxqcqs[0].rx_stats->mcast_bytes;
        using_64bit_stat = 1;
        break;
    }

    case OID_GEN_MULTICAST_FRAMES_RCV: {
        buf_u64 = ionic->idev.port_info->stats.frames_rx_multicast;
        using_64bit_stat = 1;
        break;
    }

    case OID_GEN_BROADCAST_BYTES_RCV: {
        buf_u64 = lif->rxqcqs[0].rx_stats->bcast_bytes;
        using_64bit_stat = 1;
        break;
    }

    case OID_GEN_BROADCAST_FRAMES_RCV: {
        buf_u64 = ionic->idev.port_info->stats.frames_rx_broadcast;
        using_64bit_stat = 1;
        break;
    }

    case OID_GEN_BYTES_RCV: {
        buf_u64 = ionic->idev.port_info->stats.octets_rx_ok;
        using_64bit_stat = 1;
        break;
    }

    case OID_GEN_RCV_DISCARDS: {
        buf_u64 = ionic->idev.port_info->stats.frames_rx_dropped +
                            ionic->idev.port_info->stats.frames_rx_bad_all +
                            ionic->master_lif->info->stats.rx_queue_empty;

        using_64bit_stat = 1;
        break;
    }

    case OID_GEN_BYTES_XMIT: {
        buf_u64 = ionic->idev.port_info->stats.octets_tx_ok;
        using_64bit_stat = 1;
        break;
    }

        /* ethernet statistics OIDs */
    case OID_802_3_RCV_OVERRUN: {
        buf_u64 = 0;
        using_64bit_stat = 1;
        break;
    }

    case OID_802_3_XMIT_DEFERRED:
    case OID_802_3_XMIT_MAX_COLLISIONS:
    case OID_802_3_XMIT_ONE_COLLISION:
    case OID_802_3_XMIT_MORE_COLLISIONS:
    case OID_802_3_RCV_ERROR_ALIGNMENT: {
        /* set these statistics to be zero */
        buf_u64 = 0;
        using_64bit_stat = 1;
        break;
    }

    case OID_802_3_PERMANENT_ADDRESS: {
        buf_ptr = &lif->ionic->perm_addr[0];
        buf_len = ETH_ALEN;

        DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE_2,
                  "%s (OID_802_3_PERMANENT_ADDRESS) Returning MAC "
                  "%c:%c:%c:%c:%c:%c\n",
                  __FUNCTION__, lif->ionic->perm_addr[0],
                  lif->ionic->perm_addr[1], lif->ionic->perm_addr[2],
                  lif->ionic->perm_addr[3], lif->ionic->perm_addr[4],
                  lif->ionic->perm_addr[5]));

        break;
    }

    case OID_802_3_CURRENT_ADDRESS: {
        buf_ptr = &lif->ionic->config_addr[0];
        buf_len = ETH_ALEN;

        DbgTrace(
            (TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE_2,
             "%s (OID_802_3_CURRENT_ADDRESS) Returning MAC %c:%c:%c:%c:%c:%c\n",
             __FUNCTION__, lif->ionic->config_addr[0],
             lif->ionic->config_addr[1], lif->ionic->config_addr[2],
             lif->ionic->config_addr[3], lif->ionic->config_addr[4],
             lif->ionic->config_addr[5]));

        break;
    }

    case OID_802_3_MAXIMUM_LIST_SIZE: {
        buf_u32 = IONIC_MULTICAST_PERFECT_FILTERS;
        buf_ptr = &buf_u32;
        buf_len = sizeof(buf_u32);
        break;
    }

    case OID_GEN_CURRENT_PACKET_FILTER: {
        buf_ptr = &lif->rx_mode;
        buf_len = sizeof(ULONG);
        break;
    }

    case OID_TCP_OFFLOAD_HARDWARE_CAPABILITIES: {
        NdisZeroMemory(&var.ndol, sizeof(var.ndol));
        // buf_ptr = ionic_query_offload_hw_capabilities(ionic,
        //  &buf_len, &var.ndol);
        break;
    }

    case OID_TCP_OFFLOAD_CURRENT_CONFIG: {
        NdisZeroMemory(&var.ndol, sizeof(var.ndol));
        // buf_ptr = ionic_query_offload_capabilities(ionic, &buf_len,
        // &var.ndol);
        break;
    }

    case OID_OFFLOAD_ENCAPSULATION: {
        NdisZeroMemory(&var.oe, sizeof(var.oe));
        // buf_ptr = ionic_query_offload_encapsulation(ionic, &buf_len,
        // &var.oe);
        break;
    }

    case OID_GEN_RECEIVE_SCALE_CAPABILITIES: {

        ntStatus = oid_query_rss_caps(ionic, info_buffer, info_buffer_length,
                                      bytes_needed, bytes_written);

        bNoCopy = TRUE;

        break;
    }

    case OID_GEN_RECEIVE_SCALE_PARAMETERS: {
        NdisZeroMemory(&var.sp_pad_1, sizeof(var.sp_pad_1));
        // buf_ptr = ionic_query_rss_parameters(ionic, &buf_len, &var.sp);
        break;
    }

    case OID_GEN_RECEIVE_HASH: {

        ntStatus = oid_query_rss_hash(ionic, info_buffer, info_buffer_length,
                                      bytes_needed, bytes_written);

        bNoCopy = TRUE;

        break;
    }

    case OID_GEN_INTERRUPT_MODERATION: {
        NdisZeroMemory(&var.im, sizeof(var.im));
        var.im.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
        var.im.Header.Revision =
            NDIS_INTERRUPT_MODERATION_PARAMETERS_REVISION_1;
        var.im.Header.Size = sizeof(NDIS_INTERRUPT_MODERATION_PARAMETERS);
        var.im.Flags = NDIS_INTERRUPT_MODERATION_CHANGE_NEEDS_REINITIALIZE;

        if (BooleanFlagOn(ionic->ConfigStatus, IONIC_INTERRUPT_MOD_ENABLED)) {
            var.im.InterruptModeration = NdisInterruptModerationEnabled;
        } else {
            var.im.InterruptModeration = NdisInterruptModerationDisabled;
        }

        buf_ptr = &var.im;
        buf_len = sizeof(var.im);
        break;
    }

    case OID_GEN_SUPPORTED_GUIDS: {
        // buf_ptr = (PVOID)&supportedGUIDs;
        // buf_len = sizeof(supportedGUIDs);
        bNoCopy = TRUE;
        ntStatus = NDIS_STATUS_NOT_SUPPORTED;
        break;
    }

    case OID_GEN_ISOLATION_PARAMETERS: {

        ntStatus = oid_query_isolation_info(ionic, info_buffer,
                                            info_buffer_length, bytes_written);

        bNoCopy = TRUE;
        break;
    }

    case OID_SWITCH_PORT_ARRAY: {
        NDIS_SWITCH_PORT_ARRAY *pArray = (NDIS_SWITCH_PORT_ARRAY *)info_buffer;

        if (pArray->NumElements == 0) {
        }

        break;
    }

    case OID_SWITCH_NIC_ARRAY: {
        NDIS_SWITCH_NIC_ARRAY *pArray = (NDIS_SWITCH_NIC_ARRAY *)info_buffer;

        if (pArray->NumElements == 0) {
        }

        break;
    }

    case OID_SRIOV_PROBED_BARS: {

        ntStatus = oid_query_bar_info(ionic, info_buffer, info_buffer_length,
                                      bytes_written);

        bNoCopy = TRUE;

        break;
    }

    case OID_IONIC_CUSTOM_1:
    case OID_IONIC_CUSTOM_2: {
        bNoCopy = TRUE;
        *bytes_written = buf_len;
        ntStatus = NDIS_STATUS_SUCCESS;
        break;
    }

    case OID_PNP_QUERY_POWER: {
        bNoCopy = TRUE;
        *bytes_written = buf_len;
        ntStatus = NDIS_STATUS_SUCCESS;
        break;
    }

    case OID_TCP4_OFFLOAD_STATS:
    case OID_TCP6_OFFLOAD_STATS: {
        NdisZeroMemory(&tcp_offload_stats, sizeof(TCP_OFFLOAD_STATS));
        buf_ptr = &tcp_offload_stats;
        buf_len = sizeof(TCP_OFFLOAD_STATS);
        break;
    }

    case OID_IP4_OFFLOAD_STATS:
    case OID_IP6_OFFLOAD_STATS: {
        NdisZeroMemory(&ip_offload_stats, sizeof(IP_OFFLOAD_STATS));
        buf_ptr = &ip_offload_stats;
        buf_len = sizeof(IP_OFFLOAD_STATS);
        break;
    }

    default: {
        DbgTrace(
            (TRACE_COMPONENT_OID, TRACE_LEVEL_ERROR,
             "%s Unhandled Oid 0x%08lX **********************************\n",
             __FUNCTION__, oid));
        ntStatus = NDIS_STATUS_NOT_SUPPORTED;
        break;
    }
    }

    if (!bNoCopy) {

        if (using_64bit_stat) {
            buf_ptr = &buf_u64;
            buf_len = max_t(ULONG32, info_buffer_length, sizeof(ULONG));
            buf_len = min_t(ULONG32, buf_len, sizeof(buf_u64));
        }

        if (buf_copy && !buf_ptr) {
            ntStatus = NDIS_STATUS_NOT_SUPPORTED;
        }

        if (buf_copy && (ntStatus == NDIS_STATUS_SUCCESS)) {

            if (info_buffer_length && buf_len <= info_buffer_length) {
                // Copy result into info buffer
                *bytes_written = buf_len;
                if (buf_len) {
                    NdisMoveMemory(info_buffer, buf_ptr, buf_len);
                }
            } else {
                // too short
                ntStatus = NDIS_STATUS_BUFFER_TOO_SHORT;
                *bytes_needed = buf_len;
            }
        }
    }

    return ntStatus;
}

NDIS_STATUS
OidRequest(NDIS_HANDLE MiniportAdapterContext, PNDIS_OID_REQUEST OidRequest)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_SUCCESS;
    struct ionic *ionic = (struct ionic *)MiniportAdapterContext;
    NDIS_OID oid;
    PVOID info_buffer;
    ULONG info_buffer_length;
    PULONG bytes_read;
    PULONG bytes_written;
    PULONG bytes_needed;

    KIRQL start_irql;

    start_irql = KeGetCurrentIrql();

    DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_VERBOSE,
              "%s Entered OidRequest() request %p Type %s Oid %08lX\n",
              __FUNCTION__, OidRequest, GetOidType(OidRequest->RequestType),
              OidRequest->DATA.QUERY_INFORMATION.Oid));

    switch (OidRequest->RequestType) {
    case NdisRequestMethod: {

        if (ionic->hardware_status != NdisHardwareStatusReady) {

            DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_ERROR,
                      "%s Oid %08lX not accepted due to device not ready\n",
                      __FUNCTION__, OidRequest->DATA.QUERY_INFORMATION.Oid));

            ntStatus = NDIS_STATUS_NOT_ACCEPTED;
            goto exit;
        }

        oid = OidRequest->DATA.METHOD_INFORMATION.Oid;

        info_buffer = OidRequest->DATA.METHOD_INFORMATION.InformationBuffer;
        info_buffer_length =
            (ULONG)OidRequest->DATA.METHOD_INFORMATION.InputBufferLength;
        bytes_read = (PULONG)&OidRequest->DATA.METHOD_INFORMATION.BytesRead;
        bytes_written =
            (PULONG)&OidRequest->DATA.METHOD_INFORMATION.BytesWritten;
        bytes_needed = (PULONG)&OidRequest->DATA.METHOD_INFORMATION.BytesNeeded;

        ntStatus = oid_method_information(ionic, oid, OidRequest);

        if (ntStatus != NDIS_STATUS_SUCCESS) {
            DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_ERROR,
                      "%s (NdisRequestMethod) Oid %08lX Status %08lX\n",
                      __FUNCTION__, oid, ntStatus));
        }

        break;
    }

    case NdisRequestSetInformation: {

        if (ionic->hardware_status != NdisHardwareStatusReady) {

            DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_ERROR,
                      "%s Oid %08lX not accepted due to device not ready\n",
                      __FUNCTION__, OidRequest->DATA.QUERY_INFORMATION.Oid));

            ntStatus = NDIS_STATUS_NOT_ACCEPTED;
            goto exit;
        }

        oid = OidRequest->DATA.SET_INFORMATION.Oid;

        info_buffer = OidRequest->DATA.SET_INFORMATION.InformationBuffer;
        info_buffer_length =
            (ULONG)OidRequest->DATA.SET_INFORMATION.InformationBufferLength;
        bytes_read = (PULONG)&OidRequest->DATA.SET_INFORMATION.BytesRead;
        bytes_needed = (PULONG)&OidRequest->DATA.SET_INFORMATION.BytesNeeded;

        ntStatus =
            oid_set_information(ionic, OidRequest, info_buffer,
                                info_buffer_length, bytes_read, bytes_needed);

        if (ntStatus != NDIS_STATUS_SUCCESS) {
            DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_ERROR,
                      "%s (NdisRequestSetInformation) Oid %s Status %08lX\n",
                      __FUNCTION__, GetSetOidName(oid), ntStatus));
        }

        break;
    }

    case NdisRequestQueryInformation:
    case NdisRequestQueryStatistics: {

        if (ionic->hardware_status == NdisHardwareStatusClosing) {

            DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_ERROR,
                      "%s Oid %08lX not accepted due to device not ready\n",
                      __FUNCTION__, OidRequest->DATA.QUERY_INFORMATION.Oid));

            ntStatus = NDIS_STATUS_NOT_ACCEPTED;
            goto exit;
        }

        oid = OidRequest->DATA.QUERY_INFORMATION.Oid;

        info_buffer = OidRequest->DATA.QUERY_INFORMATION.InformationBuffer;
        info_buffer_length =
            (ULONG)OidRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
        bytes_written =
            (PULONG)&OidRequest->DATA.QUERY_INFORMATION.BytesWritten;
        bytes_needed = (PULONG)&OidRequest->DATA.QUERY_INFORMATION.BytesNeeded;

        ntStatus =
            oid_query_information(ionic, oid, info_buffer, info_buffer_length,
                                  bytes_written, bytes_needed);

        if (ntStatus != NDIS_STATUS_SUCCESS) {

            DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_ERROR,
                      "%s (%s) Oid %s Status %08lX\n", __FUNCTION__,
                      GetOidType(OidRequest->RequestType), GetQueryOidName(oid),
                      ntStatus));
        }

        break;
    }

    default:
        ntStatus = NDIS_STATUS_NOT_SUPPORTED;
        DbgTrace((TRACE_COMPONENT_OID, TRACE_LEVEL_ERROR,
                  "%s Unsupported type 0x%08lX\n", __FUNCTION__,
                  OidRequest->RequestType));
        break;
    }

exit:

    ASSERT(start_irql == KeGetCurrentIrql());

    return ntStatus;
}

void
CancelOidRequest(NDIS_HANDLE MiniportAdapterContext, PVOID RequestId)
{

    UNREFERENCED_PARAMETER(MiniportAdapterContext);
    UNREFERENCED_PARAMETER(RequestId);

    return;
}

char *
GetOidType(NDIS_REQUEST_TYPE Type)
{

    char *chType = "Unknown Type";

    switch (Type) {
    case NdisRequestMethod: {
        chType = "NdisRequestMethod";
        break;
    }

    case NdisRequestSetInformation: {
        chType = "NdisRequestSetInformation";
        break;
    }

    case NdisRequestQueryInformation: {
        chType = "NdisRequestQueryInformation";
        break;
    }

    case NdisRequestQueryStatistics: {
        chType = "NdisRequestQueryStatistics";
        break;
    }

    default:
        break;
    }

    return chType;
}

NDIS_STATUS
DirectOidRequest(NDIS_HANDLE MiniportAdapterContext,
                 PNDIS_OID_REQUEST OidRequest)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_NOT_SUPPORTED;

    UNREFERENCED_PARAMETER(MiniportAdapterContext);

    IoPrint("%s Handling Oid %08lX\n", __FUNCTION__,
            OidRequest->DATA.QUERY_INFORMATION.Oid);

    return ntStatus;
}

void
CancelDirectOidRequest(NDIS_HANDLE MiniportAdapterContext, PVOID RequestId)
{

    // NDIS_STATUS			ntStatus = NDIS_STATUS_NOT_SUPPORTED;

    UNREFERENCED_PARAMETER(MiniportAdapterContext);

    IoPrint("%s Handling Oid %p\n", __FUNCTION__, RequestId);

    return;
}

NDIS_STATUS
SynchronousOidRequest(NDIS_HANDLE MiniportAdapterContext,
                      NDIS_OID_REQUEST *OidRequest)
{

    NDIS_STATUS ntStatus = NDIS_STATUS_NOT_SUPPORTED;

    UNREFERENCED_PARAMETER(MiniportAdapterContext);

    IoPrint("%s Handling Oid %08lX\n", __FUNCTION__,
            OidRequest->DATA.QUERY_INFORMATION.Oid);

    return ntStatus;
}