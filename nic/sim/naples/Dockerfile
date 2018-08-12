FROM     registry.test.pensando.io:5000/naples:base

LABEL maintainer="Pensando Systems Inc"

COPY bashrc_sim /root/.bashrc
RUN  mkdir -p /naples/nic/data/examples/config_ipsec
RUN  mkdir -p /naples/nic/data/examples/config_vxlan_overlay
COPY SF-Kingdom1.postman_collection.json /naples/nic/data/examples/config_ipsec/Kingdom1.postman_collection.json
COPY SF-Kingdom2.postman_collection.json /naples/nic/data/examples/config_ipsec/Kingdom2.postman_collection.json
COPY SF-Kingdom1-DEL.postman_collection.json /naples/nic/data/examples/config_ipsec/Kingdom1-DEL.postman_collection.json
COPY postman_env.json /naples/nic/data/examples/postman_env.json
COPY cfg-overlay/Node1.postman_collection.json cfg-overlay/Node2.postman_collection.json /naples/nic/data/examples/config_vxlan_overlay/
COPY cfg-overlay/Node1.Delete.postman_collection.json cfg-overlay/Node2.Delete.postman_collection.json /naples/nic/data/examples/config_vxlan_overlay/
COPY cfg-overlay/Node1.security_policy.json cfg-overlay/Node1.security_policy_del.json cfg-overlay/Node2.security_policy.json cfg-overlay/Node2.security_policy_del.json /naples/nic/data/examples/config_vxlan_overlay/
COPY README /naples/nic/data/examples/config_ipsec/README
COPY cfg-overlay/README /naples/nic/data/examples/config_vxlan_overlay/README
COPY bootstrap.sh /naples/nic/tools/
HEALTHCHECK --interval=2m --timeout=5s --start-period=5m --retries=3 CMD /naples/nic/tools/health-check.sh || exit 1
ADD nic.tgz /naples/

CMD ["/naples/nic/tools/start-naples.sh"]
