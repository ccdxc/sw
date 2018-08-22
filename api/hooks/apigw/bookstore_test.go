package impl

import (
	"fmt"
	"testing"

	"github.com/pensando/sw/venice/apigw/pkg/mocks"
	"github.com/pensando/sw/venice/utils/log"

	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestBookstoreHooksRegistration(t *testing.T) {
	logConfig := log.GetDefaultConfig("TestAPIGwBookstoreHooks")
	l := log.GetNewLogger(logConfig)
	svc := mocks.NewFakeAPIGwService(l, false)
	r := &bookstoreHooks{}
	r.logger = l
	err := registerBookstoreHooks(svc, l)
	AssertOk(t, err, "apigw bookstore hook registration failed")

	prof, err := svc.GetCrudServiceProfile("Order", "create")
	AssertOk(t, err, "error getting service profile for Order create")
	Assert(t, len(prof.PreCallHooks()) == 1, fmt.Sprintf("unexpected number of pre call hooks [%d] for Order create profile", len(prof.PreCallHooks())))
	Assert(t, len(prof.PreCallHooks()) == 1, fmt.Sprintf("unexpected number of post call hooks [%d] for Order create profile", len(prof.PreCallHooks())))

	// test error
	svc = mocks.NewFakeAPIGwService(l, true)
	err = registerBookstoreHooks(svc, l)
	Assert(t, err != nil, "expected error in bookstore hook registration")
}
