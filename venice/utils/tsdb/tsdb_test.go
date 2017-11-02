package tsdb

import (
	"reflect"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

// fake transmitter mimics a transmitter residing alongside client code
var fc fakeTransmitter

type fakeTransmitter struct {
	dbName         string
	receivedPoints []Point
}

func (c *fakeTransmitter) Init(opts Options) error {
	return nil
}

func (c *fakeTransmitter) Update(opts Options) error {
	return nil
}

func (c *fakeTransmitter) SendPoints(dbName string, points []Point) error {
	c.dbName = dbName
	c.receivedPoints = points

	return nil
}

func printPoints(header string, points []Point) {
	log.Infof("%s", header)
	for _, pt := range points {
		for k, v := range pt.Tags {
			log.Infof("tag = %T=%v, %T=%v", k, k, v, v)
		}
		for k, v := range pt.Fields {
			log.Infof("field = %T=%v, %T=%v", k, k, v, v)
		}
	}
}

func (c *fakeTransmitter) validate(dbName string, points []Point) bool {
	if dbName != c.dbName {
		log.Errorf("unexpected dbName, got '%s' expected '%s'", dbName, c.dbName)
		return false
	}

	// null out the time before comparing
	for idx := range c.receivedPoints {
		c.receivedPoints[idx].Time = time.Time{}
	}
	if reflect.DeepEqual(points, c.receivedPoints) {
		return true
	}

	printPoints("Expected", points)
	printPoints("Got", c.receivedPoints)
	return false
}

// initialization test
func TestInit(t *testing.T) {
	if _, err := NewTable("table1", Config{}); err == nil {
		t.Fatalf("error creating a table without global init")
	}

	if err := Init(&fc, Options{}); err != nil {
		t.Fatalf("error initializing fake transmitter")
	}
}

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

	pts := []Point{{Tags: map[string]string{"ifname": "veth1", "node": "naples23", "workload": "vm223"},
		Fields: map[string]interface{}{"rxBytes": 3443, "txBytes": 5009684, "rxPackets": 5, "txPackets": 7474}}}

	err = tbl.AddPoints("veth1-ifstats", pts)
	if err != nil {
		t.Fatalf("error adding points to the  table")
	}

	// verify vm was removed
	AssertEventually(t, func() (bool, []interface{}) {
		return fc.validate("table2", pts), nil
	}, "add points not received on transmitter if", "1ms", "1s")
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

	// validate first reading
	time.Sleep(time.Millisecond)
	pts := []Point{{Tags: map[string]string{"Tenant": "foo", "Namespace": "bar", "Name": "vm87-veth39"},
		Fields: map[string]interface{}{"rxBytes": uint64(7743), "txBytes": uint64(6644), "rxPackets": uint64(76),
			"txPackets": uint64(988), "cpuUtil": 34.1, "up": true, "user": "admin"}}}
	if !fc.validate("endpoint", pts) {
		t.Fatalf("error receiving all points: got %+v, expected %+v", fc.receivedPoints, pts)
	}

	// second reading
	epStat.Metric = epMetric{rxBytes: 80003, txBytes: 1125546, rxPackets: 1232, txPackets: 88559, cpuUtil: 23.1, up: false, user: "admin"}
	if err := tblObj.AddObjPoint(&epStat); err != nil {
		t.Fatalf("error adding points to the table: %s", err)
	}

	// validate second reading
	time.Sleep(time.Millisecond)
	tblObj.Delete()

	// TBD: validate batching
	pts = []Point{{Tags: map[string]string{"Tenant": "foo", "Namespace": "bar", "Name": "vm87-veth39"},
		Fields: map[string]interface{}{"rxBytes": uint64(80003), "txBytes": uint64(1125546), "rxPackets": uint64(1232),
			"txPackets": uint64(88559), "cpuUtil": 23.1, "up": false, "user": "admin"}}}
	if !fc.validate("endpoint", pts) {
		t.Fatalf("error receiving all points: got %+v, expected %+v", fc.receivedPoints, pts)
	}
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
