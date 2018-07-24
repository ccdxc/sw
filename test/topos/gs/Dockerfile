
# This dockerfile builds the container that untars the installer pre-package for the content in this directory
FROM registry.test.pensando.io:5000/pens-base:v0.2
RUN /bin/sh -c 'mkdir /tmp/pensando/'
COPY Vagrantfile /tmp/pensando/
COPY *.json /tmp/pensando/
COPY README.md /tmp/pensando/
ENTRYPOINT ["/bin/sh", "-c", "cp -f /tmp/pensando/* /import/"]
