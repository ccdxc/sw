package impl

import (
	"context"
	"fmt"

	opentracing "github.com/opentracing/opentracing-go"
	"github.com/pkg/errors"

	"github.com/pensando/sw/api/generated/bookstore"
	"github.com/pensando/sw/apiserver"
	apisrvpkg "github.com/pensando/sw/apiserver/pkg"
	"github.com/pensando/sw/utils/kvstore"
	"github.com/pensando/sw/utils/log"
)

type bookstoreHooks struct {
	svc     apiserver.Service
	orderId int64
	logger  log.Logger
	tracer  opentracing.Tracer
}

// ServiceHooks
// Precommit hook to create a unique order ID when a order is created via a post.
// The same hook can be used to perform other synchronous actions on receiving an API call.
func (s *bookstoreHooks) createNewOrderId(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key, oper string, i interface{}) (interface{}, bool, error) {
	s.logger.InfoLog("msg", "Got call to Create New OrderID")
	r, ok := i.(bookstore.Order)
	if !ok {
		return i, false, errors.New("Invalid input type")
	}

	if oper == "POST" {
		// Here we are just using a local ID. This might more typically be calling a distributed
		// ID generator to reserve an ID.
		s.orderId++
		r.Spec.Id = fmt.Sprintf("order-%x", s.orderId)
		r.Name = r.Spec.Id
		s.logger.InfoLog("msg", "Created new order ID", "order", r.Spec.Id)
		status := bookstore.OrderStatus{}
		status.Status = bookstore.OrderStatus_PROCESSING
		r.Status = &status
		return r, true, nil
	} else if oper == "PUT" {
		// Verify that the current order is actually editable.
		obj, err := s.svc.GetMethod("OrderOper").GetRequestType().GetFromKv(ctx, key)
		if err != nil {
			return nil, false, errors.Wrap(err, "precommit hook get key failed")
		}
		cur := obj.(bookstore.Order)
		if cur.Status != nil && cur.Status.Status > bookstore.OrderStatus_FILLED {
			return nil, false, errors.New("order status already shipped")
		}
		s.logger.Infof("set the comparator version for [%s] as [%s]", key, cur.ResourceVersion)
		txn.AddComparator(kvstore.Compare(kvstore.WithVersion(key), "=", cur.ResourceVersion))
	}

	return i, true, nil
}

// This hook is used to fixup Orders that already have pending orders for the book
// that is being removed from the bookstore. This can potentially involve blocking
// action to clean up and notifiy.
func (s *bookstoreHooks) processDelBook(ctx context.Context, oper string, i interface{}) {
	// This will involve going through the API server cache to retrieve all orders with this bookstore
	// and updating the order as unfulfillable. TBD for now.
	if oper == "DELETE" {
		book := i.(bookstore.Book)
		s.logger.InfoLog("msg", "Cleaning up order on delete book", "book", book.Spec.ISBNId)
	}
	return
}

// This hook is to validate that all the items in the order are valid books.
func (s *bookstoreHooks) validateOrder(i interface{}) error {
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
	svc.GetMethod("OrderOper").WithPreCommitHook(r.createNewOrderId).GetRequestType().WithValidate(r.validateOrder)
	svc.GetMethod("DeleteBook").WithPostCommitHook(r.processDelBook)
}

func init() {
	fmt.Printf("registered Hooks")
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("bookstore.BookstoreV1", registerBookstoreHooks)
}
