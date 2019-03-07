package vospkg

import (
	"context"

	"github.com/pensando/sw/api/generated/objstore"
	"github.com/pensando/sw/venice/vos"
)

// // pluginsMap has a map of all pluginSet per bucket.
// var pluginsMap map[string]pluginSet
// var pluginsMapMutes sync.RWMutex
// var pluginsMapOnce sync.Once
//
// func initPluginsMap() {
// 	pluginsMap = make(map[string]pluginSet)
// }

type pluginSet struct {
	bucket              string
	preUploadPlugins    []vos.CallBackFunc
	preDownloadPlugins  []vos.CallBackFunc
	preGetPlugins       []vos.CallBackFunc
	preListPlugins      []vos.CallBackFunc
	preDeletePlugins    []vos.CallBackFunc
	preUpdatePlugins    []vos.CallBackFunc
	preWatchPlugins     []vos.CallBackFunc
	postUploadPlugins   []vos.CallBackFunc
	postDownloadPlugins []vos.CallBackFunc
	postGetPlugins      []vos.CallBackFunc
	postListPlugins     []vos.CallBackFunc
	postDeletePlugins   []vos.CallBackFunc
	postUpdatePlugins   []vos.CallBackFunc
	postWatchPlugins    []vos.CallBackFunc
}

func newPluginSet(bucket string) *pluginSet {
	return &pluginSet{bucket: bucket}
}

func (p *pluginSet) registerPlugin(stage vos.OperStage, oper vos.ObjectOper, cb vos.CallBackFunc) {
	if stage == vos.PreOp {
		switch oper {
		case vos.Upload:
			p.preUploadPlugins = append(p.preUploadPlugins, cb)
		case vos.Download:
			p.preDownloadPlugins = append(p.preDownloadPlugins, cb)
		case vos.Get:
			p.preGetPlugins = append(p.preGetPlugins, cb)
		case vos.List:
			p.preListPlugins = append(p.preListPlugins, cb)
		case vos.Delete:
			p.preDeletePlugins = append(p.preDeletePlugins, cb)
		case vos.Update:
			p.preUpdatePlugins = append(p.preUpdatePlugins, cb)
		case vos.Watch:
			p.preWatchPlugins = append(p.preWatchPlugins, cb)
		}
	} else {
		switch oper {
		case vos.Upload:
			p.postUploadPlugins = append(p.postUploadPlugins, cb)
		case vos.Download:
			p.postDownloadPlugins = append(p.postDownloadPlugins, cb)
		case vos.Get:
			p.postGetPlugins = append(p.postGetPlugins, cb)
		case vos.List:
			p.postListPlugins = append(p.postListPlugins, cb)
		case vos.Delete:
			p.postDeletePlugins = append(p.postDeletePlugins, cb)
		case vos.Update:
			p.postUpdatePlugins = append(p.postUpdatePlugins, cb)
		case vos.Watch:
			p.postWatchPlugins = append(p.postWatchPlugins, cb)
		}
	}
}

func (p *pluginSet) RunPlugins(ctx context.Context, stage vos.OperStage, oper vos.ObjectOper, in *objstore.Object, client vos.BackendClient) []error {
	var plugins []vos.CallBackFunc
	if stage == vos.PreOp {
		switch oper {
		case vos.Upload:
			plugins = p.preUploadPlugins
		case vos.Download:
			plugins = p.preDownloadPlugins
		case vos.Get:
			plugins = p.preGetPlugins
		case vos.List:
			plugins = p.preListPlugins
		case vos.Delete:
			plugins = p.preDeletePlugins
		case vos.Update:
			plugins = p.preUpdatePlugins
		case vos.Watch:
			plugins = p.preWatchPlugins
		}
	} else {
		switch oper {
		case vos.Upload:
			plugins = p.postUploadPlugins
		case vos.Download:
			plugins = p.postDownloadPlugins
		case vos.Get:
			plugins = p.postGetPlugins
		case vos.List:
			plugins = p.postListPlugins
		case vos.Delete:
			plugins = p.postDeletePlugins
		case vos.Update:
			plugins = p.postUpdatePlugins
		case vos.Watch:
			plugins = p.postWatchPlugins
		}
	}
	var errs []error
	for _, cb := range plugins {
		err := cb(ctx, oper, in, client)
		if err != nil {
			errs = append(errs, err)
		}
	}
	return errs
}

func (p *pluginSet) Clear() {
	p.preUploadPlugins, p.preDownloadPlugins, p.preGetPlugins, p.preListPlugins = nil, nil, nil, nil
	p.preDeletePlugins, p.preUpdatePlugins, p.preWatchPlugins = nil, nil, nil
	p.postUploadPlugins, p.postDownloadPlugins, p.postGetPlugins, p.postListPlugins = nil, nil, nil, nil
	p.postDeletePlugins, p.postUpdatePlugins, p.postWatchPlugins = nil, nil, nil
}
