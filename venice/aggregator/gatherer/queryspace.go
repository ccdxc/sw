package gatherer

import (
	"encoding/json"
	"errors"
	"fmt"
	"log"
	"reflect"
	"regexp"
	"strings"
	"time"

	"github.com/influxdata/influxdb/client/v2"
	"github.com/influxdata/influxdb/coordinator"
	"github.com/influxdata/influxdb/models"
	"github.com/influxdata/influxdb/query"
	"github.com/influxdata/influxql"
)

// QuerySpace holds enough information to gather points from backends
// This is specific to a given query context
type QuerySpace struct {
	subQuery     string
	shardClients []client.Client
	pw           *coordinator.PointsWriter
}

// NewQuerySpace returns an instance of QuerySpace
func NewQuerySpace(pw *coordinator.PointsWriter) *QuerySpace {
	gtrMeta.RLock()
	defer gtrMeta.RUnlock()
	qs := &QuerySpace{
		shardClients: make([]client.Client, len(gtrMeta.backends)),
		pw:           pw,
	}
	copy(qs.shardClients, gtrMeta.backends)

	return qs
}

// gatherMeta collects meta data from all backends and returns a union
// Used for background sync of meta
func (qa *QuerySpace) gatherMeta(db string) (map[string]*mMeta, error) {
	tags, err := qa.gatherTagsMeta(db)
	if err != nil {
		return nil, err
	}
	fields, err := qa.gatherFieldsMeta(db)
	if err != nil {
		return nil, err
	}

	// merge the meta into required map format
	m := make(map[string]*mMeta)

	for meas, tagList := range tags {
		mM := &mMeta{tags: tagList}
		m[meas] = mM
	}

	for meas, fieldMap := range fields {
		mM := m[meas]
		if mM == nil {
			mM = &mMeta{}
		}
		mM.fields = fieldMap
		m[meas] = mM
	}

	return m, nil
}
func (qa *QuerySpace) gatherTagsMeta(db string) (map[string][]string, error) {
	// each backend will produce a map of tags for ease of combining
	respCh := make(chan map[string]map[string]string, len(qa.shardClients))
	rcv := func(res []client.Result) {
		dbMeta := make(map[string]map[string]string)
		for _, r := range res {
			for _, s := range r.Series {
				tags := make(map[string]string)
				dbMeta[s.Name] = tags
				for _, v := range s.Values {
					key := v[0].(string)
					tags[key] = ""
				}
			}
		}

		respCh <- dbMeta
	}

	cmd := "SHOW TAG KEYS"
	err := qa.queryAll(db, cmd, rcv)
	if err != nil {
		return nil, err
	}

	// read all results and merge
	accum := qa.mergeMetaResp(respCh)

	// finally convert tag keys into a slice
	result := make(map[string][]string)
	for meas, tagMap := range accum {
		tagList := make([]string, 0, len(tagMap))
		for t := range tagMap {
			tagList = append(tagList, t)
		}

		result[meas] = tagList
	}
	return result, nil
}

func merge(a, b map[string]string) map[string]string {

	if a == nil {
		return b
	}

	if b == nil {
		return a
	}

	first, second := a, b

	if len(b) > len(a) {
		first, second = b, a
	}

	for k, v := range second {
		first[k] = v
	}

	return first
}

func getIndices(columns []string) (int, int) {
	var fieldIndex, typeIndex int

	for ix, c := range columns {
		switch c {
		case "fieldKey":
			fieldIndex = ix
		case "fieldType":
			typeIndex = ix
		}
	}

	return fieldIndex, typeIndex
}

func (qa *QuerySpace) gatherFieldsMeta(db string) (map[string]map[string]string, error) {
	// each backend will produce a map of fields
	respCh := make(chan map[string]map[string]string, len(qa.shardClients))
	rcv := func(res []client.Result) {
		dbMeta := make(map[string]map[string]string)
		for _, r := range res {
			for _, s := range r.Series {
				fields := make(map[string]string)
				dbMeta[s.Name] = fields
				ki, vi := getIndices(s.Columns)
				for _, v := range s.Values {
					key := v[ki].(string)
					val := v[vi].(string)
					fields[key] = val
				}
			}
		}

		respCh <- dbMeta
	}

	cmd := "SHOW FIELD KEYS"
	err := qa.queryAll(db, cmd, rcv)
	if err != nil {
		return nil, err
	}

	// read all results and merge
	accum := qa.mergeMetaResp(respCh)
	return accum, nil
}

func (qa *QuerySpace) mergeMetaResp(ch chan map[string]map[string]string) map[string]map[string]string {
	// read all responses and merge
	accum := make(map[string]map[string]string)
	for range qa.shardClients {
		m := <-ch
		// merge each meas map into result
		for meas, t := range m {
			accum[meas] = merge(accum[meas], t)
		}
	}

	return accum
}

// GatherPoints collects all applicable points for the query from the
// backends and writes them using the point writer
func (qa *QuerySpace) GatherPoints(stmt influxql.Statement, db string) error {
	var res error
	defer func() {
		if res != nil {
			log.Printf("..error: %s", res.Error())
		}
	}()

	newStmt, err := query.RewriteStatement(stmt)
	if err != nil {
		res = err
		return err
	}

	ss, ok := newStmt.(*influxql.SelectStatement)
	if !ok {
		res = errors.New("Only select statement supported by aggregator")
		return res
	}

	query, err := FormSubquery(ss, query.CompileOptions{})
	if err != nil {
		res = err
		return err
	}

	qa.subQuery = query
	tmp1 := strings.Split(query, " ")
	if len(tmp1) < 4 {
		res = errors.New("Bad query " + query)
		return res
	}
	tmp2 := strings.Split(tmp1[3], ".")
	rp := ""
	meas := ""

	switch len(tmp2) {
	case 3:
		if db != "" {
			res = errors.New("db repeated " + query)
			return res
		}
		db = tmp2[0]
		rp = tmp2[1]
		meas = tmp2[2]
	case 2:
		if db == "" {
			db = tmp2[0]
			meas = tmp2[1]
		} else {
			rp = tmp2[0]
			meas = tmp2[1]
		}
	case 1:
		if db == "" {
			res = errors.New("db missing " + query)
			return res
		}
		meas = tmp2[0]

	}

	rcv := func(res []client.Result) {
		for _, r := range res {
			for _, s := range r.Series {
				// convert rows to points and write to local DB
				points, err1 := ConvertRowToPoints(db, meas, &s)
				if err1 != nil {
					continue
				}

				err = qa.pw.WritePointsInto(&coordinator.IntoWriteRequest{
					Database:        db,
					RetentionPolicy: rp,
					Points:          points,
				})
				if err != nil {
					log.Printf("WritePointsInto err %s", err.Error())
					return
				}
			}
		}
	}

	err = qa.queryAll(db, qa.subQuery, rcv)
	if err != nil {
		res = err
		return err
	}

	return nil
}

// queryAll sends the query to all backends and invokes the rcvFn func
// if any backend responds with error, error is returned. rcvFn is
// still invoked for successful backends
func (qa *QuerySpace) queryAll(db, cmd string, rcvFn func(r []client.Result)) error {
	errCh := make(chan error, len(qa.shardClients))

	gather := func(c client.Client) {
		var err error
		var response *client.Response
		defer func() { errCh <- err }()

		response, err = c.Query(client.Query{Command: cmd, Database: db})
		if err != nil {
			return
		}

		if response.Err != "" {
			err = errors.New(response.Err)
			return
		}

		for _, res := range response.Results {
			if res.Err != "" {
				err = errors.New(res.Err)
				return
			}
		}

		// let receiver take it from here
		rcvFn(response.Results)
	}

	// gather results from all clients
	for _, c := range qa.shardClients {
		go gather(c)
	}

	for range qa.shardClients {
		err := <-errCh
		if err != nil {
			return err
		}
	}

	return nil
}

// ConvertRowToPoints will convert a query result Row into Points that can be written back in.
func ConvertRowToPoints(db, meas string, row *models.Row) ([]models.Point, error) {
	parsers, err := getColumnParsers(db, meas, row.Columns)
	if err != nil {
		return nil, err
	}

	points := make([]models.Point, 0, len(row.Values))
	for _, v := range row.Values {
		fields := make(map[string]interface{})
		tags := make(map[string]string)
		for ix, pfunc := range parsers {
			name := row.Columns[ix]
			var val string
			switch vt := v[ix].(type) {
			case json.Number:
				val = vt.String()
			case string:
				val = vt
			default:
				return nil, fmt.Errorf("%s is unknown type %s", name, reflect.TypeOf(v[ix]))
			}

			err := pfunc(name, val, tags, fields)
			if err != nil {
				return nil, err
			}
		}

		tss, ok := fields["time"]
		if !ok {
			return nil, fmt.Errorf("Missing time")
		}
		delete(fields, "time")
		ts := tss.(time.Time)

		p, err := models.NewPoint(meas, models.NewTags(tags), fields, ts)
		if err != nil {
			// Drop points that can't be stored
			continue
		}

		points = append(points, p)
	}

	return points, nil
}

// FormSubquery creates an appropriate subquery based on the passed select
func FormSubquery(stmt *influxql.SelectStatement, opt query.CompileOptions) (string, error) {
	c := newCompiler(opt)
	if err := c.preprocess(stmt); err != nil {
		return "", err
	}
	if err := c.compileFields(stmt); err != nil {
		return "", err
	}
	if err := c.validateFields(); err != nil {
		return "", err
	}

	// recurse to find the innermost subquery
	var getSQ func(stmt *influxql.SelectStatement) (string, error)
	getSQ = func(stmt *influxql.SelectStatement) (string, error) {
		switch len(stmt.Sources) {
		case 1:
			if _, ok := stmt.Sources[0].(*influxql.Measurement); ok {
				sq := trimQuery(stmt.String())
				return sq, nil
			}

			s := stmt.Sources[0].(*influxql.SubQuery)
			return getSQ(s.Statement)

		default:
			return "", errors.New("Multiple sources not implemented")
		}

	}

	return getSQ(stmt)
}

// trimQuery trims the query to eliminate result filters
// For now, we take a simple approach. Further optimization is possible for
// specific scenarios.
func trimQuery(q string) string {
	// replace SELECT.*FROM with SELECT * FROM to make sure we get all
	// fields and tags

	sq := q
	for _, sub := range []struct {
		find    string
		replace string
	}{
		// replace SELECT.*FROM with SELECT * FROM to make sure we get
		// all fields and tags
		{find: `SELECT.*FROM `, replace: `SELECT * FROM `},

		// eliminate SLIMIT, OFFSET, LIMIT, ORDER BY and GROUP BY
		{find: ` SLIMIT.*`, replace: ""},
		{find: ` OFFSET.*`, replace: ""},
		{find: ` LIMIT.*`, replace: ""},
		{find: ` ORDER BY.*`, replace: ""},
		{find: ` GROUP BY.*`, replace: ""},
	} {
		re := regexp.MustCompile(sub.find)
		sq = re.ReplaceAllString(sq, sub.replace)
	}

	return sq
}
