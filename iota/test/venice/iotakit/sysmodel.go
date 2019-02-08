// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package iotakit

import (
	"encoding/binary"
	"fmt"
	"math"
	"math/rand"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/security"
	iota "github.com/pensando/sw/iota/protos/gogen"
	"github.com/pensando/sw/venice/utils/log"
)

// default number of workloads per host
const defaultWorkloadPerHost = 4

// default number of networks in the model
const defaultNumNetworks = 2

// SysModel represents a model of the system under test
type SysModel struct {
	hosts       map[string]*Host       // hosts
	naples      map[string]*Naples     // Naples instances
	workloads   map[string]*Workload   // workloads
	subnets     map[string]*Network    // subnets
	sgpolicies  map[string]*SGPolicy   // security policies
	veniceNodes map[string]*VeniceNode // Venice nodes

	tb *TestBed // testbed

	allocatedMac map[string]bool // allocated mac addresses
}

// NewSysModel creates a sysmodel for a testbed
func NewSysModel(tb *TestBed) (*SysModel, error) {
	sm := SysModel{
		tb:           tb,
		hosts:        make(map[string]*Host),
		naples:       make(map[string]*Naples),
		veniceNodes:  make(map[string]*VeniceNode),
		workloads:    make(map[string]*Workload),
		subnets:      make(map[string]*Network),
		sgpolicies:   make(map[string]*SGPolicy),
		allocatedMac: make(map[string]bool),
	}

	// build naples and venice nodes
	for _, nr := range sm.tb.addNodeResp.Nodes {
		if nr.Type == iota.PersonalityType_PERSONALITY_NAPLES_SIM || nr.Type == iota.PersonalityType_PERSONALITY_NAPLES {
			err := sm.createNaples(nr)
			if err != nil {
				return nil, err
			}
		} else if nr.Type == iota.PersonalityType_PERSONALITY_VENICE {
			// create
			err := sm.createVeniceNode(nr)
			if err != nil {
				return nil, err
			}
		}
	}

	return &sm, nil
}

// SetupDefaultConfig sets up a default config for the system
func (sm *SysModel) SetupDefaultConfig() error {
	log.Infof("Setting up default config...")

	// create some networks
	for i := 0; i < defaultNumNetworks; i++ {
		vlanID := sm.tb.allocatedVlans[i]
		err := sm.createNetwork(vlanID, fmt.Sprintf("192.168.%d.0/24", i+2))
		if err != nil {
			return err
		}
	}

	// build host list for configuration
	var naplesNodes []*iota.Node
	for _, nr := range sm.tb.Nodes {
		if nr.Personality == iota.PersonalityType_PERSONALITY_NAPLES_SIM || nr.Personality == iota.PersonalityType_PERSONALITY_NAPLES {
			naplesNodes = append(naplesNodes, nr.iotaNode)
		}
	}

	snc := sm.Networks()

	// Create Host Objects
	for _, n := range naplesNodes {
		h, err := sm.createHost(n)
		if err != nil {
			return err
		}

		for i := 0; i < defaultWorkloadPerHost; i++ {
			name := fmt.Sprintf("%s_wrkld_%d", n.Name, i)
			subnet := snc.subnets[i%len(snc.subnets)]
			_, err = sm.createWorkload(sm.tb.Topo.WorkloadType, sm.tb.Topo.WorkloadImage, name, h, subnet)
			if err != nil {
				return err
			}
		}
	}

	// create a default firewall profile
	err := sm.createDefaultFwprofile()
	if err != nil {
		return err
	}

	// create default allow policy
	err = sm.NewSGPolicy("default-policy").AddRule("any", "any", "", "PERMIT").Commit()
	if err != nil {
		return err
	}

	return nil
}

// allocMacAddress allocates a unique mac address
func (sm *SysModel) allocMacAddress() (string, error) {
	// Fixed seed to generate repeatable IP Addresses
	var retryCount = 1000

	for i := 0; i < retryCount; i++ {
		b := make([]byte, 8)
		num := uint64(rand.Int63n(math.MaxInt64))
		binary.BigEndian.PutUint64(b, num)
		// This will ensure that we have unicast MAC
		b[0] = (b[0] | 2) & 0xfe
		mac := fmt.Sprintf("%02x:%02x:%02x:%02x:%02x:%02x", b[0], b[1], b[2], b[3], b[4], b[5])

		// see if this mac addr was already used
		if _, ok := sm.allocatedMac[mac]; !ok {
			sm.allocatedMac[mac] = true
			return mac, nil
		}
	}

	return "", fmt.Errorf("Could not allocate a mac address")
}

// creates a default firewall profile for the tests
func (sm *SysModel) createDefaultFwprofile() error {
	fwp := security.FirewallProfile{
		TypeMeta: api.TypeMeta{Kind: "FirewallProfile"},
		ObjectMeta: api.ObjectMeta{
			Name:      "default-fwprofile",
			Namespace: "default",
			Tenant:    "default",
		},
		// FIXME: set short timeout value till HAL fixes the drop->allow transition issue
		Spec: security.FirewallProfileSpec{
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
	}

	return sm.tb.CreateFirewallProfile(&fwp)
}
