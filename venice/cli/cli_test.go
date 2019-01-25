package vcli

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"reflect"
	"strings"
	"sync"
	"testing"

	log "github.com/sirupsen/logrus"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/workload"
	"github.com/pensando/sw/venice/cli/mock"
)

const (
	veniceCmd   = "venice "
	snapshotDir = "snap3443"
)

var (
	loginOnce   sync.Once
	logInitOnce sync.Once
	tinfo       *mock.Info
)

func veniceLogin() {
	loginOnce.Do(func() {
		cmdStr := veniceCmd + "--server https://localhost:" + tinfo.VenicePort + " " + "login -u " + tinfo.UserCred.Username + " -p " + tinfo.UserCred.Password
		cmdArgs := strings.Split(cmdStr, " ")
		InvokeCLI(cmdArgs, false)
	})
}

func TestMain(m *testing.M) {
	tinfo = mock.Start()
	rcode := m.Run()
	tinfo.Stop()
	os.Exit(rcode)
}

func TestLogin(t *testing.T) {
	veniceLogin()
}

func TestCreate(t *testing.T) {
	veniceLogin()

	out := veniceCLI("create workload --host-name dc12_rack3_bm4 --mac-address 00:de:ed:de:ed:d0 --external-vlan 55 --micro-seg-vlan 2222 --mac-address 00:f0:0d:f0:0d:d0 --external-vlan 66 --micro-seg-vlan 3333 TestCreateVm")
	obj := &workload.Workload{}
	out = veniceCLI("read workload -j TestCreateVm")
	obj = &workload.Workload{}
	if err := lookForJSON(out, obj); err != nil {
		t.Fatalf("error %s reading the object - out\n%s\n", err, out)
	}
	if obj.Kind != "Workload" || obj.ObjectMeta.Name != "TestCreateVm" {
		t.Fatalf("invalid object returned %+v", obj)
	}
	if obj.Spec.HostName != "dc12_rack3_bm4" {
		t.Fatalf("invalid hostname in obj: %+v", obj)
	}
	if len(obj.Spec.Interfaces) != 2 || !reflect.DeepEqual(obj.Spec.Interfaces,
		[]workload.WorkloadIntfSpec{
			{MACAddress: "00:de:ed:de:ed:d0", ExternalVlan: 55, MicroSegVlan: 2222},
			{MACAddress: "00:f0:0d:f0:0d:d0", ExternalVlan: 66, MicroSegVlan: 3333}}) {
		t.Fatalf("invalid Interfaces: %+v", obj.Spec.Interfaces)
	}
}

func TestUpdate(t *testing.T) {
	veniceLogin()

	veniceCLI("create workload --host-name dc12_rack3_bm4 --mac-address 00:de:ed:de:ed:d0 --external-vlan 55 --micro-seg-vlan 2222 --mac-address 00:f0:0d:f0:0d:d0 --external-vlan 66  --micro-seg-vlan 3333 TestUpdateVm")
	out := veniceCLI("update workload --host-name dc12_rack4_bm4 --mac-address 00:de:ed:de:ed:d0 --external-vlan 44 --micro-seg-vlan 1111 --mac-address 00:f0:0d:f0:0d:d0 --external-vlan 22  --micro-seg-vlan 2222 TestUpdateVm")

	out = veniceCLI("read workload -j TestUpdateVm")
	obj := &workload.Workload{}
	if err := lookForJSON(out, obj); err != nil {
		t.Fatalf("error %s reading the object - out\n%s\n", err, out)
	}
	if obj.Kind != "Workload" || obj.ObjectMeta.Name != "TestUpdateVm" {
		t.Fatalf("invalid object returned %+v", obj)
	}
	if obj.Spec.HostName != "dc12_rack4_bm4" {
		t.Fatalf("invalid hostname in obj: %+v", obj)
	}
	if len(obj.Spec.Interfaces) != 2 || !reflect.DeepEqual(obj.Spec.Interfaces,
		[]workload.WorkloadIntfSpec{
			{MACAddress: "00:de:ed:de:ed:d0", ExternalVlan: 44, MicroSegVlan: 1111},
			{MACAddress: "00:f0:0d:f0:0d:d0", ExternalVlan: 22, MicroSegVlan: 2222}}) {
		t.Fatalf("invalid Interfaces: %+v", obj.Spec.Interfaces)
	}
}

func TestPatch(t *testing.T) {
	veniceLogin()

	veniceCLI("create workload --host-name dc12_rack3_bm4 --mac-address 00:de:ed:de:ed:d0 --external-vlan 55 --micro-seg-vlan 2222 --mac-address 00:f0:0d:f0:0d:d0 --external-vlan 66  --micro-seg-vlan 3333 TestPatchVm")

	out := veniceCLI("read workload -j TestPatchVm")
	obj := &workload.Workload{}
	if err := lookForJSON(out, obj); err != nil {
		t.Fatalf("error %s reading the object - out\n%s\n", err, out)
	}

	veniceCLI("patch workload --host-name dc12_rack4_bm4 TestPatchVm")
	out = veniceCLI("read workload -j TestPatchVm")
	if err := lookForJSON(out, obj); err != nil {
		t.Fatalf("error %s reading the object - out\n%s\n", err, out)
	}
	if obj.Kind != "Workload" || obj.ObjectMeta.Name != "TestPatchVm" {
		t.Fatalf("invalid object returned %+v", obj)
	}
	if obj.Spec.HostName != "dc12_rack4_bm4" {
		t.Fatalf("invalid hostname in obj: %+v", obj)
	}
	if len(obj.Spec.Interfaces) != 2 || !reflect.DeepEqual(obj.Spec.Interfaces,
		[]workload.WorkloadIntfSpec{
			{MACAddress: "00:de:ed:de:ed:d0", ExternalVlan: 55, MicroSegVlan: 2222},
			{MACAddress: "00:f0:0d:f0:0d:d0", ExternalVlan: 66, MicroSegVlan: 3333}}) {
		t.Fatalf("invalid Interfaces: %+v", obj.Spec.Interfaces)
	}

	// patching on arrays needs more support to specify the index that is being patched
	// when index is not specified it updates the first index
	veniceCLI("patch workload --mac-address 00:de:ed:de:ed:d0 --external-vlan 77 TestPatchVm")
	out = veniceCLI("read workload -j TestPatchVm")
	if err := lookForJSON(out, obj); err != nil {
		t.Fatalf("error %s reading the object - out\n%s\n", err, out)
	}
	for ii := range obj.Spec.Interfaces {
		if obj.Spec.Interfaces[ii].MACAddress == "00:de:ed:de:ed:d0" {
			if obj.Spec.Interfaces[ii].ExternalVlan != 77 {
				t.Fatalf("invalid Interfaces update: %+v", obj.Spec.Interfaces)
			}
		}
	}
}

func TestRead(t *testing.T) {
	veniceLogin()

	veniceCLI("create workload --host-name dc12_rack3_bm4 --label key1=val1 --label key2=val2 --mac-address 00:de:ed:de:ed:d0 -external-vlan 55 --micro-seg-vlan 2222 --mac-address 00:f0:0d:f0:0d:d0 --external-vlan 66  --micro-seg-vlan 3333 TestReadVm1")
	veniceCLI("create workload --host-name dc12_rack3_bm4 --label key2=val2 --mac-address 00:de:ed:de:ed:d0 --external-vlan 55 --micro-seg-vlan 2223 TestReadVm2")
	veniceCLI("create workload --host-name dc12_rack3_bm4 --label key1=val1 --label key3=val3 --mac-address 00:de:ed:de:ed:d0 --external-vlan 55 --micro-seg-vlan 2224 TestReadVm3")
	out := veniceCLI("read workload")
	if !matchLineFields(out, []string{"dc12_rack3_bm4", "TestReadVm1"}) {
		t.Fatalf("error finding a specific record: %s", out)
	}
	if !matchLineFields(out, []string{"TestReadVm2", "00:de:ed:de:ed:d0", "55", "dc12_rack3_bm4", "2223"}) {
		t.Fatalf("error finding a specific record: %s", out)
	}

	out = veniceCLI("read workload -q")
	if !matchLines(out, []string{"TestReadVm1", "TestReadVm2", "TestReadVm3"}) {
		t.Fatalf("error matching lines: %s", out)
	}

	out = veniceCLI("read workload --label key1=val1")
	if !matchLineFields(out, []string{"TestReadVm1", "dc12_rack3_bm4"}) {
		t.Fatalf("error matching lines: %s", out)
	}
	if !matchLineFields(out, []string{"TestReadVm3", "dc12_rack3_bm4"}) {
		t.Fatalf("error matching lines: %s", out)
	}
}

func TestDelete(t *testing.T) {
	veniceLogin()

	veniceCLI("create workload --host-name dc12_rack3_bm4 --mac-address 00:de:ed:de:ed:d0 --external-vlan 55 --micro-seg-vlan 2223 TestDeleteVm1")
	veniceCLI("create workload --host-name dc12_rack3_bm4 --mac-address 00:de:ed:de:ed:d0 --external-vlan 77 --micro-seg-vlan 2224 TestDeleteVm2")
	veniceCLI("create workload --host-name dc12_rack3_bm4 --mac-address 00:de:ed:de:ed:d0 --external-vlan 88 --micro-seg-vlan 2225 TestDeleteVm3")
	out := veniceCLI("delete workload TestDeleteVm1")
	fmt.Printf("out = \n%s\n", out)
	out = veniceCLI("read workload -j TestDeleteVm1")
	obj := &workload.Workload{}
	if err := lookForJSON(out, obj); err == nil {
		t.Fatalf("found the record post delete: %s", out)
	}

	out = veniceCLI("delete workload TestDeleteVm2,TestDeleteVm3")
	fmt.Printf("out = \n%s\n", out)
	out = veniceCLI("read workload TestDeleteVm3")
	if err := lookForJSON(out, obj); err == nil {
		t.Fatalf("found the record post delete: %s", out)
	}
}

func TestEdit(t *testing.T) {
	veniceLogin()

	veniceCLI("create workload --label key1=val1 --label key2=val2 --host-name dc12_rack3_bm4 --mac-address 00:de:ed:de:ed:d0 --external-vlan 55 --micro-seg-vlan 2223 TestEditVm")
	// change the editor to cat and replace
	oldEditor := os.Getenv("VENICE_EDITOR")
	os.Setenv("VENICE_EDITOR", "sed -i s/55/77/g")
	out := veniceCLI("edit workload TestEditVm")
	fmt.Printf("edit workload:---\n%s\n---", out)

	obj := &workload.Workload{}
	out = veniceCLI("read workload -j TestEditVm")
	if err := lookForJSON(out, obj); err != nil {
		t.Fatalf("error %s reading the object - out\n%s\n", err, out)
	}

	// make sure that the resource version has been updated, and label is also updated
	if obj.Kind != "Workload" || obj.ObjectMeta.Name != "TestEditVm" || obj.Labels["key2"] != "val2" {
		t.Fatalf("invalid object returned %+v", obj)
	}
	if obj.Spec.HostName != "dc12_rack3_bm4" {
		t.Fatalf("invalid hostname in obj: %+v", obj)
	}
	if len(obj.Spec.Interfaces) != 1 || !reflect.DeepEqual(obj.Spec.Interfaces,
		[]workload.WorkloadIntfSpec{
			{MACAddress: "00:de:ed:de:ed:d0", ExternalVlan: 77, MicroSegVlan: 2223}}) {
		t.Fatalf("invalid Interfaces: %+v", obj.Spec.Interfaces)
	}

	// restore the editor
	os.Setenv("VENICE_EDITOR", oldEditor)
}

func TestUpdateFromFile(t *testing.T) {
	veniceLogin()

	obj := &workload.Workload{
		TypeMeta: api.TypeMeta{Kind: "workload"},
		ObjectMeta: api.ObjectMeta{
			Name:   "TestUpdateFromFileVm",
			Labels: map[string]string{"key1": "val1", "key2": "val2"},
		},
		Spec: workload.WorkloadSpec{
			HostName: "node021",
			Interfaces: []workload.WorkloadIntfSpec{
				{MACAddress: "11:11:11:11:11:11", ExternalVlan: 11, MicroSegVlan: 1000},
				{MACAddress: "22:22:22:22:22:22", ExternalVlan: 22, MicroSegVlan: 2000},
				{MACAddress: "33:33:33:33:33:33", ExternalVlan: 33, MicroSegVlan: 3000},
			},
		},
	}

	// marshal and save the object in a file
	b, err := json.Marshal(obj)
	if err != nil {
		t.Fatalf("Unable to marshal workload object %+v", obj)
	}
	ioutil.WriteFile("tmp-3772.json", b, 0644)
	defer os.RemoveAll("tmp-3772.json")
	veniceCLI("create workload -f tmp-3772.json")

	// read back the object to confirm if all matches our expectation
	out := veniceCLI("read workload -j TestUpdateFromFileVm")
	obj = &workload.Workload{}
	if err := lookForJSON(out, obj); err != nil {
		t.Fatalf("error %s reading the object - out\n%s\n", err, out)
	}

	if obj.Kind != "Workload" || obj.ObjectMeta.Name != "TestUpdateFromFileVm" ||
		obj.Labels["key1"] != "val1" || obj.Labels["key2"] != "val2" {
		t.Fatalf("invalid object returned %+v", obj)
	}
	if obj.Spec.HostName != "node021" {
		t.Fatalf("invalid hostname in obj: %+v", obj)
	}
	if len(obj.Spec.Interfaces) != 3 || !reflect.DeepEqual(obj.Spec.Interfaces,
		[]workload.WorkloadIntfSpec{
			{MACAddress: "11:11:11:11:11:11", ExternalVlan: 11, MicroSegVlan: 1000},
			{MACAddress: "22:22:22:22:22:22", ExternalVlan: 22, MicroSegVlan: 2000},
			{MACAddress: "33:33:33:33:33:33", ExternalVlan: 33, MicroSegVlan: 3000}}) {
		t.Fatalf("invalid Interfaces: %+v", obj.Spec.Interfaces)
	}
}

func TestLabel(t *testing.T) {
	veniceLogin()

	veniceCLI("create workload --label key1=val1 --host-name r2d2 --mac-address 00:de:ed:de:ed:d0 --external-vlan 55 --micro-seg-vlan 2223 TestLabelVm")

	out := veniceCLI("label workload --update-label key1=val2 TestLabelVm")

	obj := &workload.Workload{}
	out = veniceCLI("read workload -j TestLabelVm")
	if err := lookForJSON(out, obj); err != nil {
		t.Fatalf("error %s reading the object - out\n%s\n", err, out)
	}

	// make sure that the resource version has been updated, and label is also updated
	if obj.Kind != "Workload" || obj.ObjectMeta.Name != "TestLabelVm" || obj.Labels["key1"] != "val2" {
		t.Fatalf("invalid object returned %+v", obj)
	}
	if obj.Spec.HostName != "r2d2" {
		t.Fatalf("invalid hostname in obj: %+v", obj)
	}
	if len(obj.Spec.Interfaces) != 1 || !reflect.DeepEqual(obj.Spec.Interfaces,
		[]workload.WorkloadIntfSpec{
			{MACAddress: "00:de:ed:de:ed:d0", ExternalVlan: 55, MicroSegVlan: 2223}}) {
		t.Fatalf("invalid Interfaces: %+v", obj.Spec.Interfaces)
	}
}

func TestShowVersion(t *testing.T) {
	veniceLogin()

	out := veniceCLI("version")
	fmt.Printf("\n%s\n", out)
}

func TestExample(t *testing.T) {
	veniceLogin()

	out := veniceCLI("example workload")
	fmt.Printf("\n%s\n", out)
}

func TestDefinition(t *testing.T) {
	veniceLogin()

	out := veniceCLI("definition workload")
	fmt.Printf("\n%s\n", out)
}

func TestTree(t *testing.T) {
	veniceLogin()

	out := veniceCLI("tree")
	fmt.Printf("\n%s\n", out)
}

func TestSnapshot(t *testing.T) {
	veniceLogin()

	out := veniceCLI("snapshot --id " + snapshotDir)
	if !strings.Contains(out, "Successful - stored") {
		t.Fatalf("unable to store snapshot: %s\n", out)
	}
	os.RemoveAll(snapshotDir)
}

func TestBashCompletion(t *testing.T) {
	veniceLogin()

	veniceCLI("create workload --host-name dc12_rack3_bm4 --mac-address 00:de:ed:de:ed:d0 --external-vlan 55 --micro-seg-vlan 2222 --mac-address 00:f0:0d:f0:0d:d0 --external-vlan 66  --micro-seg-vlan 3333 TestBashCompletionVm1")
	veniceCLI("create workload --host-name dc12_rack3_bm4 --mac-address 00:de:ed:de:ed:d0 --external-vlan 55 --micro-seg-vlan 2223 TestBashCompletionVm2")
	veniceCLI("create workload --host-name dc12_rack3_bm4 --mac-address 00:de:ed:de:ed:d0 --external-vlan 55 --micro-seg-vlan 2224 TestBashCompletionContainer3")

	out := veniceCLI("read workload --gbc")
	if !matchLineFields(out, []string{"TestBashCompletionVm1", "TestBashCompletionVm2", "TestBashCompletionContainer3"}) {
		t.Fatalf("error matching vm names for command completion: %s", out)
	}

	// test name completion
	out = veniceCLI("read workload TestBashCompletionVm --gbc")
	if matchLineFields(out, []string{"TestBashCompletionContainer3"}) {
		t.Fatalf("matched unexpected workloads for command completion: %s", out)
	}

	out = veniceCLI("create workload --gbc")
	if !matchLineFields(out, []string{"--host-name", "--mac-address", "--micro-seg-vlan", "--external-vlan"}) {
		t.Fatalf("error matching create command options: %s", out)
	}

	// test string value completion
	out = veniceCLI("patch workload --host-name --gbc")
	if !matchLineFields(out, []string{"{value}"}) {
		t.Fatalf("error matching value in field specific bash completer: %s", out)
	}

	// test map value completion
	out = veniceCLI("read workload --label --gbc")
	if !matchLineFields(out, []string{"{key=value}"}) {
		t.Fatalf("error matching value in field specific bash completer: %s", out)
	}

	// test an option doesn't repeat itself
	out = veniceCLI("patch workload --host-name blah --gbc")
	if matchLineFields(out, []string{"host-name"}) {
		t.Fatalf("host-name reappeared for command completion: %s", out)
	}

	// maps and string slices must be allowed to repeat
	out = veniceCLI("update workload --label key2=val22 TestBashCompletionVm2 --gbc")
	if !matchLineFields(out, []string{"--label"}) {
		t.Fatalf("error matching map value more than once: %s", out)
	}

	out = veniceCLI("delete workload --gbc")
	if !matchLineFields(out, []string{"TestBashCompletionVm1", "TestBashCompletionVm2", "TestBashCompletionContainer3"}) {
		t.Fatalf("error matching vm names for command completion: %s", out)
	}

	out = veniceCLI("label workload --gbc")
	if !matchLineFields(out, []string{"TestBashCompletionVm1", "TestBashCompletionVm2", "TestBashCompletionContainer3"}) {
		t.Fatalf("error matching vm names for command completion: %s", out)
	}

	out = veniceCLI("example workload --gbc")
	if !matchLineFields(out, []string{"TestBashCompletionVm1", "TestBashCompletionVm2", "TestBashCompletionContainer3"}) {
		t.Fatalf("error matching vm names for command completion: %s", out)
	}

	out = veniceCLI("definition workload --gbc")
	if !matchLineFields(out, []string{"TestBashCompletionVm1", "TestBashCompletionVm2", "TestBashCompletionContainer3"}) {
		t.Fatalf("error matching vm names for command completion: %s", out)
	}

	out = veniceCLI("edit workload --gbc")
	if !matchLineFields(out, []string{"TestBashCompletionVm1", "TestBashCompletionVm2", "TestBashCompletionContainer3"}) {
		t.Fatalf("error matching vm names for command completion: %s", out)
	}

	out = veniceCLI("snapshot --gbc")
	if !matchLineFields(out, []string{"--restore"}) {
		t.Fatalf("error matching snapshot command completion: %s", out)
	}

	out = veniceCLI("update workload --gbc")
	if !matchLineFields(out, []string{"TestBashCompletionVm1", "TestBashCompletionVm2", "TestBashCompletionContainer3"}) {
		t.Fatalf("error matching vm names for command completion: %s", out)
	}

	// regex for name completion
	out = veniceCLI("read workload Test.*CompletionVm --gbc")
	if !matchLineFields(out, []string{"TestBashCompletionVm1", "TestBashCompletionVm2"}) {
		t.Fatalf("error mathcing vm names for command completion: %s", out)
	}

	// invalid regex should fail
	out = veniceCLI("read workload Test.{*CompletionVm --gbc")
	if matchLineFields(out, []string{"TestBashCompletionVm1"}) {
		t.Fatalf("matched unexpected workloads for command completion: %s", out)
	}

	out = veniceCLI("--gbc")
	if !matchLineFields(out, []string{"read", "create", "update"}) {
		t.Fatalf("unable to validate main command completion: %s", out)
	}

	// bool flag in command completion
	veniceCLI("update alertdestination --default --gbc")
}

func TestTopLevelBashCompletion(t *testing.T) {
	veniceLogin()

	out := veniceCLI("edit --gbc")
	if !matchLineFields(out, []string{"sgpolicy", "workload", "securitygroup"}) {
		t.Fatalf("unable to validate command completion: %s", out)
	}

	out = veniceCLI("create --gbc")
	if !matchLineFields(out, []string{"sgpolicy", "workload", "securitygroup"}) {
		t.Fatalf("unable to validate command completion: %s", out)
	}

	out = veniceCLI("update --gbc")
	if !matchLineFields(out, []string{"sgpolicy", "workload", "securitygroup"}) {
		t.Fatalf("unable to validate command completion: %s", out)
	}

	out = veniceCLI("patch --gbc")
	if !matchLineFields(out, []string{"sgpolicy", "workload", "securitygroup"}) {
		t.Fatalf("unable to validate command completion: %s", out)
	}

	out = veniceCLI("delete --gbc")
	if !matchLineFields(out, []string{"sgpolicy", "workload", "securitygroup"}) {
		t.Fatalf("unable to validate command completion: %s", out)
	}

	out = veniceCLI("read --gbc")
	if !matchLineFields(out, []string{"sgpolicy", "workload", "securitygroup"}) {
		t.Fatalf("unable to validate command completion: %s", out)
	}

	out = veniceCLI("label --gbc")
	if !matchLineFields(out, []string{"sgpolicy", "workload", "securitygroup"}) {
		t.Fatalf("unable to validate command completion: %s", out)
	}

	out = veniceCLI("example --gbc")
	if !matchLineFields(out, []string{"sgpolicy", "workload", "securitygroup"}) {
		t.Fatalf("unable to validate command completion: %s", out)
	}

	out = veniceCLI("definition --gbc")
	if !matchLineFields(out, []string{"sgpolicy", "workload", "securitygroup"}) {
		t.Fatalf("unable to validate command completion: %s", out)
	}

	out = veniceCLI("tree --gbc")
	if !matchLineFields(out, []string{"sgpolicy", "workload", "securitygroup"}) {
		t.Fatalf("unable to validate command completion: %s", out)
	}

	out = veniceCLI("create -f --gbc")
	if !matchLineFields(out, []string{"sgpolicy"}) {
		t.Fatalf("unable to validate command completion: %s", out)
	}
}

func TestCreateYml(t *testing.T) {
	ymlBytes := `#
kind: workload
meta:
  labels:
    key1: val1
    key2: val2
  name: TestCreateYmlVm
spec:
  host-name: bm576
  interfaces:
    - mac-address: "11:11:11:11:11:11"
      external-vlan: 11
      micro-seg-vlan: 1000
    - mac-address: "22:22:22:22:22:22"
      external-vlan: 22
      micro-seg-vlan: 2000
    - mac-address: "33:33:33:33:33:33"
      external-vlan: 33
      micro-seg-vlan: 3000

`
	fileName := "/tmp/tmp3343.yml"
	ioutil.WriteFile(fileName, []byte(ymlBytes), 0644)
	defer os.Remove(fileName)

	out := veniceCLI("create workload -f /tmp/tmp3343.yml")

	// read back the object to confirm if all matches our expectation
	out = veniceCLI("read workload -j TestCreateYmlVm")
	obj := &workload.Workload{}
	if err := lookForJSON(out, obj); err != nil {
		t.Fatalf("error %s reading the object - out\n%s\n", err, out)
	}
	if obj.Kind != "Workload" || obj.ObjectMeta.Name != "TestCreateYmlVm" ||
		obj.Labels["key1"] != "val1" || obj.Labels["key2"] != "val2" {
		t.Fatalf("invalid object returned %+v", obj)
	}
	if obj.Spec.HostName != "bm576" {
		t.Fatalf("invalid hostname in obj: %+v", obj)
	}
	if len(obj.Spec.Interfaces) != 3 || !reflect.DeepEqual(obj.Spec.Interfaces,
		[]workload.WorkloadIntfSpec{
			{MACAddress: "11:11:11:11:11:11", ExternalVlan: 11, MicroSegVlan: 1000},
			{MACAddress: "22:22:22:22:22:22", ExternalVlan: 22, MicroSegVlan: 2000},
			{MACAddress: "33:33:33:33:33:33", ExternalVlan: 33, MicroSegVlan: 3000}}) {
		t.Fatalf("invalid Interfaces: %+v", obj.Spec.Interfaces)
	}
}

func TestFailureCases(t *testing.T) {
	// pass non existing dir for a create file operation
	out := veniceCLI("create workload -f /junk")
	if !strings.Contains(out, "File does not exist") {
		t.Fatalf("succeeded create workload for a non existing file: %s", out)
	}

	// null filename for a create file operation
	out = veniceCLI("create workload -f ")
	if !strings.Contains(out, "Null file name") {
		t.Fatalf("succeeded create workload for a non existing file %s", out)
	}

	ymlBytes := `#
	some junk`
	fileName := "/tmp/tmp3343.yml"
	ioutil.WriteFile(fileName, []byte(ymlBytes), 0644)
	defer os.Remove(fileName)

	// invalid yml file contents
	veniceCLI("create workload -f /tmp/tmp3343.yml")

	// patch non existing workload
	out = veniceCLI("patch workload foo --host-name blah")
	if !strings.Contains(out, "Status:(404)") {
		t.Fatalf("succeeded on non-existing object --%s--\n", out)
	}

	// delete non existing workload
	out = veniceCLI("delete workload foo")
	if !strings.Contains(out, "Status:(404)") {
		t.Fatalf("succeeded on non-existing object --%s--\n", out)
	}

	// update non existing workload
	out = veniceCLI("update workload foo")
	if !strings.Contains(out, "Status:(404)") {
		t.Fatalf("succeeded on non-existing object --%s--\n", out)
	}
}

func TestConfigCommandCompletion(t *testing.T) {
	out := veniceCLI("auto-completion")
	fmt.Printf("--%s--\n", out)
}

func TestLogout(t *testing.T) {
	veniceLogin()

	out := veniceCLI("logout")
	if !strings.Contains(out, "Logout successful") {
		t.Fatalf("logout failure")
	}
}

func veniceCLI(cmdStr string) string {
	cmdStr = veniceCmd + "--server https://localhost:" + tinfo.VenicePort + " " + cmdStr
	splitStrs := strings.Split(cmdStr, " ")
	cmdArgs := []string{}
	// remove empty words from splitStrs
	for _, cmd := range splitStrs {
		if len(strings.Trim(cmd, " ")) != 0 {
			cmdArgs = append(cmdArgs, cmd)
		}
	}

	fmt.Printf(">>> issuing cmd: '%s'\n", cmdStr)

	logInitOnce.Do(func() {
		// InvokeCLI below changes the os.Stdout temporarily to a os.Pipe() and calls backend server functions in martini context and reverts back.
		// These backend server functions can call log().
		// If the defaultLogger.LogToStdout is true, then on the first invocation of Log(), the current os.Stdout is Dup2 to os.Stderr
		//	In such a case, its possible that the pipe is duplicated instead of real os.Stdout. Hence one end of pipe is always open
		//  (only one file descriptor is closed). And hence the other end of pipe never sees EOF and is stuck forever reading from the pipe.
		//  Hence the call to InvokeCLI never returns.
		// To prevent all this, do a dummy log, which creates the defaultLogger's singleton object before invoking the InvokeCLI
		log.Debugf("Initializing log")
	})
	stdOut := InvokeCLI(cmdArgs, true)
	return stdOut
}
