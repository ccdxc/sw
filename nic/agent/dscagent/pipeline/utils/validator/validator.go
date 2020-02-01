package validator

import (
	"hash/fnv"
	"strings"
	"sync"

	"github.com/pkg/errors"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/dscagent/pipeline/utils"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// ValidateApp validates that an app has only a single ALG set
func ValidateApp(app netproto.App) (err error) {
	err = validateSingletonALG(app.Spec.ALG)
	if err != nil {
		return
	}

	return
}

// ValidateEndpoint performs static field validations on IP and MAC and named ref validations on network and vrf
func ValidateEndpoint(i types.InfraAPI, endpoint netproto.Endpoint) (network netproto.Network, vrf netproto.Vrf, err error) {
	// Static Field  Validations
	err = utils.ValidateIPAddresses(endpoint.Spec.IPv4Addresses...)
	if err != nil {
		return
	}

	err = utils.ValidateMacAddresses(endpoint.Spec.MacAddress)
	if err != nil {
		return
	}

	// Named reference validations
	vrf, err = ValidateVrf(i, endpoint.Tenant, endpoint.Namespace, endpoint.Spec.VrfName)
	if err != nil {
		return
	}

	nt := netproto.Network{
		TypeMeta: api.TypeMeta{
			Kind: "Network",
		},
		ObjectMeta: api.ObjectMeta{
			Tenant:    endpoint.Tenant,
			Namespace: endpoint.Namespace,
			Name:      endpoint.Spec.NetworkName,
		},
	}
	log.Infof("Looking up Network: %v", nt)
	dat, err := i.Read(nt.Kind, nt.GetKey())
	if err != nil {
		log.Error(errors.Wrapf(types.ErrBadRequest, "Network: %s | Err: %v", nt.GetKey(), types.ErrObjNotFound))
		return network, vrf, errors.Wrapf(types.ErrBadRequest, "Network: %s | Err: %v", nt.GetKey(), types.ErrObjNotFound)
	}
	err = nt.Unmarshal(dat)
	if err != nil {
		log.Error(errors.Wrapf(types.ErrUnmarshal, "Network: %s | Err: %v", nt.GetKey(), err))
		return network, vrf, errors.Wrapf(types.ErrUnmarshal, "Network: %s | Err: %v", nt.GetKey(), err)
	}

	network = nt
	log.Infof("Found Network: %v", network)
	return
}

// ValidateInterface performs static field validations on interface type
func ValidateInterface(intf netproto.Interface) (err error) {
	// Static Field  Validations
	switch strings.ToLower(intf.Spec.Type) {
	case "uplink_eth":
		return
	case "uplink_mgmt":
		return
	default:
		err = errors.Wrapf(types.ErrInvalidInterfaceType, "Type: %s | Err: %v", intf.Spec.Type, types.ErrBadRequest)
		return
	}
}

// ValidateNetwork performs named reference validations on vrf and unique VLAN IDs
func ValidateNetwork(i types.InfraAPI, network netproto.Network) (uplinkIDs []uint64, vrf netproto.Vrf, err error) {
	// Named reference validations
	var (
		intfs []netproto.Interface
		dat   [][]byte
	)
	vrf, err = ValidateVrf(i, network.Tenant, network.Namespace, network.Spec.VrfName)
	if err != nil {
		return
	}

	intf := &netproto.Interface{
		TypeMeta: api.TypeMeta{Kind: "Interface"},
	}

	dat, err = i.List(intf.Kind)
	if err != nil {
		log.Error(errors.Wrapf(types.ErrBadRequest, "Interface: %s | Err: %v", intf.GetKey(), types.ErrObjNotFound))
		return nil, vrf, errors.Wrapf(types.ErrBadRequest, "Interface: %s | Err: %v", intf.GetKey(), types.ErrObjNotFound)
	}

	for _, o := range dat {
		var intf netproto.Interface
		err := intf.Unmarshal(o)
		if err != nil {
			log.Error(errors.Wrapf(types.ErrUnmarshal, "Interface: %s | Err: %v", intf.GetKey(), err))
			continue
		}
		intfs = append(intfs, intf)
	}

	for _, intf := range intfs {
		if intf.Spec.Type == "UPLINK_ETH" {
			uplinkIDs = append(uplinkIDs, intf.Status.InterfaceID)
		}
	}
	return
}

// ValidateNetworkSecurityPolicy validates network security policy and generates rule id to app mapping
func ValidateNetworkSecurityPolicy(i types.InfraAPI, nsp netproto.NetworkSecurityPolicy) (vrf netproto.Vrf, ruleIDLut *sync.Map, err error) {
	// Static Validations

	var ruleIDAppMapping sync.Map

	if nsp.Spec.AttachTenant == false && len(nsp.Spec.AttachGroup) == 0 {
		err := errors.Wrapf(types.ErrMissingPolicyAttachmentPoint, "NetworkSecurityPolicy: %s | Err: %v", nsp.GetKey(), types.ErrBadRequest)
		return vrf, nil, err
	}

	if nsp.Spec.AttachTenant == true && len(nsp.Spec.AttachGroup) != 0 {
		err := errors.Wrapf(types.ErrPolicyDualAttachment, "NetworkSecurityPolicy: %s | Err: %v", nsp.GetKey(), types.ErrBadRequest)
		return vrf, nil, err
	}

	// Named reference validations
	vrf, err = ValidateVrf(i, nsp.Tenant, nsp.Namespace, nsp.Spec.VrfName)
	if err != nil {
		return
	}

	for idx, r := range nsp.Spec.Rules {
		var dat []byte
		ruleHash := nsp.Spec.Rules[idx].ID
		// Calculate the hash only if npm has not set it. Else use whatever is already set
		if ruleHash == 0 {
			ruleHash = generateRuleHash(&r, nsp.GetKey())
			nsp.Spec.Rules[idx].ID = ruleHash
		}

		if len(r.AppName) > 0 {
			app := netproto.App{
				TypeMeta: api.TypeMeta{
					Kind: "App",
				},
				ObjectMeta: api.ObjectMeta{
					Tenant:    nsp.Tenant,
					Namespace: nsp.Namespace,
					Name:      r.AppName,
				},
			}

			dat, err = i.Read(app.Kind, app.GetKey())
			if err != nil {
				log.Error(errors.Wrapf(types.ErrBadRequest, "App: %s | Err: %v", app.GetKey(), types.ErrObjNotFound))
				return vrf, nil, errors.Wrapf(types.ErrBadRequest, "App: %s | Err: %v", app.GetKey(), types.ErrObjNotFound)
			}
			err = app.Unmarshal(dat)
			if err != nil {
				log.Error(errors.Wrapf(types.ErrUnmarshal, "App: %s | Err: %v", app.GetKey(), err))
				return vrf, nil, errors.Wrapf(types.ErrUnmarshal, "App: %s | Err: %v", app.GetKey(), err)
			}

			if len(r.Dst.ProtoPorts) != 0 || len(r.Src.ProtoPorts) != 0 {
				log.Error(errors.Wrapf(types.ErrBadRequest, "App: %s | Err: %v", app.GetKey(), types.ErrDuplicateRuleinAppAndInlineRules))
				return vrf, nil, errors.Wrapf(types.ErrBadRequest, "App: %s | Err: %v", app.GetKey(), types.ErrDuplicateRuleinAppAndInlineRules)

			}

			ruleIDAppMapping.Store(idx, &app)
		}
	}

	return vrf, &ruleIDAppMapping, nil
}

// ValidateTunnel performs static field validations on srcIP, DstIP and named reference validation on vrf
func ValidateTunnel(i types.InfraAPI, tunnel netproto.Tunnel) (vrf netproto.Vrf, err error) {
	// Static Field  Validations
	err = utils.ValidateIPAddresses(tunnel.Spec.Src)
	if err != nil {
		return
	}

	err = utils.ValidateIPAddresses(tunnel.Spec.Dst)
	if err != nil {
		return
	}

	// Named reference validations
	vrf, err = ValidateVrf(i, tunnel.Tenant, tunnel.Namespace, tunnel.Spec.VrfName)
	if err != nil {
		return
	}
	return
}

// ValidateMirrorSession performs static field validations on srcIP, DstIP and named reference validation on vrf
func ValidateMirrorSession(i types.InfraAPI, mirror netproto.MirrorSession, oper types.Operation) (vrf netproto.Vrf, err error) {
	var collectorIPAddresses []string
	for _, c := range mirror.Spec.Collectors {
		collectorIPAddresses = append(collectorIPAddresses, c.ExportCfg.Destination)
	}

	dat, _ := i.List(mirror.Kind)
	if len(dat) == types.MaxMirrorSessions && oper == types.Create {
		log.Error(errors.Wrapf(types.ErrBadRequest, "MirrorSession: %s | Err: %v", mirror.GetKey(), types.ErrMaxMirrorSessionsConfigured))
		return vrf, errors.Wrapf(types.ErrBadRequest, "MirrorSession: %s | Err: %v", mirror.GetKey(), types.ErrMaxMirrorSessionsConfigured)
	}

	//cfg := i.GetConfig()

	//arpCache, err = utils.ResolveIPAddress(cfg.MgmtIP, cfg.MgmtIntf, collectorIPAddresses...)
	//if err != nil {
	//	return
	//}

	// Named reference validations
	vrf, err = ValidateVrf(i, mirror.Tenant, mirror.Namespace, mirror.Spec.VrfName)
	if err != nil {
		return
	}
	return
}

// ValidateFlowExportPolicy performs static field validations on srcIP, DstIP and named reference validation on vrf
func ValidateFlowExportPolicy(i types.InfraAPI, netflow netproto.FlowExportPolicy) (vrf netproto.Vrf, err error) {
	var collectorIPAddresses []string
	for _, c := range netflow.Spec.Exports {
		collectorIPAddresses = append(collectorIPAddresses, c.Destination)
	}

	//cfg := i.GetConfig()

	//arpCache, err = utils.ResolveIPAddress(cfg.MgmtIP, cfg.MgmtIntf, collectorIPAddresses...)
	//if err != nil {
	//	return
	//}

	// Named reference validations
	vrf, err = ValidateVrf(i, netflow.Tenant, netflow.Namespace, netflow.Spec.VrfName)
	if err != nil {
		return
	}
	return
}

// ValidateVrf validates VRF object. On empty VrfNames, it uses default vrf with id == 65
func ValidateVrf(i types.InfraAPI, tenant, namespace, name string) (vrf netproto.Vrf, err error) {
	// Pick default vrf is unspecified or specified default
	if len(name) == 0 || name == types.DefaultVrf {
		tenant = types.DefaultVrf
		namespace = types.DefaultNamespace
		name = types.DefaultVrf
	}

	v := netproto.Vrf{
		TypeMeta: api.TypeMeta{
			Kind: "Vrf",
		},
		ObjectMeta: api.ObjectMeta{
			Tenant:    tenant,
			Namespace: namespace,
			Name:      name,
		},
	}

	dat, err := i.Read(v.Kind, v.GetKey())
	if err != nil {
		log.Error(errors.Wrapf(types.ErrBadRequest, "Vrf: %s | Err: %v", vrf.GetKey(), types.ErrObjNotFound))
		return vrf, errors.Wrapf(types.ErrBadRequest, "Vrf: %s | Err: %v", vrf.GetKey(), types.ErrObjNotFound)
	}
	err = vrf.Unmarshal(dat)
	if err != nil {
		log.Error(errors.Wrapf(types.ErrUnmarshal, "Vrf: %s | Err: %v", vrf.GetKey(), err))
		return vrf, errors.Wrapf(types.ErrUnmarshal, "Vrf: %s | Err: %v", vrf.GetKey(), err)
	}

	return
}

// ValidateSecurityProfile validates SecurityProfile object and returns the attachment vrf
func ValidateSecurityProfile(i types.InfraAPI, profile netproto.SecurityProfile) (vrf netproto.Vrf, err error) {
	vrf, err = ValidateVrf(i, profile.Tenant, profile.Namespace, types.DefaultVrf)
	return
}

// ValidateRoutingConfig validates RoutingConfig object
func ValidateRoutingConfig(i types.InfraAPI, rtCfg netproto.RoutingConfig) error {
	return nil
}

// ValidateProfile validates Profile object
func ValidateProfile(profile netproto.Profile) error {
	if _, ok := netproto.ProfileSpec_Fwd_Mode_value[strings.ToUpper(profile.Spec.FwdMode)]; !ok {
		return errors.Wrapf(types.ErrInvalidProfileFwdMode, "Type: %s | Err: %v", strings.ToUpper(profile.Spec.FwdMode), types.ErrBadRequest)
	}
	if _, ok := netproto.ProfileSpec_Policy_Mode_value[strings.ToUpper(profile.Spec.PolicyMode)]; !ok {
		return errors.Wrapf(types.ErrInvalidProfilePolicyMode, "Type: %s | Err: %v", strings.ToUpper(profile.Spec.PolicyMode), types.ErrBadRequest)
	}
	return nil
}

func validateSingletonALG(alg *netproto.ALG) (err error) {
	var algMapper int
	if alg == nil {
		return
	}
	switch {
	case alg.DNS != nil:
		algMapper = setBit(algMapper, types.AlgDNS)

	case alg.FTP != nil:
		algMapper = setBit(algMapper, types.AlgFTP)
	case alg.ICMP != nil:
		algMapper = setBit(algMapper, types.AlgICMP)
	case alg.MSRPC != nil:
		algMapper = setBit(algMapper, types.AlgMSRPC)
	case alg.RTSP != nil:
		algMapper = setBit(algMapper, types.AlgRTSP)
	case alg.SIP != nil:
		algMapper = setBit(algMapper, types.AlgSIP)
	case alg.SUNRPC != nil:
		algMapper = setBit(algMapper, types.AlgSUNRPC)
	case alg.TFTP != nil:
		algMapper = setBit(algMapper, types.AlgTFTP)
	}

	if (algMapper & -algMapper) != algMapper {
		return types.ErrMultipleALGInApp
	}

	return
}

// setBit sets a particular bit position to 1. This is used for ALG Mapper
func setBit(n int, pos uint) int {
	n |= 1 << pos
	return n
}

// generateRuleHash generates a fnv 64 hash based on the policy rule and the NetworkSecurityPolicy Key.
// Ensures cluster wide uniqueness
//generateRuleHash generates rule hash for policy rule
func generateRuleHash(r *netproto.PolicyRule, key string) uint64 {
	h := fnv.New64()
	rule, _ := r.Marshal()
	rule = append(rule, []byte(key)...)
	h.Write(rule)
	return h.Sum64()
}
