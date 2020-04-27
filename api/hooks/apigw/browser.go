package impl

import (
	"context"
	"errors"
	"strings"

	"google.golang.org/grpc/metadata"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authz"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/runtime"

	"github.com/pensando/sw/api/generated/browser"
	"github.com/pensando/sw/venice/apigw"
	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/utils/log"
)

type browserHooks struct {
	logger log.Logger
}

func (h *browserHooks) getURI(ctx context.Context) (string, error) {
	md, ok := metadata.FromOutgoingContext(ctx)
	if !ok {
		h.logger.Infof("failed to get grpc MD")
		return "", errors.New("invalid input")
	}
	url, ok := md[apiserver.RequestParamsRequestURI]
	if !ok {
		h.logger.Infof("failed to determine URI")
		return "", errors.New("invalid input")
	}
	parts := strings.SplitN(url[0], "/", 5)
	if len(parts) < 4 {
		h.logger.Infof("failed to determine URI")
		return "", errors.New("invalid input")
	}
	return "/" + globals.ConfigURIPrefix + "/" + parts[4], nil
}

func (h *browserHooks) addOperations(ctx context.Context, i interface{}) (context.Context, interface{}, error) {
	h.logger.InfoLog("msg", "received addOperations PreAuthzHook callback", "obj", i)

	switch i.(type) {
	case *browser.BrowseRequest:
	case *browser.BrowseRequestList:
	default:
		return ctx, i, errors.New("invalid input")
	}
	user, ok := apigwpkg.UserFromContext(ctx)
	if !ok || user == nil {
		h.logger.ErrorLog("msg", "no user present in context passed to browser  addOperations PreAuthZ hook")
		return ctx, i, apigwpkg.ErrNoUserInContext
	}
	operations, _ := apigwpkg.OperationsFromContext(ctx)
	resource := authz.NewResource(
		user.Tenant,
		"",
		auth.Permission_Search.String(),
		"",
		"")
	resource.SetOwner(user)
	operations = append(operations, authz.NewOperation(resource, auth.Permission_Read.String()))
	nctx := apigwpkg.NewContextWithOperations(ctx, operations...)
	return nctx, i, nil
}

func (h *browserHooks) referencesPreCallHook(ctx context.Context, i, out interface{}) (context.Context, interface{}, interface{}, bool, error) {
	h.logger.InfoLog("msg", "received references PreCallHook callback", "obj", i)
	in, ok := i.(*browser.BrowseRequest)
	if !ok {
		return ctx, i, out, true, errors.New("invalid input")
	}

	in.QueryType = browser.QueryType_Dependencies.String()
	uri, err := h.getURI(ctx)
	if err != nil {
		return ctx, in, out, true, err
	}
	sch := runtime.GetDefaultScheme()
	in.URI = sch.GetKey(uri)
	if in.URI == "" {
		h.logger.ErrorLog("msg", "could not map URI to key", "URI", uri)
		return ctx, i, out, false, errors.New("unknown URI path")
	}
	return ctx, in, out, false, nil
}

func (h *browserHooks) refereesPreCallHook(ctx context.Context, i, out interface{}) (context.Context, interface{}, interface{}, bool, error) {
	h.logger.InfoLog("msg", "received referees PreCallHook callback", "obj", i)
	in, ok := i.(*browser.BrowseRequest)
	if !ok {
		return ctx, i, out, true, errors.New("invalid input")
	}

	in.QueryType = browser.QueryType_DependedBy.String()
	uri, err := h.getURI(ctx)
	if err != nil {
		return ctx, in, out, true, err
	}
	sch := runtime.GetDefaultScheme()
	in.URI = sch.GetKey(uri)
	if in.URI == "" {
		h.logger.ErrorLog("msg", "could not map URI to key", "URI", uri)
		return ctx, i, out, false, errors.New("unknown URI path")
	}
	return ctx, i, out, false, nil
}

func (h *browserHooks) queryPreCallHook(ctx context.Context, i, out interface{}) (context.Context, interface{}, interface{}, bool, error) {
	h.logger.InfoLog("msg", "received query PreCallHook callback", "obj", i)
	reqList, ok := i.(*browser.BrowseRequestList)

	if !ok {
		return ctx, i, out, true, errors.New("invalid input")
	}

	for ix, br := range reqList.RequestList {
		sch := runtime.GetDefaultScheme()
		uri := br.URI
		br.URI = sch.GetKey(uri)
		if br.URI == "" {
			h.logger.ErrorLog("msg", "could not map URI to key", "URI", uri)
			return ctx, i, out, false, errors.New("unknown URI path")
		}
		reqList.RequestList[ix] = br
	}
	return ctx, i, out, false, nil
}

func registerBrowserHooks(svc apigw.APIGatewayService, l log.Logger) error {
	r := browserHooks{logger: l}
	prof, err := svc.GetServiceProfile("References")
	if err != nil {
		return err
	}
	prof.AddPreAuthZHook(r.addOperations)
	prof.AddPreCallHook(r.referencesPreCallHook)

	prof, err = svc.GetServiceProfile("Referrers")
	if err != nil {
		return err
	}
	prof.AddPreAuthZHook(r.addOperations)
	prof.AddPreCallHook(r.refereesPreCallHook)

	prof, err = svc.GetServiceProfile("Query")
	if err != nil {
		return err
	}
	prof.AddPreAuthZHook(r.addOperations)
	prof.AddPreCallHook(r.queryPreCallHook)

	return nil
}

func init() {
	gw := apigwpkg.MustGetAPIGateway()
	gw.RegisterHooksCb("browser.BrowserV1", registerBrowserHooks)
}
