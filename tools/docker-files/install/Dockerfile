FROM registry.test.pensando.io:5000/pens-base:v0.2
MAINTAINER "Suresh Nalluru" <suresh@pensando.io>
COPY target /target
COPY initscript /initscript
ENTRYPOINT ["/initscript"]
