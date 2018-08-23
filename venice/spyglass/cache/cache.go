// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package cache

import (
	"bytes"
	"fmt"
	"net"
	"strings"
	"sync"

	"github.com/pensando/sw/api/generated/search"
	"github.com/pensando/sw/api/generated/security"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

// Cache is an implementation of the cache.Interface
type Cache struct {
	sync.RWMutex
	logger log.Logger

	// Map of {Tenant Name} -> {Namespace+ObjectName} -> {Object}
	cache map[string]map[string]interface{}
}

// NewCache instantiates a new cache
func NewCache(logger log.Logger) Interface {

	cache := &Cache{
		cache:  make(map[string]map[string]interface{}),
		logger: logger,
	}
	return cache
}

// UpdateObject adds or updates policy objects to/in the cache
func (c *Cache) UpdateObject(object interface{}) error {

	c.Lock()
	defer c.Unlock()

	// get object meta
	ometa, err := runtime.GetObjectMeta(object)
	if err != nil {
		return err
	}

	// validate kind
	kind := object.(runtime.Object).GetObjectKind()
	if kind != "SGPolicy" {
		return fmt.Errorf("Invalid object kind, SGPolicy expected")
	}

	// add/update object
	tenant := ometa.Tenant
	objKey := c.getObjectKey(ometa.Namespace, ometa.Name)
	_, ok := c.cache[tenant]
	if !ok {
		c.cache[tenant] = make(map[string]interface{})
	}
	c.logger.Debugf("Policy-cache: updating object: {%s, %s}", tenant, objKey)
	c.cache[tenant][objKey] = object
	return nil
}

// DeleteObject removes policy object from the cache
func (c *Cache) DeleteObject(object interface{}) error {

	c.Lock()
	defer c.Unlock()

	// get object meta
	ometa, err := runtime.GetObjectMeta(object)
	if err != nil {
		return err
	}

	// validate kind
	kind := object.(runtime.Object).GetObjectKind()
	if kind != "SGPolicy" {
		return fmt.Errorf("Invalid object kind, SGPolicy expected")
	}

	// delete object
	tenant := ometa.Tenant
	objKey := c.getObjectKey(ometa.Namespace, ometa.Name)
	_, ok := c.cache[tenant]
	if ok {
		c.logger.Debugf("Policy-cache: deleting object: {%s, %s}", tenant, objKey)
		delete(c.cache[tenant], objKey)
	}
	return nil
}

// Helper function to match on Apps field
func (c *Cache) matchApp(query *search.PolicySearchRequest, entry *security.SGRule) bool {

	// check for empty/any
	if query.App == "" ||
		strings.ToLower(query.App) == "any" {
		return true
	}

	for _, app := range entry.Apps {
		if strings.ToLower(app) == strings.ToLower(query.App) {
			return true
		}
	}
	return false
}

// Helper function to parse and validate Ip in range format
// eg:  30.1.1.1-30.1.1.10
// TODO: Explore if policy validators code in apiserver can be reused here.
func (c *Cache) parseIPRange(ip string) (startIP, endIP net.IP, err error) {

	// parse ip range format
	ipRange := strings.Split(ip, "-")
	if len(ipRange) != 2 {
		return nil, nil, fmt.Errorf("invalid ip range format, specified %v", ip)
	}

	// valid start and end IP
	startIP = net.ParseIP(strings.TrimSpace(ipRange[0]))
	endIP = net.ParseIP(strings.TrimSpace(ipRange[1]))
	if len(startIP) > 0 && len(endIP) > 0 {
		return startIP, endIP, nil
	}

	return nil, nil, fmt.Errorf("could not parse ip in range format, %v", ip)
}

// Helper function to match IP in query against the IPs in policy entry.
func (c *Cache) matchIP(queryIP string, entryIPs []string) bool {

	// check for empty/any
	if queryIP == "" ||
		strings.ToLower(queryIP) == "any" {
		return true
	}

	// validate input
	toIP := net.ParseIP(queryIP)
	if toIP == nil {
		c.logger.Errorf("Error parsing query toIP: %v", queryIP)
		return false
	}

	for _, ip := range entryIPs {

		// check for CIDR mask
		var toSubnet *net.IPNet
		var err error
		if strings.Contains(ip, "/") {
			_, toSubnet, err = net.ParseCIDR(ip)
			if err != nil {
				c.logger.Errorf("Error parsing CIDR toAddr: %v err: %v", ip, err)
			}
		}

		// check for range format
		var startIP, endIP net.IP
		rangeIP := false
		if strings.Contains(ip, "-") {
			startIP, endIP, err = c.parseIPRange(ip)
			if err == nil {
				rangeIP = true
			}
		}

		// perform match
		if queryIP == ip ||
			strings.ToLower(ip) == "any" ||
			(toSubnet != nil && toIP != nil && toSubnet.Contains(toIP)) ||
			(rangeIP && bytes.Compare(toIP, startIP) >= 0 && bytes.Compare(toIP, endIP) <= 0) {
			return true
		}
	}

	return false
}

// Helper function to match on SecurityGroups
func (c *Cache) matchSG(querySG string, entrySGs []string) bool {

	// check for empty/any
	if querySG == "" ||
		strings.ToLower(querySG) == "any" {
		return true
	}

	for _, sg := range entrySGs {
		if querySG == sg || strings.ToLower(sg) == "any" {
			return true
		}
	}

	return false
}

// SearchPolicy queries for match within a SG policy object
func (c *Cache) SearchPolicy(query *search.PolicySearchRequest) (*search.PolicySearchResponse, error) {

	c.RLock()
	defer c.RUnlock()

	// Validate input
	if query == nil {
		return &search.PolicySearchResponse{}, fmt.Errorf("Invalid search request")
	}

	// Handle special case, check if all the key fields are empty
	// There needs to be atleast one or more valid keys for the query
	if query.App == "" &&
		query.SGPolicy == "" &&
		query.FromIPAddress == "" &&
		query.ToIPAddress == "" &&
		query.FromSecurityGroup == "" &&
		query.ToSecurityGroup == "" {
		return &search.PolicySearchResponse{Status: search.PolicySearchResponse_MISS.String()}, nil
	}

	results := make(map[string]*search.PolicyMatchEntry)

	// Search within a specific SGPolicy object if specified
	if query.SGPolicy != "" {
		objKey := c.getObjectKey(query.Namespace, query.SGPolicy)
		obj, ok := c.cache[query.Tenant][objKey]
		if !ok {
			c.logger.Errorf("SearchPolicy: SGPolicy object key: %s not found", objKey)
			return &search.PolicySearchResponse{Status: search.PolicySearchResponse_MISS.String()}, fmt.Errorf("SGPolicy object: %s not found", query.SGPolicy)
		}
		sgp, ok := obj.(*security.SGPolicy)
		if !ok {
			c.logger.Errorf("SGPolicy kind assertion failure, for tenant: %s object: %s",
				query.Tenant, query.SGPolicy)
			return &search.PolicySearchResponse{Status: search.PolicySearchResponse_MISS.String()}, nil
		}
		if match, objName, entry, err := c.searchSGP(query, sgp); match {
			results[objName] = entry
			resp := &search.PolicySearchResponse{
				Status:  search.PolicySearchResponse_MATCH.String(),
				Results: results,
			}
			return resp, err
		}
		return &search.PolicySearchResponse{Status: search.PolicySearchResponse_MISS.String()}, nil
	}

	// Search all SGPolicies within a Tenant
	for key, obj := range c.cache[query.Tenant] {
		sgp, ok := obj.(*security.SGPolicy)
		if !ok {
			c.logger.Errorf("SGPolicy kind assertion failure, for tenant: %s object-key: %s",
				query.Tenant, key)
			continue
		}
		if match, objName, entry, _ := c.searchSGP(query, sgp); match {
			results[objName] = entry
			continue
		}
	}

	status := search.PolicySearchResponse_MISS.String()
	if len(results) > 0 {
		status = search.PolicySearchResponse_MATCH.String()
	}
	resp := &search.PolicySearchResponse{
		Status:  status,
		Results: results,
	}
	return resp, nil
}

// Helper function to match on SGPolicy attributes
func (c *Cache) searchSGP(query *search.PolicySearchRequest, sgp *security.SGPolicy) (match bool, objName string, entry *search.PolicyMatchEntry, err error) {

	if query == nil || sgp == nil {
		return false, "", nil, fmt.Errorf("Invalid arguments")
	}

	// This performs a linear search for the first match on
	// the slice of SGRules. With linear seach for 70k rules
	// this query function performance is < 50ms (based on
	// benchmark tests and meets the UX/UI requirements.
	//
	// If better performance is needed, doing parallel search
	// on policy attributes or partitioning the search on SGrule
	// slice and merging the results (Map-reduce like approach)
	// with bunch of go-routines will provide 40-50% improvement
	// in search response based on benchmark tests results listed
	// below.
	//
	// [psearch] $ go test -bench=.
	//   goos: darwin
	//   goarch: amd64
	//   pkg: github.com/pensando/sw/test/psearch
	//   BenchmarkSearchMiss-8                         30          41892769 ns/op
	//   BenchmarkParallelSearchMiss-8                 50          22525386 ns/op
	//   BenchmarkSearchHit1-8                    2000000               647 ns/op
	//   BenchmarkParallelSearchHit1-8             500000              3265 ns/op
	//   BenchmarkSearchHitAny-8                  3000000               492 ns/op
	//   BenchmarkParallelSearchHitAny-8           500000              3181 ns/op
	//   BenchmarkSearchHit35001-8                    100          20426578 ns/op
	//   BenchmarkParallelSearchHit35001-8            100          11557495 ns/op
	//   BenchmarkSearchHit70000-8                     30          45034781 ns/op
	//   BenchmarkParallelSearchHit70000-8            100          24315854 ns/op
	//   PASS
	//   ok      github.com/pensando/sw/test/psearch     17.039s

	for i, rule := range sgp.Spec.Rules {
		if c.matchApp(query, rule) &&
			c.matchIP(query.FromIPAddress, rule.FromIPAddresses) &&
			c.matchIP(query.ToIPAddress, rule.ToIPAddresses) &&
			c.matchSG(query.FromSecurityGroup, rule.FromSecurityGroups) &&
			c.matchSG(query.ToSecurityGroup, rule.ToSecurityGroups) {
			entry = &search.PolicyMatchEntry{
				Rule:  rule,
				Index: uint32(i),
			}
			return true, sgp.Name, entry, nil
		}
	}
	return false, "", nil, nil
}

// Helper function to construct object-key for policy-cache
func (c *Cache) getObjectKey(namespace, name string) string {
	return fmt.Sprintf("%s/%s", namespace, name)
}
