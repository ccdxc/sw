/* File : p4pd.i */
#define __attribute__(x)

%module p4pd
%{
#include <stdint.h>
#include "nic/include/hal_pd_error.hpp"
#include "nic/build/iris/gen/datapath/p4/include/p4pd.h"
#include "swig.h"
%}

%include <stdint.i>
%include "../../include/hal_pd_error.hpp"
%include "../../build/iris/gen/datapath/p4/include/p4pd.h"
%include "ccfiles/swig.h"
