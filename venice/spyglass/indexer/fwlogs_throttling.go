package indexer

import (
	"sync"
	"time"

	rl "golang.org/x/time/rate"
)

var (
	// The maximum number of logs accepeted in one batch by the indexer
	// burst = 50000 i.e. 5k burst for 10 seconds
	defaultFlowlogsBurst = 50000
	// The maximum rate of logs accepted by the indexer
	// 1000 = 1k CPS, 1000 DSCs
	defaultFlowlogsRateLimit = float64(1000) // logs/s
	// global rate limiter name
	globalFlowlogsRateLimiter = "globalFlowlogsRateLimiter"
)

// RateLimiterCollection is used for maintaining a collection of rate limiters keyed by name
type RateLimiterCollection struct {
	limiters *sync.Map
}

// RateLimiter can be used rate limiting indexing
type RateLimiter struct {
	limiter *rl.Limiter
}

// rateLimitConfig is the configuration for a rate-limiter that can be used to
// limit connections accepted by RPCServer listener.
type rateLimitConfig struct {
	R rl.Limit // rate in number of events per second. 0 = no events
	B int      // maxumum burst size in numbers of events. 0 = no events
}

func newRateLimiter(rate float64, burst int) *RateLimiter {
	return &RateLimiter{
		limiter: rl.NewLimiter(rl.Limit(rate), burst),
	}
}

func newRateLimitConfig() *rateLimitConfig {
	return &rateLimitConfig{
		R: rl.Limit(defaultFlowlogsRateLimit),
		B: defaultFlowlogsBurst,
	}
}

// allowN check whether n events can be allowed at this point of time or not
// Use this method if you want to drop the events in case they are not allowed
func (r *RateLimiter) allowN(count int) bool {
	return r.limiter.AllowN(time.Now(), count)
}

// setBurst is for updating the burst after creating the limiter
func (r *RateLimiter) setBurst(burst int) {
	r.limiter.SetBurst(burst)
}

// setRate is for updating the rate after creating the limiter
func (r *RateLimiter) setRate(rate float64) {
	r.limiter.SetLimit(rl.Limit(rate))
}

// rate returns the current limit value
func (r *RateLimiter) rate() float64 {
	return float64(r.limiter.Limit())
}

// burst returns the current burst value
func (r *RateLimiter) burst() int {
	return r.limiter.Burst()
}

func newRateLimiterCollection() *RateLimiterCollection {
	m := new(sync.Map)
	m.Store(globalFlowlogsRateLimiter, newRateLimiter(defaultFlowlogsRateLimit, defaultFlowlogsBurst))
	return &RateLimiterCollection{limiters: m}
}

// allowN check whether n events can be allowed at this point of time or not for the named key
func (rc *RateLimiterCollection) allowN(name string, count int) bool {
	if v, ok := rc.limiters.Load(name); ok {
		if r, ok := v.(*RateLimiter); ok {
			return r.allowN(count)
		}
		return false
	}
	// Allow if there is no limiter setup for the given key
	return true
}

// setBurst is for updating the burst after creating the named limiter
func (rc *RateLimiterCollection) setBurst(name string, burst int) bool {
	if v, ok := rc.limiters.Load(name); ok {
		if r, ok := v.(*RateLimiter); ok {
			r.setBurst(burst)
			return true
		}
		return false
	}
	// Allow if there is no limiter setup for the given key
	return true
}

// setRate is for updating the rate after creating the named limiter
func (rc *RateLimiterCollection) setRate(name string, rate float64) bool {
	if v, ok := rc.limiters.Load(name); ok {
		if r, ok := v.(*RateLimiter); ok {
			r.setRate(rate)
			return true
		}
		return false
	}
	// Allow if there is no limiter setup for the given key
	return true
}

// addRateLimiter adds or overrides (update) the named limiter
func (rc *RateLimiterCollection) addRateLimiter(name string, rate float64, burst int) {
	rc.limiters.Store(name, newRateLimiter(rate, burst))
}
