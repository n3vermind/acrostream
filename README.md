# acrostream
A POC video streamer for a ssc338q camera optimized for FPV flying.

Current settings worked well for me when used with wfb-ng using MCS 2, STBC and LDPC on a 20Hz channel.

Tested on a Runcam Wifilink 2.


## Building
```
BUILDROOT=path_to_openipc_buildroot cmake .
make
```
