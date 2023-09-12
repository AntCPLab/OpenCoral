# Run Mnist Training Exp
```
cd deep-mpc
./download.sh
./convert.sh
cd ..
# To run `coral` on network `B`, with `1` threads, for `1` epoch, with `16`-bit precision.
./deep-mpc/run-local.sh coral B near 1 1 16
```

# Run Deep Net Inference

## SqueezeNet
```
cd benchmarks/EzPC/Athos/Networks/SqueezeNetImgNet
#axel -a -n 5 -c --output ./ https://github.com/avoroshilov/tf-squeezenet/raw/master/sqz_full.mat
pip3 install numpy scipy pillow>=9.1 tensorflow
python3 squeezenet_main.py --in ./SampleImages/n02109961_36.JPEG --saveTFMetadata True
python3 squeezenet_main.py --in ./SampleImages/n02109961_36.JPEG --scalingFac 12 --saveImgAndWtData True
cd ../../../../..
Scripts/fixed-rep-to-float.py benchmarks/EzPC/Athos/Networks/SqueezeNetImgNet/SqNetImgNet_img_input.inp
# 64-bit ring (-R 64), use edabits (-Y), 8 threads
./compile.py -R 64 -Y tf benchmarks/EzPC/Athos/Networks/SqueezeNetImgNet/graphDef.bin 8
Scripts/spdz2k.sh tf-benchmarks_EzPC_Athos_Networks_SqueezeNetImgNet_graphDef.bin-8
```

# Benchmarking
We can use insecure preprocessing to generate necessary randomness data on disk, in order to analyze the cost of offline vs online.

## SqueezeNet

### Spdz2k
- Add `MY_CFLAGS = -DINSECURE` to `CONFIG.mine`.
- Make sure `Compiler.GC.types.bits.unit = 64`
```
make clean
make Fake-Offline.x spdz2k-party.x -j8
# The '-e' option specifies the edabit length types we want to generate. Here they correspond to those used in SqueezeNet.
./Fake-Offline.x 2 -Z 64 -S 64 -e 9,12,31,32,64
Scripts/spdz2k.sh -F tf-benchmarks_EzPC_Athos_Networks_SqueezeNetImgNet_graphDef.bin-8
```

### Coral
To evaluate Coral, we need to:
- Add `MY_CFLAGS = -DINSECURE -DRMFE_UNIT` to `CONFIG.mine`.
- enable the packing feature by passing the `--pack` option to the compiler.
- Change the `Compiler.GC.types.bits.unit`: from `unit = 64` to `unit = 12` (changing this elsewhere, e.g., in the *.mpc file, is always not early enough because the bits class is already used during importing, such as the code line `sbitfix.set_precision(16, 31)` in `Compiler/GC/types/bits.py`).
```
make clean
make Fake-Offline.x coral-party.x -j8
./Fake-Offline.x 2 -C 64 -S 64 -e 9,12,31,32,64
./compile.py -R 64 -Y --pack tf benchmarks/EzPC/Athos/Networks/SqueezeNetImgNet/graphDef.bin 8
Scripts/coral.sh -F -v tf-benchmarks_EzPC_Athos_Networks_SqueezeNetImgNet_graphDef.bin-8
```

## AES

### Spdz2k / Tinier
- Add `MY_CFLAGS = -DINSECURE` to `CONFIG.mine`.
- Make sure `Compiler.GC.types.bits.unit = 64`
```
./compile.py aes_circuit
./Scripts/spdz2k.sh aes_circuit
# OR
./Scripts/tinier.sh aes_circuit
```

### Coral
- Add `MY_CFLAGS = -DINSECURE -DRMFE_UNIT` to `CONFIG.mine`.
- Change the `Compiler.GC.types.bits.unit`: from `unit = 64` to `unit = 12` (changing this elsewhere, e.g., in the *.mpc file, is always not early enough because the bits class is already used during importing, such as the code line `sbitfix.set_precision(16, 31)` in `Compiler/GC/types/bits.py`).
```
./compile.py aes_circuit_rmfe
./Scripts/coral.sh aes_circuit_rmfe
```


# Compile Options
- `RMFE_UNIT`
- `SPDZ2K_SP`
- `BENCHMARK_MASCOT_APPROACH`