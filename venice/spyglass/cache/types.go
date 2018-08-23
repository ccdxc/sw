// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package cache

import "github.com/pensando/sw/api/generated/search"

// Interface is an interface for SGpolicy cache
type Interface interface {

	// UpdateObject adds or update the objects to/in cache
	UpdateObject(obj interface{}) error

	// DeleteObject removed the object from the cache
	DeleteObject(obj interface{}) error

	// SearchPolicy
	SearchPolicy(query *search.PolicySearchRequest) (*search.PolicySearchResponse, error)
}
