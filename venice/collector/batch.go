package collector

import (
	"context"
	"sync"
	"time"

	influx "github.com/influxdata/influxdb/client/v2"
	"github.com/influxdata/influxdb/models"
)

const (
	defaultBatchSize = 4 * 1024
	defPrecision     = "ms"
)

// BatchWriter is an abstraction that accumulates a threshold set of points over a specified period.
// When the threshold is met or the period expires, write of the whole batch is triggered. When the
// write completes, all individual senders are notified and the lifecycle of that BatchWriter is
// finished. BatchWriter is thread-safe.
type BatchWriter interface {
	// AddPoints adds a list of points to the batch. True on success. False indicates batch is full.
	// In that case, caller should create a new batch.
	AddPoints([]models.Point) bool
	// AddLines adds a list of strings to the batch.
	AddLines([]string) bool

	// Points returns the points of the batch
	Points() influx.BatchPoints
	// Lines returns the db name and lines of the batch
	Lines() (string, []string)

	// WaitCompletion blocks until the batch write finishes
	WaitCompletion()

	// WithSize sets the size of the batch
	WithSize(int) BatchWriter
}

// infBatch is the default implementation of BatchWriter, meant for influxdb
type infBatch struct {
	mutex       sync.Mutex
	ctx         context.Context
	bp          influx.BatchPoints
	lines       []string
	writer      WriterFunc // writer handles the backend interactions
	doneCh      chan bool
	sizeTrigger chan bool
	period      time.Duration
	size        int
	closed      bool
	dbName      string
	precision   string
}

type batchStats struct {
	created      int64 // # of batches created
	timeTriggers int64 // triggered by batch timer
	sizeTriggers int64 // triggered by batch full
	cancels      int64
}

var bStats batchStats

// WriterFunc defines a function type that writes to backends
type WriterFunc func(BatchWriter)

// NewBatch creates a BatchWriter instance for influx
func NewBatch(ctx context.Context, dbName string, period time.Duration, w WriterFunc) BatchWriter {
	bp, _ := influx.NewBatchPoints(influx.BatchPointsConfig{Database: dbName, Precision: defPrecision})
	b := &infBatch{
		ctx:         ctx,
		bp:          bp,
		writer:      w,
		doneCh:      make(chan bool),
		sizeTrigger: make(chan bool),
		period:      period,
		size:        defaultBatchSize,
		lines:       make([]string, 0, defaultBatchSize),
		dbName:      dbName,
		precision:   defPrecision,
	}

	go b.waitToSend()

	bStats.created++
	return b
}

// AddPoints adds a list of points to the batch. True on success. False indicates batch is full.
func (b *infBatch) AddPoints(ps []models.Point) bool {
	b.mutex.Lock()
	defer b.mutex.Unlock()

	if b.closed {
		return false
	}

	// TODO: fix this influx client non-sense
	tmp := make([]*influx.Point, len(ps))
	for ix, p := range ps {
		tmp[ix] = influx.NewPointFrom(p)
	}

	b.bp.AddPoints(tmp)
	if len(b.bp.Points()) >= b.size {
		b.closed = true
		b.sizeTrigger <- true
	}
	return true
}

// AddLines adds a list of lines to the batch. True on success. False indicates batch is full.
func (b *infBatch) AddLines(ls []string) bool {
	b.mutex.Lock()
	defer b.mutex.Unlock()

	if b.closed {
		return false
	}

	b.lines = append(b.lines, ls...)
	if len(b.lines) >= b.size {
		b.closed = true
		b.sizeTrigger <- true
	}
	return true
}

// Points returns the points of the batch
func (b *infBatch) Points() influx.BatchPoints {
	return b.bp
}

// Lines returns the lines of the batch
func (b *infBatch) Lines() (string, []string) {
	return b.dbName, b.lines
}

// WaitCompletion blocks until the batch write finishes
func (b *infBatch) WaitCompletion() {
	<-b.doneCh
}

// WithSize sets the size of the batch
func (b *infBatch) WithSize(s int) BatchWriter {
	b.size = s
	return b
}

func (b *infBatch) waitToSend() {
	defer close(b.doneCh)

	select {
	case <-time.After(b.period):
		b.mutex.Lock()
		b.closed = true
		b.mutex.Unlock()
		b.writer(b)
		bStats.timeTriggers++
		break
	case <-b.sizeTrigger:
		b.writer(b)
		bStats.sizeTriggers++
		break

	case <-b.ctx.Done():
		bStats.cancels++
		break
	}
}
