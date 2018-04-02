package collector

import (
	"context"
	"fmt"
	"sync"
	"time"

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
	Close()
	Write(interface{}, *sync.WaitGroup)
	WriteLines(string, []string, *sync.WaitGroup)
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
	incrID        uint32
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

	beID := fmt.Sprintf("%s:%d", addr, c.incrID)
	c.incrID++
	c.backends[beID] = be
	go c.watchErrors(beID, errCh)
	return nil
}

// WritePoints adds points to a batch which get sent out when the batch closes
func (c *Collector) WritePoints(db, meas string, points []models.Point) {
	key := db + meas
	wf := func(b BatchWriter) {
		// write to all backends and block until completion
		var batchWG sync.WaitGroup
		c.mutex.RLock()
		for _, be := range c.backends {
			batchWG.Add(1)
			be.Write(b.Points(), &batchWG)
		}
		c.mutex.RUnlock()

		batchWG.Wait()
	}

	b := c.atomicNewBatch(nil, key, db, wf)
	for !b.AddPoints(points) {
		// retry to handle batch close
		b = c.atomicNewBatch(b, key, db, wf)
	}

	b.WaitCompletion()
}

// WriteLines adds points to a batch which get sent out when the batch closes
func (c *Collector) WriteLines(db string, lines []string) {
	key := db
	lwf := func(b BatchWriter) {
		// write to all backends and block until completion
		var batchWG sync.WaitGroup

		db, lines := b.Lines()
		if len(lines) == 0 {
			return
		}
		c.mutex.RLock()
		for _, be := range c.backends {
			batchWG.Add(1)
			be.WriteLines(db, lines, &batchWG)
		}
		c.mutex.RUnlock()

		batchWG.Wait()
	}

	b := c.atomicNewBatch(nil, key, db, lwf)
	for !b.AddLines(lines) {
		// retry to handle batch close
		b = c.atomicNewBatch(b, key, db, lwf)
	}

	b.WaitCompletion()
}

func (c *Collector) atomicNewBatch(b BatchWriter, db, key string, w WriterFunc) BatchWriter {
	c.mutex.Lock()
	if c.activeBatches[key] == b {

		b = NewBatch(c.ctx, db, c.batchPeriod, w).WithSize(c.batchSize)
		c.activeBatches[key] = b
	}

	b = c.activeBatches[key]
	c.mutex.Unlock()

	return b
}

// watchErrors monitors a backend
func (c *Collector) watchErrors(beID string, errCh <-chan error) {
	cleanup := func() {
		c.mutex.Lock()
		be := c.backends[beID]
		delete(c.backends, beID)
		c.mutex.Unlock()
		if be != nil {
			go be.Close()
		}
	}
	for {
		select {
		case err, active := <-errCh:
			if !active {
				// this means we closed the backend
				return
			}
			log.Infof("Backend %s went down with %s", beID, err.Error())
			cleanup()
			// TODO notify controller
		case <-c.ctx.Done():
			cleanup()
		}
	}
}
