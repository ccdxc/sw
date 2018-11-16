#
# Copyright (c) 2017, Pensando Systems Inc.
#

LCLEANFILES += dotest.out

test: dotest.out
	$(Q)if cmp -s dotest.exp dotest.out; then \
		echo Test passed; \
		$(RM) dotest.out; \
	else \
		echo Test FAILED; \
		exit 1; \
	fi

dotest.out: $(GENLIB) $(GENBIN)/$(PROGRAM)
	$(Q)./dotest $(GENLIB) $(GENBIN)/$(PROGRAM) > dotest.out 2>&1
