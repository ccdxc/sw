// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package dispatcher

import (
	"sync"
	"time"

	memcache "github.com/pensando/sw/venice/utils/cache"
)

// global cache encapsulating the cache for each event source.
// each event source gets it's own cache. It helps reduce contention and also,
// it gives the flexibility to dedup at each source. Source of the event is
// inferred from the recorder.
type cache struct {
	sync.Mutex                           // to protect access to the underlying map
	m          map[string]memcache.Cache // cache for each source; the source keys are removed from the map only during flush
	ttl        time.Duration
}

// newMemCache create a new memcache which is used to store events from single source.
func (c *cache) newMemCache() memcache.Cache {
	return memcache.New(c.ttl, 0, nil)
}

// clears all the underlying cache (sources).
// make sure to perform this opertion under lock to avoid race conditions.
func (c *cache) clear() {
	c.m = map[string]memcache.Cache{}
}

// getCache returns the map of all the underlying memcache corresponding to each source.
func (c *cache) getCache() map[string]memcache.Cache {
	return c.m
}

// getSourceCache returns the underlying cache of the given source key.
// it creates, if it does not exists already.
// make sure to perform this opertion under lock to avoid race conditions.
func (c *cache) getSourceCache(sourceKey string) memcache.Cache {
	if c.m[sourceKey] == nil {
		c.m[sourceKey] = c.newMemCache()
	}

	return c.m[sourceKey]
}
