// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.
/*
 * Package restapi is a auto generated package.
 * Input file: ftestats.proto
 */

package restapi

import (
	"fmt"
	"net/http"

	"github.com/fatih/structs"
	"github.com/gorilla/mux"

	"github.com/pensando/sw/nic/agent/httputils"
	"github.com/pensando/sw/nic/delphi/proto/goproto"
	_ "github.com/pensando/sw/nic/utils/ntranslate/ftestats"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/tsdb"
)

// AddFteCPSMetricsAPIRoutes adds routes for FteCPSMetrics
func (s *RestServer) AddFteCPSMetricsAPIRoutes(r *mux.Router) {
	r.Methods("GET").Subrouter().HandleFunc("/{Meta.Tenant}/{Meta.Name}/", httputils.MakeHTTPHandler(s.getFteCPSMetricsHandler))
	r.Methods("GET").Subrouter().HandleFunc("/", httputils.MakeHTTPHandler(s.listFteCPSMetricsHandler))
}

// listFteCPSMetricsHandler is the List Handler for FteCPSMetrics
func (s *RestServer) listFteCPSMetricsHandler(r *http.Request) (interface{}, error) {
	iter, err := goproto.NewFteCPSMetricsIterator()
	if err != nil {
		return nil, fmt.Errorf("failed to get metrics, error: %s", err)
	}

	// for OSX tests
	if iter == nil {
		return nil, nil
	}

	var mtr []goproto.FteCPSMetrics

	for iter.HasNext() {
		temp := iter.Next()
		if temp == nil {
			continue
		}

		// Venice has no concept of FTE. Aggregate all per-fte stats to per-asic
		// since Iris pipeline has only one FTE, ignoring other FTE has same effect
		if temp.GetKey() != 0 {
			continue
		}

		objMeta := s.GetObjectMeta("FteCPSMetricsKey", temp.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for FteCPSMetrics key %+v", temp.GetKey())
			continue
		}

		temp.ObjectMeta = *objMeta
		mtr = append(mtr, *temp)
	}
	iter.Free()
	return mtr, nil
}

// getFteCPSMetricsPoints returns tags and fields to save in Venice TSDB
func (s *RestServer) getFteCPSMetricsPoints() ([]*tsdb.Point, error) {
	iter, err := goproto.NewFteCPSMetricsIterator()
	if err != nil {
		return nil, fmt.Errorf("failed to get metrics, error: %s", err)
	}

	// for OSX tests
	if iter == nil {
		return nil, nil
	}

	points := []*tsdb.Point{}

	for iter.HasNext() {
		m := iter.Next()
		if m == nil {
			continue
		}

		// translate key to meta
		objMeta := s.GetObjectMeta("FteCPSMetricsKey", m.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for FteCPSMetrics key %+v", m.GetKey())
			continue
		}
		tags := s.getTagsFromMeta(objMeta)
		fields := structs.Map(m)

		if len(fields) > 0 {
			delete(fields, "ObjectMeta")
			points = append(points, &tsdb.Point{Tags: tags, Fields: fields})
		}
	}

	iter.Free()
	return points, nil
}

// getFteCPSMetricsHandler is the Get Handler for FteCPSMetrics
func (s *RestServer) getFteCPSMetricsHandler(r *http.Request) (interface{}, error) {
	log.Infof("Got GET request FteCPSMetrics/%s", mux.Vars(r)["Meta.Name"])
	return nil, nil
}

// AddFteLifQMetricsAPIRoutes adds routes for FteLifQMetrics
func (s *RestServer) AddFteLifQMetricsAPIRoutes(r *mux.Router) {
	r.Methods("GET").Subrouter().HandleFunc("/{Meta.Tenant}/{Meta.Name}/", httputils.MakeHTTPHandler(s.getFteLifQMetricsHandler))
	r.Methods("GET").Subrouter().HandleFunc("/", httputils.MakeHTTPHandler(s.listFteLifQMetricsHandler))
}

// listFteLifQMetricsHandler is the List Handler for FteLifQMetrics
func (s *RestServer) listFteLifQMetricsHandler(r *http.Request) (interface{}, error) {
	iter, err := goproto.NewFteLifQMetricsIterator()
	if err != nil {
		return nil, fmt.Errorf("failed to get metrics, error: %s", err)
	}

	// for OSX tests
	if iter == nil {
		return nil, nil
	}

	var mtr []goproto.FteLifQMetrics

	for iter.HasNext() {
		temp := iter.Next()
		if temp == nil {
			continue
		}

		// Venice has no concept of FTE. Aggregate all per-fte stats to per-asic
		// since Iris pipeline has only one FTE, ignoring other FTE has same effect
		if temp.GetKey() != 0 {
			continue
		}

		objMeta := s.GetObjectMeta("FteLifQMetricsKey", temp.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for FteLifQMetrics key %+v", temp.GetKey())
			continue
		}

		temp.ObjectMeta = *objMeta
		mtr = append(mtr, *temp)
	}
	iter.Free()
	return mtr, nil
}

// getFteLifQMetricsPoints returns tags and fields to save in Venice TSDB
func (s *RestServer) getFteLifQMetricsPoints() ([]*tsdb.Point, error) {
	iter, err := goproto.NewFteLifQMetricsIterator()
	if err != nil {
		return nil, fmt.Errorf("failed to get metrics, error: %s", err)
	}

	// for OSX tests
	if iter == nil {
		return nil, nil
	}

	points := []*tsdb.Point{}

	for iter.HasNext() {
		m := iter.Next()
		if m == nil {
			continue
		}

		// translate key to meta
		objMeta := s.GetObjectMeta("FteLifQMetricsKey", m.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for FteLifQMetrics key %+v", m.GetKey())
			continue
		}
		tags := s.getTagsFromMeta(objMeta)
		fields := structs.Map(m)

		if len(fields) > 0 {
			delete(fields, "ObjectMeta")
			points = append(points, &tsdb.Point{Tags: tags, Fields: fields})
		}
	}

	iter.Free()
	return points, nil
}

// getFteLifQMetricsHandler is the Get Handler for FteLifQMetrics
func (s *RestServer) getFteLifQMetricsHandler(r *http.Request) (interface{}, error) {
	log.Infof("Got GET request FteLifQMetrics/%s", mux.Vars(r)["Meta.Name"])
	return nil, nil
}

// AddSessionSummaryMetricsAPIRoutes adds routes for SessionSummaryMetrics
func (s *RestServer) AddSessionSummaryMetricsAPIRoutes(r *mux.Router) {
	r.Methods("GET").Subrouter().HandleFunc("/{Meta.Tenant}/{Meta.Name}/", httputils.MakeHTTPHandler(s.getSessionSummaryMetricsHandler))
	r.Methods("GET").Subrouter().HandleFunc("/", httputils.MakeHTTPHandler(s.listSessionSummaryMetricsHandler))
}

// listSessionSummaryMetricsHandler is the List Handler for SessionSummaryMetrics
func (s *RestServer) listSessionSummaryMetricsHandler(r *http.Request) (interface{}, error) {
	iter, err := goproto.NewSessionSummaryMetricsIterator()
	if err != nil {
		return nil, fmt.Errorf("failed to get metrics, error: %s", err)
	}

	// for OSX tests
	if iter == nil {
		return nil, nil
	}

	var mtr []goproto.SessionSummaryMetrics

	for iter.HasNext() {
		temp := iter.Next()
		if temp == nil {
			continue
		}

		// Venice has no concept of FTE. Aggregate all per-fte stats to per-asic
		// since Iris pipeline has only one FTE, ignoring other FTE has same effect
		if temp.GetKey() != 0 {
			continue
		}

		objMeta := s.GetObjectMeta("SessionSummaryMetricsKey", temp.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for SessionSummaryMetrics key %+v", temp.GetKey())
			continue
		}

		temp.ObjectMeta = *objMeta
		mtr = append(mtr, *temp)
	}
	iter.Free()
	return mtr, nil
}

// getSessionSummaryMetricsPoints returns tags and fields to save in Venice TSDB
func (s *RestServer) getSessionSummaryMetricsPoints() ([]*tsdb.Point, error) {
	iter, err := goproto.NewSessionSummaryMetricsIterator()
	if err != nil {
		return nil, fmt.Errorf("failed to get metrics, error: %s", err)
	}

	// for OSX tests
	if iter == nil {
		return nil, nil
	}

	points := []*tsdb.Point{}

	for iter.HasNext() {
		m := iter.Next()
		if m == nil {
			continue
		}

		// translate key to meta
		objMeta := s.GetObjectMeta("SessionSummaryMetricsKey", m.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for SessionSummaryMetrics key %+v", m.GetKey())
			continue
		}
		tags := s.getTagsFromMeta(objMeta)
		fields := structs.Map(m)

		if len(fields) > 0 {
			delete(fields, "ObjectMeta")
			points = append(points, &tsdb.Point{Tags: tags, Fields: fields})
		}
	}

	iter.Free()
	return points, nil
}

// getSessionSummaryMetricsHandler is the Get Handler for SessionSummaryMetrics
func (s *RestServer) getSessionSummaryMetricsHandler(r *http.Request) (interface{}, error) {
	log.Infof("Got GET request SessionSummaryMetrics/%s", mux.Vars(r)["Meta.Name"])
	return nil, nil
}
