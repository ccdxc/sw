// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package objects

import (
	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/iota/test/venice/iotakit/cfg/objClient"
	"github.com/pensando/sw/iota/test/venice/iotakit/testbed"
	//"github.com/pensando/sw/venice/utils/log"
)
type FirewallProfile struct {
	VeniceFirewallProfile *security.FirewallProfile
}

type FirewallProfileCollection struct {
	CollectionCommon
	err			error
	Profiles	[]*FirewallProfile
}

func NewFirewallProfile(name string, client objClient.ObjClient, testbed *testbed.TestBed) *FirewallProfileCollection {
	return &FirewallProfileCollection{
		Profiles: []*FirewallProfile{
			{
				VeniceFirewallProfile: &security.FirewallProfile{
					TypeMeta: api.TypeMeta{Kind: "FirewallProfile"},
					ObjectMeta: api.ObjectMeta{
						Name:      name,
						Namespace: "default",
						Tenant:    "default",
					},
					Spec: security.FirewallProfileSpec{
						// FIXME: set short timeout value till HAL fixes the drop->allow transition issue
						//set 10s defaults for other fields
						SessionIdleTimeout:        "10s",
						TCPConnectionSetupTimeout: "10s",
						TCPCloseTimeout:           "10s",
						TCPHalfClosedTimeout:      "10s",
						TCPDropTimeout:            "10s",
						UDPDropTimeout:            "10s",
						DropTimeout:               "10s",
						TcpTimeout:                "10s",
						UdpTimeout:                "10s",
						ICMPDropTimeout:           "10s",
						IcmpTimeout:               "10s",
					},
				},
			},
		},
		CollectionCommon: CollectionCommon{Testbed: testbed, Client: client},
	}
}

func (fwp* FirewallProfileCollection) GetDefaultFirewallProfile() (*security.FirewallProfile, error) {
	fwps, err := fwp.Client.ListFirewallProfile()

	if err != nil {
		return nil, err
	}

	if len(fwps) < 1 {
		err = fmt.Errorf("Empty policy profile : %v", fwps)
		return nil, err
	}

	return fwps[0], nil
}

func (fwp *FirewallProfileCollection) SetFirewallLimit(sessType  string, limit int) error {
	if fwp.err != nil {
		return fwp.err
	}

	var tcpHalfOpenSessionLimit, udpActiveSessionLimit, icmpActiveSessionLimit, otherActiveSessionLimit uint32

	for _, profile := range fwp.Profiles {
		tcpHalfOpenSessionLimit = profile.VeniceFirewallProfile.Spec.TcpHalfOpenSessionLimit
		udpActiveSessionLimit	= profile.VeniceFirewallProfile.Spec.UdpActiveSessionLimit
		icmpActiveSessionLimit = profile.VeniceFirewallProfile.Spec.IcmpActiveSessionLimit
		otherActiveSessionLimit = profile.VeniceFirewallProfile.Spec.OtherActiveSessionLimit
		break
	}

	switch sessType {
	case "TCP" :
		tcpHalfOpenSessionLimit = uint32(limit)
	case "UDP" :
		udpActiveSessionLimit	= uint32(limit)
	case "ICMP" :
		icmpActiveSessionLimit = uint32(limit)
	case "OTHER" :
		otherActiveSessionLimit = uint32(limit)
	case "ALL" :
		tcpHalfOpenSessionLimit = uint32(limit)
		udpActiveSessionLimit	= uint32(limit)
		icmpActiveSessionLimit = uint32(limit)
		otherActiveSessionLimit = uint32(limit)
	default:
		return fmt.Errorf("Invalid sessionType : %s",  sessType)
	}

	for _, profile := range fwp.Profiles {
		profile.VeniceFirewallProfile.Spec.TcpHalfOpenSessionLimit	=	tcpHalfOpenSessionLimit
		profile.VeniceFirewallProfile.Spec.UdpActiveSessionLimit	=	udpActiveSessionLimit
		profile.VeniceFirewallProfile.Spec.IcmpActiveSessionLimit	=	icmpActiveSessionLimit
		profile.VeniceFirewallProfile.Spec.OtherActiveSessionLimit	=	otherActiveSessionLimit
	}

	return nil
}

func (fwp *FirewallProfileCollection) EnableFirewallLimit(limit int) error {
	return fwp.SetFirewallLimit("ALL", limit)
}

func (fwp *FirewallProfileCollection) DisableFirewallLimit() error {
	return fwp.SetFirewallLimit("ALL", 0)
}

func (fwp *FirewallProfileCollection) Commit() error {
	if fwp.err != nil {
		return fwp.err
	}

	for _, profile := range fwp.Profiles {
		err := fwp.Client.UpdateFirewallProfile(profile.VeniceFirewallProfile)
		fwp.err = err
		if err != nil {
			return err
		}
	}
	return nil
}
