package rpcserver

import (
	"context"
	"encoding/json"
	"fmt"
	"strings"
	"sync"
	"time"

	"github.com/pensando/sw/nic/agent/protos/netproto"

	"github.com/pensando/sw/api"
	apiProtos "github.com/pensando/sw/api/generated/monitoring"
	tpmProtos "github.com/pensando/sw/nic/agent/protos/tpmprotos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/ctxutils"
	"github.com/pensando/sw/venice/utils/diagnostics"
	vlog "github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
	"github.com/pensando/sw/venice/utils/rpckit"
)

var pkgName = "rpcserver"
var rpcLog = vlog.WithContext("pkg", "rpcserver")

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
	*fwlogPolicyRPCServer
	*flowExportPolicyRPCServer
}

// convert memdb event type to api event type
var apiEventTypeMap = map[memdb.EventType]api.EventType{
	memdb.CreateEvent: api.EventType_CreateEvent,
	memdb.UpdateEvent: api.EventType_UpdateEvent,
	memdb.DeleteEvent: api.EventType_DeleteEvent,
}

func (p *fwlogPolicyRPCServer) ListFwlogPolicy(ctx context.Context, meta *api.ObjectMeta) (*tpmProtos.FwlogPolicyEventList, error) {
	// track clients
	peer := ctxutils.GetPeerAddress(ctx)
	if len(peer) > 0 {
		p.clients.Store(peer, time.Now().Format(time.RFC3339))
	}

	ev := &tpmProtos.FwlogPolicyEventList{}
	for _, obj := range p.policyDb.ListObjects("FwlogPolicy", nil) {
		if policy, ok := obj.(*apiProtos.FwlogPolicy); ok {
			ev.EventList = append(ev.EventList, &tpmProtos.FwlogPolicyEvent{
				EventType: api.EventType_CreateEvent,
				Policy: &tpmProtos.FwlogPolicy{
					TypeMeta:   policy.TypeMeta,
					ObjectMeta: policy.ObjectMeta,
					Spec:       policy.Spec,
				},
			})
		}
	}
	return ev, nil
}

func (p *fwlogPolicyRPCServer) WatchFwlogPolicy(in *api.ObjectMeta, out tpmProtos.FwlogPolicyApiV1_WatchFwlogPolicyServer) error {

	watcher := memdb.Watcher{}
	watcher.Channel = make(chan memdb.Event, memdb.WatchLen)
	defer close(watcher.Channel)
	p.policyDb.WatchObjects("FwlogPolicy", &watcher)
	defer p.policyDb.StopWatchObjects("FwlogPolicy", &watcher)

	ctx := out.Context()

	// track clients
	peer := ctxutils.GetPeerAddress(ctx)
	if len(peer) > 0 {
		p.clients.Store(peer, time.Now().Format(time.RFC3339))
		defer p.clients.Delete(peer)
		rpcLog.Infof("watch fwlog policy from [%v]", peer)
	}

	// send existing policy
	for _, obj := range p.policyDb.ListObjects("FwlogPolicy", nil) {
		if policy, ok := obj.(*apiProtos.FwlogPolicy); ok {
			fwlogPolicy := &tpmProtos.FwlogPolicy{
				TypeMeta:   policy.TypeMeta,
				ObjectMeta: policy.ObjectMeta,
				Spec:       policy.Spec,
			}

			if err := out.Send(&tpmProtos.FwlogPolicyEvent{EventType: api.EventType_CreateEvent,
				Policy: fwlogPolicy}); err != nil {
				rpcLog.Errorf("failed to send fwlog policy to %s, error %s", peer, err)
				return err
			}
		} else {
			rpcLog.Errorf("invalid fwlog policy from list %T", obj)
			return fmt.Errorf("invalid fwlog policy from list")
		}
	}

	// loop forever on watch channel
	for {
		select {
		case event, ok := <-watcher.Channel:

			if !ok {
				rpcLog.Errorf("error from %v in fwlog channel ", peer)
				return fmt.Errorf("invalid event from watch channel")
			}

			// Update last activity time
			if len(peer) > 0 {
				p.clients.Store(peer, time.Now().Format(time.RFC3339))
			}

			policy, ok := event.Obj.(*apiProtos.FwlogPolicy)
			if !ok {
				rpcLog.Errorf("fwlog watch error received from [%s]", peer)
				return fmt.Errorf("watch error")
			}

			fwlogPolicy := &tpmProtos.FwlogPolicy{
				TypeMeta:   policy.TypeMeta,
				ObjectMeta: policy.ObjectMeta,
				Spec:       policy.Spec,
			}

			if err := out.Send(&tpmProtos.FwlogPolicyEvent{EventType: apiEventTypeMap[event.EventType],
				Policy: fwlogPolicy}); err != nil {
				rpcLog.Errorf("failed to send fwlog policy to %s, error %s", peer, err)
				return err
			}

		case <-ctx.Done():
			rpcLog.Errorf("fwlog policy client(%s) context canceled, error:%s", peer, ctx.Err())
			return fmt.Errorf("context canceled")
		}
	}
}

func (p *flowExportPolicyRPCServer) ListFlowExportPolicy(ctx context.Context, meta *api.ObjectMeta) (*netproto.FlowExportPolicyEventList, error) {
	// track clients
	peer := ctxutils.GetPeerAddress(ctx)
	if len(peer) > 0 {
		p.clients.Store(peer, time.Now().Format(time.RFC3339))
	}

	ev := &netproto.FlowExportPolicyEventList{}
	for _, obj := range p.policyDb.ListObjects("FlowExportPolicy", nil) {
		if apiPol, ok := obj.(*apiProtos.FlowExportPolicy); ok {
			p, err := json.Marshal(apiPol)
			if err != nil {
				rpcLog.Errorf("invalid flow export policy from list %+v", obj)
				continue
			}
			fp := &netproto.FlowExportPolicy{}

			if err := json.Unmarshal(p, &fp); err != nil {
				rpcLog.Errorf("failed to convert flow export policy from list %+v", obj)
				continue
			}

			ev.FlowExportPolicyEvents = append(ev.FlowExportPolicyEvents, &netproto.FlowExportPolicyEvent{
				EventType: api.EventType_CreateEvent,
				Policy: &netproto.FlowExportPolicy{
					TypeMeta:   fp.TypeMeta,
					ObjectMeta: fp.ObjectMeta,
					Spec:       fp.Spec,
				},
			})
		}
	}
	return ev, nil
}
func (p *flowExportPolicyRPCServer) WatchFlowExportPolicy(in *api.ObjectMeta, out netproto.FlowExportPolicyApiV1_WatchFlowExportPolicyServer) error {

	watcher := memdb.Watcher{}
	watcher.Channel = make(chan memdb.Event, memdb.WatchLen)
	p.policyDb.WatchObjects("FlowExportPolicy", &watcher)
	defer p.policyDb.StopWatchObjects("FlowExportPolicy", &watcher)

	ctx := out.Context()

	// track clients
	peer := ctxutils.GetPeerAddress(ctx)
	if len(peer) > 0 {
		p.clients.Store(peer, time.Now().Format(time.RFC3339))
		defer p.clients.Delete(peer)
	}

	rpcLog.Infof("watch flowexport policy from [%v]", peer)

	// send existing policy
	for _, obj := range p.policyDb.ListObjects("FlowExportPolicy", nil) {
		var fePolicy *apiProtos.FlowExportPolicy
		var ok bool
		if fePolicy, ok = obj.(*apiProtos.FlowExportPolicy); !ok {
			rpcLog.Errorf("invalid flow export policy from list, %T", obj)
			return fmt.Errorf("invalid flow export policy from list")
		}

		flowExportPolicy := &netproto.FlowExportPolicy{
			TypeMeta:   fePolicy.TypeMeta,
			ObjectMeta: fePolicy.ObjectMeta,
			Spec:       convertFlowExportPolicySpec(&fePolicy.Spec),
		}

		if err := out.Send(&netproto.FlowExportPolicyEventList{
			FlowExportPolicyEvents: []*netproto.FlowExportPolicyEvent{
				{
					EventType: api.EventType_CreateEvent,
					Policy:    flowExportPolicy,
				},
			}}); err != nil {
			rpcLog.Errorf("failed to send flowexport policy to %s, error %s", peer, err)
			return err
		}
	}

	// loop forever on watch channel
	for {
		select {
		case event, ok := <-watcher.Channel:
			if !ok {
				rpcLog.Errorf("error from %v in flowexport channel", peer)
				return fmt.Errorf("invalid event from watch channel")
			}

			// Update last activity time
			if len(peer) > 0 {
				p.clients.Store(peer, time.Now().Format(time.RFC3339))
			}

			fePolicy, ok := event.Obj.(*apiProtos.FlowExportPolicy)
			if !ok {
				rpcLog.Errorf("invalid flow export policy from list, %T", event.Obj)
				return fmt.Errorf("invalid flow export policy from list")
			}

			flowExportPolicy := &netproto.FlowExportPolicy{
				TypeMeta:   fePolicy.TypeMeta,
				ObjectMeta: fePolicy.ObjectMeta,
				Spec:       convertFlowExportPolicySpec(&fePolicy.Spec),
			}

			if err := out.Send(
				&netproto.FlowExportPolicyEventList{
					FlowExportPolicyEvents: []*netproto.FlowExportPolicyEvent{
						{
							EventType: apiEventTypeMap[event.EventType],
							Policy:    flowExportPolicy,
						},
					}}); err != nil {
				rpcLog.Errorf("failed to send flowexport policy to %s, error %s", peer, err)
				return err
			}

		case <-ctx.Done():
			rpcLog.Errorf("flow export policy client(%s) context canceled, error:%s", peer, ctx.Err())
			return fmt.Errorf("context canceled")
		}
	}
}

// Stop rpc server
func (s *PolicyRPCServer) Stop() error {
	return s.server.Stop()
}

// GetListenURL is to get grpc listen address
func (s *PolicyRPCServer) GetListenURL() string {
	return s.server.GetListenURL()
}

// Debug is to dump the state
func (s *PolicyRPCServer) Debug() map[string]map[string]string {
	clients := map[string]map[string]string{
		"FwlogPolicy":      {},
		"FlowExportPolicy": {},
	}
	s.fwlogPolicyRPCServer.clients.Range(func(k interface{}, v interface{}) bool {
		if key, ok := k.(string); ok {
			if val, ok := v.(string); ok {
				clients["FwlogPolicy"][key] = val
			}
		}
		return true
	})
	s.flowExportPolicyRPCServer.clients.Range(func(k interface{}, v interface{}) bool {
		if key, ok := k.(string); ok {
			if val, ok := v.(string); ok {
				clients["FlowExportPolicy"][key] = val
			}
		}
		return true
	})
	return clients
}

// NewRPCServer starts a new instance of rpc server
func NewRPCServer(listenURL string, policyDb *memdb.Memdb, diagSvc diagnostics.Service) (*PolicyRPCServer, error) {
	// create RPC server
	server, err := rpckit.NewRPCServer(globals.Tpm, listenURL, rpckit.WithLoggerEnabled(false))
	if err != nil {
		rpcLog.Errorf("failed to create rpc server with %s, err; %v", listenURL, err)
		return nil, err
	}

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
		fwlogPolicyRPCServer:      fwlog,
		flowExportPolicyRPCServer: flowexp,
	}

	// register RPCs
	tpmProtos.RegisterFwlogPolicyApiV1Server(server.GrpcServer, fwlog)
	netproto.RegisterFlowExportPolicyApiV1Server(server.GrpcServer, flowexp)
	if diagSvc != nil {
		diagnostics.RegisterService(server.GrpcServer, diagSvc)
	}
	server.Start()

	return rpcServer, nil
}

func convertFlowExportPolicySpec(fePolicySpec *apiProtos.FlowExportPolicySpec) netproto.FlowExportPolicySpec {
	var (
		matchRules                 []netproto.MatchRule
		srcAddresses, dstAddresses []string
		exports                    []netproto.ExportConfig
	)

	for _, r := range fePolicySpec.MatchRules {
		var protoPorts []*netproto.ProtoPort
		if r.AppProtoSel != nil {
			for _, pp := range r.AppProtoSel.ProtoPorts {
				var protoPort netproto.ProtoPort
				components := strings.Split(pp, "/")
				switch len(components) {
				case 1:
					protoPort.Protocol = components[0]
				case 2:
					protoPort.Protocol = components[0]
					protoPort.Port = components[1]
				case 3:
					protoPort.Protocol = components[0]
					protoPort.Port = fmt.Sprintf("%s/%s", components[1], components[2])
				default:
					continue
				}
				protoPorts = append(protoPorts, &protoPort)
			}
		}

		if r.Src != nil && r.Src.IPAddresses != nil {
			srcAddresses = r.Src.IPAddresses
		}

		if r.Dst != nil && r.Dst.IPAddresses != nil {
			dstAddresses = r.Dst.IPAddresses
		}

		m := netproto.MatchRule{
			Src: &netproto.MatchSelector{
				Addresses: srcAddresses,
			},
			Dst: &netproto.MatchSelector{
				Addresses:  dstAddresses,
				ProtoPorts: protoPorts,
			},
		}
		matchRules = append(matchRules, m)
	}

	for _, exp := range fePolicySpec.Exports {
		var protoPort netproto.ProtoPort
		components := strings.Split(exp.Transport, "/")
		switch len(components) {
		case 1:
			protoPort.Protocol = components[0]
		case 2:
			protoPort.Protocol = components[0]
			protoPort.Port = components[1]
		case 3:
			protoPort.Protocol = components[0]
			protoPort.Port = fmt.Sprintf("%s/%s", components[1], components[2])
		default:
			continue
		}
		e := netproto.ExportConfig{
			Destination: exp.Destination,
			Transport:   &protoPort,
		}

		exports = append(exports, e)
	}

	spec := netproto.FlowExportPolicySpec{
		VrfName:          fePolicySpec.VrfName,
		Interval:         fePolicySpec.Interval,
		TemplateInterval: fePolicySpec.TemplateInterval,
		Format:           fePolicySpec.Format,
		Exports:          exports,
		MatchRules:       matchRules,
	}

	return spec
}
