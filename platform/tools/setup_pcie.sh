export LD_LIBRARY_PATH=/naples/platform/lib/:$LD_LIBRARY_PATH

./pciemgrd <<-EOF
add eth
finalize
exit
EOF

./setpci -s 01:00.0 secondary_bus=3
./setpci -s 01:00.0 subordinate_bus=3
./lspci
