package vospkg

import (
	"context"
	"fmt"
	"os"
	"sync"
	"syscall"
	"time"

	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	"github.com/pensando/sw/venice/vos/protos"
)

func isThresholdReached(currPath string, size uint64, thresholdPercent float64) (bool, uint64) {
	var used uint64

	dir, err := os.Open(currPath)
	if err != nil {
		// fmt.Println(err)
		return false, used
	}
	defer dir.Close()

	files, err := dir.Readdir(-1)
	if err != nil {
		log.Errorf("monitor disk usage path %s, err %+v", currPath, err)
		return false, 0
	}

	for _, file := range files {
		if file.IsDir() {
			reached, tmp := isThresholdReached(fmt.Sprintf("%s/%s", currPath, file.Name()), size, thresholdPercent)
			if reached {
				return true, tmp
			}
			used += tmp
		} else {
			used += uint64(file.Size())
		}

		p := (float64(used) / float64(size)) * 100
		if p >= thresholdPercent {
			log.Infof("disk threshold reached for namespace %s", currPath)
			return true, used
		}
	}

	return false, used
}

// disk usage of path/disk
func diskUsage(path string, thresholdPercent float64) (bool, uint64, uint64, error) {
	fs := syscall.Statfs_t{}
	err := syscall.Statfs(path, &fs)
	if err != nil {
		log.Errorf("monitor disk usage path %s, err %+v", path, err)
		return false, 0, 0, fmt.Errorf("monitor disk usage err:  %+v", err)
	}
	all := fs.Blocks * uint64(fs.Bsize)
	reached, used := isThresholdReached(path, all, thresholdPercent)
	return reached, all, used, nil
}

func (w *storeWatcher) monitorDisks(ctx context.Context,
	monitorTimeout time.Duration, wg *sync.WaitGroup, paths map[string]float64) {
	defer wg.Done()

	for {
		select {
		case <-ctx.Done():
			return
		case <-time.After(monitorTimeout):
			w.statDisk(paths)
		}
	}
}

func (w *storeWatcher) statDisk(paths map[string]float64) {
	for p, t := range paths {
		reached, all, used, err := diskUsage(p, t)
		if err != nil {
			continue
		}

		// Generate notification only if the disk threshold is reached
		if reached {
			obj := makeEvent(p, all, used)
			evType := kvstore.Created
			log.Debugf("sending watch event [%v][%v][%+v]", diskUpdateWatchPath, evType, obj)
			qs := w.watchPrefixes.Get(diskUpdateWatchPath)
			for j := range qs {
				err := qs[j].Enqueue(evType, obj, nil)
				if err != nil {
					log.Errorf("unable to enqueue the event (%s)", err)
				}
			}
		}
	}
}

func makeEvent(path string, all, used uint64) runtime.Object {
	obj := &protos.DiskUpdate{}
	obj.Status.Path = path
	obj.Status.Size_ = all
	obj.Status.UsedByNamespace = used
	obj.ResourceVersion = fmt.Sprintf("%d", time.Now().UnixNano())
	return obj
}
