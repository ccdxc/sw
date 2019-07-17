// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package globals

import (
	"strings"
	"sync"

	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

const (

	// CategoryLabel is used to add category attribute to
	// objects indexed in elastic. This is needed to enable
	// aggregation by category attribute
	CategoryLabel = "_category"
)

var kindsMap map[string]string
var groupMap map[string][]string
var kindsMapOnce sync.Once

var overrideMap = map[string]string{
	"Event":      "Monitoring",
	"AuditEvent": "Monitoring",
}

// Kind2Category is map of Kind to its Search Category (api-group)
func Kind2Category(kind string) string {
	kindsMapOnce.Do(generateKinds2Category)
	cat, ok := kindsMap[kind]
	if !ok {
		log.Errorf("No entry for kind %v", kind)
	}
	return cat
}

// Category2Kinds is map of category to a list of kinds
func Category2Kinds(cat string) []string {
	kindsMapOnce.Do(generateKinds2Category)
	kinds, ok := groupMap[cat]
	if !ok {
		log.Errorf("No category entry for %v", cat)
	}
	return kinds
}

func generateKinds2Category() {
	schema := runtime.GetDefaultScheme()
	groups := schema.Kinds()
	kindsMap = make(map[string]string)
	groupMap = make(map[string][]string)
	for k, v := range groups {
		for _, v1 := range v {
			if _, ok := kindsMap[v1]; ok {
				log.Fatalf("duplicate kind registrated [%v]", v1)
			}
			kind := strings.Title(v1)
			cat, ok := overrideMap[kind]
			if !ok {
				cat = strings.Title(k)
			}
			kindsMap[kind] = cat
			if _, ok := groupMap[cat]; !ok {
				groupMap[cat] = []string{}
			}
			groupMap[cat] = append(groupMap[cat], kind)
		}
	}

	// For any kinds in overrideMap that aren't already added, we add now
	// AuditEvent is currently missing from schema.Kinds
	for kind, cat := range overrideMap {
		if _, ok := kindsMap[kind]; !ok {
			kindsMap[kind] = cat
			if _, ok = groupMap[cat]; !ok {
				groupMap[cat] = []string{}
			}
			groupMap[cat] = append(groupMap[cat], kind)
		}
	}
}
