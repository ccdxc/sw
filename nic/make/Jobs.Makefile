.PHONY: l4lb l4lb_dsr pull-assets

l4lb: pull-assets bazel-build
	./run.py --topo l4lb --feature l4lb --test BRIDGED_NW_NW,ROUTED_NW_NW,BRIDGED_TX_NW,ROUTED_TX_NW

l4lb_dsr: pull-assets bazel-build
	cd /sw && make pull-assets && cd nic && make bazel-build && ./run.py --topo l4lb_dsr --feature l4lb --test DSR_BRIDGED_NW_NW

pull-assets:
	make -C .. pull-assets
