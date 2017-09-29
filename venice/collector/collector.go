package collector

import (
	"context"
	"fmt"
	"sync"
	"time"

	influx "github.com/influxdata/influxdb/client/v2"
	"github.com/influxdata/influxdb/models"

	"github.com/pensando/sw/venice/utils/log"
)

const (
	beTimeout          = 9 * time.Second
	defaultBatchPeriod = 15 * time.Second
)

// Backend defines a backend interface
type Backend interface {
	Open(BEConfig, chan<- error) error
	Write(interface{}, *sync.WaitGroup)
}

// BEConfig defines backend config
type BEConfig struct {
	// Addr should be of the form "http://host:port"
	Addr string
	// Username is the influxdb username, optional.
	// TODO: TLS support
	Username string
	// Password is the influxdb password, optional.
	Password string

	// Timeout for all influxdb accesses
	Timeout time.Duration
}

// Collector collects points in batches and sends to backends
type Collector struct {
	ctx           context.Context
	mutex         sync.RWMutex
	batchPeriod   time.Duration
	batchSize     int
	backends      map[string]Backend
	activeBatches map[string]BatchWriter
}

// NewCollector returns a new instance of Collector with default settings
func NewCollector(ctx context.Context) *Collector {
	return &Collector{
		ctx:           ctx,
		batchPeriod:   defaultBatchPeriod,
		batchSize:     defaultBatchSize,
		backends:      make(map[string]Backend),
		activeBatches: make(map[string]BatchWriter),
	}
}

// WithPeriod sets the batch period of a collector
func (c *Collector) WithPeriod(p time.Duration) *Collector {
	c.batchPeriod = p
	return c
}

// WithSize sets the size of batches managed by a collector
func (c *Collector) WithSize(n int) *Collector {
	c.batchSize = n
	return c
}

// AddBackEnd adds a new backend to the collector
func (c *Collector) AddBackEnd(addr string) error {
	c.mutex.Lock()
	defer c.mutex.Unlock()
	if _, ok := c.backends[addr]; ok {
		return fmt.Errorf("Backend exists")
	}

	be := NewInfluxBE()
	errCh := make(chan error)
	err := be.Open(BEConfig{Addr: addr, Timeout: beTimeout}, errCh)
	if err != nil {
		return err
	}

	c.backends[addr] = be
	go c.watchErrors(addr, errCh)
	return nil
}

// WritePoints adds points to a batch which get sent out when the batch closes
func (c *Collector) WritePoints(db, meas string, points []models.Point) {
	key := db + meas
	wf := func(bp influx.BatchPoints) {
		// write to all backends and block until completion
		var wg sync.WaitGroup
		for _, be := range c.readBackends() {
			wg.Add(1)
			go be.Write(bp, &wg)
		}

		wg.Wait()
	}
	c.mutex.Lock()
	b := c.activeBatches[key]
	if b == nil {
		b = NewBatch(c.ctx, db, "ms", wf).WithPeriod(c.batchPeriod).WithSize(c.batchSize)
		c.activeBatches[key] = b
	}
	c.mutex.Unlock()

	for !b.AddPoints(points) {
		b = NewBatch(c.ctx, db, "ms", wf).WithPeriod(c.batchPeriod).WithSize(c.batchSize)
		c.mutex.Lock()
		c.activeBatches[key] = b
		c.mutex.Unlock()
	}

	b.WaitCompletion()
}

func (c *Collector) readBackends() []Backend {
	c.mutex.RLock()
	defer c.mutex.RUnlock()

	res := make([]Backend, 0, len(c.backends))
	for _, be := range c.backends {
		res = append(res, be)
	}

	return res
}

// watchErrors monitors a backend
func (c *Collector) watchErrors(addr string, errCh <-chan error) {
	select {
	case <-errCh:
		log.Infof("Backend %s went down", addr)
		c.mutex.Lock()
		delete(c.backends, addr)
		c.mutex.Unlock()
		// TODO notify controller
	case <-c.ctx.Done():
	}
}
