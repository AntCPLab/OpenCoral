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

Add `MY_CFLAGS = -DINSECURE` to `CONFIG.mine`.
```
make clean
make Fake-Offline.x -j8
# The '-e' option specifies the edabit length types we want to generate. Here they correspond to those used in SqueezeNet.
./Fake-Offline.x 2 -Z 64 -S 64 -e 9,12,31,32,64
Scripts/spdz2k.sh -F tf-benchmarks_EzPC_Athos_Networks_SqueezeNetImgNet_graphDef.bin-8
```
