#
# Copyright (c) 2018, Pensando Systems Inc.
#

.PHONY: test test.enc test.dec

test: test.enc test.dec

test.enc:
	$(Q)./run_enc_tests
	$(Q)./verify_enc_tests

test.dec:
	$(Q)./run_dec_tests
