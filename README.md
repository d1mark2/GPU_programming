# GPU_programming

We use metal-cpp for solving problems. If Metal.hpp doesn't work -> https://developer.apple.com/metal/cpp/

```shell
# how to start
$ make -C brut_sha_256/cpu
$ make -C brut_sha_256/gpu
$ make -C determinant/cpu
$ make -C determinant

# to find determinant
$ cd determinant/cpu && ./det_cpu.out 
$ cd determinant && ./det_gpu.out 

# to get hash
./brut_sha_256/cpu/sha_256 password

# to brutforce
$ cd brut_sha_256/cpu && ./brut_sha_256.out HASH
$ cd brut_sha_256/gpu && ./brut_sha_256.out HASH
```
