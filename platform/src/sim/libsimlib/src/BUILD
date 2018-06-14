package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license


cc_import(
     name = "libsimlib",
     shared_library = "libsimlib.so",
)


cc_binary(
        name = "libsimlib.so",
        srcs = [
                 "simclient.c",
                 "simreadwrite.c",
                 "simserver.c",
                 "simsocket.c",
                 "simlib_impl.h",
        ],
        copts = [ 
                  "-g",
                  "-Iplatform/src/sim/libsimlib/include",
        ],
        deps = [
                "//platform/src/sim/libsimlib/include:libsimlib_hdrs",
        ],
        linkshared = True,
)

