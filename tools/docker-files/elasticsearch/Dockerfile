FROM docker.elastic.co/elasticsearch/elasticsearch:6.3.2
MAINTAINER "Vijayan Ramakrishnan" <vijayan@pensando.io>
COPY docker-entrypoint.sh /
RUN ["chmod", "+x", "/docker-entrypoint.sh"]
ENV discovery.zen.hosts_provider=file
ENV discovery.zen.fd.ping_timeout=10s
ENV cluster.name=pen-elasticcluster
ENV path.logs=/var/log/pensando/elastic
ENV xpack.security.enabled=false
ENV xpack.monitoring.enabled=false
ENV xpack.graph.enabled=false
ENV xpack.watcher.enabled=false

ENV searchguard.enterprise_modules_enabled=false
ENV searchguard.ssl.transport.pemcert_filepath=auth-node/cert.pem
ENV searchguard.ssl.transport.pemkey_filepath=auth-node/key.pem
ENV searchguard.ssl.transport.pemtrustedcas_filepath=auth-node/ca-bundle.pem
ENV searchguard.ssl.transport.enforce_hostname_verification=false
ENV searchguard.nodes_dn="CN=pen-elastic-node"
ENV searchguard.allow_default_init_sgindex=true

ENV searchguard.ssl.http.enabled=true
ENV searchguard.ssl.http.clientauth_mode="REQUIRE"
ENV searchguard.ssl.http.enabled_protocols="TLSv1.2"
ENV searchguard.ssl.http.pemkey_filepath=auth-https/key.pem
ENV searchguard.ssl.http.pemcert_filepath=auth-https/cert.pem
ENV searchguard.ssl.http.pemtrustedcas_filepath=auth-https/ca-bundle.pem

ENV NODE_MASTER=true
ENV NODE_DATA=true
ENV HTTP_ENABLE=true
RUN bin/elasticsearch-plugin install https://artifacts.elastic.co/downloads/elasticsearch-plugins/discovery-file/discovery-file-6.3.2.zip
RUN bin/elasticsearch-plugin install -b com.floragunn:search-guard-6:6.3.2-25.5
COPY sg_config.yml /usr/share/elasticsearch/plugins/search-guard-6/sgconfig
COPY sg_roles.yml /usr/share/elasticsearch/plugins/search-guard-6/sgconfig
COPY sg_roles_mapping.yml /usr/share/elasticsearch/plugins/search-guard-6/sgconfig
COPY log4j2.properties /usr/share/elasticsearch/config/log4j2.properties
COPY jvm.options /usr/share/elasticsearch/config/jvm.options

ENTRYPOINT ["/docker-entrypoint.sh"]
