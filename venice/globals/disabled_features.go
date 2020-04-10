package globals

// disabledFeatureType is type for disabled feature list
type disabledFeatureType uint16

// list of disabled features; to help pull a branch for upcoming releases with desired list of features.
var disabledFeatures []disabledFeatureType

const (
	// StatsBasedAlerts feature
	StatsBasedAlerts disabledFeatureType = iota
)

func init() {
	disabledFeatures = []disabledFeatureType{
		// uncomment below line while pulling a branch for rel. B
		// StatsBasedAlerts,
	}
}

// IsFeatureDisabled returns true if the given feature is disabled, otherwise false.
func IsFeatureDisabled(feature disabledFeatureType) bool {
	for _, dFeature := range disabledFeatures {
		if dFeature == feature {
			return true
		}
	}
	return false
}
