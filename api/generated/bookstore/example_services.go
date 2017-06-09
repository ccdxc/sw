/*
Package bookstore is a auto generated package.
Input file: protos/example.proto
*/
package bookstore

import (
	"context"
)

type ServiceBookstoreV1 interface {
	AddPublisher(ctx context.Context, t Publisher) (Publisher, error)
	UpdatePublisher(ctx context.Context, t Publisher) (Publisher, error)
	DeletePublisher(ctx context.Context, t Publisher) (Publisher, error)
	GetPublisher(ctx context.Context, t Publisher) (Publisher, error)
	GetBook(ctx context.Context, t Book) (Book, error)
	AddBook(ctx context.Context, t Book) (Book, error)
	UpdateBook(ctx context.Context, t Book) (Book, error)
	DeleteBook(ctx context.Context, t Book) (Book, error)
	OrderOper(ctx context.Context, t Order) (Order, error)
}
