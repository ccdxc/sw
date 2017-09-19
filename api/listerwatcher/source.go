package listerwatcher

import (
	"sync"

	"google.golang.org/grpc"

	"github.com/pensando/sw/venice/utils/kvstore"
)

// WatcherClientBridgeFn is used to bridge between the GRPC watch stream and WatcherClient.
//  the bridge function does the following
//   - bridge notifications received on gRPC stream to the EventChannel OutCh
//   - Close  OutCh when the gRPC stream is closed
//   - returns when the gRPC stream closes.
type WatcherClientBridgeFn func(w *WatcherClient)

// WatcherClient implements kvstore.Watcher interface used by API watches.
type WatcherClient struct {
	OutCh   chan *kvstore.WatchEvent
	Stream  grpc.ClientStream
	fn      WatcherClientBridgeFn
	running bool
	mutex   sync.Mutex
}

// NewWatcherClient creates a new Watcher instance given a stream and Watcher function.
func NewWatcherClient(stream grpc.ClientStream, fn WatcherClientBridgeFn) *WatcherClient {
	return &WatcherClient{
		OutCh:  make(chan *kvstore.WatchEvent),
		fn:     fn,
		Stream: stream,
	}
}

// EventChan returns the channel to receive events on. If there is an
// error with the watch or when Stop is called, this channel will be
// closed.
func (w *WatcherClient) EventChan() <-chan *kvstore.WatchEvent {
	return w.OutCh
}

// Stop stops the watch and closes the channel returned by EventChan().
func (w *WatcherClient) Stop() {
	w.Stream.CloseSend()
}

// Run starts the event loop to receive events
func (w *WatcherClient) Run() bool {
	w.mutex.Lock()
	defer w.mutex.Unlock()
	if w.running {
		return false
	}
	w.running = true
	if w.fn == nil {
		return false
	}
	go w.fn(w)
	return true
}
