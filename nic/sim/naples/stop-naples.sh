#!/bin/bash

echo "Stopping NAPLES services/processes ..."
pkill cap_model && echo "Stopped NAPLES model" || echo "NAPLES model not running"
pkill hal && echo "Stopped HAL" || echo "HAL not running"
pkill netagent && echo "Stopped netagent" || echo "netagent not running"
pkill nic_infra_hntap && echo "Stopped hntap" || echo "hntap not running"
echo "All NAPLES services/processes stopped ..."
