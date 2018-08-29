// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package globals

const (

	// CategoryLabel is used to add category attribute to
	// objects indexed in elastic. This is needed to enable
	// aggregation by category attribute
	CategoryLabel = "_category"
)

// Kind2Category is map of Kind to its Category (api-group)
// TODO: remove this once we have auto-generated mapping from api-server
var Kind2Category = map[string]string{
	"Cluster":                 "Cluster",
	"Node":                    "Cluster",
	"Host":                    "Cluster",
	"SmartNIC":                "Cluster",
	"Rollout":                 "Cluster",
	"Tenant":                  "Cluster",
	"Endpoint":                "Workload",
	"Workload":                "Workload",
	"SecurityGroup":           "Security",
	"SGPolicy":                "Security",
	"App":                     "Security",
	"AppUser":                 "Security",
	"AppUserGrp":              "Security",
	"Certificate":             "Security",
	"TrafficEncryptionPolicy": "Security",
	"User":                 "Auth",
	"AuthenticationPolicy": "Auth",
	"Role":                 "Auth",
	"RoleBinding":          "Auth",
	"Network":              "Network",
	"Service":              "Network",
	"LbPolicy":             "Network",
	"Alert":                "Monitoring",
	"AlertDestination":     "Monitoring",
	"AlertPolicy":          "Monitoring",
	"Event":                "Monitoring",
	"EventPolicy":          "Monitoring",
	"StatsPolicy":          "Monitoring",
	"FlowExportPolicy":     "Monitoring",
	"FwlogPolicy":          "Monitoring",
	"MirrorSession":        "Monitoring",
}
