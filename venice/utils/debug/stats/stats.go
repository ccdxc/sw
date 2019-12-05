package debugStats

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
	sync.Mutex
	name             string
	tsdb             bool
	tsdbPushDuration time.Duration
	tsdbKeys         map[string]string
	debugs           *expvar.Map
	tableObj         tsdb.Obj
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
	stats            *Stats
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
	if sf.stats != nil {
		sf.stats.Lock()
		if sf.stats.tsdbCloseCh != nil {
			// stats is created and still open
			sf.stats.Unlock()
			return sf.stats
		}
		sf.stats.Unlock()
	}
	debugInstance := sf.kind + "_" + sf.name
	mutex.Lock()
	defer mutex.Unlock()
	// If it already exists, return it
	var s *Stats
	for _, v := range allStats {
		if v.name == debugInstance {
			s = v
			break
		}
	}
	if s != nil {
		sf.stats = s
		return s
	}

	var debugs *expvar.Map
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
	s = &Stats{
		name:             debugInstance,
		debugs:           debugs,
		tsdb:             sf.tsdb,
		tsdbPushDuration: sf.tsdbPushDuration,
		tsdbCloseCh:      make(chan bool, 1),
		closeAckCh:       make(chan bool, 1),
	}
	allStats = append(allStats, s)

	if sf.tsdb {
		var err error

		s.tsdbKeys = map[string]string{"Kind": sf.kind, "Name": sf.name}
		s.tableObj, err = tsdb.NewObj("objStats", s.tsdbKeys, nil, &tsdb.ObjOpts{})
		if err != nil {
			log.Fatalf("tsdb.NewTableObj with %+v gave error %#v", s.tsdbKeys, err)
		}
		if sf.tsdbPushDuration != 0 {
			go s.startTsdbTimer(sf.tsdbPushDuration)
		}
	}
	sf.stats = s
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
	fields := make(map[string]interface{})
	st.debugs.Do(func(kv expvar.KeyValue) {
		switch kv.Value.(type) {
		case *expvar.Int:
			v := kv.Value.(*expvar.Int)
			fields[kv.Key] = v.Value()
		case *expvar.Float:
			v := kv.Value.(*expvar.Float)
			fields[kv.Key] = v.Value()
		case *expvar.String:
			v := kv.Value.(*expvar.String)
			fields[kv.Key] = v.Value()
		}
	})
	if len(fields) > 0 {
		tsdbPoint := tsdb.Point{Tags: st.tsdbKeys, Fields: fields}
		st.tableObj.Points([]*tsdb.Point{&tsdbPoint}, time.Now())
	}
}

func (st *Stats) startTsdbTimer(d time.Duration) {
	for {
		select {
		case <-st.tsdbCloseCh:
			st.sendPoints()
			close(st.closeAckCh)
			return
		case <-time.After(d):
			st.sendPoints()
		}
	}
}

// Close stops any pending goroutines in background and clears stats
func (st *Stats) Close() {
	st.Lock()
	defer st.Unlock()
	if st.tsdbCloseCh == nil {
		// already closed
		return
	}
	if st.tsdb && st.tsdbPushDuration != 0 {
		st.tsdbCloseCh <- true
		close(st.tsdbCloseCh)
		<-st.closeAckCh
		st.tsdbCloseCh = nil
	}
	st.debugs.Init()
	indexToRemove := -1
	for i, v := range allStats {
		if v.name == st.name {
			indexToRemove = i
		}
	}
	if indexToRemove != -1 {
		allStats = append(allStats[:indexToRemove], allStats[indexToRemove+1:]...)
	}
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
