//{C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package mdns

import (
	"context"
	"log"
	"strings"
	"sync"

	"github.com/grandcat/zeroconf"

	"github.com/pensando/sw/api"
)

type browser struct {
	sync.RWMutex
	resolver  *zeroconf.Resolver
	service   string
	domain    string
	eventChan chan *DiscoveredNodeEvent
	ctx       context.Context
	entries   map[string]zeroconf.ServiceEntry
}

// NewBrowser starts browser to search network for service in a given domain
func NewBrowser(ctx context.Context, service, domain string, eventChan chan *DiscoveredNodeEvent) (Browser, error) {
	var err error
	var b = &browser{
		ctx:       ctx,
		service:   service,
		domain:    domain,
		entries:   make(map[string]zeroconf.ServiceEntry),
		eventChan: eventChan,
	}
	b.resolver, err = zeroconf.NewResolver(nil)
	if err != nil {
		log.Fatalf("Failed to initialize resolver with error: %v", err)
		return nil, err
	}

	servEntries := make(chan *zeroconf.ServiceEntry)

	go func(results <-chan *zeroconf.ServiceEntry) {
		for e := range results {
			b.Lock()
			if e.TTL == 0 {
				delete(b.entries, e.HostName)
			} else {
				b.entries[e.HostName] = *e
			}
			b.Unlock()

			if b.eventChan != nil {
				node := nodeFromServiceEntry(e)
				ne := &DiscoveredNodeEvent{DiscoveredNode: *node}
				// TODO: TTL handling is not being properly done upstream. push the proper changes upstream
				if e.TTL == 0 {
					ne.Type = Deleted
				} else {
					ne.Type = CreatedOrUpdated
				}
				b.eventChan <- ne
			}
		}
		if b.eventChan != nil {
			close(b.eventChan)
		}
	}(servEntries)

	err = b.resolver.Browse(b.ctx, b.service, b.domain, servEntries)
	if err != nil {
		log.Fatalf("Failed to browse with error: %v", err)
	}
	return b, err
}

// List returns list of discovered nodes from the cache
func (b *browser) List() (*DiscoveredNodeList, error) {
	pnl := DiscoveredNodeList{TypeMeta: api.TypeMeta{Kind: "DiscoveredNodeList"}}

	b.RLock()
	for _, v := range b.entries {
		node := nodeFromServiceEntry(&v)
		pnl.Items = append(pnl.Items, *node)
	}
	b.RUnlock()
	return &pnl, nil
}

func nodeFromServiceEntry(s *zeroconf.ServiceEntry) *DiscoveredNode {
	// this is because the dns package escapes the string
	r := strings.NewReplacer("\\\\", "\\", "\\t", "\t", "\\r", "\r", "\\n", "\n", "\\\"", "\"")

	pn := DiscoveredNode{
		TypeMeta:   api.TypeMeta{Kind: "DiscoveredNode"},
		ObjectMeta: api.ObjectMeta{Name: strings.Split(s.ServiceInstanceName(), ".")[0]},
		Data:       make(map[string]string),
	}
	for _, trv := range s.Text {
		txt := strings.SplitN(trv, "=", 2)
		pn.Data[txt[0]] = r.Replace(txt[1])
	}
	return &pn
}
