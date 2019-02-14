package impl

import (
	"container/list"
	"context"
	"errors"
	"fmt"
	"reflect"
	"strings"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/browser"
	"github.com/pensando/sw/api/graph"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

type browserHooks struct {
	svc     apiserver.Service
	logger  log.Logger
	graphDb graph.Interface
	apisrv  apiserver.Server
	cache   apiintf.CacheInterface
	version string
}

func mapType(in apiintf.ReferenceType) browser.ReferenceTypes {
	switch in {
	case apiintf.NamedReference:
		return browser.ReferenceTypes_NamedReference
	case apiintf.WeakReference:
		return browser.ReferenceTypes_WeakReference
	case apiintf.SelectorReference:
		return browser.ReferenceTypes_SelectorReference
	default:
		panic(fmt.Sprintf("Unknown reference type [%v]", in))
	}
}

func (r *browserHooks) makeRefererLink(in string) string {
	return fmt.Sprintf("/%s/%s/%s/dependedby/%s", globals.ConfigURIPrefix, globals.BrowserURIPrefix, r.version, strings.TrimPrefix(in, "/"+globals.ConfigURIPrefix+"/"))
}

func (r *browserHooks) makeReferenceLink(in string) string {
	return fmt.Sprintf("/%s/%s/%s/dependencies/%s", globals.ConfigURIPrefix, globals.BrowserURIPrefix, r.version, strings.TrimPrefix(in, "/"+globals.ConfigURIPrefix+"/"))
}

func (r *browserHooks) applySelector(ctx context.Context, tenant, kind, svcname, selector string, sch *runtime.Scheme) ([]api.ObjectRef, error) {
	apisrv := apisrvpkg.MustGetAPIServer()

	msg := apisrv.GetMessage(svcname, kind+"List")
	if msg == nil {
		return nil, fmt.Errorf("could not find message [%v/%v]", svcname, kind)
	}
	var ret []api.ObjectRef
	opts := api.ListWatchOptions{}
	opts.Tenant = tenant
	opts.LabelSelector = selector
	into, err := msg.ListFromKv(ctx, apisrvpkg.GetAPIServerCache(), &opts, svcname)
	if err != nil {
		log.Infof("ListFromKV returned error (%s)", err)
		return nil, err
	}
	if into == nil {
		return []api.ObjectRef{}, nil
	}
	items := reflect.ValueOf(into).FieldByName("Items")
	for i := 0; i < items.Len(); i++ {
		obj := items.Index(i).Interface().(runtime.Object)
		ref := api.ObjectRef{
			Kind: obj.GetObjectKind(),
		}
		ref.Tenant = tenant
		objm, err := runtime.GetObjectMeta(obj)
		if err != nil {
			return nil, err
		}
		ref.Name = objm.Name
		ref.Tenant = objm.Tenant
		ref.Namespace = objm.Namespace
		// only references are valid for selectors
		ref.URI = r.makeReferenceLink(sch.GetURI(objm.SelfLink, r.version))
		ret = append(ret, ref)
	}
	return ret, nil
}

func (r *browserHooks) makeObjectFromKey(key string, dir graph.Direction, sch *runtime.Scheme) (browser.Object, error) {
	ret := browser.Object{Links: make(map[string]browser.Object_URIs)}
	stat := r.cache.Stat(context.TODO(), []string{key})
	if len(stat) != 1 {
		return ret, errors.New("invalid object stat")
	}
	if stat[0].Valid {
		uri := sch.GetURI(key, r.version)
		if dir == graph.RefOut {
			ret.QueryType = browser.QueryType_Dependencies.String()
			ret.URI = r.makeReferenceLink(uri)
			ret.Reverse = r.makeRefererLink(uri)
		} else {
			ret.QueryType = browser.QueryType_DependedBy.String()
			ret.URI = r.makeRefererLink(uri)
			ret.Reverse = r.makeReferenceLink(uri)
		}
		ret.ObjectMeta = stat[0].ObjectMeta
		ret.TypeMeta = stat[0].TypeMeta
		ret.ObjectMeta.SelfLink = sch.GetURI(ret.ObjectMeta.SelfLink, r.version)
		return ret, nil
	}
	return ret, errors.New("object not found")
}

func (r *browserHooks) makeObjectFromNode(in *graph.Node, sch *runtime.Scheme) (browser.Object, error) {
	ret := browser.Object{Links: make(map[string]browser.Object_URIs)}

	if in == nil {
		return ret, errors.New("invalid object")
	}

	uri := sch.GetURI(in.This, r.version)
	if in.Dir == graph.RefOut {
		ret.QueryType = browser.QueryType_Dependencies.String()
		ret.URI = r.makeReferenceLink(uri)
		ret.Reverse = r.makeRefererLink(uri)
	} else {
		ret.QueryType = browser.QueryType_DependedBy.String()
		ret.URI = r.makeRefererLink(uri)
		ret.Reverse = r.makeReferenceLink(uri)
	}
	stat := r.cache.Stat(context.TODO(), []string{in.This})
	if len(stat) != 1 {
		return ret, errors.New("invalid object stat")
	}
	if !stat[0].Valid {
		return ret, errors.New("object not found")
	}
	ret.ObjectMeta = stat[0].ObjectMeta
	ret.TypeMeta = stat[0].TypeMeta
	ret.ObjectMeta.SelfLink = sch.GetURI(ret.ObjectMeta.SelfLink, r.version)
	for k, v := range in.Refs {
		uris := browser.Object_URIs{}
		uris.RefType = browser.ReferenceTypes_NamedReference.String()
		for _, v1 := range v {
			s1 := r.cache.Stat(context.TODO(), []string{v1})
			if s1[0].Valid {
				u := api.ObjectRef{
					Kind:      s1[0].TypeMeta.Kind,
					Namespace: s1[0].ObjectMeta.Namespace,
					Tenant:    s1[0].ObjectMeta.Tenant,
					Name:      s1[0].ObjectMeta.Name,
				}
				if in.Dir == graph.RefOut {
					u.URI = r.makeReferenceLink(sch.GetURI(v1, r.version))
				} else {
					u.URI = r.makeRefererLink(sch.GetURI(v1, r.version))
				}
				uris.URI = append(uris.URI, u)
			}
		}
		ret.Links[k] = uris
	}
	for k, v := range in.WeakRefs {
		uris := browser.Object_URIs{}
		uris.RefType = browser.ReferenceTypes_WeakReference.String()
		for _, v1 := range v {
			s1 := r.cache.Stat(context.TODO(), []string{v1})
			if s1[0].Valid {
				u := api.ObjectRef{
					Kind:      s1[0].TypeMeta.Kind,
					Namespace: s1[0].ObjectMeta.Namespace,
					Tenant:    s1[0].ObjectMeta.Tenant,
					Name:      s1[0].ObjectMeta.Name,
				}
				if in.Dir == graph.RefOut {
					u.URI = r.makeReferenceLink(sch.GetURI(v1, r.version))
				} else {
					u.URI = r.makeRefererLink(sch.GetURI(v1, r.version))
				}
				uris.URI = append(uris.URI, u)
			}
		}
		ret.Links[k] = uris
	}
	for k, v := range in.SelectorRefs {
		uris := browser.Object_URIs{}
		uris.RefType = browser.ReferenceTypes_SelectorReference.String()
		for _, v1 := range v {
			parts := strings.SplitN(v1, ":", 5)
			if len(parts) != 5 {
				return ret, fmt.Errorf("unable to parse selector for [%v]", k)
			}
			refs, err := r.applySelector(context.Background(), parts[3], parts[2], parts[1], parts[4], sch)
			if err != nil {
				return ret, err
			}
			uris.URI = append(uris.URI, refs...)
		}
		ret.Links[k] = uris
	}
	return ret, nil
}

func (r *browserHooks) makeObjectFromVertex(in *graph.Vertex, sch *runtime.Scheme) (browser.Object, error) {
	ret := browser.Object{Links: make(map[string]browser.Object_URIs)}
	if in == nil {
		return ret, errors.New("invalid object")
	}
	ret.URI = sch.GetURI(in.This, r.version)
	if in.Dir == graph.RefOut {
		ret.QueryType = browser.QueryType_Dependencies.String()
	} else {
		ret.QueryType = browser.QueryType_DependedBy.String()
	}
	stat := r.cache.Stat(context.TODO(), []string{in.This})
	if len(stat) != 1 {
		return ret, errors.New("invalid object stat")
	}
	ret.ObjectMeta = stat[0].ObjectMeta
	ret.TypeMeta = stat[0].TypeMeta
	ret.ObjectMeta.SelfLink = sch.GetURI(ret.ObjectMeta.SelfLink, r.version)
	for k, v := range in.Refs {
		uris := browser.Object_URIs{}
		uris.RefType = browser.ReferenceTypes_NamedReference.String()
		for _, v1 := range v {
			s1 := r.cache.Stat(context.TODO(), []string{v1.This})
			if s1[0].Valid {
				u := api.ObjectRef{
					Kind:      s1[0].TypeMeta.Kind,
					Namespace: s1[0].ObjectMeta.Namespace,
					Tenant:    s1[0].ObjectMeta.Tenant,
					Name:      s1[0].ObjectMeta.Name,
				}
				u.URI = sch.GetURI(v1.This, r.version)
				uris.URI = append(uris.URI, u)
			}
		}
		ret.Links[k] = uris
	}
	for k, v := range in.WeakRefs {
		uris := browser.Object_URIs{}
		uris.RefType = browser.ReferenceTypes_WeakReference.String()
		for _, v1 := range v {
			s1 := r.cache.Stat(context.TODO(), []string{v1.This})
			if s1[0].Valid {
				u := api.ObjectRef{
					Kind:      s1[0].TypeMeta.Kind,
					Namespace: s1[0].ObjectMeta.Namespace,
					Tenant:    s1[0].ObjectMeta.Tenant,
					Name:      s1[0].ObjectMeta.Name,
				}
				u.URI = sch.GetURI(v1.This, r.version)
				uris.URI = append(uris.URI, u)
			}
		}
		ret.Links[k] = uris
	}
	for k, v := range in.SelectorRefs {
		uris := browser.Object_URIs{}
		uris.RefType = browser.ReferenceTypes_SelectorReference.String()
		for _, v1 := range v {
			parts := strings.SplitN(v1, ":", 5)
			if len(parts) != 5 {
				return ret, fmt.Errorf("unable to parse selector for [%v]", k)
			}
			refs, err := r.applySelector(context.Background(), parts[3], parts[2], parts[1], parts[4], sch)
			if err != nil {
				return ret, err
			}
			uris.URI = append(uris.URI, refs...)
		}
		ret.Links[k] = uris
	}
	return ret, nil
}

func (r *browserHooks) processQuery(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	r.logger.InfoLog("msg", "processQuery action routine called")
	ret := browser.BrowseResponse{Objects: make(map[string]browser.Object)}
	in, ok := i.(browser.BrowseRequest)
	if !ok {
		return ret, false, errInvalidInputType
	}
	if r.graphDb == nil {
		r.graphDb = r.apisrv.GetGraphDB()
		r.cache = apisrvpkg.GetAPIServerCache()
	}
	if r.graphDb == nil {
		return ret, false, errors.New("uninitialized")
	}
	qtype := graph.RefIn
	if in.QueryType == browser.QueryType_Dependencies.String() {
		qtype = graph.RefOut
	}
	sch := runtime.GetDefaultScheme()
	if in.MaxDepth == 0 || in.MaxDepth > 1 {
		r.logger.InfoLog("msg", "processQuery fetching Tree", "object", in.URI, "direction", in.QueryType)
		ret.RootURI = sch.GetURI(in.URI, r.version)
		ret.QueryType = in.QueryType
		ret.MaxDepth = in.MaxDepth
		t := r.graphDb.Tree(in.URI, qtype)
		if t != nil {
			// XXX-TODO(sanjayt): Implement max-depth in the graphDB interface and supply from here. For now we ignore the max depth parameter.
			verts := list.New()
			verts.PushBack(t)
			f := verts.Front()
			queued := make(map[string]bool)
			queued[t.This] = true
			for f != nil {
				vert := f.Value.(*graph.Vertex)
				o, err := r.makeObjectFromVertex(vert, sch)
				if err != nil {
					return ret, false, fmt.Errorf("could not parse object [%v](%s)", vert.This, err)
				}
				ret.Objects[sch.GetURI(vert.This, r.version)] = o
				ret.TotalCount++
				for _, v := range vert.Refs {
					for _, v1 := range v {
						if _, ok := queued[v1.This]; !ok {
							queued[v1.This] = true
							verts.PushBack(v1)
						}
					}
				}
				verts.Remove(f)
				f = verts.Front()
			}
		} else {
			o, err := r.makeObjectFromKey(in.URI, qtype, sch)
			if err == nil {
				ret.Objects[sch.GetURI(in.URI, r.version)] = o
				return ret, false, nil
			}
			return ret, false, errors.New("object not found")
		}
	} else {
		r.logger.InfoLog("msg", "processQuery fetching Node", "object", in.URI, "direction", in.QueryType)
		var n *graph.Node
		if qtype == graph.RefOut {
			n = r.graphDb.References(in.URI)
		} else {
			n = r.graphDb.Referrers(in.URI)
		}
		ret.RootURI = sch.GetURI(in.URI, r.version)
		ret.QueryType = in.QueryType
		ret.MaxDepth = in.MaxDepth
		ret.TotalCount = 1
		if n != nil {
			o, err := r.makeObjectFromNode(n, sch)
			if err != nil {
				return ret, false, fmt.Errorf("could not parse object [%v](%s)", in.URI, err)
			}
			ret.Objects[sch.GetURI(n.This, r.version)] = o
		} else {
			o, err := r.makeObjectFromKey(in.URI, qtype, sch)
			if err == nil {
				ret.Objects[sch.GetURI(in.URI, r.version)] = o
				return ret, false, nil
			}
			return ret, false, errors.New("object not found")
		}
	}
	return ret, false, nil
}

func registerBrowserHooks(svc apiserver.Service, logger log.Logger) {
	r := browserHooks{version: "v1"}
	r.svc = svc
	r.logger = logger.WithContext("Service", "Browser")
	r.logger.InfoLog("msg", "registering=hooks")
	svc.GetMethod("Query").WithPreCommitHook(r.processQuery)
	svc.GetMethod("References").WithPreCommitHook(r.processQuery)
	svc.GetMethod("Referrers").WithPreCommitHook(r.processQuery)
	r.apisrv = apisrvpkg.MustGetAPIServer()
}

func init() {
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("browser.BrowserV1", registerBrowserHooks)
}
