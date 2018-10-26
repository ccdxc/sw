# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

# This is a dummy target to build all delphi componentes
MODULE_PREREQS = client.proto delphi_client_test.gtest delphictl.bin \
                 delphi_example.bin delphi_example_test.gtest delphi_hub.bin \
                 delphi_hub_test.gtest delphi_kvstore_test.gtest \
                 delphi_messenger_test.gtest delphi_metrics_test.gtest \
                 delphi.proto delphi_server_test.gtest delphi_shm_test.gtest \
                 delphi_slab_test.gtest delphi_transport_test.gtest \
                 delphi_utils_test.gtest delphi_example.proto libdelphiclientproto.a \
                 libdelphiexampleproto.a libdelphimessenger.a libdelphiproto.a \
                 libdelphisdk.so libdelphishm.a libdelphiutils.a \
                 libmessengerproto.a messenger.proto protoc-gen-delphi.gobin

MODULE_TEST_TARGETS = delphi_example_test delphi_hub_test delphi_server_test \
                      delphi_messenger_test delphi_transport_test \
                      delphi_client_test delphi_metrics_test \
                      delphi_kvstore_test delphi_shm_test delphi_slab_test \
                      delphi_utils_test

default: build

build:
	cd .. && make ${MODULE_PREREQS}

test: build
	for tgt in $(MODULE_TEST_TARGETS); do \
		if ! ../build/x86_64/iris/bin/$$tgt; then \
			echo "============= $$tgt Test failed ========================"; \
			exit 1; \
		 fi; \
	done

