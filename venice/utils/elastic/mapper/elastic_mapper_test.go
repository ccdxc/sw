package ref

import (
	"errors"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/api/generated/search"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	eventMapping       = `{"settings":{"number_of_shards":1,"number_of_replicas":0},"mappings":{"events":{"properties":{"api-version":{"type":"text"},"count":{"type":"integer"},"first-timestamp":{"type":"text"},"kind":{"type":"keyword"},"last-timestamp":{"type":"text"},"message":{"type":"text"},"meta":{"properties":{"creation-time":{"type":"date"},"labels":{"properties":{"key":{"type":"text"},"value":{"type":"text"}}},"mod-time":{"type":"date"},"name":{"type":"text"},"namespace":{"type":"text"},"resource-version":{"type":"text"},"self-link":{"type":"text"},"tenant":{"type":"keyword"},"uuid":{"type":"text"}}},"object-ref":{"properties":{"kind":{"type":"keyword"},"name":{"type":"text"},"namespace":{"type":"text"}}},"severity":{"type":"keyword"},"source":{"properties":{"component":{"type":"keyword"},"node-name":{"type":"text"}}},"type":{"type":"keyword"}}}}}`
	eventMappingPretty = `{
    "settings": {
        "number_of_shards": 1,
        "number_of_replicas": 0
    },
    "mappings": {
        "events": {
            "properties": {
                "api-version": {
                    "type": "text"
                },
                "count": {
                    "type": "integer"
                },
                "first-timestamp": {
                    "type": "text"
                },
                "kind": {
                    "type": "keyword"
                },
                "last-timestamp": {
                    "type": "text"
                },
                "message": {
                    "type": "text"
                },
                "meta": {
                    "properties": {
                        "creation-time": {
                            "type": "date"
                        },
                        "labels": {
                            "properties": {
                                "key": {
                                    "type": "text"
                                },
                                "value": {
                                    "type": "text"
                                }
                            }
                        },
                        "mod-time": {
                            "type": "date"
                        },
                        "name": {
                            "type": "text"
                        },
                        "namespace": {
                            "type": "text"
                        },
                        "resource-version": {
                            "type": "text"
                        },
                        "self-link": {
                            "type": "text"
                        },
                        "tenant": {
                            "type": "keyword"
                        },
                        "uuid": {
                            "type": "text"
                        }
                    }
                },
                "object-ref": {
                    "properties": {
                        "kind": {
                            "type": "keyword"
                        },
                        "name": {
                            "type": "text"
                        },
                        "namespace": {
                            "type": "text"
                        }
                    }
                },
                "severity": {
                    "type": "keyword"
                },
                "source": {
                    "properties": {
                        "component": {
                            "type": "keyword"
                        },
                        "node-name": {
                            "type": "text"
                        }
                    }
                },
                "type": {
                    "type": "keyword"
                }
            }
        }
    }
}`

	searchMapping       = `{"settings":{"number_of_shards":1,"number_of_replicas":0},"mappings":{"configs":{"properties":{"api-version":{"type":"text"},"kind":{"type":"keyword"},"meta":{"properties":{"creation-time":{"type":"date"},"labels":{"properties":{"key":{"type":"text"},"value":{"type":"text"}}},"mod-time":{"type":"date"},"name":{"type":"text"},"namespace":{"type":"text"},"resource-version":{"type":"text"},"self-link":{"type":"text"},"tenant":{"type":"keyword"},"uuid":{"type":"text"}}}}}}}`
	searchMappingPretty = `{
    "settings": {
        "number_of_shards": 1,
        "number_of_replicas": 0
    },
    "mappings": {
        "configs": {
            "properties": {
                "api-version": {
                    "type": "text"
                },
                "kind": {
                    "type": "keyword"
                },
                "meta": {
                    "properties": {
                        "creation-time": {
                            "type": "date"
                        },
                        "labels": {
                            "properties": {
                                "key": {
                                    "type": "text"
                                },
                                "value": {
                                    "type": "text"
                                }
                            }
                        },
                        "mod-time": {
                            "type": "date"
                        },
                        "name": {
                            "type": "text"
                        },
                        "namespace": {
                            "type": "text"
                        },
                        "resource-version": {
                            "type": "text"
                        },
                        "self-link": {
                            "type": "text"
                        },
                        "tenant": {
                            "type": "keyword"
                        },
                        "uuid": {
                            "type": "text"
                        }
                    }
                }
            }
        }
    }
}`
	s2Mapping       = `{"settings":{"number_of_shards":1,"number_of_replicas":0},"mappings":{"s2":{"properties":{"P":{"properties":{"X":{"type":"long"},"Y":{"type":"boolean"},"Z":{"properties":{"X":{"type":"text"}}}}}}}}}`
	s2MappingPretty = `{
    "settings": {
        "number_of_shards": 1,
        "number_of_replicas": 0
    },
    "mappings": {
        "s2": {
            "properties": {
                "P": {
                    "properties": {
                        "X": {
                            "type": "long"
                        },
                        "Y": {
                            "type": "boolean"
                        },
                        "Z": {
                            "properties": {
                                "X": {
                                    "type": "text"
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}`
	nullMapping       = `{"settings":{"number_of_shards":0,"number_of_replicas":0},"mappings":null}`
	nullMappingPretty = `{
    "settings": {
        "number_of_shards": 0,
        "number_of_replicas": 0
    },
    "mappings": null
}`
)

type S0 struct {
	X string
}

type S1 struct {
	X int
	Y []bool
	Z []S0
}

type S2 struct {
	P *S1
}

func TestElasticMapper(t *testing.T) {

	// Fill logger config params
	logConfig := &log.Config{
		Module:      "elastic-mapper-test",
		Format:      log.LogFmt,
		Filter:      log.AllowInfoFilter,
		Debug:       false,
		CtxSelector: log.ContextAll,
		LogToStdout: true,
		LogToFile:   false,
	}

	// Initialize logger config
	log.SetConfig(logConfig)

	// Mapper testcase table
	testCases := []struct {
		object                interface{}
		docType               string
		shards                uint
		replicas              uint
		err                   error
		expectedMapping       string
		expectedMappingPretty string
	}{
		{
			search.Entry{},
			elastic.GetDocType(globals.Configs),
			1,
			0,
			nil,
			searchMapping,
			searchMappingPretty,
		},
		{
			events.Event{
				EventAttributes: events.EventAttributes{
					// Need to make sure pointer fields are valid to
					// generate right mappings using reflect
					ObjectRef: &api.ObjectRef{},
					Source:    &events.EventSource{},
				},
			},
			elastic.GetDocType(globals.Events),
			1,
			0,
			nil,
			eventMapping,
			eventMappingPretty,
		},
		{
			S2{
				P: &S1{
					Y: make([]bool, 1),
					Z: make([]S0, 1),
				},
			},
			"s2",
			1,
			0,
			nil,
			s2Mapping,
			s2MappingPretty,
		},
		{
			nil,
			"Nil",
			1,
			0,
			errors.New("Invalid object"),
			nullMapping,
			nullMappingPretty,
		},
	}

	// Execute mappers testcases
	for _, tc := range testCases {
		t.Run(tc.docType, func(t *testing.T) {
			// generate Elastic mapping and settings
			config, err := ElasticMapper(tc.object, tc.docType, tc.shards, tc.replicas)
			AssertEquals(t, tc.err, err, "Error status mismatch for elastic mapper")
			t.Logf("\nEntry: %+v", tc.object)
			t.Logf("\nMapping: %+v", config)

			// validate JSON string
			str, err := config.JSONString()
			t.Logf("\nJSON string: \n%+v", str)
			AssertOk(t, err, "Failed to get JSONString")
			AssertEquals(t, tc.expectedMapping, str, "JSON string mismatch")

			// validate JSON pretty string
			str, err = config.JSONPrettyString()
			t.Logf("\nJSON pretty string: \n%+v", str)
			AssertOk(t, err, "Failed to get JSONPrettyString")
			AssertEquals(t, tc.expectedMappingPretty, str, "JSON pretty string mismatch")
		})
	}
}
