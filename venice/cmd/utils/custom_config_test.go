package utils

import (
	"io/ioutil"
	"os"
	"testing"
)

func TestVeniceConfigFile(t *testing.T) {

	tmpfile, err := ioutil.TempFile(os.TempDir(), "venice-config.json")
	if err != nil {
		t.Fatalf("Error creating temp file %#v", err)
	}
	content := `
	{
	    "DisabledModules": [ "pen-influx", "pen-collector" ],
		"OverriddenModules" :  {
			"hello" : {
				"meta" : {
					"uuid" : "dummyuuid"
				}
			}
		},
		"Properties" : {
			"prop1" : "y",
			"Prop2" : "bla"
		},
		"Retention" : {
			"Log"  : 1,
			"Event": 2,
			"Audit": 3
		}
	}`
	if _, err := tmpfile.Write([]byte(content)); err != nil {
		t.Fatal(err)
	}
	if err := tmpfile.Close(); err != nil {
		t.Fatal(err)
	}

	defer os.Remove(tmpfile.Name())
	d := GetDisabledModules(tmpfile.Name())
	if len(d) != 2 && d[0] != "pen-influx" && d[1] != "pen-collector" {
		t.Fatalf("unexpected values for GetDisabledModules. got %#v", d)
	}
	o := GetOverriddenModules(tmpfile.Name())
	if len(o) != 1 && o["hello"].UUID != "dummyuuid" {
		t.Fatalf("unexpected values for GetOverriddenModules. got %#v", o)
	}
	l := GetLogRetention(tmpfile.Name())
	if l != 1 {
		t.Fatalf("unexpected values for GetLogRetention. got %#v", l)
	}
	e := GetEventRetention(tmpfile.Name())
	if e != 2 {
		t.Fatalf("unexpected values for GetEventRetention. got %#v", e)
	}
	a := GetAuditRetention(tmpfile.Name())
	if a != 3 {
		t.Fatalf("unexpected values for GetAuditRetention. got %#v", a)
	}
	p := getConfigProperty(tmpfile.Name(), "prop1")
	if p != "y" {
		t.Fatalf("unexpected values for getConfigProperty. got %#v", p)
	}
	p = getConfigProperty("/some-non-existingfile", "prop1")
	if p != "" {
		t.Fatalf("unexpected values for getConfigProperty in non-existing file. got %#v", p)
	}

	d = GetDisabledModules("/some-non-existing-file")
	if len(d) != 0 {
		t.Fatalf("unexpected values for GetDisabledModules. got %#v", d)
	}
	o = GetOverriddenModules("/some-non-existing-file")
	if len(o) != 0 {
		t.Fatalf("unexpected values for GetOverriddenModules. got %#v", o)
	}

}
