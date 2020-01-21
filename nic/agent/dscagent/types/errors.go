package types

type (
	// Error wraps constant error types
	Error string

	// Info wraps Info messagesfo  string
	Info string
)

// Error wraps constant error types
func (e Error) Error() string { return string(e) }

// Info wraps Info messages
func (i Info) Info() string { return string(i) }

// Const Errors
const (
	ErrNotImplemented                     = Error("ErrNotImplemented")
	ErrUnsupportedOp                      = Error("ErrNotSupported")
	ErrDatapathTransport                  = Error("ErrDatapathTransport")
	ErrDatapathHandling                   = Error("ErrDatapathHandling")
	ErrPipelineNotAvailabe                = Error("ErrPipelienNotAvailable")
	ErrPipelineTimeout                    = Error("ErrPipelineTimeout")
	ErrPipelineNotReady                   = Error("ErrPipelineNotReady")
	ErrBadRequest                         = Error("ErrBadRequest")
	ErrUnmarshal                          = Error("ErrUnmarshal")
	ErrMarshal                            = Error("ErrMarshal")
	ErrBoltDBStoreCreate                  = Error("ErrBoltDBStoreCreate")
	ErrBoltDBStoreUpdate                  = Error("ErrBoltDBStoreUpdate")
	ErrBoltDBStoreDelete                  = Error("ErrBoltDBStoreDelete")
	ErrObjExists                          = Error("ErrObjExists")
	ErrObjNotFound                        = Error("ErrObjectNotFound")
	ErrNotPermitted                       = Error("ErrNotPermitted")
	ErrEmptyFields                        = Error("ErrEmptyFields")
	ErrDuplicateVLANID                    = Error("ErrDuplicateVLANID")
	ErrEnicUnwind                         = Error("ErrEnicUnwind")
	ErrInvalidIP                          = Error("ErrInvalidIP")
	ErrInvalidIPRange                     = Error("ErrInvalidIPRange")
	ErrInvalidIPPrefix                    = Error("ErrInvalidIPPrefix")
	ErrInvalidMACAddress                  = Error("ErrInvalidMACAddress")
	ErrInvalidInterfaceType               = Error("ErrInvalidInterfaceType")
	ErrMultipleALGInApp                   = Error("ErrMultipleALGInApp")
	ErrMissingPolicyAttachmentPoint       = Error("ErrMissingPolicyAttachmentPoint")
	ErrPolicyDualAttachment               = Error("ErrPolicyDualAttachment")
	ErrNPMControllerStart                 = Error("ErrNPMControllerStart")
	ErrControllerWatcherStop              = Error("ErrControllerWatcherStop")
	ErrNPMUnavailable                     = Error("ErrNPMUnavailable")
	ErrTPMUnavailable                     = Error("ErrTPMUnavailable")
	ErrTSMUnavailable                     = Error("ErrTSMUnavailable")
	ErrNimbusClient                       = Error("ErrNimbusClient")
	ErrNimbusHandling                     = Error("ErrNimbusHandling")
	ErrAggregateWatch                     = Error("ErrAggregateWatch")
	ErrMirrorSessionWatch                 = Error("ErrMirrorSessionWatch")
	ErrMirrorSessionStream                = Error("ErrMirrorSessionStream")
	ErrFlowExportPolicyWatch              = Error("ErrFlowExportPolicyWatch")
	ErrNPMWatcherClose                    = Error("ErrNPMWatcherClose")
	ErrTSMWatcherClose                    = Error("ErrTSMWatcherClose")
	ErrTPMWatcherClose                    = Error("ErrTPMWatcherClose")
	ErrRESTServerClose                    = Error("ErrRESTServerClose")
	ErrRESTServerStart                    = Error("ErrRESTServerStart")
	ErrRESTServerCreate                   = Error("ErrRESTServerCreate")
	ErrDBPathCreate                       = Error("ErrDBPathCreate")
	ErrPrimaryStoreClose                  = Error("ErrPrimaryStoreClose")
	ErrBackupStoreClose                   = Error("ErrBackupStoreClose")
	ErrPrimaryStoreCreate                 = Error("ErrPrimaryStoreCreate")
	ErrBackupStoreCreate                  = Error("ErrBackupStoreCreate")
	ErrMissingStorePaths                  = Error("ErrMissingStorePaths")
	ErrBackupStoreWrite                   = Error("ErrBackupStoreWrite")
	ErrBackupStoreDelete                  = Error("ErrBackupStoreDelete")
	ErrPrimaryStoreIDAlloc                = Error("ErrPrimaryStoreIDAlloc")
	ErrBackupStoreIDAlloc                 = Error("ErrBackupStoreIDAlloc")
	ErrPipelineInit                       = Error("ErrPipelineInit")
	ErrPipelineLifGet                     = Error("ErrPipelineLifGet")
	ErrPipelineLifStream                  = Error("ErrPipelineLifStream")
	ErrPipelineEventListen                = Error("ErrPipelineEventListen")
	ErrPipelineEventRecv                  = Error("ErrPipelineEventRecv")
	ErrPipelinePortGet                    = Error("ErrPipelinePortGet")
	ErrPipelineEventStreamClosed          = Error("ErrPipelineEventStreamClosed")
	ErrDuplicateRuleinAppAndInlineRules   = Error("ErrDuplicateRuleinAppAndInlineRules")
	ErrARPManagementInterfaceNotFound     = Error("ErrARPManagementInterfaceNotFound")
	ErrARPClientDialFailure               = Error("ErrARPClientDialFailure")
	ErrARPResolution                      = Error("ErrARPResolution")
	ErrARPResolutionTimeoutExceeded       = Error("ErrARPResolutionTimeoutExceeded")
	ErrARPMissingDMAC                     = Error("ErrARPMissingDMAC")
	ErrCollectorEPCreateFailure           = Error("ErrCollectorEPCreateFailure")
	ErrCollectorEPUpdateFailure           = Error("ErrCollectorEPUpdateFailure")
	ErrCollectorTunnelCreateFailure       = Error("ErrCollectorTunnelCreateFailure")
	ErrMirrorCollectorCreateFailure       = Error("ErrMirrorCollectorCreateFailure")
	ErrMirrorFlowMonitorRuleCreateFailure = Error("ErrMirrorFlowMonitorRuleCreateFailure")
	ErrMirrorSessionDeleteDuringUpdate    = Error("ErrMirrorSessionDeleteDuringUpdate")
	ErrMirrorSessionCreateDuringUpdate    = Error("ErrMirrorSessionCreateDuringUpdate")
	ErrARPEntryMissingForDefaultGateway   = Error("ErrARPEntryMissingForDefaultGateway")
	ErrARPEntryMissingForSameSubnetIP     = Error("ErrARPEntryMissingForSameSubnetIP")
	ErrNoRoutesFoundForDestIP             = Error("ErrNoRoutesFoundForDestIP")
	ErrDefaultGatewayNotConfigured        = Error("ErrDefaultGatewayNotConfigured")
	ErrCollectorEPDeleteFailure           = Error("ErrCollectorEPDeleteFailure")
	ErrCollectorTunnelDeleteFailure       = Error("ErrCollectorTunnelDeleteFailure")
	ErrMirrorSessionControllerHandle      = Error("ErrMirrorSessionControllerHandle")
	ErrFlowExportPolicyStream             = Error("ErrFlowExportPolicyStream")
	ErrFlowExportPolicyControllerHandle   = Error("ErrFlowExportPolicyControllerHandle")
	ErrFlowExportPolicyDeleteDuringUpdate = Error("ErrFlowExportPolicyDeleteDuringUpdate")
	ErrFlowExportPolicyCreateDuringUpdate = Error("ErrFlowExportPolicyCreateDuringUpdate")
	ErrIPFIXTemplateCreate                = Error("ErrIPFixTemplateCreate")
	ErrIPFIXTemplateSockSend              = Error("ErrIPFixTemplateSockSend")
	ErrIPFIXPacketListen                  = Error("ErrIPFIXPacketListen")
	ErrIPFIXPacketSend                    = Error("ErrIPFIXPacketSend")
	ErrMirrorCreateLateralObjects         = Error("ErrMirrorCreateLateralObjects")
	ErrNetflowCreateLateralObjects        = Error("ErrNetflowCreateLateralObjects")
	ErrMirrorDeleteLateralObjects         = Error("ErrMirrorDeleteLateralObjects")
	ErrNetflowDeleteLateralObjects        = Error("ErrNetflowDeleteLateralObjects")
	ErrMaxMirrorSessionsConfigured        = Error("ErrMaxMirrorSessionsConfigured")
)

// Const Info
const (
	InfoIgnoreDelete                 = Info("InfoIgnoreDelete")
	InfoNPMWatcherDone               = Info("InfoNPMWatcherDone")
	InfoNPMWatcherReaped             = Info("InfoNPMWatcherReaped")
	InfoHandlingVeniceCoordinates    = Info("InfoHandlingVeniceCoordinates")
	InfoUpdateVeniceCoordinates      = Info("InfoUpdateVeniceCoordinates")
	InfoConnectedToNPM               = Info("InfoConnectedToNPM")
	InfoConnectedToTSM               = Info("InfoConnectedToTSM")
	InfoConnectedToTPM               = Info("InfoConnectedToTPM")
	InfoAggWatchStarted              = Info("InfoAggWatchStarted")
	InfoControllerReconnecting       = Info("InfoControllerReconnecting")
	InfoPipelineInit                 = Info("InfoPipelineInit")
	InfoDefaultUntaggedNetworkCreate = Info("InfoDefaultUntaggedNetworkCreate")
	InfoDefaultVrfCreate             = Info("InfoDefaultVrfCreate")
	InfoSingletonLifGet              = Info("InfoSingletonLifGet")
	InfoConnectedToHAL               = Info("InfoConnectedToHAL")
	InfoConnectedToPDS               = Info("InfoConnectedToPDS")
	InfoHandleObjBegin               = Info("InfoHandleObjBegin")
	InfoHandleObjEnd                 = Info("InfoHandleObjEnd")
	InfoTSDBInitDone                 = Info("InfoTSDBInitDone")
	InfoRPClientFactoryInit          = Info("InfoRPClientFactoryInit")
	InfoDeviceCreate                 = Info("InfoDeviceCreate")
	InfoSecurityProfileCreate        = Info("InfoSecurityProfileCreate")
	InfoKnownEPUpdateNeeded          = Info("InfoKnownEPUpdateNeeded")
	InfoKnownEPNoUpdateNeeded        = Info("InfoKnownEPNoUpdateNeeded")
	InfoUnknownEPCreateNeeded        = Info("InfoUnknownEPCreateNeeded")
	InfoUnknownTunnelCreateNeeded    = Info("InfoUnknownTunnelCreateNeeded")
	InfoARPingForDefaultGateway      = Info("InfoARPingForDefaultGateway")
	InfoARPingForSameSubnetIP        = Info("InfoARPingForSameSubnetIP")
	InfoCollectorEPDeleteNeeded      = Info("InfoCollectorEPDeleteNeeded")
	InfoCollectorTunnelDeleteNeeded  = Info("InfoCollectorTunnelDeleteNeeded")
	InfoMirrorSessionEvt             = Info("InfoMirrorSessionEvt")
	InfoFlowExportPolicyEvt          = Info("InfoFlowExportPolicyEvt")
	InfoTSMWatcherDone               = Info("InfoTSMWatcherDone")
	InfoTPMWatcherDone               = Info("InfoTPMWatcherDone")
	InfoTemplateSendStart            = Info("InfoTemplateSendStart")
	InfoTemplateSendStop             = Info("InfoTemplateSendStop")
)
