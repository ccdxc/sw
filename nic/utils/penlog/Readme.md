# Usage

## From a binary

### With Delphi integration

First include `nic/utils/penlog/lib/penlog.hpp` in your source file.

Then, immediatelly after initializing the delphi sdk call `logger_init` with the 
delphi handler and the name of your process. This will return a `LoggerPtr` 
handle than you can use to log events like so: 
`logger->debug("Var is: {}", var);`.

Levels supported are: "trace", "debug", "info", "warn", "error", "critical".

To change the level us the `penlog.bin` binary like so: 
`penlog <process_name> <debug_level>`

The default logging level is "info", which mean anything from "info" and above
will be logged.

### Without Delphi

Todo

## From a library

First include `nic/utils/penlog/lib/penlog.hpp` in your source file.

Then call `logger_init_for_lib` with the name of your library. This will return
a `LoggerPtr` handle that you can use to log events as described in the 
"From a binary" section.

*Do NOT use `log()` from a libary. This will return the main, ie. binary, logger
and your logs will have the wrong prefix.

