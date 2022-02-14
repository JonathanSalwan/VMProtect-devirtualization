#!/bin/bash

gcc samples-source/sample1.c -no-pie -Wl,-rpath,\$ORIGIN,-rpath-link,./ -I ./vmp_sdk -L ./vmp_sdk -lVMProtectSDK64 -m64 -o binaries/sample1.bin && echo sample 1 OK!
gcc samples-source/sample2.c -no-pie -Wl,-rpath,\$ORIGIN,-rpath-link,./ -I ./vmp_sdk -L ./vmp_sdk -lVMProtectSDK64 -m64 -o binaries/sample2.bin && echo sample 2 OK!
gcc samples-source/sample3.c -no-pie -Wl,-rpath,\$ORIGIN,-rpath-link,./ -I ./vmp_sdk -L ./vmp_sdk -lVMProtectSDK64 -m64 -o binaries/sample3.bin && echo sample 3 OK!
gcc samples-source/sample4.c -no-pie -Wl,-rpath,\$ORIGIN,-rpath-link,./ -I ./vmp_sdk -L ./vmp_sdk -lVMProtectSDK64 -m64 -o binaries/sample4.bin && echo sample 4 OK!
gcc samples-source/sample5.c -no-pie -Wl,-rpath,\$ORIGIN,-rpath-link,./ -I ./vmp_sdk -L ./vmp_sdk -lVMProtectSDK64 -m64 -o binaries/sample5.bin && echo sample 5 OK!
gcc samples-source/sample6.c -no-pie -Wl,-rpath,\$ORIGIN,-rpath-link,./ -I ./vmp_sdk -L ./vmp_sdk -lVMProtectSDK64 -m64 -o binaries/sample6.bin && echo sample 6 OK!
