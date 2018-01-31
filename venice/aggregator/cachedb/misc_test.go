package cachedb

import (
	"io/ioutil"
	"path"
	"path/filepath"
	"testing"

	"github.com/influxdata/influxdb/models"
	"github.com/influxdata/influxdb/query"
	"github.com/influxdata/influxdb/tsdb"
	"github.com/influxdata/influxdb/tsdb/index/inmem"
	"github.com/influxdata/influxql"
)

const (
	testMeas = "meas1"
)

// Engine is a test wrapper for tsm1.Engine.
type Engine struct {
	*CacheEngine
	root  string
	index tsdb.Index
}

// NewEngine returns a new instance of Engine at a temporary location.
func NewEngine(index string) *Engine {
	root, err := ioutil.TempDir("", "tsm1-")
	if err != nil {
		panic(err)
	}

	db := path.Base(root)
	opt := tsdb.NewEngineOptions()
	opt.IndexVersion = index
	if index == "inmem" {
		opt.InmemIndex = inmem.NewIndex(db)
	}

	idx := tsdb.MustOpenIndex(1, db, filepath.Join(root, "data", "index"), opt)

	return &Engine{
		CacheEngine: NewCacheEngine(1,
			idx,
			db,
			filepath.Join(root, "data"),
			filepath.Join(root, "wal"),
			opt).(*CacheEngine),
		root:  root,
		index: idx,
	}
}

func MustOpenDefaultEngine() *Engine {
	e := NewEngine(tsdb.DefaultIndex)
	if err := e.Open(); err != nil {
		panic(err)
	}
	return e
}

// Ensure engine can create an ascending iterator for cached values.
func TestEngine_buildCursor(t *testing.T) {
	e := MustOpenDefaultEngine()
	defer e.Close()

	e.MeasurementFields([]byte(testMeas)).CreateFieldIfNotExists([]byte("fieldInteger"), influxql.Integer, false)
	e.MeasurementFields([]byte(testMeas)).CreateFieldIfNotExists([]byte("fieldFloat"), influxql.Float, false)
	e.MeasurementFields([]byte(testMeas)).CreateFieldIfNotExists([]byte("fieldUnsigned"), influxql.Unsigned, false)
	e.MeasurementFields([]byte(testMeas)).CreateFieldIfNotExists([]byte("fieldBoolean"), influxql.Boolean, false)
	e.MeasurementFields([]byte(testMeas)).CreateFieldIfNotExists([]byte("fieldString"), influxql.String, false)
	vref := influxql.VarRef{Val: "_name"}
	c := e.buildCursor(testMeas, "", models.Tags{}, &vref, query.IteratorOptions{})
	if c == nil {
		t.Fatal("_name: buildCursor returned nil")
	}

	_, s := c.next()
	if s.(string) != testMeas {
		t.Fatalf("Exp: %s, got %v", testMeas, s)
	}

	tags := models.NewTags(map[string]string{"host": "A", "node": "N1"})
	vref = influxql.VarRef{Val: "_tagKey"}
	c = e.buildCursor(testMeas, "", tags, &vref, query.IteratorOptions{})
	if c == nil {
		t.Fatal("_tagKey: buildCursor returned nil")
	}
	//	_, s = c.next()
	//	if s.(string) != tags.Keys() {
	//		t.Fatalf("Exp: %s, got %v", tags.Keys(), s)
	//	}

	vref = influxql.VarRef{Val: "_tagValue"}
	opt := query.IteratorOptions{Condition: &influxql.BinaryExpr{
		Op:  influxql.EQ,
		LHS: &influxql.VarRef{Val: "node", Type: influxql.Tag},
		RHS: &influxql.StringLiteral{Val: "N1"},
	},
	}

	c = e.buildCursor(testMeas, "", tags, &vref, opt)
	if c == nil {
		t.Fatal("_tagValue: buildCursor returned nil")
	}

	vref = influxql.VarRef{Val: "_seriesKey"}
	c = e.buildCursor(testMeas, "foo:bar", tags, &vref, opt)
	if c == nil {
		t.Fatal("_tagValue: buildCursor returned nil")
	}
	_, s = c.next()
	if s.(string) != "foo:bar" {
		t.Fatalf("Exp: foo:bar, got %v", s)
	}

	vref = influxql.VarRef{Val: "_fieldKey"}
	c = e.buildCursor(testMeas, "", tags, &vref, opt)
	if c == nil {
		t.Fatal("_fieldKey: buildCursor returned nil")
	}
	vref = influxql.VarRef{Val: "blahblah"}
	c = e.buildCursor(testMeas, "", tags, &vref, opt)
	if c != nil {
		t.Fatal("_fieldKey: buildCursor returned non-nil")
	}
	sKey := "meas1,host=A,node=N1"
	vref = influxql.VarRef{Val: "fieldInteger", Type: influxql.Float}
	c = e.buildCursor(testMeas, sKey, tags, &vref, opt)
	if c == nil {
		t.Fatal("fieldInteger: buildCursor returned nil")
	}
	vref = influxql.VarRef{Val: "fieldUnsigned", Type: influxql.Float}
	c = e.buildCursor(testMeas, sKey, tags, &vref, opt)
	if c == nil {
		t.Fatal("fieldInteger: buildCursor returned nil")
	}
	vref = influxql.VarRef{Val: "fieldFloat", Type: influxql.Float}
	c = e.buildCursor(testMeas, sKey, tags, &vref, opt)
	if c == nil {
		t.Fatal("fieldInteger: buildCursor returned nil")
	}
	vref = influxql.VarRef{Val: "fieldInteger", Type: influxql.Integer}
	c = e.buildCursor(testMeas, sKey, tags, &vref, opt)
	if c == nil {
		t.Fatal("fieldInteger: buildCursor returned nil")
	}
	vref = influxql.VarRef{Val: "fieldFloat", Type: influxql.Integer}
	c = e.buildCursor(testMeas, sKey, tags, &vref, opt)
	if c == nil {
		t.Fatal("fieldInteger: buildCursor returned nil")
	}
	vref = influxql.VarRef{Val: "fieldUnsigned", Type: influxql.Integer}
	c = e.buildCursor(testMeas, sKey, tags, &vref, opt)
	if c == nil {
		t.Fatal("fieldInteger: buildCursor returned nil")
	}
	vref = influxql.VarRef{Val: "fieldFloat", Type: influxql.Unsigned}
	c = e.buildCursor(testMeas, sKey, tags, &vref, opt)
	if c == nil {
		t.Fatal("fieldInteger: buildCursor returned nil")
	}
	vref = influxql.VarRef{Val: "fieldInteger", Type: influxql.Unsigned}
	c = e.buildCursor(testMeas, sKey, tags, &vref, opt)
	if c == nil {
		t.Fatal("fieldInteger: buildCursor returned nil")
	}
	vref = influxql.VarRef{Val: "fieldUnsigned", Type: influxql.Unsigned}
	c = e.buildCursor(testMeas, sKey, tags, &vref, opt)
	if c == nil {
		t.Fatal("fieldInteger: buildCursor returned nil")
	}
}

func TestEngine_createCallIterator(t *testing.T) {
	e := MustOpenDefaultEngine()
	defer e.Close()

	e.MeasurementFields([]byte(testMeas)).CreateFieldIfNotExists([]byte("fieldInteger"), influxql.Integer, false)
	e.MeasurementFields([]byte(testMeas)).CreateFieldIfNotExists([]byte("fieldFloat"), influxql.Float, false)
	e.MeasurementFields([]byte(testMeas)).CreateFieldIfNotExists([]byte("fieldUnsigned"), influxql.Unsigned, false)
	e.MeasurementFields([]byte(testMeas)).CreateFieldIfNotExists([]byte("fieldBoolean"), influxql.Boolean, false)
	e.MeasurementFields([]byte(testMeas)).CreateFieldIfNotExists([]byte("fieldString"), influxql.String, false)

	call := &influxql.Call{Name: "top", Args: []influxql.Expr{&influxql.VarRef{Val: "fieldInteger"}, &influxql.NumberLiteral{Val: 1}}}
	opts := query.IteratorOptions{
		Expr: call,
		Aux:  []influxql.VarRef{{Val: "fieldUnsigned", Type: influxql.Float}},
		Condition: &influxql.BinaryExpr{
			Op:  influxql.EQ,
			LHS: &influxql.VarRef{Val: "node"},
			RHS: &influxql.StringLiteral{Val: "N1"},
		},
		//Expr:       influxql.MustParseExpr("Rcv Bytes"),
		StartTime:   influxql.MinTime,
		EndTime:     influxql.MaxTime,
		Ascending:   false,
		Limit:       5,
		Offset:      1,
		InterruptCh: make(chan struct{}),
	}

	_, err := e.createCallIterator(testMeas, call, opts)
	if err != nil {
		t.Fatal(err)
	}
	tags := models.NewTags(map[string]string{"host": "A", "node": "N1"})
	sKey := "meas1,host=A,node=N1"
	err = e.CreateSeriesIfNotExists([]byte(sKey), []byte(testMeas), tags)
	if err != nil {
		t.Fatal(err)
	}
	sKey = "meas1,host=B,node=N1"
	err = e.CreateSeriesIfNotExists([]byte(sKey), []byte(testMeas), tags)
	if err != nil {
		t.Fatal(err)
	}
	_, err = e.createCallIterator(testMeas, call, opts)
	if err == nil {
		t.Fatal("expected error")
	}
}

func TestEngine_createTagSetIterators(t *testing.T) {
	type tdt struct {
		tags      models.Tags
		seriesKey string
	}
	testSeriesData := []tdt{
		{
			tags:      models.NewTags(map[string]string{"host": "A", "node": "N1", "os": "linux"}),
			seriesKey: "meas1,host=A,node=N1,os=linux",
		},
		{
			tags:      models.NewTags(map[string]string{"host": "A", "node": "N1", "os": "mac"}),
			seriesKey: "meas1,host=A,node=N1,os=mac",
		},
		{
			tags:      models.NewTags(map[string]string{"host": "A", "node": "N1", "os": "windows"}),
			seriesKey: "meas1,host=A,node=N1,os=windows",
		},
		{
			tags:      models.NewTags(map[string]string{"host": "B", "node": "N1", "os": "linux"}),
			seriesKey: "meas1,host=B,node=N1,os=linux",
		},
		{
			tags:      models.NewTags(map[string]string{"host": "B", "node": "N1", "os": "mac"}),
			seriesKey: "meas1,host=B,node=N1,os=mac",
		},
		{
			tags:      models.NewTags(map[string]string{"host": "B", "node": "N1", "os": "windows"}),
			seriesKey: "meas1,host=B,node=N1,os=windows",
		},
		{
			tags:      models.NewTags(map[string]string{"host": "C", "node": "N2", "os": "linux"}),
			seriesKey: "meas1,host=C,node=N2,os=linux",
		},
		{
			tags:      models.NewTags(map[string]string{"host": "C", "node": "N2", "os": "mac"}),
			seriesKey: "meas1,host=C,node=N2,os=mac",
		},
		{
			tags:      models.NewTags(map[string]string{"host": "C", "node": "N2", "os": "windows"}),
			seriesKey: "meas1,host=C,node=N2,os=windows",
		},
	}

	e := MustOpenDefaultEngine()
	defer e.Close()

	e.MeasurementFields([]byte(testMeas)).CreateFieldIfNotExists([]byte("fieldInteger"), influxql.Integer, false)
	e.MeasurementFields([]byte(testMeas)).CreateFieldIfNotExists([]byte("fieldFloat"), influxql.Float, false)
	e.MeasurementFields([]byte(testMeas)).CreateFieldIfNotExists([]byte("fieldUnsigned"), influxql.Unsigned, false)
	e.MeasurementFields([]byte(testMeas)).CreateFieldIfNotExists([]byte("fieldBoolean"), influxql.Boolean, false)
	e.MeasurementFields([]byte(testMeas)).CreateFieldIfNotExists([]byte("fieldString"), influxql.String, false)

	//call := &influxql.Call{Name: "top", Args: []influxql.Expr{&influxql.VarRef{Val: "fieldInteger"}, &influxql.NumberLiteral{Val: 1}}}
	opts := query.IteratorOptions{
		Condition: &influxql.BinaryExpr{
			Op: influxql.AND,
			LHS: &influxql.BinaryExpr{
				Op:  influxql.NEQ,
				LHS: &influxql.VarRef{Val: "None1", Type: influxql.String},
				RHS: &influxql.StringLiteral{Val: "None2"},
			},
			RHS: &influxql.BinaryExpr{
				Op:  influxql.NEQ,
				LHS: &influxql.VarRef{Val: "None3", Type: influxql.String},
				RHS: &influxql.StringLiteral{Val: "blah"},
			}},
		Dimensions: []string{"host", "node"},
		GroupBy: map[string]struct{}{
			"host": {},
			"node": {},
		},
		StartTime:   influxql.MinTime,
		EndTime:     influxql.MaxTime,
		Ascending:   false,
		Limit:       5,
		Offset:      1,
		InterruptCh: make(chan struct{}),
	}

	for _, td := range testSeriesData {
		tags := td.tags
		sKey := td.seriesKey
		err := e.CreateSeriesIfNotExists([]byte(sKey), []byte(testMeas), tags)
		if err != nil {
			t.Fatal(err)
		}
	}
	// Determine tagsets for this measurement based on dimensions and filters.
	tagSets, err := e.index.TagSets([]byte(testMeas), opts)
	if err != nil {
		t.Fatal(err)
	}

	varRef := &influxql.VarRef{Val: "fieldInteger"}
	for _, ts := range tagSets {
		inputs, err := e.createTagSetIterators(varRef, testMeas, ts, opts)
		if err != nil {
			t.Fatal(err)
		} else if len(inputs) != 3 {
			t.Fatalf("Expected 1, got %d", len(inputs))
		}
	}

	opts.MaxSeriesN = 1
	ic := make(chan struct{}, 3)
	e.createTagSetIterators(varRef, testMeas, tagSets[0], opts)

	opts.StripName = true
	opts.MaxSeriesN = 0
	opts.Condition = &influxql.BinaryExpr{
		Op: influxql.AND,
		LHS: &influxql.BinaryExpr{
			Op:  influxql.NEQ,
			LHS: &influxql.VarRef{Val: "Blah", Type: influxql.Unsigned},
			RHS: &influxql.UnsignedLiteral{Val: uint64(222)},
		},
		RHS: &influxql.BinaryExpr{
			Op:  influxql.NEQ,
			LHS: &influxql.VarRef{Val: "BlahBlah", Type: influxql.Integer},
			RHS: &influxql.IntegerLiteral{Val: 777},
		}}
	tagSets, err = e.index.TagSets([]byte(testMeas), opts)
	if err != nil {
		t.Fatal(err)
	}
	_, err = e.createTagSetIterators(varRef, testMeas, tagSets[0], opts)
	if err != nil {
		t.Logf("Expected interrupt error")
	}
	opts.Condition = &influxql.BinaryExpr{
		Op: influxql.AND,
		LHS: &influxql.BinaryExpr{
			Op:  influxql.NEQ,
			LHS: &influxql.VarRef{Val: "Blah", Type: influxql.Float},
			RHS: &influxql.NumberLiteral{Val: 3.14},
		},
		RHS: &influxql.BinaryExpr{
			Op:  influxql.NEQ,
			LHS: &influxql.VarRef{Val: "_tagKey", Type: influxql.Boolean},
			RHS: &influxql.BooleanLiteral{Val: true},
		}}
	tagSets, err = e.index.TagSets([]byte(testMeas), opts)
	if err != nil {
		t.Fatal(err)
	}
	_, err = e.createTagSetIterators(varRef, testMeas, tagSets[0], opts)
	if err != nil {
		t.Logf("Expected interrupt error")
	}

	opts.InterruptCh = ic
	var k struct{}
	ic <- k
	_, err = e.createTagSetIterators(varRef, testMeas, tagSets[0], opts)
	if err != nil {
		t.Logf("Expected interrupt error")
	}

}
