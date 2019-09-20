FROM registry.test.pensando.io:5000/pens-base-2:v0.5
COPY ctctl /bin
COPY citadel /bin
ENV GODEBUG=madvdontneed=1
ENTRYPOINT ["/bin/citadel"]
