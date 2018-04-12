FROM registry.test.pensando.io:5000/elasticsearch:6.2.2 
MAINTAINER "Vijayan Ramakrishnan" <vijayan@pensando.io>
COPY docker-entrypoint.sh /
RUN ["chmod", "+x", "/docker-entrypoint.sh"]
ENV discovery.zen.hosts_provider=file
ENV cluster.name=pen-elasticcluster
ENV xpack.security.enabled=false
ENV xpack.monitoring.enabled=false
ENV xpack.graph.enabled=false
ENV xpack.watcher.enabled=false
ENV NODE_MASTER=true
ENV NODE_DATA=true
ENV HTTP_ENABLE=true
RUN bin/elasticsearch-plugin install https://artifacts.elastic.co/downloads/elasticsearch-plugins/discovery-file/discovery-file-6.2.2.zip
ENTRYPOINT ["/docker-entrypoint.sh"]

