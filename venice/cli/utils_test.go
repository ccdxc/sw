package vcli

import (
	"encoding/json"
	"reflect"
	"regexp"
	"strings"
	"testing"

	"github.com/pensando/sw/api/generated/workload"
)

func TestSliceToMap(t *testing.T) {
	m, err := sliceToMap([]string{"key1" + kvSplitter + "val1", "key2" + kvSplitter + "val2"})
	if err != nil {
		t.Fatalf("erorr convering slice to map: %s", err)
	}

	if m["key1"] != "val1" || m["key2"] != "val2" {
		t.Fatalf("error finding correct keys in the map")
	}
}

func TestMatchRe(t *testing.T) {
	re := regexp.MustCompile(".*buk.*")
	if !matchRe(re, "timbuktoo") {
		t.Fatalf("couldn't match a regex")
	}
	if matchRe(re, "paris") {
		t.Fatalf("could match invalid regex")
	}
}

func TestMatchLabel(t *testing.T) {
	matchLabels := map[string]string{"key1": "val1", "key2": "val2"}
	if !matchLabel(matchLabels, map[string]string{"key2": "val2"}) {
		t.Fatalf("unable to match keys")
	}
	if matchLabel(matchLabels, map[string]string{"key3": "val3"}) {
		t.Fatalf("able to match non existing keys")
	}
	if !matchLabel(map[string]string{}, map[string]string{"key1": "val1"}) {
		t.Fatalf("unable to match empty match criteria")
	}
}

func TestMatchString(t *testing.T) {
	matchStrings := []string{"one", "two", "three"}
	if !matchString(matchStrings, "three") {
		t.Fatalf("unable to match string")
	}
	if matchString(matchStrings, "four") {
		t.Fatalf("able to match non existing string")
	}
	if !matchString([]string{}, "two") {
		t.Fatalf("unable to match empty match criteria")
	}
}

func TestLookupForJson(t *testing.T) {
	jsonMixedData := `rest get for obj T:<> O:<Name:"vm23" Tenant:"default" CreationTime:<Time:<> > ModTime:<Time:<> > > Spec:<> Status:<> 
ts=2018-08-28T18:16:33.264618214Z module=Default pid=3725 caller=new_logger.go:171 module=cli service=WorkloadV1 method=AutoGetWorkload result=Success duration=2.925191ms level=audit
		{
		  "kind": "Workload",
		  "api-version": "v1",
		  "meta": {
		    "name": "vm23",
		    "tenant": "default",
		    "resource-version": "5",
		    "uuid": "4b589680-817f-43fd-9866-1d092f58c907",
		    "creation-time": "2018-08-28T18:16:33.259122478Z",
		    "mod-time": "2018-08-28T18:16:33.259123321Z",
		    "self-link": "/configs/workload/v1/tenant/default/workloads/vm23"
		  },
		  "spec": {
		    "host-name": "esx-node12",
		    "interfaces": {
		      "00:11:22:33:44:55": {
       		 "micro-seg-vlan": 1001,
		        "external-vlan": 100
		      },
		      "00:22:33:44:55:66": {
		        "micro-seg-vlan": 2001,
		        "external-vlan": 200
		      }
		    }
		  },
		  "status": {}
		}`

	obj := &workload.Workload{}
	if err := lookForJSON(jsonMixedData, obj); err != nil {
		t.Fatalf("error parsing json data: %s", err)
	}

	if obj.Kind != "Workload" || obj.Name != "vm23" {
		t.Fatalf("invalid object returned %+v", obj)
	}
	if obj.Spec.HostName != "esx-node12" {
		t.Fatalf("invalid hostname in obj: %+v", obj)
	}
	if len(obj.Spec.Interfaces) != 2 || !reflect.DeepEqual(obj.Spec.Interfaces,
		map[string]workload.WorkloadIntfSpec{
			"00:11:22:33:44:55": workload.WorkloadIntfSpec{ExternalVlan: 100, MicroSegVlan: 1001},
			"00:22:33:44:55:66": workload.WorkloadIntfSpec{ExternalVlan: 200, MicroSegVlan: 2001}}) {
		t.Fatalf("invalid Interfaces: %+v", obj.Spec.Interfaces)
	}
}

func TestFindJsonRecord(t *testing.T) {
	jsonRecs := `
	{
  		"kind": "Workload",
		"meta": {
		    "name": "TestFindJsonRecordVm1"
		},
		"spec": {
		    "host-name": "esx-node12",
		    "interfaces": {
		      "00:11:22:33:44:55": {
		        "micro-seg-vlan": 1001,
		        "external-vlan": 101
		      },
		      "00:22:33:44:55:66": {
		        "micro-seg-vlan": 2001,
		        "external-vlan": 201
		      }
		    }
		  }
	}
	{
  		"kind": "Workload",
		"meta": {
		    "name": "TestFindJsonRecordVm2"
		},
		"spec": {
		    "host-name": "esx-node12",
		    "interfaces": {
		      "00:11:22:33:00:55": {
		        "micro-seg-vlan": 1002,
		        "external-vlan": 102
		      },
		      "00:22:33:44:00:66": {
		        "micro-seg-vlan": 2002,
		        "external-vlan": 202
		      }
		    }
		}
	}
		`
	jsonRec, nextIdx := findJSONRecord(jsonRecs, 0)
	if jsonRec == "" || nextIdx < 0 {
		t.Fatalf("unable to local json record: %s", jsonRecs)
	}
	obj := &workload.Workload{}
	if err := json.Unmarshal([]byte(jsonRec), obj); err != nil {
		t.Fatalf("error '%s' unmarshing record: '%s'\n", err, jsonRec)
	}
	if obj.Kind != "Workload" || obj.Name != "TestFindJsonRecordVm1" {
		t.Fatalf("invalid object returned %+v", obj)
	}
	if obj.Spec.HostName != "esx-node12" {
		t.Fatalf("invalid hostname in obj: %+v", obj)
	}
	if len(obj.Spec.Interfaces) != 2 || !reflect.DeepEqual(obj.Spec.Interfaces,
		map[string]workload.WorkloadIntfSpec{
			"00:11:22:33:44:55": workload.WorkloadIntfSpec{ExternalVlan: 101, MicroSegVlan: 1001},
			"00:22:33:44:55:66": workload.WorkloadIntfSpec{ExternalVlan: 201, MicroSegVlan: 2001}}) {
		t.Fatalf("invalid Interfaces: %+v", obj.Spec.Interfaces)
	}

	jsonRec, nextIdx = findJSONRecord(jsonRecs, nextIdx)
	if jsonRec == "" || nextIdx < 0 {
		t.Fatalf("unable to local json record: %s", jsonRecs)
	}
	obj = &workload.Workload{}
	if err := json.Unmarshal([]byte(jsonRec), obj); err != nil {
		t.Fatalf("error '%s' unmarshing record: '%s'\n", err, jsonRec)
	}
	if obj.Kind != "Workload" || obj.Name != "TestFindJsonRecordVm2" {
		t.Fatalf("invalid object returned json-rec %s %+v", jsonRec, obj)
	}
	if obj.Spec.HostName != "esx-node12" {
		t.Fatalf("invalid hostname in obj: %+v", obj)
	}
	if len(obj.Spec.Interfaces) != 2 || !reflect.DeepEqual(obj.Spec.Interfaces,
		map[string]workload.WorkloadIntfSpec{
			"00:11:22:33:00:55": workload.WorkloadIntfSpec{ExternalVlan: 102, MicroSegVlan: 1002},
			"00:22:33:44:00:66": workload.WorkloadIntfSpec{ExternalVlan: 202, MicroSegVlan: 2002}}) {
		t.Fatalf("invalid Interfaces: %+v", obj.Spec.Interfaces)
	}

	jsonRec, nextIdx = findJSONRecord(jsonRecs, nextIdx)
	if jsonRec != "" || nextIdx >= 0 {
		t.Fatalf("able to locate record after nextIdx %d: %s", nextIdx, jsonRecs)
	}
}

func TestDumpStruct(t *testing.T) {
	obj := &workload.Workload{}
	obj.Kind = "workload"
	obj.Name = "workload1"
	obj.Labels = map[string]string{"label-key1": "label-value1", "label-key2": "label=value2"}
	obj.Spec = workload.WorkloadSpec{
		HostName: "node021",
		Interfaces: map[string]workload.WorkloadIntfSpec{
			"11:11:11:11:11:11": {ExternalVlan: 11, MicroSegVlan: 1000},
			"22:22:22:22:22:22": {ExternalVlan: 22, MicroSegVlan: 2000},
			"33:33:33:33:33:33": {ExternalVlan: 33, MicroSegVlan: 3000},
		},
	}

	// dump yml output
	out := dumpStruct(true, obj)
	if !strings.Contains(string(out), "33:33:33:33:33:33") {
		t.Fatalf("unable to find interface object: out %s", out)
	}

	// dump json output
	out = dumpStruct(false, obj)
	if !strings.Contains(string(out), "33:33:33:33:33:33") {
		t.Fatalf("unable to find interface object: out %s", out)
	}
}

func TestMatchLineFields(t *testing.T) {
	out := `name         labels     external-vlan         host-name       micro-seg-vlan          
		----         ------     -------------         ---------       --------------          
		TestReadVm1  key1=val1  00:de:ed:de:ed:d0=55  dc12_rack3_bm4  00:de:ed:de:ed:d0=2222  
		             key2=val2  00:f0:0d:f0:0d:d0=66                  00:f0:0d:f0:0d:d0=3333  
		TestReadVm3  key1=val1  00:de:ed:de:ed:d0=55  dc12_rack3_bm4  00:de:ed:de:ed:d0=2224  
		             key3=val3                                  `
	if !matchLineFields(out, []string{"TestReadVm1", "dc12_rack3_bm4", "00:de:ed:de:ed:d0=55"}) {
		t.Fatalf("unable to match fields")
	}

	if !matchLines(out, []string{"key3=val3"}) {
		t.Fatalf("unable to match lines")
	}

	if matchLineFields(out, []string{"TestReadVm11", "dc12_rack3_bm4", "00:de:ed:de:ed:d0=55"}) {
		t.Fatalf("able to match unexpected fields")
	}

	if matchLines(out, []string{"key3=val33"}) {
		t.Fatalf("able to match expected lines")
	}

}
