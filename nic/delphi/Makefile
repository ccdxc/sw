# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

# This is a dummy target to build all delphi componentes
MODULE_PREREQS = client.proto delphi_client_test.gtest delphictl.bin delphi_example.bin \
                 delphi_example_test.gtest delphi_hub.bin delphi_hub_test.gtest \
                 delphi_kvstore_test.gtest delphi_messenger_test.gtest delphi_metrics_test.gtest \
                 delphi.proto delphi_server_test.gtest delphi_shm_test.gtest delphi_slab_test.gtest \
                 delphi_transport_test.gtest delphi_utils_test.gtest example.proto libdelphiclientproto.a \
                 libdelphiexampleproto.a libdelphimessenger.a libdelphiproto.a libdelphisdk.so \
                 libdelphishm.a libdelphiutils.a libmessengerproto.a messenger.proto


default:
	cd .. && make ${MODULE_PREREQS}
