package bulkedit

import (
	"encoding/json"
	"reflect"
	"testing"

	"github.com/gogo/protobuf/types"

	api "github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/network"
)

func TestBulkeditActionUnmarshalJSON(t *testing.T) {

	netw := &network.Network{
		TypeMeta: api.TypeMeta{
			Kind:       "Network",
			APIVersion: "v1",
		},
		ObjectMeta: api.ObjectMeta{
			Name:   "TestNtwork1",
			Tenant: "default",
		},
		Spec: network.NetworkSpec{
			Type:        network.NetworkType_Bridged.String(),
			IPv4Subnet:  "10.1.1.1/24",
			IPv4Gateway: "10.1.1.1",
		},
		Status: network.NetworkStatus{},
	}

	p, _ := types.MarshalAny(netw)

	bEdAction := BulkEditItem{
		Method: BulkEditItem_CREATE.String(),
		Object: &api.Any{Any: *p},
	}

	bedBytes, err := json.Marshal(bEdAction)
	if err != nil {
		t.Fatalf("Json Marshal failed, Err is %s\n", err.Error())
	}

	b1 := &BulkEditItem{}

	err = b1.UnmarshalJSON(bedBytes)
	if err != nil {
		t.Fatalf("Failed to UnmarshalJSON: %s", err.Error())
	}

	netw2 := &network.Network{}

	err = types.UnmarshalAny(&b1.GetObject().Any, netw2)
	if err != nil {
		t.Fatalf("failed to UnmarshalAny %s\n", err.Error())
	}

	if !reflect.DeepEqual(netw, netw2) {
		t.Fatalf("Networks not equal\n%v\n%v", netw, netw2)
	}

}

func TestBulkeditActionUnmarshalJSON2(t *testing.T) {

	text := "\n    \t\t{\n            \"method\":\"Create\",\n            \"object\": {\n            \t\"kind\":\"Network\",\n            \t\"api-version\":\"v1\",\n            \t\"meta\": {\n            \t\t\"name\":\"e2eNetwork30007\",\n            \t\t\"tenant\":\"default\",\n            \t\t\"namespace\":\"default\",\n            \t\t\"generation-id\":\"\",\n            \t\t\"creation-time\":\"\",\n            \t\t\"mod-time\":\"\"\n            \t},\n            \t\"spec\": {\n            \t\t\"type\":\"bridged\",\n            \t\t\"ipv4-subnet\":\"10.1.1.1/24\",\n            \t\t\"ipv4-gateway\":\"10.1.1.1\"\n            \t},\n            \t\"status\":{}\n            }\n            }\n        \n    "

	p := &BulkEditItem{}
	err := p.UnmarshalJSON([]byte(text))
	if err != nil {
		t.Fatalf("Unmarshal Json Failed! %s\n", err.Error())
	}

	// Negative test cases
	// 1. Send junk text

	err = p.UnmarshalJSON([]byte("abcdefghi123456"))
	if err == nil {
		t.Fatalf("Expect json Unmarshal err!\n")
	}

	// 2. Send text without method field

	text = "\n    \t\t{\n            \n            \"object\": {\n            \t\"kind\":\"Network\",\n            \t\"api-version\":\"v1\",\n            \t\"meta\": {\n            \t\t\"name\":\"e2eNetwork30007\",\n            \t\t\"tenant\":\"default\",\n            \t\t\"namespace\":\"default\",\n            \t\t\"generation-id\":\"\",\n            \t\t\"creation-time\":\"\",\n            \t\t\"mod-time\":\"\"\n            \t},\n            \t\"spec\": {\n            \t\t\"type\":\"bridged\",\n            \t\t\"ipv4-subnet\":\"10.1.1.1/24\",\n            \t\t\"ipv4-gateway\":\"10.1.1.1\"\n            \t},\n            \t\"status\":{}\n            }\n            }\n        \n    "
	err = p.UnmarshalJSON([]byte(text))
	if err == nil {
		t.Fatalf("Expected Item missing method type error\n")
	}

	// 3. Send text without object type

	text = "\n    \t\t{\n            \"method\":\"Create\",\n            \n            \t\"kind\":\"Network\",\n            \t\"api-version\":\"v1\",\n            \t\"meta\": {\n            \t\t\"name\":\"e2eNetwork30007\",\n            \t\t\"tenant\":\"default\",\n            \t\t\"namespace\":\"default\",\n            \t\t\"generation-id\":\"\",\n            \t\t\"creation-time\":\"\",\n            \t\t\"mod-time\":\"\"\n            \t},\n            \t\"spec\": {\n            \t\t\"type\":\"bridged\",\n            \t\t\"ipv4-subnet\":\"10.1.1.1/24\",\n            \t\t\"ipv4-gateway\":\"10.1.1.1\"\n            \t},\n            \t\"status\":{}\n            \n            }\n        \n    "
	err = p.UnmarshalJSON([]byte(text))
	if err == nil {
		t.Fatalf("Expected Item missing object type error\n")
	}

	// 4. Missing Kind type
	text = "\n    \t\t{\n            \"method\":\"Create\",\n            \"object\": {\n            \t\n            \t\"api-version\":\"v1\",\n            \t\"meta\": {\n            \t\t\"name\":\"e2eNetwork30007\",\n            \t\t\"tenant\":\"default\",\n            \t\t\"namespace\":\"default\",\n            \t\t\"generation-id\":\"\",\n            \t\t\"creation-time\":\"\",\n            \t\t\"mod-time\":\"\"\n            \t},\n            \t\"spec\": {\n            \t\t\"type\":\"bridged\",\n            \t\t\"ipv4-subnet\":\"10.1.1.1/24\",\n            \t\t\"ipv4-gateway\":\"10.1.1.1\"\n            \t},\n            \t\"status\":{}\n            }\n            }\n        \n    "
	err = p.UnmarshalJSON([]byte(text))
	if err == nil {
		t.Fatalf("Expected Item missing kind type error\n")
	}
}

func TestFetchObjectFromBulkEditItem(t *testing.T) {

	netw := &network.Network{
		TypeMeta: api.TypeMeta{
			Kind:       "Network",
			APIVersion: "v1",
		},
		ObjectMeta: api.ObjectMeta{
			Name:   "TestNtwork1",
			Tenant: "default",
		},
		Spec: network.NetworkSpec{
			Type:        network.NetworkType_Bridged.String(),
			IPv4Subnet:  "10.1.1.1/24",
			IPv4Gateway: "10.1.1.1",
		},
		Status: network.NetworkStatus{},
	}

	p, _ := types.MarshalAny(netw)

	bEditAction := BulkEditItem{
		Method: BulkEditItem_CREATE.String(),
		Object: &api.Any{Any: *p},
	}

	kind, group, obj, err := bEditAction.FetchObjectFromBulkEditItem()
	if err != nil {
		t.Fatalf("Error from FetchObjectFromBulkEditItem() %v\n", err.Error())
	}
	if kind != netw.TypeMeta.GetKind() {
		t.Fatalf("Invalid Kind, expected %s, actual %s\n", netw.TypeMeta.GetKind(), kind)
	}

	if group != "network" {
		t.Fatalf("Invalid group, expected %s, actual %s\n", "network", kind)
	}

	netwObj := obj.(*network.Network)
	if reflect.DeepEqual(netw, netwObj) != true {
		t.Fatalf("Received Object is not equal to expected network object\n")
	}

}
