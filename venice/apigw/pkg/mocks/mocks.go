package mocks

import (
	"context"
	"errors"
	"net/http"
	"sync"

	"google.golang.org/grpc"

	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

type fakeAPIGwService struct {
	logger        log.Logger
	svcProf       map[string]apigw.ServiceProfile
	simulateError bool
	regevents     int
}

func (m *fakeAPIGwService) CompleteRegistration(ctx context.Context, logger log.Logger, grpcserver *grpc.Server, mux *http.ServeMux, rslvr resolver.Interface, wg *sync.WaitGroup) error {
	m.regevents = m.regevents + 1
	return nil
}

func (m *fakeAPIGwService) GetServiceProfile(method string) (apigw.ServiceProfile, error) {
	if m.simulateError {
		return nil, errors.New("not found")
	}
	if ret, ok := m.svcProf[method]; ok {
		return ret, nil
	}
	prof := apigwpkg.NewServiceProfile(nil)
	m.svcProf[method] = prof
	return prof, nil
}

func (m *fakeAPIGwService) GetCrudServiceProfile(object string, oper apiserver.APIOperType) (apigw.ServiceProfile, error) {
	if m.simulateError {
		return nil, errors.New("not found")
	}
	name := apiserver.GetCrudServiceName(object, oper)
	if name != "" {
		return m.GetServiceProfile(name)
	}
	return nil, errors.New("not found")
}

// NewFakeAPIGwService is used to test API Gateway hooks
func NewFakeAPIGwService(l log.Logger, simulateError bool) apigw.APIGatewayService {
	return &fakeAPIGwService{
		svcProf:       make(map[string]apigw.ServiceProfile),
		logger:        l,
		simulateError: simulateError,
	}
}
