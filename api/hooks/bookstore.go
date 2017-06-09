package impl

import (
	"context"
	"fmt"

	"github.com/pensando/sw/api/generated/bookstore"
	"github.com/pensando/sw/apiserver"
	apisrvpkg "github.com/pensando/sw/apiserver/pkg"
	"github.com/pensando/sw/utils/log"
)

type bookstoreHooks struct {
	orderId int64
	logger  log.Logger
}

// ServiceHooks
// Precommit hook to create a unique order ID when a order is created via a post.
// The same hook can be used to perform other synchronous actions on receiving an API call.
func (s *bookstoreHooks) createNewOrderId(ctx context.Context, oper string, i interface{}) (interface{}, bool) {
	if oper == "POST" {
		r := i.(bookstore.Order)
		// Here we are just using a local ID. This might more typically be calling a distributed
		// ID generator to reserve an ID.
		s.orderId++
		r.Spec.Id = fmt.Sprintf("order-%x", s.orderId)
		r.Name = r.Spec.Id
		s.logger.InfoLog("msg", "Created new order ID", "order", r.Spec.Id)
		return r, true
	}
	return i, true
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
	r.logger = logger.WithContext("Service", "Bookstore")
	logger.Log("msg", "registering Hooks")
	svc.GetMethod("OrderOper").WithPreCommitHook(r.createNewOrderId).GetRequestType().WithValidate(r.validateOrder)
	svc.GetMethod("DeleteBook").WithPostCommitHook(r.processDelBook)
}

func init() {
	fmt.Printf("registered Hooks")
	apisrv := apisrvpkg.MustGetApiServer()
	apisrv.RegisterHooksCb("bookstore.BookstoreV1", registerBookstoreHooks)
}
