// +build ignore

package restapi

import (
	"errors"
	"fmt"
	"strings"

	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/netutils"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var ErrMirrorSpecMismatch = errors.New("Mirror specification mismatched")

func postMirorrSessions(mirrorURL string, postData *netproto.MirrorSession) error {
	var msGet netproto.MirrorSession
	//var ok bool
	postURL := fmt.Sprintf("%s/", mirrorURL)
	err := netutils.HTTPPost(postURL, postData, nil)
	if err != nil {
		fmt.Println("Error posting mirrorsession to REST Server")
		return err
	}
	getURL := fmt.Sprintf("%s/%s/%s/%s/", mirrorURL, postData.ObjectMeta.Tenant,
		postData.ObjectMeta.Namespace, postData.ObjectMeta.Name)
	err = netutils.HTTPGet(getURL, &msGet)
	if err != nil {
		fmt.Println("Error getting mirrorsessions from the REST Server")
		return err
	}
	//if msGet.Name == postData.ObjectMeta.Name && msGet.Spec.PacketSize == postData.Spec.PacketSize {
	//	ok = true
	//}
	//if !ok {
	//	jsonPostData, _ := json.Marshal(postData)
	//	jsonGetData, _ := json.Marshal(msGet)
	//	fmt.Printf("Error. Created mirrorSession did not match with retrieved mirrorSession {%s} {%s}\n",
	//		jsonPostData, jsonGetData)
	//	return ErrMirrorSpecMismatch
	//}
	return nil
}

func deleteMirorrSessions(mirrorURL string, deleteData *netproto.MirrorSession) error {
	deleteURL := fmt.Sprintf("%s/%s/%s/%s", mirrorURL, deleteData.ObjectMeta.Tenant,
		deleteData.ObjectMeta.Namespace, deleteData.ObjectMeta.Name)
	err := netutils.HTTPDelete(deleteURL, deleteData, nil)
	if err != nil {
		fmt.Printf("Error deleting mirrorsession %s\n", deleteData.Name)
		return err
	}
	return nil
}

func putMirorrSessions(mirrorURL string, putData *netproto.MirrorSession) error {
	var msGet netproto.MirrorSession
	//var ok bool
	putURL := fmt.Sprintf("%s/%s/%s/%s", mirrorURL, putData.ObjectMeta.Tenant,
		putData.ObjectMeta.Namespace, putData.ObjectMeta.Name)
	err := netutils.HTTPPut(putURL, putData, nil)
	if err != nil {
		fmt.Println("Error updating mirrorsession")
		return err
	}
	getURL := fmt.Sprintf("%s/%s/%s/%s/", mirrorURL, putData.ObjectMeta.Tenant,
		putData.ObjectMeta.Namespace, putData.ObjectMeta.Name)
	err = netutils.HTTPGet(getURL, &msGet)
	if err != nil {
		fmt.Println("Error getting mirrorsessions from the REST Server")
		return err
	}
	//if msGet.Name == putData.ObjectMeta.Name && msGet.Spec.PacketSize == putData.Spec.PacketSize {
	//	//msGet.Spec.CaptureAt == putData.Spec.CaptureAt {
	//	ok = true
	//}
	//if !ok {
	//	jsonPutData, _ := json.Marshal(putData)
	//	jsonGetData, _ := json.Marshal(msGet)
	//	fmt.Printf("Error. After mirrorSession update, spec did not match {%s}{%s}\n", jsonPutData, jsonGetData)
	//	return ErrMirrorSpecMismatch
	//}
	return nil
}

func setupMirrorSessionDependenctObjects(baseURL string) error {
	var resp Response

	epURL := fmt.Sprintf("http://%s/api/endpoints/", baseURL)
	nwURL := fmt.Sprintf("http://%s/api/networks/", baseURL)

	nw1 := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "default",
		},
		Spec: netproto.NetworkSpec{
			VlanID: 100,
		},
	}

	err := netutils.HTTPPost(nwURL, &nw1, &resp)
	if err != nil {
		fmt.Println("Could not create network")
		return err
	}

	eps := []netproto.Endpoint{
		{
			TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      "mirrorSession-EP1",
			},
			Spec: netproto.EndpointSpec{
				NetworkName:   "default",
				IPv4Addresses: []string{"66.0.2.1/16"},
				MacAddress:    "0022.0A00.0201",
				NodeUUID:      "GWUUID",
			},
		},
		{
			TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      "mirrorSession-EP2",
			},
			Spec: netproto.EndpointSpec{
				NetworkName:   "default",
				IPv4Addresses: []string{"66.0.2.1/16"},
				MacAddress:    "0022.0A00.0201",
				NodeUUID:      "GWUUID",
			},
		},
		{
			TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      "mirrorSession-EP3",
			},
			Spec: netproto.EndpointSpec{
				NetworkName:   "default",
				IPv4Addresses: []string{"66.0.3.1/16"},
				MacAddress:    "0022.0A00.0201",
				NodeUUID:      "GWUUID",
			},
		},
		{
			TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      "mirrorSession-EP4",
			},
			Spec: netproto.EndpointSpec{
				NetworkName:   "default",
				IPv4Addresses: []string{"66.0.4.1/16"},
				MacAddress:    "0022.0A00.0201",
				NodeUUID:      "GWUUID",
			},
		},
		{
			TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      "mirrorSession-EP5",
			},
			Spec: netproto.EndpointSpec{
				NetworkName:   "default",
				IPv4Addresses: []string{"66.0.5.1/16"},
				MacAddress:    "0022.0A00.0201",
				NodeUUID:      "GWUUID",
			},
		},
	}
	// Create all the endpoints
	for _, ep := range eps {
		err = netutils.HTTPPost(epURL, &ep, &resp)
		if err != nil {
			fmt.Println("Could not create endpoint")
			return err
		}
	}
	return nil
}

func tearDownMirrorSessionDependenctObjects(baseURL string) error {
	var resp Response

	epURL := fmt.Sprintf("http://%s/api/endpoints/", baseURL)
	nwURL := fmt.Sprintf("http://%s/api/networks/", baseURL)
	eps := []netproto.Endpoint{
		{
			TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      "mirrorSession-EP1",
			},
		},
		{
			TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      "mirrorSession-EP2",
			},
		},
		{
			TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      "mirrorSession-EP3",
			},
		},
		{
			TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      "mirrorSession-EP4",
			},
		},
		{
			TypeMeta: api.TypeMeta{Kind: "Endpoint"},
			ObjectMeta: api.ObjectMeta{
				Tenant:    "default",
				Namespace: "default",
				Name:      "mirrorSession-EP5",
			},
		},
	}

	// Delete endpoints
	for _, ep := range eps {
		epDeleteURL := fmt.Sprintf("%sdefault/default/%s", epURL, ep.Name)
		err := netutils.HTTPDelete(epDeleteURL, &ep, &resp)
		if err != nil {
			fmt.Println("Could not delete endpoint")
			return err
		}
	}

	nw1 := netproto.Network{
		TypeMeta: api.TypeMeta{Kind: "Network"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "default",
		},
	}

	networkDeleteURL := fmt.Sprintf("%sdefault/default/default", nwURL)
	err := netutils.HTTPDelete(networkDeleteURL, &nw1, &resp)
	if err != nil {
		fmt.Println("Could not delete network")
		return err
	}

	return nil
}

type mirrorSessionCreateUpdateDeleteProto struct {
	//create proto
	cProto *netproto.MirrorSession
	//update proto
	uProto *netproto.MirrorSession
	//delete proto
	dProto *netproto.MirrorSession
}

func prepareMirrorSessionDropTestCases() []mirrorSessionCreateUpdateDeleteProto {
	var testCases []mirrorSessionCreateUpdateDeleteProto
	var tc mirrorSessionCreateUpdateDeleteProto

	// case1: Different mirrorSession
	// case1.1: All drop reason create
	tc.cProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_all",
		},
		Spec: netproto.MirrorSessionSpec{
			Collectors: []netproto.MirrorCollector{
				{ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
		},
	}
	tc.uProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_all",
		},
		Spec: netproto.MirrorSessionSpec{
			//PacketSize:    256, //Modified
			//Enable:        true,
			//PacketFilters: []string{"ALL_DROPS"},
			//CaptureAt:     1, // Modified
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
		},
	}
	tc.dProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_all",
		},
	}
	testCases = append(testCases, tc)
	// case1.2: Network Policy drop reason create
	tc.cProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_nw",
		},
		Spec: netproto.MirrorSessionSpec{
			//PacketSize:    128,
			//Enable:        true,
			//PacketFilters: []string{"NETWORK_POLICY_DROP"},
			//CaptureAt:     0,
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
		},
	}
	tc.uProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_nw",
		},
		Spec: netproto.MirrorSessionSpec{
			//PacketSize:    256, //Modified
			//Enable:        true,
			//PacketFilters: []string{"NETWORK_POLICY_DROP"},
			//CaptureAt:     1, // Modified
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
		},
	}
	tc.dProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_nw",
		},
	}
	testCases = append(testCases, tc)
	// case1.3: Firewall Policy drop reason create
	tc.cProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw",
		},
		Spec: netproto.MirrorSessionSpec{
			//PacketSize:    128,
			//Enable:        true,
			//PacketFilters: []string{"FIREWALL_POLICY_DROP"},
			//CaptureAt:     0,
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
		},
	}
	tc.uProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw",
		},
		Spec: netproto.MirrorSessionSpec{
			//PacketSize:    256, //Modified
			//Enable:        true,
			//PacketFilters: []string{"FIREWALL_POLICY_DROP"},
			//CaptureAt:     1, // Modified
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
		},
	}
	tc.dProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_drop_fw",
		},
	}
	testCases = append(testCases, tc)
	return testCases
}

func prepareMirrorSessionFlowRuleIPAddrTestCases() []mirrorSessionCreateUpdateDeleteProto {
	var testCases []mirrorSessionCreateUpdateDeleteProto
	var tc mirrorSessionCreateUpdateDeleteProto

	// Case 2: Various mirror session test cases where flowRules specify complete IP Address

	// case2.1: srcIPaddr, destIPaddr, ipProto, L4port, 2 rules for matching, both rules modified
	tc.cProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port",
		},
		Spec: netproto.MirrorSessionSpec{
			//PacketSize: 128,
			//Enable:     true,
			//CaptureAt:  0,
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.1"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.2"},
					},
					////AppProtoSel: &netproto.AppProtoSelector{
					////	Ports: []string{"TCP/1000"},
					////},
				},
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.1"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.2"},
					},
					////AppProtoSel: &netproto.AppProtoSelector{
					////	Ports: []string{"TCP/1010"},
					////},
				},
			},
		},
	}
	tc.uProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port",
		},
		Spec: netproto.MirrorSessionSpec{
			//PacketSize: 256, //Modified
			//Enable:     true,
			//CaptureAt:  1, // Modified
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"192.1.1.1"}, // modified
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.1.1.2"}, // modified
					},
					////AppProtoSel: &netproto.AppProtoSelector{
					////	Ports: []string{"TCP/1000"},
					////},
				},
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.1"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.2"},
					},
					////AppProtoSel: &netproto.AppProtoSelector{
					////	Ports: []string{"TCP/1010"},
					////},
				},
			},
		},
	}
	tc.dProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port",
		},
	}
	testCases = append(testCases, tc)
	// case2.2: srcIPaddr, destIPaddr, ipProto, L4port, 2 rules for matching, one  rule removed during modification
	tc.cProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_1_rule_removed",
		},
		Spec: netproto.MirrorSessionSpec{
			//PacketSize: 128,
			//Enable:     true,
			//CaptureAt:  0,
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.1"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.2"},
					},
					////AppProtoSel: &netproto.AppProtoSelector{
					////	Ports: []string{"TCP/1000"},
					////},
				},
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.1"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.2"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1010"},
					//},
				},
			},
		},
	}
	tc.uProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_1_rule_removed",
		},
		Spec: netproto.MirrorSessionSpec{
			//PacketSize: 256, //Modified
			//Enable:     true,
			//CaptureAt:  1, // Modified
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"192.1.1.1"}, // modified
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.1.1.2"}, // modified
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1000"},
					//},
				},
				// one rule removed
			},
		},
	}
	tc.dProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_1_rule_removed",
		},
	}
	testCases = append(testCases, tc)
	// case2.3: srcIPaddr, destIPaddr, ipProto, L4port, 1 rules for matching, one more rule added during modification
	tc.cProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_1_rule_added",
		},
		Spec: netproto.MirrorSessionSpec{
			// PacketSize: 128,
			// Enable:     true,
			// CaptureAt:  0,
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.1"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.2"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1000"},
					//},
				},
			},
		},
	}
	tc.uProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_1_rule_added",
		},
		Spec: netproto.MirrorSessionSpec{
			//PacketSize: 256, //Modified
			//Enable:     true,
			//CaptureAt:  1, // Modified
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"192.1.1.1"}, // modified
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.1.1.2"}, // modified
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1000"},
					//},
				},
				// rule added
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.1"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.2"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1010"},
					//},
				},
			},
		},
	}
	tc.dProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_1_rule_added",
		},
	}
	testCases = append(testCases, tc)
	// case2.4: destIPaddr, ipProto, L4port, 1 rules for matching, one more rule added during modification
	tc.cProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_*sip_dip_proto_l4port_1_rule_added",
		},
		Spec: netproto.MirrorSessionSpec{
			// PacketSize: 128,
			// Enable:     true,
			// CaptureAt:  0,
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Dst: &netproto.MatchSelector{
						Addresses: []string{"10.1.1.2"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1000"},
					//},
				},
			},
		},
	}
	tc.uProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_*sip_dip_proto_l4port_1_rule_added",
		},
		Spec: netproto.MirrorSessionSpec{
			//PacketSize: 256, //Modified
			//Enable:     true,
			//CaptureAt:  1, // Modified
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.1.1.2"}, // modified
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1000"},
					//},
				},
				// rule added
				{
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.2"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1010"},
					//},
				},
			},
		},
	}
	tc.dProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_*sip_dip_proto_l4port_1_rule_added",
		},
	}
	testCases = append(testCases, tc)
	// case2.5: SIP=NIL, destIPaddr, App/Port NIL, 1 rules for matching, one more rule added during modification
	tc.cProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_*sip_dip_*appPort_1_rule_added",
		},
		Spec: netproto.MirrorSessionSpec{
			// PacketSize: 128,
			// Enable:     true,
			// CaptureAt:  0,
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Dst: &netproto.MatchSelector{
						Addresses: []string{"10.1.1.2"},
					},
				},
			},
		},
	}
	tc.uProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_*sip_dip_*appPort_1_rule_added",
		},
		Spec: netproto.MirrorSessionSpec{
			//PacketSize: 256, //Modified
			//Enable:     true,
			//CaptureAt:  1, // Modified
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.1.1.2"}, // modified
					},
				},
				// rule added
				{
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.2"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1010"},
					//},
				},
			},
		},
	}
	tc.dProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_*sip_dip_*appPort_1_rule_added",
		},
	}
	testCases = append(testCases, tc)
	// case2.6: SIP=NIL, destIP=NILL, App/Port=NIL, 1 rules for matching, one more rule added during modification
	tc.cProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_*sip_*dip_*appPort_1_rule_added",
		},
		Spec: netproto.MirrorSessionSpec{
			// PacketSize: 128,
			// Enable:     true,
			// CaptureAt:  0,
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{},
			},
		},
	}
	tc.uProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_*sip_*dip_*appPort_1_rule_added",
		},
		Spec: netproto.MirrorSessionSpec{
			//PacketSize: 256, //Modified
			//Enable:     true,
			//CaptureAt:  1, // Modified
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				// rule added
				{
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1010"},
					//},
				},
			},
		},
	}
	tc.dProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_*sip_*dip_*appPort_1_rule_added",
		},
	}
	testCases = append(testCases, tc)

	return testCases
}

func prepareMirrorSessionFlowRuleIPSubnetTestCases() []mirrorSessionCreateUpdateDeleteProto {
	var testCases []mirrorSessionCreateUpdateDeleteProto
	var tc mirrorSessionCreateUpdateDeleteProto

	// Case 3: Various mirror session test cases where flowRules specify SIP and DIP as subnet

	// case3.1: srcIPsubnet, destIPsubnet, ipProto, L4port, 2 rules for matching, one rule modified
	tc.cProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_subnet",
		},
		Spec: netproto.MirrorSessionSpec{
			// PacketSize: 128,
			// Enable:     true,
			// CaptureAt:  0,
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.0/24"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.0/24"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1000"},
					//},
				},
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.2.1.0/24"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.3.2.0/24"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1010"},
					//},
				},
			},
		},
	}
	tc.uProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_subnet",
		},
		Spec: netproto.MirrorSessionSpec{
			//PacketSize: 256, //Modified
			//Enable:     true,
			//CaptureAt:  1, // Modified
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.0/23"}, // modified
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.0/24"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1000"},
					//},
				},
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.3.1.0/24"}, // modified
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.4.2.0/24"}, // modified
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1010"},
					//},
				},
			},
		},
	}
	tc.dProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_subnet",
		},
	}
	testCases = append(testCases, tc)

	// case3.2: srcIPsubnet, destIPsubnet, ipProto, L4port, 2 rules for matching, one rule removed
	tc.cProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_subnet_rule_removed",
		},
		Spec: netproto.MirrorSessionSpec{
			// PacketSize: 128,
			// Enable:     true,
			// CaptureAt:  0,
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.0/24"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.0/24"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1000"},
					//},
				},
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.2.1.0/24"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.3.2.0/24"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1010"},
					//},
				},
			},
		},
	}
	tc.uProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_subnet_rule_removed",
		},
		Spec: netproto.MirrorSessionSpec{
			//PacketSize: 256, //Modified
			//Enable:     true,
			//CaptureAt:  1, // Modified
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.0/23"}, // modified
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.0/24"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1000"},
					//},
				},
			},
		},
	}
	tc.dProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_subnet_rule_removed",
		},
	}

	testCases = append(testCases, tc)

	// case3.3: srcIPsubnet, destIPsubnet, ipProto, L4port, 2 rules for matching, one rule added
	tc.cProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_subnet_rule_added",
		},
		Spec: netproto.MirrorSessionSpec{
			// PacketSize: 128,
			// Enable:     true,
			// CaptureAt:  0,
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.0/24"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.0/24"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1000"},
					//},
				},
			},
		},
	}
	tc.uProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_subnet_rule_added",
		},
		Spec: netproto.MirrorSessionSpec{
			//PacketSize: 256, //Modified
			//Enable:     true,
			//CaptureAt:  1, // Modified
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.0/23"}, // modified
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.0/24"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1000"},
					//},
				},
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.2.1.0/24"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.3.2.0/24"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1010"},
					//},
				},
			},
		},
	}
	tc.dProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_subnet_rule_added",
		},
	}

	testCases = append(testCases, tc)
	// case3.4: srcIPsubnet = any, destIPsubnet = any, ipProto, L4port, 2 rules for matching, one rule added
	tc.cProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sipAny_dipAny_proto_l4port_subnet_rule_added",
		},
		Spec: netproto.MirrorSessionSpec{
			// PacketSize: 128,
			// Enable:     true,
			// CaptureAt:  0,
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"any"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"any"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1000"},
					//},
				},
			},
		},
	}
	tc.uProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sipAny_dipAny_proto_l4port_subnet_rule_added",
		},
		Spec: netproto.MirrorSessionSpec{
			//PacketSize: 256, //Modified
			//Enable:     true,
			//CaptureAt:  1, // Modified
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"any"}, // modified
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"any"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1000"},
					//},
				},
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"any"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.3.2.0/24"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1010"},
					//},
				},
			},
		},
	}
	tc.dProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sipAny_dipAny_proto_l4port_subnet_rule_added",
		},
	}

	testCases = append(testCases, tc)
	// case3.5: srcIPsubnet = any, destIPsubnet = any, ipProto/L4port = any, 2 rules for matching, one rule added
	tc.cProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sipAny_dipAny_protoAny_l4portAny_subnet_rule_added",
		},
		Spec: netproto.MirrorSessionSpec{
			// PacketSize: 128,
			// Enable:     true,
			// CaptureAt:  0,
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"any"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"any"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"any/any"},
					//},
				},
			},
		},
	}
	tc.uProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sipAny_dipAny_protoAny_l4portAny_subnet_rule_added",
		},
		Spec: netproto.MirrorSessionSpec{
			//PacketSize: 256, //Modified
			//Enable:     true,
			//CaptureAt:  1, // Modified
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"any"}, // modified
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"any"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1000"},
					//},
				},
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"any"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.3.2.0/24"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"any/any"},
					//},
				},
			},
		},
	}
	tc.dProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sipAny_dipAny_protoAny_l4portAny_subnet_rule_added",
		},
	}

	testCases = append(testCases, tc)
	// case3.6: srcIPsubnet = any, destIPsubnet = any, ipProto = TCP, L4port = any, 2 rules for matching, one rule added
	tc.cProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sipAny_dipAny_proto_l4portAny_subnet_rule_added",
		},
		Spec: netproto.MirrorSessionSpec{
			// PacketSize: 128,
			// Enable:     true,
			// CaptureAt:  0,
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"any"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"any"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/any"},
					//},
				},
			},
		},
	}
	tc.uProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sipAny_dipAny_proto_l4portAny_subnet_rule_added",
		},
		Spec: netproto.MirrorSessionSpec{
			//PacketSize: 256, //Modified
			//Enable:     true,
			//CaptureAt:  1, // Modified
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"any"}, // modified
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"any"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1000"},
					//},
				},
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"any"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.3.2.0/24"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/any"},
					//},
				},
			},
		},
	}
	tc.dProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sipAny_dipAny_proto_l4portAny_subnet_rule_added",
		},
	}

	testCases = append(testCases, tc)

	return testCases
}

func prepareMirrorSessionFlowRuleDipSubnetTestCases() []mirrorSessionCreateUpdateDeleteProto {
	var testCases []mirrorSessionCreateUpdateDeleteProto
	var tc mirrorSessionCreateUpdateDeleteProto

	// Case 4: Various mirror session test cases where flowRules specify DIP as subnet

	// case4.1: srcIP, destIPsubnet, ipProto, L4port, 2 rules for matching, one rule modified
	tc.cProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_dip_subnet",
		},
		Spec: netproto.MirrorSessionSpec{
			// PacketSize: 128,
			// Enable:     true,
			// CaptureAt:  0,
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.2"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.0/24"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1000"},
					//},
				},
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.2.1.2"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.3.2.0/24"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1010"},
					//},
				},
			},
		},
	}
	tc.uProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_dip_subnet",
		},
		Spec: netproto.MirrorSessionSpec{
			//PacketSize: 256, //Modified
			//Enable:     true,
			//CaptureAt:  1, // Modified
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.3"}, // modified
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.0/24"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1000"},
					//},
				},
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.3.1.2"}, // modified
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.4.2.0/24"}, // modified
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1010"},
					//},
				},
			},
		},
	}
	tc.dProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_dip_subnet",
		},
	}
	testCases = append(testCases, tc)

	// case4.2: srcIP, destIPsubnet, ipProto, L4port, 2 rules for matching, one rule removed
	tc.cProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_dip_subnet_rule_removed",
		},
		Spec: netproto.MirrorSessionSpec{
			// PacketSize: 128,
			// Enable:     true,
			// CaptureAt:  0,
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.2"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.0/24"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1000"},
					//},
				},
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.2.1.2"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.3.2.0/24"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1010"},
					//},
				},
			},
		},
	}
	tc.uProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_dip_subnet_rule_removed",
		},
		Spec: netproto.MirrorSessionSpec{
			//PacketSize: 256, //Modified
			//Enable:     true,
			//CaptureAt:  1, // Modified
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.2"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.0/24"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1000"},
					//},
				},
			},
		},
	}
	tc.dProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_dip_subnet_rule_removed",
		},
	}

	testCases = append(testCases, tc)

	// case4.3: srcIP, destIPsubnet, ipProto, L4port, 2 rules for matching, one rule added
	tc.cProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_dip_subnet_rule_added",
		},
		Spec: netproto.MirrorSessionSpec{
			// PacketSize: 128,
			// Enable:     true,
			// CaptureAt:  0,
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.2"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.0/24"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1000"},
					//},
				},
			},
		},
	}
	tc.uProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_dip_subnet_rule_added",
		},
		Spec: netproto.MirrorSessionSpec{
			//PacketSize: 256, //Modified
			//Enable:     true,
			//CaptureAt:  1, // Modified
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.2"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.0/24"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1000"},
					//},
				},
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.2.1.2"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.3.2.0/24"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1010"},
					//},
				},
			},
		},
	}
	tc.dProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_dip_subnet_rule_added",
		},
	}

	testCases = append(testCases, tc)

	return testCases
}

func prepareMirrorSessionFlowRuleSipSubnetTestCases() []mirrorSessionCreateUpdateDeleteProto {
	var testCases []mirrorSessionCreateUpdateDeleteProto
	var tc mirrorSessionCreateUpdateDeleteProto

	// Case 4: Various mirror session test cases where flowRules specify SIP as subnet

	// case4.1: srcIPsubnet, destIP, ipProto, L4port, 2 rules for matching, one rule modified
	tc.cProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_sip_subnet",
		},
		Spec: netproto.MirrorSessionSpec{
			// PacketSize: 128,
			// Enable:     true,
			// CaptureAt:  0,
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.0/24"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.2"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1000"},
					//},
				},
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.2.1.0/24"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.3.2.2"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1010"},
					//},
				},
			},
		},
	}
	tc.uProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_sip_subnet",
		},
		Spec: netproto.MirrorSessionSpec{
			//PacketSize: 256, //Modified
			//Enable:     true,
			//CaptureAt:  1, // Modified
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.0/23"}, //modified
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.2"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1000"},
					//},
				},
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.2.1.0/24"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.3.2.2"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1010"},
					//},
				},
			},
		},
	}
	tc.dProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_sip_subnet",
		},
	}
	testCases = append(testCases, tc)

	// case4.2: srcIPsubnet, destIP, ipProto, L4port, 2 rules for matching, one rule removed
	tc.cProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_sip_subnet_rule_removed",
		},
		Spec: netproto.MirrorSessionSpec{
			// PacketSize: 128,
			// Enable:     true,
			// CaptureAt:  0,
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.0/24"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.2"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1000"},
					//},
				},
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.2.1.0/24"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.3.2.2"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1010"},
					//},
				},
			},
		},
	}
	tc.uProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_sip_subnet_rule_removed",
		},
		Spec: netproto.MirrorSessionSpec{
			//PacketSize: 256, //Modified
			//Enable:     true,
			//CaptureAt:  1, // Modified
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.0/24"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.2"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1000"},
					//},
				},
			},
		},
	}
	tc.dProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_sip_subnet_rule_removed",
		},
	}

	testCases = append(testCases, tc)

	// case4.3: srcIPsubnet, destIP, ipProto, L4port, 2 rules for matching, one rule added
	tc.cProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_sip_subnet_rule_added",
		},
		Spec: netproto.MirrorSessionSpec{
			// PacketSize: 128,
			// Enable:     true,
			// CaptureAt:  0,
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.0/24"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.2"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1000"},
					//},
				},
			},
		},
	}
	tc.uProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_sip_subnet_rule_added",
		},
		Spec: netproto.MirrorSessionSpec{
			//PacketSize: 256, //Modified
			//Enable:     true,
			//CaptureAt:  1, // Modified
			Collectors: []netproto.MirrorCollector{
				{
					//Type: "erspan",
					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.1.1.0/24"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.1.2.2"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1000"},
					//},
				},
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"1.2.1.0/24"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"1.3.2.2"},
					},
					//AppProtoSel: &netproto.AppProtoSelector{
					//	Ports: []string{"TCP/1010"},
					//},
				},
			},
		},
	}
	tc.dProto = &netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "tc_sip_dip_proto_l4port_sip_subnet_rule_added",
		},
	}

	testCases = append(testCases, tc)

	return testCases
}

//func prepareMirrorSessionFlowRuleEPTestCases() []mirrorSessionCreateUpdateDeleteProto {
//	var testCases []mirrorSessionCreateUpdateDeleteProto
//	var tc mirrorSessionCreateUpdateDeleteProto
//
//	// Case 2: Various mirror session test cases where flowRules specify complete IP Address
//
//	// case2.1: srcIPaddr, destIPaddr, ipProto, L4port, 2 rules for matching, both rules modified
//	tc.cProto = &netproto.MirrorSession{
//		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
//		ObjectMeta: api.ObjectMeta{
//			Tenant:    "default",
//			Namespace: "default",
//			Name:      "tc_sEP_dEP_proto_l4port",
//		},
//		Spec: netproto.MirrorSessionSpec{
//			// PacketSize: 128,
//			// Enable:     true,
//			// CaptureAt:  0,
//			Collectors: []netproto.MirrorCollector{
//				{
//					//Type: "erspan",
//					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
//			},
//			MatchRules: []netproto.MatchRule{
//				{
//					Src: &netproto.MatchSelector{
//						Endpoints: []string{"mirrorSession-EP1"},
//					},
//					Dst: &netproto.MatchSelector{
//						Endpoints: []string{"mirrorSession-EP2"},
//					},
//					//AppProtoSel: &netproto.AppProtoSelector{
//					//	Ports: []string{"TCP/1000"},
//					//},
//				},
//				{
//					Src: &netproto.MatchSelector{
//						Endpoints: []string{"mirrorSession-EP3"},
//					},
//					Dst: &netproto.MatchSelector{
//						Endpoints: []string{"mirrorSession-EP4"},
//					},
//					//AppProtoSel: &netproto.AppProtoSelector{
//					//	Ports: []string{"TCP/1010"},
//					//},
//				},
//			},
//		},
//	}
//	tc.uProto = &netproto.MirrorSession{
//		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
//		ObjectMeta: api.ObjectMeta{
//			Tenant:    "default",
//			Namespace: "default",
//			Name:      "tc_sEP_dEP_proto_l4port",
//		},
//		Spec: netproto.MirrorSessionSpec{
//			//PacketSize: 256, //Modified
//			//Enable:     true,
//			//CaptureAt:  1, // Modified
//			Collectors: []netproto.MirrorCollector{
//				{
//					//Type: "erspan",
//					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
//			},
//			MatchRules: []netproto.MatchRule{
//				{
//					Src: &netproto.MatchSelector{
//						Endpoints: []string{"mirrorSession-EP5"}, // modified
//					},
//					Dst: &netproto.MatchSelector{
//						Endpoints: []string{"mirrorSession-EP2"},
//					},
//					//AppProtoSel: &netproto.AppProtoSelector{
//					//	Ports: []string{"TCP/1000"},
//					//},
//				},
//				{
//					Src: &netproto.MatchSelector{
//						Endpoints: []string{"mirrorSession-EP3"},
//					},
//					Dst: &netproto.MatchSelector{
//						Endpoints: []string{"mirrorSession-EP4"},
//					},
//					//AppProtoSel: &netproto.AppProtoSelector{
//					//	Ports: []string{"TCP/1010"},
//					//},
//				},
//			},
//		},
//	}
//	tc.dProto = &netproto.MirrorSession{
//		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
//		ObjectMeta: api.ObjectMeta{
//			Tenant:    "default",
//			Namespace: "default",
//			Name:      "tc_sEP_dEP_proto_l4port",
//		},
//	}
//	testCases = append(testCases, tc)
//	// case2.2: srcIPaddr, destIPaddr, ipProto, L4port, 2 rules for matching, one  rule removed during modification
//	tc.cProto = &netproto.MirrorSession{
//		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
//		ObjectMeta: api.ObjectMeta{
//			Tenant:    "default",
//			Namespace: "default",
//			Name:      "tc_sEP_dEP_proto_l4port_1_rule_removed",
//		},
//		Spec: netproto.MirrorSessionSpec{
//			// PacketSize: 128,
//			// Enable:     true,
//			// CaptureAt:  0,
//			Collectors: []netproto.MirrorCollector{
//				{
//					//Type: "erspan",
//					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
//			},
//			MatchRules: []netproto.MatchRule{
//				{
//					Src: &netproto.MatchSelector{
//						Endpoints: []string{"mirrorSession-EP1"},
//					},
//					Dst: &netproto.MatchSelector{
//						Endpoints: []string{"mirrorSession-EP2"},
//					},
//					//AppProtoSel: &netproto.AppProtoSelector{
//					//	Ports: []string{"TCP/1000"},
//					//},
//				},
//				{
//					Src: &netproto.MatchSelector{
//						Endpoints: []string{"mirrorSession-EP3"},
//					},
//					Dst: &netproto.MatchSelector{
//						Endpoints: []string{"mirrorSession-EP4"},
//					},
//					//AppProtoSel: &netproto.AppProtoSelector{
//					//	Ports: []string{"TCP/1010"},
//					//},
//				},
//			},
//		},
//	}
//	tc.uProto = &netproto.MirrorSession{
//		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
//		ObjectMeta: api.ObjectMeta{
//			Tenant:    "default",
//			Namespace: "default",
//			Name:      "tc_sEP_dEP_proto_l4port_1_rule_removed",
//		},
//		Spec: netproto.MirrorSessionSpec{
//			//PacketSize: 256, //Modified
//			//Enable:     true,
//			//CaptureAt:  1, // Modified
//			Collectors: []netproto.MirrorCollector{
//				{
//					//Type: "erspan",
//					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
//			},
//			MatchRules: []netproto.MatchRule{
//				{
//					Src: &netproto.MatchSelector{
//						Endpoints: []string{"mirrorSession-EP1"},
//					},
//					Dst: &netproto.MatchSelector{
//						Endpoints: []string{"mirrorSession-EP2"},
//					},
//					//AppProtoSel: &netproto.AppProtoSelector{
//					//	Ports: []string{"TCP/1000"},
//					//},
//				},
//				// one rule removed
//			},
//		},
//	}
//	tc.dProto = &netproto.MirrorSession{
//		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
//		ObjectMeta: api.ObjectMeta{
//			Tenant:    "default",
//			Namespace: "default",
//			Name:      "tc_sEP_dEP_proto_l4port_1_rule_removed",
//		},
//	}
//	testCases = append(testCases, tc)
//	// case2.3: srcIPaddr, destIPaddr, ipProto, L4port, 1 rules for matching, one more rule added during modification
//	tc.cProto = &netproto.MirrorSession{
//		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
//		ObjectMeta: api.ObjectMeta{
//			Tenant:    "default",
//			Namespace: "default",
//			Name:      "tc_sEP_dEP_proto_l4port_1_rule_added",
//		},
//		Spec: netproto.MirrorSessionSpec{
//			// PacketSize: 128,
//			// Enable:     true,
//			// CaptureAt:  0,
//			Collectors: []netproto.MirrorCollector{
//				{
//					//Type: "erspan",
//					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
//			},
//			MatchRules: []netproto.MatchRule{
//				{
//					Src: &netproto.MatchSelector{
//						Endpoints: []string{"mirrorSession-EP1"},
//					},
//					Dst: &netproto.MatchSelector{
//						Endpoints: []string{"mirrorSession-EP2"},
//					},
//					//AppProtoSel: &netproto.AppProtoSelector{
//					//	Ports: []string{"TCP/1000"},
//					//},
//				},
//			},
//		},
//	}
//	tc.uProto = &netproto.MirrorSession{
//		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
//		ObjectMeta: api.ObjectMeta{
//			Tenant:    "default",
//			Namespace: "default",
//			Name:      "tc_sEP_dEP_proto_l4port_1_rule_added",
//		},
//		Spec: netproto.MirrorSessionSpec{
//			//PacketSize: 256, //Modified
//			//Enable:     true,
//			//CaptureAt:  1, // Modified
//			Collectors: []netproto.MirrorCollector{
//				{
//					//Type: "erspan",
//					ExportCfg: netproto.MirrorExportConfig{Destination: "10.10.10.1"}},
//			},
//			MatchRules: []netproto.MatchRule{
//				{
//					Src: &netproto.MatchSelector{
//						Endpoints: []string{"mirrorSession-EP1"},
//					},
//					Dst: &netproto.MatchSelector{
//						Endpoints: []string{"mirrorSession-EP2"},
//					},
//					//AppProtoSel: &netproto.AppProtoSelector{
//					//	Ports: []string{"TCP/1000"},
//					//},
//				},
//				// rule added
//				{
//					Src: &netproto.MatchSelector{
//						Endpoints: []string{"mirrorSession-EP3"},
//					},
//					Dst: &netproto.MatchSelector{
//						Endpoints: []string{"mirrorSession-EP4"},
//					},
//					//AppProtoSel: &netproto.AppProtoSelector{
//					//	Ports: []string{"TCP/1010"},
//					//},
//				},
//			},
//		},
//	}
//	tc.dProto = &netproto.MirrorSession{
//		TypeMeta: api.TypeMeta{Kind: "MirrorSession"},
//		ObjectMeta: api.ObjectMeta{
//			Tenant:    "default",
//			Namespace: "default",
//			Name:      "tc_sEP_dEP_proto_l4port_1_rule_added",
//		},
//	}
//	testCases = append(testCases, tc)
//
//	return testCases
//}

func prepareMirrorSessionTestCases() []mirrorSessionCreateUpdateDeleteProto {
	var testCases []mirrorSessionCreateUpdateDeleteProto

	// DropReason Tests
	// Case 1: Various mirror session using drop rules
	testCases = append(testCases, prepareMirrorSessionDropTestCases()...)

	// Match Rule using IPaddress/subnet/Range Tests
	// Case 2: srcIP, destIP, ipProto, L4port
	testCases = append(testCases, prepareMirrorSessionFlowRuleIPAddrTestCases()...)
	// case 3: srcIPsubnet, destIPsubnet, ipProto, L4port
	testCases = append(testCases, prepareMirrorSessionFlowRuleIPSubnetTestCases()...)
	// case 4: srcIP, destIPsubnet, ipProto, L4port
	testCases = append(testCases, prepareMirrorSessionFlowRuleDipSubnetTestCases()...)
	// case 5: srcIPsubnet, destIP, ipProto, L4port
	testCases = append(testCases, prepareMirrorSessionFlowRuleSipSubnetTestCases()...)
	// Match Rule using EP name Tests
	// Case 6: srcEP, destEP, ipProto, L4port
	//testCases = append(testCases, prepareMirrorSessionFlowRuleEPTestCases()...)

	// case 8: srcIPrange, destIPrange, ipProto, L4port
	// case 9: srcIPrange, destIP, ipProto, L4port
	// case 10: srcIP, destIPrange, ipProto, L4port

	// Match Rule using MacAddress Tests
	// case 11: srcMAC, destMAC, ether-type

	return testCases
}

func testMirrorSessionCreateUpdateDelete(baseURL string) error {
	var err error
	mirrorURL := fmt.Sprintf("http://%s/api/mirror/sessions", baseURL)

	/* when mirror session test cases depend on other objects,
	 * create then before starting mirror session test cases.
	 */
	err = setupMirrorSessionDependenctObjects(baseURL)
	if err != nil {
		fmt.Println("MirrorSession Setup failed")
		return err
	}
	tcCases := prepareMirrorSessionTestCases()
	for _, tc := range tcCases {
		// create
		fmt.Printf("Create TestMirrorSession %s .......", tc.cProto.Name)
		err = postMirorrSessions(mirrorURL, tc.cProto)
		if err != nil {
			fmt.Printf("Failed\n")
			break
		}
		fmt.Printf("Passed\n")
		// update
		fmt.Printf("Update TestMirrorSession %s ......", tc.cProto.Name)
		err = putMirorrSessions(mirrorURL, tc.uProto)
		if err != nil {
			fmt.Printf("Failed\n")
			break
		}
		fmt.Printf("Passed\n")
		// delete
		fmt.Printf("Delete TestMirrorSession %s ......", tc.cProto.Name)
		err = deleteMirorrSessions(mirrorURL, tc.dProto)
		// This test doesn't validate for the presence of tunnel. Ignoring tunnel not found error.
		// TODO fix these tests to actually validate with lateral objects
		if err != nil && !strings.Contains(err.Error(), "tunnel not found") {
			fmt.Printf("Failed\n")
			break
		}
		fmt.Printf("Passed\n")
	}
	/* All mirror session test cases depend objects that were,
	 * created delete them.
	 */
	tearDownMirrorSessionDependenctObjects(baseURL)
	return err
}

func testMirrorSessions(baseURL string) error {
	err := testMirrorSessionCreateUpdateDelete(baseURL)
	if err != nil {
		fmt.Println("TestMirrorSession................Failed")
	} else {
		fmt.Println("TestMirrorSession................Pass.")
	}
	return err
}

func TestMirrorSessionTestCases(t *testing.T) {
	t.Parallel()
	err := testMirrorSessions(agentRestURL)
	AssertOk(t, err, "MirrorSesion test cases failed")

}
