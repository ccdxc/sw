# This should be executed from <sw> directory
FROM registry.test.pensando.io:5000/pensando/nic:1.42
LABEL maintainer="Pensando Systems Inc"
COPY ./nic/apollo/tools/athena/sanity/customer-docker/dev_docker_entrypoint.sh /entrypoint.sh
ADD  ./sw.tar /
ADD  ./inc.tar /usr/local/include
ENTRYPOINT ["/entrypoint.sh"]
#RUN  mkdir -p /usr/src/github.com/pensando/sw
#RUN  mkdir -p /sw

# After login to the docker install the packages mentioned 
# in the docker_packages.txt
