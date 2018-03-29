package ref

import (
	"encoding/json"
	"errors"
	"reflect"
	"strings"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/elastic"

	"github.com/pensando/sw/venice/utils/log"
)

const (
	indentation = "    "
)

// ElasticConfig contains the settings and mappings
// definition for an Elastic Index.
type ElasticConfig struct {

	// Settings contains elastic index settings
	// https://www.elastic.co/guide/en/elasticsearch/reference/current/indices-create-index.html#create-index-settings
	Settings Settings `json:"settings"`

	// Mappings contains data mapping types for document type
	// https://www.elastic.co/guide/en/elasticsearch/reference/current/indices-put-mapping.html
	Mappings ElasticMapping `json:"mappings"`
}

// ElasticMapping is generic container for json key:value pairs
// and can be used recursively to defined nested definitions
type ElasticMapping map[string]interface{}

// Settings contains shards and replicas settings
// And more settings if required should be added here.
type Settings struct {

	// Shards is count of Shards needed
	Shards uint `json:"number_of_shards"`

	// Replicas is count of secondary replicas
	Replicas uint `json:"number_of_replicas"`
}

// Properties contains the mapping of all fields in
// a specific go struct
type Properties struct {

	// ElasticMapping is map of key-value pairs
	ElasticMapping `json:"properties"`
}

// FieldOrTypeOverride is map of special types per docType
// based on our usecase to support aggregations and mapping
// for non-primitive data types
// This is organized per docType/category to accomadate potential
// conflicts in field-name to type mapping.
var FieldOrTypeOverride = map[string]map[string]string{

	// Config DocType
	elastic.GetDocType(globals.Configs): {
		// fields that need aggregations should map to keyword
		"tenant": "keyword",
		"kind":   "keyword",

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

		// special types mapping
		"Timestamp": "date",
	},
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
func ElasticMapper(obj interface{}, docType string, shards, replicas uint) (ElasticConfig, error) {

	log.Debugf("Object: %+v docType: %s shards: %d replicas: %s",
		obj, docType, shards, replicas)

	// Value of object has to be valid
	val := reflect.ValueOf(obj)
	if val.IsValid() == false {
		return ElasticConfig{}, errors.New("Invalid object")
	}

	// Fill in index settings
	settings := Settings{
		Shards:   shards,
		Replicas: replicas,
	}

	// Generate mappings for Object
	configs := ElasticMapping{}
	mapper(docType, val.Type().String(), val, configs, "--", true, true)

	// Fill in mappings for docType
	mappings := ElasticMapping{}
	mappings[docType] = configs

	// Fill in complete index mapping
	indexMapping := ElasticConfig{
		Settings: settings,
		Mappings: mappings,
	}

	log.Debugf("Elastic Index Mapping: %+v", indexMapping)
	return indexMapping, nil
}

// mapper is a helper function to generate mapping from golang types to elastic type.
// TODO: Remove debug logs
func mapper(docType, key string, val reflect.Value, config map[string]interface{}, indent string, outer, inline bool) {

	log.Debugf("%s mapper configmap: %v N:%v T:%v K:%v concrete-value: %v",
		indent, config, key, val.Type().Name(), val.Kind(), val.Interface())

	// check for override by field name
	if kind, ok := FieldOrTypeOverride[docType][key]; ok {
		eType := ElasticMapping{
			"type": kind,
		}
		config[key] = eType
		return
	}

	// check for override by type
	if kind, ok := FieldOrTypeOverride[docType][val.Type().Name()]; ok {
		eType := ElasticMapping{
			"type": kind,
		}
		config[key] = eType
		return
	}

	switch val.Kind() {
	case reflect.Struct:

		sMap := make(map[string]interface{})
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

			mapper(docType, fieldName, f, sMap, indent+"--", false, fieldInline)
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
			properties := ElasticMapping{
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
		keyType := ElasticMapping{
			"type": GetElasticType(mapKey.Kind()),
		}
		mMap[string("key")] = keyType
		log.Debugf("%s %s: %s", indent, mapKey.String(), mMap[mapKey.String()])

		// Value field and type
		eType := ElasticMapping{
			"type": GetElasticType(mapElem.Kind()),
		}
		mMap[string("value")] = eType
		log.Debugf("%s %s: %s", indent, "value", mMap[mapElem.String()])

		properties := ElasticMapping{
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
		mapper(docType, key, pval, config, indent, false, false)

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
		eType := ElasticMapping{
			"type": GetElasticType(val.Kind()),
		}
		config[key] = eType

	case reflect.Slice:

		// To generate mapping for slice, reflect on one of
		// its elements.
		log.Debugf("%s %s: %s len: %d", indent, key,
			GetElasticType(val.Kind()), val.Len())
		if val.Len() > 0 {
			mapper(docType, key, val.Index(0), config, indent, false, false)
		} else {
			log.Warn("Unable to generate mapping for empty slice")
		}

	default:
		config[key] = "text"
		log.Debugf("Not supported yet, mapping to string as default")
	}

	log.Debugf("%s mapper configmap: %v", indent, config)
}

// JSONString generates the JSON string for the object
func (e ElasticConfig) JSONString() (string, error) {

	jd, err := json.Marshal(e)
	if err != nil {
		log.Errorf("Failed to marshal: %v into json, err: %v", e, err)
		return "", err
	}
	return string(jd), nil
}

// JSONPrettyString generates the JSON string in pretty string for the object
func (e ElasticConfig) JSONPrettyString() (string, error) {

	jd, err := json.MarshalIndent(e, "", indentation)
	if err != nil {
		log.Errorf("Failed to marshal: %v into json, err: %v", e, err)
		return "", err
	}
	return string(jd), nil
}
