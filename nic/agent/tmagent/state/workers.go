package state

import (
	"context"

	"github.com/pensando/sw/venice/utils/log"
)

type workers struct {
	ctx        context.Context
	numWorkers int
	workItems  chan func()
}

func (w *workers) postWorkItem(wi func()) {
	w.workItems <- wi
	metric.addPendingItem()
}

func doWork(ctx context.Context, workItems <-chan func()) {
	defer func() {
		if r := recover(); r != nil {
			log.Errorf("recovered in objstore's worker routine %s", r)
		}
	}()

	for {
		select {
		case <-ctx.Done():
			return
		case wi := <-workItems:
			wi()
			metric.subtractPendingItem()
		}
	}
}

// NewWorkers starts a new work farm
func newWorkers(ctx context.Context, numWorkers int, bufferSize int) *workers {
	w := &workers{ctx: ctx, numWorkers: numWorkers, workItems: make(chan func(), bufferSize)}
	for i := 0; i < numWorkers; i++ {
		go doWork(ctx, w.workItems)
	}
	return w
}
