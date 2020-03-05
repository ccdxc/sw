// +build iris

package iris

import (
	"context"
	"encoding/binary"
	"fmt"
	"net"

	"github.com/pkg/errors"

	"github.com/pensando/sw/nic/agent/dscagent/pipeline/iris/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	halapi "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// HandleEndpoint handles crud operations on endpoint
func HandleEndpoint(infraAPI types.InfraAPI, epClient halapi.EndpointClient, intfClient halapi.InterfaceClient, oper types.Operation, endpoint netproto.Endpoint, vrfID, networkID uint64) error {
	log.Infof("CREATE EP Handler")
	switch oper {
	case types.Create:
		return createEndpointHandler(infraAPI, epClient, intfClient, endpoint, vrfID, networkID)
	case types.Update:
		return updateEndpointHandler(infraAPI, epClient, intfClient, endpoint, vrfID, networkID)
	case types.Delete:
		return deleteEndpointHandler(infraAPI, epClient, intfClient, endpoint, vrfID, networkID)
	default:
		return errors.Wrapf(types.ErrUnsupportedOp, "Op: %s", oper)
	}
}

func createEndpointHandler(infraAPI types.InfraAPI, epClient halapi.EndpointClient, intfClient halapi.InterfaceClient, endpoint netproto.Endpoint, vrfID, networkID uint64) error {
	createEnic := endpoint.Status.EnicID != 0
	// Handle interface creates for local EPs
	if createEnic {
		interfaceReqMsg := convertEnicInterface(endpoint.Spec.MacAddress, endpoint.Status.EnicID, networkID, endpoint.Spec.UsegVlan)
		log.Infof("ENIC Msg: %v", interfaceReqMsg.String())
		resp, err := intfClient.InterfaceCreate(context.Background(), interfaceReqMsg)
		if resp != nil {
			if err := utils.HandleErr(types.Create, resp.Response[0].ApiStatus, err, fmt.Sprintf("create Failed for ENIC Interface. ID: %d", endpoint.Status.EnicID)); err != nil {
				return err
			}
		}
	}

	endpointReqMsg := convertEndpoint(endpoint, infraAPI.GetConfig().MgmtIntf, vrfID, networkID)
	if endpointReqMsg == nil {
		log.Errorf("Converting EP failed. ")
		return fmt.Errorf("failed to convert ep")
	}

	resp, err := epClient.EndpointCreate(context.Background(), endpointReqMsg)
	if resp != nil {
		if err := utils.HandleErr(types.Create, resp.Response[0].ApiStatus, err, fmt.Sprintf("Create Failed for %s | %s", endpoint.GetKind(), endpoint.GetKey())); err != nil {
			if createEnic {
				// Delete enic here
				intfDelReq := &halapi.InterfaceDeleteRequestMsg{
					Request: []*halapi.InterfaceDeleteRequest{
						{
							KeyOrHandle: convertIfKeyHandles(0, endpoint.Status.EnicID)[0],
						},
					},
				}
				_, err := intfClient.InterfaceDelete(context.Background(), intfDelReq)
				if err != nil {
					log.Error(errors.Wrapf(types.ErrEnicUnwind, "Endpoint: %s | Enic ID: %d", endpoint.GetKey(), endpoint.Status.EnicID))
				}
			}
			return err
		}
	}

	endpoint.Status.NodeUUID = endpoint.Spec.NodeUUID
	dat, _ := endpoint.Marshal()

	if err := infraAPI.Store(endpoint.Kind, endpoint.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "Endpoint: %s | Endpoint: %v", endpoint.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreCreate, "Endpoint: %s | Endpoint: %v", endpoint.GetKey(), err)
	}
	return nil
}

func updateEndpointHandler(infraAPI types.InfraAPI, epClient halapi.EndpointClient, intfClient halapi.InterfaceClient, endpoint netproto.Endpoint, vrfID, networkID uint64) error {
	endpointReqMsg := convertEndpointUpdate(endpoint, infraAPI.GetConfig().MgmtIntf, vrfID, networkID)
	log.Infof("Endpoint Msg: %v", endpointReqMsg.String())
	resp, err := epClient.EndpointUpdate(context.Background(), endpointReqMsg)
	if resp != nil {
		if err := utils.HandleErr(types.Update, resp.Response[0].ApiStatus, err, fmt.Sprintf("Update Failed for %s | %s", endpoint.GetKind(), endpoint.GetKey())); err != nil {
			return err
		}
	}
	dat, _ := endpoint.Marshal()

	if err := infraAPI.Store(endpoint.Kind, endpoint.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreUpdate, "Endpoint: %s | Endpoint: %v", endpoint.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreUpdate, "Endpoint: %s | Endpoint: %v", endpoint.GetKey(), err)
	}
	return nil
}

func deleteEndpointHandler(infraAPI types.InfraAPI, epClient halapi.EndpointClient, intfClient halapi.InterfaceClient, endpoint netproto.Endpoint, vrfID, networkID uint64) error {
	defer func() {
		if endpoint.Status.EnicID != 0 {
			// Delete enic here
			intfDelReq := &halapi.InterfaceDeleteRequestMsg{
				Request: []*halapi.InterfaceDeleteRequest{
					{
						KeyOrHandle: convertIfKeyHandles(0, endpoint.Status.EnicID)[0],
					},
				},
			}
			log.Infof("ENIC Msg: %v", intfDelReq.String())

			resp, err := intfClient.InterfaceDelete(context.Background(), intfDelReq)
			if resp != nil {
				log.Error(utils.HandleErr(types.Update, resp.Response[0].ApiStatus, err, fmt.Sprintf("Endpoint: %s | Enic ID: %d", endpoint.GetKind(), endpoint.Status.EnicID)))
			}
		}
	}()

	endpointDelReq := &halapi.EndpointDeleteRequestMsg{
		Request: []*halapi.EndpointDeleteRequest{
			{
				DeleteBy: &halapi.EndpointDeleteRequest_KeyOrHandle{
					KeyOrHandle: convertEpKeyHandle(networkID, endpoint.Spec.MacAddress),
				},
				VrfKeyHandle: convertVrfKeyHandle(vrfID),
			},
		},
	}

	log.Infof("Endpoint Msg: %v", endpointDelReq.String())
	resp, err := epClient.EndpointDelete(context.Background(), endpointDelReq)
	if resp != nil {
		if err := utils.HandleErr(types.Delete, resp.Response[0].ApiStatus, err, fmt.Sprintf("Endpoint: %s", endpoint.GetKey())); err != nil {
			return err
		}
	}

	if err := infraAPI.Delete(endpoint.Kind, endpoint.GetKey()); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "Endpoint: %s | Err: %v", endpoint.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreDelete, "Endpoint: %s | Err: %v", endpoint.GetKey(), err)
	}
	return nil
}

func getHalAPIMigration(migrationState string) halapi.MigrationState {
	switch migrationState {
	case netproto.MigrationState_START.String():
		return halapi.MigrationState_IN_PROGRESS
	case netproto.MigrationState_DONE.String():
		return halapi.MigrationState_SUCCESS
	case netproto.MigrationState_ABORT.String():
		return halapi.MigrationState_ABORTED
	}

	return halapi.MigrationState_NONE
}

func getNetprotoMigration(migrationState halapi.MigrationState) string {
	switch migrationState {
	case halapi.MigrationState_IN_PROGRESS:
		return netproto.MigrationState_START.String()
	case halapi.MigrationState_SUCCESS:
		return netproto.MigrationState_DONE.String()
	case halapi.MigrationState_ABORTED:
		fallthrough
	case halapi.MigrationState_FAILED:
		return netproto.MigrationState_ABORT.String()
	}

	return netproto.MigrationState_NONE.String()
}

func convertEndpoint(endpoint netproto.Endpoint, mgmtIntf string, vrfID, networkID uint64) *halapi.EndpointRequestMsg {

	ret := &halapi.EndpointRequestMsg{
		Request: []*halapi.EndpointSpec{
			{
				KeyOrHandle:   convertEpKeyHandle(networkID, endpoint.Spec.MacAddress),
				EndpointAttrs: convertEPAttrs(endpoint.Spec.IPv4Addresses, mgmtIntf, endpoint.Spec.UsegVlan, endpoint.Status.EnicID, networkID),
				VrfKeyHandle:  convertVrfKeyHandle(vrfID),
			},
		},
	}

	if endpoint.Spec.Migration != netproto.MigrationState_NONE.String() && len(endpoint.Status.NodeUUID) != 0 && endpoint.Spec.NodeUUID != endpoint.Status.NodeUUID {
		log.Infof("Migration in progress")
		if len(endpoint.Spec.HomingHostAddr) == 0 {
			log.Errorf("No homing host IP passed. Cannot initiate migration.")
			return nil
		}

		ret.Request[0].EndpointAttrs.VmotionState = getHalAPIMigration(endpoint.Spec.Migration)
		ret.Request[0].EndpointAttrs.OldHomingHostIp = utils.ConvertIPAddresses([]string{endpoint.Spec.HomingHostAddr}...)[0]
	}

	return ret
}

// TODO get HAL to accept same message types for create and update
func convertEndpointUpdate(endpoint netproto.Endpoint, mgmtIntf string, vrfID, networkID uint64) *halapi.EndpointUpdateRequestMsg {
	updateReq := &halapi.EndpointUpdateRequestMsg{
		Request: []*halapi.EndpointUpdateRequest{
			{
				KeyOrHandle:   convertEpKeyHandle(networkID, endpoint.Spec.MacAddress),
				EndpointAttrs: convertEPAttrs(endpoint.Spec.IPv4Addresses, mgmtIntf, endpoint.Spec.UsegVlan, endpoint.Status.EnicID, networkID),
				VrfKeyHandle:  convertVrfKeyHandle(vrfID),
			},
		},
	}

	updateReq.Request[0].EndpointAttrs.VmotionState = getHalAPIMigration(endpoint.Spec.Migration)
	return updateReq
}

func convertEpKeyHandle(l2SegID uint64, macAddr string) *halapi.EndpointKeyHandle {
	return &halapi.EndpointKeyHandle{
		KeyOrHandle: &halapi.EndpointKeyHandle_EndpointKey{
			EndpointKey: &halapi.EndpointKey{
				EndpointL2L3Key: &halapi.EndpointKey_L2Key{
					L2Key: &halapi.EndpointL2Key{
						L2SegmentKeyHandle: convertL2SegKeyHandle(l2SegID),
						MacAddress:         utils.ConvertMacAddress(macAddr),
					},
				},
			},
		},
	}
}

func convertEPAttrs(addresses []string, mgmtIntf string, usegVLAN uint32, enicID, l2SegID uint64) *halapi.EndpointAttributes {
	var ifKeyHandle *halapi.InterfaceKeyHandle
	// Populate ifKeyHandle only for local EPs which have a valid EnicID
	if enicID != 0 {
		ifKeyHandle = convertIfKeyHandles(0, enicID)[0]
	}

	// HAL expects agent to send uplink130 mgmt for RemoteCollector EP only when the management interface is oob_mnic0
	// This is error prone and is extremely hacky to support this configuration.
	// TODO fix this hack when HAL side telemetry code is cleaned up and removes the remote EP handholding of specific uplinks. HAL should be able to determine the reachability of EPs
	if enicID == 0 && l2SegID == types.UntaggedCollVLAN && mgmtIntf == types.NaplesOOBInterface {
		ifKeyHandle = convertIfKeyHandles(0, 0x51030001)[0]
	} else if enicID == 0 {
		// Remote EPs in HAL also expect agent to send an uplink ID even though it has logic to determine the active uplink.
		// Remote EP Creates without an uplink ID is incorrectly getting rejected which causes IOTA Sanity failures.
		// Coding specific things to make tests pass is bad. TODO clean this up on HAL Svc Validation for RemoteEP Creates
		ifKeyHandle = convertIfKeyHandles(0, 0x51010001)[0]
	}

	return &halapi.EndpointAttributes{
		InterfaceKeyHandle: ifKeyHandle,
		UsegVlan:           usegVLAN,
		IpAddress:          utils.ConvertIPAddresses(addresses...),
	}
}

func convertEnicInterface(macAddress string, intfID, l2SegID uint64, usegVLAN uint32) *halapi.InterfaceRequestMsg {
	return &halapi.InterfaceRequestMsg{
		Request: []*halapi.InterfaceSpec{
			{
				KeyOrHandle: convertIfKeyHandles(0, intfID)[0],
				Type:        halapi.IfType_IF_TYPE_ENIC,
				IfInfo: &halapi.InterfaceSpec_IfEnicInfo{
					IfEnicInfo: &halapi.IfEnicInfo{
						EnicType: halapi.IfEnicType_IF_ENIC_TYPE_USEG,
						EnicTypeInfo: &halapi.IfEnicInfo_EnicInfo{
							EnicInfo: &halapi.EnicInfo{
								L2SegmentKeyHandle: convertL2SegKeyHandle(l2SegID),
								MacAddress:         utils.ConvertMacAddress(macAddress),
								EncapVlanId:        usegVLAN,
							},
						},
					},
				},
			},
		},
	}
}

// ipv4ToUint32 converts net.IP to a 32 bit integer
func ipv4Touint32(ip net.IP) uint32 {
	if len(ip) == 16 {
		return binary.BigEndian.Uint32(ip[12:16])
	}
	return binary.BigEndian.Uint32(ip)
}
