package bootstrapper

import (
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

// FeatureID identifies feature that is checked for being bootstrapped
type FeatureID int32

const (
	// Auth feature id
	Auth FeatureID = iota
)

// Feature is a component/feature that needs to be checked if it has been bootstrapped
type Feature interface {
	// IsBootstrapped checks if the feature has been bootstrapped i.e. required API server objects have been created
	IsBootstrapped(tenant string) bool
	// IsFlagSet checks if a feature bootstrap flag has been set in Cluster for a tenant
	IsFlagSet(tenant string) (bool, error)
	// ID returns feature id
	ID() FeatureID
}

// Bootstrapper is used to check if a registered feature has been bootstrapped, for example "Auth"
type Bootstrapper interface {
	// IsBootstrapped checks if a feature has been bootstrapped in a tenant
	IsBootstrapped(tenant string, id FeatureID) (bool, error)
	// IsFlagSet checks if a feature bootstrap flag has been set in Cluster for a tenant
	IsFlagSet(tenant string, id FeatureID) (bool, error)
	// RegisterFeatureCb registers a feature registration callback to the bootstrapper
	RegisterFeatureCb(id FeatureID, cb FeatureCb)
	// CompleteRegistration calls registered feature callback to complete the initialization of feature and bootstrapper
	CompleteRegistration(name, apiServer string, rslvr resolver.Interface, l log.Logger) error
}

// FeatureCb is a feature registration callback registered to and called by Bootstrapper
type FeatureCb func(name, apiServer string, rslvr resolver.Interface, l log.Logger) (Feature, error)
