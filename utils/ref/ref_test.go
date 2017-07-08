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
	Tenant          string            `json:"tenant,omitempty"`
	Name            string            `json:"name,omitempty"`
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
	Ports     string `json:"ports,omitempty"`
	Action    string `json:"action,omitempty"`
	PeerGroup string `json:"sourceGroup,omitempty"`
}

type Permission struct {
	ToObj string `json:"toObj,omitempty"`
	RWX   string `json:"rwx, omitempty"`
}

type UserSpec struct {
	Aliases      string            `json:"aliases,omitempty"`
	Roles        []string          `json:"roles,omitempty"`
	MatchLabels  map[string]string `json:"matchLabels,omitempty"`
	AttachGroup  string            `json:"attachGroup,omitempty"`
	InRules      []SGRule          `json:"igRules,omitempty"`
	OutRules     []SGRule          `json:"egRules,omitempty"`
	Interval     int               `json:"interval,omitempty"`
	SkippedField string            `json:"uuid,omitempty" venice:"sskip"`
	Perms        *Permission       `json:"perm,omitempty"`
}

type UserList struct {
	TypeMeta `json:",inline"`
	ListMeta `json:"meta"`
	Items    []User
}

func printKvs(hdr string, kvs map[string]FInfo) {
	fmt.Printf("%s:\n", hdr)
	for key, fi := range kvs {
		if len(fi.ValueStr) != 0 && fi.ValueStr[0] != "" {
			fmt.Printf("key '%s', val '%s'\n", key, fi.ValueStr)
		}
	}
}

func TestGet(t *testing.T) {
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
			},
			OutRules: []SGRule{
				{Ports: "tcp/6379", Action: "permit,log", PeerGroup: "db-sg"},
			},
			Interval: 33,
			Perms:    &Permission{ToObj: "network", RWX: "rw"},
		},
	}
	meta := reflect.Indirect(reflect.ValueOf(&u.ObjectMeta))
	spec := reflect.Indirect(reflect.ValueOf(&u.Spec))

	metaKvs := make(map[string]FInfo)
	specKvs := make(map[string]FInfo)
	GetKvs(meta, subObj, metaKvs)
	GetKvs(spec, subObj, specKvs)

	if fi, ok := metaKvs["Name"]; ok {
		if fi.ValueStr[0] != "joe" {
			printKvs("Meta", metaKvs)
			t.Fatalf("error! value not set correctly")
		}
	} else {
		t.Fatalf("Name not found in kvs")
	}

	if fi, ok := metaKvs["Labels"]; ok {
		if fi.ValueStr[0] != "dept:eng,level:mts" && fi.ValueStr[0] != "level:mts,dept:eng" {
			printKvs("Meta", metaKvs)
			t.Fatalf("error! value not set correctly")
		}
	} else {
		t.Fatalf("Labels not found in kvs")
	}

	if fi, ok := specKvs["Aliases"]; ok {
		if fi.ValueStr[0] != "jhonny" {
			t.Fatalf("error! value is not set correctly")
		}
	} else {
		t.Fatalf("error finding key")
	}

	if fi, ok := specKvs["Roles"]; ok {
		if fi.ValueStr[0] != "security-damin,storage-admin" &&
			fi.ValueStr[0] != "storage-admin,security-admin" {
			printKvs("Spec", specKvs)
			t.Fatalf("error! value not set correctly")
		}
	} else {
		t.Fatalf("Roles not found in kvs")
	}

	if fi, ok := specKvs["MatchLabels"]; ok {
		if fi.ValueStr[0] != "io.pensando.area:network" {
			printKvs("spec", specKvs)
			t.Fatalf("error! labels fetched '%s' not set correctly", fi.ValueStr[0])
		}
	} else {
		t.Fatalf("MatchLabels not found in kvs")
	}

	if fi, ok := specKvs["Ports"]; ok {
		if len(fi.ValueStr) != 2 || fi.ValueStr[0] != "tcp/80,tcp/443" || fi.ValueStr[1] != "tcp/6379" {
			printKvs("spec", specKvs)
			t.Fatalf("error! Ports fetched '%s' not set correctly", fi.ValueStr)
		}
	} else {
		t.Fatalf("Ports not found")
	}

	if fi, ok := specKvs["Interval"]; ok {
		if v, err := strconv.Atoi(fi.ValueStr[0]); err != nil && v != 33 {
			printKvs("spec", specKvs)
			t.Fatalf("error! Interval '%v' not set correctly", fi.ValueStr)
		}
	}

	if fi, ok := specKvs["SkippedField"]; ok {
		printKvs("spec", specKvs)
		t.Fatalf("error! Found SkippedFiled '%v' improperly", fi.ValueStr)
	}

	if fi, ok := specKvs["ToObj"]; ok {
		if len(fi.ValueStr) != 1 || fi.ValueStr[0] != "network" {
			printKvs("spec", specKvs)
			t.Fatalf("error! ToObj from pointed struct not found '%v'", fi.ValueStr)
		}
	} else {
		t.Fatalf("ToObj not found")
	}

	if fi, ok := specKvs["RWX"]; ok {
		if len(fi.ValueStr) != 1 || fi.ValueStr[0] != "rw" {
			printKvs("spec", specKvs)
			t.Fatalf("error! ToObj from pointed struct not found '%v'", fi.ValueStr)
		}
	} else {
		t.Fatalf("ToObj not found")
	}
}

func subObj(kind string) interface{} {
	switch kind {
	case "SGRule":
		return &SGRule{}
	case "Permission":
		return &Permission{}
	}
	return nil
}

func TestEmptyGet(t *testing.T) {
	u := User{}
	spec := reflect.Indirect(reflect.ValueOf(&u.Spec))

	t.Logf("Struct:\n%s\n", WalkStruct(u, subObj))

	specKvs := make(map[string]FInfo)
	GetKvs(spec, subObj, specKvs)

	t.Logf("specKvs: %+v\n", specKvs)
}

func TestUpdate(t *testing.T) {
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
		},
	}
	metaKvs := make(map[string]FInfo)
	metaKvs["Name"] = NewFInfo([]string{"joe"})
	specKvs := make(map[string]FInfo)
	specKvs["Roles"] = NewFInfo([]string{"blah-blah"})

	newUser := writeUserObj(u, metaKvs, specKvs)

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
}

func TestWrite(t *testing.T) {
	metaKvs := make(map[string]FInfo)
	metaKvs["Name"] = NewFInfo([]string{"joe"})
	metaKvs["Labels"] = NewFInfo([]string{"dept:eng,level:mts"})
	specKvs := make(map[string]FInfo)
	specKvs["Roles"] = NewFInfo([]string{"storage-admin,security-admin"})
	specKvs["Aliases"] = NewFInfo([]string{"jhonny"})
	specKvs["MatchLabels"] = NewFInfo([]string{"io.pensando.area:network"})
	specKvs["Ports"] = NewFInfo([]string{"tcp/80,tcp/443", "tcp/6379"})
	specKvs["Action"] = NewFInfo([]string{"permit,log", "permit,log"})
	specKvs["PeerGroup"] = NewFInfo([]string{"web-sg", "db-sg"})
	specKvs["junk"] = NewFInfo([]string{"web-sg", "db-sg"})
	specKvs["ToObj"] = NewFInfo([]string{"sgpolicy"})
	specKvs["RWX"] = NewFInfo([]string{"rw"})

	newUser := writeUserObj(User{}, metaKvs, specKvs)

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
		t.Fatalf("unable to parse Perms into pointer object")
	}
}

func writeSGRuleObj(obj SGRule, specKvs map[string]FInfo) *SGRule {
	orig := reflect.ValueOf(obj)
	new := reflect.New(orig.Type()).Elem()
	WriteKvs(new, orig, subObj, specKvs)

	newSGRule := new.Interface().(SGRule)
	return &newSGRule
}

func writeUserObj(obj User, metaKvs, specKvs map[string]FInfo) *User {
	origObjm := reflect.ValueOf(obj.ObjectMeta)
	newObjm := reflect.New(origObjm.Type()).Elem()
	WriteKvs(newObjm, origObjm, subObj, metaKvs)

	origSpec := reflect.ValueOf(obj.Spec)
	newSpec := reflect.New(origSpec.Type()).Elem()
	WriteKvs(newSpec, origSpec, subObj, specKvs)

	newUser := &User{
		TypeMeta:   TypeMeta{Kind: "user"},
		ObjectMeta: newObjm.Interface().(ObjectMeta),
		Spec:       newSpec.Interface().(UserSpec),
	}

	for len(specKvs) > 0 {
		prevKvs := map[string]FInfo{}
		for k, v := range specKvs {
			prevKvs[k] = v
		}
		newSGRule := writeSGRuleObj(SGRule{}, specKvs)
		if reflect.DeepEqual(prevKvs, specKvs) {
			break
		}
		newUser.Spec.InRules = append(newUser.Spec.InRules, *newSGRule)
	}

	return newUser
}

func TestFieldByName(t *testing.T) {
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
		},
	}
	user := reflect.ValueOf(u)
	meta := reflect.ValueOf(u.ObjectMeta)
	spec := reflect.ValueOf(u.Spec)

	name := FieldByName(meta, "Name")
	if len(name) != 1 && name[0] != "joe" {
		t.Fatalf("Invalid name returned '%s'", name)
	}

	roles := FieldByName(spec, "Roles")
	if len(roles) != 2 && roles[0] != "storage-admin" && roles[1] != "security-admin" {
		t.Fatalf("Invalid roles returned '%s'", roles)
	}

	ports := FieldByName(user, "Spec.OutRules.Ports")
	if len(ports) != 2 && ports[0] != "tcp/6379" && ports[1] != "tcp/5505" {
		t.Fatalf("Invalid roles returned '%s'", ports)
	}

	matchLabels := FieldByName(user, "Spec.MatchLabels")
	if len(matchLabels) != 2 && matchLabels[0] != "io.pensando.area:network" && matchLabels[1] != "o.pensando.network.priority:critical" {
		t.Fatalf("Invalid matchLabels: %s", matchLabels)
	}

	ghosts := FieldByName(user, "Spec.OutRules.Ghosts")
	if len(ghosts) != 0 {
		t.Fatalf("Invalid ghosts returned '%s'", ghosts)
	}
}
