package impl

import (
	"context"
	"fmt"

	opentracing "github.com/opentracing/opentracing-go"
	"github.com/pkg/errors"

	"github.com/pensando/sw/api/generated/bookstore"
	"github.com/pensando/sw/venice/apiserver"
	apisrvpkg "github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

type bookstoreHooks struct {
	svc     apiserver.Service
	orderID int64
	logger  log.Logger
	tracer  opentracing.Tracer
}

// ServiceHooks
// Precommit hook to create a unique order ID when a order is created via a post.
// The same hook can be used to perform other synchronous actions on receiving an API call.
func (s *bookstoreHooks) createNeworderID(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiserver.APIOperType, i interface{}) (interface{}, bool, error) {
	s.logger.InfoLog("msg", "Got call to Create New orderID")
	r, ok := i.(bookstore.Order)
	if !ok {
		return i, false, errors.New("Invalid input type")
	}

	if oper == apiserver.CreateOper {
		// Here we are just using a local ID. This might more typically be calling a distributed
		// ID generator to reserve an ID.
		s.orderID++
		r.Spec.Id = fmt.Sprintf("order-%x", s.orderID)
		r.Name = r.Spec.Id
		s.logger.InfoLog("msg", "Created new order ID", "order", r.Spec.Id)
		r.Status.Status = "PROCESSING"
		return r, true, nil
	} else if oper == apiserver.UpdateOper {
		// Verify that the current order is actually editable.
		obj, err := s.svc.GetCrudService("Order", apiserver.UpdateOper).GetRequestType().GetFromKv(ctx, key)
		if err != nil {
			return bookstore.Order{}, false, errors.Wrap(err, "precommit hook get key failed")
		}
		cur := obj.(bookstore.Order)
		if bookstore.OrderStatus_OrderStatus_value[cur.Status.Status] > int32(bookstore.OrderStatus_FILLED) {
			return bookstore.Order{}, false, errors.New("order status already shipped")
		}
		// Add a comparator for CAS
		s.logger.Infof("set the comparator version for [%s] as [%s]", key, cur.ResourceVersion)
		txn.AddComparator(kvstore.Compare(kvstore.WithVersion(key), "=", cur.ResourceVersion))
	}

	return i, true, nil
}

// This hook is used to fixup Orders that already have pending orders for the book
// that is being removed from the bookstore. This can potentially involve blocking
// action to clean up and notifiy.
func (s *bookstoreHooks) processDelBook(ctx context.Context, oper apiserver.APIOperType, i interface{}) {
	// This will involve going through the API server cache to retrieve all orders with this bookstore
	// and updating the order as unfulfillable. TBD for now.
	if oper == apiserver.DeleteOper {
		book := i.(bookstore.Book)
		s.logger.InfoLog("msg", "Cleaning up order on delete book", "book", book.Spec.ISBNId)
	}
	return
}

// This hook is to validate that all the items in the order are valid books.
func (s *bookstoreHooks) validateOrder(i interface{}, ver string, ignStatus bool) error {
	r := i.(bookstore.Order)
	for _, oi := range r.Spec.Order {
		s.logger.InfoLog("msg", "Validating Book in order", "book", oi.ISBNId)
		// this would involve going through the API server cache to ensure the Book exists.
	}
	return nil
}

func registerBookstoreHooks(svc apiserver.Service, logger log.Logger) {
	r := bookstoreHooks{}
	r.svc = svc
	r.logger = logger.WithContext("Service", "Bookstore")
	logger.Log("msg", "registering Hooks")
	svc.GetCrudService("Order", apiserver.CreateOper).WithPreCommitHook(r.createNeworderID).GetRequestType().WithValidate(r.validateOrder)
	svc.GetCrudService("Order", apiserver.UpdateOper).WithPreCommitHook(r.createNeworderID).GetRequestType().WithValidate(r.validateOrder)
	svc.GetCrudService("Book", apiserver.DeleteOper).WithPostCommitHook(r.processDelBook)
}

func init() {
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("bookstore.BookstoreV1", registerBookstoreHooks)
}
