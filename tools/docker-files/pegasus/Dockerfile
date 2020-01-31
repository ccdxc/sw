FROM centos
COPY nic/ /nic/
ENV LD_LIBRARY_PATH /nic/lib
ENTRYPOINT ["/nic/bin/pegasus"]
