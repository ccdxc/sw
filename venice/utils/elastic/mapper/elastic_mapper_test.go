// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package ref

import (
	"errors"
	"testing"

	"github.com/pensando/sw/api"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/api/generated/search"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	eventTemplateMapping = `{"index_patterns":"*.events.*","settings":{"number_of_shards":1,"number_of_replicas":0,"codec":"best_compression","max_inner_result_window":8192},"mappings":{"events":{"properties":{"api-version":{"type":"text"},"category":{"fields":{"keyword":{"ignore_above":256,"type":"keyword"}},"type":"text"},"count":{"type":"integer"},"kind":{"fields":{"keyword":{"ignore_above":256,"type":"keyword"}},"type":"text"},"message":{"type":"text"},"meta":{"properties":{"creation-time":{"type":"date"},"generation-id":{"type":"text"},"labels":{"properties":{"key":{"type":"text"},"value":{"type":"text"}}},"mod-time":{"type":"date"},"name":{"fields":{"keyword":{"ignore_above":256,"type":"keyword"}},"type":"text"},"namespace":{"type":"text"},"resource-version":{"type":"text"},"self-link":{"type":"text"},"tenant":{"fields":{"keyword":{"ignore_above":256,"type":"keyword"}},"type":"text"},"uuid":{"type":"text"}}},"object-ref":{"properties":{"kind":{"fields":{"keyword":{"ignore_above":256,"type":"keyword"}},"type":"text"},"name":{"fields":{"keyword":{"ignore_above":256,"type":"keyword"}},"type":"text"},"namespace":{"type":"text"},"tenant":{"fields":{"keyword":{"ignore_above":256,"type":"keyword"}},"type":"text"},"uri":{"type":"text"}}},"severity":{"fields":{"keyword":{"ignore_above":256,"type":"keyword"}},"type":"text"},"source":{"properties":{"component":{"fields":{"keyword":{"ignore_above":256,"type":"keyword"}},"type":"text"},"node-name":{"type":"text"}}},"type":{"fields":{"keyword":{"ignore_above":256,"type":"keyword"}},"type":"text"}}}}}
`
	eventTemplateMappingPretty = `{
    "index_patterns": "*.events.*",
    "settings": {
        "number_of_shards": 1,
        "number_of_replicas": 0,
        "codec": "best_compression",
        "max_inner_result_window": 8192
    },
    "mappings": {
        "events": {
            "properties": {
                "api-version": {
                    "type": "text"
                },
                "category": {
                    "fields": {
                        "keyword": {
                            "ignore_above": 256,
                            "type": "keyword"
                        }
                    },
                    "type": "text"
                },
                "count": {
                    "type": "integer"
                },
                "kind": {
                    "fields": {
                        "keyword": {
                            "ignore_above": 256,
                            "type": "keyword"
                        }
                    },
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
                        "generation-id": {
                            "type": "text"
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
                            "fields": {
                                "keyword": {
                                    "ignore_above": 256,
                                    "type": "keyword"
                                }
                            },
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
                            "fields": {
                                "keyword": {
                                    "ignore_above": 256,
                                    "type": "keyword"
                                }
                            },
                            "type": "text"
                        },
                        "uuid": {
                            "type": "text"
                        }
                    }
                },
                "object-ref": {
                    "properties": {
                        "kind": {
                            "fields": {
                                "keyword": {
                                    "ignore_above": 256,
                                    "type": "keyword"
                                }
                            },
                            "type": "text"
                        },
                        "name": {
                            "fields": {
                                "keyword": {
                                    "ignore_above": 256,
                                    "type": "keyword"
                                }
                            },
                            "type": "text"
                        },
                        "namespace": {
                            "type": "text"
                        },
                        "tenant": {
                            "fields": {
                                "keyword": {
                                    "ignore_above": 256,
                                    "type": "keyword"
                                }
                            },
                            "type": "text"
                        },
                        "uri": {
                            "type": "text"
                        }
                    }
                },
                "severity": {
                    "fields": {
                        "keyword": {
                            "ignore_above": 256,
                            "type": "keyword"
                        }
                    },
                    "type": "text"
                },
                "source": {
                    "properties": {
                        "component": {
                            "fields": {
                                "keyword": {
                                    "ignore_above": 256,
                                    "type": "keyword"
                                }
                            },
                            "type": "text"
                        },
                        "node-name": {
                            "type": "text"
                        }
                    }
                },
                "type": {
                    "fields": {
                        "keyword": {
                            "ignore_above": 256,
                            "type": "keyword"
                        }
                    },
                    "type": "text"
                }
            }
        }
    }
}
`

	searchMapping = `{"settings":{"number_of_shards":3,"number_of_replicas":2,"codec":"best_compression","max_inner_result_window":256},"mappings":{"configs":{"properties":{"api-version":{"type":"text"},"kind":{"fields":{"keyword":{"ignore_above":256,"type":"keyword"}},"type":"text"},"meta":{"properties":{"creation-time":{"type":"date"},"generation-id":{"type":"text"},"labels":{"properties":{"key":{"type":"text"},"value":{"type":"text"}}},"mod-time":{"type":"date"},"name":{"fields":{"keyword":{"ignore_above":256,"type":"keyword"}},"type":"text"},"namespace":{"type":"text"},"resource-version":{"type":"text"},"self-link":{"type":"text"},"tenant":{"fields":{"keyword":{"ignore_above":256,"type":"keyword"}},"type":"text"},"uuid":{"type":"text"}}}}}}}
`
	searchMappingPretty = `{
    "settings": {
        "number_of_shards": 3,
        "number_of_replicas": 2,
        "codec": "best_compression",
        "max_inner_result_window": 256
    },
    "mappings": {
        "configs": {
            "properties": {
                "api-version": {
                    "type": "text"
                },
                "kind": {
                    "fields": {
                        "keyword": {
                            "ignore_above": 256,
                            "type": "keyword"
                        }
                    },
                    "type": "text"
                },
                "meta": {
                    "properties": {
                        "creation-time": {
                            "type": "date"
                        },
                        "generation-id": {
                            "type": "text"
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
                            "fields": {
                                "keyword": {
                                    "ignore_above": 256,
                                    "type": "keyword"
                                }
                            },
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
                            "fields": {
                                "keyword": {
                                    "ignore_above": 256,
                                    "type": "keyword"
                                }
                            },
                            "type": "text"
                        },
                        "uuid": {
                            "type": "text"
                        }
                    }
                }
            }
        }
    }
}
`

	searchMappingWithCharFilter = `{"settings":{"number_of_shards":3,"number_of_replicas":2,"codec":"best_compression","max_inner_result_window":256,"analysis":{"analyzer":{"t_analyzer":{"char_filter":["t_char_filter"],"filter":["lowercase"],"tokenizer":"standard"}},"char_filter":{"t_char_filter":{"mappings":["- => _"],"type":"mapping"}}}},"mappings":{"configs":{"properties":{"api-version":{"type":"text"},"kind":{"analyzer":"t_analyzer","fields":{"keyword":{"ignore_above":256,"type":"keyword"}},"type":"text"},"meta":{"properties":{"creation-time":{"type":"date"},"generation-id":{"type":"text"},"labels":{"properties":{"key":{"type":"text"},"value":{"type":"text"}}},"mod-time":{"type":"date"},"name":{"analyzer":"t_analyzer","fields":{"keyword":{"ignore_above":256,"type":"keyword"}},"type":"text"},"namespace":{"type":"text"},"resource-version":{"type":"text"},"self-link":{"type":"text"},"tenant":{"analyzer":"t_analyzer","fields":{"keyword":{"ignore_above":256,"type":"keyword"}},"type":"text"},"uuid":{"type":"text"}}}}}}}
`
	searchMappingPrettyWithCharFilter = `{
    "settings": {
        "number_of_shards": 3,
        "number_of_replicas": 2,
        "codec": "best_compression",
        "max_inner_result_window": 256,
        "analysis": {
            "analyzer": {
                "t_analyzer": {
                    "char_filter": [
                        "t_char_filter"
                    ],
                    "filter": [
                        "lowercase"
                    ],
                    "tokenizer": "standard"
                }
            },
            "char_filter": {
                "t_char_filter": {
                    "mappings": [
                        "- => _"
                    ],
                    "type": "mapping"
                }
            }
        }
    },
    "mappings": {
        "configs": {
            "properties": {
                "api-version": {
                    "type": "text"
                },
                "kind": {
                    "analyzer": "t_analyzer",
                    "fields": {
                        "keyword": {
                            "ignore_above": 256,
                            "type": "keyword"
                        }
                    },
                    "type": "text"
                },
                "meta": {
                    "properties": {
                        "creation-time": {
                            "type": "date"
                        },
                        "generation-id": {
                            "type": "text"
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
                            "analyzer": "t_analyzer",
                            "fields": {
                                "keyword": {
                                    "ignore_above": 256,
                                    "type": "keyword"
                                }
                            },
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
                            "analyzer": "t_analyzer",
                            "fields": {
                                "keyword": {
                                    "ignore_above": 256,
                                    "type": "keyword"
                                }
                            },
                            "type": "text"
                        },
                        "uuid": {
                            "type": "text"
                        }
                    }
                }
            }
        }
    }
}
`

	s2Mapping = `{"settings":{"number_of_shards":5,"number_of_replicas":1,"codec":"lz4","max_inner_result_window":256},"mappings":{"s2":{"properties":{"P":{"properties":{"X":{"type":"long"},"Y":{"type":"boolean"},"Z":{"properties":{"X":{"type":"text"}}}}}}}}}
`
	s2MappingPretty = `{
    "settings": {
        "number_of_shards": 5,
        "number_of_replicas": 1,
        "codec": "lz4",
        "max_inner_result_window": 256
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
}
`
	nullMapping = `{"settings":{"number_of_shards":0,"number_of_replicas":0,"codec":"","max_inner_result_window":0},"mappings":null}
`
	nullMappingPretty = `{
    "settings": {
        "number_of_shards": 0,
        "number_of_replicas": 0,
        "codec": "",
        "max_inner_result_window": 0
    },
    "mappings": null
}
`
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
		options               []Option
		err                   error
		expectedMapping       string
		expectedMappingPretty string
	}{
		{
			search.ConfigEntry{},
			elastic.GetDocType(globals.Configs),
			nil, // test default option
			nil,
			searchMapping,
			searchMappingPretty,
		},
		{
			evtsapi.Event{
				EventAttributes: evtsapi.EventAttributes{
					// Need to make sure pointer fields are valid to
					// generate right mappings using reflect
					ObjectRef: &api.ObjectRef{},
					Source:    &evtsapi.EventSource{},
				},
			},
			elastic.GetDocType(globals.Events),
			[]Option{
				WithReplicaCount(0),
				WithShardCount(1),
				WithMaxInnerResults(8192),
				WithIndexPatterns("*.events.*"),
			},
			nil,
			eventTemplateMapping,
			eventTemplateMappingPretty,
		},
		{
			S2{
				P: &S1{
					Y: make([]bool, 1),
					Z: make([]S0, 1),
				},
			},
			"s2",
			[]Option{
				WithShardCount(5),
				WithReplicaCount(1),
				WithCodec("lz4"),
			},
			nil,
			s2Mapping,
			s2MappingPretty,
		},
		{
			search.ConfigEntry{},
			elastic.GetDocType(globals.Configs),
			[]Option{WithCharFilter()},
			nil,
			searchMappingWithCharFilter,
			searchMappingPrettyWithCharFilter,
		},
		{
			nil,
			"Nil",
			nil,
			errors.New("Invalid object"),
			nullMapping,
			nullMappingPretty,
		},
	}

	// Execute mappers testcases
	for _, tc := range testCases {
		t.Run(tc.docType, func(t *testing.T) {
			// generate Elastic mapping and settings
			config, err := ElasticMapper(tc.object, tc.docType, tc.options...)
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

func TestMapperDisableIndexing(t *testing.T) {
	_, ok := isIndexingDisabled[elastic.GetDocType(globals.FwLogs)]["rule-id"]
	Assert(t, ok, "indexing is enabled on fwlogs::ruleid")
	_, ok = isIndexingDisabled[elastic.GetDocType(globals.FwLogs)]["session-id"]
	Assert(t, ok, "indexing is enabled on fwlogs::sessid")
	_, ok = isIndexingDisabled[elastic.GetDocType(globals.FwLogs)]["flow-action"]
	Assert(t, ok, "indexing is enabled on fwlogs::flowaction")
	_, ok = isIndexingDisabled[elastic.GetDocType(globals.FwLogs)]["icmp-type"]
	Assert(t, ok, "indexing is enabled on fwlogs::icmptype")
	_, ok = isIndexingDisabled[elastic.GetDocType(globals.FwLogs)]["icmp-id"]
	Assert(t, ok, "indexing is enabled on fwlogs::icmpid")
	_, ok = isIndexingDisabled[elastic.GetDocType(globals.FwLogs)]["icmp-code"]
	Assert(t, ok, "indexing is enabled on fwlogs::icmpcode")
	_, ok = isIndexingDisabled[elastic.GetDocType(globals.FwLogs)]["direction"]
	Assert(t, ok, "indexing is enabled on fwlogs::dir")
	_, ok = isIndexingDisabled[elastic.GetDocType(globals.FwLogs)]["source-ip"]
	Assert(t, !ok, "indexing is disabled on fwlogs::sip")
	_, ok = isIndexingDisabled[elastic.GetDocType(globals.FwLogs)]["destination-ip"]
	Assert(t, !ok, "indexing is disabled on fwlogs::dip")
	_, ok = isIndexingDisabled[elastic.GetDocType(globals.FwLogs)]["source-port"]
	Assert(t, !ok, "indexing is disabled on fwlogs::sport")
	_, ok = isIndexingDisabled[elastic.GetDocType(globals.FwLogs)]["destination-port"]
	Assert(t, !ok, "indexing is disabled on fwlogs::dport")

	_, ok = isIndexingDisabled[elastic.GetDocType(globals.FwLogsObjects)]["bucket"]
	Assert(t, ok, "indexing is enabled on fwlogobjects::bucket")
	_, ok = isIndexingDisabled[elastic.GetDocType(globals.FwLogsObjects)]["logscount"]
	Assert(t, ok, "indexing is enabled on fwlogobjects::logscount")
	_, ok = isIndexingDisabled[elastic.GetDocType(globals.FwLogsObjects)]["startts"]
	Assert(t, ok, "indexing is enabled on fwlogobjects::startts")
	_, ok = isIndexingDisabled[elastic.GetDocType(globals.FwLogsObjects)]["endts"]
	Assert(t, ok, "indexing is enabled on fwlogobjects::endts")
	_, ok = isIndexingDisabled[elastic.GetDocType(globals.FwLogsObjects)]["key"]
	Assert(t, !ok, "indexing is disabled on fwlogobjects::key")
	_, ok = isIndexingDisabled[elastic.GetDocType(globals.FwLogsObjects)]["tenant"]
	Assert(t, !ok, "indexing is disabled on fwlogobjects::tenant")
	_, ok = isIndexingDisabled[elastic.GetDocType(globals.FwLogsObjects)]["dscid"]
	Assert(t, !ok, "indexing is disabled on fwlogobjects::dscid")
}
