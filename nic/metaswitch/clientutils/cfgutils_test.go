package clientutils

import (
	"encoding/json"
	"fmt"
	"github.com/pensando/sw/api/generated/network"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
	"testing"

	"github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/ref"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func dumpNice(in interface{}) string {
	out, _ := json.MarshalIndent(in, "", "  ")
	return string(out)
}

func validateGlobal(in netproto.BGPConfig, ret pds.BGPSpec) error {
	if in.ASNumber != ret.LocalASN {
		return fmt.Errorf("mismatched ASN [%d][%d]", in.ASNumber, ret.LocalASN)
	}
	if in.RouterId != "" && ip2uint32(in.RouterId) != ret.RouterId {
		return fmt.Errorf("mismatched ASN [%v][%v]", in.RouterId, ret.RouterId)
	}
	return nil
}

func validatePeers(lb string, hold, keepalive uint32, in []*netproto.BGPNeighbor, ret pds.BGPPeerRequest) error {
	if len(in) != len(ret.Request) {
		return fmt.Errorf("numbers do not match")
	}
	inm := make(map[string]*netproto.BGPNeighbor)
	for _, v := range in {
		inm[v.IPAddress] = v
	}
	for _, r := range ret.Request {
		k := pdsIPtoStr(r.PeerAddr)
		p, ok := inm[k]
		if !ok {
			return fmt.Errorf("unexpected peer [%v]", k)
		}
		if p.RemoteAS != r.RemoteASN {
			return fmt.Errorf("mismatched AS number [%v][%v]", p.RemoteAS, r.RemoteASN)
		}
		if p.MultiHop != r.TTL {
			return fmt.Errorf("mismatched AS number [%v][%v]", p.RemoteAS, r.RemoteASN)
		}
		if p.Shutdown && r.State != pds.AdminState_ADMIN_STATE_DISABLE || !p.Shutdown && r.State != pds.AdminState_ADMIN_STATE_ENABLE {
			return fmt.Errorf("mismatched admin state [%v][%v]", p.Shutdown, r.State.String())
		}
		if p.Password != string(r.Password) {
			return fmt.Errorf("mismatched passwords [%v][%v]", p.Password, r.Password)
		}

		if p.EnableAddressFamilies[0] == network.BGPAddressFamily_L2vpnEvpn.String() {
			if pdsIPtoStr(r.LocalAddr) != lb {
				return fmt.Errorf("Invalid local address for evpn adderess family [%v][%v]", lb, pdsIPtoStr(r.LocalAddr))
			}
		}
	}
	return nil
}

func validatePeerDelete(lb string, in []*netproto.BGPNeighbor, ret pds.BGPPeerDeleteRequest) error {
	if len(in) != len(ret.Request) {
		return fmt.Errorf("numbers do not match")
	}
	inm := make(map[string]*netproto.BGPNeighbor)
	for _, v := range in {
		inm[v.IPAddress] = v
	}
	for _, r := range ret.Request {
		k := r.GetKey()
		p, ok := inm[pdsIPtoStr(k.PeerAddr)]
		if !ok {
			return fmt.Errorf("unexpected peer [%v]", k)
		}
		if p.EnableAddressFamilies[0] == network.BGPAddressFamily_L2vpnEvpn.String() {
			if lb != pdsIPtoStr(k.LocalAddr) {
				return fmt.Errorf("invalid local address [%v][%v]", lb, pdsIPtoStr(k.LocalAddr))
			}
		} else {
			if k.LocalAddr.V4OrV6 != nil {
				return fmt.Errorf("local address set for ipv4 underlay [%v]", k.LocalAddr)
			}
		}
	}
	return nil
}

func validatePeerAF(lb string, in []*netproto.BGPNeighbor, ret pds.BGPPeerAfRequest) error {
	inm := make(map[string]*netproto.BGPNeighbor)
	for _, v := range in {
		inm[v.IPAddress] = v
	}
	for _, v := range ret.Request {
		p, ok := inm[pdsIPtoStr(v.PeerAddr)]
		if !ok {
			return fmt.Errorf("unexpected peer [%v]", pdsIPtoStr(v.PeerAddr))
		}
		if p.EnableAddressFamilies[0] == network.BGPAddressFamily_L2vpnEvpn.String() {
			if lb != pdsIPtoStr(v.LocalAddr) {
				return fmt.Errorf("invalid local address [%v][%v]", lb, pdsIPtoStr(v.LocalAddr))
			}
			if v.Afi != pds.BGPAfi_BGP_AFI_L2VPN && v.Safi != pds.BGPSafi_BGP_SAFI_EVPN {
				return fmt.Errorf("unexpected AFI/SAFI [%v][%v/%v]", p.EnableAddressFamilies, v.Afi, v.Safi)
			}
		} else {
			if v.LocalAddr.V4OrV6 != nil {
				return fmt.Errorf("local address set for ipv4 underlay [%v]", v.LocalAddr)
			}
			if v.Afi != pds.BGPAfi_BGP_AFI_IPV4 && v.Safi != pds.BGPSafi_BGP_SAFI_UNICAST {
				return fmt.Errorf("unexpected AFI/SAFI [%v][%v/%v]", p.EnableAddressFamilies, v.Afi, v.Safi)
			}
		}
	}
	return nil
}

func validatePeerAFDelete(lb string, in []*netproto.BGPNeighbor, ret pds.BGPPeerAfDeleteRequest) error {
	inm := make(map[string]*netproto.BGPNeighbor)
	for _, v := range in {
		inm[v.IPAddress] = v
	}
	for _, v := range ret.Request {
		k := v.GetKey()
		p, ok := inm[pdsIPtoStr(k.PeerAddr)]
		if !ok {
			return fmt.Errorf("unexpected peer [%v]", pdsIPtoStr(k.PeerAddr))
		}
		if p.EnableAddressFamilies[0] == network.BGPAddressFamily_L2vpnEvpn.String() {
			if lb != pdsIPtoStr(k.LocalAddr) {
				return fmt.Errorf("invalid local address [%v][%v]", lb, pdsIPtoStr(k.LocalAddr))
			}
			if k.Afi != pds.BGPAfi_BGP_AFI_L2VPN && k.Safi != pds.BGPSafi_BGP_SAFI_EVPN {
				return fmt.Errorf("unexpected AFI/SAFI [%v][%v/%v]", p.EnableAddressFamilies, k.Afi, k.Safi)
			}
		} else {
			if k.LocalAddr.V4OrV6 != nil {
				return fmt.Errorf("local address set for ipv4 underlay [%v]", k.LocalAddr)
			}
			if k.Afi != pds.BGPAfi_BGP_AFI_IPV4 && k.Safi != pds.BGPSafi_BGP_SAFI_UNICAST {
				return fmt.Errorf("unexpected AFI/SAFI [%v][%v/%v]", p.EnableAddressFamilies, k.Afi, k.Safi)
			}
		}
	}
	return nil
}

func TestCfgUtils(t *testing.T) {
	// New Create
	newCfg := &netproto.RoutingConfig{
		ObjectMeta: api.ObjectMeta{Name: "test", UUID: uuid.NewV4().String()},
		Spec: netproto.RoutingConfigSpec{
			BGPConfig: &netproto.BGPConfig{
				RouterId:          "1.1.1.1",
				ASNumber:          1000,
				Holdtime:          30,
				KeepaliveInterval: 10,
				Neighbors: []*netproto.BGPNeighbor{
					{
						Shutdown:              false,
						IPAddress:             "10.1.1.1",
						RemoteAS:              1000,
						MultiHop:              10,
						EnableAddressFamilies: []string{"l2vpn-evpn"},
					},
					{
						Shutdown:              true,
						IPAddress:             "10.1.1.2",
						RemoteAS:              2000,
						MultiHop:              10,
						EnableAddressFamilies: []string{"ipv4-unicast"},
						Password:              "xyz",
					},
				},
			},
		},
	}

	ret, err := GetBGPConfiguration(nil, newCfg, "", "2.2.2.2")
	AssertOk(t, err, "failed to get BGP config (%s)", err)
	Assert(t, len(ret.DelPeers.Request) == 0, "not expecting any delete peers got [%v]", len(ret.DelPeers.Request))
	Assert(t, len(ret.AddPeers.Request) == 2, "expecting 2 Add peers got [%v]", len(ret.AddPeers.Request))
	Assert(t, len(ret.DelPeerAF.Request) == 0, "not expecting 2 del peers af got [%v]", len(ret.DelPeerAF.Request))
	Assert(t, len(ret.AddPeerAF.Request) == 2, "expecting 3 add peers af got [%v]", len(ret.AddPeerAF.Request))
	AssertOk(t, validatePeers("2.2.2.2", 30, 10, newCfg.Spec.BGPConfig.Neighbors, ret.AddPeers), "Validate peers failed (%s)", err)
	AssertOk(t, validatePeerAF("2.2.2.2", newCfg.Spec.BGPConfig.Neighbors, ret.AddPeerAF), "validation of PeerAf failed (%s)", err)

	// Update Operations
	// -  Update Global config
	oldCfg := ref.DeepCopy(newCfg).(*netproto.RoutingConfig)
	newCfg.Spec.BGPConfig.ASNumber = 10001
	ret, err = GetBGPConfiguration(oldCfg, newCfg, "", "2.2.2.2")
	Assert(t, err != nil, "should have failed due to ASN change")

	newCfg.Spec.BGPConfig.ASNumber = 1000
	newCfg.Spec.BGPConfig.RouterId = "1.1.1.2"
	ret, err = GetBGPConfiguration(oldCfg, newCfg, "", "2.2.2.2")
	AssertOk(t, err, "should have succeeded")
	Assert(t, ret.GlobalOper == Update, "expecting Update got %v", ret.GlobalOper)
	Assert(t, ret.Global.Request != nil, "expecting request to be not nil ")
	Assert(t, ret.Global.Request.RouterId == 0x01010102, "expecting request to be 1.1.1.2 got %x", ret.Global.Request.RouterId)

	// -  Update Add / del peers
	newCfg.Spec.BGPConfig.RouterId = "1.1.1.1"
	oldCfg = ref.DeepCopy(newCfg).(*netproto.RoutingConfig)
	newCfg = &netproto.RoutingConfig{
		ObjectMeta: api.ObjectMeta{Name: "test", UUID: uuid.NewV4().String()},
		Spec: netproto.RoutingConfigSpec{
			BGPConfig: &netproto.BGPConfig{
				RouterId:          "1.1.1.1",
				ASNumber:          1000,
				Holdtime:          30,
				KeepaliveInterval: 10,
				Neighbors: []*netproto.BGPNeighbor{
					{
						Shutdown:              false,
						IPAddress:             "11.1.1.1",
						RemoteAS:              1000,
						MultiHop:              10,
						EnableAddressFamilies: []string{"l2vpn-evpn"},
					},
					{
						Shutdown:              true,
						IPAddress:             "10.1.1.2",
						RemoteAS:              2000,
						MultiHop:              10,
						EnableAddressFamilies: []string{"ipv4-unicast"},
						Password:              "xyz",
					},
				},
			},
		},
	}
	ret, err = GetBGPConfiguration(oldCfg, newCfg, "", "2.2.2.2")
	AssertOk(t, err, "should have succeeded")
	Assert(t, ret.GlobalOper == None, "expecting None got %v", ret.GlobalOper)
	Assert(t, len(ret.DelPeers.Request) == 1, "not expecting any delete peers got [%v]", len(ret.DelPeers.Request))
	Assert(t, len(ret.AddPeers.Request) == 1, "expecting 2 Add peers got [%v]", len(ret.AddPeers.Request))
	Assert(t, len(ret.DelPeerAF.Request) == 0, "not expecting 2 del peers af got [%v]", len(ret.DelPeerAF.Request))
	Assert(t, len(ret.AddPeerAF.Request) == 1, "expecting 3 add peers af got [%v]", len(ret.AddPeerAF.Request))

	Assert(t, pdsIPtoStr(ret.AddPeers.Request[0].PeerAddr) == "11.1.1.1", "got wrong peer address, got [%v]", pdsIPtoStr(ret.AddPeers.Request[0].PeerAddr))
	Assert(t, pdsIPtoStr(ret.AddPeers.Request[0].LocalAddr) == "2.2.2.2", "got wrong local address, got [%v]", pdsIPtoStr(ret.AddPeers.Request[0].PeerAddr))
	Assert(t, pdsIPtoStr(ret.DelPeers.Request[0].GetKey().LocalAddr) == "0.0.0.0", "got wrong Local address, got [%v]", pdsIPtoStr(ret.AddPeers.Request[0].PeerAddr))
	Assert(t, pdsIPtoStr(ret.DelPeers.Request[0].GetKey().PeerAddr) == "10.1.1.1", "got wrong peer address, got [%v]", pdsIPtoStr(ret.AddPeers.Request[0].PeerAddr))

	// Change in loopback IP
	ret, err = GetBGPConfiguration(oldCfg, newCfg, "1.1.1.1", "2.2.2.2")
	AssertOk(t, err, "should have succeeded")
	Assert(t, ret.GlobalOper == None, "expecting None got %v", ret.GlobalOper)
	Assert(t, len(ret.DelPeers.Request) == 1, "not expecting any delete peers got [%v]", len(ret.DelPeers.Request))
	Assert(t, len(ret.AddPeers.Request) == 1, "expecting 2 Add peers got [%v]", len(ret.AddPeers.Request))
	Assert(t, len(ret.DelPeerAF.Request) == 0, "not expecting 2 del peers af got [%v]", len(ret.DelPeerAF.Request))
	Assert(t, len(ret.AddPeerAF.Request) == 1, "expecting 1 add peers af got [%v]", len(ret.AddPeerAF.Request))

	Assert(t, pdsIPtoStr(ret.AddPeers.Request[0].PeerAddr) == "11.1.1.1", "got wrong peer address, got [%v]", pdsIPtoStr(ret.AddPeers.Request[0].PeerAddr))
	Assert(t, pdsIPtoStr(ret.AddPeers.Request[0].LocalAddr) == "2.2.2.2", "got wrong local address, got [%v]", pdsIPtoStr(ret.AddPeers.Request[0].LocalAddr))
	Assert(t, pdsIPtoStr(ret.DelPeers.Request[0].GetKey().LocalAddr) == "1.1.1.1", "got wrong Local address, got [%v]", pdsIPtoStr(ret.AddPeers.Request[0].LocalAddr))
	Assert(t, pdsIPtoStr(ret.DelPeers.Request[0].GetKey().PeerAddr) == "10.1.1.1", "got wrong peer address, got [%v]", pdsIPtoStr(ret.AddPeers.Request[0].PeerAddr))

	// - Change peer config
	oldCfg = ref.DeepCopy(newCfg).(*netproto.RoutingConfig)
	newCfg = &netproto.RoutingConfig{
		ObjectMeta: api.ObjectMeta{Name: "test", UUID: uuid.NewV4().String()},
		Spec: netproto.RoutingConfigSpec{
			BGPConfig: &netproto.BGPConfig{
				RouterId:          "1.1.1.1",
				ASNumber:          1000,
				Holdtime:          30,
				KeepaliveInterval: 10,
				Neighbors: []*netproto.BGPNeighbor{
					{
						Shutdown:              false,
						IPAddress:             "11.1.1.1",
						RemoteAS:              2000,
						MultiHop:              10,
						EnableAddressFamilies: []string{"l2vpn-evpn", "ipv4-unicast"},
					},
					{
						Shutdown:              true,
						IPAddress:             "10.1.1.2",
						RemoteAS:              2000,
						MultiHop:              10,
						EnableAddressFamilies: []string{"ipv4-unicast"},
						Password:              "xyz",
					},
				},
			},
		},
	}
	ret, err = GetBGPConfiguration(oldCfg, newCfg, "", "2.2.2.2")
	AssertOk(t, err, "should have succeeded")
	Assert(t, ret.GlobalOper == None, "expecting None got %v", ret.GlobalOper)
	Assert(t, len(ret.DelPeers.Request) == 0, "not expecting any delete peers got [%v]", len(ret.DelPeers.Request))
	Assert(t, len(ret.AddPeers.Request) == 0, "expecting 2 Add peers got [%v]", len(ret.AddPeers.Request))
	Assert(t, len(ret.UpdPeers.Request) == 1, "expecting 2 Add peers got [%v]", len(ret.UpdPeers.Request))
	Assert(t, len(ret.DelPeerAF.Request) == 0, "not expecting 2 del peers af got [%v]", len(ret.DelPeerAF.Request))
	Assert(t, len(ret.AddPeerAF.Request) == 1, "expecting 1 add peers af got [%v]", len(ret.AddPeerAF.Request))

	Assert(t, pdsIPtoStr(ret.UpdPeers.Request[0].PeerAddr) == "11.1.1.1", "got wrong peer address, got [%v]", pdsIPtoStr(ret.UpdPeers.Request[0].PeerAddr))
	// EBGP forced to  0.0.0.0
	Assert(t, pdsIPtoStr(ret.UpdPeers.Request[0].LocalAddr) == "0.0.0.0", "got wrong local address, got [%v]", pdsIPtoStr(ret.UpdPeers.Request[0].LocalAddr))
	Assert(t, ret.UpdPeers.Request[0].RemoteASN == 2000, "got wrong local address, got [%v]", pdsIPtoStr(ret.UpdPeers.Request[0].PeerAddr))

	// -- remote ASN
	newCfg.Spec.BGPConfig.ASNumber = 2000
	ret, err = GetBGPConfiguration(oldCfg, newCfg, "", "2.2.2.2")
	Assert(t, err != nil, "should not have succeeded")

	// Delete operation
	ret, err = GetBGPConfiguration(newCfg, nil, "", "2.2.2.2")
	AssertOk(t, err, "should have succeeded")
	Assert(t, ret.GlobalOper == Delete, "expecting None got %v", ret.GlobalOper)
	Assert(t, len(ret.DelPeers.Request) == 2, "not expecting any delete peers got [%v]", len(ret.DelPeers.Request))
	Assert(t, len(ret.AddPeers.Request) == 0, "expecting 2 Add peers got [%v]", len(ret.AddPeers.Request))
	Assert(t, len(ret.UpdPeers.Request) == 0, "expecting 2 Add peers got [%v]", len(ret.UpdPeers.Request))
	Assert(t, len(ret.DelPeerAF.Request) == 0, "not expecting 2 del peers af got [%v]", len(ret.DelPeerAF.Request))
	Assert(t, len(ret.AddPeerAF.Request) == 0, "expecting 3 add peers af got [%v]", len(ret.AddPeerAF.Request))

	oldCfg = &netproto.RoutingConfig{
		ObjectMeta: api.ObjectMeta{Name: "test", UUID: uuid.NewV4().String()},
		Spec: netproto.RoutingConfigSpec{
			BGPConfig: &netproto.BGPConfig{
				RouterId:          "1.1.1.1",
				ASNumber:          1000,
				Holdtime:          30,
				KeepaliveInterval: 10,
				Neighbors: []*netproto.BGPNeighbor{
					{
						Shutdown:              false,
						IPAddress:             "11.1.1.1",
						RemoteAS:              1000,
						MultiHop:              10,
						EnableAddressFamilies: []string{"l2vpn-evpn"},
					},
					{
						Shutdown:              true,
						IPAddress:             "10.1.1.2",
						RemoteAS:              2000,
						MultiHop:              10,
						EnableAddressFamilies: []string{"ipv4-unicast"},
						Password:              "xyz",
					},
				},
			},
		},
	}
	ret, err = GetBGPConfiguration(oldCfg, nil, "1.1.1.1", "2.2.2.2")
	AssertOk(t, err, "should have succeeded")
	Assert(t, ret.GlobalOper == Delete, "expecting None got %v", ret.GlobalOper)
	Assert(t, len(ret.DelPeers.Request) == 2, "not expecting any delete peers got [%v]", len(ret.DelPeers.Request))
	Assert(t, len(ret.AddPeers.Request) == 0, "expecting 2 Add peers got [%v]", len(ret.AddPeers.Request))
	Assert(t, len(ret.UpdPeers.Request) == 0, "expecting 2 Add peers got [%v]", len(ret.UpdPeers.Request))
	Assert(t, len(ret.DelPeerAF.Request) == 0, "not expecting 2 del peers af got [%v]", len(ret.DelPeerAF.Request))
	Assert(t, len(ret.AddPeerAF.Request) == 0, "expecting 3 add peers af got [%v]", len(ret.AddPeerAF.Request))
	AssertOk(t, validatePeerDelete("1.1.1.1", oldCfg.Spec.BGPConfig.Neighbors, ret.DelPeers), "failed validation of delpeers (%s)", err)
	for _, d := range ret.DelPeers.Request {
		if pdsIPtoStr(d.GetKey().PeerAddr) == "11.1.1.1" && pdsIPtoStr(d.GetKey().LocalAddr) != "1.1.1.1" {
			t.Errorf("Localaddress is not set right")
		}
	}

}
