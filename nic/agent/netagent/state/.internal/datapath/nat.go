package datapath

import (
	"context"
	"fmt"
	"net"
	"strings"

	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/nic/agent/netagent/state/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
)

// ------------------------- Nat Pool CRUDs ------------------------- //

// CreateNatPool creates a NAT Pool in the datapath
func (hd *Datapath) CreateNatPool(np *netproto.NatPool, vrf *netproto.Vrf) error {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrf.Status.VrfID,
		},
	}

	ipRange := strings.Split(np.Spec.IPRange, "-")
	if len(ipRange) != 2 {
		return fmt.Errorf("could not parse IP Range from the NAT Pool IPRange. {%v}", np.Spec.IPRange)
	}

	startIP := net.ParseIP(strings.TrimSpace(ipRange[0]))
	if len(startIP) == 0 {
		return fmt.Errorf("could not parse IP from {%v}", startIP)
	}
	endIP := net.ParseIP(strings.TrimSpace(ipRange[1]))
	if len(endIP) == 0 {
		return fmt.Errorf("could not parse IP from {%v}", endIP)
	}

	lowIP := halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv4Touint32(startIP),
		},
	}

	highIP := halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv4Touint32(endIP),
		},
	}

	addrRange := &halproto.Address_Range{
		Range: &halproto.AddressRange{
			Range: &halproto.AddressRange_Ipv4Range{
				Ipv4Range: &halproto.IPRange{
					LowIpaddr:  &lowIP,
					HighIpaddr: &highIP,
				},
			},
		},
	}

	natPoolReqMsg := &halproto.NatPoolRequestMsg{
		Request: []*halproto.NatPoolSpec{
			{
				KeyOrHandle: &halproto.NatPoolKeyHandle{
					KeyOrHandle: &halproto.NatPoolKeyHandle_PoolKey{
						PoolKey: &halproto.NatPoolKey{
							VrfKh:  vrfKey,
							PoolId: np.Status.NatPoolID,
						},
					},
				},
				Address: []*halproto.Address{
					{
						Address: addrRange,
					},
				},
			},
		},
	}

	if hd.Kind == "hal" {
		resp, err := hd.Hal.Natclient.NatPoolCreate(context.Background(), natPoolReqMsg)
		if err != nil {
			log.Errorf("Error creating nat pool. Err: %v", err)
			return err
		}
		if !(resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_OK || resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_EXISTS_ALREADY) {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.Natclient.NatPoolCreate(context.Background(), natPoolReqMsg)
		if err != nil {
			log.Errorf("Error creating nat pool. Err: %v", err)
			return err
		}
	}
	return nil
}

// UpdateNatPool updates a NAT Pool in the datapath
func (hd *Datapath) UpdateNatPool(np *netproto.NatPool, vrf *netproto.Vrf) error {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrf.Status.VrfID,
		},
	}

	ipRange := strings.Split(np.Spec.IPRange, "-")
	if len(ipRange) != 2 {
		return fmt.Errorf("could not parse IP Range from the NAT Pool IPRange. {%v}", np.Spec.IPRange)
	}

	startIP := net.ParseIP(strings.TrimSpace(ipRange[0]))
	if len(startIP) == 0 {
		return fmt.Errorf("could not parse IP from {%v}", startIP)
	}
	endIP := net.ParseIP(strings.TrimSpace(ipRange[1]))
	if len(endIP) == 0 {
		return fmt.Errorf("could not parse IP from {%v}", endIP)
	}

	lowIP := halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv4Touint32(startIP),
		},
	}

	highIP := halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv4Touint32(endIP),
		},
	}

	addrRange := &halproto.Address_Range{
		Range: &halproto.AddressRange{
			Range: &halproto.AddressRange_Ipv4Range{
				Ipv4Range: &halproto.IPRange{
					LowIpaddr:  &lowIP,
					HighIpaddr: &highIP,
				},
			},
		},
	}

	natPoolUpdateReqMsg := &halproto.NatPoolRequestMsg{
		Request: []*halproto.NatPoolSpec{
			{
				KeyOrHandle: &halproto.NatPoolKeyHandle{
					KeyOrHandle: &halproto.NatPoolKeyHandle_PoolKey{
						PoolKey: &halproto.NatPoolKey{
							VrfKh:  vrfKey,
							PoolId: np.Status.NatPoolID,
						},
					},
				},
				Address: []*halproto.Address{
					{
						Address: addrRange,
					},
				},
			},
		},
	}

	if hd.Kind == "hal" {
		resp, err := hd.Hal.Natclient.NatPoolUpdate(context.Background(), natPoolUpdateReqMsg)
		if err != nil {
			log.Errorf("Error updating nat pool. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.Natclient.NatPoolUpdate(context.Background(), natPoolUpdateReqMsg)
		if err != nil {
			log.Errorf("Error updating nat pool. Err: %v", err)
			return err
		}
	}
	return nil
}

// DeleteNatPool deletes a NAT Pool in the datapath
func (hd *Datapath) DeleteNatPool(np *netproto.NatPool, vrf *netproto.Vrf) error {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrf.Status.VrfID,
		},
	}

	// Build Nat Pool Key
	natPoolKey := &halproto.NatPoolKeyHandle{
		KeyOrHandle: &halproto.NatPoolKeyHandle_PoolKey{
			PoolKey: &halproto.NatPoolKey{
				VrfKh:  vrfKey,
				PoolId: np.Status.NatPoolID,
			},
		},
	}

	npDelReq := &halproto.NatPoolDeleteRequestMsg{
		Request: []*halproto.NatPoolDeleteRequest{
			{
				KeyOrHandle: natPoolKey,
			},
		},
	}

	// delete hal objects
	if hd.Kind == "hal" {
		// delete route
		resp, err := hd.Hal.Natclient.NatPoolDelete(context.Background(), npDelReq)
		if err != nil {
			log.Errorf("Error deleting nat pool. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}

	} else {
		_, err := hd.Hal.Natclient.NatPoolDelete(context.Background(), npDelReq)
		if err != nil {
			log.Errorf("Error deleting nat pool. Err: %v", err)
			return err
		}
	}
	return nil
}

// ------------------------- Nat Policy CRUDs ------------------------- //

// CreateNatPolicy creates a NAT Policy in the datapath
func (hd *Datapath) CreateNatPolicy(np *netproto.NatPolicy, natPoolLUT map[string]*types.NatPoolRef, vrf *netproto.Vrf) error {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrf.Status.VrfID,
		},
	}

	var natRules []*halproto.NatRuleSpec

	for _, r := range np.Spec.Rules {
		ruleMatches, err := hd.buildHALRuleMatches(r.Src, r.Dst, nil, nil)
		if err != nil {
			log.Errorf("Could not convert match criteria Err: %v", err)
			return err
		}
		npRef, ok := natPoolLUT[r.NatPool]
		if !ok {
			return fmt.Errorf("nat pool not found. {%v}", r.NatPool)
		}

		natAction, err := hd.convertNatRuleAction(r.Action, npRef.NamespaceID, npRef.PoolID)
		if err != nil {
			log.Errorf("Could not convert NAT Action. Action: %v. Err: %v", r.Action, err)
		}

		for _, ruleMatch := range ruleMatches {
			rule := &halproto.NatRuleSpec{
				RuleId: r.ID,
				Match:  ruleMatch,
				Action: natAction,
			}
			natRules = append(natRules, rule)
		}

	}

	natPolicyReqMsg := &halproto.NatPolicyRequestMsg{
		Request: []*halproto.NatPolicySpec{
			{
				KeyOrHandle: &halproto.NatPolicyKeyHandle{
					KeyOrHandle: &halproto.NatPolicyKeyHandle_PolicyKey{
						PolicyKey: &halproto.NATPolicyKey{
							NatPolicyId:    np.Status.NatPolicyID,
							VrfKeyOrHandle: vrfKey,
						},
					},
				},
				Rules: natRules,
			},
		},
	}

	if hd.Kind == "hal" {
		resp, err := hd.Hal.Natclient.NatPolicyCreate(context.Background(), natPolicyReqMsg)
		if err != nil {
			log.Errorf("Error creating nat pool. Err: %v", err)
			return err
		}
		if !(resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_OK || resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_EXISTS_ALREADY) {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.Natclient.NatPolicyCreate(context.Background(), natPolicyReqMsg)
		if err != nil {
			log.Errorf("Error creating nat pool. Err: %v", err)
			return err
		}
	}
	return nil
}

// UpdateNatPolicy updates a NAT Policy in the datapath
func (hd *Datapath) UpdateNatPolicy(np *netproto.NatPolicy, natPoolLUT map[string]*types.NatPoolRef, vrf *netproto.Vrf) error {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrf.Status.VrfID,
		},
	}

	var natRules []*halproto.NatRuleSpec

	for _, r := range np.Spec.Rules {
		ruleMatches, err := hd.buildHALRuleMatches(r.Src, r.Dst, nil, nil)
		if err != nil {
			log.Errorf("Could not convert match criteria Err: %v", err)
			return err
		}
		npRef, ok := natPoolLUT[r.NatPool]
		if !ok {
			return fmt.Errorf("nat pool not found. {%v}", r.NatPool)
		}

		natAction, err := hd.convertNatRuleAction(r.Action, npRef.NamespaceID, npRef.PoolID)
		if err != nil {
			log.Errorf("Could not convert NAT Action. Action: %v. Err: %v", r.Action, err)
		}
		for _, ruleMatch := range ruleMatches {
			rule := &halproto.NatRuleSpec{
				RuleId: r.ID,
				Match:  ruleMatch,
				Action: natAction,
			}
			natRules = append(natRules, rule)
		}
	}

	natPolicyUpdateReqMsg := &halproto.NatPolicyRequestMsg{
		Request: []*halproto.NatPolicySpec{
			{
				KeyOrHandle: &halproto.NatPolicyKeyHandle{
					KeyOrHandle: &halproto.NatPolicyKeyHandle_PolicyKey{
						PolicyKey: &halproto.NATPolicyKey{
							NatPolicyId:    np.Status.NatPolicyID,
							VrfKeyOrHandle: vrfKey,
						},
					},
				},
				Rules: natRules,
			},
		},
	}

	if hd.Kind == "hal" {
		resp, err := hd.Hal.Natclient.NatPolicyUpdate(context.Background(), natPolicyUpdateReqMsg)
		if err != nil {
			log.Errorf("Error updating nat pool. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
	} else {
		_, err := hd.Hal.Natclient.NatPolicyUpdate(context.Background(), natPolicyUpdateReqMsg)
		if err != nil {
			log.Errorf("Error updating nat pool. Err: %v", err)
			return err
		}
	}
	return nil

}

// DeleteNatPolicy deletes a NAT Policy in the datapath
func (hd *Datapath) DeleteNatPolicy(np *netproto.NatPolicy, vrf *netproto.Vrf) error {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrf.Status.VrfID,
		},
	}

	// Build Nat Policy Key
	natPolicyKey := &halproto.NatPolicyKeyHandle{
		KeyOrHandle: &halproto.NatPolicyKeyHandle_PolicyKey{
			PolicyKey: &halproto.NATPolicyKey{
				VrfKeyOrHandle: vrfKey,
				NatPolicyId:    np.Status.NatPolicyID,
			},
		},
	}

	npDelReq := &halproto.NatPolicyDeleteRequestMsg{
		Request: []*halproto.NatPolicyDeleteRequest{
			{
				KeyOrHandle: natPolicyKey,
			},
		},
	}

	// delete hal objects
	if hd.Kind == "hal" {
		// delete route
		resp, err := hd.Hal.Natclient.NatPolicyDelete(context.Background(), npDelReq)
		if err != nil {
			log.Errorf("Error deleting nat policy. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}

	} else {
		_, err := hd.Hal.Natclient.NatPolicyDelete(context.Background(), npDelReq)
		if err != nil {
			log.Errorf("Error deleting nat policy. Err: %v", err)
			return err
		}
	}
	return nil
}

// ------------------------- Nat Binding CRUDs ------------------------- //

// CreateNatBinding creates a NAT Binding in the datapath
func (hd *Datapath) CreateNatBinding(nb *netproto.NatBinding, np *netproto.NatPool, natPoolVrfID uint64, vrf *netproto.Vrf) (*netproto.NatBinding, error) {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrf.Status.VrfID,
		},
	}

	natBindingIP := net.ParseIP(nb.Spec.IPAddress)
	if len(natBindingIP) == 0 {
		log.Errorf("could not parse IP from {%v}", natBindingIP)
		return nil, ErrIPParse
	}

	ipAddr := &halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv4Touint32(natBindingIP),
		},
	}

	natPoolVrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: natPoolVrfID,
		},
	}

	natPoolKey := &halproto.NatPoolKeyHandle{
		KeyOrHandle: &halproto.NatPoolKeyHandle_PoolKey{
			PoolKey: &halproto.NatPoolKey{
				VrfKh:  natPoolVrfKey,
				PoolId: np.Status.NatPoolID,
			},
		},
	}

	natBindingReqMsg := &halproto.NatMappingRequestMsg{
		Request: []*halproto.NatMappingSpec{
			{
				KeyOrHandle: &halproto.NatMappingKeyHandle{
					KeyOrHandle: &halproto.NatMappingKeyHandle_Svc{
						Svc: &halproto.Svc{
							VrfKh:  vrfKey,
							IpAddr: ipAddr,
						},
					},
				},
				NatPool: natPoolKey,
				Bidir:   true, // Set Bidirectional to true inorder to create forward and reverse mappings.
			},
		},
	}

	if hd.Kind == "hal" {
		resp, err := hd.Hal.Natclient.NatMappingCreate(context.Background(), natBindingReqMsg)
		if err != nil {
			log.Errorf("Error creating nat pool. Err: %v", err)
			return nb, err
		}
		if !(resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_OK || resp.Response[0].ApiStatus == halproto.ApiStatus_API_STATUS_EXISTS_ALREADY) {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return nb, fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}
		ipv4Int := resp.Response[0].Status.MappedIp.GetV4Addr()
		ip := uint32ToIPv4(ipv4Int)
		nb.Status.NatIP = ip.String()
		return nb, nil
	}
	_, err := hd.Hal.Natclient.NatMappingCreate(context.Background(), natBindingReqMsg)
	if err != nil {
		log.Errorf("Error creating nat pool. Err: %v", err)
		return nb, err
	}
	return nb, nil

}

// UpdateNatBinding updates a NAT Binding in the datapath. Not implemented in HAL
func (hd *Datapath) UpdateNatBinding(nb *netproto.NatBinding, np *netproto.NatPool, natPoolVrfID uint64, vrf *netproto.Vrf) error {

	return nil
}

// DeleteNatBinding deletes a NAT Binding in the datapath
func (hd *Datapath) DeleteNatBinding(nb *netproto.NatBinding, vrf *netproto.Vrf) error {
	// This will ensure that only one datapath config will be active at a time. This is a temporary restriction
	// to ensure that HAL will use a single config thread , this will be removed prior to FCS to allow parallel configs to go through.
	// TODO Remove Global Locking
	hd.Lock()
	defer hd.Unlock()
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrf.Status.VrfID,
		},
	}

	// Build Nat Binding Key
	natBindingIP := net.ParseIP(nb.Spec.IPAddress)
	if len(natBindingIP) == 0 {
		log.Errorf("could not parse IP from {%v}", natBindingIP)
		return ErrIPParse
	}

	ipAddr := &halproto.IPAddress{
		IpAf: halproto.IPAddressFamily_IP_AF_INET,
		V4OrV6: &halproto.IPAddress_V4Addr{
			V4Addr: ipv4Touint32(natBindingIP),
		},
	}

	natBindingKey := &halproto.NatMappingKeyHandle{
		KeyOrHandle: &halproto.NatMappingKeyHandle_Svc{
			Svc: &halproto.Svc{
				VrfKh:  vrfKey,
				IpAddr: ipAddr,
			},
		},
	}

	nbDelReq := &halproto.NatMappingDeleteRequestMsg{
		Request: []*halproto.NatMappingDeleteRequest{
			{
				KeyOrHandle: natBindingKey,
			},
		},
	}

	// delete hal objects
	if hd.Kind == "hal" {
		// delete route
		resp, err := hd.Hal.Natclient.NatMappingDelete(context.Background(), nbDelReq)
		if err != nil {
			log.Errorf("Error deleting nat binding. Err: %v", err)
			return err
		}
		if resp.Response[0].ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
			return fmt.Errorf("HAL returned non OK status. %v", resp.Response[0].ApiStatus.String())
		}

	} else {
		_, err := hd.Hal.Natclient.NatMappingDelete(context.Background(), nbDelReq)
		if err != nil {
			log.Errorf("Error deleting nat binding. Err: %v", err)
			return err
		}
	}
	return nil
}

func (hd *Datapath) convertNatRuleAction(action string, vrfID, poolID uint64) (*halproto.NatRuleAction, error) {
	// Build the NaT Pool Vrf Key
	vrfKey := &halproto.VrfKeyHandle{
		KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
			VrfId: vrfID,
		},
	}

	// Build the Nat Pool Key
	poolKey := &halproto.NatPoolKeyHandle{
		KeyOrHandle: &halproto.NatPoolKeyHandle_PoolKey{
			PoolKey: &halproto.NatPoolKey{
				VrfKh:  vrfKey,
				PoolId: poolID,
			},
		},
	}

	switch action {
	case "SNAT":
		natAction := &halproto.NatRuleAction{
			SrcNatAction: halproto.NatAction_NAT_TYPE_DYNAMIC_ADDRESS,
			SrcNatPool:   poolKey,
		}
		return natAction, nil
	case "DNAT":
		natAction := &halproto.NatRuleAction{
			DstNatAction: halproto.NatAction_NAT_TYPE_DYNAMIC_ADDRESS,
			DstNatPool:   poolKey,
		}
		return natAction, nil
	default:
		log.Errorf("Invalid Nat action. %v", action)
		return nil, ErrInvalidNatActionType
	}
}
