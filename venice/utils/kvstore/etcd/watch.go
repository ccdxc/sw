package etcd

import (
	"context"
	"fmt"
	"net/http"
	"strconv"
	"strings"

	"github.com/coreos/etcd/clientv3"
	etcdrpc "github.com/coreos/etcd/etcdserver/api/v3rpc/rpctypes"

	"github.com/pensando/sw/venice/utils/log"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/kvstore"
)

const (
	outCount = 512
)

// watcher implements kvstore.Watcher interface.
type watcher struct {
	watchCtx    *etcdStore               // kv store
	keyOrPrefix string                   // what is being watched
	fromVersion int64                    // version to watch from
	recursive   bool                     // set to true for prefix based watches
	outCh       chan *kvstore.WatchEvent // channel for watch events
	ctx         context.Context
	cancel      context.CancelFunc
}

// newWatcher creates a new watcher interface for key based watches.
func (e *etcdStore) newWatcher(ctx context.Context, key string, fromVersion string) (*watcher, error) {
	if strings.HasSuffix(key, "/") {
		return nil, fmt.Errorf("Watch called on a prefix")
	}
	return e.watch(ctx, key, fromVersion, false)
}

// newPrefixWatcher creates a new watcher interface for prefix based watches.
func (e *etcdStore) newPrefixWatcher(ctx context.Context, prefix string, fromVersion string) (*watcher, error) {
	if !strings.HasSuffix(prefix, "/") {
		prefix += "/"
	}
	return e.watch(ctx, prefix, fromVersion, true)
}

// watch sets up the watcher context and starts the watch.
func (e *etcdStore) watch(ctx context.Context, keyOrPrefix string, fromVersion string, recursive bool) (*watcher, error) {
	ctx, cancel := context.WithCancel(ctx)
	if fromVersion == "" {
		fromVersion = "0"
	}
	version, err := strconv.ParseInt(fromVersion, 10, 64)
	if err != nil {
		cancel()
		return nil, err
	}
	w := &watcher{
		watchCtx:    e,
		keyOrPrefix: keyOrPrefix,
		fromVersion: version,
		recursive:   recursive,
		outCh:       make(chan *kvstore.WatchEvent, outCount),
		ctx:         ctx,
		cancel:      cancel,
	}
	opts := []clientv3.OpOption{}
	if w.recursive {
		opts = append(opts, clientv3.WithPrefix())
	}
	watchVer := w.fromVersion
	var resp *clientv3.GetResponse
	// If starting from 0, need to send current object(s) on the channel.
	if w.fromVersion == 0 {
		resp, err = w.watchCtx.client.Get(w.ctx, w.keyOrPrefix, opts...)
		if err != nil {
			log.Errorf("Failed to get %v with error: %v", w.keyOrPrefix, err)
			cancel()
			return nil, err
		}
		watchVer = resp.Header.Revision
	}
	opts = append(opts, clientv3.WithRev(watchVer+1), clientv3.WithPrevKV())
	wc := w.watchCtx.client.Watch(w.ctx, w.keyOrPrefix, opts...)
	go w.startWatching(wc, resp)
	return w, nil
}

// startWatching starts the watch.
func (w *watcher) startWatching(wc clientv3.WatchChan, resp *clientv3.GetResponse) {
	// If Get response is non-nil, send current object(s) on the channel.
	if resp != nil {
		for _, kv := range resp.Kvs {
			evType := kvstore.Updated

			if kv.CreateRevision == kv.ModRevision {
				evType = kvstore.Created
			}

			w.sendEvent(evType, string(kv.Key), kv.Value, kv.ModRevision)
		}
	}

	for wr := range wc {
		if wr.Err() != nil {
			log.Errorf("Failed watch on %v with error: %v", w.keyOrPrefix, wr.Err())
			w.sendError(wr.Err())
			return
		}
		for _, e := range wr.Events {
			evType := kvstore.Updated
			value := e.Kv.Value
			key := string(e.Kv.Key)

			if e.IsCreate() {
				evType = kvstore.Created
			} else if e.Type == clientv3.EventTypeDelete {
				evType = kvstore.Deleted
				value = []byte{}
				if e.PrevKv != nil {
					value = e.PrevKv.Value
				}
			}

			w.sendEvent(evType, key, value, e.Kv.ModRevision)
		}
	}
	// Stop() was called.
	close(w.outCh)
}

// sendEvent sends out the event unless the watch is stopped.
func (w *watcher) sendEvent(evType kvstore.WatchEventType, key string, value []byte, version int64) {
	obj, err := w.watchCtx.codec.Decode(value, nil)
	if err != nil {
		log.Errorf("Failed to decode %v with error %v", string(value), err)
		w.sendError(err)
		return
	}

	err = w.watchCtx.objVersioner.SetVersion(obj, uint64(version))
	if err != nil {
		log.Errorf("Failed to set version %v with error: %v", version, err)
		w.sendError(err)
		return
	}

	e := &kvstore.WatchEvent{
		Type:   evType,
		Key:    key,
		Object: obj,
	}

	if len(w.outCh) == outCount {
		log.Warnf("Number of buffered watch events hit max count of %v", outCount)
	}

	select {
	case w.outCh <- e:
	case <-w.ctx.Done():
	}
}

// sendError sends out the status object for the given error.
func (w *watcher) sendError(err error) {
	var status *api.Status
	switch {
	case err == etcdrpc.ErrCompacted:
		status = &api.Status{
			Result:  api.StatusResultExpired,
			Message: []string{err.Error()},
			Code:    http.StatusGone,
		}
	default:
		status = &api.Status{
			Result:  api.StatusResultInternalError,
			Message: []string{err.Error()},
			Code:    http.StatusInternalServerError,
		}
	}
	event := &kvstore.WatchEvent{
		Type:   kvstore.WatcherError,
		Object: status,
	}
	select {
	case w.outCh <- event:
	case <-w.ctx.Done():
	}
}

// EventChan returns the channel for watch events.
func (w *watcher) EventChan() <-chan *kvstore.WatchEvent {
	return w.outCh
}

// Stop stops the watcher.
func (w *watcher) Stop() {
	w.cancel()
}
