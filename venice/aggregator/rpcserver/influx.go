package rpcserver

import (
	"fmt"
	"strings"
	"time"

	"github.com/pensando/sw/api/generated/metrics_query"
)

type influxSelect struct {
	measurement string
	fields      string
	objSel      string
	metricSel   string
	timerange   string
	slimit      int32
	soffset     int32
}

// InfluxQuery generates influxql queries based on the spec
func InfluxQuery(qs *metrics_query.QuerySpec) ([]string, error) {
	var is influxSelect

	is.measurement = qs.Kind
	is.fields = " * " // send everying for now
	// TODO incorporate tenant/ns/name
	if qs.Selector != nil {
		labelSel, err := qs.Selector.PrintSQL()
		if err != nil {
			return nil, err
		}

		is.objSel = labelSel
	}

	if qs.Pagination != nil {
		is.slimit = qs.Pagination.Count
		is.soffset = qs.Pagination.Offset
	}

	if qs.StartTime != nil {
		beg, err := qs.StartTime.Time()
		if err != nil {
			return nil, err
		}
		is.timerange = fmt.Sprintf("time > '%s'", beg.Format(time.RFC3339Nano))
	}

	if qs.EndTime != nil {
		end, err := qs.EndTime.Time()
		if err != nil {
			return nil, err
		}
		if is.timerange == "" {
			is.timerange = fmt.Sprintf("time < '%s'", end.Format(time.RFC3339Nano))
		} else {
			is.timerange = fmt.Sprintf("%s and time < '%s'", is.timerange, end.Format(time.RFC3339Nano))
		}
	}

	return is.queryStrings(), nil
}

func (is *influxSelect) queryStrings() []string {
	var result []string

	q := fmt.Sprintf("SELECT %s FROM %s ", is.fields, is.measurement)

	var cond []string

	if is.objSel != "" {
		cond = append(cond, is.objSel)
	}

	if is.metricSel != "" {
		cond = append(cond, is.metricSel)
	}

	if is.timerange != "" {
		cond = append(cond, is.timerange)
	}

	if len(cond) != 0 {
		q = q + "WHERE " + strings.Join(cond, " AND ")
	}

	if is.slimit != 0 {
		q = q + fmt.Sprintf(" SLIMIT %d SOFFSET %d", is.slimit, is.soffset)
	}

	result = append(result, q)
	return result
}
