package eventattrs

// SeverityLevel_normal is a map of normalized values for the enum
var Severity_normal = map[string]string{
	"CRITICAL": "CRITICAL",
	"INFO":     "INFO",
	"WARN":     "WARN",
	"DEBUG":    "DEBUG",
	"critical": "CRITICAL",
	"info":     "INFO",
	"warn":     "WARN",
	"debug":    "DEBUG",
}

// Severity_vname is a map from value to the venice name
var Severity_vname = map[int32]string{
	0: "info",
	1: "warn",
	2: "critical",
	3: "debug",
}

// Severity_vvalue is a map from venice name to the value
var Severity_vvalue = map[string]int32{
	"info":     0,
	"warn":     1,
	"critical": 2,
	"debug":    3,
}

// Category_normal is a map of normalized values for the enum
var Category_normal = map[string]string{
	"Cluster":      "Cluster",
	"System":       "System",
	"Network":      "Network",
	"Rollout":      "Rollout",
	"Orchestrator": "Orchestrator",
	"cluster":      "Cluster",
	"system":       "System",
	"network":      "Network",
	"rollout":      "Rollout",
	"config":       "Config",
	"orchestrator": "Orchestrator",
}

// Category_vname is a map from value to the venice name
var Category_vname = map[int32]string{
	0: "cluster",
	1: "network",
	2: "system",
	3: "rollout",
	4: "config",
}

// Category_vvalue is a map from venice name to the value
var Category_vvalue = map[string]int32{
	"cluster": 0,
	"network": 1,
	"system":  2,
	"rollout": 3,
}

func (x Severity) String() string {
	return Severity_vname[int32(x)]
}

func (x Category) String() string {
	return Category_vname[int32(x)]
}
