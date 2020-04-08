package featureflags

import (
	"context"
	"fmt"
	"sync"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

type featureFlags struct {
	sync.RWMutex
	overlayRouting             bool
	networkLevelSecurityPolicy bool
	appSecurity                bool
}

// List of feature flag keys
const (
	OverlayRouting         = "OverlayRouting"
	SubnetSecurityPolicies = "SubnetSecurityPolicies"
	SecurityALG            = "SecurityALG"
)

// currentFeatures is the current set of internal feature flags and their values
//  initialized to defaults for the system.
var currentFeatures = featureFlags{
	overlayRouting:             false,
	networkLevelSecurityPolicy: false,
	appSecurity:                true,
}

var initialized bool

func copyFlags(from *featureFlags, to *featureFlags) {
	to.overlayRouting = from.overlayRouting
	to.networkLevelSecurityPolicy = from.networkLevelSecurityPolicy
	to.appSecurity = from.appSecurity
}

// Validate validates the features flags passed in and returns a status will effective features.
func Validate(in []cluster.Feature) (cluster.LicenseStatus, []error) {
	ret := cluster.LicenseStatus{}
	var reterrs []error
	fmap := make(map[string]bool)
	log.Infof("Received validate: %s", in)
	for _, v := range in {
		switch v.FeatureKey {
		case OverlayRouting, SubnetSecurityPolicies, SecurityALG:
			if _, ok := fmap[v.FeatureKey]; ok {
				reterrs = append(reterrs, fmt.Errorf("duplicate feature in request [%v]", v.FeatureKey))
			} else {
				log.Infof("enabling feature [%+v]", v)
				ret.Features = append(ret.Features, cluster.FeatureStatus{FeatureKey: v.FeatureKey, Value: "enabled", Expiry: "never"})
				fmap[v.FeatureKey] = true
			}
		default:
			// for compatibility reasons these errors are only reported as unknowns but do not fail the API.
			log.Errorf("Unknown feature flag encountered [%v]", v)
			ret.Unknown = append(ret.Unknown, v.FeatureKey)
		}
	}
	return ret, reterrs
}

// Update updates the digested internal flags from feature flags enabled.
func Update(in []cluster.Feature) []error {
	updFeatures := featureFlags{}
	fmap := make(map[string]bool)
	var reterrs []error
	log.Infof("Recived update: %v", in)
	for _, v := range in {
		switch v.FeatureKey {
		case OverlayRouting:
			if _, ok := fmap[v.FeatureKey]; ok {
				reterrs = append(reterrs, fmt.Errorf("duplicate feature in request [%v]", v.FeatureKey))
			} else {
				fmap[v.FeatureKey] = true
				updFeatures.overlayRouting = true
				log.Infof("Enabling overlay Routing")
			}

		case SubnetSecurityPolicies:
			if _, ok := fmap[v.FeatureKey]; ok {
				reterrs = append(reterrs, fmt.Errorf("duplicate feature in request [%v]", v.FeatureKey))
			} else {
				fmap[v.FeatureKey] = true
				updFeatures.networkLevelSecurityPolicy = true
				log.Infof("Enabling subnet security policies")
			}

		case SecurityALG:
			if _, ok := fmap[v.FeatureKey]; ok {
				reterrs = append(reterrs, fmt.Errorf("duplicate feature in request [%v]", v.FeatureKey))
			} else {
				fmap[v.FeatureKey] = true
				updFeatures.appSecurity = true
				log.Infof("Enabling security ALG")
			}

		default:
			// for compatibility reasons these errors are only reported as unknowns but do not fail the API.
			log.Errorf("Unknown feature flag encountered [%v]", v)
		}
	}
	currentFeatures.Lock()
	copyFlags(&updFeatures, &currentFeatures)
	currentFeatures.Unlock()
	log.Infof("Done updating feature flags")
	return reterrs
}

// Initialize tries to read the Feature flags from the API server.
func Initialize(service string, apiSrvURL string, rslvr resolver.Interface) {
	log.Infof("Initialize received service: %s | apisrvUrl: %s", service, apiSrvURL)
	if initialized {
		log.Infof("Flags already initialized")
		return
	}
	var apiClientDone bool
	var apicl apiclient.Services
	var err error
	for {
		if apiClientDone == false {
			apicl, err = apiclient.NewGrpcAPIClient(service, apiSrvURL, log.GetDefaultInstance(), rpckit.WithBalancer(balancer.New(rslvr)))
		}
		if err == nil {
			apiClientDone = true
			defer apicl.Close()
			cc, err := apicl.ClusterV1().Cluster().Get(context.TODO(), &api.ObjectMeta{})
			if err == nil {
				if cc.Status.AuthBootstrapped {
					ff, err := apicl.ClusterV1().License().Get(context.TODO(), &api.ObjectMeta{})
					if err != nil {
						// no feature flag available. Continue with defaults
						log.Infof("No feature flags available, continuing with defaults (%s)", err)
						return
					}
					SetFeatures(ff.Spec.Features)
					return
				}
			}
		}
		time.Sleep(time.Second)
	}
}

// SetFeatures is a helper function to set the feature flags
func SetFeatures(in []cluster.Feature) {
	log.Infof("Received: %v", in)
	Update(in)
	SetInitialized()
	log.Infof("feature flags updated [%v]", in)
}

// SetInitialized is used for testing purposes only, so initialization by reading apiserver object can be skipped.
func SetInitialized() {
	log.Infof("Setting feature flags initialized")
	initialized = true
}

// IsOVerlayRoutingEnabled returns true if overlay routing feature is enabled.
func IsOVerlayRoutingEnabled() bool {
	defer currentFeatures.RUnlock()
	currentFeatures.RLock()
	return currentFeatures.overlayRouting
}

// IsNetworkSecPolicyEnabled returns true if Network level security policies are allowed.
func IsNetworkSecPolicyEnabled() bool {
	defer currentFeatures.RUnlock()
	currentFeatures.RLock()
	return currentFeatures.networkLevelSecurityPolicy
}

// AreALGsEnabled returns trued if ALGs are allowed in security policy rules.
func AreALGsEnabled() bool {
	defer currentFeatures.RUnlock()
	currentFeatures.RLock()
	return currentFeatures.appSecurity
}

// IsMultiTenantEnabled returns true of multi-tenancy is allowed on the cluster
func IsMultiTenantEnabled() bool {
	defer currentFeatures.RUnlock()
	currentFeatures.RLock()
	return currentFeatures.overlayRouting
}
