#! /bin/bash

set -e

export ZMQ_SOC_DIR=/sw/nic/
cd /sw/nic/
./tools/start-model.sh > /dev/null 2>&1 &
export MODEL_PID=$!

./tools/start-hal.sh > /dev/null 2>&1 &
export HAL_PID=$!

./agent/netagent/scripts/wait-for-hal.sh

#./tools/start-fake-nicmgr.sh > /dev/null 2>&1
bazel run //nic/hal/test:fake_nic_mgr

./tools/start-agent.sh > /dev/null 2>&1 & export AGENT_PID=$!

cd /sw/dol
./main.py --agent $* || status=$?

kill -9 `pgrep cap_model`
kill -9 `pgrep netagent`
kill -9 `pgrep hal`

cd /sw/nic/
exit $status
