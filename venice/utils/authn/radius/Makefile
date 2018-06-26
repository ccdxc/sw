all:
	$(MAKE) gen

gen:
	radius-dict-gen -package generated -output generated/pensando.go  dictionary.pensando
	goimports -local "github.com/pensando/sw" -l -w generated/pensando.go
