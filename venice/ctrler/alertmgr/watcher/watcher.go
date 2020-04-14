// Package watcher creates and runs the API group watchers and outputs watch events on a single output channel.
package watcher

import (
	"context"
	"fmt"
	"net/http"
	"reflect"
	"strings"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/errors"
	apiservice "github.com/pensando/sw/api/generated/apiclient"
	objectdb "github.com/pensando/sw/venice/ctrler/alertmgr/objdb"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

const (
	apiSrvWaitIntvl  = time.Second
	maxAPISrvRetries = 200
)

// Interface for watcher.
type Interface interface {
	// Run watcher in a separate go routine until a fatal error is encountered.
	Run(ctx context.Context, apiClient apiservice.Services) (<-chan *kvstore.WatchEvent, <-chan error, error)

	// Stop watcher.
	Stop()

	// GetRunningStatus of watcher.
	GetRunningStatus() bool
}

type watcher struct {
	// AlertMgr logger.
	logger log.Logger

	// Watcher run context.
	ctx    context.Context
	cancel context.CancelFunc

	// AlertMgr API client.
	apiClient apiservice.Services

	// AlertMgr Object db.
	objdb objectdb.Interface

	// API group watchers.
	groupWatchers map[string]kvstore.Watcher

	// Watcher output channel.
	// Output channel is closed when the watcher is explicitly stopped.
	outCh chan *kvstore.WatchEvent

	// Watcher error channel.
	// AlertMgr must monitor the error channel and shutdown on error, allowing itself to be restarted.
	// Error channel is never closed (it will be garbage collected).
	errCh chan error

	// Running status of watcher.
	// No mutex required as this is set and read only by the alertmgr run goroutine.
	running bool
}

// New watcher instance.
func New(objdb objectdb.Interface, logger log.Logger) (Interface, error) {
	w := &watcher{
		logger: logger,
		objdb:  objdb}

	logger.Infof("Created new watcher")
	return w, nil
}

func (w *watcher) Run(ctx context.Context, apiClient apiservice.Services) (<-chan *kvstore.WatchEvent, <-chan error, error) {
	if w.running {
		return nil, nil, fmt.Errorf("Watcher already running")
	}

	w.ctx, w.cancel = context.WithCancel(ctx)
	w.apiClient = apiClient
	w.outCh = make(chan *kvstore.WatchEvent)
	w.errCh = make(chan error, 1)

	go func() {
		defer w.cleanup()

		err := w.createWatchers()
		if err != nil {
			if err != w.ctx.Err() {
				w.errCh <- err
			}
			return
		}

		err = w.startWatchers()
		if err != nil {
			if err != w.ctx.Err() {
				w.errCh <- err
			}
			return
		}
	}()

	w.running = true
	w.logger.Infof("Started watcher")
	return w.outCh, w.errCh, nil
}

func (w *watcher) Stop() {
	w.cancel()
}

func (w *watcher) GetRunningStatus() bool {
	return w.running
}

type service interface {
	Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error)
}

// Creates API group watchers.
func (w *watcher) createWatchers() error {
	w.groupWatchers = make(map[string]kvstore.Watcher)

	groupMap := runtime.GetDefaultScheme().Kinds()
	apiClientVal := reflect.ValueOf(w.apiClient)

	for group := range groupMap {
		if group == "objstore" || group == "bookstore" {
			continue
		}

		version := "V1"
		key := strings.Title(group) + version
		groupFunc := apiClientVal.MethodByName(key)
		if !groupFunc.IsValid() {
			w.logger.Infof("Invalid API Group %s", key)
			continue
		}

		// Get current resource version from Objdb.
		opts := api.ListWatchOptions{}
		resVersion := w.objdb.GetResourceVersion(group)
		opts.ObjectMeta = api.ObjectMeta{
			ResourceVersion: resVersion,
		}

		serviceGroup := groupFunc.Call(nil)
		if serviceGroup[0].Interface() == nil {
			continue
		}
		watch, err := serviceGroup[0].Interface().(service).Watch(w.ctx, &opts)

		if err != nil {
			apiErr := apierrors.FromError(err)
			switch apiErr.GetCode() {
			case http.StatusNotImplemented:
				w.logger.Errorf("Error starting watcher for API Group %s, err unimplemented: %v", key, err)
				return err
			case http.StatusGone:
				w.logger.Errorf("Error starting watcher for API Group %s, outdated res version: %v", key, err)
				return err
			default:
				w.logger.Errorf("Error starting watcher for API Group %s, err: %v", key, err)
				return err
			}
		} else {
			w.logger.Debugf("Created watcher for API Group %s", key)
			w.groupWatchers[key] = watch
		}
	}

	return nil
}

func (w *watcher) startWatchers() error {
	cases := make([]reflect.SelectCase, len(w.groupWatchers)+1)
	apiGroupMappings := make([]string, len(w.groupWatchers)+1)

	// Look for ctx.Done.
	cases[0] = reflect.SelectCase{Dir: reflect.SelectRecv, Chan: reflect.ValueOf(w.ctx.Done())}

	i := 1
	for key, wch := range w.groupWatchers {
		ch := wch.EventChan()
		cases[i] = reflect.SelectCase{Dir: reflect.SelectRecv, Chan: reflect.ValueOf(ch)}
		apiGroupMappings[i] = key
		i++
	}

	for {
		chosen, value, ok := reflect.Select(cases)
		// First handle ctx.Done.
		if chosen == 0 {
			return w.ctx.Err()
		}

		if !ok {
			// Should restart watchers.
			return fmt.Errorf("Channel read not ok")
		}

		event := value.Interface().(*kvstore.WatchEvent)

		if event == nil {
			return fmt.Errorf("Nil event from watcher")
		}

		w.logger.Debugf("Reading from channel %d and received event: {%+v} %s", chosen, value, value.String())

		// Update object db.
		updateObjdb := func() error {
			val := reflect.ValueOf(event.Object)
			obj := val.Interface().(objectdb.Object)
			switch event.Type {
			case kvstore.Created:
				return w.objdb.Add(obj)
			case kvstore.Updated:
				return w.objdb.Update(obj)
			case kvstore.Deleted:
				return w.objdb.Delete(obj)
			default:
				return fmt.Errorf("unhandled event type %v", event.Type)
			}
		}
		err := updateObjdb()
		if err != nil {
			return err
		}

		// Send event on output channel.
		select {
		case w.outCh <- event:
		case <-w.ctx.Done():
			return w.ctx.Err()
		}
	}
}

func (w *watcher) stopWatchers() {
	for key, watcher := range w.groupWatchers {
		if watcher != nil {
			watcher.Stop()
		}
		delete(w.groupWatchers, key)
	}

	w.logger.Infof("Stopped watchers")
}

func (w *watcher) cleanup() {
	if w.running {
		w.running = false
		w.stopWatchers()
		close(w.errCh)
		close(w.outCh)
	}
}
