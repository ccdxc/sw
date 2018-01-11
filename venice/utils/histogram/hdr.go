package histogram

import (
	"expvar"
	"fmt"
	"sync"
	"time"

	hdr "github.com/codahale/hdrhistogram"
)

// HistMap is a container for the histograms
type HistMap struct {
	histmap   sync.Map
	histcount sync.Map
}

// Record records an measurement in the <name> histogram.
func (m *HistMap) Record(name string, d time.Duration) {
	if h, ok := m.histmap.Load(name); !ok {
		h = hdr.New(0, 100000000000, 4)
		m.histmap.Store(name, h)
		m.histcount.Store(name, uint64(1))
		h.(*hdr.Histogram).RecordValue(d.Nanoseconds())
	} else {
		if c, ok := m.histcount.Load(name); ok {
			cnt := c.(uint64) + 1
			m.histcount.Store(name, cnt)
		}
		h.(*hdr.Histogram).RecordValue(d.Nanoseconds())
	}
}

func (m *HistMap) printone(key string, h *hdr.Histogram) {
	c, _ := m.histcount.Load(key)
	cnt := c.(uint64)
	fmt.Printf("--[ %s ] Total: %d/%d Mean: %.3f Max: %.3f Min: %.3f\n", key, h.TotalCount(), cnt,
		float64(h.Mean())/float64(time.Millisecond),
		float64(h.Max())/float64(time.Millisecond),
		float64(h.Min())/float64(time.Millisecond))
	fmt.Printf("     10: %.3f 50: %.3f 90: %.3f 99: %.3f\n",
		float64(h.ValueAtQuantile(10))/float64(time.Millisecond),
		float64(h.ValueAtQuantile(50))/float64(time.Millisecond),
		float64(h.ValueAtQuantile(90))/float64(time.Millisecond),
		float64(h.ValueAtQuantile(99))/float64(time.Millisecond))
}

// PrintAll prints all the histograms
func (m *HistMap) PrintAll() {
	printhdr := func(key, value interface{}) bool {
		h := value.(*hdr.Histogram)
		m.printone(key.(string), h)
		return true
	}
	m.histmap.Range(printhdr)
}

// PrintOne prints one histogram
func (m *HistMap) PrintOne(key string) {
	if value, ok := m.histmap.Load(key); ok {
		h := value.(*hdr.Histogram)
		m.printone(key, h)
	}
}

// GetStats returns a map of all collected Stats in human friendly form
func (m *HistMap) GetStats() map[string]Stats {
	ret := make(map[string]Stats)
	fn := func(key, value interface{}) bool {
		h := value.(*hdr.Histogram)
		k := key.(string)
		stat := Stats{
			Count:       h.TotalCount(),
			MinMs:       float64(h.Min()) / float64(time.Millisecond),
			MaxMs:       float64(h.Max()) / float64(time.Millisecond),
			MeanMs:      float64(h.Mean()) / float64(time.Millisecond),
			StdDev:      h.StdDev(),
			Perctl10:    float64(h.ValueAtQuantile(10)) / float64(time.Millisecond),
			Perctl50:    float64(h.ValueAtQuantile(50)) / float64(time.Millisecond),
			Perctl75:    float64(h.ValueAtQuantile(75)) / float64(time.Millisecond),
			Perctl90:    float64(h.ValueAtQuantile(90)) / float64(time.Millisecond),
			Perctl99:    float64(h.ValueAtQuantile(99)) / float64(time.Millisecond),
			Perctl99_99: float64(h.ValueAtQuantile(99.99)) / float64(time.Millisecond),
		}
		ret[k] = stat
		return true
	}
	m.histmap.Range(fn)
	return ret
}

var defHistMap HistMap

// Record records an measurement in the <name> histogram
func Record(name string, d time.Duration) {
	defHistMap.Record(name, d)
}

// PrintAll prints all the hdrs in default histmap
func PrintAll() {
	defHistMap.PrintAll()
}

// PrintOne prints one histogram in default histmap
func PrintOne(key string) {
	defHistMap.PrintOne(key)
}

// GetStats returns a map of all collected Stats in human friendly form
func GetStats() map[string]Stats {
	return defHistMap.GetStats()
}

// Stats shows the histogram in human friendly format
type Stats struct {
	Count                                                         int64
	MinMs, MaxMs, MeanMs, StdDev                                  float64
	Perctl10, Perctl50, Perctl75, Perctl90, Perctl99, Perctl99_99 float64
}

func init() {
	var histfunc expvar.Func
	histfunc = func() interface{} {
		return GetStats()
	}
	expvar.Publish("histograms", histfunc)
}
