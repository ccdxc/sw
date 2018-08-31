// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package elastic

import (
	"encoding/json"
	"time"

	"github.com/pensando/sw/venice/utils/log"
)

const (
	// Index represents the index request; used in bulk requests
	Index string = "index"

	// Update represents the update request; used in bulk requests
	Update string = "update"

	// Delete represents the delete request; used in bulk requests
	Delete string = "delete"

	indentation = "    "
)

// BulkRequest represents each request in the bulk operation
type BulkRequest struct {
	RequestType string      // type of the request. e.g. index, update, delete, etc.
	Index       string      // name of the index. e.g. events
	IndexType   string      // type within in the above index. e.g. event
	Obj         interface{} // obj(doc) to be indexed
	ID          string      // ID to be used while indexing the doc; this prevents ES from generating an ID
}

// Config contains the settings and mappings
// definition for an Elastic Index.
type Config struct {
	// IndexPatterns contains elastic index patterns for the template
	// https://www.elastic.co/guide/en/elasticsearch/reference/current/indices-templates.html
	IndexPatterns string `json:"index_patterns,omitempty"`

	// Settings contains elastic index settings
	// https://www.elastic.co/guide/en/elasticsearch/reference/current/indices-create-index.html#create-index-settings
	Settings SettingsConfig `json:"settings"`

	// Mappings contains data mapping types for document type
	// https://www.elastic.co/guide/en/elasticsearch/reference/current/indices-put-mapping.html
	Mappings Mapping `json:"mappings"`
}

// Mapping is generic container for json key:value pairs
// and can be used recursively to define nested definitions
type Mapping map[string]interface{}

// SettingsConfig contains shards and replicas settings
// And more settings if required should be added here.
type SettingsConfig struct {

	// Shards is count of Shards needed
	Shards uint `json:"number_of_shards"`

	// Replicas is count of secondary replicas
	Replicas uint `json:"number_of_replicas"`

	// Codec compression config
	Codec string `json:"codec"`

	// Max number of inner results with Aggregation, default is 100.
	MaxInnerResults uint `json:"max_inner_result_window"`
}

// Properties contains the mapping of all fields in
// a specific go struct
type Properties struct {

	// Mapping is map of key-value pairs
	Mapping `json:"properties"`
}

// JSONString generates the JSON string for the object
func (e Config) JSONString() (string, error) {

	jd, err := json.Marshal(e)
	if err != nil {
		log.Errorf("Failed to marshal: %v into json, err: %v", e, err)
		return "", err
	}
	return string(jd), nil
}

// JSONPrettyString generates the JSON string in pretty string for the object
func (e Config) JSONPrettyString() (string, error) {

	jd, err := json.MarshalIndent(e, "", indentation)
	if err != nil {
		log.Errorf("Failed to marshal: %v into json, err: %v", e, err)
		return "", err
	}
	return string(jd), nil
}

// SettingsResponse contains the index settings response
// Generated with help of https://mholt.github.io/json-to-go/
type SettingsResponse struct {
	Codec                string    `json:"codec"`
	NumberOfShards       uint64    `json:"number_of_shards"`
	ProvidedName         string    `json:"provided_name"`
	MaxInnerResultWindow uint64    `json:"max_inner_result_window"`
	CreationDate         time.Time `json:"creation_date"`
	NumberOfReplicas     uint64    `json:"number_of_replicas"`
	UUID                 string    `json:"uuid"`
	Version              struct {
		Created string `json:"created"`
	} `json:"version"`
}
