package tsdb

import (
	"testing"
	"time"

	"github.com/pensando/sw/api"
)

// table creation
func TestNewTableCreate(t *testing.T) {
	tbl, err := NewTable("table1", Config{})
	if err != nil {
		t.Fatalf("error creating new table: %s", err)
	}

	tbl.Delete()
}

// table creation with invalid params
func TestNewTableCreateInvalidConfig(t *testing.T) {
	if _, err := NewTable("", Config{}); err == nil {
		t.Fatalf("no error creating table with invalid config")
	}
}

func TestTableErrors(t *testing.T) {
	if tbl, err := NewTable("junk", Config{}); err != nil {
		t.Fatalf("error creating table")
	} else {
		defer tbl.Delete()
	}

	if _, err := NewTable("junk", Config{}); err == nil {
		t.Fatalf("successfully created table with same name")
	}
}

// test adding points to the table
func TestAddPoints(t *testing.T) {
	tbl, err := NewTable("table2", Config{})
	if err != nil {
		t.Fatalf("error creating new table: %s", err)
	}
	defer tbl.Delete()

	pt1 := Point{Tags: map[string]string{"ifname": "veth1", "node": "naples23", "workload": "vm223"},
		Fields: map[string]interface{}{"rxbytes": 3443, "txbytes": 5009684, "rxpackets": 5, "txpackets": 7474}}

	err = tbl.AddPoints("veth1-ifstats", []Point{pt1})
	if err != nil {
		t.Fatalf("error adding points to the  table")
	}

	// wait for the event to arrive
	time.Sleep(time.Millisecond)
}

type epMetric struct {
	rxBytes   Counter
	rxPackets Counter
	txBytes   Increment
	txPackets Increment
	cpuUtil   Gauge
	up        Flag
	user      String
}
type endpoint struct {
	api.TypeMeta
	api.ObjectMeta
	Metric epMetric
}

func TestObjPoints(t *testing.T) {
	epStat := &endpoint{
		TypeMeta:   api.TypeMeta{Kind: "endpoint"},
		ObjectMeta: api.ObjectMeta{Tenant: "foo", Namespace: "bar", Name: "vm87-veth39"}}
	tblObj, err := NewTableObj(&epStat, Config{})
	if err != nil {
		t.Fatalf("error creating new table: %s", err)
	}

	// first reading
	epStat.Metric = epMetric{rxBytes: 7743, txBytes: 6644, rxPackets: 76, txPackets: 988, cpuUtil: 34.1, up: true, user: "admin"}
	if err := tblObj.AddObjPoint(&epStat); err != nil {
		t.Fatalf("error adding points to the table: %s", err)
	}

	// second reading
	epStat.Metric = epMetric{rxBytes: 80003, txBytes: 1125546, rxPackets: 1232, txPackets: 88559, cpuUtil: 23.1, up: false, user: "admin"}
	if err := tblObj.AddObjPoint(&epStat); err != nil {
		t.Fatalf("error adding points to the table: %s", err)
	}

	time.Sleep(time.Millisecond)

	tblObj.Delete()
}

func TestObjPointsErrors(t *testing.T) {
	type invalid struct {
		char byte
	}

	if _, err := NewTableObj(&endpoint{}, Config{}); err == nil {
		t.Fatalf("not errored with invalid kind in the object")
	}

	if _, err := NewTableObj(&endpoint{TypeMeta: api.TypeMeta{Kind: "endpoint"}}, Config{}); err == nil {
		t.Fatalf("not errored with invalid keys in the object")
	}

	epStat := &endpoint{
		TypeMeta:   api.TypeMeta{Kind: "endpoint"},
		ObjectMeta: api.ObjectMeta{Tenant: "foo", Namespace: "bar", Name: "vm87-veth39"}}
	tblObj, err := NewTableObj(&epStat, Config{})
	if err != nil {
		t.Fatalf("error creating new table: %s", err)
	}
	if err := tblObj.AddObjPoint(&invalid{char: 'a'}); err == nil {
		t.Fatalf("not errored on invalid metrics object")
	}

	tblObj.Delete()
}
