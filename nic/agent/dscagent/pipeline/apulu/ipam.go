// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

// +build apulu

package apulu

import (
	"context"
	"fmt"
	"net"
	"strings"

	"github.com/gogo/protobuf/proto"
	"github.com/pkg/errors"
	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/nic/agent/dscagent/pipeline/apulu/utils"
	"github.com/pensando/sw/nic/agent/dscagent/pipeline/utils/validator"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	halapi "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	"github.com/pensando/sw/venice/utils/log"
)

type serverUUIDs struct {
	UUID       string
	PolicyKeys []string
}

// DHCPServerIPToUUID maps the unique server IP to its UUID and policies using it
var DHCPServerIPToUUID = map[string]*serverUUIDs{}

// IPAMPolicyIDToServerIDs maps the policy ID to a list of UUID
var IPAMPolicyIDToServerIDs = map[string][]string{}

// HandleIPAMPolicy handles CRUD operations on IPAMPolicy
func HandleIPAMPolicy(infraAPI types.InfraAPI, client halapi.DHCPSvcClient, subnetcl halapi.SubnetSvcClient, oper types.Operation, policy netproto.IPAMPolicy, vrfuid []byte) error {
	switch oper {
	case types.Create:
		return createDHCPRelayHandler(infraAPI, client, policy, vrfuid)
	case types.Update:
		return updateDHCPRelayHandler(infraAPI, client, subnetcl, policy, vrfuid)
	case types.Delete:
		return deleteDHCPRelayHandler(infraAPI, client, policy)
	default:
		return errors.Wrapf(types.ErrUnsupportedOp, "Op: %s", oper)
	}
}

func createDHCPRelayHandler(infraAPI types.InfraAPI, client halapi.DHCPSvcClient, policy netproto.IPAMPolicy, vrfuid []byte) error {
	var uuids []string
	for _, srv := range policy.Spec.DHCPRelay.Servers {
		uid, err := createDhcpPolicy(srv.IPAddress, policy, vrfuid, client)
		if err != nil {
			return err
		}
		uuids = append(uuids, uid)
	}

	IPAMPolicyIDToServerIDs[policy.UUID] = uuids
	dat, _ := policy.Marshal()

	if err := infraAPI.Store(policy.Kind, policy.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "DHCPRelay: %s | Err: %v", policy.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreCreate, "DHCPRelay: %s | Err: %v", policy.GetKey(), err)
	}
	return nil
}

func updateDHCPRelayHandler(infraAPI types.InfraAPI, client halapi.DHCPSvcClient, subnetcl halapi.SubnetSvcClient, policy netproto.IPAMPolicy, vrfuid []byte) error {
	var existingPolicy netproto.IPAMPolicy
	dat, err := infraAPI.Read(policy.Kind, policy.GetKey())
	if err != nil {
		log.Error(errors.Wrapf(types.ErrBadRequest, "IPAMPolicy: %s | Err: %v", policy.GetKey(), types.ErrObjNotFound))
		return errors.Wrapf(types.ErrBadRequest, "IPAMPolicy: %s | Err: %v", policy.GetKey(), types.ErrObjNotFound)
	}
	err = existingPolicy.Unmarshal(dat)
	if err != nil {
		log.Error(errors.Wrapf(types.ErrUnmarshal, "IPAMPolicy: %s | Err: %v", policy.GetKey(), err))
		return errors.Wrapf(types.ErrUnmarshal, "IPAMPolicy: %s | Err: %v", policy.GetKey(), err)
	}

	addIPs, deleteIPs := updateIPs(existingPolicy, policy)
	for _, ip := range addIPs {
		_, err := createDhcpPolicy(ip, policy, vrfuid, client)
		if err != nil {
			return err
		}
	}
	for _, ip := range deleteIPs {
		if err := deleteDhcpPolicy(ip, policy, client); err != nil {
			return err
		}
	}

	var uuids []string
	for _, srv := range policy.Spec.DHCPRelay.Servers {
		uuids = append(uuids, DHCPServerIPToUUID[srv.IPAddress].UUID)
	}
	IPAMPolicyIDToServerIDs[policy.UUID] = uuids
	data, err := infraAPI.List("Network")
	if err != nil {
		log.Error(errors.Wrapf(types.ErrBadRequest, "Networks not found: Err: %v", types.ErrObjNotFound))
	}

	for _, o := range data {
		var nw netproto.Network
		err := proto.Unmarshal(o, &nw)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Network: %s | Err: %v", nw.GetKey(), err))
			continue
		}
		// Subnet has self ipam policy
		if nw.Spec.IPAMPolicy != "" && nw.Spec.IPAMPolicy != policy.Name {
			continue
		}
		if nw.Spec.IPAMPolicy == "" {
			vrf, err := validator.ValidateVrf(infraAPI, nw.Tenant, nw.Namespace, nw.Spec.VrfName)
			if err != nil {
				log.Errorf("Get VRF failed for %s | %s", nw.GetKind(), nw.GetKey())
				continue
			}
			if vrf.Spec.IPAMPolicy != policy.Name {
				continue
			}
		}
		// Subnet doesn't use this vpc
		if nw.Tenant != policy.Tenant || nw.Namespace != policy.Namespace {
			continue
		}
		updsubnet, err := convertNetworkToSubnet(infraAPI, nw, nil)
		if err != nil {
			log.Errorf("Network: %s could not convert | Err: %s", nw.GetKey(), err)
			continue
		}
		resp, err := subnetcl.SubnetUpdate(context.TODO(), updsubnet)
		if err != nil {
			log.Errorf("Network: %s update failed | Err: %v", nw.GetKey(), err)
			continue
		}
		if resp.ApiStatus != halapi.ApiStatus_API_STATUS_OK {
			log.Errorf("Network: %s update failed  | Status: %s", nw.GetKey(), resp.ApiStatus)
			continue
		}
		log.Infof("Network: %s update | Err: %v | Status : %v | Resp: %v", nw.GetKey(), err, resp.ApiStatus, resp.Response)
	}

	dat, _ = policy.Marshal()

	if err := infraAPI.Store(policy.Kind, policy.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreUpdate, "DHCPRelay: %s | Err: %v", policy.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreUpdate, "DHCPRelay: %s | Err: %v", policy.GetKey(), err)
	}
	return nil
}

func deleteDHCPRelayHandler(infraAPI types.InfraAPI, client halapi.DHCPSvcClient, policy netproto.IPAMPolicy) error {
	for _, srv := range policy.Spec.DHCPRelay.Servers {
		if err := deleteDhcpPolicy(srv.IPAddress, policy, client); err != nil {
			return err
		}
	}

	delete(IPAMPolicyIDToServerIDs, policy.UUID)
	if err := infraAPI.Delete(policy.Kind, policy.GetKey()); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "DHCPRelay: %s | Err: %v", policy.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreDelete, "DHCPRelay: %s | Err: %v", policy.GetKey(), err)
	}
	return nil
}

func convertIPAMPolicyToDHCPRelay(serverIP string, uID, vpcID []byte) (*halapi.DHCPPolicyRequest, error) {
	// TODO: cleanup this endianness mess !!
	ip := net.ParseIP(serverIP).To4()
	pdsIp := (((uint32(ip[0])*256)+uint32(ip[1]))*256+uint32(ip[2]))*256 + uint32(ip[3])
	return &halapi.DHCPPolicyRequest{
		BatchCtxt: nil,
		Request: []*halapi.DHCPPolicySpec{
			&halapi.DHCPPolicySpec{
				Id: uID,
				RelayOrProxy: &halapi.DHCPPolicySpec_RelaySpec{
					RelaySpec: &halapi.DHCPRelaySpec{
						VPCId: vpcID,
						ServerIP: &halapi.IPAddress{
							Af:     halapi.IPAF_IP_AF_INET,
							V4OrV6: &halapi.IPAddress_V4Addr{V4Addr: pdsIp},
						},
					},
				},
			},
		},
	}, nil
}

func createDhcpPolicy(serverIP string, policy netproto.IPAMPolicy, vrfuid []byte, client halapi.DHCPSvcClient) (string, error) {
	_, ok := DHCPServerIPToUUID[serverIP]
	if ok {
		DHCPServerIPToUUID[serverIP].PolicyKeys = append(DHCPServerIPToUUID[serverIP].PolicyKeys, policy.GetKey())
		return DHCPServerIPToUUID[serverIP].UUID, nil
	}
	uid := uuid.NewV4().String()
	dhcpuid, err := uuid.FromString(uid)
	if err != nil {
		log.Errorf("DHCPRelay: %s | could not parse dhcp UUID | Err: %v", uid, err)
		return "", errors.Wrapf(err, "parse vrf UUID")
	}

	dhcpRelayReq, err := convertIPAMPolicyToDHCPRelay(serverIP, dhcpuid.Bytes(), vrfuid)
	if err != nil {
		log.Error(errors.Wrapf(types.ErrBadRequest, "DHCPRelay: %s | Err: %v", policy.GetKey(), err))
		return "", errors.Wrapf(types.ErrBadRequest, "DHCPRelay: %s | Err: %v", policy.GetKey(), err)
	}

	resp, err := client.DHCPPolicyCreate(context.Background(), dhcpRelayReq)
	if err != nil {
		log.Errorf("DHCPRelay: %s create failed | Err: %v", serverIP, err)
		return "", errors.Wrapf(types.ErrDatapathHandling, "DHCPRelay: %s create failed | Err: %v", serverIP, err)
	}
	if resp != nil {
		if err := utils.HandleErr(types.Create, resp.ApiStatus, err, fmt.Sprintf("Create Failed for %s | %s", policy.GetKind(), policy.GetKey())); err != nil {
			return "", err
		}
	}
	DHCPServerIPToUUID[serverIP] = &serverUUIDs{
		UUID:       uid,
		PolicyKeys: []string{policy.GetKey()},
	}
	return uid, nil
}

func deleteDhcpPolicy(serverIP string, policy netproto.IPAMPolicy, client halapi.DHCPSvcClient) error {
	policyKeys, ok := DHCPServerIPToUUID[serverIP]
	if !ok {
		log.Infof("DHCPRelay: %s already deleted", serverIP)
		return nil
	}
	// Remove the policy key from the map
	length := len(policyKeys.PolicyKeys)
	index := -1
	for idx, m := range policyKeys.PolicyKeys {
		if m == policy.GetKey() {
			index = idx
			break
		}
	}
	if index != -1 {
		policyKeys.PolicyKeys[index] = policyKeys.PolicyKeys[length-1]
		policyKeys.PolicyKeys = policyKeys.PolicyKeys[:length-1]
	}

	if len(policyKeys.PolicyKeys) != 0 {
		log.Infof("DHCPRelay: %s still referenced by %s", serverIP, strings.Join(policyKeys.PolicyKeys, " "))
		return nil
	}
	uid, err := uuid.FromString(policyKeys.UUID)
	if err != nil {
		log.Errorf("DHCPRelay: %s | could not parse UUID | Err: %v", policyKeys.UUID, err)
		return errors.Wrapf(err, "parse UUID")
	}

	dhcpRelayDelReq := &halapi.DHCPPolicyDeleteRequest{
		Id: [][]byte{uid.Bytes()},
	}

	resp, err := client.DHCPPolicyDelete(context.Background(), dhcpRelayDelReq)
	if err != nil {
		log.Errorf("DHCPRelay: %s delete failed | Err: %v", serverIP, err)
		return errors.Wrapf(types.ErrDatapathHandling, "DHCPRelay: %s delete failed | Err: %v", serverIP, err)
	}
	if resp != nil {
		if err := utils.HandleErr(types.Delete, resp.ApiStatus[0], err, fmt.Sprintf("DHCPRelay: %s", serverIP)); err != nil {
			return err
		}
	}

	delete(DHCPServerIPToUUID, serverIP)
	return nil
}

func updateIPs(curPolicy, policy netproto.IPAMPolicy) (addIPs, deleteIPs []string) {
	curIPs := map[string]bool{}
	newIPs := map[string]bool{}
	for _, srv := range curPolicy.Spec.DHCPRelay.Servers {
		curIPs[srv.IPAddress] = true
	}
	for _, srv := range policy.Spec.DHCPRelay.Servers {
		newIPs[srv.IPAddress] = true
	}
	for ip := range curIPs {
		if !newIPs[ip] {
			deleteIPs = append(deleteIPs, ip)
		}
	}
	for ip := range newIPs {
		if !curIPs[ip] {
			addIPs = append(addIPs, ip)
		}
	}
	return
}
