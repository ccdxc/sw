package ref

import (
	"fmt"
	"reflect"
	"strconv"
	"testing"
)

type TypeMeta struct {
	Kind       string `json:"kind,omitempty"`
	APIVersion string `json:"apiVersion,omitempty"`
}

type ObjectMeta struct {
	Tenant          string            `json:"tenant,omitempty" venice:"key"`
	Name            string            `json:"name,omitempty" venice:"key"`
	Namespace       string            `json:"namespace,omitempty"`
	ResourceVersion string            `json:"resourceVersion,omitempty"`
	UUID            string            `json:"uuid,omitempty"`
	Labels          map[string]string `json:"labels,omitempty"`
}

type ListMeta struct {
	ResourceVersion string `json:"resourceVersion,omitempty"`
}

type User struct {
	TypeMeta   `json:",inline"`
	ObjectMeta `json:"meta"`
	Spec       UserSpec `json:"spec,omitempty"`
}

type SGRule struct {
	Ports     string `json:"ports,omitempty" venice:"id=port"`
	Action    string `json:"action,omitempty"`
	PeerGroup string `json:"sourceGroup,omitempty" venice:"id=peer_group"`
}

type Permission struct {
	ToObj string `json:"toObj,omitempty"`
	RWX   string `json:"rwx, omitempty"`
}

type Policy struct {
	ToGroup   string `json:"toGroup,omitempty"`
	FromGroup string `json:"fromGroup, omitempty"`
}

type IPOpt struct {
	Version  string `json:"ipVersion,omitempty" venice:"id=ip_ver"`
	Protocol string `json:"protocol,omitempty" venice:"id=ip_proto"`
}

type UserSpec struct {
	Aliases      string            `json:"aliases,omitempty"`
	Roles        []string          `json:"roles,omitempty"`
	MatchLabels  map[string]string `json:"matchLabels,omitempty"`
	AttachGroup  string            `json:"attachGroup,omitempty"`
	InRules      []SGRule          `json:"igRules,omitempty" venice:"ins=in"`
	OutRules     []SGRule          `json:"egRules,omitempty" venice:"ins=out"`
	Interval     int               `json:"interval,omitempty"`
	SkippedField string            `json:"uuid,omitempty" venice:"sskip"`
	Perms        *Permission       `json:"perm,omitempty"`
	Policies     map[string]Policy `json:"permMap,omitempty"`
	UserHandle   uint64            `json:"userHandle,omitempty"`
	SIPAddress   *string           `json:"sipAddress,omitempty"`
	MacAddrs     []*string         `json:"macAdds,omitempty"`
	IPOpts       []*IPOpt          `json:"fwProfiles,omitempty"`
	InRulesR     []*SGRule         `json:"igRulesR,omitempty" venice:"ins=inr"`
	OutRulesR    []*SGRule         `json:"egRulesR,omitempty" venice:"ins=outr"`
	FixedRules   [2]SGRule         `json:"fixedRule,omitempty" venice:"ins=fix"`
}

type UserList struct {
	TypeMeta `json:",inline"`
	ListMeta `json:"meta"`
	Items    []User
}

func TestWalkStruct(t *testing.T) {
	expectedStr := `{
  TypeMeta:   {
    Kind: string
    APIVersion: string
    }
  ObjectMeta:   {
    Tenant: string
    Name: string
    Namespace: string
    ResourceVersion: string
    UUID: string
    Labels: map[string]string
    }
  Spec:   {
    Aliases: string
    Roles: []string
    MatchLabels: map[string]string
    AttachGroup: string
    InRules: []      {
        Ports: string
        Action: string
        PeerGroup: string
        }
    OutRules: []      {
        Ports: string
        Action: string
        PeerGroup: string
        }
    Interval: int
    SkippedField: string
    *Perms:     {
      ToObj: string
      RWX: string
      }
    Policies: map[string]struct
      {
        ToGroup: string
        FromGroup: string
        }
    UserHandle: uint64
    *SIPAddress: string
    MacAddrs: []*string
    IPOpts: []*      {
        Version: string
        Protocol: string
        }
    InRulesR: []*      {
        Ports: string
        Action: string
        PeerGroup: string
        }
    OutRulesR: []*      {
        Ports: string
        Action: string
        PeerGroup: string
        }
    FixedRules: []      {
        Ports: string
        Action: string
        PeerGroup: string
        }
      {
        Ports: string
        Action: string
        PeerGroup: string
        }
    }
  }
`
	refCtx := &RefCtx{GetSubObj: subObj}
	outStr := WalkStruct(User{}, refCtx)
	if outStr != expectedStr {
		t.Fatalf("Out:\n--%s--\nExpected:\n--%s--\n", outStr, expectedStr)
	}
}

func TestEmptyGet(t *testing.T) {
	refCtx := &RefCtx{GetSubObj: subObj}
	kvs := make(map[string]FInfo)
	GetKvs(User{}, refCtx, kvs)
	total := 0

	if _, ok := kvs["APIVersion"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("APIVersion not found in kvs")
	}
	total++
	if _, ok := kvs["ResourceVersion"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("ResourceVersion not found in kvs")
	}
	total++
	if _, ok := kvs["Tenant"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("Tenant not found in kvs")
	}
	total++
	if _, ok := kvs["Namespace"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("Namespace not found in kvs")
	}
	total++
	if _, ok := kvs["Kind"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("Kind not found in kvs")
	}
	total++
	if _, ok := kvs["Name"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("Name not found in kvs")
	}
	total++
	if _, ok := kvs["UUID"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("UUID not found in kvs")
	}
	total++
	if _, ok := kvs["Labels"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("Labels not found in kvs")
	}
	total++
	if _, ok := kvs["Aliases"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("Aliases not found in kvs")
	}
	total++
	if _, ok := kvs["Roles"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("Roles not found in kvs")
	}
	total++
	if _, ok := kvs["MatchLabels"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("MatchLabels not found in kvs")
	}
	total++
	if _, ok := kvs["AttachGroup"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("AttachGroup not found in kvs")
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
	if _, ok := kvs["in_Action"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("in_Action not found in kvs")
	}
	total++
	if _, ok := kvs["out_Action"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("out_Action not found in kvs")
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
	if _, ok := kvs["Interval"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("Interval not found in kvs")
	}
	total++
	if _, ok := kvs["SkippedField"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("SkippedField not found in kvs")
	}
	total++
	if _, ok := kvs["ToObj"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("ToObj not found in kvs")
	}
	total++
	if _, ok := kvs["RWX"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("RWX not found in kvs")
	}
	total++
	if _, ok := kvs["ToGroup"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("ToGroup not found in kvs")
	}
	total++
	if _, ok := kvs["FromGroup"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("FromGroup not found in kvs")
	}
	total++
	if _, ok := kvs["UserHandle"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("UserHandle not found in kvs")
	}
	total++
	if _, ok := kvs["SIPAddress"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("SIPAddress not found in kvs")
	}
	total++
	if _, ok := kvs["MacAddrs"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("MacAddrs not found in kvs")
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
	if _, ok := kvs["inr_Action"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("inr_Action not found in kvs")
	}
	total++
	if _, ok := kvs["outr_Action"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("outr_Action not found in kvs")
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
	if _, ok := kvs["fix_Action"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("fix_Action not found in kvs")
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

	if len(kvs) != total {
		printKvs("kvs", kvs, true)
		t.Fatalf("unexpected number of kv pairs %d found %d", len(kvs), total)
	}
}

func TestGet(t *testing.T) {
	ip1 := "131.22.1.1"
	mac1 := "00.11.22.33.44.55"
	mac2 := "00.22.33.44.55.66"
	ipOpt1 := IPOpt{Version: "v4", Protocol: "tcp"}
	ipOpt2 := IPOpt{Version: "v6", Protocol: "udp"}
	inr1 := SGRule{Ports: "tcp/221", PeerGroup: "g2", Action: "permittt"}
	inr2 := SGRule{Ports: "tcp/222", PeerGroup: "g1", Action: "permittt"}
	outr1 := SGRule{Ports: "udp/221", PeerGroup: "g3", Action: "dennny"}

	u := User{
		TypeMeta:   TypeMeta{Kind: "user"},
		ObjectMeta: ObjectMeta{Name: "joe", Tenant: "pices", Labels: map[string]string{"dept": "eng", "level": "mts"}},
		Spec: UserSpec{
			Aliases:     "jhonny",
			Roles:       []string{"storage-admin", "security-admin"},
			MatchLabels: map[string]string{"io.pensando.area": "network"},
			AttachGroup: "app-sg",
			InRules: []SGRule{
				{Ports: "tcp/80,tcp/443", Action: "permit,log", PeerGroup: "web-sg"},
			},
			OutRules: []SGRule{
				{Ports: "tcp/6379", Action: "permit,log", PeerGroup: "db-sg"},
			},
			Interval: 33,
			Perms:    &Permission{ToObj: "network", RWX: "rw"},
			Policies: map[string]Policy{"key1": {ToGroup: "to-key1", FromGroup: "from-key1"},
				"key2": {ToGroup: "to-key2", FromGroup: "from-key2"}},
			UserHandle: 0x45544422,
			SIPAddress: &ip1,
			MacAddrs:   []*string{&mac1, &mac2},
			IPOpts:     []*IPOpt{&ipOpt1, &ipOpt2},
			InRulesR:   []*SGRule{&inr1, &inr2},
			OutRulesR:  []*SGRule{&outr1},
			FixedRules: [2]SGRule{
				{Ports: "tcp/80,tcp/443", Action: "permit,log", PeerGroup: "web-sg"},
				{Ports: "udp/80", Action: "permit,log", PeerGroup: "app-sg"},
			},
		},
	}
	kvs := make(map[string]FInfo)
	refCtx := &RefCtx{GetSubObj: subObj}
	GetKvs(u, refCtx, kvs)

	if fi, ok := kvs["Name"]; ok {
		if !fi.Key || fi.ValueStr[0] != "joe" {
			printKvs("Meta", kvs, false)
			fmt.Printf("fi = %+v\n", fi)
			t.Fatalf("error! name not set correctly")
		}
	} else {
		t.Fatalf("Name not found in kvs")
	}

	if fi, ok := kvs["Tenant"]; ok {
		if !fi.Key || fi.ValueStr[0] != "pices" {
			printKvs("Meta", kvs, false)
			fmt.Printf("fi = %+v\n", fi)
			t.Fatalf("error! tenant not set correctly")
		}
	} else {
		t.Fatalf("Tenant not found in kvs")
	}

	if fi, ok := kvs["Labels"]; ok {
		if fi.Key || fi.ValueStr[0] != "dept:eng,level:mts" && fi.ValueStr[0] != "level:mts,dept:eng" {
			printKvs("Meta", kvs, false)
			t.Fatalf("error! value not set correctly")
		}
	} else {
		t.Fatalf("Labels not found in kvs")
	}

	if fi, ok := kvs["Aliases"]; ok {
		if fi.Key || fi.ValueStr[0] != "jhonny" {
			t.Fatalf("error! value is not set correctly")
		}
	} else {
		t.Fatalf("error finding key")
	}

	if fi, ok := kvs["Roles"]; ok {
		if fi.Key || fi.ValueStr[0] != "security-admin,storage-admin" &&
			fi.ValueStr[0] != "storage-admin,security-admin" {
			printKvs("Spec", kvs, false)
			t.Fatalf("error! value not set correctly")
		}
	} else {
		t.Fatalf("Roles not found in kvs")
	}

	if fi, ok := kvs["MatchLabels"]; ok {
		if fi.Key || fi.ValueStr[0] != "io.pensando.area:network" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! labels fetched '%s' not set correctly", fi.ValueStr[0])
		}
	} else {
		t.Fatalf("MatchLabels not found in kvs")
	}

	if fi, ok := kvs["in_port"]; ok {
		if fi.Key || len(fi.ValueStr) != 1 || fi.ValueStr[0] != "tcp/80,tcp/443" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! in_port fetched '%s' not set correctly", fi.ValueStr)
		}
	} else {
		printKvs("spec", kvs, false)
		t.Fatalf("in_port not found")
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

	if fi, ok := kvs["in_peer_group"]; ok {
		if fi.Key || len(fi.ValueStr) != 1 || fi.ValueStr[0] != "web-sg" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! Peer Group fetched '%s' not set correctly", fi.ValueStr)
		}
	} else {
		printKvs("spec", kvs, false)
		t.Fatalf("in_peer_group not found")
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

	if fi, ok := kvs["Interval"]; ok {
		if v, err := strconv.Atoi(fi.ValueStr[0]); err != nil || fi.Key || v != 33 {
			printKvs("spec", kvs, false)
			t.Fatalf("error! Interval '%v' not set correctly", fi.ValueStr)
		}
	}

	if fi, ok := kvs["SkippedField"]; ok {
		if fi.Key || !fi.SSkip {
			printKvs("spec", kvs, false)
			t.Fatalf("error! not identified SkippedFiled '%v'", fi)
		}
	}

	if fi, ok := kvs["ToObj"]; ok {
		if fi.Key || len(fi.ValueStr) != 1 || fi.ValueStr[0] != "network" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! ToObj from pointed struct not found '%v'", fi.ValueStr)
		}
	} else {
		t.Fatalf("ToObj not found")
	}

	if fi, ok := kvs["RWX"]; ok {
		if fi.Key || len(fi.ValueStr) != 1 || fi.ValueStr[0] != "rw" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! ToObj from pointed struct not found '%v'", fi.ValueStr)
		}
	} else {
		t.Fatalf("ToObj not found")
	}

	if fi, ok := kvs["ToGroup"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 ||
			(fi.ValueStr[0] != "key2:to-key2" && fi.ValueStr[0] != "key1:to-key1") ||
			(fi.ValueStr[1] != "key2:to-key2" && fi.ValueStr[1] != "key1:to-key1") ||
			(fi.ValueStr[0] == fi.ValueStr[1]) {
			printKvs("spec", kvs, false)
			t.Fatalf("error! ToGroup in the map not found '%v'", fi.ValueStr)
		}
	} else {
		t.Fatalf("ToGroup not found")
	}

	if fi, ok := kvs["FromGroup"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 ||
			(fi.ValueStr[0] != "key2:from-key2" && fi.ValueStr[0] != "key1:from-key1") ||
			(fi.ValueStr[1] != "key2:from-key2" && fi.ValueStr[1] != "key1:from-key1") ||
			(fi.ValueStr[0] == fi.ValueStr[1]) {
			printKvs("spec", kvs, false)
			t.Fatalf("error! FromGroup in the map not found '%v'", fi.ValueStr)
		}
	} else {
		t.Fatalf("FromGroup not found")
	}

	if fi, ok := kvs["UserHandle"]; ok {
		handle, _ := strconv.ParseUint(fi.ValueStr[0], 10, 64)
		if fi.Key || len(fi.ValueStr) != 1 || handle != 0x45544422 {
			printKvs("spec", kvs, false)
			t.Fatalf("error! handle %v not found '%s'", handle, fi.ValueStr)
		}
	} else {
		t.Fatalf("UserHandle not found")
	}

	if fi, ok := kvs["SIPAddress"]; ok {
		if fi.Key || len(fi.ValueStr) != 1 || fi.ValueStr[0] != ip1 {
			printKvs("spec", kvs, false)
			t.Fatalf("error! SIPAddress %v not found '%s'", ip1, fi.ValueStr)
		}
	} else {
		t.Fatalf("SIPAddress not found")
	}

	if fi, ok := kvs["MacAddrs"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != mac1 && fi.ValueStr[1] != mac2 {
			printKvs("spec", kvs, false)
			t.Fatalf("error! MacAddrs %s,%s not found '%s'", mac1, mac2, fi.ValueStr)
		}
	} else {
		t.Fatalf("MacAddrs not found")
	}

	if fi, ok := kvs["ip_ver"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "v4" && fi.ValueStr[1] != "v6" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! ip_ver %s,%s not found '%s'", ipOpt1, ipOpt2, fi.ValueStr)
		}
	} else {
		t.Fatalf("ip_ver not found")
	}

	if fi, ok := kvs["inr_port"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "tcp/221" || fi.ValueStr[1] != "tcp/222" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! inr_port %s,%s not found '%s'", inr1, inr2, fi.ValueStr)
		}
	} else {
		t.Fatalf("inr_port not found")
	}
	if fi, ok := kvs["inr_peer_group"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "g2" || fi.ValueStr[1] != "g1" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! inr_peer_group %s,%s not found '%s'", inr1, inr2, fi.ValueStr)
		}
	} else {
		t.Fatalf("inr_peer_group not found")
	}

	if fi, ok := kvs["inr_Action"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "permittt" || fi.ValueStr[1] != "permittt" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! inr_Action %s,%s not found '%s'", inr1, inr2, fi.ValueStr)
		}
	} else {
		t.Fatalf("inr_Action not found")
	}

	if fi, ok := kvs["outr_port"]; ok {
		if fi.Key || len(fi.ValueStr) != 1 || fi.ValueStr[0] != "udp/221" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! outr_port %s, not found '%s'", outr1, fi.ValueStr)
		}
	} else {
		t.Fatalf("outr_port not found")
	}

	if fi, ok := kvs["outr_peer_group"]; ok {
		if fi.Key || len(fi.ValueStr) != 1 || fi.ValueStr[0] != "g3" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! outr_peer_group %s, not found '%s'", outr1, fi.ValueStr)
		}
	} else {
		t.Fatalf("outr_peer_group not found")
	}

	if fi, ok := kvs["outr_Action"]; ok {
		if fi.Key || len(fi.ValueStr) != 1 || fi.ValueStr[0] != "dennny" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! outr_Action %s not found '%s'", outr1, fi.ValueStr)
		}
	} else {
		t.Fatalf("outr_Action not found")
	}

	if fi, ok := kvs["fix_port"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "tcp/80,tcp/443" || fi.ValueStr[1] != "udp/80" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! fix_port not found '%s'", fi.ValueStr)
		}
	} else {
		t.Fatalf("fix_port not found")
	}

	if fi, ok := kvs["fix_peer_group"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "web-sg" || fi.ValueStr[1] != "app-sg" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! fix_peer_group not found '%s'", fi.ValueStr)
		}
	} else {
		t.Fatalf("fix_peer_group not found")
	}

	if fi, ok := kvs["fix_Action"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "permit,log" || fi.ValueStr[1] != "permit,log" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! fix_Action not found '%s'", fi.ValueStr)
		}
	} else {
		t.Fatalf("fix_Action not found")
	}
}

func TestUpdate(t *testing.T) {
	ip1 := "131.22.1.1"
	mac1 := "10.11.22.33.44.55"
	mac2 := "10.22.33.44.55.66"
	ipOpt1 := IPOpt{Version: "v4", Protocol: "tcp"}
	ipOpt2 := IPOpt{Version: "v6", Protocol: "udp"}
	inr1 := SGRule{Ports: "tcp/221", PeerGroup: "g2", Action: "permittt"}
	inr2 := SGRule{Ports: "tcp/222", PeerGroup: "g1", Action: "permittt"}
	outr1 := SGRule{Ports: "udp/221", PeerGroup: "g3", Action: "dennny"}
	u := User{
		TypeMeta:   TypeMeta{Kind: "user"},
		ObjectMeta: ObjectMeta{Name: "joe", Labels: map[string]string{"dept": "eng", "level": "mts"}},
		Spec: UserSpec{
			Aliases:     "jhonny",
			Roles:       []string{"storage-admin", "security-admin"},
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
			UserHandle: 0x45544422,
			SIPAddress: &ip1,
			MacAddrs:   []*string{&mac1, &mac2},
			IPOpts:     []*IPOpt{&ipOpt1, &ipOpt2},
			InRulesR:   []*SGRule{&inr1, &inr2},
			OutRulesR:  []*SGRule{&outr1},
			FixedRules: [2]SGRule{
				{Ports: "tcp/88,tcp/444", Action: "log-only", PeerGroup: "some-other-sg"},
				{Ports: "tcp/6777", Action: "bluff", PeerGroup: "some-sg"},
			},
		},
	}
	kvs := make(map[string]FInfo)
	kvs["Name"] = NewFInfo([]string{"joe"})
	kvs["Roles"] = NewFInfo([]string{"blah-blah"})
	kvs["ToGroup"] = NewFInfo([]string{"key1:new-to-key1", "key3:to-key3"})
	kvs["FromGroup"] = NewFInfo([]string{"key1:new-from-key1", "key3:from-key3"})
	kvs["SIPAddress"] = NewFInfo([]string{"10.1.1.2"})
	kvs["MacAddrs"] = NewFInfo([]string{"00.11.22.33.44.55", "00.22.33.44.55.66"})
	kvs["ip_ver"] = NewFInfo([]string{"ipv4", "ipv6"})
	kvs["ip_proto"] = NewFInfo([]string{"udper", "tcper"})
	kvs["outr_port"] = NewFInfo([]string{"tcp/4411", "tcp/2121"})
	kvs["outr_Action"] = NewFInfo([]string{"permit", "permit"})
	kvs["outr_peer_group"] = NewFInfo([]string{"g5", "g6"})
	kvs["fix_port"] = NewFInfo([]string{"tcp/5555", "icmp/echo"})
	kvs["fix_Action"] = NewFInfo([]string{"action-one", "action-two"})
	kvs["fix_peer_group"] = NewFInfo([]string{"group-foo", "group-bar"})

	refCtx := &RefCtx{GetSubObj: subObj}
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
}

func TestNewWrite(t *testing.T) {
	kvs := make(map[string]FInfo)
	kvs["Name"] = NewFInfo([]string{"joe"})
	kvs["Labels"] = NewFInfo([]string{"dept:eng,level:mts"})
	kvs["Roles"] = NewFInfo([]string{"storage-admin,security-admin"})
	kvs["Aliases"] = NewFInfo([]string{"jhonny"})
	kvs["MatchLabels"] = NewFInfo([]string{"io.pensando.area:network"})
	kvs["in_port"] = NewFInfo([]string{"tcp/80,tcp/443", "tcp/6379"})
	kvs["out_port"] = NewFInfo([]string{"tcp/8181"})
	kvs["in_Action"] = NewFInfo([]string{"permit,log", "permit,log"})
	kvs["in_peer_group"] = NewFInfo([]string{"web-sg", "db-sg"})
	kvs["junk"] = NewFInfo([]string{"web-sg", "db-sg"})
	kvs["ToObj"] = NewFInfo([]string{"sgpolicy"})
	kvs["RWX"] = NewFInfo([]string{"rw"})
	kvs["UserHandle"] = NewFInfo([]string{"0x45544422"})
	kvs["SIPAddress"] = NewFInfo([]string{"10.1.1.2"})
	kvs["MacAddrs"] = NewFInfo([]string{"00.11.22.33.44.55", "00.22.33.44.55.66"})
	kvs["ip_ver"] = NewFInfo([]string{"ipv4", "ipv6"})
	kvs["ip_proto"] = NewFInfo([]string{"udper", "tcper"})
	kvs["inr_port"] = NewFInfo([]string{"tcp/221", "tcp/222"})
	kvs["inr_Action"] = NewFInfo([]string{"permittt", "permittt"})
	kvs["inr_peer_group"] = NewFInfo([]string{"g2", "g1"})
	kvs["outr_port"] = NewFInfo([]string{"tcp/4411", "tcp/2121"})
	kvs["outr_Action"] = NewFInfo([]string{"permit", "permit"})
	kvs["outr_peer_group"] = NewFInfo([]string{"g5", "g6"})
	kvs["fix_port"] = NewFInfo([]string{"tcp/5555", "icmp/echo"})
	kvs["fix_Action"] = NewFInfo([]string{"action-one", "action-two"})
	kvs["fix_peer_group"] = NewFInfo([]string{"group-foo", "group-bar"})

	refCtx := &RefCtx{GetSubObj: subObj}
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
	if newUser.Spec.MatchLabels["io.pensando.area"] != "network" {
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

	if newUser.Spec.Perms == nil || newUser.Spec.Perms.ToObj != "sgpolicy" || newUser.Spec.Perms.RWX != "rw" {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write Perms into pointer object")
	}

	if newUser.Spec.UserHandle != 0x45544422 {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write UserHandle into pointer object")
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
}

func TestFieldByName(t *testing.T) {
	ip1 := "131.22.1.1"
	mac1 := "10.11.22.33.44.55"
	mac2 := "10.22.33.44.55.66"
	ipOpt1 := IPOpt{Version: "v4", Protocol: "tcp"}
	ipOpt2 := IPOpt{Version: "v6", Protocol: "udp"}
	inr1 := SGRule{Ports: "tcp/221", PeerGroup: "g2", Action: "permittt"}
	inr2 := SGRule{Ports: "tcp/222", PeerGroup: "g1", Action: "permittt"}
	outr1 := SGRule{Ports: "udp/221", PeerGroup: "g3", Action: "dennny"}
	u := User{
		TypeMeta:   TypeMeta{Kind: "user"},
		ObjectMeta: ObjectMeta{Name: "joe", Labels: map[string]string{"dept": "eng", "level": "mts"}},
		Spec: UserSpec{
			Aliases:     "jhonny",
			Roles:       []string{"storage-admin", "security-admin"},
			MatchLabels: map[string]string{"io.pensando.area": "network", "io.pensando.network.priority": "critical"},
			AttachGroup: "app-sg",
			InRules: []SGRule{
				{Ports: "tcp/80,tcp/443", Action: "permit,log", PeerGroup: "web-sg"},
			},
			OutRules: []SGRule{
				{Ports: "tcp/6379", Action: "permit,log", PeerGroup: "db-sg"},
				{Ports: "tcp/5505", Action: "permit,log", PeerGroup: "db-sg"},
			},
			Interval: 33,
			Perms:    &Permission{ToObj: "network", RWX: "rw"},
			Policies: map[string]Policy{"key1": {ToGroup: "to-key1", FromGroup: "from-key1"},
				"key2": {ToGroup: "to-key2", FromGroup: "from-key2"}},
			UserHandle: 0x45544422,
			SIPAddress: &ip1,
			MacAddrs:   []*string{&mac1, &mac2},
			IPOpts:     []*IPOpt{&ipOpt1, &ipOpt2},
			InRulesR:   []*SGRule{&inr1, &inr2},
			OutRulesR:  []*SGRule{&outr1},
			FixedRules: [2]SGRule{
				{Ports: "tcp/80,tcp/443", Action: "permit,log", PeerGroup: "fix-sg1"},
				{Ports: "udp/80", Action: "permit,log", PeerGroup: "fix-sg2"},
			},
		},
	}
	user := reflect.ValueOf(u)
	meta := reflect.ValueOf(u.ObjectMeta)
	spec := reflect.ValueOf(u.Spec)

	name := FieldByName(meta, "Name")
	if len(name) != 1 || name[0] != "joe" {
		t.Fatalf("Invalid name returned '%s'", name)
	}

	roles := FieldByName(spec, "Roles")
	if len(roles) != 2 || roles[0] != "storage-admin" || roles[1] != "security-admin" {
		t.Fatalf("Invalid roles returned '%s'", roles)
	}

	ports := FieldByName(user, "Spec.OutRules.Ports")
	if len(ports) != 2 || ports[0] != "tcp/6379" || ports[1] != "tcp/5505" {
		t.Fatalf("Invalid roles returned '%s'", ports)
	}

	matchLabels := FieldByName(user, "Spec.MatchLabels")
	if len(matchLabels) != 2 ||
		(matchLabels[0] != "io.pensando.area:network" ||
			matchLabels[1] != "io.pensando.network.priority:critical") &&
			(matchLabels[1] != "io.pensando.area:network" ||
				matchLabels[0] != "io.pensando.network.priority:critical") {
		t.Fatalf("Invalid matchLabels: %s", matchLabels)
	}

	ghosts := FieldByName(user, "Spec.OutRules.Ghosts")
	if len(ghosts) != 0 {
		t.Fatalf("Invalid ghosts returned '%s'", ghosts)
	}

	perms := FieldByName(user, "Spec.Perms.ToObj")
	if len(perms) != 1 || perms[0] != "network" {
		t.Fatalf("Invalid perms returned '%s'", perms)
	}

	sip := FieldByName(user, "Spec.SIPAddress")
	if len(sip) != 1 || sip[0] != ip1 {
		t.Fatalf("Invalid sip returned '%s'", sip)
	}

	macAddrs := FieldByName(user, "Spec.MacAddrs")
	if len(macAddrs) != 2 || macAddrs[0] != mac1 || macAddrs[1] != mac2 {
		t.Fatalf("Invalid macAddrs returned '%s'", macAddrs)
	}

	ipOptsProto := FieldByName(user, "Spec.IPOpts.Protocol")
	if len(ipOptsProto) != 2 || ipOptsProto[0] != "tcp" || ipOptsProto[1] != "udp" {
		t.Fatalf("Invalid ipOptsProto returned '%s'", ipOptsProto)
	}

	inrPorts := FieldByName(user, "Spec.InRulesR.Ports")
	if len(inrPorts) != 2 || inrPorts[0] != "tcp/221" || inrPorts[1] != "tcp/222" {
		t.Fatalf("Invalid inrPorts %s \n", inrPorts)
	}

	fixPgs := FieldByName(user, "Spec.FixedRules.PeerGroup")
	if len(fixPgs) != 2 || fixPgs[0] != "fix-sg1" || fixPgs[1] != "fix-sg2" {
		t.Fatalf("Invalid fixPgs %s \n", fixPgs)
	}
}

func printKvs(hdr string, kvs map[string]FInfo, onlyKey bool) {
	fmt.Printf("%s:\n", hdr)
	for key, fi := range kvs {
		if onlyKey {
			fmt.Printf("key '%s'\n", key)
		} else {
			if len(fi.ValueStr) != 0 && fi.ValueStr[0] != "" {
				fmt.Printf("key '%s', val '%s'\n", key, fi.ValueStr)
			}
		}
	}
}

func subObj(kind string) interface{} {
	switch kind {
	case "SGRule":
		return &SGRule{}
	case "Permission":
		return &Permission{}
	case "Policy":
		return &Policy{}
	case "IPOpt":
		return &IPOpt{}
	}
	return nil
}
