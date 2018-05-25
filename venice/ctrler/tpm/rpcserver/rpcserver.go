package rpcserver

import (
	"fmt"
	"sync"
	"sync/atomic"

	"github.com/pensando/sw/api"
	apiProtos "github.com/pensando/sw/api/generated/monitoring"
	tpmProtos "github.com/pensando/sw/venice/ctrler/tpm/rpcserver/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/ctxutils"
	vlog "github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
	"github.com/pensando/sw/venice/utils/rpckit"
)

var pkgName = "rpcserver"
var rpcLog = vlog.WithContext("pkg", "rpcserver")

type statsPolicyRPCServer struct {
	policyDb           *memdb.Memdb
	clients            sync.Map
	collectionInterval atomic.Value
}

type fwlogPolicyRPCServer struct {
	policyDb *memdb.Memdb
	clients  sync.Map
}

type flowExportPolicyRPCServer struct {
	policyDb *memdb.Memdb
	clients  sync.Map
}

// PolicyRPCServer grpcserver config
type PolicyRPCServer struct {
	server *rpckit.RPCServer
	*statsPolicyRPCServer
	*fwlogPolicyRPCServer
	*flowExportPolicyRPCServer
}

// convert memdb event type to api event type
var apiEventTypeMap = map[memdb.EventType]api.EventType{
	memdb.CreateEvent: api.EventType_CreateEvent,
	memdb.UpdateEvent: api.EventType_UpdateEvent,
	memdb.DeleteEvent: api.EventType_DeleteEvent,
}

func (p *statsPolicyRPCServer) WatchStatsPolicy(in *api.ObjectMeta, out tpmProtos.StatsPolicyApi_WatchStatsPolicyServer) error {
	ctx := out.Context()

	rpcLog.Infof("watch statspolicy from [%v]", in.Name)
	watchChan := make(chan memdb.Event, memdb.WatchLen)
	defer close(watchChan)

	p.policyDb.WatchObjects("StatsPolicy", watchChan)

	// track clients
	peer := ctxutils.GetPeerAddress(ctx)
	p.clients.Store(peer, true)
	defer p.clients.Delete(peer)

	// send existing policy
	for _, obj := range p.policyDb.ListObjects("StatsPolicy") {
		if policy, ok := obj.(*apiProtos.StatsPolicy); ok {
			statsPolicy := &tpmProtos.StatsPolicy{
				TypeMeta:   policy.TypeMeta,
				ObjectMeta: policy.ObjectMeta,
				Spec: tpmProtos.StatsPolicySpec{
					Interval: p.collectionInterval.Load().(string),
				},
			}

			if err := out.Send(&tpmProtos.StatsPolicyEvent{EventType: api.EventType_CreateEvent,
				Policy: statsPolicy}); err != nil {
				rpcLog.Errorf("failed to send stats policy to %s, error %s", in.GetName(), err)
				return err
			}
		} else {
			rpcLog.Errorf("invalid stats policy object from list %+v", obj)
			return fmt.Errorf("invalid stats policy object from list")
		}
	}

	// loop forever on watch channel
	for {
		select {
		case event, ok := <-watchChan:
			if !ok {
				rpcLog.Errorf("[%s] error received from", in.GetName())
				return fmt.Errorf("invalid event from watch channel")
			}

			if policy, ok := event.Obj.(*apiProtos.StatsPolicy); ok {
				statsPolicy := &tpmProtos.StatsPolicy{
					TypeMeta:   policy.TypeMeta,
					ObjectMeta: policy.ObjectMeta,
					Spec: tpmProtos.StatsPolicySpec{
						Interval: p.collectionInterval.Load().(string),
					},
				}

				if err := out.Send(&tpmProtos.StatsPolicyEvent{EventType: apiEventTypeMap[event.EventType],
					Policy: statsPolicy}); err != nil {
					rpcLog.Errorf("failed to send stats policy to %s, error %s", in.GetName(), err)
					return err
				}
			} else {
				rpcLog.Errorf("invalid stats policy object event %+v", event)
				return fmt.Errorf("invalid stats policy object from event")
			}

		case <-ctx.Done():
			rpcLog.Errorf("[%v] statspolicy client context canceled, error:%s", in.GetName(), ctx.Err())
			return fmt.Errorf("client context canceled")
		}
	}
}

func (p *fwlogPolicyRPCServer) WatchFwlogPolicy(in *api.ObjectMeta, out tpmProtos.FwlogPolicyApi_WatchFwlogPolicyServer) error {
	rpcLog.Infof("watch fwlogpolicy from [%v]", in.Name)

	watchChan := make(chan memdb.Event, memdb.WatchLen)
	defer close(watchChan)

	p.policyDb.WatchObjects("FwlogPolicy", watchChan)

	ctx := out.Context()

	// track clients
	peer := ctxutils.GetPeerAddress(ctx)
	p.clients.Store(peer, true)
	defer p.clients.Delete(peer)

	// send existing policy
	for _, obj := range p.policyDb.ListObjects("FwlogPolicy") {
		if policy, ok := obj.(*apiProtos.FwlogPolicy); ok {
			fwlogPolicy := &tpmProtos.FwlogPolicy{
				TypeMeta:   policy.TypeMeta,
				ObjectMeta: policy.ObjectMeta,
				Spec: tpmProtos.FwlogPolicySpec{
					Filter:  policy.Spec.Filter,
					Exports: policy.Spec.Exports,
				},
			}

			if err := out.Send(&tpmProtos.FwlogPolicyEvent{EventType: api.EventType_CreateEvent,
				Policy: fwlogPolicy}); err != nil {
				rpcLog.Errorf("failed to send fwlog policy to %s, error %s", in.GetName(), err)
				return err
			}
		} else {
			rpcLog.Errorf("invalid fwlog policy from list %+v", obj)
			return fmt.Errorf("invalid fwlog policy from list")
		}
	}

	// loop forever on watch channel
	for {
		select {
		case event, ok := <-watchChan:

			if !ok {
				rpcLog.Errorf("[%s] error received from", in.GetName())
				return fmt.Errorf("invalid event from watch channel")
			}

			policy, ok := event.Obj.(*apiProtos.FwlogPolicy)
			if !ok {
				rpcLog.Errorf("fwlog watch error received from [%s]", in.GetName())
				return fmt.Errorf("watch error")
			}

			fwlogPolicy := &tpmProtos.FwlogPolicy{
				TypeMeta:   policy.TypeMeta,
				ObjectMeta: policy.ObjectMeta,
				Spec: tpmProtos.FwlogPolicySpec{
					Filter:  policy.Spec.Filter,
					Exports: policy.Spec.Exports,
				},
			}

			if err := out.Send(&tpmProtos.FwlogPolicyEvent{EventType: apiEventTypeMap[event.EventType],
				Policy: fwlogPolicy}); err != nil {
				rpcLog.Errorf("failed to send fwlog policy to %s, error %s", in.GetName(), err)
				return err
			}

		case <-ctx.Done():
			rpcLog.Errorf("context canceled from %v, error:%s", in.GetName(), ctx.Err())
			return fmt.Errorf("context cancelled")
		}
	}
}

func (p *flowExportPolicyRPCServer) WatchFlowExportPolicy(in *api.ObjectMeta, out tpmProtos.FlowExportPolicyApi_WatchFlowExportPolicyServer) error {
	rpcLog.Infof("watch flowexport policy from [%v]", in.Name)

	watchChan := make(chan memdb.Event, memdb.WatchLen)
	defer close(watchChan)

	p.policyDb.WatchObjects("FlowExportPolicy", watchChan)
	ctx := out.Context()

	// track clients
	peer := ctxutils.GetPeerAddress(ctx)
	p.clients.Store(peer, true)
	defer p.clients.Delete(peer)

	// send existing policy
	for _, obj := range p.policyDb.ListObjects("FlowExportPolicy") {
		policy, ok := obj.(*apiProtos.FlowExportPolicy)
		if !ok {
			rpcLog.Errorf("invalid flow export policy from list %+v", obj)
			return fmt.Errorf("invalid flow export policy from list")
		}

		flowExportPolicy := &tpmProtos.FlowExportPolicy{
			TypeMeta:   policy.TypeMeta,
			ObjectMeta: policy.ObjectMeta,
			Spec: tpmProtos.FlowExportPolicySpec{
				Targets: policy.Spec.Targets,
			},
		}

		if err := out.Send(&tpmProtos.FlowExportPolicyEvent{EventType: api.EventType_CreateEvent,
			Policy: flowExportPolicy}); err != nil {
			rpcLog.Errorf("failed to send flowexport policy to %s, error %s", in.GetName(), err)
			return err
		}
	}

	// loop forever on watch channel
	for {
		select {
		case event, ok := <-watchChan:
			if !ok {
				rpcLog.Errorf("[%s] error received from", in.GetName())
				return fmt.Errorf("invalid event from watch channel")
			}

			policy := event.Obj.(*apiProtos.FlowExportPolicy)
			flowExportPolicy := &tpmProtos.FlowExportPolicy{
				TypeMeta:   policy.TypeMeta,
				ObjectMeta: policy.ObjectMeta,
				Spec: tpmProtos.FlowExportPolicySpec{
					Targets: policy.Spec.Targets,
				},
			}

			if err := out.Send(&tpmProtos.FlowExportPolicyEvent{EventType: apiEventTypeMap[event.EventType],
				Policy: flowExportPolicy}); err != nil {
				rpcLog.Errorf("failed to send flowexport policy to %s, error %s", in.GetName(), err)
				return err
			}

		case <-ctx.Done():
			rpcLog.Errorf("context canceled from %v, error:%s", in.GetName(), ctx.Err())
			return fmt.Errorf("context canceled")
		}
	}
}

// Stop rpc server
func (s *PolicyRPCServer) Stop() error {
	return s.server.Stop()
}

// SetCollectionInterval sets collection interval in seconds
func (s *PolicyRPCServer) SetCollectionInterval(interval string) error {
	s.statsPolicyRPCServer.collectionInterval.Store(interval)
	return nil
}

// NewRPCServer starts a new instance of rpc server
func NewRPCServer(listenURL string, policyDb *memdb.Memdb, collectionInterval string) (*PolicyRPCServer, error) {
	// create RPC server
	server, err := rpckit.NewRPCServer(globals.Tpm, listenURL, rpckit.WithLoggerEnabled(false))
	if err != nil {
		rpcLog.Errorf("failed to create rpc server with %s, err; %v", listenURL, err)
		return nil, err
	}

	stats := &statsPolicyRPCServer{
		policyDb: policyDb,
		clients:  sync.Map{},
	}

	stats.collectionInterval.Store(collectionInterval)

	fwlog := &fwlogPolicyRPCServer{
		policyDb: policyDb,
		clients:  sync.Map{},
	}

	flowexp := &flowExportPolicyRPCServer{
		policyDb: policyDb,
		clients:  sync.Map{},
	}

	rpcServer := &PolicyRPCServer{
		server:                    server,
		statsPolicyRPCServer:      stats,
		fwlogPolicyRPCServer:      fwlog,
		flowExportPolicyRPCServer: flowexp,
	}

	// register RPCs
	tpmProtos.RegisterStatsPolicyApiServer(server.GrpcServer, stats)
	tpmProtos.RegisterFwlogPolicyApiServer(server.GrpcServer, fwlog)
	tpmProtos.RegisterFlowExportPolicyApiServer(server.GrpcServer, flowexp)
	server.Start()

	return rpcServer, nil
}
