package debug

import (
	"expvar"
	"net/http"
	"runtime"
	"strings"
	"sync"
	"time"

	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/tsdb"
)

const defaultKind = "debugstats"

type tsdbMetadata struct {
	Kind string
	Name string
}

// Stats stores counters for debug purpose
type Stats struct {
	tsdb             bool
	tsdbPushDuration time.Duration
	debugs           *expvar.Map
	tableObj         tsdb.TableObj
	tsdbCloseCh      chan bool
	closeAckCh       chan bool
}

// StatFactory creates stats that are maintained as expvar hash
//  (and exposed via http handler) and optionally pushed to tsdb
type StatFactory struct {
	name             string
	kind             string
	tsdb             bool
	tsdbPushDuration time.Duration
}

// CPUStats is collection of runtime process stats
type CPUStats struct {
	NumCPUs       uint16
	NumGoroutines uint64
	NumCgoCalls   uint64
}

var once sync.Once
var mutex sync.Mutex
var allStats []*Stats

// Publish runtime process stats
func init() {
	once.Do(func() {
		http.HandleFunc("/debug/vars/clear", clearHandler)
		expvar.Publish("cpustats", expvar.Func(GetCPUStats))
	})
}

// New stats factory
func New(instance string) *StatFactory {
	// tools are more happy without ':'  Hence convert that to _
	name := strings.Replace(instance, ":", "_", -1)
	sf := &StatFactory{
		name: name,
		kind: defaultKind,
	}
	return sf
}

// Build the Stat container
func (sf *StatFactory) Build() *Stats {
	debugInstance := sf.kind + "_" + sf.name
	var debugs *expvar.Map
	// If it already exists, return it
	vars := expvar.Get(debugInstance)
	if vars == nil {
		debugs = expvar.NewMap(debugInstance)
	} else {
		var ok bool
		debugs, ok = vars.(*expvar.Map)
		if !ok {
			log.Fatalf("expvar already has non-map object with name %+v", debugInstance)
		}
	}
	s := &Stats{
		debugs:           debugs,
		tsdb:             sf.tsdb,
		tsdbPushDuration: sf.tsdbPushDuration,
		tsdbCloseCh:      make(chan bool),
		closeAckCh:       make(chan bool),
	}
	mutex.Lock()
	allStats = append(allStats, s)
	mutex.Unlock()

	if sf.tsdb {
		tsdbMeta := tsdbMetadata{Kind: sf.kind, Name: sf.name}
		var err error
		s.tableObj, err = tsdb.NewTableObj(tsdbMeta, tsdb.Config{})
		if err != nil {
			log.Fatalf("tsdb.NewTableObj with %+v gave error %#v", tsdbMeta, err)
		}
		if sf.tsdbPushDuration != 0 {
			go s.startTsdbTimer(sf.tsdbPushDuration)
		}
	}
	return s
}

// Kind sets the kind of the stats object. In http handler, the key is kind_instance
//	in tsdb, kind refers to the measurement. instance is a tag (with key 'name')
func (sf *StatFactory) Kind(k string) *StatFactory {
	sf.kind = k
	return sf
}

// Tsdb sends the stats to tsdb
func (sf *StatFactory) Tsdb() *StatFactory {
	sf.tsdb = true
	return sf
}

// TsdbPeriod periodically sends the stats to tsdb else send on every change
func (sf *StatFactory) TsdbPeriod(d time.Duration) *StatFactory {
	sf.tsdbPushDuration = d
	return sf
}

// ClearHandler is http handler to clear debug stats
func ClearHandler() http.Handler {
	return http.HandlerFunc(clearHandler)
}

func clearHandler(w http.ResponseWriter, r *http.Request) {
	Clear()
}

// Clear clears all debug stats
func Clear() {
	mutex.Lock()
	defer mutex.Unlock()
	for _, m := range allStats {
		m.debugs.Init()
	}
}
func (st *Stats) sendPoints() {
	point := tsdb.Point{Tags: st.tableObj.Tags()}
	point.Fields = make(map[string]interface{})
	st.debugs.Do(func(kv expvar.KeyValue) {
		switch kv.Value.(type) {
		case *expvar.Int:
			v := kv.Value.(*expvar.Int)
			point.Fields[kv.Key] = v.Value()
		case *expvar.Float:
			v := kv.Value.(*expvar.Float)
			point.Fields[kv.Key] = v.Value()
		case *expvar.String:
			v := kv.Value.(*expvar.String)
			point.Fields[kv.Key] = v.Value()
		}
	})
	if len(point.Fields) > 0 {
		st.tableObj.AddPoints([]tsdb.Point{point})
	}
}

func (st *Stats) startTsdbTimer(d time.Duration) {
	for {
		select {
		case <-time.After(d):
			st.sendPoints()
		case <-st.tsdbCloseCh:
			st.sendPoints()
			close(st.closeAckCh)
			return
		}
	}
}

// Close stops any pending goroutines in background and clears stats
func (st *Stats) Close() {
	if st.tsdb && st.tsdbPushDuration != 0 {
		close(st.tsdbCloseCh)
		<-st.closeAckCh
	}
	st.debugs.Init()
}

// Increment increments this counter
func (st *Stats) Increment(counterName string) {
	st.debugs.Add(counterName, 1)

	if st.tsdb && st.tsdbPushDuration == 0 {
		st.sendPoints()
	}
}

// AddFloat adds a float value to the counter entry
func (st *Stats) AddFloat(counterName string, value float64) {
	st.debugs.AddFloat(counterName, value)
	if st.tsdb && st.tsdbPushDuration == 0 {
		st.sendPoints()
	}
}

// AddInt adds an interger to the counter entry
func (st *Stats) AddInt(counterName string, value int64) {
	st.debugs.Add(counterName, value)
	if st.tsdb && st.tsdbPushDuration == 0 {
		st.sendPoints()
	}
}

// GetCPUStats returns runtime process stats
func GetCPUStats() interface{} {
	stats := CPUStats{
		NumCPUs:       uint16(runtime.NumCPU()),
		NumGoroutines: uint64(runtime.NumGoroutine()),
		NumCgoCalls:   uint64(runtime.NumCgoCall()),
	}
	return &stats
}
