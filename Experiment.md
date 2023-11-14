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

## SqueezeNet (15 layers)
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

## Lenet (5 layers)
```
cd benchmarks/EzPC/Athos/Networks/Lenet
# Just to train a usable model (NOT fully trained on the entire mnist dataset)
python3 LenetSmall_mnist_train.py
python3 LenetSmall_mnist_inference.py 1
cd ../../../../..
Scripts/fixed-rep-to-float.py benchmarks/EzPC/Athos/Networks/Lenet/LenetSmall_mnist_img_1.inp
# 64-bit ring (-R 64), use edabits (-Y), 8 threads
./compile.py -R 64 -Y tf benchmarks/EzPC/Athos/Networks/Lenet/graphDef.bin 8
Scripts/spdz2k.sh tf-benchmarks_EzPC_Athos_Networks_Lenet_graphDef.bin-8
```

# Benchmarking
We can use insecure preprocessing to generate necessary randomness data on disk, in order to analyze the cost of offline vs online.

## Resnet50

Preparation:
```bash
axel -a -n 5 -c --output ./PreTrainedModel http://download.tensorflow.org/models/official/20181001_resnet/savedmodels/resnet_v2_fp32_savedmodel_NHWC.tar.gz
cd PreTrainedModel && tar -xvzf resnet_v2_fp32_savedmodel_NHWC.tar.gz && cd ..
python3 ResNet_main.py --runPrediction True --scalingFac 12 --saveImgAndWtData True
```

### Spdz2k
- Add `MY_CFLAGS = -DINSECURE` to `CONFIG.mine`.
- Make sure `Compiler.GC.types.bits.unit = 64`
```
make clean
make Fake-Offline.x spdz2k-party.x -j8
# The '-e' option specifies the edabit length types we want to generate. Here they correspond to those used in SqueezeNet.
./Fake-Offline.x 2 -Z 64 -S 64 -e 9,10,12,31,32,64
Scripts/fixed-rep-to-float.py benchmarks/EzPC/Athos/Networks/ResNet/ResNet_img_input.inp
./compile.py -R 64 -Y tf benchmarks/EzPC/Athos/Networks/ResNet/graphDef.bin 64
Scripts/spdz2k.sh -F -v tf-benchmarks_EzPC_Athos_Networks_ResNet_graphDef.bin-64
```

### LowGear
- Add `MY_CFLAGS = -DINSECURE` to `CONFIG.mine`.
- Make sure `Compiler.GC.types.bits.unit = 64`
```
make clean
make Fake-Offline.x lowgear-party.x -j8
# The '-e' option specifies the edabit length types we want to generate. Here they correspond to those used in SqueezeNet.
./Fake-Offline.x 2 -lgp 128 -e 9,10,12,31,32,41,64,71
Scripts/fixed-rep-to-float.py benchmarks/EzPC/Athos/Networks/ResNet/ResNet_img_input.inp
# When compiling, '-F' is the bit length of the application integers, not the bit length of the prime modulus
./compile.py -F 64 -Y tf benchmarks/EzPC/Athos/Networks/ResNet/graphDef.bin 64
Scripts/lowgear.sh -F -v tf-benchmarks_EzPC_Athos_Networks_ResNet_graphDef.bin-64
```

### Mascot
- Add `MY_CFLAGS = -DINSECURE` to `CONFIG.mine`.
- Make sure `Compiler.GC.types.bits.unit = 64`
```
make clean
make Fake-Offline.x mascot-party.x -j8
# The '-e' option specifies the edabit length types we want to generate. Here they correspond to those used in SqueezeNet.
./Fake-Offline.x 2 -lgp 128 -e 9,10,12,31,32,41,64,71
Scripts/fixed-rep-to-float.py benchmarks/EzPC/Athos/Networks/ResNet/ResNet_img_input.inp
# When compiling, '-F' is the bit length of the application integers, not the bit length of the prime modulus
./compile.py -F 64 -Y tf benchmarks/EzPC/Athos/Networks/ResNet/graphDef.bin 64
Scripts/mascot.sh -F -v tf-benchmarks_EzPC_Athos_Networks_ResNet_graphDef.bin-64
```


### Coral
To evaluate Coral, we need to:
- Add `MY_CFLAGS = -DINSECURE -DRMFE_UNIT` to `CONFIG.mine`.
- enable the packing feature by passing the `--pack` option to the compiler.
- Change the `Compiler.GC.types.bits.unit`: from `unit = 64` to `unit = 12` (changing this elsewhere, e.g., in the *.mpc file, is always not early enough because the bits class is already used during importing, such as the code line `sbitfix.set_precision(16, 31)` in `Compiler/GC/types/bits.py`).
- Printing has a large effect on the benchmarking. Remember to turn off them for accurate benchmark.
```
make clean
make Fake-Offline.x coral-party.x -j8
./Fake-Offline.x 2 -C 64 -S 64 -e 9,12,31,32,64
Scripts/fixed-rep-to-float.py benchmarks/EzPC/Athos/Networks/ResNet/ResNet_img_input.inp
./compile.py -R 64 -Y --pack tf benchmarks/EzPC/Athos/Networks/ResNet/graphDef.bin 64
Scripts/coral.sh -F -v tf-benchmarks_EzPC_Athos_Networks_ResNet_graphDef.bin-64
```

### Coral-Lowgear
To evaluate Coral, we need to:
- Add `MY_CFLAGS = -DINSECURE -DRMFE_UNIT` to `CONFIG.mine`.
- enable the packing feature by passing the `--pack` option to the compiler.
- Change the `Compiler.GC.types.bits.unit`: from `unit = 64` to `unit = 12` (changing this elsewhere, e.g., in the *.mpc file, is always not early enough because the bits class is already used during importing, such as the code line `sbitfix.set_precision(16, 31)` in `Compiler/GC/types/bits.py`).
- Printing has a large effect on the benchmarking. Remember to turn off them for accurate benchmark.
```
make clean
make corallowgear-party.x -j8
Scripts/fixed-rep-to-float.py benchmarks/EzPC/Athos/Networks/ResNet/ResNet_img_input.inp
./compile.py -F 64 -Y --pack tf benchmarks/EzPC/Athos/Networks/ResNet/graphDef.bin 64
Scripts/corallowgear.sh -b 10000 -v tf-benchmarks_EzPC_Athos_Networks_ResNet_graphDef.bin-64
```

## SqueezeNet

### Spdz2k
- Add `MY_CFLAGS = -DINSECURE` to `CONFIG.mine`.
- Make sure `Compiler.GC.types.bits.unit = 64`
```
make clean
make Fake-Offline.x spdz2k-party.x -j8
# The '-e' option specifies the edabit length types we want to generate. Here they correspond to those used in SqueezeNet.
./Fake-Offline.x 2 -Z 64 -S 64 -e 9,12,31,32,64
Scripts/fixed-rep-to-float.py benchmarks/EzPC/Athos/Networks/SqueezeNetImgNet/SqNetImgNet_img_input.inp
./compile.py -R 64 -Y tf benchmarks/EzPC/Athos/Networks/SqueezeNetImgNet/graphDef.bin 8
Scripts/spdz2k.sh -F -v tf-benchmarks_EzPC_Athos_Networks_SqueezeNetImgNet_graphDef.bin-8
```

### Coral
To evaluate Coral, we need to:
- Add `MY_CFLAGS = -DINSECURE -DRMFE_UNIT` to `CONFIG.mine`.
- enable the packing feature by passing the `--pack` option to the compiler.
- Change the `Compiler.GC.types.bits.unit`: from `unit = 64` to `unit = 12` (changing this elsewhere, e.g., in the *.mpc file, is always not early enough because the bits class is already used during importing, such as the code line `sbitfix.set_precision(16, 31)` in `Compiler/GC/types/bits.py`).
- Printing has a large effect on the benchmarking. Remember to turn off them for accurate benchmark.
```
make clean
make Fake-Offline.x coral-party.x -j8
./Fake-Offline.x 2 -C 64 -S 64 -e 9,12,31,32,64
./compile.py -R 64 -Y --pack tf benchmarks/EzPC/Athos/Networks/SqueezeNetImgNet/graphDef.bin 8
Scripts/coral.sh -F -v tf-benchmarks_EzPC_Athos_Networks_SqueezeNetImgNet_graphDef.bin-8
```

### Coral-Lowgear
To evaluate Coral, we need to:
- Add `MY_CFLAGS = -DINSECURE -DRMFE_UNIT` to `CONFIG.mine`.
- enable the packing feature by passing the `--pack` option to the compiler.
- Change the `Compiler.GC.types.bits.unit`: from `unit = 64` to `unit = 12` (changing this elsewhere, e.g., in the *.mpc file, is always not early enough because the bits class is already used during importing, such as the code line `sbitfix.set_precision(16, 31)` in `Compiler/GC/types/bits.py`).
- Printing has a large effect on the benchmarking. Remember to turn off them for accurate benchmark.
```
make clean
make corallowgear-party.x -j8
Scripts/fixed-rep-to-float.py benchmarks/EzPC/Athos/Networks/SqueezeNetImgNet/SqNetImgNet_img_input.inp
./compile.py -F 64 -Y --pack tf benchmarks/EzPC/Athos/Networks/SqueezeNetImgNet/graphDef.bin 8
Scripts/corallowgear.sh -b 10000 -v tf-benchmarks_EzPC_Athos_Networks_SqueezeNetImgNet_graphDef.bin-8
```

## Lenet

### Spdz2k
- Add `MY_CFLAGS = -DINSECURE` to `CONFIG.mine`.
- Make sure `Compiler.GC.types.bits.unit = 64`
```
make clean
make Fake-Offline.x spdz2k-party.x -j8
# The '-e' option specifies the edabit length types we want to generate. Here they correspond to those used in Lenet.
./Fake-Offline.x 2 -Z 64 -S 64 -e 1,2,3,4,5,9,11,12,15,16,30,31,32,33,47,63,64
Scripts/fixed-rep-to-float.py benchmarks/EzPC/Athos/Networks/Lenet/LenetSmall_mnist_img_1.inp
./compile.py -R 64 -Y tf benchmarks/EzPC/Athos/Networks/Lenet/graphDef.bin 8
Scripts/spdz2k.sh -F -v tf-benchmarks_EzPC_Athos_Networks_Lenet_graphDef.bin-8
```

### Coral
To evaluate Coral, we need to:
- Add `MY_CFLAGS = -DINSECURE -DRMFE_UNIT` to `CONFIG.mine`.
- enable the packing feature by passing the `--pack` option to the compiler.
- Change the `Compiler.GC.types.bits.unit`: from `unit = 64` to `unit = 12` (changing this elsewhere, e.g., in the *.mpc file, is always not early enough because the bits class is already used during importing, such as the code line `sbitfix.set_precision(16, 31)` in `Compiler/GC/types/bits.py`).
- Printing has a large effect on the benchmarking. Remember to turn off them for accurate benchmark.
```
make clean
make Fake-Offline.x coral-party.x -j8
./Fake-Offline.x 2 -C 64 -S 64 -e 1,2,3,4,5,9,11,12,15,16,30,31,32,33,47,63,64
Scripts/fixed-rep-to-float.py benchmarks/EzPC/Athos/Networks/Lenet/LenetSmall_mnist_img_1.inp
./compile.py -R 64 -Y --pack tf benchmarks/EzPC/Athos/Networks/Lenet/graphDef.bin 8
Scripts/coral.sh -F -v tf-benchmarks_EzPC_Athos_Networks_Lenet_graphDef.bin-8
# To use only fake arithmetic prep data, run:
# Scripts/coral.sh -AF -v tf-benchmarks_EzPC_Athos_Networks_Lenet_graphDef.bin-8
```

### Coral-Lowgear
To evaluate Coral-Lowgear, we need to:
- Add `MY_CFLAGS = -DINSECURE -DRMFE_UNIT` to `CONFIG.mine`.
- enable the packing feature by passing the `--pack` option to the compiler.
- Change the `Compiler.GC.types.bits.unit`: from `unit = 64` to `unit = 12` (changing this elsewhere, e.g., in the *.mpc file, is always not early enough because the bits class is already used during importing, such as the code line `sbitfix.set_precision(16, 31)` in `Compiler/GC/types/bits.py`).
- Printing has a large effect on the benchmarking. Remember to turn off them for accurate benchmark.
```
make clean
make corallowgear-party.x -j8
Scripts/fixed-rep-to-float.py benchmarks/EzPC/Athos/Networks/Lenet/LenetSmall_mnist_img_1.inp
./compile.py -F 64 -Y --pack tf benchmarks/EzPC/Athos/Networks/Lenet/graphDef.bin 8
Scripts/corallowgear.sh -b 10000 -v tf-benchmarks_EzPC_Athos_Networks_Lenet_graphDef.bin-8
```

## LenetLarge

### Spdz2k
- Add `MY_CFLAGS = -DINSECURE` to `CONFIG.mine`.
- Make sure `Compiler.GC.types.bits.unit = 64`
```
make clean
make Fake-Offline.x spdz2k-party.x -j8
# The '-e' option specifies the edabit length types we want to generate. Here they correspond to those used in Lenet.
./Fake-Offline.x 2 -Z 64 -S 64 -e 1,2,3,4,5,9,11,12,15,16,30,31,32,33,47,63,64
Scripts/fixed-rep-to-float.py benchmarks/EzPC/Athos/Networks/LenetLarge/LenetLarge_mnist_img_1.inp
./compile.py -R 64 -Y tf benchmarks/EzPC/Athos/Networks/LenetLarge/graphDef.bin 8
Scripts/spdz2k.sh -F -v tf-benchmarks_EzPC_Athos_Networks_LenetLarge_graphDef.bin-8
```

### LowGear
- Add `MY_CFLAGS = -DINSECURE` to `CONFIG.mine`.
- Make sure `Compiler.GC.types.bits.unit = 64`
```
make clean
make Fake-Offline.x lowgear-party.x -j8
# The '-e' option specifies the edabit length types we want to generate. Here they correspond to those used in SqueezeNet.
./Fake-Offline.x 2 -lgp 128 -e 9,10,12,31,32,41,64,71
Scripts/fixed-rep-to-float.py benchmarks/EzPC/Athos/Networks/LenetLarge/LenetLarge_mnist_img_1.inp
# When compiling, '-F' is the bit length of the application integers, not the bit length of the prime modulus
./compile.py -F 64 -Y tf benchmarks/EzPC/Athos/Networks/LenetLarge/graphDef.bin 8
Scripts/lowgear.sh -F -v tf-benchmarks_EzPC_Athos_Networks_LenetLarge_graphDef.bin-8
```

### Coral
To evaluate Coral, we need to:
- Add `MY_CFLAGS = -DINSECURE -DRMFE_UNIT` to `CONFIG.mine`.
- enable the packing feature by passing the `--pack` option to the compiler.
- Change the `Compiler.GC.types.bits.unit`: from `unit = 64` to `unit = 12` (changing this elsewhere, e.g., in the *.mpc file, is always not early enough because the bits class is already used during importing, such as the code line `sbitfix.set_precision(16, 31)` in `Compiler/GC/types/bits.py`).
- Printing has a large effect on the benchmarking. Remember to turn off them for accurate benchmark.
```
make clean
make Fake-Offline.x coral-party.x -j8
./Fake-Offline.x 2 -C 64 -S 64 -e 1,2,3,4,5,9,11,12,15,16,30,31,32,33,47,63,64
Scripts/fixed-rep-to-float.py benchmarks/EzPC/Athos/Networks/LenetLarge/LenetLarge_mnist_img_1.inp
./compile.py -R 64 -Y --pack tf benchmarks/EzPC/Athos/Networks/LenetLarge/graphDef.bin 8
Scripts/coral.sh -F -v tf-benchmarks_EzPC_Athos_Networks_LenetLarge_graphDef.bin-8
# To use only fake arithmetic prep data, run:
# Scripts/coral.sh -AF -v tf-benchmarks_EzPC_Athos_Networks_LenetLarge_graphDef.bin-8
```

### Coral-Lowgear
To evaluate Coral-Lowgear, we need to:
- Add `MY_CFLAGS = -DINSECURE -DRMFE_UNIT` to `CONFIG.mine`.
- enable the packing feature by passing the `--pack` option to the compiler.
- Change the `Compiler.GC.types.bits.unit`: from `unit = 64` to `unit = 12` (changing this elsewhere, e.g., in the *.mpc file, is always not early enough because the bits class is already used during importing, such as the code line `sbitfix.set_precision(16, 31)` in `Compiler/GC/types/bits.py`).
- Printing has a large effect on the benchmarking. Remember to turn off them for accurate benchmark.
```
make clean
make corallowgear-party.x -j8
Scripts/fixed-rep-to-float.py benchmarks/EzPC/Athos/Networks/LenetLarge/LenetLarge_mnist_img_1.inp
./compile.py -F 64 -Y --pack tf benchmarks/EzPC/Athos/Networks/LenetLarge/graphDef.bin 8
Scripts/corallowgear.sh -b 10000 -v tf-benchmarks_EzPC_Athos_Networks_LenetLarge_graphDef.bin-8
```

## Decision Tree Prediction

### Spdz2k
- Add `MY_CFLAGS = -DINSECURE` to `CONFIG.mine`.
- Make sure `Compiler.GC.types.bits.unit = 64`
```
make clean
make Fake-Offline.x spdz2k-party.x -j8
# The '-e' option specifies the edabit length types we want to generate. Here they correspond to those used in Lenet.
./Fake-Offline.x 2 -Z 64 -S 64 -e 1,2,3,4,5,9,11,12,15,16,30,31,32,33,47,63,64
./compile.py -R 64 -Y breast_tree_predict 16
Scripts/spdz2k.sh -F -v breast_tree_predict-16
```

### Coral
To evaluate Coral, we need to:
- Add `MY_CFLAGS = -DINSECURE -DRMFE_UNIT` to `CONFIG.mine`.
- enable the packing feature by passing the `--pack` option to the compiler.
- Change the `Compiler.GC.types.bits.unit`: from `unit = 64` to `unit = 12` (changing this elsewhere, e.g., in the *.mpc file, is always not early enough because the bits class is already used during importing, such as the code line `sbitfix.set_precision(16, 31)` in `Compiler/GC/types/bits.py`).
- Printing has a large effect on the benchmarking. Remember to turn off them for accurate benchmark.
```
make clean
make Fake-Offline.x coral-party.x -j8
./Fake-Offline.x 2 -C 64 -S 64 -e 1,2,3,4,5,9,11,12,15,16,30,31,32,33,47,63,64
./compile.py -R 64 -Y --pack breast_tree_predict 16
Scripts/coral.sh -F -v breast_tree_predict-16
```

### Coral-Lowgear
To evaluate Coral, we need to:
- Add `MY_CFLAGS = -DINSECURE -DRMFE_UNIT` to `CONFIG.mine`.
- enable the packing feature by passing the `--pack` option to the compiler.
- Change the `Compiler.GC.types.bits.unit`: from `unit = 64` to `unit = 12` (changing this elsewhere, e.g., in the *.mpc file, is always not early enough because the bits class is already used during importing, such as the code line `sbitfix.set_precision(16, 31)` in `Compiler/GC/types/bits.py`).
- Printing has a large effect on the benchmarking. Remember to turn off them for accurate benchmark.
```
make clean
make corallowgear-party.x -j8
./compile.py -F 64 -Y --pack breast_tree_predict 1
Scripts/corallowgear.sh -v breast_tree_predict-1
```

## Decision Tree Training

### Spdz2k
- Add `MY_CFLAGS = -DINSECURE` to `CONFIG.mine`.
- Make sure `Compiler.GC.types.bits.unit = 64`
```
make clean
make Fake-Offline.x spdz2k-party.x -j8
# The '-e' option specifies the edabit length types we want to generate. Here they correspond to those used in Lenet.
./Fake-Offline.x 2 -Z 64 -S 64 -e 1,2,3,4,5,9,11,12,15,16,30,31,32,33,47,63,64
./compile.py -R 64 -Y breast_tree
Scripts/spdz2k.sh -F -v breast_tree
```

### Coral
To evaluate Coral, we need to:
- Add `MY_CFLAGS = -DINSECURE -DRMFE_UNIT` to `CONFIG.mine`.
- enable the packing feature by passing the `--pack` option to the compiler.
- Change the `Compiler.GC.types.bits.unit`: from `unit = 64` to `unit = 12` (changing this elsewhere, e.g., in the *.mpc file, is always not early enough because the bits class is already used during importing, such as the code line `sbitfix.set_precision(16, 31)` in `Compiler/GC/types/bits.py`).
- Printing has a large effect on the benchmarking. Remember to turn off them for accurate benchmark.
```
make clean
make Fake-Offline.x coral-party.x -j8
./Fake-Offline.x 2 -C 64 -S 64 -e 1,2,3,4,5,9,11,12,15,16,30,31,32,33,47,63,64
# `--pack` has no effect in decision tree training
./compile.py -R 64 -Y --pack breast_tree 16
Scripts/coral.sh -F -v breast_tree-16
```

### Coral-Lowgear
To evaluate Coral-Lowgear, we need to:
- Add `MY_CFLAGS = -DINSECURE -DRMFE_UNIT` to `CONFIG.mine`.
- enable the packing feature by passing the `--pack` option to the compiler.
- Change the `Compiler.GC.types.bits.unit`: from `unit = 64` to `unit = 12` (changing this elsewhere, e.g., in the *.mpc file, is always not early enough because the bits class is already used during importing, such as the code line `sbitfix.set_precision(16, 31)` in `Compiler/GC/types/bits.py`).
- Printing has a large effect on the benchmarking. Remember to turn off them for accurate benchmark.
```
make clean
make corallowgear-party.x -j8
# `--pack` has no effect in decision tree training
./compile.py -R 64 -Y --pack breast_tree 16
Scripts/corallowgear.sh -b 10000 -v breast_tree-16
```


## AES

### Spdz2k / Tinier
- Add `MY_CFLAGS = -DINSECURE` to `CONFIG.mine`.
- Make sure `Compiler.GC.types.bits.unit = 64`
```
./compile.py circuit_aes
./Scripts/spdz2k.sh circuit_aes
# OR
./Scripts/tinier.sh circuit_aes
```

### Coral
- Add `MY_CFLAGS = -DINSECURE -DRMFE_UNIT` to `CONFIG.mine`.
- Change the `Compiler.GC.types.bits.unit`: from `unit = 64` to `unit = 12` (changing this elsewhere, e.g., in the *.mpc file, is always not early enough because the bits class is already used during importing, such as the code line `sbitfix.set_precision(16, 31)` in `Compiler/GC/types/bits.py`).
```
./compile.py circuit_aes_rmfe
./Scripts/coral.sh circuit_aes_rmfe
```

## Boolean  benchmarks

### Single thread cost

```
./test_offline_binary 0 2 tiny inputs
./test_offline_binary 1 2 tiny inputs
```

### Throughput

```
# 16 threads, coral protocol, generate inputs
./binary-offline-throughput -p 0 -x 16 --prot coral --type inputs
./binary-offline-throughput -p 1 -x 16 --prot coral --type inputs
```

## Mixed  benchmarks

### Single thread cost

```
./mixed-offline-performance 0 2 coral edabit loose
./mixed-offline-performance 1 2 coral edabit loose
```

### Throughput

```
# 16 threads, coral protocol, generate edabits
./mixed-offline-throughput -p 0 -x 16 --prot coral --type strictedabit --nbits 32
./mixed-offline-throughput -p 1 -x 16 --prot coral --type strictedabit --nbits 32
```


# Compile Options
- `RMFE_UNIT`
- `USE_SILENT_OT`
- `SPDZ2K_SP`
- `BENCHMARK_MASCOT_APPROACH`
- `PRINT_PROGRESS`
- `DETAIL_BENCHMARK`
- `VERBOSE_DEBUG_PRINT`

# Others
- For better efficiency in large circuits, use `-b 10000` to enable a larger batch size = 10000, instead of the default 1000.