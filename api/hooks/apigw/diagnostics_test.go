package impl

import (
	"context"
	"errors"
	"fmt"
	"reflect"
	"testing"

	"github.com/pensando/sw/api"
	diagapi "github.com/pensando/sw/api/generated/diagnostics"
	"github.com/pensando/sw/venice/apigw/pkg/mocks"
	cmdprotos "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/diagnostics"
	"github.com/pensando/sw/venice/utils/diagnostics/mock"
	"github.com/pensando/sw/venice/utils/log"
	rslvrmock "github.com/pensando/sw/venice/utils/resolver/mock"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestDebugPreCallHook(t *testing.T) {
	tests := []struct {
		name         string
		in           interface{}
		svcInstance  *cmdprotos.ServiceInstance
		clientGetter diagnostics.ClientGetter
		moduleGetter diagnostics.Getter
		router       diagnostics.Router
		out          interface{}
		skipCall     bool
		err          error
	}{
		{
			name:     "invalid object",
			in:       &struct{ name string }{name: "invalid object type"},
			out:      nil,
			skipCall: true,
			err:      errors.New("invalid input type"),
		},
		{
			name: "error in getting client",
			in: &diagapi.DiagnosticsRequest{
				ObjectMeta: api.ObjectMeta{
					Name: "node1-pen-spyglass",
				},
			},
			svcInstance: &cmdprotos.ServiceInstance{
				Node:    "node1",
				Service: globals.Spyglass,
				URL:     "192.168.10.11:9001",
			},
			moduleGetter: mock.GetModuleGetter(
				&diagapi.Module{
					ObjectMeta: api.ObjectMeta{
						Name: "node1-pen-spyglass",
					},
					Status: diagapi.ModuleStatus{
						Module: globals.Spyglass,
						Node:   "node1",
					},
				}, false),
			clientGetter: mock.GetClientGetter(false, true),
			out:          nil,
			skipCall:     true,
			err:          errors.New("simulated client error"),
		},
		{
			name: "error in getting module",
			in: &diagapi.DiagnosticsRequest{
				ObjectMeta: api.ObjectMeta{
					Name: "node1-pen-spyglass",
				},
			},
			moduleGetter: mock.GetModuleGetter(
				&diagapi.Module{}, true),
			out:      nil,
			skipCall: true,
			err:      errors.New("simulated module error"),
		},
		{
			name: "no service instance",
			in: &diagapi.DiagnosticsRequest{
				ObjectMeta: api.ObjectMeta{
					Name: "node2-pen-spyglass",
				},
			},
			moduleGetter: mock.GetModuleGetter(
				&diagapi.Module{
					ObjectMeta: api.ObjectMeta{
						Name: "node2-pen-spyglass",
					},
					Status: diagapi.ModuleStatus{
						Module: globals.Spyglass,
						Node:   "node2",
						ServicePorts: []diagapi.ServicePort{
							{
								Name: globals.Spyglass,
								Port: 9011,
							},
						},
					},
				}, false),
			out:      nil,
			skipCall: true,
			err:      fmt.Errorf("unable to locate service instance for module [%s]", "node2-pen-spyglass"),
		},
		{
			name: "unsupported module",
			in: &diagapi.DiagnosticsRequest{
				ObjectMeta: api.ObjectMeta{
					Name: "node1-unsupported",
				},
			},
			svcInstance: &cmdprotos.ServiceInstance{
				Node:    "node1",
				Service: "unsupported",
				URL:     "192.168.10.11:9001",
			},
			moduleGetter: mock.GetModuleGetter(
				&diagapi.Module{
					ObjectMeta: api.ObjectMeta{
						Name: "node1-unsupported",
					},
					Status: diagapi.ModuleStatus{
						Module: "unsupported",
						Node:   "node1",
						ServicePorts: []diagapi.ServicePort{
							{
								Name: "unsupported",
								Port: 786,
							},
						},
					},
				}, false),
			out:      nil,
			skipCall: true,
			err:      fmt.Errorf("diagnostics not supported for module [%s]", "node1-unsupported"),
		},
		{
			name: "error in Debug rpc",
			in: &diagapi.DiagnosticsRequest{
				ObjectMeta: api.ObjectMeta{
					Name: "node1-pen-spyglass",
				},
			},
			svcInstance: &cmdprotos.ServiceInstance{
				Node:    "node1",
				Service: globals.Spyglass,
				URL:     "192.168.10.11:9001",
			},
			moduleGetter: mock.GetModuleGetter(
				&diagapi.Module{
					ObjectMeta: api.ObjectMeta{
						Name: "node1-pen-spyglass",
					},
					Status: diagapi.ModuleStatus{
						Module: globals.Spyglass,
						Node:   "node1",
					},
				}, false),
			clientGetter: mock.GetClientGetter(true, false),
			out:          nil,
			skipCall:     true,
			err:          errors.New("simulated Debug error"),
		},
		{
			name: "successful Debug request",
			in: &diagapi.DiagnosticsRequest{
				ObjectMeta: api.ObjectMeta{
					Name: "node1-pen-spyglass",
				},
				Query: diagapi.DiagnosticsRequest_Log.String(),
			},
			svcInstance: &cmdprotos.ServiceInstance{
				Node:    "node1",
				Service: globals.Spyglass,
				URL:     "192.168.10.11:9001",
			},
			moduleGetter: mock.GetModuleGetter(
				&diagapi.Module{
					ObjectMeta: api.ObjectMeta{
						Name: "node1-pen-spyglass",
					},
					Status: diagapi.ModuleStatus{
						Module: globals.Spyglass,
						Node:   "node1",
					},
				}, false),
			clientGetter: mock.GetClientGetter(false, false),
			out:          &diagapi.DiagnosticsResponse{Object: &api.Any{}},
			skipCall:     true,
			err:          nil,
		},
		{
			name: "invalid service name",
			in: &diagapi.DiagnosticsRequest{
				ObjectMeta: api.ObjectMeta{
					Name: "node1-pen-citadel",
				},
				Query: diagapi.DiagnosticsRequest_Log.String(),
				ServicePort: diagapi.ServicePort{
					Name: "invalid",
				},
			},
			svcInstance: &cmdprotos.ServiceInstance{
				Node:    "node1",
				Service: globals.Citadel,
				URL:     "192.168.10.11:7087",
			},
			moduleGetter: mock.GetModuleGetter(
				&diagapi.Module{
					ObjectMeta: api.ObjectMeta{
						Name: "node1-pen-citadel",
					},
					Status: diagapi.ModuleStatus{
						Module: globals.Citadel,
						Node:   "node1",
						ServicePorts: []diagapi.ServicePort{
							{
								Name: "pen-citadel",
								Port: 7087,
							},
							{
								Name: "pen-collector",
								Port: 10777,
							},
						},
					},
				}, false),
			out:      nil,
			skipCall: true,
			err:      fmt.Errorf("invalid service name [%s]", "invalid"),
		},
	}
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	r := &diagnosticsHooks{}
	r.logger = l
	for _, test := range tests {
		r.clientGetter = test.clientGetter
		r.moduleGetter = test.moduleGetter
		ctx := context.TODO()
		rslvr := rslvrmock.New()
		if test.svcInstance != nil {
			rslvr.AddServiceInstance(test.svcInstance)
		}
		r.rslvr = rslvr
		_, out, _, ok, err := r.DebugPreCallHook(ctx, test.in, nil)
		Assert(t, reflect.DeepEqual(err, test.err), fmt.Sprintf("[%s] test failed, expected error [%v], got [%v]", test.name, test.err, err))
		Assert(t, reflect.DeepEqual(test.out, out),
			fmt.Sprintf("[%s] test failed, expected object [%v], got [%v]", test.name, test.out, out))
		Assert(t, test.skipCall == ok, fmt.Sprintf("[%s] test failed, expected skipcall [%v], got [%v]", test.name, test.skipCall, ok))
	}
}

func TestDebugPreCallHookRegistration(t *testing.T) {
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	svc := mocks.NewFakeAPIGwService(l, false)
	r := &diagnosticsHooks{}
	r.logger = l
	err := r.registerDebugPreCallHook(svc)
	AssertOk(t, err, "debugPreCallHook hook registration failed")

	prof, err := svc.GetServiceProfile("Debug")
	AssertOk(t, err, fmt.Sprintf("error getting service profile for oper :%v", "IsAuthorized"))
	Assert(t, len(prof.PreCallHooks()) == 1, fmt.Sprintf("unexpected number of pre call hooks [%d] for User action [%v]", len(prof.PreCallHooks()), "IsAuthorized"))

	// test err
	svc = mocks.NewFakeAPIGwService(l, true)
	err = r.registerDebugPreCallHook(svc)
	Assert(t, err != nil, "expected error in isAuthorizedPreCallHook hook registration")
}
