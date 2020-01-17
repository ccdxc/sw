// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

package controller

import (
	"context"
	"io/ioutil"
	"math"
	"os"
	"strings"
	"sync"
	"testing"

	ptypes "github.com/gogo/protobuf/types"
	"google.golang.org/grpc"

	"github.com/pensando/sw/api"
	fakehal "github.com/pensando/sw/nic/agent/cmd/fakehal/hal"
	"github.com/pensando/sw/nic/agent/dscagent/infra"
	"github.com/pensando/sw/nic/agent/dscagent/pipeline"
	"github.com/pensando/sw/nic/agent/dscagent/types"
	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/rpckit"
)

var (
	appdbcreate, appdbupdate, appdbdelete,
	endpointdbcreate, endpointdbupdate, endpointdbdelete,
	networkdbcreate, networkdbupdate, networkdbdelete,
	nspdbcreate, nspdbupdate, nspdbdelete,
	mirrorSessionDB sync.Map
)

type fakeRPCServer struct {
	grpcServer *rpckit.RPCServer
}

func (srv *fakeRPCServer) WatchObjects(watchOptions *api.AggWatchOptions, stream netproto.AggWatchApiV1_WatchObjectsServer) error {
	// walk local db and send stream resp
	for {

		appdbcreate.Range(func(key, value interface{}) bool {
			obj := value.(*netproto.App)
			log.Infof("Sending App Create: %s", obj.GetKey())

			// watch event
			watchEvt := netproto.AggObjectEvent{
				EventType: api.EventType_CreateEvent,
				AggObj:    netproto.AggObject{Kind: "App", Object: &api.Any{}},
			}

			watchEvts := netproto.AggObjectEventList{}

			mobj, err := ptypes.MarshalAny(obj)
			if err != nil {
				log.Errorf("Error  marshalling any object. Err: %v", err)
				return err == nil
			}

			watchEvt.AggObj.Object = &api.Any{
				Any: *mobj,
			}

			watchEvts.AggObjectEvents = append(watchEvts.AggObjectEvents, &watchEvt)
			// send create event
			err = stream.Send(&watchEvts)
			if err != nil {
				log.Errorf("NetAgent Responded Err: %v", err)
			}

			appdbcreate.Delete(key)

			return err == nil
		})

		appdbupdate.Range(func(key, value interface{}) bool {
			obj := value.(*netproto.App)
			log.Infof("Sending App Update: %s", obj.GetKey())

			// watch event
			watchEvt := netproto.AggObjectEvent{
				EventType: api.EventType_UpdateEvent,
				AggObj:    netproto.AggObject{Kind: "App", Object: &api.Any{}},
			}

			watchEvts := netproto.AggObjectEventList{}

			mobj, err := ptypes.MarshalAny(obj)
			if err != nil {
				log.Errorf("Error  marshalling any object. Err: %v", err)
				return err == nil
			}

			watchEvt.AggObj.Object = &api.Any{
				Any: *mobj,
			}

			watchEvts.AggObjectEvents = append(watchEvts.AggObjectEvents, &watchEvt)
			// send create event
			err = stream.Send(&watchEvts)
			if err != nil {
				log.Errorf("NetAgent Responded Err: %v", err)
			}
			appdbupdate.Delete(key)

			return err == nil
		})

		appdbdelete.Range(func(key, value interface{}) bool {
			obj := value.(*netproto.App)
			log.Infof("Sending App Delete: %s", obj.GetKey())

			// watch event
			watchEvt := netproto.AggObjectEvent{
				EventType: api.EventType_DeleteEvent,
				AggObj:    netproto.AggObject{Kind: "App", Object: &api.Any{}},
			}

			watchEvts := netproto.AggObjectEventList{}

			mobj, err := ptypes.MarshalAny(obj)
			if err != nil {
				log.Errorf("Error  marshalling any object. Err: %v", err)
				return err == nil
			}

			watchEvt.AggObj.Object = &api.Any{
				Any: *mobj,
			}

			watchEvts.AggObjectEvents = append(watchEvts.AggObjectEvents, &watchEvt)
			// send create event
			err = stream.Send(&watchEvts)
			if err != nil {
				log.Errorf("NetAgent Responded Err: %v", err)
			}
			appdbdelete.Delete(key)

			return err == nil
		})

		nspdbcreate.Range(func(key, value interface{}) bool {
			obj := value.(*netproto.NetworkSecurityPolicy)
			// watch event
			log.Infof("Sending NetworkSecurityPolicy Create: %s", obj.GetKey())

			watchEvt := netproto.AggObjectEvent{
				EventType: api.EventType_CreateEvent,
				AggObj:    netproto.AggObject{Kind: "NetworkSecurityPolicy", Object: &api.Any{}},
			}

			watchEvts := netproto.AggObjectEventList{}

			mobj, err := ptypes.MarshalAny(obj)
			if err != nil {
				log.Errorf("Error  marshalling any object. Err: %v", err)
				return err == nil
			}

			watchEvt.AggObj.Object = &api.Any{
				Any: *mobj,
			}

			watchEvts.AggObjectEvents = append(watchEvts.AggObjectEvents, &watchEvt)
			// send create event
			err = stream.Send(&watchEvts)
			if err != nil {
				log.Errorf("NetAgent Responded Err: %v", err)
			} else {
				nspdbcreate.Delete(key)
			}
			return err == nil
		})

		nspdbupdate.Range(func(key, value interface{}) bool {
			obj := value.(*netproto.NetworkSecurityPolicy)
			// watch event
			log.Infof("Sending NetworkSecurityPolicy Update: %s", obj.GetKey())

			watchEvt := netproto.AggObjectEvent{
				EventType: api.EventType_UpdateEvent,
				AggObj:    netproto.AggObject{Kind: "NetworkSecurityPolicy", Object: &api.Any{}},
			}

			watchEvts := netproto.AggObjectEventList{}

			mobj, err := ptypes.MarshalAny(obj)
			if err != nil {
				log.Errorf("Error  marshalling any object. Err: %v", err)
				return err == nil
			}

			watchEvt.AggObj.Object = &api.Any{
				Any: *mobj,
			}

			watchEvts.AggObjectEvents = append(watchEvts.AggObjectEvents, &watchEvt)
			// send create event
			err = stream.Send(&watchEvts)
			if err == nil {
				nspdbupdate.Delete(key)
			} else {
				log.Errorf("NetAgent Responded Err: %v", err)
			}

			return err == nil
		})

		nspdbdelete.Range(func(key, value interface{}) bool {
			obj := value.(*netproto.NetworkSecurityPolicy)
			// watch event
			log.Infof("Sending NetworkSecurityPolicy Delete: %s", obj.GetKey())

			watchEvt := netproto.AggObjectEvent{
				EventType: api.EventType_DeleteEvent,
				AggObj:    netproto.AggObject{Kind: "NetworkSecurityPolicy", Object: &api.Any{}},
			}

			watchEvts := netproto.AggObjectEventList{}

			mobj, err := ptypes.MarshalAny(obj)
			if err != nil {
				log.Errorf("Error  marshalling any object. Err: %v", err)
				return err == nil
			}

			watchEvt.AggObj.Object = &api.Any{
				Any: *mobj,
			}

			watchEvts.AggObjectEvents = append(watchEvts.AggObjectEvents, &watchEvt)
			// send create event
			err = stream.Send(&watchEvts)

			if err == nil {
				nspdbdelete.Delete(key)
			} else {
				log.Errorf("NetAgent Responded Err: %v", err)
			}

			return err == nil
		})

		networkdbcreate.Range(func(key, value interface{}) bool {
			obj := value.(*netproto.Network)
			// watch event
			log.Infof("Sending Network Create: %s", obj.GetKey())

			watchEvt := netproto.AggObjectEvent{
				EventType: api.EventType_CreateEvent,
				AggObj:    netproto.AggObject{Kind: "Network", Object: &api.Any{}},
			}

			watchEvts := netproto.AggObjectEventList{}

			mobj, err := ptypes.MarshalAny(obj)
			if err != nil {
				log.Errorf("Error  marshalling any object. Err: %v", err)
				return err == nil
			}

			watchEvt.AggObj.Object = &api.Any{
				Any: *mobj,
			}

			watchEvts.AggObjectEvents = append(watchEvts.AggObjectEvents, &watchEvt)
			// send create event
			err = stream.Send(&watchEvts)
			if err != nil {
				log.Errorf("NetAgent Responded Err: %v", err)
			} else {
				networkdbcreate.Delete(key)
			}
			return err == nil
		})

		networkdbupdate.Range(func(key, value interface{}) bool {
			obj := value.(*netproto.Network)
			// watch event
			log.Infof("Sending Network Update: %s", obj.GetKey())

			watchEvt := netproto.AggObjectEvent{
				EventType: api.EventType_UpdateEvent,
				AggObj:    netproto.AggObject{Kind: "Network", Object: &api.Any{}},
			}

			watchEvts := netproto.AggObjectEventList{}

			mobj, err := ptypes.MarshalAny(obj)
			if err != nil {
				log.Errorf("Error  marshalling any object. Err: %v", err)
				return err == nil
			}

			watchEvt.AggObj.Object = &api.Any{
				Any: *mobj,
			}

			watchEvts.AggObjectEvents = append(watchEvts.AggObjectEvents, &watchEvt)
			// send create event
			err = stream.Send(&watchEvts)
			if err == nil {
				networkdbupdate.Delete(key)
			} else {
				log.Errorf("NetAgent Responded Err: %v", err)
			}

			return err == nil
		})

		networkdbdelete.Range(func(key, value interface{}) bool {
			obj := value.(*netproto.Network)
			// watch event
			log.Infof("Sending Network Delete: %s", obj.GetKey())

			watchEvt := netproto.AggObjectEvent{
				EventType: api.EventType_DeleteEvent,
				AggObj:    netproto.AggObject{Kind: "Network", Object: &api.Any{}},
			}

			watchEvts := netproto.AggObjectEventList{}

			mobj, err := ptypes.MarshalAny(obj)
			if err != nil {
				log.Errorf("Error  marshalling any object. Err: %v", err)
				return err == nil
			}

			watchEvt.AggObj.Object = &api.Any{
				Any: *mobj,
			}

			watchEvts.AggObjectEvents = append(watchEvts.AggObjectEvents, &watchEvt)
			// send create event
			err = stream.Send(&watchEvts)

			if err == nil {
				networkdbdelete.Delete(key)
			} else {
				log.Errorf("NetAgent Responded Err: %v", err)
			}

			return err == nil
		})

		endpointdbcreate.Range(func(key, value interface{}) bool {
			obj := value.(*netproto.Endpoint)
			// watch event
			log.Infof("Sending Endpoint Create: %s", obj.GetKey())

			watchEvt := netproto.AggObjectEvent{
				EventType: api.EventType_CreateEvent,
				AggObj:    netproto.AggObject{Kind: "Endpoint", Object: &api.Any{}},
			}

			watchEvts := netproto.AggObjectEventList{}

			mobj, err := ptypes.MarshalAny(obj)
			if err != nil {
				log.Errorf("Error  marshalling any object. Err: %v", err)
				return err == nil
			}

			watchEvt.AggObj.Object = &api.Any{
				Any: *mobj,
			}

			watchEvts.AggObjectEvents = append(watchEvts.AggObjectEvents, &watchEvt)
			// send create event
			err = stream.Send(&watchEvts)
			if err != nil {
				log.Errorf("NetAgent Responded Err: %v", err)
			} else {
				endpointdbcreate.Delete(key)
			}
			return err == nil
		})

		endpointdbupdate.Range(func(key, value interface{}) bool {
			obj := value.(*netproto.Endpoint)
			// watch event
			log.Infof("Sending Endpoint Update: %s", obj.GetKey())

			watchEvt := netproto.AggObjectEvent{
				EventType: api.EventType_UpdateEvent,
				AggObj:    netproto.AggObject{Kind: "Endpoint", Object: &api.Any{}},
			}

			watchEvts := netproto.AggObjectEventList{}

			mobj, err := ptypes.MarshalAny(obj)
			if err != nil {
				log.Errorf("Error  marshalling any object. Err: %v", err)
				return err == nil
			}

			watchEvt.AggObj.Object = &api.Any{
				Any: *mobj,
			}

			watchEvts.AggObjectEvents = append(watchEvts.AggObjectEvents, &watchEvt)
			// send create event
			err = stream.Send(&watchEvts)
			if err == nil {
				endpointdbupdate.Delete(key)
			} else {
				log.Errorf("NetAgent Responded Err: %v", err)
			}

			return err == nil
		})

		endpointdbdelete.Range(func(key, value interface{}) bool {
			obj := value.(*netproto.Endpoint)
			// watch event
			log.Infof("Sending Endpoint Delete: %s", obj.GetKey())

			watchEvt := netproto.AggObjectEvent{
				EventType: api.EventType_DeleteEvent,
				AggObj:    netproto.AggObject{Kind: "Endpoint", Object: &api.Any{}},
			}

			watchEvts := netproto.AggObjectEventList{}

			mobj, err := ptypes.MarshalAny(obj)
			if err != nil {
				log.Errorf("Error  marshalling any object. Err: %v", err)
				return err == nil
			}

			watchEvt.AggObj.Object = &api.Any{
				Any: *mobj,
			}

			watchEvts.AggObjectEvents = append(watchEvts.AggObjectEvents, &watchEvt)
			// send create event
			err = stream.Send(&watchEvts)

			if err == nil {
				endpointdbdelete.Delete(key)
			} else {
				log.Errorf("NetAgent Responded Err: %v", err)
			}

			return err == nil
		})

	}
}

func (srv *fakeRPCServer) ObjectOperUpdate(ostream netproto.AggWatchApiV1_ObjectOperUpdateServer) error {
	for {
		aggEvt, err := ostream.Recv()
		if err != nil {
			log.Errorf("Ostream Err: %v", err)
			return err
		}
		log.Infof("Got Agg Evt: %s", aggEvt.String())
	}
}

func (srv *fakeRPCServer) ListObjects(ctx context.Context, aggWatchOptions *api.AggWatchOptions) (*netproto.AggObjectList, error) {
	return &netproto.AggObjectList{}, nil
}

var (
	mockHal        *fakehal.Hal
	lis, ctrlerLis netutils.TestListenAddr
	fakeServer     *fakeRPCServer
	pipelineAPI    types.PipelineAPI
	infraAPI       types.InfraAPI
	c              types.ControllerAPI
)

// Sets up the grpc client handlers for the package
func TestMain(m *testing.M) {

	primaryDB, err := ioutil.TempFile("", "")
	if err != nil {
		log.Errorf("Test Setup Failed. Err: %v", err)
		os.Exit(1)
	}

	secondaryDB, err := ioutil.TempFile("", "")
	if err != nil {
		log.Errorf("Test Setup Failed. Err: %v", err)
		os.Exit(1)
	}

	log.Infof("Primary DB: %s | Backup DB: %s", primaryDB.Name(), secondaryDB.Name())

	infraAPI, err = infra.NewInfraAPI(primaryDB.Name(), secondaryDB.Name())
	if err != nil {
		log.Errorf("Test Setup Failed. Err: %v", err)
		os.Exit(1)
	}

	_ = lis.GetAvailablePort()
	_ = ctrlerLis.GetAvailablePort()

	if fakeServer, err = createRPCServer(); err != nil {
		log.Errorf("Test Setup Failed. Err: %v", err)
		os.Exit(1)
	}

	if _, err := grpc.Dial(lis.ListenURL.String(), grpc.WithMaxMsgSize(math.MaxInt32-1), grpc.WithInsecure()); err != nil {
		log.Errorf("Test Setup Failed. Err: %v", err)
		mockHal.Stop()
		os.Exit(1)
	}

	mockHal = fakehal.NewFakeHalServer(lis.ListenURL.String())
	if err := os.Setenv("HAL_GRPC_PORT", strings.Split(lis.ListenURL.String(), ":")[1]); err != nil {
		log.Errorf("Test Setup Failed. Err: %v", err)
		os.Exit(1)
	}

	pipelineAPI, err = pipeline.NewPipelineAPI(infraAPI)
	if err != nil {
		log.Errorf("Test Setup Failed. Err: %v", err)
		os.Exit(1)
	}

	c = NewControllerAPI(pipelineAPI, infraAPI, fakeServer.grpcServer.GetListenURL(), fakeServer.grpcServer.GetListenURL(), fakeServer.grpcServer.GetListenURL(), ctrlerLis.ListenURL.String())
	defer c.Stop()

	code := m.Run()
	mockHal.Stop()
	pipelineAPI.PurgeConfigs()
	infraAPI.Close()
	os.Remove(primaryDB.Name())
	os.Remove(secondaryDB.Name())
	os.Exit(code)
}

// ############################ Agg Watch Methods ############################
// Infinite looping of sending objects with specific oper. Ensure its gone from the db on stream.send. This
// is a poor man's simulation of looped npm cruds. On the bright side, the validation is all the way down to datapath
//func (srv *fakeRPCServer) WatchObjects(kinds *netproto.AggKinds, stream netproto.AggWatchApiV1_WatchObjectsServer) error {
//	// walk local db and send stream resp
//	for {
//
//		appdbcreate.Range(func(key, value interface{}) bool {
//			obj := value.(*netproto.App)
//			log.Infof("Sending App Create: %s", obj.GetKey())
//
//			// watch event
//			watchEvt := netproto.AggObjectEvent{
//				EventType: api.EventType_CreateEvent,
//				AggObj:    netproto.AggObject{Kind: "App", Object: &api.Any{}},
//			}
//
//			watchEvts := netproto.AggObjectEventList{}
//
//			mobj, err := ptypes.MarshalAny(obj)
//			if err != nil {
//				log.Errorf("Error  marshalling any object. Err: %v", err)
//				return err == nil
//			}
//
//			watchEvt.AggObj.Object = &api.Any{
//				Any: *mobj,
//			}
//
//			watchEvts.AggObjectEvents = append(watchEvts.AggObjectEvents, &watchEvt)
//			// send create event
//			err = stream.Send(&watchEvts)
//			if err != nil {
//				log.Errorf("NetAgent Responded Err: %v", err)
//			}
//
//			appdbcreate.Delete(key)
//
//			return err == nil
//		})
//
//		appdbupdate.Range(func(key, value interface{}) bool {
//			obj := value.(*netproto.App)
//			log.Infof("Sending App Update: %s", obj.GetKey())
//
//			// watch event
//			watchEvt := netproto.AggObjectEvent{
//				EventType: api.EventType_UpdateEvent,
//				AggObj:    netproto.AggObject{Kind: "App", Object: &api.Any{}},
//			}
//
//			watchEvts := netproto.AggObjectEventList{}
//
//			mobj, err := ptypes.MarshalAny(obj)
//			if err != nil {
//				log.Errorf("Error  marshalling any object. Err: %v", err)
//				return err == nil
//			}
//
//			watchEvt.AggObj.Object = &api.Any{
//				Any: *mobj,
//			}
//
//			watchEvts.AggObjectEvents = append(watchEvts.AggObjectEvents, &watchEvt)
//			// send create event
//			err = stream.Send(&watchEvts)
//			if err != nil {
//				log.Errorf("NetAgent Responded Err: %v", err)
//			}
//			appdbupdate.Delete(key)
//
//			return err == nil
//		})
//
//		appdbdelete.Range(func(key, value interface{}) bool {
//			obj := value.(*netproto.App)
//			log.Infof("Sending App Delete: %s", obj.GetKey())
//
//			// watch event
//			watchEvt := netproto.AggObjectEvent{
//				EventType: api.EventType_DeleteEvent,
//				AggObj:    netproto.AggObject{Kind: "App", Object: &api.Any{}},
//			}
//
//			watchEvts := netproto.AggObjectEventList{}
//
//			mobj, err := ptypes.MarshalAny(obj)
//			if err != nil {
//				log.Errorf("Error  marshalling any object. Err: %v", err)
//				return err == nil
//			}
//
//			watchEvt.AggObj.Object = &api.Any{
//				Any: *mobj,
//			}
//
//			watchEvts.AggObjectEvents = append(watchEvts.AggObjectEvents, &watchEvt)
//			// send create event
//			err = stream.Send(&watchEvts)
//			if err != nil {
//				log.Errorf("NetAgent Responded Err: %v", err)
//			}
//			appdbdelete.Delete(key)
//
//			return err == nil
//		})
//
//		nspdbcreate.Range(func(key, value interface{}) bool {
//			obj := value.(*netproto.NetworkSecurityPolicy)
//			// watch event
//			log.Infof("Sending NetworkSecurityPolicy Create: %s", obj.GetKey())
//
//			watchEvt := netproto.AggObjectEvent{
//				EventType: api.EventType_CreateEvent,
//				AggObj:    netproto.AggObject{Kind: "NetworkSecurityPolicy", Object: &api.Any{}},
//			}
//
//			watchEvts := netproto.AggObjectEventList{}
//
//			mobj, err := ptypes.MarshalAny(obj)
//			if err != nil {
//				log.Errorf("Error  marshalling any object. Err: %v", err)
//				return err == nil
//			}
//
//			watchEvt.AggObj.Object = &api.Any{
//				Any: *mobj,
//			}
//
//			watchEvts.AggObjectEvents = append(watchEvts.AggObjectEvents, &watchEvt)
//			// send create event
//			err = stream.Send(&watchEvts)
//			if err != nil {
//				log.Errorf("NetAgent Responded Err: %v", err)
//			} else {
//				nspdbcreate.Delete(key)
//			}
//			return err == nil
//		})
//
//		nspdbupdate.Range(func(key, value interface{}) bool {
//			obj := value.(*netproto.NetworkSecurityPolicy)
//			// watch event
//			log.Infof("Sending NetworkSecurityPolicy Update: %s", obj.GetKey())
//
//			watchEvt := netproto.AggObjectEvent{
//				EventType: api.EventType_UpdateEvent,
//				AggObj:    netproto.AggObject{Kind: "NetworkSecurityPolicy", Object: &api.Any{}},
//			}
//
//			watchEvts := netproto.AggObjectEventList{}
//
//			mobj, err := ptypes.MarshalAny(obj)
//			if err != nil {
//				log.Errorf("Error  marshalling any object. Err: %v", err)
//				return err == nil
//			}
//
//			watchEvt.AggObj.Object = &api.Any{
//				Any: *mobj,
//			}
//
//			watchEvts.AggObjectEvents = append(watchEvts.AggObjectEvents, &watchEvt)
//			// send create event
//			err = stream.Send(&watchEvts)
//			if err == nil {
//				nspdbupdate.Delete(key)
//			} else {
//				log.Errorf("NetAgent Responded Err: %v", err)
//			}
//
//			return err == nil
//		})
//
//		nspdbdelete.Range(func(key, value interface{}) bool {
//			obj := value.(*netproto.NetworkSecurityPolicy)
//			// watch event
//			log.Infof("Sending NetworkSecurityPolicy Delete: %s", obj.GetKey())
//
//			watchEvt := netproto.AggObjectEvent{
//				EventType: api.EventType_DeleteEvent,
//				AggObj:    netproto.AggObject{Kind: "NetworkSecurityPolicy", Object: &api.Any{}},
//			}
//
//			watchEvts := netproto.AggObjectEventList{}
//
//			mobj, err := ptypes.MarshalAny(obj)
//			if err != nil {
//				log.Errorf("Error  marshalling any object. Err: %v", err)
//				return err == nil
//			}
//
//			watchEvt.AggObj.Object = &api.Any{
//				Any: *mobj,
//			}
//
//			watchEvts.AggObjectEvents = append(watchEvts.AggObjectEvents, &watchEvt)
//			// send create event
//			err = stream.Send(&watchEvts)
//
//			if err == nil {
//				nspdbdelete.Delete(key)
//			} else {
//				log.Errorf("NetAgent Responded Err: %v", err)
//			}
//
//			return err == nil
//		})
//
//		networkdbcreate.Range(func(key, value interface{}) bool {
//			obj := value.(*netproto.Network)
//			// watch event
//			log.Infof("Sending Network Create: %s", obj.GetKey())
//
//			watchEvt := netproto.AggObjectEvent{
//				EventType: api.EventType_CreateEvent,
//				AggObj:    netproto.AggObject{Kind: "Network", Object: &api.Any{}},
//			}
//
//			watchEvts := netproto.AggObjectEventList{}
//
//			mobj, err := ptypes.MarshalAny(obj)
//			if err != nil {
//				log.Errorf("Error  marshalling any object. Err: %v", err)
//				return err == nil
//			}
//
//			watchEvt.AggObj.Object = &api.Any{
//				Any: *mobj,
//			}
//
//			watchEvts.AggObjectEvents = append(watchEvts.AggObjectEvents, &watchEvt)
//			// send create event
//			err = stream.Send(&watchEvts)
//			if err != nil {
//				log.Errorf("NetAgent Responded Err: %v", err)
//			} else {
//				networkdbcreate.Delete(key)
//			}
//			return err == nil
//		})
//
//		networkdbupdate.Range(func(key, value interface{}) bool {
//			obj := value.(*netproto.Network)
//			// watch event
//			log.Infof("Sending Network Update: %s", obj.GetKey())
//
//			watchEvt := netproto.AggObjectEvent{
//				EventType: api.EventType_UpdateEvent,
//				AggObj:    netproto.AggObject{Kind: "Network", Object: &api.Any{}},
//			}
//
//			watchEvts := netproto.AggObjectEventList{}
//
//			mobj, err := ptypes.MarshalAny(obj)
//			if err != nil {
//				log.Errorf("Error  marshalling any object. Err: %v", err)
//				return err == nil
//			}
//
//			watchEvt.AggObj.Object = &api.Any{
//				Any: *mobj,
//			}
//
//			watchEvts.AggObjectEvents = append(watchEvts.AggObjectEvents, &watchEvt)
//			// send create event
//			err = stream.Send(&watchEvts)
//			if err == nil {
//				networkdbupdate.Delete(key)
//			} else {
//				log.Errorf("NetAgent Responded Err: %v", err)
//			}
//
//			return err == nil
//		})
//
//		networkdbdelete.Range(func(key, value interface{}) bool {
//			obj := value.(*netproto.Network)
//			// watch event
//			log.Infof("Sending Network Delete: %s", obj.GetKey())
//
//			watchEvt := netproto.AggObjectEvent{
//				EventType: api.EventType_DeleteEvent,
//				AggObj:    netproto.AggObject{Kind: "Network", Object: &api.Any{}},
//			}
//
//			watchEvts := netproto.AggObjectEventList{}
//
//			mobj, err := ptypes.MarshalAny(obj)
//			if err != nil {
//				log.Errorf("Error  marshalling any object. Err: %v", err)
//				return err == nil
//			}
//
//			watchEvt.AggObj.Object = &api.Any{
//				Any: *mobj,
//			}
//
//			watchEvts.AggObjectEvents = append(watchEvts.AggObjectEvents, &watchEvt)
//			// send create event
//			err = stream.Send(&watchEvts)
//
//			if err == nil {
//				networkdbdelete.Delete(key)
//			} else {
//				log.Errorf("NetAgent Responded Err: %v", err)
//			}
//
//			return err == nil
//		})
//
//		endpointdbcreate.Range(func(key, value interface{}) bool {
//			obj := value.(*netproto.Endpoint)
//			// watch event
//			log.Infof("Sending Endpoint Create: %s", obj.GetKey())
//
//			watchEvt := netproto.AggObjectEvent{
//				EventType: api.EventType_CreateEvent,
//				AggObj:    netproto.AggObject{Kind: "Endpoint", Object: &api.Any{}},
//			}
//
//			watchEvts := netproto.AggObjectEventList{}
//
//			mobj, err := ptypes.MarshalAny(obj)
//			if err != nil {
//				log.Errorf("Error  marshalling any object. Err: %v", err)
//				return err == nil
//			}
//
//			watchEvt.AggObj.Object = &api.Any{
//				Any: *mobj,
//			}
//
//			watchEvts.AggObjectEvents = append(watchEvts.AggObjectEvents, &watchEvt)
//			// send create event
//			err = stream.Send(&watchEvts)
//			if err != nil {
//				log.Errorf("NetAgent Responded Err: %v", err)
//			} else {
//				endpointdbcreate.Delete(key)
//			}
//			return err == nil
//		})
//
//		endpointdbupdate.Range(func(key, value interface{}) bool {
//			obj := value.(*netproto.Endpoint)
//			// watch event
//			log.Infof("Sending Endpoint Update: %s", obj.GetKey())
//
//			watchEvt := netproto.AggObjectEvent{
//				EventType: api.EventType_UpdateEvent,
//				AggObj:    netproto.AggObject{Kind: "Endpoint", Object: &api.Any{}},
//			}
//
//			watchEvts := netproto.AggObjectEventList{}
//
//			mobj, err := ptypes.MarshalAny(obj)
//			if err != nil {
//				log.Errorf("Error  marshalling any object. Err: %v", err)
//				return err == nil
//			}
//
//			watchEvt.AggObj.Object = &api.Any{
//				Any: *mobj,
//			}
//
//			watchEvts.AggObjectEvents = append(watchEvts.AggObjectEvents, &watchEvt)
//			// send create event
//			err = stream.Send(&watchEvts)
//			if err == nil {
//				endpointdbupdate.Delete(key)
//			} else {
//				log.Errorf("NetAgent Responded Err: %v", err)
//			}
//
//			return err == nil
//		})
//
//		endpointdbdelete.Range(func(key, value interface{}) bool {
//			obj := value.(*netproto.Endpoint)
//			// watch event
//			log.Infof("Sending Endpoint Delete: %s", obj.GetKey())
//
//			watchEvt := netproto.AggObjectEvent{
//				EventType: api.EventType_DeleteEvent,
//				AggObj:    netproto.AggObject{Kind: "Endpoint", Object: &api.Any{}},
//			}
//
//			watchEvts := netproto.AggObjectEventList{}
//
//			mobj, err := ptypes.MarshalAny(obj)
//			if err != nil {
//				log.Errorf("Error  marshalling any object. Err: %v", err)
//				return err == nil
//			}
//
//			watchEvt.AggObj.Object = &api.Any{
//				Any: *mobj,
//			}
//
//			watchEvts.AggObjectEvents = append(watchEvts.AggObjectEvents, &watchEvt)
//			// send create event
//			err = stream.Send(&watchEvts)
//
//			if err == nil {
//				endpointdbdelete.Delete(key)
//			} else {
//				log.Errorf("NetAgent Responded Err: %v", err)
//			}
//
//			return err == nil
//		})
//
//	}
//}
//
//func (srv fakeRPCServer) ObjectOperUpdate(ostream netproto.AggWatchApiV1_ObjectOperUpdateServer) error {
//	for {
//		aggEvt, err := ostream.Recv()
//		if err != nil {
//			log.Errorf("Ostream Err: %v", err)
//			return err
//		}
//		log.Infof("Got Agg Evt: %s", aggEvt.String())
//	}
//}
//
//func (srv fakeRPCServer) ListObjects(context.Context, *netproto.AggKinds) (*netproto.AggObjectList, error) {
//	return &netproto.AggObjectList{}, nil
//}

//// ############################ MirrorSessionApi Methods ############################

func (srv *fakeRPCServer) WatchMirrorSessions(oMeta *api.ObjectMeta, stream netproto.MirrorSessionApiV1_WatchMirrorSessionsServer) error {
	mirror := netproto.MirrorSession{
		TypeMeta: api.TypeMeta{Kind: "Mirror"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testMirror",
		},
		Spec: netproto.MirrorSessionSpec{
			PacketSize: 128,
			Collectors: []netproto.MirrorCollector{
				{
					ExportCfg: netproto.MirrorExportConfig{Destination: "192.168.100.101"},
				},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"192.168.100.103"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.100.101"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Protocol: "tcp",
								Port:     "120",
							},
						},
					},
				},
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"192.168.100.101"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.100.103"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Protocol: "tcp",
								Port:     "120",
							},
						},
					},
				},
			},
		},
		Status: netproto.MirrorSessionStatus{MirrorSessionID: 1},
	}

	watchEvtList := &netproto.MirrorSessionEventList{
		MirrorSessionEvents: []*netproto.MirrorSessionEvent{
			{
				EventType:     api.EventType_CreateEvent,
				MirrorSession: &mirror,
			},
		},
	}

	_ = stream.Send(watchEvtList)
	select {}
}

func (srv *fakeRPCServer) ListMirrorSessions(ctx context.Context, oMeta *api.ObjectMeta) (*netproto.MirrorSessionEventList, error) {
	return &netproto.MirrorSessionEventList{}, nil
}

//// ############################ FlowExportPolicyAPI Methods ############################

func (srv *fakeRPCServer) WatchFlowExportPolicy(oMeta *api.ObjectMeta, stream netproto.FlowExportPolicyApiV1_WatchFlowExportPolicyServer) error {
	netflow := netproto.FlowExportPolicy{
		TypeMeta: api.TypeMeta{Kind: "Mirror"},
		ObjectMeta: api.ObjectMeta{
			Tenant:    "default",
			Namespace: "default",
			Name:      "testMirror",
		},
		Spec: netproto.FlowExportPolicySpec{
			Interval:         "30s",
			TemplateInterval: "5s",
			Exports: []netproto.ExportConfig{
				{
					Destination: "192.168.100.101",
					Transport: &netproto.ProtoPort{
						Protocol: "udp",
						Port:     "2005",
					},
				},
			},
			MatchRules: []netproto.MatchRule{
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"192.168.100.103"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.100.101"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Protocol: "tcp",
								Port:     "120",
							},
						},
					},
				},
				{
					Src: &netproto.MatchSelector{
						Addresses: []string{"192.168.100.101"},
					},
					Dst: &netproto.MatchSelector{
						Addresses: []string{"192.168.100.103"},
						ProtoPorts: []*netproto.ProtoPort{
							{
								Protocol: "tcp",
								Port:     "120",
							},
						},
					},
				},
			},
		},
		Status: netproto.FlowExportPolicyStatus{FlowExportPolicyID: 1},
	}

	watchEvtList := &netproto.FlowExportPolicyEventList{
		FlowExportPolicyEvents: []*netproto.FlowExportPolicyEvent{
			{
				EventType: api.EventType_CreateEvent,
				Policy:    &netflow,
			},
		},
	}

	_ = stream.Send(watchEvtList)
	select {}
}

func (srv *fakeRPCServer) ListFlowExportPolicy(ctx context.Context, ometa *api.ObjectMeta) (*netproto.FlowExportPolicyEventList, error) {
	return &netproto.FlowExportPolicyEventList{}, nil
}

// ############################ Test Helper Methods ############################
func createRPCServer() (*fakeRPCServer, error) {
	// create an RPC server
	grpcServer, err := rpckit.NewRPCServer("netctrler", ":0")
	if err != nil {
		return nil, err
	}

	// create fake rpc server
	srv := fakeRPCServer{
		grpcServer: grpcServer,
	}
	netproto.RegisterAggWatchApiV1Server(grpcServer.GrpcServer, &srv)
	netproto.RegisterMirrorSessionApiV1Server(grpcServer.GrpcServer, &srv)
	netproto.RegisterFlowExportPolicyApiV1Server(grpcServer.GrpcServer, &srv)

	grpcServer.Start()
	log.Infof("Fake NPM Server start at: %s", srv.grpcServer.GetListenURL())

	return &srv, nil
}
