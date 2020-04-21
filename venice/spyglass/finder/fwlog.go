package finder

import (
	"compress/gzip"
	"context"
	"encoding/csv"
	"encoding/json"
	"errors"
	"fmt"
	"strconv"
	"strings"
	"time"

	"github.com/gogo/protobuf/types"
	es "github.com/olivere/elastic"
	"google.golang.org/grpc/codes"
	"google.golang.org/grpc/status"

	"github.com/pensando/sw/api"
	apierrors "github.com/pensando/sw/api/errors"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/fwlog"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/spyglass/indexer"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/log"
	objstore "github.com/pensando/sw/venice/utils/objstore/client"
)

const (
	waitIntvl  = time.Second
	maxRetries = 200
)

type fwlogHandler struct {
	fdr                 *Finder
	vosFwLogsHTTPClient objstore.Client
}

// GetLogs returns firewall logs based upon the provided query
func (a *fwlogHandler) GetLogs(ctx context.Context, r *fwlog.FwLogQuery) (*fwlog.FwLogList, error) {
	errs := validateFwLogQuery(r)
	if len(errs) > 0 {
		var errorStrings []string
		for _, err := range errs {
			errorStrings = append(errorStrings, err.Error())
		}
		err := apierrors.ToGrpcError("Validation Failed", errorStrings, int32(codes.InvalidArgument), "", nil)
		a.fdr.logger.ErrorLog("method", "GetLogs", "msg", "validation of fwlog search query failed", "error", err)
		return nil, err
	}
	// construct query
	query := fwlogQuery(r)
	var index string
	if r.StartTime == nil && r.EndTime == nil { // get the latest index if time interval is not specified in the query
		index = elastic.GetIndex(globals.FwLogs, r.Tenants[0])
	} else {
		index = strings.ToLower(fmt.Sprintf("%s.%s.%s.*", elastic.ExternalIndexPrefix, r.Tenants[0], elastic.GetDocType(globals.FwLogs)))
	}
	// execute query
	result, err := a.fdr.elasticClient.Search(ctx,
		index, // search only in fwlogs indices
		elastic.GetDocType(globals.FwLogs),
		query, // query to be executed
		nil,   // no aggregation
		0,     // from
		r.MaxResults,
		"meta.creation-time",
		r.SortOrder == fwlog.SortOrder_Ascending.String())
	if err != nil {
		a.fdr.logger.ErrorLog("method", "GetLogs", "msg", "failed to query ElasticSearch", "error", err)
		return nil, status.Errorf(codes.Internal, "search failed for firewall logs: %v", err)
	}

	fwlogList := fwlog.FwLogList{}
	fwlogList.Kind = fmt.Sprintf("%sList", auth.Permission_FwLog.String())
	fwlogList.TotalCount = int32(result.Hits.TotalHits)
	for i, res := range result.Hits.Hits {
		var log fwlog.FwLog
		if err := json.Unmarshal(*res.Source, &log); err != nil {
			a.fdr.logger.DebugLog("method", "GetLogs", "msg", "failed to unmarshal elasticsearch result", "error", err)
			continue
		}
		a.fdr.logger.DebugLog("method", "GetLogs", "msg", fmt.Sprintf("Search hits result - fwlog entry: %d {%+v}", i, log))
		fwlogList.Items = append(fwlogList.Items, &log)
	}
	return &fwlogList, nil
}

// AutoWatchSvcAuditV1 is not implemented
func (a *fwlogHandler) AutoWatchSvcFwLogV1(*api.ListWatchOptions, fwlog.FwLogV1_AutoWatchSvcFwLogV1Server) error {
	return errors.New("not implemented")
}

func newFwLogHandler(fdr *Finder) (fwlog.FwLogV1Server, error) {
	fh := &fwlogHandler{fdr: fdr}
	if fdr.vosFinder {
		// Create objstore http client for fwlogs
		result, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
			return indexer.CreateBucketClient(ctx, fdr.rsr, globals.DefaultTenant, globals.FwlogsBucketName)
		}, waitIntvl, maxRetries)
		if err != nil {
			log.Errorf("Failed to create objstore client for fwlogs")
			return nil, err
		}
		fh.vosFwLogsHTTPClient = result.(objstore.Client)
	}
	return fh, nil
}

func fwlogQuery(r *fwlog.FwLogQuery) es.Query {
	query := es.NewBoolQuery().QueryName("CompleteQuery")
	if len(r.SourceIPs) > 0 {
		// search by source ips
		sQuery := es.NewBoolQuery().QueryName("SourceIPQuery")
		for _, sourceIP := range r.SourceIPs {
			sQuery = sQuery.Should(es.NewMatchPhraseQuery("source-ip", sourceIP))
		}
		sQuery.MinimumNumberShouldMatch(1)
		query = query.Must(sQuery)
	}
	if len(r.DestIPs) > 0 {
		// search by destination ips
		sQuery := es.NewBoolQuery().QueryName("DestinationIPQuery")
		for _, destIP := range r.DestIPs {
			sQuery = sQuery.Should(es.NewMatchPhraseQuery("destination-ip", destIP))
		}
		sQuery.MinimumNumberShouldMatch(1)
		query = query.Must(sQuery)
	}
	if len(r.SourcePorts) > 0 {
		// search by source ports
		sQuery := es.NewBoolQuery().QueryName("SourcePortQuery")
		for _, port := range r.SourcePorts {
			sQuery = sQuery.Should(es.NewMatchPhraseQuery("source-port", port))
		}
		sQuery.MinimumNumberShouldMatch(1)
		query = query.Must(sQuery)
	}
	if len(r.DestPorts) > 0 {
		// search by destination ports
		sQuery := es.NewBoolQuery().QueryName("DestinationPortQuery")
		for _, port := range r.DestPorts {
			sQuery = sQuery.Should(es.NewMatchPhraseQuery("destination-port", port))
		}
		sQuery.MinimumNumberShouldMatch(1)
		query = query.Must(sQuery)
	}
	if len(r.Protocols) > 0 {
		sQuery := es.NewBoolQuery().QueryName("ProtocolQuery")
		for _, protocol := range r.Protocols {
			sQuery = sQuery.Should(es.NewMatchPhraseQuery("protocol", protocol))
		}
		sQuery.MinimumNumberShouldMatch(1)
		query = query.Must(sQuery)
	}
	if len(r.Actions) > 0 {
		sQuery := es.NewBoolQuery().QueryName("ActionQuery")
		for _, action := range r.Actions {
			sQuery = sQuery.Should(es.NewMatchPhraseQuery("action", action))
		}
		sQuery.MinimumNumberShouldMatch(1)
		query = query.Must(sQuery)
	}
	if len(r.ReporterIDs) > 0 {
		sQuery := es.NewBoolQuery().QueryName("ReporterIDQuery")
		for _, reporterID := range r.ReporterIDs {
			sQuery = sQuery.Should(es.NewMatchPhraseQuery("reporter-id", reporterID))
		}
		sQuery.MinimumNumberShouldMatch(1)
		query = query.Must(sQuery)
	}
	// set time window
	if r.StartTime != nil {
		startTime, err := r.StartTime.Time()
		if err == nil {
			query = query.Must(es.NewRangeQuery("meta.creation-time").Gte(startTime.Format(time.RFC3339Nano)))
		}
	}
	if r.EndTime != nil {
		endTime, err := r.EndTime.Time()
		if err == nil {
			query = query.Must(es.NewRangeQuery("meta.creation-time").Lte(endTime.Format(time.RFC3339Nano)))
		}
	}
	return query
}

func validateFwLogQuery(r *fwlog.FwLogQuery) []error {
	var errs []error
	errs = append(errs, r.Validate("all", "", true, false)...)
	if len(r.Tenants) != 1 {
		errs = append(errs, errors.New("only one tenant can be queried at a time"))
	}
	if len(r.SourceIPs) > 0 && len(r.SourceIPs) != 1 {
		errs = append(errs, errors.New("only one source IP can be specified"))
	}
	if len(r.DestIPs) > 0 && len(r.DestIPs) != 1 {
		errs = append(errs, errors.New("only one destination IP can be specified"))
	}
	if len(r.ReporterIDs) > 0 && len(r.ReporterIDs) != 1 {
		errs = append(errs, errors.New("only one reporter ID can be specified"))
	}
	// validate source ports
	switch len(r.SourcePorts) {
	case 0:
	case 1: // source IP should be present
		if len(r.SourceIPs) == 0 {
			errs = append(errs, errors.New("source IP should be specified with source port"))
		}
	default: // only one source IP allowed
		errs = append(errs, errors.New("only one source port is supported"))
		if len(r.SourceIPs) == 0 {
			errs = append(errs, errors.New("source IP should be specified with source port"))
		}
	}
	// validate destination ports
	switch len(r.DestPorts) {
	case 0:
	case 1: // destination IP should be present
		if len(r.DestIPs) == 0 {
			errs = append(errs, errors.New("destination IP should be specified with destination port"))
		}
	default: // only one destination IP allowed
		errs = append(errs, errors.New("only one destination port is supported"))
		if len(r.DestIPs) == 0 {
			errs = append(errs, errors.New("destination IP should be specified with destination port"))
		}
	}
	// validate protocols
	switch len(r.Protocols) {
	case 0:
	case 1: // source or destination IP should be present
		if len(r.SourceIPs) == 0 && len(r.DestIPs) == 0 {
			errs = append(errs, errors.New("at least one of source or destination IP should be present in the query if protocol is specified"))
		}
	default: // only one protocol value allowed
		errs = append(errs, errors.New("only one protocol can be specified"))
		if len(r.SourceIPs) == 0 && len(r.DestIPs) == 0 {
			errs = append(errs, errors.New("at least one of source or destination IP should be present in the query if protocol is specified"))
		}
	}
	// validate actions
	switch len(r.Actions) {
	case 0:
	case 1: // source or destination IP should  be present
		if len(r.SourceIPs) == 0 && len(r.DestIPs) == 0 {
			errs = append(errs, errors.New("at least one of source or destination IP should be present in the query if action is specified"))
		}
	default: // only one action value allowed
		errs = append(errs, errors.New("only one action can be specified"))
		if len(r.SourceIPs) == 0 && len(r.DestIPs) == 0 {
			errs = append(errs, errors.New("at least one of source or destination IP should be present in the query if action is specified"))
		}
	}
	return errs
}

// DownloadFwLogFileContent downloads the fwlog object content.
func (a *fwlogHandler) DownloadFwLogFileContent(ctx context.Context, in *api.ListWatchOptions) (*fwlog.FwLogList, error) {
	log.Infof("got call to DownloadFwLogFileContent %s, %s", in.Tenant, in.Name)
	if in.Tenant != "" && in.Tenant != globals.DefaultTenant {
		return nil, fmt.Errorf("query is supported only for 'default' tenant")
	}
	// convert 1st 5 "_" to "/"
	name := strings.Replace(in.Name, "_", "/", 5)
	objReader, err := a.vosFwLogsHTTPClient.GetObject(ctx, name)
	if err != nil {
		return nil, err
	}
	defer objReader.Close()
	dscID := strings.Split(name, "/")[0]
	zipReader, err := gzip.NewReader(objReader)
	if err != nil {
		a.fdr.logger.ErrorLog("method", "DownloadFwLogFileContent", "msg", "error in unzipping object", "error", err)
		return nil, err
	}

	rd := csv.NewReader(zipReader)
	data, err := rd.ReadAll()
	if err != nil {
		a.fdr.logger.ErrorLog("method", "DownloadFwLogFileContent", "msg", "error in reading object", "error", err)
		return nil, err
	}
	fwlogList := fwlog.FwLogList{}
	fwlogList.Kind = fmt.Sprintf("%sList", auth.Permission_FwLog.String())
	fwlogList.TotalCount = int32(len(data))

	for i := 1; i < len(data); i++ {
		line := data[i]

		ts, err := time.Parse(time.RFC3339, line[4])
		if err != nil {
			a.fdr.logger.ErrorLog("method", "DownloadFwLogFileContent", "msg", "error in parsing time", "error", err)
			return nil, fmt.Errorf("error in parsing time %s", err.Error())
		}

		timestamp, err := types.TimestampProto(ts)
		if err != nil {
			a.fdr.logger.ErrorLog("method", "DownloadFwLogFileContent", "msg", "error in converting time to proto", "error", err)
			return nil, fmt.Errorf("error in converting time to proto %s", err.Error())
		}

		srcVRF, err := strconv.ParseUint(line[0], 10, 64)
		if err != nil {
			a.fdr.logger.ErrorLog("method", "DownloadFwLogFileContent", "msg", "error in conversion", "error", err)
			return nil, fmt.Errorf("error in conversion err %s", err.Error())
		}

		sport, err := strconv.ParseUint(line[5], 10, 64)
		if err != nil {
			a.fdr.logger.ErrorLog("method", "DownloadFwLogFileContent", "msg", "error in conversion", "error", err)
			return nil, fmt.Errorf("error in conversion err %s", err.Error())
		}

		dport, err := strconv.ParseUint(line[6], 10, 64)
		if err != nil {
			a.fdr.logger.ErrorLog("method", "DownloadFwLogFileContent", "msg", "error in conversion", "error", err)
			return nil, fmt.Errorf("error in conversion err %s", err.Error())
		}

		icmpType, err := strconv.ParseUint(line[13], 10, 64)
		if err != nil {
			a.fdr.logger.ErrorLog("method", "DownloadFwLogFileContent", "msg", "error in conversion", "error", err)
			return nil, fmt.Errorf("error in conversion err %s", err.Error())
		}

		icmpID, err := strconv.ParseUint(line[14], 10, 64)
		if err != nil {
			a.fdr.logger.ErrorLog("method", "DownloadFwLogFileContent", "msg", "error in conversion", "error", err)
			return nil, fmt.Errorf("error in conversion err %s", err.Error())
		}

		icmpCode, err := strconv.ParseUint(line[15], 10, 64)
		if err != nil {
			a.fdr.logger.ErrorLog("method", "DownloadFwLogFileContent", "msg", "error in conversion", "error", err)
			return nil, fmt.Errorf("error in conversion err %s", err.Error())
		}

		obj := fwlog.FwLog{
			TypeMeta: api.TypeMeta{
				Kind: "FwLog",
			},
			ObjectMeta: api.ObjectMeta{
				Tenant: globals.DefaultTenant,
				CreationTime: api.Timestamp{
					Timestamp: *timestamp,
				},
				ModTime: api.Timestamp{
					Timestamp: *timestamp,
				},
			},
			SrcVRF:     srcVRF,
			SrcIP:      line[2],
			DestIP:     line[3],
			SrcPort:    uint32(sport),
			DestPort:   uint32(dport),
			Protocol:   line[7],
			Action:     line[8],
			Direction:  line[9],
			RuleID:     line[10],
			SessionID:  line[11],
			ReporterID: dscID,
			FlowAction: line[12],
			IcmpType:   uint32(icmpType),
			IcmpID:     uint32(icmpID),
			IcmpCode:   uint32(icmpCode),
		}

		fwlogList.Items = append(fwlogList.Items, &obj)
	}

	return &fwlogList, nil
}
