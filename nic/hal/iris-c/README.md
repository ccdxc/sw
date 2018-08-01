iris-c:
A library for nic manager to interact with HAL. It exposes generic APIs which are valid in all the modes which HAL will be running.
This will enable nic manager to be agnostic to the HAL modes.

How to run tests:
# Start Model
nic# ./tools/start-model.sh
# Start HAL
nic# ./tools/start-hal.sh
# Run Tests
nic# ../bazel-bin/nic/hal/iris-c/gtest/filter_test
# With one command:
./run.py --filter_gtest
