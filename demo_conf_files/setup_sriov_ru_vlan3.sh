#!/bin/bash
set -e
IF=enP2p1s0f0np0
sudo ip link set $IF vf 3 mac 00:11:22:33:64:67 vlan 3 qos 0 spoofchk off mtu 9600
sudo ip link set $IF vf 4 mac 00:11:22:33:64:66 vlan 3 qos 0 spoofchk off mtu 9600
sleep 1
sudo /home/oaicicd/test_dir/dpdk-stable-20.11.9/bin/dpdk-devbind.py --unbind 0002:01:00.5
sudo /home/oaicicd/test_dir/dpdk-stable-20.11.9/bin/dpdk-devbind.py --unbind 0002:01:00.6
sudo modprobe mlx5_core
sudo /home/oaicicd/test_dir/dpdk-stable-20.11.9/bin/dpdk-devbind.py --bind mlx5_core 0002:01:00.5
sudo /home/oaicicd/test_dir/dpdk-stable-20.11.9/bin/dpdk-devbind.py --bind mlx5_core 0002:01:00.6
echo "RU SR-IOV setup done"
