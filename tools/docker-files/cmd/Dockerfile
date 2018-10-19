FROM registry.test.pensando.io:5000/pens-base-2:v0.2
RUN apk add docker
MAINTAINER "Suresh Nalluru" <suresh@pensando.io>
COPY cmd  /
ENTRYPOINT ["/cmd"]
