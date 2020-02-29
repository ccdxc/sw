// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

// +build apulu

package apulu

import (
	"context"
	"fmt"

	"github.com/pkg/errors"
	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/nic/agent/dscagent/pipeline/apulu/utils"
	"github.com/pensando/sw/nic/agent/dscagent/pipeline/utils/validator"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	halapi "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	"github.com/pensando/sw/venice/utils/log"
)

// HandleIPAMPolicy handles CRUD operations on IPAMPolicy
func HandleIPAMPolicy(infraAPI types.InfraAPI, client halapi.DHCPSvcClient, oper types.Operation, policy netproto.IPAMPolicy) error {
	switch oper {
	case types.Create:
		return createDHCPRelayHandler(infraAPI, client, policy)
	case types.Update:
		return updateDHCPRelayHandler(infraAPI, client, policy)
	case types.Delete:
		return deleteDHCPRelayHandler(infraAPI, client, policy)
	default:
		return errors.Wrapf(types.ErrUnsupportedOp, "Op: %s", oper)
	}
}

func createDHCPRelayHandler(infraAPI types.InfraAPI, client halapi.DHCPSvcClient, policy netproto.IPAMPolicy) error {
	dhcpRelayReq, err := convertIPAMPolicyToDHCPRelay(infraAPI, policy)
	if err != nil {
		log.Error(errors.Wrapf(types.ErrBadRequest, "DHCPRelay: %s | Err: %v", policy.GetKey(), err))
		return errors.Wrapf(types.ErrBadRequest, "DHCPRelay: %s | Err: %v", policy.GetKey(), err)
	}

	resp, err := client.DHCPRelayCreate(context.Background(), dhcpRelayReq)
	if resp != nil {
		if err := utils.HandleErr(types.Create, resp.ApiStatus, err, fmt.Sprintf("Create Failed for %s | %s", policy.GetKind(), policy.GetKey())); err != nil {
			return err
		}
	}

	dat, _ := policy.Marshal()

	if err := infraAPI.Store(policy.Kind, policy.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreCreate, "DHCPRelay: %s | Err: %v", policy.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreCreate, "DHCPRelay: %s | Err: %v", policy.GetKey(), err)
	}
	return nil
}

func updateDHCPRelayHandler(infraAPI types.InfraAPI, client halapi.DHCPSvcClient, policy netproto.IPAMPolicy) error {
	dhcpRelayReq, err := convertIPAMPolicyToDHCPRelay(infraAPI, policy)
	if err != nil {
		log.Error(errors.Wrapf(types.ErrBadRequest, "DHCPRelay: %s | Err: %v", policy.GetKey(), err))
		return errors.Wrapf(types.ErrBadRequest, "DHCPRelay: %s | Err: %v", policy.GetKey(), err)
	}

	resp, err := client.DHCPRelayUpdate(context.Background(), dhcpRelayReq)
	if resp != nil {
		if err := utils.HandleErr(types.Update, resp.ApiStatus, err, fmt.Sprintf("Update Failed for %s | %s", policy.GetKind(), policy.GetKey())); err != nil {
			return err
		}
	}
	dat, _ := policy.Marshal()

	if err := infraAPI.Store(policy.Kind, policy.GetKey(), dat); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreUpdate, "DHCPRelay: %s | Err: %v", policy.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreUpdate, "DHCPRelay: %s | Err: %v", policy.GetKey(), err)
	}
	return nil
}

func deleteDHCPRelayHandler(infraAPI types.InfraAPI, client halapi.DHCPSvcClient, policy netproto.IPAMPolicy) error {
	uid, err := uuid.FromString(policy.UUID)
	if err != nil {
		log.Errorf("DHCPRelay: %s | could not parse UUID | Err: %v", policy.GetKey(), err)
		return errors.Wrapf(err, "parse UUID")
	}

	dhcpRelayDelReq := &halapi.DHCPRelayDeleteRequest{
		Id: [][]byte{uid.Bytes()},
	}

	resp, err := client.DHCPRelayDelete(context.Background(), dhcpRelayDelReq)
	if resp != nil {
		if err := utils.HandleErr(types.Delete, resp.ApiStatus[0], err, fmt.Sprintf("DHCPRelay: %s", policy.GetKey())); err != nil {
			return err
		}
	}

	if err := infraAPI.Delete(policy.Kind, policy.GetKey()); err != nil {
		log.Error(errors.Wrapf(types.ErrBoltDBStoreDelete, "DHCPRelay: %s | Err: %v", policy.GetKey(), err))
		return errors.Wrapf(types.ErrBoltDBStoreDelete, "DHCPRelay: %s | Err: %v", policy.GetKey(), err)
	}
	return nil
}

func convertIPAMPolicyToDHCPRelay(infraAPI types.InfraAPI, policy netproto.IPAMPolicy) (*halapi.DHCPRelayRequest, error) {
	uid, err := uuid.FromString(policy.UUID)
	if err != nil {
		log.Errorf("DHCPRelay: %s | could not parse UUID | Err: %v", policy.GetKey(), err)
		return nil, errors.Wrapf(err, "parse UUID")
	}

	vrf, err := validator.ValidateVrf(infraAPI, types.DefaultTenant, types.DefaultNamespace, types.DefaulUnderlaytVrf)
	if err != nil {
		log.Errorf("Get VRF failed for %s", types.DefaulUnderlaytVrf)
		return nil, errors.Wrapf(err, "Get vrf failed")
	}

	vrfuid, err := uuid.FromString(vrf.UUID)
	if err != nil {
		log.Errorf("DHCPRelay: %s | could not parse vrf UUID | Err: %v", policy.GetKey(), err)
		return nil, errors.Wrapf(err, "parse vrf UUID")
	}

	ret := &halapi.DHCPRelayRequest{
		BatchCtxt: nil,
		Request:   []*halapi.DHCPRelaySpec{},
	}

	for _, srv := range policy.Spec.DHCPRelay.Servers {
		dhcpRelay := &halapi.DHCPRelaySpec{
			Id:       uid.Bytes(),
			ServerIP: ip2PDSType(srv.IPAddress),
			VPCId:    vrfuid.Bytes(),
		}
		ret.Request = append(ret.Request, dhcpRelay)
	}

	return ret, nil
}
