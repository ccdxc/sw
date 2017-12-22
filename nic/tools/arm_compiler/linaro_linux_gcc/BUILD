package(default_visibility = ["//visibility:public"])

filegroup(
  name = 'gcc',
  srcs = [
    '@linaro_linux_gcc_repo//:gcc',
    'aarch64-linux-gnu-gcc',
  ],
)

filegroup(
  name = 'ar',
  srcs = [
    '@linaro_linux_gcc_repo//:ar',
    'aarch64-linux-gnu-ar',
  ],
)

filegroup(
  name = 'ld',
  srcs = [
    '@linaro_linux_gcc_repo//:ld',
    'aarch64-linux-gnu-ld',
  ],
)

filegroup(
  name = 'nm',
  srcs = [
    '@linaro_linux_gcc_repo//:nm',
    'aarch64-linux-gnu-nm',
  ],
)

filegroup(
  name = 'objcopy',
  srcs = [
    '@linaro_linux_gcc_repo//:objcopy',
    'aarch64-linux-gnu-objcopy',
  ],
)

filegroup(
  name = 'objdump',
  srcs = [
    '@linaro_linux_gcc_repo//:objdump',
    'aarch64-linux-gnu-objdump',
  ],
)

filegroup(
  name = 'strip',
  srcs = [
    '@linaro_linux_gcc_repo//:strip',
    'aarch64-linux-gnu-strip',
  ],
)

filegroup(
  name = 'as',
  srcs = [
    '@linaro_linux_gcc_repo//:as',
    'aarch64-linux-gnu-as',
  ],
)

filegroup(
  name = 'tool-wrappers',
  srcs = [
    ':gcc',
    ':ar',
    ':ld',
    ':nm',
    ':objcopy',
    ':objdump',
    ':strip',
    ':as',
  ],
)
