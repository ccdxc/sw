SAVEPATH ?= /tmp
all: buildsim docker-build

buildsim:
	docker run -it --user 501:20 -e "GOCACHE=/import/src/github.com/pensando/sw/.cache" --rm -e "VENICE_CCOMPILE_FORCE=1" \
	-v/Users/sanjayt/Workspace/go/venice/src/github.com/pensando/sw:/import/src/github.com/pensando/sw":cached" \
	-v/Users/sanjayt/Workspace/go/venice/src/github.com/pensando/sw/bin/pkg:/import/pkg":cached" \
	-v/Users/sanjayt/Workspace/go/venice/src/github.com/pensando/sw/bin/cbin:/import/bin":cached" \
	-w /import/src/github.com/pensando/sw registry.test.pensando.io:5000/pens-bld:v0.15 \
	/bin/bash -c "cd api/simulator && go build"

docker-build:
	echo +++ copying ui/dist and docs +++
	@rm -rf docs && mkdir -p docs
	#@rm -rf dist && mkdir -p dist
	@rm -rf keys && mkdir -p keys
	@cp -r ../docs/ docs/
	#@cp -r ../../venice/ui/webapp/dist/ dist/
	#cp -r ../../venice/utils/certmgr/testdata/ keys/
	docker build -t apisim .

docker-save:
	docker save apisim | gzip > ${SAVEPATH}/apisim.tar.gz
