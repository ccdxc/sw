package impl

import (
	"context"
	"errors"
	"fmt"
	"reflect"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/apigw/pkg/mocks"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/archive"
	"github.com/pensando/sw/venice/utils/archive/mock"
	"github.com/pensando/sw/venice/utils/log"
	rslvrmock "github.com/pensando/sw/venice/utils/resolver/mock"

	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestCancelPreCallHook(t *testing.T) {
	tests := []struct {
		name          string
		in            interface{}
		clientGetter  archive.ClientGetter
		archiveGetter archive.Getter
		out           interface{}
		skipCall      bool
		err           error
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
			in: &monitoring.CancelArchiveRequest{
				ObjectMeta: api.ObjectMeta{
					Name:   "test-arch1",
					Tenant: globals.DefaultTenant,
				},
			},
			clientGetter: mock.GetClientGetter(false, true),
			archiveGetter: mock.GetArchiveGetter(&monitoring.ArchiveRequest{
				ObjectMeta: api.ObjectMeta{
					Name:   "test-arch1",
					Tenant: globals.DefaultTenant,
				},
				Spec: monitoring.ArchiveRequestSpec{
					Type: monitoring.ArchiveRequestSpec_AuditEvent.String(),
				},
			}, false),
			out:      nil,
			skipCall: true,
			err:      errors.New("simulated client error"),
		},
		{
			name: "error in CancelRequest rpc",
			in: &monitoring.CancelArchiveRequest{
				ObjectMeta: api.ObjectMeta{
					Name:   "test-arch1",
					Tenant: globals.DefaultTenant,
				},
			},
			archiveGetter: mock.GetArchiveGetter(&monitoring.ArchiveRequest{
				ObjectMeta: api.ObjectMeta{
					Name:   "test-arch1",
					Tenant: globals.DefaultTenant,
				},
				Spec: monitoring.ArchiveRequestSpec{
					Type: monitoring.ArchiveRequestSpec_AuditEvent.String(),
				},
			}, false),
			clientGetter: mock.GetClientGetter(true, false),
			out:          nil,
			skipCall:     true,
			err:          errors.New("simulated CancelRequest error"),
		},
		{
			name: "successful Cancel request",
			in: &monitoring.CancelArchiveRequest{
				ObjectMeta: api.ObjectMeta{
					Name:   "test-arch1",
					Tenant: globals.DefaultTenant,
				},
			},
			clientGetter: mock.GetClientGetter(false, false),
			archiveGetter: mock.GetArchiveGetter(&monitoring.ArchiveRequest{
				ObjectMeta: api.ObjectMeta{
					Name:   "test-arch1",
					Tenant: globals.DefaultTenant,
				},
				Spec: monitoring.ArchiveRequestSpec{
					Type: monitoring.ArchiveRequestSpec_AuditEvent.String(),
				},
			}, false),
			out: &monitoring.ArchiveRequest{
				ObjectMeta: api.ObjectMeta{
					Name:   "test-arch1",
					Tenant: globals.DefaultTenant,
				},
				Spec: monitoring.ArchiveRequestSpec{
					Type: monitoring.ArchiveRequestSpec_AuditEvent.String(),
				},
			},
			skipCall: true,
			err:      nil,
		},
	}
	logConfig := log.GetDefaultConfig("TestAPIGwArchiveHooks")
	l := log.GetNewLogger(logConfig)
	r := &archiveHooks{}
	r.logger = l
	for _, test := range tests {
		r.clientGetter = test.clientGetter
		r.archiveGetter = test.archiveGetter
		ctx := context.TODO()
		rslvr := rslvrmock.New()
		r.rslvr = rslvr
		_, out, ok, err := r.CancelPreCallHook(ctx, test.in)
		Assert(t, reflect.DeepEqual(err, test.err), fmt.Sprintf("[%s] test failed, expected error [%v], got [%v]", test.name, test.err, err))
		Assert(t, reflect.DeepEqual(test.out, out),
			fmt.Sprintf("[%s] test failed, expected object [%v], got [%v]", test.name, test.out, out))
		Assert(t, test.skipCall == ok, fmt.Sprintf("[%s] test failed, expected skipcall [%v], got [%v]", test.name, test.skipCall, ok))
	}
}

func TestCancelPreCallHookRegistration(t *testing.T) {
	logConfig := log.GetDefaultConfig("TestAPIGwArchiveHooks")
	l := log.GetNewLogger(logConfig)
	svc := mocks.NewFakeAPIGwService(l, false)
	r := &archiveHooks{}
	r.logger = l
	err := r.registerCancelPreCallHook(svc)
	AssertOk(t, err, "cancelPreCallHook hook registration failed")

	prof, err := svc.GetServiceProfile("Cancel")
	AssertOk(t, err, fmt.Sprintf("error getting service profile for oper :%v", "Cancel"))
	Assert(t, len(prof.PreCallHooks()) == 1, fmt.Sprintf("unexpected number of pre call hooks [%d] for ArchiveRequest action [%v]", len(prof.PreCallHooks()), "Cancel"))

	// test err
	svc = mocks.NewFakeAPIGwService(l, true)
	err = r.registerCancelPreCallHook(svc)
	Assert(t, err != nil, "expected error in cancelPreCallHook hook registration")
}
