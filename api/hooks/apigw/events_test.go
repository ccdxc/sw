package impl

import (
	"context"
	"fmt"
	"reflect"
	"testing"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apigw/pkg/mocks"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/log"

	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestEventsOperationsHook(t *testing.T) {
	tests := []struct {
		name               string
		in                 interface{}
		expectedOperations []authz.Operation
		out                interface{}
		err                bool
	}{
		{
			name: "events operations hook test",
			in:   &events.Event{},
			expectedOperations: []authz.Operation{
				authz.NewOperation(authz.NewResource("",
					"events", auth.Permission_Event.String(),
					"", ""),
					auth.Permission_READ.String()),
			},
			out: &events.Event{},
			err: false,
		},
	}
	r := &eventsHooks{}
	logConfig := log.GetDefaultConfig("TestAPIGwEventsHooks")
	r.logger = log.GetNewLogger(logConfig)
	for _, test := range tests {
		nctx, out, err := r.operations(context.TODO(), test.in)
		Assert(t, test.err == (err != nil), fmt.Sprintf("got error [%v], [%s] test failed", err, test.name))
		operations, _ := apigwpkg.OperationsFromContext(nctx)
		Assert(t, areOperationsEqual(test.expectedOperations, operations),
			fmt.Sprintf("unexpected operations, [%s] test failed", test.name))
		Assert(t, reflect.DeepEqual(test.out, out),
			fmt.Sprintf("expected returned object [%v], got [%v], [%s] test failed", test.out, out, test.name))
	}
}

func TestEventsHooksRegistration(t *testing.T) {
	logConfig := log.GetDefaultConfig("TestAPIGwEventsHooks")
	l := log.GetNewLogger(logConfig)
	svc := mocks.NewFakeAPIGwService(l, false)
	err := registerEventsHooks(svc, l)
	AssertOk(t, err, "apigw events hook registration failed")

	methods := []string{"GetEvent", "GetEvents"}
	for _, method := range methods {
		prof, err := svc.GetServiceProfile(method)
		AssertOk(t, err, fmt.Sprintf("error getting service profile for method [%s]", method))
		Assert(t, len(prof.PreAuthZHooks()) == 1, fmt.Sprintf("unexpected number of pre authz hooks [%d] for method [%s]", len(prof.PreAuthZHooks()), method))
	}

	// test error
	svc = mocks.NewFakeAPIGwService(l, true)
	err = registerEventsHooks(svc, l)
	Assert(t, err != nil, "expected error in events hook registration")
}
