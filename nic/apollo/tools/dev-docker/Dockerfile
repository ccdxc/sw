# This should be executed from <sw> directory
FROM registry.test.pensando.io:5000/naples:base
LABEL maintainer="Pensando Systems Inc"
COPY ./nic/apollo/tools/entrypoint.sh /
ENTRYPOINT ["/entrypoint.sh"]
RUN  mkdir -p /usr/src/github.com/pensando/sw
RUN  mkdir -p /sw

# After login to the docker install the packages mentioned 
# in the docker_packages.txt
