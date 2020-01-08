package ref_test

import (
	"fmt"
	"reflect"
	"sort"
	"strconv"
	"strings"
	"testing"

	"github.com/pensando/sw/venice/utils/ref"

	_ "github.com/pensando/sw/api/generated/bookstore"
	_ "github.com/pensando/sw/api/generated/cluster"
	_ "github.com/pensando/sw/api/generated/network"
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
	RWX   string `json:"rwx,omitempty"`
}

type Policy struct {
	ToGroup   string `json:"toGroup,omitempty"`
	FromGroup string `json:"fromGroup,omitempty"`
}

type IPOpt struct {
	Version  string `json:"ipVersion,omitempty" venice:"id=ip_ver"`
	Protocol string `json:"protocol,omitempty" venice:"id=ip_proto"`
}

type NodeSpecNodeRole int32

type NodeConditionType int32
type ConditionStatus1 int32
type NodeCondition1 struct {
	Type               NodeConditionType `json:"type,omitempty"`
	Status             ConditionStatus1  `json:"status,omitempty"`
	LastTransitionTime int64             `json:"lastTransitionTime,omitempty"`
	Reason             string            `json:"reason,omitempty"`
	Message            string            `json:"message,omitempty"`
}

type UserSpec struct {
	Aliases            string             `json:"aliases,omitempty"`
	Roles              []string           `json:"roles,omitempty"`
	DummyRoles         []string           `json:"dummyRoles,omitempty"`
	MatchLabels        map[string]string  `json:"matchLabels,omitempty"`
	AttachGroup        string             `json:"attachGroup,omitempty"`
	InRules            []SGRule           `json:"igRules,omitempty" venice:"ins=in"`
	OutRules           []SGRule           `json:"egRules,omitempty" venice:"ins=out"`
	Interval           int                `json:"interval,omitempty"`
	SkippedField       string             `json:"skippedField,omitempty" venice:"sskip"`
	Perms              *Permission        `json:"perm,omitempty"`
	Policies           map[string]Policy  `json:"policiesMap,omitempty"`
	UserHandle         uint64             `json:"userHandle,omitempty"`
	Uint32Field        uint32             `json:"uint32Field,omitempty"`
	Int32Field         int32              `json:"int32Field,omitempty"`
	SIPAddress         *string            `json:"sipAddress,omitempty" venice:"sskip"`
	MacAddrs           []*string          `json:"macAddrs,omitempty"`
	IPOpts             []*IPOpt           `json:"fwProfiles,omitempty"`
	InRulesR           []*SGRule          `json:"igRulesR,omitempty" venice:"ins=inr"`
	OutRulesR          []*SGRule          `json:"egRulesR,omitempty" venice:"ins=outr"`
	FixedRules         [2]SGRule          `json:"fixedRule,omitempty" venice:"ins=fix"`
	NodeRoles          []NodeSpecNodeRole `json:"nodeRoles,omitempty"`
	Conditions         []*NodeCondition1  `json:"conditions,omitempty"`
	BoolFlag           bool               `json:"boolFlag,omitempty"`
	FloatVal           float64            `json:"floatVal,omitempty"`
	AllocatedIPv4Addrs []byte             `json:"allocated-ipv4-addrs,omitempty" venice:"sskip"`
	CustomObj          *CustomObj         `json:"custom-obj,omitempty"`
}

type UserList struct {
	TypeMeta `json:",inline"`
	ListMeta `json:"meta"`
	Items    []User
}

type Record struct {
	Key   string `json:"key,omitempty"`
	Value string `json:"value,omitempty"`
}

type CustomObj struct {
	Records []Record `json:"records,omitempty"`
}

func (c *CustomObj) Print() string {
	pairs := []string{}
	for ii := range c.Records {
		pairs = append(pairs, fmt.Sprintf("%v=%v", c.Records[ii].Key, c.Records[ii].Value))
	}
	return strings.Join(pairs, ",")
}

func Parse(in string) (CustomObj, error) {
	var c CustomObj
	recs := strings.Split(in, ",")
	for ii := range recs {
		kvs := strings.Split(recs[ii], "=")
		if len(kvs) != 2 {
			return c, fmt.Errorf("Failed to parse %v", in)
		}
		c.Records = append(c.Records, Record{Key: kvs[0], Value: kvs[1]})
	}
	return c, nil
}

// testCustomerParser implements ref.CustomParser for CustomObj.
type testCustomParser struct {
}

func (t *testCustomParser) Print(v reflect.Value) string {
	c, ok := v.Interface().(CustomObj)
	if !ok {
		return ""
	}
	return (&c).Print()
}

func (t *testCustomParser) Parse(in string) (reflect.Value, error) {
	c, err := Parse(in)
	return reflect.ValueOf(c), err
}

func TestWalkStruct(t *testing.T) {
	expectedStr := `{
  TypeMeta: {
    Kind: string
    APIVersion: string
  }
  ObjectMeta: {
    Tenant: string
    Name: string
    Namespace: string
    ResourceVersion: string
    UUID: string
    Labels: map[string]string
  }
  Spec: {
    Aliases: string
    Roles: []string
    DummyRoles: []string
    MatchLabels: map[string]string
    AttachGroup: string
    InRules: []{
      Ports: string
      Action: string
      PeerGroup: string
    }
    OutRules: []{
      Ports: string
      Action: string
      PeerGroup: string
    }
    Interval: int
    SkippedField: string
    *Perms: {
      ToObj: string
      RWX: string
    }
    Policies: map[string]struct{
      ToGroup: string
      FromGroup: string
    }
    UserHandle: uint64
    Uint32Field: uint32
    Int32Field: int32
    *SIPAddress: string
    MacAddrs: []*string
    IPOpts: []*{
      Version: string
      Protocol: string
    }
    InRulesR: []*{
      Ports: string
      Action: string
      PeerGroup: string
    }
    OutRulesR: []*{
      Ports: string
      Action: string
      PeerGroup: string
    }
    FixedRules: [2]{
      Ports: string
      Action: string
      PeerGroup: string
    }
    {
      Ports: string
      Action: string
      PeerGroup: string
    }
    NodeRoles: []int32
    Conditions: []*{
      Type: int32
      Status: int32
      LastTransitionTime: int64
      Reason: string
      Message: string
    }
    BoolFlag: bool
    FloatVal: float64
    AllocatedIPv4Addrs: []uint8
    *CustomObj: {
      Records: []{
        Key: string
        Value: string
      }
    }
  }
}
`
	refCtx := &ref.RfCtx{GetSubObj: subObj, CustomParsers: map[string]ref.CustomParser{"*ref_test.CustomObj": &testCustomParser{}}}
	outStr := ref.WalkStruct(User{}, refCtx)
	if outStr != expectedStr {
		t.Fatalf("Out:\n--%s--\nExpected:\n--%s--\n", outStr, expectedStr)
	}
}

func TestEmptyGet(t *testing.T) {
	refCtx := &ref.RfCtx{GetSubObj: subObj, CustomParsers: map[string]ref.CustomParser{"*ref_test.CustomObj": &testCustomParser{}}}
	kvs := make(map[string]ref.FInfo)
	ref.GetKvs(User{}, refCtx, kvs)
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
	if _, ok := kvs["DummyRoles"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("DummyRoles not found in kvs")
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
	if v, ok := kvs["Interval"]; !ok || v.TypeStr != "int" {
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
	if v, ok := kvs["Uint32Field"]; !ok || v.TypeStr != "uint32" {
		printKvs("kvs", kvs, true)
		t.Fatalf("Uint32Field not found in kvs")
	}
	total++
	if v, ok := kvs["Int32Field"]; !ok || v.TypeStr != "int32" {
		printKvs("kvs", kvs, true)
		t.Fatalf("Int32Field not found in kvs")
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
	if _, ok := kvs["NodeRoles"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("NodeRoles not found in kvs")
	}
	total++
	if _, ok := kvs["Type"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("Type not found in kvs")
	}
	total++
	if _, ok := kvs["Status"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("Status not found in kvs")
	}
	total++
	if _, ok := kvs["LastTransitionTime"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("LastTransitionTime not found in kvs")
	}
	total++
	if _, ok := kvs["Reason"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("Reason not found in kvs")
	}
	total++
	if _, ok := kvs["Message"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("Message not found in kvs")
	}
	total++
	if _, ok := kvs["BoolFlag"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("BoolFlag not found in kvs")
	}
	total++
	if _, ok := kvs["FloatVal"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("FloatVal not found in kvs")
	}
	total++
	if _, ok := kvs["AllocatedIPv4Addrs"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("AllocatedIPv4Addrs not found in kvs")
	}
	total++
	if _, ok := kvs["CustomObj"]; !ok {
		printKvs("kvs", kvs, true)
		t.Fatalf("CustomObj not found in kvs")
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
	cond1 := NodeCondition1{Type: 1, Status: 2, LastTransitionTime: 90001, Reason: "some reason", Message: "some message"}
	cond2 := NodeCondition1{Type: 2, Status: 0, LastTransitionTime: 80001, Reason: "some other reason", Message: "some other message"}
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
			Policies: map[string]Policy{
				"key1": {ToGroup: "to-key1", FromGroup: "from-key1"},
				"key2": {ToGroup: "to-key2", FromGroup: "from-key2"},
			},
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
			Conditions:         []*NodeCondition1{&cond1, &cond2},
			BoolFlag:           true,
			FloatVal:           77.983,
			AllocatedIPv4Addrs: byteArray,
			CustomObj: &CustomObj{
				Records: []Record{
					{
						Key:   "k1",
						Value: "v1",
					},
					{
						Key:   "k2",
						Value: "v2",
					},
				},
			},
		},
	}
	kvs := make(map[string]ref.FInfo)
	refCtx := &ref.RfCtx{GetSubObj: subObj, CustomParsers: map[string]ref.CustomParser{"*ref_test.CustomObj": &testCustomParser{}}}
	ref.GetKvs(u, refCtx, kvs)

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
		sort.Strings(fi.ValueStr)
		if fi.Key || fi.ValueStr[0] != "dept:eng" && fi.ValueStr[1] != "level:mts" {
			printKvs("Meta", kvs, false)
			t.Fatalf("error! value not set correctly %+v", fi)
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
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "storage-admin" || fi.ValueStr[1] != "security-admin" {
			printKvs("Spec", kvs, false)
			t.Fatalf("error! value not set correctly")
		}
	} else {
		t.Fatalf("Roles not found in kvs")
	}

	if fi, ok := kvs["MatchLabels"]; ok {
		sort.Strings(fi.ValueStr)
		if fi.Key || fi.ValueStr[0] != "color:purple" || fi.ValueStr[1] != "io.pensando.area:network" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! labels fetched %+v  not set correctly", fi)
		}
	} else {
		t.Fatalf("MatchLabels not found in kvs")
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

	if fi, ok := kvs["in_Action"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "permit,log" || fi.ValueStr[1] != "permit" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! in_action fetched '%s' not set correctly", fi.ValueStr)
		}
	} else {
		printKvs("spec", kvs, false)
		t.Fatalf("in_Action not found, kvs %+v ", kvs)
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
			t.Fatalf("error! handle %v not found '%v'", handle, fi.ValueStr)
		}
	} else {
		t.Fatalf("UserHandle not found")
	}

	if fi, ok := kvs["Int32Field"]; ok {
		handle, _ := strconv.Atoi(fi.ValueStr[0])
		if fi.Key || len(fi.ValueStr) != 1 || handle != 8832 {
			printKvs("spec", kvs, false)
			t.Fatalf("error! Int32Field %d not found '%v'", handle, fi.ValueStr)
		}
	} else {
		t.Fatalf("Int32Field not found")
	}

	if fi, ok := kvs["Uint32Field"]; ok {
		handle, _ := strconv.ParseUint(fi.ValueStr[0], 10, 32)
		if fi.Key || len(fi.ValueStr) != 1 || handle != 9821 {
			printKvs("spec", kvs, false)
			t.Fatalf("error! Uint32Field %v not found '%v'", handle, fi.ValueStr)
		}
	} else {
		t.Fatalf("Uint32Field not found")
	}

	if fi, ok := kvs["SIPAddress"]; ok {
		if fi.Key || !fi.SSkip || len(fi.ValueStr) != 1 || fi.ValueStr[0] != ip1 {
			printKvs("spec", kvs, false)
			t.Fatalf("error! SIPAddress %v not found '%v'", ip1, fi.ValueStr)
		}
	} else {
		t.Fatalf("SIPAddress not found")
	}

	if fi, ok := kvs["MacAddrs"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != mac1 && fi.ValueStr[1] != mac2 {
			printKvs("spec", kvs, false)
			t.Fatalf("error! MacAddrs %s,%s not found '%v'", mac1, mac2, fi.ValueStr)
		}
	} else {
		t.Fatalf("MacAddrs not found")
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

	if fi, ok := kvs["inr_Action"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "permittt" || fi.ValueStr[1] != "permittt" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! inr_Action %s,%s not found '%v'", inr1, inr2, fi.ValueStr)
		}
	} else {
		t.Fatalf("inr_Action not found")
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

	if fi, ok := kvs["outr_Action"]; ok {
		if fi.Key || len(fi.ValueStr) != 1 || fi.ValueStr[0] != "dennny" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! outr_Action %s not found '%v'", outr1, fi.ValueStr)
		}
	} else {
		t.Fatalf("outr_Action not found")
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

	if fi, ok := kvs["fix_Action"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "permit,log" || fi.ValueStr[1] != "permit,log" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! fix_Action not found '%v'", fi.ValueStr)
		}
	} else {
		t.Fatalf("fix_Action not found")
	}

	if fi, ok := kvs["NodeRoles"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "997" || fi.ValueStr[1] != "799" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! NodeRoles not found '%v'", fi.ValueStr)
		}
	} else {
		t.Fatalf("NodeRoles not found")
	}

	if fi, ok := kvs["Type"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "1" || fi.ValueStr[1] != "2" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! Condition Type not found '%v'", fi.ValueStr)
		}
	} else {
		t.Fatalf("Condition Type not found")
	}
	if fi, ok := kvs["Status"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "2" || fi.ValueStr[1] != "0" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! Condition Status not found '%v'", fi.ValueStr)
		}
	} else {
		t.Fatalf("Condition Status not found")
	}
	if fi, ok := kvs["LastTransitionTime"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "90001" || fi.ValueStr[1] != "80001" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! Condition LastTransitionTime not found '%v'", fi.ValueStr)
		}
	} else {
		t.Fatalf("Condition LastTransitionTime not found")
	}
	if fi, ok := kvs["Reason"]; ok {
		if fi.Key || len(fi.ValueStr) != 2 || fi.ValueStr[0] != "some reason" || fi.ValueStr[1] != "some other reason" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! Condition Reason not found '%v'", fi.ValueStr)
		}
	} else {
		t.Fatalf("Condition Reason not found")
	}

	if fi, ok := kvs["BoolFlag"]; ok {
		if fi.Key || len(fi.ValueStr) != 1 || fi.ValueStr[0] != "true" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! Condition Reason not found '%v'", fi.ValueStr)
		}
	} else {
		t.Fatalf("BoolFlag not found")
	}

	if fi, ok := kvs["FloatVal"]; ok {
		if fi.Key || len(fi.ValueStr) != 1 || fi.ValueStr[0] != "77.983" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! FloatVal not found '%v'", fi.ValueStr)
		}
	} else {
		t.Fatalf("FloatVal not found")
	}

	if fi, ok := kvs["AllocatedIPv4Addrs"]; ok {
		if !fi.SSkip || fi.Key || len(fi.ValueStr) != 9 {
			printKvs("spec", kvs, false)
			t.Fatalf("error! Invalid AllocatedIPv4Addrs found '%+v'", fi)
		}
	} else {
		t.Fatalf("AllocatedIPv4Addrs not found")
	}

	if fi, ok := kvs["CustomObj"]; ok {
		if fi.ValueStr[0] != "k1=v1,k2=v2" {
			printKvs("spec", kvs, false)
			t.Fatalf("error! Invalid CustomObj found '%+v'", fi)
		}
	} else {
		t.Fatalf("CustomObj not found")
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
	cond1 := NodeCondition1{Type: 1, Status: 2, LastTransitionTime: 90001, Reason: "some reason", Message: "some message"}
	cond2 := NodeCondition1{Type: 2, Status: 0, LastTransitionTime: 80001, Reason: "some other reason", Message: "some other message"}
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
			Policies: map[string]Policy{
				"key1": {ToGroup: "to-key1", FromGroup: "from-key1"},
				"key2": {ToGroup: "to-key2", FromGroup: "from-key2"},
			},
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
			Conditions: []*NodeCondition1{&cond1, &cond2},
			BoolFlag:   false,
			FloatVal:   11.322,
			CustomObj: &CustomObj{
				Records: []Record{
					{
						Key:   "k1",
						Value: "v1",
					},
					{
						Key:   "k2",
						Value: "v2",
					},
				},
			},
		},
	}
	kvs := make(map[string]ref.FInfo)
	kvs["Name"] = ref.NewFInfo([]string{"joe"})
	kvs["Roles"] = ref.NewFInfo([]string{"blah-blah"})
	kvs["ToGroup"] = ref.NewFInfo([]string{"key1:new-to-key1", "key3:to-key3"})
	kvs["FromGroup"] = ref.NewFInfo([]string{"key1:new-from-key1", "key3:from-key3"})
	kvs["SIPAddress"] = ref.NewFInfo([]string{"10.1.1.2"})
	kvs["MacAddrs"] = ref.NewFInfo([]string{"00.11.22.33.44.55", "00.22.33.44.55.66"})
	kvs["ip_ver"] = ref.NewFInfo([]string{"ipv4", "ipv6"})
	kvs["ip_proto"] = ref.NewFInfo([]string{"udper", "tcper"})
	kvs["outr_port"] = ref.NewFInfo([]string{"tcp/4411", "tcp/2121"})
	kvs["outr_Action"] = ref.NewFInfo([]string{"permit", "permit"})
	kvs["outr_peer_group"] = ref.NewFInfo([]string{"g5", "g6"})
	kvs["fix_port"] = ref.NewFInfo([]string{"tcp/5555", "icmp/echo"})
	kvs["fix_Action"] = ref.NewFInfo([]string{"action-one", "action-two"})
	kvs["fix_peer_group"] = ref.NewFInfo([]string{"group-foo", "group-bar"})
	kvs["NodeRoles"] = ref.NewFInfo([]string{"455", "544", "5544"})
	kvs["Status"] = ref.NewFInfo([]string{"3", "4"})
	kvs["Type"] = ref.NewFInfo([]string{"9", "9"})
	kvs["Reason"] = ref.NewFInfo([]string{"new reason", "another new reason"})
	kvs["Message"] = ref.NewFInfo([]string{"one", "two"})
	kvs["LastTransitionTime"] = ref.NewFInfo([]string{"666666", "7777777"})
	kvs["BoolFlag"] = ref.NewFInfo([]string{"true"})
	kvs["FloatVal"] = ref.NewFInfo([]string{"901.019"})
	kvs["CustomObj"] = ref.NewFInfo([]string{"k1=x1,k2=x2"})

	refCtx := &ref.RfCtx{GetSubObj: subObj, CustomParsers: map[string]ref.CustomParser{"*ref_test.CustomObj": &testCustomParser{}}}
	newObj := ref.WriteKvs(u, refCtx, kvs)
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

	if len(newUser.Spec.Policies) != 2 {
		fmt.Printf("unable to write Policies: %+v \n", newUser)
		t.Fatalf("unable to write Policies pointer to struct with tags")
	} else if newUser.Spec.Policies["key1"].ToGroup != "new-to-key1" || newUser.Spec.Policies["key3"].ToGroup != "to-key3" {
		fmt.Printf("unable to write Policies: %+v \n", newUser)
		t.Fatalf("unable to write Policies pointer to struct with tags")
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

	if newUser.Spec.CustomObj.Print() != "k1=x1,k2=x2" {
		fmt.Printf("newCustomObj: %+v\n\n", newUser.Spec.CustomObj)
		t.Fatalf("unable to write CustomObj")
	}

}

func TestNewWrite(t *testing.T) {
	kvs := make(map[string]ref.FInfo)
	kvs["Name"] = ref.NewFInfo([]string{"joe"})
	kvs["Labels"] = ref.NewFInfo([]string{"dept:eng", "level:mts"})
	kvs["Roles"] = ref.NewFInfo([]string{"storage-admin", "security-admin"})
	kvs["Aliases"] = ref.NewFInfo([]string{"jhonny"})
	kvs["MatchLabels"] = ref.NewFInfo([]string{"io.pensando.area:network", "color:purple"})
	kvs["Interval"] = ref.NewFInfo([]string{"4554"})
	kvs["in_port"] = ref.NewFInfo([]string{"tcp/80,tcp/443", "tcp/6379"})
	kvs["out_port"] = ref.NewFInfo([]string{"tcp/8181"})
	kvs["in_Action"] = ref.NewFInfo([]string{"permit,log", "permit,log"})
	kvs["in_peer_group"] = ref.NewFInfo([]string{"web-sg", "db-sg"})
	kvs["ToGroup"] = ref.NewFInfo([]string{"key1:to-key1", "key2:to-key2"})
	kvs["FromGroup"] = ref.NewFInfo([]string{"key1:from-key1", "key2:from-key2"})
	kvs["junk"] = ref.NewFInfo([]string{"web-sg", "db-sg"})
	kvs["ToObj"] = ref.NewFInfo([]string{"sgpolicy"})
	kvs["RWX"] = ref.NewFInfo([]string{"rw"})
	kvs["UserHandle"] = ref.NewFInfo([]string{"0x45544422"})
	kvs["Uint32Field"] = ref.NewFInfo([]string{"9823"})
	kvs["Int32Field"] = ref.NewFInfo([]string{"7473"})
	kvs["SIPAddress"] = ref.NewFInfo([]string{"10.1.1.2"})
	kvs["MacAddrs"] = ref.NewFInfo([]string{"00.11.22.33.44.55", "00.22.33.44.55.66"})
	kvs["ip_ver"] = ref.NewFInfo([]string{"ipv4", "ipv6"})
	kvs["ip_proto"] = ref.NewFInfo([]string{"udper", "tcper"})
	kvs["inr_port"] = ref.NewFInfo([]string{"tcp/221", "tcp/222"})
	kvs["inr_Action"] = ref.NewFInfo([]string{"permittt", "permittt"})
	kvs["inr_peer_group"] = ref.NewFInfo([]string{"g2", "g1"})
	kvs["outr_port"] = ref.NewFInfo([]string{"tcp/4411", "tcp/2121"})
	kvs["outr_Action"] = ref.NewFInfo([]string{"permit", "permit"})
	kvs["outr_peer_group"] = ref.NewFInfo([]string{"g5", "g6"})
	kvs["fix_port"] = ref.NewFInfo([]string{"tcp/5555", "icmp/echo"})
	kvs["fix_Action"] = ref.NewFInfo([]string{"action-one", "action-two"})
	kvs["fix_peer_group"] = ref.NewFInfo([]string{"group-foo", "group-bar"})
	kvs["NodeRoles"] = ref.NewFInfo([]string{"455", "544", "5544"})
	kvs["Status"] = ref.NewFInfo([]string{"3", "4"})
	kvs["Type"] = ref.NewFInfo([]string{"9", "9"})
	kvs["Reason"] = ref.NewFInfo([]string{"new reason", "another new reason"})
	kvs["Message"] = ref.NewFInfo([]string{"one", "two"})
	kvs["LastTransitionTime"] = ref.NewFInfo([]string{"666666", "7777777"})
	kvs["BoolFlag"] = ref.NewFInfo([]string{"true"})
	kvs["FloatVal"] = ref.NewFInfo([]string{"901.109"})
	kvs["CustomObj"] = ref.NewFInfo([]string{"k1=v1,k2=v2"})

	refCtx := &ref.RfCtx{GetSubObj: subObj, CustomParsers: map[string]ref.CustomParser{"*ref_test.CustomObj": &testCustomParser{}}}
	newObj := ref.WriteKvs(User{}, refCtx, kvs)
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

	if len(newUser.Spec.Policies) != 2 {
		fmt.Printf("unable to write Policies: %+v \n", newUser)
		t.Fatalf("unable to write Policies pointer to struct with tags")
	} else if newUser.Spec.Policies["key1"].ToGroup != "to-key1" || newUser.Spec.Policies["key2"].ToGroup != "to-key2" {
		fmt.Printf("unable to write Policies: %+v \n", newUser)
		t.Fatalf("unable to write Policies pointer to struct with tags")
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

	if newUser.Spec.CustomObj.Print() != "k1=v1,k2=v2" {
		fmt.Printf("newUser: %+v\n\n", newUser)
		t.Fatalf("unable to write CustomObj")
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
	cond1 := NodeCondition1{Type: 1, Status: 2, LastTransitionTime: 90001, Reason: "some reason", Message: "some message"}
	cond2 := NodeCondition1{Type: 2, Status: 0, LastTransitionTime: 80001, Reason: "some other reason", Message: "some other message"}
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
			Policies: map[string]Policy{
				"key1": {ToGroup: "to-key1", FromGroup: "from-key1"},
				"key2": {ToGroup: "to-key2", FromGroup: "from-key2"},
			},
			UserHandle:  0x45544422,
			Uint32Field: 9823,
			Int32Field:  5351,
			SIPAddress:  &ip1,
			MacAddrs:    []*string{&mac1, &mac2},
			IPOpts:      []*IPOpt{&ipOpt1, &ipOpt2},
			InRulesR:    []*SGRule{&inr1, &inr2},
			OutRulesR:   []*SGRule{&outr1},
			FixedRules: [2]SGRule{
				{Ports: "tcp/80,tcp/443", Action: "permit,log", PeerGroup: "fix-sg1"},
				{Ports: "udp/80", Action: "permit,log", PeerGroup: "fix-sg2"},
			},
			NodeRoles:  []NodeSpecNodeRole{997, 799},
			Conditions: []*NodeCondition1{&cond1, &cond2},
			BoolFlag:   true,
			FloatVal:   782.1,
		},
	}
	user := reflect.ValueOf(u)
	meta := reflect.ValueOf(u.ObjectMeta)
	spec := reflect.ValueOf(u.Spec)

	name := ref.FieldByName(meta, "Name")
	if len(name) != 1 || name[0] != "joe" {
		t.Fatalf("Invalid name returned '%s'", name)
	}

	roles := ref.FieldByName(spec, "Roles")
	if len(roles) != 2 || roles[0] != "storage-admin" || roles[1] != "security-admin" {
		t.Fatalf("Invalid roles returned '%s'", roles)
	}

	ports := ref.FieldByName(user, "Spec.OutRules.Ports")
	if len(ports) != 2 || ports[0] != "tcp/6379" || ports[1] != "tcp/5505" {
		t.Fatalf("Invalid roles returned '%s'", ports)
	}

	matchLabels := ref.FieldByName(user, "Spec.MatchLabels")
	if len(matchLabels) != 2 ||
		(matchLabels[0] != "io.pensando.area:network" ||
			matchLabels[1] != "io.pensando.network.priority:critical") &&
			(matchLabels[1] != "io.pensando.area:network" ||
				matchLabels[0] != "io.pensando.network.priority:critical") {
		t.Fatalf("Invalid matchLabels: %s", matchLabels)
	}

	ghosts := ref.FieldByName(user, "Spec.OutRules.Ghosts")
	if len(ghosts) != 0 {
		t.Fatalf("Invalid ghosts returned '%s'", ghosts)
	}

	perms := ref.FieldByName(user, "Spec.Perms.ToObj")
	if len(perms) != 1 || perms[0] != "network" {
		t.Fatalf("Invalid perms returned '%s'", perms)
	}

	sip := ref.FieldByName(user, "Spec.SIPAddress")
	if len(sip) != 1 || sip[0] != ip1 {
		t.Fatalf("Invalid sip returned '%s'", sip)
	}

	macAddrs := ref.FieldByName(user, "Spec.MacAddrs")
	if len(macAddrs) != 2 || macAddrs[0] != mac1 || macAddrs[1] != mac2 {
		t.Fatalf("Invalid macAddrs returned '%s'", macAddrs)
	}

	ipOptsProto := ref.FieldByName(user, "Spec.IPOpts.Protocol")
	if len(ipOptsProto) != 2 || ipOptsProto[0] != "tcp" || ipOptsProto[1] != "udp" {
		t.Fatalf("Invalid ipOptsProto returned '%s'", ipOptsProto)
	}

	inrPorts := ref.FieldByName(user, "Spec.InRulesR.Ports")
	if len(inrPorts) != 2 || inrPorts[0] != "tcp/221" || inrPorts[1] != "tcp/222" {
		t.Fatalf("Invalid inrPorts %s \n", inrPorts)
	}

	fixPgs := ref.FieldByName(user, "Spec.FixedRules.PeerGroup")
	if len(fixPgs) != 2 || fixPgs[0] != "fix-sg1" || fixPgs[1] != "fix-sg2" {
		t.Fatalf("Invalid fixPgs %s \n", fixPgs)
	}

	nodeRoles := ref.FieldByName(user, "Spec.NodeRoles")
	if len(nodeRoles) != 2 || nodeRoles[0] != "997" || nodeRoles[1] != "799" {
		t.Fatalf("Invalid fixPgs %s \n", fixPgs)
	}

	condReasons := ref.FieldByName(user, "Spec.Conditions.Reason")
	if len(condReasons) != 2 || condReasons[0] != "some reason" || condReasons[1] != "some other reason" {
		t.Fatalf("Invalid condReasons %s \n", condReasons)
	}

	transitionTimes := ref.FieldByName(user, "Spec.Conditions.LastTransitionTime")
	if len(transitionTimes) != 2 || transitionTimes[0] != "90001" || transitionTimes[1] != "80001" {
		t.Fatalf("Invalid transitionTimes %s \n", transitionTimes)
	}

	boolFlag := ref.FieldByName(user, "Spec.BoolFlag")
	if len(boolFlag) != 1 || boolFlag[0] != "true" {
		t.Fatalf("Invalid boolFlag fetched %s \n", boolFlag)
	}

	floatVal := ref.FieldByName(user, "Spec.FloatVal")
	if len(floatVal) != 1 || floatVal[0] != "782.1" {
		t.Fatalf("Invalid transitionTimes %s \n", transitionTimes)
	}
}

func printKvs(hdr string, kvs map[string]ref.FInfo, onlyKey bool) {
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
		var v SGRule
		return &v
	case "Permission":
		var v Permission
		return &v
	case "Policy":
		var v Policy
		return &v
	case "IPOpt":
		var v IPOpt
		return &v
	case "NodeSpecNodeRole":
		var v NodeSpecNodeRole
		return &v
	case "NodeCondition1":
		var v NodeCondition1
		return &v
	case "CustomObj":
		var v CustomObj
		return &v
	case "Record":
		var v Record
		return &v
	}

	return nil
}

func TestParseVal(t *testing.T) {
	tests := []struct {
		kind       reflect.Kind
		value      string
		expSuccess bool
	}{
		{reflect.Bool, "true", true},
		{reflect.Bool, "false", true},
		{reflect.Bool, "foo", false},
		{reflect.Float32, "100.01", true},
		{reflect.Float32, "100", true},
		{reflect.Float32, "7893894732984723984792387493287493274923749237493274.342342", false},
		{reflect.Float64, "100.01", true},
		{reflect.Float64, "7893894732984723984792387493287493274923749237493274.342342", true},
		{reflect.Int8, "127", true},
		{reflect.Int8, "-127", true},
		{reflect.Int16, "128", true},
		{reflect.Int16, "32767", true},
		{reflect.Int16, "-32767", true},
		{reflect.Int16, "32768", false},
		{reflect.Int32, "32768", true},
		{reflect.Int32, "2147483647", true},
		{reflect.Int32, "-2147483647", true},
		{reflect.Int32, "2147483648", false},
		{reflect.Int64, "2147483648", true},
		{reflect.Int64, "-2147483648", true},
		{reflect.Uint8, "255", true},
		{reflect.Uint8, "-127", false},
		{reflect.Uint8, "256", false},
		{reflect.Uint16, "256", true},
		{reflect.Uint16, "65535", true},
		{reflect.Uint16, "-32767", false},
		{reflect.Uint16, "65536", false},
		{reflect.Uint32, "65536", true},
		{reflect.Uint32, "4294967295", true},
		{reflect.Uint32, "-2147483647", false},
		{reflect.Uint32, "4294967296", false},
		{reflect.Uint64, "4294967296", true},
		{reflect.Uint64, "-2147483648", false},
		{reflect.Complex64, "213312312", false},
		{reflect.Complex128, "213312312", false},
	}
	for ii := range tests {
		_, err := ref.ParseVal(tests[ii].kind, tests[ii].value)
		if tests[ii].expSuccess && err != nil {
			t.Fatalf("Expected to succeed but failed, kind %v, value %v, err: %v", tests[ii].kind, tests[ii].value, err)
		}
		if !tests[ii].expSuccess && err == nil {
			t.Fatalf("Expected to fail but succeeded, kind %v, value %v", tests[ii].kind, tests[ii].value)
		}
	}
}

// These structs are temporary until ref can handle nested maps/slices.
type Attribute struct {
	Attr string `json:"attr,omitempty"`
}

type PolicyRule struct {
	Something string `json:"something,omitempty"`
}

type NewPolicy struct {
	ToGroup       string               `json:"toGroup,omitempty"`
	FromGroup     string               `json:"fromGroup,omitempty"`
	Attributes    map[string]Attribute `json:"attrs,omitempty"`
	Rules         []PolicyRule         `json:"rules,omitempty"`
	StrAttributes map[string]string    `json:"strAttrs,omitempty"`
}

type RuleAttribute struct {
	Attr string `json:"attr,omitempty"`
}

type NewSGRule struct {
	Action     string               `json:"action,omitempty"`
	Attributes map[string]Attribute `json:"attrs,omitempty"`
}

type NewUserSpec struct {
	Alias          string               `json:"alias,omitempty"`
	Roles          []string             `json:"roles,omitempty"`
	Perm           *Permission          `json:"perm,omitempty"`
	NilPerm        *Permission          `json:"nilPerm,omitempty"`
	PolicyMap      map[string]NewPolicy `json:"policiesMap,omitempty"`
	NilPolicyMap   map[string]NewPolicy `json:"nilPoliciesMap,omitempty"`
	PolicyIdxMap   map[int8]NewPolicy   `json:"policiesIndexMap,omitempty"`
	PolicySlice    []NewPolicy          `json:"policiesSlice,omitempty"`
	NilPolicySlice []NewPolicy          `json:"nilPoliciesSlice,omitempty"`
	InRules        []NewSGRule          `json:"igRules,omitempty" venice:"ins=in"`
}

type NewUser struct {
	Spec *NewUserSpec `json:"spec,omitempty"`
}

func TestFieldValues(t *testing.T) {
	u := NewUser{
		Spec: &NewUserSpec{
			Roles: []string{"foo", "bar"},
			Perm:  &Permission{},
			PolicyMap: map[string]NewPolicy{
				"key1": {
					ToGroup:       "to-key1",
					FromGroup:     "from-key1",
					Attributes:    map[string]Attribute{"foo": {"foo"}, "bar": {"bar"}},
					StrAttributes: map[string]string{"foo": "bar"},
					Rules:         []PolicyRule{{"a"}, {"b"}},
				},
				"key2": {
					ToGroup:    "to-key2",
					FromGroup:  "from-key2",
					Attributes: map[string]Attribute{"bas": {"bas"}, "baz": {"baz"}},
				},
			},
			PolicyIdxMap: map[int8]NewPolicy{
				10: {
					ToGroup:       "to-key1",
					FromGroup:     "from-key1",
					Attributes:    map[string]Attribute{"foo": {"bar"}},
					StrAttributes: map[string]string{"foo": "bar"},
					Rules:         []PolicyRule{{"a"}, {"b"}},
				},
			},
			PolicySlice: []NewPolicy{
				{
					ToGroup:       "to-key1",
					FromGroup:     "from-key1",
					Attributes:    map[string]Attribute{"foo": {"bar"}},
					StrAttributes: map[string]string{"foo": "bar"},
					Rules:         []PolicyRule{{"a"}, {"b"}},
				},
			},
			InRules: []NewSGRule{NewSGRule{Action: "permit", Attributes: map[string]Attribute{"foo": {"bar"}}}},
		},
	}
	v := reflect.ValueOf(u)
	tests := []struct {
		key        string
		expSuccess bool
		expValues  []string
	}{
		{"Spec.Alias", true, []string{""}},                                                   // Uninitialized field
		{"Spec.Roles", true, []string{"foo", "bar"}},                                         // Slice of strings
		{"Spec.Perm.RWX", true, []string{""}},                                                // A single nested uninitialized field
		{"Spec.NilPerm.RWX", true, []string{}},                                               // Nil ptr
		{"Spec.PolicyMap[key1]", false, []string{""}},                                        // Non leaf
		{"Spec.PolicyMap[key1].Rules.Something", true, []string{"a", "b"}},                   // Map with a specific key
		{"Spec.PolicyMap[*].Attributes[*].Attr", true, []string{"foo", "bar", "bas", "baz"}}, // Map with *
		{"Spec.PolicyMap[*].Rules.Something", true, []string{"a", "b"}},                      // Map with *
		{"Spec.PolicyMap[key2].Rules.Something", true, []string{}},                           // Map with non existent key
		{"Spec.PolicyMap[key1].ToGroup", true, []string{"to-key1"}},                          // Map with a specific key
		{"Spec.NilPolicyMap[key1].Rules.Something", true, []string{}},                        // Nil map
		{"Spec.InRules.Attributes[foo].Attr", true, []string{"bar"}},                         // Map with specific key
		{"Spec.InRules.Attributes[*].Attr", true, []string{"bar"}},                           // Map with *
		{"Spec.InRules.Attributes[foo1].Attr", true, []string{}},                             // Map with non existent key
		{"Spec.PolicyIdxMap[10].StrAttributes[foo]", true, []string{"bar"}},                  // Leap map with specific key
		{"Spec.PolicyIdxMap[10].StrAttributes[*]", true, []string{"bar"}},                    // Leap map with *
		{"Spec.PolicyIdxMap[*].StrAttributes[*]", true, []string{"bar"}},                     // Multiple *
		{"Spec.PolicyIdxMap[100].StrAttributes[*]", true, []string{}},                        // Non existent key
		{"Spec.PolicyIdxMap[*].StrAttributes[fool]", true, []string{}},                       // Non existent key
		{"Spec.PolicySlice.Rules", false, []string{}},                                        // Non leaf
		{"Spec.PolicySlice.Rules.Something", true, []string{"a", "b"}},                       // Slice
		{"Spec.NilPolicySlice.Rules.Something", true, []string{}},                            // Nil Slice
		{"Spec.InRules.Action", true, []string{"permit"}},                                    // Slice
		{"Spec.ABC.DEF", false, []string{}},                                                  // Non existent field
		{"Spec.Perm.DEF", false, []string{}},                                                 // Non existent field
		{"Spec.InRules.Attributes[foo].ABC", false, []string{}},                              // Non existent field
		{"superrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrlooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooongone", false, []string{}}, // Long field
	}
	for ii := range tests {
		values, err := ref.FieldValues(v, tests[ii].key)
		if !tests[ii].expSuccess && err == nil {
			t.Fatalf("Expected %v to fail, found %v", tests[ii].key, values)
		}
		if tests[ii].expSuccess {
			if len(values) != len(tests[ii].expValues) {
				t.Fatalf("Expected %v values, found %v values for %v", len(tests[ii].expValues), len(values), tests[ii].key)
			}
			valuesMap := make(map[string]struct{})
			for jj := range tests[ii].expValues {
				valuesMap[tests[ii].expValues[jj]] = struct{}{}
			}
			for jj := range values {
				if _, ok := valuesMap[values[jj]]; !ok {
					t.Fatalf("Expected %v values, found %v values", tests[ii].expValues, values)
				}
			}
		}
	}
}

func TestFieldByJSONTag(t *testing.T) {
	tests := []struct {
		kind       string
		jsonTag    string
		expSuccess bool
		expField   string
	}{
		{"cluster.Cluster", "spec.quorum-nodes", true, "Spec.QuorumNodes"}, // A single nested field
		{"cluster.Cluster", "spec.virtual-ip", true, "Spec.VirtualIP"},
		{"cluster.Cluster", "spec.auto-admit-dscs", true, "Spec.AutoAdmitDSCs"},
		{"cluster.Cluster", "status.last-leader-transition-time", true, "Status.LastLeaderTransitionTime"},
		{"cluster.Node", "status.conditions[*].status", false, "Status.Conditions[*].Status"}, // Slice cant be indexed
		{"cluster.Node", "status.conditions.status", true, "Status.Conditions.Status"},
		{"cluster.Host", "spec.smartnics[*].mac-addrs", false, "Spec.SmartNICs[*].MacAddrs"}, // Slice cant be indexed
	}
	for ii := range tests {
		field, err := ref.FieldByJSONTag(tests[ii].kind, tests[ii].jsonTag)
		if !tests[ii].expSuccess && err == nil {
			t.Fatalf("Expected %v to fail, found %v", tests[ii].jsonTag, field)
		}
		if tests[ii].expSuccess && field != tests[ii].expField {
			t.Fatalf("Expected %v for %v, found %v with err %v", tests[ii].expField, tests[ii].jsonTag, field, err)
		}
	}
}

func TestParseableVal(t *testing.T) {
	tests := []struct {
		kind       string
		value      string
		expSuccess bool
	}{
		{"TYPE_STRING", "abc", true},
		{"TYPE_BOOL", "true", true},
		{"TYPE_BOOL", "abc", false},
		{"TYPE_FLOAT", "100.10", true},
		{"TYPE_FLOAT", "abc", false},
		{"TYPE_INT64", "123123213123", true},
		{"TYPE_INT64", "-123123213123", true},
		{"TYPE_UINT64", "123123213123", true},
		{"TYPE_UINT64", "-123123213123", false},
		{"TYPE_INT32", "123123", true},
		{"TYPE_INT32", "4294967296", false},
		{"TYPE_INT32", "-123123", true},
		{"TYPE_UINT32", "123123", true},
		{"TYPE_UINT32", "4294967296", false},
		{"TYPE_UINT32", "-123123", false},
		{"api.Timestamp", "2018-11-09T23:16:17Z", true},
		{"api.Timestamp", "12345", false},
	}
	for ii := range tests {
		if found := ref.ParseableVal(tests[ii].kind, tests[ii].value); found != tests[ii].expSuccess {
			t.Fatalf("Expected %v for kind %v, val %v, found: %v", tests[ii].expSuccess, tests[ii].kind, tests[ii].value, found)
		}
	}
}

func TestGetFieldType(t *testing.T) {
	tests := []struct {
		kind       string
		key        string
		expResp    string
		expSuccess bool
	}{
		{"cluster.Cluster", "Status", "cluster.ClusterStatus", true},
		{"cluster.Node", "Spec", "cluster.NodeSpec", true},
		{"network.Network", "Spec", "network.NetworkSpec", true},
		{"network.Network", "Spec.Type", "TYPE_STRING", true},
		{"network.Network", "Spec.VlanID", "TYPE_UINT32", true},
		{"network.Network", "Status.Workloads", "TYPE_STRING", true}, // slice of strings
		{"network.Network", "Status.AllocatedIPv4Addrs", "TYPE_BYTES", true},
		{"cluster.Invalid", "Invalid", "", false},
	}

	for ii := range tests {
		ty, _ := ref.GetFieldType(tests[ii].kind, tests[ii].key)
		if tests[ii].expSuccess && tests[ii].expResp != ty {
			t.Fatalf("expected: %v for kind %v, key %v, got: %v", tests[ii].expResp, tests[ii].kind, tests[ii].key, ty)
		}
	}
}

func TestGetScalarFieldType(t *testing.T) {
	tests := []struct {
		kind       string
		key        string
		expResp    string
		expSuccess bool
	}{
		{"network.Network", "Spec.Type", "TYPE_STRING", true},
		{"network.Network", "Spec.VlanID", "TYPE_UINT32", true},
		{"network.Network", "Status.Workloads", "TYPE_STRING", true}, // slice of strings
		{"network.Network", "Status.AllocatedIPv4Addrs", "TYPE_BYTES", true},
		{"cluster.Node", "Status.Quorum", "TYPE_BOOL", true},
		{"bookstore.Book", "Spec.UpdateTimestamp", "api.Timestamp", true},
		{"cluster.Cluster", "Status", "cluster.ClusterStatus", false},
		{"cluster.Node", "Spec", "cluster.NodeSpec", false},
		{"network.Network", "Spec", "network.NetworkSpec", false},
		{"cluster.Invalid", "Invalid", "", false},
	}

	for ii := range tests {
		ty, _ := ref.GetScalarFieldType(tests[ii].kind, tests[ii].key)
		if tests[ii].expSuccess && tests[ii].expResp != ty {
			t.Fatalf("expected: %v for kind %v, key %v, got: %v", tests[ii].expResp, tests[ii].kind, tests[ii].key, ty)
		}
	}
}
