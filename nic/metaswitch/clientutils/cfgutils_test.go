package clientutils

import (
	"encoding/json"
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

func TestCfgUtils(t *testing.T) {
	// New Create
	newCfg := &netproto.RoutingConfig{
		ObjectMeta: api.ObjectMeta{Name: "test", UUID: uuid.NewV4().String()},
		Spec: netproto.RoutingConfigSpec{
			BGPConfig: &netproto.BGPConfig{
				RouterId: "1.1.1.1",
				ASNumber: 1000,
				Neighbors: []*netproto.BGPNeighbor{
					{
						Shutdown:              false,
						IPAddress:             "10.1.1.1",
						RemoteAS:              1000,
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

	ret, err := GetBGPConfiguration(nil, newCfg, "", "2.2.2.2")
	AssertOk(t, err, "failed to get BGP config (%s)", err)
	Assert(t, len(ret.DelPeers.Request) == 0, "not expecting any delete peers got [%v]", len(ret.DelPeers.Request))
	Assert(t, len(ret.AddPeers.Request) == 2, "expecting 2 Add peers got [%v]", len(ret.AddPeers.Request))
	Assert(t, len(ret.DelPeerAF.Request) == 0, "not expecting 2 del peers af got [%v]", len(ret.DelPeerAF.Request))
	Assert(t, len(ret.AddPeerAF.Request) == 3, "expecting 3 add peers af got [%v]", len(ret.AddPeerAF.Request))

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
	Assert(t, ret.Global.Request.RouterId == 0x02010101, "expecting request to be 1.1.1.2 got %x", ret.Global.Request.RouterId)

	// -  Update Add / del peers
	newCfg.Spec.BGPConfig.RouterId = "1.1.1.1"
	oldCfg = ref.DeepCopy(newCfg).(*netproto.RoutingConfig)
	newCfg = &netproto.RoutingConfig{
		ObjectMeta: api.ObjectMeta{Name: "test", UUID: uuid.NewV4().String()},
		Spec: netproto.RoutingConfigSpec{
			BGPConfig: &netproto.BGPConfig{
				RouterId: "1.1.1.1",
				ASNumber: 1000,
				Neighbors: []*netproto.BGPNeighbor{
					{
						Shutdown:              false,
						IPAddress:             "11.1.1.1",
						RemoteAS:              1000,
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
	Assert(t, len(ret.DelPeers.Request) == 1, "not expecting any delete peers got [%v]", len(ret.DelPeers.Request))
	Assert(t, len(ret.AddPeers.Request) == 1, "expecting 2 Add peers got [%v]", len(ret.AddPeers.Request))
	Assert(t, len(ret.DelPeerAF.Request) == 0, "not expecting 2 del peers af got [%v]", len(ret.DelPeerAF.Request))
	Assert(t, len(ret.AddPeerAF.Request) == 2, "expecting 3 add peers af got [%v]", len(ret.AddPeerAF.Request))

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
	Assert(t, len(ret.AddPeerAF.Request) == 2, "expecting 3 add peers af got [%v]", len(ret.AddPeerAF.Request))

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
				RouterId: "1.1.1.1",
				ASNumber: 1000,
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
	Assert(t, len(ret.AddPeerAF.Request) == 0, "expecting 3 add peers af got [%v]", len(ret.AddPeerAF.Request))

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
				RouterId: "1.1.1.1",
				ASNumber: 1000,
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
	for _, d := range ret.DelPeers.Request {
		if pdsIPtoStr(d.GetKey().PeerAddr) == "11.1.1.1" && pdsIPtoStr(d.GetKey().LocalAddr) != "1.1.1.1" {
			t.Errorf("Localaddress is not set right")
		}
	}

}
