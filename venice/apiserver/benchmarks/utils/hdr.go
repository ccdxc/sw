package report

import (
	"fmt"
	"sync"
	"time"

	hdr "github.com/codahale/hdrhistogram"
)

var histmap sync.Map

// Record records an measurement in the <name> hdr.
func Record(name string, d time.Duration) {
	if h, ok := histmap.Load(name); !ok {
		h = hdr.New(0, 100000000000, 4)
		histmap.Store(name, h)
		h.(*hdr.Histogram).RecordValue(d.Nanoseconds())
	} else {
		h.(*hdr.Histogram).RecordValue(d.Nanoseconds())
	}
}

func printHdr(key, value interface{}) bool {
	h := value.(*hdr.Histogram)
	fmt.Printf("Name: %s Total: %d Mean: %.3f Max: %.3f Min: %.3f\n", key, h.TotalCount(),
		float64(h.Mean())/float64(time.Millisecond),
		float64(h.Max())/float64(time.Millisecond),
		float64(h.Min())/float64(time.Millisecond))
	return true
}

// PrintAll prints all the hdrs
func PrintAll() {
	histmap.Range(printHdr)
}
