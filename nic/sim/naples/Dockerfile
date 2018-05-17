FROM     registry.test.pensando.io:5000/naples:base

MAINTAINER Pensando

ADD nic.tgz /naples/
COPY bashrc_sim /root/.bashrc
COPY SF-Kingdom1.postman_collection.json /naples/nic/data/examples/
COPY SF-Kingdom2.postman_collection.json /naples/nic/data/examples/
COPY SF-Kingdom1-DEL.postman_collection.json /naples/nic/data/examples/
COPY postman_env.json /naples/nic/data/examples/
COPY README /naples/nic/data/examples/
COPY bootstrap.sh /naples/nic/tools/

HEALTHCHECK --interval=2m --timeout=5s --start-period=5m --retries=3 CMD /naples/nic/tools/health-check.sh || exit 1

CMD ["/naples/nic/tools/start-naples.sh"]
