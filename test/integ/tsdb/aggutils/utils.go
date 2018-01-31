package aggutils

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"math/rand"
	"net/http"
	"net/url"
	"os"
	"regexp"
	"sort"
	"strings"
	"sync"
	"time"

	"github.com/influxdata/influxdb/cmd/influxd/run"
	"github.com/influxdata/influxdb/models"
	"github.com/influxdata/influxdb/services/httpd"
	"github.com/influxdata/influxdb/services/meta"
	"github.com/influxdata/influxdb/toml"
)

var verboseServerLogs bool
var indexType = "inmem"

// Server represents a test wrapper for run.Server.
type Server interface {
	URL() string
	Open() error
	SetLogOutput(w io.Writer)
	Close()
	Closed() bool

	CreateDatabase(db string) (*meta.DatabaseInfo, error)
	CreateDatabaseAndRetentionPolicy(db string, rp *meta.RetentionPolicySpec, makeDefault bool) error
	CreateSubscription(database, rp, name, mode string, destinations []string) error
	DropDatabase(db string) error
	Reset() error

	Query(query string) (results string, err error)
	QueryWithParams(query string, values url.Values) (results string, err error)

	Write(db, rp, body string, params url.Values) (results string, err error)
	MustWrite(db, rp, body string, params url.Values) string
	WritePoints(database, retentionPolicy string, consistencyLevel models.ConsistencyLevel, user meta.User, points []models.Point) error
}

// NewServer returns a new instance of Server.
func NewServer(c *run.Config) Server {
	buildInfo := &run.BuildInfo{
		Version: "testServer",
		Commit:  "testCommit",
		Branch:  "testBranch",
	}

	// Create a local server
	srv, _ := run.NewServer(c, buildInfo)
	s := LocalServer{
		client: &client{},
		Server: srv,
		Config: c,
	}
	s.client.URLFn = s.URL
	return &s
}

// OpenServer opens a test server.
func OpenServer(c *run.Config) Server {
	s := NewServer(c)
	configureLogging(s)
	if err := s.Open(); err != nil {
		panic(err.Error())
	}
	return s
}

// OpenDefaultServer opens a test server with a default database & retention policy.
func OpenDefaultServer(c *run.Config) Server {
	s := OpenServer(c)
	if err := s.CreateDatabaseAndRetentionPolicy("db0", NewRetentionPolicySpec("rp0", 1, 0), true); err != nil {
		panic(err)
	}
	return s
}

// LocalServer is a Server that is running in-process and can be accessed directly
type LocalServer struct {
	mu sync.RWMutex
	*run.Server

	*client
	Config *run.Config
}

// Close shuts down the server and removes all temporary paths.
func (s *LocalServer) Close() {
	s.mu.Lock()
	defer s.mu.Unlock()

	if err := s.Server.Close(); err != nil {
		panic(err.Error())
	}
	if err := os.RemoveAll(s.Config.Meta.Dir); err != nil {
		panic(err.Error())
	}
	if err := os.RemoveAll(s.Config.Data.Dir); err != nil {
		panic(err.Error())
	}
	// Nil the server so our deadlock detector goroutine can determine if we completed writes
	// without timing out
	s.Server = nil
}

// Closed checks if the server is closed
func (s *LocalServer) Closed() bool {
	s.mu.RLock()
	defer s.mu.RUnlock()
	return s.Server == nil
}

// URL returns the base URL for the httpd endpoint.
func (s *LocalServer) URL() string {
	s.mu.RLock()
	defer s.mu.RUnlock()
	for _, service := range s.Services {
		if service, ok := service.(*httpd.Service); ok {
			return "http://" + service.Addr().String()
		}
	}
	panic("httpd server not found in services")
}

// CreateDatabase create a database
func (s *LocalServer) CreateDatabase(db string) (*meta.DatabaseInfo, error) {
	s.mu.RLock()
	defer s.mu.RUnlock()
	return s.MetaClient.CreateDatabase(db)
}

// CreateDatabaseAndRetentionPolicy will create the database and retention policy.
func (s *LocalServer) CreateDatabaseAndRetentionPolicy(db string, rp *meta.RetentionPolicySpec, makeDefault bool) error {
	s.mu.RLock()
	defer s.mu.RUnlock()
	if _, err := s.MetaClient.CreateDatabase(db); err != nil {
		return err
	} else if _, err := s.MetaClient.CreateRetentionPolicy(db, rp, makeDefault); err != nil {
		return err
	}
	return nil
}

// CreateSubscription creates a subscription
func (s *LocalServer) CreateSubscription(database, rp, name, mode string, destinations []string) error {
	s.mu.RLock()
	defer s.mu.RUnlock()
	return s.MetaClient.CreateSubscription(database, rp, name, mode, destinations)
}

// DropDatabase deletes a database
func (s *LocalServer) DropDatabase(db string) error {
	s.mu.RLock()
	defer s.mu.RUnlock()

	if err := s.TSDBStore.DeleteDatabase(db); err != nil {
		return err
	}
	return s.MetaClient.DropDatabase(db)
}

// Reset deletes all databases
func (s *LocalServer) Reset() error {
	s.mu.RLock()
	defer s.mu.RUnlock()
	for _, db := range s.MetaClient.Databases() {
		if err := s.DropDatabase(db.Name); err != nil {
			return err
		}
	}
	return nil
}

// WritePoints writes a slice of points
func (s *LocalServer) WritePoints(database, retentionPolicy string, consistencyLevel models.ConsistencyLevel, user meta.User, points []models.Point) error {
	s.mu.RLock()
	defer s.mu.RUnlock()

	if s.PointsWriter == nil {
		return fmt.Errorf("server closed")
	}

	return s.PointsWriter.WritePoints(database, retentionPolicy, consistencyLevel, user, points)
}

// client abstract querying and writing to a Server using HTTP
type client struct {
	URLFn func() string
}

func (c *client) URL() string {
	return c.URLFn()
}

// Query executes a query against the server and returns the results.
func (c *client) Query(query string) (results string, err error) {
	return c.QueryWithParams(query, nil)
}

// MustQuery executes a query against the server and returns the results.
func (c *client) MustQuery(query string) string {
	results, err := c.Query(query)
	if err != nil {
		panic(err)
	}
	return results
}

// Query executes a query against the server and returns the results.
func (c *client) QueryWithParams(query string, values url.Values) (results string, err error) {
	var v url.Values
	if values == nil {
		v = url.Values{}
	} else {
		v, _ = url.ParseQuery(values.Encode())
	}
	v.Set("q", query)
	return c.HTTPPost(c.URL()+"/query?"+v.Encode(), nil)
}

// MustQueryWithParams executes a query against the server and returns the results.
func (c *client) MustQueryWithParams(query string, values url.Values) string {
	results, err := c.QueryWithParams(query, values)
	if err != nil {
		panic(err)
	}
	return results
}

// HTTPGet makes an HTTP GET request to the server and returns the response.
func (c *client) HTTPGet(url string) (results string, err error) {
	resp, err := http.Get(url)
	if err != nil {
		return "", err
	}
	body := strings.TrimSpace(string(MustReadAll(resp.Body)))
	switch resp.StatusCode {
	case http.StatusBadRequest:
		if !expectPattern(".*error parsing query*.", body) {
			return "", fmt.Errorf("unexpected status code: code=%d, body=%s", resp.StatusCode, body)
		}
		return body, nil
	case http.StatusOK:
		return body, nil
	default:
		return "", fmt.Errorf("unexpected status code: code=%d, body=%s", resp.StatusCode, body)
	}
}

// HTTPPost makes an HTTP POST request to the server and returns the response.
func (c *client) HTTPPost(url string, content []byte) (results string, err error) {
	buf := bytes.NewBuffer(content)
	resp, err := http.Post(url, "application/json", buf)
	if err != nil {
		return "", err
	}
	body := strings.TrimSpace(string(MustReadAll(resp.Body)))
	switch resp.StatusCode {
	case http.StatusBadRequest:
		if !expectPattern(".*error parsing query*.", body) {
			return "", fmt.Errorf("unexpected status code: code=%d, body=%s", resp.StatusCode, body)
		}
		return body, nil
	case http.StatusOK, http.StatusNoContent:
		return body, nil
	default:
		return "", fmt.Errorf("unexpected status code: code=%d, body=%s", resp.StatusCode, body)
	}
}

// WriteError contains info about a write error
type WriteError struct {
	body       string
	statusCode int
}

// StatusCode returns the statuscode
func (wr WriteError) StatusCode() int {
	return wr.statusCode
}

// Body returns body
func (wr WriteError) Body() string {
	return wr.body
}

// Error returns the error string
func (wr WriteError) Error() string {
	return fmt.Sprintf("invalid status code: code=%d, body=%s", wr.statusCode, wr.body)
}

// Write executes a write against the server and returns the results.
func (c *client) Write(db, rp, body string, params url.Values) (results string, err error) {
	if params == nil {
		params = url.Values{}
	}
	if params.Get("db") == "" {
		params.Set("db", db)
	}
	if params.Get("rp") == "" {
		params.Set("rp", rp)
	}
	resp, err := http.Post(c.URL()+"/write?"+params.Encode(), "", strings.NewReader(body))
	if err != nil {
		return "", err
	} else if resp.StatusCode != http.StatusOK && resp.StatusCode != http.StatusNoContent {
		return "", WriteError{statusCode: resp.StatusCode, body: string(MustReadAll(resp.Body))}
	}
	return string(MustReadAll(resp.Body)), nil
}

// MustWrite executes a write to the server. Panic on error.
func (c *client) MustWrite(db, rp, body string, params url.Values) string {
	results, err := c.Write(db, rp, body, params)
	if err != nil {
		panic(err)
	}
	return results
}

// NewConfig returns the default config with temporary paths.
func NewConfig() *run.Config {
	c := run.NewConfig()
	c.BindAddress = "127.0.0.1:0"
	c.ReportingDisabled = true
	c.Coordinator.WriteTimeout = toml.Duration(30 * time.Second)
	c.Meta.Dir = MustTempFile()
	c.Meta.LoggingEnabled = verboseServerLogs

	c.Data.Dir = MustTempFile()
	c.Data.WALDir = MustTempFile()
	c.Data.QueryLogEnabled = verboseServerLogs
	c.Data.TraceLoggingEnabled = verboseServerLogs
	c.Data.Index = indexType

	c.HTTPD.Enabled = true
	c.HTTPD.BindAddress = "127.0.0.1:0"
	c.HTTPD.LogEnabled = verboseServerLogs

	c.Monitor.StoreEnabled = false

	c.Storage.Enabled = false

	return c
}

// NewRetentionPolicySpec creates a retention policy spec
func NewRetentionPolicySpec(name string, rf int, duration time.Duration) *meta.RetentionPolicySpec {
	return &meta.RetentionPolicySpec{Name: name, ReplicaN: &rf, Duration: &duration}
}

func maxInt64() string {
	maxInt64, _ := json.Marshal(^int64(0))
	return string(maxInt64)
}

func mustParseTime(layout, value string) time.Time {
	tm, err := time.Parse(layout, value)
	if err != nil {
		panic(err)
	}
	return tm
}

func mustParseLocation(tzname string) *time.Location {
	loc, err := time.LoadLocation(tzname)
	if err != nil {
		panic(err)
	}
	return loc
}

// LosAngeles is a location
var LosAngeles = mustParseLocation("America/Los_Angeles")

// MustReadAll reads r. Panic on error.
func MustReadAll(r io.Reader) []byte {
	b, err := ioutil.ReadAll(r)
	if err != nil {
		panic(err)
	}
	return b
}

// MustTempFile returns a path to a temporary file.
func MustTempFile() string {
	f, err := ioutil.TempFile("", "influxd-")
	if err != nil {
		panic(err)
	}
	f.Close()
	os.Remove(f.Name())
	return f.Name()
}

func expectPattern(exp, act string) bool {
	re := regexp.MustCompile(exp)
	if !re.MatchString(act) {
		return false
	}
	return true
}

// Query holds information about a query
type Query struct {
	name     string
	command  string
	params   url.Values
	exp, act string
	pattern  bool
	skip     bool
	repeat   int
	once     bool
}

// Execute runs the command and returns an err if it fails
func (q *Query) Execute(s Server) (err error) {
	if q.params == nil {
		q.act, err = s.Query(q.command)
		return
	}
	q.act, err = s.QueryWithParams(q.command, q.params)
	return
}

func (q *Query) success() bool {
	if q.pattern {
		return expectPattern(q.exp, q.act)
	}
	return q.exp == q.act
}

// Error returns the error string
func (q *Query) Error(err error) string {
	return fmt.Sprintf("%s: %v", q.name, err)
}

func (q *Query) failureMessage() string {
	return fmt.Sprintf("%s: unexpected results\nquery:  %s\nparams:  %v\nexp:    %s\nactual: %s\n", q.name, q.command, q.params, q.exp, q.act)
}

// Write holds info about a write operation
type Write struct {
	db   string
	rp   string
	data string
}

func (w *Write) duplicate() *Write {
	return &Write{
		db:   w.db,
		rp:   w.rp,
		data: w.data,
	}
}

// Writes is a slice of write structs
type Writes []*Write

func (a Writes) duplicate() Writes {
	writes := make(Writes, 0, len(a))
	for _, w := range a {
		writes = append(writes, w.duplicate())
	}
	return writes
}

// Tests is a dict of tests
type Tests map[string]Test

// Test holds info about a test
type Test struct {
	initialized bool
	writes      Writes
	params      url.Values
	db          string
	rp          string
	exp         string
	queries     []*Query
}

// NewTest creates a new test
func NewTest(db, rp string) Test {
	return Test{
		db: db,
		rp: rp,
	}
}

func (t Test) duplicate() Test {
	test := Test{
		initialized: t.initialized,
		writes:      t.writes.duplicate(),
		db:          t.db,
		rp:          t.rp,
		exp:         t.exp,
		queries:     make([]*Query, len(t.queries)),
	}

	if t.params != nil {
		t.params = url.Values{}
		for k, a := range t.params {
			vals := make([]string, len(a))
			copy(vals, a)
			test.params[k] = vals
		}
	}
	copy(test.queries, t.queries)
	return test
}

func (t *Test) addQueries(q ...*Query) {
	t.queries = append(t.queries, q...)
}

func (t *Test) database() string {
	if t.db != "" {
		return t.db
	}
	return "db0"
}

func (t *Test) retentionPolicy() string {
	if t.rp != "" {
		return t.rp
	}
	return "default"
}

func (t *Test) init(s Server) error {
	if len(t.writes) == 0 || t.initialized {
		return nil
	}
	if t.db == "" {
		t.db = "db0"
	}
	if t.rp == "" {
		t.rp = "rp0"
	}

	if err := writeTestData(s, t); err != nil {
		return err
	}

	t.initialized = true

	return nil
}

func writeTestData(s Server, t *Test) error {
	for i, w := range t.writes {
		if w.db == "" {
			w.db = t.database()
		}
		if w.rp == "" {
			w.rp = t.retentionPolicy()
		}

		if err := s.CreateDatabaseAndRetentionPolicy(w.db, NewRetentionPolicySpec(w.rp, 1, 0), true); err != nil {
			return err
		}
		if res, err := s.Write(w.db, w.rp, w.data, t.params); err != nil {
			return fmt.Errorf("write #%d: %s", i, err)
		} else if t.exp != res {
			return fmt.Errorf("unexpected results\nexp: %s\ngot: %s%s", t.exp, res, "\n")
		}
	}

	return nil
}

func configureLogging(s Server) {
	// Set the logger to discard unless verbose is on
	if !verboseServerLogs {
		s.SetLogOutput(ioutil.Discard)
	}
}

// BECluster is backend cluster
type BECluster struct {
	backends []Server
}

// NewBECluster creates a new cluster
func NewBECluster(size int) *BECluster {
	c := &BECluster{
		backends: make([]Server, size),
	}

	c.open()
	return c
}

func (c *BECluster) open() {
	for ix := range c.backends {
		c.backends[ix] = OpenDefaultServer(NewConfig())
	}
}

// URLs returns the URLs of the backends
func (c *BECluster) URLs() []string {
	u := make([]string, len(c.backends))
	for ix, s := range c.backends {
		u[ix] = s.URL()
	}

	return u
}

// TearDown closes the backends
func (c *BECluster) TearDown() {
	for _, s := range c.backends {
		s.Close()
	}
}

// WritePoints partitions points randomly and writes each partition to
// a backend
func (c *BECluster) WritePoints(p []models.Point) error {
	nodes := len(c.backends)

	// partion points randomly and write each partition to a backend node
	indices := make([]int, nodes)
	pCount := len(p)
	for ix := range indices {
		indices[ix] = rand.Intn(pCount)
	}

	indices[nodes-1] = pCount

	sort.Ints(indices)
	return c.writePoints(p, indices)
}

// WritePointsOrdered partitions in a deterministic order and writes
// to backends
func (c *BECluster) WritePointsOrdered(p []models.Point) error {
	nodes := len(c.backends)
	if nodes == 0 {
		return fmt.Errorf("No backends")
	}
	perNode := len(p) / nodes
	indices := make([]int, nodes)
	for ix := range indices {
		indices[ix] = (ix + 1) * perNode
	}
	indices[nodes-1] = len(p) // if perNode = 0
	return c.writePoints(p, indices)
}

func (c *BECluster) writePoints(p []models.Point, indices []int) error {
	prev := 0
	for ix, s := range c.backends {
		curr := indices[ix]
		if prev > curr {
			prev = curr
		}
		err := s.WritePoints("db0", "rp0", models.ConsistencyLevelAny, nil, p[prev:curr])
		if err != nil {
			return fmt.Errorf("Failed to write to %s: %v", s.URL(), err)
		}

		log.Printf("Wrote %d points(p[%d:%d]) to be%d", len(p[prev:curr]), prev, curr, ix)
		prev = curr
	}

	return nil
}
