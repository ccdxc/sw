package collectorinteg

import (
	"fmt"
	"time"

	"github.com/influxdata/influxdb/models"
)

const (
	mKey = "reporterID"
)

// Row is a set of columns
type Row struct {
	columns map[string]string
}

// TimeTable is a set of rows indexed by a timestamp
type TimeTable struct {
	name string
	rows map[string]Row
}

// NewTimeTable creates an instance of a TimeTable
func NewTimeTable(name string) *TimeTable {
	return &TimeTable{
		name: name,
		rows: make(map[string]Row),
	}
}

// AddRow adds a row to the table
func (tt *TimeTable) AddRow(ts string, tags map[string]string, fields map[string]interface{}) {
	tt.rows[ts] = MakeRow(tags, fields)
}

// MatchRow checks if r is subset of a row in the table
func (tt *TimeTable) MatchRow(ts string, r Row) bool {
	ref, found := tt.rows[ts]
	if !found {
		return false
	}

	// subset is treated as match
	for k, v := range r.columns {
		if k == mKey { // skip the meta key
			continue
		}

		if ref.columns[k] != v {
			return false
		}
	}

	return true
}

// MatchQueryRow checks if the query result row matches the table
func (tt *TimeTable) MatchQueryRow(qr models.Row) error {
	if qr.Name != tt.name {
		return fmt.Errorf("Expected name %s, got %s", tt.name, qr.Name)
	}

	if len(tt.rows) != len(qr.Values) {
		return fmt.Errorf("Expected %d rows, got %d", len(tt.rows), len(qr.Values))
	}

	for ix, val := range qr.Values {
		ts, row := parseValues(qr.Columns, val)
		if !tt.MatchRow(ts, row) {
			return fmt.Errorf("Row %d ts: %s does not match", ix, ts)
		}
	}

	return nil
}

func parseValues(keys []string, values []interface{}) (string, Row) {
	ts := ""
	row := Row{
		columns: make(map[string]string),
	}

	if len(keys) != len(values) {
		return ts, row
	}

	for ix, k := range keys {
		if k == "time" {
			ts = values[ix].(string)
		} else {
			row.columns[k] = fmt.Sprintf("%v", values[ix])
		}
	}

	return ts, row
}

// MakeRow creates a row
func MakeRow(tags map[string]string, fields map[string]interface{}) Row {
	row := Row{
		columns: make(map[string]string),
	}

	for k, v := range tags {
		row.columns[k] = v
	}

	for k, v := range fields {
		switch v.(type) {
		case int64:
			row.columns[k] = fmt.Sprintf("%d", v.(int64))
		case uint64:
			row.columns[k] = fmt.Sprintf("%d", v.(uint64))
		case float64:
			row.columns[k] = fmt.Sprintf("%v", v.(float64))
		case bool:
			row.columns[k] = fmt.Sprintf("%v", v.(bool))
		case string:
			row.columns[k] = v.(string)
		}
	}

	return row
}

// InfluxTS returns influx formatted timestamp
func InfluxTS(ts time.Time, precision time.Duration) string {
	return ts.Truncate(precision).UTC().Format(time.RFC3339Nano)
}
