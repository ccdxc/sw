// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package impl

import (
	"context"
	"errors"
	"fmt"
	"net"
	"time"

	"github.com/gogo/protobuf/types"
	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/api/generated/workload"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apiserver"
	apisrvpkg "github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/globals"
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

func (h *networkHooks) validateRoutingConfig(i interface{}, ver string, ignStatus, ignoreSpec bool) []error {
	var ret []error
	in, ok := i.(network.RoutingConfig)
	if !ok {
		return nil
	}
	if in.Spec.BGPConfig == nil {
		return nil
	}
	var autoCfg, evpn, ipv4 bool
	rip := net.ParseIP(in.Spec.BGPConfig.RouterId)
	if rip.IsUnspecified() {
		autoCfg = true
	}

	peerMap := make(map[string]bool)
	for _, n := range in.Spec.BGPConfig.Neighbors {
		if n.IPAddress == "0.0.0.0" {
			if len(n.EnableAddressFamilies) != 1 {
				ret = append(ret, fmt.Errorf("exactly one address family allowed with auto config peering %v", n.EnableAddressFamilies))
				continue
			}
			switch n.EnableAddressFamilies[0] {
			case network.BGPAddressFamily_EVPN.String():
				if evpn {
					ret = append(ret, fmt.Errorf("only one auto-config peer per address family [l2vpn-evpn] allowed"))
				} else {
					evpn = true
				}
				if n.RemoteAS != in.Spec.BGPConfig.ASNumber {
					ret = append(ret, fmt.Errorf("EVPN auto-peering allowed only for iBGP peers"))
				}
			case network.BGPAddressFamily_IPv4Unicast.String():
				if !autoCfg {
					ret = append(ret, fmt.Errorf("auto-config peer only allowed when Router ID is also 0.0.0.0"))
				}
				if ipv4 {
					ret = append(ret, fmt.Errorf("only one auto-config peer per address family [ipv4-unicast] allowed"))
				} else {
					ipv4 = true
				}
				if n.RemoteAS == in.Spec.BGPConfig.ASNumber {
					ret = append(ret, fmt.Errorf("ipv4-unicast auto-peering allowed only for eBGP peers"))
				}
			}
		} else {
			if _, ok := peerMap[n.IPAddress]; ok {
				ret = append(ret, fmt.Errorf("duplicate peer in spec [%v]", n.IPAddress))
			}
			peerMap[n.IPAddress] = true
		}
	}
	return ret
}

// Checks that for any orch info deletion, no workloads from the orch info are using this network
func (h *networkHooks) routingConfigPreCommit(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	in, ok := i.(network.RoutingConfig)
	if !ok {
		h.logger.ErrorLog("method", "routingConfigPreCommit", "msg", fmt.Sprintf("invalid object type [%#v]", i))
		return i, true, errors.New("invalid input type")
	}

	existingObj := &network.RoutingConfig{}
	err := kv.Get(ctx, key, existingObj)
	if err != nil {
		return i, true, fmt.Errorf("Failed to get existing object: %s", err)
	}
	if existingObj.Spec.BGPConfig != nil && in.Spec.BGPConfig != nil {
		if existingObj.Spec.BGPConfig.ASNumber != in.Spec.BGPConfig.ASNumber {
			return i, true, fmt.Errorf("Change in local ASN not allowed, delete and recreate")
		}
	}
	return i, true, nil
}

// Checks that for any orch info deletion, no workloads from the orch info are using this network
func (h *networkHooks) networkOrchConfigPrecommit(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	in, ok := i.(network.Network)
	if !ok {
		h.logger.ErrorLog("method", "networkOrchConfigPrecommit", "msg", fmt.Sprintf("API server network hook called for invalid object type [%#v]", i))
		return i, true, errors.New("invalid input type")
	}

	if in.Tenant != globals.DefaultTenant {
		// Since orch config can only live in default tenant scope,
		// this hook check is not needed if tenant is different
		return i, true, nil
	}

	existingObj := &network.Network{}
	err := kv.Get(ctx, key, existingObj)
	if err != nil {
		return i, true, fmt.Errorf("Failed to get existing object: %s", err)
	}

	// Build new network's orch info map
	orchMap := map[string](map[string]bool){}

	for _, config := range in.Spec.Orchestrators {
		if _, ok := orchMap[config.Name]; !ok {
			orchMap[config.Name] = map[string]bool{}
		}
		entry := orchMap[config.Name]
		entry[config.Namespace] = true
	}

	// determine which orch infos are being deleted
	delMap := map[string](map[string]bool){}
	for _, config := range existingObj.Spec.Orchestrators {
		entry, ok := orchMap[config.Name]
		deleted := false
		if !ok {
			deleted = true
		}
		if _, ok := entry[config.Namespace]; !ok {
			deleted = true
		}
		if deleted {
			if _, ok := delMap[config.Name]; !ok {
				delMap[config.Name] = map[string]bool{}
			}
			delMap[config.Name][config.Namespace] = true
		}
	}

	if len(delMap) == 0 {
		// no deletions to orchestrator info
		return i, true, nil
	}

	// Fetch workloads
	var workloads workload.WorkloadList
	wl := workload.Workload{
		ObjectMeta: api.ObjectMeta{
			Tenant: in.Tenant,
		},
	}
	listKey := wl.MakeKey(string(apiclient.GroupWorkload))
	err = kv.List(ctx, listKey, &workloads)
	if err != nil {
		return i, true, fmt.Errorf("Error retrieving workloads: %s", err)
	}

	// check workloads
	for _, wl := range workloads.Items {
		orch, ok := utils.GetOrchNameFromObj(wl.Labels)
		if !ok {
			continue
		}
		delNs, ok := delMap[orch]
		if !ok {
			continue
		}
		namespace, ok := utils.GetOrchNamespaceFromObj(wl.Labels)
		if !ok {
			continue
		}
		if _, ok := delNs[namespace]; !ok {
			continue
		}
		// Check the workloads interfaces to see if it's using this network
		for _, inf := range wl.Spec.Interfaces {
			if inf.Network == in.Name {
				// workload relies on this orch config
				return i, true, fmt.Errorf("Cannot remove orchestrator info %s, namespace %s, as workloads from this orchestrator are using this network", orch, namespace)
			}
		}
	}

	return i, true, nil
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
	svc.GetCrudService("Network", apiintf.UpdateOper).WithPreCommitHook(hooks.networkOrchConfigPrecommit)
	svc.GetCrudService("RoutingConfig", apiintf.UpdateOper).WithPreCommitHook(hooks.routingConfigPreCommit)
	svc.GetCrudService("RoutingConfig", apiintf.UpdateOper).GetRequestType().WithValidate(hooks.validateRoutingConfig)
}

func init() {
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("network.NetworkV1", registerNetworkHooks)
}
