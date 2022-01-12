# LRU-TX
The code in CPU version can run directly with a random dataset "udp.txt" in "Release Mode", which can show the throughput of LRU-TX.

This repository includes a part of DPDK-20.11-stable version, it cannot compile unless it is put in the DPDK directory. This repository was made only for convenient, to prevent cloning the entire DPDK repo and load them to the code editor. In a nutshell, LRU-TX achieves per-flow traffic monitoring at nearly 14.88Mpps (i.e., 10 Gpbs line rate fully loaded with minimum size packets, 64B) with negligible packet losses (just a few packets per million) using a minimal amount of CPU resources.

Our LRU-TX test is based on I/O mode of testpmd. Please replace the files in folder testpmd in the origional with our version, which includes codes of LRU-TX. Then, compile the whole DPDK following the standard installation process of DPDK. Last, the LRU-TX test can be done with a line rate packet generator (e.g., PktGen-21.03.0 in our experiment). For convenience, we have attached a video (i.e., LRU-TX-DPDK.mp4) of the operation process of the test for the DPDK version. Besides, we DIY a command "lru" to show the latest 20 flows in LRU-TX, in order to testify it.

Besides, this repository also includes a part of ovs, it cannot compile unless it is put in the ovs directory. This repository was made only for convenient, to prevent cloning the entire ovs repo and load them to the code editor. Compilation environment is ubuntu 16.04 and openvswitch-2.10.2. The LRU-TX is integrated into the file datapath.c.
