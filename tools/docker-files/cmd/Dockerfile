FROM srv1.pensando.io:5000/pens-base:v0.1
MAINTAINER "Suresh Nalluru" <suresh@pensando.io>
COPY cmd  /cmd
COPY target /target
COPY initscript /initscript
CMD ["-c","/cmd"]
ENTRYPOINT ["/bin/sh"]
