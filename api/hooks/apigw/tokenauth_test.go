// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package impl

import (
	"context"
	"fmt"
	"reflect"
	"strings"
	"testing"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/tokenauth"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apigw/pkg/mocks"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/log"

	. "github.com/pensando/sw/venice/utils/authz/testutils"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestTokenAuthAccess(t *testing.T) {
	logConfig := log.GetDefaultConfig("TestAPIGwAuthHooks")
	l := log.GetNewLogger(logConfig)
	r := &tokenAuthHooks{}
	r.logger = l

	req := &tokenauth.NodeTokenRequest{
		Audience: []string{"Hello"},
	}

	user := &auth.User{}
	user.Defaults("all")

	expectedOperations := []authz.Operation{
		authz.NewOperation(
			authz.NewResource(user.Tenant, "", auth.Permission_TokenAuth.String(), globals.DefaultNamespace, ""),
			auth.Permission_Read.String(),
		),
	}

	// negative test cases
	_, _, err := r.operations(context.TODO(), req)
	Assert(t, err == apigwpkg.ErrNoUserInContext, "Hook did not return error without user context")

	userCtx := apigwpkg.NewContextWithUser(context.TODO(), user)
	_, _, err = r.operations(userCtx, &user)
	Assert(t, strings.Contains(err.Error(), "invalid input type"), "Hook did not return error with invalid input type")

	// positive test case
	outCtx, out, err := r.operations(userCtx, req)
	AssertOk(t, err, "Hook returned unexpected error")
	Assert(t, reflect.DeepEqual(out, req), "Output object does not match input. Have: %+v, Want: %+v", out, req)
	operations, _ := apigwpkg.OperationsFromContext(outCtx)
	Assert(t, AreOperationsEqual(expectedOperations, operations),
		fmt.Sprintf("unexpected operations, Have:%+v, want:%+v", authz.PrintOperations(expectedOperations), authz.PrintOperations(operations)))
}

func TestTokenAuthHooksRegistration(t *testing.T) {
	logConfig := log.GetDefaultConfig("TestAPIGwTokenAuthHooks")
	l := log.GetNewLogger(logConfig)
	h := &tokenAuthHooks{}
	svc := mocks.NewFakeAPIGwService(l, false)
	err := h.registerTokenAuthHooks(svc)
	AssertOk(t, err, "apigw tokenauth hook registration failed")

	prof, err := svc.GetServiceProfile("GenerateNodeToken")
	AssertOk(t, err, "error getting service profile for method GenerateNodeToken")
	Assert(t, len(prof.PreAuthZHooks()) == 1, "Unexpected number of pre authz hooks for method GenerateNodeToken. Have: %d, want: %d", len(prof.PreAuthZHooks()), 1)

	// test error
	svc = mocks.NewFakeAPIGwService(l, true)
	err = h.registerTokenAuthHooks(svc)
	Assert(t, err != nil, "expected error in tokenauth hook registration")
}
