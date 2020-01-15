// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package data

// this file contains the datanode management code

import (
	"bytes"
	"errors"
	"fmt"
	"io"
	"sync"
	"time"

	context "golang.org/x/net/context"

	"github.com/cenkalti/backoff"

	"github.com/pensando/sw/venice/citadel/meta"
	"github.com/pensando/sw/venice/citadel/tproto"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/safelist"
)

// SyncWriter implements io.Writer interface
type SyncWriter struct {
	msg     *tproto.SyncDataMsg
	wstream tproto.DataNode_SyncDataClient
	maxSize int
}

// NewSyncWriter returns a new writer instance
func NewSyncWriter(msg *tproto.SyncDataMsg, wstream tproto.DataNode_SyncDataClient, maxSize int) *SyncWriter {
	return &SyncWriter{
		msg:     msg,
		wstream: wstream,
		maxSize: maxSize,
	}
}

// Write writes data over to GRPC stream
func (swr *SyncWriter) Write(p []byte) (n int, err error) {
	sendLen := len(p)
	if sendLen > swr.maxSize {
		swr.msg.Data = p[:swr.maxSize]
		sendLen = swr.maxSize
	} else {
		swr.msg.Data = p
	}

	log.Debugf("Synced data from replica %d chunk %d len: %d", swr.msg.SrcReplicaID, swr.msg.ChunkID, sendLen)

	err = swr.wstream.Send(swr.msg)
	if err != nil {
		log.Errorf("Error sending Replica %d, chunk %d. Err: %v", swr.msg.SrcReplicaID, swr.msg.ChunkID, err)
		return 0, err
	}

	return sendLen, nil
}

// SyncReader implements io.Reader interface
type SyncReader struct {
	buf     []byte
	rstream tproto.DataNode_SyncDataServer
}

// NewSyncReader returns a new reader that reads from grpc stream
func NewSyncReader(data []byte, rstream tproto.DataNode_SyncDataServer) *SyncReader {
	tmpbuf := bytes.NewBuffer(data)
	return &SyncReader{
		buf:     tmpbuf.Bytes(),
		rstream: rstream,
	}
}

// Read reads next set of bytes from the grpc stream
func (srd *SyncReader) Read(p []byte) (n int, err error) {
	if srd.buf != nil {
		log.Debugf("Restoring chunk of len %d, available(%d)", len(srd.buf), len(p))
		rdlen := len(srd.buf)
		if len(p) < rdlen {
			// copy partial data
			copy(p, srd.buf[:len(p)])
			srd.buf = srd.buf[len(p):]
			return len(p), nil
		}

		// copy the data
		copy(p, srd.buf)
		srd.buf = nil
		return rdlen, nil

	}

	// receive more data from the stream
	req, err := srd.rstream.Recv()
	if err != nil {
		return 0, err
	}

	log.Debugf("Restoring %s Chunk %d on replica %d, len %d, available(%d)", req.ClusterType, req.ChunkID, req.DestReplicaID, len(req.Data), len(p))

	rdlen := len(req.Data)
	if len(p) < rdlen {
		// copy partial data
		copy(p, req.Data[:len(p)])
		tmpbuf := bytes.NewBuffer(req.Data[len(p):])
		srd.buf = tmpbuf.Bytes()

		return len(p), nil
	}

	// copy the data
	copy(p, req.Data)
	return rdlen, nil

}

// SyncShardReq is request message from metadata server to primary
func (dn *DNode) SyncShardReq(ctx context.Context, req *tproto.SyncShardMsg) (*tproto.StatusResp, error) {
	var resp tproto.StatusResp
	var store StoreAPI
	var lock *sync.Mutex

	dn.logger.Infof("%s Received SyncShardReq req %+v", dn.nodeUUID, req)

	// handle based on cluster type
	switch req.ClusterType {
	case meta.ClusterTypeTstore:
		// find the data store from shard id
		val, ok := dn.tshards.Load(req.SrcReplicaID)
		if !ok || val.(*TshardState).store == nil {
			dn.logger.Errorf("Replica %d not found for cluster %s", req.SrcReplicaID, req.ClusterType)
			return &resp, errors.New("Shard not found")
		}
		shard := val.(*TshardState)
		shard.isPrimary = req.SrcIsPrimary

		// verify we are the primary for this shard
		if !shard.isPrimary {
			dn.logger.Errorf("Non-primary received shard sync request. Shard: %+v", shard)
			return &resp, errors.New("Non-primary received shard sync req")
		}

		store = shard.store
		lock = &shard.syncLock

		// update the replicas list
		shard.replicas = req.Replicas
	case meta.ClusterTypeKstore:
		// find the data store from shard id
		val, ok := dn.kshards.Load(req.SrcReplicaID)
		if !ok || val.(*KshardState).kstore == nil {
			dn.logger.Errorf("Replica %d not found for cluster %s", req.SrcReplicaID, req.ClusterType)
			return &resp, errors.New("Shard not found")
		}
		shard := val.(*KshardState)
		shard.isPrimary = req.SrcIsPrimary

		// verify we are the primary for this shard
		if !shard.isPrimary {
			dn.logger.Errorf("Non-primary received shard sync request. Shard: %+v", shard)
			return &resp, errors.New("Non-primary received shard sync req")
		}
		store = shard.kstore
		lock = &shard.syncLock

		// update the replicas list
		shard.replicas = req.Replicas

	default:
		dn.logger.Fatalf("Unknown cluster type :%s.", req.ClusterType)
		return &resp, errors.New("Unknown cluster type")
	}

	// copy data in background
	go func() {
		for i := 0; i < dn.clusterCfg.MaxSyncRetry; i++ {
			if err := dn.syncShard(context.Background(), req, store, lock); err == nil {
				return
			}
			log.Errorf("failed[%d] to sync shard from %v", i+1, req)
			time.Sleep(time.Second * 2)
		}
		log.Errorf("failed all attempts to sync shard from %v", req)
	}()

	return &resp, nil
}

// syncShard sync's contents of a shard from source replica to dest replica
func (dn *DNode) syncShard(ctx context.Context, req *tproto.SyncShardMsg, store StoreAPI, lock *sync.Mutex) error {
	// lock the shard
	// FIXME: we need to remove this lock and keep sync buffer
	lock.Lock()
	defer lock.Unlock()

	// collect all local shard info
	sinfo := tproto.SyncShardInfoMsg{
		ClusterType:   req.ClusterType,
		SrcReplicaID:  req.SrcReplicaID,
		DestReplicaID: req.DestReplicaID,
		ShardID:       req.ShardID,
	}

	// get the shard info from the store
	err := store.GetShardInfo(&sinfo)
	if err != nil {
		dn.logger.Errorf("Error getting shard info from tstore. Err: %v", err)
		return err
	}

	// dial a new connection for syncing data
	rclient, err := rpckit.NewRPCClient(fmt.Sprintf("datanode-%s", dn.nodeUUID), req.DestNodeURL, rpckit.WithLoggerEnabled(false), rpckit.WithRemoteServerName(globals.Citadel))
	if err != nil {
		dn.logger.Errorf("Error connecting to rpc server %s. err: %v", req.DestNodeURL, err)
		return err
	}
	defer rclient.Close()

	// send shard info message to destination
	dnclient := tproto.NewDataNodeClient(rclient.ClientConn)
	_, err = dnclient.SyncShardInfo(ctx, &sinfo)
	if err != nil {
		dn.logger.Errorf("Error sending sync shard info message to %s. Err: %v", req.DestNodeURL, err)
		return err
	}

	// walk thru all chunks and send them over the stream
	for _, chunkInfo := range sinfo.Chunks {
		// send stream of sync messages
		stream, err := dnclient.SyncData(ctx)
		if err != nil {
			dn.logger.Errorf("Error syncing data to %s. Err: %v", req.DestNodeURL, err)
			return err
		}

		// create the message
		msg := tproto.SyncDataMsg{
			ClusterType:   req.ClusterType,
			SrcReplicaID:  req.SrcReplicaID,
			DestReplicaID: req.DestReplicaID,
			ShardID:       req.ShardID,
			ChunkID:       chunkInfo.ChunkID,
		}

		// backup chunk
		err = store.BackupChunk(chunkInfo.ChunkID, NewSyncWriter(&msg, stream, dn.clusterCfg.MaxSyncMsgSize))
		if err != nil && err != io.EOF {
			dn.logger.Errorf("Error backing up chunk %d. Err: %v", chunkInfo.ChunkID, err)
			return err
		}

		dn.logger.Infof("Sync complete for replica %d", req.SrcReplicaID)

		// close the stream
		_, err = stream.CloseAndRecv()
		if err != nil {
			dn.logger.Errorf("Error closing sync channel. Err: %v", err)
			return err
		}
	}

	return nil

}

// SyncShardInfo contains the shard info about a shard thats about to be synced
func (dn *DNode) SyncShardInfo(ctx context.Context, req *tproto.SyncShardInfoMsg) (*tproto.StatusResp, error) {
	var resp tproto.StatusResp
	var store StoreAPI

	dn.logger.Infof("%s Received SyncShardInfo req %+v", dn.nodeUUID, req)

	switch req.ClusterType {
	case meta.ClusterTypeTstore:
		// find the data store from replica id
		val, ok := dn.tshards.Load(req.DestReplicaID)
		if !ok || val.(*TshardState).store == nil {
			dn.logger.Errorf("Shard %d not found", req.DestReplicaID)
			return nil, errors.New("Shard not found")
		}
		shard := val.(*TshardState)
		store = shard.store

		// if there are chunks to be synced, set the sync pending flag
		if len(req.Chunks) != 0 {
			shard.syncPending = true
		}
	case meta.ClusterTypeKstore:
		// find the data store from shard id
		val, ok := dn.kshards.Load(req.DestReplicaID)
		if !ok || val.(*KshardState).kstore == nil {
			dn.logger.Errorf("Shard %d not found", req.DestReplicaID)
			return nil, errors.New("Shard not found")
		}
		shard := val.(*KshardState)
		store = shard.kstore

		// if there are chunks to be synced, set the sync pending flag
		if len(req.Chunks) != 0 {
			shard.syncPending = true
		}
	default:
		dn.logger.Fatalf("Unknown cluster type :%s.", req.ClusterType)
	}

	// restore the local metadata
	err := store.RestoreShardInfo(req)
	if err != nil {
		dn.logger.Errorf("Error restoring local metadata. Err: %v", err)
		return nil, err
	}

	return &resp, nil
}

// SyncData is streaming sync message from primary to replica
func (dn *DNode) SyncData(stream tproto.DataNode_SyncDataServer) error {
	var store StoreAPI

	dn.logger.Infof("%s Received SyncData req", dn.nodeUUID)

	// keep receiving
	req, err := stream.Recv()
	if err == io.EOF {
		log.Errorf("Error first sync message was EOF")
		// send success at the end of the message
		return stream.SendAndClose(&tproto.StatusResp{})
	}
	if err != nil {
		dn.logger.Errorf("Error receiving from stream. Err: %v", err)
		return err
	}

	dn.logger.Infof("%s Restoring %s Chunk %d on replica %d", dn.nodeUUID, req.ClusterType, req.ChunkID, req.DestReplicaID)

	switch req.ClusterType {
	case meta.ClusterTypeTstore:
		// find the data store from shard id
		val, ok := dn.tshards.Load(req.DestReplicaID)
		if !ok || val.(*TshardState).store == nil {
			dn.logger.Errorf("Shard %d not found", req.SrcReplicaID)
			return errors.New("Shard not found")
		}
		shard := val.(*TshardState)
		store = shard.store

		// clear sync pending flag
		defer func() { shard.syncPending = false }()
	case meta.ClusterTypeKstore:
		// find the data store from shard id
		val, ok := dn.kshards.Load(req.DestReplicaID)
		if !ok || val.(*KshardState).kstore == nil {
			dn.logger.Errorf("Shard %d not found", req.SrcReplicaID)
			return errors.New("Shard not found")
		}
		shard := val.(*KshardState)
		store = shard.kstore

		// clear sync pending flag
		defer func() { shard.syncPending = false }()
	default:
		dn.logger.Fatalf("Unknown cluster type :%s.", req.ClusterType)
	}

	// restore
	err = store.RestoreChunk(req.ChunkID, NewSyncReader(req.Data, stream))
	if err != nil && err != io.EOF {
		dn.logger.Errorf("Error restoring chunk %d. Err: %v", req.ChunkID, err)
		return err
	}

	dn.logger.Infof("Restore complete for Chunk %d on replica %d", req.ChunkID, req.DestReplicaID)

	// send success at the end of the message
	return stream.SendAndClose(&tproto.StatusResp{})
}

// go routine to process the pending requests
func (dn *DNode) processPendingQueue(sb *syncBufferState) {
	defer sb.wg.Done()

	// forever
	for {
		wait := sb.backoff.NextBackOff()
		// we don't stop the back off timer
		if wait == backoff.Stop {
			dn.logger.Errorf("abort processing sync buffer queue due to timeout, %+v", sb)
			return
		}

		select {
		case <-time.After(wait):
			if err := dn.replicateFailedRequest(sb); err != nil {
				log.Infof("failed to replicate points from sync buffer, %v", err)
				sb.backoff.NextBackOff()

			} else { // reset
				sb.backoff.Reset()
			}

		case <-sb.ctx.Done():
			// try one more time
			if err := dn.replicateFailedRequest(sb); err != nil {
				dn.logger.Errorf("sync buffer failed to replicate writes. err:%s, discard queue, length:%d, {%+v}", err, sb.queue.Len(), sb)
			}
			dn.logger.Infof("exit sync buffer processing, %+v", sb)
			return
		}
	}
}

// addSyncBuffer queues failed writes and triggers retry routine
func (dn *DNode) addSyncBuffer(sm *sync.Map, nodeUUID string, req interface{}) error {
	var sb *syncBufferState
	var clusterType string
	var replicaID uint64
	var shardID uint64

	switch v := req.(type) {
	case *tproto.PointsWriteReq:
		clusterType = meta.ClusterTypeTstore
		replicaID = v.ReplicaID
		shardID = v.ShardID

	case *tproto.KeyValueMsg:
		clusterType = meta.ClusterTypeKstore
		replicaID = v.ReplicaID
		shardID = v.ShardID
	default:
		dn.logger.Fatalf("invalid cluster type in sync buffer req %T", v)
	}

	if sbinter, ok := sm.Load(replicaID); ok {
		sb = sbinter.(*syncBufferState)
	} else {
		ctx, cancel := context.WithCancel(context.Background())
		// add new entry with exponential back off
		sb = &syncBufferState{
			ctx:         ctx,
			cancel:      cancel,
			queue:       safelist.New(),
			backoff:     backoff.NewExponentialBackOff(),
			nodeUUID:    nodeUUID,
			replicaID:   replicaID,
			shardID:     shardID,
			clusterType: clusterType,
		}
		sb.backoff.InitialInterval = 2 * time.Second
		sb.backoff.RandomizationFactor = 0.2
		sb.backoff.Multiplier = 2
		sb.backoff.MaxInterval = meta.DefaultNodeDeadInterval / 2
		sb.backoff.MaxElapsedTime = 0 // run forever
		sb.backoff.Reset()

		dn.logger.Infof("%s created sync buffer for cluster-type:%s, replica id:%v", dn.nodeUUID, sb.clusterType, replicaID)

		// store new entry
		sm.Store(replicaID, sb)
		sb.wg.Add(1)
		go dn.processPendingQueue(sb)
	}

	if sb.queue.Len() < dn.clusterCfg.MaxSyncBuffSize {
		sb.queue.Insert(req)
	} else {
		log.Errorf("sync buffer limit reached for node %v replica %v(%v)", sb.nodeUUID, sb.replicaID, sb.shardID)
	}

	return nil
}

// updateSyncBuffer clean up the sync buffer based on the updated shard
func (dn *DNode) updateSyncBuffer(sm *sync.Map, req *tproto.ShardReq) error {
	dn.logger.Infof("%s sync buffer received update request: %+v", dn.nodeUUID, req)
	newReplicaMap := map[uint64]*tproto.ReplicaInfo{}

	// not primary ? delete all replica ids of this shard
	if req.IsPrimary == false {
		return dn.deleteShardSyncBuffer(sm)
	}

	// primary replica ? update the sync buffer
	for _, e := range req.Replicas {
		newReplicaMap[e.ReplicaID] = e
	}

	replicaToDel := []uint64{}
	sm.Range(func(key, val interface{}) bool {
		replicaID := key.(uint64)
		// select replica ids that got removed
		if _, ok := newReplicaMap[replicaID]; !ok {
			replicaToDel = append(replicaToDel, replicaID)
		}
		return true
	})

	dn.logger.Infof("delete replica ids from sync buffer %+v", replicaToDel)
	for _, r := range replicaToDel {
		dn.deleteSyncBuffer(sm, r)
	}

	return nil
}

// deleteSyncBuffer deletes replica id from sync buffer
func (dn *DNode) deleteSyncBuffer(sm *sync.Map, replicaID uint64) error {
	dn.logger.Infof("%s sync buffer  received delete request for replica %v", dn.nodeUUID, replicaID)
	sbinter, ok := sm.Load(replicaID)
	if !ok {
		return nil
	}
	sb := sbinter.(*syncBufferState)
	// stop the go routine
	sb.cancel()
	sb.wg.Wait()
	sm.Delete(replicaID)

	// empty the queue
	sb.queue.RemoveTill(func(i int, v interface{}) bool {
		return true
	})
	return nil
}

// deleteShardSyncBuffer deletes all sync buffer in the shard
func (dn *DNode) deleteShardSyncBuffer(sm *sync.Map) error {
	replicaList := []uint64{}

	sm.Range(func(k interface{}, v interface{}) bool {
		replicaID, ok := k.(uint64)
		if ok {
			replicaList = append(replicaList, replicaID)
		}
		return true
	})

	dn.logger.Infof("%s sync buffer received delete all request, replica-ids: %+v", dn.nodeUUID, replicaList)
	for _, k := range replicaList {
		dn.deleteSyncBuffer(sm, k)
	}
	return nil
}
