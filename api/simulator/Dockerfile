FROM registry.test.pensando.io:5000/pens-base-2:latest
MAINTAINER "Sanjay Thyamagundalu" <sanjayt@pensando.io>
COPY simulator /
COPY dist /dist
COPY docs /docs
COPY keys /keys
ENTRYPOINT ["/simulator"]
