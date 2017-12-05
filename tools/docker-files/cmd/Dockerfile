FROM registry.test.pensando.io:5000/pens-base:v0.1
MAINTAINER "Suresh Nalluru" <suresh@pensando.io>
RUN apk update
RUN apk add libtool
COPY cmd  /cmd
COPY target /target
COPY initscript /initscript
CMD ["-c","/cmd"]
ENTRYPOINT ["/bin/sh"]
