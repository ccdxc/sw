package utils

import (
	"context"
	"crypto/tls"
	"fmt"
	"time"

	"github.com/pensando/sw/api/generated/search"
	loginctx "github.com/pensando/sw/api/login/context"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
)

// Search sends a search query to API Gateway
func Search(ctx context.Context, apigw string, query *search.SearchRequest, resp *search.SearchResponse) error {
	searchURL := fmt.Sprintf("https://%s/search/v1/query", apigw)
	restcl := netutils.NewHTTPClient()
	restcl.WithTLSConfig(&tls.Config{InsecureSkipVerify: true})
	// get authz header
	authzHeader, ok := loginctx.AuthzHeaderFromContext(ctx)
	if !ok {
		return fmt.Errorf("no authorizaton header in context")
	}
	restcl.SetHeader("Authorization", authzHeader)
	log.Infof("@@@ Search request: %+v\n", query)
	start := time.Now().UTC()
	_, err := restcl.Req("POST", searchURL, query, &resp)
	log.Infof("@@@ Search response time: %+v\n", time.Since(start))
	if err != nil {
		return err
	}
	log.Infof("@@@ Search response : %+v\n", resp)
	return nil
}
