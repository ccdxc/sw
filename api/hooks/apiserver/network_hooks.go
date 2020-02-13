// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package impl

import (
	"context"
	"errors"
	"fmt"
	"time"

	"github.com/gogo/protobuf/types"
	"github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/network"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apiserver"
	apisrvpkg "github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

type networkHooks struct {
	svc    apiserver.Service
	logger log.Logger
}

func (h *networkHooks) validateIPAMPolicyConfig(i interface{}, ver string, ignStatus, ignoreSpec bool) []error {
	cfg, ok := i.(network.IPAMPolicy)

	if ok == false {
		return []error{errors.New("Invalid input configuration")}
	}

	// in the current implementation only one dhcp server configuration is supported even though the model
	// defines it as a slice for future enhancements
	if len(cfg.Spec.DHCPRelay.Servers) > 1 {
		return []error{errors.New("Only one DHCP server configuration is supported")}
	}
	return nil
}

func (h *networkHooks) validateNetworkConfig(i interface{}, ver string, ignStatus, ignoreSpec bool) []error {
	var ret []error

	in := i.(network.Network)
	if in.Spec.Type == network.NetworkType_Routed.String() {
		if in.Spec.IPv4Subnet == "" || in.Spec.IPv4Gateway == "" {
			ret = append(ret, fmt.Errorf("IP Subnet information must be specified for routed networks"))
		}
		if in.Spec.IPv6Gateway != "" || in.Spec.IPv6Subnet != "" {
			ret = append(ret, fmt.Errorf("IPv6 not supported"))
		}
	}

	if in.Spec.RouteImportExport != nil {
		if in.Spec.RouteImportExport.RDAuto && in.Spec.RouteImportExport.RD != nil {
			ret = append(ret, fmt.Errorf("rd and rd-auto specified, only one of them can be specified"))
		}
	}

	if len(in.Spec.IngressSecurityPolicy) > 2 {
		ret = append(ret, fmt.Errorf("maximum of 2 ingress security policies are allowed"))
	}
	if len(in.Spec.EgressSecurityPolicy) > 2 {
		ret = append(ret, fmt.Errorf("maximum of 2 egress security policies are allowed"))
	}
	return ret
}

func (h *networkHooks) validateVirtualrouterConfig(i interface{}, ver string, ignStatus, ignoreSpec bool) []error {
	var ret []error

	in := i.(network.VirtualRouter)
	if in.Spec.Type == network.VirtualRouterSpec_Infra.String() {
		if in.Spec.VxLanVNI != 0 {
			ret = append(ret, fmt.Errorf("VxLAN VNI cannot be specified for an Infra Virtual Router"))
		}
		if in.Spec.RouteImportExport != nil {
			ret = append(ret, fmt.Errorf("Route Import Export cannot be specified for an Infra Virtual Router"))
		}
	}
	return ret
}
func (h *networkHooks) validateNetworkIntfConfig(i interface{}, ver string, ignStatus, ignoreSpec bool) []error {
	var ret []error

	in := i.(network.NetworkInterface)
	log.Infof("Got Newtwork interface [%+v]", in)
	if in.Spec.Type != network.IFType_HOST_PF.String() && in.Spec.AttachNetwork != "" && in.Spec.AttachTenant != "" {
		ret = append(ret, fmt.Errorf("attach-tenant and attach-network can only be specified for HOST_PFs"))
	}

	if in.Spec.AttachNetwork != "" && in.Spec.AttachTenant == "" {
		ret = append(ret, fmt.Errorf("attach-network needs attach-tenant to be specified"))
	}

	if in.Spec.AttachNetwork == "" && in.Spec.AttachTenant != "" {
		ret = append(ret, fmt.Errorf("attach-tenant needs attach-network to be specified"))
	}

	return ret
}

func (h *networkHooks) networkIntfPrecommitHook(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	in, ok := i.(network.NetworkInterface)
	if !ok {
		h.logger.ErrorLog("method", "networkIntfPrecommitHook", "msg", fmt.Sprintf("API server hook to create RouteTable called for invalid object type [%#v]", i))
		return i, true, errors.New("invalid input type")
	}

	if in.Spec.AttachNetwork != "" {
		// Make sure the Network exists.
		netw := network.Network{
			ObjectMeta: api.ObjectMeta{
				Name:   in.Spec.AttachNetwork,
				Tenant: in.Spec.AttachTenant,
			},
		}
		nk := netw.MakeKey(string(apiclient.GroupNetwork))
		log.Infof("adding requirement to have [%v]", nk)
		txn.AddComparator(kvstore.Compare(kvstore.WithVersion(nk), ">", 0))
	}
	log.Infof("NetworkIF Object is [%+v]", i)
	return i, true, nil
}

// createDefaultRoutingTable is a pre-commit hook to creates default RouteTable when a tenant is created
func (h *networkHooks) createDefaultVRFRouteTable(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	r, ok := i.(network.VirtualRouter)
	if !ok {
		h.logger.ErrorLog("method", "createDefaultVRFRouteTable", "msg", fmt.Sprintf("API server hook to create RouteTable called for invalid object type [%#v]", i))
		return i, true, errors.New("invalid input type")
	}
	rt := &network.RouteTable{}
	rt.Defaults("all")
	apiSrv := apisrvpkg.MustGetAPIServer()
	rt.APIVersion = apiSrv.GetVersion()
	rt.SelfLink = rt.MakeURI("configs", rt.APIVersion, string(apiclient.GroupNetwork))
	rt.Name = r.Name + "." + "default"
	rt.Tenant = r.Tenant
	rt.Namespace = r.Namespace
	rt.GenerationID = "1"
	rt.UUID = uuid.NewV4().String()
	ts, err := types.TimestampProto(time.Now())
	if err != nil {
		return i, true, err
	}
	rt.CreationTime, rt.ModTime = api.Timestamp{Timestamp: *ts}, api.Timestamp{Timestamp: *ts}
	rtk := rt.MakeKey(string(apiclient.GroupNetwork))
	err = txn.Create(rtk, rt)
	if err != nil {
		return r, true, errors.New("adding create operation to transaction failed")
	}
	return r, true, nil
}

// deleteDefaultRoutingTable is a pre-commit hook to delete default RouteTable when a tenant is deleted
func (h *networkHooks) deleteDefaultVRFRouteTable(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	r, ok := i.(network.VirtualRouter)
	if !ok {
		h.logger.ErrorLog("method", "deleteDefaultVRFRouteTable", "msg", fmt.Sprintf("API server hook to delete default RouteTable called for invalid object type [%#v]", i))
		return i, true, errors.New("invalid input type")
	}
	rt := &network.RouteTable{}
	rt.Defaults("all")
	apiSrv := apisrvpkg.MustGetAPIServer()
	rt.APIVersion = apiSrv.GetVersion()
	rt.SelfLink = rt.MakeURI("configs", rt.APIVersion, string(apiclient.GroupNetwork))
	rt.Name = r.Name + "." + "default"
	rt.Tenant = r.Tenant
	rt.Namespace = r.Namespace
	rtk := rt.MakeKey(string(apiclient.GroupNetwork))
	err := txn.Delete(rtk)
	if err != nil {
		return r, true, errors.New("adding delete operation to transaction failed")
	}
	return r, true, nil
}

func registerNetworkHooks(svc apiserver.Service, logger log.Logger) {
	hooks := networkHooks{}
	hooks.svc = svc
	hooks.logger = logger
	logger.InfoLog("Service", "NetworkV1", "msg", "registering networkAction hook")
	svc.GetCrudService("IPAMPolicy", apiintf.CreateOper).GetRequestType().WithValidate(hooks.validateIPAMPolicyConfig)
	svc.GetCrudService("IPAMPolicy", apiintf.UpdateOper).GetRequestType().WithValidate(hooks.validateIPAMPolicyConfig)
	svc.GetCrudService("Network", apiintf.UpdateOper).GetRequestType().WithValidate(hooks.validateNetworkConfig)
	svc.GetCrudService("VirtualRouter", apiintf.UpdateOper).GetRequestType().WithValidate(hooks.validateVirtualrouterConfig)
	svc.GetCrudService("NetworkInterface", apiintf.UpdateOper).GetRequestType().WithValidate(hooks.validateNetworkIntfConfig)
	svc.GetCrudService("VirtualRouter", apiintf.CreateOper).WithPreCommitHook(hooks.createDefaultVRFRouteTable)
	svc.GetCrudService("VirtualRouter", apiintf.DeleteOper).WithPreCommitHook(hooks.deleteDefaultVRFRouteTable)
	svc.GetCrudService("NetworkInterface", apiintf.CreateOper).WithPreCommitHook(hooks.networkIntfPrecommitHook)
	svc.GetCrudService("NetworkInterface", apiintf.UpdateOper).WithPreCommitHook(hooks.networkIntfPrecommitHook)
}

func init() {
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("network.NetworkV1", registerNetworkHooks)
}
