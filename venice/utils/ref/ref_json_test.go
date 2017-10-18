package ref

import (
	"fmt"
	"sort"
	"strconv"
	"testing"
)

func TestWalkStructJson(t *testing.T) {
	expectedStr := `{
  TypeMeta: {
    kind: string
    apiVersion: string
  }
  meta: {
    tenant: string
    name: string
    namespace: string
    resourceVersion: string
    uuid: string
    labels: map[string]string
  }
  spec: {
    aliases: string
    roles: []string
    dummyRoles: []string
    matchLabels: map[string]string
    attachGroup: string
    igRules: []{
      ports: string
      action: string
      sourceGroup: string
    }
    egRules: []{
      ports: string
      action: string
      sourceGroup: string
    }
    interval: int
    skippedField: string
    *perm: {
      toObj: string
      rwx: string
    }
    policiesMap: map[string]struct{
      toGroup: string
      fromGroup: string
    }
    userHandle: uint64
    uint32Field: uint32
    int32Field: int32
    *sipAddress: string
    macAddrs: []*string
    fwProfiles: []*{
      ipVersion: string
      protocol: string
    }
    igRulesR: []*{
      ports: string
      action: string
      sourceGroup: string
    }
    egRulesR: []*{
      ports: string
      action: string
      sourceGroup: string
    }
    fixedRule: [2]{
      ports: string
      action: string
      sourceGroup: string
    }
    {
      ports: string
      action: string
      sourceGroup: string
    }
    nodeRoles: []int32
    conditions: []*{
      type: int32
      status: int32
      lastTransitionTime: int64
      reason: string
      message: string
    }
    boolFlag: bool
    floatVal: float64
    allocated-ipv4-addrs: []uint8
  }
}
`
	refCtx := &RfCtx{GetSubObj: subObj, UseJSONTag: true}
	outStr := WalkStruct(User{}, refCtx)
	if outStr != expectedStr {
		t.Fatalf("Out:\n--%s--\nExpected:\n--%s--\n", outStr, expectedStr)
	}
}

func TestEmptyGetJson(t *testing.T) {
	refCtx := &RfCtx{GetSubObj: subObj, UseJSONTag: true}
	kvs := make(map[string]FInfo)
	GetKvs(User{}, refCtx, kvs)
	total := 0

	if _, ok := kvs["apiVersion"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("apiVersion not found in kvs")
	}
	total++
	if _, ok := kvs["resourceVersion"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("resourceVersion not found in kvs")
	}
	total++
	if _, ok := kvs["tenant"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("Tenant not found in kvs")
	}
	total++
	if _, ok := kvs["namespace"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("namespace not found in kvs")
	}
	total++
	if _, ok := kvs["kind"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("kind not found in kvs")
	}
	total++
	if _, ok := kvs["name"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("name not found in kvs")
	}
	total++
	if _, ok := kvs["uuid"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("uuid not found in kvs")
	}
	total++
	if _, ok := kvs["labels"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("labels not found in kvs")
	}
	total++
	if _, ok := kvs["aliases"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("aliases not found in kvs")
	}
	total++
	if _, ok := kvs["roles"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("roles not found in kvs")
	}
	total++
	if _, ok := kvs["dummyRoles"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("dummyRoles not found in kvs")
	}
	total++
	if _, ok := kvs["matchLabels"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("matchLabels not found in kvs")
	}
	total++
	if _, ok := kvs["attachGroup"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("attachGroup not found in kvs")
	}
	total++
	if _, ok := kvs["in_port"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("in_port not found in kvs")
	}
	total++
	if _, ok := kvs["out_port"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("out_port not found in kvs")
	}
	total++
	if _, ok := kvs["in_action"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("in_action not found in kvs")
	}
	total++
	if _, ok := kvs["out_action"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("out_action not found in kvs")
	}
	total++
	if _, ok := kvs["in_peer_group"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("in_peer_group not found in kvs")
	}
	total++
	if _, ok := kvs["out_peer_group"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("out_peer_group not found in kvs")
	}
	total++
	if v, ok := kvs["interval"]; !ok || v.TypeStr != "int" {
		printKvs("kvs", kvs, true)
		t.Fatalf("interval not found in kvs")
	}
	total++
	if _, ok := kvs["skippedField"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("skippedField not found in kvs")
	}
	total++
	if _, ok := kvs["toObj"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("toObj not found in kvs")
	}
	total++
	if _, ok := kvs["rwx"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("rwx not found in kvs")
	}
	total++
	if _, ok := kvs["toGroup"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("toGroup not found in kvs")
	}
	total++
	if _, ok := kvs["fromGroup"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("fromGroup not found in kvs")
	}
	total++
	if _, ok := kvs["userHandle"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("userHandle not found in kvs")
	}
	total++
	if v, ok := kvs["uint32Field"]; !ok || v.TypeStr != "uint32" {
		printKvs("kvs", kvs, true)
		t.Fatalf("uint32Field not found in kvs")
	}
	total++
	if v, ok := kvs["int32Field"]; !ok || v.TypeStr != "int32" {
		printKvs("kvs", kvs, true)
		t.Fatalf("int32Field not found in kvs")
	}
	total++
	if _, ok := kvs["sipAddress"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("sipAddress not found in kvs")
	}
	total++
	if _, ok := kvs["macAddrs"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("macAddrs not found in kvs")
	}
	total++
	if _, ok := kvs["ip_ver"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("ip_ver not found in kvs")
	}
	total++
	if _, ok := kvs["ip_proto"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("ip_proto not found in kvs")
	}
	total++
	if _, ok := kvs["inr_port"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("inr_port not found in kvs")
	}
	total++
	if _, ok := kvs["outr_port"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("outr_port not found in kvs")
	}
	total++
	if _, ok := kvs["inr_action"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("inr_action not found in kvs")
	}
	total++
	if _, ok := kvs["outr_action"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("outr_action not found in kvs")
	}
	total++
	if _, ok := kvs["inr_peer_group"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("inr_peer_group not found in kvs")
	}
	total++
	if _, ok := kvs["outr_peer_group"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("outr_peer_group not found in kvs")
	}
	total++
	if _, ok := kvs["fix_action"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("fix_action not found in kvs")
	}
	total++
	if _, ok := kvs["fix_peer_group"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("fix_peer_group not found in kvs")
	}
	total++
	if _, ok := kvs["fix_peer_group"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("fix_peer_group not found in kvs")
	}
	total++
	if _, ok := kvs["nodeRoles"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("nodeRoles not found in kvs")
	}
	total++
	if _, ok := kvs["type"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("type not found in kvs")
	}
	total++
	if _, ok := kvs["status"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("status not found in kvs")
	}
	total++
	if _, ok := kvs["lastTransitionTime"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("lastTransitionTime not found in kvs")
	}
	total++
	if _, ok := kvs["reason"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("reason not found in kvs")
	}
	total++
	if _, ok := kvs["message"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("message not found in kvs")
	}
	total++
	if _, ok := kvs["boolFlag"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("boolFlag not found in kvs")
	}
	total++
	if _, ok := kvs["floatVal"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("floatVal not found in kvs")
	}
	total++
	if _, ok := kvs["allocated-ipv4-addrs"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("allocated-ipv4-addrs not found in kvs")
	}
	total++

	if len(kvs) != total {
		printKvs("kvs", kvs, true)
		t.Fatalf("unexpected number of kv pairs %d found %d", len(kvs), total)
	}
}

func TestGetJson(t *testing.T) {
	ip1 := "131.22.1.1"
	mac1 := "00.11.22.33.44.55"
	mac2 := "00.22.33.44.55.66"
	ipOpt1 := IPOpt{Version: "v4", Protocol: "tcp"}
	ipOpt2 := IPOpt{Version: "v6", Protocol: "udp"}
	inr1 := SGRule{Ports: "tcp/221", PeerGroup: "g2", Action: "permittt"}
	inr2 := SGRule{Ports: "tcp/222", PeerGroup: "g1", Action: "permittt"}
	outr1 := SGRule{Ports: "udp/221", PeerGroup: "g3", Action: "dennny"}
	cond1 := NodeCondition{Type: 1, Status: 2, LastTransitionTime: 90001, Reason: "some reason", Message: "some message"}
	cond2 := NodeCondition{Type: 2, Status: 0, LastTransitionTime: 80001, Reason: "some other reason", Message: "some other message"}
	byteArray := []byte("EIIUSXN!@")

	u := User{
		TypeMeta:   TypeMeta{Kind: "user"},
		ObjectMeta: ObjectMeta{Name: "joe", Tenant: "pices", Labels: map[string]string{"dept": "eng", "level": "mts"}},
		Spec: UserSpec{
			Aliases:     "jhonny",
			Roles:       []string{"storage-admin", "security-admin"},
			MatchLabels: map[string]string{"io.pensando.area": "network", "color": "purple"},
			AttachGroup: "app-sg",
			InRules: []SGRule{
				{Ports: "tcp/80,tcp/443", Action: "permit,log", PeerGroup: "web-sg"},
				{Ports: "tcp/844", Action: "permit", PeerGroup: "db-sg"}},
			OutRules: []SGRule{
				{Ports: "tcp/6379", Action: "permit,log", PeerGroup: "db-sg"}},
			Interval: 33,
			Perms:    &Permission{ToObj: "network", RWX: "rw"},
			Policies: map[string]Policy{"key1": {ToGroup: "to-key1", FromGroup: "from-key1"},
				"key2": {ToGroup: "to-key2", FromGroup: "from-key2"}},
			UserHandle:  0x45544422,
			Uint32Field: 9821,
			Int32Field:  8832,
			SIPAddress:  &ip1,
			MacAddrs:    []*string{&mac1, &mac2},
			IPOpts:      []*IPOpt{&ipOpt1, &ipOpt2},
			InRulesR:    []*SGRule{&inr1, &inr2},
			OutRulesR:   []*SGRule{&outr1},
			FixedRules: [2]SGRule{
				{Ports: "tcp/80,tcp/443", Action: "permit,log", PeerGroup: "web-sg"},
				{Ports: "udp/80", Action: "permit,log", PeerGroup: "app-sg"},
			},
			NodeRoles:          []NodeSpecNodeRole{997, 799},
			Conditions:         []*NodeCondition{&cond1, &cond2},
			BoolFlag:           true,
			FloatVal:           77.983,
			AllocatedIPv4Addrs: byteArray,
		},
	}
	kvs := make(map[string]FInfo)
	refCtx := &RfCtx{GetSubObj: subObj, UseJSONTag: true}
	GetKvs(u, refCtx, kvs)

	if fi, ok := kvs["name"]; ok {
		if !fi.Key || fi.ValueStr[0] != "joe" {
			printKvs("Meta", kvs, false)
			fmt.Printf("fi = %+v\n", fi)
			t.Fatalf("error! name not set correctly")
		}
	} else {
		t.Fatalf("Name not found in kvs")
	}

	if fi, ok := kvs["tenant"]; ok {
		if !fi.Key || fi.ValueStr[0] != "pices" {
			printKvs("Meta", kvs, false)
			fmt.Printf("fi = %+v\n", fi)
			t.Fatalf("error! tenant not set correctly")
		}
	} else {
		t.Fatalf("tenant not found in kvs")
	}

	if fi, ok := kvs["labels"]; ok {
		sort.Strings(fi.ValueStr)
		if fi.Key || fi.ValueStr[0] != "dept:eng" && fi.ValueStr[1] != "level:mts" {
			printKvs("Meta", kvs, false)
			t.Fatalf("error! value not set correctly %+v", fi)
		}
	} else {
		t.Fatalf("labels not found in kvs")
	}

	if fi, ok := kvs["aliases"]; ok {
		if fi.Key || fi.ValueStr[0] != "jhonny" {
			t.Fatalf("error! value is not set correctly")
		}
	} else {
		t.Fatalf("error finding key")
	}

	if fi, ok := kvs["roles"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "storage-admin" || fi.ValueStr[1] != "security-admin" {
			printKvs("Spec", kvs, false)
			t.Fatalf("error! value not set correctly")
		}
	} else {
		t.Fatalf("roles not found in kvs")
	}

	if fi, ok := kvs["matchLabels"]; ok {
		sort.Strings(fi.ValueStr)
		if fi.Key || fi.ValueStr[0] != "color:purple" || fi.ValueStr[1] != "io.pensando.area:network" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! labels fetched %+v  not set correctly", fi)
		}
	} else {
		t.Fatalf("matchLabels not found in kvs")
	}

	if fi, ok := kvs["in_port"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "tcp/80,tcp/443" || fi.ValueStr[1] != "tcp/844" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! in_port fetched '%s' not set correctly", fi.ValueStr)
		}
		if fi.TypeStr != "slice" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! in_port type set to '%s'\n", fi.TypeStr)
		}

	} else {
		printKvs("spec", kvs, false)
		t.Fatalf("in_port not found")
	}

	if fi, ok := kvs["in_action"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "permit,log" || fi.ValueStr[1] != "permit" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! in_action fetched '%s' not set correctly", fi.ValueStr)
		}
	} else {
		printKvs("spec", kvs, false)
		t.Fatalf("in_action not found, kvs %+v ", kvs)
	}

	if fi, ok := kvs["in_peer_group"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "web-sg" || fi.ValueStr[1] != "db-sg" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! Peer Group fetched '%s' not set correctly", fi.ValueStr)
		}
	} else {
		printKvs("spec", kvs, false)
		t.Fatalf("in_peer_group not found")
	}

	if fi, ok := kvs["out_port"]; ok {
		if fi.Key || len(fi.ValueStr) != 1 || fi.ValueStr[0] != "tcp/6379" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! out_port fetched '%s' not set correctly", fi.ValueStr)
		}
	} else {
		printKvs("spec", kvs, false)
		t.Fatalf("out_port not found")
	}

	if fi, ok := kvs["out_peer_group"]; ok {
		if fi.Key || len(fi.ValueStr) != 1 || fi.ValueStr[0] != "db-sg" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! Peer Group fetched '%s' not set correctly", fi.ValueStr)
		}
	} else {
		printKvs("spec", kvs, false)
		t.Fatalf("out_peer_group not found")
	}

	if fi, ok := kvs["interval"]; ok {
		if v, err := strconv.Atoi(fi.ValueStr[0]); err != nil || fi.Key || v != 33 {
			printKvs("spec", kvs, false)
			t.Fatalf("error! Interval '%v' not set correctly", fi.ValueStr)
		}
	}

	if fi, ok := kvs["skippedField"]; ok {
		if fi.Key || !fi.SSkip {
			printKvs("spec", kvs, false)
			t.Fatalf("error! not identified SkippedFiled '%v'", fi)
		}
	}

	if fi, ok := kvs["toObj"]; ok {
		if fi.Key || len(fi.ValueStr) != 1 || fi.ValueStr[0] != "network" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! ToObj from pointed struct not found '%v'", fi.ValueStr)
		}
	} else {
		t.Fatalf("toObj not found")
	}

	if fi, ok := kvs["rwx"]; ok {
		if fi.Key || len(fi.ValueStr) != 1 || fi.ValueStr[0] != "rw" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! ToObj from pointed struct not found '%v'", fi.ValueStr)
		}
	} else {
		t.Fatalf("rwx not found")
	}

	if fi, ok := kvs["toGroup"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 ||
			(fi.ValueStr[0] != "key2:to-key2" && fi.ValueStr[0] != "key1:to-key1") ||
			(fi.ValueStr[1] != "key2:to-key2" && fi.ValueStr[1] != "key1:to-key1") ||
			(fi.ValueStr[0] == fi.ValueStr[1]) {
			printKvs("spec", kvs, false)
			t.Fatalf("error! ToGroup in the map not found '%v'", fi.ValueStr)
		}
	} else {
		t.Fatalf("toGroup not found")
	}

	if fi, ok := kvs["fromGroup"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 ||
			(fi.ValueStr[0] != "key2:from-key2" && fi.ValueStr[0] != "key1:from-key1") ||
			(fi.ValueStr[1] != "key2:from-key2" && fi.ValueStr[1] != "key1:from-key1") ||
			(fi.ValueStr[0] == fi.ValueStr[1]) {
			printKvs("spec", kvs, false)
			t.Fatalf("error! FromGroup in the map not found '%v'", fi.ValueStr)
		}
	} else {
		t.Fatalf("fromGroup not found")
	}

	if fi, ok := kvs["userHandle"]; ok {
		handle, _ := strconv.ParseUint(fi.ValueStr[0], 10, 64)
		if fi.Key || len(fi.ValueStr) != 1 || handle != 0x45544422 {
			printKvs("spec", kvs, false)
			t.Fatalf("error! handle %v not found '%v'", handle, fi.ValueStr)
		}
	} else {
		t.Fatalf("userHandle not found")
	}

	if fi, ok := kvs["int32Field"]; ok {
		handle, _ := strconv.Atoi(fi.ValueStr[0])
		if fi.Key || len(fi.ValueStr) != 1 || handle != 8832 {
			printKvs("spec", kvs, false)
			t.Fatalf("error! Int32Field %d not found '%v'", handle, fi.ValueStr)
		}
	} else {
		t.Fatalf("int32Field not found")
	}

	if fi, ok := kvs["uint32Field"]; ok {
		handle, _ := strconv.ParseUint(fi.ValueStr[0], 10, 32)
		if fi.Key || len(fi.ValueStr) != 1 || handle != 9821 {
			printKvs("spec", kvs, false)
			t.Fatalf("error! Uint32Field %v not found '%v'", handle, fi.ValueStr)
		}
	} else {
		t.Fatalf("uint32Field not found")
	}

	if fi, ok := kvs["sipAddress"]; ok {
		if fi.Key || !fi.SSkip || len(fi.ValueStr) != 1 || fi.ValueStr[0] != ip1 {
			printKvs("spec", kvs, false)
			t.Fatalf("error! SIPAddress %v not found '%v'", ip1, fi.ValueStr)
		}
	} else {
		t.Fatalf("sipAddress not found")
	}

	if fi, ok := kvs["macAddrs"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != mac1 && fi.ValueStr[1] != mac2 {
			printKvs("spec", kvs, false)
			t.Fatalf("error! MacAddrs %s,%s not found '%v'", mac1, mac2, fi.ValueStr)
		}
	} else {
		t.Fatalf("macAddrs not found")
	}

	if fi, ok := kvs["ip_ver"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "v4" && fi.ValueStr[1] != "v6" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! ip_ver %s,%s not found '%v'", ipOpt1, ipOpt2, fi.ValueStr)
		}
	} else {
		t.Fatalf("ip_ver not found")
	}

	if fi, ok := kvs["inr_port"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "tcp/221" || fi.ValueStr[1] != "tcp/222" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! inr_port %s,%s not found '%v'", inr1, inr2, fi.ValueStr)
		}
	} else {
		t.Fatalf("inr_port not found")
	}
	if fi, ok := kvs["inr_peer_group"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "g2" || fi.ValueStr[1] != "g1" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! inr_peer_group %s,%s not found '%v'", inr1, inr2, fi.ValueStr)
		}
	} else {
		t.Fatalf("inr_peer_group not found")
	}

	if fi, ok := kvs["inr_action"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "permittt" || fi.ValueStr[1] != "permittt" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! inr_action %s,%s not found '%v'", inr1, inr2, fi.ValueStr)
		}
	} else {
		t.Fatalf("inr_action not found")
	}

	if fi, ok := kvs["outr_port"]; ok {
		if fi.Key || len(fi.ValueStr) != 1 || fi.ValueStr[0] != "udp/221" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! outr_port %s, not found '%v'", outr1, fi.ValueStr)
		}
	} else {
		t.Fatalf("outr_port not found")
	}

	if fi, ok := kvs["outr_peer_group"]; ok {
		if fi.Key || len(fi.ValueStr) != 1 || fi.ValueStr[0] != "g3" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! outr_peer_group %s, not found '%v'", outr1, fi.ValueStr)
		}
	} else {
		t.Fatalf("outr_peer_group not found")
	}

	if fi, ok := kvs["outr_action"]; ok {
		if fi.Key || len(fi.ValueStr) != 1 || fi.ValueStr[0] != "dennny" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! outr_action %s not found '%v'", outr1, fi.ValueStr)
		}
	} else {
		t.Fatalf("outr_action not found")
	}

	if fi, ok := kvs["fix_port"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "tcp/80,tcp/443" || fi.ValueStr[1] != "udp/80" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! fix_port not found '%v'", fi.ValueStr)
		}
	} else {
		t.Fatalf("fix_port not found")
	}

	if fi, ok := kvs["fix_peer_group"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "web-sg" || fi.ValueStr[1] != "app-sg" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! fix_peer_group not found '%v'", fi.ValueStr)
		}
	} else {
		t.Fatalf("fix_peer_group not found")
	}

	if fi, ok := kvs["fix_action"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "permit,log" || fi.ValueStr[1] != "permit,log" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! fix_action not found '%v'", fi.ValueStr)
		}
	} else {
		t.Fatalf("fix_action not found")
	}

	if fi, ok := kvs["nodeRoles"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "997" || fi.ValueStr[1] != "799" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! NodeRoles not found '%v'", fi.ValueStr)
		}
	} else {
		t.Fatalf("nodeRoles not found")
	}

	if fi, ok := kvs["type"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "1" || fi.ValueStr[1] != "2" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! Condition Type not found '%v'", fi.ValueStr)
		}
	} else {
		t.Fatalf("condition Type not found")
	}
	if fi, ok := kvs["status"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "2" || fi.ValueStr[1] != "0" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! Condition Status not found '%v'", fi.ValueStr)
		}
	} else {
		t.Fatalf("condition Status not found")
	}
	if fi, ok := kvs["lastTransitionTime"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "90001" || fi.ValueStr[1] != "80001" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! Condition LastTransitionTime not found '%v'", fi.ValueStr)
		}
	} else {
		t.Fatalf("Condition lastTransitionTime not found")
	}
	if fi, ok := kvs["reason"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "some reason" || fi.ValueStr[1] != "some other reason" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! Condition Reason not found '%v'", fi.ValueStr)
		}
	} else {
		t.Fatalf("condition Reason not found")
	}

	if fi, ok := kvs["boolFlag"]; ok {
		if fi.Key || len(fi.ValueStr) != 1 || fi.ValueStr[0] != "true" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! Condition Reason not found '%v'", fi.ValueStr)
		}
	} else {
		t.Fatalf("boolFlag not found")
	}

	if fi, ok := kvs["floatVal"]; ok {
		if fi.Key || len(fi.ValueStr) != 1 || fi.ValueStr[0] != "77.983" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! FloatVal not found '%v'", fi.ValueStr)
		}
	} else {
		t.Fatalf("floatVal not found")
	}

	if fi, ok := kvs["allocated-ipv4-addrs"]; ok {
		if !fi.SSkip || fi.Key || len(fi.ValueStr) != 9 {
			printKvs("spec", kvs, false)
			t.Fatalf("error! Invalid allocated-ipv4-addrs found '%+v'", fi)
		}
	} else {
		t.Fatalf("allocated-ipv4-addrs not found")
	}
}

func TestUpdateJson(t *testing.T) {
	ip1 := "131.22.1.1"
	mac1 := "10.11.22.33.44.55"
	mac2 := "10.22.33.44.55.66"
	ipOpt1 := IPOpt{Version: "v4", Protocol: "tcp"}
	ipOpt2 := IPOpt{Version: "v6", Protocol: "udp"}
	inr1 := SGRule{Ports: "tcp/221", PeerGroup: "g2", Action: "permittt"}
	inr2 := SGRule{Ports: "tcp/222", PeerGroup: "g1", Action: "permittt"}
	outr1 := SGRule{Ports: "udp/221", PeerGroup: "g3", Action: "dennny"}
	cond1 := NodeCondition{Type: 1, Status: 2, LastTransitionTime: 90001, Reason: "some reason", Message: "some message"}
	cond2 := NodeCondition{Type: 2, Status: 0, LastTransitionTime: 80001, Reason: "some other reason", Message: "some other message"}
	u := User{
		TypeMeta:   TypeMeta{Kind: "user"},
		ObjectMeta: ObjectMeta{Name: "joe", Labels: map[string]string{"dept": "eng", "level": "mts"}},
		Spec: UserSpec{
			Aliases:     "jhonny",
			Roles:       []string{"storage-admin", "security-admin"},
			DummyRoles:  []string{"dummy1", "dummy2"},
			MatchLabels: map[string]string{"io.pensando.area": "network"},
			AttachGroup: "app-sg",
			InRules: []SGRule{
				{Ports: "tcp/80,tcp/443", Action: "permit,log", PeerGroup: "web-sg"},
				{Ports: "tcp/6379", Action: "permit", PeerGroup: "db-sg"},
			},
			OutRules: []SGRule{
				{Ports: "tcp/6379", Action: "permit,log", PeerGroup: "db-sg"},
			},
			Interval: 33,
			Perms:    &Permission{ToObj: "sgpolicy", RWX: "r"},
			Policies: map[string]Policy{"key1": {ToGroup: "to-key1", FromGroup: "from-key1"},
				"key2": {ToGroup: "to-key2", FromGroup: "from-key2"}},
			UserHandle:  0x45544422,
			Uint32Field: 9821,
			Int32Field:  7473,
			SIPAddress:  &ip1,
			MacAddrs:    []*string{&mac1, &mac2},
			IPOpts:      []*IPOpt{&ipOpt1, &ipOpt2},
			InRulesR:    []*SGRule{&inr1, &inr2},
			OutRulesR:   []*SGRule{&outr1},
			FixedRules: [2]SGRule{
				{Ports: "tcp/88,tcp/444", Action: "log-only", PeerGroup: "some-other-sg"},
				{Ports: "tcp/6777", Action: "bluff", PeerGroup: "some-sg"},
			},
			NodeRoles:  []NodeSpecNodeRole{997, 799},
			Conditions: []*NodeCondition{&cond1, &cond2},
			BoolFlag:   false,
			FloatVal:   11.322,
		},
	}
	kvs := make(map[string]FInfo)
	kvs["name"] = NewFInfo([]string{"joe"})
	kvs["roles"] = NewFInfo([]string{"blah-blah"})
	kvs["toGroup"] = NewFInfo([]string{"key1:new-to-key1", "key3:to-key3"})
	kvs["fromGroup"] = NewFInfo([]string{"key1:new-from-key1", "key3:from-key3"})
	kvs["sipAddress"] = NewFInfo([]string{"10.1.1.2"})
	kvs["macAddrs"] = NewFInfo([]string{"00.11.22.33.44.55", "00.22.33.44.55.66"})
	kvs["ip_ver"] = NewFInfo([]string{"ipv4", "ipv6"})
	kvs["ip_proto"] = NewFInfo([]string{"udper", "tcper"})
	kvs["outr_port"] = NewFInfo([]string{"tcp/4411", "tcp/2121"})
	kvs["outr_action"] = NewFInfo([]string{"permit", "permit"})
	kvs["outr_peer_group"] = NewFInfo([]string{"g5", "g6"})
	kvs["fix_port"] = NewFInfo([]string{"tcp/5555", "icmp/echo"})
	kvs["fix_action"] = NewFInfo([]string{"action-one", "action-two"})
	kvs["fix_peer_group"] = NewFInfo([]string{"group-foo", "group-bar"})
	kvs["nodeRoles"] = NewFInfo([]string{"455", "544", "5544"})
	kvs["status"] = NewFInfo([]string{"3", "4"})
	kvs["type"] = NewFInfo([]string{"9", "9"})
	kvs["reason"] = NewFInfo([]string{"new reason", "another new reason"})
	kvs["message"] = NewFInfo([]string{"one", "two"})
	kvs["lastTransitionTime"] = NewFInfo([]string{"666666", "7777777"})
	kvs["boolFlag"] = NewFInfo([]string{"true"})
	kvs["floatVal"] = NewFInfo([]string{"901.019"})

	refCtx := &RfCtx{GetSubObj: subObj, UseJSONTag: true}
	newObj := WriteKvs(u, refCtx, kvs)
	newUser := newObj.(User)

	if newUser.ObjectMeta.Name != "joe" {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write Name")
	}
	if newUser.ObjectMeta.Labels["dept"] != "eng" || newUser.ObjectMeta.Labels["level"] != "mts" {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to find labels")
	}
	if len(newUser.Spec.Roles) != 1 ||
		newUser.Spec.Roles[0] != "blah-blah" {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write Roles")
	}
	if len(newUser.Spec.DummyRoles) != 2 ||
		newUser.Spec.DummyRoles[0] != "dummy1" ||
		newUser.Spec.DummyRoles[1] != "dummy2" {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to preserve DummyRoles")
	}
	if newUser.Spec.Aliases != "jhonny" {
		t.Fatalf("unable to find aliases")
	}
	if newUser.Spec.MatchLabels["io.pensando.area"] != "network" {
		t.Fatalf("unable to find label based Permissions")
	}

	if len(newUser.Spec.InRules) != 2 ||
		newUser.Spec.InRules[0].Ports != "tcp/80,tcp/443" ||
		newUser.Spec.InRules[1].Ports != "tcp/6379" {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to find all InRules")
	}
	if newUser.Spec.InRules[0].Action != "permit,log" || newUser.Spec.InRules[1].Action != "permit" {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write Action InRules")
	}
	if newUser.Spec.InRules[0].PeerGroup != "web-sg" || newUser.Spec.InRules[1].PeerGroup != "db-sg" {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write PeerGroup InRules")
	}

	if newUser.Spec.Perms == nil || newUser.Spec.Perms.ToObj != "sgpolicy" || newUser.Spec.Perms.RWX != "r" {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to parse Perms into pointer object")
	}

	if *newUser.Spec.SIPAddress != "10.1.1.2" {
		fmt.Printf("IPAddress: --%s--\n\n", *newUser.Spec.SIPAddress)
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write SIPAddress into pointer object")
	}

	if len(newUser.Spec.MacAddrs) != 2 {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write MacAddrs into pointer slice")
	} else if *newUser.Spec.MacAddrs[0] != "00.11.22.33.44.55" || *newUser.Spec.MacAddrs[1] != "00.22.33.44.55.66" {
		fmt.Printf("MacAddrs obtained: --%s--, --%s--\n\n", *newUser.Spec.MacAddrs[0], *newUser.Spec.MacAddrs[1])
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write MacAddrs into pointer slice")
	}

	if len(newUser.Spec.IPOpts) != 2 {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write IPOpts into pointer slice")
	}
	retIPOpt1 := *newUser.Spec.IPOpts[0]
	retIPOpt2 := *newUser.Spec.IPOpts[1]
	if retIPOpt1.Version != "ipv4" || retIPOpt2.Version != "ipv6" ||
		retIPOpt1.Protocol != "udper" || retIPOpt2.Protocol != "tcper" {
		fmt.Printf("IPOpts obtained: --%s--, --%s--\n\n", retIPOpt1, retIPOpt2)
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write IPOpts into pointer slice")
	}

	if len(newUser.Spec.OutRulesR) != 2 {
		fmt.Printf("unable to fetch outRuleR: %+v \n", newUser)
		t.Fatalf("unable to write OuterRulesR pointer to struct with tags")
	}
	outr2 := *newUser.Spec.OutRulesR[0]
	outr3 := *newUser.Spec.OutRulesR[1]
	if outr2.Ports != "tcp/4411" || outr3.Ports != "tcp/2121" ||
		outr2.Action != "permit" || outr3.Action != "permit" ||
		outr2.PeerGroup != "g5" || outr3.PeerGroup != "g6" {
		fmt.Printf("unable to fetch outRuleR: got %+v %+v \n", outr2, outr3)
		t.Fatalf("unable to write OuterRulesR struct pointer with tags")
	}

	inr3 := *newUser.Spec.InRulesR[0]
	inr4 := *newUser.Spec.InRulesR[1]
	if inr3.Ports != "tcp/221" || inr4.Ports != "tcp/222" ||
		inr3.Action != "permittt" || inr4.Action != "permittt" ||
		inr3.PeerGroup != "g2" || inr4.PeerGroup != "g1" {
		fmt.Printf("unable to fetch InRuleR: got %+v %+v \n", inr3, inr4)
		t.Fatalf("unable to write InRulesR struct pointer with tags")
	}

	fix1 := newUser.Spec.FixedRules[0]
	fix2 := newUser.Spec.FixedRules[1]
	if fix1.Ports != "tcp/5555" || fix2.Ports != "icmp/echo" ||
		fix1.Action != "action-one" || fix2.Action != "action-two" ||
		fix1.PeerGroup != "group-foo" || fix2.PeerGroup != "group-bar" {
		fmt.Printf("unable to fetch FixedRuleR: got %+v %+v \n", fix1, fix2)
		t.Fatalf("unable to write FixedRulesR struct pointer with tags")
	}

	if len(newUser.Spec.NodeRoles) != 3 || newUser.Spec.NodeRoles[0] != 455 ||
		newUser.Spec.NodeRoles[1] != 544 || newUser.Spec.NodeRoles[2] != 5544 {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write NodeRoles array to indirect type")
	}

	if newUser.Spec.BoolFlag != true {
		t.Fatalf("unable to write BoolFlag")
	}

	newCond1 := newUser.Spec.Conditions[0]
	newCond2 := newUser.Spec.Conditions[1]
	if newCond1.Status != 3 || newCond2.Status != 4 ||
		newCond1.Type != 9 || newCond2.Type != 9 ||
		newCond1.Reason != "new reason" || newCond2.Reason != "another new reason" ||
		newCond1.Message != "one" || newCond2.Message != "two" ||
		newCond1.LastTransitionTime != 666666 || newCond2.LastTransitionTime != 7777777 {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write NodeRoles array to indirect type")
	}

	if newUser.Spec.FloatVal != 901.019 {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write FloatVal")
	}

}

func TestNewWriteJson(t *testing.T) {
	kvs := make(map[string]FInfo)
	kvs["name"] = NewFInfo([]string{"joe"})
	kvs["labels"] = NewFInfo([]string{"dept:eng", "level:mts"})
	kvs["roles"] = NewFInfo([]string{"storage-admin", "security-admin"})
	kvs["aliases"] = NewFInfo([]string{"jhonny"})
	kvs["matchLabels"] = NewFInfo([]string{"io.pensando.area:network", "color:purple"})
	kvs["interval"] = NewFInfo([]string{"4554"})
	kvs["in_port"] = NewFInfo([]string{"tcp/80,tcp/443", "tcp/6379"})
	kvs["out_port"] = NewFInfo([]string{"tcp/8181"})
	kvs["in_action"] = NewFInfo([]string{"permit,log", "permit,log"})
	kvs["in_peer_group"] = NewFInfo([]string{"web-sg", "db-sg"})
	kvs["junk"] = NewFInfo([]string{"web-sg", "db-sg"})
	kvs["toObj"] = NewFInfo([]string{"sgpolicy"})
	kvs["rwx"] = NewFInfo([]string{"rw"})
	kvs["userHandle"] = NewFInfo([]string{"0x45544422"})
	kvs["uint32Field"] = NewFInfo([]string{"9823"})
	kvs["int32Field"] = NewFInfo([]string{"7473"})
	kvs["sipAddress"] = NewFInfo([]string{"10.1.1.2"})
	kvs["macAddrs"] = NewFInfo([]string{"00.11.22.33.44.55", "00.22.33.44.55.66"})
	kvs["ip_ver"] = NewFInfo([]string{"ipv4", "ipv6"})
	kvs["ip_proto"] = NewFInfo([]string{"udper", "tcper"})
	kvs["inr_port"] = NewFInfo([]string{"tcp/221", "tcp/222"})
	kvs["inr_action"] = NewFInfo([]string{"permittt", "permittt"})
	kvs["inr_peer_group"] = NewFInfo([]string{"g2", "g1"})
	kvs["outr_port"] = NewFInfo([]string{"tcp/4411", "tcp/2121"})
	kvs["outr_action"] = NewFInfo([]string{"permit", "permit"})
	kvs["outr_peer_group"] = NewFInfo([]string{"g5", "g6"})
	kvs["fix_port"] = NewFInfo([]string{"tcp/5555", "icmp/echo"})
	kvs["fix_action"] = NewFInfo([]string{"action-one", "action-two"})
	kvs["fix_peer_group"] = NewFInfo([]string{"group-foo", "group-bar"})
	kvs["nodeRoles"] = NewFInfo([]string{"455", "544", "5544"})
	kvs["status"] = NewFInfo([]string{"3", "4"})
	kvs["type"] = NewFInfo([]string{"9", "9"})
	kvs["reason"] = NewFInfo([]string{"new reason", "another new reason"})
	kvs["message"] = NewFInfo([]string{"one", "two"})
	kvs["lastTransitionTime"] = NewFInfo([]string{"666666", "7777777"})
	kvs["boolFlag"] = NewFInfo([]string{"true"})
	kvs["floatVal"] = NewFInfo([]string{"901.109"})

	refCtx := &RfCtx{GetSubObj: subObj, UseJSONTag: true}
	newObj := WriteKvs(User{}, refCtx, kvs)
	newUser := newObj.(User)

	if newUser.ObjectMeta.Name != "joe" {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write Name")
	}
	if newUser.ObjectMeta.Labels["dept"] != "eng" || newUser.ObjectMeta.Labels["level"] != "mts" {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to find labels")
	}
	if len(newUser.Spec.Roles) != 2 ||
		newUser.Spec.Roles[0] != "storage-admin" ||
		newUser.Spec.Roles[1] != "security-admin" {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write Roles")
	}
	if newUser.Spec.Aliases != "jhonny" {
		t.Fatalf("unable to find aliases")
	}
	if newUser.Spec.MatchLabels["io.pensando.area"] != "network" && newUser.Spec.MatchLabels["color"] != "purple" {
		t.Fatalf("unable to find label based Permissions")
	}

	if len(newUser.Spec.InRules) != 2 ||
		newUser.Spec.InRules[0].Ports != "tcp/80,tcp/443" ||
		newUser.Spec.InRules[1].Ports != "tcp/6379" {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to find all InRules")
	}
	if newUser.Spec.InRules[0].Action != "permit,log" || newUser.Spec.InRules[1].Action != "permit,log" {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write Action InRules")
	}
	if newUser.Spec.InRules[0].PeerGroup != "web-sg" || newUser.Spec.InRules[1].PeerGroup != "db-sg" {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write PeerGroup InRules")
	}

	if newUser.Spec.Interval != 4554 {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write integer value Interval")
	}

	if newUser.Spec.Perms == nil || newUser.Spec.Perms.ToObj != "sgpolicy" || newUser.Spec.Perms.RWX != "rw" {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write Perms into pointer object")
	}

	if newUser.Spec.UserHandle != 0x45544422 {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write UserHandle into pointer object")
	}

	if newUser.Spec.Uint32Field != 9823 {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write Uint32Field into pointer object")
	}

	if newUser.Spec.Int32Field != 7473 {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write Int32Field into pointer object")
	}

	if *newUser.Spec.SIPAddress != "10.1.1.2" {
		fmt.Printf("IPAddress: --%s--\n\n", *newUser.Spec.SIPAddress)
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write SIPAddress into pointer object")
	}
	if len(newUser.Spec.MacAddrs) != 2 {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write MacAddrs into pointer slice")
	} else if *newUser.Spec.MacAddrs[0] != "00.11.22.33.44.55" || *newUser.Spec.MacAddrs[1] != "00.22.33.44.55.66" {
		fmt.Printf("MacAddrs obtained: --%s--, --%s--\n\n", *newUser.Spec.MacAddrs[0], *newUser.Spec.MacAddrs[1])
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write MacAddrs into pointer slice")
	}

	if len(newUser.Spec.IPOpts) != 2 {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write IPOpts into pointer slice")
	}
	retIPOpt1 := *newUser.Spec.IPOpts[0]
	retIPOpt2 := *newUser.Spec.IPOpts[1]
	if retIPOpt1.Version != "ipv4" || retIPOpt2.Version != "ipv6" ||
		retIPOpt1.Protocol != "udper" || retIPOpt2.Protocol != "tcper" {
		fmt.Printf("IPOpts obtained: --%s--, --%s--\n\n", retIPOpt1, retIPOpt2)
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write IPOpts into pointer slice")
	}

	outr2 := *newUser.Spec.OutRulesR[0]
	outr3 := *newUser.Spec.OutRulesR[1]
	if outr2.Ports != "tcp/4411" || outr3.Ports != "tcp/2121" ||
		outr2.Action != "permit" || outr3.Action != "permit" ||
		outr2.PeerGroup != "g5" || outr3.PeerGroup != "g6" {
		fmt.Printf("unable to fetch outRuleR: got %+v %+v \n", outr2, outr3)
		t.Fatalf("unable to write OuterRulesR struct pointer with tags")
	}

	inr3 := *newUser.Spec.InRulesR[0]
	inr4 := *newUser.Spec.InRulesR[1]
	if inr3.Ports != "tcp/221" || inr4.Ports != "tcp/222" ||
		inr3.Action != "permittt" || inr4.Action != "permittt" ||
		inr3.PeerGroup != "g2" || inr4.PeerGroup != "g1" {
		fmt.Printf("unable to fetch InRuleR: got %+v %+v \n", inr3, inr4)
		t.Fatalf("unable to write InRulesR struct pointer with tags")
	}

	fix1 := newUser.Spec.FixedRules[0]
	fix2 := newUser.Spec.FixedRules[1]
	if fix1.Ports != "tcp/5555" || fix2.Ports != "icmp/echo" ||
		fix1.Action != "action-one" || fix2.Action != "action-two" ||
		fix1.PeerGroup != "group-foo" || fix2.PeerGroup != "group-bar" {
		fmt.Printf("unable to fetch FixedRuleR: got %+v %+v \n", fix1, fix2)
		t.Fatalf("unable to write FixedRulesR struct pointer with tags")
	}

	if len(newUser.Spec.NodeRoles) != 3 || newUser.Spec.NodeRoles[0] != 455 ||
		newUser.Spec.NodeRoles[1] != 544 || newUser.Spec.NodeRoles[2] != 5544 {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write NodeRoles array to indirect type")
	}

	newCond1 := newUser.Spec.Conditions[0]
	newCond2 := newUser.Spec.Conditions[1]
	if newCond1.Status != 3 || newCond2.Status != 4 ||
		newCond1.Type != 9 || newCond2.Type != 9 ||
		newCond1.Reason != "new reason" || newCond2.Reason != "another new reason" ||
		newCond1.Message != "one" || newCond2.Message != "two" ||
		newCond1.LastTransitionTime != 666666 || newCond2.LastTransitionTime != 7777777 {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write NodeRoles array to indirect type")
	}

	if newUser.Spec.BoolFlag != true {
		t.Fatalf("unable to write BoolFlag")
	}

	if newUser.Spec.FloatVal != 901.109 {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write FloatVal")
	}
}
