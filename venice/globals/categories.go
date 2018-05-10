// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package globals

// Kind2Category is map of Kind to its Category (api-group)
// TODO: remove this once we have auto-generated mapping from api-server
var Kind2Category = map[string]string{
	"Cluster":                 "Cluster",
	"Node":                    "Cluster",
	"SmartNIC":                "Cluster",
	"Rollout":                 "Cluster",
	"Tenant":                  "Cluster",
	"Endpoint":                "Workload",
	"SecurityGroup":           "Security",
	"Sgpolicy":                "Security",
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
