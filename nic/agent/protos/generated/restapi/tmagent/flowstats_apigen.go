// Code generated by protoc-gen-grpc-pensando DO NOT EDIT.
/*
 * Package restapi is a auto generated package.
 * Input file: flowstats.proto
 */

package restapi

import (
	"fmt"
	"net/http"

	"github.com/fatih/structs"
	"github.com/gorilla/mux"

	"github.com/pensando/sw/nic/agent/httputils"
	"github.com/pensando/sw/nic/delphi/proto/goproto"
	_ "github.com/pensando/sw/nic/utils/ntranslate/flowstats"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/tsdb"
)

// AddIPv4FlowBehavioralMetricsAPIRoutes adds routes for IPv4FlowBehavioralMetrics
func (s *RestServer) AddIPv4FlowBehavioralMetricsAPIRoutes(r *mux.Router) {
	r.Methods("GET").Subrouter().HandleFunc("/{Meta.Tenant}/{Meta.Name}/", httputils.MakeHTTPHandler(s.getIPv4FlowBehavioralMetricsHandler))
	r.Methods("GET").Subrouter().HandleFunc("/", httputils.MakeHTTPHandler(s.listIPv4FlowBehavioralMetricsHandler))
}

// listIPv4FlowBehavioralMetricsHandler is the List Handler for IPv4FlowBehavioralMetrics
func (s *RestServer) listIPv4FlowBehavioralMetricsHandler(r *http.Request) (interface{}, error) {
	iter, err := goproto.NewIPv4FlowBehavioralMetricsIterator()
	if err != nil {
		return nil, fmt.Errorf("failed to get metrics, error: %s", err)
	}

	// for OSX tests
	if iter == nil {
		return nil, nil
	}

	var mtr []goproto.IPv4FlowBehavioralMetrics

	for iter.HasNext() {
		temp := iter.Next()
		if temp == nil {
			continue
		}

		objMeta := s.GetObjectMeta("IPv4FlowBehavioralMetricsKey", temp.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for IPv4FlowBehavioralMetrics key %+v", temp.GetKey())
			continue
		}

		temp.ObjectMeta = *objMeta
		mtr = append(mtr, *temp)
	}
	iter.Free()
	return mtr, nil
}

// getIPv4FlowBehavioralMetricsPoints returns tags and fields to save in Venice TSDB
func (s *RestServer) getIPv4FlowBehavioralMetricsPoints() ([]*tsdb.Point, error) {

	iter, err := goproto.NewIPv4FlowBehavioralMetricsIterator()
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
		objMeta := s.GetObjectMeta("IPv4FlowBehavioralMetricsKey", m.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for IPv4FlowBehavioralMetrics key %+v", m.GetKey())
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

// getIPv4FlowBehavioralMetricsHandler is the Get Handler for IPv4FlowBehavioralMetrics
func (s *RestServer) getIPv4FlowBehavioralMetricsHandler(r *http.Request) (interface{}, error) {
	log.Infof("Got GET request IPv4FlowBehavioralMetrics/%s", mux.Vars(r)["Meta.Name"])
	return nil, nil
}

// AddIPv4FlowDropMetricsAPIRoutes adds routes for IPv4FlowDropMetrics
func (s *RestServer) AddIPv4FlowDropMetricsAPIRoutes(r *mux.Router) {
	r.Methods("GET").Subrouter().HandleFunc("/{Meta.Tenant}/{Meta.Name}/", httputils.MakeHTTPHandler(s.getIPv4FlowDropMetricsHandler))
	r.Methods("GET").Subrouter().HandleFunc("/", httputils.MakeHTTPHandler(s.listIPv4FlowDropMetricsHandler))
}

// listIPv4FlowDropMetricsHandler is the List Handler for IPv4FlowDropMetrics
func (s *RestServer) listIPv4FlowDropMetricsHandler(r *http.Request) (interface{}, error) {
	iter, err := goproto.NewIPv4FlowDropMetricsIterator()
	if err != nil {
		return nil, fmt.Errorf("failed to get metrics, error: %s", err)
	}

	// for OSX tests
	if iter == nil {
		return nil, nil
	}

	var mtr []goproto.IPv4FlowDropMetrics

	for iter.HasNext() {
		temp := iter.Next()
		if temp == nil {
			continue
		}

		objMeta := s.GetObjectMeta("IPv4FlowDropMetricsKey", temp.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for IPv4FlowDropMetrics key %+v", temp.GetKey())
			continue
		}

		temp.ObjectMeta = *objMeta
		mtr = append(mtr, *temp)
	}
	iter.Free()
	return mtr, nil
}

// getIPv4FlowDropMetricsPoints returns tags and fields to save in Venice TSDB
func (s *RestServer) getIPv4FlowDropMetricsPoints() ([]*tsdb.Point, error) {

	iter, err := goproto.NewIPv4FlowDropMetricsIterator()
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
		objMeta := s.GetObjectMeta("IPv4FlowDropMetricsKey", m.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for IPv4FlowDropMetrics key %+v", m.GetKey())
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

// getIPv4FlowDropMetricsHandler is the Get Handler for IPv4FlowDropMetrics
func (s *RestServer) getIPv4FlowDropMetricsHandler(r *http.Request) (interface{}, error) {
	log.Infof("Got GET request IPv4FlowDropMetrics/%s", mux.Vars(r)["Meta.Name"])
	return nil, nil
}

// AddIPv4FlowLatencyMetricsAPIRoutes adds routes for IPv4FlowLatencyMetrics
func (s *RestServer) AddIPv4FlowLatencyMetricsAPIRoutes(r *mux.Router) {
	r.Methods("GET").Subrouter().HandleFunc("/{Meta.Tenant}/{Meta.Name}/", httputils.MakeHTTPHandler(s.getIPv4FlowLatencyMetricsHandler))
	r.Methods("GET").Subrouter().HandleFunc("/", httputils.MakeHTTPHandler(s.listIPv4FlowLatencyMetricsHandler))
}

// listIPv4FlowLatencyMetricsHandler is the List Handler for IPv4FlowLatencyMetrics
func (s *RestServer) listIPv4FlowLatencyMetricsHandler(r *http.Request) (interface{}, error) {
	iter, err := goproto.NewIPv4FlowLatencyMetricsIterator()
	if err != nil {
		return nil, fmt.Errorf("failed to get metrics, error: %s", err)
	}

	// for OSX tests
	if iter == nil {
		return nil, nil
	}

	var mtr []goproto.IPv4FlowLatencyMetrics

	for iter.HasNext() {
		temp := iter.Next()
		if temp == nil {
			continue
		}

		objMeta := s.GetObjectMeta("IPv4FlowLatencyMetricsKey", temp.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for IPv4FlowLatencyMetrics key %+v", temp.GetKey())
			continue
		}

		temp.ObjectMeta = *objMeta
		mtr = append(mtr, *temp)
	}
	iter.Free()
	return mtr, nil
}

// getIPv4FlowLatencyMetricsPoints returns tags and fields to save in Venice TSDB
func (s *RestServer) getIPv4FlowLatencyMetricsPoints() ([]*tsdb.Point, error) {

	iter, err := goproto.NewIPv4FlowLatencyMetricsIterator()
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
		objMeta := s.GetObjectMeta("IPv4FlowLatencyMetricsKey", m.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for IPv4FlowLatencyMetrics key %+v", m.GetKey())
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

// getIPv4FlowLatencyMetricsHandler is the Get Handler for IPv4FlowLatencyMetrics
func (s *RestServer) getIPv4FlowLatencyMetricsHandler(r *http.Request) (interface{}, error) {
	log.Infof("Got GET request IPv4FlowLatencyMetrics/%s", mux.Vars(r)["Meta.Name"])
	return nil, nil
}

// AddIPv4FlowPerformanceMetricsAPIRoutes adds routes for IPv4FlowPerformanceMetrics
func (s *RestServer) AddIPv4FlowPerformanceMetricsAPIRoutes(r *mux.Router) {
	r.Methods("GET").Subrouter().HandleFunc("/{Meta.Tenant}/{Meta.Name}/", httputils.MakeHTTPHandler(s.getIPv4FlowPerformanceMetricsHandler))
	r.Methods("GET").Subrouter().HandleFunc("/", httputils.MakeHTTPHandler(s.listIPv4FlowPerformanceMetricsHandler))
}

// listIPv4FlowPerformanceMetricsHandler is the List Handler for IPv4FlowPerformanceMetrics
func (s *RestServer) listIPv4FlowPerformanceMetricsHandler(r *http.Request) (interface{}, error) {
	iter, err := goproto.NewIPv4FlowPerformanceMetricsIterator()
	if err != nil {
		return nil, fmt.Errorf("failed to get metrics, error: %s", err)
	}

	// for OSX tests
	if iter == nil {
		return nil, nil
	}

	var mtr []goproto.IPv4FlowPerformanceMetrics

	for iter.HasNext() {
		temp := iter.Next()
		if temp == nil {
			continue
		}

		objMeta := s.GetObjectMeta("IPv4FlowPerformanceMetricsKey", temp.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for IPv4FlowPerformanceMetrics key %+v", temp.GetKey())
			continue
		}

		temp.ObjectMeta = *objMeta
		mtr = append(mtr, *temp)
	}
	iter.Free()
	return mtr, nil
}

// getIPv4FlowPerformanceMetricsPoints returns tags and fields to save in Venice TSDB
func (s *RestServer) getIPv4FlowPerformanceMetricsPoints() ([]*tsdb.Point, error) {

	iter, err := goproto.NewIPv4FlowPerformanceMetricsIterator()
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
		objMeta := s.GetObjectMeta("IPv4FlowPerformanceMetricsKey", m.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for IPv4FlowPerformanceMetrics key %+v", m.GetKey())
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

// getIPv4FlowPerformanceMetricsHandler is the Get Handler for IPv4FlowPerformanceMetrics
func (s *RestServer) getIPv4FlowPerformanceMetricsHandler(r *http.Request) (interface{}, error) {
	log.Infof("Got GET request IPv4FlowPerformanceMetrics/%s", mux.Vars(r)["Meta.Name"])
	return nil, nil
}

// AddIPv4FlowRawMetricsAPIRoutes adds routes for IPv4FlowRawMetrics
func (s *RestServer) AddIPv4FlowRawMetricsAPIRoutes(r *mux.Router) {
	r.Methods("GET").Subrouter().HandleFunc("/{Meta.Tenant}/{Meta.Name}/", httputils.MakeHTTPHandler(s.getIPv4FlowRawMetricsHandler))
	r.Methods("GET").Subrouter().HandleFunc("/", httputils.MakeHTTPHandler(s.listIPv4FlowRawMetricsHandler))
}

// listIPv4FlowRawMetricsHandler is the List Handler for IPv4FlowRawMetrics
func (s *RestServer) listIPv4FlowRawMetricsHandler(r *http.Request) (interface{}, error) {
	iter, err := goproto.NewIPv4FlowRawMetricsIterator()
	if err != nil {
		return nil, fmt.Errorf("failed to get metrics, error: %s", err)
	}

	// for OSX tests
	if iter == nil {
		return nil, nil
	}

	var mtr []goproto.IPv4FlowRawMetrics

	for iter.HasNext() {
		temp := iter.Next()
		if temp == nil {
			continue
		}

		objMeta := s.GetObjectMeta("IPv4FlowRawMetricsKey", temp.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for IPv4FlowRawMetrics key %+v", temp.GetKey())
			continue
		}

		temp.ObjectMeta = *objMeta
		mtr = append(mtr, *temp)
	}
	iter.Free()
	return mtr, nil
}

// getIPv4FlowRawMetricsPoints returns tags and fields to save in Venice TSDB
func (s *RestServer) getIPv4FlowRawMetricsPoints() ([]*tsdb.Point, error) {

	iter, err := goproto.NewIPv4FlowRawMetricsIterator()
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
		objMeta := s.GetObjectMeta("IPv4FlowRawMetricsKey", m.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for IPv4FlowRawMetrics key %+v", m.GetKey())
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

// getIPv4FlowRawMetricsHandler is the Get Handler for IPv4FlowRawMetrics
func (s *RestServer) getIPv4FlowRawMetricsHandler(r *http.Request) (interface{}, error) {
	log.Infof("Got GET request IPv4FlowRawMetrics/%s", mux.Vars(r)["Meta.Name"])
	return nil, nil
}

// AddIPv6FlowBehavioralMetricsAPIRoutes adds routes for IPv6FlowBehavioralMetrics
func (s *RestServer) AddIPv6FlowBehavioralMetricsAPIRoutes(r *mux.Router) {
	r.Methods("GET").Subrouter().HandleFunc("/{Meta.Tenant}/{Meta.Name}/", httputils.MakeHTTPHandler(s.getIPv6FlowBehavioralMetricsHandler))
	r.Methods("GET").Subrouter().HandleFunc("/", httputils.MakeHTTPHandler(s.listIPv6FlowBehavioralMetricsHandler))
}

// listIPv6FlowBehavioralMetricsHandler is the List Handler for IPv6FlowBehavioralMetrics
func (s *RestServer) listIPv6FlowBehavioralMetricsHandler(r *http.Request) (interface{}, error) {
	iter, err := goproto.NewIPv6FlowBehavioralMetricsIterator()
	if err != nil {
		return nil, fmt.Errorf("failed to get metrics, error: %s", err)
	}

	// for OSX tests
	if iter == nil {
		return nil, nil
	}

	var mtr []goproto.IPv6FlowBehavioralMetrics

	for iter.HasNext() {
		temp := iter.Next()
		if temp == nil {
			continue
		}

		objMeta := s.GetObjectMeta("IPv6FlowBehavioralMetricsKey", temp.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for IPv6FlowBehavioralMetrics key %+v", temp.GetKey())
			continue
		}

		temp.ObjectMeta = *objMeta
		mtr = append(mtr, *temp)
	}
	iter.Free()
	return mtr, nil
}

// getIPv6FlowBehavioralMetricsPoints returns tags and fields to save in Venice TSDB
func (s *RestServer) getIPv6FlowBehavioralMetricsPoints() ([]*tsdb.Point, error) {

	iter, err := goproto.NewIPv6FlowBehavioralMetricsIterator()
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
		objMeta := s.GetObjectMeta("IPv6FlowBehavioralMetricsKey", m.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for IPv6FlowBehavioralMetrics key %+v", m.GetKey())
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

// getIPv6FlowBehavioralMetricsHandler is the Get Handler for IPv6FlowBehavioralMetrics
func (s *RestServer) getIPv6FlowBehavioralMetricsHandler(r *http.Request) (interface{}, error) {
	log.Infof("Got GET request IPv6FlowBehavioralMetrics/%s", mux.Vars(r)["Meta.Name"])
	return nil, nil
}

// AddIPv6FlowDropMetricsAPIRoutes adds routes for IPv6FlowDropMetrics
func (s *RestServer) AddIPv6FlowDropMetricsAPIRoutes(r *mux.Router) {
	r.Methods("GET").Subrouter().HandleFunc("/{Meta.Tenant}/{Meta.Name}/", httputils.MakeHTTPHandler(s.getIPv6FlowDropMetricsHandler))
	r.Methods("GET").Subrouter().HandleFunc("/", httputils.MakeHTTPHandler(s.listIPv6FlowDropMetricsHandler))
}

// listIPv6FlowDropMetricsHandler is the List Handler for IPv6FlowDropMetrics
func (s *RestServer) listIPv6FlowDropMetricsHandler(r *http.Request) (interface{}, error) {
	iter, err := goproto.NewIPv6FlowDropMetricsIterator()
	if err != nil {
		return nil, fmt.Errorf("failed to get metrics, error: %s", err)
	}

	// for OSX tests
	if iter == nil {
		return nil, nil
	}

	var mtr []goproto.IPv6FlowDropMetrics

	for iter.HasNext() {
		temp := iter.Next()
		if temp == nil {
			continue
		}

		objMeta := s.GetObjectMeta("IPv6FlowDropMetricsKey", temp.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for IPv6FlowDropMetrics key %+v", temp.GetKey())
			continue
		}

		temp.ObjectMeta = *objMeta
		mtr = append(mtr, *temp)
	}
	iter.Free()
	return mtr, nil
}

// getIPv6FlowDropMetricsPoints returns tags and fields to save in Venice TSDB
func (s *RestServer) getIPv6FlowDropMetricsPoints() ([]*tsdb.Point, error) {

	iter, err := goproto.NewIPv6FlowDropMetricsIterator()
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
		objMeta := s.GetObjectMeta("IPv6FlowDropMetricsKey", m.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for IPv6FlowDropMetrics key %+v", m.GetKey())
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

// getIPv6FlowDropMetricsHandler is the Get Handler for IPv6FlowDropMetrics
func (s *RestServer) getIPv6FlowDropMetricsHandler(r *http.Request) (interface{}, error) {
	log.Infof("Got GET request IPv6FlowDropMetrics/%s", mux.Vars(r)["Meta.Name"])
	return nil, nil
}

// AddIPv6FlowLatencyMetricsAPIRoutes adds routes for IPv6FlowLatencyMetrics
func (s *RestServer) AddIPv6FlowLatencyMetricsAPIRoutes(r *mux.Router) {
	r.Methods("GET").Subrouter().HandleFunc("/{Meta.Tenant}/{Meta.Name}/", httputils.MakeHTTPHandler(s.getIPv6FlowLatencyMetricsHandler))
	r.Methods("GET").Subrouter().HandleFunc("/", httputils.MakeHTTPHandler(s.listIPv6FlowLatencyMetricsHandler))
}

// listIPv6FlowLatencyMetricsHandler is the List Handler for IPv6FlowLatencyMetrics
func (s *RestServer) listIPv6FlowLatencyMetricsHandler(r *http.Request) (interface{}, error) {
	iter, err := goproto.NewIPv6FlowLatencyMetricsIterator()
	if err != nil {
		return nil, fmt.Errorf("failed to get metrics, error: %s", err)
	}

	// for OSX tests
	if iter == nil {
		return nil, nil
	}

	var mtr []goproto.IPv6FlowLatencyMetrics

	for iter.HasNext() {
		temp := iter.Next()
		if temp == nil {
			continue
		}

		objMeta := s.GetObjectMeta("IPv6FlowLatencyMetricsKey", temp.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for IPv6FlowLatencyMetrics key %+v", temp.GetKey())
			continue
		}

		temp.ObjectMeta = *objMeta
		mtr = append(mtr, *temp)
	}
	iter.Free()
	return mtr, nil
}

// getIPv6FlowLatencyMetricsPoints returns tags and fields to save in Venice TSDB
func (s *RestServer) getIPv6FlowLatencyMetricsPoints() ([]*tsdb.Point, error) {

	iter, err := goproto.NewIPv6FlowLatencyMetricsIterator()
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
		objMeta := s.GetObjectMeta("IPv6FlowLatencyMetricsKey", m.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for IPv6FlowLatencyMetrics key %+v", m.GetKey())
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

// getIPv6FlowLatencyMetricsHandler is the Get Handler for IPv6FlowLatencyMetrics
func (s *RestServer) getIPv6FlowLatencyMetricsHandler(r *http.Request) (interface{}, error) {
	log.Infof("Got GET request IPv6FlowLatencyMetrics/%s", mux.Vars(r)["Meta.Name"])
	return nil, nil
}

// AddIPv6FlowPerformanceMetricsAPIRoutes adds routes for IPv6FlowPerformanceMetrics
func (s *RestServer) AddIPv6FlowPerformanceMetricsAPIRoutes(r *mux.Router) {
	r.Methods("GET").Subrouter().HandleFunc("/{Meta.Tenant}/{Meta.Name}/", httputils.MakeHTTPHandler(s.getIPv6FlowPerformanceMetricsHandler))
	r.Methods("GET").Subrouter().HandleFunc("/", httputils.MakeHTTPHandler(s.listIPv6FlowPerformanceMetricsHandler))
}

// listIPv6FlowPerformanceMetricsHandler is the List Handler for IPv6FlowPerformanceMetrics
func (s *RestServer) listIPv6FlowPerformanceMetricsHandler(r *http.Request) (interface{}, error) {
	iter, err := goproto.NewIPv6FlowPerformanceMetricsIterator()
	if err != nil {
		return nil, fmt.Errorf("failed to get metrics, error: %s", err)
	}

	// for OSX tests
	if iter == nil {
		return nil, nil
	}

	var mtr []goproto.IPv6FlowPerformanceMetrics

	for iter.HasNext() {
		temp := iter.Next()
		if temp == nil {
			continue
		}

		objMeta := s.GetObjectMeta("IPv6FlowPerformanceMetricsKey", temp.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for IPv6FlowPerformanceMetrics key %+v", temp.GetKey())
			continue
		}

		temp.ObjectMeta = *objMeta
		mtr = append(mtr, *temp)
	}
	iter.Free()
	return mtr, nil
}

// getIPv6FlowPerformanceMetricsPoints returns tags and fields to save in Venice TSDB
func (s *RestServer) getIPv6FlowPerformanceMetricsPoints() ([]*tsdb.Point, error) {

	iter, err := goproto.NewIPv6FlowPerformanceMetricsIterator()
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
		objMeta := s.GetObjectMeta("IPv6FlowPerformanceMetricsKey", m.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for IPv6FlowPerformanceMetrics key %+v", m.GetKey())
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

// getIPv6FlowPerformanceMetricsHandler is the Get Handler for IPv6FlowPerformanceMetrics
func (s *RestServer) getIPv6FlowPerformanceMetricsHandler(r *http.Request) (interface{}, error) {
	log.Infof("Got GET request IPv6FlowPerformanceMetrics/%s", mux.Vars(r)["Meta.Name"])
	return nil, nil
}

// AddIPv6FlowRawMetricsAPIRoutes adds routes for IPv6FlowRawMetrics
func (s *RestServer) AddIPv6FlowRawMetricsAPIRoutes(r *mux.Router) {
	r.Methods("GET").Subrouter().HandleFunc("/{Meta.Tenant}/{Meta.Name}/", httputils.MakeHTTPHandler(s.getIPv6FlowRawMetricsHandler))
	r.Methods("GET").Subrouter().HandleFunc("/", httputils.MakeHTTPHandler(s.listIPv6FlowRawMetricsHandler))
}

// listIPv6FlowRawMetricsHandler is the List Handler for IPv6FlowRawMetrics
func (s *RestServer) listIPv6FlowRawMetricsHandler(r *http.Request) (interface{}, error) {
	iter, err := goproto.NewIPv6FlowRawMetricsIterator()
	if err != nil {
		return nil, fmt.Errorf("failed to get metrics, error: %s", err)
	}

	// for OSX tests
	if iter == nil {
		return nil, nil
	}

	var mtr []goproto.IPv6FlowRawMetrics

	for iter.HasNext() {
		temp := iter.Next()
		if temp == nil {
			continue
		}

		objMeta := s.GetObjectMeta("IPv6FlowRawMetricsKey", temp.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for IPv6FlowRawMetrics key %+v", temp.GetKey())
			continue
		}

		temp.ObjectMeta = *objMeta
		mtr = append(mtr, *temp)
	}
	iter.Free()
	return mtr, nil
}

// getIPv6FlowRawMetricsPoints returns tags and fields to save in Venice TSDB
func (s *RestServer) getIPv6FlowRawMetricsPoints() ([]*tsdb.Point, error) {

	iter, err := goproto.NewIPv6FlowRawMetricsIterator()
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
		objMeta := s.GetObjectMeta("IPv6FlowRawMetricsKey", m.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for IPv6FlowRawMetrics key %+v", m.GetKey())
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

// getIPv6FlowRawMetricsHandler is the Get Handler for IPv6FlowRawMetrics
func (s *RestServer) getIPv6FlowRawMetricsHandler(r *http.Request) (interface{}, error) {
	log.Infof("Got GET request IPv6FlowRawMetrics/%s", mux.Vars(r)["Meta.Name"])
	return nil, nil
}

// AddL2FlowBehavioralMetricsAPIRoutes adds routes for L2FlowBehavioralMetrics
func (s *RestServer) AddL2FlowBehavioralMetricsAPIRoutes(r *mux.Router) {
	r.Methods("GET").Subrouter().HandleFunc("/{Meta.Tenant}/{Meta.Name}/", httputils.MakeHTTPHandler(s.getL2FlowBehavioralMetricsHandler))
	r.Methods("GET").Subrouter().HandleFunc("/", httputils.MakeHTTPHandler(s.listL2FlowBehavioralMetricsHandler))
}

// listL2FlowBehavioralMetricsHandler is the List Handler for L2FlowBehavioralMetrics
func (s *RestServer) listL2FlowBehavioralMetricsHandler(r *http.Request) (interface{}, error) {
	iter, err := goproto.NewL2FlowBehavioralMetricsIterator()
	if err != nil {
		return nil, fmt.Errorf("failed to get metrics, error: %s", err)
	}

	// for OSX tests
	if iter == nil {
		return nil, nil
	}

	var mtr []goproto.L2FlowBehavioralMetrics

	for iter.HasNext() {
		temp := iter.Next()
		if temp == nil {
			continue
		}

		objMeta := s.GetObjectMeta("L2FlowBehavioralMetricsKey", temp.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for L2FlowBehavioralMetrics key %+v", temp.GetKey())
			continue
		}

		temp.ObjectMeta = *objMeta
		mtr = append(mtr, *temp)
	}
	iter.Free()
	return mtr, nil
}

// getL2FlowBehavioralMetricsPoints returns tags and fields to save in Venice TSDB
func (s *RestServer) getL2FlowBehavioralMetricsPoints() ([]*tsdb.Point, error) {

	iter, err := goproto.NewL2FlowBehavioralMetricsIterator()
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
		objMeta := s.GetObjectMeta("L2FlowBehavioralMetricsKey", m.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for L2FlowBehavioralMetrics key %+v", m.GetKey())
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

// getL2FlowBehavioralMetricsHandler is the Get Handler for L2FlowBehavioralMetrics
func (s *RestServer) getL2FlowBehavioralMetricsHandler(r *http.Request) (interface{}, error) {
	log.Infof("Got GET request L2FlowBehavioralMetrics/%s", mux.Vars(r)["Meta.Name"])
	return nil, nil
}

// AddL2FlowDropMetricsAPIRoutes adds routes for L2FlowDropMetrics
func (s *RestServer) AddL2FlowDropMetricsAPIRoutes(r *mux.Router) {
	r.Methods("GET").Subrouter().HandleFunc("/{Meta.Tenant}/{Meta.Name}/", httputils.MakeHTTPHandler(s.getL2FlowDropMetricsHandler))
	r.Methods("GET").Subrouter().HandleFunc("/", httputils.MakeHTTPHandler(s.listL2FlowDropMetricsHandler))
}

// listL2FlowDropMetricsHandler is the List Handler for L2FlowDropMetrics
func (s *RestServer) listL2FlowDropMetricsHandler(r *http.Request) (interface{}, error) {
	iter, err := goproto.NewL2FlowDropMetricsIterator()
	if err != nil {
		return nil, fmt.Errorf("failed to get metrics, error: %s", err)
	}

	// for OSX tests
	if iter == nil {
		return nil, nil
	}

	var mtr []goproto.L2FlowDropMetrics

	for iter.HasNext() {
		temp := iter.Next()
		if temp == nil {
			continue
		}

		objMeta := s.GetObjectMeta("L2FlowDropMetricsKey", temp.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for L2FlowDropMetrics key %+v", temp.GetKey())
			continue
		}

		temp.ObjectMeta = *objMeta
		mtr = append(mtr, *temp)
	}
	iter.Free()
	return mtr, nil
}

// getL2FlowDropMetricsPoints returns tags and fields to save in Venice TSDB
func (s *RestServer) getL2FlowDropMetricsPoints() ([]*tsdb.Point, error) {

	iter, err := goproto.NewL2FlowDropMetricsIterator()
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
		objMeta := s.GetObjectMeta("L2FlowDropMetricsKey", m.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for L2FlowDropMetrics key %+v", m.GetKey())
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

// getL2FlowDropMetricsHandler is the Get Handler for L2FlowDropMetrics
func (s *RestServer) getL2FlowDropMetricsHandler(r *http.Request) (interface{}, error) {
	log.Infof("Got GET request L2FlowDropMetrics/%s", mux.Vars(r)["Meta.Name"])
	return nil, nil
}

// AddL2FlowPerformanceMetricsAPIRoutes adds routes for L2FlowPerformanceMetrics
func (s *RestServer) AddL2FlowPerformanceMetricsAPIRoutes(r *mux.Router) {
	r.Methods("GET").Subrouter().HandleFunc("/{Meta.Tenant}/{Meta.Name}/", httputils.MakeHTTPHandler(s.getL2FlowPerformanceMetricsHandler))
	r.Methods("GET").Subrouter().HandleFunc("/", httputils.MakeHTTPHandler(s.listL2FlowPerformanceMetricsHandler))
}

// listL2FlowPerformanceMetricsHandler is the List Handler for L2FlowPerformanceMetrics
func (s *RestServer) listL2FlowPerformanceMetricsHandler(r *http.Request) (interface{}, error) {
	iter, err := goproto.NewL2FlowPerformanceMetricsIterator()
	if err != nil {
		return nil, fmt.Errorf("failed to get metrics, error: %s", err)
	}

	// for OSX tests
	if iter == nil {
		return nil, nil
	}

	var mtr []goproto.L2FlowPerformanceMetrics

	for iter.HasNext() {
		temp := iter.Next()
		if temp == nil {
			continue
		}

		objMeta := s.GetObjectMeta("L2FlowPerformanceMetricsKey", temp.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for L2FlowPerformanceMetrics key %+v", temp.GetKey())
			continue
		}

		temp.ObjectMeta = *objMeta
		mtr = append(mtr, *temp)
	}
	iter.Free()
	return mtr, nil
}

// getL2FlowPerformanceMetricsPoints returns tags and fields to save in Venice TSDB
func (s *RestServer) getL2FlowPerformanceMetricsPoints() ([]*tsdb.Point, error) {

	iter, err := goproto.NewL2FlowPerformanceMetricsIterator()
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
		objMeta := s.GetObjectMeta("L2FlowPerformanceMetricsKey", m.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for L2FlowPerformanceMetrics key %+v", m.GetKey())
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

// getL2FlowPerformanceMetricsHandler is the Get Handler for L2FlowPerformanceMetrics
func (s *RestServer) getL2FlowPerformanceMetricsHandler(r *http.Request) (interface{}, error) {
	log.Infof("Got GET request L2FlowPerformanceMetrics/%s", mux.Vars(r)["Meta.Name"])
	return nil, nil
}

// AddL2FlowRawMetricsAPIRoutes adds routes for L2FlowRawMetrics
func (s *RestServer) AddL2FlowRawMetricsAPIRoutes(r *mux.Router) {
	r.Methods("GET").Subrouter().HandleFunc("/{Meta.Tenant}/{Meta.Name}/", httputils.MakeHTTPHandler(s.getL2FlowRawMetricsHandler))
	r.Methods("GET").Subrouter().HandleFunc("/", httputils.MakeHTTPHandler(s.listL2FlowRawMetricsHandler))
}

// listL2FlowRawMetricsHandler is the List Handler for L2FlowRawMetrics
func (s *RestServer) listL2FlowRawMetricsHandler(r *http.Request) (interface{}, error) {
	iter, err := goproto.NewL2FlowRawMetricsIterator()
	if err != nil {
		return nil, fmt.Errorf("failed to get metrics, error: %s", err)
	}

	// for OSX tests
	if iter == nil {
		return nil, nil
	}

	var mtr []goproto.L2FlowRawMetrics

	for iter.HasNext() {
		temp := iter.Next()
		if temp == nil {
			continue
		}

		objMeta := s.GetObjectMeta("L2FlowRawMetricsKey", temp.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for L2FlowRawMetrics key %+v", temp.GetKey())
			continue
		}

		temp.ObjectMeta = *objMeta
		mtr = append(mtr, *temp)
	}
	iter.Free()
	return mtr, nil
}

// getL2FlowRawMetricsPoints returns tags and fields to save in Venice TSDB
func (s *RestServer) getL2FlowRawMetricsPoints() ([]*tsdb.Point, error) {

	iter, err := goproto.NewL2FlowRawMetricsIterator()
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
		objMeta := s.GetObjectMeta("L2FlowRawMetricsKey", m.GetKey())
		if objMeta == nil {
			log.Errorf("failed to get objMeta for L2FlowRawMetrics key %+v", m.GetKey())
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

// getL2FlowRawMetricsHandler is the Get Handler for L2FlowRawMetrics
func (s *RestServer) getL2FlowRawMetricsHandler(r *http.Request) (interface{}, error) {
	log.Infof("Got GET request L2FlowRawMetrics/%s", mux.Vars(r)["Meta.Name"])
	return nil, nil
}