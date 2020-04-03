package ctrlerif

import (
	"context"
	"fmt"
	"io/ioutil"
	"os"
	"reflect"
	"strings"
	"sync"
	"sync/atomic"
	"testing"
	"time"

	protobuftypes "github.com/gogo/protobuf/types"
	uuid "github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/nic/agent/nevtsproxy/ctrlerif/types"
	evtsmgrprotos "github.com/pensando/sw/nic/agent/protos/evtprotos"
	"github.com/pensando/sw/venice/evtsproxy"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/emstore"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/events/policy"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/syslog"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"
)

func setup(t *testing.T) (*evtsproxy.EventsProxy, *policy.Manager, types.CtrlerIntf, string, string, emstore.Emstore, log.Logger) {
	logConfig := log.GetDefaultConfig(t.Name())
	logConfig.Filter = log.AllowAllFilter
	logConfig.Debug = true
	logger := log.GetNewLogger(logConfig).WithContext("t_name", t.Name())

	eventsDir, err := ioutil.TempDir("", "")
	AssertOk(t, err, "failed to create temp directory, err: %v", err)

	eps, err := evtsproxy.NewEventsProxy(t.Name(), globals.EvtsProxy, ":0",
		nil, 100*time.Second, 500*time.Millisecond, &events.StoreConfig{Dir: eventsDir}, logger)
	AssertOk(t, err, "failed to start events proxy, err: %v", err)
	eps.StartDispatch()

	// create agent store
	storePath := fmt.Sprintf("/tmp/%s", fmt.Sprintf("%s-%s", t.Name(), uuid.NewV4().String()))
	agentStore, err := emstore.NewEmstore(emstore.BoltDBType, storePath)
	AssertOk(t, err, "failed to create agent store, err: %v", err)

	// start events policy manager
	policyMgr, err := policy.NewManager(t.Name(), eps.GetEventsDispatcher(), logger, policy.WithStore(agentStore))
	AssertOk(t, err, "failed to create policy manager, err: %v", err)

	pHandler, err := NewEventPolicyHandler(policyMgr, logger)
	AssertOk(t, err, "failed to create event policy handler, err: %v", err)
	return eps, policyMgr, pHandler, storePath, eventsDir, agentStore, logger
}

func TestValidateEventPolicy(t *testing.T) {
	eps, policyMgr, pHandler, dbPath, eventsDir, agentStore, logger := setup(t)
	defer os.RemoveAll(eventsDir)
	defer os.Remove(dbPath)
	defer policyMgr.Stop()
	defer eps.Stop()
	defer agentStore.Close()

	eventPolicies := []struct {
		name   string
		fail   bool
		errStr string
		policy *evtsmgrprotos.EventPolicy
	}{
		{
			name:   "no name in the config",
			fail:   true,
			errStr: "no name provided in the config",
			policy: &evtsmgrprotos.EventPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "EventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
				},
				Spec: monitoring.EventPolicySpec{
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					SyslogConfig: &monitoring.SyslogExportConfig{
						FacilityOverride: "EVENTS-RFC3164",
					},
				},
			},
		},
		{
			name:   "no targets in the config",
			fail:   true,
			errStr: "no targets provided in the config",
			policy: &evtsmgrprotos.EventPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "EventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "no-targets-in-config",
				},
				Spec: monitoring.EventPolicySpec{
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					SyslogConfig: &monitoring.SyslogExportConfig{
						FacilityOverride: "EVENTS-RFC3164",
					},
				},
			},
		},
		{
			name:   "no destination in target",
			fail:   true,
			errStr: "destination cannot be empty",
			policy: &evtsmgrprotos.EventPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "EventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Name: "no-destination-in-target",
				},
				Spec: monitoring.EventPolicySpec{
					Targets: []*monitoring.ExportConfig{
						{
							Destination: "192.168.100.11",
							Transport:   "tcp/10001",
						},
						{
							Destination: "",
							Transport:   "tcp/10001",
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					SyslogConfig: &monitoring.SyslogExportConfig{
						FacilityOverride: "EVENTS-RFC3164",
					},
				},
			},
		},
		{
			name:   "invalid destination (no DNS)",
			fail:   true,
			errStr: "failed to resolve destination",
			policy: &evtsmgrprotos.EventPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "EventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Tenant:    globals.DefaultTenant,
					Name:      "invalid-destination-no-DNS",
				},
				Spec: monitoring.EventPolicySpec{
					Targets: []*monitoring.ExportConfig{
						{
							Destination: "192.168.100.11",
							Transport:   "tcp/10001",
						},
						{
							Destination: "test.pensando.iox",
							Transport:   "tcp/15001",
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					SyslogConfig: &monitoring.SyslogExportConfig{
						FacilityOverride: "EVENTS-RFC3164",
					},
				},
			},
		},
		{
			name:   "no port in target",
			fail:   true,
			errStr: "transport should be in protocol/port format",
			policy: &evtsmgrprotos.EventPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "EventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Tenant:    globals.DefaultTenant,
					Name:      "missing-port",
				},
				Spec: monitoring.EventPolicySpec{
					Targets: []*monitoring.ExportConfig{
						{
							Destination: "192.168.100.11",
							Transport:   "tcp/10001",
						},
						{
							Destination: "192.168.100.11",
							Transport:   "tcp",
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					SyslogConfig: &monitoring.SyslogExportConfig{
						FacilityOverride: "EVENTS-RFC3164",
					},
				},
			},
		},
		{
			name:   "invalid protocol, should be TCP/UDP",
			fail:   true,
			errStr: "invalid protocol",
			policy: &evtsmgrprotos.EventPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "EventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Tenant:    globals.DefaultTenant,
					Name:      "invalid-protocol",
				},
				Spec: monitoring.EventPolicySpec{
					Targets: []*monitoring.ExportConfig{
						{
							Destination: "192.168.100.11",
							Transport:   "ip/10001",
						},
						{
							Destination: "192.168.100.11",
							Transport:   "tcp/10001",
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					SyslogConfig: &monitoring.SyslogExportConfig{
						FacilityOverride: "EVENTS-RFC3164",
					},
				},
			},
		},
		{
			name:   "invalid port (abcd)",
			fail:   true,
			errStr: "invalid port",
			policy: &evtsmgrprotos.EventPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "EventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Tenant:    globals.DefaultTenant,
					Name:      "invalid-port",
				},
				Spec: monitoring.EventPolicySpec{
					Targets: []*monitoring.ExportConfig{
						{
							Destination: "192.168.100.11",
							Transport:   "tcp/10001",
						},
						{
							Destination: "192.168.100.11",
							Transport:   "tcp/abcd",
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					SyslogConfig: &monitoring.SyslogExportConfig{
						FacilityOverride: "EVENTS-RFC3164",
					},
				},
			},
		},
		{
			name:   "invalid port (65536)",
			fail:   true,
			errStr: "invalid port",
			policy: &evtsmgrprotos.EventPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "EventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Tenant:    globals.DefaultTenant,
					Name:      "invalid-port",
				},
				Spec: monitoring.EventPolicySpec{
					Targets: []*monitoring.ExportConfig{
						{
							Destination: "192.168.100.11",
							Transport:   "tcp/10001",
						},
						{
							Destination: "192.168.100.11",
							Transport:   "tcp/65536",
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					SyslogConfig: &monitoring.SyslogExportConfig{
						FacilityOverride: "EVENTS-RFC3164",
					},
				},
			},
		},
		{
			name: "good config",
			fail: false,
			policy: &evtsmgrprotos.EventPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "EventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Tenant:    globals.DefaultTenant,
					Name:      "good-config",
				},
				Spec: monitoring.EventPolicySpec{
					Targets: []*monitoring.ExportConfig{
						{
							Destination: "192.168.100.11",
							Transport:   "udp/10001",
						},
						{
							Destination: "registry.test.pensando.io",
							Transport:   "udp/15001",
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					SyslogConfig: &monitoring.SyslogExportConfig{
						FacilityOverride: "EVENTS-RFC3164",
					},
				},
			},
		},
	}

	ctx := context.Background()
	for _, p := range eventPolicies {
		logger.Infof("running test {%s}", p.name)
		err := pHandler.CreateEventPolicy(ctx, p.policy)
		if p.fail {
			fmt.Println(strings.Contains(err.Error(), p.errStr))
			Assert(t, err != nil, "{tc: %s} expected failure, but test succeeded", p.name)
			Assert(t, strings.Contains(err.Error(), p.errStr), "expected error string {%s} not found", p.errStr)
		} else {
			AssertOk(t, err, "{tc: %s} err: %v", p.name, err)

			l, err := pHandler.ListEventPolicy(ctx)
			AssertOk(t, err, "{tc: %s} failed to list event policies, err: %v", p.name, err)
			Assert(t, len(l) == 1, "{tc: %s} total event policies, expected: %v, got: %v", p.name, 1, len(l))

			g, err := pHandler.GetEventPolicy(ctx, p.policy)
			AssertOk(t, err, "{tc: %s} failed to get event policy, err: %v", p.name, err)
			Assert(t, reflect.DeepEqual(g, p.policy) == true, "{tc: %s} policy mismatch found, expected: %v, got: %v", p.name, p.policy, g)

			err = pHandler.UpdateEventPolicy(ctx, p.policy)
			AssertOk(t, err, "{tc: %s} failed to update event policy, err: %v", p.name, err)

			err = pHandler.DeleteEventPolicy(ctx, p.policy)
			AssertOk(t, err, "{tc: %s} failed to delete event policy, err: %v", p.name, err)
		}
	}
}

// TestEventPolicy
// 1. posts a event policy
// 2. ensures the events are exported to the syslog targets as per the posted policy
func TestEventPolicy(t *testing.T) {
	eps, policyMgr, pHandler, dbPath, eventsDir, agentStore, logger := setup(t)
	defer os.RemoveAll(eventsDir)
	defer os.Remove(dbPath)
	defer policyMgr.Stop()
	defer eps.Stop()
	defer agentStore.Close()

	var wg sync.WaitGroup

	// 1. start syslog servers
	// start UDP server to receive syslog messages
	pConn1, receivedMsgsAtUDPServer1, err := serviceutils.StartUDPServer(":0")
	AssertOk(t, err, "failed to start UDP server, err: %v", err)
	defer pConn1.Close()
	tmp1 := strings.Split(pConn1.LocalAddr().String(), ":")

	// start TCP server - 1 to receive syslog messages
	ln1, receivedMsgsAtTCPServer1, err := serviceutils.StartTCPServer(":0", 100, 0)
	AssertOk(t, err, "failed to start TCP server, err: %v", err)
	defer ln1.Close()
	tmp2 := strings.Split(ln1.Addr().String(), ":")

	// 2. upload policy
	// add event policy - 1
	eventPolicy1 := createEventPolicyObject(globals.DefaultTenant, globals.DefaultNamespace, "ep-1",
		monitoring.MonitoringExportFormat_name[int32(monitoring.MonitoringExportFormat_SYSLOG_BSD)],
		[]*monitoring.ExportConfig{
			{ // receivedMsgsAtUDPServer1
				Destination: "127.0.0.1",
				Transport:   fmt.Sprintf("udp/%s", tmp1[len(tmp1)-1]),
			},
			{ // receivedMsgsAtTCPServer1
				Destination: "127.0.0.1",
				Transport:   fmt.Sprintf("tcp/%s", tmp2[len(tmp2)-1]),
			},
		}, nil)
	err = pHandler.CreateEventPolicy(context.Background(), eventPolicy1)
	AssertOk(t, err, "failed to create event policy, err: %v", err)
	defer pHandler.DeleteEventPolicy(context.Background(), eventPolicy1)

	// 3. create events
	wg.Add(1)
	var count uint32
	stopEvtsRecorder := make(chan struct{})
	go func() {
		defer wg.Done()

		evtsRecorder, err := recorder.NewRecorder(&recorder.Config{
			Component:    fmt.Sprintf("%s-%s", t.Name(), uuid.NewV4().String()),
			EvtsProxyURL: eps.RPCServer.GetListenURL(),
			BackupDir:    eventsDir}, logger)
		if err != nil {
			log.Errorf("failed to create recorder, err: %v", err)
			return
		}

		for {
			select {
			case <-stopEvtsRecorder:
				return
			case <-time.After(100 * time.Millisecond):
				evtType := eventtypes.SERVICE_RUNNING
				if atomic.LoadUint32(&count)%2 == 0 {
					evtType = eventtypes.SERVICE_STOPPED
				}

				evtsRecorder.Event(evtType, fmt.Sprintf("event message - %d", count), nil)
				atomic.AddUint32(&count, 1)
			}
		}
	}()
	time.Sleep(2 * time.Second)
	close(stopEvtsRecorder)

	// 4. check if the events are received at the syslog server
	messages := []struct {
		format monitoring.MonitoringExportFormat // BSD style message contains the JSON formatted alert; RFC contains <msgID, structured data, msg>
		ch     chan string
	}{
		{format: monitoring.MonitoringExportFormat_SYSLOG_BSD, ch: receivedMsgsAtUDPServer1},
		{format: monitoring.MonitoringExportFormat_SYSLOG_BSD, ch: receivedMsgsAtTCPServer1},
	}
	for _, messageCh := range messages {
		wg.Add(1)
		go func(format monitoring.MonitoringExportFormat, messageCh chan string) {
			defer wg.Done()
			var totalMessagesReceived uint32
			for {
				select {
				case msg, ok := <-messageCh:
					if !ok {
						Assert(t, atomic.LoadUint32(&count) == totalMessagesReceived, "total sent: %v, received: %v", count, totalMessagesReceived)
						return
					}
					Assert(t, syslog.ValidateSyslogMessage(format, msg), "message format does not match")
					totalMessagesReceived++
					if atomic.LoadUint32(&count) == totalMessagesReceived {
						return
					}
				}
			}
		}(messageCh.format, messageCh.ch)
	}

	wg.Wait()
}

func TestEventPolicyCRUD(t *testing.T) {
	eps, policyMgr, pHandler, dbPath, eventsDir, agentStore, _ := setup(t)
	defer os.RemoveAll(eventsDir)
	defer os.Remove(dbPath)
	defer policyMgr.Stop()
	defer eps.Stop()
	defer agentStore.Close()

	policies := []*evtsmgrprotos.EventPolicyEvent{
		{ // create policy1
			EventType: api.EventType_CreateEvent,
			Policy: &evtsmgrprotos.EventPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "EventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Name: "policy1",
				},
				Spec: monitoring.EventPolicySpec{
					Targets: []*monitoring.ExportConfig{
						{
							Destination: "192.168.99.1",
							Transport:   "udp/11001",
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					SyslogConfig: &monitoring.SyslogExportConfig{
						FacilityOverride: "test",
					},
				},
			},
		},
		{ // create policy2
			EventType: api.EventType_CreateEvent,
			Policy: &evtsmgrprotos.EventPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "EventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Name: "policy2",
				},
				Spec: monitoring.EventPolicySpec{
					Targets: []*monitoring.ExportConfig{
						{
							Destination: "192.168.99.1",
							Transport:   "udp/11001",
						},
						{
							Destination: "192.168.99.2",
							Transport:   "udp/11002",
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					SyslogConfig: &monitoring.SyslogExportConfig{
						FacilityOverride: "test",
					},
				},
			},
		},
		{ // update - remove a target from policy2
			EventType: api.EventType_UpdateEvent,
			Policy: &evtsmgrprotos.EventPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "EventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Name: "policy2",
				},
				Spec: monitoring.EventPolicySpec{
					Targets: []*monitoring.ExportConfig{
						{
							Destination: "192.168.99.2",
							Transport:   "udp/11002",
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					SyslogConfig: &monitoring.SyslogExportConfig{
						FacilityOverride: "test",
					},
				},
			},
		},
		{ // delete policy1
			EventType: api.EventType_DeleteEvent,
			Policy: &evtsmgrprotos.EventPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "EventPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Name: "policy1",
				},
				Spec: monitoring.EventPolicySpec{
					Targets: []*monitoring.ExportConfig{
						{
							Destination: "192.168.99.1",
							Transport:   "udp/11001",
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					SyslogConfig: &monitoring.SyslogExportConfig{
						FacilityOverride: "test",
					},
				},
			},
		},
	}

	ctx := context.Background()
	for _, eventPolicyEvent := range policies {
		pName := eventPolicyEvent.Policy.GetName()
		switch eventPolicyEvent.EventType {
		case api.EventType_CreateEvent:
			err := pHandler.CreateEventPolicy(ctx, eventPolicyEvent.Policy)
			AssertOk(t, err, "failed to create policy {%s}, err: %v", pName, err)
		case api.EventType_UpdateEvent:
			err := pHandler.UpdateEventPolicy(ctx, eventPolicyEvent.Policy)
			AssertOk(t, err, "failed to update policy {%s}, err: %v", pName, err)
		case api.EventType_DeleteEvent:
			err := pHandler.DeleteEventPolicy(ctx, eventPolicyEvent.Policy)
			AssertOk(t, err, "failed to delete policy {%s}, err: %v", pName, err)
		}

		// check if the respective exporters are created/deleted
		switch eventPolicyEvent.EventType {
		case api.EventType_CreateEvent, api.EventType_UpdateEvent:
			// check if the exporter exists
			Assert(t, policyMgr.GetExportManager().ExporterExists(pName), "exporter for policy {%s} does not exists", pName)

			// check if each target exists in the exporter
			spec := eventPolicyEvent.Policy.Spec
			for _, target := range spec.GetTargets() {
				Assert(t, policyMgr.GetExportManager().TargetExists(pName, target), "target {%+v} from policy {%s} does not exists in the exporter", target, pName)
			}
		case api.EventType_DeleteEvent:
			Assert(t, !policyMgr.GetExportManager().ExporterExists(pName), "exporter exists for the deleted policy {%s}", pName)
		}
	}

	// delete non-existent policy
	err := pHandler.DeleteEventPolicy(ctx, &evtsmgrprotos.EventPolicy{TypeMeta: api.TypeMeta{Kind: "EventPolicy"}, ObjectMeta: api.ObjectMeta{Name: "dummy"}})
	Assert(t, strings.Contains(err.Error(), "policy does not exists"), "unexpected err: %v", err)

	// query without the kind
	_, err = pHandler.GetEventPolicy(ctx, &evtsmgrprotos.EventPolicy{ObjectMeta: api.ObjectMeta{Name: "dummy"}})
	Assert(t, err == policy.ErrFailedToGetPolicy, "unexpected err: %v", err)

	// query non-existent policy
	_, err = pHandler.GetEventPolicy(ctx, &evtsmgrprotos.EventPolicy{TypeMeta: api.TypeMeta{Kind: "EventPolicy"}, ObjectMeta: api.ObjectMeta{Name: "dummy"}})
	Assert(t, err == policy.ErrFailedToGetPolicy, "unexpected err: %v", err)

	// update non-existent policy
	err = pHandler.UpdateEventPolicy(ctx, policies[0].Policy)
	Assert(t, strings.Contains(err.Error(), "policy does not exists"), "unexpected err: %v", err)

	// create the existing policy
	err = pHandler.CreateEventPolicy(ctx, policies[1].Policy)
	Assert(t, strings.Contains(err.Error(), "policy exists already"), "unexpected err: %v", err)

	// TCP dial will fail  "dial tcp 192.168.99.2:11001: connect: connection refused"
	// it will internally retry the connection until it becomes successful
	policies[1].Policy.Spec.Targets = []*monitoring.ExportConfig{
		{
			Destination: "192.168.99.2",
			Transport:   "tcp/11001",
		},
	}
	err = pHandler.UpdateEventPolicy(ctx, policies[1].Policy) // stop and start with new config.
	AssertOk(t, err, "unexpected err")

	// TCP dial will fail "dial tcp 192.168.99.1:11001: connect: connection refused"
	// it will internally retry the connection until it becomes successful
	err = pHandler.CreateEventPolicy(ctx, &evtsmgrprotos.EventPolicy{
		TypeMeta: api.TypeMeta{
			Kind: "EventPolicy",
		},
		ObjectMeta: api.ObjectMeta{
			Name: "policy1",
		},
		Spec: monitoring.EventPolicySpec{
			Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
			Targets: []*monitoring.ExportConfig{
				{
					Destination: "192.168.99.1",
					Transport:   "tcp/11001",
				},
			},
		},
	})
	AssertOk(t, err, "unexpected err")

	// delete agent store
	os.Remove(dbPath)
	agentStore.Close()

	// perform CRUD on the non-existent DB; all the operations will fail as database is not open
	err = pHandler.CreateEventPolicy(ctx, &evtsmgrprotos.EventPolicy{
		TypeMeta: api.TypeMeta{
			Kind: "EventPolicy",
		},
		ObjectMeta: api.ObjectMeta{
			Name: "policy10",
		},
		Spec: monitoring.EventPolicySpec{
			Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
			Targets: []*monitoring.ExportConfig{
				{
					Destination: "192.168.99.10",
					Transport:   "tcp/11001",
				},
			},
		},
	})
	Assert(t, err == policy.ErrFailedToCreatePolicy, "unexpected err: %v", err)

	_, err = pHandler.GetEventPolicy(ctx, &evtsmgrprotos.EventPolicy{TypeMeta: api.TypeMeta{Kind: "EventPolicy"}, ObjectMeta: api.ObjectMeta{Name: "policy1"}})
	Assert(t, err == policy.ErrFailedToGetPolicy, "unexpected err: %v", err)

	err = pHandler.UpdateEventPolicy(ctx, policies[1].Policy)
	Assert(t, err == policy.ErrFailedToUpdatePolicy, "unexpected err: %v", err)

	err = pHandler.DeleteEventPolicy(ctx, policies[1].Policy)
	Assert(t, err == policy.ErrFailedToDeletePolicy, "unexpected err: %v", err)

	_, err = pHandler.ListEventPolicy(ctx)
	Assert(t, err == policy.ErrFailedToListPolicies, "unexpected err: %v", err)
}

func createEventPolicyObject(tenant, namespace, name, format string, targets []*monitoring.ExportConfig,
	syslogConfig *monitoring.SyslogExportConfig) *evtsmgrprotos.EventPolicy {
	creationTime, _ := protobuftypes.TimestampProto(time.Now())

	eventPolicyObj := &evtsmgrprotos.EventPolicy{
		TypeMeta: api.TypeMeta{Kind: "EventPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name:      name,
			UUID:      name,
			Tenant:    tenant,
			Namespace: namespace,
			CreationTime: api.Timestamp{
				Timestamp: *creationTime,
			},
			ModTime: api.Timestamp{
				Timestamp: *creationTime,
			},
		},
		Spec: monitoring.EventPolicySpec{
			Format:       format,
			Targets:      targets,
			SyslogConfig: syslogConfig,
		},
	}

	return eventPolicyObj
}
