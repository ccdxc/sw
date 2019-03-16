
# Builds the venice tools container
FROM alpine:3.8
RUN /bin/sh -c 'mkdir /tmp/pensando/'
COPY ./bin/cbin/debug/* /tmp/pensando/
ENTRYPOINT ["/bin/sh", "-c", "cp -f /tmp/pensando/* /import/"]
