// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package ref

import (
	"errors"
	"reflect"
	"strings"
	"time"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/log"
)

// Elastic mapper options
type options struct {
	shards          uint                   // Shard count
	replicas        uint                   // Replica count
	codec           string                 // Codec compression scheme
	maxInnerResults uint                   // Max inner results
	indexPatterns   string                 // index pattern for the template
	textAnalysis    map[string]interface{} // character filter to be used in the index
}

// Option fills the optional params for Mapper
type Option func(opt *options)

// fieldOrTypeOverride is map of special types per docType
// based on our use case to support aggregations and mapping
// for non-primitive data types
// This is organized per docType/category to accommodate potential
// conflicts in field-name to type mapping.
var fieldOrTypeOverride = map[string]map[string]string{

	// Config DocType
	elastic.GetDocType(globals.Configs): {
		// fields that need aggregations should map to keyword
		"tenant":  "keyword",
		"kind":    "keyword",
		"name":    "keyword",
		"id":      "keyword", // dsc.spec.id
		"version": "keyword", // desc.status.version
		"host":    "keyword", // desc.status.host

		// special types mapping
		"Timestamp": "date",
	},
	// Events DocType
	elastic.GetDocType(globals.Events): {
		// fields that need aggregations should map to keyword
		"tenant":    "keyword",
		"kind":      "keyword",
		"severity":  "keyword",
		"type":      "keyword",
		"component": "keyword",
		"name":      "keyword",
		"category":  "keyword",

		// special types mapping
		"Timestamp": "date",
	},
	// AuditLogs DocType
	elastic.GetDocType(globals.AuditLogs): {
		// fields that need aggregations should map to keyword
		"tenant":      "keyword",
		"kind":        "keyword",
		"stage":       "keyword",
		"action":      "keyword",
		"outcome":     "keyword",
		"name":        "keyword",
		"external-id": "keyword",

		// special types mapping
		"Timestamp": "date",
	},
	// Fwlogs DocType
	elastic.GetDocType(globals.FwLogs): {
		// special types mapping
		"Timestamp": "date",
	},
}

// Fields that don not indexing should be listed here.
// Please read "https://www.elastic.co/guide/en/elasticsearch/reference/6.3/enabled.html"
// for understanding how it works & gets configured in elastic.
var isIndexingDisabled = map[string]map[string]struct{}{
	elastic.GetDocType(globals.FwLogs): {
		"rule-id":         struct{}{},
		"session-id":      struct{}{},
		"flow-action":     struct{}{},
		"icmp-type":       struct{}{},
		"icmp-id":         struct{}{},
		"icmp-code":       struct{}{},
		"time":            struct{}{},
		"direction":       struct{}{},
		"destination-vrf": struct{}{},
		"source-vrf":      struct{}{},
		"policy-name":     struct{}{},
		"kind":            struct{}{},
		"api-version":     struct{}{},
		"mod-time":        struct{}{},
		"namespace":       struct{}{},
		"uuid":            struct{}{},
		"generation-id":   struct{}{},
		"name":            struct{}{},
		"tenant":          struct{}{},
		"labels":          struct{}{},
		"self-link":       struct{}{},
	},
	elastic.GetDocType(globals.FwLogsObjects): {
		"logscount": struct{}{},
		"startts":   struct{}{},
		"endts":     struct{}{},
		"bucket":    struct{}{},
	},
}

func defaultOptions() *options {
	return &options{
		shards:          3,
		replicas:        2,
		codec:           "best_compression",
		maxInnerResults: 256,
	}
}

// WithShardCount specifies the shard count
func WithShardCount(shards uint) Option {
	return func(o *options) {
		o.shards = shards
	}
}

// WithReplicaCount specifies the replica count
func WithReplicaCount(replicas uint) Option {
	return func(o *options) {
		o.replicas = replicas
	}
}

// WithCodec specifies the compression algorithm
func WithCodec(codec string) Option {
	return func(o *options) {
		o.codec = codec
	}
}

// WithMaxInnerResults specifies the max number of inner results
// supported with aggregations in query
func WithMaxInnerResults(maxResults uint) Option {
	return func(o *options) {
		o.maxInnerResults = maxResults
	}
}

// WithIndexPatterns specifies index pattern for the template
func WithIndexPatterns(pattern string) Option {
	return func(o *options) {
		o.indexPatterns = pattern
	}
}

// WithCharFilter character filter to replace "-" to "_"
func WithCharFilter() Option {
	return func(o *options) {
		charFilter := make(elastic.Mapping)
		charFilter["type"] = "mapping"
		charFilter["mappings"] = []string{
			"- => _",
		}

		analyzer := make(elastic.Mapping)
		analyzer["tokenizer"] = "standard"
		analyzer["char_filter"] = []string{"t_char_filter"}
		analyzer["filter"] = []string{"lowercase"}

		analysis := make(elastic.Mapping)
		analysis["char_filter"] = elastic.Mapping{"t_char_filter": charFilter}
		analysis["analyzer"] = elastic.Mapping{"t_analyzer": analyzer}

		o.textAnalysis = analysis
	}
}

// GetElasticType returns the mapping to go data type
// to elastic data types listed below.
//
//	string
//		- text or keyword
//	Numeric datatypes
//		- long, integer, short, byte, double, float, half_float, scaled_float
//	Date datatype
//		- date
//	Boolean datatype
//		- boolean
func GetElasticType(kind reflect.Kind) string {

	switch kind {
	case reflect.String:
		return "text"
	case reflect.Bool:
		return "boolean"
	case reflect.Int:
		fallthrough
	case reflect.Uint:
		return "long"
	case reflect.Int8:
		fallthrough
	case reflect.Uint8:
		return "char"
	case reflect.Int16:
		fallthrough
	case reflect.Uint16:
		return "short"
	case reflect.Int32:
		fallthrough
	case reflect.Uint32:
		return "integer"
	case reflect.Int64:
		fallthrough
	case reflect.Uint64:
		return "long"
	default:
		return "text"
	}
}

// ElasticMapper generates the index mapping
// for a given object and settings for an associated
// docType
// Usage example:
// 		config := ElasticMapper(events.Event{},
//                              elastic.GetDocType(globals.Events), 1, 0)
// To get JSON string output:
//  	str, err = config.JSONString()
//              (or)
//  	str, err = config.JSONPrettyString()
func ElasticMapper(obj interface{}, docType string, opts ...Option) (elastic.Config, error) {

	options := *defaultOptions()

	// add custom options
	for _, o := range opts {
		if o != nil {
			o(&options)
		}
	}

	log.Debugf("Object: %+v docType: %s options: %+v",
		obj, docType, options)

	// Value of object has to be valid
	val := reflect.ValueOf(obj)
	if val.IsValid() == false {
		return elastic.Config{}, errors.New("Invalid object")
	}

	// Fill in index settings
	settings := elastic.SettingsConfig{
		Shards:          options.shards,
		Replicas:        options.replicas,
		Codec:           options.codec,
		MaxInnerResults: options.maxInnerResults,
	}

	// add character filter if required
	if options.textAnalysis != nil {
		settings.Analysis = options.textAnalysis
	}

	// Generate mappings for Object
	configs := elastic.Mapping{}
	mapper(docType, val.Type().String(), val, configs, "--", true, true, &options)

	// Fill in mappings for docType
	mappings := elastic.Mapping{}
	mappings[docType] = configs

	// Fill in complete index mapping
	indexMapping := elastic.Config{
		IndexPatterns: options.indexPatterns,
		Settings:      settings,
		Mappings:      mappings,
	}

	log.Debugf("Elastic Index Mapping: %+v", indexMapping)
	return indexMapping, nil
}

// mapper is a helper function to generate mapping from golang types to elastic type.
// TODO: Remove debug logs
func mapper(docType, key string, val reflect.Value, config map[string]interface{}, indent string, outer, inline bool,
	opts *options) {

	log.Debugf("%s mapper configmap: %v N:%v T:%v K:%v concrete-value: %v",
		indent, config, key, val.Type().Name(), val.Kind(), val.Interface())

	// check using feild name whether indexing is disabled on the field
	if _, ok := isIndexingDisabled[docType][key]; ok {
		eType := elastic.Mapping{
			"enabled": false,
		}
		config[key] = eType
		return
	}

	// check for override by field name
	if kind, ok := fieldOrTypeOverride[docType][key]; ok {
		if kind == "keyword" {

			// Generate both text and keyword mapping
			kwMap := make(map[string]interface{})
			kwMap[string("type")] = "keyword"
			kwMap[string("ignore_above")] = 256
			kw := elastic.Mapping{
				"keyword": kwMap,
			}

			cMap := make(map[string]interface{})
			cMap[string("type")] = "text"
			cMap[string("fields")] = kw

			if opts.textAnalysis != nil {
				if analyzer, ok := opts.textAnalysis["analyzer"]; ok {
					textAnalyzer := analyzer.(elastic.Mapping)
					if _, ok := textAnalyzer["t_analyzer"]; ok {
						cMap[string("analyzer")] = "t_analyzer"
					}
				}
			}
			config[key] = cMap

		} else {
			eType := elastic.Mapping{
				"type": kind,
			}
			config[key] = eType
		}
		return
	}

	// check for override by type
	if kind, ok := fieldOrTypeOverride[docType][val.Type().Name()]; ok {
		eType := elastic.Mapping{
			"type": kind,
		}
		config[key] = eType
		return
	}

	switch val.Kind() {
	case reflect.Struct:

		if val.Type() == reflect.ValueOf(time.Now()).Type() {
			eType := elastic.Mapping{
				"type": "date",
			}
			config[key] = eType
			return
		}

		sMap := make(map[string]interface{})
		// Do not nest into the time.Time struct
		for i := 0; i < val.Type().NumField(); i++ {
			f := val.Field(i)
			log.Debugf("%s Field#%d: N:%v T:%v K:%v = %+v\n",
				indent,
				i,
				val.Type().Field(i).Name,
				f.Type().Name(),
				f.Kind(),
				f)

			var fieldName string
			fieldInline := false
			jsonTags := strings.Split(val.Type().Field(i).Tag.Get("json"), ",")

			// find if the attribute is inline
			for _, tag := range jsonTags {
				if strings.Contains(tag, string("inline")) == true {
					fieldInline = true
				}
			}

			// get json field name
			if len(jsonTags) > 0 && jsonTags[0] != "" {
				fieldName = jsonTags[0]
			} else {
				fieldName = val.Type().Field(i).Name
			}

			mapper(docType, fieldName, f, sMap, indent+"--", false, fieldInline, opts)
		}

		if inline == true {
			if outer == true {
				config["properties"] = sMap
			} else {
				for k, v := range sMap {
					config[k] = v
				}
			}
		} else {
			// include struct name with properties nested
			properties := elastic.Mapping{
				"properties": sMap,
			}
			config[key] = properties
		}

	case reflect.Map:
		indent = indent + "--"

		mapKey := val.Type().Key()
		mapElem := val.Type().Elem()
		log.Debugf("%s MAP: Key-Type: %s Value-Type: %s",
			indent,
			mapKey.String(),
			mapElem.String())

		mMap := make(map[string]interface{})

		// Key field and type
		keyType := elastic.Mapping{
			"type": GetElasticType(mapKey.Kind()),
		}
		mMap[string("key")] = keyType
		log.Debugf("%s %s: %s", indent, mapKey.String(), mMap[mapKey.String()])

		// Value field and type
		eType := elastic.Mapping{
			"type": GetElasticType(mapElem.Kind()),
		}
		mMap[string("value")] = eType
		log.Debugf("%s %s: %s", indent, "value", mMap[mapElem.String()])

		properties := elastic.Mapping{
			"properties": mMap,
		}
		config[key] = properties

	case reflect.Ptr:

		pval := val.Elem()
		if !pval.IsValid() {
			log.Debugf("%s Invalid ptr", indent)
			return
		}
		log.Debugf("%s Ptr %s: %s", indent, pval.Type(), pval.String())
		mapper(docType, key, pval, config, indent, false, false, opts)

	case reflect.String:
		fallthrough
	case reflect.Bool:
		fallthrough
	case reflect.Int:
		fallthrough
	case reflect.Int8:
		fallthrough
	case reflect.Int16:
		fallthrough
	case reflect.Int32:
		fallthrough
	case reflect.Uint:
		fallthrough
	case reflect.Uint8:
		fallthrough
	case reflect.Uint16:
		fallthrough
	case reflect.Uint32:
		fallthrough
	case reflect.Uint64:
		log.Debugf("%s %s: %s", indent, key, GetElasticType(val.Kind()))
		eType := elastic.Mapping{
			"type": GetElasticType(val.Kind()),
		}
		config[key] = eType

	case reflect.Slice:

		// To generate mapping for slice, reflect on one of
		// its elements.
		log.Debugf("%s %s: %s len: %d", indent, key,
			GetElasticType(val.Kind()), val.Len())
		if val.Len() > 0 {
			mapper(docType, key, val.Index(0), config, indent, false, false, opts)
		} else {
			log.Warn("Unable to generate mapping for empty slice")
		}

	default:
		config[key] = "text"
		log.Debugf("Not supported yet, mapping to string as default")
	}

	log.Debugf("%s mapper configmap: %v", indent, config)
}
