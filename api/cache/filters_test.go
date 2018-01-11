package cache

import (
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/bookstore"
)

func TestGetFilters(t *testing.T) {
	t.Logf("  --> Test empty opts")
	opts := api.ListWatchOptions{}
	filters := getFilters(opts)
	if len(filters) != 0 {
		t.Errorf("expecting empty filters got %d instead", len(filters))
	}
	book := &bookstore.Book{}
	book.Name = "exampleBook"
	book.Tenant = "exampleTenant"
	book.Namespace = "exampleNamespace"
	book.ResourceVersion = "10001"

	passFilter := true
	for _, fn := range filters {
		passFilter = passFilter && fn(book)
	}
	if passFilter != true {
		t.Errorf("expecting to pass filters failed instead")
	}
	t.Logf("  --> Test FromVer opts")
	opts.ResourceVersion = "10001"
	filters = getFilters(opts)
	if len(filters) != 1 {
		t.Errorf("expecting 1 filter got %d instead", len(filters))
	}
	passFilter = true
	for _, fn := range filters {
		passFilter = passFilter && fn(book)
	}
	if passFilter != true {
		t.Errorf("expecting to pass filters failed instead")
	}

	book.ResourceVersion = "10011"
	passFilter = true
	for _, fn := range filters {
		passFilter = passFilter && fn(book)
	}
	if passFilter != true {
		t.Errorf("expecting to pass filters failed instead")
	}

	book.ResourceVersion = "9999"
	passFilter = true
	for _, fn := range filters {
		passFilter = passFilter && fn(book)
	}
	if passFilter != false {
		t.Errorf("expecting to fail filters, passed instead")
	}
	opts.ResourceVersion = ""

	t.Logf("  --> Test Name opts")
	opts.Name = "exampleBook"
	filters = getFilters(opts)
	if len(filters) != 1 {
		t.Errorf("expecting 1 filter got %d instead", len(filters))
	}
	passFilter = true
	for _, fn := range filters {
		passFilter = passFilter && fn(book)
	}
	if passFilter != true {
		t.Errorf("expecting to pass filters failed instead")
	}

	book.Name = "anotherBook"
	passFilter = true
	for _, fn := range filters {
		passFilter = passFilter && fn(book)
	}
	if passFilter != false {
		t.Errorf("expecting to fail filters, passed instead")
	}
	opts.Name = ""

	t.Logf("  --> Test Namespace opts")
	opts.Namespace = "exampleNamespace"
	filters = getFilters(opts)
	if len(filters) != 1 {
		t.Errorf("expecting 1 filter got %d instead", len(filters))
	}
	passFilter = true
	for _, fn := range filters {
		passFilter = passFilter && fn(book)
	}
	if passFilter != true {
		t.Errorf("expecting to pass filters failed instead")
	}

	book.Namespace = "anotherNamespace"
	passFilter = true
	for _, fn := range filters {
		passFilter = passFilter && fn(book)
	}
	if passFilter != false {
		t.Errorf("expecting to fail filters, passed instead")
	}
	opts.Namespace = ""

	t.Logf("  --> Test Tenant opts")
	opts.Tenant = "exampleTenant"
	filters = getFilters(opts)
	if len(filters) != 1 {
		t.Errorf("expecting 1 filter got %d instead", len(filters))
	}
	passFilter = true
	for _, fn := range filters {
		passFilter = passFilter && fn(book)
	}
	if passFilter != true {
		t.Errorf("expecting to pass filters failed instead")
	}

	book.Tenant = "anotherTenant"
	passFilter = true
	for _, fn := range filters {
		passFilter = passFilter && fn(book)
	}
	if passFilter != false {
		t.Errorf("expecting to fail filters, passed instead")
	}
	opts.Tenant = ""

	t.Logf("  --> Test multiple filters in opts")
	opts.ResourceVersion = "10001"
	opts.Name = "anotherBook"
	opts.Namespace = "anotherNamespace"

	filters = getFilters(opts)
	if len(filters) != 3 {
		t.Errorf("expecting 3 filter got %d instead", len(filters))
	}

	book.ResourceVersion = "10011"
	passFilter = true
	for _, fn := range filters {
		passFilter = passFilter && fn(book)
	}
	if passFilter != true {
		t.Errorf("expecting to pass filters failed instead")
	}

	book.ResourceVersion = "9999"
	passFilter = true
	for _, fn := range filters {
		passFilter = passFilter && fn(book)
	}
	if passFilter != false {
		t.Errorf("expecting to fail filters, passed instead")
	}
}
