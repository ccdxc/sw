package gatherer

import (
	"strings"
	"testing"

	"github.com/go-test/deep"
	"github.com/influxdata/influxdb/models"

	"github.com/pensando/sw/test/integ/tsdb/aggutils"
)

func mustParsePt(t *testing.T, ptStr []string) []models.Point {
	a, err := models.ParsePointsString(strings.Join(ptStr, "\n"))
	if err != nil {
		t.Fatal(err)
	}

	return a
}

func mustWritePoints(t *testing.T, bec *aggutils.BECluster, p []models.Point) {
	err := bec.WritePointsOrdered(p)
	if err != nil {
		t.Fatal(err)
	}
}

func TestBGSync(t *testing.T) {
	// create a 3 node cluster
	bec := aggutils.NewBECluster(3)
	defer bec.TearDown()
	SetBackends(bec.URLs())
	AddDatabase("db0")

	for _, tc := range []struct {
		ptStr    []string
		metaKey  string
		cParsers map[string]parserFunc
	}{
		// simple case - one meas, one tag, one field
		{ptStr: []string{`cpu,host=A value=1.11000000000`}, metaKey: `db0<>cpu`, cParsers: map[string]parserFunc{"host": parseTag, "time": parseTime, "value": parseFloat}},
		// fields distributed across backends
		{ptStr: []string{`cpu,host=A value=1.11000000000`, `cpu,host=B value=1.2,aux=128i 1100000000`, `cpu,host=C sby=true 1100000000`}, metaKey: `db0<>cpu`, cParsers: map[string]parserFunc{"host": parseTag, "time": parseTime, "value": parseFloat, "aux": parseInteger, "sby": parseBool}},
		// second meas
		{ptStr: []string{`disk,host=A value="nvme" 1000000000`}, metaKey: `db0<>disk`, cParsers: map[string]parserFunc{"host": parseTag, "time": parseTime, "value": parseString}},
		{ptStr: []string{`cpu,host=A value=1.1 1200000000`}, metaKey: `db0<>cpu`, cParsers: map[string]parserFunc{"host": parseTag, "time": parseTime, "value": parseFloat, "aux": parseInteger, "sby": parseBool}},
	} {
		p := mustParsePt(t, tc.ptStr)
		mustWritePoints(t, bec, p)
		err := SyncMeta()
		if err != nil {
			t.Fatal(err)
		}

		cp := gtrMeta.tsdbInfo[tc.metaKey]
		if d := deep.Equal(cp.columnParsers, tc.cParsers); d != nil {
			t.Errorf("%s -- %s", tc.ptStr, d)
		}
	}
}

func TestConvertRowToPoints(t *testing.T) {
	testTags := []string{"tag1", "tag2"}
	testFields := map[string]string{
		"intField1":    "integer",
		"intField2":    "integer",
		"floatField1":  "float",
		"boolField1":   "bool",
		"stringField1": "string",
	}
	CreateMeasurement("testdb", "measA", testTags, testFields)

	for _, tc := range []struct {
		row   models.Row
		ptStr []string
	}{
		{
			row: models.Row{
				Name:    "measA",
				Columns: []string{"tag1", "intField1", "tag2", "time"},
				Values:  [][]interface{}{[]interface{}{"hostA", "33", "west", "2002-10-02T10:00:00-05:00"}, []interface{}{"hostA", "450", "east", "2002-10-02T10:00:01-05:00"}},
			},
			ptStr: []string{`measA,tag1=hostA,tag2=west intField1=33 1000000000`, `measA,tag1=hostA,tag2=east intField1=450 1100000000`},
		},
	} {
		exp := mustParsePt(t, tc.ptStr)
		got, err := ConvertRowToPoints("testdb", "measA", &tc.row)
		if err != nil {
			t.Fatal(err)
		}
		if d := deep.Equal(exp, got); d != nil {
			t.Errorf("%s -- %s", tc.ptStr, d)
		}
	}
}
