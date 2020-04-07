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
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/api/utils"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/ctrler/orchhub/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/featureflags"
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
	autoCfg = in.Spec.BGPConfig.DSCAutoConfig
	// validate Holdtime and Keepalive timers
	if in.Spec.BGPConfig.Holdtime != 0 && in.Spec.BGPConfig.KeepaliveInterval == 0 || in.Spec.BGPConfig.Holdtime == 0 && in.Spec.BGPConfig.KeepaliveInterval != 0 {
		ret = append(ret, fmt.Errorf("inconsistent holdtime and keepalive-interval values, either both should be zero or both should be non-zero"))
	}
	if in.Spec.BGPConfig.Holdtime != 0 {
		if in.Spec.BGPConfig.Holdtime < 3 {
			ret = append(ret, fmt.Errorf("holdtime cannot be smaller than 3secs"))
		} else {
			if in.Spec.BGPConfig.KeepaliveInterval*3 > in.Spec.BGPConfig.Holdtime {
				ret = append(ret, fmt.Errorf("holdtime should be 3 times keepalive-interval or more"))
			}
		}
	}
	if in.Spec.BGPConfig.RouterId != "" && in.Spec.BGPConfig.DSCAutoConfig {
		ret = append(ret, fmt.Errorf("router id cannot be specified when dsc-auto-config is true"))
	}
	peerMap := make(map[string]bool)
	for _, n := range in.Spec.BGPConfig.Neighbors {
		if len(n.EnableAddressFamilies) != 1 {
			ret = append(ret, fmt.Errorf("there should be one address family %v", n.EnableAddressFamilies))
		}
		if n.DSCAutoConfig {
			if n.IPAddress != "" {
				ret = append(ret, fmt.Errorf("peer IP Address not allowed when dsc-auto-config is true"))
			}
			switch n.EnableAddressFamilies[0] {
			case network.BGPAddressFamily_L2vpnEvpn.String():
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
					ret = append(ret, fmt.Errorf("dsc-auto-config peer only allowed when BGP config is also dsc-auto-config"))
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
			if n.IPAddress == "" {
				ret = append(ret, fmt.Errorf("IPAddress should be specified if DSCAutoConfig is false"))
			} else {
				if _, ok := peerMap[n.IPAddress]; ok {
					ret = append(ret, fmt.Errorf("duplicate peer in spec [%v]", n.IPAddress))
				}
				peerMap[n.IPAddress] = true
			}
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

	switch oper {
	case apiintf.CreateOper:
		if !featureflags.IsOVerlayRoutingEnabled() {
			return i, true, fmt.Errorf("not licensed to enable overlay routing")
		}
	case apiintf.UpdateOper:
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
	}
	return i, true, nil
}

func (h *networkHooks) checkNetworkMutableFields(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	in, ok := i.(network.Network)
	if !ok {
		h.logger.ErrorLog("method", "checkNetworkMutableFields", "msg", fmt.Sprintf("API server network hook called for invalid object type [%#v]", i))
		return i, true, errors.New("invalid input type")
	}

	existingNw := &network.Network{}
	err := kv.Get(ctx, key, existingNw)
	if err != nil {
		log.Errorf("did not find obj [%v] on update (%s)", key, err)
		return i, true, err
	}
	if in.Spec.Type != existingNw.Spec.Type {
		return i, true, fmt.Errorf("cannot modify type of network [%v->%v]", existingNw.Spec.Type, in.Spec.Type)
	}
	if in.Spec.VirtualRouter != existingNw.Spec.VirtualRouter {
		return i, true, fmt.Errorf("cannot modify Virtual Router [%v->%v]", existingNw.Spec.Type, in.Spec.Type)
	}
	if in.Spec.VlanID != existingNw.Spec.VlanID {
		return i, true, fmt.Errorf("cannot modify VlanID of a network")
	}
	return i, true, nil
}

func (h *networkHooks) checkNetworkCreateConfig(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	in, ok := i.(network.Network)
	if !ok {
		h.logger.ErrorLog("method", "checkNetworkCreateConfig", "msg", fmt.Sprintf("API server network hook called for invalid object type [%#v]", i))
		return i, true, errors.New("invalid input type")
	}

	// make sure that vlanID is unique (network:vlan relationship is 1:1)
	nw := &network.Network{
		ObjectMeta: api.ObjectMeta{
			Tenant: globals.DefaultTenant,
		},
	}
	// CAVEAT: List approach has a small timing window where this check does not work on
	// back-to-back operations
	// create-create with same vlan will not detect the error until 1st transaction is committed
	// delete-create will report false error if 1st transaction is not committed
	// NPM and other controllers which act on network objects shuold perform their own checks
	var networks network.NetworkList
	nwKey := nw.MakeKey(string(apiclient.GroupNetwork))
	err := kv.List(ctx, nwKey, &networks)
	if err != nil {
		return i, true, fmt.Errorf("Error retrieving networks: %s", err)
	}
	for _, exNw := range networks.Items {
		if exNw.Spec.VlanID == in.Spec.VlanID {
			return i, true, fmt.Errorf("Network vlanID must be unique, already used by %s", exNw.Name)
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
func (h *networkHooks) checkVirtualRouterMutableUpdate(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	existingObj := &network.VirtualRouter{}
	err := kv.Get(ctx, key, existingObj)
	if err != nil {
		log.Errorf("did not find obj [%v] on update (%s)", key, err)
		return i, true, err
	}
	curObj, ok := i.(network.VirtualRouter)
	if !ok {
		h.logger.ErrorLog("method", "checkVirtualRouterMutableUpdate", "msg", fmt.Sprintf("API server hook to create RouteTable called for invalid object type [%#v]", i))
		return i, true, errors.New("invalid input type")
	}
	if curObj.Spec.Type != existingObj.Spec.Type {
		return i, true, fmt.Errorf("VirtualRouter Type cannot be modified [%v->%v]", existingObj.Spec.Type, curObj.Spec.Type)
	}
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

// checkNetworkInterfaceMutable is a pre-commit hook check for immutable fields
func (h *networkHooks) checkNetworkInterfaceMutable(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	r, ok := i.(network.NetworkInterface)
	if !ok {
		h.logger.ErrorLog("method", "checkNetworkInterfaceMutable", "msg", fmt.Sprintf("API server hook checkNetworkInterfaceMutable with invalid object type [%#v]", i))
		return i, true, errors.New("invalid input type")
	}
	if oper != apiintf.UpdateOper {
		return i, true, nil
	}
	pctx := apiutils.SetVar(ctx, apiutils.CtxKeyGetPersistedKV, true)
	existingObj := &network.NetworkInterface{}
	err := kv.Get(pctx, key, existingObj)
	if err != nil {
		return i, true, fmt.Errorf("Failed to get existing object: %s", err)
	}
	if existingObj.Spec.Type != r.Spec.Type {
		return i, true, fmt.Errorf("Interface Type cannot be changed")
	}
	return i, true, nil
}

func registerNetworkHooks(svc apiserver.Service, logger log.Logger) {
	hooks := networkHooks{}
	hooks.svc = svc
	hooks.logger = logger
	logger.InfoLog("Service", "NetworkV1", "msg", "registering networkAction hook")
	svc.GetCrudService("IPAMPolicy", apiintf.CreateOper).GetRequestType().WithValidate(hooks.validateIPAMPolicyConfig)
	svc.GetCrudService("IPAMPolicy", apiintf.UpdateOper).GetRequestType().WithValidate(hooks.validateIPAMPolicyConfig)
	svc.GetCrudService("Network", apiintf.CreateOper).WithPreCommitHook(hooks.checkNetworkCreateConfig)
	svc.GetCrudService("Network", apiintf.UpdateOper).GetRequestType().WithValidate(hooks.validateNetworkConfig)
	svc.GetCrudService("Network", apiintf.UpdateOper).WithPreCommitHook(hooks.networkOrchConfigPrecommit)
	svc.GetCrudService("Network", apiintf.UpdateOper).WithPreCommitHook(hooks.checkNetworkMutableFields)
	svc.GetCrudService("NetworkInterface", apiintf.UpdateOper).GetRequestType().WithValidate(hooks.validateNetworkIntfConfig)
	svc.GetCrudService("NetworkInterface", apiintf.UpdateOper).WithPreCommitHook(hooks.checkNetworkInterfaceMutable)
	svc.GetCrudService("VirtualRouter", apiintf.CreateOper).WithPreCommitHook(hooks.createDefaultVRFRouteTable)
	svc.GetCrudService("VirtualRouter", apiintf.UpdateOper).GetRequestType().WithValidate(hooks.validateVirtualrouterConfig)
	svc.GetCrudService("VirtualRouter", apiintf.UpdateOper).WithPreCommitHook(hooks.checkVirtualRouterMutableUpdate)
	svc.GetCrudService("VirtualRouter", apiintf.DeleteOper).WithPreCommitHook(hooks.deleteDefaultVRFRouteTable)
	svc.GetCrudService("RoutingConfig", apiintf.CreateOper).WithPreCommitHook(hooks.routingConfigPreCommit)
	svc.GetCrudService("RoutingConfig", apiintf.UpdateOper).WithPreCommitHook(hooks.routingConfigPreCommit)
	svc.GetCrudService("RoutingConfig", apiintf.UpdateOper).GetRequestType().WithValidate(hooks.validateRoutingConfig)
}

func init() {
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("network.NetworkV1", registerNetworkHooks)
}
