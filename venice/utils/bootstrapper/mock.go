package bootstrapper

import (
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

const (
	// Mock is mock feature id
	Mock FeatureID = 1
)

type mockFeature struct{}

func (f *mockFeature) IsBootstrapped(tenant string) bool {
	if tenant == globals.DefaultTenant {
		return true
	}
	return false
}

func (f *mockFeature) IsFlagSet(tenant string) (bool, error) {
	if tenant == globals.DefaultTenant {
		return true, nil
	}
	return false, nil
}

func (f *mockFeature) ID() FeatureID {
	return Mock
}

// NewMockFeature returns a mock feature for testing
func NewMockFeature() Feature {
	return &mockFeature{}
}

type mockBootstrapper struct {
	simulateError bool
}

func (b *mockBootstrapper) IsBootstrapped(tenant string, id FeatureID) (bool, error) {
	if b.simulateError {
		return false, ErrFeatureNotFound
	}
	return true, nil
}

func (b *mockBootstrapper) IsFlagSet(tenant string, id FeatureID) (bool, error) {
	if b.simulateError {
		return false, ErrFeatureNotFound
	}
	return true, nil
}

func (b *mockBootstrapper) RegisterFeatureCb(id FeatureID, cb FeatureCb) {
	return
}

func (b *mockBootstrapper) CompleteRegistration(name, apiServer string, rslvr resolver.Interface, l log.Logger) error {
	return nil
}

// NewMockBootstrapper return a mock Bootstrapper for testing
func NewMockBootstrapper(simulateError bool) Bootstrapper {
	return &mockBootstrapper{simulateError: simulateError}
}
