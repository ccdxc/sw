package listerwatcher

import (
	"context"
	"errors"
	"fmt"
	"time"

	"github.com/gogo/protobuf/proto"
	"github.com/gogo/protobuf/types"
	"google.golang.org/grpc"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

// SvcWatch provides a watch bridge function for service level watches
func SvcWatch(ctx context.Context, watcher kvstore.Watcher, stream grpc.ServerStream, txfnMap map[string]func(from, to string, i interface{}) (interface{}, error), apiVersion string, l log.Logger) error {
	timer := time.NewTimer(apiserver.DefaultWatchHoldInterval)
	if !timer.Stop() {
		<-timer.C
	}
	running := false
	events := &api.WatchEventList{}
	sendToStream := func() error {
		log.ErrorLog("msg", "writing to stream", "len", len(events.Events))
		if err := stream.SendMsg(events); err != nil {
			l.DebugLog("msg", "Stream send error'ed for Order", "err", err)
			return err
		}
		events = &api.WatchEventList{}
		return nil
	}
	addEvent := func(strEvent *api.WatchEvent) error {
		events.Events = append(events.Events, strEvent)
		if !running {
			running = true
			timer.Reset(apiserver.DefaultWatchHoldInterval)
		}
		if len(events.Events) >= apiserver.DefaultWatchBatchSize {
			if err := sendToStream(); err != nil {
				return err
			}
			if !timer.Stop() {
				<-timer.C
			}
			timer.Reset(apiserver.DefaultWatchHoldInterval)
		}
		return nil
	}
	var err error
	for {
		select {
		case ev, ok := <-watcher.EventChan():
			if !ok {
				l.DebugLog("Channel closed for service watcher")
				return nil
			}
			switch ev.Type {
			case kvstore.Created, kvstore.Deleted, kvstore.Updated:
				robj := ev.Object
				l.DebugLog("msg", "received watch event from KV", "type", ev.Type)
				if apiVersion != robj.GetObjectAPIVersion() {
					i, err := txfnMap[robj.GetObjectKind()](robj.GetObjectAPIVersion(), apiVersion, robj)
					if err != nil {
						l.ErrorLog("msg", "Failed to transform message", "fromver", robj.GetObjectAPIVersion(), "tover", apiVersion)
						return err
					}
					robj = i.(runtime.Object)
				}
				obj, err := types.MarshalAny(robj.(proto.Message))
				if err != nil {
					return fmt.Errorf("unable to unmarshall object (%s) ", err)
				}
				strEvent := &api.WatchEvent{
					Type:   string(ev.Type),
					Object: obj,
				}
				err = addEvent(strEvent)
				if err != nil {
					return err
				}
			case kvstore.WatcherError:
				status, ok := ev.Object.(*api.Status)
				if ok {
					return fmt.Errorf("%v:(%s) %s", status.Code, status.Result, status.Message)
				}
				return fmt.Errorf("watcher error [%v]", ev.Object)

			case kvstore.WatcherControl:
				ctrl := ev.Control
				strEvent := &api.WatchEvent{
					Type: string(kvstore.WatcherControl),
					Control: &api.WatchControl{
						Code:    ctrl.Code,
						Message: ctrl.Message,
					},
				}
				err = addEvent(strEvent)
				if err != nil {
					return err
				}
			}

		case <-timer.C:
			running = false
			if err = sendToStream(); err != nil {
				return err
			}
		case <-ctx.Done():
			l.DebugLog("msg", "Context cancelled for Order Watcher")
			return stream.Context().Err()
		}
	}
}

// GetObject retrieves the runtime.Object from a svc watch event
func GetObject(obj *api.WatchEvent) (runtime.Object, error) {
	robj := &types.DynamicAny{}
	err := types.UnmarshalAny(obj.Object, robj)
	if err != nil {
		return nil, err
	}
	if ret, ok := robj.Message.(runtime.Object); ok {
		return ret, nil
	}
	return nil, errors.New("failed to unmarshal event")
}
