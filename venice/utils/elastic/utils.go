// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package elastic

import (
	"fmt"
	"time"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
)

const (

	// ExternalIndexPrefix is the elastic index prefix
	// for user visible data
	ExternalIndexPrefix = "venice.external"

	// InternalIndexPrefix is the elastic index prefix
	// for internal data
	InternalIndexPrefix = "venice.internal"

	// ConfigIndexPrefix is the elastic index prefix for config/policies
	ConfigIndexPrefix = "venice.external.configs.*"

	// LogIndexPrefix is the elastic index prefix for config/policies
	LogIndexPrefix = "venice.internal.default.systemlogs.*"

	// TenantAggKey is the Aggregation key to group by Tenant
	TenantAggKey = "tenant_agg"

	// CategoryAggKey is the Aggregation key to group by Category
	CategoryAggKey = "category_agg"

	// KindAggKey is the Aggregation key to group by Kind
	KindAggKey = "kind_agg"

	// TopHitsKey is the Aggregation key for top search hits
	TopHitsKey = "top_agg"

	// maxRetries maximum number of retries for fetching elasticsearch URLs
	// and creating client.
	maxRetries = 60

	// delay between retries
	retryDelay = 2 * time.Second
)

// GetIndex returns the Elastic Index based on the data type & tenant name
//
// - The index is of the following format for Alerts, Events, AuditTrail and Logs
//   with daily indexing allowing to search historical data.
//   "venice.<external/internal>.<tenant>.<datatype>.<YYYY-MM-DD>"
// - The index is of the following format for Configs. For configs search there
//   is no need for historical data and hence we don't need daily indexing.
//   "venice.external.default.configs"
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
		return fmt.Sprintf("%s.%s.%s", ExternalIndexPrefix, tenant, GetDocType(dtype))
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
		return "systemlogs"
	case globals.Stats:
		return "N/A"
	}

	return ""
}

// GetTemplateName returns the template name for the given data type
func GetTemplateName(dtype globals.DataType) string {
	switch dtype {
	case globals.Events:
		return fmt.Sprintf("%s-template", GetDocType(dtype))
	}

	return ""
}

// getElasticSearchAddrs helper function to get the elasticsearch addresses using the resolver
func getElasticSearchAddrs(resolverClient resolver.Interface) ([]string, error) {
	for i := 0; i < maxRetries; i++ {
		elasticURLs := resolverClient.GetURLs(globals.ElasticSearch)
		if len(elasticURLs) > 0 {
			log.Debugf("list of elastic URLs found %v", elasticURLs)
			return elasticURLs, nil
		}

		time.Sleep(retryDelay)
		log.Debug("couldn't find elasticsearch. retrying.")
	}

	return []string{}, fmt.Errorf("failed to get `%v` URLs using the resolver", globals.ElasticSearch)
}
