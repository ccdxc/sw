package bootstrapper

import (
	"errors"
	"fmt"
	"reflect"
	"testing"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"

	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestFeatureRegistration(t *testing.T) {
	tests := []struct {
		name string
		id   FeatureID
		cb   FeatureCb
		err  error
	}{
		{
			name: "successful feature registration",
			id:   Mock,
			cb: func(name, apiServer string, rslvr resolver.Interface, l log.Logger) (Feature, error) {
				return NewMockFeature(), nil
			},
			err: nil,
		},
		{
			name: "error in feature registration",
			id:   FeatureID(2),
			cb: func(name, apiServer string, rslvr resolver.Interface, l log.Logger) (Feature, error) {
				return nil, errors.New("feature callback failed")
			},
			err: ErrFeatureRegistration,
		},
	}

	for _, test := range tests {
		bs := &bootstrapper{
			featureCbs: make(map[FeatureID]FeatureCb),
			features:   make(map[FeatureID]Feature),
		}
		bs.RegisterFeatureCb(test.id, test.cb)
		logConfig := log.GetDefaultConfig("TestBootstrapper")
		logger := log.GetNewLogger(logConfig)
		err := bs.CompleteRegistration("", "", nil, logger)
		Assert(t, reflect.DeepEqual(err, test.err), fmt.Sprintf("[%s] test failed", test.name))
	}
}

func TestBootstrapper(t *testing.T) {
	bs := GetBootstrapper()
	bs.RegisterFeatureCb(Mock, func(name, apiServer string, rslvr resolver.Interface, l log.Logger) (Feature, error) {
		return NewMockFeature(), nil
	})
	logConfig := log.GetDefaultConfig("TestBootstrapper")
	logger := log.GetNewLogger(logConfig)
	err := bs.CompleteRegistration("", "", nil, logger)
	AssertOk(t, err, "error completing bootstrapper registration")
	ok, err := bs.IsBootstrapped(globals.DefaultTenant, Mock)
	AssertOk(t, err, "IsBootstrapped: unexpected error")
	Assert(t, ok, "IsBootstrapped: expected true, got false")
	ok, err = bs.IsBootstrapped("testTenant", Mock)
	AssertOk(t, err, "IsBootstrapped: unexpected error")
	Assert(t, !ok, "IsBootstrapped: expected false, got true")
	ok, err = bs.IsBootstrapped(globals.DefaultTenant, FeatureID(3))
	Assert(t, reflect.DeepEqual(err, ErrFeatureNotFound), fmt.Sprintf("IsBootstrapped: unexpected error: %v", err))

	ok, err = bs.IsFlagSet(globals.DefaultTenant, Mock)
	AssertOk(t, err, "IsFlagSet: unexpected error")
	Assert(t, ok, "IsFlagSet: expected true, got false")
	ok, err = bs.IsFlagSet("testTenant", Mock)
	AssertOk(t, err, "IsFlagSet: unexpected error")
	Assert(t, !ok, "IsFlagSet: expected false, got true")
	ok, err = bs.IsFlagSet(globals.DefaultTenant, FeatureID(3))
	Assert(t, reflect.DeepEqual(err, ErrFeatureNotFound), fmt.Sprintf("IsFlagSet: unexpected error: %v", err))
}
