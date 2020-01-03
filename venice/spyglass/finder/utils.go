package finder

import (
	"errors"
	"fmt"
	"strings"

	es "github.com/olivere/elastic"

	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/search"
	"github.com/pensando/sw/api/labels"
)

// QueryBuilder builds Elastic Bool query based on search requirements
func QueryBuilder(req *search.SearchRequest) (es.Query, error) {

	// Validate request
	if req == nil {
		return nil, errors.New("nil search request")
	}

	// Check if QueryString is valid and construct QueryStringQuery
	if len(req.QueryString) != 0 {
		return es.NewQueryStringQuery(req.QueryString), nil
	}

	// Construct Bool query based on search requirements
	if req.Query == nil {
		return nil, errors.New("nil search request")
	}

	query := es.NewBoolQuery().QueryName("CompleteQuery")

	// Process Text Requirements
	textReq := es.NewBoolQuery().QueryName("TextQuery")
	for _, treq := range req.Query.Texts {

		tQuery := es.NewBoolQuery()
		for _, text := range treq.Text {
			if strings.Contains(text, " ") {
				// Phrase query
				tQuery = tQuery.Must(es.NewQueryStringQuery(fmt.Sprintf("\"%s\"", text)))
			} else {
				// Word query
				tQuery = tQuery.Must(es.NewQueryStringQuery(text))
			}
		}
		textReq = textReq.Should(tQuery).MinimumNumberShouldMatch(1)
	}
	if len(req.Query.Texts) > 0 {
		query = query.Must(textReq)
	}

	// Process categories requirement
	catReq := es.NewBoolQuery().QueryName("CategoryQuery")
	for _, cat := range req.Query.Categories {
		catReq = catReq.Should(es.NewTermQuery("meta.labels._category.keyword", cat)).MinimumNumberShouldMatch(1)
	}
	if len(req.Query.Categories) > 0 {
		query = query.Must(catReq)
	}

	// Process kinds requirement
	kindReq := es.NewBoolQuery().QueryName("KindQuery")
	for _, cat := range req.Query.Kinds {
		kindReq = kindReq.Should(es.NewTermQuery("kind.keyword", cat)).MinimumNumberShouldMatch(1)
	}
	if len(req.Query.Kinds) > 0 {
		query = query.Must(kindReq)
	}

	// Process field requirement if it is valid
	if req.Query.Fields != nil {
		for _, field := range req.Query.Fields.Requirements {
			switch field.Operator {
			case fields.Operator_name[int32(fields.Operator_equals)]:
				if len(field.Values) > 0 {
					query = query.Must(es.NewMatchPhraseQuery(field.Key, field.Values[0]))
				}
			case fields.Operator_name[int32(fields.Operator_notEquals)]:
				if len(field.Values) > 0 {
					query = query.MustNot(es.NewMatchPhraseQuery(field.Key, field.Values[0]))
				}
			case fields.Operator_name[int32(fields.Operator_in)]:
				fieldQuery := es.NewBoolQuery().MinimumNumberShouldMatch(1)
				for _, val := range field.GetValues() {
					fieldQuery.Should(es.NewMatchPhraseQuery(field.Key, val))
				}
				query = query.Must(fieldQuery)
			case fields.Operator_name[int32(fields.Operator_notIn)]:
				for _, val := range field.GetValues() {
					query = query.MustNot(es.NewMatchPhraseQuery(field.Key, val))
				}
			case fields.Operator_name[int32(fields.Operator_gt)]:
				if len(field.Values) > 0 {
					query = query.Must(es.NewRangeQuery(field.Key).Gt(field.Values[0]))
				}
			case fields.Operator_name[int32(fields.Operator_gte)]:
				if len(field.Values) > 0 {
					query = query.Must(es.NewRangeQuery(field.Key).Gte(field.Values[0]))
				}
			case fields.Operator_name[int32(fields.Operator_lt)]:
				if len(field.Values) > 0 {
					query = query.Must(es.NewRangeQuery(field.Key).Lt(field.Values[0]))
				}
			case fields.Operator_name[int32(fields.Operator_lte)]:
				if len(field.Values) > 0 {
					query = query.Must(es.NewRangeQuery(field.Key).Lte(field.Values[0]))
				}
			}
		}
	}

	// Process label requirements if it is valid
	if req.Query.Labels != nil {
		for _, label := range req.Query.Labels.Requirements {
			lKey := fmt.Sprintf("%s.keyword", label.Key)
			switch label.Operator {
			case labels.Operator_name[int32(labels.Operator_equals)]:
				if len(label.Values) > 0 {
					query = query.Must(es.NewTermQuery(lKey, label.Values[0]))
				}
			case labels.Operator_name[int32(labels.Operator_notEquals)]:
				if len(label.Values) > 0 {
					query = query.MustNot(es.NewTermQuery(lKey, label.Values[0]))
				}
			case labels.Operator_name[int32(labels.Operator_in)]:
				values := make([]interface{}, len(label.Values))
				for i, v := range label.Values {
					values[i] = v
				}
				query = query.Must(es.NewTermsQuery(lKey, values...))
			case labels.Operator_name[int32(labels.Operator_notIn)]:
				values := make([]interface{}, len(label.Values))
				for i, v := range label.Values {
					values[i] = v
				}
				query = query.MustNot(es.NewTermsQuery(lKey, values...))
			}
		}
	}

	return query, nil
}
