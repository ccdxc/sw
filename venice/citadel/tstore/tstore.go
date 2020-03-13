// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

package tstore

import (
	"errors"
	"fmt"
	"io"
	"path/filepath"
	"strings"
	"time"

	"github.com/influxdata/influxdb/toml"

	"github.com/influxdata/influxdb/cmd/influxd/run"
	"github.com/influxdata/influxdb/services/continuous_querier"

	"github.com/influxdata/influxdb/services/retention"

	"github.com/influxdata/influxdb/coordinator"
	"github.com/influxdata/influxdb/models"
	"github.com/influxdata/influxdb/query"
	"github.com/influxdata/influxdb/services/meta"
	"github.com/influxdata/influxdb/tsdb"
	"github.com/influxdata/influxql"

	"github.com/pensando/sw/venice/citadel/tproto"
	"github.com/pensando/sw/venice/utils/log"
)

// Tstore is a timeseries store instance
type Tstore struct {
	services      []run.Service                  // tsdb services
	dbPath        string                         // directory where files are stored
	tsdb          *tsdb.Store                    // tsdb store instance
	stmtExecutor  *coordinator.StatementExecutor // statement executor
	queryExecutor *query.QueryExecutor           // query executor
	pointsWriter  *coordinator.PointsWriter      // points writer
	metaClient    *meta.Client                   // metadata client
}

// number of times to retru backup operation if it fails transiently..
const numBackupRetries = 5

// ContinuousQueryRunInterval CQ service run interval
var ContinuousQueryRunInterval = time.Minute

// NewTstoreWithConfig returns a new tstore instance with the custom engine config
func NewTstoreWithConfig(dbPath string, cfg tsdb.Config) (*Tstore, error) {
	// create a new tsdb store
	ts := tsdb.NewStore(dbPath)

	// set the custom config, use tsdb.NewConfig() for deaults
	ts.EngineOptions.Config = cfg
	// switch to tsi1
	ts.EngineOptions.Config.Index = "tsi1"

	// open the tsdb store
	err := ts.Open()
	if err != nil {
		log.Fatalf("Error opening the tsdb. Err: %v", err)
	}

	return newTstore(dbPath, ts)
}

// NewTstore returns a new tstore instance
func NewTstore(dbPath string) (*Tstore, error) {
	// create a new tsdb store
	ts := tsdb.NewStore(dbPath)

	// set the WAL directory
	ts.EngineOptions.Config.WALDir = filepath.Join(dbPath, "wal")
	// switch to tsi1
	ts.EngineOptions.Config.Index = "tsi1"

	// update tag/series limits, TODO: check mem. utilization
	ts.EngineOptions.Config.MaxSeriesPerDatabase = 5 * tsdb.DefaultMaxSeriesPerDatabase
	ts.EngineOptions.Config.MaxValuesPerTag = 0

	// open the tsdb store
	err := ts.Open()
	if err != nil {
		log.Fatalf("Error opening the tsdb. Err: %v", err)
	}

	return newTstore(dbPath, ts)
}

func newTstore(dbPath string, ts *tsdb.Store) (*Tstore, error) {

	// local meta
	cfg := meta.NewConfig()
	cfg.Dir = dbPath
	localMeta := meta.NewClient(cfg)

	// open the metadata if it already exists
	err := localMeta.Open()
	if err != nil {
		// try again after a small delay
		time.Sleep(time.Millisecond * 10)
		err = localMeta.Open()
		if err != nil {
			log.Fatalf("Error opening the local metadata. Err: %v", err)
		}
	}

	// create a points writer
	pwr := coordinator.NewPointsWriter()
	pwr.TSDBStore = ts
	pwr.MetaClient = localMeta

	// create a statemete executor
	stEx := coordinator.StatementExecutor{
		MetaClient: localMeta,
		TSDBStore:  ts,
		ShardMapper: &coordinator.LocalShardMapper{
			MetaClient: localMeta,
			TSDBStore:  ts,
		},
		PointsWriter: pwr,
	}

	// create a query executor
	qEx := query.NewQueryExecutor()
	qEx.StatementExecutor = &stEx

	ret := retention.NewService(retention.NewConfig())
	ret.TSDBStore = ts
	ret.MetaClient = localMeta

	cqCfg := continuous_querier.NewConfig()
	cqCfg.RunInterval = toml.Duration(ContinuousQueryRunInterval)
	cq := continuous_querier.NewService(cqCfg)
	cq.QueryExecutor = qEx
	cq.MetaClient = localMeta

	// create a tstore instance
	svc := Tstore{
		services:      []run.Service{ret, cq},
		dbPath:        dbPath,
		tsdb:          ts,
		stmtExecutor:  &stEx,
		queryExecutor: qEx,
		pointsWriter:  pwr,
		metaClient:    localMeta,
	}

	for _, s := range svc.services {
		if err := s.Open(); err != nil {
			return nil, fmt.Errorf("failed to open service: %s", err)
		}
	}

	return &svc, nil
}

// CreateDatabase creates a database
func (ts *Tstore) CreateDatabase(database string, spec *meta.RetentionPolicySpec) error {
	// if retention spec was nil, return error
	if spec == nil {
		return fmt.Errorf("invalid retention policy")
	}

	dbinfo := ts.metaClient.Database(database)
	if dbinfo == nil {
		// create the database with retention policy
		if _, err := ts.metaClient.CreateDatabaseWithRetentionPolicy(database, spec); err != nil {
			return err
		}

	} else {
		// update retention policy
		sgd := time.Duration(0)
		rpu := &meta.RetentionPolicyUpdate{
			Name:               &spec.Name,
			Duration:           spec.Duration,
			ShardGroupDuration: &sgd, // will be normalized by influxdb
		}

		log.Infof("update [%v] retention to %v", ts.dbPath, rpu.Duration)
		if err := ts.metaClient.UpdateRetentionPolicy(database, spec.Name, rpu, true); err != nil {
			return fmt.Errorf("failed to update retention policy %v", err)
		}
	}

	return nil
}

// ReadDatabases reads all databases
func (ts *Tstore) ReadDatabases() []meta.DatabaseInfo {
	// read databases
	return ts.metaClient.Databases()
}

// DeleteDatabase deletes the database
func (ts *Tstore) DeleteDatabase(database string) error {
	// delete it from tsdb
	err := ts.tsdb.DeleteDatabase(database)
	if err != nil {
		log.Errorf("Error deleting the database %s from tsdb. Err: %v", database, err)
	}

	// delete it from local metadata
	return ts.metaClient.DropDatabase(database)
}

// GetShardInfo will fill in the shard info
func (ts *Tstore) GetShardInfo(sinfo *tproto.SyncShardInfoMsg) error {
	// get the current snapshot of current metadata
	lmd := ts.metaClient.Data()
	data, err := lmd.MarshalBinary()
	if err != nil {
		return err
	}
	sinfo.ChunkMeta = data

	// read all the shards
	shards := ts.tsdb.Shards(ts.tsdb.ShardIDs())
	for _, shard := range shards {
		chunkInfo := tproto.ChunkInfo{
			ChunkID:  shard.ID(),
			Database: shard.Database(),
		}
		sinfo.Chunks = append(sinfo.Chunks, &chunkInfo)
	}

	return nil
}

// RestoreShardInfo restores local meta and chunk state
func (ts *Tstore) RestoreShardInfo(sinfo *tproto.SyncShardInfoMsg) error {
	var lmd meta.Data

	// read meta data from binary
	err := lmd.UnmarshalBinary(sinfo.ChunkMeta)
	if err != nil {
		return err
	}

	// restore local metadata
	err = ts.metaClient.SetData(&lmd)
	if err != nil {
		log.Errorf("Error restoring local metadata. Err: %v", err)
		return err
	}

	// recreate all the shards
	for _, chunk := range sinfo.Chunks {
		err = ts.tsdb.CreateShard(chunk.Database, "default", chunk.ChunkID, true)
		if err != nil {
			log.Errorf("Error creating shard %d for db %s. Err: %v", chunk.ChunkID, chunk.Database, err)
			return err
		}
	}

	return nil
}

// BackupChunk backs up a specific chunk
func (ts *Tstore) BackupChunk(chunkID uint64, w io.Writer) error {
	var err error
	i := 0
	// retry backup few times if compaction is in progress
	for i < numBackupRetries {
		err = ts.tsdb.BackupShard(chunkID, time.Unix(0, 0), w)
		if err == nil {
			return nil
		}

		// retry after a small delay
		time.Sleep(time.Millisecond * 10)

		// retry on "snapshots disabled" errror
		if strings.Contains(err.Error(), "snapshots") {
			log.Infof("retrying backup, err: %s", err)
			continue
		}
		i++
	}

	return err
}

// RestoreChunk restores a specific chunk
func (ts *Tstore) RestoreChunk(chunkID uint64, r io.Reader) error {
	return ts.tsdb.RestoreShard(chunkID, r)
}

// WritePoints writes points to a shard
func (ts *Tstore) WritePoints(database string, points []models.Point) error {
	return ts.pointsWriter.WritePointsPrivileged(database, "default", models.ConsistencyLevelOne, points)
}

// WritePointsInto is a wrapper to write points using BufferedPointsWriter
func (ts *Tstore) WritePointsInto(wr *coordinator.IntoWriteRequest) error {
	return ts.stmtExecutor.PointsWriter.WritePointsInto(wr)
}

// ExecuteQuery executes a query
func (ts *Tstore) ExecuteQuery(q, database string) (<-chan *query.Result, error) {
	// parse the query
	pq, err := influxql.ParseQuery(q)
	if err != nil {
		return nil, err
	}

	// execute the query
	ret := ts.queryExecutor.ExecuteQuery(pq, query.ExecutionOptions{
		Database:   database,
		ChunkSize:  0,
		Authorizer: &tstoreAuth{},
	}, make(chan struct{}))

	return ret, nil
}

// tstoreAuth is the Authorizer in tstore to avoid failure in imem.go:987 where iterator expects valid Authorizer
type tstoreAuth struct {
}

// AuthorizeDatabase indicates whether the given Privilege is authorized on the database with the given name.
func (ta *tstoreAuth) AuthorizeDatabase(p influxql.Privilege, name string) bool {
	return true
}

// AuthorizeQuery returns an error if the query cannot be executed
func (ta *tstoreAuth) AuthorizeQuery(database string, query *influxql.Query) error {
	return nil
}

// AuthorizeSeriesRead determines if a series is authorized for reading
func (ta *tstoreAuth) AuthorizeSeriesRead(database string, measurement []byte, tags models.Tags) bool {
	return true
}

// AuthorizeSeriesWrite determines if a series is authorized for writing
func (ta *tstoreAuth) AuthorizeSeriesWrite(database string, measurement []byte, tags models.Tags) bool {
	return true
}

// Close closes the tstore
func (ts *Tstore) Close() error {

	for _, s := range ts.services {
		if s != nil {
			s.Close()
		}
	}

	if ts.pointsWriter != nil {
		ts.pointsWriter.Close()
	}

	if ts.queryExecutor != nil {
		ts.queryExecutor.Close()
	}
	if ts.metaClient != nil {
		ts.metaClient.Close()
	}
	if ts.tsdb != nil {
		ts.tsdb.Close()
	}
	return nil
}

// CreateRetentionPolicy create retention policy
func (ts *Tstore) CreateRetentionPolicy(database string, rpName string, durationInHours uint64) error {
	duration := time.Duration(durationInHours) * time.Hour
	rpSpec := &meta.RetentionPolicySpec{
		Name:     rpName,
		Duration: &duration,
	}
	dbInfo := ts.metaClient.Database(database)
	if dbInfo == nil {
		return fmt.Errorf("Cannot find database %v", database)
	}
	_, err := ts.metaClient.CreateRetentionPolicy(database, rpSpec, false)
	if err != nil {
		log.Errorf("Error creating retention policy %s. Err: %v", rpName, err)
	}
	return err
}

// GetRetentionPolicy get retention policy for a specific database
func (ts *Tstore) GetRetentionPolicy(database string) ([]string, error) {
	dbInfo := ts.metaClient.Database(database)
	if dbInfo == nil {
		return nil, fmt.Errorf("Error find database named %v", database)
	}
	rpList := []string{}
	for _, rpInfo := range dbInfo.RetentionPolicies {
		rpList = append(rpList, rpInfo.Name)
	}
	return rpList, nil
}

// CheckRetentionPolicy check the existence of retention policy in specific database
func (ts *Tstore) CheckRetentionPolicy(database, rpName string) (bool, error) {
	dbInfo := ts.metaClient.Database(database)
	if dbInfo == nil {
		return false, fmt.Errorf("Error find database named %v", database)
	}
	rpInfo, err := ts.metaClient.RetentionPolicy(database, rpName)
	if err != nil {
		return false, err
	}
	if rpInfo == nil {
		return false, nil
	}
	return true, nil
}

// DeleteRetentionPolicy drop retention policy
func (ts *Tstore) DeleteRetentionPolicy(database, rpName string) error {
	dbInfo := ts.metaClient.Database(database)
	if dbInfo == nil {
		return fmt.Errorf("Error find database named %v", database)
	}
	return ts.metaClient.DropRetentionPolicy(database, rpName)
}

// CreateContinuousQuery create continuous query
func (ts *Tstore) CreateContinuousQuery(database string, cq string, rpName string, query string) error {
	dbInfo := ts.metaClient.Database(database)
	if dbInfo == nil {
		return errors.New("Cannot find database")
	}

	// If the retention policy does not exist, throw out an error
	existed, err := ts.CheckRetentionPolicy(database, rpName)
	if err != nil {
		return fmt.Errorf("Error check retention policy on database %v", database)
	}
	if !existed {
		return fmt.Errorf("Error retention policy %v not existed in database %v", rpName, database)
	}

	err = ts.metaClient.CreateContinuousQuery(database, cq, query)
	if err != nil {
		return fmt.Errorf("Error create continuous query for metaclient. Error: %v", err)
	}
	return nil
}

// CheckContinuousQuery check whether a CQ existed in a database or not
func (ts *Tstore) CheckContinuousQuery(database string, cqName string) (bool, error) {
	// read continuous queries
	dbInfo := ts.metaClient.Database(database)
	if dbInfo == nil {
		return false, fmt.Errorf("Database %+v doesn't exist", database)
	}
	for _, cq := range dbInfo.ContinuousQueries {
		if cq.Name == cqName {
			return true, nil
		}
	}
	return false, nil
}

// GetContinuousQuery reads all CQs in the database
func (ts *Tstore) GetContinuousQuery(database string) ([]string, error) {
	result := []string{}
	// read continuous queries
	dbInfo := ts.metaClient.Database(database)
	if dbInfo == nil {
		return result, fmt.Errorf("Database %+v doesn't exist", database)
	}
	for _, cq := range dbInfo.ContinuousQueries {
		result = append(result, cq.Name)
	}
	return result, nil
}

// DeleteContinuousQuery deletes the database
func (ts *Tstore) DeleteContinuousQuery(database string, cq string) error {
	// delete it from local metadata
	return ts.metaClient.DropContinuousQuery(database, cq)
}
