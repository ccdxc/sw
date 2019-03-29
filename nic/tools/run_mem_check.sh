#!/bin/bash

echo `date` >> /data/mem_track_out
echo 'Hal' >> /data/mem_track_out
cat /proc/$(pidof hal)/status >> /data/mem_track_out
echo -en '\n' >> /data/mem_track_out

echo 'nicmgrd' >> /data/mem_track_out
cat /proc/$(pidof nicmgrd)/status >> /data/mem_track_out
echo -en '\n' >> /data/mem_trace_out

echo 'free -m' >> /data/mem_trace_out
free -m >> /data/mem_track_out
echo '============================================================' >> /data/mem_track_out

