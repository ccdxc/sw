package bootstrapper

import (
	"errors"
	"fmt"
	"sync"

	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

var (
	// ErrFeatureNotFound error is returned when a feature with a given ID is not registered
	ErrFeatureNotFound = errors.New("feature not found")
	// ErrFeatureRegistration error is returned when a feature is not successfully registered
	ErrFeatureRegistration = errors.New("unsuccessful feature registration")
)

// bootstrapper is a singleton that implements Bootstrapper interface
type bootstrapper struct {
	logger     log.Logger
	featureCbs map[FeatureID]FeatureCb
	features   map[FeatureID]Feature
}

func (b *bootstrapper) IsBootstrapped(tenant string, id FeatureID) (bool, error) {
	feature, ok := b.features[id]
	if !ok {
		b.logger.Errorf("Feature with id [%v] not found", id)
		return ok, ErrFeatureNotFound
	}
	return feature.IsBootstrapped(tenant), nil
}

func (b *bootstrapper) IsFlagSet(tenant string, id FeatureID) (bool, error) {
	feature, ok := b.features[id]
	if !ok {
		b.logger.Errorf("Feature with id [%v] not found", id)
		return ok, ErrFeatureNotFound
	}
	return feature.IsFlagSet(tenant)
}

func (b *bootstrapper) RegisterFeatureCb(id FeatureID, featureCb FeatureCb) {
	if _, ok := b.featureCbs[id]; ok {
		panic(fmt.Sprintf("Duplicate feature callback registration with ID [%v]", id))
	}
	b.featureCbs[id] = featureCb
}

func (b *bootstrapper) CompleteRegistration(name, apiServer string, rslvr resolver.Interface, l log.Logger) error {
	b.logger = l
	for id, featureCb := range b.featureCbs {
		feature, err := featureCb(name, apiServer, rslvr, l)
		if err != nil {
			b.logger.Errorf("Failed to register feature with id [%v]", id)
			return ErrFeatureRegistration
		}
		b.features[feature.ID()] = feature
	}
	return nil
}

var gBootStrapper *bootstrapper
var once sync.Once

// GetBootstrapper returns a singleton implementation of Bootstrapper
func GetBootstrapper() Bootstrapper {
	once.Do(func() {
		gBootStrapper = &bootstrapper{
			featureCbs: make(map[FeatureID]FeatureCb),
			features:   make(map[FeatureID]Feature),
		}
	})
	return gBootStrapper
}
