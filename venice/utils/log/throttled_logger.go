// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
//
// This file contains an implementation of a logger paired with a per-message rate-limiter

package log

import (
	"fmt"
	"sync"
	"time"

	kitlog "github.com/go-kit/kit/log"
	"github.com/go-logfmt/logfmt"
)

var (
	defaultLogThrottleThreshold     = 30
	defaultLogThrottleFlushInterval = 30 * time.Second
	defaultMaxLogMsgKeyLength       = 50
)

type messageThrottle struct {
	hits uint64
	kv   []interface{}
}

type throttledLogger struct {
	sync.Mutex
	logger kitlog.Logger
	config ThrottleConfig
	msgMap map[string]*messageThrottle
}

func (l *throttledLogger) Log(keyvals ...interface{}) error {
	// compute key
	l.Lock()
	defer l.Unlock()
	if l.msgMap == nil {
		return fmt.Errorf("Logger not running")
	}
	key, err := l.config.GetKeyFn(keyvals...)
	if err != nil {
		// log "as-is" and return err
		l.logger.Log(keyvals...)
		return fmt.Errorf("Error marshaling %+v", keyvals...)
	}
	mt := l.msgMap[string(key)]
	if mt == nil {
		mt = new(messageThrottle)
		l.msgMap[string(key)] = mt
	}
	mt.hits++
	if mt.hits <= l.config.ThrottleThreshold {
		// below threshold, log and return
		l.logger.Log(keyvals...)
		return nil
	}
	// above threshold, copy over the KV pairs (once) so that we can reconstruct
	// the summarized message when we flush
	if mt.kv == nil {
		for _, kv := range keyvals {
			mt.kv = append(mt.kv, kv)
		}
	}
	return nil
}

func (l *throttledLogger) periodicFlush() {
	t := time.NewTicker(l.config.FlushInterval)
	for {
		select {
		case <-t.C:
			l.Lock()
			for k, mt := range l.msgMap {
				if mt.hits > l.config.ThrottleThreshold {
					kitlog.With(l.logger,
						"throttle_hit_count", fmt.Sprintf("%d", mt.hits-l.config.ThrottleThreshold),
						"throttle_key_len", fmt.Sprintf("%d", len(k)),
						"throttle_period", fmt.Sprintf("%v", l.config.FlushInterval),
					).Log(mt.kv...)
				}
			}
			// clear the map
			l.msgMap = make(map[string]*messageThrottle, 0)
			l.Unlock()
		}
	}
}

func newThrottledLogger(c *ThrottleConfig, l kitlog.Logger) *throttledLogger {
	r := &throttledLogger{
		logger: l,
		config: *c,
		msgMap: make(map[string]*messageThrottle, 0),
	}
	if c.FlushInterval == 0 {
		fmt.Printf("Throttled logger config cannot have FlushInterval = 0")
		return nil
	}
	if c.GetKeyFn == nil {
		fmt.Printf("Throttled logger config cannot have nil GetKeyFn")
		return nil
	}
	go r.periodicFlush()
	return r
}

// GetDefaultThrottledLoggerConfig returns the default configuration for a logger with message throttling
// given a base logger config.
func GetDefaultThrottledLoggerConfig(base Config) *Config {
	// When throttling, only consider the first maxLogMsgKeyLength  bytes of each message (including level).
	// This is a quick and easy way to get rid of remote client params (IP, port, etc.) that could vary
	// across connections and prevent throttling. If this turns out to be too crude, we can use some more
	//  sophisticated (and expensive) mechanism like regex to zero them.
	getKeyFn := func(kvs ...interface{}) (string, error) {
		kb, err := logfmt.MarshalKeyvals(kvs...)
		ks := string(kb)
		if len(ks) > defaultMaxLogMsgKeyLength {
			return ks[:defaultMaxLogMsgKeyLength], err
		}
		return ks, err
	}

	base.ThrottleCfg = &ThrottleConfig{
		ThrottleThreshold: uint64(defaultLogThrottleThreshold),
		FlushInterval:     defaultLogThrottleFlushInterval,
		GetKeyFn:          getKeyFn,
	}
	return &base
}
