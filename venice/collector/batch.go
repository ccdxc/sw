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
)

// BatchWriter is an abstraction that accumulates a threshold set of points over a specified period.
// When the threshold is met or the period expires, write of the whole batch is triggered. When the
// write completes, all individual senders are notified and the lifecycle of that BatchWriter is
// finished. BatchWriter is thread-safe.
type BatchWriter interface {
	// AddPoints adds a list of points to the batch. True on success. False indicates batch is full.
	// In that case, caller should create a new batch.
	AddPoints([]models.Point) bool

	// WaitCompletion blocks until the batch write finishes
	WaitCompletion()

	// WithPeriod sets the period for the batch
	WithPeriod(time.Duration) BatchWriter

	// WithSize sets the size of the batch
	WithSize(int) BatchWriter
}

// infBatch is the default implementation of BatchWriter, meant for influxdb
type infBatch struct {
	mutex       sync.Mutex
	ctx         context.Context
	bp          influx.BatchPoints
	writer      WriterFunc // writer handles the backend interactions
	doneCh      chan bool
	sizeTrigger chan bool
	period      time.Duration
	size        int
	closed      bool
}

// WriterFunc defines a function type that writes to backends
type WriterFunc func(influx.BatchPoints)

// NewBatch creates a BatchWriter instance for influx
func NewBatch(ctx context.Context, dbName, precision string, w WriterFunc) BatchWriter {
	bp, _ := influx.NewBatchPoints(influx.BatchPointsConfig{Database: dbName, Precision: precision})
	b := &infBatch{
		ctx:         ctx,
		bp:          bp,
		writer:      w,
		doneCh:      make(chan bool),
		sizeTrigger: make(chan bool),
		period:      defaultBatchPeriod,
		size:        defaultBatchSize,
	}

	go b.waitToSend()

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

// WaitCompletion blocks until the batch write finishes
func (b *infBatch) WaitCompletion() {
	<-b.doneCh
}

// WithPeriod sets the period for the batch
func (b *infBatch) WithPeriod(t time.Duration) BatchWriter {
	b.period = t
	return b
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
		b.writer(b.bp)
		break
	case <-b.sizeTrigger:
		b.writer(b.bp)
		break

	case <-b.ctx.Done():
		break
	}
}
