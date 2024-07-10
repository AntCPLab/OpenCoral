# Coral: Maliciously Secure Computation Framework for Packed and Mixed Circuits

This is the source code for our work in CCS 2024, named ***Coral: Maliciously Secure Computation Framework for Packed and Mixed Circuits***.

Our implementation is based on the [MP-SPDZ](https://github.com/data61/MP-SPDZ/) library, so this repository is a heavily modified fork. 


## General Information About the Code Structure
### RMFE/MFE Math Implementation
This part is located at:
```
# Baseline implementation that supports fields and vectors of any length
./Math/mfe.h
./Math/mfe.cpp

# Optimized implementation that targets our parameter choices in MPC applications
./Math/mfe64.h
./Math/mfe64.cpp

# Tests
./test/test_mfe.cpp
./test/test_mfe64.cpp
```

#### Usage
The above code is heavily documented, especially for the 64-bit optimized version. For a quick start, please check the following functions in `mfe64.h`:
```
get_composite_gf2_rmfe64_type2
get_composite_gf2_rmfe64_type1_type2
get_composite_gf2_mfe64
```

### Coral Protocols
Following the tradition in MP-SPDZ library, we create top-level parties for mixed-circuit computation at:
```
# Coral paired with SPDZ2k
./Machines/coral-party.cpp

# Coral paired with Lowgear
./Machines/corallowgear-party.cpp

# Coral paired with Mascot
./Machines/coralmascot-party.cpp
```

If we want to simply use the RMFE-based boolean part, we can just run `coral-party.cpp` on the boolean circuit.

#### Usage
Check the test cases and microbenchmarks we create for boolean computation here:
```
./test/test_rmfe_beaver.cpp
./test/test_offline_binary.cpp
./Utils/coral-dabit-example.cpp
./Utils/coral-edabit-example.cpp
./Utils/binary-offline-throughput.cpp
./Utils/mixed-offline-throughput.cpp
./Utils/mixed-offline-performance.cpp
```

## (R)MFE Tests

Run the following to compile and test the 64-bit implementation:
```bash
make test_mfe64
./test_mfe64
```

Run the following to compile and test the general implementation:
```bash
make test_mfe
./test_mfe
```

## Compile and Run Coral
The codebase is only tested on Ubuntu. We will have to follow the instructions in MP-SPDZ in order to compile the library. 

Install the toolchain:
```bash
sudo apt-get install automake build-essential clang cmake git libboost-dev libboost-iostreams-dev libboost-thread-dev libgmp-dev libntl-dev libsodium-dev libssl-dev libtool python3
```

As an example, we compile and run Coral (paired with SPDZ2k) for Lenet inference below:
```bash
# Compile libraries
make setup
make -j8 coral-party.x

# Download the NN examples from EzPC
git submodule update --init benchmarks/EzPC || git clone https://github.com/zicofish/MP-SPDZ-EzPC benchmarks/EzPC
cd benchmarks/EzPC/Athos/Networks/Lenet
# Just to train a usable Lenet model (NOT fully trained on the entire mnist dataset)
python3 LenetSmall_mnist_train.py
python3 LenetSmall_mnist_inference.py 1
cd ../../../../..

# Convert input representation for MP SPDZ
Scripts/fixed-rep-to-float.py benchmarks/EzPC/Athos/Networks/Lenet/LenetSmall_mnist_img_1.inp

# Compile to instructions: 64-bit ring (-R 64), use edabits (-Y), enable packing (necessary for Coral), 8 threads
./compile.py -R 64 -Y --pack tf benchmarks/EzPC/Athos/Networks/Lenet/graphDef.bin 8

# Run the inference for Lenet with Coral protocols
Scripts/coral.sh -F -v tf-benchmarks_EzPC_Athos_Networks_Lenet_graphDef.bin-8
```

### Tests and Benchmarking
There are a list of tests and benchmarks that we have coded to evaluate the performance of Coral (and also other frameworks).

For example, to run the benchmark for Coral's binary offline throughput:
```bash
# Compile
make setup
make -j8 binary-offline-throughput
# 16 threads, coral protocol, generate inputs
./binary-offline-throughput -p 0 -x 16 --prot coral --type inputs
./binary-offline-throughput -p 1 -x 16 --prot coral --type inputs
```

## More on Configuration
Because Coral operates in a packing mode, we have made a series of edits to MP-SDPZ's frontend compiler and backend protocols. In order to compare Coral with other frameworks in MP-SPDZ, we need to manually change to different configurations like below:

### To Run Other Frameworks in MP-SPDZ (e.g., SPDZ2k, Mascot, Lowgear)
Make sure `CONFIG.mine` has:
```
MY_CFLAGS = -DINSECURE -DPRINT_PROGRESS
```
Make sure (in `Compiler/GC/types.py`):
```python
Compiler.GC.types.bits.unit = 64
```

### To Run Coral (e.g., coral, corallowgear, coralmascot)
Make sure `CONFIG.mine` has:
```
MY_CFLAGS = -DINSECURE -DRMFE_UNIT -DUSE_SILENT_OT -DPRINT_PROGRESS
```
Make sure (in `Compiler/GC/types.py`):
```python
Compiler.GC.types.bits.unit = 14
```
Finally, enable the packing feature by passing the `--pack` option to the MP-SDPZ's compiler when calling `./compile.py`.


## Other Experiments
All experiments that we show in the paper are listed in `Experiment.md`.