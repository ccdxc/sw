// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package elastic

import (
	"fmt"
	"time"

	"github.com/pensando/sw/venice/globals"
)

const (

	// ExternalIndexPrefix is the elastic index prefix
	// for user visible data
	ExternalIndexPrefix = "venice.external"

	// InternalIndexPrefix is the elastic index prefix
	// for internal data
	InternalIndexPrefix = "venice.internal"

	// TenantAggKey is the Aggregation key to group by Tenant
	TenantAggKey = "tenant_agg"

	// KindAggKey is the Aggregation key to group by Kind
	KindAggKey = "kind_agg"

	// TopHitsKey is the Aggregation key for top search hits
	TopHitsKey = "top_agg"
)

// GetIndex returns the Elastic Index based on the data type & tenant name
//
// - The index is of the following format
//   <venice>.<external/internal>.<tenant>.<datatype>.<YYYY-MM-DD>
// - Having the convention above facilitates multi-index search using common prefix
// - Indicies marked external are subject to search scope by Spyglass service and
//   and the user visibility is bounded by RBAC rules.
// - The index marked internal is not user visible and is meant for internal debugging.
// - Indices with tenant name embedded is used for DataTypes that requires unique
//   retention policy per tenant and hence different indices are needed.
func GetIndex(dtype globals.DataType, tenant string) string {

	currentDay := time.Now().Local().Format("2006-01-02")

	switch dtype {
	case globals.Configs:
		return fmt.Sprintf("%s.%s.%s.%s", ExternalIndexPrefix, tenant, GetDocType(dtype), currentDay)
	case globals.Alerts:
		return fmt.Sprintf("%s.%s.%s.%s", ExternalIndexPrefix, tenant, GetDocType(dtype), currentDay)
	case globals.Events:
		return fmt.Sprintf("%s.%s.%s.%s", ExternalIndexPrefix, tenant, GetDocType(dtype), currentDay)
	case globals.AuditLogs:
		return fmt.Sprintf("%s.%s.%s.%s", ExternalIndexPrefix, tenant, GetDocType(dtype), currentDay)
	case globals.DebugLogs:
		return fmt.Sprintf("%s.%s.%s.%s", InternalIndexPrefix, tenant, GetDocType(dtype), currentDay)
	case globals.Stats:
		return "N/A"
	}

	return ""
}

// GetDocType returns the string-enum of DataType.
// This is intended to used as a document type
// in ElasticDB.
func GetDocType(dtype globals.DataType) string {

	switch dtype {
	case globals.Configs:
		return "configs"
	case globals.Alerts:
		return "alerts"
	case globals.Events:
		return "events"
	case globals.AuditLogs:
		return "auditlogs"
	case globals.DebugLogs:
		return "debuglogs"
	case globals.Stats:
		return "N/A"
	}

	return ""
}
